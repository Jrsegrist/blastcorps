#include "common.h"
#include <ultra64.h>

/* __ull_rshift through __ll_rshift (10 functions, this whole block):
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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ull_rshift.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ull_rem.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ull_div.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_lshift.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_rem.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_div.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_mul.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ull_divremi.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_mod.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/90390/__ll_rshift.s")

s32 func_802D4E10(void *arg0) {
    return *(s32 *) ((u8 *) arg0 + 0x2c);
}
