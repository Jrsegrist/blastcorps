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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B0DA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDF10[]; /* this vehicle's state block */

/* Exit check for vehicle type 2 (called from func_8024B4B8 in hd.c, which
 * declares it void but returns the leftover v0). Returns 1 when none of the
 * bytes at +0x96, +0x97, +0x98 equals 1 and the byte at +0xA1 is 0, else 0.
 * Returns s32: the asm leaves the full 0/1 in v0. Same shape as
 * func_802B2EF8/func_802B45FC (6E200), func_802B5F04 (71140) and
 * func_802B76F8 (72B80), which lack the +0xA1 test. */
s32 func_802B1150(void) {
    if (D_803EDF10[0x96] == 1 || D_803EDF10[0x97] == 1 || D_803EDF10[0x98] == 1) {
        return 0;
    }
    if (D_803EDF10[0xA1] != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B1150.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B11B8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B1228.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B13D8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B14E8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B152C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B18F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2768.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0 when the speed (s16 at +0x76 of D_803EDF10, the asm's $gp) is 0, else 50
 * when the byte at +0xA1 is 1 or 2, else 120. The asm returns it in s3 (see
 * tools_port/conventions.txt). Its asm caller func_802B152C keeps a0-a3 live
 * across the call (a mixed N64 build would need a thunk). Same shape as
 * func_802D2444 (8AEE0). */
s32 func_802B28B8(void) {
    u8 t;

    if (*(s16 *) (D_803EDF10 + 0x76) == 0) {
        return 0;
    }
    t = D_803EDF10[0xA1];
    if (t == 2 || t == 1) {
        return 50;
    }
    return 120;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B28B8.s")
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
 * func_802B152C keeps a0-a3 live across the call; a mixed N64 build would
 * need a thunk preserving those, the native port does not. */
void func_802B2900(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2900.s")
#endif

/* func_802B295C: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDF10[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EDFB8[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803EDF10[0..0xA5] plus the three words
 * D_803EDFB8[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B295C(u8 *dst) {
    return func_802AC7DC(dst, D_803EDF10, D_803EDFB8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B295C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B295C: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B2988(void *)`. */
void func_802B2988(void *src) {
    func_802AC85C(src, D_803EDF10, D_803EDFB8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2988.s")
#endif
