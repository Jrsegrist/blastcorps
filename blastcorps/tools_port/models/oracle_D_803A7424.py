"""Stand-in for func_802BE77C (vehicle collision pass) in checks of the
vehicle per-frame functions (853D0 func_802CA4E0, 7FB50 func_802C5AFC):
the only effect their branches depend on is D_803A7424 ("hit the ring end"),
which the real pass sets deep in its triangle helpers. This model writes
D_803A7424 from an oracle byte the check line randomizes at heap0
(--heap 4 --mem @heap0:1=bytes:0,1), identically in both builds.

    eqcheck.py func_802CA4E0 --model func_802BE77C=oracle_D_803A7424.py --heap 4 --mem @heap0:1=bytes:0,1 ...
"""


def model(args, mem):
    mem.w8(mem.sym("D_803A7424"), mem.u8(0x80B00000))
    return []
