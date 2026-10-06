#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_80364460 ((u8 *) D_80364460)
#ifdef NON_MATCHING
#define D_803ED390 (*(s16 *) D_803ED390)
#define D_803ED3A8 (*(s32 *) D_803ED3A8)
#endif
/* end of views */

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Port-phase rewrites (functional, not matching). Register conventions of the
 * hand asm are in tools_port/conventions.txt.
 *
 * Vehicle record: much of this file's asm reaches the current vehicle's state
 * through $gp, which the vehicle code points at that vehicle's record
 * (D_803EDB40, D_803EDF10, D_803EE2E0, ... - one per vehicle, so it is not a
 * fixed global). The C versions take it as an explicit `u8 *veh` argument
 * (convention `in gp=a0`). Known fields:
 *   0x00 f32, 0x28..0x48 s32 x9, 0x4C u16 heading (12-bit angle),
 *   0x4E u16 target heading, 0x50 u8 gear-ish index, 0x76 s16 signed speed,
 *   0x78 5 x {s16 lo, s16 hi, s16 value} band table, 0x96..0xA5 u8 flags
 *   (0x9F mode byte, 0xA5 s8 direction). The record is 0xA6 bytes long as far
 *   as func_802A75DC/func_802A768C (save/restore) are concerned. */
#define VEH_U8(v, off) (*(u8 *) ((u8 *) (v) + (off)))
#define VEH_S8(v, off) (*(s8 *) ((u8 *) (v) + (off)))
#define VEH_U16(v, off) (*(u16 *) ((u8 *) (v) + (off)))
#define VEH_S16(v, off) (*(s16 *) ((u8 *) (v) + (off)))
#define VEH_S32(v, off) (*(s32 *) ((u8 *) (v) + (off)))
#define VEH_F32(v, off) (*(f32 *) ((u8 *) (v) + (off)))

extern s32 D_80305C58[]; /* {key, value, threshold} triples, value 0 ends */
extern u8 D_803EB7A0[];  /* vehicle save buffer: 0x300 + 0xA6 + 3 words */
extern s32 D_803EBBD8[]; /* 0x14-byte swap temp */
extern u8 *D_803EBC00;
extern u8 *D_803EBC04;
extern u8 *D_803EBC08;
extern s32 D_803ED398;
extern s32 D_803ED39C;
extern s32 D_803ED3A0;
extern s32 D_803ED3AC;
extern s32 D_803ED3B0;
extern u8 D_803ED3EA;
extern u8 D_803ED3EB;
extern u8 D_803ED3EE;
extern u8 D_803ED3EF;
extern u8 D_803ED3F2;
extern u8 D_803ED410;


/* Prototypes shared by the ground-height functions (func_802A92C8 ..
 * func_802A9A60, func_802A8768); their definitions are further down. */
s32 func_802A9540(s32 index, s32 *a, s32 *b, s32 *c, s32 extra, s32 value);
s32 func_802A9514(s32 x);
s32 func_802A9710(s32 index, s32 *a, s32 *b, s32 *c, s32 extra, s32 pos, s32 s3);
/* func_802A8CCC's t0 / t1: position in, reset position out (unchanged if none). */
typedef struct {
    s32 t0; /* x */
    s32 t1; /* z */
} Pos802A8CCC;

#define D_803ED394_ ((&D_803ED390)[2])

/* Find the D_803ED3B8 record keyed `key` (or claim the end marker for it:
 * byte 0 = key, the -1 word otherwise left) and copy the three slot kinds
 * D_803ED3EA[0..2] into its bytes 1..3. The key compare is against the full
 * register (a key above 0xFF never matches); 0xFF bytes that aren't the end
 * marker are skipped. Shared by func_802A92C8 and func_802A992C. */
#define PORT_STORE_KINDS(key)                                                       \
    {                                                                               \
        u8 *p_ = D_803ED3B8;                                                        \
        for (;;) {                                                                  \
            if (p_[0] == (key)) {                                                   \
                break;                                                              \
            }                                                                       \
            if (p_[0] == 0xFF && *(s32 *) p_ == -1) {                               \
                p_[0] = (key);                                                      \
                break;                                                              \
            }                                                                       \
            p_ += 4;                                                                \
        }                                                                           \
        p_[1] = (&D_803ED3EA)[0];                                                   \
        p_[2] = (&D_803ED3EA)[1];                                                   \
        p_[3] = (&D_803ED3EA)[2];                                                   \
    }

/* cvt.w.s under the game's FCSR: float -> s32, round to nearest even (a C
 * cast truncates). NaN and out-of-range inputs (degenerate geometry) give
 * 0x7FFFFFFF, the invalid-operation result the emulator produces (the VR4300
 * itself raises an unimplemented-operation exception there). */
static s32 port_cvt_w_s(f32 x) {
    s32 t;
    f32 frac;

    if (!(x >= -2147483648.0f && x < 2147483648.0f)) {
        return 0x7FFFFFFF;
    }
    t = (s32) x;
    frac = x - (f32) t;

    if (frac > 0.5f || (frac == 0.5f && (t & 1))) {
        t++;
    } else if (frac < -0.5f || (frac == -0.5f && (t & 1))) {
        t--;
    }
    return t;
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* If D_80367C10 is set, double both bounds of each of the 5 band-table
 * entries at veh+0x78. (The asm saves v0/v1/a0; asm callers keep f12/f14.) */
void func_802A6F00(u8 *veh) {
    s16 *band;
    s32 i;

    if (D_80367C10 != 0) {
        band = &VEH_S16(veh, 0x78);
        for (i = 5; i != 0; i--) {
            band[0] = band[0] << 1;
            band[1] = band[1] << 1;
            band += 3;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6F00.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (D_803A7410/D_803A7412 are declared at the top of the file.) */

/* Midpoint between ring indices A and B, going forward from A (wrapping
 * through 0xFFF when B < A). Returns s32 because the asm leaves the full
 * value in v0 (C callers declare s16; values are < 0x1000 for 12-bit input).
 * Register note: the asm saves/restores v1 and a0 and touches nothing but
 * v0/at. Its asm callers rely on that: func_802A71DC keeps a1 live, and the
 * vehicle update functions (func_802AEEC8, func_802B327C, func_802B6294,
 * func_802CFDE8, ...) keep a3, t6, f12 and f14 live across the call. This C
 * version is plain o32, so a mixed N64 build would need a thunk preserving
 * those; the native port does not. */
s32 func_802A6F6C(void) {
    s32 a = ((u16) D_803A7410);
    s32 b = ((u16) D_803A7412);
    s32 mid;

    if (b < a) {
        mid = (((0xFFF - a) + b) >> 1) + a;
        if (mid >= 0x1000) {
            mid -= 0xFFF;
        }
        return mid;
    }
    return (u32) (a + b) >> 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6F6C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* When D_80358064 is set: record the speed's direction (veh+0x76 > 0 ? 1 : -1)
 * in veh+0xA5, then move the speed 3 toward zero, but forward speed not below
 * `limit` and reverse speed not above -limit. (The asm computes -limit with a
 * trapping `neg`, so limit = 0x80000000 raises an overflow exception there.)
 * Asm callers keep a0, a3, t6, f12 and f14 live across the call (the asm
 * saves a0/a2/v1). */
void func_802A6FE4(u8 *veh, s32 limit) {
    s32 speed;

    if (D_80358064 != 0) {
        speed = VEH_S16(veh, 0x76);
        VEH_S8(veh, 0xA5) = (speed > 0) ? 1 : -1;
        if (speed >= 0) {
            speed -= 3;
            if (!(limit < speed)) {
                speed = limit;
            }
        } else {
            speed += 3;
            if (!(speed < -limit)) {
                speed = -limit;
            }
        }
        VEH_S16(veh, 0x76) = speed;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6FE4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Target heading (veh+0x4E) = *angle when the stored direction (veh+0xA5)
 * still matches the speed's sign, else *angle turned half a circle (-0x800,
 * wrapping by +0xFFF). Asm callers keep a0-a3, t6 and f12 live (the asm saves
 * v0/v1). */
void func_802A7070(u8 *veh, s16 *angle) {
    s32 dir = (VEH_S16(veh, 0x76) > 0) ? 1 : -1;
    s32 target;

    if (VEH_S8(veh, 0xA5) == dir) {
        target = *angle;
    } else {
        target = *angle - 0x800;
        if (target < 0) {
            target += 0xFFF;
        }
    }
    VEH_U16(veh, 0x4E) = target;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7070.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* If the target heading (veh+0x4E) is outside the +-0x400 window around the
 * heading (veh+0x4C) - tested as written for the wrapped window (lo > hi) -
 * turn the target half a circle, negate the speed and move ring index A back
 * and B forward by 0x800 (mod 0xFFF). Asm callers keep a3, t6, f12 and f14
 * live (the asm saves v0, v1, a0, a1). */
void func_802A70D8(u8 *veh) {
    s32 heading = VEH_U16(veh, 0x4C);
    s32 lo = heading - 0x400;
    s32 hi = heading + 0x400;
    s32 t;

    if (lo < 0) {
        lo += 0xFFF;
    }
    if (hi >= 0x1000) {
        hi -= 0xFFF;
    }
    t = VEH_U16(veh, 0x4E);
    if (hi < lo) {
        if (!(t < lo && hi < t)) {
            return;
        }
    } else if (!(t < lo || hi < t)) {
        return;
    }
    t += 0x800;
    if (t >= 0x1000) {
        t -= 0xFFF;
    }
    VEH_U16(veh, 0x4E) = t;
    VEH_S16(veh, 0x76) = -VEH_S16(veh, 0x76);
    t = ((u16) D_803A7410) - 0x800;
    if (t < 0) {
        t += 0xFFF;
    }
    D_803A7410 = t;
    t = ((u16) D_803A7412) + 0x800;
    if (0xFFF < t) {
        t -= 0xFFF;
    }
    D_803A7412 = t;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A70D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Steering toward the ring: lo = A + 0x78 and hi = B - 0x78 (ring indices
 * D_803A7410/D_803A7412, wrapped by 0x1000) bound a window. If `target` is in
 * the "stop" region of that window (tested as the asm does for the wrapped
 * and unwrapped cases), the new index is `target` and the turn is 0.
 * Otherwise, if `cur` is outside the window, pull it to the nearer end (A +
 * 0x78 or B - 0x78, wrapped by 0xFFF) and, if that lands outside A..B, use
 * the ring midpoint (func_802A6F6C). The turn is round(scale * |speed|)
 * (veh+0x76), signed toward `target` the short way round (|cur - target| <=
 * 0x800 decides).
 * Register convention: gp, cur in a0, target in a1, scale in f0; outputs the
 * new index in a0 (*curOut) and the turn in a1 (return value). The asm saves
 * v0, v1, a2, a3 and t0-t2 and keeps f0 live across func_802A6F6C; asm
 * callers keep a3, t6, f12 and f14 live (a mixed N64 build would need a
 * thunk). */
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale) {
    s32 a = ((u16) D_803A7410);
    s32 b = ((u16) D_803A7412);
    s32 lo;
    s32 hi;
    s32 da;
    s32 db;
    s32 speed;
    s32 turn;
    s32 d;
    s32 pull;

    if (b < a) {
        lo = a + 0x78;
        if (lo >= 0x1000) {
            lo -= 0x1000;
        }
        hi = b - 0x78;
        if (hi < 0) {
            hi += 0x1000;
        }
        if (hi < lo && !(target < lo && hi < target)) {
            *curOut = target;
            return 0;
        }
        pull = !(hi < lo) || (cur < lo && hi < cur);
    } else {
        lo = a + 0x78;
        hi = b - 0x78;
        if (lo < hi && !(target < lo) && !(hi < target)) {
            *curOut = target;
            return 0;
        }
        pull = !(lo < hi) || cur < lo || hi < cur;
    }
    if (pull) {
        da = a - cur;
        if (da < 0) {
            da = -da;
        }
        if (da > 0x800) {
            da = 0xFFF - da;
        }
        db = b - cur;
        if (db < 0) {
            db = -db;
        }
        if (db > 0x800) {
            db = 0xFFF - db;
        }
        if (db < da) {
            cur = b - 0x78;
            if (cur < 0) {
                cur += 0xFFF;
            }
        } else {
            cur = a + 0x78;
            if (cur >= 0x1000) {
                cur -= 0xFFF;
            }
        }
        if (b < a) {
            if (cur < a && b < cur) {
                cur = func_802A6F6C();
            }
        } else if (cur < a || b < cur) {
            cur = func_802A6F6C();
        }
    }
    speed = VEH_S16(veh, 0x76);
    if (speed < 0) {
        speed = -speed;
    }
    turn = port_cvt_w_s(scale * (f32) speed);
    *curOut = cur;
    d = cur - target;
    if (cur < target) {
        if (d < 0) {
            d = -d;
        }
        return (d <= 0x800) ? -turn : turn;
    }
    return (d <= 0x800) ? turn : -turn;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A71DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Turn the heading (veh+0x4C) by `delta` toward the target (veh+0x4E),
 * wrapping mod 0xFFF, and snap to the target if the step passes it.
 * Register convention: the asm returns the stepped heading in v1 (left as the
 * caller's v1 when heading == target) and the target in a0; the C takes the
 * caller's v1 as `v1`, returns v1 and stores the target through `targetOut`.
 * (The asm also leaves the old heading in v0.) Asm callers keep a3, t6, f12
 * and f14 live. */
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut) {
    s32 heading = VEH_U16(veh, 0x4C);
    s32 target = VEH_U16(veh, 0x4E);
    s32 next;
    s32 snap;

    *targetOut = target;
    if (heading == target) {
        return v1;
    }
    next = heading + delta;
    if (next < 0) {
        next += 0xFFF;
    }
    if (next >= 0x1000) {
        next -= 0xFFF;
    }
    if (delta >= 0) {
        if (heading < target) {
            snap = (target < next) || (next < heading);
        } else {
            snap = (target < heading) && (next < heading) && (target < next);
        }
    } else {
        if (target < heading) {
            snap = !(target < next) || !(next < heading);
        } else {
            snap = (heading < target) && (next < target) && (heading < next);
        }
    }
    VEH_U16(veh, 0x4C) = snap ? target : next;
    return next;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A746C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Reset a vehicle record: clear the f32 at +0, the words at +0x28..+0x48, the
 * speed and most flag bytes; set +0x50 and +0xA0 to 1. Asm callers keep a1,
 * a3, t0, t1, t6, t7, f12 and f14 live (the asm saves t1; it zeroes f0). */
void func_802A754C(u8 *veh) {
    VEH_U8(veh, 0x96) = 0;
    VEH_U8(veh, 0x97) = 0;
    VEH_U8(veh, 0x98) = 0;
    VEH_U8(veh, 0x50) = 1;
    VEH_U8(veh, 0xA0) = 1;
    VEH_S32(veh, 0x28) = 0;
    VEH_S32(veh, 0x2C) = 0;
    VEH_S32(veh, 0x30) = 0;
    VEH_S32(veh, 0x34) = 0;
    VEH_S32(veh, 0x38) = 0;
    VEH_S32(veh, 0x3C) = 0;
    VEH_S32(veh, 0x40) = 0;
    VEH_S32(veh, 0x44) = 0;
    VEH_S32(veh, 0x48) = 0;
    VEH_S16(veh, 0x76) = 0;
    VEH_U8(veh, 0x99) = 0;
    VEH_U8(veh, 0x9B) = 0;
    VEH_U8(veh, 0x9C) = 0;
    VEH_U8(veh, 0x9D) = 0;
    VEH_U8(veh, 0x9E) = 0;
    VEH_U8(veh, 0xA1) = 0;
    VEH_U8(veh, 0xA2) = 0;
    VEH_U8(veh, 0xA3) = 0;
    VEH_U8(veh, 0xA4) = 0;
    VEH_U8(veh, 0xA5) = 0;
    VEH_F32(veh, 0x00) = 0.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A754C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Save a vehicle: copy 0x300 bytes from `src` and the 0xA6-byte record into
 * D_803EB7A0, followed by the words *w0, *w1, *w2 (unaligned, at +0x3A6).
 * Register convention: asm inputs gp, v0, v1, a0, a1 (see conventions.txt);
 * the asm restores every register it touches. Asm callers keep a0-a3, t6,
 * t7, f12 and f14 live. */
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2) {
    u8 *dst = D_803EB7A0;
    s32 i;
    s32 w;

    for (i = 0x300; i != 0; i--) {
        *dst++ = *src++;
    }
    for (i = 0xA6; i != 0; i--) {
        *dst++ = *veh++;
    }
    w = *w0;
    dst[0] = w >> 24;
    dst[1] = w >> 16;
    dst[2] = w >> 8;
    dst[3] = w;
    w = *w1;
    dst[4] = w >> 24;
    dst[5] = w >> 16;
    dst[6] = w >> 8;
    dst[7] = w;
    w = *w2;
    dst[8] = w >> 24;
    dst[9] = w >> 16;
    dst[10] = w >> 8;
    dst[11] = w;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A75DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Restore what func_802A75DC saved: 0x300 bytes from D_803EB7A0 to `dst`, the
 * 0xA6-byte record to `veh`, then copy `size` bytes (8-byte units, size a
 * multiple of 8: the asm loops on size != 0) from src to dst2, then the three
 * saved words to *w0, *w1, *w2.
 * Register convention: asm inputs gp, v0, v1, a0, a1, a2 (src), a3 (dst2),
 * t0 (size); the asm returns the advanced src/dst2 in a2/a3 - here the return
 * value and *dst2End. Asm callers keep a0, a1, f12 and f14 live. */
u64 *func_802A768C(u8 *veh, u8 *dst, s32 *w0, s32 *w1, s32 *w2, u64 *src, u64 *dst2, s32 size,
                   u64 **dst2End) {
    u8 *buf = D_803EB7A0;
    u64 *s = src; /* locals, so the stack-passed parameters stay untouched */
    u64 *d = dst2;
    s32 n = size;
    s32 i;

    for (i = 0x300; i != 0; i--) {
        *dst++ = *buf++;
    }
    for (i = 0xA6; i != 0; i--) {
        *veh++ = *buf++;
    }
    while (n != 0) {
        n -= 8;
        *d++ = *s++;
    }
    *w0 = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    *w1 = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
    *w2 = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];
    *dst2End = d;
    return s;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A768C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Copy `size` bytes in 8-byte units from a to b, or from b to a when
 * D_8035805C (the frame buffer index) is nonzero. Both pointers must be
 * 8-aligned and size a multiple of 8 (the asm loops on `size != 0`).
 * Register note: the asm saves/restores a0-a3, and its asm callers
 * (func_802B0254, func_802C5120, func_802D07E0, ...) keep a0-a3, f12 and f14
 * live across the call; a mixed N64 build would need a thunk, the native
 * port does not. */
void func_802A7764(u64 *a, u64 *b, s32 size) {
    u64 *src = a;
    u64 *dst = b;

    if (D_8035805C != 0) {
        src = b;
        dst = a;
    }
    while (size != 0) {
        *dst++ = *src++;
        size -= 8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7764.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* With no steering input (D_80370C2D == 0), brake the speed (veh+0x76) by 8
 * toward zero, stopping at zero. Asm callers keep a0-a3 and t6 live (the asm
 * saves s7). */
void func_802A77D0(u8 *veh) {
    s32 speed;

    if (D_80370C2D == 0) {
        speed = VEH_S16(veh, 0x76);
        if (speed >= 0) {
            speed -= 8;
            if (speed < 0) {
                speed = 0;
            }
        } else {
            speed += 8;
            if (speed > 0) {
                speed = 0;
            }
        }
        VEH_S16(veh, 0x76) = speed;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A77D0.s")
#endif

/* func_802A7834: two sequential no-arg calls (`func_802A7E70();
 * func_802A785C();`) using the 8-byte addiu-sp/sd-ra/ld-ra frame instead
 * of the 24-byte o32 shadow frame plain C produces for any real call -
 * same confirmed-unreachable-from-C frame family as func_802AC284 in
 * hd_code/679E0.c (see that file's comment for the probe evidence).
 * Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Steering then throttle for one vehicle: func_802A7E70(rate, angle), then
 * func_802A785C(veh, speed, mode, flags, bands, delta).
 * Register convention: rate s3, angle s4 (func_802A7E70's), veh gp, speed t6,
 * mode t7, flags s0, bands s1, delta s2 (func_802A785C's); s1 and s3 are
 * clobbered by the callees. Asm callers (func_802AEEC8, func_802B03F4,
 * func_802B6294) keep t6 live across the call (a mixed N64 build would need
 * a thunk; the native port doesn't). */
void func_802A7834(s32 rate, u16 *angle, u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta) {
    func_802A7E70(rate, angle);
    func_802A785C(veh, speed, mode, flags, bands, delta);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7834.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A7A1C(s32 x, s16 *p);
s32 func_802A7AAC(s32 x, s16 *p);
s32 func_802A7C28(u8 *veh, s32 x, s16 *band);
s32 func_802A7D68(s32 mode, u8 *f);

/* Throttle/brake step for the speed *speed (s16). With D_80370C22 set: if
 * D_803ED400 is nonzero, brake by `delta` toward 0 (by the speed's sign).
 * Otherwise: with D_803ED40C set (and D_802E8BDC != 0x22, D_80364456 not
 * 0xB/0x11/0x12) clamp the speed to half the top band bound (bands[13]) or,
 * in reverse, half bands[0]; then with steer input D_80370C2D > 0 accelerate
 * (or brake by `delta` when rolling backward) and with < 0 the mirror image:
 * the step is the band value func_802A7C28 finds times
 * func_802A7D68(mode, flags), or -6 going forward with no band; nothing when
 * the speed is already past the limit (func_802A7AAC / func_802A7A1C). The
 * result is copied to D_803ED400.
 * Register convention: gp, speed in t6, mode in t7, flags in s0, bands in s1
 * (clobbered by func_802A7C28), delta in s2. Asm callers keep a0-a3 and t6
 * live (a mixed N64 build would need a thunk). */
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta) {
    s32 s;
    s32 lim;
    s32 mult;
    s32 r;

    if (D_80370C22 != 0) {
        if (D_803ED400 != 0) {
            s = *speed;
            if (s > 0) {
                s -= delta;
                if (s <= 0) {
                    s = 0;
                }
            } else {
                s += delta;
                if (s >= 0) {
                    s = 0;
                }
            }
            *speed = s;
        }
    } else {
        if (((s8) D_803ED40C) != 0 && D_802E8BDC != 0x22 && D_80364456 != 0xB && D_80364456 != 0x11 &&
            D_80364456 != 0x12) {
            s = *speed;
            if (s >= 0) {
                lim = bands[13] >> 1;
                if (lim < s) {
                    *speed = lim;
                }
            } else {
                lim = bands[0] >> 1;
                if (s < lim) {
                    *speed = lim;
                }
            }
        }
        mult = func_802A7D68(mode, flags);
        if (D_80370C2D != 0) {
            s = *speed;
            if (D_80370C2D > 0) {
                if (s < 0) {
                    *speed = s + delta;
                } else if (func_802A7AAC(s, bands) == 0) {
                    r = func_802A7C28(veh, s, bands);
                    if (r != 0) {
                        *speed = s + r * mult;
                    } else {
                        *speed = s - 6;
                    }
                }
            } else {
                if (s > 0) {
                    *speed = s - delta;
                } else if (func_802A7A1C(s, bands) == 0) {
                    r = func_802A7C28(veh, s, bands);
                    *speed = s - r * mult;
                }
            }
        }
    }
    D_803ED400 = *speed;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A785C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 1 if x < (p[0] * steer) / -80 (steer = D_80370C2D), else 0.
 * Register convention: x in t2, p in s1, result in t4 (asm saves v0, v1, a0).
 * Asm callers keep t2, t3 and t6 live. */
s32 func_802A7A1C(s32 x, s16 *p) {
    return x < (p[0] * D_80370C2D) / -80;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7A1C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 1 if (p[13] * steer) / 80 < x (p[13] is the halfword at +0x1A), else 0.
 * Register convention: x in t2, p in s1, result in t4 (asm saves v0, v1, a0).
 * Asm callers keep t2, t3 and t6 live. */
s32 func_802A7AAC(s32 x, s16 *p) {
    return (p[13] * D_80370C2D) / 80 < x;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7AAC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Variant of func_802A785C's speed step (no callers found): with D_80370C22
 * set and D_803ED400 nonzero, brake *speed by `delta` toward 0 by the sign of
 * D_803ED400; otherwise, if D_80370C1A or D_80370C1B is set, accelerate: a
 * negative speed by `delta`, else by 4 * the band value func_802A7C28 finds
 * (or -6 with none). The result is copied to D_803ED400.
 * Register convention: gp, speed in t6, bands in s1 (clobbered by
 * func_802A7C28), delta in s2. */
void func_802A7B3C(u8 *veh, s16 *speed, s16 *bands, s32 delta) {
    s32 s;
    s32 d;
    s32 r;

    if (D_80370C22 != 0) {
        d = D_803ED400;
        if (d != 0) {
            s = *speed;
            if (d > 0) {
                s -= delta;
                if (s <= 0) {
                    s = 0;
                }
            } else {
                s += delta;
                if (s >= 0) {
                    s = 0;
                }
            }
            *speed = s;
        }
    } else if (D_80370C1A != 0 || D_80370C1B != 0) {
        s = *speed;
        if (s < 0) {
            *speed = s + delta;
        } else {
            r = func_802A7C28(veh, s, bands);
            if (r != 0) {
                *speed = s + r * 4;
            } else {
                *speed = s - 6;
            }
        }
    }
    D_803ED400 = *speed;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7B3C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Search n {s16 lo, s16 hi, s16 value} bands (n = 5 when veh+0x50 == 1, else
 * 7 - veh+0x50; 2 fewer when D_803ED40C is set) for lo <= x <= hi and return
 * that band's value, or 0. n must come out >= 0 (the asm loops on n != 0).
 * Register convention: gp, x in t2, bands in s1 (advanced: s1 is clobbered),
 * result in t4. Asm callers keep t2, t3 and t6 live (the asm saves t1). */
s32 func_802A7C28(u8 *veh, s32 x, s16 *band) {
    s32 n = VEH_U8(veh, 0x50);

    if (n == 1) {
        n = 5;
    } else {
        n = 7 - n;
    }
    if (((s8) D_803ED40C) != 0) {
        n -= 2;
    }
    while (n != 0) {
        n--;
        band += 3;
        if (x < band[-3]) {
            continue;
        }
        if (band[-2] < x) {
            continue;
        }
        return band[-1];
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7C28.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 1 if the speed (veh+0x76) is within `range` of a band bound: going forward,
 * the hi bound of band (veh+0x50 == 1 ? 4 : 6 - veh+0x50); in reverse, the lo
 * bound of band 0. Register convention: gp, range in v1, result in v0 (the asm
 * saves t1, t2, t4). Asm callers keep a1-a3, f12 and f14 live. */
s32 func_802A7CB0(u8 *veh, s32 range) {
    s32 speed = VEH_S16(veh, 0x76);
    s32 i;
    s32 d;

    if (speed >= 0) {
        i = VEH_U8(veh, 0x50);
        if (i == 1) {
            i = 4;
        } else {
            i = 6 - i;
        }
        d = speed - *(s16 *) (veh + 0x78 + i * 6 + 2);
    } else {
        d = speed - VEH_S16(veh, 0x78);
    }
    if (d < 0) {
        d = -d;
    }
    return d < range;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7CB0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Classify the three flag bytes f[0..2] by `mode`:
 *   0: f[0], f[1] both set -> 0, one set -> 2, neither -> 4
 *   1: f[2] set -> 0, else 4
 *   2: (f[0] == 0) + (f[1] == 0) + 2 * (f[2] == 0)
 *   4: 4
 *   other: 4 if any of f[0..2] is 0, else 0
 * Register convention: mode in t7, f in s0, result in t3 (the asm saves t1 and
 * s0; it clobbers t2 in the default case). Asm callers keep t6 live. */
s32 func_802A7D68(s32 mode, u8 *f) {
    s32 n;
    s32 i;

    switch (mode) {
        case 0:
            if (f[0] != 0) {
                return (f[1] != 0) ? 0 : 2;
            }
            return (f[1] != 0) ? 2 : 4;
        case 1:
            return (f[2] != 0) ? 0 : 4;
        case 2:
            n = 0;
            if (f[0] == 0) {
                n++;
            }
            if (f[1] == 0) {
                n++;
            }
            if (f[2] == 0) {
                n += 2;
            }
            return n;
        case 4:
            return 4;
        default:
            for (i = 0; i < 3; i++) {
                if (f[i] == 0) {
                    return 4;
                }
            }
            return 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7D68.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Steering: turn rate = s3 (minus a third when D_80367C10; forced to 100/200
 * when D_80370C75, by D_80370C34), saved to D_80370C72; then turn *angle by
 * |steer| * rate / 80 against the sign of steer (D_80370C2C), wrapped to
 * 0..0xFFF. When D_803ED40A is set and D_803649EE isn't, *angle is overridden
 * by D_803ED408. The result also goes to D_80370C70.
 * Register convention: rate in s3 (clobbered), angle in s4 (the asm saves s5;
 * it clobbers t0, t2, t4). Asm callers keep a0-a3, t6 and t7 live. */
void func_802A7E70(s32 rate, u16 *angle) {
    s32 steer;
    s32 a;
    s32 mag;

    if (D_80367C10 != 0) {
        rate -= rate / 3;
    }
    if (D_80370C75 != 0) {
        rate = (D_80370C34 != 0) ? 100 : 200;
    }
    D_80370C72 = rate;
    steer = D_80370C2C;
    a = *angle;
    mag = (steer < 0) ? -steer : steer;
    rate = (mag * rate) / 80;
    if (steer != 0) {
        if (steer < 0) {
            a += rate;
        } else {
            a -= rate;
        }
        if (a < 0) {
            a += 0x1000;
        }
        if (a >= 0x1000) {
            a -= 0x1000;
        }
        *angle = a;
    }
    if (D_803ED40A != 0 && D_803649EE == 0) {
        a = D_803ED408;
    }
    *angle = a;
    D_80370C70 = a;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7E70.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A8314(s32 cur);
extern u8 D_803ED3F8;  /* sound retrigger counter */
extern s32 D_803ED3FC; /* f32 bits: pitch */
extern f32 D_8030D890;

/* Turn the heading *heading toward *target (12-bit angles, wrapping through
 * 0x1000, snapping to the target when the step passes it). The step is
 * |func_802A8314(rate) * veh+0x50 / *speed| (no division when the speed is
 * 0). Unless *flag is set, a heading within 0x156 of the target just copies
 * the target to *out and stops; otherwise *flag = 1. With `sound` set, every
 * 7th call sets pitch D_803ED3FC = 0.5 + |speed| * D_8030D890 and plays it
 * (func_80260650 / func_80260AB8). Finally *out = heading, and *flag = 0 once
 * the target is reached.
 * Register convention: gp, heading t1, rate t5, speed t6, target s4, out s5,
 * flag s6, sound s7 (the asm saves t6 and, around the sound calls, every
 * register). Asm callers keep a0-a3 and t6 live. (0x80000000 / -1 traps in
 * the asm's divide.) */
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag,
                   s32 sound) {
    s32 speed;
    s32 h;
    s32 t;
    s32 d;
    s32 n;
    f32 pitch;

    rate = func_802A8314(rate);
    speed = *speedp;
    h = *heading;
    t = *target;
    if (speed != 0) {
        rate = (s32) (rate * VEH_U8(veh, 0x50)) / speed;
    }
    d = h - t;
    if (rate < 0) {
        rate = -rate;
    }
    if (d < 0) {
        d = -d;
    }
    if (d > 0x800) {
        if (t < h) {
            h += rate;
            if (h >= 0x1000) {
                h -= 0x1000;
                if (t < h) {
                    h = t;
                }
            }
        } else {
            h -= rate;
            if (h < 0) {
                h += 0x1000;
                if (h < t) {
                    h = t;
                }
            }
        }
    } else if (h < t) {
        h += rate;
        if (h >= 0x1000 || t < h) {
            h = t;
        }
    } else {
        h -= rate;
        if (h < 0 || h < t) {
            h = t;
        }
    }
    *heading = h;
    if (*flag == 0) {
        d = h - t;
        if (d < 0) {
            d = -d;
        }
        if (d > 0x800) {
            d = 0x1000 - d;
        }
        if (d < 0x156) {
            *out = t;
            return;
        }
        *flag = 1;
    }
    if (sound != 0) {
        n = D_803ED3F8 + 1;
        if (n >= 7) {
            if (speed < 0) {
                speed = -speed;
            }
            D_803ED3F8 = 0;
            pitch = 0.5f + (f32) speed * D_8030D890;
            D_803ED3FC = *(s32 *) &pitch;
            func_80260AB8(func_80260650(D_80367738, 10, NULL), 0x10, D_803ED3FC);
            n = 0;
        }
        D_803ED3F8 = n;
    }
    *out = h;
    if (h == t) {
        *flag = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7FD8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unless D_803A740C + 10 < D_80358068, walk the {key, value, threshold}
 * table D_80305C58 (ended by value 0) for the first entry with key ==
 * D_802E8BDC and threshold + D_803A740C >= D_80358068, and return its value;
 * otherwise return `cur` unchanged.
 * Register convention: the asm returns the value in t5 and leaves t5 alone
 * otherwise, so the C takes the caller's t5 as `cur` (the asm saves every
 * other register it uses). Asm callers keep t1 and t6 live. */
s32 func_802A8314(s32 cur) {
    s32 base = D_803A740C;
    s32 limit = D_80358068;
    s32 *e;

    if (base + 10 < limit) {
        return cur;
    }
    for (e = D_80305C58; e[1] != 0; e += 3) {
        if (e[0] != D_802E8BDC) {
            continue;
        }
        if (e[2] + base < limit) {
            continue;
        }
        return e[1];
    }
    return cur;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8314.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unless one of the flag bytes f[1], f[2], f[0] is 1 or *div is 0, set
 * *out = (f32) (p[0] - p[1]) / *div; return *out either way.
 * Register convention: div in t6, f in s0, p in s7, out in t8, result in f12
 * (the asm saves s7 and clobbers t0, t1, t3). Asm callers keep a0-a3 and t6
 * live. Note the asm computes p[0] - p[1] with a trapping `sub` even when
 * *div is 0. */
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out) {
    s32 d;
    f32 r;

    if (f[1] == 1 || f[2] == 1 || f[0] == 1) {
        return *out;
    }
    d = *div;
    if (d == 0) {
        return *out;
    }
    r = (f32) (p[0] - p[1]) / (f32) d;
    *out = r;
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A83B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A8590(s32 *p);

/* Slope/drag step for *speed. Nothing if D_80370C22 is set and the speed is
 * 0. With `clamp`, cap the speed at 1.5 * veh+0x92 (forward) or 1.5 *
 * veh+0x78 (reverse). Then, unless a flag byte f[0..2] is 1, add
 * round(func_802A8590(p) / div * -4) and drag the result toward 0 by 3 (kind
 * 0, 2, 9, 0x10) or 1, stopping at 0.
 * Register convention: gp, speed t6, kind t7, f s0, p s7, clamp t8 (clobbered),
 * div f2 (clobbered). Asm callers keep a0-a3, t6 and f12 live (a mixed N64
 * build would need a thunk). div must be nonzero (else the cvt.w.s faults). */
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div) {
    s32 s;
    s32 lim;
    s32 step;

    if (D_80370C22 != 0 && *speed == 0) {
        return;
    }
    if (clamp != 0) {
        s = *speed;
        if (s >= 0) {
            lim = VEH_S16(veh, 0x92);
            lim += lim >> 1;
            if (lim < s) {
                *speed = lim;
            }
        } else {
            lim = VEH_S16(veh, 0x78);
            lim += lim >> 1;
            if (s < lim) {
                *speed = lim;
            }
        }
    }
    if (f[0] == 1 || f[1] == 1 || f[2] == 1) {
        return;
    }
    s = func_802A8590(p);
    s = *speed + port_cvt_w_s((f32) s / div * -4.0f);
    if (kind == 0 || kind == 2 || kind == 0x10 || kind == 9) {
        step = 3;
    } else {
        step = 1;
    }
    if (s != 0) {
        if (s > 0) {
            s -= step;
            if (s < 0) {
                s = 0;
            }
        } else {
            s += step;
            if (s > 0) {
                s = 0;
            }
        }
    }
    *speed = s;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A843C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Of p[0] - p[6] and p[3] - p[6], return the one with the smaller magnitude
 * (the first on a tie). Register convention: p in s7, result in t1 (the asm
 * saves t3-t6 and clobbers t2). Asm callers keep t6, t7 and f2 live. */
s32 func_802A8590(s32 *p) {
    s32 a = p[0] - p[6];
    s32 b = p[3] - p[6];
    s32 absA = (a < 0) ? -a : a;
    s32 absB = (b < 0) ? -b : b;

    return (absB < absA) ? b : a;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8590.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING


/* Offset a point by a scaled length along a heading: n = *len (s16) scaled
 * by |f| (n * (1 - |f| / 2) when |f| < 1 (or NaN), else n / (2|f|), rounded
 * to nearest; n = 0 stays 0). With a = angle % 0x400 (C remainder),
 * s = n * sin(a) >> 16 and c = n * cos(a) >> 16 (32-bit products), the
 * quadrant of `angle` (< 0x400, < 0x800, < 0xC00, else) gives (x, z) =
 * (*px + s, *pz + c), (*px + c, *pz - s), (*px - s, *pz - c) or
 * (*px - c, *pz + s). Returns x (t0); z, *px and the cosine go to out.
 * Register convention: f in f12, angle t4, len t6, px t7, pz s1; outputs t0,
 * t1, s3, fp (conventions.txt). Asm callers keep t7 (and some t6) live; the
 * survey also lists f12/f14 as read afterwards (func_802AE104's leftovers),
 * which a C version can't hand back (a mixed N64 build would need a thunk;
 * the native port won't). The add/sub trap on overflow (game range). */
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out) {
    union {
        f32 f;
        u32 u;
    } a;
    s32 n = *len;
    s32 r;
    s32 s;
    s32 c;
    s32 x;
    s32 z;

    a.f = f;
    a.u &= 0x7FFFFFFF;
    if (n != 0) {
        if (!(a.f >= 1.0f)) {
            n = port_cvt_w_s((1.0f - a.f / 2.0f) * (f32) n);
        } else {
            n = port_cvt_w_s((f32) n / (a.f * 2.0f));
        }
    }
    r = angle % 0x400;
    s = (s32) ((u32) n * (u32) func_802AE160(r)) >> 16;
    out->fp = func_802AE104(r);
    c = (s32) ((u32) n * (u32) out->fp) >> 16;
    x = *px;
    z = *pz;
    out->s3 = x;
    if (angle < 0x400) {
        x += s;
        z += c;
    } else if (angle < 0x800) {
        x += c;
        z -= s;
    } else if (angle < 0xC00) {
        x -= s;
        z -= c;
    } else {
        x -= c;
        z += s;
    }
    out->t1 = z;
    return x;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A860C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803ED402;
extern s16 D_803ED404;
extern s16 D_803ED406;
extern u8 D_803ED40E;
void func_802A8CCC(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, Pos802A8CCC *pos);
void func_802A90E4(u16 *p);
void func_802A8FF4(u8 *veh);
void func_802A8FB4(void);
void func_802A9038(u8 *veh);
void func_802A9164(u8 *veh, u8 *f, s32 kind);
s32 func_802A8B10(s32 *a3Out);
s32 func_802A93B0(s32 index, s32 *a, s32 *b, s32 *c, s16 *tbl, s32 x, s32 z, s16 *angle, s32 *ys, s32 key,
                  u8 *veh, s32 fpIn, TriSideOut *f);
void func_802A95A4(s32 index, s32 *a, s32 *b, s32 *c, s16 *tbl, s32 x, s32 z, s16 *angle, s32 *ys, s32 key,
                   u8 *veh, s32 fpIn, s32 *s3, TriSideOut *f);


/* (s32) (|d| << 16) / div (signed 32-bit divide: traps on div 0 and on
 * 0x80000000 / -1), through func_802ACF64, >> 4 (logical); negated as asked. */
#define TILT_8768(d, div, negIfPos)                                                    \
    ((d) >= 0 ? ((negIfPos) ? -((u32) func_802ACF64((s32) ((u32) (d) << 16) / (div)) >> 4) \
                            : (u32) func_802ACF64((s32) ((u32) (d) << 16) / (div)) >> 4)   \
              : ((negIfPos) ? (u32) func_802ACF64((s32) ((u32) -(d) << 16) / (div)) >> 4   \
                            : -((u32) func_802ACF64((s32) ((u32) -(d) << 16) / (div)) >> 4)))

/* Per-frame ground contact of vehicle `id` (record veh): out-of-bounds reset
 * (func_802A8CCC, may move (x, z) and write *px/*py/*pz); D_803ED402 = divB,
 * D_803ED404 = divA, D_803ED406 = *angle, D_803ED40E = flags[0]; veh+0x9B =
 * 0; func_802A90E4(tbl), func_802A8FF4, func_802A8FB4. Then the three wheel
 * slots: for i = 0..2, if D_803ED410 is set and i > 0, func_802A9038(veh) and
 * stop; else (s8) flags[i] == 1 -> func_802A95A4 (airborne), 0 ->
 * func_802A93B0 (grounded; other values hit the asm's `syscall` debug trap
 * first). After: flags[0..2] = D_803ED3EE[0..2]; with h0..h2 =
 * D_803ED398[0..2] the height records ys (three of {now, prev, prev2})
 * shift in h0/h1/h2; *px = x, *pz = z, *py = (h1 + h2) >> 1 (logical);
 * D_803ED394 = tilt (h1 - h0, divA, negated when >= 0) and D_803ED390 = fp =
 * tilt (h2 - h0, divB, negated when < 0) (TILT_8768). Then
 * func_802582C4(id, *px, (D_803ED3A8[1] + [2]) >> 1, *pz, (h1 + h2) >> 1,
 * func_802A8B10's two results, D_803ED406), func_802A9164(veh, flags, id)
 * and, when D_803ED3F5 is 0, id != 0xFF and D_803ED40F is set,
 * func_802A92C8(*px, *pz, veh + 0x5E, veh + 0x4C, veh + 4, id, veh, fp, f).
 * The asm's add/sub/neg trap on overflow.
 * Register convention: veh gp, id t8, px t7, py s2, pz s1, x t0, z t1, divB t9,
 * divA fp, angle s4, flags s0, tbl v1, a/b/c a1-a3, ys s7; s3 in/out, s4 and
 * fp out (r), f12-f26 in/out (f). The asm restores t4 and clobbers s5-s7 (s7
 * = veh + 4 on the func_802A92C8 path); t6 is left as the callees leave it
 * (listed as read by the vehicle callers; not modelled). Asm callers keep t7
 * live. */
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f) {
    Pos802A8CCC pos;
    s32 s3 = r->s3;
    s32 i;
    s32 h0;
    s32 h1;
    s32 h2;
    s32 t;
    s32 fp;
    s32 r1;
    s32 r2;

    pos.t0 = x;
    pos.t1 = z;
    func_802A8CCC(veh, id, px, py, pz, &pos);
    x = pos.t0;
    z = pos.t1;
    D_803ED402 = divB;
    D_803ED404 = divA;
    D_803ED406 = *angle;
    D_803ED40E = flags[0];
    VEH_U8(veh, 0x9B) = 0;
    func_802A90E4((u16 *) tbl);
    func_802A8FF4(veh);
    func_802A8FB4();
    for (i = 0; i < 3; i++) {
        if (D_803ED410 != 0 && i != 0) {
            func_802A9038(veh);
            break;
        }
        if (((s8 *) flags)[i] == 1) {
            func_802A95A4(i, a, b, c, tbl, x, z, angle, ys, id, veh, divA, &s3, f);
        } else { /* 0 (other values: the asm's syscall, then this) */
            s3 = func_802A93B0(i, a, b, c, tbl, x, z, angle, ys, id, veh, divA, f);
        }
    }
    flags[0] = (&D_803ED3EE)[0];
    flags[1] = (&D_803ED3EE)[1];
    flags[2] = (&D_803ED3EE)[2];
    h0 = (&D_803ED398)[0];
    h1 = (&D_803ED398)[1];
    h2 = (&D_803ED398)[2];
    t = ys[1];
    ys[2] = t;
    ys[1] = ys[0];
    ys[0] = h0;
    ys[5] = ys[4];
    ys[4] = ys[3];
    ys[3] = h1;
    ys[8] = ys[7];
    ys[7] = ys[6];
    ys[6] = h2;
    *px = x;
    *pz = z;
    *py = (u32) (h1 + h2) >> 1;
    t = h1 - h0;
    D_803ED394_ = TILT_8768(t, divA, 1);
    t = h2 - h0;
    fp = TILT_8768(t, divB, 0);
    D_803ED390 = fp;
    r1 = func_802A8B10(&r2);
    func_802582C4(id, *px, (u32) ((&D_803ED3A8)[1] + (&D_803ED3A8)[2]) >> 1, *pz,
                  (u32) ((&D_803ED398)[1] + (&D_803ED398)[2]) >> 1, r1, r2, D_803ED406);
    func_802A9164(veh, flags, id);
    r->s4 = angle;
    if (D_803ED3F5 == 0 && id != 0xFF && D_803ED40F != 0) {
        r->s4 = &VEH_S16(veh, 0x4C);
        func_802A92C8(*px, *pz, &VEH_S16(veh, 0x5E), &VEH_S16(veh, 0x4C), &VEH_S32(veh, 4), id, veh, fp, f);
    }
    r->s3 = s3;
    r->fp = fp;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8768.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803ED402;
extern s16 D_803ED404;

/* Two tilt angles from the word triple D_803ED3A8/AC/B0: a = func_802ACF64(
 * (|AC - A8| << 16) / D_803ED404) >> 4 (logical shift), negated when
 * AC >= A8, goes to *a3Out; b likewise from B0 - A8 and D_803ED402, negated
 * when B0 < A8, is returned. The differences use trapping `sub`/`neg` and the
 * divides trap on a zero divisor.
 * Register convention: outputs a1 (return value) and a3 (*a3Out). The asm saves
 * v0, v1, a2, t2, t3, s5, t9 and fp; its caller func_802A8768 keeps a2, t7,
 * t8, f12 and f14 live. */
s32 func_802A8B10(s32 *a3Out) {
    s32 base = D_803ED3A8;
    s32 d = D_803ED3AC - base;
    s32 r;

    if (d >= 0) {
        r = func_802ACF64((s32) ((u32) d << 16) / D_803ED404);
        *a3Out = -((u32) r >> 4);
    } else {
        r = func_802ACF64((s32) ((u32) -d << 16) / D_803ED404);
        *a3Out = (u32) r >> 4;
    }
    d = D_803ED3B0 - base;
    if (d >= 0) {
        r = func_802ACF64((s32) ((u32) d << 16) / D_803ED402);
        return (u32) r >> 4;
    }
    r = func_802ACF64((s32) ((u32) -d << 16) / D_803ED402);
    return -((u32) r >> 4);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8B10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* (Pos802A8CCC is declared at the top of the file.) */

/* Out-of-bounds reset for vehicle `id` (record veh): with gx = x >> 5 and
 * gz = z >> 5, vehicle 7 counts as out on level 0 when gx < 1000, on level
 * 0x12 when gz < 900 and on level 0xD when gz < 0x44C; any vehicle is out
 * when (gx, gz) leaves the box D_803BE730..36. When out: the respawn record
 * with key `id` (key 1 unless D_80364AA8 is 1 or 0x80; the scan has no end
 * check, the key must exist) gives x, y, z (each s16 << 5), stored to *px,
 * *py, *pz and handed back in pos; D_80364412 = 1; func_802A754C(veh);
 * words 1..9 of veh = y; effect kind 0x13 (data 1000000) at the new position
 * (func_802AC6FC) and func_80277EDC(1, 1, 4, 0x6C).
 * Register convention: veh in gp, id t8, px t7, py s2, pz s1, x/z in t0/t1
 * (in/out through pos; conventions.txt). The asm saves every other register
 * (around the two calls all of them); its asm caller func_802A8768 keeps
 * a1-a3, t7-t9 live. f12/f14 are left as the callees leave them, which the
 * survey lists as read afterwards; not modelled. */
void func_802A8CCC(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, Pos802A8CCC *pos) {
    s32 gx = pos->t0 >> 5;
    s32 gz = pos->t1 >> 5;
    s32 out = 0;
    s32 key;
    u8 *rec;
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    if (id == 7) {
        if (D_802E8BDC == 0) {
            out = gx < 1000;
        } else if (D_802E8BDC == 0x12) {
            out = gz < 900;
        } else if (D_802E8BDC == 0xD) {
            out = gz < 0x44C;
        }
    }
    if (!out && gx >= D_803BE730 && gx <= D_803BE732 && gz >= D_803BE734 && gz <= D_803BE736) {
        return;
    }
    key = (((s32) D_80364AA8) == 1 || ((s32) D_80364AA8) == 0x80) ? id : 1;
    rec = D_803BE6F8;
    while (rec[0] != key) {
        rec += 9;
    }
    D_80364412 = 1;
    x = (s16) ((rec[1] << 8) | rec[2]) << 5;
    *px = x;
    y = (s16) ((rec[3] << 8) | rec[4]) << 5;
    *py = y;
    z = (s16) ((rec[5] << 8) | rec[6]) << 5;
    *pz = z;
    func_802A754C(veh);
    for (i = 1; i <= 9; i++) {
        VEH_S32(veh, i * 4) = y;
    }
    func_802AC6FC(x, y, z, 0x13, 1000000);
    func_80277EDC(1, 1, 4, 0x6C);
    pos->t0 = x;
    pos->t1 = z;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8CCC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* If D_803ED3F5 is set, triple D_803EBBF4.
 * Register note: the asm touches only v0/at/f0/f2; its caller func_802A8768
 * keeps a1-a3, t0, t1, t7-t9, f12 and f14 live across the call (a mixed N64
 * build would need a thunk, the native port does not). */
void func_802A8FB4(void) {
    if (D_803ED3F5 != 0) {
        D_803EBBF4 = D_803EBBF4 * 3.0f;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8FB4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* If the vehicle's mode byte (veh+0x9F) is 0x67 and D_802E8BDC == 12, set
 * D_803ED3F7 = 8. (The asm clobbers v0.) Asm callers keep a1-a3, t0, t1,
 * t7-t9, f12 and f14 live. */
void func_802A8FF4(u8 *veh) {
    if (VEH_U8(veh, 0x9F) == 0x67 && D_802E8BDC == 12) {
        D_803ED3F7 = 8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8FF4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copy each "current" value into its two shadow copies: the bytes D_803ED3EE,
 * D_803ED3F2, D_803ED3EA, the words D_803ED398, D_803ED3A8 and the vehicle
 * words +0x34, +0x28, +0x40. (The asm saves v0.) Asm callers keep t0, t1,
 * t7-t9, f12 and f14 live. */
void func_802A9038(u8 *veh) {
    s32 w;

    (&D_803ED3EF)[0] = D_803ED3EE;
    (&D_803ED3EF)[1] = D_803ED3EE;
    D_803ED39C = D_803ED398;
    D_803ED3A0 = D_803ED398;
    D_803ED3AC = D_803ED3A8;
    D_803ED3B0 = D_803ED3A8;
    w = VEH_S32(veh, 0x34);
    VEH_S32(veh, 0x38) = w;
    VEH_S32(veh, 0x3C) = w;
    w = VEH_S32(veh, 0x28);
    VEH_S32(veh, 0x2C) = w;
    VEH_S32(veh, 0x30) = w;
    w = VEH_S32(veh, 0x40);
    VEH_S32(veh, 0x44) = w;
    VEH_S32(veh, 0x48) = w;
    (&D_803ED3F2)[1] = D_803ED3F2;
    (&D_803ED3F2)[2] = D_803ED3F2;
    D_803ED3EB = D_803ED3EA;
    (&D_803ED3EB)[1] = D_803ED3EA;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9038.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* D_803ED410 = 1 if all six halfwords p[0..5] are 0, else 0.
 * Register convention: p in v1 (the asm saves v0). Asm callers keep a1-a3,
 * t0, t1, t7-t9, f12 and f14 live. */
void func_802A90E4(u16 *p) {
    if (p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0 && p[4] == 0 && p[5] == 0) {
        D_803ED410 = 1;
    } else {
        D_803ED410 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A90E4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* From the three bytes at D_803ED3F2 (each >= 100 counted as 2): veh+0x50 =
 * their average; veh+0x9F = D_803ED40D = their maximum. Then set D_803BE738
 * when the max is 0x66, or when it is 100 with D_80358064 set and kind not
 * 9/11/0x11/0x12 - both only if *f != 1.
 * Register convention: gp, f in s0, kind in t8 (the asm clobbers t0-t2).
 * Asm callers keep a0, a2, a3, t7, t8, f12 and f14 live. */
void func_802A9164(u8 *veh, u8 *f, s32 kind) {
    u8 *v = &D_803ED3F2;
    s32 sum;
    s32 x;
    s32 max;

    x = v[0];
    if (x >= 100) {
        x = 2;
    }
    sum = x;
    x = v[1];
    if (x >= 100) {
        x = 2;
    }
    sum += x;
    x = v[2];
    if (x >= 100) {
        x = 2;
    }
    sum += x;
    VEH_U8(veh, 0x50) = (u32) sum / 3;
    max = 0;
    if (max < v[0]) {
        max = v[0];
    }
    if (max < v[1]) {
        max = v[1];
    }
    if (max < v[2]) {
        max = v[2];
    }
    VEH_U8(veh, 0x9F) = max;
    D_803ED40D = max;
    if (D_803ED40D == 0x66 && f[0] != 1) {
        D_803BE738 = 1;
    }
    if (kind != 9 && kind != 11 && kind != 0x11 && kind != 0x12 && D_80358064 != 0 && D_803ED40D == 100 &&
        f[0] != 1) {
        D_803BE738 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9164.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Ground heights for the three wheel slots: for i = 0..2, (dx, dz) =
 * func_802A94A4(i, tbl, angle) and func_802A9B1C(i, x + dx, z + dz,
 * ys[3 * i], key, veh, fpIn, f) (12-byte records, y first; the FP state f
 * chains through). Then the slot kinds go to key's D_803ED3B8 record
 * (PORT_STORE_KINDS). Returns D_803ED3EA[1] (the asm's t6 at the end). The
 * asm's adds trap on overflow.
 * Register convention: x t0, z t1, tbl t3, angle s4, ys s7, key t8, veh gp,
 * fp; f12-f26 in/out (f); result t6. The asm restores t0, t1, t7, s0, s2, s7,
 * t9, clobbers s5/s6 and leaves v0 = 3, t2 = -1. Asm caller func_802BB274
 * keeps t7 live. */
s32 func_802A92C8(s32 x, s32 z, s16 *tbl, s16 *angle, s32 *ys, s32 key, u8 *veh, s32 fpIn, TriSideOut *f) {
    s32 i;
    s32 dx;
    s32 dz;

    for (i = 0; i < 3; i++) {
        dx = func_802A94A4(i, tbl, angle, &dz);
        func_802A9B1C(i, x + dx, z + dz, ys[i * 3], key, veh, fpIn, f);
    }
    PORT_STORE_KINDS(key);
    return (&D_803ED3EA)[1];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A92C8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Ground slot `index` while on the ground: (dx, dz) = func_802A94A4(index,
 * tbl, angle); y = ys[3 * index], prev = ys[3 * index + 1]; v =
 * func_802A9514(y - prev) (clamped step); expected = v +
 * round(D_803EBBF4) + y; h = func_802A9B1C(index, x + dx, z + dz, y, key,
 * veh, fpIn, f). If h <= expected - 0x1E (the ground fell away):
 * v = func_802A9540(index, a, b, c, y, v) and the height is `expected`;
 * else D_803ED3EE[index] = 0 and the height is h. D_803ED398[index] = the
 * height. Returns v. The asm's add/sub trap on overflow.
 * Register convention: index v0, a/b/c a1-a3, tbl v1, x t0, z t1, angle s4,
 * ys s7, key t8, veh gp, fp; result s3, f12-f26 in/out (f). The asm restores
 * t0, t1, t3, t6, t7, s0-s2, s4; asm caller func_802A8768 keeps v0, a1-a3,
 * t0, t1, t7-t9 live. */
s32 func_802A93B0(s32 index, s32 *a, s32 *b, s32 *c, s16 *tbl, s32 x, s32 z, s16 *angle, s32 *ys, s32 key,
                  u8 *veh, s32 fpIn, TriSideOut *f) {
    s32 dx;
    s32 dz;
    s32 y;
    s32 v;
    s32 expected;
    s32 h;

    dx = func_802A94A4(index, tbl, angle, &dz);
    y = ys[index * 3];
    v = func_802A9514(y - ys[index * 3 + 1]);
    expected = v + port_cvt_w_s(D_803EBBF4) + y;
    h = func_802A9B1C(index, x + dx, z + dz, y, key, veh, fpIn, f);
    if (!(expected - 0x1E < h)) {
        v = func_802A9540(index, a, b, c, y, v);
        h = expected;
    } else {
        (&D_803ED3EE)[index] = 0;
    }
    (&D_803ED398)[index] = h;
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A93B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rotate table entry `index` of tbl ({s16 x, s16 z} pairs) by the angle *angle
 * (s16) with func_802ACE38: returns its a3 result (the x offset) and stores its
 * t1 result (the z offset) in *dz. (The asm's add of index * 4 traps.)
 * Register convention: index v0, tbl v1, angle s4; results t5 (return value)
 * and t6 (*dz). The asm saves a0-a3, t0, t1 and fp around the call; asm
 * callers keep v0, v1, a0-a3, t0-t4, t7 and t8 live. It also leaves the sine
 * routine's f12/f14 temporaries, which the survey lists as read by
 * func_802A92C8/93B0/95A4/992C/9A60: they only flow into the triangle scans'
 * FP pass-through state (TriSideOut), not modelled here (a mixed N64 build
 * would need a thunk; the native port won't). */
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz) {
    return func_802ACE38(tbl[index * 2], tbl[index * 2 + 1], *angle, dz);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A94A4.s")
#endif

/* func_802A9514: clamps $s3 to a max of 0x240 (`if (s3 >= 0x241) s3 =
 * 0x240;`), reading AND writing $s3 directly with no parameter or
 * return value involved at all - same non-ABI register-threading
 * character as the func_802A06B4 family in hd_code/56040.c, here used
 * as a persistent "hot" value shared across functions via a dedicated
 * callee-saved register instead of a global variable. Not expressible
 * as a normal C function. */
#ifdef NON_MATCHING
/* Port rewrite: clamp to at most 0x240 (negative values pass through).
 * Register convention: value in s3, result in s3 (conventions.txt maps it to
 * a0/ret). Asm callers keep a1-a3, t0-t2, t8, t9, f12 and f14 live. */
s32 func_802A9514(s32 x) {
    if (x >= 0 && x >= 0x241) {
        x = 0x240;
    }
    return x;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9514.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Record slot `index`: a[index] = value clamped by func_802A9514, b[index] = 2,
 * c[index] = extra, D_803ED3EE[index] = 1; returns the clamped value.
 * Register convention: index v0, a/b/c in a1-a3, extra t2, value s3; result in
 * s3 (the asm saves a1-a3; it sets s0 = 1 and clobbers t3, t7). Asm callers
 * keep t8, t9, f12 and f14 live. */
s32 func_802A9540(s32 index, s32 *a, s32 *b, s32 *c, s32 extra, s32 value) {
    value = func_802A9514(value);
    a[index] = value;
    b[index] = 2;
    c[index] = extra;
    (&D_803ED3EE)[index] = 1;
    return value;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9540.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Ground slot `index` while airborne: with n = b[index] (then b[index] =
 * n + 1), drop = a[index] * n + round(D_803EBBF4 * (f32) (n * n)) (32-bit
 * products) and expected = c[index] + drop. (dx, dz) = func_802A94A4(index,
 * tbl, angle), h = func_802A9B1C(index, x + dx, z + dz, ys[3 * index], key,
 * veh, fpIn, f). Landing test: with key 9 and D_803F7C49 set, it lands when
 * h - expected >= 0x6A5 or h < expected; otherwise when h < expected. Landed:
 * D_803ED398[index] = max(expected, 0), D_803ED3EE[index] = 1, s3 unchanged.
 * Not landed: D_803ED398[index] = h and *s3 = func_802A9710(index, a, b, c, h,
 * drop, *s3). The asm's add/sub trap on overflow.
 * Register convention: index v0, a/b/c a1-a3, tbl v1, x t0, z t1, angle s4,
 * ys s7, key t8, veh gp, fp; s3 in/out (*s3), f12-f26 in/out (f); the asm
 * restores t0-t7, s0-s2 and leaves s6 = drop or what func_802A9710 leaves
 * (clobbers s6). Asm caller func_802A8768 keeps v0, a1-a3, t0, t1, t7-t9
 * live. */
void func_802A95A4(s32 index, s32 *a, s32 *b, s32 *c, s16 *tbl, s32 x, s32 z, s16 *angle, s32 *ys, s32 key,
                   u8 *veh, s32 fpIn, s32 *s3, TriSideOut *f) {
    s32 n = b[index];
    s32 drop;
    s32 expected;
    s32 dx;
    s32 dz;
    s32 h;

    b[index] = n + 1;
    drop = a[index] * n + port_cvt_w_s(D_803EBBF4 * (f32) (n * n));
    expected = c[index] + drop;
    dx = func_802A94A4(index, tbl, angle, &dz);
    h = func_802A9B1C(index, x + dx, z + dz, ys[index * 3], key, veh, fpIn, f);
    if ((key == 9 && D_803F7C49 != 0 && h - expected >= 0x6A5) || h < expected) {
        if (expected < 0) {
            expected = 0;
        }
        (&D_803ED398)[index] = expected;
        (&D_803ED3EE)[index] = 1;
    } else {
        (&D_803ED398)[index] = h;
        *s3 = func_802A9710(index, a, b, c, h, drop, *s3);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A95A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Slot `index` check: predicted = a[index] * (b[index] - 2) + round(D_803EBBF4
 * * (b[index] - 2)^2) (32-bit products, cvt.w.s rounding); if |pos -
 * predicted| exceeds D_803ED3F6 (signed compare), play sound 0xC when
 * D_803ED40B is set (the asm saves every register around that call) and
 * restart the slot with func_802A9540(index, a, b, c, extra, |pos -
 * predicted| / D_803ED3F7) (unsigned divide; D_803ED3F7 == 0 traps),
 * returning its result. Otherwise D_803ED3EE[index] = 0 and the result is
 * the caller's s3, unchanged. The asm's add/sub/neg trap on overflow.
 * Register convention: index v0, a/b/c a1-a3, extra t3, pos s6, s3 in/out
 * (result); it clobbers s0 and s6. Asm caller func_802A95A4 keeps f12 and
 * f14 live. */
s32 func_802A9710(s32 index, s32 *a, s32 *b, s32 *c, s32 extra, s32 pos, s32 s3) {
    s32 n = b[index] - 2;
    s32 d = pos - (a[index] * n + port_cvt_w_s(D_803EBBF4 * (f32) (n * n)));

    if (d < 0) {
        d = -d;
    }
    if (!(D_803ED3F6 < d)) {
        (&D_803ED3EE)[index] = 0;
        return s3;
    }
    if (D_803ED40B != 0) {
        func_80260650(D_80367738, 0xC, NULL);
    }
    return func_802A9540(index, a, b, c, extra, (u32) d / D_803ED3F7);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9710.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Reset the three ground slots from the triangle scans: for i = 0..2, (dx, dz)
 * = func_802A94A4(i, tbl, angle); h = func_802A9F24(x + dx, z + dz, y, key, f,
 * &kind, ..); when kind is 0, func_802AA094(x + dx, z + dz, y, f, &h, &fp)
 * may replace h (and fp). D_803ED3EA[i] = kind, dst[3i..3i+2] = h,
 * D_803ED3F2[i] = fp (low byte; fp carries over between slots). Then *mid =
 * (dst[3] + dst[6]) >> 1 (logical), veh+0x50 = (D_803ED3F2[0] + [1] + [2]) / 3
 * and the kinds go to key's D_803ED3B8 record (PORT_STORE_KINDS). Returns dst
 * (the asm's s3). The asm's adds trap on overflow.
 * Register convention: tbl v1, y t2, x t7, z s0, dst s1, mid s2, angle s4, key
 * t8, fp, veh gp; f12-f26 in/out (f); result s3. The asm restores s1 and fp
 * and clobbers s5/s6 (and a1/a2 through the scans). */
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh,
                   TriSideOut *f) {
    s32 fp = fpIn;
    s32 *p = dst;
    s32 i;
    s32 dx;
    s32 dz;
    s32 h;
    s32 kind;
    s32 a2;

    for (i = 0; i < 3; i++) {
        dx = func_802A94A4(i, tbl, angle, &dz);
        h = func_802A9F24(x + dx, z + dz, y, key, f, &kind, &a2);
        if (kind == 0) {
            func_802AA094(x + dx, z + dz, y, f, &h, &fp);
            kind = 0;
        }
        (&D_803ED3EA)[i] = kind;
        p[0] = h;
        p[1] = h;
        p[2] = h;
        p += 3;
        (&D_803ED3F2)[i] = fp;
    }
    *mid = (u32) (dst[3] + dst[6]) >> 1;
    VEH_U8(veh, 0x50) = (u32) ((&D_803ED3F2)[0] + (&D_803ED3F2)[1] + (&D_803ED3F2)[2]) / 3;
    PORT_STORE_KINDS(key);
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A992C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* As func_802A992C but through func_802A9B1C: for i = 0..2, veh+0x9B = 0,
 * h = func_802A9B1C(i, x + dx, z + dz, y, key, veh, fp, f), dst[3i..3i+2] =
 * h, D_803ED3F2[i] = fp (overwriting func_802A9B1C's flag). Then *mid =
 * (dst[3] + dst[6]) >> 1 (logical), veh+0x50 = (D_803ED3F2[0] + [1] + [2]) /
 * 3, D_803ED390 = D_803ED394 = 0. Returns the last dz (the asm's t6); out
 * gets dst + 9 and dst (s1, s3). The asm's adds trap on overflow.
 * Register convention: tbl v1, y t2, x t7, z s0, dst s1, mid s2, angle s4, key
 * t8, fp, veh gp; f12-f26 in/out (f); results t6, s1, s3 (out); clobbers
 * s5/s6. Asm callers keep t7 live. */
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh,
                  TriSideOut *f, Out802A9A60 *out) {
    s32 *p = dst;
    s32 i;
    s32 dx;
    s32 dz;
    s32 h;

    for (i = 0; i < 3; i++) {
        dx = func_802A94A4(i, tbl, angle, &dz);
        VEH_U8(veh, 0x9B) = 0;
        h = func_802A9B1C(i, x + dx, z + dz, y, key, veh, fp, f);
        p[0] = h;
        p[1] = h;
        p[2] = h;
        p += 3;
        (&D_803ED3F2)[i] = fp;
    }
    *mid = (u32) (dst[3] + dst[6]) >> 1;
    VEH_U8(veh, 0x50) = (u32) ((&D_803ED3F2)[0] + (&D_803ED3F2)[1] + (&D_803ED3F2)[2]) / 3;
    D_803ED390 = 0;
    D_803ED394_ = 0;
    out->s1 = p;
    out->s3 = dst;
    return dz;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9A60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802A9CAC(s32 index, s32 kind);

#define ABS_9B1C(v) ((v) < 0 ? -(v) : (v))

/* Ground height for slot `index` at (x, y + 0x78, z): h1 = func_802A9DC0
 * (its fp result kept as fp1), h2 = func_802A9F24 (its id result kept),
 * then func_802AA094 (found flag, h3 and fp in/out, seeded with h2 and fp1).
 * Not found and h1, h2 both 0x5F5E0FF: result y, D_803ED3F2 = 1. Found and
 * h3 at least as close (|h - (y + 0x78)|) as h2 and h1: func_802A9CAC(index,
 * skip), result h3, D_803ED3F2 = its fp. Otherwise h2 if strictly closer than
 * h1 (D_803ED3F2 = 1, D_803ED3EA = the id), else h1 (veh byte 0x9B = 1,
 * D_803ED3F2 = fp1). D_803ED3EA[index] is 0 except on the h2 path; the result
 * is clamped at 0 and stored to D_803ED3A8[index]. The TriSideOut FP results
 * chain through the three scans.
 * Register convention: index v0, x t0, z t1, y t2, skip t8, veh gp, fp in;
 * result t3, f12-f26 in/out (f); f28 clobbered by the scans. The asm's
 * add/sub/neg trap on overflow. Asm callers keep a1-a3, t2, t6-t8 live (a
 * mixed N64 build would need a thunk). */
s32 func_802A9B1C(s32 index, s32 x, s32 z, s32 y, s32 skip, u8 *veh, s32 fpIn, TriSideOut *f) {
    s32 yy = y + 0x78;
    s32 fp = fpIn;
    s32 fp1;
    s32 h1;
    s32 h2;
    s32 h3;
    s32 id;
    s32 a2;
    s32 d1;
    s32 d2;
    s32 d3;
    s32 res;
    s32 flag;
    s32 kind = 0;

    h1 = func_802A9DC0(x, z, yy, f, &fp);
    fp1 = fp;
    h2 = func_802A9F24(x, z, yy, skip, f, &id, &a2);
    h3 = h2;
    if (func_802AA094(x, z, yy, f, &h3, &fp) == 0) {
        if (h1 == 0x5F5E0FF && h2 == 0x5F5E0FF) {
            res = yy - 0x78;
            flag = 1;
            goto store;
        }
        d2 = ABS_9B1C(h2 - yy);
        d1 = ABS_9B1C(h1 - yy);
    } else {
        d2 = ABS_9B1C(h2 - yy);
        d3 = ABS_9B1C(h3 - yy);
        d1 = ABS_9B1C(h1 - yy);
        if (!(d2 < d3) && !(d1 < d3)) {
            func_802A9CAC(index, skip);
            res = h3;
            flag = fp;
            goto store;
        }
    }
    if (d2 < d1) {
        res = h2;
        flag = 1;
        kind = id;
    } else {
        VEH_U8(veh, 0x9B) = 1;
        res = h1;
        flag = fp1;
    }
store:
    (&D_803ED3EA)[index] = kind;
    (&D_803ED3F2)[index] = flag;
    if (res < 0) {
        res = 0;
    }
    (&D_803ED3A8)[index] = res;
    return res;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9B1C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* If the 0x14-byte record list D_803EBC00..D_803EBC04 spans at least 0x8C
 * bytes, swap record (index + (kind == 0xFF ? 3 : 0)) with the record
 * D_803EBC08 points at (through the temp D_803EBBD8), unless they're the same.
 * Register convention: index in v0, kind in t8 (the asm saves v1, a0-a3).
 * Asm callers keep t3, f12 and f14 live. */
void func_802A9CAC(s32 index, s32 kind) {
    u8 *base = D_803EBC00;
    s32 *rec;
    s32 *cur;
    s32 i;

    if ((s32) (D_803EBC04 - base) < 0x8C) {
        return;
    }
    if (kind == 0xFF) {
        index += 3;
    }
    rec = (s32 *) (base + index * 0x14);
    cur = (s32 *) D_803EBC08;
    if (cur == rec) {
        return;
    }
    for (i = 0; i < 5; i++) {
        D_803EBBD8[i] = cur[i];
    }
    cur = (s32 *) D_803EBC08;
    for (i = 0; i < 5; i++) {
        cur[i] = rec[i];
    }
    for (i = 0; i < 5; i++) {
        rec[i] = D_803EBBD8[i];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9CAC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802AA2E4(s32 x, s32 z, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2,
                  s32 *a3Out, s32 *t6Out);

/* The per-triangle test every scan below makes: (x, z) inside the bounding
 * box (func_802AA5E0) and the triangle (func_802AA460, which also fills *f),
 * then *h = the plane height there (func_802AA2E4, which also sets the asm's
 * a3 and t6). v = x0, y0, z0, x1, y1, z1, x2, y2, z2. */
static s32 port_tri_height(s32 x, s32 z, s32 *v, TriSideOut *f, s32 *h, s32 *a3, s32 *t6) {
    if (func_802AA5E0(x, z, v[0], v[2], v[3], v[5], v[6], v[8]) == 0) {
        return 0;
    }
    if (func_802AA460(x, z, v[0], v[2], v[3], v[5], v[6], v[8], f) == 0) {
        return 0;
    }
    *h = func_802AA2E4(x, z, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], a3, t6);
    return 1;
}

/* Loads a triangle stored as nine s16 coordinates in 1/32 units. */
#define PORT_TRI_S16(v, rec)                       \
    {                                              \
        s32 i_;                                    \
        for (i_ = 0; i_ < 9; i_++) {               \
            (v)[i_] = ((s16 *) (rec))[i_] << 5;    \
        }                                          \
    }

/* Of the triangles in D_803F7828 .. D_803F782C containing (x, z), the one
 * whose plane height is nearest y (the later one on a tie, |y - h| compared
 * unsigned): returns its height (0x5F5E0FF if none) and puts its byte +0x24
 * in *fpOut (unchanged if none).
 * Register convention: x t0, z t1, y t2; result t3, fp (*fpOut) and the
 * func_802AA460 FP results (f12, f14, f20, f22, f24, f26 through f, in/out).
 * The asm saves every other register it touches and clobbers f28; `y - h` is a
 * trapping `sub`. Asm callers keep t0, t1, t2 and t8 live. */
s32 func_802A9DC0(s32 x, s32 z, s32 y, TriSideOut *f, s32 *fpOut) {
    u8 *rec = D_803F7828;
    u8 *end = D_803F782C;
    s32 best = 0x5F5E0FF;
    u32 bestD = 0x5F5E0FF;
    s32 h;
    s32 d;
    s32 a3;
    s32 t6;

    while (rec != end) {
        u8 *tri = rec;

        rec += 0x28;
        if (!port_tri_height(x, z, (s32 *) tri, f, &h, &a3, &t6)) {
            continue;
        }
        d = y - h;
        if (d < 0) {
            d = -d;
        }
        if (bestD < (u32) d) {
            continue;
        }
        best = h;
        bestD = d;
        *fpOut = tri[0x24];
    }
    return best;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9DC0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_802A9DC0 over the transformed triangles D_803EBDB0 .. *D_803EBBEC
 * (0x38 bytes, 9 s32 coords, u16 id at +0x36), skipping those with id ==
 * `skip`: returns the nearest height (0x5F5E0FF if none); *idOut = its id,
 * else 0 - but func_802AA2E4's t6 (y0 - y2) when the last triangle it
 * measured was not the nearest, as in the asm; *a2Out = skip.
 * Register convention: x t0, z t1, y t2, skip t8; results t3, t6 (*idOut),
 * a2 (*a2Out) and the FP results through f (in/out). The asm saves every
 * other register it touches and clobbers f28; asm callers keep a1, t0-t2 and
 * t8 live. */
s32 func_802A9F24(s32 x, s32 z, s32 y, s32 skip, TriSideOut *f, s32 *idOut, s32 *a2Out) {
    u8 *rec = D_803EBDB0;
    u8 *end = D_803EBBEC;
    s32 best = 0x5F5E0FF;
    u32 bestD = 0x5F5E0FF;
    s32 t6 = 0;
    s32 id;
    s32 h;
    s32 d;
    s32 a3;

    *a2Out = skip;
    while (rec != end) {
        u8 *tri = rec;

        id = *(u16 *) (tri + 0x36);
        rec += 0x38;
        if (id == skip) {
            continue;
        }
        if (!port_tri_height(x, z, (s32 *) tri, f, &h, &a3, &t6)) {
            continue;
        }
        d = y - h;
        if (d < 0) {
            d = -d;
        }
        if (bestD < (u32) d) {
            continue;
        }
        best = h;
        bestD = d;
        t6 = id;
    }
    *idOut = t6;
    return best;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9F24.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Ground lookup: the grid cell of (x, z) (signed divides by D_803BE718 /
 * D_803BE71C, row stride D_803BE720) gives a list of 0x14-byte triangles (9
 * s16 coords in 1/32 units, u8 at +0x12, stop flag at +0x13); its bounds go
 * to D_803EBC00/D_803EBC04. Scanning like func_802A9DC0, each nearer-or-equal
 * hit records the triangle in D_803EBC08, its height in *t3io and byte +0x12
 * in *fpio, and ends the scan if byte +0x13 is set. Returns 1 if any hit.
 * Register convention: x t0, z t1, y t2; results a1 (return value), t3 and fp
 * (in/out: unchanged without a hit) and the FP results through f. The asm
 * saves every other register it touches and clobbers f28; the divides trap on
 * a zero cell size, `y - h` and the index arithmetic are trapping. Asm callers
 * keep a2, t2, t4, t6 and t8 live. */
s32 func_802AA094(s32 x, s32 z, s32 y, TriSideOut *f, s32 *t3io, s32 *fpio) {
    s32 cx = x / ((s32) D_803BE718);
    s32 cz = z / ((s32) D_803BE71C);
    u8 **cell = &D_803BDB10[D_803BE720 * cz + cx];
    u8 *rec = cell[0];
    u8 *end = cell[1] - 4;
    u32 bestD = 0x5F5E0FF;
    s32 found = 0;
    s32 v[9];
    s32 h;
    s32 d;
    s32 a3;
    s32 t6;

    D_803EBC00 = rec;
    D_803EBC04 = end;
    while (rec != end) {
        u8 *tri = rec;

        PORT_TRI_S16(v, tri);
        rec += 0x14;
        if (!port_tri_height(x, z, v, f, &h, &a3, &t6)) {
            continue;
        }
        d = y - h;
        if (d < 0) {
            d = -d;
        }
        if (bestD < (u32) d) {
            continue;
        }
        D_803EBC08 = tri;
        found = 1;
        *t3io = h;
        bestD = d;
        *fpio = tri[0x12];
        if (tri[0x13] != 0) {
            break;
        }
    }
    return found;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA094.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
f64 fabs(f64);
#pragma intrinsic(fabs)
extern f64 D_80305C50; /* 2000.0 */

/* Height of the plane through the triangle p0 = (x0, y0, z0), p1, p2 above
 * the point (x, z), as round(|h / (ny * 2000)| * D_80305C50) - 1000, where
 * n = (nx, ny, nz) is the triangle normal (64-bit integer cross product),
 * d = -(n . p1) and h = nx * x - 1000 * ny + nz * z + d. Returns 0xFF676981
 * when ny == 0 (vertical triangle).
 * Register convention: x t0, z t1, p0 s1/s2/s3, p1 s4/s5/s6, p2 s7/t8/t9;
 * result in v0. The asm also leaves nz * z1 (low 32 bits) in a3 and y0 - y2
 * in t6, which some callers read: here *a3Out and *t6Out. The asm's 32-bit
 * subtractions, 64-bit dsubs and final add trap on overflow; the C wraps.
 * Rounding is to nearest even (cvt.l.d under the default FCSR).
 * Asm callers keep a1, a2, t0-t3, t7, f12 and f14 live. */
s32 func_802AA2E4(s32 x, s32 z, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2,
                  s32 *a3Out, s32 *t6Out) {
    s64 dy01 = y0 - y1;
    s64 dz02 = z0 - z2;
    s64 dz01 = z0 - z1;
    s64 dy02 = y0 - y2;
    s64 dx02 = x0 - x2;
    s64 dx01 = x0 - x1;
    s64 nx = dy01 * dz02 - dz01 * dy02;
    s64 ny = dz01 * dx02 - dx01 * dz02;
    s64 nz = dx01 * dy02 - dy01 * dx02;
    s64 nzz1 = nz * z1;
    s64 d = -(nx * x1 + ny * y1 + nzz1);
    s64 h = nx * x + ny * -1000 + nz * z + d;
    s64 t;
    f64 r;
    f64 frac;

    *t6Out = y0 - y2;
    *a3Out = (s32) nzz1;
    if (ny == 0) {
        return 0xFF676981;
    }
    r = D_80305C50 * fabs((f64) h / (f64) (ny * 2000));
    t = (s64) r; /* truncates; round to nearest even below */
    frac = r - (f64) t;
    if (frac > 0.5 || (frac == 0.5 && (t & 1))) {
        t++;
    } else if (frac < -0.5 || (frac == -0.5 && (t & 1))) {
        t--;
    }
    return (s32) t - 1000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA2E4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (TriSideOut is declared at the top of the file.) */

/* 1 if the point (x, z) is on the same side of each edge of the triangle
 * (x0, z0), (x1, z1), (x2, z2) as the point c = ((x0 + (x1 + x2) / 2) / 2,
 * (z0 + (z1 + z2) / 2) / 2); edges whose line passes through (x, z) are
 * skipped. 0 otherwise. Float math in single precision, as the asm.
 * Register convention: x t0, z t1, x0 s1, z0 s3, x1 s4, z1 s6, x2 s7, z2 t9;
 * result in v0, plus f12/f14/f20/f22/f24/f26 (see TriSideOut) through `out`
 * (f24 is passed in too). The asm clobbers f28 (callee-saved) and f2-f18;
 * its x1 + x2, z1 + z2 and edge differences use trapping add/sub. Asm callers
 * keep a1-a3 and t0-t9 live. */
s32 func_802AA460(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, TriSideOut *out) {
    f32 px = x;
    f32 pz = z;
    f32 cx = ((f32) x0 + (f32) (x1 + x2) / 2.0f) / 2.0f;
    f32 cz = ((f32) z0 + (f32) (z1 + z2) / 2.0f) / 2.0f;
    f32 cross;
    f32 side = 2.0f;
    f32 sideZ = out->sideZ;
    f32 ax;
    f32 az;
    f32 dx;
    f32 dz;
    s32 edge;
    s32 result = 1;

    for (edge = 3; edge != 0; edge--) {
        if (edge == 3) {
            ax = x0;
            az = z0;
            dz = z1 - z0;
            dx = x1 - x0;
        } else if (edge == 2) {
            ax = x0;
            az = z0;
            dz = z2 - z0;
            dx = x2 - x0;
        } else {
            ax = x1;
            az = z1;
            dz = z2 - z1;
            dx = x2 - x1;
        }
        cross = (px - ax) * dz - (pz - az) * dx;
        if (cross == 0.0f) {
            continue;
        }
        sideZ = (cz - az) * dx;
        side = (cx - ax) * dz - sideZ;
        if (cross > 0.0f) {
            if (side > 0.0f) {
                continue;
            }
        } else if (side < 0.0f) {
            continue;
        }
        result = 0;
        break;
    }
    out->pz = pz;
    out->cross = cross;
    out->cz = cz;
    out->side = side;
    out->sideZ = sideZ;
    out->dz = dz;
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA460.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 1 if the point (x, z) lies inside the bounding box of the triangle
 * (x0, z0), (x1, z1), (x2, z2), bounds inclusive; else 0.
 * Register convention: x t0, z t1, x0 s1, z0 s3, x1 s4, z1 s6, x2 s7, z2 t9;
 * result in v0 (the asm saves t2-t5). Asm callers keep a1-a3, t0-t9, f12 and
 * f14 live. */
s32 func_802AA5E0(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2) {
    s32 minX = x0;
    s32 minZ = z0;
    s32 maxX = x0;
    s32 maxZ = z0;

    if (x1 < minX) {
        minX = x1;
    }
    if (x2 < minX) {
        minX = x2;
    }
    if (maxX < x1) {
        maxX = x1;
    }
    if (maxX < x2) {
        maxX = x2;
    }
    if (z1 < minZ) {
        minZ = z1;
    }
    if (z2 < minZ) {
        minZ = z2;
    }
    if (maxZ < z1) {
        maxZ = z1;
    }
    if (maxZ < z2) {
        maxZ = z2;
    }
    if (x < minX || maxX < x || z < minZ || maxZ < z) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA5E0.s")
#endif

#ifdef NON_MATCHING
extern u16 D_803EBB58[]; /* accumulated Mtx (see func_802ACCCC) */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* o32 entry: D_803ED390 = rx, D_803ED392 = ry, D_803ED394 = rz (halfwords),
 * then func_802AA764(x, y, z, scale, m). The asm saves s0-s7, gp and fp. */
void func_802AA6D0(s32 x, s32 y, s32 z, s16 rx, s16 ry, s16 rz, s32 scale, Mtx *m) {
    D_803ED392 = ry;
    D_803ED390 = rx;
    D_803ED394_ = rz;
    func_802AA764(x, y, z, scale, (s32 *) m);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA6D0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Build the model matrix m (16.16 words, converted in place to the Mtx
 * layout at the end by func_802AC8CC): m = scale matrix (scale on all three
 * axes, func_802ACC68), then for each of rotation by D_803ED390
 * (func_802ACBDC), D_803ED394 (func_802ACB50), D_803ED392 (func_802ACAC4)
 * (angles read as u16) and the translation (x, y, z) << 11 (func_802ACA60)
 * build that matrix in D_803EBB58 and combine it into m with
 * func_802ACCCC(D_803EBB58, m).
 * Register convention: x s4, y s5, z s6, scale s7, m t8. The asm leaves a0 =
 * &D_803EBB58 and s2 = m (clobbers s2), and the cosine/sine routines' f12/f14
 * temporaries, which the survey lists as read by asm callers (not modelled,
 * see func_802AE104). */
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m) {
    s32 *tmp = (s32 *) D_803EBB58;

    func_802ACC68(scale, scale, scale, m);
    func_802ACBDC((u16) D_803ED390, tmp);
    func_802ACCCC(tmp, m);
    func_802ACB50((u16) D_803ED394_, tmp);
    func_802ACCCC(tmp, m);
    func_802ACAC4((u16) D_803ED392, tmp);
    func_802ACCCC(tmp, m);
    func_802ACA60(x << 11, y << 11, z << 11, tmp);
    func_802ACCCC(tmp, m);
    func_802AC8CC((u16 *) m);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA764.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copy 64 bytes (8 doublewords) from src + off to dst + off (8-aligned).
 * Register convention: src t0, dst t1, off t2 (the asm saves t0, t1, t3, t4).
 * Asm callers keep a1-a3, f12 and f14 live. */
void func_802AA838(u8 *src, u8 *dst, s32 off) {
    u64 *s = (u64 *) (src + off);
    u64 *d = (u64 *) (dst + off);
    s32 i;

    for (i = 8; i != 0; i--) {
        *d++ = *s++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA838.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803EBB58[]; /* accumulated Mtx (s15.16: int halves, then frac halves at +0x20) */
extern u16 D_803EBB98[]; /* product temp, same layout */

/* Element (row, col) of an N64 fixed-point Mtx as s15.16. */
#define MTX_FIX(m, row, col) \
    ((s32) (((m)[(row) * 4 + (col)] << 16) | (m)[16 + (row) * 4 + (col)]))


/* Concatenate `count` Mtx (each at base + offsets[i]) as M = M0 * M1 * ...
 * (64-bit products, >> 16) into D_803EBB58, then transform (x, y, z) by M in
 * 32-bit integer math: x' = M00 x + M10 y + M20 z + (M30 int part only - the
 * asm drops its fraction), y' and z' likewise with full M31/M32. Returns
 * x' >> 11; y' >> 11 and z' >> 11 go to regs->v1/a0, y'/z' themselves to
 * regs->s1/s2, and the last matrix address to regs->a3 (with count == 1:
 * the first matrix + 0x40, where its copy loop leaves it). With count == 0
 * nothing is computed and the results are the caller's s0/s1/s2 >> 11.
 * Register convention: count a1, offsets a2, base s4, x v0, y v1, z a0;
 * outputs v0, v1, a0, a3, s1, s2 (asm clobbers s1/s2; saves the rest).
 * Asm callers keep a1, a2, t0-t7, f12 and f14 live. */
s32 func_802AA890(s32 count, s32 *offsets, u8 *base, s32 x, s32 y, s32 z, MtxChainRegs *regs) {
    s32 *src;
    s32 *dst;
    u16 *b;
    s64 sum;
    s32 i;
    s32 j;
    s32 k;
    s32 rx;
    s32 ry;
    s32 rz;

    if (count == 0) {
        regs->v1 = regs->s1 >> 11;
        regs->a0 = regs->s2 >> 11;
        return regs->s0 >> 11;
    }
    src = (s32 *) (base + *offsets);
    dst = (s32 *) D_803EBB58;
    for (i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
    b = (u16 *) (src + 16); /* the asm's a3 ends past the first matrix */
    offsets++;
    count--;
    while (count != 0) {
        b = (u16 *) (base + *offsets);
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                sum = 0;
                for (k = 0; k < 4; k++) {
                    sum += (s64) MTX_FIX(D_803EBB58, i, k) * (s64) MTX_FIX(b, k, j);
                }
                sum >>= 16;
                D_803EBB98[16 + i * 4 + j] = sum;
                D_803EBB98[i * 4 + j] = (u32) sum >> 16;
            }
        }
        src = (s32 *) D_803EBB98;
        dst = (s32 *) D_803EBB58;
        for (i = 0; i < 16; i++) {
            dst[i] = src[i];
        }
        offsets++;
        count--;
    }
    rx = MTX_FIX(D_803EBB58, 0, 0) * x + MTX_FIX(D_803EBB58, 1, 0) * y + MTX_FIX(D_803EBB58, 2, 0) * z +
         (D_803EBB58[12] << 16);
    ry = MTX_FIX(D_803EBB58, 0, 1) * x + MTX_FIX(D_803EBB58, 1, 1) * y + MTX_FIX(D_803EBB58, 2, 1) * z +
         MTX_FIX(D_803EBB58, 3, 1);
    rz = MTX_FIX(D_803EBB58, 0, 2) * x + MTX_FIX(D_803EBB58, 1, 2) * y + MTX_FIX(D_803EBB58, 2, 2) * z +
         MTX_FIX(D_803EBB58, 3, 2);
    regs->a3 = (s32) b;
    regs->s1 = ry;
    regs->s2 = rz;
    regs->v1 = ry >> 11;
    regs->a0 = rz >> 11;
    return rx >> 11;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA890.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (D_803EBBEC and D_803EBDB0 are declared at the top of the file.) */

/* Transform desc[0] triangles (3 x {s16 x, y, z}, 0x14 bytes each, after the
 * header {u16 n, u16 count, s32 offsets[count]}) through the matrix chain
 * (func_802AA890 with count/offsets/base) into D_803EBDB0 records tagged
 * `id`: each triangle reuses the next record with that id at or after the
 * current one, else takes a new one at the end (*D_803EBBEC grows). A record
 * holds the three transformed points as words at +0, the source points as
 * halves at +0x24. Returns the remaining count (always 0, the asm's t7) and
 * leaves the source pointer past the last triangle in *vertsOut.
 * Register convention: id t3, desc t4, base s4; func_802AA890's v1/a3/s1/s2
 * pass through `regs`; outputs t7 (return value), s3 (*vertsOut). The asm
 * also clobbers s0 (= the record, which func_802AA890 reads only for a count
 * of 0). Asm callers keep f12 and f14 live. */
s32 func_802AABE4(s32 id, u16 *desc, u8 *base, MtxChainRegs *regs, s16 **vertsOut) {
    s32 count = desc[1];
    s32 *offsets = (s32 *) (desc + 2);
    s16 *v = (s16 *) ((u8 *) desc + count * 4 + 4);
    u8 *end = D_803EBBEC;
    u8 *slot = D_803EBDB0;
    s32 n = desc[0];
    s32 k;

    while (n != 0) {
        n--;
        while (slot != end && *(u16 *) (slot + 0x36) != id) {
            slot += 0x38;
        }
        if (slot == end) {
            end += 0x38;
        }
        *(u16 *) (slot + 0x36) = id;
        for (k = 0; k < 3; k++) {
            *(s16 *) (slot + 0x24 + k * 6) = v[k * 3 + 0];
            *(s16 *) (slot + 0x26 + k * 6) = v[k * 3 + 1];
            *(s16 *) (slot + 0x28 + k * 6) = v[k * 3 + 2];
            regs->s0 = (s32) slot;
            *(s32 *) (slot + k * 12) =
                func_802AA890(count, offsets, base, v[k * 3 + 0], v[k * 3 + 1], v[k * 3 + 2], regs);
            *(s32 *) (slot + k * 12 + 4) = regs->v1;
            *(s32 *) (slot + k * 12 + 8) = regs->a0;
        }
        v += 10;
        slot += 0x38;
    }
    D_803EBBEC = end;
    *vertsOut = v;
    return n;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AABE4.s")
#endif


/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* o32 entry for C callers (48D00, 4B5E0): the value pair at (x, z) on the
 * D_803EBDB0 triangle `id` (func_802AAD0C) is stored as halfwords to *uOut
 * and *wOut. The asm passes the caller's f24 through to func_802AAD0C, where
 * it only feeds the FP side results, and doesn't save the f20-f28 that
 * func_802AAD0C changes (conventions.txt: clobbers); those results are
 * dropped here, so f24 starts as 0. id is used as a full word (the C
 * callers declare it u8). */
void func_802AACD4(s32 id, s32 x, s32 z, s16 *uOut, s16 *wOut) {
    InterpRegs r;

    r.f24 = 0.0f;
    func_802AAD0C(id, x, z, &r);
    *uOut = r.t3;
    *wOut = r.t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AACD4.s")
#endif

#ifdef NON_MATCHING
void func_802AAF64(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, s32 u1, s32 w1, s32 u2,
                   s32 w2, InterpRegs *r);

#define REC_S32(rec, off) (*(s32 *) ((rec) + (off)))
#define REC_S16(rec, off) (*(s16 *) ((rec) + (off)))

/* Shared body of func_802AAD0C / func_802AAE54: find, from the start of the
 * 0x38-byte records D_803EBDB0, the first record with u16 id (+0x36) == id
 * whose triangle contains (x, z) by func_802AA5E0 (bounding box) and
 * func_802AA460 (edge sides); the search has no end check (such a record
 * must exist). Each record carries two coordinate sets: s32 words at
 * +0/+8, +0xC/+0x14, +0x18/+0x20 and s16 halves at +0x24/+0x28,
 * +0x2A/+0x2E, +0x30/+0x34. `byHalves` picks the s16 set as the triangle
 * (and the s32 set as the interpolated values), else the reverse. Then
 * func_802AAF64 interpolates the values at (x, z) into r->t3/t4, starting
 * from corner 0's value pair; r's FP fields start as func_802AA460's FP
 * results (f12 = pz, f14 = cross bits, f20 = cz, f22 = side, f24 = sideZ,
 * f26 = dz; f24 is also an input, r->f24), which func_802AAF64 overwrites
 * unless the point is a corner. */
static void port_tri_interp(s32 id, s32 x, s32 z, InterpRegs *r, s32 byHalves) {
    u8 *rec = D_803EBDB0 - 0x38;
    TriSideOut f;
    s32 c[6];
    s32 u[6];
    s32 i;

    f.sideZ = r->f24;
    for (;;) {
        do {
            rec += 0x38;
        } while (*(u16 *) (rec + 0x36) != id);
        if (byHalves) {
            c[0] = REC_S16(rec, 0x24);
            c[1] = REC_S16(rec, 0x28);
            c[2] = REC_S16(rec, 0x2A);
            c[3] = REC_S16(rec, 0x2E);
            c[4] = REC_S16(rec, 0x30);
            c[5] = REC_S16(rec, 0x34);
        } else {
            c[0] = REC_S32(rec, 0x00);
            c[1] = REC_S32(rec, 0x08);
            c[2] = REC_S32(rec, 0x0C);
            c[3] = REC_S32(rec, 0x14);
            c[4] = REC_S32(rec, 0x18);
            c[5] = REC_S32(rec, 0x20);
        }
        if (func_802AA5E0(x, z, c[0], c[1], c[2], c[3], c[4], c[5]) == 0) {
            continue;
        }
        if (func_802AA460(x, z, c[0], c[1], c[2], c[3], c[4], c[5], &f) != 0) {
            break;
        }
    }
    if (byHalves) {
        u[0] = REC_S32(rec, 0x00);
        u[1] = REC_S32(rec, 0x08);
        u[2] = REC_S32(rec, 0x0C);
        u[3] = REC_S32(rec, 0x14);
        u[4] = REC_S32(rec, 0x18);
        u[5] = REC_S32(rec, 0x20);
    } else {
        u[0] = REC_S16(rec, 0x24);
        u[1] = REC_S16(rec, 0x28);
        u[2] = REC_S16(rec, 0x2A);
        u[3] = REC_S16(rec, 0x2E);
        u[4] = REC_S16(rec, 0x30);
        u[5] = REC_S16(rec, 0x34);
    }
    r->t3 = u[0];
    r->t4 = u[1];
    r->f12 = f.pz;
    {
        /* f14 holds the float's raw bits (a union, not *(s32 *) &f.cross:
         * that read is type punning gcc treats as uninitialised) */
        union {
            f32 f;
            s32 i;
        } bits;
        bits.f = f.cross;
        r->f14 = bits.i;
    }
    r->f20 = f.cz;
    r->f22 = f.side;
    r->f24 = f.sideZ;
    r->f26 = f.dz;
    func_802AAF64(x, z, c[0], c[1], c[2], c[3], c[4], c[5], u[2], u[3], u[4], u[5], r);
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Value pair at (x, z) on the D_803EBDB0 triangle with id `id`, by its s32
 * coordinates; values from its s16 fields (see port_tri_interp).
 * Register convention: id a3, x t0, z t1, f24 in; results t3, t4 and
 * f12-f26 through r. The asm saves everything else it touches (f28 is
 * clobbered by func_802AA460). Asm callers keep a3, t0, t1 (and t2) live (a
 * mixed N64 build would need a thunk). */
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r) {
    port_tri_interp(id, x, z, r, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAD0C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802AACD4 with func_802AAE54 (triangle from the record's s16
 * coordinates, values from its s32 words), stored as words. The C callers
 * declare it s32 and pass s16 coordinates, but ignore the result; the asm
 * doesn't set v0. */
void func_802AAE1C(s32 id, s32 x, s32 z, s32 *uOut, s32 *wOut) {
    InterpRegs r;

    r.f24 = 0.0f;
    func_802AAE54(id, x, z, &r);
    *uOut = r.t3;
    *wOut = r.t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE1C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802AAD0C with the roles swapped: the triangle is the record's s16
 * coordinates, the values its s32 words. Same register convention; asm
 * callers keep t2 (func_802AAE1C) or a2, a3, t7 (func_802AB9A4) live. */
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r) {
    port_tri_interp(id, x, z, r, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
f32 func_802AB1B0(s32 t0, s32 s7, s32 v1, s32 a1, s32 t9, s32 t1, s32 a0, f32 *f20Out);
f32 func_802AB234(s32 s7, s32 t0, s32 v0, s32 a0, s32 t1, s32 t9, s32 a1, f32 *f20Out);
f32 func_802AB2B8(s32 t1, s32 t9, s32 a1, s32 v1, s32 s7, s32 t0, s32 v0, f32 *f20Out);
f32 func_802AB33C(s32 t9, s32 t1, s32 a0, s32 v0, s32 t0, s32 s7, s32 v1, f32 *f20Out);

/* (InterpRegs is declared above func_802AAD0C.) */

/* Interpolate the value pair (u, w) at the point (x, z) of the triangle with
 * corners (x0, z0), (x1, z1), (x2, z2) carrying (r->t3, r->t4), (u1, w1),
 * (u2, w2): if the point is corner 0 nothing changes, at corner 1 or 2 its
 * pair is copied. Otherwise the line from corner 0 through the point meets
 * edge 1-2 at parameter t along it (func_802AB1B0/234/2B8/33C handle the
 * axis-aligned cases, inline 64-bit products and single-precision math the
 * general one) with k the point's position on that line, and
 * u = round((edge value - u0 * k) / (1 - k)), likewise w. Every result
 * (and the FP intermediates the asm callers read) goes to `r`; on the corner
 * paths the FP fields keep their old values.
 * Register convention: x t0, z t1, corners s1/s3, s4/s6, s7/t9, (u1, w1) =
 * t5/t6, (u2, w2) = t7/s0, (u0, w0) = t3/t4 in/out, plus f12, f14, f20, f22,
 * f24, f26 out (in/out). The asm clobbers s2 and uses trapping `sub` for all
 * the differences. A degenerate triangle divides by 0 and the cvt.w.s faults
 * (NaN/inf) in the asm. */
void func_802AAF64(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, s32 u1, s32 w1, s32 u2,
                   s32 w2, InterpRegs *r) {
    s32 dx0;
    s32 dx12;
    s32 dz0;
    s32 dz12;
    f32 k;
    f32 t;
    f32 fdx0;
    f32 den;
    f32 b;
    f32 eu;
    f32 ew;
    f32 omk;

    if (x == x0 && z == z0) {
        return;
    }
    if (x == x1 && z == z1) {
        r->t3 = u1;
        r->t4 = w1;
        return;
    }
    if (x == x2 && z == z2) {
        r->t3 = u2;
        r->t4 = w2;
        return;
    }
    dx0 = x0 - x;
    dx12 = x1 - x2;
    dz0 = z0 - z;
    dz12 = z1 - z2;
    if (dx0 == 0) {
        k = func_802AB1B0(x, x2, dx12, dz12, z2, z, dz0, &t);
    } else if (dx12 == 0) {
        k = func_802AB234(x2, x, dx0, dz0, z, z2, dz12, &t);
    } else if (dz0 == 0) {
        k = func_802AB2B8(z, z2, dz12, dx12, x2, x, dx0, &t);
    } else if (dz12 == 0) {
        k = func_802AB33C(z2, z, dz0, dx0, x, x2, dx12, &t);
    } else {
        fdx0 = (f32) dx0;
        b = (f32) ((s64) dz12 * dx0);
        den = 1.0f - (f32) ((s64) dx12 * dz0) / b;
        k = (f32) ((s64) dx12 * z) / b - (f32) ((s64) dx12 * z2) / b;
        k = k + (f32) x2 / fdx0;
        k = k - (f32) x / fdx0;
        k = k / den;
        t = (fdx0 * k + (f32) x - (f32) x2) / (f32) dx12;
    }
    omk = 1.0f - k;
    eu = (f32) (u1 - u2) * t + (f32) u2;
    ew = (f32) (w1 - w2) * t + (f32) w2;
    r->t3 = port_cvt_w_s((eu - (f32) r->t3 * k) / omk);
    r->t4 = port_cvt_w_s((ew - (f32) r->t4 * k) / omk);
    r->f12 = omk;
    r->f14 = r->t4;
    r->f20 = t;
    r->f22 = (f32) dx0 * k + (f32) x;
    r->f24 = (f32) dz0 * k + (f32) z;
    r->f26 = (f32) z;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAF64.s")
#endif

/* func_802AB1B0 / func_802AB234 / func_802AB2B8 / func_802AB33C: four
 * variants of one line-intersection step used by func_802AAF64. Each computes
 * a slope k = (f32) (p - q) / (f32) d and then
 * t = ((f32) m * k + (f32) c - (f32) e) / (f32) n, returning k and t in f10/f20
 * (which one is which differs per variant). The C versions return f10 and
 * store f20 through `f20Out`; register mappings are in conventions.txt. The
 * asm saves every integer register it reads and clobbers f0 and f2; asm
 * callers keep a0, t0, t1 and t3-t7 live. `p - q` is a trapping `sub`. */

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* f20 = (t0 - s7) / v1; f10 = (a1 * f20 + t9 - t1) / a0. */
f32 func_802AB1B0(s32 t0, s32 s7, s32 v1, s32 a1, s32 t9, s32 t1, s32 a0, f32 *f20Out) {
    f32 k = (f32) (t0 - s7) / (f32) v1;

    *f20Out = k;
    return ((f32) a1 * k + (f32) t9 - (f32) t1) / (f32) a0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB1B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* f10 = (s7 - t0) / v0; f20 = (a0 * f10 + t1 - t9) / a1. */
f32 func_802AB234(s32 s7, s32 t0, s32 v0, s32 a0, s32 t1, s32 t9, s32 a1, f32 *f20Out) {
    f32 k = (f32) (s7 - t0) / (f32) v0;

    *f20Out = ((f32) a0 * k + (f32) t1 - (f32) t9) / (f32) a1;
    return k;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB234.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* f20 = (t1 - t9) / a1; f10 = (v1 * f20 + s7 - t0) / v0. */
f32 func_802AB2B8(s32 t1, s32 t9, s32 a1, s32 v1, s32 s7, s32 t0, s32 v0, f32 *f20Out) {
    f32 k = (f32) (t1 - t9) / (f32) a1;

    *f20Out = k;
    return ((f32) v1 * k + (f32) s7 - (f32) t0) / (f32) v0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB2B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* f10 = (t9 - t1) / a0; f20 = (v0 * f10 + t0 - s7) / v1. */
f32 func_802AB33C(s32 t9, s32 t1, s32 a0, s32 v0, s32 t0, s32 s7, s32 v1, f32 *f20Out) {
    f32 k = (f32) (t9 - t1) / (f32) a0;

    *f20Out = ((f32) v0 * k + (f32) t0 - (f32) s7) / (f32) v1;
    return k;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB33C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 4-byte entries {u8 id, u8 flag, u8 pad[2]}, terminated by a 0xFFFFFFFF word. */

/* Looks up `id` in the D_803ED3B8 list. Returns 1 if the first entry with
 * that id has a nonzero flag byte, 0 if the flag is 0 or the id is absent.
 * (The asm also saves/restores t0/t1; its only caller is C, 23C20.) */
s32 func_802AB3C0(s32 id) {
    u8 *entry;

    for (entry = D_803ED3B8; *(s32 *) entry != -1; entry += 4) {
        if (entry[0] == id) {
            return entry[1] != 0;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB3C0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 1 if some D_803ED3B8 entry (scanning to the -1 terminator) has byte 1 ==
 * key1 and byte 0 == key0, else 0.
 * Register convention: key0 in t4, key1 in t8, result in a2 (the asm saves
 * t0, t1). Asm callers keep a0, a1, a3, t2-t8, f12 and f14 live. */
s32 func_802AB41C(s32 key0, s32 key1) {
    u8 *entry;

    for (entry = D_803ED3B8; *(s32 *) entry != -1; entry += 4) {
        if (entry[1] == key1 && entry[0] == key0) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB41C.s")
#endif

#ifdef NON_MATCHING
/* Per-type hooks of the D_803ED3B8 tree walks below (vehicle files). */
void func_802AB50C(s32 id, InterpRegs *r);
void func_802AB714(s32 id, s32 *s3, InterpRegs *r);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Entry from C (00000.c func_8024AE2C, which declares a u8 parameter; the asm
 * moves all of a0 to s2): func_802AB50C(id). The asm saves every callee-saved
 * register around the call, so the walk's register results are dropped; its
 * FP state starts as whatever the C caller left in f20-f26 (f24 is an input
 * of the per-type hooks, where it only feeds FP side results that nothing
 * stores): 0 here. */
void func_802AB478(s32 id) {
    InterpRegs r;

    r.f20 = 0.0f;
    r.f22 = 0.0f;
    r.f24 = 0.0f;
    r.f26 = 0.0f;
    func_802AB50C(id, &r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB478.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Walk the D_803ED3B8 tree (4-byte entries {u8 type, u8 parent, ..}, scanned
 * from the start to the -1 word) below node `id`: for every entry whose
 * parent byte is `id`, run its type's hook with that parent id (3:
 * func_802B30F4, 4: func_802B4818, 5: func_802B6100, 8: func_802B78F4, 9:
 * func_802C59B4, 0xA: func_802CA34C, 0xD: func_802CBD5C, 0xE: func_802CCED4,
 * 0xF: func_802CFC54; their results are ignored) and then recurse on the type
 * byte as the new id. Type 0 and other types are skipped without recursing.
 * The type is the byte read before the hook (the asm keeps it in t1, which
 * the hooks preserve; they may rewrite D_803ED3B8). An entry whose type
 * equals its parent recurses forever in the asm too.
 * Register convention: id in s2; the hooks' FP state chains through r (f24
 * in, f20-f26 out; conventions.txt). The asm saves a2, a3, t0, t1 and s2 and
 * leaves the hooks' gp, s4 and f28 changed. */
void func_802AB50C(s32 id, InterpRegs *r) {
    u8 *e;
    s32 type;

    for (e = D_803ED3B8; *(s32 *) e != -1; e += 4) {
        if (e[1] != id) {
            continue;
        }
        type = e[0];
        switch (type) {
            case 3:
                func_802B30F4(e[1], r);
                break;
            case 5:
                func_802B6100(e[1], r);
                break;
            case 4:
                func_802B4818(e[1], r);
                break;
            case 8:
                func_802B78F4(e[1], r);
                break;
            case 0xD:
                func_802CBD5C(e[1], r);
                break;
            case 0xE:
                func_802CCED4(e[1], r);
                break;
            case 0xF:
                func_802CFC54(e[1], r);
                break;
            case 9:
                func_802C59B4(e[1], r);
                break;
            case 0xA:
                func_802CA34C(e[1], r);
                break;
            default: /* 0 and unknown types */
                continue;
        }
        func_802AB50C(type, r);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB50C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Entry from C (00000.c func_8024B5E8): func_802AB714(id). The asm saves
 * every callee-saved register (gp and fp too) around the call. The walk's s3
 * and FP state start as whatever the C caller left in s3 and f20-f26; the
 * hooks only pass s3 through (func_802C5A14 / func_802CA3D8 hand it to
 * func_802A8768's in/out slot result) and f24 only feeds FP side results,
 * none of which is stored: 0 here. */
void func_802AB670(s32 id) {
    InterpRegs r;
    s32 s3 = 0;

    r.f20 = 0.0f;
    r.f22 = 0.0f;
    r.f24 = 0.0f;
    r.f26 = 0.0f;
    func_802AB714(id, &s3, &r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB670.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802AB50C with the second set of hooks (3: func_802B3180, 4:
 * func_802B48A4, 5: func_802B618C, 8: func_802B7980, 9: func_802C5A14, 0xA:
 * func_802CA3D8, 0xD: func_802CBDE8, 0xE: func_802CCF60, 0xF: func_802CFCE0),
 * and entries whose parent byte is 0 are skipped (so id 0 matches nothing).
 * Register convention: id in a3; s3 in/out (*s3: the hooks' s3 result, or
 * their in/out s3) and the FP state through r (f24 in; f20-f26 out). The
 * asm saves a3, t0, t1, t2; the hooks leave s0-s2, s4-s7, fp, gp and f28
 * changed, and func_802B7980 also s3 (not modelled: s3 only passes through
 * to func_802A8768's result, which nothing stores). */
void func_802AB714(s32 id, s32 *s3, InterpRegs *r) {
    u8 *e;
    s32 type;

    for (e = D_803ED3B8; *(s32 *) e != -1; e += 4) {
        if (e[1] != id || e[1] == 0) {
            continue;
        }
        type = e[0];
        switch (type) {
            case 3:
                *s3 = func_802B3180(id, r);
                break;
            case 5:
                *s3 = func_802B618C(id, (TriSideOut *) &r->f12);
                break;
            case 4:
                *s3 = func_802B48A4(id, r);
                break;
            case 8:
                func_802B7980(id, &r->f24);
                break;
            case 0xD:
                *s3 = func_802CBDE8(id, (TriSideOut *) &r->f12);
                break;
            case 0xE:
                *s3 = func_802CCF60(id, (TriSideOut *) &r->f12);
                break;
            case 0xF:
                *s3 = func_802CFCE0(id, r);
                break;
            case 9:
                func_802C5A14(id, s3, r);
                break;
            case 0xA:
                func_802CA3D8(id, s3, r);
                break;
            default: /* 0 and unknown types */
                continue;
        }
        func_802AB714(type, s3, r);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB714.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802AB8D8(s32 id);

/* Wrapper: returns func_802AB8D8(id). 00000.c declares it taking a u8; the
 * asm passes all of a0 through, so this takes s32. */
s32 func_802AB878(s32 id) {
    return func_802AB8D8(id);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB878.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (D_803ED3B8 is declared above, with func_802AB3C0.) */

/* Returns 1 if `id` is in bytes 1-3 of a D_803ED3B8 entry, scanning up to the
 * -1 terminator; an entry whose bytes 1-3 all equal `id` instead recurses on
 * its byte 0 (when nonzero), and counts only if that finds a match.
 * Register convention: the asm takes id in s2 and returns the result in t0,
 * keeping t1-t4 and s3 but leaving s2 changed on a recursive hit
 * (tools_port/conventions.txt); this C is plain o32. The asm recursion
 * relies on t1 surviving its own call, which C handles itself. */
s32 func_802AB8D8(s32 id) {
    u8 *entry;

    for (entry = D_803ED3B8; *(s32 *) entry != -1; entry += 4) {
        if (entry[1] == id) {
            if (entry[2] != id || entry[3] != id) {
                return 1;
            }
            if (entry[0] != 0 && func_802AB8D8(entry[0]) == 1) {
                return 1;
            }
        } else if (entry[2] == id || entry[3] == id) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB8D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803ED3E8; /* scratch heading for func_802A94A4 */

#define WRAP_12(v) ((v) >= 0x1000 ? (v) - 0xFFF : (v))
#define ABS_B9A4(v) ((v) < 0 ? -(v) : (v))

/* Steer toward a point on triangle `id`: B = func_802AAE54(id, x1, z1) and
 * A = func_802AAE54(id, x2, z2) (interpolated value pairs; r's f24 chains
 * through both). d = func_802ABB1C(A, offset of tbl[0] rotated by *angle, B);
 * when d > 0x400 it is retried with the heading turned by 0x400 (d += 0x400)
 * and, if then > 0x800, by 0x800 (d += 0x800), the turned heading going
 * through D_803ED3E8 (wrapped as h >= 0x1000 ? h - 0xFFF : h). Then the two
 * headings *angle + d (wrapped the same way) and *angle - d (+ 0xFFF when
 * negative) are tried: each rotated offset is added to B and its distance
 * |B + off - A| (|dx| + |dz|) taken; returns the + heading when its distance
 * is strictly smaller, else the - heading, and the - heading's distance in
 * *s3Out. D_803ED3E8 is left = the - heading. The asm's add/sub/neg trap.
 * Register convention: tbl v1, x1 t0, angle a2, id a3, z1 t1, x2 s1, z2 s2;
 * results t0 (return value) and s3 (*s3Out); r: f24 in, f24/f26 out (r's
 * other fields are scratch). The asm clobbers s0-s2, s4-s6, fp, f20, f22;
 * asm callers keep a3 live. */
s32 func_802AB9A4(s16 *tbl, s32 x1, u16 *angle, s32 id, s32 z1, s32 x2, s32 z2, s32 *s3Out, InterpRegs *r) {
    s32 ax;
    s32 az;
    s32 bx;
    s32 bz;
    s32 dx;
    s32 dz;
    s32 d;
    s32 h;
    s32 a0;
    s32 plus;
    s32 minus;
    s32 d1;
    s32 d2;
    s32 e1;
    s32 e2;

    func_802AAE54(id, x1, z1, r);
    bx = r->t3;
    bz = r->t4;
    func_802AAE54(id, x2, z2, r);
    ax = r->t3;
    az = r->t4;
    dx = func_802A94A4(0, tbl, (s16 *) angle, &dz);
    d = func_802ABB1C(ax, az, dx, dz, bx, bz);
    if (d >= 0x401) {
        h = *angle + 0x400;
        D_803ED3E8 = WRAP_12(h);
        dx = func_802A94A4(0, tbl, &D_803ED3E8, &dz);
        d = func_802ABB1C(ax, az, dx, dz, bx, bz) + 0x400;
        if (d >= 0x801) {
            h = *angle + 0x800;
            D_803ED3E8 = WRAP_12(h);
            dx = func_802A94A4(0, tbl, &D_803ED3E8, &dz);
            d = func_802ABB1C(ax, az, dx, dz, bx, bz) + 0x800;
        }
    }
    a0 = *angle;
    plus = a0 + d;
    plus = WRAP_12(plus);
    D_803ED3E8 = plus;
    dx = func_802A94A4(0, tbl, &D_803ED3E8, &dz);
    minus = a0 - d;
    d1 = dx + bx;
    d2 = dz + bz;
    if (minus < 0) {
        minus += 0xFFF;
    }
    D_803ED3E8 = minus;
    dx = func_802A94A4(0, tbl, &D_803ED3E8, &dz);
    e1 = dx + bx - ax;
    e2 = dz + bz - az;
    d1 -= ax;
    d2 -= az;
    d1 = ABS_B9A4(d1) + ABS_B9A4(d2);
    e1 = ABS_B9A4(e1) + ABS_B9A4(e2);
    *s3Out = e1;
    if (!(d1 < e1)) {
        plus = minus;
    }
    return plus;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB9A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
float sqrtf(float);
#pragma intrinsic(sqrtf)

/* Angle at point A = (ax, az) between A->B (B = (bx, bz)) and the offset
 * (dx, dz) from B: d0 = |A - B|, d1 = |B + (dx, dz) - A| (64-bit squares,
 * cvt.s.l, sqrt.s), sine = round(d1 / 2 / d0 * 65536) clamped to 0xFFFF,
 * then func_802AD7FC (arcsine) >> 3 (logical). The differences and sums
 * wrap at 32 bits. d0 = 0 makes the rounding invalid (keep A != B).
 * Register convention: ax t3, az t4, dx t5, dz t6, bx t7, bz s0; result in
 * s6; the asm restores v1 and clobbers s1-s4 and fp (conventions.txt). Asm
 * callers keep a2, t3-t7 live (a mixed N64 build would need a thunk; the
 * native port won't). */
s32 func_802ABB1C(s32 ax, s32 az, s32 dx, s32 dz, s32 bx, s32 bz) {
    s64 u = (s32) ((u32) ax - (u32) bx);
    s64 w = (s32) ((u32) az - (u32) bz);
    s64 p = (s32) ((u32) bx + (u32) dx - (u32) ax);
    s64 q = (s32) ((u32) bz + (u32) dz - (u32) az);
    f32 d0 = sqrtf((f32) (u * u + w * w));
    f32 d1 = sqrtf((f32) (p * p + q * q));
    s32 sine = port_cvt_w_s(d1 / 2.0f / d0 * 65536.0f);

    if (sine >= 0x10000) {
        sine = 0xFFFF;
    }
    return (u32) func_802AD7FC(sine) >> 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABB1C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Transform a list of points into D_803EBC10 records tagged `id`: start at the
 * first record with id byte +0xC == id (or the -1 end marker), then for each
 * entry {s16 x, y, z; u16 count; s32 offsets[count]} between p and end
 * (exclusive) store func_802AA890(count, offsets, base, x, y, z) and its y/z
 * results as the record's three words, tag it, and move to the next record.
 * Register convention: id t0, p t1, end t2, base s4; the func_802AA890
 * registers v1, a0, a3, s1, s2 (and s0 for a count of 0) pass through
 * `regs` (asm callers read them after the call). The asm saves a1, a2, t3,
 * t4; asm callers keep t0, t2, t4, t6, t7, f12 and f14 live. */
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs) {
    u8 *rec = D_803EBC10;
    s32 count;

    while (*(s32 *) rec != -1 && rec[0xC] != id) {
        rec += 0x10;
    }
    while (p != end) {
        count = (u16) p[3];
        *(s32 *) (rec + 0) = func_802AA890(count, (s32 *) (p + 4), base, p[0], p[1], p[2], regs);
        *(s32 *) (rec + 4) = regs->v1;
        *(s32 *) (rec + 8) = regs->a0;
        rec[0xC] = id;
        p = (s16 *) ((u8 *) p + count * 4 + 8);
        rec += 0x10;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABBEC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Find the first 0x10-byte D_803EBC10 record whose byte +0xC is `id` (no
 * terminator: one must exist), step (n - 1) records further, and return the
 * record through `recOut` and (n - 1) * 0x10 as the result.
 * Register convention: id in v0, n in v1; outputs v1 (here the return value)
 * and a0 (*recOut). The asm saves a1. Asm callers keep a1-a3, t0, t3, t6,
 * t7, f12 and f14 live. */
s32 func_802ABC88(s32 id, s32 n, u8 **recOut) {
    u8 *rec = D_803EBC10;

    while (rec[0xC] != id) {
        rec += 0x10;
    }
    n--;
    if (n != 0) {
        n *= 0x10;
        rec += n;
    }
    *recOut = rec;
    return n;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABC88.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
f64 sqrt(f64);
#pragma intrinsic(sqrt)

/* Rounded 3-D distance between (ax, ay, az) and (bx, by, bz): each difference
 * is a wrapping 32-bit subtraction, the squares and their sum are 64-bit
 * (wrapping), then sqrt in double and round to nearest even (the asm's
 * cvt.d.l / sqrt.d / cvt.l.d under the default FCSR); the low 32 bits are
 * returned. A sum that wraps negative gives NaN, where the VR4300 traps in
 * cvt.l.d. The Python model tools_port/models/func_802ABCDC.py does the same.
 * Register convention: inputs t3, t4, t5, t6, t7, s0; result in s1 (the asm
 * restores t6, t7, s0). Asm callers keep a0-a3, t0, t1, t3-t7, f12 and f14
 * live. */
s32 func_802ABCDC(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz) {
    s64 dx = (s32) ((u32) bx - (u32) ax);
    s64 dy = (s32) ((u32) by - (u32) ay);
    s64 dz = (s32) ((u32) bz - (u32) az);
    f64 d = sqrt((f64) (dx * dx + dy * dy + dz * dz));
    u32 r = d; /* d < 2^32: truncation fits */
    f64 frac = d - r;

    if (frac > 0.5 || (frac == 0.5 && (r & 1))) {
        r++;
    }
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABCDC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING


/* Find the first zone (D_803BDFD8 .. *D_803BDFD4, 0x24 bytes) within whose
 * radius the point (x, y, z) lies (distance by func_802ABCDC), whose byte
 * +0x12 is set and whose id list (byte +0x13 = length, ids from +0x15)
 * contains `id`. Its level: with byte +0x14 == 0, 0xFF if byte +0x10 == 1, else
 * fading from 0xFF at the centre to D_80364A6E at the radius; with byte +0x14
 * = L != 0, L if byte +0x10 == 1, else fading from L to D_80364A6E. No zone:
 * D_80364A6E. The level goes to +0x60 of the D_80364460 record whose word
 * +0x5C is `id` (one must exist). Returns *D_803BDFD4.
 * Register convention: id a3, point t3/t4/t5; outputs v1 (return value) and
 * t6, t7, s0-s4 (through `r`, unchanged if no zone is looked at). The asm
 * clobbers s5, s6; asm callers keep a3 live. The fades divide by the radius
 * (unsigned; a zero radius traps). */
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r) {
    u8 *end = D_803BDFD4;
    u8 *zone = D_803BDFD8;
    s32 t6 = r->t6;
    s32 t7 = r->t7;
    s32 s0 = r->s0;
    s32 s1 = r->s1;
    s32 s2 = r->s2;
    s32 s3 = r->s3;
    s32 s4 = r->s4;
    s32 level;
    s32 k;
    u8 *rec;

    for (; zone != end; zone += 0x24) {
        t6 = ((s32 *) zone)[0];
        t7 = ((s32 *) zone)[1];
        s0 = ((s32 *) zone)[2];
        s1 = func_802ABCDC(x, y, z, t6, t7, s0);
        s2 = ((s32 *) zone)[3];
        if (s2 < s1) {
            continue;
        }
        s3 = zone[0x12];
        if (s3 == 0) {
            continue;
        }
        s3 = zone[0x13];
        s4 = (s32) (zone + 0x15);
        while (s3 != 0) {
            if (id == *(u8 *) s4) {
                break;
            }
            s4++;
            s3--;
        }
        if (s3 == 0) {
            continue;
        }
        s3 = zone[0x14];
        if (s3 == 0) {
            s3 = zone[0x10];
            if (s3 == 1) {
                level = 0xFF;
            } else {
                k = 0xFF - D_80364A6E;
                s1 = (u32) (k * s1) / (u32) s2;
                level = k - s1 + D_80364A6E;
            }
        } else {
            s4 = zone[0x10];
            if (s4 == 1) {
                level = s3;
            } else {
                k = D_80364A6E - s3;
                s1 = (u32) (k * s1) / (u32) s2;
                level = s3 + s1;
            }
        }
        goto found;
    }
    level = D_80364A6E;
found:
    rec = D_80364460;
    while (*(s32 *) (rec + 0x5C) != id) {
        rec += 0x74;
    }
    *(s32 *) (rec + 0x60) = level;
    r->t6 = t6;
    r->t7 = t7;
    r->s0 = s0;
    r->s1 = s1;
    r->s2 = s2;
    r->s3 = s3;
    r->s4 = s4;
    return (s32) end;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABD54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802ABFC8(s32 x, s32 z, s32 y, u8 *obj, s32 *found, TriSideOut *f);

/* 1 if (x, z) at height y is over some object's own ground: for each object
 * record D_803F4030 .. D_803F7654 (read once, walked with `!=`), its info's
 * s16 box at info + info->unk20 (x0, z0, x1, z1, << 5) is tested with
 * func_802AA5E0 as the triangle (x0, z0), (x1, z1), (x1, z0) and, failing
 * that, (x0, z0), (x1, z1), (x0, z1); on a hit func_802ABFC8(x, z, y, info)
 * scans its triangles and sets the found flag (and D_803EBBF8). Returns the
 * flag (13A70.c declares the function u8; the asm returns the word).
 * The asm leaves f20-f28 as func_802ABFC8 leaves them (FP results of the
 * scans, passing the caller's f24 in) although its only caller is C; the C
 * keeps them. Its v1 is func_802ABFC8's last result (unused by the caller). */
s32 func_802ABEDC(s32 x, s32 y, s32 z) {
    u8 *end = D_803F7654;
    u8 *e;
    s32 found = 0;
    TriSideOut f;

    f.sideZ = 0.0f;
    for (e = D_803F4030; e != end; e += 0xFC) {
        u8 *info = *(u8 **) e;
        s16 *box = (s16 *) (info + *(s32 *) (info + 0x20));
        s32 x0 = box[0] << 5;
        s32 z0 = box[1] << 5;
        s32 x1 = box[2] << 5;
        s32 z1 = box[3] << 5;

        if (func_802AA5E0(x, z, x0, z0, x1, z1, x1, z0) != 0
            || func_802AA5E0(x, z, x0, z0, x1, z1, x0, z1) != 0) {
            func_802ABFC8(x, z, y, info, &found, &f);
        }
    }
    return found;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABEDC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Highest-below ground in an object's own triangle list (obj + its words
 * +0x24 .. +0x28; 0x14-byte triangles as in func_802AA094, skipped when byte
 * +0x13 is set): among triangles containing (x, z) whose height h <= y, the
 * one with the smallest y - h (the later one on a tie: the asm skips only when
 * best < d). Each such hit sets *found = 1 and D_803EBBF8 = h. Returns the
 * best y - h (0x5F5E0FF if none).
 * Register convention: x t0, z t1, y t2, obj s0; results v1 (return value),
 * a1 (*found, in/out) and f24/f26 through f (in/out). The asm clobbers s1-s7,
 * t8, t9, fp, f20, f22 and f28; asm callers keep t0-t2, t6 and t7 live. */
s32 func_802ABFC8(s32 x, s32 z, s32 y, u8 *obj, s32 *found, TriSideOut *f) {
    u8 *rec = obj + *(s32 *) (obj + 0x24);
    u8 *end = obj + *(s32 *) (obj + 0x28);
    u32 bestD = 0x5F5E0FF;
    s32 v[9];
    s32 h;
    s32 d;
    s32 a3;
    s32 t6;

    for (; rec != end; rec += 0x14) {
        if (rec[0x13] != 0) {
            continue;
        }
        PORT_TRI_S16(v, rec);
        if (!port_tri_height(x, z, v, f, &h, &a3, &t6)) {
            continue;
        }
        d = y - h;
        if (d < 0) {
            continue;
        }
        if (bestD < (u32) d) {
            continue;
        }
        *found = 1;
        D_803EBBF8 = h;
        bestD = d;
    }
    return bestD;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABFC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING


/* func_802A9DC0's nearest-height scan over D_803BDAF4 .. D_803BDAF8: each
 * nearer-or-equal hit sets the found flag and D_803EBBFC = h. Returns the
 * best |y - h| (0x5F5E0FF if none).
 * Register convention: x t0, z t1, y t2; results v1 (return value), a1, a3,
 * t6, t7, fp, s1-s4 through r (a3, t6, s1-s4 in/out) and the FP results
 * through f (in/out). The asm clobbers s5-s7, t8, t9 and f28; `neg` traps on
 * 0x80000000. */
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r) {
    u8 *rec = D_803BDAF4;
    u8 *end = D_803BDAF8;
    u32 bestD = 0x5F5E0FF;
    s32 v[9];
    s32 h;
    s32 d;

    r->a1 = 0;
    for (; rec != end; rec += 0x14) {
        PORT_TRI_S16(v, rec);
        r->s1 = v[0];
        r->s2 = v[1];
        r->s3 = v[2];
        r->s4 = v[3];
        if (!port_tri_height(x, z, v, f, &h, &r->a3, &r->t6)) {
            continue;
        }
        d = y - h;
        if (d < 0) {
            d = -d;
        }
        if (bestD < (u32) d) {
            continue;
        }
        r->a1 = 1;
        D_803EBBFC = h;
        bestD = d;
    }
    r->t7 = (s32) end;
    r->fp = (s32) end;
    return bestD;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AC0BC.s")
#endif
