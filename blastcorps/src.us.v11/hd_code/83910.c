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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C80D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8150.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8470.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8790.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): a0 passes straight
 * through to func_802C4310 with a1 = 0x72. */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

void func_802C8AB0(s32 arg0) {
    D_8036444C = 3000;
    D_80364450 = 0;
    func_802C4310(arg0, 0x72);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8AB0.s")
#endif

/* func_802C8AF0: same dead-$ra-frame-around-`return 1;` as func_802BBE10
 * in 772A0.c - confirmed hand-written, not compiler-reachable. See that
 * file's comment. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): always returns 1
 * (vehicle-type 11/17/18 "can exit" check). 00000.c declares it void and
 * ignores the result, but the asm returns 1 in v0. */
s32 func_802C8AF0(void) {
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8AF0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803F8778; /* save copy pairs (func_802A7764), one per vehicle */
extern u64 *D_803F877C;
extern u64 *D_803F8780;
extern u64 *D_803F8784;
extern u64 *D_803F8788;
extern u64 *D_803F878C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 0x11 / 0x12 / other (11) (called from hd.c with the
 * type): func_802A7764(pair for that type, 0x800), then stops the looping
 * sounds (func_802C444C). The asm points $gp at that vehicle's block
 * (D_803F85F8 / D_803F86A0 / D_803F8550) around the calls and restores it.
 * The asm compares the whole register; 00000.c declares the parameter u8. */
void func_802C8B0C(s32 type) {
    if (type == 0x11) {
        func_802A7764(D_803F8780, D_803F8784, 0x800);
    } else if (type == 0x12) {
        func_802A7764(D_803F8788, D_803F878C, 0x800);
    } else {
        func_802A7764(D_803F8778, D_803F877C, 0x800);
    }
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8B0C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8BB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8C90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8FA8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C92C0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Bounce: the speed (s16 at +0x76 of the vehicle block, which the asm takes
 * in $gp: three vehicles share this, see tools_port/conventions.txt) is
 * raised to a magnitude of at least 80 keeping its sign (0 counts as
 * positive), then negated and halved (arithmetic shift). Asm callers
 * func_802C8C90, func_802C8FA8 and func_802C92C0 keep a0-a3, f12 and f14
 * live across the call (a mixed N64 build would need a thunk). */
void func_802C95D8(u8 *state) {
    s32 v = *(s16 *) (state + 0x76);

    if (v >= 0) {
        if (v < 0x50) {
            v = 0x50;
        }
    } else if (v >= -0x4F) {
        v = -0x50;
    }
    *(s16 *) (state + 0x76) = -v >> 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C95D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9624.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C97C0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C995C.s")

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

extern f32 D_8030D980;

/* Speed (s16 at +0x76 of the vehicle block) / D_8030D980, rounded to
 * nearest. The asm takes the block in $gp and returns the value in s3 (see
 * tools_port/conventions.txt); nothing calls it directly, so which of this
 * file's three blocks it gets is unknown, hence the parameter. */
s32 func_802C9AF8(u8 *state) {
    s32 r;

    CVT_W_S(r, *(s16 *) (state + 0x76) / D_8030D980);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9AF8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers func_802C8C90, func_802C8FA8
 * and func_802C92C0 rely on a0-a3, f12, f14 being preserved (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802C9B30(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9B30.s")
#endif
