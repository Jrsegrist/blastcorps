#include "common.h"
#include <ultra64.h>

/* __d_to_ll/__f_to_ll: `trunc.l.d`/`trunc.l.s` + `dmfc1` directly
 * truncate f64/f32 to a 64-bit int and move the full 64 bits out of the
 * FPU in one shot - confirmed via a standalone probe compile that IDO's
 * own `(s64) x` cast NEVER generates this: it always calls a runtime
 * helper (`__d_to_ll`/`__f_to_ll`) instead, even for the exact same
 * source shape. `dmfc1` moving a full 64-bit FPU register into a GPR is
 * also not something O32-targeting IDO code generation does on its own.
 * Same hand-written-leaf-stub character as init's __osGetSR/__osSetSR
 * and this segment's raw COP0 stubs - these are permanently GLOBAL_ASM,
 * not blocked on a missing runtime symbol. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__d_to_ll.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__f_to_ll.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__d_to_ull.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__f_to_ull.s")

/* __ll_to_d/__ll_to_f: the reverse direction - reconstruct a
 * 64-bit value from two 32-bit argument-register halves (`dmtc1`) and
 * convert to double/float (`cvt.d.l`/`cvt.s.l`) directly, no runtime
 * call. Same hand-written-leaf-stub family as __d_to_ll/__f_to_ll
 * above - `dmtc1` of two separately-passed 32-bit GPR args is likewise
 * not O32 IDO codegen. Permanently GLOBAL_ASM. (Checked the
 * n64decomp/banjo-kazooie repo for __ll_to_d/__ll_to_f/__d_to_ll/__f_to_ll
 * by name first - no hits there either, consistent with these being
 * hand-written rather than IDO runtime library routines at all.) */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__ll_to_d.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__ll_to_f.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__ull_to_d.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/95C40/__ull_to_f.s")
