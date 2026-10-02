#include "common.h"
#include <ultra64.h>

extern u32 D_802229E4;
extern u32 D_802229E8;
extern u8 *D_802229F0;
extern u8 *D_802229F4;
extern s32 D_80222A1C;
extern s32 D_80222A20;

/*
 * This whole file is Blast Corps' INFLATE (DEFLATE decompression, RFC 1951)
 * implementation, almost certainly adapted from the classic public-domain
 * inflate.c (Mark Adler, used by zlib/gzip/info-zip throughout the 1990s).
 * Confirmed definitively: inflate_fixed()'s fixed Huffman code-length table
 * initialization (144/256/280/288 boundaries with lengths 8/9/7/8, and 30
 * distance codes all length 5) matches RFC 1951 and the reference source
 * exactly, right down to the table addresses lining up with the expected
 * byte sizes (cplens: 31 entries * 2 bytes = 62, rounded to 64; cpdist: 30
 * entries * 2 bytes = 60, exact fit). huft_build's NEEDBITS/DUMPBITS-style
 * `register` bit-buffer convention (see func_8021F7F4 in this same file)
 * matches the reference source's own `register ulg b; register unsigned k;`
 * declarations verbatim.
 *
 * RFC 1951 fixed Huffman tables (copy lengths/dists + their extra-bit
 * counts). These live inside the opaque init/3A40 binary blob (not
 * individually addressable linker symbols), so reference by raw address -
 * byte-identical to a resolved symbol reference, just without requiring
 * the blob to be split into real data symbols.
 */
#define cplens ((u16 *) 0x80222754)
#define cplext ((u8 *) 0x80222794)
#define cpdist ((u16 *) 0x802227B4)
#define cpdext ((u8 *) 0x802227F0)

extern s32 huft_build(s32 *b, s32 n, s32 s, void *d, void *e, s32 *t, s32 *m);

#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/huft_build.s")

extern s32 inflate_codes(s32 tl, s32 td, s32 bl, s32 bd);

#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/inflate_codes.s")

/* TODO: func_8021F7F4 - bit-accumulator refill/flush routine (same family as
 * the matched func_80220268). Confirmed behavior via m2c; got very close to
 * matching with `register u32 a0, a1` mirroring the target's use of $a0/$a1
 * as persistent values across the whole function (not just call-preserved
 * temps) - this closed almost the entire diff (10269 -> ~900 lines) and
 * correctly reproduced the target's sltiu/srlv instruction choices. The
 * remaining gap is a stack frame size mismatch (target reserves 0x18 bytes,
 * every attempt here lands on 0x10) despite only 8 bytes of that actually
 * being addressed by named locals (sp10/sp14) in either version - likely
 * IDO reserving a standard 16-byte argument-build area that this specific
 * register-heavy, call-free leaf function triggers under some condition
 * not yet identified. Needs more specific IDO knowledge than trial-and-error
 * register/declaration-order tweaks turned up this round. */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/func_8021F7F4.s")

/* TODO: inflate_fixed - confirmed identity (see file header comment) and
 * confirmed logic via m2c (fills l[288] with the RFC 1951 fixed length
 * table, calls huft_build twice, then inflate_codes). Behavior is fully
 * understood; what's missing is the exact stack layout IDO produces. Target
 * places the 288-entry array `l` at the very bottom of the frame (sp+0x2c,
 * right after the $ra save) and all five scalars (the loop index plus
 * tl/bl/td/bd) packed at the very top (sp+0x4ac-0x4bc), whereas every
 * declaration order/register-hint combination tried here instead grouped
 * the scalars immediately after $ra and pushed the array later. Promoting
 * the loop index to `register` does get it into a real register ($s0) but
 * target doesn't use a callee-saved register for it at all, so that's the
 * wrong lever to pull here. Needs a fresh angle on why IDO would place a
 * large address-taken array first and multiple scalars (some of which also
 * have their address taken for huft_build's out-params) last. */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/inflate_fixed.s")
