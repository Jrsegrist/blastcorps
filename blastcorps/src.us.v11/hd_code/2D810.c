#include "common.h"
#include <ultra64.h>

/* The file after sched.c. Its .rodata (two pairs of doubles at
 * 0x8030C4D0) and .bss (0x8036BFC0) each start on a fresh 16-byte
 * boundary, which is what left the 8 zero bytes after sched.c's jump
 * table. The .bss is placed by hd_code_bss.us.v11.ld. */
f32 D_8036BFC0;
u8 D_8036BFC4;
u8 D_8036BFC5;
f32 D_8036BFC8;
f32 D_8036BFCC;
f32 D_8036BFD0;

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2D810/func_80271FD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2D810/func_802729F0.s")

void func_80272C40(s32 arg0) {
}
