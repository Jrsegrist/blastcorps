#include "common.h"

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE370.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE860.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE888.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEB9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEC3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEE84.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEEC8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF340.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF4BC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFA64.s")

/* TODO: func_802AFB84 - sets $s3 = 0x8c directly (`addiu $s3, $zero, 0x8c`)
 * inside a dead $ra save/restore frame, never read or returned. Not
 * reproducible from plain C - a plain local goes dead and gets optimized
 * away under -O1 since nothing reads it, and C has no way to pin a value
 * to a specific callee-saved register by name. Possibly a vestigial/dead
 * leftover in the original source, or part of a larger pattern not yet
 * understood. Logic not yet matched. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFB84.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFC28.s")
