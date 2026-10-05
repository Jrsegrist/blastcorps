#include "common.h"
#include <ultra64.h>

/* bestTimes.c (assert file name at 0x8020FF70) */

typedef struct {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u8 pad10[6];
    /* 0x16 */ u16 unk16;
} Struct801F7FF4;

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F7850.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F7F74.s")

s32 func_801F7FF4(Struct801F7FF4 *a, Struct801F7FF4 *b) {
    if (a->unkC != 0 && b->unkC != 0) {
        return a->unk16 - b->unk16;
    }
    if (a->unkC != 0) {
        return -1;
    }
    return 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F803C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F81B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F8228.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F8354.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/10850/func_801F8440.s")
