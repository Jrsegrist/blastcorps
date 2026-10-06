#!/usr/bin/env python3
"""Frame-by-frame comparison of bc_headless.exe with the emulator.

The emulator (mupen64plus through tools_port/m64trace, spec port/tools/cmp_spec.py)
runs a ROM (default: the NON_MATCHING test ROM, which runs the same C as the
exe) and logs, per frame, the point where the frame-ending gfx task starts
(where bc_headless dumps), when each frame's RDP work finishes, and every
osGetTime/osGetCount result with its caller.  From that log the exe gets the
emulator's clock (--clock, --frame-done, --boot-count), runs the same frames
and dumps RDRAM at the same frame numbers; the dumps are then compared word by
word after normalising byte order (see diff below).

usage (run from port/; `make -C port compare DEMO=n` drives it):
  compare.py emu    EMUDIR [--rom ROM] [--vis N] [--dump SPEC] [--stop F]
  compare.py inject EMUDIR                    -> EMUDIR/{clock,framedone,boot}.txt
  compare.py native EMUDIR NATDIR [--frames N] [--dump SPEC] [--exe EXE] [--rom ROM]
  compare.py diff   EMUDIR NATDIR [--from F] [--to F] [--detail N] [--all]
  compare.py frames EMUDIR NATDIR             per-frame timeline (vi, mode, level) side by side
  compare.py demos  EMUDIR                    frame ranges of the attract demos in the log

Dump SPEC: "every:N", "a-b", "f1,f2" joined with '+'.

Byte order.  The emulator dumps are N64 big-endian; the exe's are host order
with every multi-byte field swapped by the width the code reads it at.  A word
matches when some layout of it agrees: one u32 (or f32/pointer), two u16, u16
+ two bytes, four bytes, or (8-aligned pairs) one u64/f64.  Code pointers match
when both name the same function (emulator: the ROM's ELFs; exe: --syms).
Regions that legitimately differ are listed in tools/compare_ignore.txt.
"""
import array, bisect, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
ROOT = os.path.dirname(PORT)  # blastcorps/
SIZE = 0x400000  # the game only uses the first 4 MB
BASE = 0x80000000
NATIVE_PERIOD = 781250


def die(msg):
    sys.stderr.write("compare: %s\n" % msg)
    sys.exit(1)


def opt(args, name, default=None, conv=str):
    if name in args:
        i = args.index(name)
        v = args[i + 1]
        del args[i:i + 2]
        return conv(v)
    return default


def flag(args, name):
    if name in args:
        args.remove(name)
        return True
    return False


# ---------------------------------------------------------------- symbols

class Syms:
    """sorted (addr, name, size) from ELF symtabs / nm output"""

    def __init__(self):
        self.items = []

    def add(self, addr, name, size=0):
        self.items.append((addr, name, size))

    def done(self):
        self.items.sort()
        self.addrs = [a for a, _, _ in self.items]
        return self

    def find(self, addr):
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i < 0:
            return None
        return self.items[i]

    def name(self, addr):
        it = self.find(addr)
        if it is None:
            return "%08X" % addr
        a, n, _ = it
        return n if a == addr else "%s+0x%X" % (n, addr - a)


def elf_syms(paths, want):
    from elftools.elf.elffile import ELFFile
    s = Syms()
    for p in paths:
        with open(p, "rb") as f:
            for sec in ELFFile(f).iter_sections():
                if sec.name != ".symtab":
                    continue
                for sym in sec.iter_symbols():
                    t = sym["st_info"]["type"]
                    if not sym.name or sym.name.startswith(("$", ".")) or t not in want:
                        continue
                    s.add(sym["st_value"], sym.name, sym["st_size"])
    return s.done()


def nm_syms(path):
    s = Syms()
    for line in open(path):
        p = line.split()
        if len(p) == 3 and p[1] in "Tt" and len(p[0]) == 8:
            name = p[2][1:] if p[2].startswith("_") else p[2]
            s.add(int(p[0], 16), name.split(".")[0])
    return s.done()


def rom_elfs(rom_kind):
    if rom_kind == "nm":
        return [os.path.join(ROOT, "build_nm/hd_code.rom.us.v11.elf"),
                os.path.join(ROOT, "build_nm/hd_front_end.rom.us.v11.elf")]
    return [os.path.join(ROOT, "build/hd_code.us.v11.elf"), os.path.join(ROOT, "build/hd_front_end.us.v11.elf")]


def data_syms():
    """data/bss names: the NON_MATCHING ELFs (pinned at the N64 addresses) + gensyms' list"""
    s = elf_syms([os.path.join(ROOT, "build_nm/hd_code.us.v11.elf"),
                  os.path.join(ROOT, "build_nm/hd_front_end.us.v11.elf")],
                 ("STT_OBJECT", "STT_NOTYPE"))
    have = set(a for a, _, _ in s.items)
    p = os.path.join(PORT, "build/headless/addrs.txt")
    if os.path.exists(p):
        for line in open(p):
            f = line.split()
            if len(f) >= 2:
                a = int(f[1], 16)
                if a not in have:
                    s.add(a, f[0], int(f[2]) if len(f) > 2 else 0)
    s.items = [it for it in s.items if BASE <= it[0] < BASE + 0x800000]
    return s.done()


# ---------------------------------------------------------------- emulator log

def parse_emu(emudir):
    ev = {"F": [], "R": [], "T": [], "C": [], "A": [], "U": [], "B": None, "V": [], "M": []}
    wrap = [0, 0]  # added, previous: the 32-bit count register, unwrapped in log (= time) order

    def unwrap(c):
        if c + wrap[0] < wrap[1] - (1 << 31):
            wrap[0] += 1 << 32
        wrap[1] = c + wrap[0]
        return wrap[1]

    for line in open(os.path.join(emudir, "emu.txt")):
        if line.startswith("#in "):
            m = re.match(r"#in vi=\d+ V (\d+) count=(\d+)", line)
            if m:
                ev["V"].append((int(m.group(1)), unwrap(int(m.group(2)))))
            continue
        if line.startswith("#"):
            continue
        i = line.find("| ")
        if i < 0:
            continue
        m = re.match(r"\S+ vi=(\d+) ", line)
        vi = int(m.group(1)) if m else 0
        body = line[i + 2:].split()
        kind = body[0]
        kv = dict(x.split("=", 1) for x in body[1:] if "=" in x)
        if kind == "F":
            kv["n"] = int(body[1])
            ev["F"].append(kv)
        elif kind == "R":
            ev["R"].append(kv)
        elif kind in ("T", "C", "A", "U"):
            ev[kind].append((int(kv["ra"], 16), int(kv["th"]), int(kv["v"], 16), vi))
        elif kind == "M":
            ev["M"].append((int(kv["ra"], 16), int(kv["th"]), unwrap(int(kv["c"])), kv["f"], int(kv.get("q", "0"), 16)))
        elif kind == "B":
            kv["count"] = str(unwrap(int(kv["count"])))
            ev["B"] = kv
    return ev


def emu_timing(ev):
    v = ev["V"]
    if len(v) < 2 or ev["B"] is None:
        die("emulator log has no V/B lines")
    period = (v[-1][1] - v[0][1]) / float(v[-1][0] - v[0][0])
    boot = int(ev["B"]["count"])
    c1 = v[0][1] - (v[0][0] - 1) * period  # count at emulator VI 1
    delta = int(round((c1 - boot) / period * NATIVE_PERIOD))
    r1 = int(round((boot + delta) / float(NATIVE_PERIOD)))
    # The emulator's first VI after hd_code starts is its VI 1 (its VI
    # interrupts start with the VI setup): no native retrace before it.
    delta = min(delta, NATIVE_PERIOD - 1000)
    native_boot = r1 * NATIVE_PERIOD - delta
    ev["to_native"] = lambda c: int(r1 * NATIVE_PERIOD + (c - c1) * NATIVE_PERIOD / period)
    ev["r1"], ev["c1"] = r1, int(c1)
    return period, native_boot, r1 - 1  # native retrace = emulator VI + offset


def cmd_inject(args):
    emudir = args[0]
    ev = parse_emu(emudir)
    period, boot, off = emu_timing(ev)
    elfs = open(os.path.join(emudir, "elfs.txt")).read().split()
    fs = elf_syms(elfs, ("STT_FUNC", "STT_NOTYPE"))
    fs.items = [it for it in fs.items if it[0] >= 0x80000000]
    fs.done()
    with open(os.path.join(emudir, "clock.txt"), "w") as f:
        # the emulator's clock: count = C1 + (native time - R1 * 781250) * PERIOD / 781250
        f.write("M %d %d %.6f\n" % (ev["r1"], ev["c1"], period))
        for kind in "TCAU":
            for ra, th, v, vi in ev[kind]:
                it = fs.find(ra)
                f.write("%s %s %d %x\n" % (kind, it[1] if it else "?", vi + off, v))
    with open(os.path.join(emudir, "framedone.txt"), "w") as f:
        for n, r in enumerate(ev["R"], 1):
            fr = min(int(r["d"]) / period, 0.999999)
            f.write("@%d %d.%06d\n" % (n, int(r["vi"]) + off, int(fr * 1000000)))
    nsync = 0
    with open(os.path.join(emudir, "sync.txt"), "w") as f:
        kinds = {"osSendMesg": "s", "osRecvMesg": "r", "osJamMesg": "j", "osStartThread": "t"}
        for ra, th, c, fn, q in ev["M"]:
            it = fs.find(ra)
            if it and re.match(r"^func_[0-9A-F]{8}$", it[1]):
                f.write("S %d %s %s %X %d\n" % (th, it[1], kinds[fn], q, ev["to_native"](c)))
                nsync += 1
    with open(os.path.join(emudir, "boot.txt"), "w") as f:
        f.write("%d %d %.3f\n" % (boot, off, period))
    print("inject: %d frames, %d osGetTime, %d osGetCount values, %d sync points; VI period %.1f counts, "
          "boot count %d, native retrace = emulator VI + %d"
          % (len(ev["F"]), len(ev["T"]), len(ev["C"]), nsync, period, boot, off))


# ---------------------------------------------------------------- runs

def wpath(p):
    return subprocess.check_output(["wslpath", "-w", os.path.abspath(p)]).decode().strip()


def default_rom(kind):
    if kind == "nm":
        return os.path.join(ROOT, "build_nm/blastcorps.nm.us.v11.z64")
    for p in (os.path.join(ROOT, "../baserom.us.v11.z64"), os.path.expanduser("~/blastcorps/baserom.us.v11.z64")):
        if os.path.exists(p):
            return p
    die("no base ROM")


def cmd_emu(args):
    emudir = args.pop(0)
    kind = opt(args, "--kind", "nm")
    rom = opt(args, "--rom", None) or default_rom(kind)
    os.makedirs(emudir, exist_ok=True)
    elfs = rom_elfs(kind)
    with open(os.path.join(emudir, "elfs.txt"), "w") as f:
        f.write("\n".join(elfs) + "\n")
    env = dict(os.environ, CMP_DIR=emudir, CMP_ELFS=",".join(elfs), CMP_VIS=str(opt(args, "--vis", 36000)),
               CMP_DUMP=opt(args, "--dump", "every:10"), CMP_STOP=str(opt(args, "--stop", 0)))
    py = sys.executable
    tracer = os.path.join(ROOT, "tools_port/m64trace/m64trace.py")
    with open(os.path.join(emudir, "emu.log"), "w") as log:
        r = subprocess.call([py, tracer, rom, os.path.join(HERE, "cmp_spec.py"), os.path.join(emudir, "emu.txt")],
                            env=env, stdout=log, stderr=subprocess.STDOUT, cwd=ROOT)
    if r != 0:
        die("emulator run failed (%s)" % os.path.join(emudir, "emu.log"))
    cmd_inject([emudir])


def cmd_native(args):
    emudir, natdir = args[0], args[1]
    args = args[2:]
    exe = opt(args, "--exe", os.path.join(PORT, "build/headless/bc_headless.exe"))
    rom = opt(args, "--rom", None) or default_rom("base")
    frames = opt(args, "--frames", None)
    dump = opt(args, "--dump", "every:10")
    extra = args
    os.makedirs(natdir, exist_ok=True)
    for f in os.listdir(natdir):
        if f.startswith("frame_"):
            os.remove(os.path.join(natdir, f))
    boot, off, _ = open(os.path.join(emudir, "boot.txt")).read().split()
    every, lst = parse_dump(dump)
    cmd = [exe, wpath(rom), "--boot-count", boot, "--clock", wpath(os.path.join(emudir, "clock.txt")),
           "--syms", wpath(exe[:-4] + ".syms"), "--frame-done", wpath(os.path.join(emudir, "framedone.txt")),
           "--trace", wpath(os.path.join(natdir, "trace.txt")), "--dump-dir", wpath(natdir)]
    if not flag(extra, "--no-sync"):
        cmd += ["--sync", wpath(os.path.join(emudir, "sync.txt"))]
    if every:
        cmd += ["--dump-every", str(every)]
    if lst:
        cmd += ["--dump", ",".join(str(x) for x in sorted(lst))]
    if frames:
        cmd += ["--frames", str(frames)]
    cmd += extra
    with open(os.path.join(natdir, "run.log"), "w") as log:
        r = subprocess.call(cmd, stdout=log, stderr=subprocess.STDOUT, cwd=natdir)
    print("native: exit %d (%s)" % (r, os.path.join(natdir, "run.log")))
    return r


def parse_dump(s):
    every, frames = 0, set()
    for part in s.split("+"):
        if part.startswith("every:"):
            every = int(part[6:])
        elif "-" in part:
            a, b = part.split("-")
            frames.update(range(int(a), int(b) + 1))
        elif part:
            frames.update(int(x) for x in part.split(","))
    return every, frames


# ---------------------------------------------------------------- comparison

def load_ignore(dsyms):
    """tools/compare_ignore.txt: `LO HI reason` (hex, [LO, HI)) or `SYMBOL [SIZE] reason`;
    plus the .data/.bss of the libultra objects the platform layer replaces"""
    mask = bytearray(SIZE // 4)
    reasons = []

    def ign(lo, hi, why):
        lo, hi = max(lo, BASE), min(hi, BASE + SIZE)
        if hi <= lo:
            return
        a, b = (lo - BASE) // 4, (hi - BASE + 3) // 4
        mask[a:b] = b"\x01" * (b - a)
        reasons.append((lo, hi, why))

    byname = {}
    for a, n, sz in dsyms.items:
        byname.setdefault(n, (a, sz))
    small = []  # re-applied after the pinned tables are unmasked

    def ign_small(lo, hi, why):
        ign(lo, hi, why)
        if hi - lo < 0x1000:
            small.append((lo, hi, why))

    for line in open(os.path.join(HERE, "compare_ignore.txt")):
        line = line.split("#")[0].strip()
        if not line:
            continue
        f = line.split(None, 2)
        if re.match(r"^[0-9A-Fa-f]{8}$", f[0]):
            ign_small(int(f[0], 16), int(f[1], 16), f[2] if len(f) > 2 else "")
        else:
            if f[0] not in byname:
                die("compare_ignore.txt: unknown symbol %s" % f[0])
            a, sz = byname[f[0]]
            if len(f) > 1 and re.match(r"^(0x)?[0-9A-Fa-f]+$", f[1]):
                sz = int(f[1], 16)
                why = f[2] if len(f) > 2 else ""
            else:
                why = " ".join(f[1:])
            if sz == 0:
                die("compare_ignore.txt: %s needs a size" % f[0])
            ign_small(a, a + sz, why)
    # libultra os/io objects replaced by the platform layer: their data and bss
    native_ul = set()
    mk = open(os.path.join(PORT, "Makefile")).read()
    m = re.search(r"^HL_UL :=((?:.*\\\n)*.*)$", mk, re.M)
    if m:
        native_ul = set(m.group(1).replace("\\\n", " ").split())
    native_ul |= {"ul_xprintf", "ul_xldtob", "ul_crc"}
    for mp in ("build_nm/hd_code.us.v11.map", "build_nm/hd_front_end.us.v11.map"):
        for line in open(os.path.join(ROOT, mp)):
            m = re.match(r"^ \.(data|bss|rodata|sdata|sbss)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+\S*/(ul_\w+)\.c\.o", line)
            if m and m.group(4) not in native_ul:
                lo = int(m.group(2), 16)
                if BASE <= lo < BASE + SIZE:
                    ign(lo, lo + int(m.group(3), 16), "libultra %s (platform layer)" % m.group(4))
    # the data tables pinned inside the hd_code text (NM_PIN_HD_CODE) are compared
    from elftools.elf.elffile import ELFFile
    with open(os.path.join(ROOT, "build_nm/hd_code.us.v11.elf"), "rb") as f:
        elf = ELFFile(f)
        pins = [(s["sh_addr"], s["sh_addr"] + s["sh_size"]) for s in elf.iter_sections()
                if s.name.startswith(".nm_pin_")]
        for sec in elf.iter_sections():
            if sec.name != ".symtab":
                continue
            for sym in sec.iter_symbols():
                a, sz = sym["st_value"], sym["st_size"]
                if sz and sym["st_info"]["type"] == "STT_OBJECT" and any(lo <= a < hi for lo, hi in pins):
                    i, j = (a - BASE) // 4, (a + sz - BASE) // 4
                    mask[i:j] = bytes(j - i)
    for lo, hi, why in small:
        a, b = (lo - BASE) // 4, (hi - BASE + 3) // 4
        mask[a:b] = b"\x01" * (b - a)
    return mask, reasons


def word_ok(n, e):
    """native 4 bytes vs emulator big-endian 4 bytes: some layout of u16/u8 agrees (u32 checked before)"""
    if n == e:
        return True
    h0 = n[0] == e[1] and n[1] == e[0]
    b0 = n[0] == e[0] and n[1] == e[1]
    h1 = n[2] == e[3] and n[3] == e[2]
    b1 = n[2] == e[2] and n[3] == e[3]
    return (h0 or b0) and (h1 or b1)


class Cmp:
    def __init__(self, emudir, natdir):
        self.emudir, self.natdir = emudir, natdir
        self.dsyms = data_syms()
        self.mask, self.reasons = load_ignore(self.dsyms)
        elfs = open(os.path.join(emudir, "elfs.txt")).read().split()
        self.efun = elf_syms(elfs, ("STT_FUNC", "STT_NOTYPE", "STT_OBJECT"))
        tails = [a for a, n, _ in self.efun.items if n == "__osThreadTail"]
        self.thread_tail = tails[0] if tails else -1
        # NM test ROM: code relinked at 0x80400000+ (hd_code, then the front end)
        self.nm_text_lo, self.nm_text_hi = (0x80400000, 0x80800000) if "build_nm" in elfs[0] else (0, 0)
        self.orig_fun = {}
        if self.nm_text_hi:
            for a, n, _ in elf_syms([os.path.join(ROOT, "build/hd_code.us.v11.elf"),
                                     os.path.join(ROOT, "build/hd_front_end.us.v11.elf")],
                                    ("STT_FUNC", "STT_NOTYPE", "STT_OBJECT")).items:
                if 0x801E7000 <= a < 0x80208040 or 0x802447C0 <= a < 0x802E8BD0:
                    self.orig_fun.setdefault(n, a)
        sp = os.path.join(PORT, "build/headless/bc_headless.syms")
        self.nfun = nm_syms(sp) if os.path.exists(sp) else Syms().done()
        # end of the exe's .text: after it, pointers are data (rdata, data, bss)
        self.text_end = 0
        if os.path.exists(sp):
            for line in open(sp):
                p = line.split()
                if len(p) == 3 and p[2] in ("etext", "_etext", "__etext"):
                    self.text_end = int(p[0], 16)
        if not self.text_end and self.nfun.items:
            self.text_end = self.nfun.items[-1][0] + 0x1000

    def codeptr(self, ev, nv):
        """A pointer into the exe image where the emulator has an N64 address:
        a function pointer (both name the same function), or a pointer to
        the native copy of constant data (string literals of C data tables:
        the exe keeps them in its own .rdata).  Also: an empty thread queue
        (libultra's __osThreadTail sentinel, the platform's NULL), and an NM
        ROM code address where the exe holds the original one (pointers into
        the front end's text that came with the data image)."""
        if nv == 0 and ev == self.thread_tail:
            return True
        if self.nm_text_lo <= ev < self.nm_text_hi and (0x801E7000 <= nv < 0x80208040 or 0x802447C0 <= nv < 0x802E8BD0):
            # jump tables and text pointers in the data images: NM addresses
            # don't map back exactly (rewritten functions change size); the
            # native code never jumps through them
            return True
        if not (0x80000000 <= ev < 0x80800000 and 0x00401000 <= nv < 0x01000000):
            return False
        a = self.efun.find(ev)
        b = self.nfun.find(nv)
        if a is not None and b is not None and a[0] == ev and b[0] == nv:
            return a[1] == b[1]
        return not (b is not None and b[0] <= nv < self.text_end)

    def rule_ignored(self, a, e):
        """Conditional ignores (state-dependent, so not in compare_ignore.txt)."""
        # (none at the moment; an example: the sound player's pending event at
        # D_80366BD0 + 0x28 is posted from a stack SndEvent with only .type set
        # for type 0x20, so the rest of it is stack garbage)
        return False

    def frame(self, n):
        e = open(os.path.join(self.emudir, "frame_%07d.bin" % n), "rb").read()[:SIZE]
        with open(os.path.join(self.natdir, "frame_%07d.bin" % n), "rb") as f:
            nat = f.read(SIZE)
        sw = array.array("I", nat)
        sw.byteswap()
        n32 = sw.tobytes()
        diffs = []
        mask = self.mask
        P = 4096
        for p in range(0, SIZE, P):
            if n32[p:p + P] == e[p:p + P]:
                continue
            for o in range(p, p + P, 4):
                if n32[o:o + 4] == e[o:o + 4] or mask[o >> 2]:
                    continue
                nb, eb = nat[o:o + 4], e[o:o + 4]
                if word_ok(nb, eb):
                    continue
                q = o & ~7  # u64/f64
                if nat[q:q + 8] == e[q:q + 8][::-1]:
                    continue
                ev = int.from_bytes(eb, "big")
                nv = int.from_bytes(nb, "little")
                if self.codeptr(ev, nv) or self.codeptr(ev, int.from_bytes(nb, "big")):
                    continue
                if self.rule_ignored(BASE + o, e):
                    continue
                diffs.append(o)
        return diffs, nat, e

    def runs(self, diffs, gap=16):
        out = []
        for o in diffs:
            if out and o - out[-1][1] <= gap:
                out[-1][1] = o + 4
                out[-1][2] += 1
            else:
                out.append([o, o + 4, 1])
        return out

    def describe(self, n, diffs, nat, e, detail):
        rs = self.runs(diffs)
        print("frame %d: %d words differ in %d runs" % (n, len(diffs), len(rs)))
        for lo, hi, cnt in rs[:detail]:
            a = BASE + lo
            it = self.dsyms.find(a)
            nm = self.dsyms.name(a)
            sz = ""
            if it and it[2]:
                sz = " (%s size 0x%X)" % (it[1], it[2])
            samples = []
            for o in diffs:
                if lo <= o < hi and len(samples) < 3:
                    samples.append("+%X emu %s nat %s" % (o - lo, e[o:o + 4].hex(), nat[o:o + 4].hex()))
            # (both columns are memory bytes; a native u32 shows byte-reversed)
            print("  %08X-%08X %-28s %4d words%s | %s" % (a, BASE + hi, nm, cnt, sz, "; ".join(samples)))
        if len(rs) > detail:
            print("  ... %d more runs" % (len(rs) - detail))


def frames_in(d):
    out = set()
    for f in os.listdir(d):
        m = re.match(r"frame_(\d{7})\.bin$", f)
        if m and int(m.group(1)) != 9999999:
            out.add(int(m.group(1)))
    return out


def cmd_diff(args):
    emudir, natdir = args[0], args[1]
    args = args[2:]
    lo = opt(args, "--from", 0, int)
    hi = opt(args, "--to", 1 << 30, int)
    detail = opt(args, "--detail", 30, int)
    show_all = flag(args, "--all")
    c = Cmp(emudir, natdir)
    common = sorted(x for x in frames_in(emudir) & frames_in(natdir) if lo <= x <= hi)
    if not common:
        die("no common dumps")
    first = None
    matched = 0
    for n in common:
        diffs, nat, e = c.frame(n)
        if not diffs:
            matched += 1
            if show_all:
                print("frame %d: match" % n)
            continue
        if first is None:
            first = n
            c.describe(n, diffs, nat, e, detail)
            if not show_all:
                break
        else:
            rs = c.runs(diffs)
            print("frame %d: %d words differ in %d runs, first %s" % (n, len(diffs), len(rs),
                                                                       c.dsyms.name(BASE + rs[0][0])))
    print("diff: %d of %d compared frames match%s" % (
        matched, len(common) if show_all or first is None else common.index(first) + 1,
        "; first difference at frame %d" % first if first is not None else ""))


def native_trace(natdir):
    out = {}
    p = os.path.join(natdir, "trace.txt")
    if not os.path.exists(p):
        return out
    for line in open(p):
        m = re.match(r"(\d+) vi=(\d+) t=\d+ mode=([0-9A-F]+) lvl=(-?\d+) mf=(\d+) gvi=(\d+)", line)
        if m:
            out[int(m.group(1))] = (int(m.group(2)), int(m.group(3), 16), int(m.group(4)), int(m.group(5)),
                                    int(m.group(6)))
    return out


def cmd_frames(args):
    emudir, natdir = args[0], args[1]
    ev = parse_emu(emudir)
    _, _, off = emu_timing(ev)
    nat = native_trace(natdir)
    same_vi = same_state = 0
    shown = 0
    limit = int(args[2]) if len(args) > 2 else 20
    for f in ev["F"]:
        n = f["n"]
        if n not in nat:
            continue
        evi = int(f["vi"]) + off
        es = (int(f["mode"], 16), int(f["lvl"], 16), int(f["mf"], 16), int(f["gvi"], 16))
        nv = nat[n]
        ns = (nv[1], nv[2], nv[3], nv[4])
        if evi == nv[0]:
            same_vi += 1
        if es == ns:
            same_state += 1
        if (evi != nv[0] or es != ns) and shown < limit:
            shown += 1
            print("frame %d: emu vi %d mode %X lvl %d mf %d gvi %d | native vi %d mode %X lvl %d mf %d gvi %d"
                  % ((n, evi) + es + (nv[0],) + ns))
    print("frames: %d in both; same submission retrace %d, same (mode, level, frames-in-mode, game VI) %d"
          % (len([f for f in ev["F"] if f["n"] in nat]), same_vi, same_state))


def cmd_hex(args):
    """hex EMUDIR NATDIR FRAME ADDR [LEN]: emulator bytes | native bytes | native as LE words"""
    emudir, natdir, n, a = args[0], args[1], int(args[2]), int(args[3], 16)
    ln = int(args[4], 16) if len(args) > 4 else 0x100
    e = open(os.path.join(emudir, "frame_%07d.bin" % n), "rb").read()
    nat = open(os.path.join(natdir, "frame_%07d.bin" % n), "rb").read()
    ds = data_syms()
    for o in range(a - BASE, a - BASE + ln, 16):
        eb, nb = e[o:o + 16], nat[o:o + 16]
        mark = "  " if eb == nb else "!="
        nw = " ".join("%08x" % int.from_bytes(nb[i:i + 4], "little") for i in range(0, 16, 4))
        print("%08X %s %s | %s | %s  %s" % (BASE + o, mark, eb.hex(), nb.hex(), nw, ds.name(BASE + o)))


def cmd_demos(args):
    ev = parse_emu(args[0])
    cur = None
    for f in ev["F"]:
        key = (f["lvl"], f["demo"], f["mode"])
        if key != cur:
            cur = key
            print("frame %6d vi %6s: mode %s level %s demo %s" % (f["n"], f["vi"], f["mode"], f["lvl"], f["demo"]))


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    cmd, args = sys.argv[1], sys.argv[2:]
    {"emu": cmd_emu, "inject": cmd_inject, "native": cmd_native, "diff": cmd_diff, "frames": cmd_frames,
     "demos": cmd_demos, "hex": cmd_hex}[cmd](args)


if __name__ == "__main__":
    main()
