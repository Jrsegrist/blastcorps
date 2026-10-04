#include "common.h"
#include <ultra64.h>

/* This file's .bss starts at 0x8036C790 */
extern s32 D_8036C790;
extern s32 D_8036C794;
extern s32 D_8036C7A0[10];
extern u8 D_8036C7CC;

void func_80276130(s32 arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1,
                   u8 r2, u8 g2, u8 b2, u8 a2, u8 r3, u8 g3, u8 b3, u8 a3);

void func_80275430(void) {
    s32 i;

    for (i = 0; i < 10; i++) {
        D_8036C7A0[i] = 0;
    }
    D_8036C794 = 0;
    D_8036C7CC = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80275478.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80275DA4.s")

void func_80276080(s32 arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u8 r, u8 g, u8 b, u8 a) {
    func_80276130(arg0, arg1, arg2, arg3, arg4, arg5, arg6, r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80276130.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_8027656C.s")

void func_8027684C(void) {
    s32 i;

    i = 0;
    while (i < 10) {
        if (!D_8036C7A0[i]) {
            D_8036C7A0[i] = D_8036C794;
            return;
        }
        i++;
    }
}

s32 func_802768A8(void) {
    s32 i;

    i = 0;
    while (i < 10) {
        if (D_8036C7A0[i] && D_8036C7A0[i] == D_8036C790) {
            return 1;
        }
        i++;
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_8027690C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80276D1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80276E50.s")
