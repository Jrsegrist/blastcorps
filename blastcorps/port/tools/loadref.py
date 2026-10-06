#!/usr/bin/env python3
"""Reference outputs for port/src/load/loadcheck_main.c: the same cases run
through the ORIGINAL MIPS code (build/, the matching ELFs) in unicorn on
big-endian memory, with the assets taken straight from the ROM.  Prints the
same lines as loadcheck.exe.

usage: loadref.py ROM [TESTS] [build-dir]     (run from the project root)
"""
import re
import struct
import sys
import zlib

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from unicorn import Uc, UcError, UC_ARCH_MIPS, UC_MODE_MIPS64, UC_MODE_BIG_ENDIAN
from unicorn import mips_const as M

ROMP = sys.argv[1]
TESTS = sys.argv[2] if len(sys.argv) > 2 else "5678"
BUILD = sys.argv[3] if len(sys.argv) > 3 else "build"
rom = open(ROMP, "rb").read()
RAM = 0x00800000
SENTINEL = 0x80000010
STACK = 0x807FF000


def sext(v):
    v &= 0xFFFFFFFF
    return v | 0xFFFFFFFF00000000 if v & 0x80000000 else v


uc = Uc(UC_ARCH_MIPS, UC_MODE_MIPS64 | UC_MODE_BIG_ENDIAN)
uc.ctl_set_cpu_model(M.UC_CPU_MIPS64_R4000)
uc.mem_map(0, RAM)
syms = {}
for name in ("hd_code", "hd_front_end"):
    with open("%s/%s.us.v11.elf" % (BUILD, name), "rb") as f:
        elf = ELFFile(f)
        for s in elf.iter_sections():
            if s["sh_flags"] & 2 and s["sh_size"] and s["sh_type"] != "SHT_NOBITS":
                uc.mem_write(s["sh_addr"] & 0x1FFFFFFF, s.data())
            if isinstance(s, SymbolTableSection):
                for sy in s.iter_symbols():
                    if sy.name and sy.name not in syms:
                        syms[sy.name] = sy["st_value"]
REG = {n: getattr(M, "UC_MIPS_REG_" + n.upper()) for n in
       ("a0", "a1", "a2", "a3", "v0", "t0", "gp", "sp", "ra", "pc")}


def call(func, **regs):
    for r, v in regs.items():
        uc.reg_write(REG[r], sext(v))
    uc.reg_write(REG["sp"], sext(STACK))
    uc.reg_write(REG["ra"], sext(SENTINEL))
    uc.emu_start(sext(syms[func]), sext(SENTINEL), count=20000000)
    return uc.reg_read(REG["v0"]) & 0xFFFFFFFF


def rd(a, n): return bytes(uc.mem_read(a & 0x1FFFFFFF, n))
def r8(a): return rd(a, 1)[0]
def r16(a): return struct.unpack(">H", rd(a, 2))[0]
def r32(a): return struct.unpack(">I", rd(a, 4))[0]
def wr(a, b): uc.mem_write(a & 0x1FFFFFFF, bytes(b))
def w8(a, v): wr(a, [v & 0xFF])
def w16(a, v): wr(a, struct.pack(">H", v & 0xFFFF))
def w32(a, v): wr(a, struct.pack(">I", v & 0xFFFFFFFF))


class Rng:
    def __init__(self, s): self.s = s

    def __call__(self):
        self.s = (self.s * 1103515245 + 12345) & 0xFFFFFFFF
        return self.s >> 8


HEAP, PARAM = 0x80100000, 0x801F0000
out = []

if "5" in TESTS:
    for flag in (0, 1):
        for i in range(19):
            w8(0x802E8BF8, flag)
            out.append("T5 %d.%02d %08X" % (flag, i, r32(0x802E8BF8 + 4 * i)))

if "6" in TESTS:
    for i in range(0x200):
        w16(PARAM + i * 2, i * 0x2B7 + 0x55)
    TAB = 0x4CE0
    for tid in range(0, 0xFFF, 7):
        romo, size, typ = struct.unpack(">IHH", rom[TAB + tid * 8:TAB + tid * 8 + 8])
        nxt = struct.unpack(">I", rom[TAB + tid * 8 + 8:TAB + tid * 8 + 12])[0]
        if nxt <= romo or nxt - romo > 0x8000 or typ > 6:
            continue
        buf = 0x80180000
        wr(buf, rom[TAB + romo:TAB + nxt])
        req = 0x803C4B58
        w32(req, buf); w32(req + 4, size); w32(req + 8, typ)
        w32(req + 12, PARAM if typ in (4, 5) else 0)
        n = call("func_802A57DC", a0=req)
        out.append("T6 %03X %d %05X %08X" % (tid, typ, n, zlib.crc32(rd(buf, n)) & 0xFFFFFFFF))


def level_starts():
    """[(start, end)] of the 60 levels, from func_8025615C's switch (00000.c)"""
    src = open("src.us.v11/hd_code/00000.c").read()
    body = src[src.index("void func_8025615C(s32 level"):]
    body = body[:body.index("\n}\n")]
    res = {}
    for m in re.finditer(r"case (\d+):\s*start = D_00([0-9A-F]{6});\s*\*size = D_00([0-9A-F]{6})", body):
        res[int(m.group(1))] = (int(m.group(2), 16), int(m.group(3), 16))
    return res


def level_blob(lvl):
    st, en = LEVELS[lvl]
    data = rom[st:en]
    blob = b""
    for _ in range(2):     # func_8028B4C4(..., 12, 10, 1): two gzip members
        d = zlib.decompressobj(31)
        blob += d.decompress(data)
        data = d.unused_data
    return blob


if "7" in TESTS or "8" in TESTS:
    LEVELS = level_starts()

if "7" in TESTS:
    for lvl in range(60):
        blob = level_blob(lvl)
        wr(HEAP, blob)
        try:
            call("func_802A2C54", t0=HEAP)
        except UcError:
            out.append("T7 %02d.size CRASH" % lvl)
            continue
        end = r32(0x803BDFD4)
        out.append("T7 %02d.size %05X first %02X n %u" % (lvl, len(blob), r8(0x80364A6E), (end - 0x803BDFD8) // 0x24))
        e = 0x803BDFD8
        while e < end and e < 0x803BDFD8 + 0x24 * 400:
            n = r8(e + 0x13)
            out.append("T7 %02d.%04X %08X %08X %08X %08X %02X %02X %02X %02X %02X %s" % (
                lvl, (e - 0x803BDFD8) // 0x24, r32(e), r32(e + 4), r32(e + 8), r32(e + 0xC), r8(e + 0x10),
                r8(e + 0x11), r8(e + 0x12), r8(e + 0x13), r8(e + 0x14), rd(e + 0x15, min(n, 0xF)).hex().upper()))
            e += 0x24

if "8" in TESTS:
    rnd = Rng(8)
    for lvl in range(60):
        wr(HEAP, level_blob(lvl))
        for i in range(40):
            a = ((rnd() % 0x400) << 5) | (rnd() & 31)
            w32(0x803643E0, a)
            a = ((rnd() % 0x400) << 5) | (rnd() & 31)
            w32(0x803643E8, a)
            w16(0x80364450, rnd() & 0xFFFF)
            w16(0x8036444E, 0x1234)
            w8(0x80364411, 0x55)
            try:
                call("func_802A5510", a0=HEAP)
            except UcError:
                out.append("T8 %02d.%02d CRASH" % (lvl, i))
                continue
            out.append("T8 %02d.%02d %04X %02X" % (lvl, i, r16(0x8036444E), r8(0x80364411)))

print("\n".join(out))
