#include "common.h"
#include <ultra64.h>

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

/* C-callable wrapper: return func_802AD7FC(a0) in v0 (00000.c camera code,
 * 3E4C0.c). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69000/func_802AD7D4.s")

/* Arcsine lookup: in v1 = sine in 0..0xFFFF, out s8 = angle (0x4000 = 90
 * degrees). Linear interpolation in D_802AD880[v1 >> 6] (1025 entries); the
 * last segment uses the finer table D_802AE084[v1 & 0x3F]. Preserves v1/a0/a1. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69000/func_802AD7FC.s")
