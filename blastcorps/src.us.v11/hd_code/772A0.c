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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBA60.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

/* Enter vehicle type 7: D_8036444C/50 = 3000, 0, then func_802C4310(arg0,
 * 0x20) (arg0 passes straight through; hd.c calls this with no arguments
 * and func_802C4310 ignores it). The asm also points $gp at D_803EFDF0 and
 * leaves it there (conventions.txt: clobbers gp); C code doesn't use $gp.
 * Same shape as func_802C8AB0 (83910) and func_802B76AC (72B80). */
void func_802BBDC8(s32 arg0) {
    D_8036444C = 3000;
    D_80364450 = 0;
    func_802C4310(arg0, 0x20);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBDC8.s")
#endif

/* func_802BBE10: `return 1;` wrapped in a dead `addiu sp,sp,-8`/`sd
 * $ra,($sp)`/`ld $ra,($sp)`/`addiu sp,sp,8` frame that saves/restores
 * nothing. Confirmed via probe compiles that this is NOT reachable from
 * any plausible C: IDO's own codegen for a real function call (verified
 * directly - a genuine `void f(void) { g(); }`) always uses the 24-byte
 * o32 argument-shadow frame (`sw $ra`), never this 8-byte `sd $ra` style;
 * a leaf with no calls at all (like this one) gets no frame whatsoever
 * unless a local variable is declared, and a declared local only
 * reserves the frame, it doesn't explain a `return 1;` needing one in
 * the first place. Same hand-written-leaf-stub character as init's
 * __osGetSR and this segment's COP0 stubs. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): always returns 1
 * (vehicle-type 7 "can exit" check; same as func_802C8AF0). 00000.c
 * declares it void and ignores the result, but the asm returns 1 in v0. */
s32 func_802BBE10(void) {
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EFEA8; /* save copy pair (func_802A7764) */
extern u64 *D_803EFEAC;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 7 (called from hd.c): func_802A7764(D_803EFEA8,
 * D_803EFEAC, 0x800), then stop the looping sounds (func_802C444C). The asm
 * points $gp at D_803EFDF0 around the calls and restores it. */
void func_802BBE2C(void) {
    func_802A7764(D_803EFEA8, D_803EFEAC, 0x800);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE2C.s")
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
extern u32 D_803EFE98[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 7 at its position
 * D_803EFE98..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BBEB8 keeps a0, f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BBE74(ZoneScanRegs *r) {
    return func_802ABD54(7, D_803EFE98[0], D_803EFE98[1], D_803EFE98[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBEB8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef IO802A6274_DEFINED
#define IO802A6274_DEFINED
/* The three registers func_802A6274 (60F60.c) takes and may hand back changed. */
typedef struct {
    /* 0x0 */ s32 a3;
    /* 0x4 */ s32 t6;
    /* 0x8 */ s32 s1;
} Io802A6274;
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
#endif
extern u8 D_803EFDF0[];  /* this vehicle's state block (the asm's $gp) */
extern u8 D_803EFEC9;    /* countdown */
extern u8 D_80370C1C;    /* flag tested when the speed is <= 0 */
extern u8 D_80370C23;    /* flag tested when the speed is > 0 */
extern u8 D_802C2984[];  /* definition handed to func_802A6274 (7D9D0 text blob) */
extern void *D_80367738; /* sound player */
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802C4584(s32 level);
void func_802C4724(s32 sfx);

/* Countdown D_803EFEC9: when nonzero it just counts down. At zero, if the
 * flag for the speed's sign (s16 at +0x76: > 0 -> D_80370C23, else
 * D_80370C1C) is set, two func_802A6274 records are set up (def D_802C2984,
 * data 0x9C40, tag 0, type 1 at (7, 2, 1) and (7, 3, 1)), sound 0x29 is
 * played (func_80260650) and the countdown restarts at 1. Then always
 * func_802C4584(|speed >> 4|) and func_802C4724(0x21).
 * Register convention: the asm passes t6, t7, s0-s4 through to
 * func_802A6274 (t6 and s1 in its in/out block); $gp (= D_803EFDF0) is read
 * as the global. It leaves func_802A6274's s1 and changes s5
 * (conventions.txt: clobbers). */
void func_802BC2C8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 v;

    if (D_803EFEC9 != 0) {
        D_803EFEC9--;
    } else if ((*(s16 *) (D_803EFDF0 + 0x76) > 0) ? (D_80370C23 != 0) : (D_80370C1C != 0)) {
        io.a3 = 0;
        io.t6 = t6;
        io.s1 = s1;
        func_802A6274(&io, D_802C2984, 0x9C40, 1, 7, 2, 1, t7, s0, s2, s3, s4, 0);
        io.a3 = 0;
        func_802A6274(&io, D_802C2984, 0x9C40, 1, 7, 3, 1, t7, s0, s2, s3, s4, 0);
        func_80260650(D_80367738, 0x29, NULL);
        D_803EFEC9 = 1;
    }
    v = *(s16 *) (D_803EFDF0 + 0x76) >> 4;
    if (v < 0) {
        v = -v;
    }
    func_802C4584(v);
    func_802C4724(0x21);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC2C8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC3D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm caller func_802BBEB8 relies on
 * a0-a3 being preserved (mixed N64 build would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802BC578(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC578.s")
#endif
