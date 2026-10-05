#include "common.h"
#include <ultra64.h>

/* func_802A1320: `mfc0 $v0, $12` (read COP0 Status register) wrapped in a
 * dead $ra save/restore frame - same hand-written COP0-leaf-stub character
 * as __osGetSR in init/2330.c. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
u32 __osGetSR(void);

/* Returns the COP0 Status register (the asm is a bare `mfc0 $v0, $12`);
 * the port's platform layer supplies __osGetSR. */
u32 func_802A1320(void) {
    return __osGetSR();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1320.s")
#endif

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A133C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1388.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1558.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1674.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* An entry of the table at 0x80306480 (stride 0x30), ended by an entry
 * whose `end` byte is -1. The asm builds the address as a raw lui/addiu. */
typedef struct {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ u8 pad0C[8];
    /* 0x14 */ u8 radius;
    /* 0x15 */ s8 end;
    /* 0x16 */ s16 box[12]; /* four corners (x, y, z) with x/z = centre -/+ radius */
    /* 0x2E */ u8 pad2E[2];
} BoxEntry; /* size 0x30 */

#define D_80306480 ((BoxEntry *) 0x80306480)

/* For each entry of the table at 0x80306480 converts the centre (x, y, z)
 * >> 5 (arithmetic) to the four corner points of a square of half-size
 * `radius`: (x-r, y, z-r), (x+r, y, z-r), (x-r, y, z+r) at 0x16/0x1C/...,
 * i.e. box = {x-r, y, z-r, x+r, y, z-r, x-r, y, z+r, x+r, y, z+r}.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14 (the asm
 * saves v0, v1, a0-a3). */
void func_802A1934(void) {
    BoxEntry *e;

    for (e = D_80306480; e->end != -1; e++) {
        s32 x = e->x >> 5;
        s32 y = e->y >> 5;
        s32 z = e->z >> 5;
        s32 r = e->radius;

        e->box[0] = x - r;
        e->box[1] = y;
        e->box[2] = z - r;
        e->box[3] = x + r;
        e->box[4] = y;
        e->box[5] = z - r;
        e->box[6] = x - r;
        e->box[7] = y;
        e->box[8] = z + r;
        e->box[9] = x + r;
        e->box[10] = y;
        e->box[11] = z + r;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1934.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A19F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1A9C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C20.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1C88.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1D54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_802E8BDC; /* current level */
extern u8 *D_803BE704;
extern u8 *D_803BE708;
extern u8 D_802D30D0[], D_802D3194[], D_802D32A0[], D_802D331C[], D_802D33C8[];
extern u8 D_802D3444[], D_802D3538[], D_802D3614[], D_802D36C0[], D_802D3784[];
extern u8 D_802D3890[], D_802D393C[], D_802D3A00[], D_802D3A4C[], D_802D3BA0[];
extern u8 D_802D3BD4[], D_802D3CE0[], D_802D3DD4[], D_802D3EB0[], D_802D3F74[];

/* Picks the per-level data block for the current level D_802E8BDC:
 * D_803BE708 = block, D_803BE704 = block + 4; both NULL for levels without
 * one.
 * Asm callers rely on preserved: func_802A1D54 keeps t0, f12, f14 (the asm
 * saves t2). */
void func_802A1EC8(void) {
    u8 *p;

    switch (D_802E8BDC) {
        case 0x00: p = D_802D30D0; break;
        case 0x01: p = D_802D3194; break;
        case 0x02: p = D_802D32A0; break;
        case 0x03: p = D_802D331C; break;
        case 0x04: p = D_802D33C8; break;
        case 0x05: p = D_802D3444; break;
        case 0x09: p = D_802D3538; break;
        case 0x0A: p = D_802D3614; break;
        case 0x0C: p = D_802D36C0; break;
        case 0x0D: p = D_802D3784; break;
        case 0x0E: p = D_802D3890; break;
        case 0x0F: p = D_802D393C; break;
        case 0x10: p = D_802D3A00; break;
        case 0x11: p = D_802D3A4C; break;
        case 0x12: p = D_802D3BA0; break;
        case 0x1A: p = D_802D3BD4; break;
        case 0x1D: p = D_802D3CE0; break;
        case 0x21: p = D_802D3DD4; break;
        case 0x39: p = D_802D3EB0; break;
        case 0x3A: p = D_802D3F74; break;
        default:
            D_803BE704 = NULL;
            D_803BE708 = NULL;
            return;
    }
    D_803BE708 = p;
    D_803BE704 = p + 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A1EC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A20F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2164.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A21AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A23E0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2458.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A24BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2608.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A26A8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2A98.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2BB0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2C54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A2D68.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3008.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A303C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A30DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3134.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3198.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A32CC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A350C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3824.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A396C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3D54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3DF8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3E9C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A3F80.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4168.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A41B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A4464.s")

/* TODO: func_802A44E4 - round up to a multiple of 8 if not already a
 * multiple of 4: `if ((arg1 & 3) != 0) arg1 = (arg1 & ~7) + 8; return
 * arg1;` (target: 11 instructions, a `nop` in the branch's delay slot -
 * $a1 is the real parameter, with an unused leading parameter shadowing
 * $a0). Every type tried for the unused first parameter (void*, s32,
 * f32) makes IDO spill it into the delay slot anyway (`sw`/`swc1` at
 * 0(sp), no actual stack frame reserved for it) - the inverse of the
 * usual "unused argument still gets a dead stack home" pattern seen
 * elsewhere (func_80272C40 in init/26570.c): there the target WANTS the
 * dead spill and plain C gives it one; here the target doesn't want it
 * and plain C can't be talked out of it. Logic confirmed correct. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5CB60/func_802A44E4.s")
