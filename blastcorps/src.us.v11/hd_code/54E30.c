#include "common.h"
#include <ultra64.h>
#include "game/game.h"

typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];  /* 0x18 */
    u8 pad54[0xEE - 0x54];
    u8 f0EE;        /* 0xEE */
    u8 padEF[0x100 - 0xEF];
} Player;

extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern u64 D_80364A98;
extern s32 D_802E8BDC;
extern s32 D_802E8BEC;
extern u8 D_802E8BF0;
extern u8 D_803643D4;
extern s32 D_80367738;
/* .bss, defined here (paired u64 stores share one lui) */
u64 D_803A6AF0; /* loop mask used when the sequence finishes */
u64 D_803A6AF8; /* loop mask used when it is aborted */
u8 D_803A6B00;  /* level to go to when finished */
u8 D_803A6B01;  /* level to go to when aborted */
u8 D_803A6B02;  /* level the sequence starts in */
u8 D_803A6B03;  /* current sequence number */
u8 D_803A6B04;


#define players D_80364AF0
#define playerNumber D_80364AE8
#define LEVEL_DONE(l) ((players[playerNumber].rank[l] > 0 && players[playerNumber].rank[l] < 6) ? 1 : 0)

/* Set up level sequence seq: its start level, exit levels and loop masks.
 * D_803A6B02 is read back through ugen's store cache here, so the rank
 * index is not folded into the address. */
void func_802995F0(s32 seq) {
    D_803A6B03 = seq;
    func_8029A7E4("prepare sequence %d\n", seq);
    switch (D_803A6B03) {
        case 1:
            D_803A6B02 = 0x26;
            D_803A6B01 = 0x28;
            D_803A6B00 = 0x28;
            D_803A6AF0 = 0x4000;
            D_803A6AF8 = 0x4000;
            D_803A6B04 = 1;
            break;
        case 3:
            D_803A6B02 = 0x26;
            D_803A6B01 = 0x2B;
            D_803A6B00 = 0x2B;
            D_803A6AF0 = 0x4000;
            D_803A6AF8 = 0x4000;
            D_803A6B04 = 1;
            break;
        case 0:
            D_803A6B02 = 0x31;
            D_803A6B01 = 0x32;
            D_803A6B00 = 0x32;
            D_803A6AF0 = 0x4000;
            D_803A6AF8 = 0x4000;
            D_803A6B04 = 0;
            break;
        case 4:
            D_803A6B02 = 0x2F;
            D_803A6B01 = 0;
            D_803A6B00 = 0;
            D_803A6AF0 = 0x4000;
            D_803A6AF8 = 0x4000;
            D_803A6B04 = 1;
            break;
        case 5:
            D_803A6B02 = 0x37;
            D_803A6B01 = 0x37;
            D_803A6B00 = 0x37;
            if (LEVEL_DONE(D_803A6B02)) {
                D_803A6AF0 = 0x80;
                D_803A6AF8 = 0x80;
            } else {
                D_803A6AF0 = 0x2000;
                D_803A6AF8 = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 6:
            D_803A6B02 = 0x1C;
            D_803A6B01 = 0x1C;
            D_803A6B00 = 0x1C;
            if (LEVEL_DONE(D_803A6B02)) {
                D_803A6AF0 = 0x80;
                D_803A6AF8 = 0x80;
            } else {
                D_803A6AF0 = 0x2000;
                D_803A6AF8 = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 7:
            D_803A6B02 = 0x35;
            D_803A6B01 = 0x35;
            D_803A6B00 = 0x35;
            if (LEVEL_DONE(D_803A6B02)) {
                D_803A6AF0 = 0x80;
                D_803A6AF8 = 0x80;
            } else {
                D_803A6AF0 = 0x2000;
                D_803A6AF8 = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 8:
            D_803A6B02 = 7;
            D_803A6B01 = 7;
            D_803A6B00 = 7;
            if (LEVEL_DONE(D_803A6B02)) {
                D_803A6AF0 = 0x80;
                D_803A6AF8 = 0x80;
            } else {
                D_803A6AF0 = 0x2000;
                D_803A6AF8 = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 9:
            D_803A6B02 = 0x13;
            D_803A6B01 = 0x13;
            D_803A6B00 = 0x13;
            if (LEVEL_DONE(D_803A6B02)) {
                D_803A6AF0 = 0x80;
                D_803A6AF8 = 0x80;
            } else {
                D_803A6AF0 = 0x2000;
                D_803A6AF8 = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        default:
            func_8029A7E4("unknown sequence number %d\n", D_803A6B03);
            break;
    }
}

/* Enter the sequence's first level */
void func_80299C0C(void) {
    D_802E8BDC = D_803A6B02;
}

/* Per-level sequence start actions (cutscenes, jingles) */
void func_80299C20(void) {
    switch (D_802E8BDC) {
        case 47:
            D_802E8BEC = 0;
            D_802E8BF0 = 0;
            func_8025B9D0(0, &D_802E8BDC);
            break;
        case 55:
            D_802E8BEC = 9;
            D_802E8BF0 = 0;
            func_8025B9D0(9, &D_802E8BDC);
            D_803643D4 = 5;
            func_8026AF6C(0x8041);
            func_80295E50();
            break;
        case 28:
            D_802E8BEC = 10;
            D_802E8BF0 = 0;
            func_8025B9D0(10, &D_802E8BDC);
            D_803643D4 = 1;
            func_8026AF6C(0x8042);
            func_80295E50();
            break;
        case 53:
            D_802E8BEC = 11;
            D_802E8BF0 = 0;
            func_8025B9D0(11, &D_802E8BDC);
            D_803643D4 = 2;
            func_8026AF6C(0x8043);
            func_80295E50();
            break;
        case 7:
            D_802E8BEC = 12;
            D_802E8BF0 = 0;
            func_8025B9D0(12, &D_802E8BDC);
            D_803643D4 = 3;
            func_8026AF6C(0x8044);
            func_80295E50();
            break;
        case 19:
            D_802E8BEC = 13;
            D_802E8BF0 = 0;
            func_8025B9D0(13, &D_802E8BDC);
            D_803643D4 = 9;
            func_8026AF6C(0x8045);
            func_80295E50();
            break;
        case 49:
            func_8029A500();
            break;
        case 38:
            func_8025BB50();
            break;
    }
}

/* Finish the current sequence level: record completion, pick the next level */
void func_80299E10(s32 arg0) {
    func_802609D0();
    switch (D_802E8BDC) {
        case 38:
        case 47:
        case 49:
            players[playerNumber].rank[D_802E8BDC] = 5;
            break;
        case 55:
            players[playerNumber].f0EE |= 1;
            break;
        case 28:
            players[playerNumber].f0EE |= 2;
            break;
        case 53:
            players[playerNumber].f0EE |= 4;
            break;
        case 7:
            players[playerNumber].f0EE |= 8;
            break;
        case 19:
            players[playerNumber].f0EE |= 0x10;
            break;
    }
    if (arg0) {
        func_80260650(D_80367738, 0x1E, 0);
        D_80364A98 = D_803A6AF0;
        D_802E8BDC = D_803A6B00;
    } else {
        D_80364A98 = D_803A6AF8;
        D_802E8BDC = D_803A6B01;
    }
    func_8029A7E4("finishing sequence and going to level %d\n", D_802E8BDC);
}

/* Return the loop mask for a level, starting its sequence where it has one */
u64 func_80299FE8(u8 level) {
    u64 mask;

    switch (level) {
        case 38:
            mask = 0x100000000000;
            func_802995F0(1);
            break;
        case 55:
            mask = 0x100000000000;
            func_802995F0(5);
            break;
        case 28:
            mask = 0x100000000000;
            func_802995F0(6);
            break;
        case 53:
            mask = 0x100000000000;
            func_802995F0(7);
            break;
        case 7:
            mask = 0x100000000000;
            func_802995F0(8);
            break;
        case 19:
            mask = 0x100000000000;
            func_802995F0(9);
            break;
        default:
            mask = func_801ECA50(level);
            break;
    }
    func_8029A7E4("get loop done for world %d\n", func_8026F92C(mask));
    return mask;
}
