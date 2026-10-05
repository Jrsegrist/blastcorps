"""Test stand-in for an object-hit pass (func_802BE77C and friends) for eqcheck
--model: it only sets the hit flag D_803A7424 to the byte at 0x80B00000 (the
first --heap block), so one check line can drive both the "hit" and "no hit"
paths of a vehicle update that clears the flag, runs the pass and tests it
(func_802CBEF0 / func_802CD068 and other vehicle updates).

    eqcheck.py func_802CBEF0 --heap 0x10 --mem @heap0:1=choice:0,1 \
        --model func_802BE77C=hit_flag_803A7424.py ...
"""


def model(args, mem):
    mem.w8(mem.sym("D_803A7424"), mem.u8(0x80B00000))
    return []
