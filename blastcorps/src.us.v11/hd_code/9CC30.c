#include "common.h"
#include <ultra64.h>

/* Scale a double by 2^n (ldexp-like). */
f64 func_802E13F0(f64 arg0, s32 arg2) {
    if (arg2 != 0) {
        arg0 *= (f64) (1 << arg2);
    }
    return arg0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E1418.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E1504.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E15E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E16A4.s")

void func_802E1944(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9CC30/func_802E194C.s")
