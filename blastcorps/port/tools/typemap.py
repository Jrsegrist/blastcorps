#!/usr/bin/env python3
"""Byte-order schema for the RDRAM image, from the game's own C declarations.

Compiles every hd_code game file with the host gcc (-m32 -malign-double: the
i686 MinGW struct layout) and -g, then walks the DWARF of every variable the
C declares or defines (extern declarations included, struct types expanded)
to get, per N64 data symbol, its scalar leaves: (offset, width) of every 2-,
4- and 8-byte field.  Byte-swapping the big-endian image by these leaves is
the "swap on load by type" option.

Writes OUT (one line per symbol: 'addr name size leaves'), and prints how
much of the hd_code .data/.rodata image the declarations type, and where
files disagree about a symbol's layout (the accesses a whole-image swap would
get wrong for one of the readers).

usage: typemap.py ADDRS OUT    (from the project root; ADDRS = port/build/addrs.txt)
"""
import collections
import glob
import os
import subprocess
import sys
import tempfile

from elftools.elf.elffile import ELFFile

ADDRS, OUT = sys.argv[1], sys.argv[2]
SHIM = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "include", "port_ultratypes.h")
FLAGS = ["-m32", "-malign-double", "-g", "-O0", "-c", "-std=gnu89", "-nostdinc", "-I", ".", "-I", "include",
         "-I", "include/2.0I", "-I", "include/2.0I/PR", "-include", SHIM, "-D_LANGUAGE_C", "-D_FINALROM",
         "-DNON_MATCHING", "-D_MIPS_SZLONG=32", "-D_MIPS_SIM=1", "-w", "-fno-eliminate-unused-debug-symbols"]
IMAGE = (0x802E8BD0, 0x8030F660)
# hd .text tables (pinned), hd .data/.rodata, front-end .data/.rodata
INIT_RANGES = [(0x802447C0, 0x802E8BD0), IMAGE, (0x80208040, 0x80210E90)]

addr = {}
nmsize = {}
for line in open(ADDRS):
    p = line.split()
    addr[p[0]] = int(p[1], 16)
    nmsize[p[0]] = int(p[2]) if len(p) > 2 else 0
order = sorted(set(addr.values()))


def next_sym(a):
    import bisect
    i = bisect.bisect_right(order, a)
    return order[i] if i < len(order) else a


def attr(die, name):
    a = die.attributes.get(name)
    return a.value if a is not None else None


def tref(die):
    if "DW_AT_type" not in die.attributes:
        return None
    return die.get_DIE_from_attribute("DW_AT_type")


def leaves(t, base, out, depth=0):
    """Append (offset, width) scalar leaves of type DIE t at offset base.
    Returns the type's size (None if unknown, e.g. T[])."""
    while t is not None and t.tag in ("DW_TAG_typedef", "DW_TAG_const_type", "DW_TAG_volatile_type"):
        t = tref(t)
    if t is None:
        return None
    if t.tag in ("DW_TAG_base_type", "DW_TAG_pointer_type", "DW_TAG_enumeration_type"):
        n = attr(t, "DW_AT_byte_size") or 4
        out.append((base, n, t.tag == "DW_TAG_base_type" and attr(t, "DW_AT_encoding") == 4))
        return n
    if t.tag in ("DW_TAG_structure_type", "DW_TAG_union_type"):
        for m in t.iter_children():
            if m.tag == "DW_TAG_member":
                off = attr(m, "DW_AT_data_member_location") or 0
                leaves(tref(m), base + (off if isinstance(off, int) else 0), out, depth + 1)
        return attr(t, "DW_AT_byte_size")
    if t.tag == "DW_TAG_array_type":
        et = tref(t)
        dims = []
        for sub in t.iter_children():
            if sub.tag == "DW_TAG_subrange_type":
                ub = attr(sub, "DW_AT_upper_bound")
                cnt = attr(sub, "DW_AT_count")
                dims.append(cnt if cnt is not None else (ub + 1 if isinstance(ub, int) else None))
        one = []
        esz = leaves(et, 0, one, depth + 1)
        one = flatten(one, esz or 0)
        inner = 1
        for d in dims[1:]:
            inner *= d or 0
        stride = (esz or 0) * inner
        n0 = dims[0] if dims else None
        # expand the element leaves for the inner dims once; outer count may be unknown
        elem = []
        for k in range(inner):
            elem += [(o + k * (esz or 0), w, f) for o, w, f in one]
        out.append(("ARRAY", base, stride, n0, elem))
        return stride * n0 if (n0 is not None and stride) else None
    return None


def flatten(lv, size):
    flat = []
    for x in lv:
        if x[0] == "ARRAY":
            _, base, stride, n, elem = x
            if not stride:
                if n == 1 or n is None:   # zero-size element / flexible: keep the leaves once
                    flat += [(base + o, w, f) for o, w, f in elem]
                continue
            if n is None:
                n = max(0, (size - base) // stride) if size else 0
            for k in range(n):
                flat += [(base + k * stride + o, w, f) for o, w, f in elem]
        else:
            flat.append(x)
    return flat


decls = collections.defaultdict(dict)   # name -> {file: (size, leaves)}
tmp = tempfile.mkdtemp()
files = [f for f in sorted(glob.glob("src.us.v11/hd_code/*.c")) if not os.path.basename(f).startswith("ul_")]
for src in files:
    obj = os.path.join(tmp, os.path.basename(src) + ".o")
    r = subprocess.run(["gcc"] + FLAGS + ["-o", obj, src], capture_output=True, text=True)
    if r.returncode:
        print("compile failed: %s\n%s" % (src, r.stderr[:500]))
        continue
    with open(obj, "rb") as f:
        dw = ELFFile(f).get_dwarf_info()
        for cu in dw.iter_CUs():
            for die in cu.get_top_DIE().iter_children():
                if die.tag != "DW_TAG_variable":
                    continue
                nm = attr(die, "DW_AT_name")
                nm = nm.decode() if isinstance(nm, bytes) else nm
                # only initialised memory needs a byte-order schema (.bss starts
                # zeroed and is only ever written by native code)
                if nm not in addr or not any(r0 <= addr[nm] < r1 for r0, r1 in INIT_RANGES):
                    continue
                a = addr[nm]
                lv = []
                sz = leaves(tref(die), 0, lv)
                if sz is None:
                    sz = nmsize.get(nm) or (next_sym(a) - a)
                decls[nm][os.path.basename(src)] = (sz, [x for x in flatten(lv, sz) if x[1] > 1])

# per symbol: pick the layout most files agree on; record disagreements
conflicts = []
chosen = {}
for nm, per in decls.items():
    lay = collections.Counter((sz, tuple(lv)) for sz, lv in per.values())
    (sz, lv), _ = lay.most_common(1)[0]
    chosen[nm] = (sz, list(lv))
    if len(lay) > 1:
        conflicts.append((addr[nm], nm, {f: (s, len(l)) for f, (s, l) in per.items()}))

with open(OUT, "w") as f:
    for nm, (sz, lv) in sorted(chosen.items(), key=lambda kv: addr[kv[0]]):
        f.write("%08X %s %d %s\n" % (addr[nm], nm, sz, " ".join("%X:%d%s" % (o, w, "f" if fl else "")
                                                               for o, w, fl in lv)))

# coverage of the hd_code data image
lo, hi = IMAGE
state = bytearray(hi - lo)   # 0 untyped, 1 declared byte-only/opaque, 2 multi-byte leaf
for nm, (sz, lv) in chosen.items():
    a = addr[nm]
    if not (lo <= a < hi):
        continue
    for k in range(a, min(a + sz, hi)):
        if state[k - lo] == 0:
            state[k - lo] = 1
    for o, w, _ in lv:
        for k in range(a + o, min(a + o + w, hi)):
            state[k - lo] = 2
tot = hi - lo
c = collections.Counter(state)
nsyms_img = sum(1 for nm in chosen if lo <= addr[nm] < hi)
nsyms_all = sum(1 for nm, a in addr.items() if lo <= a < hi)
print("hd_code data image 0x%X bytes; %d of its %d symbols are declared in C" % (tot, nsyms_img, nsyms_all))
print("  bytes inside multi-byte typed leaves: 0x%X (%.1f%%)" % (c[2], 100.0 * c[2] / tot))
print("  bytes declared but byte-typed/opaque (u8[], char): 0x%X (%.1f%%)" % (c[1], 100.0 * c[1] / tot))
print("  bytes no declaration covers: 0x%X (%.1f%%)" % (c[0], 100.0 * c[0] / tot))
print("symbols whose layout differs between files: %d" % len(conflicts))
for a, nm, per in sorted(conflicts)[:40]:
    print("  %08X %s: %s" % (a, nm, ", ".join("%s size %s/%d leaves" % (f, s, n) for f, (s, n) in sorted(per.items()))))
