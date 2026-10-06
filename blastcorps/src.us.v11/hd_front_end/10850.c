#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_8036BB24
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_80364AF0 ((Player *) D_80364AF0)
#ifdef NON_MATCHING
#define D_8036BB24 (*(MenuItem * *) &D_8036BB24)
#endif
/* end of views */

/*
 * bestTimes.c (assert file name at 0x8020FF70): the best-times screen, a
 * menu of 4 rows per level (one per player) built from the save records.
 */

int sprintf(char *, const char *, ...);
void bcopy(const void *, void *, int);

#define BT_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "bestTimes.c", line)

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
    char *text;  /* 0x0C */
    void *unk10; /* 0x10 */
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

extern u32 D_8021A828;        /* number of best-time menu items */
extern u8 D_8021A7E8[];       /* player of each item */
extern u8 D_8021A7D0[];       /* level of each row */
#ifndef NON_MATCHING
extern MenuItem *D_8036BB24;
#endif
extern char D_80219FD0[][0x20];
extern char D_8020D800[][4];

#define playerNumberAtStart D_80364AEA
#define frontEndPresent D_80370C50

s32 func_801F7F74(u8 lvl);
s32 func_801F7FF4(MenuItem *a, MenuItem *b);
s32 func_801F81B4(u8 pn);

/* build the best-times menu */
void func_801F7850(void) {
    Player *p;
    MenuHeader *hdr;
    MenuItem *item;
    s32 lvl;
    s32 i;
    s32 n;
    char timeStr[0x20];
    char buf[0x20];

    p = &D_80364AF0[D_80364AE8];
    hdr = (MenuHeader *) (D_802F8BDC + 0x268);
    D_8036BB24 = (MenuItem *) D_80358070;
    D_80358070 += 0x71C;
    for (i = 0; i < 4; i++) {
        if (D_80365060[i] == 1 && D_8039C53C[i] == 0
            && ((D_80364AF0[i].rank[D_802E8BDC] > 0 && D_80364AF0[i].rank[D_802E8BDC] < 6) ? 1 : 0)) {
            osSendMesg(&D_80219EF8, (OSMesg) PAK_MSG(8, D_802E8BDC, i, 1), OS_MESG_BLOCK);
            osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
        } else {
            func_801F8354(i);
        }
    }
    lvl = 0;
    n = 0;
    for (; lvl < 16; lvl++) {
        if (func_801F7F74(lvl) && D_80364EF0[playerNumberAtStart][D_802E8C44[lvl]] > 0) {
            for (i = 0; i < 4; i++) {
                item = &D_8036BB24[n * 4 + i];
                p = &D_80364AF0[i];
                if (D_80365060[i] == 1 && D_80364EF0[i][D_802E8C44[lvl]] > 0
                    && (D_802E8F94[D_802E8BDC].type != 0x80 || p->unk91 >= 11)) {
                    func_80264A34(timeStr, D_80364EF0[i][D_802E8C44[lvl]], 0);
                    sprintf(D_80219FD0[n * 4 + i], "%-7.7s %s", p, timeStr);
                    item->text = D_80219FD0[n * 4 + i];
                    item->unk10 = 0;
                    item->unk14 = func_801EF2BC(D_80364EF0[i][D_802E8C44[lvl]], D_802E8BDC, D_80364AF0[i].unk91) % 5 + 0x12;
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
                item->unk16 = D_80364EF0[i][D_802E8C44[lvl]];
                item->unk1A = lvl;
            }
            func_802595E0((u8 *) &D_8036BB24[n * 4], 4, 0x1C, (s32 (*)(void *, void *)) func_801F7FF4);
            for (i = 0; i < 4; i++) {
                item = &D_8036BB24[n * 4 + i];
                item->unk4 = i * 17;
                if (i == 2) {
                    item->flags |= 1;
                }
                if (item->text != 0) {
                    bcopy(item->text, buf, func_8025B300((u8 *) item->text) + 1);
                    sprintf(item->text, "%s %s", D_8020D800[i], buf);
                }
            }
            n++;
        }
    }
    func_802595E0((u8 *) D_8036BB24, n, 0x70, (s32 (*)(void *, void *)) func_801F7FF4);
    for (lvl = 0; lvl < n * 4; lvl++) {
        item = &D_8036BB24[lvl];
        D_8021A7E8[lvl] = item->unk18;
        if (!(lvl & 3)) {
            D_8021A7D0[lvl / 4] = item->unk1A;
        }
        item->unk16 = 0x1E;
        item->unk4 += lvl / 4 * 100;
    }
    if (D_802E8F94[D_802E8BDC].type == 0x80) {
        item = &D_8036BB24[lvl];
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
    D_8021A828 = n * 4;
    func_801F8228();
    func_801FDE50();
}

/* whether best-time slot `lvl` is shown (special levels: only slot 0) */
s32 func_801F7F74(u8 lvl) {
    if (D_802E8F94[D_802E8BDC].type == 0x80) {
        return lvl == 0;
    }
    return (D_80364AF0[playerNumberAtStart].unk10 & (1 << lvl)) ? 1 : 0;
}

/* qsort comparator: items with text first, then by time */
s32 func_801F7FF4(MenuItem *a, MenuItem *b) {
    if (a->text != 0 && b->text != 0) {
        return a->unk16 - b->unk16;
    }
    if (a->text != 0) {
        return -1;
    }
    return 1;
}

/* cycle the highlighted player on a button press */
void func_801F803C(void) {
    s32 next;
    s32 i;

    if ((D_80370C28 & 0x2010) && !(D_80370C2A & 0x2010)) {
        next = (D_80364AE8 + 1) % 4;
        i = 0;
        for (; i < 4 && (D_80365060[next % 4] != 1 || func_801F81B4(next % 4) == 0); i++, next = (next + 1) % 4) {
        }
        if (D_80364AE8 != next) {
            func_80260650(D_80367738, 0x1D, 0);
            D_80364AE8 = next;
            func_801E8EB8(D_80364AE8, 1);
            func_801F8228();
        } else {
            func_80260650(D_80367738, 0xD0, 0);
        }
    }
}

/* whether player `pn` has an item in the menu */
s32 func_801F81B4(u8 pn) {
    u32 n;
    u32 i;

    i = 0;
    n = D_8021A828;
    for (; i < n && D_8021A7E8[i] != pn; i++) {
    }
    return i != n;
}

/* highlight and colour the items by player */
void func_801F8228(void) {
    MenuItem *p;
    u32 i;

    for (i = 0; i < D_8021A828; i++) {
        p = &D_8036BB24[i];
        if (D_8021A7E8[i] == D_80364AE8) {
            p->flags |= 4;
        } else {
            p->flags &= ~4;
        }
        if (D_8021A7E8[i] == playerNumberAtStart) {
            p->unk18 = p->unk19 = 6;
        } else if (D_8021A7E8[i] == D_80364AE8) {
            p->unk18 = p->unk19 = 2;
        } else {
            p->unk18 = p->unk19 = 7;
        }
    }
}

/* clear player `pn`'s best times unless the current level is ranked */
void func_801F8354(u8 pn) {
    s32 i;

    BT_ASSERT(frontEndPresent, 279);
    if (!((D_80364AF0[pn].rank[D_802E8BDC] > 0 && D_80364AF0[pn].rank[D_802E8BDC] < 6) ? 1 : 0)) {
        for (i = 0; i < 16; i++) {
            D_80364EF0[pn][D_802E8C44[i]] = 0;
        }
    }
}

/* draw the current row's level icon */
Gfx *func_801F8440(s32 arg0, Gfx *gfx) {
    Gfx *g;
    s16 off;

    off = 0;
    g = gfx;
    if (D_802E8F94[D_802E8BDC].type != 0x80) {
        g = func_80274868(g);
        g = func_80272ED8(g, D_8021A7D0[((u16 *) D_802F8BDC)[0x280 / 2] / 4] + D_8021A8F0, 0x18 - off, 100, 0xFF - off * 2, 1, 1.0f);
        g = func_80274AA4(g);
    }
    return g;
}
