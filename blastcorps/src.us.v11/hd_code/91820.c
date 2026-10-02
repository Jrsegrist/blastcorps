#include "common.h"
#include <ultra64.h>

/* func_802D5FE0: `jr $ra; sqrt.s $f0,$f12` - almost certainly the libm/SDK
 * sqrtf() implementation itself (a single hardware FPU instruction), not
 * a caller of it - writing `return sqrtf(arg0);` compiles to an actual
 * CALL to an undefined `sqrtf` symbol instead of inlining the hardware
 * instruction. Needs either the right intrinsic/pragma to get IDO to
 * inline sqrt.s, or recognizing this literally IS sqrtf and should be
 * named/declared as such. Same libultra version as Banjo-Kazooie
 * (n64decomp/banjo-kazooie) - worth checking how that project matched its
 * copy of this exact function. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/91820/func_802D5FE0.s")
