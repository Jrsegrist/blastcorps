#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E2EC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E31C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E37C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E38A0.s")

void func_802E3F90(void *arg0, void *arg1) {
    *(s32 *) arg1 = *(s32 *) ((u8 *) arg0 + 8);
    *(s16 *) ((u8 *) arg1 + 0xc) = *(s16 *) ((u8 *) arg0 + 0x1a);
    *(s32 *) ((u8 *) arg1 + 4) = *(s32 *) ((u8 *) arg0 + 0xc);
}

void func_802E3FAC(void *arg0, void *arg1) {
    *(s32 *) ((u8 *) arg0 + 8) = *(s32 *) arg1;
    *(s16 *) ((u8 *) arg0 + 0x1a) = *(s16 *) ((u8 *) arg1 + 0xc);
    *(s32 *) ((u8 *) arg0 + 0xc) = *(s32 *) ((u8 *) arg1 + 4);
}

s32 func_802E3FC8(void *arg0) {
    return *(s32 *) ((u8 *) arg0 + 0xc);
}

void func_802E3FD0(void) {
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E3FD8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4024.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E41A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E42C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E43AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4400.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E4458.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E44A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E44D8.s")

/* TODO: func_802E45B0 - stores arg2 into a struct field: `*(s16 *)
 * ((u8 *) arg1 + 0x16) = arg2;` (target: 4 instructions). Logic is
 * certainly right (confirmed via m2c-style reading of the disassembly),
 * but the target also spills `arg0` and `arg2` to their standard o32
 * argument-shadow stack slots (sp+0, sp+8) even though arg0 is otherwise
 * unused and arg2 is also used directly from its register - the same
 * "unused/redundant incoming-argument gets a stack home anyway" pattern
 * seen elsewhere in this project (e.g. func_80272C40 in init), but it
 * doesn't reproduce here from the obvious 3-statement form, and forcing
 * it via explicit `s32 sp0 = arg0;`-style locals instead makes IDO
 * allocate a brand-new frame (worse). Needs the right phrasing to get
 * the dead a0/a2 stores without an actual sp adjustment.
 *
 * void func_802E45B0(s32 arg0, void *arg1, s32 arg2) {
 *     *(s16 *) ((u8 *) arg1 + 0x16) = arg2;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E45B0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/9E700/func_802E45C0.s")
