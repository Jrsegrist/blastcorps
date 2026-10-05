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
extern u8 D_803F8AA0[];  /* this vehicle's state block (the asm's $gp) */
extern u8 D_803F87A0[];  /* its animation channel table */
extern u32 D_803F8B48[]; /* x, y, z */
extern u8 *D_803F8B54;   /* its model */
extern u64 *D_803F8B58;  /* save copy pair */
extern u64 *D_803F8B5C;
extern f32 D_803F8B60;
extern u16 D_803F8B72;
extern u8 D_803F8B76;
extern u8 D_803F8B77;
extern s8 D_803F8B78;
extern u8 D_803F8B7A;
extern u8 D_803F8B7B;
extern u8 D_803F8B7C;
extern u8 D_803F8B7D;
extern u8 D_803F8B7E;
extern u8 *D_80358070; /* allocation pointer for the save copies */
extern u8 D_80364A6B;
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_8029E558(u8 *base, u8 *other, void *ch);
s32 func_8029F85C(u32 *bufA, u32 *bufB, void *ch, u8 *hdr);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0320(s32 idx, void *base);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_802A754C(u8 *veh);
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh,
                   TriSideOut *f);
void func_802AA838(u8 *src, u8 *dst, s32 off);
void func_802CA4E0(void);

/* Load vehicle 10 (the 5CB60 dispatcher func_802A350C, record type 10):
 * model `model` (D_803F8B54), save copies D_803F8B58 / D_803F8B5C taken
 * 0x700 bytes each from D_80358070 (func_802A1388(10, 0, copies, model)),
 * record reset (func_802A754C), wheel table +0x52..0x68, position (x, y,
 * z), heading `heading` at +0x4C/+0x4E/+0x74, D_803F8B60 = 0.5, ground
 * heights (func_802A992C, key 10, fp), the model's animation channels
 * (func_8029F85C, channel 0 set to 100 / 0 / 0 / 0.0 / restart, run on both
 * copies with func_8029E558 and func_802A0320 between), tuning values
 * +0x78..0x94, its flags and counters, parts (func_8029C354, scale 0x3E80),
 * func_80258230(10, 100, 45, 45), one update with +0x9A set
 * (func_802CA4E0), copy the model matrix from the second copy to the first
 * (func_802AA838) and D_80364A6B = 1.
 * Register convention (conventions.txt): x t7, y s3, z s0, heading s1,
 * model s2, fp (handed to func_802A992C, which stores it into D_803ED3F2:
 * the dispatcher's leftover $fp in the game). The FP block func_802A992C
 * passes through starts at 0 here (not read first). The asm saves t0-t5
 * (asm caller func_802A350C keeps t1, t2 live: a mixed N64 build would need
 * a thunk) and leaves s2-s4, fp and f12-f26 as its callees leave them
 * (survey: read by func_802A350C's next records; not modelled) and $gp =
 * the block. Adds are trapping in the asm. */
void func_802C9B90(s32 x, s32 y, s32 z, s32 heading, u8 *model, s32 fp) {
    TriSideOut f;
    u8 *m;

    D_803F8B54 = model;
    D_803F8B58 = (u64 *) D_80358070;
    D_803F8B5C = (u64 *) (D_80358070 + 0x700);
    D_80358070 += 0xE00;
    func_802A1388(10, 0, (s32) D_803F8B58, (s32) D_803F8B5C, model);
    func_802A754C(D_803F8AA0);
    *(s16 *) (D_803F8AA0 + 0x52) = 0x104;
    *(s16 *) (D_803F8AA0 + 0x54) = 0x140;
    *(s16 *) (D_803F8AA0 + 0x56) = -0x104;
    *(s16 *) (D_803F8AA0 + 0x58) = 0x140;
    *(s16 *) (D_803F8AA0 + 0x5A) = 0x104;
    *(s16 *) (D_803F8AA0 + 0x5C) = -0x140;
    *(s16 *) (D_803F8AA0 + 0x5E) = 0x12C;
    *(s16 *) (D_803F8AA0 + 0x60) = 0x154;
    *(s16 *) (D_803F8AA0 + 0x62) = -0x12C;
    *(s16 *) (D_803F8AA0 + 0x64) = 0x154;
    *(s16 *) (D_803F8AA0 + 0x66) = 0x12C;
    *(s16 *) (D_803F8AA0 + 0x68) = -0x154;
    D_803F8B48[0] = x;
    D_803F8B48[1] = y;
    D_803F8B48[2] = z;
    D_803F8B7C = 0;
    D_803F8B7E = 0;
    D_803F8B60 = 0.5f;
    *(s16 *) (D_803F8AA0 + 0x4C) = heading;
    *(s16 *) (D_803F8AA0 + 0x4E) = heading;
    *(s16 *) (D_803F8AA0 + 0x74) = heading;
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A992C((s16 *) (D_803F8AA0 + 0x52), D_803F8B48[1], x, z, (s32 *) (D_803F8AA0 + 4),
                  (s32 *) &D_803F8B48[1], (s16 *) (D_803F8AA0 + 0x4C), 10, fp, D_803F8AA0, &f);
    func_8029F85C((u32 *) D_803F8B5C, (u32 *) D_803F8B58, D_803F87A0, D_803F8B54);
    func_802A039C(D_803F87A0, 0, 100);
    func_802A03D4(D_803F87A0, 0, 0);
    func_802A040C(D_803F87A0, 0, 0);
    func_802A0480(0.0f, D_803F87A0, 0, 0);
    func_802A0290(D_803F87A0, 0, 1);
    func_8029E558((u8 *) D_803F8B58, (u8 *) D_803F8B5C, D_803F87A0);
    func_802A0320(0, D_803F87A0);
    func_802A0290(D_803F87A0, 0, 1);
    func_8029E558((u8 *) D_803F8B5C, (u8 *) D_803F8B58, D_803F87A0);
    *(s16 *) (D_803F8AA0 + 0x78) = -0x96;
    *(s16 *) (D_803F8AA0 + 0x7A) = 0;
    *(s16 *) (D_803F8AA0 + 0x7C) = 2;
    *(s16 *) (D_803F8AA0 + 0x7E) = 0;
    *(s16 *) (D_803F8AA0 + 0x80) = 0xB4;
    *(s16 *) (D_803F8AA0 + 0x82) = 8;
    *(s16 *) (D_803F8AA0 + 0x84) = 0xB4;
    *(s16 *) (D_803F8AA0 + 0x86) = 0xC8;
    *(s16 *) (D_803F8AA0 + 0x88) = 4;
    *(s16 *) (D_803F8AA0 + 0x8A) = 0xC8;
    *(s16 *) (D_803F8AA0 + 0x8C) = 0xFA;
    *(s16 *) (D_803F8AA0 + 0x8E) = 2;
    *(s16 *) (D_803F8AA0 + 0x90) = 0xFA;
    *(s16 *) (D_803F8AA0 + 0x92) = 0x104;
    *(s16 *) (D_803F8AA0 + 0x94) = 2;
    D_803F8B78 = 0;
    D_803F8B7B = 0;
    D_803F8B7A = 0;
    D_803F8B76 = 0;
    D_803F8B77 = 0x32;
    D_803F8B7D = 0;
    D_803F8B72 = 0;
    m = D_803F8B54;
    func_8029C354(10, m + *(s32 *) (m + 4), m + *(s32 *) (m + 8), 0x3E80);
    func_80258230(10, 0x64, 0x2D, 0x2D);
    D_803F8AA0[0x9A] = 1;
    func_802CA4E0();
    D_803F8AA0[0x9A] = 0;
    m = D_803F8B54;
    func_802AA838((u8 *) D_803F8B5C, (u8 *) D_803F8B58, *(s32 *) (m + *(s32 *) (m + 0x18) + 4));
    D_80364A6B = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9B90.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8AA0[]; /* this vehicle's state block (the asm's $gp) */
extern u8 D_803F87A0[]; /* its animation channel table (Unk8029DEA0Entry, 56040.c) */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);

/* Enter this vehicle (vehicle type 10; called from hd.c and 17210.c): clears
 * the byte at +0x99, sets the pair D_8036444C / D_80364450 to (0xD48, 1000),
 * starts sound 0x94 (func_802C4310; the asm passes its caller's a0 through
 * as the ignored first argument) and sets up channels 1-3 (0 / 0 / 0|1,
 * restart with -1) and 4-5 (8|8 / 0 / 0, 1.0 with 1).
 * The asm points $gp at D_803F8AA0 and leaves it there (conventions.txt:
 * clobbers gp) and leaves v1 = 1 (func_802A0480 preserves it); the C callers
 * declare it void and use neither. Same shape as func_802C5714 (7FB50). */
void func_802C9F54(void) {
    D_803F8AA0[0x99] = 0;
    D_8036444C = 0xD48;
    D_80364450 = 1000;
    func_802C4310(0, 0x94);
    func_802A039C(D_803F87A0, 1, 0);
    func_802A03D4(D_803F87A0, 1, 0);
    func_802A040C(D_803F87A0, 1, 0);
    func_802A0290(D_803F87A0, 1, -1);
    func_802A039C(D_803F87A0, 2, 0);
    func_802A03D4(D_803F87A0, 2, 0);
    func_802A040C(D_803F87A0, 2, 1);
    func_802A0290(D_803F87A0, 2, -1);
    func_802A039C(D_803F87A0, 3, 0);
    func_802A03D4(D_803F87A0, 3, 0);
    func_802A040C(D_803F87A0, 3, 1);
    func_802A0290(D_803F87A0, 3, -1);
    func_802A039C(D_803F87A0, 4, 8);
    func_802A03D4(D_803F87A0, 4, 0);
    func_802A040C(D_803F87A0, 4, 0);
    func_802A0480(1.0f, D_803F87A0, 4, 1);
    func_802A039C(D_803F87A0, 5, 8);
    func_802A03D4(D_803F87A0, 5, 0);
    func_802A040C(D_803F87A0, 5, 0);
    func_802A0480(1.0f, D_803F87A0, 5, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9F54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 10
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1 or
 * D_803F8B7C (= its byte 0xDC) is nonzero, else 1. Same shape as
 * func_802CBB60, func_802CCCD8, func_802CFA58, func_802D0B90. 00000.c
 * declares it void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F8AA0[];
extern u8 D_803F8B7C;

s32 func_802CA140(void) {
    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1 || D_803F8B7C != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA140.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803F8B58; /* save copy pair (func_802A7764) */
extern u64 *D_803F8B5C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Teardown for this vehicle (called from func_8024B188 in hd.c): clears the
 * s16 at +0x76, func_802A7764(D_803F8B58, D_803F8B5C, 0x700) and stops its
 * sounds (func_802C444C). The asm saves and restores $gp. Same shape as
 * func_802C5688 (7FB50). */
void func_802CA1AC(void) {
    *(s16 *) (D_803F8AA0 + 0x76) = 0;
    func_802A7764(D_803F8B58, D_803F8B5C, 0x700);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1AC.s")
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
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* func_802AA890's extra registers (62740.c), as func_8029C454 and
 * func_802ABBEC pass them through. */
typedef struct {
    s32 v1; /* out */
    s32 a0; /* out */
    s32 a3; /* in/out */
    s32 s1; /* in/out */
    s32 s2; /* in/out */
    s32 s0; /* in */
} MtxChainRegs;
#endif
#ifndef OUT_802A9A60_DEFINED
#define OUT_802A9A60_DEFINED
/* func_802A9A60's pointer results (the asm's s1 and s3; 62740.c). */
typedef struct {
    s32 *s1; /* dst + 9 */
    s32 *s3; /* dst */
} Out802A9A60;
#endif
extern u32 D_803F8B48[]; /* x, y, z */
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh,
                  TriSideOut *f, Out802A9A60 *out);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802CB42C(MtxChainRegs *regs);

/* Move this vehicle while it isn't the player's (vehicle 10's counterpart
 * of func_802C5860; no caller references it: func_8024B618 has no case
 * 10): ground heights for its three slots (func_802A9A60 with table +0x52,
 * angle +0x4C, heights to +4.., the middle y to D_803F8B4C, key 10), place
 * the model (func_802CB42C), then func_802A133C(z, 10, x, y, block).
 * Register convention (conventions.txt): `fp` is the asm's $fp, which goes
 * on to func_802A9A60 (stored into D_803ED3F2[0..2], so the block's +0x50
 * becomes (u8) fp): a leaked register in the game (FIDELITY), an explicit
 * parameter here. f12-f26 only pass through func_802A9A60 (not read first;
 * the asm restores f20-f30), so the C starts them at 0. func_802CB42C gets
 * s0 = z and s1 = func_802A9A60's s1. The asm saves every s-register, gp,
 * fp and f20-f30. */
void func_802CA1F8(s32 fp) {
    TriSideOut f;
    Out802A9A60 out;
    MtxChainRegs regs;
    s32 z = D_803F8B48[2];

    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A9A60((s16 *) (D_803F8AA0 + 0x52), D_803F8B48[1], D_803F8B48[0], z, (s32 *) (D_803F8AA0 + 4),
                  (s32 *) &D_803F8B48[1], (s16 *) (D_803F8AA0 + 0x4C), 10, fp, D_803F8AA0, &f, &out);
    regs.s0 = z;
    regs.s1 = (s32) out.s1;
    func_802CB42C(&regs);
    func_802A133C(D_803F8B48[2], 10, D_803F8B48[0], D_803F8B48[1], D_803F8AA0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1F8.s")
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
extern u32 D_803F8B48[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xA at its position
 * D_803F8B48..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CA4E0 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CA308(ZoneScanRegs *r) {
    return func_802ABD54(0xA, D_803F8B48[0], D_803F8B48[1], D_803F8B48[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA308.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef INTERP_REGS_DEFINED
#define INTERP_REGS_DEFINED
/* func_802AAD0C's register results (62740.c). */
typedef struct {
    s32 t3;  /* interpolated u */
    s32 t4;  /* interpolated w */
    f32 f12;
    s32 f14; /* raw bits */
    f32 f20;
    f32 f22;
    f32 f24; /* also an input (kept on some paths) */
    f32 f26;
} InterpRegs;
#endif
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r);
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz);

/* Value pairs on triangle `id` (func_802AAD0C) at this vehicle's x/z
 * (D_803F8B48[0], [2]) -> s16s +0x6A/+0x6C of D_803F8AA0, and at that point
 * moved by ground slot 0 (func_802A94A4(0, +0x52 table, +0x4C angle)) ->
 * +0x70/+0x72; +0x6E gets a copy of the angle at +0x4C.
 * Register convention (conventions.txt): id in a3; func_802AAD0C's FP
 * results pass through to the caller (r; f24 also an input, chained through
 * both calls). The asm saves and restores t0, t1, t3, t4 (asm caller
 * func_802AB50C keeps t0 and t1 live: a mixed N64 build would need a thunk),
 * points $gp at D_803F8AA0 without restoring it, leaves s4 = &+0x4C and
 * v1 = &+0x52 (dead in its caller). Same shape as func_802C59B4 (7FB50) plus
 * the second point. The x + dx / z + dz sums are trapping in the asm. */
void func_802CA34C(s32 id, InterpRegs *r) {
    s32 dx;
    s32 dz;

    func_802AAD0C(id, ((s32 *) D_803F8B48)[0], ((s32 *) D_803F8B48)[2], r);
    *(s16 *) (D_803F8AA0 + 0x6A) = r->t3;
    *(s16 *) (D_803F8AA0 + 0x6C) = r->t4;
    *(u16 *) (D_803F8AA0 + 0x6E) = *(u16 *) (D_803F8AA0 + 0x4C);
    dx = func_802A94A4(0, (s16 *) (D_803F8AA0 + 0x52), (s16 *) (D_803F8AA0 + 0x4C), &dz);
    func_802AAD0C(id, ((s32 *) D_803F8B48)[0] + dx, ((s32 *) D_803F8B48)[2] + dz, r);
    *(s16 *) (D_803F8AA0 + 0x70) = r->t3;
    *(s16 *) (D_803F8AA0 + 0x72) = r->t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA34C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef REGS_802A8768_DEFINED
#define REGS_802A8768_DEFINED
/* func_802A8768's register results besides the FP state (62740.c). */
typedef struct {
    s32 s3;  /* in/out */
    s16 *s4; /* out */
    s32 fp;  /* out */
} Regs802A8768;
#endif
extern u8 D_803ED40B;
s32 func_802AB9A4(s16 *tbl, s32 x1, u16 *angle, s32 id, s32 z1, s32 x2, s32 z2, s32 *s3Out, InterpRegs *r);
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
void func_802CB5D8(void);

/* Set this vehicle down on triangle `id` between the two value pairs at
 * +0x6A/+0x6C and +0x70/+0x72 (func_802CA34C's): heading = func_802AB9A4(
 * table +0x52, pair 1, angle +0x6E, id, pair 2) into +0x4E and +0x4C, the
 * point at pair 1 (func_802AAE54), timers (func_802CB5D8), D_803ED40B = 1,
 * ground contact (func_802A8768: id 10, position D_803F8B48..+8, divisors
 * 0x280 / 0x208, angle +0x4C, flags +0x96, table +0x52, records
 * +0x28/+0x40/+0x34, heights +4), place the model (func_802CB42C) and
 * func_802A133C(z, 10, x, y, block).
 * Register convention (conventions.txt): id in a3; s3 out (func_802AB9A4's
 * distance, on through func_802A8768's in/out slot result: *s3); the FP
 * state chains through func_802AB9A4, func_802AAE54 and func_802A8768 (r:
 * f24 in; f20-f26 out, f12/f14 left as func_802AA764's cos/sin
 * temporaries, not modelled). func_802CB42C gets s0 = &+0x96 and s1 =
 * &D_803F8B50, the flags/pz pointers func_802A8768 leaves. The asm saves and
 * restores a3, t0, t1, t2 (asm caller func_802AB714 keeps a3, t0, t1 live: a
 * mixed N64 build would need a thunk) and points $gp at D_803F8AA0 without
 * restoring it. */
void func_802CA3D8(s32 id, s32 *s3, InterpRegs *r) {
    Regs802A8768 r8768;
    MtxChainRegs regs;
    s32 heading;

    heading = func_802AB9A4((s16 *) (D_803F8AA0 + 0x52), *(s16 *) (D_803F8AA0 + 0x6A), (u16 *) (D_803F8AA0 + 0x6E),
                            id, *(s16 *) (D_803F8AA0 + 0x6C), *(s16 *) (D_803F8AA0 + 0x70),
                            *(s16 *) (D_803F8AA0 + 0x72), &r8768.s3, r);
    *(s16 *) (D_803F8AA0 + 0x4E) = heading;
    *(s16 *) (D_803F8AA0 + 0x4C) = heading;
    func_802AAE54(id, *(s16 *) (D_803F8AA0 + 0x6A), *(s16 *) (D_803F8AA0 + 0x6C), r);
    func_802CB5D8();
    D_803ED40B = 1;
    func_802A8768(D_803F8AA0, 10, (s32 *) &D_803F8B48[0], (s32 *) &D_803F8B48[1], (s32 *) &D_803F8B48[2], r->t3,
                  r->t4, 0x280, 0x208, (s16 *) (D_803F8AA0 + 0x4C), D_803F8AA0 + 0x96, (s16 *) (D_803F8AA0 + 0x52),
                  (s32 *) (D_803F8AA0 + 0x28), (s32 *) (D_803F8AA0 + 0x40), (s32 *) (D_803F8AA0 + 0x34),
                  (s32 *) (D_803F8AA0 + 4), &r8768, (TriSideOut *) &r->f12);
    *s3 = r8768.s3;
    regs.s0 = (s32) (D_803F8AA0 + 0x96);
    regs.s1 = (s32) &D_803F8B48[2];
    func_802CB42C(&regs);
    func_802A133C(D_803F8B48[2], 10, D_803F8B48[0], D_803F8B48[1], D_803F8AA0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA3D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef OUT_802A860C_DEFINED
#define OUT_802A860C_DEFINED
/* func_802A860C's results besides t0 (62740.c). */
typedef struct {
    s32 t1; /* new z */
    s32 s3; /* *px as read */
    s32 fp; /* the cosine */
} Out802A860C;
#endif
extern u8 D_8035805C;
extern u64 *D_803F8B58;
extern u64 *D_803F8B5C;
extern void *D_80367738;
extern u8 D_80367BFF;
extern u8 D_803F8B7A;  /* throttle hold-off countdown */
extern s8 D_803F8B78;  /* "turning back to the ring" flag */
extern s16 D_803F8B74; /* ring target heading */
extern s8 D_803F8B79;  /* sound to start this frame (-1 = none) */
extern u8 D_803F8B7B;  /* bounced off the ring end */
extern u8 D_803A7424;
extern u8 D_803A7425;
extern void *D_803F77D0;
extern f32 D_8030D990;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern s16 D_8036443E;
extern s16 D_80364440;
extern u8 D_80306420[];
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);
void func_8029A914(void);
void func_8029AA10(s32 kind);
void func_8029C52C(s32 tag);
void func_8029E558(u8 *base, u8 *other, void *ch);
s32 func_802A6F6C(void);
void func_802A6FE4(u8 *veh, s32 limit);
void func_802A7070(u8 *veh, s16 *angle);
void func_802A70D8(u8 *veh);
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale);
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut);
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2);
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
void func_802A7E70(s32 rate, u16 *angle);
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag,
                   s32 sound);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_802BE77C(s32 id, u8 *vehicle);
void func_802CAAFC(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
s32 func_802CB564(void);
void func_802CB690(u8 *state);

#define V853_U8(off) (D_803F8AA0[off])
#define V853_S16(off) (*(s16 *) (D_803F8AA0 + (off)))
#define V853_U16(off) (*(u16 *) (D_803F8AA0 + (off)))

/* Per-frame update of vehicle 10 when it is the player's (func_8024B7AC and
 * its init func_802C9B90): zone lookup (func_802CA308), save copy
 * (func_802A75DC), effects (func_802CAAFC, unless +0x9A is set), the shared
 * func_802CB690 (when D_80367BFF), timers, steering (func_802A7E70 at rate
 * func_802CB564()), throttle (func_802A785C, held off while D_803F8B7A
 * counts down), heading (func_802A7FD8), slope and drag (func_802A83B8 /
 * func_802A843C), ring turn-back (func_802A7070 when D_803F8B78), the
 * forward step (func_802A860C) and ground contact (func_802A8768), the
 * model's animation (func_8029E558 on the current copy pair) and placement
 * (func_802CB42C), collision (func_8029A800 / func_8029C52C /
 * func_8029AA10). Then either (no ring hit, D_803A7425 == 0) func_802BE77C
 * and, if that set D_803A7424, a bounce off the ring end (speed reversed
 * into 0x41..0x82 or -0x25..-0x4B, D_803F8B7A = 5) unless one is already
 * running; or (ring hit) func_8029A914, D_803F8B78 = 1, and the turn back
 * toward the ring midpoint (func_802A70D8, func_802A71DC, func_802A746C,
 * func_802A6FE4(0), func_802BE77C). Finally the position, speed and angles
 * go to D_803643E0.. / D_8036443C.., func_802A133C(z, 10, x, y, block), and
 * sound D_803F8B79 (12, or -1 after func_802BE77C) is started if >= 0.
 * Register notes (FIDELITY, see tools_port/port_followups.md): the asm
 * hands func_802CA308 (and so func_802CAAFC -> func_802A6274) the caller's
 * t6, t7, s0-s4 on the paths where func_802ABD54 doesn't set them, and
 * func_802A8768 the caller's f20-f26 (f12 = the constant func_802AE104
 * leaves, f14 its sine temporary): the C starts them at 0 (nothing reads
 * the FP ones first). func_802CB42C gets s0 = &+0x96 and s1 = &D_803F8B50
 * (what is left after func_802A8768; func_8029E558's own s0/s1 leftovers in
 * the game) - they only matter for a count-0 point record. func_8029A914 /
 * func_8029C52C take the block in $gp, but their C rewrites (56040) read
 * D_803EEA90: reported. The asm leaves v1 and f12/f14 as its callees leave
 * them (survey: read by func_8024B7AC / func_802C9B90; not real uses).
 * Integer sums are trapping in the asm (game range). */
void func_802CA4E0(void) {
    ZoneScanRegs zr;
    Out802A860C o860c;
    Regs802A8768 r8768;
    TriSideOut f;
    MtxChainRegs regs;
    s32 x;
    s32 z;
    s32 v;
    s32 mid;
    s32 cur;
    s32 turn;
    s32 tgt;

    zr.t6 = zr.t7 = zr.s0 = zr.s1 = zr.s2 = zr.s3 = zr.s4 = 0;
    func_802CA308(&zr);
    func_802A75DC(D_803F8AA0, D_803F87A0, (s32 *) &D_803F8B48[0], (s32 *) &D_803F8B48[1], (s32 *) &D_803F8B48[2]);
    if ((s8) V853_U8(0x9A) == 0) {
        func_802CAAFC(zr.t6, zr.t7, zr.s0, zr.s1, zr.s2, zr.s3, zr.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(D_803F8AA0);
    }
    func_802CB5D8();
    func_802A7E70(func_802CB564(), &V853_U16(0x4C));
    if (D_803F8B7A == 0) {
        func_802A785C(D_803F8AA0, &V853_S16(0x76), 3, &V853_U8(0x96), &V853_S16(0x78), 0x10);
    } else {
        D_803F8B7A--;
    }
    func_802A7FD8(D_803F8AA0, &V853_U16(0x74), 0x3E80, &V853_S16(0x76), &V853_U16(0x4C), &V853_U16(0x4E),
                  (s8 *) &V853_U8(0x99), 1);
    f.pz = func_802A83B8(&V853_S16(0x76), &V853_U8(0x96), (s32 *) (D_803F8AA0 + 4), (f32 *) (D_803F8AA0 + 0));
    func_802A843C(D_803F8AA0, &V853_S16(0x76), 10, (s8 *) &V853_U8(0x96), (s32 *) (D_803F8AA0 + 4), 1, 640.0f);
    if (D_803F8B78 != 0) {
        func_802A7070(D_803F8AA0, &D_803F8B74);
    }
    x = func_802A860C(f.pz, V853_U16(0x4E), &V853_S16(0x76), (s32 *) &D_803F8B48[0], (s32 *) &D_803F8B48[2],
                      &o860c);
    z = o860c.t1;
    D_803ED40B = 1;
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    r8768.s3 = o860c.s3;
    func_802A8768(D_803F8AA0, 10, (s32 *) &D_803F8B48[0], (s32 *) &D_803F8B48[1], (s32 *) &D_803F8B48[2], x, z,
                  0x280, 0x208, &V853_S16(0x4C), &V853_U8(0x96), &V853_S16(0x52), (s32 *) (D_803F8AA0 + 0x28),
                  (s32 *) (D_803F8AA0 + 0x40), (s32 *) (D_803F8AA0 + 0x34), (s32 *) (D_803F8AA0 + 4), &r8768, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803F8B58, (u8 *) D_803F8B5C, D_803F87A0);
    } else {
        func_8029E558((u8 *) D_803F8B5C, (u8 *) D_803F8B58, D_803F87A0);
    }
    regs.s0 = (s32) &V853_U8(0x96);
    regs.s1 = (s32) &D_803F8B48[2];
    func_802CB42C(&regs);
    func_8029A800(D_803F8B48[2], (s32) D_80306420, 1, 1, D_803F8B48[0], D_803F8B48[1], 8, V853_S16(0x76), 0x64, 0, 10,
                  D_803F8AA0);
    func_8029C52C(10);
    func_8029AA10(10);
    D_803F8B79 = 12;
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803F87A0;
        func_802BE77C(10, D_803F8AA0);
        D_803F8B79 = -1;
        if (D_803A7424 == 0) {
            D_803F8B78 = 0;
            D_803F8B7B = 0;
        } else {
            D_803F8B78 = 0;
            if (D_803F8B7B == 0 && D_803F8B7A == 0) {
                /* bounce off the ring end */
                D_803F8B7A = 5;
                v = V853_S16(0x76);
                if (v >= 0) {
                    if (v < 0x41) {
                        v = 0x41;
                    } else if (!(v < 0x83)) {
                        v = 0x82;
                    }
                } else if (!(v < -0x24)) {
                    v = -0x25;
                } else if (v < -0x4B) {
                    v = -0x4B;
                }
                V853_S16(0x76) = -v;
                D_803F8B7B = 1;
            }
        }
    } else {
        /* turn back toward the ring */
        func_8029A914();
        D_803F8B78 = 1;
        mid = func_802A6F6C();
        v = V853_U16(0x4E) - 0x800;
        if (v < 0) {
            v += 0xFFF;
        }
        v -= mid;
        if (v < 0) {
            v = -v;
        }
        if (!(v < 0x801)) {
            v = 0xFFF - v;
        }
        func_802A70D8(D_803F8AA0);
        turn = func_802A71DC(D_803F8AA0, V853_U16(0x4E), V853_U16(0x4C), &cur, D_8030D990);
        D_803F8B74 = cur;
        V853_S16(0x4E) = cur;
        V853_S16(0x74) = cur;
        func_802A746C(D_803F8AA0, turn, v, &tgt);
        func_802A6FE4(D_803F8AA0, 0);
        D_803F77D0 = D_803F87A0;
        func_802BE77C(10, D_803F8AA0);
    }
    D_803643E0 = D_803F8B48[0];
    D_803643E4 = D_803F8B48[1];
    D_803643E8 = D_803F8B48[2];
    D_8036443C = V853_S16(0x76);
    D_8036443E = V853_U16(0x4E);
    D_80364440 = V853_U16(0x4C);
    func_802A133C(D_803F8B48[2], 10, D_803F8B48[0], D_803F8B48[1], D_803F8AA0);
    if (D_803F8B79 >= 0) {
        func_80260650(D_80367738, D_803F8B79, NULL);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA4E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef CVT_W_S
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
#endif
#ifndef IO_802A6274_DEFINED
#define IO_802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
#endif
extern f32 D_803F8B60; /* engine/tilt animation value (0.5 = centred) */
extern s32 D_803F8B64; /* jump: a (initial speed) */
extern s32 D_803F8B68; /* jump: n (frame) */
extern s32 D_803F8B6C; /* jump: base height */
extern u16 D_803F8B70; /* jump: boost frames */
extern u16 D_803F8B72; /* horn presses left */
extern u8 D_803F8B76;  /* record cooldown */
extern u8 D_803F8B7D;  /* horn cooldown */
extern u8 D_803F8B7E;  /* horn side toggle */
extern u8 D_80370C15;  /* inputs */
extern u8 D_80370C16;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern s8 D_80370C2D;
extern void *D_80367738;
extern f32 D_8030D994;
extern f32 D_8030D998;
extern f32 D_8030D99C;
extern f32 D_8030D9A0;
extern u8 D_802C2954[]; /* definition handed to func_802A6274 */
void *func_80260650(void *arg0, s16 arg1, void *arg2);
s32 func_80292288(s16 speed, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, u8 type, s16 arg8);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
s32 func_802A5ED0(void);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
s32 func_802A7CB0(u8 *veh, s32 range);
s32 func_802ABC88(s32 id, s32 n, u8 **recOut);
void func_802C4584(s32 level);
void func_802CB224(void);
f32 func_802CB3C8(s32 side);

/* a * n + round(-16 * n * n): the jump height after n frames at initial
 * speed a (32-bit products, cvt.w.s rounding; the asm's sum is a trapping
 * `add`). */
#define JUMP_HEIGHT(out, a, n)                    \
    do {                                          \
        s32 _n = (n);                             \
        s32 _q;                                   \
                                                  \
        CVT_W_S(_q, -16.0f * (f32) (_n * _n));    \
        (out) = (a) * _n + _q;                    \
    } while (0)

/* Per-frame effects of vehicle type 10 ($gp = D_803F8AA0, read as the
 * global): with +0x99 set and no cooldown (D_803F8B76), and fewer than 4
 * active D_803C4B70 records (func_802A5ED0), sets up a func_802A6274 record
 * (def D_802C2954, data 0x30D40, type 1 at (10, 1, 1), the caller's t6, t7,
 * s0-s4 passed through); channel 1 gets the speed sign and |speed| / 6, the
 * engine sound |speed| >> 3 (func_802C4584), then the trail (func_802CB224).
 * D_803F8B60 steers towards func_802CB3C8's limit while D_80370C15 /
 * D_80370C16 is held (else back to 0.5) and drives channel 3. The horn
 * (D_80370C1A/1B, D_803F8B72 presses, cooldown D_803F8B7D = 5) alternates
 * channels 4 / 5 and fires func_80292288 from record (10, 3|1) to (10, 4|2)
 * at speed |speed| * 320 / 260 + 640. A jump (D_80370C2D >= 60, not near a
 * band bound per func_802A7CB0) starts or boosts the parabola D_803F8B64 /
 * 68 / 6C; while it is active channel 2 gets height / D_8030D9A0 (at most
 * 1.0), and on landing it bounces (when the impact is >= 61 and no flag
 * +0x96..0x98 is 1) or ends.
 * Register convention (conventions.txt): t6, t7, s0-s4 come in (from the
 * caller's func_802CA308 zone lookup) and only pass through to
 * func_802A6274; the asm changes s1-s7. Integer counters/sums use trapping
 * `addi`/`add`/`sub` in the asm: keep inputs in game range. */
void func_802CAAFC(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 speed;
    s32 cur;
    s32 prev;
    s32 h;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 *rec;
    f32 f;
    f32 lim;

    if (D_803F8B76 != 0) {
        D_803F8B76--;
    } else if (D_803F8AA0[0x99] != 0) {
        D_803F8B76 = 1;
        if (func_802A5ED0() < 4) {
            io.a3 = 0;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2954, 0x30D40, 1, 10, 1, 1, t7, s0, s2, s3, s4, 1);
        }
    }

    speed = *(s16 *) (D_803F8AA0 + 0x76);
    func_802A03D4(D_803F87A0, 1, (speed < 0) ? 1 : 0);
    if (speed < 0) {
        speed = -speed;
    }
    func_802A039C(D_803F87A0, 1, (u32) speed / 6);
    speed = *(s16 *) (D_803F8AA0 + 0x76);
    if (speed < 0) {
        speed = -speed;
    }
    func_802C4584((u32) speed >> 3);
    func_802CB224();

    f = D_803F8B60;
    if (D_80370C15 != 0) {
        f -= D_8030D994;
        lim = func_802CB3C8(0);
        if (f < lim) {
            f = lim;
        }
    } else if (D_80370C16 != 0) {
        f += D_8030D998;
        lim = func_802CB3C8(1);
        if (!(f <= lim)) {
            f = lim;
        }
    } else if (f < 0.5f) {
        f += D_8030D99C;
        if (!(f <= 0.5f)) {
            f = 0.5f;
        }
    } else {
        f -= D_8030D99C;
        if (f < 0.5f) {
            f = 0.5f;
        }
    }
    D_803F8B60 = f;
    func_802A0360(f, D_803F87A0, 3, 0);

    if (D_803F8B7D != 0) {
        D_803F8B7D--;
    } else if ((D_80370C1A != 0 || D_80370C1B != 0) && D_803F8B72 != 0) {
        D_803F8B72--;
        func_80260650(D_80367738, 3, NULL);
        D_803F8B7E ^= 1;
        if (D_803F8B7E != 0) {
            func_802A0360(0.0f, D_803F87A0, 4, 0);
            func_802A0290(D_803F87A0, 4, 1);
            func_802ABC88(10, 3, (u8 **) &rec);
            x1 = rec[0];
            y1 = rec[1];
            z1 = rec[2];
            func_802ABC88(10, 4, (u8 **) &rec);
        } else {
            func_802A0360(0.0f, D_803F87A0, 5, 0);
            func_802A0290(D_803F87A0, 5, 1);
            func_802ABC88(10, 1, (u8 **) &rec);
            x1 = rec[0];
            y1 = rec[1];
            z1 = rec[2];
            func_802ABC88(10, 2, (u8 **) &rec);
        }
        speed = *(s16 *) (D_803F8AA0 + 0x76);
        if (speed < 0) {
            speed = -speed;
        }
        /* the asm passes the whole word; the callee takes an s16 */
        func_80292288((u32) (speed * 0x140) / 0x104 + 0x280, rec[0], rec[1], rec[2], x1, y1, z1, 0, 0x1A4);
        D_803F8B7D = 5;
    }

    if (func_802A7CB0(D_803F8AA0, 10) == 0 && D_80370C2D >= 0x3C) {
        if (D_803F8B7C == 0) {
            /* take off */
            D_803F8B7C = 1;
            D_803F8B6C = 0;
            D_803F8B68 = 1;
            D_803F8B64 = 0x3C;
            D_803F8B70 = 0;
        } else if (D_803F8B70 < 0x17) {
            /* boost: rebase the parabola at the current height */
            D_803F8B70++;
            JUMP_HEIGHT(cur, D_803F8B64, D_803F8B68);
            D_803F8B6C += cur;
            JUMP_HEIGHT(prev, D_803F8B64, D_803F8B68 - 1);
            D_803F8B64 = cur - prev + 0x33;
            D_803F8B68 = 1;
        }
    }

    if (D_803F8B7C != 0) {
        JUMP_HEIGHT(cur, D_803F8B64, D_803F8B68);
        h = D_803F8B6C + cur;
        if (h <= 0) {
            /* landed: bounce with half the impact speed, or stop */
            JUMP_HEIGHT(prev, D_803F8B64, D_803F8B68 - 1);
            prev -= cur;
            if (prev < 0) {
                prev = -prev;
            }
            if (prev < 0x3D || D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1) {
                goto stop;
            }
            D_803F8B6C = 0;
            D_803F8B64 = (u32) prev / 2;
            D_803F8B68 = 1;
            h = 0;
        }
        f = (f32) h / D_8030D9A0;
        if (!(f <= 1.0f)) {
            f = 1.0f;
        }
        func_802A0360(f, D_803F87A0, 2, 0);
        D_803F8B68++;
        return;
    }
stop:
    D_803F8B7C = 0;
    func_802A0360(0.0f, D_803F87A0, 2, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CAAFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F8B48[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Trail (shape of func_802B3C68 in 6E200; $gp = D_803F8AA0): if the byte at
 * +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2], yaw,
 * 5, 40, 0, 1): a single centred track.
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CAAFC keeps a1-a3 live (a mixed N64 build would need a
 * thunk). */
void func_802CB224(void) {
    if (D_803F8AA0[0x99] != 0 && D_803F8AA0[0x98] != 1 && D_803F8AA0[0x50] < 3 && D_803F8AA0[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F8AA0 + 0x1C), 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2],
                      *(u16 *) (D_803F8AA0 + 0x4E), 5, 40, 0, 1);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB224.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0.5 +- |speed| / 280 * 0.5 (plus when side != 0, minus otherwise), speed
 * being the s16 at +0x76 of D_803F8AA0 (the asm's $gp). The asm takes side
 * in s2, returns the value in f2 and leaves s3 = 280 (see
 * tools_port/conventions.txt). Its asm caller func_802CAAFC keeps a1-a3
 * live (a mixed N64 build would need a thunk). Same shape as func_802B71DC
 * (71140). */
f32 func_802CB3C8(s32 side) {
    s32 speed = *(s16 *) (D_803F8AA0 + 0x76);
    f32 v;

    if (speed < 0) {
        speed = -speed;
    }
    v = (f32) speed / 280.0f * 0.5f;
    if (side != 0) {
        return v + 0.5f;
    }
    return 0.5f - v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB3C8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* func_802AA890's extra registers (62740.c), as func_8029C454 and
 * func_802ABBEC pass them through. */
typedef struct {
    s32 v1; /* out */
    s32 a0; /* out */
    s32 a3; /* in/out */
    s32 s1; /* in/out */
    s32 s2; /* in/out */
    s32 s0; /* in */
} MtxChainRegs;
#endif
extern u8 *D_803F8B54; /* this vehicle's model: +0 / +4 / +8 offsets of the point lists, +0x18 the matrix */
extern u8 D_8035805C;  /* which of the save copy pair is current */
extern s16 D_803ED390[]; /* rotation angles x, y, z for func_802AA764 */
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);

/* Place this vehicle's model (vehicle 10): builds its matrix (scale 0x3E80,
 * yaw = +0x4C of D_803F8AA0, the other two angles as they are) at the
 * model's +0x18 entry inside the current copy (D_8035805C ? D_803F8B58 :
 * D_803F8B5C) with func_802AA764, then places the parts
 * (func_8029C454(x, y, z, 10, model + [4], model + [8], copy)) and the
 * points (func_802ABBEC(10, model + [0], model + [4], copy)).
 * Register convention (conventions.txt): s0 and s1 come in for the
 * func_802AA890 chain, whose registers (v1, a0, a3, s1, s2) go out; regs
 * carries them. func_802AA764 leaves a3 = 0 (func_802ACCCC's loop counter)
 * and s2 = the matrix, which the asm then hands func_8029C454: modelled
 * explicitly. func_8029C454 restores v1/a0, so func_802ABBEC gets y and z
 * there. The asm reads the block through $gp (= D_803F8AA0, set by every
 * caller) and leaves t0 = 10. Its cos/sin f12/f14 temporaries, which the
 * survey lists as read by func_802CA4E0, are not modelled (see
 * func_802AA764). Asm caller func_802CA4E0 keeps t6, t7 live (a mixed N64
 * build would need a thunk; the native port won't). Adds are trapping in
 * the asm (pointer sums, in range). */
void func_802CB42C(MtxChainRegs *regs) {
    u8 *mdl = D_803F8B54;
    u8 *base;
    s32 *m;
    s32 x;
    s32 y;
    s32 z;

    m = (s32 *) (*(u32 *) (mdl + *(s32 *) (mdl + 0x18) + 4) +
                 (u32) (D_8035805C != 0 ? (u8 *) D_803F8B58 : (u8 *) D_803F8B5C));
    D_803ED390[1] = *(u16 *) (D_803F8AA0 + 0x4C);
    func_802AA764(D_803F8B48[0], D_803F8B48[1], D_803F8B48[2], 0x3E80, m);
    base = D_8035805C != 0 ? (u8 *) D_803F8B58 : (u8 *) D_803F8B5C;
    mdl = D_803F8B54;
    x = D_803F8B48[0];
    y = D_803F8B48[1];
    z = D_803F8B48[2];
    regs->a3 = 0;
    regs->s2 = (s32) m;
    func_8029C454(x, y, z, 10, mdl + *(s32 *) (mdl + 4), mdl + *(s32 *) (mdl + 8), base, regs);
    regs->v1 = y;
    regs->a0 = z;
    mdl = D_803F8B54;
    func_802ABBEC(10, (s16 *) (mdl + *(s32 *) (mdl + 0)), (s16 *) (mdl + *(s32 *) (mdl + 4)), base, regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB42C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef CVT_W_S
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
#endif

extern f32 D_8030D9A4;

/* Speed (s16 at +0x76 of D_803F8AA0, the asm's $gp) / 6.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9A4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CA4E0 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CB564(void) {
    f32 div;
    s32 r;

    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D9A4;
    }
    CVT_W_S(r, *(s16 *) (D_803F8AA0 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB564.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CA3D8 on t3, t4, f12, f14; func_802CA4E0 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CB5D8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB5D8.s")
#endif

/* func_802CB634: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8AA0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803F8B48[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803F8AA0[0..0xA5] plus the three words
 * D_803F8B48[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802CB634(u8 *dst) {
    return func_802AC7DC(dst, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB634.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802CB634: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802CB660(void *)`. */
void func_802CB660(void *src) {
    func_802AC85C(src, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB660.s")
#endif
