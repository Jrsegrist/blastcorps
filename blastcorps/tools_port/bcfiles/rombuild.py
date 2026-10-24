"""Putting edited files back into a ROM.

In place: a file whose new compressed bytes fit its slot (up to the next
file's start: levels and models are padded to 16 bytes, buildings are
packed back to back) is written over the old bytes, the rest of the slot
zeroed.  Every loader takes the length as "next file's start minus this
one's" and inflates exactly two members, so trailing zeros are never read
as data.  The ROM keeps every offset, so the code, the tables and the port
(which loads the original offsets) need no change.

A file that doesn't fit needs the shiftable build (docs/shiftable.md):
`shift_config()` writes the compressed file and a pads JSON entry
("rom": {"replace": ...}) for `make SHIFT=1 SHIFT_PADS=...`, which moves
everything after it and rewrites the tables and the code's ROM offsets.
"""
import json
import os
import struct

from . import gz

CIC6102_SEED = 0xF8CA4DDC


def _rol(v, n):
    n &= 31
    return ((v << n) | (v >> (32 - n))) & 0xFFFFFFFF if n else v


def cic6102_crc(rom):
    """The two header checksums for a CIC-6102 cartridge (over ROM 0x1000-0x101000)."""
    t1 = t2 = t3 = t4 = t5 = t6 = CIC6102_SEED
    words = struct.unpack_from(">%dI" % (0x100000 // 4), rom, 0x1000)
    for d in words:
        if (t6 + d) & 0xFFFFFFFF < t6:
            t4 = (t4 + 1) & 0xFFFFFFFF
        t6 = (t6 + d) & 0xFFFFFFFF
        t3 ^= d
        r = _rol(d, d & 0x1F)
        t5 = (t5 + r) & 0xFFFFFFFF
        if t2 > d:
            t2 ^= r
        else:
            t2 ^= t6 ^ d
        t1 = (t1 + (t5 ^ d)) & 0xFFFFFFFF
    return (t6 ^ t4 ^ t3) & 0xFFFFFFFF, (t5 ^ t2 ^ t1) & 0xFFFFFFFF


def fix_crc(rom):
    struct.pack_into(">II", rom, 0x10, *cic6102_crc(rom))


def build_members(romfile, data, split, level=6, all_members=False):
    """gzip bytes for a pair with new inflated data (split = first member's
    length); unchanged members keep their original deflate stream (unless
    all_members: then every member is compressed again at `level`)."""
    out = b""
    parts = [data[:split], data[split:]] if len(romfile.members) == 2 else [data]
    for m, part in zip(romfile.members, parts):
        nm = gz.Member(part, m.name, m.mtime, m.xfl, m.os, m.flags, m.extra, m.comment)
        if part == m.data and not all_members:
            out += nm.header() + m.deflate + nm.trailer()
        else:
            out += nm.build(gz.compress(part, level))
    return out


def pack_to_fit(romfile, data, split):
    """The gzip bytes for a changed file, trying to stay inside its slot:
    level 6 (what the ROM used; byte-identical for unchanged data), then
    level 9 for the changed member, then level 9 for both.  Returns
    (blob, how)."""
    slot = romfile.info.get("slot_end", romfile.end) - romfile.start
    best = None
    for how, kw in (("level 6", {}), ("level 9", {"level": 9}), ("level 9, both members", {"level": 9, "all_members": True})):
        blob = build_members(romfile, data, split, **kw)
        if len(blob) <= slot:
            return blob, how
        if best is None or len(blob) < len(best[0]):
            best = (blob, how)
    return best


class Patch:
    """one replaced file: its RomFile, the new gzip bytes, and how it was placed"""

    def __init__(self, romfile, blob, changed, how="level 6"):
        self.file, self.blob, self.changed, self.how = romfile, blob, changed, how
        self.slot = romfile.info.get("slot_end", romfile.end) - romfile.start
        self.fits = len(blob) <= self.slot


def apply_in_place(rom_bytes, patches):
    """Write the patches that fit into a copy of the ROM; returns (rom, [patches that don't fit])."""
    out = bytearray(rom_bytes)
    rest = []
    for p in patches:
        if not p.changed:
            continue
        if not p.fits:
            rest.append(p)
            continue
        s = p.file.start
        out[s:s + p.slot] = p.blob + bytes(p.slot - len(p.blob))
    fix_crc(out)
    return bytes(out), rest


def shift_config(patches, outdir, assets_units=None):
    """For patches that don't fit: write each compressed file and a pads JSON
    for the shiftable build.  The whole pair goes into the unit at the file's
    start; the unit of its second member (if the assets yaml has one) is
    emptied, so nothing can come between the two members."""
    os.makedirs(outdir, exist_ok=True)
    rep = []
    for p in patches:
        path = os.path.join(outdir, "%06X_%s.gz" % (p.file.start, p.file.name))
        with open(path, "wb") as f:
            f.write(p.blob)
        e = {"at": "0x%06X" % p.file.start, "path": os.path.abspath(path)}
        if len(p.file.members) == 2:
            e["empty"] = ["0x%06X" % (p.file.members[1].start - p.file.members[1].pad)]
        rep.append(e)
    cfg = {"rom": {"replace": rep}}
    path = os.path.join(outdir, "shift_pads.json")
    with open(path, "w") as f:
        json.dump(cfg, f, indent=1)
    return path
