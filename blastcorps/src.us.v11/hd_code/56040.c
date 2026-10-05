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

/* Same for cvt.w.s (float -> s32, round to nearest even). */
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

/* Spline basis (4x4, row stride 4 floats) built by func_8029F110 and the
 * powers t^2 / t^3 stored by func_8029F060. */
extern f32 D_803B3778[16];
extern f32 D_803B37B8;
extern f32 D_803B37BC;
/* 4x4 s32 (16.16) matrix built by the func_8029F3D0 family. */
extern s32 D_803B3730[16];

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

/* Callees in 62740.c (their C rewrites; register mappings in conventions.txt). */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 (in/out) */
    f32 dz;    /* f26 */
} TriSideOut; /* as in 62740.c */
typedef struct {
    s32 v1; /* out */
    s32 a0; /* out */
    s32 a3; /* in/out */
    s32 s1; /* in/out */
    s32 s2; /* in/out */
    s32 s0; /* in */
} MtxChainRegs; /* as in 62740.c */
s32 func_802AA460(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, TriSideOut *out);
s32 func_802AA5E0(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_802AA890(s32 count, s32 *offsets, u8 *base, s32 x, s32 y, s32 z, MtxChainRegs *regs);
s32 func_802AB41C(s32 key0, s32 key1);

/* Register results of func_8029DA90 its asm caller reads (all in/out: left
 * alone on paths that don't write them). */
typedef struct {
    /* 0x00 */ s32 s1;
    /* 0x04 */ s32 s3;
    /* 0x08 */ TriSideOut fp; /* f12, f14, f20, f22, f24 (in/out), f26 */
} Unk8029DA90Regs;

/* Register outputs of func_8029F110 (f12/f14 are in/out: only written when
 * mode == 1). */
typedef struct {
    f32 f12;
    f32 f14;
    f32 f20;
} Unk8029F110Out;

/* Register outputs of func_8029F1BC. */
typedef struct {
    s32 frame; /* t2 (in/out) */
    s32 dir;   /* t7 */
    f32 frac;  /* f30 */
} Unk8029F1BCOut;
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410;
extern u16 D_803A7412;
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
extern s32 D_803A7408;
extern s16 D_803A7422;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 D_803A7427;
extern u8 D_803A7428;
extern u8 D_803A7429;
extern u8 D_803A742C;
extern u8 D_803A742D;
extern u8 D_803A742E;
extern u8 D_803A742F;
extern s16 D_803F77FC;
extern u8 D_803F7801;
extern u8 D_803F7811;
extern s32 D_80358060;
void func_802BCBD8(void);
void func_802BCC10(void);

/* Vehicle-state reset on entering a vehicle: the ring span D_803A7410/7412 =
 * 0/0xFFF, the position D_803A73F0..F8 = (x, y, z), the various mode bytes and
 * halves from the arguments, D_803A742D = veh[0xA0] (the vehicle record the
 * asm has in gp), and the flag bytes cleared. Then resets buffer B
 * (func_802BCC10) if D_80358060 is 0, again if b2 != 0 and b8 != 0xFF (b2 is
 * compared as the full register), and always buffer A (func_802BCBD8).
 * Register convention (conventions.txt): z, a1, b2, b3 in a0-a3 as in the asm;
 * x, y, b0, h1, h2, b4, b8, veh in v0, v1, t0, t1, t2, t3, t8, gp. The
 * asm saves t7/s0 and leaves v0 = D_80358060. Asm callers keep a0-a3,
 * t0, t1, t4, t6, t7, t8, f12, f14 live (a mixed build would need a thunk). */
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh) {
    D_803A7410 = 0;
    D_803A7412 = 0xFFF;
    D_803A73F0 = x;
    D_803A73F4 = y;
    D_803A73F8 = z;
    D_803F7811 = b2;
    D_803A742C = 0;
    D_803A742E = 0;
    D_803A742D = veh[0xA0];
    D_803A742F = 0;
    D_803F7801 = b4;
    D_803A7427 = b3;
    D_803A7428 = b0;
    D_803A7429 = 0;
    D_803A7422 = h2;
    D_803F77FC = h1;
    D_803A7408 = a1;
    D_803A7424 = 0;
    D_803A7425 = 0;
    if (D_80358060 == 0) {
        func_802BCC10();
    }
    if (b2 != 0 && b8 != 0xFF) {
        func_802BCC10();
    }
    func_802BCBD8();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A800.s")
#endif

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
#ifdef NON_MATCHING
extern u8 D_803A7300[]; /* 0x14-byte vehicle spheres: x, y, z, r words, byte 0x10 = kind, 0x11 = state (0xFF = end) */
extern u8 D_803A6B30[]; /* 0x14-byte part spheres: x, y, z, r words, byte 0x12 = kind, 0x13 = state (0xFF = end) */
extern u8 D_803A742B;
extern s32 D_80358068;
extern s32 D_803A740C;
s32 func_8029AB88(s32 x, s32 y, s32 z, s32 r, s32 kind);
void func_8029B02C(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 id);

/* World collision of vehicle `kind` for this frame. Clears D_803A742B, finds
 * the vehicle's D_803A7300 entry (byte 0x10 == kind; unbounded scan) and, if
 * its byte 0x11 is set and its bounding sphere (words >> 2, arithmetic) hits
 * the world (func_8029AB88), reports the hits of each of its parts: every
 * D_803A6B30 entry (up to the one with byte 0x13 == 0xFF) with byte 0x13
 * nonzero and byte 0x12 == kind is numbered id = 1, 2, ... and its sphere
 * (>> 2) goes to func_8029B02C(x, y, z, r, kind, id). Finally, if the ring
 * span D_803A7410/D_803A7412 is no longer the empty 0/0xFFF pair, sets
 * D_803A7425 and copies D_80358068 to D_803A740C.
 * The asm's register interface, as a struct would hold it: in t8 = kind;
 * out (survey) a0, a3, s3, fp, f12, f14, f20-f26 = whatever the triangle
 * tests last left there, plus fp = the last id. None of those is live in
 * any asm caller (each goes on to reload them or returns), so the C has a
 * plain (kind) interface and no outputs.
 * Register convention: kind in t8 (conventions.txt); the asm saves v1, t0,
 * t1, t3, t6, t7, s0-s2, s4, s7, t9. Asm callers keep t6 (func_802BA9A0 also
 * t7) live; a mixed build would need a thunk, the native port doesn't. */
void func_8029AA10(s32 kind) {
    u8 *e = D_803A7300;
    u8 *p;
    s32 id;

    D_803A742B = 0;
    while (e[0x10] != kind) {
        e += 0x14;
    }
    if ((s8) e[0x11] != 0 &&
        func_8029AB88(*(s32 *) (e + 0) >> 2, *(s32 *) (e + 4) >> 2, *(s32 *) (e + 8) >> 2, *(s32 *) (e + 0xC) >> 2,
                      kind) != 0) {
        id = 0;
        for (p = D_803A6B30; (s8) p[0x13] != -1; p += 0x14) {
            if ((s8) p[0x13] == 0 || p[0x12] != kind) {
                continue;
            }
            id++;
            func_8029B02C(*(s32 *) (p + 0) >> 2, *(s32 *) (p + 4) >> 2, *(s32 *) (p + 8) >> 2,
                          *(s32 *) (p + 0xC) >> 2, kind, id);
        }
    }
    if (D_803A7410 != 0 || D_803A7412 != 0xFFF) {
        D_803A7425 = 1;
        D_803A740C = D_80358068;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AA10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8029B514(s32 x, s32 y, s32 z, s32 r, u8 *rec);
s32 func_8029C160(s32 x, s32 y, s32 z, s32 r, u8 *tri, s32 *hit);
void func_8029C0DC(u8 *tri, s32 px, s32 py, s32 pz, s32 *out);
s32 func_8029BF64(s32 bu, s32 bv, s32 cu, s32 cv, s32 au, s32 av, s32 pu, s32 pv);
s32 func_8029BD0C(s32 x, s32 y, s32 z, s32 r, u8 *tri);
s32 func_8029BEE4(s32 x, s32 y, s32 z, s32 r, u8 *tri);
void func_8029C284(s32 x, s32 z);
s32 func_802BD8C8(void); /* 77E20 */
extern u32 D_803059F0[];  /* {level (0xFFFF = any), object kind, vehicle mask} triples, ended by mask 0 */
extern s32 D_802E8BDC;    /* current level */
extern u8 D_803F4030[];   /* 0xFC-byte objects: +4 / +8 triangle range, +0x30 kind */
extern u8 *D_803F7654;    /* end of the objects */
extern u8 D_803F9330[];   /* 0x60-byte triangles */
extern u8 *D_803FB8B0;    /* their end */
extern u8 *D_803BD308;    /* triangle range */
extern u8 *D_803BD30C;
extern u8 D_803A742A;
extern u8 *D_803BDE40[];  /* per-cell triangle lists (cell c spans [c] .. [c + 1]) */
extern u8 *D_803BDCA8[];
extern s16 D_803A7418[];  /* cells from func_8029C284, ended by -1 */

/* The sphere-vs-triangle test (see the comment above func_8029BD0C): plane
 * distance (func_8029C160), then the projected point-in-triangle test, the
 * edge test and the vertex test; 1 on the first that hits. */
static s32 port_sphere_hits_tri(s32 x, s32 y, s32 z, s32 r, u8 *tri) {
    s32 hit[3];
    s32 uv[8];

    if (func_8029C160(x, y, z, r, tri, hit) == 0) {
        return 0;
    }
    func_8029C0DC(tri, hit[0], hit[1], hit[2], uv);
    if (func_8029BF64(uv[2], uv[3], uv[4], uv[5], uv[0], uv[1], uv[6], uv[7]) != 0) {
        return 1;
    }
    if (func_8029BD0C(x, y, z, r, tri) != 0) {
        return 1;
    }
    return func_8029BEE4(x, y, z, r, tri) != 0;
}

/* Does the sphere (x, y, z) radius r hit the world for vehicle `kind`? In
 * order (each list walked with `!=`, 0x60-byte triangles):
 * 1. for each D_803059F0 entry whose mask has bit (kind & 31) and whose level
 *    is 0xFFFF or D_802E8BDC (object kind 0x38 only while func_802BD8C8()
 *    is 0): every D_803F4030 object of that kind that func_8029B514 says the
 *    sphere touches, its triangles with byte 0x51 set;
 * 2. D_803F9330 .. D_803FB8B0 with byte 0x51 set, except (kind 0xC8) those
 *    whose u16 at +0x52 equals D_803A742A;
 * 3. every triangle of D_803BD308 .. D_803BD30C;
 * 4. D_803B9890 .. D_803BD300 with byte 0x51 set, except those whose byte
 *    0x4F is nonzero and equal to kind;
 * 5. the grid cells of (x, z) (func_8029C284): unless kind == 9, the
 *    D_803BDE40 lists, then the D_803BDCA8 lists.
 * Returns 1 at the first triangle hit (port_sphere_hits_tri), else 0.
 * Register convention: x, y, z, r, kind in t3, t4, t5, t6, t8, result in a1
 * (conventions.txt); the asm saves v1, t0, t1, t3, t6-t9, s0-s2, s4, s7 and
 * leaves s3 / f20-f28 as the callees do (a0, a3 and the FP results too: dead
 * in its caller func_8029AA10, which keeps t0, t1, t8). The add/addi on the
 * cell offsets trap on overflow. */
s32 func_8029AB88(s32 x, s32 y, s32 z, s32 r, s32 kind) {
    u32 *e;
    u8 *obj;
    u8 *objEnd;
    u8 *tri;
    u8 *end;
    s16 *cell;

    for (e = D_803059F0; e[2] != 0; e += 3) {
        if ((e[2] & (1 << (kind & 31))) == 0) {
            continue;
        }
        if (e[0] != 0xFFFF && e[0] != (u32) D_802E8BDC) {
            continue;
        }
        if (e[1] == 0x38 && func_802BD8C8() != 0) {
            continue;
        }
        objEnd = D_803F7654;
        for (obj = D_803F4030; obj != objEnd; obj += 0xFC) {
            if (*(u32 *) (obj + 0x30) != e[1]) {
                continue;
            }
            if (func_8029B514(x, y, z, r, obj) == 0) {
                continue;
            }
            end = *(u8 **) (obj + 8);
            for (tri = *(u8 **) (obj + 4); tri != end; tri += 0x60) {
                if (tri[0x51] != 0 && port_sphere_hits_tri(x, y, z, r, tri)) {
                    return 1;
                }
            }
        }
    }
    end = D_803FB8B0;
    for (tri = D_803F9330; tri != end; tri += 0x60) {
        if (tri[0x51] == 0) {
            continue;
        }
        if (kind == 0xC8 && *(u16 *) (tri + 0x52) == D_803A742A) {
            continue;
        }
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            return 1;
        }
    }
    end = D_803BD30C;
    for (tri = D_803BD308; tri != end; tri += 0x60) {
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            return 1;
        }
    }
    end = D_803BD300;
    for (tri = D_803B9890; tri != end; tri += 0x60) {
        if (tri[0x51] == 0) {
            continue;
        }
        if (tri[0x4F] != 0 && tri[0x4F] == kind) {
            continue;
        }
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            return 1;
        }
    }
    func_8029C284(x, z);
    if (kind != 9) {
        for (cell = D_803A7418; *cell != -1; cell++) {
            end = D_803BDE40[*cell + 1];
            for (tri = D_803BDE40[*cell]; tri != end; tri += 0x60) {
                if (port_sphere_hits_tri(x, y, z, r, tri)) {
                    return 1;
                }
            }
        }
    }
    for (cell = D_803A7418; *cell != -1; cell++) {
        end = D_803BDCA8[*cell + 1];
        for (tri = D_803BDCA8[*cell]; tri != end; tri += 0x60) {
            if (port_sphere_hits_tri(x, y, z, r, tri)) {
                return 1;
            }
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AB88.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_8029B614(u8 *tri, s32 id);
void func_8029B5B8(u8 *rec);
s32 func_8029BB28(u8 *tri, s32 kind, s32 id);

/* The reporting twin of func_8029AB88: walks the same triangle lists in the
 * same order with the same filters for the sphere (x, y, z) radius r of
 * vehicle `kind`, but instead of stopping at the first hit it reports every
 * triangle the sphere touches (port_sphere_hits_tri) and carries on:
 *   objects of the D_803059F0 kinds and D_803F9330: func_8029B614(tri, id)
 *   D_803BD308 (switches):                          func_8029BB28(tri, kind, id)
 *   D_803B9890:                                     func_8029B614, func_8029B5B8(tri)
 *   the grid cells' D_803BDE40 (unless kind == 9) and D_803BDCA8 lists:
 *                                                   func_8029B614(tri, id)
 * `id` is the part number its caller func_8029AA10 counts.
 * Register convention: x, y, z, r, kind, id in t3, t4, t5, t6, t8, fp
 * (conventions.txt). The asm saves v1, t0, t1, t3, t6-t9, s0-s2, s4, s7 and
 * leaves a0, a1, a3, s3, f12-f28 as the triangle tests and callbacks leave
 * them (dead in func_8029AA10, which keeps t0, t1, t7, t8). The add on the
 * cell offsets traps on overflow. */
void func_8029B02C(s32 x, s32 y, s32 z, s32 r, s32 kind, s32 id) {
    u32 *e;
    u8 *obj;
    u8 *objEnd;
    u8 *tri;
    u8 *end;
    s16 *cell;

    for (e = D_803059F0; e[2] != 0; e += 3) {
        if ((e[2] & (1 << (kind & 31))) == 0) {
            continue;
        }
        if (e[0] != 0xFFFF && e[0] != (u32) D_802E8BDC) {
            continue;
        }
        if (e[1] == 0x38 && func_802BD8C8() != 0) {
            continue;
        }
        objEnd = D_803F7654;
        for (obj = D_803F4030; obj != objEnd; obj += 0xFC) {
            if (*(u32 *) (obj + 0x30) != e[1]) {
                continue;
            }
            if (func_8029B514(x, y, z, r, obj) == 0) {
                continue;
            }
            end = *(u8 **) (obj + 8);
            for (tri = *(u8 **) (obj + 4); tri != end; tri += 0x60) {
                if (tri[0x51] != 0 && port_sphere_hits_tri(x, y, z, r, tri)) {
                    func_8029B614(tri, id);
                }
            }
        }
    }
    end = D_803FB8B0;
    for (tri = D_803F9330; tri != end; tri += 0x60) {
        if (tri[0x51] == 0) {
            continue;
        }
        if (kind == 0xC8 && *(u16 *) (tri + 0x52) == D_803A742A) {
            continue;
        }
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            func_8029B614(tri, id);
        }
    }
    end = D_803BD30C;
    for (tri = D_803BD308; tri != end; tri += 0x60) {
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            func_8029BB28(tri, kind, id);
        }
    }
    end = D_803BD300;
    for (tri = D_803B9890; tri != end; tri += 0x60) {
        if (tri[0x51] == 0) {
            continue;
        }
        if (tri[0x4F] != 0 && tri[0x4F] == kind) {
            continue;
        }
        if (port_sphere_hits_tri(x, y, z, r, tri)) {
            func_8029B614(tri, id);
            func_8029B5B8(tri);
        }
    }
    func_8029C284(x, z);
    if (kind != 9) {
        for (cell = D_803A7418; *cell != -1; cell++) {
            end = D_803BDE40[*cell + 1];
            for (tri = D_803BDE40[*cell]; tri != end; tri += 0x60) {
                if (port_sphere_hits_tri(x, y, z, r, tri)) {
                    func_8029B614(tri, id);
                }
            }
        }
    }
    for (cell = D_803A7418; *cell != -1; cell++) {
        end = D_803BDCA8[*cell + 1];
        for (tri = D_803BDCA8[*cell]; tri != end; tri += 0x60) {
            if (port_sphere_hits_tri(x, y, z, r, tri)) {
                func_8029B614(tri, id);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B02C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8029CFA4(s32 pz, s32 r1, s32 qx, s32 qy, s32 px, s32 py, s32 qz, s32 r2);

/* Sphere (x, y, z) radius r against the sphere of record `rec`: centre at
 * words 0x10/0x14/0x18, radius at word 0xC, each >> 2 (arithmetic); returns
 * func_8029CFA4's overlap flag.
 * Register convention: asm takes x, y, z, r in t3, t4, t5, t6 and rec in s0,
 * returns in v1 (conventions.txt); it restores v0, a0-a3, t0-t2. Asm callers
 * keep a0-a3, t3-t6, t8, t9, f12, f14 live. */
s32 func_8029B514(s32 x, s32 y, s32 z, s32 r, u8 *rec) {
    return func_8029CFA4(z, r, *(s32 *) (rec + 0x10) >> 2, *(s32 *) (rec + 0x14) >> 2, x, y,
                         *(s32 *) (rec + 0x18) >> 2, *(s32 *) (rec + 0xC) >> 2);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B514.s")
#endif

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
#ifdef NON_MATCHING
extern u8 D_803BE738;
void func_802BCCD4(s32 value);
void func_8029B7CC(s32 a, s32 b);
void func_8029B994(void);

/* Collision hit on triangle `tri` (id `id`): appends id to buffer A
 * (func_802BCCD4). Unless id is already in the byte list at D_803A7408
 * (signed bytes, scanned up to the first negative one, which is compared
 * too): sets D_803A7424, takes the plane function s = normal . (D_803A73F0/
 * F4/F8 >> 2) + d (s64 fields at 0/8/0x10, d at 0x18; 64-bit, wrapping
 * where the asm's dadd traps) and, as func_802BF264: if s is on d's side
 * (s > 0 with d > 0, s < 0 with d <= 0) calls func_8029B7CC(h - 0x400,
 * h + 0x400) when byte 0x55 is 1 or byte 0x56 is set, else func_8029B7CC(
 * h + 0x400, h - 0x400) when byte 0x55 is 1 or byte 0x56 is clear (h = u16
 * at 0x4C); after a call a nonzero byte 0x59 goes to D_803BE738. Then, if
 * D_803A7429 is clear, |D_803F77FC| >= |D_803A7422| and D_803A7427 is set:
 * D_803A7429 = D_803A7427 and func_8029B994().
 * Register convention: tri in s0, id in fp (conventions.txt); the asm saves
 * v0-a3 and t0. Asm callers rely on preserved: func_8029B02C / func_8029BB28
 * keep a0, a1, a3, t3-t6, t8, t9 (and f12/f14, left as the callees leave
 * them). */
void func_8029B614(u8 *tri, s32 id) {
    s8 *p;
    s32 c;
    s64 s;
    s64 d;
    s32 h;

    func_802BCCD4(id);
    for (p = (s8 *) D_803A7408;; p++) {
        c = *p;
        if (c == id) {
            goto tail;
        }
        if (c < 0) {
            break;
        }
    }
    D_803A7424 = 1;
    d = *(s64 *) (tri + 0x18);
    s = *(s64 *) (tri + 0) * (D_803A73F0 >> 2) + *(s64 *) (tri + 8) * (D_803A73F4 >> 2) +
        *(s64 *) (tri + 0x10) * (D_803A73F8 >> 2) + d;
    h = *(u16 *) (tri + 0x4C);
    if ((d > 0) ? (s > 0) : (s < 0)) {
        if (tri[0x55] != 1 && tri[0x56] == 0) {
            goto tail;
        }
        func_8029B7CC(h - 0x400, h + 0x400);
    } else {
        if (tri[0x55] != 1 && tri[0x56] != 0) {
            goto tail;
        }
        func_8029B7CC(h + 0x400, h - 0x400);
    }
    if (tri[0x59] != 0) {
        D_803BE738 = tri[0x59];
    }
tail:
    if (D_803A7429 == 0) {
        s32 a = D_803F77FC;
        s32 b = D_803A7422;

        if (a < 0) {
            a = -a;
        }
        if (b < 0) {
            b = -b;
        }
        if (!(a < b) && D_803A7427 != 0) {
            D_803A7429 = D_803A7427;
            func_8029B994();
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B614.s")
#endif

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
#ifdef NON_MATCHING
typedef struct {
    /* 0x0 */ s32 a3;
    /* 0x4 */ s32 t6;
    /* 0x8 */ s32 s1;
} Io802A6274; /* as in 60F60.c */
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
extern u8 D_803A7428;
extern s32 D_803A73FC;
extern s32 D_803A7400;
extern s32 D_803A7404;
extern u8 D_80370C3C;
extern u8 D_802C2984[]; /* a definition inside the 7D9D0 blob */

/* Sets up a D_803C4B70 record (func_802A6274) for the definition D_802C2984
 * with data D_803A7428 * 7000, type 0, position D_803A73FC/7400/7404 << 13,
 * b35 = 1 and every other input 0; then D_80370C3C = 1.
 * The asm saves and restores every register (f12/f14 are left as the callee
 * leaves them); its asm caller func_8029B614 relies on nothing else. */
void func_8029B994(void) {
    Io802A6274 io;

    io.a3 = 0;
    io.t6 = 0;
    io.s1 = 0;
    func_802A6274(&io, D_802C2984, D_803A7428 * 0x1B58, 0, D_803A73FC << 13, D_803A7400 << 13, D_803A7404 << 13,
                  0, 0, 0, 0, 0, 1);
    D_80370C3C = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B994.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803BD310[]; /* 0xFC-byte switch groups: kind count at 0, kinds from 1, tri pointers from 8 (count at 0xF8), flag 0xF9 */
extern u8 D_803ED825;
void func_8029B614(u8 *tri, s32 id);

/* Hit on a triangle of the D_803BD308 list (a switch): finds the
 * D_803BD310 group whose pointer list (g[0xF8] words from g + 8) holds tri
 * (unbounded scan: it must be listed). If the vehicle kind is one of the
 * g[0] bytes from g + 1, it is an ordinary hit: func_8029B614(tri, id),
 * returns 1. Otherwise (only on level 9 or for kinds below 0x13, and only
 * when g[0xF9] is set) the side of the plane the vehicle is on (the plane
 * function at D_803A73F0/F4/F8 >> 2, as in func_8029B614: 1 on d's side,
 * else 2) is compared with tri[0x54]: a 0 there just records the side; a
 * different side flips D_803ED825 (bit 0) and every nonzero byte 0x54 of
 * the group's triangles between 1 and 2. Returns 0 on those paths.
 * Register convention: tri in s0, kind in t8, id in fp, result in a1
 * (conventions.txt; dead in its caller func_8029B02C). The asm saves v0-a0,
 * a2; func_8029B02C keeps a0, a3, t3-t6, t8 live. Its 64-bit dadd traps on
 * overflow where this C wraps. */
s32 func_8029BB28(u8 *tri, s32 kind, s32 id) {
    u8 *g = D_803BD310;
    u8 **p;
    u8 *b;
    s32 n;
    s64 s;
    s64 d;
    s32 side;

    for (;; g += 0xFC) {
        p = (u8 **) (g + 8);
        for (n = g[0xF8]; n != 0; n--) {
            if (*p++ == tri) {
                goto found;
            }
        }
    }
found:
    b = g + 1;
    for (n = g[0]; n != 0; n--) {
        if (*b++ == kind) {
            func_8029B614(tri, id);
            return 1;
        }
    }
    if (D_802E8BDC != 9 && kind >= 0x13) {
        return 0;
    }
    if (g[0xF9] == 0) {
        return 0;
    }
    d = *(s64 *) (tri + 0x18);
    s = *(s64 *) (tri + 0) * (D_803A73F0 >> 2) + *(s64 *) (tri + 8) * (D_803A73F4 >> 2) +
        *(s64 *) (tri + 0x10) * (D_803A73F8 >> 2) + d;
    side = ((d > 0) ? (s > 0) : (s < 0)) ? 1 : 2;
    if (tri[0x54] == 0) {
        tri[0x54] = side;
        return 0;
    }
    if (tri[0x54] == side) {
        return 0;
    }
    D_803ED825 ^= 1;
    p = (u8 **) (g + 8);
    for (n = g[0xF8]; n != 0; n--) {
        b = *p++;
        if (b[0x54] != 0) {
            b[0x54] = (b[0x54] == 1) ? 2 : 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BB28.s")
#endif

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
#ifdef NON_MATCHING
/* Places an object's parts (the counterpart of func_8029C354): the
 * D_803A7300 entry with byte 0x10 == tag gets words 0/4/8 = (x, y, z) and
 * byte 0x11 = 1. Then for each variable-size record from p + 4 up to `end`
 * (stops only at exactly `end`), consecutive D_803A6B30 entries from the
 * first with byte 0x12 == tag get words 0/4/8 = the record's s16 point
 * (rec+0, +2, +4) transformed by func_802AA890 through the u16 rec+0xA
 * matrices listed at rec+0xC (base `base`), and byte 0x13 = 1. Both scans are
 * unbounded (the tag must be present). `regs` carries func_802AA890's extra
 * register inputs/outputs (a3, s1, s2 in/out, s0 in), as the asm passes them
 * through.
 * Register convention: x, y, z, tag, p, end, base in v0, v1, a0, t0, t1, t2,
 * s4 (conventions.txt); the asm restores v0-a2, t3, t4 and leaves t1 = end.
 * Its add/addi are trapping. Asm callers keep a0, a2, t0, t2, t4, t6, t7,
 * f12, f14 live. */
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs) {
    u8 *e = D_803A7300;
    s32 n;

    while (e[0x10] != tag) {
        e += 0x14;
    }
    p += 4;
    *(s32 *) (e + 0) = x;
    *(s32 *) (e + 4) = y;
    *(s32 *) (e + 8) = z;
    e[0x11] = 1;
    if (p == end) {
        return;
    }
    e = D_803A6B30;
    while (e[0x12] != tag) {
        e += 0x14;
    }
    while (p != end) {
        n = *(u16 *) (p + 0xA);
        *(s32 *) (e + 0) = func_802AA890(n, (s32 *) (p + 0xC), base, *(s16 *) (p + 0), *(s16 *) (p + 2),
                                         *(s16 *) (p + 4), regs);
        *(s32 *) (e + 4) = regs->v1;
        *(s32 *) (e + 8) = regs->a0;
        e[0x13] = 1;
        p += (n << 2) + 0xC;
        e += 0x14;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C454.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_8029C9D4(s32 tag);
void func_8029C828(s32 tag);
void func_8029C748(s32 tag);

/* Part-vs-part / part-vs-marker contacts of the vehicle whose parts carry
 * `tag` (func_8029C9D4, then func_8029C828, then func_8029C748); then, if the
 * ring span D_803A7410/D_803A7412 is no longer the empty 0/0xFFF pair, sets
 * D_803A7425 and D_803A7424.
 * Register convention: tag in t8 (conventions.txt); the asm saves v0-a0,
 * a2-a3, t0-t5 and leaves s0, s1, fp as its callees leave them (dead in
 * every asm caller, which goes on to call func_8029AA10). Asm callers keep
 * a0, a3, t0, t1, t6 live (and f12/f14, which nothing here changes); a
 * mixed build would need a thunk, the native port doesn't. */
void func_8029C52C(s32 tag) {
    func_8029C9D4(tag);
    func_8029C828(tag);
    func_8029C748(tag);
    if (D_803A7410 != 0 || D_803A7412 != 0xFFF) {
        D_803A7425 = 1;
        D_803A7424 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C52C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_8029C6E4(Unk8029C6E4Out *o);
void func_8028FAC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_802920DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Looks up the kind-6 / id-0x3BD part with func_8029C6E4; if found, calls
 * func_8028FAC0 and func_802920DC with its words 0/4/8 and the last word 0xC
 * seen (s0-s3 pass through to them when the lookup doesn't load them).
 * Returns the found flag.
 * Register convention: asm takes s0-s3 and returns the flag in s4
 * (conventions.txt); it saves every other integer register. Its one asm
 * caller (func_802BB274) also reads f12 afterwards, i.e. whatever the C
 * callees left there; that isn't modelled. It keeps a0, a2, a3, t0, t1, t6
 * and f14 live (a mixed build would need a thunk). */
s32 func_8029C5EC(s32 s0, s32 s1, s32 s2, s32 s3) {
    Unk8029C6E4Out o;

    o.s0 = s0;
    o.s1 = s1;
    o.s2 = s2;
    o.s3 = s3;
    func_8029C6E4(&o);
    if (o.s4 != 0) {
        func_8028FAC0(o.s0, o.s1, o.s2, o.s3);
        func_802920DC(o.s0, o.s1, o.s2, o.s3);
    }
    return o.s4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C5EC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (Unk8029C6E4Out is declared in the file-level block at the top.) */
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
#ifdef NON_MATCHING
extern u8 D_803FBBB0[]; /* 0x14-byte spheres: x, y, z, r words */
extern u8 D_803FC1F0;   /* their count */
void func_8029C914(s32 tag, s32 qx, s32 qy, s32 qz, s32 qr);

/* Contacts with the D_803FC1F0 spheres of D_803FBBB0: finds the D_803A7300
 * entry with byte 0x10 == tag (unbounded scan); unless its byte 0x11 is 1,
 * does nothing. Otherwise, for each sphere q that overlaps the entry's
 * sphere (words 0..0xC; func_8029CFA4), runs func_8029C914(tag, q).
 * Register convention: tag in t8 (conventions.txt); the asm saves v0-t3 and
 * leaves s0, s1, fp, t7 as func_8029C914 leaves them (dead in its caller
 * func_8029C52C). Its caller keeps t8, f12, f14 live (no thunk needed in the
 * native port). */
void func_8029C748(s32 tag) {
    u8 *e = D_803A7300;
    u8 *q;
    s32 n;

    while (e[0x10] != tag) {
        e += 0x14;
    }
    if ((s8) e[0x11] != 1) {
        return;
    }
    q = D_803FBBB0;
    for (n = D_803FC1F0; n != 0; n--, q += 0x14) {
        if (func_8029CFA4(*(s32 *) (e + 8), *(s32 *) (e + 0xC), *(s32 *) (q + 0), *(s32 *) (q + 4),
                          *(s32 *) (e + 0), *(s32 *) (e + 4), *(s32 *) (q + 8), *(s32 *) (q + 0xC))) {
            func_8029C914(tag, *(s32 *) (q + 0), *(s32 *) (q + 4), *(s32 *) (q + 8), *(s32 *) (q + 0xC));
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C748.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803FB8B8[]; /* the 25 0x14-byte radar markers of 8A080.c: word 0 = id (-1 = free), x, y, z, r */
void func_8029C914(s32 tag, s32 qx, s32 qy, s32 qz, s32 qr);

/* As func_8029C748, against the 25 radar markers D_803FB8B8 (free ones,
 * id -1, skipped; sphere at words 1..4).
 * Register convention: tag in t8 (conventions.txt); same register notes as
 * func_8029C748. */
void func_8029C828(s32 tag) {
    u8 *e = D_803A7300;
    s32 *q;
    s32 n;

    while (e[0x10] != tag) {
        e += 0x14;
    }
    if ((s8) e[0x11] != 1) {
        return;
    }
    q = D_803FB8B8;
    for (n = 25; n != 0; n--, q += 5) {
        if (q[0] == -1) {
            continue;
        }
        if (func_8029CFA4(*(s32 *) (e + 8), *(s32 *) (e + 0xC), q[1], q[2], *(s32 *) (e + 0), *(s32 *) (e + 4), q[3],
                          q[4])) {
            func_8029C914(tag, q[1], q[2], q[3], q[4]);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C828.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802CE204(s32 x1, s32 z1, s32 x2, s32 z2); /* 89250 */
void func_802BCCD4(s32 value);                      /* 77E20 */

/* Each part of the run of D_803A6B30 entries tagged `tag` (the run that
 * starts at the first such entry and ends at the next entry with another tag
 * or the 0xFF end marker at byte 0x13) is numbered n = 1, 2, ...; for each
 * part whose sphere (words 0..0xC) overlaps the sphere q (func_8029CFA4)
 * calls func_802CE204(part x, part z, qx, qz) and func_802BCCD4(n).
 * Register convention: tag in t8, q in a2, a3, t0, t1 (conventions.txt);
 * the asm saves v0-a1, t3, t5 and leaves t7, s0, s1, fp changed (dead in its
 * callers func_8029C748 / func_8029C828, which keep a0, a1, t3, t5, t8
 * live). */
void func_8029C914(s32 tag, s32 qx, s32 qy, s32 qz, s32 qr) {
    u8 *e = D_803A6B30;
    s32 n = 0;
    s32 px;
    s32 pz;

    for (;;) {
        if ((s8) e[0x13] == -1) {
            return;
        }
        if (e[0x12] == tag) {
            break;
        }
        e += 0x14;
    }
    for (; (s8) e[0x13] != -1 && e[0x12] == tag; e += 0x14) {
        n++;
        px = *(s32 *) (e + 0);
        pz = *(s32 *) (e + 8);
        if (func_8029CFA4(pz, *(s32 *) (e + 0xC), qx, qy, px, *(s32 *) (e + 4), qz, qr)) {
            func_802CE204(px, pz, qx, qz);
            func_802BCCD4(n);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C914.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803649E8;
u32 func_8029CB04(u32 v, s32 key);
void func_8029CD54(s32 tag, s32 other);

/* Vehicle-vs-object contacts: finds the D_803A7300 entry with byte 0x10 ==
 * tag (unbounded scan); unless its byte 0x11 is 1, does nothing. Otherwise
 * walks D_803A7300 up to the entry with byte 0x11 == 0xFF; each other entry
 * p (kind p[0x10] != tag) is skipped when tag == 6 and func_802AB41C(kind,
 * 6) is set, or when D_803649E8 is set and kind == 0. Else its radius
 * (word 0xC) is scaled by func_8029CB04 (key = kind) and if the spheres
 * overlap (func_8029CFA4) func_8029CD54(tag, kind) handles the contact.
 * Register convention: tag in t8 (conventions.txt); gp (D_803EEA90) is read
 * by the callees as the global. The asm saves v0-a0, a2-t5 and leaves fp
 * changed (by func_8029CD54); its caller func_8029C52C keeps t7 live. */
void func_8029C9D4(s32 tag) {
    u8 *e = D_803A7300;
    u8 *p;
    s32 kind;
    u32 r;

    while (e[0x10] != tag) {
        e += 0x14;
    }
    if ((s8) e[0x11] != 1) {
        return;
    }
    for (p = D_803A7300; (s8) p[0x11] != -1; p += 0x14) {
        kind = p[0x10];
        if (kind == tag) {
            continue;
        }
        if (tag == 6 && func_802AB41C(kind, tag) != 0) {
            continue;
        }
        if (D_803649E8 != 0 && kind == 0) {
            continue;
        }
        r = func_8029CB04(*(s32 *) (p + 0xC), kind);
        if (func_8029CFA4(*(s32 *) (e + 8), *(s32 *) (e + 0xC), *(s32 *) (p + 0), *(s32 *) (p + 4),
                          *(s32 *) (e + 0), *(s32 *) (e + 4), *(s32 *) (p + 8), r)) {
            func_8029CD54(tag, kind);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C9D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Divides v by the per-vehicle byte gp+0xA0 (unsigned) unless key == 0xFF
 * (compared as the full register) or the byte is 1; returns v (unchanged
 * otherwise). The asm's key == 0xFF branch lands in func_8029CF04's epilogue
 * (.L8029CF40), which is the same "restore v0, return" as its own. A zero
 * divisor traps (break 7) in the asm and in this C alike.
 * Register convention: asm takes v in t1 and key in t4, returns in t1
 * (conventions.txt); it saves v0. Its asm caller func_8029C9D4 keeps a0-a3,
 * t0, t3, t4, t5, t8, f12, f14 live. */
u32 func_8029CB04(u32 v, s32 key) {
    u32 d;

    if (key != 0xFF) {
        d = GP_U8(0xA0);
        if (d != 1) {
            v /= d;
        }
    }
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CB04.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803A7426;
extern u8 D_803A742C;
void func_802BCCD4(s32 value);
s32 func_802AD7D4(s32 sine); /* C-callable wrapper of the arcsine func_802AD7FC (69000.c) */
void func_8029B7CC(s32 a, s32 b);
#pragma intrinsic(sqrtf) /* sqrt.s, as the asm (applies to the rest of the file) */

/* Heading from point 1 (x1, z1) to point 2 (x2, z2) of a pair of linked
 * entries: appends n to buffer A (func_802BCCD4); sets D_803A7426 when
 * kind == 0xFF and the byte next[-2] is non-zero, or when kind is neither 0
 * nor 0xFF and next[-2] == 0xFF (next[-2] is byte 0x12 of the entry before
 * `next`). The angle (0..0xFFF, 0x400 per quadrant) is
 * (asin(65536 * leg / dist) >> 4) + quadrant * 0x400 with dist = sqrtf of
 * the float squares, the quadrant picked by signed compares of the raw
 * coordinates, and 0 when both differences are zero. Then widens the ring
 * span with func_8029B7CC(angle + 0x400, angle - 0x400) and sets D_803A742C.
 * The same quadrant split as the camera code in 00000.c.
 * Register convention: asm takes n, kind, next, x1, z1, x2, z2 in fp, t8, s0,
 * v0, a0, a2, t0 (conventions.txt); it saves v0-a2, t0, t7, s0, fp and
 * leaves a3 changed (by func_8029B7CC). Its asm caller func_8029CD54 keeps
 * v0, v1, a0, a1, t4-t8, s0, s1, fp live (a mixed build would need a thunk). */
void func_8029CB54(s32 n, s32 kind, u8 *next, s32 x1, s32 z1, s32 x2, s32 z2) {
    s32 dx;
    s32 dz;
    f32 dist;
    u32 angle = 0;

    func_802BCCD4(n);
    if (kind == 0xFF) {
        if (next[-2] != 0) {
            D_803A7426 = 1;
        }
    } else if (kind != 0 && next[-2] == 0xFF) {
        D_803A7426 = 1;
    }
    dx = x2 - x1;
    dz = z2 - z1;
    if (dx != 0 || dz != 0) {
        f32 fx = dx;
        f32 fz = dz;

        dist = sqrtf(fx * fx + fz * fz);
        if (x2 >= x1) {
            if (z2 >= z1) {
                angle = (u32) func_802AD7D4(port_cvt_w_s(65536.0f * ((f32) (x2 - x1) / dist))) >> 4;
            } else {
                angle = ((u32) func_802AD7D4(port_cvt_w_s(65536.0f * ((f32) (z1 - z2) / dist))) >> 4) + 0x400;
            }
        } else if (z2 < z1) {
            angle = ((u32) func_802AD7D4(port_cvt_w_s(65536.0f * ((f32) (x1 - x2) / dist))) >> 4) + 0x800;
        } else {
            angle = ((u32) func_802AD7D4(port_cvt_w_s(65536.0f * ((f32) (z2 - z1) / dist))) >> 4) + 0xC00;
        }
    }
    func_8029B7CC(angle + 0x400, angle - 0x400);
    D_803A742C = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CB54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803A7430;
u32 func_8029CF04(u32 v, s32 key);
void func_8029CF54(s32 v);
void func_8029CB54(s32 n, s32 kind, u8 *next, s32 x1, s32 z1, s32 x2, s32 z2);

/* Contact between the parts of two objects: A = the run of D_803A6B30
 * entries tagged `tag`, B = the run tagged `other` (each run starts at the
 * first entry with that tag and ends at the next entry with another tag or
 * the 0xFF end marker at byte 0x13; nothing happens if either is missing).
 * The A parts are numbered n = 1, 2, ...; each pair (a, b) is tested, with
 * b's radius (word 0xC) scaled by func_8029CF04 (key = other), skipping
 * the pair when tag == 6 and a's word 0xC is 0x30E, or other == 6 and the
 * scaled radius is 0x3BD. For an overlapping pair (func_8029CFA4): if
 * other == 7, D_803A7430 counts up while below 0xC9; then unless tag != 0
 * and func_802AB41C(other, tag) is set, func_8029CB54(n, tag, b + 0x14,
 * a.x, a.z, b.x, b.z) (heading / ring span) and func_8029CF54(other).
 * Register convention: tag in t8, other in t4 (conventions.txt); gp
 * (D_803EEA90) is read by func_8029CF04 as the global. The asm saves v0-t0,
 * t6, t7, s0, s1 and leaves t5 = -1, fp changed; its caller func_8029C9D4
 * keeps a0, a1 live (a mixed build would need a thunk). */
void func_8029CD54(s32 tag, s32 other) {
    u8 *a = D_803A6B30;
    u8 *b0 = D_803A6B30;
    u8 *b;
    s32 n = 0;
    s32 ax;
    s32 az;
    s32 ar;
    u32 br;

    for (;;) {
        if ((s8) a[0x13] == -1) {
            return;
        }
        if (a[0x12] == tag) {
            break;
        }
        a += 0x14;
    }
    for (;;) {
        if ((s8) b0[0x13] == -1) {
            return;
        }
        if (b0[0x12] == other) {
            break;
        }
        b0 += 0x14;
    }
    for (; (s8) a[0x13] != -1 && a[0x12] == tag; a += 0x14) {
        n++;
        ax = *(s32 *) (a + 0);
        az = *(s32 *) (a + 8);
        ar = *(s32 *) (a + 0xC);
        for (b = b0; (s8) b[0x13] != -1 && b[0x12] == other;) {
            br = func_8029CF04(*(s32 *) (b + 0xC), other);
            b += 0x14;
            if (tag == 6 && ar == 0x30E) {
                continue;
            }
            if (other == 6 && br == 0x3BD) {
                continue;
            }
            if (!func_8029CFA4(az, ar, *(s32 *) (b - 0x14), *(s32 *) (b - 0x10), ax, *(s32 *) (a + 4),
                               *(s32 *) (b - 0xC), br)) {
                continue;
            }
            if (other == 7 && D_803A7430 < 0xC9) {
                D_803A7430++;
            }
            if (tag != 0 && func_802AB41C(other, tag) != 0) {
                continue;
            }
            func_8029CB54(n, tag, b, ax, az, *(s32 *) (b - 0x14), *(s32 *) (b - 0xC));
            func_8029CF54(other);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CD54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Same as func_8029CB04 with the key in s1: v / gp+0xA0 (unsigned) unless
 * key == 0xFF or the byte is 1. func_8029CB04's asm branches into this
 * function's epilogue (.L8029CF40), so the two are rewritten together (in
 * the NON_MATCHING build neither keeps the label).
 * Register convention: asm takes v in t1 and key in s1, returns in t1
 * (conventions.txt); it saves v0. Its asm caller func_8029CD54 keeps a0-a3,
 * t0, t3-t8, f12, f14 live. */
u32 func_8029CF04(u32 v, s32 key) {
    u32 d;

    if (key != 0xFF) {
        d = GP_U8(0xA0);
        if (d != 1) {
            v /= d;
        }
    }
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF04.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EFECB;
extern u8 D_803649ED;

/* D_803649ED = v, unless D_803EFECB is set and the D_803ED3B8 list has the
 * pair (v, 7) (func_802AB41C).
 * Register convention: asm takes v in s1 (conventions.txt); it saves t4/t8
 * and leaves a2 changed. Its asm caller func_8029CD54 keeps a0, a1, t3-t8,
 * f12, f14 live. */
void func_8029CF54(s32 v) {
    if (D_803EFECB == 0 || func_802AB41C(v, 7) == 0) {
        D_803649ED = v;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF54.s")
#endif

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
#ifdef NON_MATCHING
extern u8 D_803BC1D0[]; /* 0xDC-byte groups */
extern u8 *D_803BD304;  /* end of the groups in use */
void func_8029D24C(s32 id, u8 *model, u8 *mtxBase);
void func_8029D120(s32 id);
u8 *func_8029D1D4(s32 sub, s32 id, s32 *last);
s32 func_8029DA90(s32 val, s32 x, s32 z, s32 n, s32 *table, Unk8029DA90Regs *r);
s32 func_8029DB7C(s32 n, u8 *pairs, Unk8029DEA0Entry *tbl);

/* Collision setup for object `id`: builds its triangle records from the
 * model (func_8029D24C), marks the records of its groups (func_8029D120),
 * then for each group g in D_803BC1D0..D_803BD304 (0xDC bytes, the end read
 * once) with g[0xC4] == id runs the group's condition: g[0xC5] == 0 is the
 * 2-D area test func_8029DA90(val, x, z, g[0xC6] triangles at g), else the
 * channel-state test func_8029DB7C(g[0xC6] pairs at g, tbl). When it gives 0,
 * the records listed in the g[0xC7] sub ids at g + 0xC8 (id 0) and the
 * g[0xD0] sub ids at g + 0xD1 (id g[0xC4]) are re-enabled (func_8029D1D4
 * clears their byte 0x51).
 * Register convention: val, tbl, x, z, id, model, mtxBase in a1, a2, t0, t1,
 * t2, t6, s4 (conventions.txt). The asm saves only ra: it leaves t4-t7,
 * s0-s7, fp and (through func_8029DA90) f12-f28 changed, all dead in its asm
 * callers (each returns straight after the call, func_802BC3D0 after
 * func_8029DC14). func_8029DA90's register in/outs (s1, s3, f12-f26) are not
 * threaded through: `r` is scratch. Callers keep t0, t2 live (no thunk
 * needed in the native port). */
void func_8029D040(s32 val, Unk8029DEA0Entry *tbl, s32 x, s32 z, s32 id, u8 *model, u8 *mtxBase) {
    u8 *g;
    u8 *end;
    u8 *p;
    s32 n;
    s32 res;
    s32 last;
    Unk8029DA90Regs r;

    func_8029D24C(id, model, mtxBase);
    func_8029D120(id);
    end = D_803BD304;
    for (g = D_803BC1D0; g != end; g += 0xDC) {
        if (g[0xC4] != id) {
            continue;
        }
        if (g[0xC5] == 0) {
            res = func_8029DA90(val, x, z, g[0xC6], (s32 *) g, &r);
        } else {
            res = func_8029DB7C(g[0xC6], g, tbl);
        }
        if (res != 0) {
            continue;
        }
        p = g + 0xC8;
        for (n = g[0xC7]; n != 0; n--) {
            func_8029D1D4(*p++, 0, &last);
        }
        p = g + 0xD1;
        for (n = g[0xD0]; n != 0; n--) {
            func_8029D1D4(*p++, g[0xC4], &last);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D040.s")
#endif

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
#ifdef NON_MATCHING
void func_8029D56C(u8 *tri);
void func_8029D534(s32 kind, u8 *rec);

/* Builds the collision triangles of object `id` from its model: u16 count at
 * model+0, u16 matrix count m at +2, m matrix offsets from +4, then one
 * 0x14-byte record per triangle (three s16 x, y, z vertices). Triangle
 * k = 1..count goes into the 0x60-byte record of D_803B9890 with byte 0x4F ==
 * id and 0x50 == k, or a new one appended at D_803BD300 (which is advanced).
 * Each vertex goes through the matrix chain (func_802AA890) and is stored
 * >> 2 (arithmetic) at +0x28 / +0x34 / +0x40. Then, from V0, V1, V2 (32-bit
 * differences, 64-bit products): normal n = (V0 - V1) x (V0 - V2) at
 * +0 / +8 / +0x10, d = -(n . V1) at +0x18, |n|^2 (f32 squares of the f32
 * components, cvt.s.l) at +0x24 and its sqrt.s at +0x20, and at +0x4E the
 * axis to drop for 2-D tests: 0 (z) if |nz| >= |nx| and |ny|, else 1 (y) if
 * |ny| >= |nx| and |nz|, else 2 (x). Finally func_8029D56C (heading) and
 * func_8029D534 (byte 0x55).
 * func_802AA890's register in/outs are threaded as the asm has them: s0 =
 * the record, s1 = the records' end, s2 = &D_803BD300 before the first
 * vertex (only used when m == 0, which then gives those >> 11), a3 dead.
 * Register convention: id, model, mtxBase in t2, t6, s4 (conventions.txt);
 * the asm saves v1-a2, t0, t1, t3 and leaves t4, t5, t7, t8, t9, s0-s3,
 * s5-s7, fp changed (dead in its caller func_8029D040, which keeps a1, a2,
 * t0-t2, t6 live). Its sub/add/dsub/dadd trap on overflow where this C
 * wraps (not reached with game coordinates). */
void func_8029D24C(s32 id, u8 *model, u8 *mtxBase) {
    s32 count = *(u16 *) model;
    s32 sub = 1;

    for (; count != 0; count--, sub++) {
        u8 *rec = D_803B9890;
        u8 *end = D_803BD300;
        MtxChainRegs regs;
        s32 m;
        s32 *offs;
        s16 *v;
        s32 *p;
        s32 i;
        s64 dz02;
        s64 dy01;
        s64 dy02;
        s64 dz01;
        s64 dx02;
        s64 dx01;
        s64 nx;
        s64 ny;
        s64 nz;
        f32 fx;
        f32 fy;
        f32 fz;
        f32 sum;

        for (;;) {
            if (rec == end) {
                end += 0x60;
                break;
            }
            if (rec[0x4F] == id && rec[0x50] == sub) {
                break;
            }
            rec += 0x60;
        }
        D_803BD300 = end;
        rec[0x4F] = id;
        rec[0x50] = sub;
        m = *(u16 *) (model + 2);
        offs = (s32 *) (model + 4);
        v = (s16 *) (model + (m << 2) + 4 + (sub - 1) * 0x14);
        regs.a3 = 0;
        regs.s0 = (s32) rec;
        regs.s1 = (s32) end;
        regs.s2 = (s32) &D_803BD300;
        p = (s32 *) (rec + 0x28);
        for (i = 0; i < 3; i++) {
            p[0] = func_802AA890(m, offs, mtxBase, v[0], v[1], v[2], &regs) >> 2;
            p[1] = regs.v1 >> 2;
            p[2] = regs.a0 >> 2;
            p += 3;
            v += 3;
        }
        p = (s32 *) (rec + 0x28);
        dz02 = p[2] - p[8];
        dy01 = p[1] - p[4];
        dy02 = p[1] - p[7];
        dz01 = p[2] - p[5];
        dx02 = p[0] - p[6];
        dx01 = p[0] - p[3];
        nx = dy01 * dz02 - dz01 * dy02;
        ny = dz01 * dx02 - dx01 * dz02;
        nz = dx01 * dy02 - dy01 * dx02;
        *(s64 *) (rec + 0x00) = nx;
        *(s64 *) (rec + 0x08) = ny;
        *(s64 *) (rec + 0x10) = nz;
        fx = (f32) nx;
        fy = (f32) ny;
        fz = (f32) nz;
        sum = fx * fx + fy * fy + fz * fz;
        *(f32 *) (rec + 0x24) = sum;
        *(s64 *) (rec + 0x18) = -(nx * p[3] + ny * p[4] + nz * p[5]);
        *(f32 *) (rec + 0x20) = sqrtf(sum);
        if (nx < 0) {
            nx = -nx;
        }
        if (ny < 0) {
            ny = -ny;
        }
        if (nz < 0) {
            nz = -nz;
        }
        if (!(nz < nx) && !(nz < ny)) {
            rec[0x4E] = 0;
        } else if (!(ny < nx) && !(ny < nz)) {
            rec[0x4E] = 1;
        } else {
            rec[0x4E] = 2;
        }
        func_8029D56C(rec);
        func_8029D534(id, rec);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D24C.s")
#endif

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
#ifdef NON_MATCHING
f64 sqrt(f64);
#pragma intrinsic(sqrt)
s32 func_802AD7FC(u32 sine);
void func_8029D90C(u8 *tri, s64 *out);

/* cvt.l.d under the game's FCSR: f64 -> s64, round to nearest, ties to even. */
static s64 port_cvt_l_d(f64 x) {
    s64 t = (s64) x;
    f64 frac = x - (f64) t;

    if (frac > 0.5 || (frac == 0.5 && (t & 1))) {
        t++;
    } else if (frac < -0.5 || (frac == -0.5 && (t & 1))) {
        t--;
    }
    return t;
}

/* Heading and facing side of the triangle whose vertices are the u32 triples
 * at tri+0x28, +0x34, +0x40 (each >> 3, logical). With n = (V2 - V0) x
 * (V1 - V0) in 64 bits (32-bit differences; nx = dy2*dz1 - dz2*dy1, ...):
 * angle = arcsine(func_802AD7FC) of (|nx| << 16) / round(sqrt(nx^2 + nz^2))
 * (64-bit signed division; the asm traps (break 7) when nx = nz = 0), >> 4,
 * folded by the signs of nx / nz into 0..0xFFF (nx < 0: 0xFFF - a if nz >= 0,
 * else a + 0x800; nx >= 0 and nz < 0: 0x800 - a). Then the point V0 + 100 *
 * n / |n| (double, each component rounded as cvt.l.d, added in 32 bits) is
 * put into the plane function of func_8029D90C (normal and d, 64-bit): byte
 * 0x56 = 0 if it is on d's side (s > 0 with d > 0, s <= 0 with d < 0), else
 * 1 and the angle turns by 0x800 (wrapping by + 0xFFF when negative). The
 * angle goes to the halfword at 0x4C.
 * Register convention: tri in s0 (conventions.txt); the asm saves and
 * restores every register. Its add/dsub trap on overflow where this C wraps
 * (not reached with game coordinates). */
void func_8029D56C(u8 *tri) {
    u32 *w = (u32 *) (tri + 0x28);
    s32 x0 = w[0] >> 3;
    s32 y0 = w[1] >> 3;
    s32 z0 = w[2] >> 3;
    s32 x1 = w[3] >> 3;
    s32 y1 = w[4] >> 3;
    s32 z1 = w[5] >> 3;
    s32 x2 = w[6] >> 3;
    s32 y2 = w[7] >> 3;
    s32 z2 = w[8] >> 3;
    s64 dy2 = (s32) (y2 - y0);
    s64 dz1 = (s32) (z1 - z0);
    s64 dz2 = (s32) (z2 - z0);
    s64 dy1 = (s32) (y1 - y0);
    s64 dx1 = (s32) (x1 - x0);
    s64 dx2 = (s32) (x2 - x0);
    s64 nx = dy2 * dz1 - dz2 * dy1;
    s64 ny = dz2 * dx1 - dx2 * dz1;
    s64 nz = dx2 * dy1 - dy2 * dx1;
    s64 ax = (nx < 0) ? -nx : nx;
    s64 az = (nz < 0) ? -nz : nz;
    s64 len = port_cvt_l_d(sqrt((f64) (az * az + ax * ax)));
    s32 angle = (u32) func_802AD7FC((u32) ((ax << 16) / len)) >> 4;
    f64 n3;
    s32 px;
    s32 py;
    s32 pz;
    s64 pl[4];
    s64 s;

    if (nx < 0) {
        if (nz >= 0) {
            angle = 0xFFF - angle;
        } else {
            angle += 0x800;
        }
    } else if (nz < 0) {
        angle = 0x800 - angle;
    }
    n3 = sqrt((f64) (nx * nx + ny * ny + nz * nz));
    px = x0 + (s32) port_cvt_l_d((f64) nx / n3 * 100.0);
    py = y0 + (s32) port_cvt_l_d((f64) ny / n3 * 100.0);
    pz = z0 + (s32) port_cvt_l_d((f64) nz / n3 * 100.0);
    func_8029D90C(tri, pl);
    tri[0x56] = 0;
    s = px * pl[0] + py * pl[1] + pz * pl[2] + pl[3];
    if (!((s > 0) ? (pl[3] > 0) : (pl[3] < 0))) {
        tri[0x56] = 1;
        angle -= 0x800;
        if (angle < 0) {
            angle += 0xFFF;
        }
    }
    *(s16 *) (tri + 0x4C) = angle;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D56C.s")
#endif

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
#ifdef NON_MATCHING
/* (Unk8029DA90Regs is declared in the file-level block at the top.) */

/* Walks `n` 2-D triangles (6 words: x0, z0, x1, z1, x2, z2) from `tri` for
 * the first that contains (x, z) (bounding box, func_802AA5E0, then the edge
 * test func_802AA460). Then reads the u16 pair (lo, hi) that follows the
 * table and returns 0 if `val` lies in [lo, hi] (wrapping round when hi < lo),
 * else 1; 1 if no triangle contains the point. r->s1/s3 get each triangle's
 * x0/z0 as it is tried, then lo/hi; r->fp gets func_802AA460's FP results.
 * Register convention: asm takes val, x, z, n, tri in a1, t0, t1, t5, s4 and
 * returns in t4, plus s1, s3, f12-f26 (conventions.txt); it clobbers s6, s7
 * (loaded for the callees) and f28, restores s4. Its asm caller
 * func_8029D040 keeps a1-a3, t0-t2, t6, t7 live. */
s32 func_8029DA90(s32 val, s32 x, s32 z, s32 n, s32 *table, Unk8029DA90Regs *r) {
    s32 *tri = table; /* (a local: the stack argument itself must stay unchanged) */
    u16 *range;
    s32 lo;
    s32 hi;

    while (n != 0) {
        n--;
        r->s1 = tri[0];
        r->s3 = tri[1];
        tri += 6;
        if (func_802AA5E0(x, z, r->s1, r->s3, tri[-4], tri[-3], tri[-2], tri[-1]) == 0) {
            continue;
        }
        if (func_802AA460(x, z, r->s1, r->s3, tri[-4], tri[-3], tri[-2], tri[-1], &r->fp) == 0) {
            continue;
        }
        range = (u16 *) (tri + n * 6);
        lo = range[0];
        hi = range[1];
        r->s1 = lo;
        r->s3 = hi;
        if (hi < lo) {
            return (val < lo && hi < val) ? 1 : 0;
        }
        return (val < lo || hi < val) ? 1 : 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DA90.s")
#endif

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
#ifdef NON_MATCHING
void func_8029E21C(Unk8029DEA0Entry *ch, u8 *param);

/* Steps every active texture-animation channel: func_8029E21C(ch, param)
 * for each entry of the -1-terminated table D_803B35F8 whose byte unk10 is
 * nonzero.
 * Register convention: param in fp (conventions.txt): the asm passes its
 * caller's fp through to func_8029E21C, whose texture loads hand it to the
 * decoder (func_802A1074). Its only caller, func_802475D8 in 00000.c, is C
 * and declares `void func_8029E0AC(void)`, so what it passes is whatever its
 * fp (and here a0) happens to hold: a port must decide what that should be.
 * The asm saves and restores every register (f20-f30 included). */
void func_8029E0AC(u8 *param) {
    Unk8029DEA0Entry *e;

    for (e = D_803B35F8; e->id != -1; e++) {
        if (e->unk10 != 0) {
            func_8029E21C(e, param);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E0AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (Unk8029F1BCOut is declared in the file-level block at the top.) */
f32 func_8029F1BC(f32 delta, f32 frac, Unk8029DEA0Entry *ch, s32 frame, s32 n, Unk8029F1BCOut *o);
s32 func_8029E4E4(s32 key, s32 sub);
u32 func_8029E47C(s32 key, s32 id, u8 *param);
extern u8 D_80364460[]; /* 0x74-byte records: s32 id at +0x5C, data bases at +0xC / +0x30 */

/* Texture block for (key, sub): the loaded one (func_8029E4E4) or a new load
 * (func_8029E47C with param). */
#define TEX_BLOCK(key, sub, param) \
    ((r_ = func_8029E4E4((key), (sub))) != 0 ? r_ : (s32) func_8029E47C((key), (sub), (param)))
/* (sub is evaluated twice: pass a plain variable) */

/* Advances the texture animation of channel ch (ch->id = the animation data
 * `a`: u8 id at 0, frame count n at 1, textures per frame k at 2, blend flag
 * at 3, n u16 keys from 4, then n frames of k u16 texture ids). First every
 * in-use block (byte 6) of D_803A7440 tagged with a gets its use count (byte
 * 7) incremented. The playback step is ((key[f + 1] - key[f]) * frac +
 * key[f]) / 300 (f = ch->unk13, frac = ch->unk4), fed to func_8029F1BC,
 * which gives the new frame f' and frac'. For each texture i of frame f'
 * (and of frame f' + 1 when blending) the block is found or loaded
 * (TEX_BLOCK), and every 0xC-byte patch {a, off, i} in [D_803B3500,
 * D_803B35F0) stores it at base + off (base = the level's D_80364460
 * record with id a[0]: +0xC if D_8035805C else +0x30; unbounded scan);
 * when blending, the second block goes to base + the next patch's off and
 * the first command after it with opcode 0xFA gets its low byte(s) =
 * round(frac' * 255) (the patch after is skipped). Finally blocks tagged
 * with a still in use with a count >= 2 are released (byte 6 = 0).
 * Register convention: ch in t0, param in fp (conventions.txt); the asm
 * restores t0 and leaves s0-s6 and f30 changed (also f12 / f14 as
 * func_8029E47C's callee leaves them; its caller func_8029E0AC is said to
 * read those, not modelled). */
void func_8029E21C(Unk8029DEA0Entry *ch, u8 *param) {
    u8 *a = (u8 *) ch->id;
    s32 key = ch->id;
    s32 frame = ch->unk13;
    f32 frac = ch->unk4;
    u16 *keys = (u16 *) (a + 4) + frame;
    Unk8029F1BCOut o;
    s32 n;
    s32 k;
    s32 blend;
    u16 *ids;
    u16 *ids2;
    s32 i;
    s32 r_;
    u8 *e;
    u8 *rec;
    s32 base;

    for (i = 0; i < 12; i++) {
        if (*(s32 *) D_803A7440[i] == key && D_803A7440[i][6] != 0) {
            D_803A7440[i][7]++;
        }
    }
    n = a[1];
    o.frame = frame;
    func_8029F1BC(((f32) (keys[1] - keys[0]) * frac + (f32) keys[0]) / 300.0f, frac, ch, frame, n, &o);
    k = a[2];
    blend = a[3];
    ids = (u16 *) (a + n * 2 + 4 + (u32) (k * 2) * o.frame);
    ids2 = ids + k;
    for (i = 0; k != 0; k--, i++) {
        s32 sub = *ids++;
        s32 first = TEX_BLOCK(key, sub, param);
        s32 second = first;

        if (blend != 0) {
            sub = *ids2++;
            second = TEX_BLOCK(key, sub, param);
        }
        for (rec = D_80364460; *(s32 *) (rec + 0x5C) != a[0]; rec += 0x74) {
        }
        base = (D_8035805C != 0) ? *(s32 *) (rec + 0xC) : *(s32 *) (rec + 0x30);
        for (e = D_803B3500; e != (u8 *) D_803B35F0; e += 0xC) {
            u32 *p;
            u32 w;

            if (*(s32 *) (e + 0) != key || *(s32 *) (e + 8) != i) {
                continue;
            }
            *(s32 *) (base + *(s32 *) (e + 4)) = first;
            if (blend == 0) {
                continue;
            }
            p = (u32 *) (base + *(s32 *) (e + 0x10));
            e += 0xC;
            *p++ = second;
            do {
                w = *p;
                p += 2;
            } while ((w & 0xFF000000) >> 24 != 0xFA);
            p[-2] = port_cvt_w_s(o.frac * 255.0f) | 0xFA000000;
        }
    }
    for (i = 0; i < 12; i++) {
        if (D_803A7440[i][6] != 0 && D_803A7440[i][7] >= 2 && *(s32 *) D_803A7440[i] == key) {
            D_803A7440[i][6] = 0;
        }
    }
}
#undef TEX_BLOCK
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E21C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802A1074(s32 id, u8 *dest, u8 *param); /* 5BF40 */

/* Takes the first of the 12 0x1010-byte blocks at D_803A7440 with byte 6
 * clear (when all 12 are in use, the asm runs on to the block just past the
 * table; kept), tags it (word 0 = key, u16 +4 = id, byte 6 = 1, byte 7 = 0),
 * queues the load of table entry `id` into its data at +0x10 with decode
 * param `param` (func_802A1074) and returns the physical address of that
 * data (block + 0x10 - 0x80000000): the counterpart of func_8029E4E4.
 * Register convention: asm takes key, id, param in t3, t6, fp and returns in
 * s1 (conventions.txt); it changes a0-a2. Its asm caller func_8029E21C keeps
 * t0, t3 live. */
u32 func_8029E47C(s32 key, s32 id, u8 *param) {
    u8 *b = D_803A7440[0];
    s32 n = 12;

    while (n != 0) {
        n--;
        if (b[6] == 0) {
            break;
        }
        b += 0x1010;
    }
    *(s32 *) b = key;
    *(s16 *) (b + 4) = id;
    b[6] = 1;
    b[7] = 0;
    func_802A1074(id, b + 0x10, param);
    return (u32) (b + 0x10) - 0x80000000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E47C.s")
#endif

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
#ifdef NON_MATCHING
extern u8 *D_803B3770; /* the other matrix buffer of the pair (see func_8029E5AC) */
void func_8029E5AC(Unk8029DEA0Entry *ch, u8 *base);

/* Animates a model's 32 channels: D_803B3770 = other, then
 * func_8029E5AC(ch, base) for each of the 32 0x18-byte channels from ch
 * whose byte unk10 is nonzero.
 * Register convention: base, other, ch in v0, v1, t0 (conventions.txt). The
 * asm saves s6, s7 and leaves t0 advanced, t1 = 0, t2 and s0-s5, fp, f20,
 * f30 as func_8029E5AC leaves them; none of those is read by its asm callers
 * (the survey's list is their later saves). */
void func_8029E558(u8 *base, u8 *other, Unk8029DEA0Entry *ch) {
    s32 n;

    D_803B3770 = other;
    for (n = 0x20; n != 0; n--, ch++) {
        if (ch->unk10 != 0) {
            func_8029E5AC(ch, base);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E558.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803B3770;
void func_8029F110(u8 *p, s32 mode, Unk8029F110Out *o);
f32 func_8029F060(f32 t, s32 idx, s32 n);
void func_8029EF80(u8 *mtx, s16 *key, f32 t);
void func_8029E730(u8 *rec, u8 *mtx, f32 t);
void func_802ACCCC(s32 *b, s32 *m); /* 679E0 */
void func_802AC8CC(u16 *m);         /* 679E0 */

/* One animation channel of a model (matrices in buffer `base`, the other
 * buffer of the pair in D_803B3770). ch->id points at the track data a:
 * n = a[0] frames, u8 keys from a + 1, then the part count a[n + 1] and, at
 * a + n + 2 rounded up to 4, the part records (stride n * 0x14 + 8: s32 dst
 * offset, s32 src offset, n 0x14-byte keyframes). The playback step
 * ((key[f + 1] - key[f]) * frac + key[f]) / 300 (f = ch->unk13, frac =
 * ch->unk4) goes to func_8029F1BC, which advances the channel (new frame f',
 * frac'). For spline channels (mode ch->unk15 != 0) the basis and control
 * indices are set up (func_8029F110, func_8029F060(frac', f', n)). Then for
 * each part: copy the 64-byte matrix base + src to base + dst, apply the
 * keyframe transform (mode 0: linear, func_8029EF80 with keyframe f'; mode 1:
 * spline, func_8029E730; other modes hit a `syscall` debug trap in the asm,
 * not modelled), multiply by the matrix that follows the source
 * (func_802ACCCC(base + src + 0x40, dst)), convert to Mtx layout
 * (func_802AC8CC) and then cancel the queued copies to dst (func_8029DD54)
 * if ch->unk10 (read after func_8029F1BC, which may clear it) is 1, else
 * queue a copy of dst to the same offset in D_803B3770 (func_8029DCD4).
 * Register convention: ch, base in t0, v0 (conventions.txt). The asm saves
 * t0, t1 and leaves t2-t7, s0-s7, fp, f20 (1.0 from func_8029F110), f30
 * changed, none of them read by its caller func_8029E558 (which keeps t0,
 * t1). Its add/sub/addi trap on overflow (not reached). */
void func_8029E5AC(Unk8029DEA0Entry *ch, u8 *base) {
    s32 frame = ch->unk13;
    u8 *a = (u8 *) ch->id;
    f32 frac = ch->unk4;
    u8 *k = a + frame;
    s32 n = a[0];
    s32 k0 = k[1];
    s32 mode;
    s32 flag;
    s32 parts;
    s32 stride;
    s32 i;
    u8 *rec;
    Unk8029F1BCOut o;
    Unk8029F110Out o2;

    o.frame = frame;
    func_8029F1BC(((f32) (k[2] - k0) * frac + (f32) k0) / 300.0f, frac, ch, frame, n, &o);
    mode = ch->unk15;
    flag = (u8) ch->unk10;
    if (mode != 0) {
        func_8029F110((u8 *) ch, mode, &o2);
        func_8029F060(o.frac, o.frame, n);
    }
    stride = n * 0x14 + 8;
    rec = a + n + 2;
    if ((u32) rec & 3) {
        rec += 4 - ((u32) rec & 3);
    }
    for (parts = a[n + 1]; parts != 0; parts--, rec += stride) {
        s32 dstOff = *(s32 *) (rec + 0);
        s32 srcOff = *(s32 *) (rec + 4);
        u64 *dst = (u64 *) (base + dstOff);
        u64 *src = (u64 *) (base + srcOff);

        for (i = 0; i < 8; i++) {
            dst[i] = src[i];
        }
        if (mode == 0) {
            func_8029EF80((u8 *) dst, (s16 *) (rec + 8 + o.frame * 0x14), o.frac);
        } else if (mode == 1) {
            func_8029E730(rec, (u8 *) dst, o.frac);
        }
        func_802ACCCC((s32 *) (src + 8), (s32 *) dst);
        func_802AC8CC((u16 *) dst);
        if (flag == 1) {
            func_8029DD54((u8 *) dst);
        } else {
            func_8029DCD4(D_803B3770 + dstOff, (u8 *) dst);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E5AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B7FC0[4];
void func_802ACCCC(s32 *b, s32 *m); /* 679E0 */
s32 func_8029E938(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t);
s32 func_8029EA48(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t);
s32 func_8029EB58(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t);
s32 func_8029EC68(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t);
s32 func_8029EDEC(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t);

/* Spline keyframe transform of one part: with the four control keyframes
 * rec + 8 + D_803B7FC0[i] * 0x14 (set by func_8029F060), multiplies mtx by
 * each non-trivial step matrix built in D_803B3730 (func_802ACCCC), in the
 * order func_8029EDEC, func_8029EB58 (z), func_8029EA48 (y), func_8029E938
 * (x), func_8029EC68.
 * Register convention: rec, mtx, t in s1, s2, f30 (conventions.txt; the
 * survey misses s1 and f30). The asm saves v0, a0, a3, t5-t7, s0, s1, fp;
 * its caller func_8029E5AC keeps t2, t4-t7 live. */
void func_8029E730(u8 *rec, u8 *mtx, f32 t) {
    u8 *keys = rec + 8;
    u8 *k0 = keys + D_803B7FC0[0] * 0x14;
    u8 *k1 = keys + D_803B7FC0[1] * 0x14;
    u8 *k2 = keys + D_803B7FC0[2] * 0x14;
    u8 *k3 = keys + D_803B7FC0[3] * 0x14;

    if (func_8029EDEC(k0, k1, k2, k3, t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029EB58(k0, k1, k2, k3, t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029EA48(k0, k1, k2, k3, t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029E938(k0, k1, k2, k3, t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029EC68(k0, k1, k2, k3, t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E730.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Evaluates the cubic spline segment through control values p0..p3 at t:
 * c_j = B[0][j] p0 + B[1][j] p1 + B[2][j] p2 + B[3][j] p3 for the basis
 * B = D_803B3778, then c0 t^3 + c1 t^2 + c2 t + c3, with t^3 / t^2 taken from
 * D_803B37BC / D_803B37B8 (func_8029F060) and t itself from the argument.
 * Same single-precision operation order as the asm.
 * Register convention: p0..p3 in f0, f2, f4, f6, t in f30, result in f8
 * (conventions.txt; the asm also leaves the last product in f12, which its
 * callers don't use). It restores v0/v1; asm callers keep a0-a3, t3-t7, f14
 * live. */
f32 func_8029E878(f32 p0, f32 p1, f32 p2, f32 p3, f32 t) {
    f32 *b = D_803B3778;
    f32 acc;
    f32 c;
    s32 i;

    for (i = 3;; i--) {
        c = b[0] * p0;
        c = c + b[4] * p1;
        c = c + b[8] * p2;
        c = c + b[12] * p3;
        if (i == 3) {
            acc = c * D_803B37BC;
        } else if (i == 2) {
            acc = acc + c * D_803B37B8;
        } else if (i == 1) {
            acc = acc + c * t;
        } else {
            acc = acc + c;
            break;
        }
        b++;
    }
    return acc;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E878.s")
#endif

#ifdef NON_MATCHING
s32 func_802AE104(s32 angle);
s32 func_802AE160(s32 angle);

/* 16.16 rotation matrices at m from sine s and cosine c (about x, y, z). */
#define ROT_MTX_X(m, s, c)                                     \
    ((m)[0] = 0x10000, (m)[1] = 0, (m)[2] = 0, (m)[3] = 0,     \
     (m)[4] = 0, (m)[5] = (c), (m)[6] = (s), (m)[7] = 0,       \
     (m)[8] = 0, (m)[9] = -(s), (m)[10] = (c), (m)[11] = 0,    \
     (m)[12] = 0, (m)[13] = 0, (m)[14] = 0, (m)[15] = 0x10000)
#define ROT_MTX_Y(m, s, c)                                     \
    ((m)[0] = (c), (m)[1] = 0, (m)[2] = -(s), (m)[3] = 0,      \
     (m)[4] = 0, (m)[5] = 0x10000, (m)[6] = 0, (m)[7] = 0,     \
     (m)[8] = (s), (m)[9] = 0, (m)[10] = (c), (m)[11] = 0,     \
     (m)[12] = 0, (m)[13] = 0, (m)[14] = 0, (m)[15] = 0x10000)
#define ROT_MTX_Z(m, s, c)                                     \
    ((m)[0] = (c), (m)[1] = (s), (m)[2] = 0, (m)[3] = 0,       \
     (m)[4] = -(s), (m)[5] = (c), (m)[6] = 0, (m)[7] = 0,      \
     (m)[8] = 0, (m)[9] = 0, (m)[10] = 0x10000, (m)[11] = 0,   \
     (m)[12] = 0, (m)[13] = 0, (m)[14] = 0, (m)[15] = 0x10000)

/* Spline-interpolated rotation angle: func_8029E878 of the s16 at `off` in
 * the four keyframes at t, rounded as cvt.w.s does. */
#define KEY_ANGLE(k0, k1, k2, k3, off, t)                                        \
    port_cvt_w_s(func_8029E878((f32) *(s16 *) ((u8 *) (k0) + (off)),             \
                               (f32) *(s16 *) ((u8 *) (k1) + (off)),             \
                               (f32) *(s16 *) ((u8 *) (k2) + (off)),             \
                               (f32) *(s16 *) ((u8 *) (k3) + (off)), (t)))
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_8029E938 / func_8029EA48 / func_8029EB58: spline-interpolated rotation
 * about x / y / z from four 0x14-byte keyframes (angle s16 at 6 / 8 / 0xA,
 * 0x1000 = 360 degrees in 1/16 units). If the angle rounds to 0 returns 0
 * (nothing written); else D_803B3730 = the 16.16 rotation matrix with sine /
 * cosine func_802AE160 / func_802AE104 of angle >> 4 (srl), returns 1.
 * Register convention: keys in t5, t6, t7, s0, t in f30, result in a3
 * (conventions.txt), as func_8029EC68. The asm saves every other register it
 * uses except the callee's f0-f8 scratch (the angle's cvt.w.s bits stay in
 * f8). Its `neg` traps on 0x80000000 (a 16.16 sine never is). */
s32 func_8029E938(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t) {
    s32 *m = D_803B3730;
    s32 a = KEY_ANGLE(k0, k1, k2, k3, 6, t);
    s32 s;
    s32 c;

    if (a == 0) {
        return 0;
    }
    s = func_802AE160((u32) a >> 4);
    c = func_802AE104((u32) a >> 4);
    ROT_MTX_X(m, s, c);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E938.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8029EA48(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t) {
    s32 *m = D_803B3730;
    s32 a = KEY_ANGLE(k0, k1, k2, k3, 8, t);
    s32 s;
    s32 c;

    if (a == 0) {
        return 0;
    }
    s = func_802AE160((u32) a >> 4);
    c = func_802AE104((u32) a >> 4);
    ROT_MTX_Y(m, s, c);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EA48.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8029EB58(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t) {
    s32 *m = D_803B3730;
    s32 a = KEY_ANGLE(k0, k1, k2, k3, 0xA, t);
    s32 s;
    s32 c;

    if (a == 0) {
        return 0;
    }
    s = func_802AE160((u32) a >> 4);
    c = func_802AE104((u32) a >> 4);
    ROT_MTX_Z(m, s, c);
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EB58.s")
#endif

/* func_8029EC68 / func_8029EDEC: spline-interpolated translation / scale
 * matrices from four 0x14-byte keyframes (s16 fields; scale at 0/2/4,
 * translation at 0xC/0xE/0x10). Each axis is func_8029E878 of the four keys'
 * values at t, rounded to an integer as cvt.w.s does (nearest even).
 * Register convention: keys in t5, t6, t7, s0, t in f30, result (0 = identity,
 * nothing written) in a3 (conventions.txt). The asm saves the other registers
 * it uses; it leaves the last axis in f8 (as cvt.w.s bits) and the callee's
 * scratch in f12, which the C doesn't reproduce. Asm caller func_8029E730
 * keeps t4, f14 (and a0-a2, t5-t7) live. */
#ifdef NON_MATCHING
#define KEY_S16(k, off) ((f32) *(s16 *) ((u8 *) (k) + (off)))
#define KEY_SPLINE(k0, k1, k2, k3, off, t) \
    port_cvt_w_s(func_8029E878(KEY_S16(k0, off), KEY_S16(k1, off), KEY_S16(k2, off), KEY_S16(k3, off), t))

f32 func_8029E878(f32 p0, f32 p1, f32 p2, f32 p3, f32 t);

/* Translation: 0 if all three axes round to 0, else D_803B3730 = identity
 * (16.16) with the axes << 16 in the last row, and 1. */
s32 func_8029EC68(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t) {
    s32 *m = D_803B3730;
    s32 x = KEY_SPLINE(k0, k1, k2, k3, 0xC, t);
    s32 y = KEY_SPLINE(k0, k1, k2, k3, 0xE, t);
    s32 z = KEY_SPLINE(k0, k1, k2, k3, 0x10, t);

    if (x == 0 && y == 0 && z == 0) {
        return 0;
    }
    m[0] = 0x10000;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = 0x10000;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = 0x10000;
    m[11] = 0;
    m[12] = x << 16;
    m[13] = y << 16;
    m[14] = z << 16;
    m[15] = 0x10000;
    return 1;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EC68.s")
#endif

#ifdef NON_MATCHING
/* Scale (0x100 = 1.0): 0 if all three axes are 0x100, else D_803B3730 =
 * diag((s32) (axis << 16) >> 8 each, 0x10000), and 1. */
s32 func_8029EDEC(u8 *k0, u8 *k1, u8 *k2, u8 *k3, f32 t) {
    s32 *m = D_803B3730;
    s32 x = KEY_SPLINE(k0, k1, k2, k3, 0, t);
    s32 y = KEY_SPLINE(k0, k1, k2, k3, 2, t);
    s32 z = KEY_SPLINE(k0, k1, k2, k3, 4, t);

    if (x == 0x100 && y == 0x100 && z == 0x100) {
        return 0;
    }
    m[0] = (x << 16) >> 8;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = (y << 16) >> 8;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = (z << 16) >> 8;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
    return 1;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EDEC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802ACCCC(s32 *b, s32 *m); /* 679E0 */
s32 func_8029F3D0(s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, f32 t);
s32 func_8029F4B8(s32 a, s32 b, f32 t);
s32 func_8029F560(s32 a, s32 b, f32 t);
s32 func_8029F608(s32 a, s32 b, f32 t);
s32 func_8029F760(s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, f32 t);

/* Linear keyframe transform of one part: between keyframe k (ten s16: scale
 * x/y/z at 0..4, angles x/y/z at 6..0xA, translation at 0xC..0x10) and the
 * next one (k + 0x14) at t, multiplies mtx by each non-trivial step matrix
 * built in D_803B3730 (func_802ACCCC): scale (func_8029F760), rotation about
 * z, y, x (func_8029F608, func_8029F560, func_8029F4B8), translation
 * (func_8029F3D0).
 * Register convention: mtx, k, t in s2, s3, f30 (conventions.txt). The asm
 * saves fp and leaves a0-a3, t0, t1, v1 and f8-f14 as the callees leave
 * them (dead in its caller func_8029E5AC, which keeps t2, t4-t7). */
void func_8029EF80(u8 *mtx, s16 *k, f32 t) {
    if (func_8029F760(k[0], k[1], k[2], k[10], k[11], k[12], t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029F608(k[5], k[15], t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029F560(k[4], k[14], t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029F4B8(k[3], k[13], t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
    if (func_8029F3D0(k[6], k[7], k[8], k[16], k[17], k[18], t)) {
        func_802ACCCC(D_803B3730, (s32 *) mtx);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EF80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B7FC0[4];

/* Stores t^2 / t^3 for func_8029E878 and the four control-point indices
 * around frame `idx` of an n-frame track: D_803B7FC0[0..3] = idx - 1
 * (+ (n - 1) if negative), idx, idx + 1 and idx + 2 (each - (n - 1) unless
 * below n - 1). Returns t^3 (left in f8 by the asm).
 * Register convention: t in f30, idx in t2, n in t6 (conventions.txt);
 * restores v0/v1. Asm callers keep a0-a3, t2-t7, f12, f14 live. */
f32 func_8029F060(f32 t, s32 idx, s32 n) {
    f32 t2 = t * t;
    f32 t3;
    s32 last = n - 1;
    s32 k;

    D_803B37B8 = t2;
    t3 = t2 * t;
    D_803B37BC = t3;
    k = idx - 1;
    D_803B7FC0[0] = (k < 0) ? k + last : k;
    D_803B7FC0[1] = idx;
    k = idx + 1;
    D_803B7FC0[2] = (k < last) ? k : k - last;
    k = idx + 2;
    D_803B7FC0[3] = (k < last) ? k : k - last;
    return t3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F060.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (Unk8029F110Out is declared in the file-level block at the top.) */

/* If mode == 1, fills the spline basis D_803B3778 for tension s = the f32 at
 * p+8 (a cardinal-spline matrix):
 *   [ -s, 2s, -s, 0;  2-s, s-3, 0, 1;  s-2, 3-2s, s, 0;  s, -s, 0, 0 ]
 * and leaves f12 = 2.0, f14 = s - 2; f20 = 1.0 in any case.
 * Register convention: p in t0, mode in fp; outputs f12, f14, f20
 * (conventions.txt). Restores v0; asm callers keep a0-a3, t2-t7 live. */
void func_8029F110(u8 *p, s32 mode, Unk8029F110Out *o) {
    f32 s = *(f32 *) (p + 8);
    f32 n = -s;
    f32 *b = D_803B3778;

    o->f20 = 1.0f;
    if (mode == 1) {
        f32 s2 = s * 2.0f;

        b[0] = n;
        b[2] = n;
        b[3] = 0.0f;
        b[6] = 0.0f;
        b[7] = 1.0f;
        b[10] = s;
        b[11] = 0.0f;
        b[12] = s;
        b[1] = s2;
        b[13] = n;
        b[14] = 0.0f;
        b[15] = 0.0f;
        b[9] = 3.0f - s2;
        b[4] = 2.0f - s;
        b[5] = s - 3.0f;
        b[8] = s - 2.0f;
        o->f12 = 2.0f;
        o->f14 = s - 2.0f;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F110.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (Unk8029F1BCOut, its register outputs, is declared above func_8029E21C.) */

/* Advances animation channel ch through an n-frame track by delta * ch->unk14
 * (speed). Forward (ch->unk11 != 1): frac += step; whole frames move
 * `frame`; past the last segment (frame >= n - 1) the loop count ch->unkC
 * grows by frame / (n - 1) (unsigned); if it reaches ch->unkE (unless that is
 * -1) the channel stops (unk10 = 0, frac = 1.0, frame = n - 2, dir = 1 if
 * the mode ch->unk12 is 1), else mode 0 wraps (frame %= n - 1, unsigned)
 * and mode 1 turns around (frac = 1.0, dir = 1, frame = n - 2). Backward
 * (dir 1) mirrors it towards frame 0 (loop count + 1 per wrap; mode 0 wraps
 * to (n - 1) - (-frame % (n - 1)), mode 1 turns around at 0).
 * Writes ch->unk11 = dir, ch->unk13 = frame, ch->unk4 = frac and returns
 * (f32) ch->unk14 (the asm's f8). Modes other than 0/1 hit a `syscall` in
 * the asm (a debug trap); they are not modelled. The asm also divides
 * -frame by n on the backward wrap path and discards the quotient (its only
 * effect is a divide-by-zero break when n == 0); omitted.
 * Register convention: delta in f6, frac in f30, ch in t0, frame in t2, n in
 * t6; outputs t2, t7, f30, f8; s0-s2 scratch (conventions.txt). */
f32 func_8029F1BC(f32 delta, f32 frac, Unk8029DEA0Entry *ch, s32 frame, s32 n, Unk8029F1BCOut *o) {
    f32 speed = ch->unk14;
    s32 dir = ch->unk11;
    s32 mode = (u8) ch->unk12;
    s32 whole;
    s32 cnt;
    s32 lim;
    f32 step = delta * speed;

    if (dir != 1) {
        frac = frac + step;
        whole = (s32) frac;
        frame = frame + whole;
        frac = frac - (f32) whole;
        if (frame >= n - 1) {
            cnt = ch->unkC + (u32) frame / (u32) (n - 1);
            lim = ch->unkE;
            ch->unkC = cnt;
            if (cnt >= lim && lim != -1) {
                frac = 1.0f;
                if (mode == 1) {
                    dir = 1;
                }
                frame = n - 2;
                ch->unk10 = 0;
            } else if (mode == 0) {
                frame = (u32) frame % (u32) (n - 1);
            } else if (mode == 1) {
                frac = 1.0f;
                dir = 1;
                frame = n - 2;
            }
        }
    } else {
        frac = frac - step;
        if (frac <= 0.0f) {
            f32 w = (f32) (s32) frac - 1.0f;

            frac = frac - w;
            frame = frame + port_cvt_w_s(w);
        }
        if (frame < 0) {
            cnt = ch->unkC + dir;
            lim = ch->unkE;
            ch->unkC = cnt;
            if (cnt >= lim && lim != -1) {
                frac = 0.0f;
                if (mode == 1) {
                    dir = 0;
                }
                frame = 0;
                ch->unk10 = 0;
            } else if (mode == 0) {
                frame = (u32) -frame % (u32) (n - 1);
                if (frame != 0) {
                    frame = (n - 1) - frame;
                }
            } else if (mode == 1) {
                frac = 0.0f;
                dir = 0;
                frame = 0;
            }
        }
    }
    ch->unk11 = dir;
    ch->unk13 = frame;
    ch->unk4 = frac;
    o->frame = frame;
    o->dir = dir;
    o->frac = frac;
    return speed;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F1BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Moves (x0, y0, z0) towards (x1, y1, z1) by t: each axis becomes
 * x0 + round((f32)(x1 - x0) * t) (cvt.w.s, round to nearest). If all three
 * results are 0 returns 0; else fills D_803B3730 with the 16.16 translation
 * matrix for them (identity, translation << 16) and returns 0x10000.
 * Register convention: x0, y0, z0, x1 in a0-a3, y1, z1 in t0, t1, t in f30;
 * result in a0 (conventions.txt). Saves s2; asm callers keep t7, f8, f12,
 * f14 live. */
s32 func_8029F3D0(s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, f32 t) {
    s32 x = port_cvt_w_s((f32) (x1 - x0) * t) + x0;
    s32 y = port_cvt_w_s((f32) (y1 - y0) * t) + y0;
    s32 z = port_cvt_w_s((f32) (z1 - z0) * t) + z0;
    s32 *m = D_803B3730;

    if (x == 0 && y == 0 && z == 0) {
        return 0;
    }
    m[0] = 0x10000;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = 0x10000;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = 0x10000;
    m[11] = 0;
    m[12] = x << 16;
    m[13] = y << 16;
    m[14] = z << 16;
    m[15] = 0x10000;
    return 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F3D0.s")
#endif

#ifdef NON_MATCHING
s32 func_8029F6B0(s32 a, s32 b, f32 t);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rotation about x by the angle func_8029F6B0(a, b, t) (angle lerp): if
 * that is 0 returns 0; else, with s / c = func_802AE160 / func_802AE104 of
 * angle >> 4 (16.16 sine / cosine), fills D_803B3730 with the 16.16 matrix
 * {1,0,0,0, 0,c,s,0, 0,-s,c,0, 0,0,0,1} and returns 0x10000.
 * Register convention: a, b in a0, a1, t in f30, result in a0
 * (conventions.txt). The asm saves t2, t4, t5 and s2, leaves fp changed
 * (the cosine) and v1 = angle >> 4 on the rotating path; its only caller
 * (func_8029EF80) reads neither. */
s32 func_8029F4B8(s32 a, s32 b, f32 t) {
    s32 *m = D_803B3730;
    s32 r = func_8029F6B0(a, b, t);
    s32 s;
    s32 c;

    if (r == 0) {
        return 0;
    }
    s = func_802AE160((u32) r >> 4);
    c = func_802AE104((u32) r >> 4);
    ROT_MTX_X(m, s, c);
    return 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F4B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_8029F4B8, rotation about y: {c,0,-s,0, 0,1,0,0, s,0,c,0, 0,0,0,1}.
 * Same register convention. */
s32 func_8029F560(s32 a, s32 b, f32 t) {
    s32 *m = D_803B3730;
    s32 r = func_8029F6B0(a, b, t);
    s32 s;
    s32 c;

    if (r == 0) {
        return 0;
    }
    s = func_802AE160((u32) r >> 4);
    c = func_802AE104((u32) r >> 4);
    ROT_MTX_Y(m, s, c);
    return 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F560.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_8029F4B8, rotation about z: {c,s,0,0, -s,c,0,0, 0,0,1,0, 0,0,0,1}.
 * Same register convention. */
s32 func_8029F608(s32 a, s32 b, f32 t) {
    s32 *m = D_803B3730;
    s32 r = func_8029F6B0(a, b, t);
    s32 s;
    s32 c;

    if (r == 0) {
        return 0;
    }
    s = func_802AE160((u32) r >> 4);
    c = func_802AE104((u32) r >> 4);
    ROT_MTX_Z(m, s, c);
    return 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F608.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_8030D870; /* full turn in 1/16 units */

/* Angle interpolation from a to b by t, the shorter way round: with
 * A = a / 16 and D = b / 16 - A wrapped into [-2048, 2048] by +-D_8030D870,
 * r = D * t + A, wrapped into [0, D_8030D870], returned as round(r * 16)
 * (cvt.w.s). All single precision.
 * Register convention: a, b in a0, a1, t in f30, result in t4
 * (conventions.txt; the asm also leaves the constants 0.0 / 2048.0 /
 * -2048.0 in f8 / f12 / f14, which its callers don't use). Asm callers
 * keep a0-a3, t1, t5, t7 live. */
s32 func_8029F6B0(s32 a, s32 b, f32 t) {
    f32 full = D_8030D870;
    f32 fa = (f32) a / 16.0f;
    f32 d = (f32) b / 16.0f;

    d = d - fa;
    if (d < -2048.0f) {
        d = d + full;
    } else if (!(d <= 2048.0f)) {
        d = d - full;
    }
    d = d * t + fa;
    if (d < 0.0f) {
        d = d + full;
    }
    if (!(d <= full)) {
        d = d - full;
    }
    return port_cvt_w_s(d * 16.0f);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F6B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Scale counterpart of func_8029F3D0: interpolates (x0, y0, z0) towards
 * (x1, y1, z1) by t the same way; if all three are 0x100 (unit scale)
 * returns 0, else fills D_803B3730 with the diagonal scale matrix
 * ((v << 16) >> 8, arithmetic, i.e. 8.8 -> 16.16) and returns 0x10000.
 * Register convention as func_8029F3D0, result in a0 (the asm also leaves
 * the scaled x in a3, unused by its caller). Saves s2; asm callers keep a2,
 * t7 live. */
s32 func_8029F760(s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, f32 t) {
    s32 x = port_cvt_w_s((f32) (x1 - x0) * t) + x0;
    s32 y = port_cvt_w_s((f32) (y1 - y0) * t) + y0;
    s32 z = port_cvt_w_s((f32) (z1 - z0) * t) + z0;
    s32 *m = D_803B3730;

    if (x == 0x100 && y == 0x100 && z == 0x100) {
        return 0;
    }
    m[0] = (x << 16) >> 8;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = (y << 16) >> 8;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = (z << 16) >> 8;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
    return 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F760.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets up an animated model from its file header hdr:
 * - the 32 channel entries ch[0..31] get id = hdr + hdr->word[0x10] + the
 *   u16 at that table's index i, and every other field zeroed;
 * - the block at hdr + hdr->word[0x18] (word 0: total bytes, word 4: bytes
 *   of data that follow at +8) is copied word by word to both bufB and bufA,
 *   and the rest of the total (a multiple of 0x40, or this never ends) is
 *   filled with identity Mtx (s16 integer parts 1 on the diagonal) in both.
 * Returns the remaining count, always 0 (the asm leaves it in t7, and leaves
 * s0 = s1 = 0, which its 22 asm callers may rely on).
 * Register convention: bufA in a0, bufB in v1, ch in t0, hdr in t1
 * (conventions.txt); asm callers keep a1-a3, f12, f14 live. */
s32 func_8029F85C(u32 *bufA, u32 *bufB, Unk8029DEA0Entry *ch, u8 *hdr) {
    u8 *base = hdr + *(s32 *) (hdr + 0x10);
    u16 *offs = (u16 *) base;
    u8 *blk;
    u32 *src;
    u32 *end;
    s32 remain;
    s32 i;

    for (i = 0; i < 32; i++) {
        ch->unk4 = 0.0f;
        ch->id = (s32) (base + offs[i]);
        ch->unkC = 0;
        ch->unkE = 0;
        ch->unk10 = 0;
        ch->unk11 = 0;
        ch->unk12 = 0;
        ch->unk13 = 0;
        ch->unk14 = 0;
        ch->unk15 = 0;
        ch++;
    }

    blk = hdr + *(s32 *) (hdr + 0x18);
    remain = *(s32 *) blk;
    src = (u32 *) (blk + 8);
    end = (u32 *) ((u8 *) src + *(s32 *) (blk + 4));
    while (src != end) {
        u32 w = *src++;

        *bufB++ = w;
        *bufA++ = w;
        remain -= 4;
    }
    while (remain != 0) {
        u16 *h;

        for (h = (u16 *) bufB, i = 0; i < 32; i++) {
            h[i] = (i == 0 || i == 5 || i == 10 || i == 15) ? 1 : 0;
        }
        for (h = (u16 *) bufA, i = 0; i < 32; i++) {
            h[i] = (i == 0 || i == 5 || i == 10 || i == 15) ? 1 : 0;
        }
        bufB += 16;
        bufA += 16;
        remain -= 0x40;
    }
    return remain;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F85C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s16 *func_8029FF2C(s16 *dst);
s16 *func_8029FFA0(s16 *dst, Unk8029DEA0Entry *ch, u8 *keys);
s16 *func_802A0118(s16 *dst, s16 *key, f32 t);
extern u8 D_803B37C0[];
extern u8 D_803B4FC0[];
/* The 0x1800-byte scratch track both functions below build (the asm forms
 * the address with a bare lui/addiu; no symbol). */
#define PORT_TRACK_BUF ((u8 *) 0x803B67C0)

/* Keyframe records of the track at hdr: hdr[0] = n, a count byte at
 * hdr + n + 1 and the records from hdr + n + 2 rounded up to 4, each
 * n * 0x14 + 8 bytes long. */
static u8 *port_track_records(u8 *hdr, s32 *count, s32 *stride) {
    s32 n = hdr[0];
    u32 p = (u32) (hdr + n + 2);

    if (p & 3) {
        p += 4 - (p & 3);
    }
    *count = hdr[n + 1];
    *stride = n * 0x14 + 8;
    return (u8 *) p;
}

/* Pose of channel ch for keyframe record rec into dst: the spline
 * (func_8029FFA0) when ch->unk15 == 1, the linear pose (func_802A0118) at
 * key rec + 8 + (s8) ch->unk13 * 0x14 and t = ch->unk4 when it is 0. (The asm
 * executes `syscall` for any other mode; no C equivalent.) */
static s16 *port_channel_pose(s16 *dst, Unk8029DEA0Entry *ch, u8 *rec) {
    if (ch->unk15 == 0) {
        return func_802A0118(dst, (s16 *) (rec + ch->unk13 * 0x14 + 8), ch->unk4);
    }
    return func_8029FFA0(dst, ch, rec);
}

/* Makes channel e play the finished track: unk10-unk13, unk15 = 0, id = dst,
 * unk4 = 0.0f, then copies the 0x1800-byte scratch track to dst (in that
 * order: with the D_803B35F8 table e itself lies inside D_803B37C0's copy). */
static void port_publish_track(Unk8029DEA0Entry *e, u8 *dst) {
    u32 *s = (u32 *) PORT_TRACK_BUF;
    u32 *d = (u32 *) dst;
    s32 i;

    e->unk10 = 0;
    e->unk11 = 0;
    e->unk12 = 0;
    e->unk15 = 0;
    e->unk13 = 0;
    e->id = (s32) dst;
    e->unk4 = 0.0f;
    for (i = 0; i < 0x1800 / 4; i++) {
        d[i] = s[i];
    }
}

/* Blends two animation channels into a two-pose track: header bytes 2, 1, 1,
 * count; then for each keyframe record of channel a's track whose key word
 * also appears in channel b's track, the key, the record's second word, a's
 * pose and b's pose (port_channel_pose) - count is the number of such
 * records. The track goes to D_803B37C0 and channel 31 of base plays it.
 * Register convention: a, b, base in v0, v1, a0 (conventions.txt); the asm
 * restores a2, a3, t0-t4, t7, s0-s7, t8 and leaves f20/f30 (and f12/f14) as the
 * pose callees do. Asm callers keep t6, t7, t8 live. */
void func_8029F9D4(s32 a, s32 b, Unk8029DEA0Entry *base) {
    Unk8029DEA0Entry *ea = base + a;
    Unk8029DEA0Entry *eb = base + b;
    u8 *out = PORT_TRACK_BUF;
    s16 *dst;
    u8 *ra;
    u8 *rb;
    u8 *r;
    s32 na;
    s32 nb;
    s32 sa;
    s32 sb;
    s32 k;
    s32 n = 0;

    out[0] = 2;
    out[1] = 1;
    out[2] = 1;
    dst = (s16 *) (out + 4);
    ra = port_track_records((u8 *) ea->id, &na, &sa);
    rb = port_track_records((u8 *) eb->id, &nb, &sb);
    for (; na != 0; na--, ra += sa) {
        u32 key = *(u32 *) ra;

        for (r = rb, k = nb; k != 0; k--, r += sb) {
            if (*(u32 *) r == key) {
                break;
            }
        }
        if (k == 0) {
            continue;
        }
        ((u32 *) dst)[0] = key;
        ((u32 *) dst)[1] = *(u32 *) (ra + 4);
        dst += 4;
        n++;
        dst = port_channel_pose(dst, ea, ra);
        dst = port_channel_pose(dst, eb, r);
    }
    out[3] = n;
    port_publish_track(base + 31, D_803B37C0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F9D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Concatenates two channels' tracks into one with every pose doubled: header
 * bytes 2, 1, 1, count; then for every keyframe record of channel a's track
 * and then of channel b's: the key, the record's second word, the channel's
 * pose (port_channel_pose) and a copy of it (func_8029FF2C); count = all
 * records. The track goes to D_803B4FC0 and channel 30 of base plays it.
 * Register convention, saved registers and callers' needs as func_8029F9D4. */
void func_8029FC74(s32 a, s32 b, Unk8029DEA0Entry *base) {
    Unk8029DEA0Entry *ea = base + a;
    Unk8029DEA0Entry *eb = base + b;
    u8 *hb = (u8 *) eb->id;
    u8 *out = PORT_TRACK_BUF;
    s16 *dst;
    u8 *r;
    s32 cnt;
    s32 stride;
    s32 n = 0;

    out[0] = 2;
    out[1] = 1;
    out[2] = 1;
    dst = (s16 *) (out + 4);
    for (r = port_track_records((u8 *) ea->id, &cnt, &stride); cnt != 0; cnt--, r += stride) {
        ((u32 *) dst)[0] = *(u32 *) r;
        ((u32 *) dst)[1] = *(u32 *) (r + 4);
        dst += 4;
        n++;
        dst = port_channel_pose(dst, ea, r);
        dst = func_8029FF2C(dst);
    }
    for (r = port_track_records(hb, &cnt, &stride); cnt != 0; cnt--, r += stride) {
        ((u32 *) dst)[0] = *(u32 *) r;
        ((u32 *) dst)[1] = *(u32 *) (r + 4);
        dst += 4;
        n++;
        dst = port_channel_pose(dst, eb, r);
        dst = func_8029FF2C(dst);
    }
    out[3] = n;
    port_publish_track(base + 30, D_803B4FC0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FC74.s")
#endif

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
#ifdef NON_MATCHING
/* Spline pose of animation channel ch: sets up the basis (func_8029F110, by
 * ch->unk15) and the four control keyframes around frame (u8) ch->unk13 of
 * the n-frame track (n = first byte of the header ch->id points at;
 * func_8029F060 at t = ch->unk4), then writes 9 halfwords to dst: the spline
 * (func_8029E878) of each s16 of the four 0x14-byte keyframes at
 * keys + 8 + D_803B7FC0[i] * 0x14, rounded as cvt.w.s. Returns dst + 10
 * halfwords (the asm's t1 = last store + 2).
 * Register convention: dst t1, ch t4, keys t5, result t1 (conventions.txt).
 * The asm saves the rest except f20 (= 1.0 from func_8029F110) and f30 (= t);
 * f12/f14 are left as callee scratch (f14 from func_8029F110, f12 from
 * func_8029E878), not reproduced. Asm callers keep a0, t0, t3, t8 live. */
void func_8029F110(u8 *p, s32 mode, Unk8029F110Out *o);
f32 func_8029F060(f32 t, s32 idx, s32 n);
extern u8 D_803B7FC0[4];

s16 *func_8029FFA0(s16 *dst, Unk8029DEA0Entry *ch, u8 *keys) {
    Unk8029F110Out basis;
    f32 t = ch->unk4;
    s16 *k0;
    s16 *k1;
    s16 *k2;
    s16 *k3;
    s32 i;

    func_8029F110((u8 *) ch, (u8) ch->unk15, &basis);
    func_8029F060(t, (u8) ch->unk13, *(u8 *) ch->id);
    keys += 8;
    k0 = (s16 *) (keys + D_803B7FC0[0] * 0x14);
    k1 = (s16 *) (keys + D_803B7FC0[1] * 0x14);
    k2 = (s16 *) (keys + D_803B7FC0[2] * 0x14);
    k3 = (s16 *) (keys + D_803B7FC0[3] * 0x14);
    for (i = 9; i != 0; i--) {
        *dst++ = port_cvt_w_s(func_8029E878(*k0++, *k1++, *k2++, *k3++, t));
    }
    return dst + 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FFA0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_8029F6B0(s32 a, s32 b, f32 t);

/* Linear pose between keyframes a (key) and b (key + 0x14), s16 fields:
 * dst[0..2] and dst[6..8] = a + cvt.w.s((b - a) * t) for fields 0..2 and
 * 6..8, dst[3..5] = the shortest-way angle lerp func_8029F6B0 of fields 3..5.
 * Returns dst + 10 halfwords.
 * Register convention: dst t1, key t5, t in f30, result t1 (conventions.txt);
 * the asm restores a0-a3, t0, t2, t4 and leaves f12/f14 = 2048.0/-2048.0
 * from func_8029F6B0 (not reproduced). Its subtractions/adds trap on
 * overflow (s16 inputs can't). Asm callers keep a0, t0, t3, t8 live. */
s16 *func_802A0118(s16 *dst, s16 *key, f32 t) {
    s32 i;

    for (i = 0; i < 3; i++) {
        dst[i] = port_cvt_w_s((f32) (key[10 + i] - key[i]) * t) + key[i];
    }
    for (i = 3; i < 6; i++) {
        dst[i] = func_8029F6B0(key[i], key[10 + i], t);
    }
    for (i = 6; i < 9; i++) {
        dst[i] = port_cvt_w_s((f32) (key[10 + i] - key[i]) * t) + key[i];
    }
    return dst + 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0118.s")
#endif

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

/* func_802A0508 .. func_802A0674: the same field setters/getter as above, but
 * on the D_803B35F8 entry whose id is the key (looked up by func_802A06B4).
 * Register convention (conventions.txt): the key arrives in v0 (put there by
 * callers several levels up and forwarded unchanged), the value in v1 and a
 * float in f0. Each restores v0 (and v1/a0 where it uses them); asm callers
 * keep a0-a3, t6/t7, f12/f14 live across the call, which a mixed N64 build
 * would need a thunk for (the native port doesn't). */
#ifdef NON_MATCHING
Unk8029DEA0Entry *func_802A06B4(s32 id);

/* unk10 = 1, unkE = val, unkC = 0 (key in v0, val in v1). */
void func_802A0508(s32 key, s32 val) {
    Unk8029DEA0Entry *e = func_802A06B4(key);

    e->unk10 = 1;
    e->unkE = val;
    e->unkC = 0;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0508.s")
#endif

/* func_802A0540: also part of the func_802A06B4 hidden-$v0-search-key
 * family documented below - saves the incoming $v0 across the call
 * (forwarding it one layer further) and returns that original value,
 * not the callee's. */
#ifdef NON_MATCHING
/* unk10 = 0 (key in v0). */
void func_802A0540(s32 key) {
    func_802A06B4(key)->unk10 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0540.s")
#endif

#ifdef NON_MATCHING
/* unk13 = 0, unk4 = 0.0f (key in v0; the asm leaves f0 = 0.0f). */
void func_802A0570(s32 key) {
    Unk8029DEA0Entry *e = func_802A06B4(key);

    e->unk13 = 0;
    e->unk4 = 0.0f;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0570.s")
#endif

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
 * register-correct wrapper, not a straight decomp.
 * (Port note: func_802A06B4 actually restores v1, so v1 here is the caller's
 * value argument, not a second result. The NON_MATCHING rewrites take the
 * key and value as parameters; see conventions.txt.) */
#ifdef NON_MATCHING
/* unk13 = val, unk4 = f (f in f0, key in v0, val in v1). */
void func_802A05A4(f32 f, s32 key, s32 val) {
    Unk8029DEA0Entry *e = func_802A06B4(key);

    e->unk13 = val;
    e->unk4 = f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05A4.s")
#endif

#ifdef NON_MATCHING
/* unk14 = val (key in v0, val in v1). */
void func_802A05D0(s32 key, s32 val) {
    func_802A06B4(key)->unk14 = val;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05D0.s")
#endif

#ifdef NON_MATCHING
/* unk11 = val (key in v0, val in v1). */
void func_802A05F8(s32 key, s32 val) {
    func_802A06B4(key)->unk11 = val;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05F8.s")
#endif

#ifdef NON_MATCHING
/* unk12 = val (key in v0, val in v1). */
void func_802A0620(s32 key, s32 val) {
    func_802A06B4(key)->unk12 = val;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0620.s")
#endif

#ifdef NON_MATCHING
/* unkC = 0, unkE = val (key in v0, val in v1). */
void func_802A0648(s32 key, s32 val) {
    Unk8029DEA0Entry *e = func_802A06B4(key);

    e->unkC = 0;
    e->unkE = val;
}
#else
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0648.s")
#endif

#ifdef NON_MATCHING
/* Reads the entry with id key (in v0) like func_802A04BC: the asm returns
 * v1 = (s8) unk10, a0 = (s8) unk11, a1 = (s8) unk12, a2 = unk14,
 * a3 = (u16) unkC, t0 = (u16) unkE, t1 = (s8) unk13, f0 = unk4 (v0 restored);
 * here out[0..6] get those integers and out[7] the float's bits. */
void func_802A0674(s32 key, s32 *out) {
    Unk8029DEA0Entry *e = func_802A06B4(key);

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
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0674.s")
#endif

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
