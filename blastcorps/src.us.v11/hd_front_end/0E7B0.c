#include "common.h"
#include <ultra64.h>

typedef struct {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u8 pad10[6];
    /* 0x16 */ u16 unk16;
} Struct801F7FF4;

extern OSPfs D_8039B630;      /* hd_code .bss: the Controller Pak file system */
extern u8 D_8020C000[];       /* game name */
extern u8 D_8020C014[];       /* extension name */
extern OSPfsState D_80218B20[];
extern s32 D_80218D28;

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F57B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F58E8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F5FE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F60C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6160.s")

void func_801F61C8(s32 arg0) {
    osPfsDeleteFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6210.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6264.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F65C4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F67E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6AF4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6BD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6CA4.s")

void func_801F6ED4(u8 arg0) {
    osPfsFileState(&D_8039B630, arg0, &D_80218B20[D_80218D28]);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6F18.s")

s32 func_801F73FC(void) {
    return D_80218D28 != 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F7410.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F74B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F75A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F76E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F7850.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F7F74.s")

s32 func_801F7FF4(Struct801F7FF4 *a, Struct801F7FF4 *b) {
    if (a->unkC != 0 && b->unkC != 0) {
        return a->unk16 - b->unk16;
    }
    if (a->unkC != 0) {
        return -1;
    }
    return 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F803C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F81B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F8228.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F8354.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F8440.s")
