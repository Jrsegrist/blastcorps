#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6F00.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6F6C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A6FE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7070.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A70D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A71DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A746C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A754C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A75DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A768C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7764.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A77D0.s")

/* func_802A7834: two sequential no-arg calls (`func_802A7E70();
 * func_802A785C();`) using the 8-byte addiu-sp/sd-ra/ld-ra frame instead
 * of the 24-byte o32 shadow frame plain C produces for any real call -
 * same confirmed-unreachable-from-C frame family as func_802AC284 in
 * hd_code/679E0.c (see that file's comment for the probe evidence).
 * Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7834.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A785C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7A1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7AAC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7B3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7C28.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7CB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7D68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7E70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A7FD8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8314.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A83B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A843C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8590.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A860C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8768.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8B10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8CCC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8FB4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A8FF4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9038.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A90E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9164.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A92C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A93B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A94A4.s")

/* func_802A9514: clamps $s3 to a max of 0x240 (`if (s3 >= 0x241) s3 =
 * 0x240;`), reading AND writing $s3 directly with no parameter or
 * return value involved at all - same non-ABI register-threading
 * character as the func_802A06B4 family in hd_code/56040.c, here used
 * as a persistent "hot" value shared across functions via a dedicated
 * callee-saved register instead of a global variable. Not expressible
 * as a normal C function. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9514.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9540.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A95A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9710.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A992C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9A60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9B1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9CAC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9DC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802A9F24.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA094.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA2E4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA460.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA5E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA6D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA764.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA838.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AA890.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AABE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AACD4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAD0C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAE54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AAF64.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB1B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB234.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB2B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB33C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB3C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB41C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB478.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB50C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB670.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB714.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB878.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB8D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AB9A4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABB1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABBEC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABC88.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABCDC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABD54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABEDC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802ABFC8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/62740/func_802AC0BC.s")
