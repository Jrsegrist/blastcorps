#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: hand-written assembly (sd/ld $ra frames, trapping
 * addi on $sp, every register saved including k0/k1/gp/sp), split out of the
 * old 7D9D0_data bin blob. Like the 56040..8DDB0 block, these stay GLOBAL_ASM
 * permanently in the matching build. Data on both sides (7D9D0_data,
 * 800E0_data) stays bin; both boundaries are 16-aligned. */

/* C-callable (46C20.c func_8028B4C4): saves s0-s7/gp/s8, then
 * func_802C41C0(*a0, *a1) and writes the advanced pointers back to *a0/*a1. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4070.s")

/* Saves s-regs, stores a0/a1 to D_803F7830/D_803F7834 and calls
 * func_8025C230(&D_803F7830, &D_803F7834, a2); returns the updated values in
 * a0/a1 (non-ABI outputs). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4108.s")

/* Bit-packed LZSS decompressor: a0 = source, a1 = destination, a2 = ring
 * window, a3 = offset width in bits (window = 1 << a3, length field
 * 16 - a3 bits). Flag bits are read MSB-first: 1 = 8-bit literal (to the
 * output and the window), 0 = window offset (0 ends) + length, copying
 * length + 3 bytes. Returns the source end (rounded up to even) in a0 and the
 * destination end in a1. Non-ABI: clobbers s0/s4-s7 (func_802C4070 saves
 * them). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C41C0.s")

/* Bit reader for func_802C41C0: reads bits MSB-first into v0 while the mask
 * a3 shifts right; reader state lives in a0/t3/t4 (s4 = 0x80). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C42CC.s")

/* Save-everything sound wrapper: resets D_803F7840 to -1, starts the looping
 * effect D_80367738 via func_80260650 (sndPlaySfx) with its handle stored in
 * D_803F7844, then func_802C4584 with s5 = 0. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4310.s")

/* Save-everything sound wrapper: stops the sound handles a0 (on entry) and
 * D_803F7848 via func_802608C8 when they are non-null. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C444C.s")

/* Save-everything sound wrapper: non-ABI input s5 (a signed level). Stores
 * |s5| in D_803F7840 and, when it changed (or D_80364AB0 was set, which it
 * clears), sets the D_803F7844 sound's parameter 0x10 (likely pitch) to
 * 0.5 + |s5| * (D_8030D940, or D_8030D944 when D_80364456 == 1) via
 * func_80260AB8. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4584.s")

/* Save-everything sound wrapper: flag = D_80370C1A | D_80370C1B. When it
 * differs from D_803F784C, stores it; on starts the second loop
 * (func_80260650(D_80367738, .., &D_803F7848)) if not already playing, off
 * stops it (func_802608C8) and clears D_803F7848. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4724.s")
