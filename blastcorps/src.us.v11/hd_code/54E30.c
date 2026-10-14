#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define players ((Player *) players)
/* end of views */

typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];  /* 0x18 */
    u8 pad54[0xEE - 0x54];
    u8 f0EE;        /* 0xEE */
    u8 padEF[0x100 - 0xEF];
} Player;

#ifndef NON_MATCHING
extern s32 D_80367738;
#endif
/* .bss, defined here (paired u64 stores share one lui) */
u64 gSeqDoneMode; /* loop mask used when the sequence finishes */
u64 gSeqAbortMode; /* loop mask used when it is aborted */
u8 gSeqDoneLevel;  /* level to go to when finished */
u8 gSeqAbortLevel;  /* level to go to when aborted */
u8 gSeqStartLevel;  /* level the sequence starts in */
u8 gSeqNumber;  /* current sequence number */
u8 D_803A6B04;


#define LEVEL_DONE(l) ((players[playerNumber].rank[l] > 0 && players[playerNumber].rank[l] < 6) ? 1 : 0)

/* "prepare sequence %d" (0x802995F0).
 * Set up level sequence seq: its start level, exit levels and loop masks.
 * gSeqStartLevel is read back through ugen's store cache here, so the rank
 * index is not folded into the address. */
void seq_prepare(s32 seq) {
    gSeqNumber = seq;
    debug_printf("prepare sequence %d\n", seq);
    switch (gSeqNumber) {
        case 1:
            gSeqStartLevel = 0x26;
            gSeqAbortLevel = 0x28;
            gSeqDoneLevel = 0x28;
            gSeqDoneMode = 0x4000;
            gSeqAbortMode = 0x4000;
            D_803A6B04 = 1;
            break;
        case 3:
            gSeqStartLevel = 0x26;
            gSeqAbortLevel = 0x2B;
            gSeqDoneLevel = 0x2B;
            gSeqDoneMode = 0x4000;
            gSeqAbortMode = 0x4000;
            D_803A6B04 = 1;
            break;
        case 0:
            gSeqStartLevel = 0x31;
            gSeqAbortLevel = 0x32;
            gSeqDoneLevel = 0x32;
            gSeqDoneMode = 0x4000;
            gSeqAbortMode = 0x4000;
            D_803A6B04 = 0;
            break;
        case 4:
            gSeqStartLevel = 0x2F;
            gSeqAbortLevel = 0;
            gSeqDoneLevel = 0;
            gSeqDoneMode = 0x4000;
            gSeqAbortMode = 0x4000;
            D_803A6B04 = 1;
            break;
        case 5:
            gSeqStartLevel = 0x37;
            gSeqAbortLevel = 0x37;
            gSeqDoneLevel = 0x37;
            if (LEVEL_DONE(gSeqStartLevel)) {
                gSeqDoneMode = 0x80;
                gSeqAbortMode = 0x80;
            } else {
                gSeqDoneMode = 0x2000;
                gSeqAbortMode = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 6:
            gSeqStartLevel = 0x1C;
            gSeqAbortLevel = 0x1C;
            gSeqDoneLevel = 0x1C;
            if (LEVEL_DONE(gSeqStartLevel)) {
                gSeqDoneMode = 0x80;
                gSeqAbortMode = 0x80;
            } else {
                gSeqDoneMode = 0x2000;
                gSeqAbortMode = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 7:
            gSeqStartLevel = 0x35;
            gSeqAbortLevel = 0x35;
            gSeqDoneLevel = 0x35;
            if (LEVEL_DONE(gSeqStartLevel)) {
                gSeqDoneMode = 0x80;
                gSeqAbortMode = 0x80;
            } else {
                gSeqDoneMode = 0x2000;
                gSeqAbortMode = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 8:
            gSeqStartLevel = 7;
            gSeqAbortLevel = 7;
            gSeqDoneLevel = 7;
            if (LEVEL_DONE(gSeqStartLevel)) {
                gSeqDoneMode = 0x80;
                gSeqAbortMode = 0x80;
            } else {
                gSeqDoneMode = 0x2000;
                gSeqAbortMode = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        case 9:
            gSeqStartLevel = 0x13;
            gSeqAbortLevel = 0x13;
            gSeqDoneLevel = 0x13;
            if (LEVEL_DONE(gSeqStartLevel)) {
                gSeqDoneMode = 0x80;
                gSeqAbortMode = 0x80;
            } else {
                gSeqDoneMode = 0x2000;
                gSeqAbortMode = 0x2000;
            }
            D_803A6B04 = 1;
            break;
        default:
            debug_printf("unknown sequence number %d\n", gSeqNumber);
            break;
    }
}

/* Enters the sequence's start level (0x80299C0C).
 * Enter the sequence's first level */
void seq_enter_start_level(void) {
    levelno = gSeqStartLevel;
}

/* Starts the sequence level (demos, credits ...) (0x80299C20).
 * Per-level sequence start actions (cutscenes, jingles) */
void seq_start_level(void) {
    switch (levelno) {
        case 47:
            D_802E8BEC = 0;
            D_802E8BF0 = 0;
            rec_start_playback(0, &levelno);
            break;
        case 55:
            D_802E8BEC = 9;
            D_802E8BF0 = 0;
            rec_start_playback(9, &levelno);
            D_803643D4 = 5;
            yoshi_demand(0x8041);
            hud_load_pad_image();
            break;
        case 28:
            D_802E8BEC = 10;
            D_802E8BF0 = 0;
            rec_start_playback(10, &levelno);
            D_803643D4 = 1;
            yoshi_demand(0x8042);
            hud_load_pad_image();
            break;
        case 53:
            D_802E8BEC = 11;
            D_802E8BF0 = 0;
            rec_start_playback(11, &levelno);
            D_803643D4 = 2;
            yoshi_demand(0x8043);
            hud_load_pad_image();
            break;
        case 7:
            D_802E8BEC = 12;
            D_802E8BF0 = 0;
            rec_start_playback(12, &levelno);
            D_803643D4 = 3;
            yoshi_demand(0x8044);
            hud_load_pad_image();
            break;
        case 19:
            D_802E8BEC = 13;
            D_802E8BF0 = 0;
            rec_start_playback(13, &levelno);
            D_803643D4 = 9;
            yoshi_demand(0x8045);
            hud_load_pad_image();
            break;
        case 49:
            credits_init();
            break;
        case 38:
            func_8025BB50();
            break;
    }
}

/* "finishing sequence and going to level" (0x80299E10).
 * Finish the current sequence level: record completion, pick the next level */
void seq_finish(s32 arg0) {
    func_802609D0();
    switch (levelno) {
        case 38:
        case 47:
        case 49:
            players[playerNumber].rank[levelno] = 5;
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
        gNextGameMode = gSeqDoneMode;
        levelno = gSeqDoneLevel;
    } else {
        gNextGameMode = gSeqAbortMode;
        levelno = gSeqAbortLevel;
    }
    debug_printf("finishing sequence and going to level %d\n", levelno);
}

/* "get loop done for world %d" (0x80299FE8).
 * Return the loop mask for a level, starting its sequence where it has one */
u64 seq_get_loop_done(u8 level) {
    u64 mask;

    switch (level) {
        case 38:
            mask = 0x100000000000;
            seq_prepare(1);
            break;
        case 55:
            mask = 0x100000000000;
            seq_prepare(5);
            break;
        case 28:
            mask = 0x100000000000;
            seq_prepare(6);
            break;
        case 53:
            mask = 0x100000000000;
            seq_prepare(7);
            break;
        case 7:
            mask = 0x100000000000;
            seq_prepare(8);
            break;
        case 19:
            mask = 0x100000000000;
            seq_prepare(9);
            break;
        default:
            mask = func_801ECA50(level);
            break;
    }
    debug_printf("get loop done for world %d\n", lowest_set_bit(mask));
    return mask;
}
