#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE840.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE880.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE90C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE958.s")

/* TODO: func_802CE9A4 - `D_803FB8B0 = &D_803F9330;` (target: 9
 * instructions, wrapped in a dead `addiu sp,sp,-8`/`sd $ra,($sp)`/`ld
 * $ra,($sp)`/`addiu sp,sp,8` frame that saves/restores nothing - plain C
 * compiles this as a true 5-instruction leaf, no frame at all. Same
 * phantom-dead-frame quirk family as func_802BBE10/func_802C8AF0 and
 * func_802D6EE0 above, this time on a single pointer-address store with
 * no calls, pointer derefs, or named locals of any kind. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CEA68.s")
