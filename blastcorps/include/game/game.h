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

#endif
