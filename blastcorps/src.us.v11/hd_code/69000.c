#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: hand-written assembly (sd/ld $ra frames, trapping
 * add/sub, results returned in $s8), split out of the old 68810 bin blob.
 * Like the 56040..8DDB0 block, these stay GLOBAL_ASM permanently in the
 * matching build.
 *
 * Layout of ROM 0x68810-0x690C0: u16 table D_802ACFD0 (read by
 * func_802ACF64), then this code at 0x802AD7D4. Code units must start on a
 * 16-byte boundary, so this file begins 0x14 bytes early, at 0x802AD7C0, and
 * carries the table's last five words as data below. */

/* Last 5 words of the u16 table D_802ACFD0 (data, not code). */
GLOBAL_ASM(
glabel D_802AD7C0
.word 0x35EF35F1, 0x35F335F6, 0x35F835FB, 0x35FD35FF, 0x36020000
)

#ifdef NON_MATCHING
/* Arcsine tables in the 690C0 text bin (read in place). */
extern u16 D_802AD880[]; /* 1025 entries, sin -> angle every 64 steps */
extern u16 D_802AE084[]; /* 64 entries, the last (steep) segment */

s32 func_802AD7FC(u32 sine);
#endif

/* C-callable wrapper: return func_802AD7FC(a0) in v0 (00000.c camera code,
 * 3E4C0.c). */
#ifdef NON_MATCHING
/* The asm also leaves v1 = a0 and preserves fp; no caller (all C) uses that. */
s32 func_802AD7D4(s32 sine) {
    return func_802AD7FC(sine);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69000/func_802AD7D4.s")
#endif

/* Arcsine lookup: in v1 = sine in 0..0xFFFF, out s8 = angle (0x4000 = 90
 * degrees). Linear interpolation in D_802AD880[v1 >> 6] (1025 entries); the
 * last segment uses the finer table D_802AE084[v1 & 0x3F]. Preserves v1/a0/a1. */
#ifdef NON_MATCHING
/* Convention: in v1=a0 ; out fp=ret. Asm callers rely on it preserving v1, a0,
 * a1 (it saves them) and t1/t2/t3/t6/t7 (untouched); a mixed build needs a
 * thunk. Any 32-bit input is accepted: index = sine >> 6 (logical), anything
 * from 0x3FF up (i.e. sine >= 0xFFC0, or "negative" values) takes the fine
 * table by the low 6 bits. The product and shift are unsigned as in the asm
 * (mult, then srl). */
s32 func_802AD7FC(u32 sine) {
    u32 idx = sine >> 6;
    u32 frac = sine & 0x3F;
    s32 lo;

    if ((s32) idx < 0x3FF) {
        lo = D_802AD880[idx];
        return lo + ((u32) ((D_802AD880[idx + 1] - lo) * (s32) frac) >> 6);
    }
    return D_802AE084[frac];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69000/func_802AD7FC.s")
#endif
