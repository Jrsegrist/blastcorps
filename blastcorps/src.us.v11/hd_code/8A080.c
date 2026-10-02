#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE840.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE880.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE90C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE958.s")

/* func_802CE9A4: `D_803FB8B0 = &D_803F9330;` wrapped in the same dead
 * `addiu sp,sp,-8`/`sd $ra`/`ld $ra`/`addiu sp,sp,8` frame as
 * func_802BBE10/func_802C8AF0 - confirmed hand-written by the same
 * probe evidence (no calls, no locals, nothing that could need a frame;
 * IDO never generates this shape on its own). Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CEA68.s")
