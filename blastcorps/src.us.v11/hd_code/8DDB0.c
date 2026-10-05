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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2570.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D291C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_80269258(void);

/* Calls func_80269258, saving gp around it (the asm caller func_802D291C
 * uses gp as its base pointer). The survey lists a0, a2, a3, t6, t7, f12 and
 * f14 as read by that caller afterwards; they're whatever func_80269258
 * leaves, nothing this function sets. */
void func_802D2A40(void) {
    func_80269258();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2A40.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
double sqrt(double);
#pragma intrinsic(sqrt)

/* func_802ABCDC (62740; points in t3-t5 and t6,t7,s0, result in s1) can't be
 * called from C: this is its logic (same macro as in 77E20.c). The distance
 * between two points, rounded to nearest (ties to even) as cvt.l.d does; the
 * squares are summed as s64 and converted as hi * 2^32 + lo (one rounding,
 * like cvt.d.l; a plain cast would call __ll_to_d, which the harness can't run). */
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

extern void *D_803FCD64; /* sound state, NULL = none */
extern s32 D_803643F8;   /* listener x, y, z (<< 11) */
extern s32 D_803643FC;
extern s32 D_80364400;
extern s32 D_803FCD48;   /* sound source x, y, z */
extern s32 D_803FCD4C;
extern s32 D_803FCD50;
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);

/* Distance attenuation: if D_803FCD64 is set, sets its volume (parameter 8)
 * to 0x7FFF - max(d - 0x3200, 0) / 4, clamped at 0, where d is the distance
 * from (D_803643F8, D_803643FC, D_80364400) >> 11 to D_803FCD48/4C/50.
 * The asm's dead `sub` (x - D_803FCD48) and its `addi` trap on overflow; C
 * doesn't. Register note: the asm saves and restores every register; asm
 * caller func_802D291C keeps a0, a2, a3, t6, t7, f12 and f14 live across the
 * call (a mixed N64 build would need a thunk; the native port doesn't). */
void func_802D2A74(void) {
    s32 dist;
    s32 vol;

    if (D_803FCD64 == NULL) {
        return;
    }
    DIST3_802ABCDC(dist, D_803643F8 >> 11, D_803643FC >> 11, D_80364400 >> 11,
                   D_803FCD48, D_803FCD4C, D_803FCD50);
    dist -= 0x3200;
    if (dist < 0) {
        dist = 0;
    }
    vol = 0x7FFF - (dist >> 2);
    if (vol < 0) {
        vol = 0;
    }
    func_80260AB8(D_803FCD64, 8, vol);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2A74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* The three registers func_802A6274 (60F60.c) takes and may hand back changed. */
typedef struct {
    /* 0x0 */ s32 a3;
    /* 0x4 */ s32 t6;
    /* 0x8 */ s32 s1;
} Io802A6274;

s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
s32 func_802ABC88(s32 id, s32 n, u8 **recOut);           /* 62740 */
void func_802A0290(void *base, s32 idx, s32 val);         /* 56040 */
void func_802A039C(void *base, s32 idx, s32 val);         /* 56040 */
void func_802A03D4(void *base, s32 idx, s32 val);         /* 56040 */
void *func_80260650(void *arg0, s16 arg1, void *arg2);

extern void *D_80367738; /* sound player */
extern u8 D_803FC9A0[];
extern u8 D_803FCD70;    /* event this frame */
extern s32 D_803FCD60;
extern u8 D_8036B964;
extern u8 D_803FCD72;
extern u8 D_803FCD73;
extern u8 D_803FCD74;
extern u8 D_803FCD76;
extern u8 D_803FCD77;
extern u8 D_803FCD78;
extern u8 D_803FCD79;
extern u8 D_803FCD7A;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern u8 D_802C28E4[]; /* effect definitions in the 7D9D0 text blob */
extern u8 D_802C3804[];

/* Spawns effect def at D_803EBC10 record (0xFD, n) (func_802A6274 type 1 with
 * z = 1, tag 0, b35 0). For that type func_802A6274 ignores its other fields;
 * the asm passes whatever its caller left in t6, t7, s0-s4 there, the C 0. */
#define SPAWN_AT_FD(def, data, n)                                         \
    {                                                                     \
        Io802A6274 _io;                                                   \
                                                                          \
        _io.a3 = 0;                                                       \
        _io.t6 = 0;                                                       \
        _io.s1 = 0;                                                       \
        func_802A6274(&_io, (def), (data), 1, 0xFD, (n), 1, 0, 0, 0, 0, 0, 0); \
    }

/* Per-frame event handling for this vehicle (id 0xFD in D_803EBC10): event
 * D_803FCD70 2 sets D_803FC9A0 entry 1's fields (func_802A039C/03D4/0290)
 * and plays sound 0x66; 3 plays 0x78; 0xB sets D_803FCD77/7A/78 and plays
 * 0x83; 0xC plays 0x7F into D_803FCD64 and clears D_803FCD7A; 0xD clears
 * D_803FCD78. Unless D_8036B964, each of the three one-shot flags
 * D_803FCD72..74 still clear plays sound 6, 6, 8 (and gets set) once
 * D_803FCD60 (read once) reaches word +4 of D_803EBC10 record (0xFD, 1..3).
 * D_803FCD7A forces D_802E8BE4 = 20 and D_802E8BE8 = 400. D_803FCD78 and
 * D_803FCD77 each run a 5-frame countdown (D_803FCD79 / D_803FCD76) that
 * spawns effects at the vehicle's points 4 (D_802C28E4) or 4, 5, 6
 * (D_802C3804). Only caller func_802D291C (asm); the survey's t6/t7/s0/s1
 * "outputs" are what func_8029E558 sets after it, not read from here. */
void func_802D2C20(void) {
    u8 *rec;
    s32 level;

    switch (D_803FCD70) {
        case 2:
            func_802A039C(D_803FC9A0, 1, 1);
            func_802A03D4(D_803FC9A0, 1, 0);
            func_802A0290(D_803FC9A0, 1, 1);
            func_80260650(D_80367738, 0x66, NULL);
            break;
        case 3:
            func_80260650(D_80367738, 0x78, NULL);
            break;
        case 0xB:
            D_803FCD77 = 1;
            D_803FCD7A = 1;
            D_803FCD78 = 1;
            func_80260650(D_80367738, 0x83, NULL);
            break;
        case 0xC:
            func_80260650(D_80367738, 0x7F, &D_803FCD64);
            D_803FCD7A = 0;
            break;
        case 0xD:
            D_803FCD78 = 0;
            break;
    }

    if (D_8036B964 == 0) {
        level = D_803FCD60;
        if (D_803FCD72 == 0) {
            func_802ABC88(0xFD, 1, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 6, NULL);
                D_803FCD72 = 1;
            }
        }
        if (D_803FCD73 == 0) {
            func_802ABC88(0xFD, 2, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 6, NULL);
                D_803FCD73 = 1;
            }
        }
        if (D_803FCD74 == 0) {
            func_802ABC88(0xFD, 3, &rec);
            if (!(level < ((s32 *) rec)[1])) {
                func_80260650(D_80367738, 8, NULL);
                D_803FCD74 = 1;
            }
        }
    }

    if (D_803FCD7A != 0) {
        D_802E8BE4 = 20;
        D_802E8BE8 = 400;
    }
    if (D_803FCD78 != 0) {
        if (D_803FCD79 != 0) {
            D_803FCD79 -= 1;
        } else {
            D_803FCD79 = 4;
            SPAWN_AT_FD(D_802C28E4, 0x107AC0, 4);
        }
    }
    if (D_803FCD77 != 0) {
        if (D_803FCD76 != 0) {
            D_803FCD76 -= 1;
        } else {
            D_803FCD76 = 4;
            SPAWN_AT_FD(D_802C3804, 0x53020, 4);
            SPAWN_AT_FD(D_802C3804, 0x53020, 5);
            SPAWN_AT_FD(D_802C3804, 0x5CC60, 6);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2C20.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2FA4.s")
