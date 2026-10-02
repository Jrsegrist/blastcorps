#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6B70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6C1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6C8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6DB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6E3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6EB0.s")

/* TODO: func_802D6EE0 - doubly-linked-list insert-before: `arg0->next =
 * arg1; arg0->prev = *arg1; if (*arg1 != NULL) (*arg1)->prev = arg0;
 * *arg1 = arg0;` (target: 9 instructions, no stack frame, rereading
 * `*arg1` exactly twice - once cached across the null-check and the body,
 * once for the initial copy). Every phrasing tried (fully inlined triple
 * reread, a plain local reused for check+body, a register-qualified
 * local reused for check+body) either adds a third reread or picks up
 * the same pointless `addiu sp,sp,-8`/`+8` dead-frame quirk seen on
 * func_802D9D18/func_802DC178 in hd_code/95460.c and 972A0.c - even once
 * the local itself lands in a real register (`$a2`) with no spill.
 * Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6EE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6FC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D70A8.s")
