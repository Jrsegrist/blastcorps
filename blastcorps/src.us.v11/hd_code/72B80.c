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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7340.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEE70[]; /* this vehicle's state block */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

/* Enter vehicle type 8: clears the byte at +0x99, D_8036444C/50 = 3000,
 * 1000, then func_802C4310(arg0, 0xCE) (arg0 passes straight through; hd.c
 * calls this with no arguments and func_802C4310 ignores it). The asm also
 * points $gp at D_803EEE70 and leaves it there (conventions.txt: clobbers
 * gp); C code doesn't use $gp. Same shape as func_802BBDC8 (772A0),
 * func_802CCC8C (88160), func_802CFA0C (8AEE0). */
void func_802B76AC(s32 arg0) {
    D_803EEE70[0x99] = 0;
    D_8036444C = 3000;
    D_80364450 = 1000;
    func_802C4310(arg0, 0xCE);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B76AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEE70[]; /* this vehicle's state block */

/* Exit check for vehicle type 8, same shape as func_802B2EF8 (6E200): returns
 * 1 when none of the bytes at +0x96, +0x97, +0x98 equals 1, else 0 (s32; the
 * C caller in hd.c declares it void and returns the leftover v0). */
s32 func_802B76F8(void) {
    if (D_803EEE70[0x96] == 1 || D_803EEE70[0x97] == 1 || D_803EEE70[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B76F8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EEF28;
extern u64 *D_803EEF2C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Vehicle-type 8 exit: zero the speed (s16 at +0x76), copy 0x100 bytes
 * between the two buffers D_803EEF28/D_803EEF2C point at (func_802A7764),
 * then func_802C444C. Same shape as func_802CFAB4 (8AEE0). */
void func_802B7754(void) {
    *(s16 *) (D_803EEE70 + 0x76) = 0;
    func_802A7764(D_803EEF28, D_803EEF2C, 0x100);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7754.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B77A0.s")

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
extern u32 D_803EEF18[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 8 at its position
 * D_803EEF18..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802B7A88 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802B78B0(ZoneScanRegs *r) {
    return func_802ABD54(8, D_803EEF18[0], D_803EEF18[1], D_803EEF18[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7980.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7A88.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef IO802A6274_DEFINED
#define IO802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
#endif
extern u8 D_803EEF32;   /* effect cooldown */
extern u8 D_802C2954[]; /* definition handed to func_802A6274 */
void func_802B80D8(void);
s32 func_802A5ED0(void);
void func_802C4584(s32 level);

/* Per-frame effects for vehicle type 8 ($gp = D_803EEE70, read as the
 * global): the tyre trail (func_802B80D8); then, if the cooldown
 * D_803EEF32 is nonzero it just counts down, else when byte +0x99 is set the
 * cooldown becomes 1 and, with fewer than 15 active func_802A6274 records,
 * four are set up (def D_802C2954, type 1 at (8, 1..4, 1), data 0x29810
 * for the first two and 0x1D4C0 for the last two). Finally
 * func_802C4584(|speed| >> 5) with speed the s16 at +0x76.
 * Register convention (conventions.txt): t6, t7, s0-s4 pass through to
 * func_802A6274 (t6 and s1 in its in/out block, chained between the calls);
 * the asm changes s1 and s5-s7. Same shape as func_802D02F8 (8AEE0). */
void func_802B7F98(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 v;

    func_802B80D8();
    if (D_803EEF32 != 0) {
        D_803EEF32--;
    } else if (D_803EEE70[0x99] != 0) {
        D_803EEF32 = 1;
        if (func_802A5ED0() < 15) {
            io.t6 = t6;
            io.s1 = s1;
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 8, 1, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 8, 2, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 8, 3, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 8, 4, 1, t7, s0, s2, s3, s4, 1);
        }
    }
    v = *(s16 *) (D_803EEE70 + 0x76);
    if (v < 0) {
        v = -v;
    }
    func_802C4584((u32) v >> 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7F98.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803EEF18[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803EEE70): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803EEF18[0], D_803EEF18[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802B7F98 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802B80D8(void) {
    if (D_803EEE70[0x99] != 0 && D_803EEE70[0x98] != 1 && D_803EEE70[0x50] < 3 && D_803EEE70[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803EEE70 + 0x1C), 250, -400, -400, -400, D_803EEF18[0], D_803EEF18[2],
                      *(u16 *) (D_803EEE70 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B80D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8278.s")

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

extern f32 D_8030D904;

/* Speed (s16 at +0x76 of D_803EEE70, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D904, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B7A88 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802B83B0(void) {
    f32 div;
    s32 r;

    if (D_803EEE70[0x96] == 1 || D_803EEE70[0x97] == 1 || D_803EEE70[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D904;
    }
    CVT_W_S(r, *(s16 *) (D_803EEE70 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B83B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf (shape of func_802AFBA0 in 69BB0, other
 * constants): D_803EBBF4 = D_803EBBF0 * 4, D_803ED3F6/7 = 60, 3.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm callers keep
 * registers live across the call (func_802B7980: t3, t4, f12, f14;
 * func_802B7A88: a0-a3); a mixed N64 build would need a thunk preserving
 * those, the native port does not. */
void func_802B8424(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8424.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8480.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rounded 3-D distance from (ax, ay, az) to (bx, by, bz) (62740.c; asm
 * convention in tools_port/conventions.txt: t3-t5, t6, t7, s0 -> s1). */
s32 func_802ABCDC(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz);

extern s32 D_803643E0; /* player x, y, z */
extern s32 D_803643E4;
extern s32 D_803643E8;
extern void *D_80367738;  /* sound player */
extern void *D_803EF2E8;  /* this sound's handle, NULL = none */
extern s32 D_803EF2EC;    /* sound source x, y, z */
extern s32 D_803EF2F0;
extern s32 D_803EF2F4;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);

/* Positional sound 0x13 at D_803EF2EC/F0/F4 (called from hd.c). d is the
 * rounded distance from the player D_803643E0/E4/E8. Beyond 16000 the sound
 * is stopped (func_802608C8) and its handle cleared. Otherwise it is
 * started if it isn't playing, its volume (parameter 8) set to 0x7FFF -
 * max(d - 500, 0) and its pan (parameter 4) to 64 + (player x - source
 * x) / 32, clamped to 0..127. The asm's `sub`/`add` trap on overflow; C
 * doesn't. Register note: the asm saves and restores every register. Same
 * shape as func_802BA148 (75490). */
void func_802B8794(void) {
    s32 dx = D_803643E0 - D_803EF2EC;
    s32 dist;
    s32 pan;

    dist = func_802ABCDC(D_803643E0, D_803643E4, D_803643E8, D_803EF2EC, D_803EF2F0, D_803EF2F4);
    if (dist > 16000) {
        if (D_803EF2E8 != NULL) {
            func_802608C8(D_803EF2E8);
            D_803EF2E8 = NULL;
        }
        return;
    }
    if (D_803EF2E8 == NULL) {
        func_80260650(D_80367738, 0x13, &D_803EF2E8);
    }
    dist -= 500;
    if (dist < 0) {
        dist = 0;
    }
    func_80260AB8(D_803EF2E8, 8, 0x7FFF - dist);
    pan = 0x40 + (dx >> 5);
    if (pan < 0) {
        pan = 0;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    func_80260AB8(D_803EF2E8, 4, pan);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8794.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B899C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EF240[]; /* this vehicle's state block */
extern s16 D_803EF328;
extern s16 D_803EF32A;
extern s32 D_803EF310; /* marker position x, y, z, w */
extern s32 D_803EF314;
extern s32 D_803EF318;
extern s32 D_803EF31C;
extern s32 D_803ED808; /* player position x, y, z */
extern s32 D_803ED80C;
extern s32 D_803ED810;
extern s32 D_80368030;
extern u8 D_803EF32C;
extern u64 D_80364A90; /* game mode */
extern u64 D_80364A98; /* next game mode */
void func_80275390(u64);

/* Copies two halfwords of the vehicle block (+0x4E, +0x76) to D_803EF32A /
 * D_803EF328, and the marker position D_803EF310..1C to D_803ED808..10 and
 * D_80368030. Then, in game mode 0x1000: if D_803EF32C == 5 or
 * D_803EF31C >= D_803EF314, calls func_80275390(0x2000). In any other mode:
 * if D_803EF32C == 4, sets the next game mode to 0x1000.
 * The game-mode compare is a full 64-bit compare in the asm (ld/beq). */
void func_802B8AE4(void) {
    D_803EF32A = *(s16 *) (D_803EF240 + 0x4E);
    D_803EF328 = *(s16 *) (D_803EF240 + 0x76);
    D_803ED808 = D_803EF310;
    D_803ED80C = D_803EF314;
    D_803ED810 = D_803EF318;
    D_80368030 = D_803EF31C;
    if (D_80364A90 == 0x1000) {
        if (D_803EF32C == 5 || !(D_803EF31C < D_803EF314)) {
            func_80275390(0x2000);
        }
    } else if (D_803EF32C == 4) {
        D_80364A98 = 0x1000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8AE4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef TRISIDEOUT_DEFINED
#define TRISIDEOUT_DEFINED
/* func_802A9B1C's FP side results (62740.c), in and out. */
typedef struct {
    f32 pz;    /* f12 */
    f32 cross; /* f14 */
    f32 cz;    /* f20 */
    f32 side;  /* f22 */
    f32 sideZ; /* f24 */
    f32 dz;    /* f26 */
} TriSideOut;
#endif
extern s16 D_803BE732; /* level extent x, z (>> 5) */
extern s16 D_803BE736;
extern s16 D_803EF326;
s32 func_802A9B1C(s32 index, s32 x, s32 z, s32 y, s32 skip, u8 *veh, s32 fpIn, TriSideOut *f);
void func_802582C4(u8 id, s32 x, s32 y, s32 z, s32 arg4, s32 arg5, s32 arg6, s32 arg7);

/* Shadow/marker 0xFE at the sound source D_803EF2EC/F0/F4 ($gp = D_803EF240,
 * read as the global): when x and z are positive and inside the level
 * (D_803BE732 / D_803BE736 << 5), the ground height under it comes from
 * func_802A9B1C(slot 0, x, z, D_803EF31C, skip 0xFE) and is stored to
 * D_803EF31C; otherwise the height is the incoming t3 (whatever the asm
 * callers left there). Then func_802582C4(0xFE, x, height, z, D_803EF2F0, 0,
 * 0, (s16) +0x4C) and D_803EF326 = +0x4C.
 * Register convention (conventions.txt): t3 and fp (func_802A9B1C's fp input)
 * come in from the asm callers, f12-f26 pass through func_802A9B1C (here
 * through f). The asm's v1 (read by func_802475D8 via func_802B899C) is just
 * func_802582C4's leftover; f12/f14 too after that C call. */
void func_802B8C18(s32 t3, s32 fp, TriSideOut *f) {
    s32 x = D_803EF2EC;
    s32 z = D_803EF2F4;
    s32 y = t3;

    if (x > 0 && z > 0 && x < (D_803BE732 << 5) && z < (D_803BE736 << 5)) {
        y = func_802A9B1C(0, x, z, D_803EF31C, 0xFE, D_803EF240, fp, f);
        D_803EF31C = y;
    }
    func_802582C4(0xFE, D_803EF2EC, y, D_803EF2F4, D_803EF2F0, 0, 0, *(s16 *) (D_803EF240 + 0x4C));
    D_803EF326 = *(s16 *) (D_803EF240 + 0x4C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8C18.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef OUT802A860C_DEFINED
#define OUT802A860C_DEFINED
/* func_802A860C's results besides its return value (62740.c). */
typedef struct {
    s32 t1; /* new z */
    s32 s3; /* *px as read */
    s32 fp; /* the cosine */
} Out802A860C;
#endif
extern u8 D_803EF32D;
extern u8 D_803EF32E;
extern s32 D_803EF304; /* target height */
extern s32 D_803EF308; /* target x, z */
extern s32 D_803EF30C;
extern s32 D_803EF320; /* best distance so far */
extern s16 D_803EF324; /* turn rate */
extern s32 D_80368048;
extern s32 D_802E8BDC; /* current level */
extern u8 *D_80358074;
extern char D_80305D40[];
extern u8 D_803EEF40[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A04BC(s32 idx, void *base, s32 *out);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A0290(void *base, s32 idx, s32 val);
s32 func_802B988C(void);
void func_8029A7E4(const char *fmt, ...);
void func_8026AF6C(s32 arg0);
s32 func_8026A610(s32 x1, s32 y1, s32 x2, s32 y2);
void func_802A5604(u8 *level);
s32 func_802ABB1C(s32 ax, s32 az, s32 dx, s32 dz, s32 bx, s32 bz);
s32 func_802ACE38(s32 x, s32 z, s32 angle, s32 *t1out);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);

#define VEH8D04_S16(off) (*(s16 *) (D_803EF240 + (off)))
#define VEH8D04_U16(off) (*(u16 *) (D_803EF240 + (off)))
#define ABS_8D04(v) ((v) < 0 ? -(v) : (v))

/* Move cur toward target by at most 0x14 (signed compares). */
static s32 port_approach_14(s32 cur, s32 target) {
    if (cur != target) {
        if (!(target < cur)) {
            cur += 0x14;
            if (target < cur) {
                cur = target;
            }
        } else {
            cur -= 0x14;
            if (cur < target) {
                cur = target;
            }
        }
    }
    return cur;
}

/* One axis of the state-3 approach: pos moves toward target by
 * (|target - pos| << 10) / dist * speed >> 10 (unsigned 32-bit arithmetic,
 * then the sign of target - pos). */
static s32 port_step_axis(s32 pos, s32 target, s32 dist, s32 speed) {
    s32 d = target - pos;
    u32 m = (u32) ABS_8D04(d) << 10;

    m = (m / (u32) dist) * (u32) speed >> 10;
    if (d < 0) {
        m = -m;
    }
    return pos + m;
}

/* State 1 (approaching the target D_803EF308/30C): bx/bz are the asm's
 * leftover registers from func_802B988C (D_803EF2EC/F4, or D_80368048 as bx
 * after the debug warp). Returns 1 to go on to the heading update. */
static s32 port_state1(void) {
    s32 ax = D_803EF308;
    s32 az = D_803EF30C;
    s32 bx = D_803EF2EC;
    s32 bz = D_803EF2F4;
    s32 dist = func_802B988C();
    s32 limit;
    s32 a;
    s32 rx;
    s32 rz;
    s32 d1;
    s32 d2;
    s32 t0;
    s32 t3;
    s32 t2;

    if (D_80364A90 == 0x800) {
        if (D_802E8BDC == 0x1A || D_802E8BDC == 4) {
            limit = 48000;
        } else if (D_802E8BDC == 0x1D || D_802E8BDC == 0x3A || D_802E8BDC == 0xD) {
            limit = 70000;
        } else {
            limit = 30000;
        }
        if (!(limit < dist)) {
            D_80364A98 = 1;
            func_8029A7E4(D_80305D40);
            D_803EF308 = (&D_80368048)[-1];
            bx = D_80368048;
            D_803EF30C = bx;
            func_8026AF6C(0x4000);
        }
    }
    if (dist < 0x7D0) {
        D_803EF32C = 2;
        return 1;
    }
    t0 = VEH8D04_S16(0x76) + 2;
    if (t0 >= 0xA1) {
        t0 = 0xA0;
    }
    VEH8D04_S16(0x76) = t0;
    a = func_802ABB1C(ax, az, 0, dist, bx, bz);
    if (a >= 0x401) {
        a = func_802ABB1C(ax, az, dist, 0, bx, bz) + 0x400;
        if (a >= 0x801) {
            a = func_802ABB1C(ax, az, 0, -dist, bx, bz) + 0x800;
        }
    }
    rx = func_802ACE38(0, dist, a, &rz);
    d1 = ABS_8D04(rx + bx - ax) + ABS_8D04(rz + bz - az);
    rx = func_802ACE38(0, dist, 0xFFF - a, &rz);
    d2 = ABS_8D04(rx + bx - ax) + ABS_8D04(rz + bz - az);
    if (!(d1 < d2)) {
        a = 0xFFF - a;
    }
    t0 = VEH8D04_U16(0x4E) - a;
    t3 = t0;
    if (t3 >= 0) {
        if (t3 >= 0x800) {
            t3 = 0xFFF - t3;
        }
    } else if (t3 < -0x7FF) {
        t3 += 0xFFF;
    } else {
        t3 = -t3;
    }
    t3 = (u32) t3 >> 6;
    if ((t0 > 0) ? (t0 < 0x800) : (t0 < -0x800)) {
        t2 = D_803EF324 - 1;
        if (t2 < -t3) {
            t2 = -t3;
        }
    } else {
        t2 = D_803EF324 + 1;
        if (t3 < t2) {
            t2 = t3;
        }
    }
    D_803EF324 = t2;
    return 1;
}

/* State 3 (homing in): returns 1 when the target is reached (go to state
 * 4), 0 when still closing in. */
static s32 port_state3(void) {
    s32 t = D_803EF324;
    s32 dist;
    s32 speed;

    if (t >= 0) {
        t--;
        if (t < 0) {
            t = 0;
        }
    } else {
        t++;
        if (t > 0) {
            t = 0;
        }
    }
    D_803EF324 = t;
    dist = func_802B988C();
    if (dist == 0) {
        return 1;
    }
    speed = VEH8D04_S16(0x76);
    D_803EF2EC = port_step_axis(D_803EF2EC, D_803EF308, dist, speed);
    D_803EF2F4 = port_step_axis(D_803EF2F4, D_803EF30C, dist, speed);
    dist = func_802B988C();
    if (dist < D_803EF320) {
        D_803EF320 = dist;
        return 0;
    }
    return 1;
}

/* State 4 (landing at D_80368030 + 0xFA0). */
static void port_state4(void) {
    s32 target = D_80368030 + 0xFA0;
    s32 out[8];
    s32 v;
    void *h;

    if (target != D_803EF2F0) {
        D_803EF2F0 = port_approach_14(D_803EF2F0, target);
        return;
    }
    func_802A04BC(4, D_803EEF40, out);
    if (out[0] == 1) {
        return;
    }
    D_803EF32D = 1;
    if (D_80364A90 != 0x1000) {
        v = func_8026A610(D_803643E0, D_803643E0, D_803EF2EC, D_803EF2F4);
        v = 0x88B8 - (v << 1);
        if (v >= 0xFA1) {
            if (v >= 0x8000) {
                v = 0x7FFF;
            }
            h = func_80260650(D_80367738, 0x28, NULL);
            func_80260AB8(h, 8, v);
        }
    }
    D_803EF32C = 5;
    func_802A0290(D_803EEF40, 4, 1);
}

/* Per-frame update of the flying vehicle D_803EF240 (the asm points $gp at
 * it), a state machine on D_803EF32C (any other value hits the asm's
 * `syscall` debug trap and then runs as 6):
 *   6: if animation channel 4 of D_803EEF40 (func_802A04BC) has a nonzero
 *      float, restart it (func_802A03D4, func_802A0290); state 0, then as 0.
 *   0: D_803EF324 one step toward 4; heading update.
 *   1: approach D_803EF308/30C (port_state1); heading update.
 *   2: slow down by 4 to 0x14 (heading update while above), then state 3
 *      with D_803EF320 = distance, then as 3.
 *   3: D_803EF324 one step toward 0, move straight at the target
 *      (port_state3); when reached: position = target, state 4 and channel 4
 *      restarted (func_802A0290).
 *   4: descend (D_803EF2F0 toward D_80368030 + 0xFA0, 0x14 a frame); there,
 *      once channel 4 is done: D_803EF32D = 1, a landing sound outside game
 *      mode 0x1000, state 5, channel 4 restarted.
 *   5: wait for channel 4 (D_803EF32E = 1 when done), climb back to
 *      D_803EF304, then state 0 once both.
 * Heading update: heading +0x4E (and +0x4C) += D_803EF324 (wrapped by 0xFFF)
 * and the position D_803EF2EC/F4 advanced by the speed +0x76 along it
 * (func_802A860C). Finally, outside states 4/5: func_802A5604(D_80358074)
 * and D_803EF2F0 0x14 a frame toward D_803EF304. The asm's add/sub/neg trap
 * on overflow; the state-3 divides trap on a zero distance (not reached).
 * Register convention: no inputs. The asm leaves gp = D_803EF240 and its
 * callees' t6, t7, s0-s4, fp, f12/f14 (listed as read by func_802B899C;
 * not modelled) and clobbers s0-s4, s6, fp (conventions.txt). Unlike the
 * asm, which relies on func_802B988C leaving D_803EF2EC/F4 and the old
 * D_803EF308/30C in t3/t5/t6/s0, the C reads those values itself. */
void func_802B8D04(void) {
    s32 out[8];
    s32 t;
    Out802A860C o;

    switch (D_803EF32C) {
        default: /* syscall */
        case 6:
            func_802A04BC(4, D_803EEF40, out);
            if (!(*(f32 *) &out[7] == 0.0f)) {
                func_802A03D4(D_803EEF40, 4, 1);
                func_802A0290(D_803EEF40, 4, 1);
            }
            D_803EF32C = 0;
            /* fallthrough */
        case 0:
            t = D_803EF324;
            if (t >= 4) {
                t--;
                if (t < 4) {
                    t = 4;
                }
            } else {
                t++;
                if (t >= 5) {
                    t = 4;
                }
            }
            D_803EF324 = t;
            goto heading;
        case 1:
            port_state1();
            goto heading;
        case 2:
            t = VEH8D04_S16(0x76);
            if (t >= 0x15) {
                VEH8D04_S16(0x76) = t - 4;
                goto heading;
            }
            VEH8D04_S16(0x76) = 0x14;
            D_803EF32C = 3;
            D_803EF320 = func_802B988C();
            /* fallthrough */
        case 3:
            if (port_state3()) {
                D_803EF2EC = D_803EF308;
                D_803EF2F4 = D_803EF30C;
                D_803EF32C = 4;
                func_802A0290(D_803EEF40, 4, 1);
            }
            goto tail;
        case 4:
            port_state4();
            goto tail;
        case 5:
            func_802A04BC(4, D_803EEF40, out);
            if (out[0] != 1) {
                D_803EF32E = 1;
            }
            if (D_803EF304 != D_803EF2F0) {
                D_803EF2F0 = port_approach_14(D_803EF2F0, D_803EF304);
            } else if (D_803EF32E != 0) {
                D_803EF32C = 0;
                D_803EF32E = 0;
            }
            goto tail;
    }
heading:
    t = VEH8D04_U16(0x4E) + D_803EF324;
    if (t >= 0x1000) {
        t -= 0xFFF;
    } else if (t < 0) {
        t += 0xFFF;
    }
    VEH8D04_S16(0x4E) = t;
    VEH8D04_S16(0x4C) = t;
    D_803EF2EC = func_802A860C(0.0f, t, &VEH8D04_S16(0x76), &D_803EF2EC, &D_803EF2F4, &o);
    D_803EF2F4 = o.t1;
tail:
    if (D_803EF32C != 4 && D_803EF32C != 5) {
        func_802A5604(D_80358074);
        D_803EF2F0 = port_approach_14(D_803EF2F0, D_803EF304);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8D04.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF308; /* marker x, z (y = 0) */
extern s32 D_803EF30C;

/* Distance (func_802ABCDC, y = 0) from (D_803EF2EC, D_803EF2F4) to
 * (D_803EF308, D_803EF30C), returned (the asm's s1; conventions.txt). The
 * asm also leaves t6 = D_803EF308, t7 = 0, s0 = D_803EF30C and
 * func_802ABCDC's scratch in t3/t5; asm caller func_802B8D04 reloads those
 * before using them, and keeps a0-a3, f12, f14 live (a mixed N64 build
 * would need a thunk; the native port won't). */
s32 func_802B988C(void) {
    return func_802ABCDC(D_803EF2EC, 0, D_803EF2F4, D_803EF308, 0, D_803EF30C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B988C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803EF2E6;  /* last value sent to the sound */
extern s16 D_803EF324;
extern f32 D_8030D910;
extern f32 D_8030D914;
extern u8 D_803EEF40[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A0360(f32 f, void *base, s32 idx, s32 val);

/* Sound and animation channels for vehicle D_803EF240 (the asm's $gp; s16 at
 * +0x76). v = 160 - that value. When v differs from D_803EF2E6 (which is set
 * to v) and the sound D_803EF2E8 is playing, its parameter 0x10 is set to the
 * float D_8030D910 + v * D_8030D914 (passed as raw bits). Channel 2 of
 * D_803EEF40 gets v / 320.0 (value 0) and channel 3 gets, from
 * t = D_803EF324 (with +-1 treated as 0), (32 - t) / 64.0 for t >= 0 or
 * -t / 64.0 + 0.5 for t < 0 (func_802A0360). Returns v / 320.0 (the asm's
 * f20; conventions.txt). Asm caller func_802B899C keeps a2, a3, t7, f12, f14
 * live (a mixed N64 build would need a thunk; the native port won't). */
f32 func_802B98E0(void) {
    s32 v = 0xA0 - *(s16 *) (D_803EF240 + 0x76);
    s16 last = D_803EF2E6;
    f32 ratio = (f32) v / 320.0f;
    s32 t;

    D_803EF2E6 = v;
    if (last != v && D_803EF2E8 != NULL) {
        f32 p = D_8030D910 + (f32) v * D_8030D914;

        func_80260AB8(D_803EF2E8, 0x10, *(s32 *) &p);
    }
    func_802A0360(ratio, D_803EEF40, 2, 0);
    t = D_803EF324;
    if (t == 1 || t == -1) {
        t = 0;
    }
    if (t >= 0) {
        func_802A0360((f32) (0x20 - t) / 64.0f, D_803EEF40, 3, 0);
    } else {
        func_802A0360((f32) -t / 64.0f + 0.5f, D_803EEF40, 3, 0);
    }
    return ratio;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B98E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B9B4C.s")
