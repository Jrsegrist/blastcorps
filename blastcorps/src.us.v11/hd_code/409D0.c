#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285190.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_802852EC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285814.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285A78.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285AB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285B10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285B68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285CA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285CC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80285EF4.s")

/* TODO: func_80286038 - computes `(u16)arg0 * 6` via shift/subtract/shift
 * (classic strength reduction, avoiding an actual multiply instruction):
 * `andi a0,a0,0xffff; sll v0,a0,2; subu v0,v0,a0; jr ra; sll v0,v0,1`
 * (5 instructions). Closest found: `s32 func_80286038(u16 arg0) { return
 * ((arg0 << 2) - arg0) << 1; }` - compiles to the identical 4 instructions
 * plus one trailing `move v0,t7` (6 total), since the outer `<<1` on a
 * freshly-computed subu result gets a scratch register before moving to
 * v0 instead of writing v0 directly. Tried: u32 param + explicit `arg0 &=
 * 0xffff` statement (adds a leading move instead), a named intermediate
 * (register and plain, both added a real stack frame). Needs the right
 * phrasing for IDO to treat the final shift as computing directly into
 * $v0 the way it does for the `subu` immediately before it. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80286038.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_8028604C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/409D0/func_80286090.s")
