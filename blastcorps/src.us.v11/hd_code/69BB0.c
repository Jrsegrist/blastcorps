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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE370.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED460[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A02E4(s32 idx, void *base);

/* Stops channel 31 of D_803ED460 (func_802A02E4; called from hd.c's vehicle
 * switch). The asm leaves v0 = 31 and v1 = &D_803ED460; the C caller uses
 * neither. */
void func_802AE860(void) {
    func_802A02E4(0x1F, D_803ED460);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE860.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE888.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803643E0; /* player x (1/32 units) */
extern s32 D_803643E8; /* player z */

/* Zone test for mode *mode (asm: a3 in and out, result in a1; see
 * tools_port/conventions.txt). Mode 1: 1 when z/32 is in [0xCCD, 0xDB5).
 * Mode 2: 1 when x/32 is in [0x834, 0x961) and z/32 in [0x4B0, 0x5DD).
 * Otherwise 0. The asm leaves the last value it compared (z/32, or x/32 when
 * the x test fails) in a3, so *mode gets that; other modes leave it alone.
 * Register note: asm caller func_802AE888 keeps a0, a2, t0-t3, f12 and f14
 * live across the call (a mixed N64 build would need a thunk). */
s32 func_802AEB9C(s32 *mode) {
    s32 v;

    if (*mode == 1) {
        *mode = v = D_803643E8 >> 5;
        return v >= 0xCCD && v < 0xDB5;
    }
    if (*mode == 2) {
        *mode = v = D_803643E0 >> 5;
        if (v < 0x834 || v >= 0x961) {
            return 0;
        }
        *mode = v = D_803643E8 >> 5;
        return v >= 0x4B0 && v < 0x5DD;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEB9C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEC3C.s")

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
extern u32 D_803ED808[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0 at its position
 * D_803ED808..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802AEEC8 keeps t8, f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802AEE84(ZoneScanRegs *r) {
    return func_802ABD54(0, D_803ED808[0], D_803ED808[1], D_803ED808[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEE84.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEEC8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF340.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* this vehicle's state block (the asm's $gp) */
extern u8 D_802E8BDC;
extern s16 D_803ED820;   /* last channel-1 mode */
extern void *D_80367738;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
s32 func_8026A8E0(s32 lo, s32 hi);
void func_8029F9D4(s32 a, s32 b, void *base);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A04BC(s32 idx, void *base, s32 *out);

/* Field 0x10 of channel idx of D_803ED460 (1 = running). */
static s32 AnimChannelState(s32 idx) {
    s32 ch[8];

    func_802A04BC(idx, D_803ED460, ch);
    return ch[0];
}

/* Per-frame animation update of vehicle 0 (asm caller func_802AEEC8; the asm
 * reads the state block through $gp = D_803ED760, read directly here). Byte
 * +0xA1 records whether the vehicle was moving last frame.
 * Stopped (speed +0x76 == 0): on the first stopped frame channel 1 is stopped
 * and channel 31 restarted with 70; then, when none of channels 31, 2, 3, 4 is
 * running and func_8026A8E0(0, 20) is 0, one of channels 3 / 4 / 2 (by
 * func_8026A8E0(0, 2) = 0 / 1 / else) is started.
 * Moving: on the first moving frame channel 1 is set to mode 2 and the running
 * one of channels 2, 3, 4 (else 3, after resetting it) is handed over via
 * func_8029F9D4 and channel 31 restarted with 40. Then, unless channel 31 is
 * running, channel 1 gets the direction (speed < 0) and |speed| / 11 and is
 * restarted, and (outside levels 0x31 and 0x26) a change of channel 1's mode
 * to 2 or 6 starts sound 0x14 / 0x15.
 * Register note: the asm clobbers s5 (and f20/f30 through func_8029F9D4);
 * conventions.txt. Asm caller func_802AEEC8 reads t6, t8, f12 and f14 after
 * the call, which the asm passes through except where its C callees change
 * them (func_8026A8E0 leaves its seeds in t6/t8): a mixed N64 build would need
 * a thunk, the native port won't. */
void func_802AF4BC(void) {
    u8 *base = D_803ED460;
    s32 speed = *(s16 *) (D_803ED760 + 0x76);
    s32 r;
    s32 ch[8];

    if (speed == 0) {
        if (D_803ED760[0xA1] != 0) {
            func_802A02E4(1, base);
            func_802A0360(0.0f, base, 3, 0);
            func_8029F9D4(1, 3, base);
            func_802A039C(base, 0x1F, 0x46);
            func_802A03D4(base, 0x1F, 0);
            func_802A040C(base, 0x1F, 0);
            func_802A0290(base, 0x1F, 1);
        }
        D_803ED760[0xA1] = 0;
        if (AnimChannelState(0x1F) == 1 || AnimChannelState(2) == 1 || AnimChannelState(3) == 1 ||
            AnimChannelState(4) == 1) {
            return;
        }
        if (func_8026A8E0(0, 0x14) != 0) {
            return;
        }
        r = func_8026A8E0(0, 2);
        if (r == 0) {
            func_802A0360(0.0f, base, 3, 0);
            func_802A0290(base, 3, 1);
        } else if (r == 1) {
            func_802A0360(0.0f, base, 4, 0);
            func_802A0290(base, 4, 1);
        } else {
            func_802A0360(0.0f, base, 2, 0);
            func_802A0290(base, 2, 1);
        }
        return;
    }

    if (D_803ED760[0xA1] != 1) {
        func_802A0360(0.0f, base, 1, 2);
        if (AnimChannelState(2) != 0) {
            func_8029F9D4(2, 1, base);
            func_802A02E4(2, base);
        } else if (AnimChannelState(3) != 0) {
            func_8029F9D4(3, 1, base);
            func_802A02E4(3, base);
        } else if (AnimChannelState(4) != 0) {
            func_8029F9D4(4, 1, base);
            func_802A02E4(4, base);
        } else {
            func_802A0360(0.0f, base, 3, 0);
            func_8029F9D4(3, 1, base);
        }
        func_802A039C(base, 0x1F, 0x28);
        func_802A03D4(base, 0x1F, 0);
        func_802A040C(base, 0x1F, 0);
        func_802A0290(base, 0x1F, 1);
    }
    if (AnimChannelState(0x1F) != 1) {
        speed = *(s16 *) (D_803ED760 + 0x76);
        if (speed < 0) {
            func_802A03D4(base, 1, 1);
        } else {
            func_802A03D4(base, 1, 0);
        }
        if (D_802E8BDC != 0x31 && D_802E8BDC != 0x26) {
            s32 old = D_803ED820;
            s32 mode;

            func_802A04BC(1, base, ch);
            mode = ch[6];
            D_803ED820 = mode;
            if (mode != old) {
                if (mode == 2) {
                    func_80260650(D_80367738, 0x14, NULL);
                } else if (mode == 6) {
                    func_80260650(D_80367738, 0x15, NULL);
                }
            }
        }
        if (speed < 0) {
            speed = -speed;
        }
        func_802A039C(base, 1, (u32) speed / 11);
        func_802A0290(base, 1, -1);
    }
    D_803ED760[0xA1] = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF4BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFA64.s")

/* func_802AFB84: sets $s3 = 0x8c directly (`addiu $s3, $zero, 0x8c`)
 * inside the same dead 8-byte `sd $ra` frame as func_802BBE10/
 * func_802C8AF0/func_802CE9A4/func_802AC284 - same confirmed hand-
 * written family, and doubly so here since C has no way to pin a value
 * to a specific callee-saved register by name regardless. Value is
 * never read or returned; likely a vestigial/debug leftover in the
 * original assembly. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 0x8C (asm: in s3, which asm caller func_802AEEC8 reads; see
 * tools_port/conventions.txt). That caller keeps a0-a3, f12 and f14 live
 * across the call (a mixed N64 build would need a thunk). */
s32 func_802AFB84(void) {
    return 0x8C;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFB84.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf: D_803EBBF4 = D_803EBBF0 * 4, then the byte pair
 * D_803ED3F6/7 = 40, 3. Same shape as func_802B0CE8 (6B4A0), func_802B2900
 * (6C5E0) and, with other constants, func_802B8424 (72B80), func_802B5814
 * (6E200), func_802B7240 (71140).
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802AEEC8 keeps a0-a3, f12 and f14 live across the call; a mixed N64
 * build would need a thunk preserving those, the native port does not. */
void func_802AFBA0(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBA0.s")
#endif

/* func_802AFBFC: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803ED808[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803ED760[0..0xA5] plus the three words
 * D_803ED808[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802AFBFC(u8 *dst) {
    return func_802AC7DC(dst, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802AFBFC: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802AFC28(void *)`. */
void func_802AFC28(void *src) {
    func_802AC85C(src, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFC28.s")
#endif
