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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE370.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE860.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE888.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803643E0; /* player x (1/32 units) */
extern s32 D_803643E8; /* player z */

/* Zone test for mode *mode (asm: a3 in and out, result in a1; see
 * tools_port/conventions.txt). Mode 1: 1 when z/32 is in [0xCCD, 0xDB5).
 * Mode 2: 1 when x/32 is in [0x834, 0x961) and z/32 in [0x4B0, 0x5DD).
 * Otherwise 0. The asm leaves the last value it compared (z/32, or x/32 when
 * the x test fails) in a3, so *mode gets that; other modes leave it alone.
 * Register note: asm caller func_802AE888 keeps a0, a2, t0-t3, f12 and f14
 * live across the call (a mixed N64 build would need a thunk). */
s32 func_802AEB9C(s32 *mode) {
    s32 v;

    if (*mode == 1) {
        *mode = v = D_803643E8 >> 5;
        return v >= 0xCCD && v < 0xDB5;
    }
    if (*mode == 2) {
        *mode = v = D_803643E0 >> 5;
        if (v < 0x834 || v >= 0x961) {
            return 0;
        }
        *mode = v = D_803643E8 >> 5;
        return v >= 0x4B0 && v < 0x5DD;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEB9C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEC3C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEE84.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEEC8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF340.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF4BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFA64.s")

/* func_802AFB84: sets $s3 = 0x8c directly (`addiu $s3, $zero, 0x8c`)
 * inside the same dead 8-byte `sd $ra` frame as func_802BBE10/
 * func_802C8AF0/func_802CE9A4/func_802AC284 - same confirmed hand-
 * written family, and doubly so here since C has no way to pin a value
 * to a specific callee-saved register by name regardless. Value is
 * never read or returned; likely a vestigial/debug leftover in the
 * original assembly. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 0x8C (asm: in s3, which asm caller func_802AEEC8 reads; see
 * tools_port/conventions.txt). That caller keeps a0-a3, f12 and f14 live
 * across the call (a mixed N64 build would need a thunk). */
s32 func_802AFB84(void) {
    return 0x8C;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFB84.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf: D_803EBBF4 = D_803EBBF0 * 4, then the byte pair
 * D_803ED3F6/7 = 40, 3. Same shape as func_802B0CE8 (6B4A0), func_802B2900
 * (6C5E0) and, with other constants, func_802B8424 (72B80), func_802B5814
 * (6E200), func_802B7240 (71140).
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802AEEC8 keeps a0-a3, f12 and f14 live across the call; a mixed N64
 * build would need a thunk preserving those, the native port does not. */
void func_802AFBA0(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBA0.s")
#endif

/* func_802AFBFC: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803ED808[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803ED760[0..0xA5] plus the three words
 * D_803ED808[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802AFBFC(u8 *dst) {
    return func_802AC7DC(dst, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802AFBFC: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802AFC28(void *)`. */
void func_802AFC28(void *src) {
    func_802AC85C(src, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFC28.s")
#endif
