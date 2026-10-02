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
 * instructions, no stack frame). Same unresolved IDO quirk as
 * func_802D9D18 in 95460.c - any phrasing that dereferences twice through
 * an incoming pointer parameter picks up a pointless `addiu sp,sp,-8`/
 * `+8` pair that saves/restores nothing. Logic confirmed correct, not
 * byte-exact yet.
 *
 * void func_802DC178(void **arg0) {
 *     void *v0 = *arg0;
 *     *arg0 = *(void **) v0;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC178.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC188.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/972A0/func_802DC2D0.s")
