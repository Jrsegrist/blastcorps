#include "common.h"
#include <ultra64.h>

extern u32 D_802229E4;
extern u32 D_802229E8;
extern u8 *D_802229F0;
extern u8 *D_802229F4;
extern s32 D_80222A1C;
extern s32 D_80222A20;

#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/func_8021ED50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/func_8021F3EC.s")

/* TODO: func_8021F7F4 - bit-accumulator refill/flush routine. Several
 * attempts (m2c as-is, simplified, single-load literal translation) all
 * produced correct behavior but wrong IDO register allocation. Needs more
 * iteration; left as GLOBAL_ASM for now. */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/func_8021F7F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/func_8021F988.s")
