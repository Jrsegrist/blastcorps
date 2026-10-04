#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E050/__osSiCreateAccessQueue.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E050/__osSiGetAccess.s")

extern u8 __osSiAccessQueue[];

void __osSiRelAccess(void) {
    osSendMesg(__osSiAccessQueue, 0, 0);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E050/__osSiDeviceBusy.s")
