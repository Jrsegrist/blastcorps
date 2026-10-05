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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802B9C50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Looks up the current
 * level in a table of {level, x, z} records terminated by a negative level
 * and sets the spawn position (x, z in 1/32 units) from the matching record;
 * position stays 0 if the level isn't listed. The asm also saves/restores
 * v0, v1, a0; its asm caller func_802B9C50 relies on a1, a3, f12, f14 being
 * preserved (a mixed N64 build would need a thunk; the native port won't). */
typedef struct {
    s16 level;
    u16 x;
    u16 z;
} LevelPos;
extern s32 D_802E8BDC; /* current level */
extern LevelPos D_80305D74[];
extern s32 D_803EF6E8;
extern s32 D_803EF6EC;
extern u8 D_803EF710;
extern u8 D_803EF711;

void func_802BA074(void) {
    LevelPos *e;
    s32 level;

    D_803EF6E8 = 0;
    D_803EF710 = 0;
    D_803EF6EC = 0;
    D_803EF711 = 0;
    level = D_802E8BDC;
    for (e = D_80305D74; e->level >= 0; e++) {
        if (e->level == level) {
            D_803EF6E8 = e->x << 5;
            D_803EF6EC = e->z << 5;
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA074.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA104.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA148.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA354.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA5A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA638.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA6AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA91C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA9A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BABEC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle "set timers"
 * leaf, the shape shared by func_802BC578, func_802C9B30, func_802CB5D8,
 * func_802CC8B8, func_802CD9AC, func_802D0784 and func_802D249C (only the
 * scale and the two byte values differ). Asm caller func_802BA354 relies on
 * a0-a3 being preserved (mixed N64 build would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802BAD24(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD24.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD80.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB054.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB170.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB1A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB230.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB274.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB4C0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB868.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB8B8.s")
