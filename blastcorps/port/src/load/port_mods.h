/* Edited level files in the port (port_mods.c): mods (.bcmod, level_mod.h)
 * applied in memory to the level files read from the player's ROM, and the
 * level editor's edits (bc.exe's viewer).  The game reads an edited file
 * instead of the ROM's through one hook around lvl_load_file's hd_rom_load
 * (game/port.h PORT_LEVEL_PRE / PORT_LEVEL_POST); the ROM itself, on disk
 * and in memory, is never changed.  With no mod and no edit, nothing here
 * runs: the game reads the ROM's files exactly as before. */
#ifndef PORT_MODS_H
#define PORT_MODS_H

#include <stddef.h>
#include <stdint.h>

#include "load/level_mod.h"

#ifdef __cplusplus
extern "C" {
#endif

/* log lines go here (default: stderr; the exes point it at their log) */
extern void (*port_mods_log_hook)(const char *fmt, ...);

/* load a .bcmod (UTF-8 path): every level it changes is checked against the ROM's file (its SHA-1) and built;
 * from then on the game reads the edited files.  0 ok, -1 and *err (nothing of the mod is used) */
int port_mods_add(const char *path, char *err, size_t errlen);
int port_mods_count(void);
const char *port_mods_name(int i);   /* the mod's name (or its file name) */

/* the ROM's level file, inflated (both gzip members, big-endian), cached; 0 ok */
int port_level_original(int level, const uint8_t **file, size_t *size);
/* the edited file the game reads for this level (a mod's or the editor's); 0 if there is one */
int port_level_edited(int level, const uint8_t **file, size_t *size);
/* put an edited file in place for the level's next load (copied; NULL: the ROM's again); source: for the log */
int port_level_set_edit(int level, const uint8_t *file, size_t size, const char *source);
const char *port_level_edit_source(int level);   /* NULL: none */
/* the changes the loaded mods make to a level, in order (the editor starts from them); 0 if any */
int port_level_mod_changes(int level, const LmChange **changes, int *n, const char **modName);
/* every object placement's type has a building in the ROM's object table; 0 ok, -1 and *err */
int port_level_check_objects(const uint8_t *file, size_t size, char *err, size_t errlen);
/* the times the game read an edited file of this level */
unsigned port_level_edit_loads(int level);
/* (testing: bc_headless --mod-out) every edited file to DIR\level_NN.bin; the count written, -1 on an error */
int port_mods_write_levels(const char *dir);

#ifdef __cplusplus
}
#endif

#endif
