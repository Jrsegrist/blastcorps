#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80278BF0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80278E3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80278EB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_802794A4.s")

extern u8 D_8036D178;

void func_802794E4(void) {
    D_8036D178 = 0;
}

/* TODO: func_802794F0 - boolean double-negation of a byte global:
 * `!!D_8036D178` (target: 9 instructions, with a dead `addiu sp,sp,-8`/
 * `+8` pair - same phantom-frame quirk as elsewhere in this segment).
 * Closest found (`register s32 temp = (D_8036D178==0); temp = (temp==0);
 * return temp;`) reproduces the dead frame and the two `sltiu`
 * instructions exactly, but target's second `sltiu` writes directly back
 * into the same register as the first (`a0`) with no further move before
 * the final `andi`/`jr`, while every phrasing tried adds one trailing
 * register-to-register `move` (10 instructions, one over). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_802794F0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80279514.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_802796D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80279778.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80279EE8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027A7DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027AA04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027AC00.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B200.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B5D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B87C.s")
