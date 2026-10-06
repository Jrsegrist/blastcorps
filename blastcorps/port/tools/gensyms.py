#!/usr/bin/env python3
"""Symbol plumbing for the fixed-RAM Windows build (port/).

  gensyms.py addrs OUT                 'name addr' for every data symbol at an N64
                                       address (from the NON_MATCHING ELFs)
  gensyms.py link OUTDIR OBJ...        after compiling: OUTDIR/abs_syms.ld (absolute
                                       definitions for the data the objects use but
                                       nothing defines), OUTDIR/stubs.c (a trap for
                                       every function nobody defines), and a report
  gensyms.py copytab OUT.c RECORD...   the start-up table that copies native
                                       initialisers of pinned objects into RDRAM

The NON_MATCHING ELFs (build_nm/) are the source of truth for addresses: data
and .bss are pinned at their original N64 addresses there, and the pinned
.text tables (NM_PIN_HD_CODE) too.  Text moved (0x80800000+), and anything at
0x80800000 or above only exists in the NM build, so it stays native.
"""
import os
import re
import subprocess
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
VERSION = "us.v11"
ELFS = ["build_nm/hd_code.%s.elf" % VERSION, "build_nm/hd_front_end.%s.elf" % VERSION]
NM = os.environ.get("NM", "i686-w64-mingw32-nm")
PIN_LIMIT = 0x80800000
# emitted by gcc itself (64-bit division helpers, block clears it synthesises,
# sqrt for the IDO sqrt.s intrinsic): from libgcc / the C runtime
RUNTIME = {"__divdi3", "__udivdi3", "__moddi3", "__umoddi3", "__muldi3", "__ashldi3", "__ashrdi3",
           "__lshrdi3", "__fixdfdi", "__fixsfdi", "__fixunsdfdi", "__fixunssfdi", "__floatdidf",
           "__floatdisf", "__floatundidf", "__floatundisf", "memset", "memcpy", "memmove",
           "sqrt", "sqrtf", "fabs", "fabsf", "___chkstk_ms", "__alloca"}


def undefined_funcs():
    out = set()
    for fn in os.listdir(ROOT):
        if fn.startswith("undefined_funcs") and fn.endswith(".txt"):
            for line in open(os.path.join(ROOT, fn)):
                m = re.match(r"\s*(\w+)\s*=", line)
                if m:
                    out.add(m.group(1))
    return out


def elf_symbols():
    """name -> (addr, is_func, size).  hd_code wins over the front end (it links first)."""
    ufuncs = undefined_funcs()
    syms = {}
    for rel in ELFS:
        with open(os.path.join(ROOT, rel), "rb") as f:
            elf = ELFFile(f)
            secs = list(elf.iter_sections())
            for st in secs:
                if not isinstance(st, SymbolTableSection):
                    continue
                for sy in st.iter_symbols():
                    n = sy.name
                    typ = sy["st_info"]["type"]
                    if not n or typ in ("STT_SECTION", "STT_FILE") or n.startswith(("$", ".L", "L8")):
                        continue
                    shndx = sy["st_shndx"]
                    if shndx == "SHN_UNDEF":
                        continue
                    v = sy["st_value"]
                    if shndx == "SHN_ABS":
                        is_func = n.startswith("func_") or n in ufuncs
                    else:
                        is_func = typ == "STT_FUNC" or bool(secs[shndx]["sh_flags"] & 4)
                    if n in syms and syms[n][1] == is_func:
                        continue
                    syms.setdefault(n, (v, is_func, sy["st_size"]))
    return syms


def cmd_addrs(out):
    syms = elf_symbols()
    with open(out, "w") as f:
        for n, (v, is_func, size) in sorted(syms.items(), key=lambda kv: kv[1][0]):
            if not is_func and 0x80000000 <= v < PIN_LIMIT:
                f.write("%s %08X %d\n" % (n, v, size))


def nm_objects(objs):
    defined, undefined = set(), {}
    for o in objs:
        r = subprocess.run([NM, o], capture_output=True, text=True, check=True)
        for line in r.stdout.splitlines():
            p = line.split()
            if len(p) == 3 and p[1] in "TDBRAC" and p[1].isupper():
                defined.add(p[2])
            elif len(p) == 2 and p[0] == "U" and "/game/" in o:   # host-side objects use the C runtime
                undefined.setdefault(p[1], []).append(os.path.basename(o))
    return defined, undefined


def cmd_link(outdir, objs):
    syms = elf_symbols()
    defined, undefined = nm_objects(objs)
    abs_lines, stubs, unknown, data_used = [], [], [], 0
    for sym, users in sorted(undefined.items()):
        if sym in defined:
            continue
        if not sym.startswith("_"):
            continue
        c = sym[1:]
        if c in RUNTIME:     # compiler helpers / libc the host toolchain supplies
            continue
        if c in syms:
            v, is_func, _ = syms[c]
            if is_func:
                stubs.append(c)
            else:
                abs_lines.append("%s = 0x%08X;" % (sym, v))
                data_used += 1
        elif c.startswith("func_") or c.startswith("os") or c.startswith("__os") or c.startswith("gu") \
                or c.startswith("al") or c in ("bzero", "bcopy", "bcmp", "sprintf", "sqrtf", "sinf", "cosf"):
            stubs.append(c)
        else:
            unknown.append((c, users))
    with open(os.path.join(outdir, "abs_syms.ld"), "w") as f:
        f.write("/* generated by port/tools/gensyms.py: data symbols at their N64 addresses */\n")
        f.write("\n".join(abs_lines) + "\n")
    with open(os.path.join(outdir, "stubs.c"), "w") as f:
        f.write("/* generated by port/tools/gensyms.py: functions no linked object defines */\n")
        f.write("void port_stub_hit(const char *name);\n")
        for c in stubs:
            f.write("void %s(void) { port_stub_hit(\"%s\"); }\n" % (c, c))
    with open(os.path.join(outdir, "link_report.txt"), "w") as f:
        f.write("absolute data symbols: %d\nstubbed functions: %d\nunknown: %d\n" %
                (data_used, len(stubs), len(unknown)))
        for c, users in unknown:
            f.write("UNKNOWN %s (used by %s)\n" % (c, ", ".join(sorted(set(users)))))
    print("gensyms: %d absolute data symbols, %d stubbed functions, %d unknown" %
          (data_used, len(stubs), len(unknown)))
    for c, users in unknown:
        print("  unknown symbol %s (used by %s)" % (c, ", ".join(sorted(set(users)))))
    return 1 if unknown else 0


def cmd_copytab(out, addrs_path, records):
    sizes = {}
    for line in open(addrs_path):
        p = line.split()
        if len(p) >= 3:
            sizes[int(p[1], 16)] = max(sizes.get(int(p[1], 16), 0), int(p[2]))
    rows = []
    for r in records:
        for line in open(r):
            p = line.split()
            if len(p) == 3 and p[2] == "data":
                rows.append((p[0], int(p[1], 16)))
    with open(out, "w") as f:
        f.write("/* generated by port/tools/gensyms.py: native initialisers of pinned objects */\n")
        f.write("#include \"rdram.h\"\n")
        for n, _ in rows:
            f.write("extern const char _native_%s[], _native_end_%s[];\n" % (n, n))
        f.write("const PortCopy port_copytab[] = {\n")
        for n, a in rows:
            f.write("    { 0x%08Xu, _native_%s, _native_end_%s, \"%s\", %du },\n" % (a, n, n, n, sizes.get(a, 0)))
        f.write("    { 0, 0, 0, 0, 0 }\n};\n")


def main():
    cmd = sys.argv[1]
    if cmd == "addrs":
        cmd_addrs(sys.argv[2])
    elif cmd == "link":
        sys.exit(cmd_link(sys.argv[2], sys.argv[3:]))
    elif cmd == "copytab":
        cmd_copytab(sys.argv[2], sys.argv[3], sys.argv[4:])
    else:
        sys.exit("usage: see docstring")


if __name__ == "__main__":
    main()
