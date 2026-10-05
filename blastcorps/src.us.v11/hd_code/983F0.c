#include "common.h"
#include <ultra64.h>

/* libultra libc/xprintf.c (_Printf and its static _Putfld) from an older
 * SDK than ultralib models; ultralib's version at -O1 differs throughout. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/983F0/_Printf.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/983F0/_Putfld.s")
