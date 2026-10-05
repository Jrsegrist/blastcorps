#!/usr/bin/env python3
"""nm_symaudit: check that no data symbol moved between build/ and build_nm/.

The NON_MATCHING link relocates only the hd_code text.  Every data, .bss and
absolute symbol must keep its original address, otherwise the hand asm (which
uses raw addresses) and a C rewrite (which uses the symbol) would see two
different homes for one variable.  This compares the symbol tables of both
builds' ELFs and lists every non-text symbol whose address differs.

    python3 tools_port/nm_symaudit.py [--ref build] [--new build_nm]

Exit status 1 if anything moved.  runchecks.py runs it after -b/-B builds.
"""
import argparse
import os
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection


def load(path):
    syms, dup = {}, set()
    with open(path, "rb") as f:
        e = ELFFile(f)
        secs = list(e.iter_sections())
        text = [(s["sh_addr"], s["sh_addr"] + s["sh_size"]) for s in secs
                if s["sh_flags"] & 4 and s["sh_flags"] & 2]
        for st in secs:
            if not isinstance(st, SymbolTableSection):
                continue
            for sy in st.iter_symbols():
                n = sy.name
                if not n or sy["st_info"]["type"] in ("STT_SECTION", "STT_FILE") \
                        or sy["st_shndx"] == "SHN_UNDEF":
                    continue
                if n in syms and syms[n][0] != sy["st_value"]:
                    dup.add(n)          # same-named locals in several files: ambiguous
                sec = "ABS" if sy["st_shndx"] == "SHN_ABS" else secs[sy["st_shndx"]].name
                syms[n] = (sy["st_value"], sec)
    return syms, text, dup


def audit(ref_dir, new_dir, version="us.v11"):
    """-> list of (elf, name, ref_addr, ref_sec, new_addr, new_sec)"""
    moved = []
    for elf in ("hd_code", "init"):
        r, rtext, rdup = load(os.path.join(ref_dir, "%s.%s.elf" % (elf, version)))
        n, _, ndup = load(os.path.join(new_dir, "%s.%s.elf" % (elf, version)))
        for name, (v, sec) in sorted(r.items()):
            if name in rdup or name in ndup or name not in n:
                continue
            # code (moves on purpose); _binary_*_end labels at the end of the text
            if any(lo <= v < hi for lo, hi in rtext) or \
                    (name.startswith("_binary_") and any(v == hi for _, hi in rtext)):
                continue
            v2, sec2 = n[name]
            if v2 != v:
                moved.append((elf, name, v, sec, v2, sec2))
    return moved


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ref", default="build")
    ap.add_argument("--new", default="build_nm")
    ap.add_argument("--version", default="us.v11")
    o = ap.parse_args()
    moved = audit(o.ref, o.new, o.version)
    if not moved:
        print("nm_symaudit: OK, no data symbol moved between %s and %s" % (o.ref, o.new))
        return 0
    print("nm_symaudit: %d data symbol(s) moved between %s and %s:" % (len(moved), o.ref, o.new))
    for elf, name, v, sec, v2, sec2 in moved:
        print("  %-8s %-28s 0x%08X (%s) -> 0x%08X (%s)" % (elf, name, v, sec, v2, sec2))
    return 1


if __name__ == "__main__":
    sys.exit(main())
