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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBA60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBDC8.s")

/* func_802BBE10: `return 1;` wrapped in a dead `addiu sp,sp,-8`/`sd
 * $ra,($sp)`/`ld $ra,($sp)`/`addiu sp,sp,8` frame that saves/restores
 * nothing. Confirmed via probe compiles that this is NOT reachable from
 * any plausible C: IDO's own codegen for a real function call (verified
 * directly - a genuine `void f(void) { g(); }`) always uses the 24-byte
 * o32 argument-shadow frame (`sw $ra`), never this 8-byte `sd $ra` style;
 * a leaf with no calls at all (like this one) gets no frame whatsoever
 * unless a local variable is declared, and a declared local only
 * reserves the frame, it doesn't explain a `return 1;` needing one in
 * the first place. Same hand-written-leaf-stub character as init's
 * __osGetSR and this segment's COP0 stubs. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): always returns 1
 * (vehicle-type 7 "can exit" check; same as func_802C8AF0). 00000.c
 * declares it void and ignores the result, but the asm returns 1 in v0. */
s32 func_802BBE10(void) {
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE2C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE74.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBEB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC2C8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC3D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm caller func_802BBEB8 relies on
 * a0-a3 being preserved (mixed N64 build would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802BC578(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC578.s")
#endif
