#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alEvtqFlushType.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alEvtqFlush.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alEvtqPostEvent.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alEvtqNextEvent.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alEvtqNew.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alUnlink.s")

/* libaudio alLink: insert ln after to. */
void alLink(ALLink *ln, ALLink *to) {
    ln->next = to->next;
    ln->prev = to;
    if (to->next) {
        to->next->prev = ln;
    }
    to->next = ln;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alClose.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alInit.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alSynAddPlayer.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/_allocatePVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/alSynAllocVoice.s")
