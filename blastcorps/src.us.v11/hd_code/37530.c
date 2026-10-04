#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BCF0.s")

extern u8 D_8036DC90;
extern u8 D_8036DC91;
extern u8 D_8036DC92;
extern s32 D_8036DC94;

/* The dead mid-function `addiu sp` pair is an unused local (-O1 gives
 * every declared local a frame slot, even one never touched). */
void func_8027BE4C(void) {
    s32 unused;

    D_8036DC90 = 0;
    D_8036DC91 = 0;
    D_8036DC92 = 0;
    D_8036DC94 = -1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BE7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027C4C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D350.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D5AC.s")
