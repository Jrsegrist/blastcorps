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

/* func_802AFB84: sets $s3 = 0x8c directly (`addiu $s3, $zero, 0x8c`)
 * inside the same dead 8-byte `sd $ra` frame as func_802BBE10/
 * func_802C8AF0/func_802CE9A4/func_802AC284 - same confirmed hand-
 * written family, and doubly so here since C has no way to pin a value
 * to a specific callee-saved register by name regardless. Value is
 * never read or returned; likely a vestigial/debug leftover in the
 * original assembly. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFB84.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFC28.s")
