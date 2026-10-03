#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028D4C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DA5C.s")

/* TODO: func_8028DD64 - for the D_8039B610 array entry at index arg0 (0x48
 * stride): forward its first 3 words into func_802CDA10, set its unk19 byte
 * to 5 and unk18 byte to 0, then set D_802E8BE4=10/D_802E8BE8=0x190 and, if
 * its unk40 word is nonzero, notify func_802608C8 and - if func_8028DE94()
 * (a no-arg linear search, see below) finds a match - forward it into
 * func_80260650(D_80367738, 0x73, match+0x40); finally, if unk44 is nonzero,
 * notify func_802608C8 of it too, then unconditionally call
 * func_80260650(D_80367738, 0x10, 0). Logic, every field offset, and the
 * overall control flow are all confirmed correct (diff score down to 300,
 * zero inserts/deletes - every single instruction present and in the right
 * place except a 4-instruction window). The one remaining gap: while
 * clearing the unk18 byte, target interleaves the tail of that address
 * calculation with the *start* of the next statement's (the unk40 lookup's)
 * address calculation one instruction later than every phrasing tried
 * produces - a pure instruction-scheduling-window artifact between two
 * independent, back-to-back statements, not a logic or layout gap. Tried:
 * several different statement orderings/interleavings of the two preceding
 * global stores (D_802E8BE4/D_802E8BE8) relative to the array writes, all
 * of which only made the score worse (710-1010) by disturbing other,
 * already-matching regions - this phrasing is the local optimum found. */
extern u8 D_8039B070;
extern u8 D_8039B088;
extern u8 D_8039B089;
extern u8 D_8039B0B0;
extern u8 D_8039B0B4;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern s32 D_80367738;

void func_802CDA10(s32, s32, s32);
void func_802608C8(s32);
void func_80260650(s32, s32, s32);
void *func_8028DE94(void);

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

/* TODO: func_8028F994 - scale arg0/arg1/arg2 (a 3D point, 1/32 fixed point)
 * down to integer units, then for each D_8039B610 array entry with unk18
 * nonzero: call func_8026A6F0(point, entry's unk0/unk4/unk8 point, also
 * scaled down) to get a distance, look up a per-category radius via a
 * stride-0x18 table at 0x802FDBAC indexed by the entry's unk0E byte (also
 * scaled down), and set D_803A7424=1 if that radius is >= the distance (an
 * "is this entry within its category's radius" check). The 0x802FDBAC table
 * has no existing symbol - nothing else in the project touches it yet, so
 * it's addressed as a raw literal the same way splat itself left it
 * unresolved in the target disassembly. Logic, every field offset, and the
 * overall control flow are all confirmed correct (diff score down to 236,
 * zero inserts/deletes, frame size exact) - every remaining difference is a
 * same-value register rename (IDO chose a different physical register for
 * the same operation), stemming from reloading arg0/arg1/arg2 from their
 * stack homes in a different order than target at the very top of the
 * function. Tried reversing the 3 scale-down statements' source order;
 * made no difference, so the reload order isn't driven by source order
 * here and wasn't tracked down further. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028F994.s")

/* TODO: func_8028FAC0 - same per-entry radius check as func_8028F994
 * (above: scale arg0/arg1/arg2 down, get a distance via func_8026A6F0,
 * look up a per-category radius via the 0x802FDBAC table indexed by
 * unk0E), plus two differences: entries with unk24 nonzero are skipped
 * entirely, and a 4th arg (also scaled down) is added to the looked-up
 * radius before the >= dist comparison. Logic, every field offset, and
 * the overall control flow are all confirmed correct (diff score down to
 * 387, zero inserts/deletes, frame size exact) - every difference is a
 * same-value register rename, the same reload-order artifact documented
 * on func_8028F994, plus one extra instance of it (dist's post-call spill
 * swapped with arg3's reload, one slot apart) in the new tail. Tried
 * commuting the final addition's operand order; no effect. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FAC0.s")

/* TODO: func_8028FC10 - set up D_80370BF8 via func_802DB4D0/func_802D4910,
 * read a u16 status word via func_802DB594 and set a local flag if bit
 * 0x1000 is set, call func_8028FCD4 and conditionally latch the flag into
 * D_802FDBD0, then derive D_802FDBD4 as "flag was set AND D_802FDBD0 was
 * zero" (target: 0x40-byte frame, no loops). Logic and call sequence
 * confirmed correct; declaring the final boolean `register` (to match
 * target's use of the callee-saved $s0 across the whole function, visible
 * directly in the target disassembly) dropped the score from 1706 to 740
 * and fixed the $s0 save/restore and register-class markers throughout.
 * Remaining gap: target's frame is 0x40 bytes but every phrasing tried
 * only needs 0x28-0x30 for the same three locals - something about the
 * original source uses roughly 24 more bytes of stack than this
 * reconstruction does (not a padding/alignment artifact; the three local
 * variables' own offsets shift to fill whatever space is allocated, so
 * this isn't simply "declare one more unused local" without knowing what
 * it should be). */
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
