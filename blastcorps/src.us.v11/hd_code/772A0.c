#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBA60.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBEB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC2C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC3D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC578.s")
