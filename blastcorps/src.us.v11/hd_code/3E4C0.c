#include "common.h"
#include <ultra64.h>

extern f32 sqrtf(f32);
extern s32 func_802AD7D4(s32);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/3E4C0/func_80282C80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/3E4C0/func_8028376C.s")

/*
 * Heading in degrees (0..360) from (x1, z1) to (x2, z2), one quadrant at a time.
 * func_802AD7D4 maps a 0..65535 sine to a 0..0x3FFF binary angle. No return if
 * no quadrant test passes (original falls off the end).
 */
f32 func_80284ADC(s16 x1, s16 z1, s16 x2, s16 z2) {
    f32 dist;

    dist = sqrtf((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1));
    if (dist < 1.0) {
        return 0.0f;
    }
    if (x2 >= x1 && z2 >= z1) {
        return func_802AD7D4((x2 - x1) * 65535.9 / dist) / 65536.0 * 360.0;
    }
    if (x2 >= x1 && z2 < z1) {
        return (func_802AD7D4((z1 - z2) * 65535.9 / dist) + 0x4000) / 65536.0 * 360.0;
    }
    if (x2 < x1 && z2 < z1) {
        return (func_802AD7D4((x1 - x2) * 65535.9 / dist) + 0x8000) / 65536.0 * 360.0;
    }
    if (x2 < x1 && z2 >= z1) {
        return (func_802AD7D4((z2 - z1) * 65535.9 / dist) + 0xC000) / 65536.0 * 360.0;
    }
}
