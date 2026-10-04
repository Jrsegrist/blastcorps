#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/__osDequeueThread.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/__osPiCreateAccessQueue.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97BB0/__osPiGetAccess.s")

extern u8 __osPiAccessQueue[];

void __osPiRelAccess(void) {
    osSendMesg(__osPiAccessQueue, 0, 0);
}

extern OSThread *__osRunningThread;

OSPri osGetThreadPri(OSThread *thread) {
    if (thread == NULL) {
        thread = __osRunningThread;
    }
    return thread->priority;
}
