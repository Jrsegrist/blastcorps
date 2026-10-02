#include "common.h"
#include <ultra64.h>

extern s32 D_80222840;
extern s32 D_802229E0;
extern u8 *D_802229F0;
extern s32 D_802229F4;
extern s32 D_80222A08;
extern s32 D_80222A0C;
extern s32 D_80222A18;
extern s32 D_80222A1C;
extern s32 D_80222A20;

void func_802206D0(void);
s32  func_8022043C(void);
extern s32 inflate(void);


void func_80220360(s32 *arg0, s32 *arg1, s32 arg2) {
    D_802229F0 = *arg0;
    D_802229F4 = *arg1;
    D_802229E0 = arg2;

    func_802206D0();

    if (*(D_802229F0 + D_80222A1C) != 0x1F) {
        D_80222A1C += 1;
    }
    D_80222840 = func_8022043C();
    if (D_80222840 >= 0) {
        inflate();
        *arg0 += D_80222A1C;
        *arg1 += D_80222A20;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/init/1660/func_8022043C.s")

/* TODO: reverse_bits - reverses the low `arg1` bits of `arg0` (classic
 * bit-reversal, used when emitting/reading canonical Huffman codes MSB-first
 * from an LSB-first bitstream). Fully matched down to a single reordered
 * instruction pair: this form produces byte-identical registers and operand
 * order to the target for every instruction except the arg0-shift (srl) and
 * the accumulator-shift (sll), which the target schedules in the opposite
 * order (sll immediately after the `or`, srl afterward) despite arg0's
 * update statement sitting textually between them below - every statement
 * order/compound-operator/register-hint permutation tried still scheduled
 * srl before sll here. Needs real IDO instruction-scheduler knowledge, not
 * more C-level reordering, to close:
 *
 * u32 reverse_bits(u32 arg0, s32 arg1) {
 *     register u32 phi_a2 = 0;
 *
 *     do {
 *         phi_a2 |= arg0 & 1;
 *         arg0 = arg0 >> 1;
 *         phi_a2 <<= 1;
 *     } while (--arg1 > 0);
 *     return phi_a2 >> 1;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/1660/reverse_bits.s")

void func_802206D0(void) {
    D_80222A20 = 0;
    D_80222A1C = 0;
    D_80222A18 = 0;
    D_80222A0C = 0;
    D_80222A08 = 0;
}
