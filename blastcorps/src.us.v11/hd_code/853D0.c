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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9B90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9F54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 10
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1 or
 * D_803F8B7C (= its byte 0xDC) is nonzero, else 1. Same shape as
 * func_802CBB60, func_802CCCD8, func_802CFA58, func_802D0B90. 00000.c
 * declares it void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F8AA0[];
extern u8 D_803F8B7C;

s32 func_802CA140(void) {
    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1 || D_803F8B7C != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA140.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1F8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef ZONE_SCAN_REGS_DEFINED
#define ZONE_SCAN_REGS_DEFINED
/* func_802ABD54's scan registers (62740.c), in and out. */
typedef struct {
    s32 t6; /* zone x */
    s32 t7; /* zone y */
    s32 s0; /* zone z */
    s32 s1; /* distance / level term */
    s32 s2; /* zone radius */
    s32 s3; /* scan counter / zone byte */
    s32 s4; /* scan pointer / zone byte */
} ZoneScanRegs;
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
#endif
extern u32 D_803F8B48[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xA at its position
 * D_803F8B48..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CA4E0 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CA308(ZoneScanRegs *r) {
    return func_802ABD54(0xA, D_803F8B48[0], D_803F8B48[1], D_803F8B48[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA308.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA34C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA3D8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA4E0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CAAFC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F8B48[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Trail (shape of func_802B3C68 in 6E200; $gp = D_803F8AA0): if the byte at
 * +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2], yaw,
 * 5, 40, 0, 1): a single centred track.
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CAAFC keeps a1-a3 live (a mixed N64 build would need a
 * thunk). */
void func_802CB224(void) {
    if (D_803F8AA0[0x99] != 0 && D_803F8AA0[0x98] != 1 && D_803F8AA0[0x50] < 3 && D_803F8AA0[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F8AA0 + 0x1C), 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2],
                      *(u16 *) (D_803F8AA0 + 0x4E), 5, 40, 0, 1);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB224.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0.5 +- |speed| / 280 * 0.5 (plus when side != 0, minus otherwise), speed
 * being the s16 at +0x76 of D_803F8AA0 (the asm's $gp). The asm takes side
 * in s2, returns the value in f2 and leaves s3 = 280 (see
 * tools_port/conventions.txt). Its asm caller func_802CAAFC keeps a1-a3
 * live (a mixed N64 build would need a thunk). Same shape as func_802B71DC
 * (71140). */
f32 func_802CB3C8(s32 side) {
    s32 speed = *(s16 *) (D_803F8AA0 + 0x76);
    f32 v;

    if (speed < 0) {
        speed = -speed;
    }
    v = (f32) speed / 280.0f * 0.5f;
    if (side != 0) {
        return v + 0.5f;
    }
    return 0.5f - v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB3C8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB42C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* cvt.w.s under the FCSR's default rounding (nearest, ties to even); a C cast
 * truncates instead. */
#define CVT_W_S(out, x)                                                 \
    do {                                                                \
        f32 _x = (x);                                                   \
        s32 _r = (s32) _x;                                              \
        f32 _f = _x - (f32) _r;                                         \
                                                                        \
        if (_f > 0.5f || (_f == 0.5f && (_r & 1))) {                    \
            _r++;                                                       \
        } else if (_f < -0.5f || (_f == -0.5f && (_r & 1))) {           \
            _r--;                                                       \
        }                                                               \
        (out) = _r;                                                     \
    } while (0)

extern f32 D_8030D9A4;

/* Speed (s16 at +0x76 of D_803F8AA0, the asm's $gp) / 6.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9A4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CA4E0 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CB564(void) {
    f32 div;
    s32 r;

    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D9A4;
    }
    CVT_W_S(r, *(s16 *) (D_803F8AA0 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB564.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CA3D8 on t3, t4, f12, f14; func_802CA4E0 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CB5D8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB5D8.s")
#endif

/* func_802CB634: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8AA0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803F8B48[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803F8AA0[0..0xA5] plus the three words
 * D_803F8B48[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802CB634(u8 *dst) {
    return func_802AC7DC(dst, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB634.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802CB634: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802CB660(void *)`. */
void func_802CB660(void *src) {
    func_802AC85C(src, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB660.s")
#endif
