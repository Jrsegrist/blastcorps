#include "common.h"
#include <ultra64.h>

void func_802D9C20(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9C28.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9C80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9CB8.s")

/* TODO: func_802D9D18 - classic linked-list push: `s32 *v0 = D_803065C0;
 * *arg0 = v0[0xb]; v0[0xb] = (s32) arg0;` (target: 5 instructions, no
 * stack frame, v0 cached in a single register throughout). Every
 * phrasing tried (void-pointer or s32-pointer typed, register or plain,
 * pointer-cast vs integer-cast) either reloads the global twice (no
 * register caching)
 * or introduces a pointless `addiu sp,sp,-8`/`+8` pair that saves/
 * restores nothing - looks like an IDO quirk for this specific shape
 * (two writes through an incoming pointer param plus one cached global)
 * rather than anything fixable by rephrasing the expression. Logic is
 * 100% understood, just not byte-exact yet.
 *
 * extern s32 *D_803065C0;
 *
 * void func_802D9D18(s32 *arg0) {
 *     s32 *v0 = D_803065C0;
 *     *arg0 = v0[0xb];
 *     v0[0xb] = (s32) arg0;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9D18.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9D30.s")

void func_802D9D60(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9D68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9FF0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802DA2F0.s")
