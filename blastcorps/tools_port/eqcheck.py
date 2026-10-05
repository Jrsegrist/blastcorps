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
import ctypes
import hashlib
import math
import os
import pickle
import random
import re
import struct
import sys
import time

try:
    from unicorn import Uc, UcError, UC_ARCH_MIPS, UC_MODE_MIPS64, UC_MODE_BIG_ENDIAN, UC_PROT_ALL
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
PAGE = 0x1000                    # granularity of the per-trial restore/diff

_libc = ctypes.CDLL(None)
_memcmp = _libc.memcmp
_memcmp.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t]
_memcmp.restype = ctypes.c_int

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
# Register conventions of non-ABI hand-asm functions (tools_port/conventions.txt).
#
#   NAME: in REG=SLOT,.. ; out REG=DEST,.. ; preserve REG,.. ; clobbers REG,..
#
# SLOT is where the C version takes the value: a0-a3, stackN (the word at
# sp+0x10+4N; sp+0x10 is accepted too), f12, f14, or *SLOT[+OFF][:W] (read
# through a pointer argument).  DEST is where the C version delivers an
# output: ret (v0), fret (f0), any register, or *SLOT[+OFF][:W] (written
# through a pointer argument; W = 1, 2 or 4 bytes, default 4).  A line with
# `in` or `out` is a full convention: it lists every input and output.
CONV_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "conventions.txt")
REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTP_BASE = 0x80D80000          # pointer-argument scratch blocks of the function under test (not diffed)
INT_SLOTS = ["a0", "a1", "a2", "a3"]
FLOAT_SLOTS = ["f12", "f14"]


def expand_regs(s, what):
    """'v1,a0,s0-s7' -> ['v1', 'a0', 's0', .., 's7']"""
    out = []
    for x in s.split(","):
        x = x.strip()
        if not x:
            continue
        m = re.match(r"^([a-z]+)(\d+)-(?:[a-z]+)?(\d+)$", x)
        regs = ["%s%d" % (m.group(1), i) for i in range(int(m.group(2)), int(m.group(3)) + 1)] if m else [x]
        for r in regs:
            if r not in REG or r in ("pc", "fcsr", "zero"):
                raise ValueError("%s: unknown register %r" % (what, r))
            out.append("fp" if r == "s8" else r)
    return out


def parse_slot(s, what):
    s = s.strip()
    if s in INT_SLOTS or s in FLOAT_SLOTS:
        return s
    if re.match(r"^stack\d+$", s):
        return s
    m = re.match(r"^sp\s*\+\s*(0x[0-9A-Fa-f]+|\d+)$", s)
    if m:
        off = int(m.group(1), 0)
        if off >= 0x10 and off % 4 == 0:
            return "stack%d" % ((off - 0x10) // 4)
    raise ValueError("%s: bad C argument slot %r (want a0-a3, f12, f14, stackN or sp+0x10..)" % (what, s))


def slot_off(slot):
    return 0x10 + 4 * int(slot[5:])


def slot_key(slot):
    if slot in INT_SLOTS:
        return (0, INT_SLOTS.index(slot))
    if slot.startswith("stack"):
        return (1, int(slot[5:]))
    return (2, FLOAT_SLOTS.index(slot) if slot in FLOAT_SLOTS else 9)


def parse_loc(s, what, regs_ok):
    """A SLOT/DEST: ('reg', REG, 0, W) or ('mem', SLOT, OFF, W)."""
    s = s.strip()
    w = 4
    m = re.match(r"^(.*?):([124])$", s)
    if m:
        s, w = m.group(1).strip(), int(m.group(2))
    if s.startswith("*"):
        m = re.match(r"^\*\s*(\w+)\s*(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?$", s)
        if not m:
            raise ValueError("%s: bad pointer slot %r (want *a2, *a2+4, *stack0:2)" % (what, s))
        slot = parse_slot(m.group(1), what)
        if slot in FLOAT_SLOTS:
            raise ValueError("%s: %r: a pointer can't be passed in %s" % (what, s, slot))
        return ("mem", slot, int(m.group(2), 0) if m.group(2) else 0, w)
    if regs_ok:
        r = {"ret": "v0", "fret": "f0", "s8": "fp"}.get(s, s)
        if r in REG and r not in ("pc", "fcsr", "zero"):
            return ("reg", r, 0, w)
        raise ValueError("%s: bad output %r (want ret, fret, a register or *SLOT[+OFF][:W])" % (what, s))
    return ("reg", parse_slot(s, what), 0, w)


def loc_str(loc):
    kind, where, off, w = loc
    s = ("*%s+0x%X" % (where, off) if off else "*" + where) if kind == "mem" else where
    return s + (":%d" % w if w != 4 else "")


class Conv:
    """One function's register convention (a conventions.txt line)."""

    def __init__(self, name, where):
        self.name, self.where = name, where
        self.ins = []           # (asm reg or stackN, loc) in C argument order
        self.outs = []          # (asm reg, loc)
        self.preserve = set()
        self.clobbers = set()
        self.full = False

    def ptr_slots(self):
        """C argument slots that carry a pointer to an input/output word."""
        return sorted(set(l[1] for _, l in self.ins + self.outs if l[0] == "mem"), key=slot_key)

    def text(self):
        parts = []
        if self.ins:
            parts.append("in " + ",".join("%s=%s" % (r, loc_str(l)) for r, l in self.ins))
        if self.outs:
            parts.append("out " + ",".join("%s=%s" % (r, loc_str(l)) for r, l in self.outs))
        if self.preserve:
            parts.append("preserve " + ",".join(sorted(self.preserve)))
        if self.clobbers:
            parts.append("clobbers " + ",".join(sorted(self.clobbers)))
        return " ; ".join(parts)


def parse_conv_line(line, where):
    m = re.match(r"^([A-Za-z_]\w*)\s*:\s*(.*)$", line.strip())
    if not m:
        raise ValueError("%s: expected 'NAME: in REG=SLOT,.. ; out REG=DEST,.. ; preserve ..'" % where)
    c = Conv(m.group(1), where)
    for clause in m.group(2).split(";"):
        clause = clause.strip()
        if not clause:
            continue
        kw, _, rest = clause.partition(" ")
        if kw in ("in", "out"):
            c.full = True
            for item in rest.split(","):
                item = item.strip()
                if not item:
                    continue
                if "=" not in item:
                    raise ValueError("%s: %s item %r needs REG=%s" % (where, kw, item, "SLOT" if kw == "in" else "DEST"))
                r, loc = item.split("=", 1)
                r = r.strip()
                if kw == "in" and re.match(r"^(stack\d+|sp\s*\+.*)$", r):
                    r = parse_slot(r, where)           # a stack argument the asm reads in place
                else:
                    r = expand_regs(r, where)[0]
                if kw == "in":
                    c.ins.append((r, parse_loc(loc, where, False)))
                else:
                    c.outs.append((r, parse_loc(loc, where, True)))
        elif kw == "preserve":
            c.preserve |= set(expand_regs(rest, where))
        elif kw == "clobbers":
            c.clobbers |= set(expand_regs(rest, where))
        else:
            raise ValueError("%s: unknown clause %r (want in, out, preserve, clobbers)" % (where, kw))
    # sanity
    seen = {}
    for r, l in c.ins:
        k = (l[1], l[2]) if l[0] == "mem" else l[1]
        if k in seen:
            raise ValueError("%s: C slot %s used by both %s and %s" % (where, loc_str(l), seen[k], r))
        seen[k] = r
    ptrs = set(c.ptr_slots())
    for r, l in c.ins:
        if l[0] == "reg" and l[1] in ptrs:
            raise ValueError("%s: %s is both a value and a pointer slot" % (where, l[1]))
    regs_in = [r for r, _ in c.ins]
    if len(set(regs_in)) != len(regs_in):
        raise ValueError("%s: an input register is listed twice" % where)
    outs = [r for r, _ in c.outs]
    if len(set(outs)) != len(outs):
        raise ValueError("%s: an output register is listed twice" % where)
    bad = c.preserve & set(outs)
    if bad:
        raise ValueError("%s: %s both preserved and an output" % (where, ",".join(sorted(bad))))
    return c


def load_convs(path, must_exist=False):
    convs = {}
    if not os.path.exists(path):
        if must_exist:
            raise SystemExit("eqcheck: conventions file %s not found" % path)
        return convs
    with open(path) as f:
        lines = f.read().split("\n")
    i = 0
    while i < len(lines):
        lineno, line = i + 1, lines[i]
        i += 1
        while line.rstrip().endswith("\\") and i < len(lines):
            line = line.rstrip()[:-1] + " " + lines[i]
            i += 1
        s = line.split("#", 1)[0].strip()
        if not s:
            continue
        where = "%s:%d" % (os.path.basename(path), lineno)
        try:
            c = parse_conv_line(s, where)
        except ValueError as e:
            raise SystemExit("eqcheck: %s" % e)
        if c.name in convs:
            raise SystemExit("eqcheck: %s: %s already defined at %s" % (where, c.name, convs[c.name].where))
        convs[c.name] = c
    return convs


_SRCINFO = {}


def source_info(version):
    """(asm, rewritten): every function with a GLOBAL_ASM pragma in src.<version>,
    and those whose pragma sits in the #else of an #ifdef NON_MATCHING (so
    the NON_MATCHING build has C for them)."""
    if version in _SRCINFO:
        return _SRCINFO[version]
    asm, rew = set(), set()
    src = os.path.join(REPO_DIR, "src.%s" % version)
    for dirpath, _, files in os.walk(src):
        for fn in files:
            if not fn.endswith(".c"):
                continue
            stack = []          # per open #if: [is_nm, in_else]
            with open(os.path.join(dirpath, fn), errors="replace") as f:
                for line in f:
                    s = line.strip()
                    if not s.startswith("#"):
                        continue
                    if re.match(r"#\s*if(n?def)?\b", s):
                        stack.append([bool(re.match(r"#\s*ifdef\s+NON_MATCHING\b", s)
                                           or re.match(r"#\s*if\s+defined\s*\(?\s*NON_MATCHING", s)), False])
                    elif re.match(r"#\s*else\b", s) and stack:
                        stack[-1][1] = True
                    elif re.match(r"#\s*endif\b", s) and stack:
                        stack.pop()
                    else:
                        m = re.search(r'GLOBAL_ASM\(\s*"([^"]+)"', s)
                        if m:
                            n = os.path.splitext(os.path.basename(m.group(1)))[0]
                            asm.add(n)
                            if any(nm and el for nm, el in stack):
                                rew.add(n)
    _SRCINFO[version] = (asm, rew)
    return asm, rew


# ---------------------------------------------------------------------------
class Build:
    """Symbols and loadable sections of one build (hd_code ELF + init ELF)."""

    def __init__(self, label, elfs):
        self.label = label
        self.sections = []        # (vaddr, bytes or None(size), name, exec)
        self.sym = {}             # name -> addr (u32)
        self.size = {}            # name -> st_size (object symbols that have one; absolute ones don't)
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
                        if sy["st_size"]:
                            self.size[n] = sy["st_size"]
                        else:
                            self.size.pop(n, None)
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

    def sym_off(self, a, skip_labels=False):
        i = bisect.bisect_right(self._keys, a) - 1
        if i < 0:
            return None, 0
        if skip_labels:
            # nearest preceding D_/named symbol, not a local code label (L8xxxxxxx, .L..)
            j = i
            while j >= 0 and i - j < 256:
                n = self.addr_syms[j][1]
                if n.startswith("D_") or not SKIP_SYM.match(n):
                    i = j
                    break
                j -= 1
        base, n = self.addr_syms[i]
        return n, a - base

    def resolve(self, expr):
        """'SYM', 'SYM+0x10', '0x8036444C' -> u32 address."""
        m = re.match(r"^([A-Za-z_][\w]*)?\s*([+-]\s*(?:0x[0-9A-Fa-f]+|\d+))?$", expr.strip())
        try:
            if not m or m.group(1) is None:
                return int(expr, 0) & 0xFFFFFFFF
        except ValueError:
            raise SystemExit("eqcheck: can't parse address %r (want SYM, SYM+0x10 or 0x80xxxxxx)" % expr)
        name, off = m.group(1), m.group(2)
        if name not in self.sym:
            raise SystemExit("eqcheck: unknown symbol %r in %r (not in %s)" % (name, expr, self.label))
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
class ModelMem:
    """Guest memory as seen by a --model function (KSEG0/physical addresses)."""

    def __init__(self, m):
        self.m = m

    def read(self, a, n):
        return self.m.read(u32(a), n)

    def write(self, a, data):
        self.m.uc.mem_write(phys(u32(a)), bytes(data))

    def u8(self, a):
        return self.read(a, 1)[0]

    def u16(self, a):
        return struct.unpack(">H", self.read(a, 2))[0]

    def s16(self, a):
        return struct.unpack(">h", self.read(a, 2))[0]

    def u32(self, a):
        return struct.unpack(">I", self.read(a, 4))[0]

    def s32(self, a):
        return struct.unpack(">i", self.read(a, 4))[0]

    def f32(self, a):
        return struct.unpack(">f", self.read(a, 4))[0]

    def w8(self, a, v):
        self.write(a, [v & 0xFF])

    def w16(self, a, v):
        self.write(a, struct.pack(">H", v & 0xFFFF))

    def w32(self, a, v):
        self.write(a, struct.pack(">I", v & 0xFFFFFFFF))

    def sym(self, name):
        """Address of NAME in the build this run uses."""
        return self.m.b.resolve(name)


def load_model(spec):
    """'NAME=FILE.py:FUNC' or 'NAME=FILE.py' (FUNC defaults to model) -> (NAME, callable).
    A bare FILE is looked up in tools_port/models/ too."""
    if "=" not in spec:
        raise SystemExit("eqcheck: bad --model %r: want NAME=FILE.py[:FUNC]" % spec)
    name, rest = spec.split("=", 1)
    path, _, fn = rest.partition(":")
    fn = fn or "model"
    cands = [path, os.path.join(os.path.dirname(os.path.abspath(__file__)), "models", path)]
    for p in cands:
        if os.path.exists(p):
            break
    else:
        raise SystemExit("eqcheck: --model %s: %s not found" % (name, path))
    import importlib.util
    sp = importlib.util.spec_from_file_location("eqmodel_%s" % name, p)
    mod = importlib.util.module_from_spec(sp)
    sp.loader.exec_module(mod)
    if not hasattr(mod, fn):
        raise SystemExit("eqcheck: --model %s: %s has no function %s" % (name, p, fn))
    return name, getattr(mod, fn)


# ---------------------------------------------------------------------------
class Machine:
    def __init__(self, build, prefill=None, opts=None):
        self.b = build
        self.opts = opts
        uc = Uc(UC_ARCH_MIPS, UC_MODE_MIPS64 | UC_MODE_BIG_ENDIAN)
        uc.ctl_set_cpu_model(M.UC_CPU_MIPS64_R4000)
        # RAM and MMIO are backed by our own host buffers (mem_map_ptr), so the
        # per-trial diff can memcmp guest memory in place instead of copying
        # megabytes out through mem_read.
        self.ram = ctypes.create_string_buffer(RAM_SIZE)
        self.mmio = ctypes.create_string_buffer(MMIO_SIZE)
        uc.mem_map_ptr(0, RAM_SIZE, UC_PROT_ALL, self.ram)
        uc.mem_map_ptr(MMIO_BASE, MMIO_SIZE, UC_PROT_ALL, self.mmio)
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
        self.long_ops = {}
        self._patch_long_ops(build.sections + (prefill.sections if prefill is not None else []))
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
        self.ram_addr = ctypes.addressof(self.ram)
        self.mmio_addr = ctypes.addressof(self.mmio)
        # base: pristine contents; snap: base plus the current trial's inputs
        # (the start-of-run state the diff compares against).  Both are host
        # buffers so pages can be compared/restored with memcmp/memmove.
        self.base = [ctypes.create_string_buffer(bytes(uc.mem_read(s, n)), n) for s, n in regions]
        self.snap = [ctypes.create_string_buffer(bytes(b.raw), n) for b, (s, n) in zip(self.base, regions)]
        self.dirty = set()        # (region index, page offset) to restore from base before the next run
        self.dirty_other = []     # input writes outside the regions (restored before the next run)
        uc.hook_add(UC_HOOK_BLOCK, self._on_block)
        uc.hook_add(UC_HOOK_INTR, self._on_intr)
        uc.hook_add(UC_HOOK_MEM_UNMAPPED | UC_HOOK_MEM_FETCH_UNMAPPED, self._on_unmapped)
        self.code_hook = None
        self.icache = {}
        self.auto_follow = set()  # callees run for real because the other build has no such function
        # a NON_MATCHING build (text moved up) has C for the functions rewritten under #ifdef NON_MATCHING
        self.is_nm = any(lo >= 0x80800000 for lo, hi in build.text)
        self._fent = sorted(build.funcs)
        self.prev_block = 0

    # -- which side of a register convention a piece of code is on ------------
    def func_at(self, a):
        i = bisect.bisect_right(self._fent, a) - 1
        return self.b.funcs[self._fent[i]] if i >= 0 else None

    def func_side(self, name):
        """'asm' if NAME is hand asm (a GLOBAL_ASM pragma) in this build, else 'c'."""
        asm, rew = source_info(self.opts.version)
        if name is None:
            return "asm"
        if name not in asm or (self.is_nm and name in rew):
            return "c"
        return "asm"

    def caller_side(self):
        """Side of the code that just made a call (the block before the callee's entry)."""
        a = self.prev_block
        if not self.b.in_text(a):
            return "asm"        # original-build code (prefilled text, a blob)
        return self.func_side(self.func_at(a))

    def read_loc(self, loc):
        """Value of a register, or of a stack argument (stackN / sp+0x10) at the current sp."""
        uc = self.uc
        if loc.startswith("stack") or loc.startswith("sp+"):
            off = slot_off(loc) if loc.startswith("stack") else int(loc[3:], 0)
            return struct.unpack(">I", self.read(u32(uc.reg_read(REG["sp"])) + off, 4))[0]
        return u32(uc.reg_read(REG[loc]))

    def read_ptr(self, ptr, w):
        try:
            return int.from_bytes(self.read(ptr, w), "big")
        except UcError:
            return None

    def write_reg(self, r, v):
        self.uc.reg_write(REG[r], v & 0xFFFFFFFF if r.startswith("f") else sext32(v))

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
        self.prev_block = 0

    def in_code(self, a):
        return self.b.in_text(a) or any(lo <= a < hi for lo, hi in self.prefill_text)

    def _on_block(self, uc, addr, size, ud):
        a = u32(addr)
        self.prev_block = self.last_block
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
        if name in self.opts.models and name != self.opts.func:
            self._stub(name)        # runs the model
            return
        if name in self.opts.follow_set or self.opts.follow_all and not name.startswith("sub_") \
                or name == self.opts.func:
            if target_addr is not None:
                uc.reg_write(REG["pc"], sext32(target_addr))
            return
        if name in self.auto_follow:
            # a function that exists only in this build (a static helper of a
            # rewrite): part of the rewrite, so run it instead of stubbing it
            if ("helper", name) not in self.notes:
                self.notes.append(("helper", name))
            return
        self._stub(name)

    def _stub(self, name):
        """Record a call to NAME and return from it at once.  For a callee with
        a full register convention the arguments are read, and the canned
        results written, on the caller's side of the convention: the asm
        registers when the caller is hand asm, the C slots (a0-a3, stack,
        f12/f14, pointer arguments) and v0/f0 when it is C.  Arguments are
        labelled by C slot, so the two builds compare."""
        uc = self.uc
        model = self.opts.models.get(name)
        if model is not None:
            return self._run_model(name, model)
        k = self.call_counts.get(name, 0)
        self.call_counts[name] = k + 1
        conv = self.opts.convs.get(name)
        full = conv is not None and conv.full
        side = self.caller_side() if full else None
        if full:
            sel = self.opts.sigs.get(name)          # --sig with a convention selects C slots
            args = tuple((label, v) for label, slot, v in self._conv_args(conv, side)
                         if sel is None or label in sel or slot in sel)
        else:
            args = tuple((r, self.read_loc(r)) for r in self.opts.sig_for(name))
        self.events.append(("call", name, args))
        keep = self.opts.preserve_for(name)
        if not full or side == "c":
            rv = self.opts.ret_for(name, k, self)
            for r, v in (("v0", rv[0]), ("v1", rv[1]), ("f0", rv[2])):
                if r not in keep:
                    self.write_reg(r, v)
        if full:
            self._deliver(name, conv, side, [self.opts.out_value(name, k, idx, r, self)
                                             for idx, (r, loc) in enumerate(conv.outs)])
        uc.reg_write(REG["pc"], uc.reg_read(REG["ra"]))

    def _conv_args(self, conv, side):
        """[(C slot label, slot, value)] of a call to a function with convention CONV."""
        out = []
        for r, loc in conv.ins:
            if side == "asm":
                v = self.read_loc(r)
            elif loc[0] == "reg":
                v = self.read_loc(loc[1])
            else:
                v = self.read_ptr(self.read_loc(loc[1]) + loc[2], loc[3])
                v = 0xDEADDEAD if v is None else v
            if loc[3] != 4:
                v &= (1 << (8 * loc[3])) - 1
            out.append((loc_str(loc), loc[1], v))
        return out

    def _deliver(self, name, conv, side, vals):
        """Write a callee's outputs on the caller's side of its convention."""
        for (r, loc), v in zip(conv.outs, vals):
            if loc[3] != 4:
                v &= (1 << (8 * loc[3])) - 1
            if side == "asm":
                self.write_reg(r, v)
            elif loc[0] == "reg":
                self.write_reg(loc[1], v)
            else:
                p = u32(self.read_loc(loc[1]) + loc[2])
                try:
                    self.uc.mem_write(phys(p), v.to_bytes(loc[3], "big"))
                except UcError:
                    if ("badptr", name) not in self.notes:
                        self.notes.append(("badptr", name))

    def _run_model(self, name, model):
        """--model NAME=FILE.py:FUNC: a Python function stands in for NAME in
        both builds (like --follow, so no call is recorded).  It is called as
        FUNC(args, mem): args are NAME's inputs in C argument order (its
        convention's `in` list, else a0-a3), mem a ModelMem.  It returns the
        outputs in `out` order (a list; an int is the first one, v0 without a
        convention), delivered on the caller's side like a stub's."""
        uc = self.uc
        conv = self.opts.convs.get(name)
        full = conv is not None and conv.full
        side = self.caller_side() if full else "c"
        if full:
            args = [v for _, _, v in self._conv_args(conv, side)]
        else:
            args = [self.read_loc(r) for r in ("a0", "a1", "a2", "a3")]
        try:
            res = model(args, ModelMem(self))
        except Exception as e:      # a model bug: this run can't be compared
            self.fault = ("error", "model %s raised %s: %s" % (name, type(e).__name__, e), self.last_block)
            uc.emu_stop()
            return
        res = [] if res is None else [res] if isinstance(res, int) else list(res)
        if full:
            self._deliver(name, conv, side, [v & 0xFFFFFFFF for v in res])
        elif res:
            self.write_reg("v0", res[0])
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

    # -- FPU long-integer (L) format ops under Status.FR=0 ----------------------
    # The game's threads run with FR=0 (osCreateThread gives them
    # SR = IMASK|IE|EXL and the exception handler only ORs in CU1), and the
    # VR4300 executes cvt.d.l / cvt.l.d etc. there on an even/odd register pair
    # (libultra's __ll_to_d relies on it).  QEMU raises a reserved-instruction
    # exception for them unless FR=1, so every such word in the loaded code is
    # replaced by a nop at load time and emulated by a code hook on its address
    # (which also works in a branch delay slot).
    L_TO_FLOAT = (0x20, 0x21)                       # cvt.s.l, cvt.d.l (fmt L)
    FLOAT_TO_L = {0x25: None, 0x08: 0, 0x09: 1, 0x0A: 2, 0x0B: 3}   # cvt/round/trunc/ceil/floor .l (fmt S/D)

    @classmethod
    def _is_long_op(cls, w):
        if w >> 26 != 0x11:
            return False
        fmt, fn = (w >> 21) & 31, w & 63
        return fmt == 21 and fn in cls.L_TO_FLOAT or fmt in (16, 17) and fn in cls.FLOAT_TO_L

    def _patch_long_ops(self, sections):
        """Nop out the FPU long ops in executable SECTIONS and hook their addresses."""
        for v, d, name, ex in sections:
            if not ex or isinstance(d, int):
                continue
            for i in range(0, len(d) - 3, 4):
                if d[i] & 0xFC != 0x44:             # COP1 major opcode
                    continue
                w = struct.unpack(">I", d[i:i + 4])[0]
                if self._is_long_op(w):
                    a = v + i
                    self.long_ops[a] = w
                    self.uc.mem_write(phys(a), b"\0\0\0\0")
                    self.uc.hook_add(UC_HOOK_CODE, self._on_long_op, begin=sext32(a), end=sext32(a))

    def _on_long_op(self, uc, addr, size, ud):
        a = u32(addr)
        w = self.long_ops.get(a)
        if w is not None and not self._exec_long_op(uc, w):
            self.fault = ("error", "can't emulate FPU long op %08X (odd register, NaN/inf or out of range)" % w, a)
            uc.emu_stop()

    def _exec_long_op(self, uc, w):
        fmt, fs, fd, fn = (w >> 21) & 31, (w >> 11) & 31, (w >> 6) & 31, w & 63
        if fs & 1 or fd & 1 and not (fmt == 21 and fn == 0x20):
            return False                    # FR=0 pairs must be even

        def lo(n):
            return u32(uc.reg_read(REG["f%d" % n]))
        if fmt == 21:
            x = lo(fs) | lo(fs + 1) << 32
            x = x - (1 << 64) if x >> 63 else x
            if fn == 0x21:
                bits = struct.unpack(">Q", struct.pack(">d", float(x)))[0]   # int -> double rounds to nearest
                uc.reg_write(REG["f%d" % fd], bits & 0xFFFFFFFF)
                uc.reg_write(REG["f%d" % (fd + 1)], bits >> 32)
            else:
                # round to 24 significant bits first (nearest-even) so int -> double -> single is exact
                a, s = abs(x), -1 if x < 0 else 1
                n = a.bit_length()
                if n > 24:
                    sh = n - 24
                    q, r, half = a >> sh, a & ((1 << sh) - 1), 1 << (sh - 1)
                    if r > half or r == half and q & 1:
                        q += 1
                    a = q << sh
                uc.reg_write(REG["f%d" % fd], fbits(float(s * a)))
            return True
        if fmt == 16:
            v = struct.unpack(">f", struct.pack(">I", lo(fs)))[0]
        else:
            v = struct.unpack(">d", struct.pack(">Q", lo(fs) | lo(fs + 1) << 32))[0]
        mode = self.FLOAT_TO_L[fn]
        if mode is None:
            mode = u32(uc.reg_read(REG["fcsr"])) & 3
        if v != v or v in (float("inf"), float("-inf")):
            return False                    # the VR4300 traps (unimplemented operation)
        r = [round, math.trunc, math.ceil, math.floor][mode](v)
        if not -(1 << 63) <= r < (1 << 63):
            return False
        r &= (1 << 64) - 1
        uc.reg_write(REG["f%d" % fd], r & 0xFFFFFFFF)
        uc.reg_write(REG["f%d" % (fd + 1)], r >> 32)
        return True

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
        # Restore the writable regions, then apply this trial's inputs.  Only
        # pages that the previous run wrote or that held its inputs differ
        # from base, so only those are copied back (memory and snap alike).
        self.restore()
        writes = [(phys(addr_fn(self.b)), data) for addr_fn, data in plan.mem]
        writes += [(phys(a), struct.pack(">I", v & 0xFFFFFFFF)) for a, v in self.opts.mmio_vals.items()]
        for p, data in writes:
            ln = len(data)
            for ri, (s, n) in enumerate(self.regions):
                if s <= p and p + ln <= s + n:
                    o = p - s
                    ctypes.memmove(ctypes.addressof(self.snap[ri]) + o, data, ln)
                    uc.mem_write(p, data)
                    for pg in range(o // PAGE, (o + ln - 1) // PAGE + 1):
                        self.dirty.add((ri, pg * PAGE))
                    break
            else:
                # outside the diffed regions (e.g. code): undone before the next run
                self.dirty_other.append((p, bytes(uc.mem_read(p, ln))))
                uc.mem_write(p, data)
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
        self.diff()
        return res

    def host(self, p):
        """Host address of guest physical address p (RAM or MMIO)."""
        if p < RAM_SIZE:
            return self.ram_addr + p
        return self.mmio_addr + (p - MMIO_BASE)

    def restore(self):
        uc = self.uc
        for ri, off in self.dirty:
            s, n = self.regions[ri]
            ln = min(PAGE, n - off)
            src = ctypes.addressof(self.base[ri]) + off
            ctypes.memmove(ctypes.addressof(self.snap[ri]) + off, src, ln)
            uc.mem_write(s + off, ctypes.string_at(src, ln))
        self.dirty.clear()
        for p, data in reversed(self.dirty_other):
            uc.mem_write(p, data)
        self.dirty_other = []

    def diff(self):
        """Add every byte that differs from snap to self.written: memcmp over
        64 KB chunks, then pages, then a byte compare inside changed pages."""
        CHUNK = 0x10000
        for ri, (s, n) in enumerate(self.regions):
            la, sa = self.host(s), ctypes.addressof(self.snap[ri])
            seg = 0xA0000000 if 0x04000000 <= s < 0x05000000 else 0x80000000
            for c0 in range(0, n, CHUNK):
                c1 = min(c0 + CHUNK, n)
                if _memcmp(la + c0, sa + c0, c1 - c0) == 0:
                    continue
                for lo in range(c0, c1, PAGE):
                    hi = min(lo + PAGE, n)
                    if _memcmp(la + lo, sa + lo, hi - lo) == 0:
                        continue
                    self.dirty.add((ri, lo))
                    o, c = ctypes.string_at(sa + lo, hi - lo), ctypes.string_at(la + lo, hi - lo)
                    for i in range(0, hi - lo, 64):
                        if o[i:i + 64] != c[i:i + 64]:
                            self.written.update(seg | (s + lo + j) for j in range(i, min(i + 64, hi - lo))
                                                if o[j] != c[j])

    def read(self, a, n=1):
        p = phys(a)
        if p + n <= RAM_SIZE or MMIO_BASE <= p and p + n <= MMIO_BASE + MMIO_SIZE:
            return ctypes.string_at(self.host(p), n)
        return bytes(self.uc.mem_read(p, n))


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
        self.outp = {}            # convention pointer slot -> scratch block address
        self.side_regs = {"asm": {}, "c": {}}   # convention inputs that differ between the sides

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
        if name.startswith("sym:"):         # rel:sym:NAME+.. is the same as rel:NAME+..
            n = name[4:].strip()
            if not re.match(r"^[A-Za-z_]\w*$", n):
                raise SystemExit("%s: bad symbol name %r" % (what, n))
            return lambda b, n=n: b.resolve(n)
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
        name = parts[1].strip() if len(parts) > 1 else ""
        if not re.match(r"^[A-Za-z_]\w*\s*([+-]\s*(0x[0-9A-Fa-f]+|\d+))?$", name):
            raise SystemExit("bad value spec for %s: %r (want sym:NAME or sym:NAME+OFF)" % (what, spec))
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


STRIDE_RE = re.compile(r"^(.*?)\s*\*\s*(0x[0-9A-Fa-f]+|\d+)\s*:\s*(0x[0-9A-Fa-f]+|\d+)(?:/(0x[0-9A-Fa-f]+|\d+))?$")


def gen_mem(spec, rng, plan):
    """--mem TARGET[:SIZE]=FILL, or TARGET*STRIDE:COUNT[/W]=FILL (the same W-byte
    field in COUNT records STRIDE bytes apart, each drawn separately)"""
    if "=" not in spec:
        raise SystemExit("bad --mem %r: want TARGET[:SIZE]=FILL" % spec)
    tgt, fill = spec.split("=", 1)
    m = STRIDE_RE.match(tgt)
    if m:
        tgt = m.group(1)
        stride, count = int(m.group(2), 0), int(m.group(3), 0)
        width = int(m.group(4), 0) if m.group(4) else 4
        if count < 1 or width < 1:
            raise SystemExit("bad --mem %r: COUNT and W must be >= 1" % spec)
        base = mem_target(tgt, plan)
        descs = []
        for i in range(count):
            res = (lambda b, base=base, o=i * stride: (base(b) + o) & 0xFFFFFFFF)
            data, desc = gen_fill(fill, width, rng, plan, tgt)
            if callable(data):
                if width != 4:
                    raise SystemExit("--mem %s: a pointer fill needs W = 4" % spec)
                plan.mem.append((res, None, data))
            else:
                plan.mem.append((res, data))
            descs.append(desc)
        if len(set(descs)) == 1:
            shown = descs[0]
        else:
            shown = "; ".join(descs[:8]) + ("; ..." if count > 8 else "")
        plan.desc.append("%s*%#x x%d = %s" % (tgt, stride, count, shown))
        return
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


def size_mem_specs(opts, b):
    """Unsized `--mem SYM=FILL` writes a word.  If the ELF gives SYM a size of
    1 or 2 bytes, size the write from it; otherwise warn when the next symbol
    starts less than 4 bytes after the target (a u8/u16 global whose absolute
    symbol has no size)."""
    out = []
    for spec in opts.mem:
        tgt, _, fill = spec.partition("=")
        kind = fill.split(":")[0]
        m = re.match(r"^([A-Za-z_]\w*)\s*(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?$", tgt.strip())
        if ":" in tgt or "*" in tgt or not m or m.group(1) not in b.sym \
                or kind in ("halves", "bytes", "onehot"):
            out.append(spec)
            continue
        name, off = m.group(1), int(m.group(2), 0) if m.group(2) else 0
        sz = b.size.get(name)
        if off == 0 and sz in (1, 2):
            out.append("%s:%d=%s" % (tgt, sz, fill))
            print("note: --mem %s: %s is %d byte(s) in the ELF, writing %d byte(s)" % (tgt, name, sz, sz))
            continue
        out.append(spec)
        a = b.resolve(tgt)
        i = bisect.bisect_right(b._keys, a)
        if i < len(b._keys) and b._keys[i] - a < 4:
            print("warning: --mem %s writes 4 bytes, but %s starts %d byte(s) after it; give the size "
                  "(%s:1=... or %s:2=...)" % (tgt, b.addr_syms[i][1], b._keys[i] - a, tgt, tgt))
    opts.mem = out


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
    # --heap blocks: allocated first (heap0, heap1, ..), not bound to any register
    for spec in opts.heap:
        sz, _, fill = spec.partition("=")
        try:
            size = int(sz, 0)
        except ValueError:
            raise SystemExit("bad --heap %r: want SIZE or SIZE=FILL" % spec)
        a = plan.alloc(size)
        k = len(plan.blocks) - 1
        data, desc = gen_fill(fill or "rand", size, rng, plan, "heap%d" % k)
        if callable(data) or len(data) != size:
            raise SystemExit("bad --heap %r: FILL must produce SIZE plain bytes (rand, zero, words:.., ..)" % spec)
        plan.mem.append((lambda b, a=a: a, data))
        plan.desc.append("heap%d = 0x%08X (0x%X, %s)" % (k, a, size, desc))
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
    if opts.tconv is not None:
        conv_plan(plan, opts.tconv, rng)
    for spec in opts.mem:
        gen_mem(spec, rng, plan)
    return plan


def conv_plan(plan, conv, rng):
    """The function under test has a register convention: every input gets the
    same value in its asm register (asm side) and its C slot (C side).  Where
    a register isn't claimed by the other side's convention it gets the value
    on both sides, so either version reads the same thing and callee-saved
    comparisons stay fair; where it is (asm a0 is one input, C slot a0
    another) the two sides differ (plan.side_regs).  Each pointer slot points
    at a scratch block outside the diffed memory (OUTP_BASE..) whose words the
    C version reads inputs from / writes outputs to.  --arg REG names an asm
    register when REG is one of the convention's asm registers, else a C slot."""
    frame = plan.mem[0][1]          # the poisoned caller frame at STACK_TOP
    given = {}
    for k, v in plan.vals.items():
        m = re.match(r"^sp\+(0x[0-9A-Fa-f]+|\d+)$", k)
        given["stack%d" % ((int(m.group(1), 0) - 0x10) // 4) if m else k] = v
    asm_regs = set(r for r, _ in conv.ins + conv.outs if not r.startswith("stack"))
    c_slots = set(l[1] for _, l in conv.ins + conv.outs if not l[1].startswith("stack"))

    def set_slot(slot, v):
        if slot.startswith("stack"):
            o = slot_off(slot)
            if callable(v):
                plan.mem.append((lambda b, o=o: STACK_TOP + o, None, v))
            else:
                plan.mem.append((lambda b, o=o: STACK_TOP + o, struct.pack(">I", v & 0xFFFFFFFF)))
            plan.vals["sp+0x%x" % o] = v
            return
        plan.side_regs["c"][slot] = v
        if slot not in asm_regs:
            plan.regs[slot] = v
            plan.vals[slot] = v

    def set_asm(r, v):
        plan.side_regs["asm"][r] = v
        plan.vals.setdefault(r, v)
        if r not in c_slots:
            plan.regs[r] = v

    def get_slot(slot):
        if slot.startswith("stack"):
            o = slot_off(slot)
            return struct.unpack(">I", frame[o:o + 4])[0]
        return plan.regs[slot]

    blocks = {}
    for i, slot in enumerate(conv.ptr_slots()):
        if slot in given and slot not in asm_regs:
            raise SystemExit("--arg %s: %s is a pointer slot of %s's convention (the harness sets it)"
                             % (slot, slot, conv.name))
        a = OUTP_BASE + 0x100 * i
        plan.mem.append((lambda b, a=a: a, bytes(rng.getrandbits(8) for _ in range(0x100))))
        set_slot(slot, a)
        blocks[slot] = a
    plan.outp = blocks
    for r, loc in conv.ins:
        kind, slot, off, w = loc
        cslot = slot if kind == "reg" and slot not in asm_regs else None
        if r in given and cslot in given and r != cslot:
            raise SystemExit("--arg gives both %s and %s, which %s's convention ties together" % (r, cslot, conv.name))
        if r in given:
            v = given[r]
        elif cslot in given:
            v = given[cslot]
        elif r.startswith("stack"):
            v = get_slot(r)
        elif kind == "reg":
            v = get_slot(slot)
        else:
            v = plan.regs[r]
        if kind == "mem":
            if callable(v):
                raise SystemExit("%s: a pointer-slot input (%s) must be a number" % (conv.name, loc_str(loc)))
            v &= (1 << (8 * w)) - 1
            plan.mem.append((lambda b, a=blocks[slot] + off: a, v.to_bytes(w, "big")))
        else:
            set_slot(slot, v)
        if r.startswith("stack"):
            if r != slot:
                set_slot(r, v)
        else:
            set_asm(r, v)


class PlanView:
    """A plan with per-build pointer values filled in."""

    def __init__(self, plan, build, side=None):
        regs = dict(plan.regs)
        if side is not None:
            regs.update(plan.side_regs[side])
        self.regs = {r: (v(build) if callable(v) else v) for r, v in regs.items()}
        self.fcsr = plan.fcsr
        self.outp = plan.outp
        self.mem = []
        for e in plan.mem:
            if len(e) == 3:
                res, _, valfn = e
                self.mem.append((res, struct.pack(">I", valfn(build))))
            else:
                self.mem.append(e)


# ---------------------------------------------------------------------------
def conv_outputs(conv, side, m, res, plan):
    """The function under test's outputs, read on its side of the convention:
    the asm registers, or v0/f0/the pointer-slot scratch words for C."""
    out = []
    for r, loc in conv.outs:
        kind, where, off, w = loc
        if side == "asm":
            v = res[r]
        elif kind == "reg":
            v = res[where]
        else:
            v = int.from_bytes(m.read(plan.outp[where] + off, w), "big")
        out.append(v & ((1 << (8 * w)) - 1))
    return out


def compare(opts, ref_m, new_m, amap, r_ref, r_new, outs=None):
    """Returns a list of difference strings (empty = equivalent).  OUTS is a
    list of (label, ref value, new value) convention outputs."""
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

    # convention outputs (asm register vs C return value / pointer argument)
    for label, vr, vn in outs or []:
        if not same_val(vr, vn):
            diffs.append("output %s: %s=0x%08X %s=0x%08X" % (label, ref_m.b.label, vr, new_m.b.label, vn))

    # registers
    regs = list(opts.ret_regs) + ([] if opts.no_saved else [r for r in SAVED_REGS if r not in opts.ret_regs])
    for r in regs:
        if r in opts.ignore_regs or r in opts.conv_skip_saved and r not in opts.ret_regs:
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
                         "onehot:STRIDE[@OFF][/W]:POOL:FILL, or any --arg SPEC); applied after --arg. "
                         "TARGET*STRIDE:COUNT[/W]=FILL fills the same W-byte field (default 4) of COUNT "
                         "records. Unsized SYM= writes are sized from the ELF symbol size when it is 1 or 2")
    ap.add_argument("--heap", action="append", default=[],
                    help="SIZE[=FILL]: allocate a scratch-heap block (heap0, heap1, .. before any --arg ptr) "
                         "without putting its address in a register; FILL defaults to rand")
    ap.add_argument("--ret", default=None,
                    help="return kind: int, ptr, void, u64, float, double, or regs:v0,t0,... (default int; "
                         "void when FUNC has a full convention, whose outputs are compared instead)")
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
                    help="NAME=VALUE|rand|ptr:SIZE : value stubbed NAME returns in v0 / its first convention "
                         "output (default rand); NAME.REG=.. sets the convention output in asm register REG")
    ap.add_argument("--stub-preserve", action="append", default=[],
                    help="NAME=v1,a0 : registers a stubbed NAME must leave alone (the default stub writes "
                         "v0, v1 and f0); NAME=* for every stub")
    ap.add_argument("--conv", dest="conv_lines", action="append", default=[],
                    help="'NAME: in REG=SLOT,.. ; out REG=DEST,.. ; preserve REG,.. ; clobbers REG,..' "
                         "convention line, added to (or replacing) the conventions file's")
    ap.add_argument("--conv-file", default=CONV_FILE, help="conventions file (default tools_port/conventions.txt)")
    ap.add_argument("--model", action="append", default=[],
                    help="NAME=FILE.py[:FUNC] : run Python FUNC(args, mem) (default `model`) in place of NAME "
                         "in both builds; FILE is also looked up in tools_port/models/")
    ap.add_argument("--no-conv", action="store_true", help="ignore all register conventions")
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
    # register conventions
    opts.convs = {} if o.no_conv else load_convs(o.conv_file, must_exist=o.conv_file != CONV_FILE)
    for i, line in enumerate(o.conv_lines):
        try:
            c = parse_conv_line(line, "--conv #%d" % (i + 1))
        except ValueError as e:
            raise SystemExit("eqcheck: %s" % e)
        opts.convs[c.name] = c
    opts.models = dict(load_model(s) for s in o.model)
    tc = opts.convs.get(o.func)
    opts.tconv =tc if tc is not None and tc.full else None
    # registers the function under test may legitimately change although the
    # o32 ABI saves them (its asm outputs and documented clobbers): the C
    # version keeps them, so they are not compared as callee-saved
    opts.conv_skip_saved = set()
    if tc is not None:
        opts.conv_skip_saved = set(r for r, _ in tc.outs) | tc.clobbers
    ret = o.ret if o.ret is not None else ("void" if opts.tconv else "int")
    if ret.startswith("regs:"):
        opts.ret_regs = ret[5:].split(",")
    else:
        if ret not in RET_KINDS:
            raise SystemExit("eqcheck: bad --ret %r" % ret)
        opts.ret_regs = RET_KINDS[ret]
    sigs = {}
    default_sig = ["a0", "a1", "a2", "a3"]
    for s in o.sig:
        n, regs = s.split("=", 1)
        lst = [r.strip() for r in regs.split(",") if r.strip()]
        for r in lst:
            if r not in REG and not re.match(r"^(stack\d+|sp\+(0x[0-9A-Fa-f]+|\d+))$", r) \
                    and not (n in opts.convs and r.startswith("*")):
                raise SystemExit("eqcheck: --sig %s: unknown register/slot %r" % (s, r))
        if n == "*":
            default_sig = lst
        else:
            sigs[n] = lst
    opts.sigs = sigs
    opts.sig_for = lambda name: sigs.get(name, default_sig)
    preserve = {}
    for s in o.stub_preserve:
        if "=" not in s:
            raise SystemExit("eqcheck: bad --stub-preserve %r: want NAME=REG,REG" % s)
        n, regs = s.split("=", 1)
        try:
            preserve.setdefault(n, set()).update(expand_regs(regs, "--stub-preserve"))
        except ValueError as e:
            raise SystemExit("eqcheck: %s" % e)

    def preserve_for(name):
        c = opts.convs.get(name)
        return (c.preserve if c is not None else set()) | preserve.get(name, set()) | preserve.get("*", set())
    opts.preserve_for = preserve_for
    rets = {}
    for s in o.stub_ret:
        n, v = s.split("=", 1)
        rets[n] = v

    def out_value(name, k, idx, reg, machine):
        """Canned value for output IDX (asm register REG) of a stubbed NAME."""
        spec = rets.get("%s.%s" % (name, reg))
        if spec is None and idx == 0:
            return ret_for(name, k, machine)[0]
        spec = spec or "rand"
        if spec == "rand":
            h = hashlib.sha1(("%s/%s/%s/%d/%d" % (opts.seed, opts._trial, name, k, idx)).encode()).digest()
            r0 = struct.unpack(">I", h[:4])[0]
            return r0 if r0 & 1 else r0 & 0xFF
        if spec.startswith("ptr"):
            return ret_for_spec(spec, name, k, machine)[0]
        return int(spec, 0) & 0xFFFFFFFF
    opts.out_value = out_value

    def ret_for(name, k, machine):
        return ret_for_spec(rets.get(name, "rand"), name, k, machine)

    def ret_for_spec(spec, name, k, machine):
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
    # Parsing the ELF symbol tables takes most of a short run's startup, so the
    # parsed Build is cached next to the ELFs, keyed by their size/mtime and by
    # this script's own mtime.
    key = tuple((os.path.abspath(p), os.stat(p).st_size, os.stat(p).st_mtime_ns)
                for p in (ini, hd, os.path.abspath(__file__)))
    cache = os.path.join(d, ".eqcheck_cache.%s.pickle" % version)
    try:
        with open(cache, "rb") as f:
            k, b = pickle.load(f)
        if k == key:
            b.label = label
            return b
    except Exception:
        pass
    b = Build(label, [ini, hd])
    tmp = "%s.%d.tmp" % (cache, os.getpid())
    try:
        with open(tmp, "wb") as f:
            pickle.dump((key, b), f, protocol=pickle.HIGHEST_PROTOCOL)
        os.replace(tmp, cache)
    except Exception:
        try:
            os.remove(tmp)
        except OSError:
            pass
    return b


def explore(opts, ref, m, entry):
    plan = make_plan(opts, 0)
    r = m.run(entry, PlanView(plan, ref, m.func_side(opts.func) if opts.tconv else None), track_reads=True)
    print("explore %s @0x%08X (%s), trial 0 inputs: %s" % (opts.func, entry, ref.label, "; ".join(plan.desc) or "-"))
    print("outcome:", "returned" if m.fault is None else m.fault)
    print("return v0=0x%08X v1=0x%08X f0=0x%08X" % (r["v0"], r["v1"], r["f0"]))
    if opts.func in opts.convs:
        c = opts.convs[opts.func]
        print("convention: %s" % c.text())
        if opts.tconv is not None and m.fault is None:
            side = m.func_side(opts.func)
            vals = conv_outputs(c, side, m, r, PlanView(plan, ref))
            print("outputs (%s side): %s" % (side, ", ".join("%s=0x%X" % (rr, v) for (rr, _), v in zip(c.outs, vals))))

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
            else:
                # (loads from the text range are data tables embedded in it,
                # e.g. D_802C23B4 in the 7D9D0 blob; instruction fetches are
                # not recorded)
                intext = ref.in_text(a)
                n, off = ref.sym_off(a, skip_labels=intext)
                if n is not None and off < 0x10000:
                    key = n + (" (in text)" if intext else "")
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
        size_mem_specs(opts, ref)
        explore(opts, ref, ref_m, ref.sym[opts.func])
        return 0
    new = load_build(opts.new, opts.version, opts.new.rstrip("/"))
    if opts.new.rstrip("/") == opts.ref.rstrip("/"):
        new.label = new.label + "(2)"
    if opts.func not in new.sym:
        sys.exit("eqcheck: %s not in %s" % (opts.func, new.label))
    amap = AddrMap(ref, new)
    new_m = Machine(new, prefill=None if amap.identity else ref, opts=opts)
    # functions only the new build has (static helpers of a rewrite) are part
    # of the rewrite: run them instead of recording them as extra calls
    new_m.auto_follow = set(n for n in new.funcs.values() if n not in ref.sym)
    size_mem_specs(opts, ref)
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
    tc = opts.tconv
    side_ref = side_new = None
    if opts.func in opts.convs:
        c = opts.convs[opts.func]
        side_ref, side_new = ref_m.func_side(opts.func), new_m.func_side(opts.func)
        print("convention %s: %s  (%s: %s, %s: %s)" % (opts.func, c.text(), ref.label, side_ref,
                                                      new.label, "C" if side_new == "c" else side_new))
    fails = 0
    both_faulted = 0
    notes = set()
    t0 = time.time()
    for t in range(opts.trials):
        opts._trial = t
        plan = make_plan(opts, t)
        r_ref = ref_m.run(e_ref, PlanView(plan, ref, side_ref))
        r_new = new_m.run(e_new, PlanView(plan, new, side_new))
        for lbl, m in ((ref.label, ref_m), (new.label, new_m)):
            for n in m.notes:
                if (lbl, n) in notes:
                    continue
                notes.add((lbl, n))
                if n[0] == "helper":
                    print("note: following %s, which exists only in %s (static helper?)" % (n[1], lbl))
                elif n[0] == "badptr":
                    print("note: %s: stubbed %s's pointer argument is unmapped, output not written"
                          % (lbl, n[1]))
                elif m is new_m:
                    print("note: %s executed original-build code at %s (not a function entry; "
                          "jump table or pointer in a binary blob?)" % (new.label, ref.name_at(n[1])))
        if ref_m.fault and new_m.fault:
            both_faulted += 1
        outs = None
        if tc is not None and ref_m.fault is None and new_m.fault is None:
            pv = PlanView(plan, ref)
            outs = list(zip([("%s=%s" % (r, loc_str(l))) for r, l in tc.outs],
                            conv_outputs(tc, side_ref, ref_m, r_ref, pv),
                            conv_outputs(tc, side_new, new_m, r_new, pv)))
        diffs = compare(opts, ref_m, new_m, amap, r_ref, r_new, outs)
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
