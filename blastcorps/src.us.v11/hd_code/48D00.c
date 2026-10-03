#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028D4C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DA5C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DD64.s")

extern u8 D_8039B070;
extern s32 D_8039B610;

/* TODO: func_8028DE94 - linear search of a D_8039B610-length, 0x48-stride
 * array at D_8039B070 for the first entry with both unk18 and unk14
 * nonzero, returning its address or NULL:
 * `do { if (entry->unk18 && entry->unk14) return entry;
 *  temp = i+1; i = temp; } while (temp < D_8039B610); return NULL;`
 * (target: 32 instructions, frame/offsets/structure all exact). Logic and
 * layout fully confirmed correct - current gap is pure register-rename
 * (diff score 45, every remaining line marked 'r', zero inserts/deletes):
 * target uses $t3/$t5 for the loop-bound reload and increment-compare
 * temp where every phrasing tried (plain locals, `register` on the temp
 * alone, `register` on both the index and the temp) lands on $a0/$t3
 * instead. A 90s decomp-permuter run (tools/permuter) against this exact
 * state found nothing better. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DE94.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DF14.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028E9E4.s")

void func_802AACD4(u8, s32, s32, void *, void *);
extern u8 D_8039B094;

/* TODO: func_8028F6B4 - for each D_8039B610-length array entry (0x48
 * stride) matching both unk18!=0 and unk23==arg0: clear unk1E, set the
 * corresponding D_8039B094 flag byte to 1, then forward several of the
 * entry's fields into func_802AACD4. Logic, structure, and every offset
 * confirmed correct (diff score as low as 130 with zero inserts/deletes
 * besides one spurious reload). Remaining gap: after the func_802AACD4
 * call clobbers the loop index's register, target reuses the freshly
 * computed `i+1` value directly for the loop condition with no further
 * reload, while every phrasing tried (plain post-increment, an explicit
 * `next` temp matching m2c's own inferred split, both declaration orders,
 * `register` on the index or the temp) either leaves one extra reload in
 * or - worse - gives the temp its own separate stack slot, growing the
 * frame and drifting every later address in the file. The explicit-temp
 * trick that fixed this exact pattern in func_8028DE94 (above) does not
 * carry over here, apparently because of the intervening function call. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028F6B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028F794.s")

extern u8 D_8039B094;

/* TODO: func_8028F93C - zero one byte field (offset 0x24, stride 0x48)
 * across D_8039B610 array entries: `*(&D_8039B094 + i * 0x48) = 0;` (target:
 * 20 instructions, no frame). Logic/structure confirmed correct - the
 * remaining gap is the increment instruction (`addiu t0,t9,1`) landing one
 * slot earlier in the target's schedule than any phrasing tried produces
 * (plain post-increment matches structurally but the scheduler orders it
 * after the store instead of before; hoisting it into an explicit temp
 * computed first adds a second stack slot instead of just reordering).
 * Pure instruction-scheduling gap. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028F93C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028F994.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FAC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FC10.s")

/* TODO: func_8028FCD4 - wait for arg0->unk8 to become nonzero
 * (func_802DB850(arg0); while (arg0->unk8 == 0) {}), reset it via
 * func_802D4910(arg0,0,0), fill a 4-entry/4-byte-stride local array via
 * func_802DB8D4((s32) sp20), then for each entry with (unk2&1) and
 * unk3==0 set the matching bit in *arg1; return sp20[0].unk3 (always the
 * first entry's byte, not sp20[i]). Logic, the spin-wait shape, every
 * field offset, and the overall structure are all confirmed correct
 * (asm-differ score as low as 290, zero structural inserts in the bulk of
 * the function). Remaining gap is the same loop-tail pattern documented
 * on func_8028F6B4 above: target defers the incremented index's store
 * into the branch's own delay slot (`slti at,t6,4; bnez at,loop; sw
 * t6,0x1c(sp)`), while this phrasing stores-then-reloads before the
 * compare instead. The m2c-style split-temp fix that works for this
 * exact pattern when there's no function call in the loop body (see
 * func_8028DE94 above) does not reproduce it here either - it trades the
 * extra reload for an extra dedicated stack slot instead. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FCD4.s")
