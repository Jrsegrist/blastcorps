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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEAA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Vtx D_803FBAB0[]; /* 8 vertices per box */
extern u8 D_803FC1F0;    /* current box index */

/* Writes the 8 corner positions of the box x-10..x+10, y..y+20, z-10..z+20
 * into the vertices of box D_803FC1F0 (only ob[] is written). The asm takes
 * x, y, z in t4, t5, t6 (conventions.txt) and saves everything it uses; its
 * asm caller func_802CEAA0 keeps a1-a3, t1-t3, f12 and f14 live across the
 * call (a mixed N64 build would need a thunk, the native port doesn't). */
void func_802CEE14(s32 x, s32 y, s32 z) {
    Vtx *v = &D_803FBAB0[D_803FC1F0 * 8];
    s32 x0 = x - 10;
    s32 x1 = x + 10;
    s32 y1 = y + 20;
    s32 z0 = z - 10;
    s32 z1 = z + 20;

    v[0].v.ob[0] = x0, v[0].v.ob[1] = y, v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x0, v[1].v.ob[1] = y1, v[1].v.ob[2] = z0;
    v[2].v.ob[0] = x1, v[2].v.ob[1] = y, v[2].v.ob[2] = z0;
    v[3].v.ob[0] = x1, v[3].v.ob[1] = y1, v[3].v.ob[2] = z0;
    v[4].v.ob[0] = x0, v[4].v.ob[1] = y, v[4].v.ob[2] = z1;
    v[5].v.ob[0] = x0, v[5].v.ob[1] = y1, v[5].v.ob[2] = z1;
    v[6].v.ob[0] = x1, v[6].v.ob[1] = y, v[6].v.ob[2] = z1;
    v[7].v.ob[0] = x1, v[7].v.ob[1] = y1, v[7].v.ob[2] = z1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEE14.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEEFC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF1A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF3E0.s")

#ifdef NON_MATCHING
/* D_803FBBB0: table of 0x14-byte entries, D_803FC1F0 of them in use. Only the
 * halfword at +0x10 (a bit/flag state, see 409D0's func_80285AB0/func_80285B10)
 * is touched here. */
typedef struct {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u8 pad12[2];
} UnkEntry8A2E0; /* size 0x14 */

extern UnkEntry8A2E0 D_803FBBB0[];
extern u8 D_803FC1F0;

void func_80285AB0(u8 bit);
s32 func_80285B10(u8 bit);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* For each of the D_803FC1F0 entries whose +0x10 flag is set, call
 * func_80285AB0. Quirk kept from the asm: the argument is the countdown value
 * (D_803FC1F0 for entry 0, down to 1 for the last entry), not the index. */
void func_802CF5B0(void) {
    UnkEntry8A2E0 *e = D_803FBBB0;
    u8 n;

    for (n = D_803FC1F0; n != 0; n--, e++) {
        if (e->unk10 != 0) {
            func_80285AB0(n);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF5B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Refresh every entry's +0x10 flag from func_80285B10, passing the same
 * countdown value as func_802CF5B0 (stored as a halfword). */
void func_802CF628(void) {
    UnkEntry8A2E0 *e = D_803FBBB0;
    u8 n;

    for (n = D_803FC1F0; n != 0; n--, e++) {
        e->unk10 = func_80285B10(n);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF628.s")
#endif
