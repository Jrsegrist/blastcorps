#include "common.h"
#include <ultra64.h>

/* func_802DBA60: `lui $k0,%hi(func_802DBA70); addiu $k0,$k0,%lo(func_802DBA70);
 * jr $k0; nop` - a raw kernel-register ($k0) tail-jump stub, same character
 * as __osExceptionPreamble in init/2330.c (confirmed hand-written SDK/boot
 * assembly there, not decompilable from idiomatic C). Permanently
 * GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBA60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBA70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DBFB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC0A0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC130.s")

/* TODO: func_802DC178 - pointer-chase/unlink: `*arg0 = **arg0;` (target: 4
 * instructions, no stack frame, correct instruction shape achieved with
 * `void func_802DC178(void **arg0) { *arg0 = *(void **) *arg0; }` -
 * identical structure to target (lw/lw/jr/sw, no frame), but target uses
 * $v0 for the first loaded value and $t9 for the second, while every
 * phrasing tried (plain, returning the loaded value, returning the
 * assignment expression directly) allocates $t6/$t7 instead - pure
 * register-rename gap, lowest severity, but still real bytes differing.
 * Logic and structure fully confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC178.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC188.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC2D0.s")
