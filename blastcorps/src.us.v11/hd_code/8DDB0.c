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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2A40.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2C20.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8DDB0/func_802D2FA4.s")
