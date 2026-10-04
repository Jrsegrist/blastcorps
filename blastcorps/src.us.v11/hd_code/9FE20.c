#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/_doModFunc.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/_filterBuffer.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/_saveBuffer.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/_loadBuffer.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/_loadOutputBuffer.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/func_802E4C78.s")

s32 alFxParam(s32 *arg0, s32 arg1, s32 arg2) {
    if (arg1 == 1) {
        *arg0 = arg2;
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9FE20/alFxPull.s")
