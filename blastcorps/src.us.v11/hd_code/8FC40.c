#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8FC40/osStartThread.s")

void func_802D4550(u32 arg0) {
    *(volatile u32 *) 0xA410000C = arg0;
}
