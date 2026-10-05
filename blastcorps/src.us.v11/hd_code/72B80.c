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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7754.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B77A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7980.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7A88.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7F98.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8C18.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8D04.s")

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
