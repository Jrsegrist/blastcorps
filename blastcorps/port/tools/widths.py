#!/usr/bin/env python3
"""Reduce access-width traces (port/tools/m64widths.py output) to what the
byte-order layer consumes.

  widths.py image OUT TRACE...    the reads of the hd/front-end data images:
                                  'region offset width' (port/data/image_widths.txt)
  widths.py assets TRACE...       per loaded asset (by ROM start): load sites,
                                  sizes, and a summary of the read/write widths
  widths.py facts OUT TRACE...    per asset, every offset read and its width(s):
                                  the oracle of `make -C port loadcheck` (T9)

The traces hold no ROM bytes, only which offsets the original code reads at
which width (and how often).
"""
import collections
import sys


# copy loops: they move bytes at whatever width suits them, which says
# nothing about the data's own layout (the copy is byte-order neutral)
COPIERS = {"func_8026A5CC",   # 23C20: copy in 8-byte units
           "func_802A57DC",   # 60F60: packed texture -> scratch copy (u64, u16)
           "func_802A5E10",   # 60F60: type-0 (stored) texture copy
           "func_80285A78",   # 409D0: 16-byte record copy
           "memcpy", "bcopy", "bzero", "alCopy"}


def parse(paths, keep_copiers=False):
    acc = collections.defaultdict(collections.Counter)    # (region, off) -> Counter((width, kind))
    loads = []
    for p in paths:
        for line in open(p):
            f = line.split()
            if not f:
                continue
            if f[0] == "A":
                func = f[6] if len(f) > 6 else "?"
                if (func in COPIERS or func.startswith("~")) and not keep_copiers:
                    continue
                off = int(f[2], 16)
                if f[3] == "8" and off % 8 == 4:
                    continue      # the debugger reports a doubleword access twice (each half)
                acc[(f[1], off)][(f[3], f[4])] += int(f[5])
            elif f[0] == "L":
                loads.append((int(f[1]), int(f[2], 16), int(f[3], 16), int(f[4], 16), f[5], int(f[6], 16)))
    return acc, loads


def cmd_image(out, paths):
    acc, _ = parse(paths)
    rows = []
    for (region, off), c in acc.items():
        if region not in ("hd", "fe"):
            continue
        for (w, kind), n in c.items():
            if kind == "R":
                rows.append((region, off, w))
    rows.sort(key=lambda r: (r[0], r[1], r[2]))
    with open(out, "w") as f:
        f.write("# region offset width: reads of the hd_code / front-end data images by the original code\n")
        f.write("# (port/tools/m64widths.py over boot, front end and the attract demos; reduced by widths.py)\n")
        for r in rows:
            f.write("%s %X %s\n" % r)
    print("%d image read facts" % len(rows))


def cmd_facts(out, paths):
    """The oracle for loadcheck's T9: per traced asset (ROM start, decompressed
    size, how it arrived), every offset the code read and at which width
    (one width per offset; offsets read at several widths are listed as such)."""
    acc, loads = parse(paths)
    size = {}
    how = {}
    for vi, rom, dest, sz, kind, ra in loads:
        if kind in ("gz", "pk"):
            size[rom] = max(size.get(rom, 0), sz)
            how[rom] = kind
        elif kind == "dma" and rom not in how:
            size[rom] = max(size.get(rom, 0), sz)
            how[rom] = "dma"
    rows = collections.defaultdict(lambda: collections.defaultdict(set))
    for (region, off), c in acc.items():
        if region[0] != "r":
            continue
        rom = int(region[1:], 16)
        for (w, kind), n in c.items():
            if kind == "R" and w in ("1", "2", "4", "8"):
                rows[rom][off].add(int(w))
    with open(out, "w") as f:
        f.write("# ROM SIZE HOW then OFF W[,W...] lines (widths.py facts; local, not committed)\n")
        for rom in sorted(rows):
            if rom not in size:
                continue
            f.write("A %06X %X %s\n" % (rom, size[rom], how[rom]))
            for off in sorted(rows[rom]):
                if off < size[rom]:
                    f.write("%X %s\n" % (off, ",".join(str(w) for w in sorted(rows[rom][off]))))
    print("%d assets" % len(rows))


def cmd_assets(paths):
    acc, loads = parse(paths)
    by = collections.defaultdict(list)
    for vi, rom, dest, size, kind, ra in loads:
        by[rom].append((vi, dest, size, kind, ra))
    per = collections.defaultdict(lambda: collections.Counter())
    for (region, off), c in acc.items():
        if region[0] != "r":
            continue
        for (w, kind), n in c.items():
            per[int(region[1:], 16)][(w, kind)] += 1
    for rom in sorted(by):
        ls = by[rom]
        sites = collections.Counter("%08X/%s" % (l[4], l[3]) for l in ls)
        sizes = sorted(set(l[2] for l in ls))
        wc = per.get(rom, {})
        print("%06X loads=%d sizes=%s sites=%s widths=%s" % (
            rom, len(ls), ",".join("%X" % s for s in sizes[:4]), ",".join(sites),
            " ".join("%s%s:%d" % (w, k, n) for (w, k), n in sorted(wc.items()))))


if __name__ == "__main__":
    if sys.argv[1] == "image":
        cmd_image(sys.argv[2], sys.argv[3:])
    elif sys.argv[1] == "assets":
        cmd_assets(sys.argv[2:])
    elif sys.argv[1] == "facts":
        cmd_facts(sys.argv[2], sys.argv[3:])
    else:
        sys.exit(__doc__)
