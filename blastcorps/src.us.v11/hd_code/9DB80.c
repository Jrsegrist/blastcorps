#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9DB80/func_802E2340.s")

void func_802E2650(u32 arg0) {
    *(volatile u32 *) 0xA4040010 = arg0;
}
