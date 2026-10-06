#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* A rectangle in a per-level zone list (coordinates are world units >> 5). */
typedef struct {
    /* 0x00 */ s16 minX;
    /* 0x02 */ s16 minZ;
    /* 0x04 */ s16 maxX;
    /* 0x06 */ s16 maxZ;
    /* 0x08 */ s16 height;
} HeightZone; /* size 0xA */

#define HEIGHT_ZONE_NONE 3000 /* sentinel height value */

extern s32 D_803643E0; /* player x */
extern s32 D_803643E8; /* player z */
extern u8 D_80364411;
extern s16 D_8036444E;
extern u16 D_80364450;

/* `level` is the loaded level header (D_80358074). Its words at 0x40/0x44 are
 * the start/end offsets of a HeightZone list. Finds the highest zone that
 * contains the player (inclusive bounds, starting from -1). A result of
 * HEIGHT_ZONE_NONE sets D_80364411; otherwise D_8036444E = height * 32 +
 * D_80364450 and D_80364411 is cleared. The asm loops with `!=`, so the end
 * offset must be start + n * 10. */
void func_802A5510(u8 *level) {
    HeightZone *zone = (HeightZone *) (level + *(s32 *) (level + 0x40));
    HeightZone *end = (HeightZone *) (level + *(s32 *) (level + 0x44));
    /* logical shifts (srl) in the asm, then signed compares */
    s32 px = (u32) D_803643E0 >> 5;
    s32 pz = (u32) D_803643E8 >> 5;
    s32 best = -1;

    for (; zone != end; zone++) {
        if (px < zone->minX || pz < zone->minZ || zone->maxX < px || zone->maxZ < pz) {
            continue;
        }
        if (zone->height >= best) {
            best = zone->height;
        }
    }

    if (best == HEIGHT_ZONE_NONE) {
        D_80364411 = 1;
    } else {
        D_8036444E = (best << 5) + D_80364450;
        D_80364411 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60D50/func_802A5510.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF2EC; /* x */
extern s32 D_803EF2F4; /* z */
extern s32 D_803EF304; /* result height (world units) */

/* Like func_802A5510, for the point (D_803EF2EC, D_803EF2F4) and the
 * HeightZone list at level+0x44 / +0x48: D_803EF304 = (highest containing
 * zone's height, or -1) << 5. The end offset must be start + n * 10.
 * Asm callers rely on preserved: func_802B8D04 keeps a2, a3, t7, f12, f14. */
void func_802A5604(u8 *level) {
    HeightZone *zone = (HeightZone *) (level + *(s32 *) (level + 0x44));
    HeightZone *end = (HeightZone *) (level + *(s32 *) (level + 0x48));
    /* logical shifts (srl) in the asm, then signed compares */
    s32 px = (u32) D_803EF2EC >> 5;
    s32 pz = (u32) D_803EF2F4 >> 5;
    s32 best = -1;

    for (; zone != end; zone++) {
        if (px < zone->minX || pz < zone->minZ || zone->maxX < px || zone->maxZ < pz) {
            continue;
        }
        if (zone->height >= best) {
            best = zone->height;
        }
    }
    D_803EF304 = best << 5;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60D50/func_802A5604.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803BE73A;       /* current level index */
extern s16 D_80305B90[][3]; /* per-level {D_8036444C, D_80364450, return value} */
extern s16 D_8036444C;

/* Loads the per-level triple for level D_803BE73A: stores the first two
 * entries in D_8036444C / D_80364450 and returns the third (sign-extended). */
s32 func_802A56C4(void) {
    s16 *entry = D_80305B90[D_803BE73A];

    D_8036444C = entry[0];
    D_80364450 = entry[1];
    return entry[2];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60D50/func_802A56C4.s")
#endif
