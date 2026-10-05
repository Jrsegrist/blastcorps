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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC1A0.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC284.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC2A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC3B8.s")

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

s32 func_802AA460(s32 x, s32 z, s32 x0, s32 z0, s32 x1, s32 z1, s32 x2, s32 z2, TriSideOut679E0 *out);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC544.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC61C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC6FC.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACAC4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACB50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACBDC.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACDB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACE38.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACEB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802ACF64(u32 x);

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
