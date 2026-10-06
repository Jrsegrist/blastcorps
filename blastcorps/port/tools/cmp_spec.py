# m64trace spec (tools_port/m64trace/m64trace.py) behind `make -C port compare`:
# the emulator side of the frame-by-frame comparison (port/tools/compare.py).
#
# Run by compare.py, configured through the environment:
#   CMP_DIR    output directory (emu.txt log written by m64trace, frame_NNNNNNN.bin dumps)
#   CMP_ELFS   comma-separated ELFs giving the ROM's addresses (NM test ROM:
#              build_nm/hd_code.rom.us.v11.elf; base ROM: build/hd_code.us.v11.elf)
#   CMP_DUMP   frames to dump: "every:N", "a-b" (every frame of a range), "f1,f2,..."
#              (combinations with '+'), e.g. "every:50+700-760"
#   CMP_VIS    stop after this many VIs
#   CMP_STOP   stop after this many frames (0: no limit)
#
# Logged (one line per event, parsed by compare.py):
#   B vi=V count=C                 hd_code's entry (func_802447C0): boot offset
#   F n vi=V d=D mode=..           frame n: the frame-ending gfx task (BcScTask flag 0x40)
#                                  reaches osSpTaskStartGo -- the point bc_headless dumps at
#                                  (D: counts since VI V)
#   R vi=V d=D                     __scHandleRDP (func_80271904): a frame's RDP work done
#   T ra=RA th=ID v=VALUE          osGetTime returns VALUE to RA (thread ID)
#   C ra=RA th=ID v=VALUE          osGetCount (callers other than osGetTime)
#   A ra=RA th=ID v=VALUE          osAiGetLength (the audio thread sizes its frames by it)
#   U ra=RA th=ID v=WORD           func_802A4B0C's RSP visibility test: the output word it
#                                  tests (0xE8000000 = not visible)
#   M ra=RA th=ID c=COUNT f=FUNC q=A0   a message call (queue A0) / osStartThread (thread
#                                  A0) from RA (running thread ID) at COUNT
#   V (as "#in" lines)             the count register at some VIs (period, wrap-around)
# Dumps: RDRAM 0x80000000-0x80400000 as N64 big-endian bytes.
import ctypes as C, os, struct

from elftools.elf.elffile import ELFFile

OUT = os.environ["CMP_DIR"]
ELFS = os.environ["CMP_ELFS"].split(",")
VIS = int(os.environ.get("CMP_VIS", "36000"))
STOP = int(os.environ.get("CMP_STOP", "0"))


def parse_dumps(s):
    every, frames = 0, set()
    for part in s.split("+"):
        part = part.strip()
        if not part:
            continue
        if part.startswith("every:"):
            every = int(part[6:])
        elif "-" in part:
            a, b = part.split("-")
            frames.update(range(int(a), int(b) + 1))
        else:
            frames.update(int(x) for x in part.split(","))
    return every, frames


DUMP_EVERY, DUMP_FRAMES = parse_dumps(os.environ.get("CMP_DUMP", "every:100"))

syms = {}
code = {}
for path in ELFS:
    with open(path, "rb") as f:
        elf = ELFFile(f)
        for sec in elf.iter_sections():
            if sec.name == ".symtab":
                for s in sec.iter_symbols():
                    if s.name and s["st_info"]["type"] in ("STT_FUNC", "STT_NOTYPE", "STT_OBJECT"):
                        syms.setdefault(s.name, s["st_value"])
        for seg in elf.iter_segments():
            if seg["p_type"] == "PT_LOAD" and seg["p_filesz"]:
                code[seg["p_vaddr"]] = seg.data()


def word_at(addr):
    for base, data in code.items():
        if base <= addr < base + len(data) - 3:
            return struct.unpack(">I", data[addr - base:addr - base + 4])[0]
    raise KeyError(hex(addr))


def ret_of(name):
    """address of the function's (first) jr ra"""
    a = syms[name]
    for i in range(400):
        if word_at(a + 4 * i) == 0x03E00008:
            return a + 4 * i
    raise KeyError(name)


A_BOOT = syms["func_802447C0"]
A_TASK = syms["osSpTaskStartGo"]
A_RDP = syms["func_80271904"]
A_TIME = ret_of("osGetTime")
A_COUNT = ret_of("osGetCount")
A_RUNNING = syms["__osRunningThread"]
A_AILEN = ret_of("osAiGetLength")


def call_site(func, callee):
    """address just after func's (first) jal callee: the call has returned"""
    a, want = syms[func], 0x0C000000 | ((syms[callee] >> 2) & 0x3FFFFFF)
    for i in range(2000):
        if word_at(a + 4 * i) == want:
            return a + 4 * i + 8
    raise KeyError((func, callee))


# func_802A4B0C (5FD50.c): the RSP visibility test; its answer is the first
# word of the output buffer D_803BEB80 once func_80285110 has waited for it
A_CULL = call_site("func_802A4B0C", "func_80285110")
A_CULLBUF = syms.get("D_803BEB80", 0x803BEB80)
# thread switch points: the native clock catches up with the emulator's here
SYNC = {syms[n]: n for n in ("osSendMesg", "osRecvMesg", "osJamMesg", "osStartThread")}

BPS = {A_BOOT: "B", A_TASK: "F", A_RDP: "R", A_TIME: "T", A_COUNT: "C", A_AILEN: "A", A_CULL: "U"}
BPS.update({a: "M" for a in SYNC})
GPRS, FPRS, MEM = [], [], []
DEDUPE = False
DEFMAX = 1 << 40

core = C.CDLL("/usr/lib/x86_64-linux-gnu/libmupen64plus.so.2")
core.DebugGetCPUDataPtr.restype = C.c_void_p
core.DebugGetCPUDataPtr.argtypes = [C.c_int]
core.DebugMemGetPointer.restype = C.c_void_p
core.DebugMemGetPointer.argtypes = [C.c_int]

st = {"vi": 0, "vi_count": 0, "frame": 0}
os.makedirs(OUT, exist_ok=True)


def count():
    p = core.DebugGetCPUDataPtr(5)  # M64P_CPU_REG_COP0: u32 regs
    return C.cast(p, C.POINTER(C.c_uint32))[9]


def frac():
    """counts since the last VI (compare.py divides by the measured VI period)"""
    return "%d" % ((count() - st["vi_count"]) & 0xFFFFFFFF)


def dump(n):
    p = core.DebugMemGetPointer(1)  # M64P_DBG_PTR_RDRAM: host-order u32 words
    raw = bytearray(C.string_at(p, 0x400000))
    import array
    a = array.array("I", bytes(raw))
    a.byteswap()  # -> big-endian byte image (on a little-endian host)
    with open(os.path.join(OUT, "frame_%07d.bin" % n), "wb") as f:
        f.write(a.tobytes())


def thread_id(rd):
    t = rd(A_RUNNING, 4)
    return rd(t + 0x14, 4) if 0x80000000 <= t < 0x80800000 else -1


def ONHIT(pc, g, rd):
    if pc == A_TASK:
        t = g["a0"]
        if not (rd(t - 0x10 + 0x08, 4) & 0x40):
            return None
        st["frame"] += 1
        n = st["frame"]
        if (DUMP_EVERY and n % DUMP_EVERY == 0) or n in DUMP_FRAMES:
            dump(n)
        if STOP and n >= STOP:
            core.CoreDoCommand(6, 0, None)
        return "F %d vi=%d d=%s mode=%08x%08x lvl=%x mf=%x gvi=%x demo=%x" % (
            n, st["vi"], frac(), rd(0x80364A90, 4), rd(0x80364A94, 4), rd(0x802E8BDC, 4), rd(0x80358060, 4),
            rd(0x803156C4, 4), rd(0x802E8BEC, 4))
    if pc == A_RDP:
        return "R vi=%d d=%s" % (st["vi"], frac())
    if pc == A_TIME:
        return "T ra=%x th=%d v=%x" % (g["ra"], thread_id(rd), (g["v0"] << 32) | g["v1"])
    if pc == A_COUNT:
        if syms["osGetTime"] <= g["ra"] < A_TIME:
            return None
        return "C ra=%x th=%d v=%x" % (g["ra"], thread_id(rd), g["v0"])
    if pc == A_CULL:
        return "U ra=%x th=%d v=%x" % (syms["func_802A4B0C"], thread_id(rd), rd(A_CULLBUF, 4))
    if pc == A_AILEN:
        return "A ra=%x th=%d v=%x" % (g["ra"], thread_id(rd), g["v0"])
    if pc in SYNC:
        return "M ra=%x th=%d c=%d f=%s q=%x" % (g["ra"], thread_id(rd), count(), SYNC[pc], g["a0"])
    if pc == A_BOOT:
        return "B vi=%d count=%d vicount=%d" % (st["vi"], count(), st["vi_count"])
    return None


def INPUT(vi, rd, ctl):
    st["vi"] = vi
    st["vi_count"] = count()
    if vi <= 3 or vi % 600 == 0:
        ctl.log("V %d count=%d" % (vi, st["vi_count"]))
    return 0, 0, 0
