#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F38 ((D_802E8F38_s *) D_802E8F38)
#define D_80364AF0 ((Player *) D_80364AF0)
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
extern Mtx *D_8039CAC4;
extern u8 D_8039CAC8;


/* K&R in the matching build (hd.c passes an int). */
#ifdef NON_MATCHING
void func_80297530(u8 arg0)
#else
void func_80297530(arg0)
    u8 arg0;
#endif
{
    u8 found;
    u8 idx;

    found = 0;
    if (D_80364A98 == 0x2000) {
        found = func_8029766C(arg0, &idx);
    } else {
        found = 0;
    }
    D_8039CAC8 = D_80364AF0[D_80364AE8].unk90;
    if (found) {
        D_8039CAB7 = 1;
        D_8039CAB0 = D_802E8F38[idx].x;
        D_8039CAB2 = D_802E8F38[idx].y;
        D_8039CAB4 = D_802E8F38[idx].z;
        D_8039CAB6 = idx;
        D_8039CAB8 = 0;
        D_8039CAC4 = (Mtx *) D_80358070;
        D_80358070 = (u8 *) D_80358070 + 0x80;
        guTranslate(D_8039CAC4, 0, 0, 0);
        guTranslate(D_8039CAC4 + 1, 0, 0, 0);
    } else {
        D_8039CAB7 = 0;
    }
}

u8 func_8029766C(u8 arg0, u8 *arg1) {
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

void func_802976E8(Gfx **arg0) {
    Gfx *gdl = *arg0;

    if (D_8039CAB7) {
        gSPSegment(gdl++, 6, osVirtualToPhysical(D_8039CAC0));
        gSPSegment(gdl++, 7, osVirtualToPhysical(D_8039CAC4));
        gSPClearGeometryMode(gdl++, -1);
        gSPDisplayList(gdl++, osVirtualToPhysical(D_8039CABC));
        gDPPipeSync(gdl++);
    }
    *arg0 = gdl;
}

void func_80297804(s32 x, s32 y, s32 z) {
    if (D_8039CAB7 && D_80364AF0[D_80364AE8].unk91 < 5) {
        if (D_8039CAB8) {
            if (func_8026A6F0(x >> 5, y >> 5, z >> 5, D_8039CAB0, D_8039CAB2, D_8039CAB4) >= 0x8D) {
                D_8039CAB8 = 0;
            }
        } else if (func_8026A6F0(x >> 5, y >> 5, z >> 5, D_8039CAB0, D_8039CAB2, D_8039CAB4) < 0x50) {
            D_8039CAB8 = 1;
            D_8039CAC8 |= 1 << D_8039CAB6;
            D_80364A98 = 0x1000000000LL;
        }
    }
}

void func_80297960(void) {
    if (D_802FA268 && (D_80370C28 & 0x2000) && D_8039CAB7) {
        D_8039CAC8 |= 1 << D_8039CAB6;
    }
    D_80364AF0[D_80364AE8].unk90 = D_8039CAC8;
}
