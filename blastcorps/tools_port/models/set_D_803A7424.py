"""Stand-in for the collision pass func_802BE77C(id, vehicle) in checks of the
vehicle per-frame updates (6E200.c func_802B327C / func_802B49AC): reports a
hit by setting the byte D_803A7424 = 1, so the "restore and bounce" path runs.
It does nothing else (the real pass is checked on its own in 77E20).

    eqcheck.py func_802B327C --model func_802BE77C=set_D_803A7424.py ...
"""


def model(args, mem):
    mem.w8(mem.sym("D_803A7424"), 1)
    return []
