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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFC60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFFD4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B01DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0254.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B02A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B03B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B03F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B07DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0AAC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0B3C.s")

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

extern u8 D_803EDB40[]; /* this vehicle's state block (the asm's $gp) */
extern f32 D_8030D8A8;

/* Speed (s16 at +0x76) scaled for the engine sound: rounded speed / 6.0 when
 * any of the bytes at +0x96/+0x97/+0x98 is 1, else speed / D_8030D8A8.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B03F4 keeps a0-a3 live across the call (a mixed N64 build would
 * need a thunk). Same shape as func_802B3F78 (6E200), func_802B7168 (71140),
 * func_802B83B0 (72B80), func_802CB564 (853D0), func_802CC844 (86F60),
 * func_802CD938 (88160), func_802D0710 (8AEE0); only the divisors differ. */
s32 func_802B0C74(void) {
    f32 div;
    s32 r;

    if (D_803EDB40[0x96] == 1 || D_803EDB40[0x97] == 1 || D_803EDB40[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D8A8;
    }
    CVT_W_S(r, *(s16 *) (D_803EDB40 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0C74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf, identical to func_802AFBA0 (69BB0):
 * D_803EBBF4 = D_803EBBF0 * 4, D_803ED3F6/7 = 40, 3.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802B03F4 keeps a0-a3 live across the call; a mixed N64 build would
 * need a thunk preserving those, the native port does not. */
void func_802B0CE8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0CE8.s")
#endif

/* func_802B0D44: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDB40[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EDBE8[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803EDB40[0..0xA5] plus the three words
 * D_803EDBE8[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B0D44(u8 *dst) {
    return func_802AC7DC(dst, D_803EDB40, D_803EDBE8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0D44.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B0D44: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B0D70(void *)`. */
void func_802B0D70(void *src) {
    func_802AC85C(src, D_803EDB40, D_803EDBE8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0D70.s")
#endif
