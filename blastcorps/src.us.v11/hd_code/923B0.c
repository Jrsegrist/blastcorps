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
 * `*arg1` exactly twice total - once for the initial copy, once more
 * reused for *both* the null-check and the body).
 *
 * Same forced either/or as func_802D9D18 in hd_code/95460.c (see that
 * file's comment for the full probe evidence): no declared local means
 * no frame, but this compiler has no cross-statement CSE for globals, so
 * every textually-separate dereference reloads fresh - confirmed even
 * for an if-condition and its own body (`if (*arg1) { use *arg1 }`
 * reloads *arg1* a third time for the body, measured directly). Declaring
 * a local to carry the condition's value into the body avoids that third
 * reload but reintroduces the dead frame, even once the local lands in a
 * real register (`$a2`) with no spill. Three-reread/no-frame and
 * two-reread/dead-frame are the only two phrasings this compiler
 * produces; target's two-reread/no-frame combination isn't reachable
 * from any C tried. Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6EE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6FC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D70A8.s")
