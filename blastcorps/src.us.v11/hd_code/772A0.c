#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBA60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBDC8.s")

/* TODO: func_802BBE10 - `return 1;` (target: 7 instructions, with a dead
 * `addiu sp,sp,-8`/`sd $ra,($sp)`/`ld $ra,($sp)`/`addiu sp,sp,8` frame that
 * saves/restores $ra despite never branching out - same phantom-stack-frame
 * character as func_802D9D18/func_802DC178's pointer-double-deref quirk,
 * but here on a function with no calls or pointer derefs at all. Plain
 * `return 1;` compiles as a true 3-instruction leaf (no frame), 0x10 bytes
 * short. Needs the right phrasing to make IDO treat this as non-leaf. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBEB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC2C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC3D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC578.s")
