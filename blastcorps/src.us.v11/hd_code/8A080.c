#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): marks all 25 entries
 * of a 0x14-byte-stride table starting at D_803FB8B8 with -1. */
extern u8 D_803FB8B8[];

void func_802CE840(void) {
    s32 i;

    for (i = 0; i < 25; i++) {
        *(s32 *) (D_803FB8B8 + i * 0x14) = -1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE840.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE880.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE90C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE958.s")

/* func_802CE9A4: `D_803FB8B0 = &D_803F9330;` wrapped in the same dead
 * `addiu sp,sp,-8`/`sd $ra`/`ld $ra`/`addiu sp,sp,8` frame as
 * func_802BBE10/func_802C8AF0 - confirmed hand-written by the same
 * probe evidence (no calls, no locals, nothing that could need a frame;
 * IDO never generates this shape on its own). Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9C8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CEA68.s")
