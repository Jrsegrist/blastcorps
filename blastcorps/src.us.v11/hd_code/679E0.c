#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
#ifdef NON_MATCHING


extern u8 D_803F4030[];  /* 0xFC-byte object records */
extern u8 *D_803F7654;   /* end of the records in use */
extern u8 D_802C2A5C[];  /* definition in the 7D9D0 text blob */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* For every D_803F4030 record (up to D_803F7654, read once; walked with `!=`,
 * so it must be D_803F4030 + n records) whose byte 0xEA is 0 and whose
 * position (words +0x10/+0x14/+0x18) is closer than radius << 5 to the
 * player (func_802ABCDC, signed compare), apply func_802C18D4 blast damage
 * at the position << 11 with radius 10 and amount 100000. The player
 * position is reloaded for every record. Saves every s-register, gp and fp. */
void func_802AC1A0(s32 radius) {
    u8 *end = D_803F7654;
    u8 *e;
    s32 limit = radius << 5;

    for (e = D_803F4030; e != end; e += 0xFC) {
        if (e[0xEA] == 0) {
            s32 x = *(s32 *) (e + 0x10);
            s32 y = *(s32 *) (e + 0x14);
            s32 z = *(s32 *) (e + 0x18);

            if (func_802ABCDC(x, y, z, D_803EF6DC, D_803EF6E0, D_803EF6E4) < limit) {
                func_802C18D4(x << 11, y << 11, z << 11, 10, 100000);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC1A0.s")
#endif

/* func_802AC284: trivial no-arg trampoline (`func_802AC3B8();`) using an
 * 8-byte `addiu sp,sp,-8`/`sd $ra,($sp)`/.../`addiu sp,sp,8` frame.
 * Confirmed via probe compile that IDO's own codegen for a real call
 * ALWAYS uses the 24-byte o32 argument-shadow frame (`sw $ra`) instead,
 * regardless of callee prototype visibility or any other phrasing tried
 * - the 8-byte `sd $ra` style literally never comes out of this
 * compiler for a genuine call. Same hand-written-stub family as
 * func_802BBE10/func_802C8AF0/func_802CE9A4 above. func_80260EC0 in
 * hd_code/1C460.c is an outwardly identical trampoline that matched
 * fine with the normal 24-byte form - that one really is compiler
 * output; this one isn't. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802AC3B8(s32 *px, s32 *py, s32 *pz);

/* Trampoline to func_802AC3B8 (level 0x17 position wrap) with the same
 * registers: px, py, pz in v0, v1, a0 (conventions.txt). The t6/t7 the
 * survey lists as read by asm caller func_802B6294 are dead there. */
void func_802AC284(s32 *px, s32 *py, s32 *pz) {
    func_802AC3B8(px, py, pz);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC284.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80364AA8;  /* game mode */
extern s32 D_80367738;
extern u16 D_8036E4C8;
extern u8 *D_803F3960;  /* end of the (object, part) hit list */

/* Only in game mode 0x40: vehicle-state reset (func_8029A800 with b2 = 1,
 * b3 = 0, h1 = 0, h2 = 0x40, b4 = 0), the collision pass of vehicle id
 * (func_802BE77C), then if anything was hit: with D_8036E4C8 zero sets
 * D_803BE738; else plays func_80260650(D_80367738, 0x3D, 0) and moves every
 * hit object (the list D_803F3910 .. D_803F3960, read once, walked by 8 with
 * `!=`) to x = z = 0xBB80 (func_802BD99C by the difference; words 0x38 = 1,
 * 0x40 = -1).
 * Register convention: z, a1 in a0, a1, x, y, b0 in v0, v1, t0, vehicle id in
 * t8 and the vehicle record in gp, as func_8029A800 takes them. The asm leaves
 * s0-s7, fp (func_802BD99C) and f20-f28 (func_802BE77C) changed; the v1, f12
 * and f14 the survey lists as read by asm caller func_802B6294 are dead there
 * (saved and restored around a call). */
void func_802AC2A4(s32 z, s32 a1, s32 x, s32 y, s32 b0, s32 id, u8 *vehicle) {
    u8 *p;
    u8 *end;

    if (D_80364AA8 != 0x40) {
        return;
    }
    func_8029A800(z, a1, 1, 0, x, y, b0, 0, 0x40, 0, id, vehicle);
    func_802BE77C(id, vehicle);
    if (D_803F3910 == D_803F3960) {
        return;
    }
    if (D_8036E4C8 == 0) {
        D_803BE738 = 1;
        return;
    }
    func_80260650(D_80367738, 0x3D, 0);
    end = D_803F3960;
    for (p = D_803F3910; p != end; p += 8) {
        u8 *obj = *(u8 **) p;
        s32 ox;
        s32 oz;

        *(s32 *) (obj + 0x38) = 1;
        ox = *(s32 *) (obj + 0x10);
        *(s32 *) (obj + 0x40) = -1;
        *(s32 *) (obj + 0x10) = 0xBB80;
        oz = *(s32 *) (obj + 0x18);
        *(s32 *) (obj + 0x18) = 0xBB80;
        func_802BD99C(obj, 0xBB80 - ox, 0, 0xBB80 - oz);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC2A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_802E8BDC; /* current level */

/* Level 0x17 only: wraps the position x (*px) and z (*pz) into the level's
 * box (x >= 0x15181 -> 0x2BC0, then x < 0x2581 -> 0x14B40; z >= 0x13881 ->
 * 0x44C0, then z < 0x3E81 -> 0x13240), setting D_80364412 on each wrap.
 * When D_80364412 is set (now or before), spawns effect kind 0x13 with
 * data 1000000 at (*px, *py, *pz) (func_802AC6FC). The second test of each
 * axis uses the value after the first wrap, but a wrapped value isn't
 * written again.
 * Register convention: px in v0, py in v1, pz in a0 (conventions.txt). The
 * asm saves gp and hands back the caller's t2 in v0; its only caller is the
 * asm trampoline func_802AC284, whose callers don't read v0. */
void func_802AC3B8(s32 *px, s32 *py, s32 *pz) {
    s32 v;

    if (D_802E8BDC != 0x17) {
        return;
    }
    v = *px;
    if (v >= 0x15181) {
        *px = v = 0x2BC0;
        D_80364412 = 1;
    }
    if (v < 0x2581) {
        *px = 0x14B40;
        D_80364412 = 1;
    }
    v = *pz;
    if (v >= 0x13881) {
        *pz = v = 0x44C0;
        D_80364412 = 1;
    }
    if (v < 0x3E81) {
        *pz = 0x13240;
        D_80364412 = 1;
    }
    if (D_80364412 != 0) {
        func_802AC6FC(*px, *py, *pz, 0x13, 1000000);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC3B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_802AA460's floating-point side results (62740.c's TriSideOut). */
typedef struct {
    f32 pz;
    f32 cross;
    f32 cz;
    f32 side;
    f32 sideZ; /* also an input (f24) */
    f32 dz;
} TriSideOut679E0;


/* 1 if the point (px, pz) is inside the triangle (x0, z0), (x1, z1),
 * (x2, z2) by func_802AA460's edge-side test, else 0: the o32 entry point
 * for C callers. The asm moves the arguments into func_802AA460's registers
 * (t0, t1, s1, s3, s4, s6, s7, t9), saves s0-s7 and fp, and returns its v0.
 * It doesn't save f20-f28, which func_802AA460 changes (and passes the
 * caller's f24 through as an input that only feeds those FP results); its C
 * callers only use v0, so the FP side results are dropped here. */
s32 func_802AC4C4(s32 px, s32 pz, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2) {
    TriSideOut679E0 out;

    out.sideZ = 0.0f;
    return func_802AA460(px, pz, x0, z0, x1, z1, x2, z2, &out);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC4C4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawns the D_802C2A5C effect (func_802A6274, type 0, data 0x28488) at the
 * whole-unit position (x, y, z) << 16; every other field 0. Saves every
 * s-register and fp; the asm leaves the caller's v0 (C callers: void). */
void func_802AC544(s32 x, s32 y, s32 z) {
    Io802A6274 io;

    io.a3 = 0;
    io.t6 = 0;
    io.s1 = 0;
    func_802A6274(&io, D_802C2A5C, 0x28488, 0, x << 16, y << 16, z << 16, 0, 0, 0, 0, 0, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC544.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawns effect definition D_802C3FFC[kind] (func_802A6274, type 0) with
 * data `data` at (x, y, z) << 11, tag byte 0; every other field 0. kind is
 * used as a full word (4DA80.c declares it u8). */
void func_802AC61C(s32 x, s32 y, s32 z, s32 kind, s32 data) {
    Io802A6274 io;

    io.a3 = 0;
    io.t6 = 0;
    io.s1 = 0;
    func_802A6274(&io, D_802C3FFC[kind], data, 0, x << 11, y << 11, z << 11, 0, 0, 0, 0, 0, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC61C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802AC61C, but with tag byte 1 (func_802A6274's a3 input). The asm
 * leaves t6 = func_802A6274's t6 output and t7 = 0; the survey lists its asm
 * callers (func_802AC3B8, func_802A8CCC) as reading t6/t7/f12/f14, but they
 * don't read them after the call (only through their own callers'
 * conventions), so they are not modelled. */
void func_802AC6FC(s32 x, s32 y, s32 z, s32 kind, s32 data) {
    Io802A6274 io;

    io.a3 = 1;
    io.t6 = 0;
    io.s1 = 0;
    func_802A6274(&io, D_802C3FFC[kind], data, 0, x << 11, y << 11, z << 11, 0, 0, 0, 0, 0, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC6FC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Serialize a 0xA6-byte block plus three words into `dst` (0xB2 bytes, dst
 * need not be aligned; `words` must be word-aligned and not overlap dst).
 * Returns the number of bytes written (always 0xB2; the asm leaves it in v0,
 * though its callers ignore it). Inverse of func_802AC85C. */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words) {
    u8 *start = dst;
    s32 i;

    for (i = 0; i < 0xA6; i++) {
        *dst++ = *src++;
    }
    for (i = 0; i < 3; i++) {
        dst[0] = words[i] >> 24;
        dst[1] = words[i] >> 16;
        dst[2] = words[i] >> 8;
        dst[3] = words[i];
        dst += 4;
    }
    return dst - start;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC7DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Deserialize what func_802AC7DC wrote: copy 0xA6 bytes from `src` to `dst`,
 * then the three (unaligned) big-endian words after them into `words`. */
void func_802AC85C(u8 *src, u8 *dst, u32 *words) {
    s32 i;

    for (i = 0; i < 0xA6; i++) {
        *dst++ = *src++;
    }
    for (i = 0; i < 3; i++) {
        words[i] = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
        src += 4;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC85C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Converts, in place, a 4x4 matrix of s32 16.16 values into the N64 Mtx
 * layout: the 16 integer halves first, then the 16 fraction halves.
 * The asm takes m in s2 (conventions.txt) and saves every register it uses.
 * Asm callers keep many registers live across the call (func_8029E5AC a0-a3,
 * t2, t4, t5, t7, f8, f12, f14; func_802AA764 a0-a3, t4, t6, t7, f12, f14;
 * func_802C1F30 t0-t2, t4, t6; func_802CEEFC a0-a3, t3, t7, f12, f14; ...):
 * a mixed N64 build would need a thunk, the native port doesn't. */
void func_802AC8CC(u16 *m) {
    u16 tmp[32];
    s32 i;

    for (i = 0; i < 32; i++) {
        tmp[i] = m[i];
    }
    for (i = 0; i < 16; i++) {
        m[i] = tmp[i * 2];
        m[16 + i] = tmp[i * 2 + 1];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC8CC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Writes a 16.16 translation matrix (identity plus x, y, z in the last row)
 * to m. The asm takes x, y, z, m in t0-t3 and saves a0; asm callers keep
 * many registers live (func_802AA764 t4, t6-t8, f12, f14; func_802BE574
 * a0-a3, t6, f12, f14; func_802C1F30 t0-t4, t6; ...): a mixed N64 build
 * would need a thunk, the native port doesn't. */
void func_802ACA60(s32 x, s32 y, s32 z, s32 *m) {
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
    m[12] = x;
    m[13] = y;
    m[14] = z;
    m[15] = 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACA60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* func_802ACAC4 / func_802ACB50 / func_802ACBDC write a 16.16 rotation matrix
 * (about y, z and x) for `angle` (0x1000 = 360 degrees) to m. Each calls
 * func_802AE160 (sine) first, then func_802AE104 (cosine).
 * Register convention: angle in t0, m in t3 (conventions.txt); the asm saves
 * v1, a0, t5 and fp. Asm callers keep t6/t7 (func_802A68D4) or t8
 * (func_802AA764, func_802BE574) live across the call; the survey lists
 * f12/f14 (func_802AE104's leftovers) as read by func_802A68D4 and
 * func_802AA764, which a C version can't hand back (a mixed N64 build would
 * need a thunk; the native port won't). */
void func_802ACAC4(s32 angle, s32 *m) {
    s32 s = func_802AE160(angle);
    s32 c = func_802AE104(angle);

    m[8] = s;
    m[0] = c;
    m[1] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = 0x10000;
    m[6] = 0;
    m[7] = 0;
    m[9] = 0;
    m[10] = c;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
    m[2] = -s;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACAC4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rotation about z (see func_802ACAC4). */
void func_802ACB50(s32 angle, s32 *m) {
    s32 s = func_802AE160(angle);
    s32 c = func_802AE104(angle);

    m[1] = s;
    m[0] = c;
    m[2] = 0;
    m[3] = 0;
    m[4] = -s;
    m[5] = c;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = 0x10000;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACB50.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rotation about x (see func_802ACAC4). */
void func_802ACBDC(s32 angle, s32 *m) {
    s32 s = func_802AE160(angle);
    s32 c = func_802AE104(angle);

    m[6] = s;
    m[0] = 0x10000;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = c;
    m[7] = 0;
    m[8] = 0;
    m[9] = -s;
    m[10] = c;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACBDC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Writes a 16.16 scale matrix diag(x, y, z, 1) to m. Same register
 * interface as func_802ACA60 (t0-t3); asm callers func_802A68D4 / func_802AA764
 * keep a0-a3, t4, f12, f14 live (a mixed N64 build would need a thunk). */
void func_802ACC68(s32 x, s32 y, s32 z, s32 *m) {
    m[0] = x;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    m[4] = 0;
    m[5] = y;
    m[6] = 0;
    m[7] = 0;
    m[8] = 0;
    m[9] = 0;
    m[10] = z;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 0x10000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACC68.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803ED420[16]; /* scratch product */

/* m = m * b for 4x4 matrices of s32 16.16 values: each element is the s64
 * sum of four s32 x s32 products (dmult, wrapping), >> 16 arithmetic, low
 * 32 bits kept. The product goes through D_803ED420 and is then copied to m.
 * The asm takes b in a0 and m in s2 (conventions.txt); it leaves a1 = 4,
 * a2 = 4, a3 = 0 (the survey shows asm callers reading a1-a3 afterwards; a
 * C caller can't). Asm callers keep a0, t2, t4-t8, f8, f12, f14 live
 * across the call (a mixed N64 build would need a thunk). */
void func_802ACCCC(s32 *b, s32 *m) {
    s32 i;
    s32 j;
    s32 k;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            s64 sum = 0;

            for (k = 0; k < 4; k++) {
                sum += (s64) m[i * 4 + k] * b[k * 4 + j];
            }
            D_803ED420[i * 4 + j] = (s32) (sum >> 16);
        }
    }
    for (i = 0; i < 16; i++) {
        m[i] = D_803ED420[i];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACCCC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* (s64) a * b >> 16 (dmult, dsra), kept to 32 bits as the following 32-bit
 * add/sub use it. */
#define MUL16(a, b) ((s32) (((s64) (a) * (s64) (b)) >> 16))

/* 2-D rotation by `angle` (cosine c first, then sine s; 16.16):
 * out[0] = x*c - z*s, out[1] = z*c + x*s (each product >> 16 on its own).
 * Register convention: x in a1, z in a2, angle in a3; results in t0 and t1
 * (here out[0], out[1]); the asm saves v0/v1, leaves a3 = its a0 and
 * a0 = x*s >> 16 and changes fp (conventions.txt). Nothing calls it. The
 * trapping `sub`/`add` limit inputs to game range. */
void func_802ACDB8(s32 x, s32 z, s32 angle, s32 *out) {
    s32 c = func_802AE104(angle);
    s32 s = func_802AE160(angle);

    out[0] = MUL16(x, c) - MUL16(z, s);
    out[1] = MUL16(z, c) + MUL16(x, s);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACDB8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 2-D rotation the other way: returns z*s + x*c (the asm's a3) and stores
 * z*c - x*s (its t1) in *t1out (cosine c first, then sine s; each product
 * 16.16 >> 16).
 * Register convention: x in a0, z in a2, angle in a3; results in a3 and t1
 * (conventions.txt). The asm saves v0/v1, leaves t0 = its a1 and a1 = x*s
 * >> 16, and changes fp. Asm caller func_802B8D04 keeps a0, a2, t3, t4 and
 * t7 live; func_802A94A4 is listed as reading f12/f14 afterwards
 * (func_802AE160's leftovers), which a C version can't hand back (a mixed
 * N64 build would need a thunk; the native port won't). */
s32 func_802ACE38(s32 x, s32 z, s32 angle, s32 *t1out) {
    s32 c = func_802AE104(angle);
    s32 s = func_802AE160(angle);

    *t1out = MUL16(z, c) - MUL16(x, s);
    return MUL16(z, s) + MUL16(x, c);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACE38.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Meant as func_802ACDB8's rotation with x in a0 and z = x, but the asm's
 * schedule multiplies the not yet shifted product: with p = x*c (64-bit),
 * out[0] (t0) = ((p >> 16) * c >> 16) + (x*s >> 16) and out[1] (a3) =
 * (p >> 16) - (p * s >> 16), all 64-bit with the add/sub on the low words
 * (cosine c first, then sine s).
 * Register convention: x in a0, angle in a3; results in t0 and a3 (here
 * out[0], out[1]); the asm also leaves t1 = its a2 and a1 = x*s >> 16, saves
 * v0/v1 and changes fp (conventions.txt). Nothing calls it. p * s can leave
 * the 32-bit range, which makes the trapping `sub` unpredictable on the
 * hardware; keep |x| < 0x8000. */
void func_802ACEB8(s32 x, s32 angle, s32 *out) {
    s32 c = func_802AE104(angle);
    s32 s = func_802AE160(angle);
    s64 p = (s64) x * c;
    s32 hi = (s32) (p >> 16);

    out[1] = hi - (s32) ((s64) ((u64) p * (u64) (s64) s) >> 16);
    out[0] = MUL16(hi, c) + MUL16(x, s);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACEB8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Wrapper: returns func_802ACF64(x) (34430.c uses it to turn a 16.16 ratio
 * into an angle). */
s32 func_802ACF3C(s32 x) {
    return func_802ACF64(x);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF3C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_802ACFD0[0x401]; /* table right after this function in the text */

/* Linear interpolation in D_802ACFD0: entry x >> 8 (clamped to 0x3FF) plus
 * (next - entry) * (x & 0xFF) / 256. The product is shifted logically, so a
 * decreasing step yields a large positive term, as in the asm.
 * Register convention: the asm takes x in v1 and returns the result in fp,
 * keeping a0, a1 and v1 (tools_port/conventions.txt); this C is plain o32.
 * Asm callers func_802A8768 (a1, a3, t7-t9, f12, f14) and func_802A8B10
 * (a1, a3, t2, t9) rely on registers surviving the call (a mixed N64 build
 * would need a thunk; the native port doesn't). */
s32 func_802ACF64(u32 x) {
    u32 i = x >> 8;
    s32 lo;
    s32 step;

    if (i > 0x3FF) {
        i = 0x3FF;
    }
    lo = D_802ACFD0[i];
    step = D_802ACFD0[i + 1] - lo;
    return lo + ((u32) (step * (s32) (x & 0xFF)) >> 8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF64.s")
#endif
