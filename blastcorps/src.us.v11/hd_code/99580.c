#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/99580/func_802DDD40.s")

/* TODO: func_802DDD6C - strlen: `u8 *v1 = arg0; while (*v1) v1++; return
 * v1 - arg0;` (target: 10 instructions, no stack frame, v1 reused across
 * the whole loop with no reload). Same forced either/or as
 * func_802D9D18/func_802D6EE0 (see hd_code/95460.c) - any declared local
 * that needs to persist across more than one statement reserves a dead
 * addiu-sp,-8/+8 frame in this compiler, even register-qualified and
 * even kept purely in a register (confirmed: `register u8 *v1` lands in
 * $a1 with no spill, but the frame still appears, 12 instructions
 * instead of target's 10). Computing the length needs two live values
 * (the moving pointer and the original start) which can't both be had
 * without a declared local. Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/99580/func_802DDD6C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/99580/func_802DDD94.s")
