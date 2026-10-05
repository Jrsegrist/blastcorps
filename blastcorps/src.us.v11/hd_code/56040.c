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
/* Shared types for the NON_MATCHING (port) rewrites below. Register-keyed
 * functions take their register inputs as ordinary parameters; the mapping is
 * recorded per function in tools_port/conventions.txt. */

/* 0x18-byte animation-channel entry (the -1-terminated table D_803B35F8 and
 * the 0x20-entry blocks built by func_8029F85C). */
typedef struct {
    /* 0x00 */ s32 id;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s8 unk10;
    /* 0x11 */ s8 unk11;
    /* 0x12 */ s8 unk12;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[2];
} Unk8029DEA0Entry; /* size 0x18 */

/* 12-byte pending-copy slot of the 120-entry pool D_803B7FC8. */
typedef struct {
    /* 0x00 */ u8 *src;  /* 0 = free */
    /* 0x04 */ u8 *dst;
    /* 0x08 */ s32 parity;
} Unk8029DCD4Slot;

extern Unk8029DEA0Entry D_803B35F8[];
extern u8 D_803B9890[]; /* 0x60-byte records */
extern u8 *D_803BD300;  /* end of the records in use */

/* The hand asm keeps $gp pointing at D_803EEA90 (set in 71140's
 * func_802B5F04 and friends) and reads fields of it as gp+off; the rewrites
 * read them directly. */
extern u8 D_803EEA90[];
#define GP_U8(off) (*(u8 *) (D_803EEA90 + (off)))
#define GP_S16(off) (*(s16 *) (D_803EEA90 + (off)))

f32 sqrtf(f32);

/* The asm converts float -> s64 with cvt.l.s, which rounds by the FCSR mode
 * (round to nearest, ties to even, as the game runs) rather than truncating
 * like a C cast. This reproduces that: truncate, then fix up from the exact
 * fractional part. */
static s64 port_cvt_l_s(f32 x) {
    s64 t = (s64) x;
    f32 frac = x - (f32) t;

    if (frac > 0.5f || (frac == 0.5f && (t & 1))) {
        t++;
    } else if (frac < -0.5f || (frac == -0.5f && (t & 1))) {
        t--;
    }
    return t;
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A800.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410;
extern u16 D_803A7412;
extern s32 D_80358064;
extern u8 D_803A742D;
extern u8 D_803A742E;

/* Per-frame update of gp+0xA0 (a byte: set to min(D_803A742D, 200) while
 * D_803A742E is set, else counted down to 1) and of the s16 timer gp+0x76
 * (set to 10 when zero and D_80358064 is set). While the timer is negative
 * the ring span D_803A7410/D_803A7412 is shifted by 0x800 each way (A down,
 * B up), wrapping by 0xFFF.
 * gp is D_803EEA90 in the asm (read here directly). The asm saves a1, t0,
 * t1, t3, t6; asm callers keep a3, t6, f12, f14 live (no thunk needed in the
 * native port). */
void func_8029A914(void) {
    s32 v;

    if (D_803A742E != 0) {
        v = D_803A742D;
        if (v >= 0xC9) {
            v = 0xC8;
        }
        GP_U8(0xA0) = v;
    } else {
        v = GP_U8(0xA0);
        if (v != 1) {
            GP_U8(0xA0) = v - 1;
        }
    }
    if (GP_S16(0x76) == 0 && D_80358064 != 0) {
        GP_S16(0x76) = 10;
    }
    if (GP_S16(0x76) < 0) {
        v = D_803A7410 - 0x800;
        if (v < 0) {
            v += 0xFFF;
        }
        D_803A7410 = v;
        v = D_803A7412 + 0x800;
        if (v > 0xFFF) {
            v -= 0xFFF;
        }
        D_803A7412 = v;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A914.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AA10.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AB88.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B02C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B514.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803A742B;
extern u8 D_803A7430;

/* If the record's byte 0x4F is 7: sets D_803A742B and counts D_803A7430 up
 * (while it is <= 200).
 * Register convention: asm takes rec in s0 and restores v0 (conventions.txt);
 * its asm callers keep a0-a3, t3-t6, t8, f12, f14 live. */
void func_8029B5B8(u8 *rec) {
    if (rec[0x4F] == 7) {
        D_803A742B = 1;
        if (D_803A7430 < 0xC9) {
            D_803A7430 = D_803A7430 + 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B5B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B614.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */
extern s32 D_80358064;
extern u8 D_803A742C;
extern u8 D_803A742D;
extern u8 D_803A742E;
extern u8 D_803A742F;
s32 func_8029B930(void);

/* Widens the ring span [D_803A7410, D_803A7412] to take in indices a and b
 * (each first wrapped into 0..0xFFE by one +-0xFFF step, as in the asm).
 * If the span is still the "empty" 0/0xFFF pair it is simply set to a/b.
 * Otherwise B moves back to b when b is at or before B, and A moves on to a
 * when a is after A, comparing 12-bit ring differences (<<20, signed).
 * If the span got longer (func_8029B930 before vs after) and D_80358064 is
 * set, D_803A742F is set, and if D_803A742C is set and D_803A742E clear,
 * D_803A742D steps 1 -> 8, otherwise +1, and D_803A742E is set.
 * Asm callers rely on preserved: func_8029B614 keeps f12, f14;
 * func_8029CB04 keeps t1; func_802CE204 keeps t1, t3-t6, t8, f12, f14
 * (the asm saves t0). The native port doesn't need a thunk. */
void func_8029B7CC(s32 a, s32 b) {
    s32 before;
    u32 ca;
    u32 cb;

    if (a < 0) {
        a += 0xFFF;
    }
    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    if (b < 0) {
        b += 0xFFF;
    }
    if (b >= 0x1000) {
        b -= 0xFFF;
    }
    before = func_8029B930();
    ca = D_803A7410;
    cb = D_803A7412;
    if (ca == 0 && cb == 0xFFF) {
        D_803A7410 = a;
        D_803A7412 = b;
        return;
    }
    ca <<= 20;
    cb <<= 20;
    if ((s32) (((u32) b << 20) - cb) <= 0) {
        cb = (u32) b << 20;
    }
    if ((s32) (((u32) a << 20) - ca) > 0) {
        ca = (u32) a << 20;
    }
    D_803A7410 = ca >> 20;
    D_803A7412 = cb >> 20;
    if (before < func_8029B930() && D_80358064 != 0) {
        D_803A742F = 1;
        if (D_803A742C != 0 && D_803A742E == 0) {
            D_803A742D = (D_803A742D == 1) ? 8 : D_803A742D + 1;
            D_803A742E = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B7CC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */

/* Distance from index A forward to index B in the 0x1000-entry ring, with
 * the asm's quirks kept: the wrapped case uses 0xFFF (not 0x1000), and a
 * non-wrapped distance above 0x800 has 0x800 subtracted. Called from C
 * (48D00, 4B5E0) and from asm func_8029B7CC.
 * Register note: the asm saves/restores v1 and a0 and touches nothing but
 * v0/at; func_8029B7CC keeps a0, a1 and t0 live across the call. This C
 * version is plain o32, so a mixed N64 build would need a thunk preserving
 * those for the asm caller; the native port does not. */
s32 func_8029B930(void) {
    s32 a = D_803A7410;
    s32 b = D_803A7412;

    if (b < a) {
        return b + (0xFFF - a);
    }
    b -= a;
    if (b > 0x800) {
        b -= 0x800;
    }
    return b;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B930.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B994.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BB28.s")

/* func_8029C160, func_8029C0DC, func_8029BF64, func_8029BD0C, func_8029BEE4:
 * the sphere-vs-triangle test the collision loops (func_8029AB88,
 * func_8029B02C, 77E20, 89250) run in that order for each triangle `tri`
 * (s0) against a sphere at (t3, t4, t5) with radius t6:
 *   C160  sphere centre within r of the triangle's plane? (projects the
 *         centre onto it: hit point in v1, a0, a1 and D_803A73FC..7404)
 *   C0DC  drop one axis (tri byte 0x4E) from the vertices and hit point
 *   BF64  2-D point-in-triangle test of the projected hit point (t7)
 *   BD0C  else: does any edge pass within r of the centre? (t7)
 *   BEE4  else: is vertex 0 within r? (t7)
 * Triangle layout: s64 plane normal at +0 / +8 / +0x10, s64 d at +0x18,
 * f32 at +0x20 and +0x24, s32 vertices at +0x28, +0x34, +0x40, s8 axis at
 * +0x4E. Register conventions in conventions.txt; the outputs the survey
 * lists beyond these (leftover v0-a3, t0-t2, f12-f28, s2-s4) are dead in
 * every asm caller (overwritten before use or only saved/restored).
 * The asm used raw dmult/cvt.s.l/cvt.l.s; the C uses s64 arithmetic and
 * port_cvt_l_s for the round-to-nearest conversions. */
#ifdef NON_MATCHING
/* Segment-vs-sphere: for the triangle edges V0V1, V0V2, V1V2 (in that
 * order), solves |A + t(B - A) - C|^2 = r^2 with 64-bit coefficients
 * a = |B - A|^2, b = 2 (B - A).(A - C), c = |A - C|^2 - r^2; if the
 * discriminant b^2 - 4ac (64-bit, wrapping) is >= 0 and either root
 * (-b + sqrt) / 2a or (-b - sqrt) / 2a (single precision) lies in [0, 1]
 * (a NaN root counts too, as in the asm), returns 1; else 0.
 * Register convention: x, y, z, r in t3-t6 and tri in s0, result in t7;
 * s2-s4 are scratch (the asm also leaves v0-a3, t0-t2 changed). The asm's
 * trapping dadd/dsub fault on 64-bit overflow where this C wraps. */
s32 func_8029BD0C(s32 x, s32 y, s32 z, s32 r, u8 *tri) {
    s32 *v = (s32 *) (tri + 0x28);
    s32 *a;
    s32 *b;
    s32 i;

    for (i = 3; i != 0; i--) {
        s64 ax;
        s64 ay;
        s64 az;
        s64 dx;
        s64 dy;
        s64 dz;
        s64 qa;
        s64 qb;
        s64 qc;
        s64 disc;
        f32 sq;
        f32 nb;
        f32 den;
        f32 t;

        if (i == 3) {
            a = v + 0, b = v + 3;
        } else if (i == 2) {
            a = v + 0, b = v + 6;
        } else {
            a = v + 3, b = v + 6;
        }
        ax = a[0] - x;
        ay = a[1] - y;
        az = a[2] - z;
        dx = b[0] - a[0];
        dy = b[1] - a[1];
        dz = b[2] - a[2];
        qa = dx * dx + dy * dy + dz * dz;
        qb = (dx * ax + dy * ay + dz * az) * 2;
        qc = ax * ax + ay * ay + az * az - (s64) r * r;
        disc = qb * qb - ((qa * qc) << 2);
        if (disc < 0) {
            continue;
        }
        sq = sqrtf((f32) disc);
        nb = (f32) -qb;
        den = (f32) (qa * 2);
        t = (nb + sq) / den;
        if (t < 0.0f) {
        } else if (t > 1.0f) {
        } else {
            return 1;
        }
        t = (nb - sq) / den;
        if (t < 0.0f) {
            continue;
        }
        if (t > 1.0f) {
            continue;
        }
        return 1;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BD0C.s")
#endif

#ifdef NON_MATCHING
/* Vertex-vs-sphere: 1 unless r < round(sqrtf(|V0 - C|^2)) (64-bit squares,
 * round to nearest as cvt.l.s), i.e. 1 when vertex 0 is within r.
 * Register convention: x, y, z, r in t3-t6 and tri in s0, result in t7. */
s32 func_8029BEE4(s32 x, s32 y, s32 z, s32 r, u8 *tri) {
    s32 *v = (s32 *) (tri + 0x28);
    s64 dx = v[0] - x;
    s64 dy = v[1] - y;
    s64 dz = v[2] - z;
    s64 d = port_cvt_l_s(sqrtf((f32) (dx * dx + dy * dy + dz * dz)));

    if ((s64) r < d) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BEE4.s")
#endif

#ifdef NON_MATCHING
/* 2-D point-in-triangle: triangle A (au, av), B (bu, bv), C (cu, cv),
 * point P (pu, pv). An interior reference point M = ((A + B) / 2 + C) / 2 is
 * computed in single precision; for the edges AB, AC, BC (in that order) the
 * 2-D cross products of P and of M against the edge are compared: returns 0
 * as soon as P lies strictly on the other side from M (P exactly on an edge
 * line skips that edge), else 1.
 * Register convention: bu, bv, cu, cv in a0-a3, au, av, pu, pv in v0, v1,
 * t0, t1, result in t7 (the asm saves t3-t6 and changes f12-f28). */
s32 func_8029BF64(s32 bu, s32 bv, s32 cu, s32 cv, s32 au, s32 av, s32 pu, s32 pv) {
    f32 mu = ((f32) cu + (f32) (au + bu) / 2.0f) / 2.0f;
    f32 mv = ((f32) cv + (f32) (av + bv) / 2.0f) / 2.0f;
    f32 fpu = pu;
    f32 fpv = pv;
    s32 i;

    for (i = 3; i != 0; i--) {
        f32 eu;
        f32 ev;
        f32 du;
        f32 dv;
        f32 cp;
        f32 cm;

        if (i == 3) {
            eu = au, ev = av;
            dv = bv - av, du = bu - au;
        } else if (i == 2) {
            eu = au, ev = av;
            dv = cv - av, du = cu - au;
        } else {
            eu = bu, ev = bv;
            dv = cv - bv, du = cu - bu;
        }
        cp = (fpu - eu) * dv - (fpv - ev) * du;
        if (cp == 0.0f) {
            continue;
        }
        cm = (mu - eu) * dv - (mv - ev) * du;
        if (cp > 0.0f) {
            if (cm > 0.0f) {
                continue;
            }
            return 0;
        }
        if (cm < 0.0f) {
            continue;
        }
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BF64.s")
#endif

#ifdef NON_MATCHING
/* Projects the triangle and the hit point (px, py, pz) to 2-D by dropping
 * one axis chosen by the s8 at tri+0x4E: 0 drops z, 1 drops y, anything else
 * drops x. out[0..7] = A.u, A.v, B.u, B.v, C.u, C.v, P.u, P.v (the asm's
 * v0, v1, a0, a1, a2, a3, t0, t1, which func_8029BF64 takes).
 * Register convention: tri in s0, px, py, pz in v1, a0, a1 (left there by
 * func_8029C160). */
void func_8029C0DC(u8 *tri, s32 px, s32 py, s32 pz, s32 *out) {
    s32 *v = (s32 *) (tri + 0x28);
    s32 axis = (s8) tri[0x4E];

    if (axis == 0) {
        out[6] = px, out[7] = py;
        out[0] = v[0], out[1] = v[1];
        out[2] = v[3], out[3] = v[4];
        out[4] = v[6], out[5] = v[7];
    } else if (axis == 1) {
        out[6] = px, out[7] = pz;
        out[0] = v[0], out[1] = v[2];
        out[2] = v[3], out[3] = v[5];
        out[4] = v[6], out[5] = v[8];
    } else {
        out[6] = py, out[7] = pz;
        out[0] = v[1], out[1] = v[2];
        out[2] = v[4], out[3] = v[5];
        out[4] = v[7], out[5] = v[8];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C0DC.s")
#endif

#ifdef NON_MATCHING
extern s32 D_803A73FC;
extern s32 D_803A7400;
extern s32 D_803A7404;

/* Plane test: dist = round((n . C + d) / f32 at +0x20) with 64-bit n . C; if
 * |dist| > r returns 0, else projects C onto the plane:
 * t = (f32)(-d - n . C) / f32 at +0x24, hit = C + round(n * t) per axis,
 * stored to D_803A73FC/7400/7404 and to hit[0..2], and returns 1.
 * hit[] mirrors the asm's v1, a0, a1: on a miss they keep the low words of
 * n.y and n.z and the incoming a1 (dead values; no caller reads them then).
 * Register convention: x, y, z, r in t3-t6, tri in s0; result in v0, hit
 * point in v1, a0, a1; s2/s3 are scratch (the asm saves s1). */
s32 func_8029C160(s32 x, s32 y, s32 z, s32 r, u8 *tri, s32 *hit) {
    s64 nx = *(s64 *) (tri + 0x00);
    s64 ny = *(s64 *) (tri + 0x08);
    s64 nz = *(s64 *) (tri + 0x10);
    s64 d = *(s64 *) (tri + 0x18);
    s64 dot = nx * x + ny * y + nz * z;
    s64 dist = port_cvt_l_s((f32) (dot + d) / *(f32 *) (tri + 0x20));
    f32 t;

    if (dist < 0) {
        dist = -dist;
    }
    if ((s64) r < dist) {
        hit[0] = ny;
        hit[1] = nz;
        return 0;
    }
    t = (f32) (-d - dot) / *(f32 *) (tri + 0x24);
    hit[0] = D_803A73FC = port_cvt_l_s(t * (f32) nx) + x;
    hit[1] = D_803A7400 = port_cvt_l_s(t * (f32) ny) + y;
    hit[2] = D_803A7404 = port_cvt_l_s(t * (f32) nz) + z;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C160.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803BE724;
extern s32 D_803BE728;
extern s16 D_803BE72C;
extern s16 D_803A7418[];

/* Grid cell of (x, z): D_803A7418[0] = (x*4) / D_803BE724 +
 * ((z*4) / D_803BE728) * D_803BE72C (signed divisions), and the list is
 * ended with D_803A7418[1] = -1.
 * Register convention: asm takes x in t3 and z in t5 (conventions.txt) and
 * uses s0/s1 as scratch without saving them; asm callers keep a0-a3, t3-t6,
 * t8, f12, f14 live. */
void func_8029C284(s32 x, s32 z) {
    D_803A7418[0] = (x * 4) / D_803BE724 + ((z * 4) / D_803BE728) * D_803BE72C;
    D_803A7418[1] = -1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C284.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803A7300[]; /* 0x14-byte entries, free when byte 0x11 == 0xFF */
extern u8 D_803A6B30[]; /* 0x14-byte entries, free when byte 0x13 == 0xFF */

#define SCALE16(h, scale) (((u32) ((h) << 5) * (u32) (scale)) >> 16)

/* Registers an object's parts: the first free D_803A7300 entry gets
 * word 0xC = SCALE16(*(u16 *) p), byte 0x10 = tag, byte 0x11 = 0. Then for
 * each variable-size record from p + 4 up to `end` (next = rec + 0xC +
 * 4 * u16 at rec+0xA; the walk stops only at exactly `end`), consecutive
 * D_803A6B30 entries from the first free one get byte 0x12 = tag, 0x13 = 0,
 * half 0x10 = u16 at rec+8, word 0xC = SCALE16(u16 at rec+6). SCALE16 is
 * ((h << 5) * scale) >> 16 in unsigned 32-bit arithmetic.
 * Register convention: asm takes tag/p/end/scale in t0/t1/t2/t3
 * (conventions.txt); it saves t6, t7, s0 and changes t1, t4, t5; asm callers
 * keep f12, f14 live. */
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale) {
    u8 *e = D_803A7300;

    while ((s8) e[0x11] != -1) {
        e += 0x14;
    }
    *(u32 *) (e + 0xC) = SCALE16(*(u16 *) p, scale);
    e[0x10] = tag;
    e[0x11] = 0;

    e = D_803A6B30;
    p += 4;
    while ((s8) e[0x13] != -1) {
        e += 0x14;
    }
    while (p != end) {
        e[0x12] = tag;
        e[0x13] = 0;
        *(s16 *) (e + 0x10) = *(u16 *) (p + 8);
        *(u32 *) (e + 0xC) = SCALE16(*(u16 *) (p + 6), scale);
        p += (*(u16 *) (p + 0xA) << 2) + 0xC;
        e += 0x14;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C354.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C454.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C52C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C5EC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Register outputs of func_8029C6E4 (s0-s3 are in/out: left alone unless
 * the asm loads them). */
typedef struct {
    /* 0x00 */ s32 s0; /* entry word 0 */
    /* 0x04 */ s32 s1; /* entry word 4 */
    /* 0x08 */ s32 s2; /* entry word 8 */
    /* 0x0C */ s32 s3; /* entry word 0xC of the last kind-6 entry looked at */
    /* 0x10 */ s32 s4; /* 1 = found */
    /* 0x14 */ u8 *t6; /* entry where the scan stopped */
    /* 0x18 */ s32 t7; /* last byte compared */
} Unk8029C6E4Out;

/* Scans D_803A6B30 (0x14-byte entries, ended by byte 0x13 == 0xFF) for the
 * first entry with byte 0x12 == 6 and word 0xC == 0x3BD; if found, o->s4 = 1
 * and o->s0..s2 = its words 0, 4, 8, else o->s4 = 0. o->s3 receives word 0xC
 * of every kind-6 entry checked, o->t6/t7 where the scan stopped and the last
 * byte read, as the asm leaves them in registers.
 * Register convention: outputs in s0-s4, t6, t7 (s0-s3 pass through when not
 * loaded); conventions.txt. Asm callers keep a3, f12, f14 live. */
void func_8029C6E4(Unk8029C6E4Out *o) {
    u8 *e = D_803A6B30;

    o->s4 = 0;
    for (;;) {
        o->t7 = (s8) e[0x13];
        if (o->t7 == -1) {
            break;
        }
        o->t7 = e[0x12];
        if (o->t7 == 6) {
            o->s3 = *(s32 *) (e + 0xC);
            if (o->s3 == 0x3BD) {
                o->s0 = *(s32 *) (e + 0x0);
                o->s1 = *(s32 *) (e + 0x4);
                o->s2 = *(s32 *) (e + 0x8);
                o->s4 = 1;
                break;
            }
        }
        e += 0x14;
    }
    o->t6 = e;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C6E4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C748.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C828.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C914.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C9D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CB04.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CD54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM.
 * Port note: not rewritten. It computes t1 = t1 / gp[0xA0] (divu) unless
 * s1 == 0xFF or the byte is 1, but func_8029CB04 branches straight into its
 * epilogue (.L8029CF40), so a C version would leave that label undefined in
 * the NON_MATCHING link. Port it together with func_8029CB04. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF04.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sphere overlap: 1 if |Q - P| < r1 + r2, else 0, with P = (px, py, pz),
 * Q = (qx, qy, qz). The squared distance is summed in 64 bits and converted
 * to float before the square root, as in the asm (cvt.s.l, sqrt.s). The
 * asm's sub/add/dadd trap on overflow where this C wraps; game coordinates
 * stay far below that.
 * Register convention: asm takes pz, r1, qx, qy in a0-a3 and px, py, qz, r2
 * in v0, v1, t0, t1, and returns in t2 (conventions.txt); it restores a1-a3
 * and t0. Asm callers keep a0-a3, t0, t1, t3-t9, f12, f14 live. */
s32 func_8029CFA4(s32 pz, s32 r1, s32 qx, s32 qy, s32 px, s32 py, s32 qz, s32 r2) {
    s64 dx = qx - px;
    s64 dy = qy - py;
    s64 dz = qz - pz;
    f32 r = r1 + r2;

    return sqrtf((f32) (dx * dx + dy * dy + dz * dz)) < r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CFA4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D040.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803BC1D0[]; /* 0xDC-byte groups */
extern u8 *D_803BD304;  /* end of the groups in use */

/* Marks (byte 0x51 = 1) every 0x60-byte record in D_803B9890..D_803BD300
 * whose byte 0x4F is `id`, and every record with byte 0x4F == 0 whose byte
 * 0x50 appears in the member list (count at +0xC7, bytes from +0xC8) of a
 * group in D_803BC1D0..D_803BD304 with byte 0xC4 == id.
 * Register convention: asm takes id in t2 (conventions.txt) and uses s0-s2,
 * s4, s5 and t4-t7 as scratch without saving them; its asm caller
 * func_8029D040 keeps t0-t2, f12, f14 live and reloads the rest. */
void func_8029D120(s32 id) {
    u8 *rec;
    u8 *grp;
    u8 *m;
    s32 n;
    s32 sub;
    s32 kind;

    for (rec = D_803B9890; rec != D_803BD300; rec += 0x60) {
        kind = rec[0x4F];
        if (kind == id) {
            rec[0x51] = 1;
            continue;
        }
        if (kind != 0) {
            continue;
        }
        sub = rec[0x50];
        for (grp = D_803BC1D0; grp != D_803BD304; grp += 0xDC) {
            if (grp[0xC4] != id) {
                continue;
            }
            m = grp + 0xC8;
            for (n = grp[0xC7]; n != 0; n--) {
                if (*m == sub) {
                    break;
                }
                m++;
            }
            if (n != 0) {
                rec[0x51] = 1;
                break;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D120.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Finds the first 0x60-byte record from D_803B9890 with byte 0x4F == id and
 * byte 0x50 == sub (no bound: it must exist), clears its byte 0x51 and returns
 * it; *last gets the last byte compared (== sub).
 * Register convention: asm takes sub in t6 and id in t7 and leaves the record
 * in s0 and the byte in s1 (conventions.txt). Its asm caller func_8029D040
 * keeps a1-a3, t0-t2, t4-t7, f12, f14 live. */
u8 *func_8029D1D4(s32 sub, s32 id, s32 *last) {
    u8 *rec = D_803B9890;

    while (rec[0x4F] != id || rec[0x50] != sub) {
        rec += 0x60;
    }
    rec[0x51] = 0;
    *last = rec[0x50];
    return rec;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D1D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns byte 0x51 of the first 0x60-byte record from D_803B9890 whose byte
 * 0x50 == sub (no bound: it must exist).
 * Register convention: asm takes sub in t4 and returns in t5, saving s0/s1
 * (conventions.txt); asm callers keep t0, t3, t6, t7 live. */
s32 func_8029D210(s32 sub) {
    u8 *rec = D_803B9890;

    while (rec[0x50] != sub) {
        rec += 0x60;
    }
    return rec[0x51];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D210.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D24C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* rec[0x55] = (kind != 7).
 * Register convention: asm takes kind in t2 and rec in s0, restoring v0
 * (conventions.txt); asm callers keep a3, t2, t6, t7, f12, f14 live. */
void func_8029D534(s32 kind, u8 *rec) {
    rec[0x55] = (kind == 7) ? 0 : 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D534.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D56C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Plane of the triangle whose vertices are the s32 triples at tri+0x28,
 * +0x34, +0x40 (each coordinate >> 3, arithmetic): out[0..2] = normal
 * (V0 - V1) x (V0 - V2), out[3] = -(normal . V1), all in 64 bits (the
 * differences are 32-bit). The asm's sub/dsub/dadd trap on overflow where
 * this C wraps (not reached with game coordinates).
 * Register convention: asm takes tri in s0 and returns the four 64-bit
 * values in a1, a2, a3, t0 (conventions.txt maps their low words; its asm
 * caller func_8029D56C uses the full 64 bits). It restores everything else
 * but t5. */
void func_8029D90C(u8 *tri, s64 *out) {
    s32 *w = (s32 *) (tri + 0x28);
    s32 x0 = w[0] >> 3;
    s32 y0 = w[1] >> 3;
    s32 z0 = w[2] >> 3;
    s32 x1 = w[3] >> 3;
    s32 y1 = w[4] >> 3;
    s32 z1 = w[5] >> 3;
    s32 x2 = w[6] >> 3;
    s32 y2 = w[7] >> 3;
    s32 z2 = w[8] >> 3;
    s64 dy01 = y0 - y1;
    s64 dz02 = z0 - z2;
    s64 dz01 = z0 - z1;
    s64 dy02 = y0 - y2;
    s64 dx02 = x0 - x2;
    s64 dx01 = x0 - x1;

    out[0] = dy01 * dz02 - dz01 * dy02;
    out[1] = dz01 * dx02 - dx01 * dz02;
    out[2] = dx01 * dy02 - dy01 * dx02;
    out[3] = -(out[0] * x1 + out[1] * y1 + out[2] * z1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D90C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DA90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 0 if any of the n (index, value) byte pairs at `pairs` has
 * (s8) tbl[index].unk13 == value (value read unsigned, so 0x80..0xFF never
 * match), else 1.
 * Register convention: asm takes n in t5, pairs in s4, tbl in a2 and returns
 * in t4, restoring v0, v1, t2, t6 (t5 is left counted down); conventions.txt.
 * Its asm caller func_8029D040 keeps a1-a3, t0-t2, t6, t7, f12, f14 live. */
s32 func_8029DB7C(s32 n, u8 *pairs, Unk8029DEA0Entry *tbl) {
    while (n != 0) {
        n--;
        if (tbl[pairs[0]].unk13 == pairs[1]) {
            return 0;
        }
        pairs += 2;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DB7C.s")
#endif

/* TODO: func_8029DBF0 - wrapper around func_8029DC14, but that callee
 * reads its real input out of $v0 (set via a delay-slot `or v0,a0,zero`
 * right before the `jal`, not through the normal a0-a3 argument
 * registers) and returns its result in $v1 instead of $v0 - a hand-tuned
 * non-ABI register convention between these two specific functions, not
 * expressible as a normal C function call. Logic (looking at
 * func_8029DC14 directly): search a fixed-stride struct array starting
 * at D_803B9890 up to D_803BD300's current pointer value for a byte field
 * matching the caller's input, returning found/not-found. Needs either
 * inline asm or leaving as GLOBAL_ASM. (The NON_MATCHING port below uses
 * plain o32 for both; eqcheck maps the v0/v1 convention.) */
#ifdef NON_MATCHING
s32 func_8029DC14(s32 id);

/* Wrapper: returns func_8029DC14(id). 39050.c declares it taking a u8; the
 * asm passes all of a0 through, so this takes s32. */
s32 func_8029DBF0(s32 id) {
    return func_8029DC14(id);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DBF0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B9890[]; /* 0x60-byte records */
extern u8 *D_803BD300;  /* end of the records in use */

/* Returns 0 if a record in D_803B9890..D_803BD300 (0x60 bytes each) has
 * byte 0x4F == id and byte 0x51 == 0, else 1. The scan stops only at exactly
 * D_803BD300.
 * Register convention: the asm takes id in v0 and returns the result in v1
 * (tools_port/conventions.txt); this C is plain o32. Its asm caller
 * func_802BC3D0 relies on a3, t0, t2, t4, t6, t7, f12 and f14 surviving the
 * call (a mixed N64 build would need a thunk; the native port doesn't). */
s32 func_8029DC14(s32 id) {
    u8 *rec;

    for (rec = D_803B9890; rec != D_803BD300; rec += 0x60) {
        if (rec[0x4F] == id && rec[0x51] == 0) {
            return 0;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC14.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Unk8029DCD4Slot D_803B7FC8[120];
extern Unk8029DCD4Slot *D_803B8568; /* last slot in use (sits right after D_803B7FC8) */

/* Resets the 120-entry pool D_803B7FC8: the next-free pointer goes back to
 * the start and words 0 and 4 of every 12-byte entry are cleared.
 * Register note: the asm saves/restores v0 and v1 (clobbers at); asm caller
 * func_802A1674 keeps t0, f12 and f14 live across the call (a mixed N64
 * build would need a thunk; the native port doesn't). */
void func_8029DC80(void) {
    s32 i;

    D_803B8568 = D_803B7FC8;
    for (i = 0; i < 120; i++) {
        D_803B7FC8[i].src = NULL;
        D_803B7FC8[i].dst = NULL;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8035805C;

/* Queues a 64-byte copy src -> dst in the first free slot (src == 0) of the
 * 120-entry pool D_803B7FC8, tagged with !D_8035805C (low bit flipped), and
 * raises the last-used pointer D_803B8568 to that slot if it is below it
 * (signed compare). Does nothing if the pool is full.
 * Register convention: asm takes dst in t5 and src in s2 (conventions.txt).
 * Asm callers keep a0-a3, t2, t4, t7, f8, f12, f14 live (the asm saves v0, v1,
 * a0); a mixed N64 build would need a thunk, the native port doesn't. */
void func_8029DCD4(u8 *dst, u8 *src) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    s32 n = 120;

    while (n != 0) {
        n--;
        if (slot->src == NULL) {
            slot->src = src;
            slot->dst = dst;
            slot->parity = D_8035805C ^ 1;
            if ((s32) D_803B8568 < (s32) slot) {
                D_803B8568 = slot;
            }
            return;
        }
        slot++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DCD4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Cancels every queued copy whose dst is `dst` (clears src and dst).
 * Quirk kept from the asm: the scan bound is the ADDRESS of D_803B8568 (which
 * sits right after the 120 slots), not its value, so all 120 slots plus a
 * 121st "slot" overlaying D_803B8568 itself are scanned. If that pseudo-slot
 * matches (word D_803B8568+4 == dst), D_803B8568 and the following word are
 * zeroed and D_803B8568 is then set to the last real slot.
 * Register convention: asm takes dst in s2 (conventions.txt); the asm saves
 * v0, v1, a0, and its asm callers keep a0-a3, t2, t4, t7, f8, f12, f14 live. */
void func_8029DD54(u8 *dst) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    Unk8029DCD4Slot *end = (Unk8029DCD4Slot *) &D_803B8568;

    while ((s32) slot <= (s32) end) {
        if (slot->dst == dst) {
            slot->src = NULL;
            slot->dst = NULL;
            if (slot == end) {
                D_803B8568 = slot - 1;
            }
        }
        slot++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DD54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_8029DE50(u64 *dst, u64 *src);

/* Runs the queued copies: for each used slot of D_803B7FC8 up to D_803B8568
 * (signed compare) whose parity equals D_8035805C, copies 64 bytes src -> dst
 * (func_8029DE50) and frees the slot. Slots with the other parity stay; the
 * last of those becomes the new D_803B8568 (or the pool start if none). */
void func_8029DDC8(void) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    Unk8029DCD4Slot *last = D_803B7FC8;
    Unk8029DCD4Slot *end = D_803B8568;
    s32 parity = D_8035805C;

    while ((s32) slot <= (s32) end) {
        if (slot->src != NULL) {
            if (slot->parity != parity) {
                last = slot;
            } else {
                func_8029DE50((u64 *) slot->dst, (u64 *) slot->src);
                slot->src = NULL;
                slot->dst = NULL;
            }
        }
        slot++;
    }
    D_803B8568 = last;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DDC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies 64 bytes (8 doublewords) src -> dst.
 * Register convention: asm takes dst in t5 and src in s2, and restores both
 * plus v0/v1 (conventions.txt). Its asm caller func_8029DDC8 keeps a0 and a2
 * live; the native port needs no thunk. */
void func_8029DE50(u64 *dst, u64 *src) {
    s32 i;

    for (i = 0; i < 8; i++) {
        dst[i] = src[i];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DE50.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unk8029DEA0Entry is defined in the file-level NON_MATCHING block at the top. */
extern u8 D_803A7440[12][0x1010];
extern void *D_803B35F0;
extern u8 D_803B3500[];
extern Unk8029DEA0Entry D_803B35F8[];
/* List in the 7D9D0 data blob, ended by -1. In the ROM its 12 entries are
 * addresses inside that blob (0x802C2190...), so `id` is really a pointer. */
extern s32 D_802C23B4[];

/* Clears byte 6 of each of the 12 0x1010-byte blocks at D_803A7440, points
 * D_803B35F0 at D_803B3500, then builds D_803B35F8 from the -1-terminated id
 * list D_802C23B4: one 0x18-byte entry per id with every other field (except
 * unk8) zeroed. The terminating -1 is also stored, as the id of the entry
 * after the last.
 * Register note: the asm saves/restores v0, v1 and a0-a3 (clobbers at and
 * f0); asm caller func_802A1674 keeps t0, t6, t9, f12 and f14 live. */
void func_8029DEA0(void) {
    s32 i;
    s32 *src;
    Unk8029DEA0Entry *dst;
    s32 id;

    for (i = 0; i < 12; i++) {
        D_803A7440[i][6] = 0;
    }
    D_803B35F0 = D_803B3500;

    src = D_802C23B4;
    dst = D_803B35F8;
    for (;;) {
        id = *src;
        dst->id = id;
        if (id == -1) {
            break;
        }
        src++;
        dst->unk4 = 0.0f;
        dst->unkC = 0;
        dst->unkE = 0;
        dst->unk10 = 0;
        dst->unk11 = 0;
        dst->unk12 = 0;
        dst->unk13 = 0;
        dst->unk14 = 0;
        dst++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DEA0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* For each entry e of the -1-terminated pointer list D_802C23B4 with
 * e[0] == key and e[2] != 0, takes the texture id t = u16 at e + 4 + 2 * e[1]
 * and records every G_SETTIMG (top byte 0xFD) command in the display list
 * dl..dlEnd (8-byte commands, stops only at exactly dlEnd) whose second word
 * equals t: three words (e, offset of that word from dl, 0) are appended at
 * D_803B35F0, which is advanced. (The asm counts e[2] down but never loops
 * on it, so only the first id of each entry is used.)
 * Register convention: asm takes dl, dlEnd, key in s0, s1, s2 and restores
 * every register it uses (conventions.txt). */
void func_8029DF78(u8 *dl, u8 *dlEnd, s32 key) {
    s32 *out = D_803B35F0;
    s32 *lp = D_802C23B4;
    u8 *e;
    u8 *p;
    u32 tex;

    while ((s32) (e = (u8 *) *lp) != -1) {
        lp++;
        if (e[0] != key || e[2] == 0) {
            continue;
        }
        tex = *(u16 *) (e + e[1] * 2 + 4);
        for (p = dl; p != dlEnd;) {
            u32 w0 = *(u32 *) p;

            p += 8;
            if ((w0 & 0xFF000000) >> 24 == 0xFD && *(u32 *) (p - 4) == tex) {
                out[0] = (s32) e;
                out[1] = p - dl - 4;
                out[2] = 0;
                out += 3;
            }
        }
    }
    D_803B35F0 = out;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DF78.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E0AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E21C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E47C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Finds the first of the 12 0x1010-byte blocks at D_803A7440 with byte 6 set,
 * u16 at +4 == sub and word 0 == key; clears its byte 7 (sic: the asm stores
 * at data - 9) and returns the physical address of its data
 * (block + 0x10 - 0x80000000), else 0.
 * Register convention: asm takes key in t3 and sub in t6 and returns in s1
 * (conventions.txt); it changes a0-a2. Its asm caller func_8029E21C keeps
 * t0, t3, t6, f12, f14 live. */
s32 func_8029E4E4(s32 key, s32 sub) {
    s32 i;

    for (i = 0; i < 12; i++) {
        u8 *b = D_803A7440[i];

        if (b[6] != 0 && *(u16 *) (b + 4) == sub && *(s32 *) b == key) {
            b[7] = 0;
            return (u32) (b + 0x10) - 0x80000000;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E4E4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E558.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E5AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E730.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E878.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E938.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EA48.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EB58.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EC68.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EDEC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EF80.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F060.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F110.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F1BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F3D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F4B8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F560.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F608.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F6B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F760.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F85C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F9D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FC74.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies the ten halfwords just before dst (dst[-10..-1]) to dst[0..9] and
 * returns dst + 10.
 * Register convention: asm takes and returns the pointer in t1, restoring v0
 * (conventions.txt); its asm caller keeps a0, t0, t3, t8, f12, f14 live. */
s16 *func_8029FF2C(s16 *dst) {
    s32 i;

    for (i = 0; i < 10; i++) {
        dst[i] = dst[i - 10];
    }
    return dst + 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FF2C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FFA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0118.s")

/* func_802A0290 .. func_802A04BC: field setters/getter for entry `idx` of an
 * Unk8029DEA0Entry table (entry address = base + idx * 0x18, a signed 32-bit
 * multiply as in the asm). Register convention (conventions.txt): idx in v0,
 * the base in a0 or v1, the value in v1 and a float in f0. All of them restore
 * every register they use; asm callers keep various t-regs and f12/f14 live,
 * which the native port doesn't need a thunk for. */
#ifdef NON_MATCHING
#define ENTRY_AT(base, idx) ((Unk8029DEA0Entry *) ((u8 *) (base) + (idx) * 0x18))

/* unk10 = 1, unkE = val, unkC = 0 (base in a0, idx in v0, val in v1). */
void func_802A0290(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk10 = 1;
    e->unkE = val;
    e->unkC = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0290.s")
#endif

#ifdef NON_MATCHING
/* unk10 = 0 (idx in v0, base in v1). */
void func_802A02E4(s32 idx, Unk8029DEA0Entry *base) {
    ENTRY_AT(base, idx)->unk10 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A02E4.s")
#endif

#ifdef NON_MATCHING
/* unk13 = 0, unk4 = 0.0f (idx in v0, base in v1). */
void func_802A0320(s32 idx, Unk8029DEA0Entry *base) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk13 = 0;
    e->unk4 = 0.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0320.s")
#endif

#ifdef NON_MATCHING
/* unk13 = val, unk4 = f (f in f0, base in a0, idx in v0, val in v1). */
void func_802A0360(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk13 = val;
    e->unk4 = f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0360.s")
#endif

#ifdef NON_MATCHING
/* unk14 = val (base in a0, idx in v0, val in v1). */
void func_802A039C(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk14 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A039C.s")
#endif

#ifdef NON_MATCHING
/* unk11 = val (base in a0, idx in v0, val in v1). */
void func_802A03D4(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk11 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A03D4.s")
#endif

#ifdef NON_MATCHING
/* unk12 = val (base in a0, idx in v0, val in v1). */
void func_802A040C(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk12 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A040C.s")
#endif

#ifdef NON_MATCHING
/* unkC = 0, unkE = val (base in a0, idx in v0, val in v1). */
void func_802A0444(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unkC = 0;
    e->unkE = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0444.s")
#endif

#ifdef NON_MATCHING
/* unk15 = val, unk8 = f (f in f0, base in a0, idx in v0, val in v1). */
void func_802A0480(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk15 = val;
    e->unk8 = f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0480.s")
#endif

#ifdef NON_MATCHING
/* Reads entry idx (idx in v0, base in v1) into out[]: the asm returns
 * v1 = (s8) unk10, a0 = (s8) unk11, a1 = (s8) unk12, a2 = unk14,
 * a3 = (u16) unkC, t0 = (u16) unkE, t1 = (s8) unk13, f0 = unk4; here
 * out[0..6] get those integers in that order and out[7] the float's bits. */
void func_802A04BC(s32 idx, Unk8029DEA0Entry *base, s32 *out) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    out[0] = e->unk10;
    out[1] = e->unk11;
    out[2] = e->unk12;
    out[3] = e->unk14;
    out[4] = (u16) e->unkC;
    out[5] = (u16) e->unkE;
    out[6] = e->unk13;
    ((f32 *) out)[7] = e->unk4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A04BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0508.s")

/* func_802A0540: also part of the func_802A06B4 hidden-$v0-search-key
 * family documented below - saves the incoming $v0 across the call
 * (forwarding it one layer further) and returns that original value,
 * not the callee's. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0540.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0570.s")

/* func_802A05A4/func_802A05D0/func_802A05F8/func_802A0620/func_802A0648
 * (and likely more below): all call func_802A06B4 with no visible
 * arguments, then immediately use BOTH $v0 and $v1 from the return.
 * func_802A06B4 itself reads $v0 as an INPUT (compares it against a
 * table it walks via $v1, with no instruction anywhere setting $v0
 * first) and returns the matching entry's address in both $v0 and $v1 -
 * meaning these wrappers don't actually get two distinct values back,
 * they're each transparently forwarding a search key that some much
 * earlier caller stuffed into $v0, unchanged, through every layer in
 * between. Classic hand-tuned non-ABI register threading for a hot
 * dispatch-table lookup, not expressible as a normal C function call at
 * any layer - would need either inline asm or a hand-maintained
 * register-correct wrapper, not a straight decomp. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05F8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0620.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0648.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0674.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns the D_803B35F8 entry whose id equals `id`. The scan has no bound
 * (the asm doesn't stop at the -1 terminator), so the id must be present.
 * Register convention: asm takes id in v0 and returns the entry in v0
 * (it restores v1 and a0); conventions.txt. */
Unk8029DEA0Entry *func_802A06B4(s32 id) {
    Unk8029DEA0Entry *e = D_803B35F8;

    while (e->id != id) {
        e++;
    }
    return e;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A06B4.s")
#endif
