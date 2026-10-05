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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5900.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEA90[]; /* this vehicle's state block */
extern u8 D_803EE790[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
extern u8 D_802C2208[]; /* keys of this vehicle's func_802A06B4 entries */
extern u8 D_802C226C[];
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0508(s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);
void func_802C4310(s32 arg0, s32 arg1);

/* Enter vehicle type 5 (called from 00000.c / 17210.c): clears byte 0x99 of
 * the state block, sets fields 0x14 / 0x11 / 0x12 of animation channels 1, 2,
 * 4, 5 of D_803EE790 (channel 2 also gets 0.5f / 0 via func_802A0360) and
 * restarts each with -1, sets the D_802C2208 and D_802C226C entries' fields to
 * 0 and restarts them, D_8036444C/50 = 3400, 300, then func_802C4310(a0, 5)
 * with a0 = D_803EE790 left over from the setters (func_802C4310 ignores it).
 * The asm points $gp at D_803EEA90 and leaves it there (conventions.txt:
 * clobbers gp) and returns with v1 = -1 (the last setter's v1); C callers
 * ignore both. */
void func_802B5CD8(void) {
    D_803EEA90[0x99] = 0;
    func_802A039C(D_803EE790, 1, 0);
    func_802A03D4(D_803EE790, 1, 0);
    func_802A040C(D_803EE790, 1, 0);
    func_802A0290(D_803EE790, 1, -1);
    func_802A039C(D_803EE790, 2, 0);
    func_802A03D4(D_803EE790, 2, 0);
    func_802A040C(D_803EE790, 2, 1);
    func_802A0360(0.5f, D_803EE790, 2, 0);
    func_802A0290(D_803EE790, 2, -1);
    func_802A039C(D_803EE790, 4, 0);
    func_802A03D4(D_803EE790, 4, 0);
    func_802A040C(D_803EE790, 4, 1);
    func_802A0290(D_803EE790, 4, -1);
    func_802A039C(D_803EE790, 5, 0);
    func_802A03D4(D_803EE790, 5, 0);
    func_802A040C(D_803EE790, 5, 1);
    func_802A0290(D_803EE790, 5, -1);
    func_802A05D0((s32) D_802C2208, 0);
    func_802A05F8((s32) D_802C2208, 0);
    func_802A0620((s32) D_802C2208, 0);
    func_802A0508((s32) D_802C2208, -1);
    func_802A05D0((s32) D_802C226C, 0);
    func_802A05F8((s32) D_802C226C, 0);
    func_802A0620((s32) D_802C226C, 0);
    func_802A0508((s32) D_802C226C, -1);
    D_8036444C = 0xD48;
    D_80364450 = 0x12C;
    func_802C4310((s32) D_803EE790, 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5CD8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEA90[]; /* this vehicle's state block */

/* Exit check for vehicle type 5, same shape as func_802B2EF8 (6E200): returns
 * 1 when none of the bytes at +0x96, +0x97, +0x98 equals 1, else 0 (s32; the
 * C caller in hd.c declares it void and returns the leftover v0). */
s32 func_802B5F04(void) {
    if (D_803EEA90[0x96] == 1 || D_803EEA90[0x97] == 1 || D_803EEA90[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5F04.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EEB48; /* save copy pair (func_802A7764) */
extern u64 *D_803EEB4C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 5 (called from hd.c): zeroes the speed (s16 at +0x76 of
 * the state block), func_802A7764(D_803EEB48, D_803EEB4C, 0x800), then
 * func_802C444C(). The asm saves and restores $gp. */
void func_802B5F60(void) {
    *(s16 *) (D_803EEA90 + 0x76) = 0;
    func_802A7764(D_803EEB48, D_803EEB4C, 0x800);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5F60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5FAC.s")

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
extern u32 D_803EEB38[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 5 at its position
 * D_803EEB38..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). */
s32 func_802B60BC(ZoneScanRegs *r) {
    return func_802ABD54(5, D_803EEB38[0], D_803EEB38[1], D_803EEB38[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B60BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6100.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B618C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6294.s")

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

extern u32 D_803EEB38[]; /* [0], [2]: trail x, z */
extern f32 D_803EEB54;   /* left/right balance, 0.5 = centred */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803EEA90). The two
 * sides' a/b values are 40 -+ d, where d = round(|D_803EEB54 - 0.5| * 2 *
 * 20): a = 40 - d, b = 40 + d when D_803EEB54 <= 0.5, else (NaN included)
 * a = 40 + d, b = 40 - d. Then func_8027BE7C(3, y, 400, -400, -400,
 * -400, D_803EEB38[0], D_803EEB38[2], yaw, 5, a, b, 0). The value is only
 * computed when the trail is drawn.
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802B6294 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802B69F8(void) {
    f32 bal;
    s32 d;
    s32 a;
    s32 b;

    if (D_803EEA90[0x99] == 0 || D_803EEA90[0x98] == 1 || D_803EEA90[0x50] >= 3 || D_803EEA90[0x9B] != 0) {
        return;
    }
    bal = D_803EEB54;
    if (bal <= 0.5f) {
        CVT_W_S(d, (0.5f - bal) * 2.0f * 20.0f);
        b = d + 40;
        a = 40 - d;
    } else {
        CVT_W_S(d, (bal - 0.5f) * 2.0f * 20.0f);
        b = 40 - d;
        a = d + 40;
    }
    func_8027BE7C(3, *(s32 *) (D_803EEA90 + 0x1C), 400, -400, -400, -400, D_803EEB38[0], D_803EEB38[2],
                  *(u16 *) (D_803EEA90 + 0x4E), 5, a, b, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B69F8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6C28.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B7030.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_8030D8FC;

/* Speed (s16 at +0x76 of D_803EEA90, the asm's $gp) / 6.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D8FC, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B6294 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802B7168(void) {
    f32 div;
    s32 r;

    if (D_803EEA90[0x96] == 1 || D_803EEA90[0x97] == 1 || D_803EEA90[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D8FC;
    }
    CVT_W_S(r, *(s16 *) (D_803EEA90 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B7168.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0.5 +- |speed| / 240 * 0.5 (plus when side != 0, minus otherwise), speed
 * being the s16 at +0x76 of D_803EEA90 (the asm's $gp). The asm takes side
 * in s2, returns the value in f2 and leaves s3 = 240 (see
 * tools_port/conventions.txt). Its asm caller func_802B6C28 keeps a1-a3, f12
 * and f14 live (a mixed N64 build would need a thunk). Same shape as
 * func_802CB3C8 (853D0, divisor 280). */
f32 func_802B71DC(s32 side) {
    s32 speed = *(s16 *) (D_803EEA90 + 0x76);
    f32 v;

    if (speed < 0) {
        speed = -speed;
    }
    v = (f32) speed / 240.0f * 0.5f;
    if (side != 0) {
        return v + 0.5f;
    }
    return 0.5f - v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B71DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;
extern u8 D_80370C1A; /* L_TRIG held */
extern u8 D_80370C1B;
extern s16 D_803EEB58;

/* Vehicle-module setup leaf (shape of func_802AFBA0 in 69BB0, other
 * constants): D_803EBBF4 = D_803EBBF0 * 2, D_803ED3F6/7 = 60, 2, then
 * D_803EEB58 = 3200 if either button flag D_80370C1A/B is set, else 9000.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm callers keep
 * registers live across the call (func_802B618C: t3, t4, f12, f14;
 * func_802B6294: a0-a3); a mixed N64 build would need a thunk preserving
 * those, the native port does not. */
void func_802B7240(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 2;
    D_803EEB58 = (D_80370C1A != 0 || D_80370C1B != 0) ? 3200 : 9000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B7240.s")
#endif

/* func_802B72DC: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEA90[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EEB38[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803EEA90[0..0xA5] plus the three words
 * D_803EEB38[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B72DC(u8 *dst) {
    return func_802AC7DC(dst, D_803EEA90, D_803EEB38);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B72DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B72DC: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B7308(void *)`. */
void func_802B7308(void *src) {
    func_802AC85C(src, D_803EEA90, D_803EEB38);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B7308.s")
#endif
