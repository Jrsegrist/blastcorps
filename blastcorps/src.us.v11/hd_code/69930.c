#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: hand-written assembly (sd/ld $ra frames, results
 * returned in $s8 or $f0), split out of the old 68810 bin blob. Like the
 * 56040..8DDB0 block, these stay GLOBAL_ASM permanently in the matching build.
 *
 * ROM 0x690C0-0x69944 holds the arcsine tables D_802AD880 / D_802AE084 (see
 * 69000.c). Code units must start on a 16-byte boundary, so this file begins
 * 0x14 bytes early, at 0x802AE0F0, and carries the last five words of
 * D_802AE084 as data below. */

/* Last 5 words of the u16 table D_802AE084 (data, not code). */
GLOBAL_ASM(
glabel D_802AE0F0
.word 0x3F513F5B, 0x3F663F71, 0x3F7E3F8B, 0x3F9B3FAD, 0x3FC53FFF
)

/* Fixed-point cosine: in v1 = angle (low 12 bits, 0x1000 = 360 degrees),
 * out s8 = (s32)(cos * scale) via func_802AE1BC. Preserves v1/a0. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE104.s")

/* Fixed-point sine: as func_802AE104 but via func_802AE290. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE160.s")

/* Float cosine core: f12 = radians, returns f0 (range reduction + even
 * polynomial). Clobbers a0, f2-f16. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE1BC.s")

/* Float sine core: f12 = radians, returns f0 (odd polynomial). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE290.s")
