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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FCD4.s")
