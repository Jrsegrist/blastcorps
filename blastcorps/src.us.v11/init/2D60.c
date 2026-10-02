#include "common.h"
#include <ultra64.h>

/*
 * IDO's own internal runtime helpers for 64-bit arithmetic the compiler
 * won't inline for -mips2 (confirmed empirically: compiling e.g. `u64 >> u64`
 * anywhere makes IDO emit a `jal __ull_rshift` relocation, so these can't be
 * written in C without the compiler calling itself recursively - must stay
 * asm). Kept as one combined nonmatching block, not split per-function, since
 * splitting let the assembler's per-unit 16-byte section padding silently
 * insert extra bytes between them that aren't in the real ROM.
 *
 * func_80221B30 duplicates __ull_rem's exact codegen at a different address;
 * func_80221BF8 has a different shape entirely (two ddivu ops against
 * halfword operands, storing two results) and is likely Rare's own code, not
 * an IDO runtime helper. Neither is renamed.
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/2D60/ido_ll_helpers.s")
