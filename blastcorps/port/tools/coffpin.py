#!/usr/bin/env python3
"""Pin a game object file's data to N64 addresses (fixed-RAM model, any COFF compiler).

The object-file counterpart of relabel.py (which rewrites gcc's assembly text):
it works on the COFF object itself, so clang (the 64-bit build) and MSVC objects
go through the same step.  Every data object the NON_MATCHING ELF places in N64
memory (ADDRS, from gensyms.py addrs; function-local statics by their
D_<address> name: clang calls them `fn.D_xxxxxxxx`, gcc `D_xxxxxxxx.N`) is taken
out of the object:

  initialised   its bytes stay, renamed __native_<name> (an external symbol, for
                the start-up copy table); every relocation that pointed at it
                points at a new undefined <name>
  zero (bss)    the symbol becomes an undefined external <name>

<name> is then defined by the RDRAM section object (rdramobj.py) at its N64
address, so code in this file and every other one uses the RDRAM copy.  The
object must be built with one section per variable (-fdata-sections; MSVC /Gw):
relocations against a section symbol are moved to the object in that section.

usage: coffpin.py ADDRS IN.o OUT.o RECORD
  RECORD written: 'cname addr kind size' per pinned object (kind data|bss;
          cname = the C name the copy table uses, without __native_)
"""
import re
import struct
import sys

SCN_CNT_CODE = 0x20
SCN_CNT_INIT = 0x40
SCN_CNT_UNINIT = 0x80
CLASS_EXTERNAL = 2
CLASS_STATIC = 3
STATIC_NAME = re.compile(r"^(?:.*\.)?(D_([0-9A-F]{8}))(?:\.\d+)?$")


class Coff:
    def __init__(self, data):
        self.d = bytearray(data)
        (self.machine, self.nsec, _, self.symptr, self.nsyms, self.opthdr, _) = struct.unpack_from("<HHIIIHH", data, 0)
        if self.machine == 0 and self.nsec == 0xFFFF:
            raise SystemExit("coffpin: bigobj objects are not supported")
        strtab = self.symptr + self.nsyms * 18
        (strsize,) = struct.unpack_from("<I", data, strtab)
        self.strtab = bytearray(data[strtab:strtab + strsize])
        if strtab + strsize != len(data):
            raise SystemExit("coffpin: the symbol and string tables are not at the end of the object")
        self.secs = []
        off = 20 + self.opthdr
        for i in range(self.nsec):
            name, vsz, va, rawsz, rawptr, relptr, lnptr, nrel, nln, ch = struct.unpack_from("<8sIIIIIIHHI", data, off)
            if ch & 0x01000000:     # IMAGE_SCN_LNK_NRELOC_OVFL
                (nrel,) = struct.unpack_from("<I", data, relptr)
                relptr += 10
                nrel -= 1
            self.secs.append({"name": self.name_of(name), "size": rawsz, "ptr": rawptr, "relptr": relptr,
                              "nrel": nrel, "ch": ch})
            off += 40
        self.syms = []   # [name, value, secno, type, cls, naux, aux bytes]
        i = 0
        while i < self.nsyms:
            raw = data[self.symptr + i * 18:self.symptr + i * 18 + 18]
            name8, value, secno, typ, cls, naux = struct.unpack("<8sIhHBB", raw)
            aux = bytes(data[self.symptr + (i + 1) * 18:self.symptr + (i + 1 + naux) * 18])
            self.syms.append([self.name_of(name8), value, secno, typ, cls, naux, aux, i])
            i += 1 + naux

    def name_of(self, name8):
        if name8[:4] == b"\0\0\0\0":
            (off,) = struct.unpack_from("<I", name8, 4)
            end = self.strtab.index(b"\0", off)
            return self.strtab[off:end].decode()
        if name8[:1] == b"/" and name8[1:2].isdigit():
            off = int(name8[1:].rstrip(b"\0"))
            end = self.strtab.index(b"\0", off)
            return self.strtab[off:end].decode()
        return name8.rstrip(b"\0").decode()

    def relocs(self):
        """(section index, file offset of the reloc's symbol index field, symbol index)"""
        for si, s in enumerate(self.secs):
            for k in range(s["nrel"]):
                p = s["relptr"] + k * 10
                (_, symi, _) = struct.unpack_from("<IIH", self.d, p)
                yield si, p + 4, symi

    def encode_name(self, name):
        b = name.encode()
        if len(b) <= 8:
            return b.ljust(8, b"\0")
        off = len(self.strtab)
        self.strtab += b + b"\0"
        return b"\0\0\0\0" + struct.pack("<I", off)

    def write(self):
        out = bytearray(self.d[:self.symptr])
        n = 0
        for name, value, secno, typ, cls, naux, aux, _ in self.syms:
            out += self.encode_name(name) + struct.pack("<IhHBB", value, secno, typ, cls, naux) + aux
            n += 1 + naux
        struct.pack_into("<I", self.strtab, 0, len(self.strtab))
        out += self.strtab
        struct.pack_into("<I", out, 12, n)
        return bytes(out)


def main():
    addrs_path, inp, outp, rec_path = sys.argv[1:5]
    addrs = {}
    for line in open(addrs_path):
        p = line.split()
        if len(p) >= 2:
            addrs[p[0]] = int(p[1], 16)
    c = Coff(open(inp, "rb").read())
    # symbol table index -> position in c.syms
    by_index = {s[7]: k for k, s in enumerate(c.syms)}
    # objects per section (non-section symbols), to size them and to resolve
    # relocations against section symbols
    per_sec = {}
    for k, s in enumerate(c.syms):
        name, value, secno, typ, cls, naux = s[:6]
        if secno > 0 and not (cls == CLASS_STATIC and naux > 0 and value == 0 and name == c.secs[secno - 1]["name"]):
            if cls in (CLASS_EXTERNAL, CLASS_STATIC) and not name.startswith((".L", "$")):
                per_sec.setdefault(secno, []).append((value, k))
    for v in per_sec.values():
        v.sort()
    pinned = {}     # c.syms position -> (key, kind, size)
    for k, s in enumerate(c.syms):
        name, value, secno, typ, cls, naux = s[:6]
        if secno <= 0 or cls not in (CLASS_EXTERNAL, CLASS_STATIC):
            continue
        sec = c.secs[secno - 1]
        if sec["ch"] & SCN_CNT_CODE:
            continue
        if name == sec["name"] and naux:
            continue     # the section symbol
        if name in addrs and cls == CLASS_EXTERNAL:
            key = name
        else:
            m = STATIC_NAME.match(name)
            if not m or not (0x80000000 <= int(m.group(2), 16) < 0x80800000):
                continue
            key = m.group(1)
            if key not in addrs:
                addrs[key] = int(m.group(2), 16)
        objs = per_sec[secno]
        pos = [o[0] for o in objs].index(value)
        end = objs[pos + 1][0] if pos + 1 < len(objs) else sec["size"]
        kind = "bss" if sec["ch"] & SCN_CNT_UNINIT else "data"
        pinned[k] = (key, kind, end - value, secno, value)
    # relocations: to a pinned symbol, or to the section symbol of a pinned
    # object's section
    sec_sym = {}
    for k, s in enumerate(c.syms):
        name, value, secno, typ, cls, naux = s[:6]
        if secno > 0 and cls == CLASS_STATIC and naux and value == 0 and name == c.secs[secno - 1]["name"]:
            sec_sym[s[7]] = secno
    sec_pinned = {}
    for k, (key, kind, size, secno, value) in pinned.items():
        sec_pinned.setdefault(secno, []).append(k)
    new_syms = {}    # key -> new symbol table index (appended undefined externals)
    next_index = c.nsyms

    def undef_index(key):
        nonlocal next_index
        if key not in new_syms:
            new_syms[key] = next_index
            c.syms.append([key, 0, 0, 0, CLASS_EXTERNAL, 0, b"", next_index])
            next_index += 1
        return new_syms[key]

    recs = []
    for k, (key, kind, size, secno, value) in sorted(pinned.items()):
        s = c.syms[k]
        if s[5]:
            raise SystemExit("coffpin: %s: data symbol %s has auxiliary records" % (inp, s[0]))
        if kind == "data":
            s[0] = "__native_" + key
            s[4] = CLASS_EXTERNAL
            undef_index(key)
        else:
            s[0], s[1], s[2], s[4] = key, 0, 0, CLASS_EXTERNAL
        recs.append("%s %08X %s %d" % (key, addrs[key], kind, size))
    moved = 0
    for si, field, symi in c.relocs():
        k = by_index.get(symi)
        if k is None:
            continue
        if k in pinned:
            key, kind = pinned[k][0], pinned[k][1]
            if kind == "data":
                struct.pack_into("<I", c.d, field, undef_index(key))
                moved += 1
            continue
        if symi in sec_sym and sec_sym[symi] in sec_pinned:
            secno = sec_sym[symi]
            ks = sec_pinned[secno]
            others = [o for o in per_sec.get(secno, []) if o[1] not in ks]
            if len(ks) != 1 or pinned[ks[0]][4] != 0 or others:
                raise SystemExit("coffpin: %s: relocation against section %s, which holds more than one "
                                 "object (build with -fdata-sections)" % (inp, c.secs[secno - 1]["name"]))
            key, kind = pinned[ks[0]][0], pinned[ks[0]][1]
            struct.pack_into("<I", c.d, field, undef_index(key) if kind == "data" else c.syms[ks[0]][7])
            moved += 1
    # duplicate keys (two statics with the same address name) would clash
    keys = [p[0] for p in pinned.values()]
    if len(keys) != len(set(keys)):
        raise SystemExit("coffpin: %s: an N64 address is defined twice: %s" %
                         (inp, sorted(k for k in keys if keys.count(k) > 1)))
    open(outp, "wb").write(c.write())
    with open(rec_path, "w") as f:
        f.write("".join(r + "\n" for r in recs))


if __name__ == "__main__":
    main()
