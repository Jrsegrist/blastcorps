#include "common.h"
#include <ultra64.h>

extern s16 D_802154D2;
extern s32 D_802154DC;
extern u8 D_80215900[];

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8C40.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8DCC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8EB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E93DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E9528.s")

s32 func_801E96F8(void) {
    return D_802154D2 == D_802154DC + 8;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E9718.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA108.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA268.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA278.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA4B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA6E8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA93C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EAA7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC288.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC30C.s")

void func_801EC464(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        D_80215900[i] = 3;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC49C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC770.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECA50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECB18.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECC8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECE9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECF5C.s")

void func_801ED480(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 0x20; i++) {
        dst[i] = src[i];
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ED4B8.s")
