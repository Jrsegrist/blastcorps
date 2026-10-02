#include "common.h"

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A800.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A914.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AA10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AB88.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B02C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B514.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B5B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B614.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B7CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B930.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B994.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BB28.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BD0C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BEE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BF64.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C0DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C160.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C284.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C354.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C454.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C52C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C5EC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C6E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C748.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C828.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C914.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C9D4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CB04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CD54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CFA4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D040.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D120.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D1D4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D210.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D24C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D534.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D56C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D90C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DA90.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DB7C.s")

/* TODO: func_8029DBF0 - wrapper around func_8029DC14, but that callee
 * reads its real input out of $v0 (set via a delay-slot `or v0,a0,zero`
 * right before the `jal`, not through the normal a0-a3 argument
 * registers) and returns its result in $v1 instead of $v0 - a hand-tuned
 * non-ABI register convention between these two specific functions, not
 * expressible as a normal C function call. Logic (looking at
 * func_8029DC14 directly): search a fixed-stride struct array starting
 * at D_803B9890 up to D_803BD300's current pointer value for a byte field
 * matching the caller's input, returning found/not-found. Needs either
 * inline asm or leaving as GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DBF0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC14.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DCD4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DD54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DDC8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DE50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DEA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DF78.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E0AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E21C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E47C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E4E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E558.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E5AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E730.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E878.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E938.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EA48.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EB58.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EC68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EDEC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EF80.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F060.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F110.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F1BC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F3D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F4B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F560.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F608.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F6B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F760.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F85C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F9D4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FC74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FF2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FFA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0118.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0290.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A02E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0320.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0360.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A039C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A03D4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A040C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0444.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0480.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A04BC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0508.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0540.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0570.s")

/* func_802A05A4/func_802A05D0/func_802A05F8/func_802A0620/func_802A0648
 * (and likely more below): all call func_802A06B4 with no visible
 * arguments, then immediately use BOTH $v0 and $v1 from the return.
 * func_802A06B4 itself reads $v0 as an INPUT (compares it against a
 * table it walks via $v1, with no instruction anywhere setting $v0
 * first) and returns the matching entry's address in both $v0 and $v1 -
 * meaning these wrappers don't actually get two distinct values back,
 * they're each transparently forwarding a search key that some much
 * earlier caller stuffed into $v0, unchanged, through every layer in
 * between. Classic hand-tuned non-ABI register threading for a hot
 * dispatch-table lookup, not expressible as a normal C function call at
 * any layer - would need either inline asm or a hand-maintained
 * register-correct wrapper, not a straight decomp. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0620.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0648.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0674.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A06B4.s")
