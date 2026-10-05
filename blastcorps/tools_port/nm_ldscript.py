#!/usr/bin/env python3
"""Derive NON_MATCHING linker scripts from the splat-generated ones.

Usage: nm_ldscript.py IN.ld OUT.ld --build-dir build_nm
                      [--relocate SECTION=VRAM] [--pin-data SECTION]
                      [--extra-vram ADDR --extra-glob GLOB]
       nm_ldscript.py undefined_syms.X.txt OUT.txt --provide

What it does (see tools_port/README.md for the why):
  * rewrites every `build/...` object path to `<build-dir>/...`;
  * --relocate .hd_code=0x80800000 moves a code section to a new VRAM, so a
    NON_MATCHING rewrite that is bigger than the original asm can never run
    into the data segment that follows it (which must stay at its original
    address: most data symbols are absolute `D_xxxxxxxx = 0x...` definitions);
  * --pin-data .hd_code_data pins every `<seg>_<ROMOFF>_bin = .;` label in
    that section back to its original address (`. = ROMOFF - SEG_ROM_START`),
    so a C file whose .rodata changed size cannot shift the binary blobs
    after it.  A file whose .rodata *grew* past the next blob makes ld fail
    with "cannot move location counter backwards", which is what we want;
  * --extra-vram/--extra-glob add catch-all output sections before
    /DISCARD/ for .rodata/.data/.bss that a rewritten file now has but the
    matching layout never placed (e.g. float constants or a jump table in
    one of the hand-written-asm files);
  * --provide turns `name = 0x...;` symbol files into `PROVIDE(name = ...);`.
    In the matching link those absolute assignments override object
    definitions (harmless, the addresses agree). In the relocated link they
    would send every call of a function listed there to its *original*
    address, silently bypassing the rewrite.
"""
import argparse
import re
import sys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("inp")
    ap.add_argument("out")
    ap.add_argument("--build-dir", default="build_nm")
    ap.add_argument("--relocate", action="append", default=[])
    ap.add_argument("--pin-data", action="append", default=[])
    ap.add_argument("--extra-vram")
    ap.add_argument("--extra-glob", action="append", default=[])
    ap.add_argument("--provide", action="store_true",
                    help="symbol-assignment file: wrap every assignment in PROVIDE()")
    args = ap.parse_args()

    text = open(args.inp).read()
    text = re.sub(r"(?<![\w/])build/", args.build_dir + "/", text)

    if args.provide:
        # `name = value;` -> `PROVIDE(name = value);` so an object's own
        # definition (e.g. a function that moved) wins over the absolute one
        text = re.sub(r"^(\s*)([A-Za-z_.$][\w.$]*)\s*=\s*([^;]+);",
                      r"\1PROVIDE(\2 = \3);", text, flags=re.M)
        open(args.out, "w").write(text)
        return

    for r in args.relocate:
        sec, vram = r.split("=")
        pat = re.compile(r"^(\s*)" + re.escape(sec) + r" 0x[0-9A-Fa-f]+ :", re.M)
        text, n = pat.subn(lambda m: "%s%s %s :" % (m.group(1), sec, vram), text)
        if n != 1:
            sys.exit("nm_ldscript: section %s not found exactly once" % sec)

    lines = text.split("\n")
    out = []
    cur_sec = None
    cur_rom = None
    last_rompos = None
    for line in lines:
        m = re.match(r"\s*__romPos = (0x[0-9A-Fa-f]+);", line)
        if m:
            last_rompos = int(m.group(1), 16)
        m = re.match(r"\s*(\.\w+) 0x[0-9A-Fa-f]+ : AT", line)
        if m:
            cur_sec = m.group(1)
            cur_rom = last_rompos
        m = re.match(r"(\s*)\w+_([0-9A-F]{5,})_bin = \.;", line)
        if m and cur_sec in args.pin_data and cur_rom is not None:
            off = int(m.group(2), 16) - cur_rom
            out.append("%s. = 0x%X;" % (m.group(1), off))
        if args.extra_vram and re.match(r"\s*/DISCARD/", line):
            globs = args.extra_glob
            sel = lambda secs: " ".join("%s(%s)" % (g, secs) for g in globs)
            out.append("    /* NON_MATCHING: sections the matching layout never placed */")
            out.append("    .nm_extra %s : SUBALIGN(16)" % args.extra_vram)
            out.append("    {")
            out.append("        nm_extra_START = .;")
            out.append("        %s;" % sel(".text"))
            out.append("        %s;" % sel(".rodata .rodata.* .late_rodata"))
            out.append("        %s;" % sel(".data .data.* .sdata"))
            out.append("        nm_extra_END = .;")
            out.append("    }")
            out.append("    .nm_extra_bss (NOLOAD) : SUBALIGN(16)")
            out.append("    {")
            out.append("        %s;" % sel(".bss .sbss COMMON .scommon"))
            out.append("    }")
            out.append("")
        out.append(line)

    open(args.out, "w").write("\n".join(out))


if __name__ == "__main__":
    main()
