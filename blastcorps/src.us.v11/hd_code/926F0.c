#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/926F0/alUnlink.s")

/* libaudio alLink: insert ln after to. */
void alLink(ALLink *ln, ALLink *to) {
    ln->next = to->next;
    ln->prev = to;
    if (to->next) {
        to->next->prev = ln;
    }
    to->next = ln;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/926F0/alClose.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/926F0/alInit.s")
