#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802B9C50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Looks up the current
 * level in a table of {level, x, z} records terminated by a negative level
 * and sets the spawn position (x, z in 1/32 units) from the matching record;
 * position stays 0 if the level isn't listed. The asm also saves/restores
 * v0, v1, a0; its asm caller func_802B9C50 relies on a1, a3, f12, f14 being
 * preserved (a mixed N64 build would need a thunk; the native port won't). */
typedef struct {
    s16 level;
    u16 x;
    u16 z;
} LevelPos;
extern s32 D_802E8BDC; /* current level */
extern LevelPos D_80305D74[];
extern s32 D_803EF6E8;
extern s32 D_803EF6EC;
extern u8 D_803EF710;
extern u8 D_803EF711;

void func_802BA074(void) {
    LevelPos *e;
    s32 level;

    D_803EF6E8 = 0;
    D_803EF710 = 0;
    D_803EF6EC = 0;
    D_803EF711 = 0;
    level = D_802E8BDC;
    for (e = D_80305D74; e->level >= 0; e++) {
        if (e->level == level) {
            D_803EF6E8 = e->x << 5;
            D_803EF6EC = e->z << 5;
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA074.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA104.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
double sqrt(double);
#pragma intrinsic(sqrt)

/* func_802ABCDC (62740; points in t3-t5 and t6,t7,s0, result in s1) can't be
 * called from C: this is its logic (same macro as in 77E20.c/8DDB0.c). The
 * distance between two points, rounded to nearest (ties to even) as cvt.l.d
 * does; the squares are summed as s64 and converted as hi * 2^32 + lo (one
 * rounding, like cvt.d.l; a plain cast would call __ll_to_d). */
#define DIST3_802ABCDC(out, ax, ay, az, bx, by, bz)                     \
    do {                                                                \
        s64 _dx = (s32) ((bx) - (ax));                                  \
        s64 _dy = (s32) ((by) - (ay));                                  \
        s64 _dz = (s32) ((bz) - (az));                                  \
        s64 _sq = _dx * _dx + _dy * _dy + _dz * _dz;                    \
        f64 _d = sqrt((f64) (s32) (_sq >> 32) * 4294967296.0 + (f64) (u32) _sq); \
        s32 _r = (s32) _d;                                              \
        f64 _f = _d - _r;                                               \
                                                                        \
        if (_f > 0.5 || (_f == 0.5 && (_r & 1))) {                      \
            _r++;                                                       \
        }                                                               \
        (out) = _r;                                                     \
    } while (0)

extern s32 D_803643E0; /* player x, y, z */
extern s32 D_803643E4;
extern s32 D_803643E8;
extern void *D_80367738;  /* sound player */
extern void *D_803EF6D8;  /* this sound's handle, NULL = none */
extern s32 D_803EF6DC;    /* sound source x, y, z */
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);

/* Positional sound 0x75 at D_803EF6DC/E0/E4 (called from hd.c); shape of
 * func_802B8794 (72B80). d is the rounded distance from the player
 * D_803643E0/E4/E8. Beyond 16000 the sound is stopped (func_802608C8) and
 * its handle cleared. Otherwise it is started if it isn't playing, its
 * volume (parameter 8) set to 0x7FFF - 2 * max(d - 4000, 0) and its pan
 * (parameter 4) to 64 + (player x - source x) / 32, clamped to 0..127. The
 * asm's `sub`/`add` trap on overflow; C doesn't. Register note: the asm
 * saves and restores every register. */
void func_802BA148(void) {
    s32 dx = D_803643E0 - D_803EF6DC;
    s32 dist;
    s32 pan;

    DIST3_802ABCDC(dist, D_803643E0, D_803643E4, D_803643E8, D_803EF6DC, D_803EF6E0, D_803EF6E4);
    if (dist > 16000) {
        if (D_803EF6D8 != NULL) {
            func_802608C8(D_803EF6D8);
            D_803EF6D8 = NULL;
        }
        return;
    }
    if (D_803EF6D8 == NULL) {
        func_80260650(D_80367738, 0x75, &D_803EF6D8);
    }
    dist -= 4000;
    if (dist < 0) {
        dist = 0;
    }
    func_80260AB8(D_803EF6D8, 8, 0x7FFF - (dist << 1));
    pan = 0x40 + (dx >> 5);
    if (pan < 0) {
        pan = 0;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    func_80260AB8(D_803EF6D8, 4, pan);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA148.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA354.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA5A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    s16 z;    /* negative ends the table */
    u8 level;
    u8 value;
} ZoneEntry;
extern ZoneEntry D_80305D62[];
extern s16 D_803EF6FC;

/* Scans D_80305D62 (terminated by a negative z) for entries of the current
 * level (D_802E8BDC) whose z <= player z / 32 (D_803EF6E4 >> 5); the last
 * one's value wins (0 if none). A nonzero result is stored in D_803EF6FC.
 * The asm leaves its scan registers behind and asm caller func_802BA354
 * doesn't overwrite them before func_802A860C reads a0-a3, so they are all
 * outputs (see tools_port/conventions.txt): result (a0), the terminator z
 * (a1), the last level byte read (a2, unchanged if the table is empty) and
 * the current level (a3). */
s32 func_802BA638(s32 *termZ, s32 *lastLevel, s32 *level) {
    s32 pz = D_803EF6E4 >> 5;
    s32 lv = D_802E8BDC;
    s32 result = 0;
    ZoneEntry *e;
    s32 z;

    for (e = D_80305D62; (z = e->z) >= 0; e++) {
        *lastLevel = e->level;
        if (e->level == lv && !(pz < z)) {
            result = e->value;
        }
    }
    if (result != 0) {
        D_803EF6FC = result;
    }
    *termZ = z;
    *level = lv;
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA638.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA6AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA91C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA9A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BABEC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle "set timers"
 * leaf, the shape shared by func_802BC578, func_802C9B30, func_802CB5D8,
 * func_802CC8B8, func_802CD9AC, func_802D0784 and func_802D249C (only the
 * scale and the two byte values differ). Asm caller func_802BA354 relies on
 * a0-a3 being preserved (mixed N64 build would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802BAD24(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD24.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD80.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB054.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EFA20[]; /* this vehicle's state block */

/* Exit check for vehicle type 6 (called from func_8024B4B8 in hd.c, which
 * declares it void and returns the leftover v0): 1 when the byte at +0xA1 is
 * 0, else 0. Returns s32 (the asm's v0). The asm also points $gp at
 * D_803EFA20 and leaves it there (conventions.txt: clobbers gp), and leaves
 * the byte in v1; C callers use neither. */
s32 func_802BB170(void) {
    return D_803EFA20[0xA1] == 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB170.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB1A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB230.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB274.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB4C0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB868.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB8B8.s")
