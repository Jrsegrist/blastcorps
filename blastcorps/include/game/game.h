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
 * output calls them game_mode and loop_done).  Named where the code shows
 * what a mode is (hd.c's mode switches and the setup each mode runs); the
 * other bits stay literals.  Each macro is the literal the code used, so
 * the compiled code is the same.  Asserts keep their literals: Rare's assert
 * macros put the expression's text into the ROM.
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

#endif
