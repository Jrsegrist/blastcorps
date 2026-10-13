"""Stand-in for obj_collide_vehicle (the collision pass) that reports a hit: it only
sets the byte D_803A7424 to 1, as the real pass does when the vehicle touches
something. Lets a caller's "hit" branch run without building a collision scene
(wt-v5: func_802B6294 / func_802B5900 bounce path).

    eqcheck.py func_802B6294 --model obj_collide_vehicle=func_802BE77C_hit.py ...
"""


def model(args, mem):
    a = mem.sym("D_803A7424")
    w = a & ~3
    shift = (3 - (a & 3)) * 8
    v = mem.u32(w)
    mem.w32(w, (v & ~(0xFF << shift)) | (1 << shift))
    return []
