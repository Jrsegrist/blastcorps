#include "common.h"

/* func_802DB990/func_802DB9A0: raw COP0 Status-register writes/reads
 * (mtc0/mfc0 $12), same character as __osSetSR/__osGetSR in init - genuine
 * hand-written SDK leaf stubs, not expressible in plain C. Permanently
 * GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/971D0/func_802DB990.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/971D0/func_802DB9A0.s")
