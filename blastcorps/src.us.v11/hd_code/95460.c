#include "common.h"
#include <ultra64.h>

void func_802D9C20(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/_timeToSamples.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/_freePVoice.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/_collectPVoices.s")


void __freeParam(s32 *arg0) {
    s32 *v0 = alGlobals;

    *arg0 = v0[0xb];
    v0[0xb] = (s32) arg0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/__allocParam.s")

void func_802D9D60(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/func_802D9D68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95460/alSynNew.s")
