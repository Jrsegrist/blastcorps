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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDA10.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDAE8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDB70.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDC7C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDD74.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CDF94.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE0E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE204.s")

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. */
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
extern void *D_803A7408;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 D_803A7427;
extern u8 D_803A742F;
extern u8 D_80306450[];
s32 func_802A6F6C(void);
void func_802BCBD8(void);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Given a ring index (0..0xFFF), finds which end of the span
 * [D_803A7410, D_803A7412] is nearer in ring distance (|d|, folded as
 * 0xFFF - |d| above 0x800). Nearer to B: step 0x78 back from B (+0xFFF if
 * negative); otherwise step 0x78 on from A (-0xFFF if >= 0x1000). If that
 * candidate lies outside the span (for a wrapped span, B < A: strictly
 * between B and A; otherwise below A or above B), func_802A6F6C's value is
 * returned instead. The asm returns the full 32-bit value; the C callers
 * declare s16. */
s32 func_802CE3B8(s32 idx) {
    s32 a = D_803A7410;
    s32 b = D_803A7412;
    s32 da = a - idx;
    s32 db = b - idx;
    s32 v;

    if (da < 0) {
        da = -da;
    }
    if (da > 0x800) {
        da = 0xFFF - da;
    }
    if (db < 0) {
        db = -db;
    }
    if (db > 0x800) {
        db = 0xFFF - db;
    }
    if (db < da) {
        v = b - 0x78;
        if (v < 0) {
            v += 0xFFF;
        }
    } else {
        v = a + 0x78;
        if (v >= 0x1000) {
            v -= 0xFFF;
        }
    }
    if (b < a) {
        if (v < a && b < v) {
            v = func_802A6F6C();
        }
    } else {
        if (v < a || b < v) {
            v = func_802A6F6C();
        }
    }
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE3B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Starts a new span at position (x, y, z): the ring span becomes the empty
 * 0/0xFFF pair, the position goes to D_803A73F0/F4/F8, the flags
 * D_803A742F/7427/7424/7425 are cleared, D_803A7408 points at D_80306450,
 * then func_802BCBD8 is called. */
void func_802CE4F0(s32 x, s32 y, s32 z) {
    D_803A7410 = 0;
    D_803A7412 = 0xFFF;
    D_803A73F0 = x;
    D_803A73F4 = y;
    D_803A73F8 = z;
    D_803A742F = 0;
    D_803A7427 = 0;
    D_803A7408 = D_80306450;
    D_803A7424 = 0;
    D_803A7425 = 0;
    func_802BCBD8();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE4F0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE5BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE65C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/89250/func_802CE6F8.s")
