#!/usr/bin/env python3
"""Reference outputs for port/src/spike_main.c: the same test cases run
through the ORIGINAL MIPS code (build/, the matching ELFs) in unicorn, with
memory big-endian as on the N64.  Prints the same lines as spike.exe.

usage: n64ref.py [build-dir]     (run from the project root)
"""
import struct
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from unicorn import Uc, UcError, UC_ARCH_MIPS, UC_MODE_MIPS64, UC_MODE_BIG_ENDIAN, UC_HOOK_CODE
from unicorn import mips_const as M

BUILD = sys.argv[1] if len(sys.argv) > 1 else "build"
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

REG = {"a0": M.UC_MIPS_REG_A0, "a1": M.UC_MIPS_REG_A1, "a2": M.UC_MIPS_REG_A2, "a3": M.UC_MIPS_REG_A3,
       "v0": M.UC_MIPS_REG_V0, "gp": M.UC_MIPS_REG_GP, "sp": M.UC_MIPS_REG_SP, "ra": M.UC_MIPS_REG_RA,
       "pc": M.UC_MIPS_REG_PC}

hooks = {}


def on_code(uc_, addr, size, ud):
    a = addr & 0xFFFFFFFF
    if a in hooks:
        hooks[a]()
        uc_.reg_write(REG["pc"], uc_.reg_read(REG["ra"]))


uc.hook_add(UC_HOOK_CODE, on_code)


def call(func, **regs):
    for r, v in regs.items():
        uc.reg_write(REG[r], sext(v))
    uc.reg_write(REG["sp"], sext(STACK))
    uc.reg_write(REG["ra"], sext(SENTINEL))
    try:
        uc.emu_start(sext(syms[func]), sext(SENTINEL), count=1000000)
    except UcError as e:
        raise SystemExit("%s: %s at pc %08X" % (func, e, uc.reg_read(REG["pc"]) & 0xFFFFFFFF))
    return uc.reg_read(REG["v0"]) & 0xFFFFFFFF


def r8(a): return uc.mem_read(a & 0x1FFFFFFF, 1)[0]
def r16(a): return struct.unpack(">H", uc.mem_read(a & 0x1FFFFFFF, 2))[0]
def w8(a, v): uc.mem_write(a & 0x1FFFFFFF, bytes([v & 0xFF]))
def w16(a, v): uc.mem_write(a & 0x1FFFFFFF, struct.pack(">H", v & 0xFFFF))
def w32(a, v): uc.mem_write(a & 0x1FFFFFFF, struct.pack(">I", v & 0xFFFFFFFF))
def w64(a, v): uc.mem_write(a & 0x1FFFFFFF, struct.pack(">Q", v & 0xFFFFFFFFFFFFFFFF))


class Rng:
    def __init__(self, s): self.s = s

    def __call__(self):
        self.s = (self.s * 1103515245 + 12345) & 0xFFFFFFFF
        return self.s >> 8


out = []
# T1
for i in range(0x10000):
    out.append("T1 %04X %08X" % (i, call("func_802AD7D4", a0=i)))
# T2
for i in range(21):
    w8(0x803BE73A, i); w16(0x8036444C, 0xAAAA); w16(0x80364450, 0xBBBB)
    r = call("func_802A56C4")
    out.append("T2 %02X %08X %04X %04X" % (i, r, r16(0x8036444C), r16(0x80364450)))
# T3
LEVEL = 0x80100000
rnd = Rng(3)
for i in range(300):
    nz = rnd() % 9
    w32(LEVEL + 0x40, 0x50); w32(LEVEL + 0x44, 0x50 + nz * 10)
    for z in range(nz):
        a = LEVEL + 0x50 + z * 10
        mx = rnd() % 8; mz = rnd() % 8
        h = 3000 if rnd() % 5 == 0 else (rnd() % 4000) - 500
        w16(a, mx); w16(a + 2, mz); w16(a + 4, mx + rnd() % 8); w16(a + 6, mz + rnd() % 8); w16(a + 8, h)
    a = (rnd() % 10) << 5; a |= rnd() & 31; w32(0x803643E0, a)
    a = (rnd() % 10) << 5; a |= rnd() & 31; w32(0x803643E8, a)
    w16(0x80364450, rnd() & 0xFFFF); w16(0x8036444E, 0x1234); w8(0x80364411, 0x55)
    call("func_802A5510", a0=LEVEL)
    out.append("T3 %03d %04X %02X" % (i, r16(0x8036444E), r8(0x80364411)))
# T4: func_802CB690 takes the vehicle block in $gp (tools_port/conventions.txt)
calls = []
hooks[syms["func_80260650"] & 0xFFFFFFFF] = lambda: calls.append("S(%08X,%04X,%08X)" % (
    uc.reg_read(REG["a0"]) & 0xFFFFFFFF, uc.reg_read(REG["a1"]) & 0xFFFF, uc.reg_read(REG["a2"]) & 0xFFFFFFFF))
hooks[syms["func_80278EB0"] & 0xFFFFFFFF] = lambda: calls.append("M(%d,%08X,%d)" % (
    struct.unpack(">i", struct.pack(">I", uc.reg_read(REG["a0"]) & 0xFFFFFFFF))[0],
    uc.reg_read(REG["a1"]) & 0xFFFFFFFF,
    struct.unpack(">i", struct.pack(">I", uc.reg_read(REG["a2"]) & 0xFFFFFFFF))[0]))
rnd = Rng(4)
lo = [0x40, 0x40, 0x40, 0, 0x41, 0x1000]
for i in range(200):
    st = 0x803F8AA0
    hi = 1 if rnd() % 4 == 0 else 0
    w64(0x80364A98, (hi << 32) | lo[rnd() % 6])
    w16(0x80364A72, rnd() & 0xFFFF)
    w32(0x80367738, rnd())
    w16(st + 0x76, rnd() & 0xFFFF)
    w8(0x80367BFF, 1); w8(0x80367C00, 0)
    calls.clear()
    call("func_802CB690", gp=st)
    out.append("T4 %03d %s 76=%04X BFF=%02X C00=%02X" % (i, "".join(calls), r16(st + 0x76), r8(0x80367BFF),
                                                        r8(0x80367C00)))
print("\n".join(out))
