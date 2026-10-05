#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Functional rewrites (verified with tools_port/eqcheck.py). D_803FB8B8 is a
 * 25-slot table of radar/map markers keyed by an id; a free slot has
 * id == -1. Callers: 479D0 (ids 0x10000+), 48D00 (0x100+), 4B5E0 (0+). */
typedef struct {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 x;
    /* 0x08 */ s32 y;
    /* 0x0C */ s32 z;
    /* 0x10 */ s32 size; /* 479D0 passes its radarSize here */
} RadarMarker; /* size 0x14 */

#define RADAR_MARKER_COUNT 25
#define RADAR_MARKER_FREE (-1)

extern RadarMarker D_803FB8B8[RADAR_MARKER_COUNT];
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Clear the marker table: mark every slot free. */
void func_802CE840(void) {
    s32 i;

    for (i = 0; i < RADAR_MARKER_COUNT; i++) {
        D_803FB8B8[i].id = RADAR_MARKER_FREE;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE840.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Add or update marker `id`: reuse its slot if present, else claim the first
 * free slot. If the table is full, nothing happens. */
void func_802CE880(s32 id, s32 x, s32 y, s32 z, s32 size) {
    RadarMarker *m;
    s32 i;

    for (i = 0, m = D_803FB8B8; i < RADAR_MARKER_COUNT; i++, m++) {
        if (m->id == id) {
            goto found;
        }
    }
    for (i = 0, m = D_803FB8B8; i < RADAR_MARKER_COUNT; i++, m++) {
        if (m->id == RADAR_MARKER_FREE) {
            m->id = id;
            goto found;
        }
    }
    return;

found:
    m->x = x;
    m->y = y;
    m->z = z;
    m->size = size;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE880.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Remove marker `id` (first matching slot only). */
void func_802CE90C(s32 id) {
    s32 i;

    for (i = 0; i < RADAR_MARKER_COUNT; i++) {
        if (D_803FB8B8[i].id == id) {
            D_803FB8B8[i].id = RADAR_MARKER_FREE;
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE90C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 1 if marker `id` is in the table, else 0. */
s32 func_802CE958(s32 id) {
    s32 i;

    for (i = 0; i < RADAR_MARKER_COUNT; i++) {
        if (D_803FB8B8[i].id == id) {
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE958.s")
#endif

/* func_802CE9A4: `D_803FB8B0 = &D_803F9330;` wrapped in the same dead
 * `addiu sp,sp,-8`/`sd $ra`/`ld $ra`/`addiu sp,sp,8` frame as
 * func_802BBE10/func_802C8AF0 - confirmed hand-written by the same
 * probe evidence (no calls, no locals, nothing that could need a frame;
 * IDO never generates this shape on its own). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F9330[];
extern u8 *D_803FB8B0;

/* Reset a buffer cursor (read by 4B5E0) to the start of D_803F9330. */
void func_802CE9A4(void) {
    D_803FB8B0 = D_803F9330;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM.
 * Port note: not rewritten. It sets up `count` collision triangles at
 * *D_803FB8B0 (func_802A41B0, byte 0x51 = 1, h52 = a2, b56 = item byte 0x14,
 * b55 = 0) from 0x16-byte items at a0, but also passes func_802A41B0 the
 * registers v0 (id, byte 0x50), t6 (byte 0x57), t9 (byte 0x4F) and s1 (byte
 * 0x58, chained) that its only caller, the IDO-compiled func_8028FDA0
 * (4B5E0.c), never sets for it: there they hold a stale call result,
 * &D_8039C718[i], the old D_803FB8B0 and the caller's caller's s1. A C version
 * can't reproduce those without changing that caller's interface. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE9C8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Clear byte 0x51 of each 0x60-byte record in [start, end). Like the asm,
 * this loops with `!=`, so `end` must be `start + n * 0x60`. */
void func_802CEA68(u8 *start, u8 *end) {
    u8 *p;

    for (p = start; p != end; p += 0x60) {
        p[0x51] = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CEA68.s")
#endif
