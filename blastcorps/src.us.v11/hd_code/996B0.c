#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DDE70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DDEC4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DDF44.s")

void func_802DDFF8(void) {
}

/* TODO: func_802DE000 - clamp: `s32 diff = *(s32*)(arg0+0x24) - arg1; if
 * (diff < 0) return 0x3e8; return diff;` (target: 9 instructions, no
 * stack frame, two independent `jr ra` exits each with their own delay
 * slot - `v0` preloaded with 0x3e8 before the branch, so the taken path
 * needs no further work, and the fallthrough path overwrites v0 with the
 * cached diff (`v1`) in its own delay slot). Every phrasing tried (a
 * register local for the diff, pre-loading a second register local for
 * the default return value, fully inlining the subtraction twice instead
 * of caching it, explicit if/else) produces either a real stack frame, an
 * unconditional `b` plus a shared single-exit epilogue, or (fully
 * inlining) a correctly frame-free but 10-instruction version that
 * recomputes the subtraction instead of reusing one cached value. Logic
 * confirmed correct throughout. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE000.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE024.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE0AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE1D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE280.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE2F0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE3CC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE444.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE48C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE500.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE570.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DE66C.s")

void func_802DEE84(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DEE8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DF4B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DF5B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/996B0/func_802DF624.s")
