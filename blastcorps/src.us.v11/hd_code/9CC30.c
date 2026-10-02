#include "common.h"
#include <ultra64.h>

/* TODO: func_802E13F0 - scale a double by 2^arg2: `if (arg2 != 0) arg0 *=
 * (f64)(1 << arg2); return arg0;` (target: 10 instructions). Matches
 * target exactly except the branch's own delay slot: target schedules
 * the (unconditional, branch-independent) `addiu $t6,$zero,1` into it,
 * every phrasing tried (plain if, ternary, pre-hoisting the shift above
 * the branch) leaves a `nop` there instead and does the `li` afterward
 * (11 instructions, one over) or restructures the whole function less
 * favorably. Pure delay-slot-scheduling gap, logic exactly right. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E13F0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E1418.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E1504.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E15E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E16A4.s")

void func_802E1944(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E194C.s")
