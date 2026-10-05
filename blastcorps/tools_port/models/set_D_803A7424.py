"""Stand-in for func_802BE77C (vehicle collision pass) that reports a hit.

Sets the hit flag D_803A7424 = 1 and changes nothing else, so a check can
drive a caller's "collided" branch (the vehicle per-frame updates in 8AEE0 /
6C5E0 clear D_803A7424, call func_802BE77C and test it afterwards), which a
plain stub never reaches. Use: --model func_802BE77C=set_D_803A7424.py:run
"""


def run(args, mem):
    mem.w8(mem.sym("D_803A7424"), 1)
    return 0
