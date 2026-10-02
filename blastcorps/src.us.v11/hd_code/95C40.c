#include "common.h"
#include <ultra64.h>

/* func_802DA400/func_802DA41C: `(s64) arg0` from f64/f32 - exactly what the
 * trunc.l.d/trunc.l.s hardware instructions compute, but IDO instead emits
 * calls to its own `__d_to_ll`/`__f_to_ll` runtime helpers (same family as
 * __ll_to_d/__ll_to_f below), which aren't linked into this segment. Same
 * missing-private-copy situation as func_802DA574/func_802DA58C. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA400.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA41C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA438.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA4D8.s")

/* func_802DA574/func_802DA58C: reconstruct a 64-bit value from its two
 * 32-bit argument-register halves (a0/a1) and convert to double/float -
 * exactly what `(f64)(s64)x` / `(f32)(s64)x` should compile to, but IDO
 * emits calls to `__ll_to_d`/`__ll_to_f` (its own s64->float runtime
 * helpers, same family as __ll_mul/__ll_div/__ll_rshift already matched
 * in init/ido_ll_helpers.s) which aren't linked into this segment -
 * hd_code likely has its own private copies somewhere among its still-
 * anonymous functions, same pattern as the duplicated __os-family/COP0
 * stubs found elsewhere in this segment. Needs those symbols located and
 * declared (or symbol_addrs'd) before these two can link. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA574.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA58C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA5A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/func_802DA5D8.s")
