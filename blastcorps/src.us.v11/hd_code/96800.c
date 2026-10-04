#include "common.h"
#include <ultra64.h>

void __osSpSetStatus(s32 arg0);

void osSpTaskYield(void) {
    __osSpSetStatus(0x400);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/96800/guMtxXFML.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/96800/guMtxCatL.s")
