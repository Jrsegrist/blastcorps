#include "common.h"
#include <ultra64.h>

void func_802D4550(u32 arg0) {
    *(volatile u32 *) 0xA410000C = arg0;
}
