#include "common.h"
#include <ultra64.h>

/* func_802A1320: `mfc0 $v0, $12` (read COP0 Status register) wrapped in a
 * dead $ra save/restore frame - same hand-written COP0-leaf-stub character
 * as __osGetSR in init/2330.c. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1320.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A133C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1388.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1558.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1674.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1934.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A19F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1A9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C20.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C88.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1D54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1EC8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A20F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2164.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A21AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A23E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2458.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A24BC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2608.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A26A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2A98.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2BB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2C54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2D68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3008.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A303C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A30DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3134.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3198.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A32CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A350C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3824.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A396C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3D54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3DF8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3E9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3F80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4168.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A41B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4464.s")

/* TODO: func_802A44E4 - round up to a multiple of 8 if not already a
 * multiple of 4: `if ((arg1 & 3) != 0) arg1 = (arg1 & ~7) + 8; return
 * arg1;` (target: 11 instructions, a `nop` in the branch's delay slot -
 * $a1 is the real parameter, with an unused leading parameter shadowing
 * $a0). Every type tried for the unused first parameter (void*, s32,
 * f32) makes IDO spill it into the delay slot anyway (`sw`/`swc1` at
 * 0(sp), no actual stack frame reserved for it) - the inverse of the
 * usual "unused argument still gets a dead stack home" pattern seen
 * elsewhere (func_80272C40 in init/26570.c): there the target WANTS the
 * dead spill and plain C gives it one; here the target doesn't want it
 * and plain C can't be talked out of it. Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A44E4.s")
