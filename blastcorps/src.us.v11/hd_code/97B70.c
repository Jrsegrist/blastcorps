#include "common.h"

/* func_802DC330/func_802DC350: raw COP0 Status-register read-modify-write
 * (`mfc0`/`mtc0 $12`, clearing or setting a bit directly) - same hand-
 * written COP0-leaf-stub family as init's __osSetSR/__osGetSR.
 * Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97B70/func_802DC330.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97B70/func_802DC350.s")
