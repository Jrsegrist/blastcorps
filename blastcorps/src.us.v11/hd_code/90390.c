#include "common.h"
#include <ultra64.h>

/* func_802D4B50 through func_802D4DE4 (10 functions, this whole block):
 * byte-for-byte the same IDO 64-bit integer arithmetic runtime helpers
 * already identified and matched in init/2D60/ido_ll_helpers.s
 * (__ull_rshift/__ull_div/__ull_rem/__ll_div/__ll_mul/__ll_rem/
 * __ll_rshift/__ll_lshift plus the two odd ones that don't fit a named
 * pattern) - confirmed via a standalone probe compile in that earlier
 * session that IDO's own 64-bit shift/divide/multiply operators call
 * these exact symbols, and writing matching C for them makes the
 * compiler call itself recursively. hd_code gets its own private
 * statically-linked copy rather than sharing init's, same situation as
 * the duplicated COP0/__os-family stubs found elsewhere in this
 * segment. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4B50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4B7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4BB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4BF4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4C20.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4C5C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4CB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4CE8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4D48.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/func_802D4DE4.s")

s32 func_802D4E10(void *arg0) {
    return *(s32 *) ((u8 *) arg0 + 0x2c);
}
