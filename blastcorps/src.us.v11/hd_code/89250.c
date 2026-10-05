#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x0 */ s32 a3;
    /* 0x4 */ s32 t6;
    /* 0x8 */ s32 s1;
} Io802A6274; /* as in 60F60.c */
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
extern u8 D_802C382C[]; /* a definition inside the 7D9D0 blob */

/* Sets up a D_803C4B70 record (func_802A6274) for the definition D_802C382C
 * with data 0x591C8, type 0, position (x, y, z) << 11 and every other input
 * (tag, words, b35) 0. The asm saves s0-s7 and fp; 48D00.c calls it. */
void func_802CDA10(s32 x, s32 y, s32 z) {
    Io802A6274 io;

    io.a3 = 0;
    io.t6 = 0;
    io.s1 = 0;
    func_802A6274(&io, D_802C382C, 0x591C8, 0, x << 11, y << 11, z << 11, 0, 0, 0, 0, 0, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDA10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
void func_802C18D4(s32 x, s32 y, s32 z, s32 radius, s32 amount);

/* Blast damage (func_802C18D4) at the position D_803A73F0/F4/F8 << 11 with
 * the given radius and amount. The asm uses the whole registers (48D00.c
 * declares both s16) and saves s0-s7, fp. */
void func_802CDAE8(s32 amount, s32 radius) {
    func_802C18D4(D_803A73F0 << 11, D_803A73F4 << 11, D_803A73F8 << 11, radius, amount);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDAE8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDB70.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDC7C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDD74.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDF94.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE0E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
f32 sqrtf(f32);
#pragma intrinsic(sqrtf)
s32 func_802AD7FC(u32 sine);
void func_8029B7CC(s32 a, s32 b);

/* cvt.w.s under the game's FCSR: round to nearest, ties to even. */
static s32 port_cvt_w_s(f32 x) {
    s32 t = (s32) x;
    f32 frac = x - (f32) t;

    if (frac > 0.5f || (frac == 0.5f && (t & 1))) {
        t++;
    } else if (frac < -0.5f || (frac == -0.5f && (t & 1))) {
        t--;
    }
    return t;
}

/* Heading from (x1, z1) to (x2, z2), as in func_8029CB54 (56040.c): 0 when
 * both differences are 0, else (arcsine(round(65536 * leg / dist)) >> 4) +
 * quadrant * 0x400 with dist = sqrtf of the float squares and the quadrant
 * picked by signed compares (x2 >= x1: z2 >= z1 -> leg x2 - x1, else z1 - z2
 * + 0x400; x2 < x1: z2 < z1 -> x1 - x2 + 0x800, else z2 - z1 + 0xC00).
 * Then widens the ring span with func_8029B7CC(angle + 0x400, angle - 0x400).
 * Register convention: x1, z1, x2, z2 in v0, a0, a2, t0 (conventions.txt);
 * the asm saves every register it uses (v0-t0, t7, s0, fp). Asm callers
 * rely on preserved: func_8029C914 keeps a2, a3, t0, t1, t5, t7, t8;
 * func_802CE0E4 keeps a0, a1, t4, t5, t7. */
void func_802CE204(s32 x1, s32 z1, s32 x2, s32 z2) {
    s32 dx = x2 - x1;
    s32 dz = z2 - z1;
    u32 angle = 0;

    if (dx != 0 || dz != 0) {
        f32 fx = dx;
        f32 fz = dz;
        f32 dist = sqrtf(fx * fx + fz * fz);

        if (!(x2 < x1)) {
            if (!(z2 < z1)) {
                angle = (u32) func_802AD7FC(port_cvt_w_s(65536.0f * ((f32) (x2 - x1) / dist))) >> 4;
            } else {
                angle = ((u32) func_802AD7FC(port_cvt_w_s(65536.0f * ((f32) (z1 - z2) / dist))) >> 4) + 0x400;
            }
        } else if (z2 < z1) {
            angle = ((u32) func_802AD7FC(port_cvt_w_s(65536.0f * ((f32) (x1 - x2) / dist))) >> 4) + 0x800;
        } else {
            angle = ((u32) func_802AD7FC(port_cvt_w_s(65536.0f * ((f32) (z2 - z1) / dist))) >> 4) + 0xC00;
        }
    }
    func_8029B7CC(angle + 0x400, angle - 0x400);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE204.s")
#endif

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. */
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
extern void *D_803A7408;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 D_803A7427;
extern u8 D_803A742F;
extern u8 D_80306450[];
s32 func_802A6F6C(void);
void func_802BCBD8(void);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Given a ring index (0..0xFFF), finds which end of the span
 * [D_803A7410, D_803A7412] is nearer in ring distance (|d|, folded as
 * 0xFFF - |d| above 0x800). Nearer to B: step 0x78 back from B (+0xFFF if
 * negative); otherwise step 0x78 on from A (-0xFFF if >= 0x1000). If that
 * candidate lies outside the span (for a wrapped span, B < A: strictly
 * between B and A; otherwise below A or above B), func_802A6F6C's value is
 * returned instead. The asm returns the full 32-bit value; the C callers
 * declare s16. */
s32 func_802CE3B8(s32 idx) {
    s32 a = D_803A7410;
    s32 b = D_803A7412;
    s32 da = a - idx;
    s32 db = b - idx;
    s32 v;

    if (da < 0) {
        da = -da;
    }
    if (da > 0x800) {
        da = 0xFFF - da;
    }
    if (db < 0) {
        db = -db;
    }
    if (db > 0x800) {
        db = 0xFFF - db;
    }
    if (db < da) {
        v = b - 0x78;
        if (v < 0) {
            v += 0xFFF;
        }
    } else {
        v = a + 0x78;
        if (v >= 0x1000) {
            v -= 0xFFF;
        }
    }
    if (b < a) {
        if (v < a && b < v) {
            v = func_802A6F6C();
        }
    } else {
        if (v < a || b < v) {
            v = func_802A6F6C();
        }
    }
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE3B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Starts a new span at position (x, y, z): the ring span becomes the empty
 * 0/0xFFF pair, the position goes to D_803A73F0/F4/F8, the flags
 * D_803A742F/7427/7424/7425 are cleared, D_803A7408 points at D_80306450,
 * then func_802BCBD8 is called. */
void func_802CE4F0(s32 x, s32 y, s32 z) {
    D_803A7410 = 0;
    D_803A7412 = 0xFFF;
    D_803A73F0 = x;
    D_803A73F4 = y;
    D_803A73F8 = z;
    D_803A742F = 0;
    D_803A7427 = 0;
    D_803A7408 = D_80306450;
    D_803A7424 = 0;
    D_803A7425 = 0;
    func_802BCBD8();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE4F0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE5BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE65C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 (in/out) */
    f32 dz;    /* f26 */
} TriSideOut; /* as in 62740.c */
s32 func_802A9DC0(s32 x, s32 z, s32 y, TriSideOut *f, s32 *fpOut);
s32 func_802A9F24(s32 x, s32 z, s32 y, s32 skip, TriSideOut *f, s32 *idOut, s32 *a2Out);
s32 func_802AA094(s32 x, s32 z, s32 y, TriSideOut *f, s32 *t3io, s32 *fpio);
extern u8 D_803F932C;

#define PORT_ABS(d) ((d) < 0 ? -(d) : (d))

/* Ground height under (x, z) for an object at height y: with y' = y + 0x78,
 * takes the nearest-to-y' heights of the D_803F7828 triangles (h1,
 * func_802A9DC0), of the transformed triangles (h2, func_802A9F24, no id
 * skipped) and of the level grid (h3, func_802AA094, which leaves h3 = h2
 * when it finds nothing). With a grid hit, h3 wins unless h2 or h1 is
 * strictly nearer to y'; then (or without a grid hit) h2 wins if strictly
 * nearer than h1, else h1; with no hit at all (h1 and h2 both 0x5F5E0FF) the
 * result is y. D_803F932C = func_802A9F24's id when h2 won, else 0. Returns
 * the height, clamped at 0 from below.
 * The asm's sub/addi trap on overflow (game coordinates don't reach it); it
 * saves s0-s7 and fp, so the fp passed through to func_802A9DC0 /
 * func_802AA094 (*fpOut, only written) and the FP results they leave in
 * f12-f26 are dead here, but it does change f20-f28 (conventions.txt
 * `clobbers`). Called from C (4DA80, 48D00, 4B5E0) and asm func_802A24BC. */
s32 func_802CE6F8(s32 x, s32 z, s32 y) {
    TriSideOut f;
    s32 fp;
    s32 id;
    s32 a2;
    s32 h1;
    s32 h2;
    s32 h3;
    s32 d1;
    s32 d2;
    s32 d3;
    s32 res;

    y += 0x78;
    h1 = func_802A9DC0(x, z, y, &f, &fp);
    h2 = func_802A9F24(x, z, y, 0, &f, &id, &a2);
    h3 = h2;
    if (func_802AA094(x, z, y, &f, &h3, &fp) != 0) {
        d2 = PORT_ABS(h2 - y);
        d3 = PORT_ABS(h3 - y);
        d1 = PORT_ABS(h1 - y);
        if (d2 >= d3 && d1 >= d3) {
            res = h3;
            id = 0;
            goto done;
        }
    } else {
        if (h1 == 0x5F5E0FF && h2 == 0x5F5E0FF) {
            res = y - 0x78;
            id = 0;
            goto done;
        }
        d2 = PORT_ABS(h2 - y);
        d1 = PORT_ABS(h1 - y);
    }
    if (d2 < d1) {
        res = h2;
    } else {
        res = h1;
        id = 0;
    }
done:
    D_803F932C = id;
    if (res < 0) {
        res = 0;
    }
    return res;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE6F8.s")
#endif
