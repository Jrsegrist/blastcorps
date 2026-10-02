#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/679E0/func_802AC1A0.s")

/* TODO: func_802AC284 - trivial no-arg trampoline: `func_802AC3B8();`
 * (target: 8 instructions, `addiu sp,sp,-8`/`sd $ra,($sp)`/.../`addiu
 * sp,sp,8` - an 8-byte doubleword $ra save). Plain C compiles this to the
 * "normal" o32 argument-shadow frame instead (`addiu sp,sp,-24`/`sw
 * $ra,20($sp)`/.../`addiu sp,sp,24`) regardless of whether the callee's
 * prototype is visible at the call site - same instruction count, same
 * total size (so it doesn't cause address drift), just the wrong frame
 * convention. Likely the same unresolved frame-size-choice quirk behind
 * func_802BBE10/func_802C8AF0's dead-frame near-miss, just manifesting on
 * a real (non-dead) $ra save this time. func_80260EC0 in hd_code/1C460.c
 * is an outwardly-identical no-arg trampoline that *did* match with the
 * plain 24-byte-frame form, so whatever picks between the two conventions
 * isn't visible from this function's C alone. */
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
