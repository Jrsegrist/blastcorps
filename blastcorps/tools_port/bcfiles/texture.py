"""Textures: the texture table's seven packings and N64 texel formats.

Table (ROM 0x4CE0, 4096 entries): {u32 offset from 0x4CE0; u16 packed size;
u16 packing}.  An entry's DMA runs to the next entry's offset; `size` bytes
are decoded (tex_decode, src.us.v11/hd_code/60F60.c).

Packings 1-6 read big-endian s16 tokens.  Bit 15 set: copy (tok & 0x1F)
units from earlier output, one at a time; the distance is
(tok & 0x7FFF) >> 5 bytes for 16-bit units (1, 3, 6) and
(tok & 0x7FE0) >> 4 bytes for 32-bit units (2, 4, 5).  Otherwise a literal:

  0  stored: size >> 3 doublewords (a trailing size & 7 is dropped)
  1  one u16: ((tok & 0xFFC0) << 1) | (tok & 0x3F)          RGBA16 5551
  2  one u32: 4-bit R, G, B to the top of bytes 0-2, 3-bit A to byte 3
  3  two bytes, each << 1                                     (IA8 pairs)
  4  two u16 via a 128-entry u16 palette: (pal[b >> 1] << 1) | (b & 1)
  5  one u32 via a palette of 5-5-5 colours, 4-bit alpha
  6  two bytes xxAAABBB -> AAA0BBB0                           (I4 pairs)

The texel format and size are not stored with the texture; the display
list that uses it says (G_SETTIMG / G_SETTILE / G_SETTILESIZE).
"""
import struct

TEX_TABLE = 0x4CE0
TEX_ENTRIES = 0x1000

UNIT = {1: 2, 2: 4, 3: 2, 4: 4, 5: 4, 6: 2}


def _backref(out, tok, wide):
    n = tok & 0x1F
    if wide:
        dist = (tok & 0x7FE0) >> 4
        unit = 4
    else:
        dist = (tok & 0x7FFF) >> 5
        unit = 2
    src = len(out) - dist
    for _ in range(n):
        out += out[src:src + unit] if src + unit <= len(out) else bytes(
            out[src + k] if src + k < len(out) else 0 for k in range(unit))
        src += unit


def decode(packed, ptype, palette=None):
    """Decode one texture's packed bytes (exactly `size` of them)."""
    if ptype == 0:
        return bytes(packed[:len(packed) & ~7])
    if ptype not in UNIT:
        return b""
    wide = UNIT[ptype] == 4
    out = bytearray()
    pal = palette or bytes(0x100)
    for (tok,) in struct.iter_unpack(">H", bytes(packed[:len(packed) & ~1])):
        if tok & 0x8000:
            _backref(out, tok, wide)
            continue
        if ptype == 1:
            out += struct.pack(">H", ((tok & 0xFFC0) << 1) | (tok & 0x3F))
        elif ptype == 2:
            v = ((tok & 0x7800) << 17) | ((tok & 0x0780) << 13) | ((tok & 0x78) << 9) | ((tok & 7) << 5)
            out += struct.pack(">I", v)
        elif ptype == 3:
            out += bytes((((tok >> 8) << 1) & 0xFF, ((tok & 0xFF) << 1) & 0xFF))
        elif ptype == 4:
            b0, b1 = tok >> 8, tok & 0xFF
            p0 = struct.unpack_from(">H", pal, b0 & 0xFE)[0]
            p1 = struct.unpack_from(">H", pal, b1 & 0xFE)[0]
            out += struct.pack(">HH", ((p0 << 1) | (b0 & 1)) & 0xFFFF, ((p1 << 1) | (b1 & 1)) & 0xFFFF)
        elif ptype == 5:
            i = (tok >> 4) << 1
            p = struct.unpack_from(">H", pal, i)[0] if i + 2 <= len(pal) else 0
            v = ((p & 0x7C00) << 17) | ((p & 0x03E0) << 14) | ((p & 0x1F) << 11) | ((tok & 0xF) << 4)
            out += struct.pack(">I", v)
        elif ptype == 6:
            def f(b):
                return ((b & 0x38) << 2) | ((b & 7) << 1)
            out += bytes((f(tok >> 8), f(tok & 0xFF)))
    return bytes(out)


def encode_stored(texels):
    """Packing 0 for raw texels (padded to a multiple of 8)."""
    t = bytes(texels)
    return t + bytes(-len(t) % 8)


class TexEntry:
    __slots__ = ("index", "offset", "size", "type", "dma_len")

    def __init__(self, index, offset, size, ptype, dma_len):
        self.index, self.offset, self.size, self.type, self.dma_len = index, offset, size, ptype, dma_len

    @property
    def rom(self):
        return TEX_TABLE + self.offset


def read_table(rom):
    ents = []
    raw = [struct.unpack_from(">IHH", rom, TEX_TABLE + 8 * i) for i in range(TEX_ENTRIES)]
    for i, (off, size, t) in enumerate(raw):
        nxt = raw[i + 1][0] if i + 1 < TEX_ENTRIES else off + size
        ents.append(TexEntry(i, off, size, t, nxt - off))
    return ents


# --------------------------------------------------------------- texel formats

G_IM_FMT = {0: "RGBA", 1: "YUV", 2: "CI", 3: "IA", 4: "I"}
G_IM_SIZ = {0: 4, 1: 8, 2: 16, 3: 32}


def to_rgba8(texels, fmt, siz, width, height, tlut=None):
    """N64 texels -> RGBA8 rows (bytes, width*height*4).  fmt: 0 RGBA, 2 CI,
    3 IA, 4 I; siz: bits per texel (4, 8, 16, 32)."""
    n = width * height
    out = bytearray(n * 4)
    t = bytes(texels)

    def px(i):
        if siz == 4:
            b = t[i >> 1] if (i >> 1) < len(t) else 0
            return (b >> 4) if not i & 1 else (b & 0xF)
        if siz == 8:
            return t[i] if i < len(t) else 0
        if siz == 16:
            return (t[2 * i] << 8 | t[2 * i + 1]) if 2 * i + 1 < len(t) else 0
        return struct.unpack_from(">I", t, 4 * i)[0] if 4 * i + 3 < len(t) else 0

    for i in range(n):
        v = px(i)
        if fmt == 0 and siz == 16:
            r, g, b, a = (v >> 11) & 31, (v >> 6) & 31, (v >> 1) & 31, v & 1
            c = ((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2), 255 if a else 0)
        elif fmt == 0 and siz == 32:
            c = (v >> 24, (v >> 16) & 255, (v >> 8) & 255, v & 255)
        elif fmt == 3 and siz == 4:
            iv = (v >> 1) * 255 // 7
            c = (iv, iv, iv, 255 if v & 1 else 0)
        elif fmt == 3 and siz == 8:
            iv, a = (v >> 4) * 17, (v & 15) * 17
            c = (iv, iv, iv, a)
        elif fmt == 3 and siz == 16:
            c = (v >> 8, v >> 8, v >> 8, v & 255)
        elif fmt == 4 and siz == 4:
            c = (v * 17,) * 4
        elif fmt == 4 and siz == 8:
            c = (v,) * 4
        elif fmt == 2 and tlut is not None:
            e = struct.unpack_from(">H", tlut, 2 * v)[0] if 2 * v + 2 <= len(tlut) else 0
            r, g, b, a = (e >> 11) & 31, (e >> 6) & 31, (e >> 1) & 31, e & 1
            c = ((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2), 255 if a else 0)
        else:
            iv = v & 255
            c = (iv, iv, iv, 255)
        out[4 * i:4 * i + 4] = bytes(c)
    return bytes(out)
