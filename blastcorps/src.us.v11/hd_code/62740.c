#include "common.h"
#include <ultra64.h>

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

extern u8 D_80367C10;
extern s32 D_80358064;
extern s32 D_80358068;
extern s32 D_803A740C;
extern u16 D_803A7410; /* ring index A (12-bit, see func_8029B930) */
extern u16 D_803A7412; /* ring index B */
extern s8 D_80370C2C;
extern s8 D_80370C2D;
extern s16 D_80370C70;
extern s16 D_80370C72;
extern u8 D_80370C34;
extern u8 D_80370C75;
extern u8 D_803649EE;
extern u8 D_803BE738;
extern s32 D_802E8BDC;
extern s32 D_80305C58[]; /* {key, value, threshold} triples, value 0 ends */
extern u8 D_803EB7A0[];  /* vehicle save buffer: 0x300 + 0xA6 + 3 words */
extern s32 D_803EBBD8[]; /* 0x14-byte swap temp */
extern u8 *D_803EBC00;
extern u8 *D_803EBC04;
extern u8 *D_803EBC08;
extern u8 D_803EBC10[]; /* 0x10-byte records, id byte at +0xC */
extern s32 D_803ED398;
extern s32 D_803ED39C;
extern s32 D_803ED3A0;
extern s32 D_803ED3A8;
extern s32 D_803ED3AC;
extern s32 D_803ED3B0;
extern u8 D_803ED3EA;
extern u8 D_803ED3EB;
extern u8 D_803ED3EE;
extern u8 D_803ED3EF;
extern u8 D_803ED3F2;
extern u8 D_803ED3F7;
extern s16 D_803ED408;
extern u8 D_803ED40A;
extern s8 D_803ED40C;
extern u8 D_803ED40D;
extern u8 D_803ED410;
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
    s32 a = D_803A7410;
    s32 b = D_803A7412;
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
    t = D_803A7410 - 0x800;
    if (t < 0) {
        t += 0xFFF;
    }
    D_803A7410 = t;
    t = D_803A7412 + 0x800;
    if (0xFFF < t) {
        t -= 0xFFF;
    }
    D_803A7412 = t;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A70D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A71DC.s")

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
extern u8 D_8035805C; /* current frame buffer index */

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7834.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A785C.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7B3C.s")

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
    if (D_803ED40C != 0) {
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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7FD8.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A843C.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A860C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8768.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8B10.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8CCC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED3F5;
extern f32 D_803EBBF4;

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A92C8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A93B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A94A4.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9540.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A95A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9710.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A992C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9A60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9B1C.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9DC0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9F24.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA094.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA2E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA460.s")

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

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA6D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA764.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA890.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AABE4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AACD4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAD0C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE1C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAF64.s")

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
extern u8 D_803ED3B8[];

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

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB478.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB50C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB670.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB714.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB9A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABB1C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABBEC.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABD54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABEDC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABFC8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AC0BC.s")
