#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gSfxBank
#define LEGACY_D_8036EA60
#define LEGACY_nss
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gYoshiFeItems ((MenuItem *) gYoshiFeItems)
#define gLevelInfo ((LevelInfo *) gLevelInfo)
#define gYoshiGameItems ((MenuItem *) gYoshiGameItems)
#define gYoshiWindows ((MenuItem *) gYoshiWindows)
#define players ((PlayerRec *) players)
#define D_8036EA80 ((u32 *) &D_8036EA80)
#define D_8036EA90 ((u32 *) &D_8036EA90)
#ifdef NON_MATCHING
#define D_8036EA60 (*(u32 *) &D_8036EA60)
#define nss (*(u32 *) &nss)
#endif
/* end of views */

/* stats_perm.c: per-level results (score, time, counts and the best/saved
 * copies), the end-of-level status screen and the status save to the pak. */

/* Per-player save record, 0x100 bytes, indexed by level */
typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];          /* 0x18 per level: 1..5 once completed */
    u8 flags[0x100 - 0x54]; /* 0x54 per level */
} PlayerRec;

typedef struct {
    u8 type; /* 1: no status save */
    u8 pad[0x43];
} LevelInfo; /* 0x44 */

/* A text/menu item */
typedef struct {
    u16 flags;
    u8 pad[6];
    u32 f8;
    s16 fC;
    u8 pad2[0xA];
    s16 f18;
    u8 pad3[2];
} MenuItem; /* 0x1C */

extern u8 D_80364B08[][0x100]; /* = players[p].rank */
extern u8 D_80364B44[][0x100]; /* = players[p].flags */

/* This level's results (nss) and the best (D_8036EA60), 16 bytes each:
 * money, time, then three counts with their targets in gRduCount..93 */
#ifndef NON_MATCHING
extern u32 D_8036EA60;
#endif
extern u8 D_8036EA68;
extern u8 D_8036EA69;
extern u16 D_8036EA6C;
#ifndef NON_MATCHING
extern u32 nss;
#endif
extern s32 D_8036EA74;
extern u8 D_8036EA7A;
extern u8 D_8036EA7B;
extern u8 gBonusEarned[]; /* bonus earned, per bonus */
extern u8 gBonusEarnedPrev[];

#ifndef NON_MATCHING
extern s32 gSfxBank;
#endif

#define CUR (players[playerNumber])
#define LEVEL_DONE(l) ((CUR.rank[l] > 0 && CUR.rank[l] < 6) ? 1 : 0)

#define LEVEL_SAVE_SIZE 64
#define STATS_ASSERT(EX, line) \
    if (!(EX)) { debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "stats_perm.c", line); }

/* Loads the counts, resets the bonuses (0x80285190).
 * Start-of-level: load this level's previous counts and reset the bonuses */
void stats_level_start(void) {
    s32 i;

    D_8036EA7B = players[playerNumber].flags[levelno + 0x3E];
    D_8036EA74 = gBestTimes[playerNumber][D_802E8C44[(gLevelInfo[levelno].type == 1) ? 1 : D_8036EA7B]];
    D_8036EA7A = D_80364B08[playerNumber][levelno] % 8;
    for (i = 0; i < 4; i++) {
        gBonusEarned[i] = 0;
    }
    gTargetsCleared = LEVEL_DONE(levelno);
}

/* The status screen's lines; returns the average percentage (0x802852EC).
 * Print the status screen lines, highlight improved results, return the
 * average completion percentage */
u32 stats_print_status(void) {
    u32 pctA;
    u32 pctB;
    u32 pctC;
    u32 i;
    s32 f;
    s32 f2;
    s32 f1;
    char buf[0x20];

    f2 = 0;
    f1 = 0;
    if (D_8036EB92) {
        pctA = (D_8036EA78 * 100) / D_8036EB92;
    } else {
        pctA = 100;
    }
    if (D_8036EB93) {
        pctB = (D_8036EA79 * 100) / D_8036EB93;
    } else {
        pctB = 100;
    }
    if (gRduCount) {
        pctC = (D_8036EA7C * 100) / gRduCount;
    } else {
        pctC = 100;
    }
    sprintf(D_8036B9A8, "***%2d (%d%c)*", D_8036EA78, pctA, '%');
    sprintf(D_8036B9A8 + 0x20, "***$%d*", nss);
    sprintf(D_8036B9A8 + 0x40, "***%2d (%d%c)*", D_8036EA79, pctB, '%');
    sprintf(D_8036B9A8 + 0x60, "***%2d (%d%c)*", D_8036EA7C, pctC, '%');
    hud_format_time(buf, D_8036EA74, 0);
    sprintf(D_8036B9A8 + 0x80, "***%s*", buf);
    for (i = 18; i < 23; i++) {
        gYoshiGameItems[i].flags = 0x400;
    }
    for (i = 14; i < 18; i++) {
        gYoshiFeItems[i].flags = 0x400;
    }
    if (gNextGameMode == 0x40) {
        f2 = 0x100;
    }
    if (gGameMode & 0x30C) {
        f1 = 0x100;
    }
    if (D_8036EA78 > D_8036EA68) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    gYoshiFeItems[14].flags |= f | 0x80;
    gYoshiGameItems[18].flags |= f | f2 | 0x80;
    if (nss > D_8036EA60) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    gYoshiFeItems[15].flags |= f | 0x80;
    gYoshiGameItems[19].flags |= f | f2 | 0x80;
    if (D_8036EA79 > D_8036EA69) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    gYoshiFeItems[16].flags |= f | 0x80;
    gYoshiGameItems[20].flags |= f | f2 | 0x80;
    if (D_8036EA7C > D_8036EA6C) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    gYoshiFeItems[17].flags |= f | 0x80;
    gYoshiGameItems[21].flags |= f | f2 | 0x80;
    gYoshiGameItems[22].flags |= f2 | 0x80;
    if (gUseEeprom) {
        return (pctA + pctC) / 2;
    } else {
        return (pctA + pctB + pctC) / 3;
    }
}

/* "Creating status ...": the level status to the save (0x80285814).
 * End-of-level: write the status to the pak if this level saves one, then
 * copy the results to the best/saved slots. Returns 1 if a status was written. */
u8 stats_save_status(void) {
    u8 saved = 0;
    u8 coin;

    STATS_ASSERT(frontEndPresent, 140);
    frontEndPresent = 1;
    hd_mode_setup();
    if (gGameMode == MODE_WORLD) {
        osRecvMesg(&pakToGameMessageQ, NULL, OS_MESG_BLOCK);
    }
    if (LEVEL_DONE(levelno)) {
        if (gLevelInfo[levelno].type == 1) {
            if (pakBuffer[0] == 0x1234567887654321) {
                hd_setup_level(NULL);
                debug_printf("Creating status ...\n");
                coin = D_80364B08[playerNumber][levelno];
                if (coin == 5) {
                    coin = 4;
                }
                STATS_ASSERT(create_status(pakBuffer,coin)<=LEVEL_SAVE_SIZE-4, 161);
                apply_status(pakBuffer);
                obj_tally(0);
                people_init((s32) pakBuffer);
                saved = 1;
            } else {
                hd_setup_level((s32) pakBuffer);
            }
        } else {
            hd_setup_level(NULL);
        }
    } else {
        hd_setup_level(NULL);
    }
    stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
    stats_copy_result((u8 *) &nss, (u8 *) D_8036EA80);
    stats_copy_result((u8 *) &nss, (u8 *) D_8036EA90);
    return saved;
}

/* Copies a 16-byte Score (0x80285A78).
 * Copy a 16-byte results record */
void stats_copy_result(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}

/* Sets a per-level flag bit (0x80285AB0).
 * Set flag `bit` (1-based) for the current level. The shift is 32 or more:
 * MIPS sllv uses its low 5 bits (bit 1 -> 1 << 0); C leaves it undefined,
 * so the port masks it (PORT_SHAMT). */
void stats_set_level_flag(u8 bit) {
    D_80364A87 |= 2;
    CUR.flags[levelno] |= 1 << PORT_SHAMT(bit + 0x1F);
}

/* Tests a per-level flag bit (0x80285B10).
 * Test flag `bit` (1-based) for the current level */
s32 stats_test_level_flag(u8 bit) {
    s32 unused;

    return (D_80364B44[playerNumber][levelno] & (1 << PORT_SHAMT(bit + 0x1F))) ? 1 : 0;
}

/* A non-mission level done: fade to the world screen (0x80285B68).
 * Level just completed by the active player: mark it and fade out */
void stats_finish_bonus_level(s32 arg0) {
    s32 unused;

    if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
        if (LEVEL_DONE(levelno)) {
            if (gLevelInfo[levelno].type != 1 && playerNumber == playerNumberAtStart) {
                commpt_save_flags();
                gPauseRequest = 1;
                fade_to_mode_speed(0x4000, 1.25f);
                saveIt[playerNumber] = levelno + 1;
                D_8036EB99 = 1;
            }
        }
    }
}

/* Shows the communication-point hint window (hint 0x47) (0x80285CA0). */
void stats_show_commpt_hint(void) {
    yoshi_show_hint(0x47);
}

/* The per-count bonus jingles (0x80285CC0).
 * Award each bonus once its count reaches the target, with a jingle */
void stats_award_bonuses(void) {
    s32 sfx = -1;
    s32 snd = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        gBonusEarnedPrev[i] = gBonusEarned[i];
        if (gBonusEarned[i] == 0) {
            switch (i) {
                case 0:
                    if (gBonusEarned[i] = (D_8036EA7C == gRduCount)) {
                        sfx = 0x39;
                    }
                    break;
                case 1:
                    if (gBonusEarned[i] = (D_8036EA79 == D_8036EB93)) {
                        sfx = 0x3A;
                    }
                    break;
                case 2:
                    obj_tally(0);
                    if (gBonusEarned[i] = (D_8036EA78 == D_8036EB92)) {
                        sfx = 0x3B;
                    }
                    break;
                case 3:
                    if (gBonusEarned[i] = (gBonusEarned[0] && gBonusEarned[2] && gBonusEarned[1])) {
                        sfx = 0x3C;
                        snd = 0xC6;
                    }
                    break;
            }
        }
    }
    if (D_80358064) {
        if (sfx != -1 && !(yoshi_get_demand() & 0x8000)) {
            yoshi_demand(sfx | 0x8000);
        }
        if (snd) {
            snd_play(gSfxBank, snd, 0);
        }
    }
}

/* Elapsed time, status screen, pause (0x80285EF4).
 * Level finished at frame `start`: build the status screen with the elapsed
 * time and save the results */
void stats_finish_level(s32 start) {
    s32 t;

    t = stats_frames_to_tenths(((s32) D_803156C0) - start);
    obj_tally(0);
    D_8036EA74 += t;
    stats_print_status();
    D_8036EA74 -= t;
    stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
    gYoshiGameItems[24].flags &= ~1;
    gYoshiGameItems[24].flags |= 0x800;
    gYoshiGameItems[23].flags &= ~1;
    gYoshiGameItems[23].flags |= 0x800;
    gYoshiGameItems[17].flags |= 1;
    gYoshiGameItems[17].flags &= ~0x800;
    gYoshiWindows[6].f18 = 0x11;
    gYoshiWindows[6].f8 |= 0x80;
    yoshi_demand(0x8006);
    gYoshiWindows[6].fC = 0;
    gPauseRequest = 1;
}

/* Tenths of a second to frames (0x80286038).
 * Time units (1/10 s) to frames */
s32 stats_tenths_to_frames(u16 arg0) {
    return arg0 * 6;
}

/* Frames to tenths of a second (capped at 59999) (0x8028604C).
 * Frames to time units (1/10 s), capped at 59999 */
u16 stats_frames_to_tenths(u32 frames) {
    s32 unused;

    return (frames / 6 >= 60000) ? 59999 : frames / 6;
}

/* Has the current player completed the level (0x80286090)?
 * Has the current player completed `level`? */
u8 stats_level_done(s32 level) {
    s32 unused;

    return LEVEL_DONE(level);
}
