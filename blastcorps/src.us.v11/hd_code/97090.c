#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/97090/osContStartQuery.s")

void __osContGetInitData(u8 *pattern, OSContStatus *data);

void osContGetQuery(OSContStatus *data) {
    u8 pattern;
    __osContGetInitData(&pattern, data);
}
