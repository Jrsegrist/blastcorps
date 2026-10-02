#include "common.h"
#include <ultra64.h>

void func_802D9C20(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9C28.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9C80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9CB8.s")

/* TODO: func_802D9D18 - classic linked-list push: `s32 *v0 = D_803065C0;
 * *arg0 = v0[0xb]; v0[0xb] = (s32) arg0;` (target: 5 instructions, no
 * stack frame, v0 cached in a single register throughout, used again
 * *after* an intervening store through arg0 with no reload).
 *
 * Root cause now understood (not just observed) via systematic probe
 * compiles: (1) declaring ANY local in this function - even
 * `register`-qualified, even when IDO keeps it purely in a register and
 * never spills it - unconditionally reserves a dead 8-byte stack frame;
 * confirmed with single-use plain-int locals that have nothing to do
 * with pointers. (2) This compiler does no cross-statement CSE for
 * globals: writing the same dereference expression twice with no local
 * at all reloads it fresh every time, even with nothing in between that
 * could plausibly alias it. Since the target has neither a frame nor a
 * reload, caching `v0` across the intervening store without a declared
 * local is the one combination this compiler cannot produce from any
 * phrasing tried - a genuine either/or forced by the compiler, not a
 * phrasing failure. Logic 100% understood either way.
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
