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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CB720.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBA94.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 13
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1,
 * else 1 (shape shared with func_802CA140 in 853D0.c). 00000.c declares it
 * void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F8E80[];

s32 func_802CBB60(void) {
    if (D_803F8E80[0x96] == 1 || D_803F8E80[0x97] == 1 || D_803F8E80[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBB60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBBBC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBC08.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBD18.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBD5C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBDE8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBEF0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC400.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F8F28[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803F8E80): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803F8F28[0], D_803F8F28[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CC400 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802CC56C(void) {
    if (D_803F8E80[0x99] != 0 && D_803F8E80[0x98] != 1 && D_803F8E80[0x50] < 3 && D_803F8E80[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F8E80 + 0x1C), 250, -400, -400, -400, D_803F8F28[0], D_803F8F28[2],
                      *(u16 *) (D_803F8E80 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC56C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC70C.s")

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

extern f32 D_8030D9B4;

/* Speed (s16 at +0x76 of D_803F8E80, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9B4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CBEF0 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CC844(void) {
    f32 div;
    s32 r;

    if (D_803F8E80[0x96] == 1 || D_803F8E80[0x97] == 1 || D_803F8E80[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D9B4;
    }
    CVT_W_S(r, *(s16 *) (D_803F8E80 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC844.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CBDE8 on t3, t4, f12, f14; func_802CBEF0 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CC8B8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 150;
    D_803ED3F7 = 6;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC8B8.s")
#endif
