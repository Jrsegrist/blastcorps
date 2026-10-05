"""Stand-in for func_802BE77C (vehicle collision pass) that reports a hit.

Sets the hit flag D_803A7424 = 1 and changes nothing else, so a check can
drive a caller's "collided" branch, which a plain stub never reaches. The
vehicle per-frame updates (6E200 func_802B327C / func_802B49AC, 8AEE0, 6C5E0)
clear D_803A7424, call func_802BE77C and test it afterwards. The real pass is
checked on its own in 77E20.

Use: --model func_802BE77C=set_D_803A7424.py        (default entry `model`)
 or: --model func_802BE77C=set_D_803A7424.py:run    (same behaviour)
"""


def model(args, mem):
    mem.w8(mem.sym("D_803A7424"), 1)
    return []


def run(args, mem):
    model(args, mem)
    return 0
