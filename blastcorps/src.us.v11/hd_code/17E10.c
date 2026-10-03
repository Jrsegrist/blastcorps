#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C5D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025C878.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025CE74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D0B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D184.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025D2B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E1E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E2CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025E67C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025EDF0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F044.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8025F0F0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_8026005C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_802600D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260148.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260210.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260300.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_802604FC.s")

void func_80260618(void *arg0, u8 arg1) {
    if (arg0 != NULL) {
        *(s8 *)((u8 *) arg0 + 0x36) = (s8)(s16)(u8) arg1;
    }
}

u8 func_80260634(void *arg0) {
    if (arg0 != NULL) {
        return *(u8 *) ((u8 *) arg0 + 0x3f);
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260650.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_802608C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260934.s")

void func_80260934(s32 arg0);

void func_802609D0(void) {
    func_80260934(1);
}

void func_802609F0(void) {
    func_80260934(0x11);
}

void func_80260A10(void) {
    func_80260934(3);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260A30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260AB8.s")

extern u16 *D_80366C28;

u16 func_80260B24(u8 arg0) {
    return D_80366C28[arg0];
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/17E10/func_80260B40.s")
