#include "common.h"
#include <ultra64.h>

extern s32 D_8021AB80;
extern u8 D_8021AB84;

void func_80200714(s32);

void func_802006F0(void) {
    func_80200714(D_8021AB84);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/196F0/func_80200714.s")

void func_80200BD4(s32 arg0) {
    D_8021AB80 = arg0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/196F0/func_80200BE0.s")
