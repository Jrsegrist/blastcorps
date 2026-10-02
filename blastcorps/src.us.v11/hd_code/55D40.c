#include "common.h"
#include <ultra64.h>

extern s32 D_803A6B20;
extern s32 D_803A6B24;

void func_8029A500(void) {
    D_803A6B20 = -0xef;
    D_803A6B24 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/55D40/func_8029A518.s")
