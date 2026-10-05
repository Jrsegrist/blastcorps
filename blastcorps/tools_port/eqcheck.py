#!/usr/bin/env python3
"""eqcheck: functional-equivalence harness for NON_MATCHING rewrites.

Runs one function from two builds (by default the matching build in build/
and the NON_MATCHING build in build_nm/) in a unicorn MIPS (R4000, 64-bit,
big-endian) emulator with identical randomized inputs, and compares the
results: return registers, callee-saved registers, every byte of memory
either version wrote, the sequence of calls it made to other functions
(which are stubbed by default) with their arguments, and MMIO accesses.

See tools_port/README.md for usage, options and limitations.
"""
import argparse
import bisect
import hashlib
import os
import random
import re
import struct
import sys
import time

try:
    from unicorn import Uc, UcError, UC_ARCH_MIPS, UC_MODE_MIPS64, UC_MODE_BIG_ENDIAN
    from unicorn import (UC_HOOK_BLOCK, UC_HOOK_CODE, UC_HOOK_INTR,
                         UC_HOOK_MEM_UNMAPPED, UC_HOOK_MEM_FETCH_UNMAPPED)
    from unicorn import mips_const as M
except ImportError:
    sys.exit("eqcheck: needs unicorn (pip install unicorn pyelftools in ~/blastcorps/.env)")
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

# ---------------------------------------------------------------------------
# memory map of the emulated machine (all KSEG0 virtual addresses)
RAM_SIZE = 0x01000000            # physical 0..16MB (N64 RDRAM is 4/8MB; rest is harness space)
MMIO_BASE, MMIO_SIZE = 0x04000000, 0x01000000   # 0xA4000000.. (SP/DP/VI/AI/PI/RI/SI regs)
HEAP_BASE, HEAP_END = 0x80B00000, 0x80D00000
STACK_TOP = 0x80E00000           # initial $sp
STACK_WINDOW = 0x10000           # bytes below $sp treated as the callee's private frame
SENTINEL = 0x80F00000            # initial $ra; reaching it means "returned"
K = 0xFFFFFFFF00000000           # sign-extension of a KSEG0 address in 64-bit mode

GPR_NAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
             "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
             "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
             "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
REG = {n: getattr(M, "UC_MIPS_REG_" + n.upper()) for n in GPR_NAMES}
REG["s8"] = REG["fp"]
for i in range(32):
    REG["f%d" % i] = getattr(M, "UC_MIPS_REG_F%d" % i)
REG["hi"], REG["lo"], REG["pc"], REG["fcsr"] = M.UC_MIPS_REG_HI, M.UC_MIPS_REG_LO, M.UC_MIPS_REG_PC, M.UC_MIPS_REG_FCSR

SAVED_REGS = ["s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "fp", "sp", "gp"] + \
             ["f%d" % i for i in range(20, 32)]
RET_KINDS = {"int": ["v0"], "void": [], "u64": ["v0", "v1"], "float": ["f0"],
             "double": ["f0", "f1"], "ptr": ["v0"]}

SKIP_SYM = re.compile(r"^(\.|L[0-9A-F]{8}|jtbl_|D_|_asmpp_|_binary_|.*_(START|END|VRAM|bin|SIZE)$|__)")


def sext32(v):
    v &= 0xFFFFFFFF
    return v | 0xFFFFFFFF00000000 if v & 0x80000000 else v


def u32(v):
    return v & 0xFFFFFFFF


def phys(v):
    return v & 0x1FFFFFFF


def kseg0(p):
    return (p & 0x1FFFFFFF) | 0x80000000


# ---------------------------------------------------------------------------
class Build:
    """Symbols and loadable sections of one build (hd_code ELF + init ELF)."""

    def __init__(self, label, elfs):
        self.label = label
        self.sections = []        # (vaddr, bytes or None(size), name, exec)
        self.sym = {}             # name -> addr (u32)
        self.addr_syms = []       # sorted (addr, name) of all named syms in loaded sections
        self.funcs = {}           # entry addr -> name (code symbols)
        self.abs_funcs = {}       # addr -> name for absolute func_ symbols (other overlays)
        self.text = []            # (lo, hi) executable ranges
        for path in elfs:
            self._load(path)
        # absolute symbols (undefined_syms*.txt assignments override object
        # definitions in the matching link) that land in our own code are
        # function entries too
        for a, n in self.abs_funcs.items():
            if self.in_text(a):
                self.funcs.setdefault(a, n)
        self.addr_syms.sort()
        self._keys = [a for a, _ in self.addr_syms]

    def _load(self, path):
        with open(path, "rb") as f:
            elf = ELFFile(f)
            secs = list(elf.iter_sections())
            for s in secs:
                if not (s["sh_flags"] & 2) or s["sh_size"] == 0:   # SHF_ALLOC
                    continue
                ex = bool(s["sh_flags"] & 4)
                data = s.data() if s["sh_type"] != "SHT_NOBITS" else None
                self.sections.append((s["sh_addr"], data if data is not None else s["sh_size"], s.name, ex))
                if ex:
                    self.text.append((s["sh_addr"], s["sh_addr"] + s["sh_size"]))
            for st in secs:
                if not isinstance(st, SymbolTableSection):
                    continue
                for sy in st.iter_symbols():
                    n, v = sy.name, sy["st_value"]
                    if not n or sy["st_info"]["type"] in ("STT_SECTION", "STT_FILE"):
                        continue
                    shndx = sy["st_shndx"]
                    if shndx == "SHN_ABS":
                        if n.startswith("func_") or (not n.startswith("D_") and 0x80000000 <= v < 0x80800000
                                                     and not SKIP_SYM.match(n)):
                            self.abs_funcs.setdefault(v, n)
                        if n not in self.sym:
                            self.sym[n] = v
                        if 0x80000000 <= v < 0xC0000000 and not n.startswith("_binary"):
                            self.addr_syms.append((v, n))
                        continue
                    if shndx == "SHN_UNDEF" or not isinstance(shndx, int):
                        continue
                    sec = secs[shndx]
                    bind = sy["st_info"]["bind"]
                    if bind == "STB_GLOBAL" or n not in self.sym:
                        self.sym[n] = v
                    if n.startswith("_binary"):
                        continue
                    self.addr_syms.append((v, n))
                    if sec["sh_flags"] & 4 and not SKIP_SYM.match(n):
                        typ = sy["st_info"]["type"]
                        if typ == "STT_FUNC" or bind == "STB_GLOBAL":
                            if v not in self.funcs or n.startswith("func_") is False:
                                self.funcs.setdefault(v, n)

    def in_text(self, a):
        return any(lo <= a < hi for lo, hi in self.text)

    def name_at(self, a):
        """'sym+0xoff' for an address (nearest preceding symbol)."""
        if HEAP_BASE <= a < HEAP_END:
            return "heap+0x%X" % (a - HEAP_BASE)
        if STACK_TOP - STACK_WINDOW <= a < STACK_TOP + 0x1000:
            return "sp%+#x" % (a - STACK_TOP)
        i = bisect.bisect_right(self._keys, a) - 1
        if i < 0:
            return "0x%08X" % a
        base, n = self.addr_syms[i]
        # prefer a non-local-label name at the same address
        j = i
        while j > 0 and self.addr_syms[j - 1][0] == base:
            j -= 1
            if not SKIP_SYM.match(self.addr_syms[j][1]) or self.addr_syms[j][1].startswith("D_"):
                n = self.addr_syms[j][1]
        off = a - base
        if off > 0x10000:
            return "0x%08X" % a
        return n if off == 0 else "%s+0x%X" % (n, off)

    def sym_off(self, a):
        i = bisect.bisect_right(self._keys, a) - 1
        if i < 0:
            return None, 0
        base, n = self.addr_syms[i]
        return n, a - base

    def resolve(self, expr):
        """'SYM', 'SYM+0x10', '0x8036444C' -> u32 address."""
        m = re.match(r"^([A-Za-z_][\w]*)?\s*([+-]\s*(?:0x[0-9A-Fa-f]+|\d+))?$", expr.strip())
        if not m:
            return int(expr, 0) & 0xFFFFFFFF
        name, off = m.group(1), m.group(2)
        if name is None:
            return int(expr, 0) & 0xFFFFFFFF
        if name not in self.sym:
            raise KeyError("%s: symbol %s not found" % (self.label, name))
        return (self.sym[name] + (int(off.replace(" ", ""), 0) if off else 0)) & 0xFFFFFFFF


# ---------------------------------------------------------------------------
class AddrMap:
    """Translate addresses of the 'new' build into the 'ref' build's space.

    Everything that did not move (the data segment, .bss, absolute symbols,
    the heap/stack) maps to itself.  Addresses inside regions that exist only
    in the new build (relocated text, .nm_extra) map through symbol names."""

    def __init__(self, ref, new):
        self.ref, self.new = ref, new
        ref_ranges = [(v, v + (len(d) if not isinstance(d, int) else d)) for v, d, _, _ in ref.sections]
        self.moved = []
        for v, d, name, ex in new.sections:
            size = len(d) if not isinstance(d, int) else d
            same = any(v == lo and v + size <= hi for lo, hi in ref_ranges) and \
                any(rv == v and rn == name for rv, _, rn, _ in ref.sections)
            if not same:
                self.moved.append((v, v + size))
        self.identity = not self.moved

    def to_ref(self, a):
        """Returns (ref_addr or None, label)."""
        if self.identity or not any(lo <= a < hi for lo, hi in self.moved):
            return a, None
        n, off = self.new.sym_off(a)
        if n is not None and n in self.ref.sym:
            return (self.ref.sym[n] + off) & 0xFFFFFFFF, None
        return None, "%s+0x%X (only in %s)" % (n, off, self.new.label)


# ---------------------------------------------------------------------------
class Machine:
    def __init__(self, build, prefill=None, opts=None):
        self.b = build
        self.opts = opts
        uc = Uc(UC_ARCH_MIPS, UC_MODE_MIPS64 | UC_MODE_BIG_ENDIAN)
        uc.ctl_set_cpu_model(M.UC_CPU_MIPS64_R4000)
        uc.mem_map(0, RAM_SIZE)
        uc.mem_map(MMIO_BASE, MMIO_SIZE)
        self.uc = uc
        self.prefill_text = []
        if prefill is not None:
            # The relocated build still references data embedded in the original
            # text range by absolute address, so give it the original bytes there.
            for v, d, name, ex in prefill.sections:
                if ex and not isinstance(d, int):
                    uc.mem_write(phys(v), d)
                    self.prefill_text.append((v, v + len(d)))
        for v, d, name, ex in build.sections:
            if isinstance(d, int):
                uc.mem_write(phys(v), b"\0" * d)
            else:
                uc.mem_write(phys(v), d)
        uc.mem_write(phys(SENTINEL), b"\0" * 16)
        self.prefill = prefill
        # Writable regions that are restored before and diffed after every run.
        # (Writes are found by diffing, not by UC_HOOK_MEM_WRITE: unicorn 2.1
        # corrupts MIPS execution when a memory hook fires for an access in a
        # branch delay slot.)
        game_end = max([v + (d if isinstance(d, int) else len(d)) for v, d, _, _ in build.sections
                        if v < 0x80800000] + [0x80400000])
        regions = [(0, phys(game_end + 0xFFF) & ~0xFFF)]
        for v, d, name, ex in build.sections:
            if v >= 0x80800000 and phys(v) < HEAP_BASE & 0x1FFFFFFF:
                size = d if isinstance(d, int) else len(d)
                regions.append((phys(v) & ~0xF, (size + 0x1F) & ~0xF))
        regions.append((phys(HEAP_BASE), HEAP_END - HEAP_BASE))
        regions.append((phys(STACK_TOP - STACK_WINDOW), STACK_WINDOW + 0x1000))
        # MMIO: SP DMEM/IMEM and the register blocks of SP/DP/DPS/MI/VI/AI/PI/RI/SI
        regions.append((0x04000000, 0x2000))
        regions.append((0x04040000, 0x100))
        regions.append((0x04080000, 0x10))
        for k in range(1, 9):
            regions.append((0x04000000 + k * 0x100000, 0x100))
        self.regions = regions
        self.base = [bytes(uc.mem_read(s, n)) for s, n in regions]
        uc.hook_add(UC_HOOK_BLOCK, self._on_block)
        uc.hook_add(UC_HOOK_INTR, self._on_intr)
        uc.hook_add(UC_HOOK_MEM_UNMAPPED | UC_HOOK_MEM_FETCH_UNMAPPED, self._on_unmapped)
        self.code_hook = None
        self.icache = {}

    # -- per-run state ------------------------------------------------------
    def reset(self):
        self.written = set()
        self.events = []          # ('call', name, args) / ('mmio_w', ...) / ('mmio_r', ...)
        self.notes = []
        self.fault = None
        self.first = True
        self.last_block = 0
        self.call_counts = {}
        self.reads = None
        self.trace_writes = None

    def in_code(self, a):
        return self.b.in_text(a) or any(lo <= a < hi for lo, hi in self.prefill_text)

    def _on_block(self, uc, addr, size, ud):
        a = u32(addr)
        self.last_block = a
        if self.opts.trace:
            print("    [%s] block %s (%d bytes)" % (self.b.label, self.b.name_at(a), size))
        if a == SENTINEL:
            return
        if self.first:
            self.first = False
            return
        name = self.b.funcs.get(a)
        target_addr = None
        if name is None:
            if self.b.in_text(a):
                return      # an ordinary block inside some function
            if self.prefill is not None and any(lo <= a < hi for lo, hi in self.prefill_text):
                # executing the original build's copy of the code (a stale pointer,
                # a jump table in a binary blob, or an absolute func_ symbol)
                oname = self.prefill.funcs.get(a)
                if oname is None or oname not in self.b.sym:
                    if ("stale", a) not in self.notes:
                        self.notes.append(("stale", a))
                    return
                name, target_addr = oname, self.b.sym[oname]
            else:
                name = self.b.abs_funcs.get(a) or "sub_%08X" % a
        if name in self.opts.follow_set or self.opts.follow_all and not name.startswith("sub_") \
                or name == self.opts.func:
            if target_addr is not None:
                uc.reg_write(REG["pc"], sext32(target_addr))
            return
        self._stub(name)

    def _stub(self, name):
        uc = self.uc
        k = self.call_counts.get(name, 0)
        self.call_counts[name] = k + 1
        args = tuple((r, u32(uc.reg_read(REG[r]))) for r in self.opts.sig_for(name))
        self.events.append(("call", name, args))
        rv = self.opts.ret_for(name, k, self)
        uc.reg_write(REG["v0"], sext32(rv[0]))
        uc.reg_write(REG["v1"], sext32(rv[1]))
        uc.reg_write(REG["f0"], rv[2])
        uc.reg_write(REG["pc"], uc.reg_read(REG["ra"]))

    # loads/stores decoded in a code hook (only for --explore / --mmio-log;
    # memory hooks are unusable, see __init__)
    LOADS = {0x20: 1, 0x21: 2, 0x22: 4, 0x23: 4, 0x24: 1, 0x25: 2, 0x26: 4, 0x27: 4,
             0x37: 8, 0x1A: 8, 0x1B: 8, 0x31: 4, 0x35: 8}
    STORES = {0x28: 1, 0x29: 2, 0x2A: 4, 0x2B: 4, 0x2E: 4, 0x3F: 8, 0x2C: 8, 0x2D: 8, 0x39: 4, 0x3D: 8}

    def _on_code(self, uc, addr, size, ud):
        a = u32(addr)
        w = self.icache.get(a)
        if w is None:
            w = struct.unpack(">I", bytes(uc.mem_read(phys(a), 4)))[0]
            self.icache[a] = w
        op = w >> 26
        if op not in self.LOADS and op not in self.STORES:
            return
        base = uc.reg_read(REG[GPR_NAMES[(w >> 21) & 31]])
        off = w & 0xFFFF
        ea = u32(base + (off - 0x10000 if off & 0x8000 else off))
        if op in (0x22, 0x26, 0x2A, 0x2E):          # lwl/lwr/swl/swr
            ea &= ~3
        elif op in (0x1A, 0x1B, 0x2C, 0x2D):        # ldl/ldr/sdl/sdr
            ea &= ~7
        is_load = op in self.LOADS
        n = self.LOADS[op] if is_load else self.STORES[op]
        mmio = 0x04000000 <= phys(ea) < 0x05000000
        va = (0xA0000000 | phys(ea)) if mmio else kseg0(ea)
        if self.reads is not None:
            (self.reads if is_load else self.trace_writes).update(range(va, va + n))
        if mmio and self.opts.mmio_log:
            if is_load:
                self.events.append(("mmio_r", va, n))
            else:
                rt = (w >> 16) & 31
                if op in (0x39, 0x3D):
                    v = u32(uc.reg_read(REG["f%d" % rt]))
                else:
                    v = uc.reg_read(REG[GPR_NAMES[rt]]) & ((1 << (8 * n)) - 1)
                self.events.append(("mmio_w", va, n, v))

    def _on_intr(self, uc, intno, ud):
        self.fault = ("exception", intno, self.last_block)
        uc.emu_stop()

    def _on_unmapped(self, uc, access, addr, size, value, ud):
        self.fault = ("unmapped", access, u32(addr), u32(uc.reg_read(REG["pc"])))
        return False

    # -- running ------------------------------------------------------------
    def run(self, entry, plan, track_reads=False):
        self.reset()
        uc = self.uc
        if track_reads:
            self.reads = set()
            self.trace_writes = set()
        want_code_hook = track_reads or self.opts.mmio_log
        if want_code_hook and self.code_hook is None:
            self.code_hook = uc.hook_add(UC_HOOK_CODE, self._on_code)
        elif not want_code_hook and self.code_hook is not None:
            uc.hook_del(self.code_hook)
            self.code_hook = None
        # restore the writable regions with this trial's inputs applied
        snap = [bytearray(b) for b in self.base]
        writes = [(phys(addr_fn(self.b)), data) for addr_fn, data in plan.mem]
        writes += [(phys(a), struct.pack(">I", v & 0xFFFFFFFF)) for a, v in self.opts.mmio_vals.items()]
        for p, data in writes:
            for (s, n), buf in zip(self.regions, snap):
                if s <= p and p + len(data) <= s + n:
                    buf[p - s:p - s + len(data)] = data
                    break
            else:
                uc.mem_write(p, data)       # outside the diffed regions (e.g. code)
        for (s, n), buf in zip(self.regions, snap):
            uc.mem_write(s, bytes(buf))
        self.snap = snap
        uc.reg_write(REG["fcsr"], plan.fcsr)
        for r, v in plan.regs.items():
            if r.startswith("f"):
                uc.reg_write(REG[r], v & 0xFFFFFFFF)
            else:
                uc.reg_write(REG[r], sext32(v))
        uc.reg_write(REG["sp"], sext32(STACK_TOP))
        uc.reg_write(REG["ra"], sext32(SENTINEL))
        uc.reg_write(REG["zero"], 0)
        t0 = time.time()
        err = None
        try:
            uc.emu_start(sext32(entry), sext32(SENTINEL),
                         timeout=int(self.opts.timeout * 1e6), count=self.opts.max_insns)
        except UcError as e:
            err = str(e)
        pc = u32(uc.reg_read(REG["pc"]))
        if self.fault is None and pc != SENTINEL:
            if err:
                self.fault = ("error", err, pc)
            else:
                self.fault = ("timeout", "no return after %d insns / %.1fs" %
                              (self.opts.max_insns, time.time() - t0), pc)
        res = {}
        for r in GPR_NAMES + ["hi", "lo"] + ["f%d" % i for i in range(32)]:
            res[r] = u32(uc.reg_read(REG[r]))
        res["s8"] = res["fp"]
        # every byte that differs from the start-of-run state
        CH = 0x1000
        for (s, n), buf in zip(self.regions, snap):
            now = bytes(uc.mem_read(s, n))
            if now == buf:
                continue
            seg = 0xA0000000 if 0x04000000 <= s < 0x05000000 else 0x80000000
            for i in range(0, n, CH):
                o, c = buf[i:i + CH], now[i:i + CH]
                if o != c:
                    self.written.update(seg | (s + i + j) for j in range(len(o)) if o[j] != c[j])
        return res

    def read(self, a, n=1):
        return bytes(self.uc.mem_read(phys(a), n))


# ---------------------------------------------------------------------------
class Plan:
    """One trial's inputs.  Memory entries are (resolver(build) -> addr, bytes) so a
    global named by symbol lands at that symbol's address in each build."""

    def __init__(self):
        self.regs = {}
        self.mem = []
        self.fcsr = 0
        self.desc = []
        self.heap = HEAP_BASE
        self.blocks = []          # addresses of ptr/ptrz blocks, in allocation order (heap0, heap1, ..)
        self.vals = {}            # values set by --arg (reg name -> int or per-build callable)

    def alloc(self, size):
        a = self.heap
        self.heap = (self.heap + size + 0x10 + 15) & ~15   # 16-byte red zone between blocks
        if self.heap > HEAP_END:
            raise RuntimeError("scratch heap exhausted")
        self.blocks.append(a)
        return a

    def base_value(self, name, what):
        """Value of a base name used by rel:/@ specs: an --arg register, heap, heapN,
        or a symbol.  Returns an int or a callable(build) -> int."""
        name = name.strip().lstrip("@")
        if name in self.vals:
            return self.vals[name]
        if name == "heap":
            return HEAP_BASE
        m = re.match(r"^heap(\d+)$", name)
        if m:
            k = int(m.group(1))
            if k >= len(self.blocks):
                raise SystemExit("%s: heap%d not allocated (only %d ptr block(s) so far; --arg ptr specs "
                                 "allocate first, left to right, then --mem ptr fills)" % (what, k, len(self.blocks)))
            return self.blocks[k]
        if name in REG:
            raise SystemExit("%s: %s has no --arg value to be relative to" % (what, name))
        try:
            return int(name, 0) & 0xFFFFFFFF
        except ValueError:
            return lambda b, n=name: b.resolve(n)


def rand_int(rng):
    r = rng.random()
    if r < 0.45:
        return rng.randint(-8, 64)
    if r < 0.7:
        return rng.randint(0, 0xFFFF)
    if r < 0.8:
        return rng.choice([0, 1, -1, 0x7FFFFFFF, -0x80000000, 0x8000, 0xFFFF])
    return rng.randint(0, 0xFFFFFFFF)


def rand_float(rng, lo=None, hi=None):
    if lo is not None:
        return rng.uniform(lo, hi)
    r = rng.random()
    if r < 0.25:
        return rng.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 100.0, -100.0])
    if r < 0.6:
        return float(rng.randint(-1000, 1000))
    if r < 0.9:
        return rng.uniform(-1000.0, 1000.0)
    return rng.uniform(-1e7, 1e7)


def fbits(x):
    return struct.unpack(">I", struct.pack(">f", x))[0]


def parse_pool(s):
    """'0x50*3,0x5A,-1' -> [0x50, 0x50, 0x50, 0x5A, -1] (VALUE*WEIGHT repeats a value)."""
    out = []
    for x in s.split(","):
        x = x.strip()
        if not x:
            continue
        if "*" in x:
            v, w = x.split("*", 1)
            out += [int(v, 0)] * int(w, 0)
        else:
            out.append(int(x, 0))
    if not out:
        raise SystemExit("empty value pool %r" % s)
    return out


def add_vals(a, b):
    """a + b where either may be a per-build callable."""
    if callable(a) or callable(b):
        fa = a if callable(a) else (lambda bb, v=a: v)
        fb = b if callable(b) else (lambda bb, v=b: v)
        return lambda bb: (fa(bb) + fb(bb)) & 0xFFFFFFFF
    return (a + b) & 0xFFFFFFFF


def gen_rel(spec, rng, plan, what):
    """rel:BASE+TERM+TERM..  BASE = an --arg register (a0..), heap, heapN or a symbol;
    TERM = CONST, K*SPEC (K times a random value spec) or SPEC."""
    terms = spec.split("+")
    v = plan.base_value(terms[0], what)
    descs = [terms[0]]
    for t in terms[1:]:
        t = t.strip()
        k = 1
        if "*" in t:
            head, rest = t.split("*", 1)
            try:
                k = int(head, 0)
                t = rest
            except ValueError:
                pass
        x, d = gen_value(t, rng, plan, what)
        if callable(x):
            raise SystemExit("%s: rel term %r must be a number" % (what, t))
        x = x - 0x100000000 if x & 0x80000000 else x
        v = add_vals(v, k * x)
        descs.append(("%#x*%s" % (k, d)) if k != 1 else d)
    if callable(v):
        return v, "rel " + "+".join(descs)
    return v, "0x%08X (%s)" % (v, "+".join(descs))


def gen_value(spec, rng, plan, what):
    """Value spec -> 32-bit int (may allocate heap / add memory to the plan)."""
    parts = spec.split(":")
    kind = parts[0]
    try:
        return int(spec, 0) & 0xFFFFFFFF, spec
    except ValueError:
        pass
    if kind == "rel":
        return gen_rel(spec[4:], rng, plan, what)
    if kind == "int":
        if len(parts) == 3:
            v = rng.randint(int(parts[1], 0), int(parts[2], 0))
        else:
            v = rand_int(rng)
        return v & 0xFFFFFFFF, "%d" % v
    if kind == "choice":
        v = rng.choice(parse_pool(parts[1]))
        return v & 0xFFFFFFFF, "%d" % v
    if kind in ("float", "fbits"):
        x = rand_float(rng, float(parts[1]), float(parts[2])) if len(parts) == 3 else rand_float(rng)
        return fbits(x), "%r" % struct.unpack(">f", struct.pack(">I", fbits(x)))[0]
    if kind in ("ptr", "ptrz"):
        size = int(parts[1], 0) if len(parts) > 1 else 0x200
        a = plan.alloc(size)
        data = bytes(size) if kind == "ptrz" else bytes(rng.getrandbits(8) for _ in range(size))
        plan.mem.append((lambda b, a=a: a, data))
        return a, "heap 0x%08X (%s 0x%X)" % (a, kind, size)
    if kind == "sym":
        name = parts[1]
        return (lambda b: b.resolve(name)), "&" + name
    raise SystemExit("bad value spec for %s: %r" % (what, spec))


def mem_target(tgt, plan):
    """--mem target -> resolver(build) -> address.  'SYM', 'SYM+0x10', '0x8036444C',
    or relative to an --arg value / heap block: '@a0', '@a0+0x40', '@heap1+0x10'."""
    t = tgt.strip()
    if t.startswith("@"):
        m = re.match(r"^@(\w+)\s*(?:([+-])\s*(0x[0-9A-Fa-f]+|\d+))?$", t)
        if not m:
            raise SystemExit("bad --mem target %r" % tgt)
        base = plan.base_value(m.group(1), tgt)
        off = int(m.group(3), 0) * (-1 if m.group(2) == "-" else 1) if m.group(3) else 0
        v = add_vals(base, off)
        return v if callable(v) else (lambda b, a=v: a)
    return lambda b, t=t: b.resolve(t)


def gen_fill(fill, size, rng, plan, tgt):
    """FILL for SIZE bytes (size None = default) -> (bytes, desc) or (callable, desc)."""
    fparts = fill.split(":")
    kind = fparts[0]
    if kind == "rand":
        size = size or 4
        return bytes(rng.getrandbits(8) for _ in range(size)), "random %d bytes" % size
    if kind == "zero":
        size = size or 4
        return bytes(size), "zero"
    if kind == "words":
        pool = parse_pool(fparts[1])
        n = (size or 4) // 4
        ws = [rng.choice(pool) & 0xFFFFFFFF for _ in range(n)]
        return b"".join(struct.pack(">I", w) for w in ws), "words " + ",".join("%X" % w for w in ws)
    if kind == "halves":
        pool = parse_pool(fparts[1])
        n = (size or 2) // 2
        hs = [rng.choice(pool) & 0xFFFF for _ in range(n)]
        return b"".join(struct.pack(">H", h) for h in hs), "halves " + ",".join("%X" % h for h in hs)
    if kind == "bytes":
        pool = parse_pool(fparts[1])
        n = size or 1
        bs = [rng.choice(pool) & 0xFF for _ in range(n)]
        return bytes(bs), "bytes " + ",".join("%X" % x for x in bs)
    if kind == "floats":
        n = (size or 4) // 4
        lo, hi = (float(fparts[1]), float(fparts[2])) if len(fparts) == 3 else (None, None)
        return b"".join(struct.pack(">I", fbits(rand_float(rng, lo, hi))) for _ in range(n)), "floats"
    if kind == "onehot":
        # onehot:STRIDE[@OFF][/W]:POOL:FILL -- FILL the whole area, then put one value from
        # POOL into one slot (first, last or a random one) at byte OFF, W bytes wide
        if len(fparts) < 4 or not size:
            raise SystemExit("%s: onehot needs TARGET:SIZE=onehot:STRIDE[@OFF][/W]:POOL:FILL" % tgt)
        m = re.match(r"^(0x[0-9A-Fa-f]+|\d+)(?:@(0x[0-9A-Fa-f]+|\d+))?(?:/([124]))?$", fparts[1])
        if not m:
            raise SystemExit("%s: bad onehot stride %r" % (tgt, fparts[1]))
        stride = int(m.group(1), 0)
        foff = int(m.group(2), 0) if m.group(2) else 0
        width = int(m.group(3)) if m.group(3) else 4
        pool = parse_pool(fparts[2])
        bfill = ":".join(fparts[3:])
        try:      # a constant FILL repeats as a word
            base, bdesc = struct.pack(">I", int(bfill, 0) & 0xFFFFFFFF) * (size // 4 + 1), bfill
            base = base[:size]
        except ValueError:
            base, bdesc = gen_fill(bfill, size, rng, plan, tgt)
        if callable(base) or len(base) != size:
            raise SystemExit("%s: onehot FILL must produce SIZE plain bytes" % tgt)
        if foff + width > stride:
            raise SystemExit("%s: onehot field @%d/%d doesn't fit the stride" % (tgt, foff, width))
        nslots = size // stride
        r = rng.random()
        slot = 0 if r < 0.3 else nslots - 1 if r < 0.6 else rng.randrange(nslots)
        v = rng.choice(pool) & ((1 << (8 * width)) - 1)
        o = slot * stride + foff
        data = bytearray(base)
        data[o:o + width] = v.to_bytes(width, "big")
        return bytes(data), "onehot slot %d/%d = 0x%X over %s" % (slot, nslots, v, bdesc)
    v, desc = gen_value(fill, rng, plan, tgt)
    if callable(v):
        return v, desc
    data = struct.pack(">I", v)
    if size and size != 4:
        data = data[4 - size:] if size < 4 else data + bytes(size - 4)
    return data, desc


def gen_mem(spec, rng, plan):
    """--mem TARGET[:SIZE]=FILL"""
    tgt, fill = spec.split("=", 1)
    size = None
    if ":" in tgt:
        tgt, sz = tgt.rsplit(":", 1)
        size = int(sz, 0)
    resolver = mem_target(tgt, plan)
    data, desc = gen_fill(fill, size, rng, plan, tgt)
    if callable(data):
        plan.mem.append((resolver, None, data))   # pointer to a symbol, resolved per build
    else:
        plan.mem.append((resolver, data))
    plan.desc.append("%s = %s" % (tgt, desc))


class Options:
    pass


def make_plan(opts, trial):
    rng = random.Random("%s/%d" % (opts.seed, trial))
    plan = Plan()
    # poison every register identically so non-ABI inputs are at least deterministic
    for r in GPR_NAMES:
        if r not in ("zero", "sp", "ra"):
            plan.regs[r] = rng.getrandbits(32)
    for i in range(32):
        plan.regs["f%d" % i] = rng.getrandbits(32)
    plan.regs["hi"] = rng.getrandbits(32)
    plan.regs["lo"] = rng.getrandbits(32)
    # caller's frame: 16 home bytes + stack args (sp+0x10..) poisoned identically
    plan.mem.append((lambda b: STACK_TOP, bytes(rng.getrandbits(8) for _ in range(0x100))))
    # --arg first (so a ptr block's random contents land before, and can be
    # overwritten by, --mem specs that target it), rel: args after the others
    args = [s for s in opts.args if not s.split("=", 1)[1].startswith("rel:")] + \
           [s for s in opts.args if s.split("=", 1)[1].startswith("rel:")]
    for spec in args:
        tgt, val = spec.split("=", 1)
        v, desc = gen_value(val, rng, plan, tgt)
        m = re.match(r"^sp\+(0x[0-9A-Fa-f]+|\d+)$", tgt)
        if m:
            off = int(m.group(1), 0)
            if callable(v):
                plan.mem.append((lambda b, o=off: STACK_TOP + o, None, v))
            else:
                plan.mem.append((lambda b, o=off: STACK_TOP + o, struct.pack(">I", v)))
        elif tgt in REG:
            plan.regs[tgt] = v       # a callable (sym:/rel: on a symbol) is resolved per build
        else:
            raise SystemExit("bad --arg target %r" % tgt)
        plan.vals[tgt] = v
        plan.desc.append("%s = %s" % (tgt, desc))
    for spec in opts.mem:
        gen_mem(spec, rng, plan)
    return plan


class PlanView:
    """A plan with per-build pointer values filled in."""

    def __init__(self, plan, build):
        self.regs = {r: (v(build) if callable(v) else v) for r, v in plan.regs.items()}
        self.fcsr = plan.fcsr
        self.mem = []
        for e in plan.mem:
            if len(e) == 3:
                res, _, valfn = e
                self.mem.append((res, struct.pack(">I", valfn(build))))
            else:
                self.mem.append(e)


# ---------------------------------------------------------------------------
def compare(opts, ref_m, new_m, amap, r_ref, r_new):
    """Returns a list of difference strings (empty = equivalent)."""
    diffs = []
    fr, fn = ref_m.fault, new_m.fault

    def fault_str(f, b):
        if f is None:
            return "returned"
        if f[0] == "unmapped":
            return "unmapped access at 0x%08X (pc %s)" % (f[2], b.name_at(f[3]))
        if f[0] == "exception":
            return "CPU exception %d at %s" % (f[1], b.name_at(f[2]))
        return "%s: %s (pc %s)" % (f[0], f[1], b.name_at(f[2]))

    def fault_key(f):
        if f is None:
            return None
        if f[0] == "unmapped":
            return f[:3]
        if f[0] == "exception":
            return f[:2]
        return f[:1]

    if fault_key(fr) != fault_key(fn):
        diffs.append("outcome: %s=%s, %s=%s" % (ref_m.b.label, fault_str(fr, ref_m.b),
                                                new_m.b.label, fault_str(fn, new_m.b)))
        return diffs
    if fr is not None and fr[0] in ("timeout", "error"):
        diffs.append("both runs failed to return (%s / %s); can't compare" %
                     (fault_str(fr, ref_m.b), fault_str(fn, new_m.b)))
        return diffs
    if fr is not None:
        # both hit the same unmapped address / exception: counted as equivalent
        # (state at the fault point is not comparable between asm and C)
        return diffs

    def same_val(vr, vn):
        if vr == vn:
            return True
        t, _ = amap.to_ref(vn)
        return t == vr

    # registers
    regs = list(opts.ret_regs) + ([] if opts.no_saved else [r for r in SAVED_REGS if r not in opts.ret_regs])
    for r in regs:
        if r in opts.ignore_regs:
            continue
        vr, vn = r_ref[r], r_new[r]
        if not same_val(vr, vn):
            what = "return" if r in opts.ret_regs else "callee-saved"
            extra = ""
            if r.startswith("f"):
                extra = " (%r vs %r)" % (struct.unpack(">f", struct.pack(">I", vr))[0],
                                         struct.unpack(">f", struct.pack(">I", vn))[0])
            diffs.append("%s register $%s: %s=0x%08X %s=0x%08X%s" % (what, r, ref_m.b.label, vr,
                                                                  new_m.b.label, vn, extra))

    # call / mmio sequence
    er, en = ref_m.events, new_m.events
    for i in range(max(len(er), len(en))):
        a = er[i] if i < len(er) else None
        b = en[i] if i < len(en) else None
        ok = a is not None and b is not None and a[0] == b[0]
        if ok and a[0] == "call":
            ok = a[1] == b[1] and all(ra == rb and same_val(va, vb) for (ra, va), (rb, vb) in zip(a[2], b[2]))
        elif ok:
            ok = a == b

        def ev_str(e, m):
            if e is None:
                return "(nothing)"
            if e[0] == "call":
                return "%s(%s)" % (e[1], ", ".join("%s=0x%X" % (r, v) for r, v in e[2]))
            if e[0] == "mmio_w":
                return "MMIO write [0x%08X].%d = 0x%X" % (e[1], e[2], e[3])
            return "MMIO read [0x%08X].%d" % (e[1], e[2])
        if not ok:
            diffs.append("event #%d differs: %s: %s | %s: %s" % (i, ref_m.b.label, ev_str(a, ref_m),
                                                                new_m.b.label, ev_str(b, new_m)))
            break

    # memory: every byte either side wrote, outside the callee's private stack frame
    lo_ex, hi_ex = STACK_TOP - STACK_WINDOW, STACK_TOP + (0 if opts.check_home else 0x10)

    def private(a):
        return lo_ex <= a < hi_ex

    new_map = {}
    only_new = []
    for a in new_m.written:
        if private(a):
            continue
        t, label = amap.to_ref(a)
        if t is None:
            only_new.append((a, label))
        else:
            new_map[t] = a
    addrs = set(a for a in ref_m.written if not private(a)) | set(new_map)
    bad = []
    for a in sorted(addrs):
        na = new_map.get(a, a)
        vr, vn = ref_m.read(a)[0], new_m.read(na)[0]
        if vr != vn:
            bad.append((a, vr, vn))
    if bad:
        a, vr, vn = bad[0]
        # show the aligned word around the first difference
        wa = a & ~3
        wr = ref_m.read(wa, 4).hex()
        wn = new_m.read(new_map.get(wa, wa), 4).hex()
        diffs.append("memory: %d byte(s) differ; first at 0x%08X %s: %s=%02X %s=%02X (word @0x%08X: %s vs %s)"
                     % (len(bad), a, ref_m.b.name_at(a), ref_m.b.label, vr, new_m.b.label, vn, wa, wr, wn))
        if opts.verbose:
            for a, vr, vn in bad[1:16]:
                diffs.append("    0x%08X %-28s %02X %02X" % (a, ref_m.b.name_at(a), vr, vn))
    if only_new and not opts.ignore_new_only:
        diffs.append("memory: %s wrote %d byte(s) with no counterpart, first at %s" %
                     (new_m.b.label, len(only_new), only_new[0][1]))
    return diffs


# ---------------------------------------------------------------------------
def parse_args(argv):
    ap = argparse.ArgumentParser(
        description="Fuzz-compare one function between two builds (default build/ vs build_nm/).",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__)
    ap.add_argument("func", help="function name (symbol) to test")
    ap.add_argument("-n", "--trials", type=int, default=200)
    ap.add_argument("--seed", default="0")
    ap.add_argument("--ref", default="build", help="reference build dir (default build)")
    ap.add_argument("--new", default="build_nm", help="build dir with the rewrite (default build_nm)")
    ap.add_argument("--version", default="us.v11")
    ap.add_argument("--arg", dest="args", action="append", default=[],
                    help="REG=SPEC or sp+0xOFF=SPEC (SPEC: const, int[:lo:hi], choice:a,b*3, "
                         "float[:lo:hi], fbits[:lo:hi], ptr[:size], ptrz[:size], sym:NAME, "
                         "rel:BASE+K*SPEC with BASE an --arg reg, heap, heapN or a symbol)")
    ap.add_argument("--mem", action="append", default=[],
                    help="TARGET[:size]=FILL, TARGET = SYM[+off], 0xADDR, @a0[+off], @heapN[+off] "
                         "(FILL: rand, zero, words:a,b*3,.., halves:.., bytes:.., floats[:lo:hi], "
                         "onehot:STRIDE[@OFF][/W]:POOL:FILL, or any --arg SPEC); applied after --arg")
    ap.add_argument("--ret", default="int",
                    help="return kind: int, ptr, void, u64, float, double, or regs:v0,t0,...")
    ap.add_argument("--ignore-reg", dest="ignore_regs", action="append", default=[])
    ap.add_argument("--no-saved-check", dest="no_saved", action="store_true",
                    help="don't compare callee-saved registers")
    ap.add_argument("--follow", action="append", default=[],
                    help="run this callee for real instead of stubbing it (repeatable, comma list)")
    ap.add_argument("--follow-all", action="store_true", help="run every callee for real")
    ap.add_argument("--sig", action="append", default=[],
                    help="NAME=a0,a1,f12 : which arg registers to record for calls to NAME "
                         "(default a0-a3; NAME=* sets the default)")
    ap.add_argument("--stub-ret", action="append", default=[],
                    help="NAME=VALUE|rand|ptr:SIZE : value stubbed NAME returns in v0 (default rand)")
    ap.add_argument("--mmio", action="append", default=[],
                    help="0xA4xxxxxx=VALUE: initial word at an MMIO register (MMIO is plain memory here)")
    ap.add_argument("--mmio-log", action="store_true",
                    help="also compare the ordered sequence of MMIO loads/stores (slower: per-insn hook)")
    ap.add_argument("--max-insns", type=int, default=2000000)
    ap.add_argument("--timeout", type=float, default=10.0, help="seconds per run")
    ap.add_argument("--max-fail", type=int, default=1, help="stop after this many failing trials")
    ap.add_argument("--check-home", action="store_true",
                    help="also compare writes to the caller's 16 arg home bytes at 0(sp)..0xF(sp)")
    ap.add_argument("--ignore-new-only", action="store_true",
                    help="ignore writes to memory that only exists in the new build")
    ap.add_argument("--explore", action="store_true",
                    help="run the reference once and list the calls made and globals read/written")
    ap.add_argument("--trace", action="store_true", help="print every basic block executed")
    ap.add_argument("-v", "--verbose", action="store_true")
    o = ap.parse_args(argv)

    opts = Options()
    for k, v in vars(o).items():
        setattr(opts, k, v)
    opts.follow_set = set(x for f in o.follow for x in f.split(","))
    if o.ret.startswith("regs:"):
        opts.ret_regs = o.ret[5:].split(",")
    else:
        opts.ret_regs = RET_KINDS[o.ret]
    sigs = {}
    default_sig = ["a0", "a1", "a2", "a3"]
    for s in o.sig:
        n, regs = s.split("=", 1)
        lst = [r for r in regs.split(",") if r]
        if n == "*":
            default_sig = lst
        else:
            sigs[n] = lst
    opts.sig_for = lambda name: sigs.get(name, default_sig)
    rets = {}
    for s in o.stub_ret:
        n, v = s.split("=", 1)
        rets[n] = v

    def ret_for(name, k, machine):
        spec = rets.get(name, "rand")
        h = hashlib.sha1(("%s/%s/%s/%d" % (opts.seed, opts._trial, name, k)).encode()).digest()
        r0, r1, r2 = struct.unpack(">III", h[:12])
        if spec == "rand":
            return (r0 if r0 & 1 else r0 & 0xFF), r1, r2
        if spec.startswith("ptr"):
            # deterministic per (trial, name, k): a fresh zeroed block high in the heap
            size = int(spec.split(":")[1], 0) if ":" in spec else 0x100
            a = HEAP_END - 0x40000 + (sum(machine.call_counts.values()) * ((size + 31) & ~15)) % 0x40000
            return a, 0, 0
        v = int(spec, 0)
        return v & 0xFFFFFFFF, 0, 0
    opts.ret_for = ret_for
    opts.mmio_vals = {}
    for s in o.mmio:
        a, v = s.split("=")
        opts.mmio_vals[int(a, 0) & ~3] = int(v, 0)
    opts._trial = 0
    return opts


def load_build(d, version, label):
    hd = os.path.join(d, "hd_code.%s.elf" % version)
    ini = os.path.join(d, "init.%s.elf" % version)
    for p in (hd, ini):
        if not os.path.exists(p):
            sys.exit("eqcheck: %s not found (build it first)" % p)
    return Build(label, [ini, hd])


def explore(opts, ref, m, entry):
    plan = make_plan(opts, 0)
    r = m.run(entry, PlanView(plan, ref), track_reads=True)
    print("explore %s @0x%08X (%s), trial 0 inputs: %s" % (opts.func, entry, ref.label, "; ".join(plan.desc) or "-"))
    print("outcome:", "returned" if m.fault is None else m.fault)
    print("return v0=0x%08X v1=0x%08X f0=0x%08X" % (r["v0"], r["v1"], r["f0"]))

    blocks = plan.blocks

    def regions(addrs):
        out = {}
        run_key, prev = None, None     # contiguous unnamed bytes are merged into one range
        for a in sorted(addrs):
            if STACK_TOP - STACK_WINDOW <= a < STACK_TOP:
                key = "own stack frame"
            elif STACK_TOP <= a < STACK_TOP + 0x100:
                key = "caller frame (sp+0x%X..)" % (a - STACK_TOP & ~0xF)
            elif HEAP_END - 0x40000 <= a < HEAP_END:
                key = "heap (--stub-ret ptr blocks)"
            elif HEAP_BASE <= a < HEAP_END:
                k = bisect.bisect_right(blocks, a) - 1
                key = "heap%d (0x%08X)" % (k, blocks[k]) if k >= 0 else "heap"
            elif ref.in_text(a):
                continue
            else:
                n, off = ref.sym_off(a)
                if n is not None and off < 0x10000:
                    key = n
                else:
                    if run_key is None or prev != a - 1:
                        run_key = "(unnamed) 0x%08X" % a
                    key = run_key
                    prev = a
            out.setdefault(key, []).append(a)
        return out
    for title, s in (("reads", m.reads), ("writes", m.trace_writes)):
        print("%s:" % title)
        groups = sorted(regions(s).items(), key=lambda kv: kv[1][0])
        # a run of more than 4 back-to-back groups (a scan running across many
        # symbols) is printed as one line
        i = 0
        while i < len(groups):
            j = i
            while j + 1 < len(groups) and groups[j][1][-1] + 1 == groups[j + 1][1][0] \
                    and all(groups[x][1][-1] - groups[x][1][0] + 1 == len(groups[x][1]) for x in (j, j + 1)):
                j += 1
            if j - i >= 4:
                lo, hi = groups[i][1][0], groups[j][1][-1] + 1
                print("  %-24s 0x%08X..0x%08X (%d bytes, %d symbols, contiguous)" %
                      ("%s .. %s" % (groups[i][0], groups[j][0]), lo, hi, hi - lo, j - i + 1))
                i = j + 1
                continue
            k, v = groups[i]
            print("  %-24s 0x%08X..0x%08X (%d bytes)" % (k, v[0], v[-1] + 1, len(v)))
            i += 1
    print("events:")
    for e in m.events:
        if e[0] == "call":
            print("  call %s(%s)" % (e[1], ", ".join("%s=0x%X" % (rr, vv) for rr, vv in e[2])))
        else:
            print("  %s" % (e,))
    for n in m.notes:
        print("  note:", n)


def main(argv=None):
    opts = parse_args(argv if argv is not None else sys.argv[1:])
    ref = load_build(opts.ref, opts.version, opts.ref.rstrip("/"))
    if opts.func not in ref.sym:
        sys.exit("eqcheck: %s not in %s" % (opts.func, ref.label))
    ref_m = Machine(ref, opts=opts)
    if opts.explore:
        explore(opts, ref, ref_m, ref.sym[opts.func])
        return 0
    new = load_build(opts.new, opts.version, opts.new.rstrip("/"))
    if opts.new.rstrip("/") == opts.ref.rstrip("/"):
        new.label = new.label + "(2)"
    if opts.func not in new.sym:
        sys.exit("eqcheck: %s not in %s" % (opts.func, new.label))
    amap = AddrMap(ref, new)
    new_m = Machine(new, prefill=None if amap.identity else ref, opts=opts)
    # sanity: data that should not have moved must be byte-identical
    for v, d, name, ex in ref.sections:
        if ex or isinstance(d, int):
            continue
        for v2, d2, n2, _ in new.sections:
            if v2 == v and n2 == name and not isinstance(d2, int) and d2 != d:
                if len(d) != len(d2):
                    print("warning: section %s changed size between builds; globals may not line up" % name)
                    continue
                # words that are relocated code pointers (jump tables, function
                # pointer tables in C .data) are expected to differ
                bad = []
                for i in range(0, len(d) - 3, 4):
                    wr, wn = d[i:i + 4], d2[i:i + 4]
                    if wr != wn:
                        vr, vn = struct.unpack(">I", wr)[0], struct.unpack(">I", wn)[0]
                        if amap.to_ref(vn)[0] != vr:
                            bad.append(v + i)
                if bad:
                    print("warning: section %s: %d word(s) differ between builds beyond relocated code "
                          "pointers (first at %s)" % (name, len(bad), ref.name_at(bad[0])))
    e_ref, e_new = ref.sym[opts.func], new.sym[opts.func]
    if not amap.identity and not new.in_text(e_new):
        print("warning: %s resolves to 0x%08X in %s, outside its code: an absolute symbol is overriding the "
              "rewrite's definition (rebuild with the current Makefile, which PROVIDE()s them)"
              % (opts.func, e_new, new.label))
    print("eqcheck %s: %s @0x%08X vs %s @0x%08X, %d trials, seed %s" %
          (opts.func, ref.label, e_ref, new.label, e_new, opts.trials, opts.seed))
    fails = 0
    both_faulted = 0
    notes = set()
    t0 = time.time()
    for t in range(opts.trials):
        opts._trial = t
        plan = make_plan(opts, t)
        r_ref = ref_m.run(e_ref, PlanView(plan, ref))
        r_new = new_m.run(e_new, PlanView(plan, new))
        for n in new_m.notes:
            if n not in notes:
                notes.add(n)
                print("note: %s executed original-build code at %s (not a function entry; "
                      "jump table or pointer in a binary blob?)" % (new.label, ref.name_at(n[1])))
        if ref_m.fault and new_m.fault:
            both_faulted += 1
        diffs = compare(opts, ref_m, new_m, amap, r_ref, r_new)
        if diffs:
            fails += 1
            if fails <= opts.max_fail:
                print("FAIL trial %d (seed %s)" % (t, opts.seed))
                print("  inputs: %s" % ("; ".join(plan.desc) or "(registers/stack poisoned only)"))
                for d in diffs:
                    print("  " + d)
                if opts.verbose:
                    for lbl, m in ((ref.label, ref_m), (new.label, new_m)):
                        print("  %s events: %s" % (lbl, m.events[:20]))
            if fails >= opts.max_fail:
                break
    ran = t + 1
    status = "PASS" if fails == 0 else "FAIL"
    print("%s: %s %d/%d trials equivalent%s (%.1fs)" %
          (status, opts.func, ran - fails, ran,
           ", %d where both faulted identically" % both_faulted if both_faulted else "",
           time.time() - t0))
    if fails == 0 and both_faulted == ran:
        print("warning: every trial faulted; inputs probably need --arg/--mem specs (try --explore)")
    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
