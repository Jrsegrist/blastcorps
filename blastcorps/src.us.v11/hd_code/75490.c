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
#ifdef NON_MATCHING
#ifndef ZONE_SCAN_REGS_DEFINED
#define ZONE_SCAN_REGS_DEFINED
/* func_802ABD54's scan registers (62740.c), in and out. */
typedef struct {
    s32 t6; /* zone x */
    s32 t7; /* zone y */
    s32 s0; /* zone z */
    s32 s1; /* distance / level term */
    s32 s2; /* zone radius */
    s32 s3; /* scan counter / zone byte */
    s32 s4; /* scan pointer / zone byte */
} ZoneScanRegs;
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
#endif
extern s32 D_803EF6DC;
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
/* Zone level lookup (func_802ABD54) for vehicle id 0xFF at its position
 * D_803EF6DC..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BA354 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BA104(ZoneScanRegs *r) {
    return func_802ABD54(0xFF, D_803EF6DC, D_803EF6E0, D_803EF6E4, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA104.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rounded 3-D distance from (ax, ay, az) to (bx, by, bz) (62740.c; asm
 * convention in tools_port/conventions.txt: t3-t5, t6, t7, s0 -> s1). */
s32 func_802ABCDC(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz);

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

    dist = func_802ABCDC(D_803643E0, D_803643E4, D_803643E8, D_803EF6DC, D_803EF6E0, D_803EF6E4);
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
#ifdef NON_MATCHING
extern s32 D_803EF6F4; /* second point x, z (y = 0) */
extern s32 D_803EF6F8;

/* Distance (func_802ABCDC, y = 0) from (D_803EF6DC, D_803EF6E4) to
 * (D_803EF6F4, D_803EF6F8); sets D_803EF710 when it is >= D_803EF6E8 and
 * D_803EF711 when it is >= D_803EF6EC (signed). Returns the distance (the
 * asm leaves it in s1; conventions.txt). The asm also leaves t7 = 0 and
 * s0 = D_803EF6F8; asm caller func_802BA354 keeps a2, a3, f12, f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BA5A4(void) {
    s32 dist = func_802ABCDC(D_803EF6DC, 0, D_803EF6E4, D_803EF6F4, 0, D_803EF6F8);

    if (dist >= D_803EF6E8) {
        D_803EF710 = 1;
    }
    if (dist >= D_803EF6EC) {
        D_803EF711 = 1;
    }
    return dist;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA5A4.s")
#endif

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
#ifdef NON_MATCHING
/* cvt.w.s under the FCSR's default rounding (nearest, ties to even); a C cast
 * truncates instead. */
#define CVT_W_S(out, x)                                                 \
    do {                                                                \
        f32 _x = (x);                                                   \
        s32 _r = (s32) _x;                                              \
        f32 _f = _x - (f32) _r;                                         \
                                                                        \
        if (_f > 0.5f || (_f == 0.5f && (_r & 1))) {                    \
            _r++;                                                       \
        } else if (_f < -0.5f || (_f == -0.5f && (_r & 1))) {           \
            _r--;                                                       \
        }                                                               \
        (out) = _r;                                                     \
    } while (0)

extern u8 D_803EF630[];  /* this vehicle's state block (the asm's $gp) */
extern s16 D_803EF6D6;   /* last speed sent to the engine sound */
extern f32 D_8030D920;
extern f32 D_8030D924;
extern f32 D_8030D928;
extern u16 D_80364452;   /* camera / player angle (0..0xFFF) */
extern u8 D_802E8BD0;
extern u8 D_803EF330[];  /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);

/* Engine sound and animation channels for vehicle D_803EF630 (speed = s16 at
 * +0x76). When the speed changed since the last call (D_803EF6D6) and the
 * sound D_803EF6D8 is playing, its parameter 0x10 is set to the float
 * 1.5 + speed * D_8030D920 (passed as raw bits). Then a = (D_80364452 +
 * 0x800) wrapped by -0xFFF at 0x1000; channel 2 gets value a / 0x555
 * (0, 1, else 2) and fraction (a % 0x555) / D_8030D924 (func_802A0360);
 * channel 1 gets round(speed * D_8030D928) (0 while D_802E8BD0 is set,
 * func_802A039C). Returns a / 0x555 (the asm's s4; conventions.txt). The asm
 * also clobbers s5; asm caller func_802BA354 keeps a2, a3, t7, f12, f14 live
 * (a mixed N64 build would need a thunk; the native port won't). The asm's
 * `addi` traps on overflow, which can't happen for a u16. */
s32 func_802BA6AC(void) {
    s32 speed = *(s16 *) (D_803EF630 + 0x76);
    s16 last = D_803EF6D6;
    u32 a;
    u32 q;
    s32 v;
    f32 frac;

    D_803EF6D6 = speed;
    if (last != speed && D_803EF6D8 != NULL) {
        f32 p = 1.5f + (f32) speed * D_8030D920;

        func_80260AB8(D_803EF6D8, 0x10, *(s32 *) &p);
    }
    a = D_80364452 + 0x800;
    if ((s32) a >= 0x1000) {
        a -= 0xFFF;
    }
    q = a / 0x555;
    frac = (f32) (s32) (a % 0x555) / D_8030D924;
    if (q == 0) {
        func_802A0360(frac, D_803EF330, 2, 0);
    } else if (q == 1) {
        func_802A0360(frac, D_803EF330, 2, 1);
    } else {
        func_802A0360(frac, D_803EF330, 2, 2);
    }
    v = D_802E8BD0 != 0 ? 0 : *(s16 *) (D_803EF630 + 0x76);
    CVT_W_S(v, (f32) v * D_8030D928);
    func_802A039C(D_803EF330, 1, v);
    return q;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA6AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF6F0;
extern u8 D_803643DA;
extern u8 D_802E8BD8;

/* Distance (func_802ABCDC, y = 0) from (D_803EF6DC, D_803EF6E4) to
 * (D_803EF6F4, D_803EF6F8); when it is >= D_803EF6F0 (signed) sets
 * D_803643DA and D_802E8BD8 to 1. The asm leaves the distance in s1 and
 * D_803EF6F8 in s0 (conventions.txt: clobbers s0, s1); asm caller
 * func_802BA354 keeps a2, a3, f12, f14 live (a mixed N64 build would need a
 * thunk; the native port won't). */
void func_802BA91C(void) {
    s32 dist = func_802ABCDC(D_803EF6DC, 0, D_803EF6E4, D_803EF6F4, 0, D_803EF6F8);

    if (dist >= D_803EF6F0) {
        D_803643DA = 1;
        D_802E8BD8 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA91C.s")
#endif

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
#ifdef NON_MATCHING
extern s16 D_8036444C;
extern s16 D_80364450;
extern u8 D_803EF720[];  /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);

/* Enter vehicle type 6 (called from hd.c): D_8036444C/50 = 6000, 9000, then
 * sets fields 0x14/0x11/0x12 of channels 2, 4, 5 of D_803EF720 and starts
 * channels 4 and 5 with value -1 (func_802A0290). The asm also points $gp at
 * D_803EFA20 and leaves it there (conventions.txt: clobbers gp) and leaves
 * v1 = -1; the C caller uses neither. */
void func_802BB054(void) {
    D_8036444C = 6000;
    D_80364450 = 9000;
    func_802A039C(D_803EF720, 2, 1);
    func_802A03D4(D_803EF720, 2, 0);
    func_802A040C(D_803EF720, 2, 1);
    func_802A039C(D_803EF720, 4, 0);
    func_802A03D4(D_803EF720, 4, 0);
    func_802A040C(D_803EF720, 4, 0);
    func_802A0290(D_803EF720, 4, -1);
    func_802A039C(D_803EF720, 5, 0);
    func_802A03D4(D_803EF720, 5, 0);
    func_802A040C(D_803EF720, 5, 1);
    func_802A0290(D_803EF720, 5, -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB054.s")
#endif

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
#ifdef NON_MATCHING
extern u64 *D_803EFAE4;  /* save copy pair (func_802A7764) */
extern u64 *D_803EFAE8;
extern void *D_803EFAD8; /* sound handles, NULL = none */
extern void *D_803EFADC;
extern void *D_803EFAE0;
void func_802A7764(u64 *a, u64 *b, s32 size);

/* Leave vehicle type 6 (called from hd.c): func_802A7764(D_803EFAE4,
 * D_803EFAE8, 0x800), then stops the sounds D_803EFADC, D_803EFAD8 and
 * D_803EFAE0 that are playing (func_802608C8); the handles are left as they
 * are. The asm saves and restores $gp. */
void func_802BB1A0(void) {
    func_802A7764(D_803EFAE4, D_803EFAE8, 0x800);
    if (D_803EFADC != NULL) {
        func_802608C8(D_803EFADC);
    }
    if (D_803EFAD8 != NULL) {
        func_802608C8(D_803EFAD8);
    }
    if (D_803EFAE0 != NULL) {
        func_802608C8(D_803EFAE0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB1A0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef ZONE_SCAN_REGS_DEFINED
#define ZONE_SCAN_REGS_DEFINED
/* func_802ABD54's scan registers (62740.c), in and out. */
typedef struct {
    s32 t6; /* zone x */
    s32 t7; /* zone y */
    s32 s0; /* zone z */
    s32 s1; /* distance / level term */
    s32 s2; /* zone radius */
    s32 s3; /* scan counter / zone byte */
    s32 s4; /* scan pointer / zone byte */
} ZoneScanRegs;
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
#endif
extern u32 D_803EFAC8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 6 at its position
 * D_803EFAC8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BB274 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BB230(ZoneScanRegs *r) {
    return func_802ABD54(6, D_803EFAC8[0], D_803EFAC8[1], D_803EFAC8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB230.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB274.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80370C15; /* input flags */
extern u8 D_80370C16;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C1D;
extern f32 D_8030D930; /* channel 5 limits */
extern f32 D_8030D934;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802A04BC(s32 idx, void *base, s32 *out);
s32 func_802BB868(void);

/* Animation channels and sounds of vehicle type 6 ($gp = D_803EFA20, read as
 * the global; channels in D_803EF720, an Unk8029DEA0Entry table).
 * - Byte +0xA2 set: channels 5, 4 off (unk14 = 0), channel 2 unk11 = 0 and
 *   restarted (func_802A0290), +0xA1 = 1.
 * - Otherwise channel 5 follows input D_80370C1C while its unk4 <= D_8030D930
 *   (direction 0) or D_80370C1D while !(unk4 < D_8030D934) (direction 1),
 *   starting sound 0x6F (handle D_803EFAD8) if needed, else it is switched
 *   off and that sound stopped; channel 4 likewise follows D_80370C15 /
 *   D_80370C16 with sound 0x6E (D_803EFADC). Then, when func_802BB868 reports
 *   ground under the level-0x11 part, channel 2 is restarted with
 *   direction 0 and +0xA1 = 1 unless it is already running in direction 1
 *   at unk4 * 100 == 100 (cvt.w.s, nearest); with no ground, if +0xA1 is 0
 *   and input D_80370C1A or D_80370C1B is set, sound 0x6D (D_803EFAE0) is
 *   started if needed, channel 2 restarted and +0xA1 = 1, and nothing else
 *   happens.
 * - Finally (all other paths): when channel 2 isn't active (unk10 != 1),
 *   +0xA1 = 0 and sound D_803EFAE0 is stopped if playing (handle kept).
 * Register note: the asm leaves func_802BB868's registers behind (s0-s7, fp,
 * f20-f28, t7 per the survey; conventions.txt: clobbers); the asm caller
 * func_802BB274 passes some of them on to func_802A92C8 unchanged, which the
 * C can't reproduce (they are func_802BB868's scratch). */
void func_802BB4C0(void) {
    s32 e[8];
    s32 r;

    if (D_803EFA20[0xA2] != 0) {
        func_802A039C(D_803EF720, 5, 0);
        func_802A039C(D_803EF720, 4, 0);
        func_802A03D4(D_803EF720, 2, 0);
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
        goto check2;
    }

    if (D_80370C1C != 0 && (func_802A04BC(5, D_803EF720, e), ((f32 *) e)[7] <= D_8030D930)) {
        func_802A03D4(D_803EF720, 5, 0);
        func_802A039C(D_803EF720, 5, 1);
        goto start5;
    }
    if (D_80370C1D != 0 && (func_802A04BC(5, D_803EF720, e), !(((f32 *) e)[7] < D_8030D934))) {
        func_802A03D4(D_803EF720, 5, 1);
        func_802A039C(D_803EF720, 5, 1);
        goto start5;
    }
    func_802A039C(D_803EF720, 5, 0);
    if (D_803EFAD8 != NULL) {
        func_802608C8(D_803EFAD8);
    }
    goto chan4;
start5:
    if (D_803EFAD8 == NULL) {
        func_80260650(D_80367738, 0x6F, &D_803EFAD8);
    }

chan4:
    if (D_80370C15 != 0) {
        func_802A03D4(D_803EF720, 4, 0);
        func_802A039C(D_803EF720, 4, 1);
    } else if (D_80370C16 != 0) {
        func_802A03D4(D_803EF720, 4, 1);
        func_802A039C(D_803EF720, 4, 1);
    } else {
        func_802A039C(D_803EF720, 4, 0);
        if (D_803EFADC != NULL) {
            func_802608C8(D_803EFADC);
        }
        goto ground;
    }
    if (D_803EFADC == NULL) {
        func_80260650(D_80367738, 0x6E, &D_803EFADC);
    }

ground:
    if (func_802BB868() != 0) {
        func_802A04BC(2, D_803EF720, e);
        if (e[6] == 1) {
            CVT_W_S(r, ((f32 *) e)[7] * 100.0f);
            if (r == 100) {
                goto check2;
            }
        }
        func_802A03D4(D_803EF720, 2, 0);
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
    } else {
        if (D_803EFA20[0xA1] != 0) {
            goto check2;
        }
        if (D_80370C1A == 0 && D_80370C1B == 0) {
            goto check2;
        }
        if (D_803EFAE0 == NULL) {
            func_80260650(D_80367738, 0x6D, &D_803EFAE0);
        }
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
        return;
    }

check2:
    func_802A04BC(2, D_803EF720, e);
    if (e[0] != 1) {
        D_803EFA20[0xA1] = 0;
        if (D_803EFAE0 != NULL) {
            func_802608C8(D_803EFAE0);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB4C0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef TRI_SCAN_TYPES_DEFINED
#define TRI_SCAN_TYPES_DEFINED
/* func_802AC0BC's FP and integer register results (62740.c), in and out. */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 */
    f32 dz;    /* f26 */
} TriSideOut;
typedef struct {
    s32 a1; /* found flag */
    s32 a3;
    s32 t6;
    s32 t7;
    s32 fp;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
} TriScanRegs;
s32 func_802AC0BC(s32 x, s32 z, s32 y, TriSideOut *f, TriScanRegs *r);
#endif
/* func_8029C6E4's register results (56040.c). */
typedef struct {
    s32 s0;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4; /* 1 = found */
    u8 *t6;
    s32 t7;
} Unk8029C6E4Out;
void func_8029C6E4(Unk8029C6E4Out *o);
extern s32 D_802E8BDC; /* current level */

/* On level 0x11 only: finds the kind-6 / id-0x3BD part (func_8029C6E4) and
 * returns whether func_802AC0BC finds ground under its position (words 0, 8
 * as x, z; word 4 as y); 0 otherwise.
 * Register convention: result in v0 (ABI). The asm leaves both callees'
 * registers behind (s0-s4, t6, t7, fp, f12-f26) and its caller
 * func_802BB4C0 only tests v0, so they're not modelled (conventions.txt:
 * clobbers). Its s0-s3 inputs only pass through func_8029C6E4 when nothing is
 * found, and func_802AC0BC's pass-through inputs a3 / f12-f26 don't affect
 * the found flag, so the C starts them at 0. */
s32 func_802BB868(void) {
    Unk8029C6E4Out o;
    TriSideOut f;
    TriScanRegs r;

    if (D_802E8BDC != 0x11) {
        return 0;
    }
    o.s0 = 0;
    o.s1 = 0;
    o.s2 = 0;
    o.s3 = 0;
    func_8029C6E4(&o);
    if (o.s4 == 0) {
        return 0;
    }
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    r.a1 = 0;
    r.a3 = 0;
    r.t6 = (s32) o.t6;
    r.t7 = o.t7;
    r.fp = 0;
    r.s1 = o.s1;
    r.s2 = o.s2;
    r.s3 = o.s3;
    r.s4 = o.s4;
    func_802AC0BC(o.s0, o.s2, o.s1, &f, &r);
    return r.a1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB868.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB8B8.s")
