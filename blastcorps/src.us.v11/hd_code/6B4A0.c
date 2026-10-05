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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFC60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFFD4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDB40[]; /* this vehicle's state block */
extern u8 D_803ED840[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
/* Reads channel idx of base into out[0..7] (56040.c; asm convention in
 * tools_port/conventions.txt): out[0] = (s8) field 0x10, out[3] = field
 * 0x14, out[4] = (u16) field 0xC, ... */
void func_802A04BC(s32 idx, void *base, s32 *out);

/* Exit check for this vehicle type (called from func_8024B4B8 in hd.c, which
 * declares it void and returns the leftover v0): 0 when any of the bytes at
 * +0x96, +0x97, +0x98 of D_803EDB40 is 1; else 5 when channel 5 of
 * D_803ED840 has field 0x10 == 1 (the asm returns the channel index it left
 * in v0, which func_802A04BC preserves; kept as is); else 1. Returns s32
 * (the asm's v0). The asm saves and restores $gp; the v1 it leaves
 * (channel 5's field 0x10) isn't used by the C caller. */
s32 func_802B01DC(void) {
    s32 ch[8];

    if (D_803EDB40[0x96] == 1 || D_803EDB40[0x97] == 1 || D_803EDB40[0x98] == 1) {
        return 0;
    }
    func_802A04BC(5, D_803ED840, ch);
    if (ch[0] == 1) {
        return 5;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B01DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0254.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B02A0.s")

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
extern u32 D_803EDBE8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 1 at its position
 * D_803EDBE8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). */
s32 func_802B03B0(ZoneScanRegs *r) {
    return func_802ABD54(1, D_803EDBE8[0], D_803EDBE8[1], D_803EDBE8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B03B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B03F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B07DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
s32 func_802BCD80(s32 value);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);

/* When channel 5 of D_803ED840 is active (field 0x10 != 0) and
 * func_802BCD80(4) or func_802BCD80(5) is nonzero: sets its field 0x11 to 1,
 * restarts it with value 1 (func_802A0290) and sets D_802E8BE4 = 10,
 * D_802E8BE8 = 600. Returns channel 5's (u16) field 0xC as read at the start
 * (the asm leaves it in a3, which asm caller func_802B03F4 reads;
 * conventions.txt). The asm also clobbers s5 (10 / 600 scratch); its caller
 * keeps f12, f14 live (a mixed N64 build would need a thunk; the native port
 * won't). */
s32 func_802B0AAC(void) {
    s32 ch[8];

    func_802A04BC(5, D_803ED840, ch);
    if (ch[0] != 0 && (func_802BCD80(4) != 0 || func_802BCD80(5) != 0)) {
        func_802A03D4(D_803ED840, 5, 1);
        func_802A0290(D_803ED840, 5, 1);
        D_802E8BE4 = 10;
        D_802E8BE8 = 600;
    }
    return ch[4];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0AAC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0B3C.s")

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

extern u8 D_803EDB40[]; /* this vehicle's state block (the asm's $gp) */
extern f32 D_8030D8A8;

/* Speed (s16 at +0x76) scaled for the engine sound: rounded speed / 6.0 when
 * any of the bytes at +0x96/+0x97/+0x98 is 1, else speed / D_8030D8A8.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B03F4 keeps a0-a3 live across the call (a mixed N64 build would
 * need a thunk). Same shape as func_802B3F78 (6E200), func_802B7168 (71140),
 * func_802B83B0 (72B80), func_802CB564 (853D0), func_802CC844 (86F60),
 * func_802CD938 (88160), func_802D0710 (8AEE0); only the divisors differ. */
s32 func_802B0C74(void) {
    f32 div;
    s32 r;

    if (D_803EDB40[0x96] == 1 || D_803EDB40[0x97] == 1 || D_803EDB40[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D8A8;
    }
    CVT_W_S(r, *(s16 *) (D_803EDB40 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0C74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf, identical to func_802AFBA0 (69BB0):
 * D_803EBBF4 = D_803EBBF0 * 4, D_803ED3F6/7 = 40, 3.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802B03F4 keeps a0-a3 live across the call; a mixed N64 build would
 * need a thunk preserving those, the native port does not. */
void func_802B0CE8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0CE8.s")
#endif

/* func_802B0D44: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDB40[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EDBE8[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803EDB40[0..0xA5] plus the three words
 * D_803EDBE8[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B0D44(u8 *dst) {
    return func_802AC7DC(dst, D_803EDB40, D_803EDBE8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0D44.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B0D44: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B0D70(void *)`. */
void func_802B0D70(void *src) {
    func_802AC85C(src, D_803EDB40, D_803EDBE8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0D70.s")
#endif
