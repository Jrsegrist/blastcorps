#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC1A0.s")

/* func_802AC284: trivial no-arg trampoline (`func_802AC3B8();`) using an
 * 8-byte `addiu sp,sp,-8`/`sd $ra,($sp)`/.../`addiu sp,sp,8` frame.
 * Confirmed via probe compile that IDO's own codegen for a real call
 * ALWAYS uses the 24-byte o32 argument-shadow frame (`sw $ra`) instead,
 * regardless of callee prototype visibility or any other phrasing tried
 * - the 8-byte `sd $ra` style literally never comes out of this
 * compiler for a genuine call. Same hand-written-stub family as
 * func_802BBE10/func_802C8AF0/func_802CE9A4 above. func_80260EC0 in
 * hd_code/1C460.c is an outwardly identical trampoline that matched
 * fine with the normal 24-byte form - that one really is compiler
 * output; this one isn't. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC284.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC2A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC3B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC4C4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC544.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC61C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC6FC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC7DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC85C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC8CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACA60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACAC4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACB50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACBDC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACC68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACCCC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACDB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACE38.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACEB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802ACF64.s")
