#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F38 ((D_802E8F38_s *) D_802E8F38)
#define players ((Player *) players)
/* end of views */

/* Per-player state, 0x100 bytes */
typedef struct {
    /* 0x00 */ u8 unk0[0x90];
    /* 0x90 */ u8 unk90;
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 unk92[0x6E];
} Player;

/* 8 bytes */
typedef struct {
    /* 0x00 */ u8 id;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 z;
} D_802E8F38_s;

extern u8 D_8039CAB8;
extern Mtx * N64P D_8039CAC4;
extern u8 gAcademyBits;


/* Places the level's academy marker (0x80297530).
 * K&R in the matching build (hd.c passes an int). */
#ifdef NON_MATCHING
void academy_place_marker(u8 arg0)
#else
void academy_place_marker(arg0)
    u8 arg0;
#endif
{
    u8 found;
    u8 idx;

    found = 0;
    if (gNextGameMode == 0x2000) {
        found = academy_find_level(arg0, &idx);
    } else {
        found = 0;
    }
    gAcademyBits = players[playerNumber].unk90;
    if (found) {
        gAcademyMarkerOn = 1;
        D_8039CAB0 = D_802E8F38[idx].x;
        D_8039CAB2 = D_802E8F38[idx].y;
        D_8039CAB4 = D_802E8F38[idx].z;
        D_8039CAB6 = idx;
        D_8039CAB8 = 0;
        D_8039CAC4 = (Mtx *) gHeapPtr;
        gHeapPtr = (u8 *) gHeapPtr + 0x80;
        guTranslate(D_8039CAC4, 0, 0, 0);
        guTranslate(D_8039CAC4 + 1, 0, 0, 0);
    } else {
        gAcademyMarkerOn = 0;
    }
}

/* The level's entry in the academy table (0x8029766C). */
u8 academy_find_level(u8 arg0, u8 *arg1) {
    u8 found;
    s32 i;

    found = 0;
    i = 0;
    do {
        if (D_802E8F38[i].id == arg0) {
            found = 1;
        } else {
            i++;
        }
    } while (!found && i < 6);
    if (arg1 != NULL) {
        *arg1 = i;
    }
    return found;
}

/* Draws the academy marker (0x802976E8). */
void academy_draw_marker(Gfx **arg0) {
    Gfx *gdl = *arg0;

    if (gAcademyMarkerOn) {
        gSPSegment(gdl++, 6, osVirtualToPhysical(D_8039CAC0));
        gSPSegment(gdl++, 7, osVirtualToPhysical(D_8039CAC4));
        gSPClearGeometryMode(gdl++, -1);
        gSPDisplayList(gdl++, osVirtualToPhysical(D_8039CABC));
        gDPPipeSync(gdl++);
    }
    *arg0 = gdl;
}

/* Within 0x50: the bit set, mode switch (0x80297804). */
void academy_check_marker(s32 x, s32 y, s32 z) {
    if (gAcademyMarkerOn && players[playerNumber].unk91 < 5) {
        if (D_8039CAB8) {
            if (util_dist3d(x >> 5, y >> 5, z >> 5, D_8039CAB0, D_8039CAB2, D_8039CAB4) >= 0x8D) {
                D_8039CAB8 = 0;
            }
        } else if (util_dist3d(x >> 5, y >> 5, z >> 5, D_8039CAB0, D_8039CAB2, D_8039CAB4) < 0x50) {
            D_8039CAB8 = 1;
            gAcademyBits |= 1 << D_8039CAB6;
            gNextGameMode = 0x1000000000LL;
        }
    }
}

/* The markers into players[].academy bits (0x80297960). */
void academy_save_markers(void) {
    if (gDebugCheats && (gContButtons & 0x2000) && gAcademyMarkerOn) {
        gAcademyBits |= 1 << D_8039CAB6;
    }
    players[playerNumber].unk90 = gAcademyBits;
}
