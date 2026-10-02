#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/func_802DC370.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/func_802DC3B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/func_802DC400.s")

void func_802D47C0(void *arg0, s32 arg1, s32 arg2);
extern u8 D_803FF2D8[];

void func_802DC444(void) {
    func_802D47C0(D_803FF2D8, 0, 0);
}

extern void *D_80307770;

s32 func_802DC470(void *arg0) {
    if (arg0 == NULL) {
        arg0 = D_80307770;
    }
    return *(s32 *) ((u8 *) arg0 + 4);
}
