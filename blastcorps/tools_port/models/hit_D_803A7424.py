"""Stand-in for an object-collision pass (func_802BE77C) for eqcheck --model:
it only copies the byte at heap0 (0x80B00000) into the hit flag D_803A7424, so
a check can drive its caller's crash branch both ways:

    --heap 0x10=bytes:0,1 --model func_802BE77C=hit_D_803A7424.py

No call is recorded for a modelled callee (its arguments aren't compared).
"""


def model(args, mem):
    mem.w8(mem.sym("D_803A7424"), mem.u8(0x80B00000))
