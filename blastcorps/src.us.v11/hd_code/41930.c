#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gYoshiFeItems ((MenuItem *) gYoshiFeItems)
#define D_802E8F38 ((Entry8 *) D_802E8F38)
#define gLevelInfo ((LevelInfo *) gLevelInfo)
#define players ((Player *) players)
/* end of views */

/* academy.c: each player's progress state machine (players[].gameState):
 * which levels/messages unlock next, and the jingles/menus that go with it */

typedef struct {
    u8 pad0[8];
    u8 levelno;  /* 0x08 */
    u8 pad9;
    u16 f0A;     /* 0x0A */
    u8 f0C;      /* 0x0C */
    u8 padD[0x18 - 0xD];
    u8 rank[0x3C];  /* 0x18 */
    u8 flags[0x3C]; /* 0x54 */
    u8 academy;     /* 0x90 */
    u8 gameState;   /* 0x91 */
    u8 pad92[0x100 - 0x92];
} Player;

typedef struct {
    u8 type;
    u8 state;
    u8 pad[0x42];
} LevelInfo;

typedef struct {
    u8 a;
    u8 pad[7];
} Entry8;

typedef struct {
    char * N64P name;
    u8 pad[0x2C];
} Entry30;

typedef struct {
    u8 pad[0xC];
    char * N64P text;
    u8 pad2[0xC];
} MenuItem;

extern u8 D_802FDA60[];
extern u8 D_802FDA70[];
extern Entry30 D_8020D7E4[];
extern char D_8036EBA0[];


#define LEVEL_DONE(l) ((players[playerNumber].rank[l] > 0 && players[playerNumber].rank[l] < 6) ? 1 : 0)
#define ACADEMY_ASSERT_S(EX, text, line) \
    if (!(EX)) { debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", text, "academy.c", line); }
#define ACADEMY_ASSERT(EX, line) \
    if (!(EX)) { debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "academy.c", line); }

/* The window of the game state (0x802860F0).
 * Announce the current game state (menu, message, sound); state 4 names the
 * first level of the next uncompleted academy group */
void academy_announce_state(void) {
    u8 found;
    s32 lvl;
    s32 i;
    s32 any;

    if (players[playerNumber].gameState != 13 && players[playerNumber].gameState != 8 &&
        players[playerNumber].gameState != 1) {
        gNextGameMode = 0x800000000000;
        hd_mode_setup();
        fe_bg_load(D_802FDA60[players[playerNumber].gameState]);
        switch (players[playerNumber].gameState) {
            case 4:
                found = 0;
                for (lvl = 0; lvl < 60 && !found; lvl++) {
                    for (i = 0, any = 0; i < 6 && !any; i++) {
                        if (D_802E8F38[i].a == lvl) {
                            any = 1;
                            if (!(players[playerNumber].academy & (1 << i))) {
                                found = 1;
                            }
                        }
                    }
                }
                func_802D6A60(D_8036EBA0, "IN %s.", D_8020D7E4[lvl].name);
                gYoshiFeItems[82].text = D_8036EBA0;
                break;
            case 6:
                fe_player_save_pending();
                break;
        }
        yoshi_demand((players[playerNumber].gameState + 0x16) | 0x8000);
    }
}

/* The tune of the game state (0x802862DC).
 * Play the current game state's sound */
void academy_play_state_tune(void) {
    if (((s32) gFramesInMode) == 0) {
        music_play_tune(D_802FDA70[players[playerNumber].gameState], 1.0f);
    }
}

/* GNextGameMode for the game state (0x80286330).
 * Pick the next top-level mode (gNextGameMode) for the current game state */
void academy_next_mode(void) {
    switch (players[playerNumber].gameState) {
        case 2:
        case 3:
        case 4:
            gNextGameMode = MODE_WORLD;
            break;
        case 5:
            seq_prepare(0);
            gNextGameMode = MODE_GLOBE_SEQ;
            break;
        case 6:
            ACADEMY_ASSERT_S(levelno == 50, "levelno==50", 140);
            ACADEMY_ASSERT(players[playerNumber].levelno==50, 141);
            ACADEMY_ASSERT(frontEndPresent, 142);
            gNextGameMode = 0x800;
            D_803643D5 = 0;
            fe_best_clear_times(playerNumber);
            break;
        case 7:
            seq_prepare(1);
            gNextGameMode = MODE_GLOBE_SEQ;
            break;
        case 9:
            seq_prepare(3);
            gNextGameMode = MODE_GLOBE_SEQ;
            break;
        case 10:
            gNextGameMode = 0x4000000000000;
            break;
        case 11:
            gNextGameMode = MODE_WORLD;
            break;
        case 12:
            gNextGameMode = MODE_WORLD;
            break;
    }
}

/* "going to game state %d" (0x8028653C).
 * Advance the game state as far as the player's results allow; returns
 * whether it changed */
u8 academy_advance_state(void) {
    s32 i;
    Player *p;
    u8 advance;
    u8 stop;
    u8 oldState;
    u8 unused;
    u8 changed;

    p = &players[playerNumber];
    advance = 1;
    stop = 0;
    oldState = p->gameState;
    do {
        advance = 1;
        switch (players[playerNumber].gameState) {
            case 0:
                if (gGameMode == MODE_GLOBE_SEQ) {
                    advance = 0;
                }
                break;
            case 1:
            case 2:
            case 3:
                for (i = 0; i < 60 && advance; i++) {
                    if (gLevelInfo[i].state == players[playerNumber].gameState && !LEVEL_DONE(i) &&
                        gLevelInfo[i].type == 1) {
                        advance = 0;
                    }
                }
                break;
            case 4:
                if (p->academy != 0x3F) {
                    advance = 0;
                }
                break;
            case 5:
                if (!LEVEL_DONE(0x31)) {
                    advance = 0;
                }
                break;
            case 6:
                if (!LEVEL_DONE(0x32)) {
                    advance = 0;
                }
                break;
            case 7:
                if (!LEVEL_DONE(0x28)) {
                    advance = 0;
                }
                break;
            case 8:
                if (p->f0A < 0xDE) {
                    advance = 0;
                }
                break;
            case 9:
                if (p->f0A >= 0xEA || D_802FA26C) {
                    music_fade_to(0.0f);
                    func_8028B3E0();
                    fe_player_regrade_missions();
                    stop = 1;
                } else {
                    advance = 0;
                }
                break;
            case 10:
                break;
            case 11:
                if (p->f0A < 0x129 || D_802FA26C) {
                    advance = 0;
                } else {
                    music_fade_to(0.0f);
                    func_8028B3E0();
                    fe_player_regrade_platinum();
                }
                break;
            case 12:
                if (p->f0A >= 0x162 || D_802FA26C) {
                    p->f0A = 0x168;
                    p->f0C = 0x1E;
                    debug_printf(" ***** YOU CAN STOP NOW!! ***** \n");
                } else {
                    advance = 0;
                }
                break;
            case 13:
                advance = 0;
                break;
            default:
                debug_printf("Undefined gameState case !!!!\n");
                break;
        }
        if (D_802FA26C && players[playerNumber].gameState != 13) {
            if (saveIt[playerNumber]) {
                players[playerNumber].gameState++;
            }
        } else {
            if (advance) {
                players[playerNumber].gameState++;
            }
            debug_printf("going to game state %d\n", players[playerNumber].gameState);
        }
    } while (advance && !stop && !D_802FA26C);
    changed = players[playerNumber].gameState != oldState;
    debug_printf("game state %d to %d\n", oldState, players[playerNumber].gameState);
    return changed;
}
