#!/usr/bin/env python3
"""Pin a game file's data definitions to their N64 addresses (fixed-RAM model).

gcc compiles a game .c to assembly; this rewrites the assembly so that every
data object the N64 build places in N64 memory (an address below
0x80800000 in the NON_MATCHING ELF: .data/.rodata/.bss, the pinned text
tables) becomes an absolute symbol at that address:

    _D_802AD880:      ->   __native_D_802AD880:   (the initialiser, kept)
                           ...
                           __native_end_D_802AD880:
                           .globl _D_802AD880
                           .set _D_802AD880, 0x802AD880

Code in this file and in every other file then reads and writes the variable
in the RDRAM image.  The native initialiser stays in the exe, already in
host byte order and host layout; port/src/rdram.c copies it over the image at
start-up (see the generated copy table).  Zero-initialised objects (.bss,
.lcomm/.comm) need no copy: the image's bss is cleared.

usage: relabel.py ADDRS IN.s OUT.s RECORD
  ADDRS   'name addr' per line (C names), from gensyms.py addrs
  RECORD  written: 'name addr kind' per pinned object (kind = data|bss)
"""
import re
import sys

LABEL = re.compile(r"^(_[A-Za-z_][\w.]*):\s*$")
LCOMM = re.compile(r"^\s*\.(l?comm)\s+(_[A-Za-z_][\w.]*)\s*,\s*(\d+)")
SECTION = re.compile(r"^\s*\.(data|bss|text|section\s+([^,\s]+))")
STOP = re.compile(r"^(\S+:|\s*\.(globl|data|bss|text|section|align|p2align|balign|def|ident|lcomm|comm)\b)")


def main():
    addrs_path, inp, outp, rec_path = sys.argv[1:5]
    addrs = {}
    with open(addrs_path) as f:
        for line in f:
            p = line.split()
            if len(p) >= 2:
                addrs["_" + p[0]] = int(p[1], 16)
    lines = open(inp).read().split("\n")
    # IDO emits no symbol for a function-local static; the project names them
    # D_<address> by convention (22EE0's `static s32 D_802F3C04`), so trust the name
    for line in lines:
        m = re.match(r"^(_D_([0-9A-F]{8}))\.\d+:", line)
        if m and m.group(1) not in addrs and 0x80000000 <= int(m.group(2), 16) < 0x80800000:
            addrs[m.group(1)] = int(m.group(2), 16)
    out, rec = [], []
    sec = ".text"
    i = 0
    while i < len(lines):
        line = lines[i]
        m = SECTION.match(line)
        if m:
            sec = m.group(2) or "." + m.group(1)
        m = LCOMM.match(line)
        if m and m.group(2).split(".")[0] in addrs:
            name = m.group(2)
            key = name.split(".")[0]
            out.append("\t.set %s, 0x%08X" % (name, addrs[key]))
            if name == key:
                out.append("\t.globl %s" % name)
            rec.append("%s %08X bss" % (name[1:].replace(".", "_"), addrs[key]))
            i += 1
            continue
        m = LABEL.match(line)
        # a function-local static comes out as _D_xxxxxxxx.N: same N64 symbol
        key = m.group(1).split(".")[0] if m else None
        if m and key in addrs and not sec.startswith(".text"):
            name = m.group(1)
            cname = name.replace(".", "_")    # C-visible name for the copy table
            out.append("__native%s:" % cname)
            i += 1
            while i < len(lines) and not STOP.match(lines[i]):
                out.append(lines[i])
                i += 1
            out.append("__native_end%s:" % cname)
            out.append("\t.globl __native%s\n\t.globl __native_end%s" % (cname, cname))
            if name != key:
                out.append("\t.set %s, 0x%08X" % (name, addrs[key]))
            else:
                out.append("\t.globl %s\n\t.set %s, 0x%08X" % (name, name, addrs[key]))
            kind = "bss" if sec.startswith(".bss") else "data"
            rec.append("%s %08X %s" % (cname[1:], addrs[key], kind))
            continue
        out.append(line)
        i += 1
    # a .globl for a name we turned into an absolute is harmless (kept)
    with open(outp, "w") as f:
        f.write("\n".join(out))
    with open(rec_path, "w") as f:
        f.write("".join(r + "\n" for r in rec))


if __name__ == "__main__":
    main()
