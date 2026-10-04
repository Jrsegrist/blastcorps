#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__setInstChanState.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__resetPerfChanState.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__initFromBank.s")

void func_802DDFF8(void) {
}

s32 __vsDelta(void *arg0, s32 arg1) {
    s32 diff = *(s32 *) ((u8 *) arg0 + 0x24) - arg1;

    if (diff >= 0) {
        return diff;
    } else {
        return 1000;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__vsVol.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__seqpReleaseVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__voiceNeedsNoteKill.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__unmapVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__postNextSeqEvent.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE3CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__vsPan.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__lookupVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__mapVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__lookupSoundQuick.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE66C.s")

void func_802DEE84(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DEE8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__seqpStopOsc.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/__initChanState.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/alSeqpNew.s")
