#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E2EC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E31C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E37C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E38A0.s")

void alSeqGetLoc(ALSeq *seq, ALSeqMarker *m) {
    m->curPtr = seq->curPtr;
    m->lastStatus = seq->lastStatus;
    m->lastTicks = seq->lastTicks;
}

void alSeqSetLoc(ALSeq *seq, ALSeqMarker *m) {
    seq->curPtr = m->curPtr;
    seq->lastStatus = m->lastStatus;
    seq->lastTicks = m->lastTicks;
}

s32 alSeqGetTicks(ALSeq *seq) {
    return seq->lastTicks;
}

void func_802E3FD0(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E3FD8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/alSeqNextEvent.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/alSeqNewMarker.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/alSeqSecToTicks.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/alSeqTicksToSec.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/__alSeqNextDelta.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4458.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E44A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/alSeqNew.s")
