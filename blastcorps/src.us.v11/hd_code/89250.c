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

#ifdef NON_MATCHING
/* The parts of 77E20.c's D_803F4030 object (Unk802C1DD0Entry) and collision
 * triangle (Unk803B9890) that func_802CDB70..func_802CDD74 use. */
typedef struct {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u8 type; /* damage divisor */
} Obj89250Info;

typedef struct {
    /* 0x00 */ u8 pad0[0x51];
    /* 0x51 */ u8 unk51; /* collidable */
    /* 0x52 */ u16 unk52; /* part, 1-based */
    /* 0x54 */ u8 pad54[3];
    /* 0x57 */ u8 unk57; /* part this triangle hangs on */
    /* 0x58 */ u8 pad58[8];
} Tri89250; /* size 0x60 */

typedef struct {
    /* 0x00 */ Obj89250Info *info;
    /* 0x04 */ Tri89250 *start; /* collision triangles */
    /* 0x08 */ Tri89250 *end;
    /* 0x0C */ s32 radius;
    /* 0x10 */ s32 pos[3];
    /* 0x1C */ u8 pad1C[0x14];
    /* 0x30 */ s32 kind;
    /* 0x34 */ u8 pad34[0xB8];
    /* 0xEC */ u8 pct[0x10]; /* per part; 100 = destroyed */
} Obj89250; /* size 0xFC */

extern Obj89250 D_803F4030[];
extern Obj89250 *D_803F7654;
extern u8 D_803A7300[]; /* 0x14-byte vehicle spheres: s32 x, y, z, r, u8 id at 0x10, s8 flag at 0x11 (-1 = end) */
extern u8 D_803A6B30[]; /* 0x14-byte hit spheres: s32 x, y, z, r, u8 id at 0x12, s8 flag at 0x13 (-1 = end) */
extern u16 D_803F932A; /* impact damage */
extern u8 D_803F932D;
extern u8 D_803F932E;
extern s32 D_803649E8;
extern u8 D_80364456;
extern u8 D_802E8BE4; /* screen shake time */
extern s32 D_802E8BE8; /* screen shake size */
extern u16 D_803A7410;
extern u16 D_803A7412;
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 D_803A742F;

s32 func_8029C160(s32 x, s32 y, s32 z, s32 r, u8 *tri, s32 *hit); /* 56040 */
void func_8029C0DC(u8 *tri, s32 px, s32 py, s32 pz, s32 *out);     /* 56040 */
s32 func_8029BF64(s32 bu, s32 bv, s32 cu, s32 cv, s32 au, s32 av, s32 pu, s32 pv); /* 56040 */
s32 func_8029BD0C(s32 x, s32 y, s32 z, s32 r, u8 *tri);           /* 56040 */
s32 func_8029BEE4(s32 x, s32 y, s32 z, s32 r, u8 *tri);           /* 56040 */
s32 func_8029CFA4(s32 pz, s32 r1, s32 qx, s32 qy, s32 px, s32 py, s32 qz, s32 r2); /* 56040 */
s32 func_802BD8C8(void);                                          /* 77E20 */
void func_802BF1F0(Obj89250 *e, s32 id);                          /* 77E20 */
void func_802BF264(Tri89250 *t);                                  /* 77E20 */
void func_802BF384(Obj89250 *e);                                  /* 77E20 */
void func_802BF534(Obj89250 *e);                                  /* 77E20 */
void func_802BF668(Obj89250 *e);                                  /* 77E20 */
void func_802BF898(Obj89250 *e, s32 part, s32 level);             /* 77E20 */
void func_802C09B8(s32 id, Obj89250 *e);                          /* 77E20 */
void func_802C0E8C(s32 id, Obj89250 *e);                          /* 77E20 */
void func_802C1438(Obj89250 *e, s32 index);                       /* 77E20 */
void func_802CE204(s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_802CDC7C(s32 x, s32 y, s32 z, s32 r, Obj89250 *e, s32 hit);
void func_802CDD74(Obj89250 *e, Tri89250 *tri, s32 amount);
s32 func_802CE0E4(s32 x, s32 y, s32 z, s32 r, s32 id);

/* The sphere-vs-triangle test the asm runs as a call chain (see 56040.c,
 * func_8029BD0C): plane distance, projected point-in-triangle, edge, vertex. */
static s32 sphere_hits_tri_89250(s32 x, s32 y, s32 z, s32 r, Tri89250 *tri) {
    s32 hit[3];
    s32 uv[8];

    if (func_8029C160(x, y, z, r, (u8 *) tri, hit) == 0) {
        return 0;
    }
    func_8029C0DC((u8 *) tri, hit[0], hit[1], hit[2], uv);
    if (func_8029BF64(uv[2], uv[3], uv[4], uv[5], uv[0], uv[1], uv[6], uv[7]) != 0) {
        return 1;
    }
    if (func_8029BD0C(x, y, z, r, (u8 *) tri) != 0) {
        return 1;
    }
    return func_8029BEE4(x, y, z, r, (u8 *) tri) != 0;
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Impact of the point D_803A73F0/F4/F8 with radius r against the objects:
 * stores amount (low 16 bits) to D_803F932A, then every object (D_803F4030
 * up to D_803F7654, read once, walked with `!=`) within range (func_8029CFA4)
 * and not a 0x38 object while func_802BD8C8 is set gets the triangle pass
 * func_802CDC7C, which carries a "hit" flag from object to object (the first
 * hit applies the damage). Clears the ring span D_803A7410/7412 when
 * D_803A742F is set. Returns the flag (0/1).
 * ABI apart from f20-f28, which the triangle tests leave changed (the asm
 * doesn't save them). The asm uses whole registers; the C callers declare
 * (s16, s16) (48D00.c, 4DA80.c) or (s16, s32) returning void (4B5E0.c). */
s32 func_802CDB70(s32 r, s32 amount) {
    s32 x = D_803A73F0;
    s32 y = D_803A73F4;
    s32 z = D_803A73F8;
    Obj89250 *end = D_803F7654;
    Obj89250 *o;
    s32 hit = 0;

    D_803F932A = amount;
    for (o = D_803F4030; o != end; o++) {
        if (func_8029CFA4(z, r, o->pos[0], o->pos[1], x, y, o->pos[2], o->radius) == 0) {
            continue;
        }
        if (o->kind == 0x38 && func_802BD8C8() != 0) {
            continue;
        }
        hit = func_802CDC7C(x, y, z, r, o, hit);
    }
    if (D_803A742F != 0) {
        D_803A7410 = 0;
        D_803A7412 = 0;
    }
    return hit;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDB70.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Every collidable triangle of e (start up to end, read once, walked with
 * `!=`) that the sphere (x, y, z, r) >> 2 (arithmetic shifts) hits: on the
 * first hit overall (hit == 0) with a nonzero D_803F932A, applies that damage
 * (func_802CDD74); then the plane-side update func_802BF264(triangle), and
 * hit becomes 1. Returns hit.
 * Register convention: x, y, z, r in v0, v1, a0, a1, e in t3, hit in fp (in
 * and out). The asm saves v0-a2, t3 and t4 and leaves t5/t6 = z/r >> 2, t9 =
 * e, s0/s1 = the triangle range, s2-s4 and f20-f28 changed by the tests;
 * asm caller func_802CDB70 keeps a0, a1, t3 and t4 live across the call (the
 * s4/f24/f26 the survey lists as read there are dead: s4 is restored). */
s32 func_802CDC7C(s32 x, s32 y, s32 z, s32 r, Obj89250 *e, s32 hit) {
    Tri89250 *t = e->start;
    Tri89250 *end = e->end;

    x >>= 2;
    y >>= 2;
    z >>= 2;
    r >>= 2;
    for (; t != end; t++) {
        if (t->unk51 == 0) {
            continue;
        }
        if (!sphere_hits_tri_89250(x, y, z, r, t)) {
            continue;
        }
        if (hit == 0) {
            hit = D_803F932A;
            if (hit != 0) {
                func_802CDD74(e, t, hit);
            }
        }
        func_802BF264(t);
        hit = 1;
    }
    return hit;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDC7C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Impact damage to the part of triangle tri of object e: screen shake 10/200;
 * unless e is a 0x38 object, the part's percentage += amount / e->info->type
 * (unsigned; the asm traps (break 7) on type 0), capped at 100, and
 * func_802BF898 updates the part. A part reaching 100 collapses
 * (func_802BF1F0, func_802C1438, func_802C09B8, func_802C0E8C,
 * func_802BF384; its triangles off and the triangles whose unk57 is the part
 * on unless their own part is done; func_802BF668, func_802BF534).
 * Register convention: e in t9, tri in s0, amount in fp; the asm saves every
 * register (asm caller func_802CDC7C keeps a1, t3-t6 and t9 live). */
void func_802CDD74(Obj89250 *e, Tri89250 *tri, s32 amount) {
    u8 *pcts = e->pct;
    s32 part;
    s32 level;

    D_802E8BE4 = 10;
    D_802E8BE8 = 200;
    if (e->kind == 0x38) {
        return;
    }
    part = tri->unk52;
    level = pcts[part - 1] + (u32) amount / e->info->type;
    if (level >= 100) {
        level = 100;
    }
    pcts[part - 1] = level;
    func_802BF898(e, part, level);
    if (level == 100) {
        Tri89250 *t;
        Tri89250 *end;

        func_802BF1F0(e, part);
        func_802C1438(e, part);
        func_802C09B8(part, e);
        func_802C0E8C(part, e);
        func_802BF384(e);
        end = e->end;
        for (t = e->start; t != end; t++) {
            if (t->unk52 == part) {
                t->unk51 = 0;
            }
            if (t->unk57 == part && pcts[t->unk52 - 1] != 100) {
                t->unk51 = 1;
            }
        }
        func_802BF668(e);
        func_802BF534(e);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDD74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Impact of the point D_803A73F0/F4/F8 with radius r against the vehicles:
 * clears D_803F932D/932E, then for every D_803A7300 sphere (until flag -1;
 * vehicle 0 is skipped while D_803649E8 is set) within range (func_8029CFA4)
 * adds func_802CE0E4's 0/1 (that vehicle's hit spheres) to the count. Sets
 * D_803A7425 and D_803A7424 unless the ring span is 0/0xFFF, clears it when
 * D_803A742F is set, and returns the count.
 * ABI (the asm saves s0-s7 and fp). The asm uses the whole a0; the C
 * callers declare (s16) returning u8 (48D00.c, 4B5E0.c) or void (4DA80.c). */
s32 func_802CDF94(s32 r) {
    s32 x = D_803A73F0;
    s32 y = D_803A73F4;
    s32 z = D_803A73F8;
    u8 *p = D_803A7300;
    s32 count = 0;

    D_803F932D = 0;
    D_803F932E = 0;
    while ((s8) p[0x11] != -1) {
        s32 id = p[0x10];

        if (id == 0 && D_803649E8 != 0) {
            p += 0x14;
            continue;
        }
        p += 0x14;
        if (func_8029CFA4(z, r, ((s32 *) p)[-5], ((s32 *) p)[-4], x, y, ((s32 *) p)[-3], ((s32 *) p)[-2]) != 0) {
            count += func_802CE0E4(x, y, z, r, id);
        }
    }
    if (D_803A7410 != 0 || D_803A7412 != 0xFFF) {
        D_803A7425 = 1;
        D_803A7424 = 1;
    }
    if (D_803A742F != 0) {
        D_803A7410 = 0;
        D_803A7412 = 0;
    }
    return count;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDF94.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Vehicle id's hit spheres vs the point (x, y, z) radius r: finds id's first
 * D_803A6B30 sphere (until flag -1), then walks that run of consecutive
 * spheres of id (vehicle 6 skips spheres of radius 0x3BD); for each within
 * range (func_8029CFA4): D_803F932E = 1 if id is the mode D_80364456,
 * D_803F932D = 1 for vehicle 0xFF, and the plane-side update
 * func_802CE204(x, z, sphere x, sphere z). Returns 1 if any sphere was in
 * range, else 0.
 * Register convention: x, y, z, r in v0, v1, a0, a1, id in t4, result in s4
 * (clobbered); the asm saves v0-t0, t6, t7, s0, s1 and leaves t5 = -1, the
 * constant asm caller func_802CDF94 keeps there. Asm caller func_802CDF94
 * keeps a0, a1 and t3 live across the call. */
s32 func_802CE0E4(s32 x, s32 y, s32 z, s32 r, s32 id) {
    u8 *p = D_803A6B30;
    s32 hit = 0;

    for (;; p += 0x14) {
        if ((s8) p[0x13] == -1) {
            return 0;
        }
        if (p[0x12] == id) {
            break;
        }
    }
    while ((s8) p[0x13] != -1 && p[0x12] == id) {
        s32 ex = ((s32 *) p)[0];
        s32 ey = ((s32 *) p)[1];
        s32 ez = ((s32 *) p)[2];
        s32 er = ((s32 *) p)[3];

        p += 0x14;
        if (id == 6 && er == 0x3BD) {
            continue;
        }
        if (func_8029CFA4(z, r, ex, ey, x, y, ez, er) == 0) {
            continue;
        }
        if (D_80364456 == id) {
            D_803F932E = 1;
        }
        if (id == 0xFF) {
            D_803F932D = 1;
        }
        hit = 1;
        func_802CE204(x, z, ex, ez);
    }
    return hit;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE0E4.s")
#endif

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
#ifdef NON_MATCHING
extern u8 D_803A742A;
void func_8029B02C(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 id); /* 56040 */

/* Reports every triangle the sphere (x, y, z) radius r touches for vehicle
 * `kind` (func_8029B02C with everything >> 2 and part id 0), after storing
 * the low byte of `tag` to D_803A742A (the part func_8029B02C skips for kind
 * 0xC8). If that set D_803A742F, the ring span D_803A7410/12 is reset to
 * 0/0. The C callers declare r s16; the asm shifts the whole register. The
 * asm saves s0-s7 and fp. */
void func_802CE5BC(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 tag) {
    D_803A742A = tag;
    func_8029B02C(x >> 2, y >> 2, z >> 2, r >> 2, kind, 0);
    if (D_803A742F != 0) {
        D_803A7410 = 0;
        D_803A7412 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE5BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    s32 t1; /* new z */
    s32 s3; /* x as read */
    s32 fp; /* the cosine */
} Out802A860C; /* as in 62740.c */
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out); /* 62740 */
extern s32 D_803F9320;
extern s32 D_803F9324;
extern s16 D_803F9328;

/* Stores (x, z) to D_803F9320/9324 and len to D_803F9328, then moves the
 * point by len along heading angle (func_802A860C with scale 0.0) and stores
 * the result back. ABI (the asm saves s0-s7 and fp); the C callers declare
 * (s32, s32, s16, s16), the asm uses the whole angle register. */
void func_802CE65C(s32 x, s32 z, s32 len, s32 angle) {
    Out802A860C out;

    D_803F9320 = x;
    D_803F9324 = z;
    D_803F9328 = len;
    D_803F9320 = func_802A860C(0.0f, angle, &D_803F9328, &D_803F9320, &D_803F9324, &out);
    D_803F9324 = out.t1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE65C.s")
#endif

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
