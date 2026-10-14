#ifndef GAME_GAME_H
#define GAME_GAME_H

/*
 * The game's shared declarations, one copy each: include this after
 * <ultra64.h> instead of declaring other files' functions and variables
 * locally.
 *
 *   types.h      structures used by more than one file
 *   regs.h       register structs of the NON_MATCHING asm rewrites
 *   functions.h  every function used outside its file
 *   variables.h  every variable used by more than one file
 */
#include "game/types.h"
#include "game/object.h"
#include "game/regs.h"
#include "game/functions.h"
#include "game/variables.h"
#include "game/port.h"

/*
 * LEAKED(reg): an argument the original passes in a register its (matched,
 * IDO-compiled) caller never sets for the call: whatever an earlier call or
 * statement left in `reg`. The NON_MATCHING rewrites take these registers
 * as parameters; call sites that pass LEAKED(...) still need the value
 * traced from the original (port_followups.md, agent "trace"). Until then
 * they pass 0.
 */
#define LEAKED(reg) 0

/*
 * Game modes: one bit each of gGameMode / gNextGameMode (u64; hd.c's debug
 * output calls them game_mode and loop_done).  Named from what the code
 * shows a mode is (hd.c's mode switches, the setup each mode runs, the front
 * end's fe_back_loop); every bit the code uses has a name (bits 35 and 60
 * are never used).  "Never entered": handled somewhere, but nothing sets it.
 * Each macro is the literal the code used, so the compiled code is the same.
 * Asserts keep their literals: Rare's assert macros put the expression's
 * text into the ROM.
 */
#define MODE_ATTRACT 0x2                    /* the attract demos ("PRESS START") */
#define MODE_LEVEL 0x4                      /* a level in play */
#define MODE_BOOT_SCREEN1 0x10              /* the front end's boot screens (fe_boot_screen_init) */
#define MODE_BOOT_SCREEN2 0x20
#define MODE_MAP_VIEW 0x100                 /* "MISSILE VIEW" / "SHUTTLE VIEW" */
#define MODE_LEVEL_START 0x2000             /* a level's file is loaded, then MODE_LEVEL */
#define MODE_WORLD 0x4000                   /* the level-select globe ("World screen centred on level") */
#define MODE_LOAD_GAME 0x10000              /* the load-game menu */
#define MODE_NAME_ENTRY 0x40000             /* "ENTER NAME!" */
#define MODE_ERASE_MENU 0x400000            /* the erase-game menu */
#define MODE_RESULTS 0x8000000              /* after a level: results, saving */
#define MODE_VEHICLE_SELECT 0x20000000      /* digger_loop.c */
#define MODE_PROMOTION 0x40000000           /* "CONGRATULATIONS ON YOUR PROMOTION!" */
#define MODE_BEST_TIMES 0x40000000000       /* bestTimes.c */
#define MODE_GLOBE_SEQ 0x100000000000       /* the globe's sequences */
#define MODE_ATTRACT_PICTURE 0x1000000000000 /* the attract mode's picture screen */
#define MODE_BOOT_CHECK 0x20000000000000    /* the boot-time EEPROM / Controller Pak check */
#define MODE_ERASE_SAVE 0x40000000000000    /* "ERASE SAVED GAME!" */
#define MODE_GUEST_PLAYER 0x100000000000000 /* "BECOME GUEST PLAYER:" */
#define MODE_ATTRACT_START 0x400000000000000 /* the start of the attract sequence */
#define MODE_YOSHI_PAGES 0x2000000000000000 /* the text-window pages ("USE Z/R TO TURN PAGES") */
#define MODE_QUIT_GAME 0x4000000000000000   /* "QUIT GAME!" */
/* (naming phase 3) */
#define MODE_INTRO_APPROACH 0x1                  /* level intro: the BCT chopper nears its landing site */
#define MODE_LEVEL_CLEAR 0x8                     /* end object touched, no carrier: clear tune, then results */
#define MODE_REPLAY 0x40                         /* the replay of the player's recorded run */
#define MODE_LEVEL_STATUS 0x80                   /* a completed level picked on the globe: medals, menu */
#define MODE_CARRIER_END 0x200                   /* a carrier/shuttle level ends (hit or done): carrier camera */
#define MODE_REPLAY_CARRIER_END 0x400            /* the same, in a replay */
#define MODE_INTRO_FLY 0x800                     /* level intro: the BCT chopper flies in */
#define MODE_INTRO_LANDING 0x1000                /* level intro: the chopper lands and drops the driver */
#define MODE_PAK_CHECKED 0x8000                  /* after the boot pak check: load menu, pak check or boot check */
#define MODE_NEW_PLAYER 0x20000                  /* an empty slot chosen, then MODE_NAME_ENTRY */
#define MODE_ERASE_CONFIRM 0x80000               /* erase this player? */
#define MODE_ERASE_PLAYER_CHOSEN 0x100000        /* a player picked in the erase menu */
#define MODE_LOAD_GAME_RETURN 0x200000           /* back to the load-game menu */
#define MODE_TO_ERASE_MENU 0x800000              /* on to the erase menu */
#define MODE_NAME_ENTERED 0x1000000              /* a new player named: saved, then the intro sequence */
#define MODE_PLAYER_CHOSEN 0x2000000             /* a load-menu slot chosen */
#define MODE_TIMED_END 0x4000000                 /* an objective level ended (done, time out or failed) */
#define MODE_PAK_CHECK 0x10000000                /* waiting for the Controller Pak check (no EEPROM) */
#define MODE_PRE_PROMOTION 0x80000000            /* (never entered) a delay, then MODE_PROMOTION */
#define MODE_PAK_FILES 0x100000000               /* pak manager: the file list */
#define MODE_UNUSED_33 0x200000000               /* (never entered) tune 0x21 */
#define MODE_UNUSED_WINDOW_13 0x400000000        /* (never entered) text window 0x13 */
#define MODE_ACADEMY_MARKER 0x1000000000         /* the academy marker reached: message window, paused */
#define MODE_PAK_MANAGER_CHECKED 0x2000000000    /* pak manager: the check's reply */
#define MODE_PAK_FILE_CHOSEN 0x4000000000        /* pak manager: a file picked */
#define MODE_PAK_DELETE_CONFIRM 0x8000000000     /* pak manager: delete this file? */
#define MODE_PAK_FILE_DELETED 0x10000000000      /* pak manager: file deleted, back to the list */
#define MODE_UNUSED_41 0x20000000000             /* (never entered) */
#define MODE_PAK_MANAGER 0x80000000000           /* START held at boot: pak check for the file manager */
#define MODE_SEQ_END 0x200000000000              /* request: a sequence ended -> seq_finish(0) */
#define MODE_SEQ_SKIP 0x400000000000             /* request: a sequence skipped -> seq_finish(1) */
#define MODE_ACADEMY_MESSAGE 0x800000000000      /* the game state's (academy) message window and tune */
#define MODE_ACADEMY_NEXT 0x2000000000000        /* request: academy_next_mode picks the mode */
#define MODE_REGRADE_NOTICE 0x4000000000000      /* game state 10: missions regraded notice */
#define MODE_REGRADE_NOTICE_END 0x8000000000000  /* then a fade to the world */
#define MODE_UNUSED_52 0x10000000000000          /* (never entered) tune 0x28 */
#define MODE_BEST_TIMES_UNUSED 0x80000000000000  /* (never entered) a best-times variant */
#define MODE_BEST_TIMES_RETURN 0x200000000000000 /* back into best times without rebuilding it */
#define MODE_NO_CONTROLLER 0x800000000000000     /* no controller 1 at boot */
#define MODE_LOAD_GAME_EXIT 0x8000000000000000   /* B on the load menu, then the attract sequence */

#include "game/sound.h"

#endif
