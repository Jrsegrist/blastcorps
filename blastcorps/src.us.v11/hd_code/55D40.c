#include "common.h"
#include <ultra64.h>
#include "game/game.h"

extern s32 D_803A6B20;
extern s32 D_803A6B24;

void func_8029A500(void) {
    D_803A6B20 = -0xef;
    D_803A6B24 = 0;
}
typedef struct {
    u32 flags;
    u8 color;
    u8 size;
    char str[42];
} CreditLine;

extern CreditLine D_80304A90[];
extern u8 D_802F47B0[][8];
extern s32 D_80358060;
extern u8 D_803643D6;
extern s16 yoshiState;


/* Credits scroller: draws the visible credit lines, advances the scroll, and ends the credits */
Gfx *func_8029A518(Gfx **gfxp, Gfx *gfx) {
    Gfx *ret = gfx;
    CreditLine *line;
    s32 i;
    s32 x;
    s32 y = 0;
    s32 color;

    for (i = 0; i < 0x52; i++) {
        line = &D_80304A90[i];
        if (line->flags & 1) {
            color = 0xA0;
        } else if (line->flags & 4) {
            color = 0x6A;
        } else if (line->flags & 8) {
            color = 0xD5;
        } else {
            color = 0;
        }
        if (y - D_803A6B20 >= -0x31 && y - D_803A6B20 < 0xF0) {
            func_80259DC8(gfxp, line->str, 0, 0, color, x, y - D_803A6B20, line->size, line->size, 1,
                          D_802F47B0[line->color][0], D_802F47B0[line->color][1], D_802F47B0[line->color][2],
                          D_802F47B0[line->color][3], D_802F47B0[line->color][4], D_802F47B0[line->color][5],
                          D_802F47B0[line->color][6], D_802F47B0[line->color][7]);
        }
        if (line->flags & 0x20) {
            y += 0x26;
        }
        if (line->flags & 0x10) {
            y += 0x16;
        }
        if (line->flags & 0x40) {
            y += 0x11;
        }
    }
    if (D_80358060 == 100) {
        func_8026AF6C(0x8036);
    }
    if ((u32) D_80358060 >= 0x18C) {
        D_803A6B20++;
    }
    if (D_803643D6) {
        D_803A6B24++;
    }
    if (D_803A6B24 == 0x7D) {
        func_8026AF6C(0x8037);
        func_80260EE0(0x25);
    }
    if (D_803A6B24 >= 0x7E && yoshiState == 1 && func_802753C0() == 0) {
        func_80275270(0x200000000000, 0.75f);
    }
    return ret;
}
