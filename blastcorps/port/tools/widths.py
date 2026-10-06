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
           "func_802C04F0",   # 77E20: 0x38-byte debris record copy (words)
           "func_802AC85C",   # 679E0: vehicle state restore (byte copy)
           "func_802AC7DC",   # 679E0: vehicle state save (byte copy)
           "func_802A75DC",   # vehicle block byte loop
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


def parse_by_func(paths):
    """like parse, keyed (region, off, func)"""
    acc = collections.defaultdict(collections.Counter)
    for p in paths:
        for line in open(p):
            f = line.split()
            if not f or f[0] != "A":
                continue
            func = f[6] if len(f) > 6 else "?"
            if func in COPIERS or func.startswith("~"):
                continue
            off = int(f[2], 16)
            if f[3] == "8" and off % 8 == 4:
                continue
            acc[(f[1], off, func)][(f[3], f[4])] += int(f[5])
    return acc, None


def is_fe_func(name):
    """a front-end function (text 0x801E7000-0x80207090)"""
    try:
        a = int(name[5:], 16) if name.startswith("func_") else 0
    except ValueError:
        return False
    return 0x801E7000 <= a < 0x80207090


def heap_users(paths):
    """Functions that touch loaded assets.  Once a level loads, the heap
    overwrites the front end (0x801E7000-0x8021ED00), and the tracer, which
    keys the front-end data image by address, then charges their heap
    accesses to front-end offsets (e.g. 60F60's texture code reading u16s
    over the front end's "BLAST CORPS" pak name).  cmd_image drops the
    front-end reads wider than a byte of hd_code functions that also read
    assets (found by port/tools/compare.py: the pak name D_8020C000 was
    half-swapped)."""
    users = set()
    for p in paths:
        for line in open(p):
            f = line.split()
            if len(f) > 6 and f[0] == "A" and f[1].startswith("r"):
                users.add(f[6])
    return users


def cmd_image(out, paths):
    acc, _ = parse_by_func(paths)
    users = heap_users(paths)
    rows = set()
    dropped = 0
    for (region, off, func), c in acc.items():
        if region not in ("hd", "fe"):
            continue
        for (w, kind), n in c.items():
            if kind != "R":
                continue
            if region == "fe" and func in users and w != "1" and not is_fe_func(func):
                dropped += 1
                continue
            rows.add((region, off, w))
    rows = list(rows)
    print("front end: %d reads by heap users dropped" % dropped)
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
            f.write("@ %06X %X %s\n" % (rom, size[rom], how[rom]))
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
