#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E2EC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E31C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E37C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E38A0.s")

void func_802E3F90(void *arg0, void *arg1) {
    *(s32 *) arg1 = *(s32 *) ((u8 *) arg0 + 8);
    *(s16 *) ((u8 *) arg1 + 0xc) = *(s16 *) ((u8 *) arg0 + 0x1a);
    *(s32 *) ((u8 *) arg1 + 4) = *(s32 *) ((u8 *) arg0 + 0xc);
}

void func_802E3FAC(void *arg0, void *arg1) {
    *(s32 *) ((u8 *) arg0 + 8) = *(s32 *) arg1;
    *(s16 *) ((u8 *) arg0 + 0x1a) = *(s16 *) ((u8 *) arg1 + 0xc);
    *(s32 *) ((u8 *) arg0 + 0xc) = *(s32 *) ((u8 *) arg1 + 4);
}

s32 func_802E3FC8(void *arg0) {
    return *(s32 *) ((u8 *) arg0 + 0xc);
}

void func_802E3FD0(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E3FD8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4024.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E41A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E42C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E43AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4400.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4458.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E44A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E44D8.s")

void func_802E45B0(s32 arg0, void *arg1, s16 arg2) {
    *(s16 *) ((u8 *) arg1 + 0x16) = arg2;
}

void func_802E45C0(void *arg0, s32 arg1, s32 arg2, s32 arg3) {
    *(s32 *) arg0 = 0;
    *(s32 *) ((u8 *) arg0 + 4) = arg1;
    *(s32 *) ((u8 *) arg0 + 8) = arg2;
    *(s16 *) ((u8 *) arg0 + 0xc) = 0;
    *(s16 *) ((u8 *) arg0 + 0xe) = 0;
    *(s32 *) ((u8 *) arg0 + 0x10) = arg3;
}
