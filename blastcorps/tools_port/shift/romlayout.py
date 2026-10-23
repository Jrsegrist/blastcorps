"""The ROM as units (blastcorps.us.v11.assets.yaml), laid out again.

Every top-level entry of the assets splat file is a ROM unit (header, IPL3,
init, the texture table, each compressed file, each texture, the gaps
between them).  A few places the game addresses that the file doesn't split
are split here (EXTRA_STARTS).  Units are placed in order, each at the next
address with its original alignment (the lowest set bit of its original start,
at most 16), so inserting, growing or dropping a unit moves the ones after it
and nothing else.

Dynamic units take built bytes (init, hd_code's and the front end's
compressed text and data); the two self-relative tables are rewritten
(the texture table at 0x4CE0, offsets from itself, 8-byte entries; the object
table at 0x6EC4C0, offsets from itself, words); the trailer (0xFF) fills to
the end; the header CRCs are recomputed (CIC-6102).
"""
import re
import struct

from shiftlib import die

EXTRA_STARTS = {
    0x6EC4C0: "objtable",       # lvl_load_object(_table): the building/model table (0x200 offsets from here)
    0x787F40: "static_segment",  # romStaticSegment (hd_mode_setup DMAs 0xC0 bytes from here)
    0x77762D: "cohut1_dupe",     # a second copy of cohut1, used by the object table (entry 241)
    0x777AFC: "cohut1_dl_dupe",
}
TEX_TABLE = 0x4CE0
TEX_ENTRIES = 0x1000
OBJ_TABLE = 0x6EC4C0
OBJ_ENTRIES = 0x200
DYNAMIC = {"init.us.v11": "init", "hd_code_text": "hd_code_text", "hd_code_data": "hd_code_data",
           "hd_front_end_text": "hd_front_end_text", "hd_front_end_data": "hd_front_end_data"}
ROM_LIMIT = 0xFFB000            # hd_boot reads a debug command line from ROM 0xFFB000
# second gzip members: inflated straight after the first one (init's boot for
# hd_code, hd_load_front_end for the front end), so no gap may separate them
STREAM_NEXT = ("hd_code_data", "hd_front_end_data")


class RomUnit:
    __slots__ = ("start", "size", "type", "name", "align", "data", "new_start", "dyn", "fill")

    def __init__(self, start, typ, name):
        self.start, self.type, self.name = start, typ, name
        self.size = None
        self.align = 16 if start is None else 1
        while self.align < 16 and not start & self.align:
            self.align <<= 1
        if name in STREAM_NEXT:
            self.align = 1
        self.data = None
        self.new_start = None
        self.dyn = DYNAMIC.get(name)
        self.fill = False       # alignment padding: no bytes of its own, the next unit's alignment makes it

    @property
    def label(self):
        return "rom_%06X" % self.start if self.start is not None else "rom_new_%s" % self.name


def parse_assets(path):
    """[RomUnit] in ROM order, with sizes."""
    units = {}
    pending = None
    for line in open(path):
        m = re.match(r"^  #?- \[(0x[0-9A-Fa-f]+)(?:, *([\w.]+))?(?:, *([\w.]+))?", line)
        if m:
            a = int(m.group(1), 16)
            units[a] = (m.group(2) or "", m.group(3) or "")
            continue
        m = re.match(r"^  - name: +(\w+)", line)
        if m:
            pending = m.group(1)
            continue
        m = re.match(r"^    type: +(\w+)", line)
        if m and pending:
            ptype = m.group(1)
            continue
        m = re.match(r"^    start: +(0x[0-9A-Fa-f]+)", line)
        if m and pending:
            units[int(m.group(1), 16)] = (ptype, pending)
            pending = None
    for a, n in EXTRA_STARTS.items():
        units.setdefault(a, ("bin", n))
    starts = sorted(units)
    out = []
    for a, b in zip(starts, starts[1:]):
        typ, name = units[a]
        u = RomUnit(a, typ, name if name else "")
        u.size = b - a
        out.append(u)
    if starts[-1] != 0x800000:
        die("assets yaml doesn't end at 0x800000")
    return out


def mark_fills(units, base):
    """Gaps that only align the next unit (all zero, shorter than its
    alignment) become fills, so they grow or shrink with what's before them."""
    for u, nxt in zip(units, units[1:]):
        if u.type in ("", "bin") and (u.type == "" or not u.name) and u.size < nxt.align \
                and not any(base[u.start:u.start + u.size]):
            u.fill = True


class RomMapper:
    def __init__(self, units):
        self.units = [u for u in units if u.start is not None]
        self.starts = [u.start for u in self.units]

    def unit_of(self, rom):
        import bisect
        i = bisect.bisect_right(self.starts, rom) - 1
        if i < 0:
            return None
        u = self.units[i]
        if rom < u.start + u.size or (rom == u.start + u.size and i == len(self.units) - 1):
            return u
        return None

    def map(self, rom, prefer_end=False):
        u = self.unit_of(rom)
        if prefer_end:
            import bisect
            j = bisect.bisect_left(self.starts, rom) - 1
            if j >= 0 and self.units[j].start + self.units[j].size == rom:
                p = self.units[j]
                return p.new_start + len(p.data)
        if u is None:
            die("ROM offset %06X is in no unit" % rom)
        if u.new_start is None:
            die("ROM offset %06X is in %s (%06X), which this build drops" % (rom, u.name, u.start))
        return u.new_start + rom - u.start

    def by_name(self, name):
        for u in self.units:
            if u.name == name:
                return u
        return None


def place(units, pads=(), drops=(), adds=()):
    """Assign new_start to every unit (data must be set).  pads/adds:
    [(before_start, RomUnit)] inserted before the unit that starts at
    before_start; drops: unit starts to leave out."""
    seq = []
    ins = {}
    for before, u in list(pads) + list(adds):
        ins.setdefault(before, []).append(u)
    for u in units:
        for x in ins.get(u.start, []):
            seq.append(x)
        if u.start in drops:
            u.new_start = None
            continue
        seq.append(u)
    pos = 0
    for u in seq:
        if u.fill:
            u.new_start = pos
            continue
        a = u.align
        if pos % a:
            pos += a - pos % a
        u.new_start = pos
        if u.name == "trailer":
            continue
        pos += len(u.data)
    return seq, pos


def write_rom(seq, end, fill_trailer=0xFF):
    size = 0x800000
    while size < end:
        size += 0x100000
    if size > ROM_LIMIT:
        die("ROM too big (%X bytes): hd_boot reads its debug command line at %X" % (end, ROM_LIMIT))
    rom = bytearray(b"\xFF" * size) if fill_trailer == 0xFF else bytearray(size)
    for u in seq:
        if u.name == "trailer" or u.fill:
            continue
        rom[u.new_start:u.new_start + len(u.data)] = u.data
    # zero the alignment gaps (between units) up to the trailer
    prev_end = 0
    for u in seq:
        if u.fill:
            continue
        if u.new_start > prev_end:
            rom[prev_end:u.new_start] = bytes(u.new_start - prev_end)
        prev_end = u.new_start + len(u.data)
    return rom


def rewrite_tex_table(data, mapper, table_new):
    out = bytearray(data)
    for i in range(TEX_ENTRIES):
        off, n, t = struct.unpack_from(">IHH", data, 8 * i)
        rom = TEX_TABLE + off
        new = mapper.map(rom)
        struct.pack_into(">I", out, 8 * i, new - table_new)
    return bytes(out)


def rewrite_obj_table(data, mapper, table_new):
    out = bytearray(data)
    for i in range(OBJ_ENTRIES):
        off = struct.unpack_from(">I", data, 4 * i)[0]
        rom = OBJ_TABLE + off
        new = mapper.map(rom)
        struct.pack_into(">I", out, 4 * i, new - table_new)
    return bytes(out)


def fix_crc(rom):
    from nm_rom import cic6102_crc      # tools_port/nm_rom.py
    struct.pack_into(">II", rom, 0x10, *cic6102_crc(bytes(rom)))
