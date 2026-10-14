#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gYoshiHeapItems
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gLevelInfo ((LevelInfo *) gLevelInfo)
#define players ((Player *) players)
#ifdef NON_MATCHING
#define gYoshiHeapItems (*(MenuItem * N64P *) &gYoshiHeapItems)
#endif
/* end of views */

/*
 * bestTimes.c (assert file name at 0x8020FF70): the best-times screen, a
 * menu of 4 rows per vehicle slot (one per player) built from the save records.
 */

int sprintf(char *, const char *, ...);
void bcopy(const void *, void *, int);

#define BT_ASSERT(EX, line) \
    if (!(EX)) debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "bestTimes.c", line)

/* pfsHandler.c's message to the save thread: command, argument, player, reply wanted */
#define PAK_MSG(cmd, arg, pn, reply) ((cmd) | ((arg) << 8) | ((pn) << 16) | ((reply) << 24))

typedef struct {
    u8 name[0x10];
    u32 unk10;         /* 0x10: bit per best-time slot */
    u8 pad14[4];
    u8 rank[0x3C];     /* 0x18 */
    u8 pad54[0x91 - 0x54];
    u8 unk91;
    u8 timeSlot[0x3C]; /* 0x92 */
    u8 padCE[0x100 - 0xCE];
} Player;

typedef struct {
    u8 type; /* 0x80: special level */
    u8 pad[0x43];
} LevelInfo; /* 0x44 */

typedef struct {
    u16 flags;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    u8 padA[2];
    char * N64P text;  /* 0x0C */
    void * N64P unk10; /* 0x10 */
    u8 unk14;
    u8 pad15;
    u16 unk16;   /* 0x16: best time */
    u8 unk18;    /* 0x18: player */
    u8 unk19;
    u8 unk1A;    /* 0x1A: level */
    u8 pad1B;
} MenuItem; /* 0x1C */

typedef struct {
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[6];
    s16 unk18;
} MenuHeader;

extern u32 gBestItemCount;        /* number of best-time menu items */
extern u8 gBestItemPlayer[];       /* player of each item */
extern u8 gBestRowSlot[];       /* level of each row */
#ifndef NON_MATCHING
extern MenuItem *gYoshiHeapItems;
#endif
extern char D_80219FD0[][0x20];
extern char D_8020D800[][4];


s32 fe_best_slot_shown(u8 lvl);
s32 fe_best_cmp(MenuItem *a, MenuItem *b);
s32 fe_best_player_listed(u8 pn);

/* The best-times menu: per vehicle slot, a row per player (0x801F7850).
 * build the best-times menu */
void fe_best_build_menu(void) {
    Player *p;
    MenuHeader *hdr;
    MenuItem *item;
    s32 lvl;
    s32 i;
    s32 n;
    char timeStr[0x20];
    char buf[0x20];

    p = &players[playerNumber];
    hdr = (MenuHeader *) (gYoshiWindows + 0x268);
    gYoshiHeapItems = (MenuItem *) gHeapPtr;
    gHeapPtr += 0x71C;
    for (i = 0; i < 4; i++) {
        if (gSlotState[i] == 1 && saveIt[i] == 0
            && ((players[i].rank[levelno] > 0 && players[i].rank[levelno] < 6) ? 1 : 0)) {
            osSendMesg(&gPakRequestQueue, (OSMesg) PAK_MSG(8, levelno, i, 1), OS_MESG_BLOCK);
            osRecvMesg(&pakToGameMessageQ, NULL, OS_MESG_BLOCK);
        } else {
            fe_best_clear_times(i);
        }
    }
    lvl = 0;
    n = 0;
    for (; lvl < 16; lvl++) {
        if (fe_best_slot_shown(lvl) && gBestTimes[playerNumberAtStart][D_802E8C44[lvl]] > 0) {
            for (i = 0; i < 4; i++) {
                item = &gYoshiHeapItems[n * 4 + i];
                p = &players[i];
                if (gSlotState[i] == 1 && gBestTimes[i][D_802E8C44[lvl]] > 0
                    && (gLevelInfo[levelno].type != 0x80 || p->unk91 >= 11)) {
                    hud_format_time(timeStr, gBestTimes[i][D_802E8C44[lvl]], 0);
                    sprintf(D_80219FD0[n * 4 + i], "%-7.7s %s", p, timeStr);
                    item->text = D_80219FD0[n * 4 + i];
                    item->unk10 = 0;
                    item->unk14 = fe_stats_grade_time(gBestTimes[i][D_802E8C44[lvl]], levelno, players[i].unk91) % 5 + 0x12;
                    item->unk18 = i;
                } else {
                    item->text = 0;
                    item->unk10 = 0;
                    item->unk14 = 0;
                    item->unk18 = 4;
                }
                item->flags = 0x1400;
                item->unk2 = 0x24;
                item->unk6 = 0x10;
                item->unk8 = 0x11;
                item->unk16 = gBestTimes[i][D_802E8C44[lvl]];
                item->unk1A = lvl;
            }
            text_shell_sort((u8 *) &gYoshiHeapItems[n * 4], 4, 0x1C, (s32 (*)(void *, void *)) fe_best_cmp);
            for (i = 0; i < 4; i++) {
                item = &gYoshiHeapItems[n * 4 + i];
                item->unk4 = i * 17;
                if (i == 2) {
                    item->flags |= 1;
                }
                if (item->text != 0) {
                    bcopy(item->text, buf, font_strlen((u8 *) item->text) + 1);
                    sprintf(item->text, "%s %s", D_8020D800[i], buf);
                }
            }
            n++;
        }
    }
    text_shell_sort((u8 *) gYoshiHeapItems, n, 0x70, (s32 (*)(void *, void *)) fe_best_cmp);
    for (lvl = 0; lvl < n * 4; lvl++) {
        item = &gYoshiHeapItems[lvl];
        gBestItemPlayer[lvl] = item->unk18;
        if (!(lvl & 3)) {
            gBestRowSlot[lvl / 4] = item->unk1A;
        }
        item->unk16 = 0x1E;
        item->unk4 += lvl / 4 * 100;
    }
    if (gLevelInfo[levelno].type == 0x80) {
        item = &gYoshiHeapItems[lvl];
        item->text = 0;
        item->unk10 = 0;
        item->flags = 0x400;
        item->unk2 = -0x20;
        item->unk4 = 0x28;
        item->unk14 = 0xD;
        item->unk16 = item->unk1A = 0;
        hdr->unk10 = n * 4 + 1;
    } else {
        hdr->unk10 = n * 4;
    }
    hdr->unk18 = 2;
    gBestItemCount = n * 4;
    fe_best_color_items();
    fe_load_vehicle_icons();
}

/* A vehicle slot the player knows (special levels: slot 0) (0x801F7F74).
 * whether best-time slot `lvl` is shown (special levels: only slot 0) */
s32 fe_best_slot_shown(u8 lvl) {
    if (gLevelInfo[levelno].type == 0x80) {
        return lvl == 0;
    }
    return (players[playerNumberAtStart].unk10 & (1 << lvl)) ? 1 : 0;
}

/* Sort: items with text first, then by time (0x801F7FF4).
 * qsort comparator: items with text first, then by time */
s32 fe_best_cmp(MenuItem *a, MenuItem *b) {
    if (a->text != 0 && b->text != 0) {
        return a->unk16 - b->unk16;
    }
    if (a->text != 0) {
        return -1;
    }
    return 1;
}

/* Z/R: highlights the next player with entries (0x801F803C).
 * cycle the highlighted player on a button press */
void fe_best_next_player(void) {
    s32 next;
    s32 i;

    if ((gContButtons & 0x2010) && !(gContPrevButtons & 0x2010)) {
        next = (playerNumber + 1) % 4;
        i = 0;
        for (; i < 4 && (gSlotState[next % 4] != 1 || fe_best_player_listed(next % 4) == 0); i++, next = (next + 1) % 4) {
        }
        if (playerNumber != next) {
            func_80260650(D_80367738, 0x1D, 0);
            playerNumber = next;
            fe_player_ticker_set_text(playerNumber, 1);
            fe_best_color_items();
        } else {
            func_80260650(D_80367738, 0xD0, 0);
        }
    }
}

/* Whether the player has entries (0x801F81B4).
 * whether player `pn` has an item in the menu */
s32 fe_best_player_listed(u8 pn) {
    u32 n;
    u32 i;

    i = 0;
    n = gBestItemCount;
    for (; i < n && gBestItemPlayer[i] != pn; i++) {
    }
    return i != n;
}

/* The highlight and colours by player (0x801F8228).
 * highlight and colour the items by player */
void fe_best_color_items(void) {
    MenuItem *p;
    u32 i;

    for (i = 0; i < gBestItemCount; i++) {
        p = &gYoshiHeapItems[i];
        if (gBestItemPlayer[i] == playerNumber) {
            p->flags |= 4;
        } else {
            p->flags &= ~4;
        }
        if (gBestItemPlayer[i] == playerNumberAtStart) {
            p->unk18 = p->unk19 = 6;
        } else if (gBestItemPlayer[i] == playerNumber) {
            p->unk18 = p->unk19 = 2;
        } else {
            p->unk18 = p->unk19 = 7;
        }
    }
}

/* Zeroes a player's times unless the level is done (0x801F8354).
 * clear player `pn`'s best times unless the current level is ranked */
void fe_best_clear_times(u8 pn) {
    s32 i;

    BT_ASSERT(frontEndPresent, 279);
    if (!((players[pn].rank[levelno] > 0 && players[pn].rank[levelno] < 6) ? 1 : 0)) {
        for (i = 0; i < 16; i++) {
            gBestTimes[pn][D_802E8C44[i]] = 0;
        }
    }
}

/* The current row's vehicle icon (0x801F8440).
 * draw the current row's level icon */
Gfx *fe_best_draw_icon(s32 arg0, Gfx *gfx) {
    Gfx *g;
    s16 off;

    off = 0;
    g = gfx;
    if (gLevelInfo[levelno].type != 0x80) {
        g = spr_begin(g);
        g = spr_draw(g, gBestRowSlot[((u16 *) gYoshiWindows)[0x280 / 2] / 4] + gVehicleIconTex, 0x18 - off, 100, 0xFF - off * 2, 1, 1.0f);
        g = spr_end(g);
    }
    return g;
}
