#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BCF0.s")

/* TODO: func_8027BE4C - four sequential global stores: `D_8036DC90 = 0;
 * D_8036DC91 = 0; D_8036DC92 = 0; D_8036DC94 = -1;` (target: 11
 * instructions, with a dead `addiu sp,sp,-8`/`+8` pair appearing mid-
 * function, right before the last store - no locals, no calls, no
 * pointer derefs at all, yet still wants a frame). Plain C compiles the
 * identical statements as a true 10-instruction leaf, no frame
 * anywhere. An outwardly identical 4-statement global-store sequence in
 * the same file's neighborhood (func_80294E88 in hd_code/4EBE0.c)
 * matches fine with no frame, so this isn't simply "4+ sequential
 * stores always want one" - some property of the original source not
 * visible from the statement shape alone. Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BE4C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BE7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027C4C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D350.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D5AC.s")
