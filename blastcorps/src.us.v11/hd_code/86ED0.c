#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Shared by eight vehicle modules, each calling it with its own state block
 * in $gp (the C takes it as a parameter; see tools_port/conventions.txt).
 * Plays sound 0x55 unless the next game mode is 0x40 (a full 64-bit
 * compare), sets the vehicle's speed (s16 at +0x76) to 1.5 * D_80364A72
 * (D_80364A72 + (D_80364A72 >> 1), truncated to 16 bits), clears
 * D_80367BFF, calls func_80278EB0(6, 0.25f, 100) and sets D_80367C00 = 1.
 * The asm returns with a0 = 1 and a1-a3 as func_80278EB0 left them; its asm
 * callers go on to calls that don't read them. */
void func_802CB690(u8 *state) {
    s32 v;

    if (D_80364A98 != 0x40) {
        func_80260650(D_80367738, 0x55, NULL);
    }
    v = D_80364A72;
    *(s16 *) (state + 0x76) = v + (v >> 1);
    D_80367BFF = 0;
    func_80278EB0(6, 0.25f, 100);
    D_80367C00 = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86ED0/func_802CB690.s")
#endif
