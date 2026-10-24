"""Gzip members as the game reads them (kiunzip, src.us.v11/hd_code/17A70.c).

The reader skips one byte when a member doesn't start with 0x1F, needs
method 8 (deflate), skips FEXTRA/FNAME/FCOMMENT/FHCRC when the flags say so,
inflates, and steps over the 8-byte CRC32/ISIZE trailer without checking it.
Every compressed file in the ROM carries FNAME (the original file name),
MTIME (a time stamp from 1997), XFL 0 and OS 3 (Unix).

Recompression: the members in the ROM were made by gzip 1.2.4-era deflate at
level 6.  Python's zlib reproduces some of them exactly but not all (its
block-splitting differs); the repo's own gzip binary (tools/gzip, the one the
shiftable build compresses the code with) reproduces all of them.
`compress()` uses that binary when it is there and zlib otherwise; either
output is a standard deflate member the game accepts.
"""
import os
import struct
import subprocess
import zlib

from .binio import FormatError

FTEXT, FHCRC, FEXTRA, FNAME, FCOMMENT = 1, 2, 4, 8, 0x10


class Member:
    """One gzip member: header fields, the inflated data, where it sat."""

    def __init__(self, data=b"", name=None, mtime=0, xfl=0, os_=3, flags=None, extra=None, comment=None,
                 deflate=None, start=None, end=None, pad=0):
        self.data = data
        self.name = name
        self.mtime = mtime
        self.xfl = xfl
        self.os = os_
        self.flags = flags if flags is not None else (FNAME if name is not None else 0)
        self.extra = extra
        self.comment = comment
        self.deflate = deflate        # the original deflate stream (for byte-exact rebuilds)
        self.start = start            # offset of the member (after the optional pad byte)
        self.end = end                # offset just past its trailer
        self.pad = pad                # 1 if the reader skipped a byte before it

    def header(self):
        h = bytearray(b"\x1f\x8b\x08")
        h.append(self.flags)
        h += struct.pack("<I", self.mtime & 0xFFFFFFFF)
        h.append(self.xfl)
        h.append(self.os)
        if self.flags & FEXTRA:
            h += struct.pack("<H", len(self.extra)) + self.extra
        if self.flags & FNAME:
            h += self.name.encode("latin-1") + b"\0"
        if self.flags & FCOMMENT:
            h += self.comment.encode("latin-1") + b"\0"
        if self.flags & FHCRC:
            h += struct.pack("<H", zlib.crc32(bytes(h)) & 0xFFFF)
        return bytes(h)

    def trailer(self):
        return struct.pack("<II", zlib.crc32(self.data) & 0xFFFFFFFF, len(self.data) & 0xFFFFFFFF)

    def build(self, deflate=None):
        """The member's bytes.  deflate: a deflate stream for self.data (default:
        the original one if the data is unchanged, else a new compression)."""
        if deflate is None:
            deflate = self.deflate if self.deflate is not None and self.original_data_matches() else compress(self.data)
        return self.header() + deflate + self.trailer()

    def original_data_matches(self):
        return getattr(self, "_orig_crc", None) == (zlib.crc32(self.data), len(self.data))

    def meta_json(self):
        d = {"name": self.name, "mtime": self.mtime, "xfl": self.xfl, "os": self.os, "flags": self.flags}
        if self.extra is not None:
            d["extra"] = self.extra.hex()
        if self.comment is not None:
            d["comment"] = self.comment
        return d

    @classmethod
    def from_meta(cls, d, data):
        return cls(data, d.get("name"), d.get("mtime", 0), d.get("xfl", 0), d.get("os", 3), d.get("flags"),
                   bytes.fromhex(d["extra"]) if d.get("extra") else None, d.get("comment"))


def read_member(buf, pos):
    """Parse the gzip member at buf[pos:] the way kiunzip does (one pad byte
    allowed).  Returns a Member with .start/.end set."""
    pad = 0
    if pos < len(buf) and buf[pos] != 0x1F:
        pos += 1
        pad = 1
    if buf[pos:pos + 2] != b"\x1f\x8b":
        raise FormatError("no gzip member at 0x%X" % pos)
    method, flg = buf[pos + 2], buf[pos + 3]
    if method != 8:
        raise FormatError("gzip member at 0x%X: method %d (kiunzip: unknown method)" % (pos, method))
    mtime = struct.unpack_from("<I", buf, pos + 4)[0]
    xfl, os_ = buf[pos + 8], buf[pos + 9]
    q = pos + 10
    extra = name = comment = None
    if flg & FEXTRA:
        n = struct.unpack_from("<H", buf, q)[0]
        extra = bytes(buf[q + 2:q + 2 + n])
        q += 2 + n
    if flg & FNAME:
        e = buf.index(0, q)
        name = bytes(buf[q:e]).decode("latin-1")
        q = e + 1
    if flg & FCOMMENT:
        e = buf.index(0, q)
        comment = bytes(buf[q:e]).decode("latin-1")
        q = e + 1
    if flg & FHCRC:
        q += 2
    d = zlib.decompressobj(-15)
    src = bytes(buf[q:])
    data = d.decompress(src)
    if not d.eof:
        raise FormatError("gzip member at 0x%X: truncated deflate stream" % pos)
    used = len(src) - len(d.unused_data)
    m = Member(data, name, mtime, xfl, os_, flg, extra, comment, src[:used], pos, q + used + 8, pad)
    m._orig_crc = (zlib.crc32(data), len(data))
    crc, isize = struct.unpack_from("<II", buf, q + used)
    m.trailer_ok = crc == zlib.crc32(data) & 0xFFFFFFFF and isize == len(data) & 0xFFFFFFFF
    return m


def read_members(buf, pos, count):
    out = []
    for _ in range(count):
        m = read_member(buf, pos)
        out.append(m)
        pos = m.end
    return out


# ------------------------------------------------------------- recompression

_GZIP_TOOL = None


def gzip_tool():
    """The repo's gzip binary (tools/gzip), or $BCFILES_GZIP, or None."""
    global _GZIP_TOOL
    if _GZIP_TOOL is None:
        cand = [os.environ.get("BCFILES_GZIP", "")]
        here = os.path.dirname(os.path.abspath(__file__))
        for up in (os.path.join(here, "..", "..", "..", "tools", "gzip"), os.path.join(here, "..", "..", "tools", "gzip")):
            cand.append(os.path.normpath(up))
        _GZIP_TOOL = ""
        for c in cand:
            if c and os.path.isfile(c) and os.access(c, os.X_OK):
                _GZIP_TOOL = c
                break
    return _GZIP_TOOL or None


MODE = os.environ.get("BCFILES_DEFLATE", "auto")   # auto | tool | zlib


def compress(data, level=6):
    """A raw deflate stream for data: the repo's gzip binary when available
    (byte-identical to the ROM's members), else zlib (level 6)."""
    tool = gzip_tool() if MODE in ("auto", "tool") else None
    if tool:
        r = subprocess.run([tool, "-%d" % level, "--no-name", "-c"], input=bytes(data), capture_output=True)
        if r.returncode == 0 and len(r.stdout) >= 18:
            return r.stdout[10:-8]
        if MODE == "tool":
            raise RuntimeError("gzip tool failed: %s" % r.stderr.decode(errors="replace"))
    elif MODE == "tool":
        raise RuntimeError("BCFILES_DEFLATE=tool but no gzip binary found (set BCFILES_GZIP)")
    c = zlib.compressobj(level, zlib.DEFLATED, -15, 8)
    return c.compress(bytes(data)) + c.flush()


def compress_zlib(data, level=6, mem=8):
    c = zlib.compressobj(level, zlib.DEFLATED, -15, mem)
    return c.compress(bytes(data)) + c.flush()


def which_compressor():
    t = gzip_tool() if MODE in ("auto", "tool") else None
    return ("gzip binary %s" % t) if t else "zlib level 6"
