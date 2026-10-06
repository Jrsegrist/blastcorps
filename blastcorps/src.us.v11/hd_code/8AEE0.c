#include "common.h"
#include <ultra64.h>
#include "game/game.h"

#ifdef NON_MATCHING
/* Shared declarations for the vehicle type 15 (state block D_803FC500) and
 * type 16 (D_803FC8D0) rewrites below. The structs mirror the register
 * blocks of the callees' C rewrites (56040.c, 62740.c). */


extern u8 D_803FC500[]; /* vehicle 15 state block */
extern u8 D_803FC8D0[]; /* vehicle 16 state block */
extern u32 D_803FC5A8[]; /* vehicle 15 x, y, z */
extern u32 D_803FC978[]; /* vehicle 16 x, y, z */
extern u8 *D_803FC5B4;   /* vehicle 15 model */
extern u8 *D_803FC984;   /* vehicle 16 model */
extern u64 *D_803FC5B8;
extern u64 *D_803FC5BC;
extern u64 *D_803FC988;
extern u64 *D_803FC98C;
extern u8 *D_80358070; /* matrix buffer allocator */
extern s16 D_803ED390[]; /* rotation angles x, y, z for func_802AA764 */
extern u8 D_803FC200[];  /* vehicle 15 animation channel table */
extern u8 D_803FC5D0[];  /* vehicle 16 animation channel table */
extern u8 D_803FC5C2;
extern s8 D_803FC5C3;
extern u8 D_803FC5C4;
extern s16 D_803FC5C0;
extern s16 D_803FC994;
extern s8 D_803FC998;
extern u8 D_803FC999;
extern u8 D_803FC99A;
extern f32 D_8030D9D0;
extern u8 *D_803F77D0;
extern u8 D_80306460[];
extern u8 D_80306470[];
extern void *D_803F7844;


s32 func_802CFC10(ZoneScanRegs *r);
void func_802D02F8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
s32 func_802D0710(void);
void func_802D0784(void);
s32 func_802D0F54(ZoneScanRegs *r);
void func_802D1360(void);
s32 func_802D2444(void);
void func_802D249C(void);
s32 func_802D05D8(s32 a3, s32 s0, s32 s1, VehMtxOut *out);
s32 func_802D22F4(s32 a3, s32 s0, s32 s1, VehMtxOut *out);

/* Ring distance used by both per-frame functions: the target heading turned
 * half a circle (wrapping by 0xFFF), minus the ring midpoint, as a distance
 * mod 0xFFF folded at 0x800. */
#define PORT_RING_DIST(d, heading, mid) \
    do {                                \
        (d) = (heading) - 0x800;        \
        if ((d) < 0) {                  \
            (d) += 0xFFF;               \
        }                               \
        (d) -= (mid);                   \
        if ((d) < 0) {                  \
            (d) = -(d);                 \
        }                               \
        if ((d) > 0x800) {              \
            (d) = 0xFFF - (d);          \
        }                               \
    } while (0)
#endif

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 15 (called from the level-object dispatcher
 * func_802A350C with a spawn record decoded into registers): model `model`
 * (D_803FC5B4), two 0x100-byte matrix buffers taken from D_80358070 (into
 * D_803FC5B8/BC), func_802A1388(0xF, 0, ...), the record reset
 * (func_802A754C), the wheel offsets at +0x52..+0x68 and speed bands at
 * +0x78..+0x94, position (x, y, z) and heading, the ground slots
 * (func_802A992C), the model's animation channels (func_8029F85C, channel 0
 * set up and run twice through func_8029E558), the object parts
 * (func_8029C354, scale 0x32C8) and func_80258230; then one frame of
 * func_802CFDE8 with +0x9A set, and the matrix copy func_802AA838.
 * Register convention (conventions.txt): model s2, x t7, y s3, z s0, heading
 * s1, fp; t6 and func_802A992C's FP state (f) only pass through to
 * func_802CFDE8. The asm restores t0-t5 (asm caller func_802A350C keeps t1
 * and t2 live) and leaves s0-s7, fp, gp and f20-f30 changed; the survey's
 * s2-s4, fp, f12-f26 "outputs" are rewritten by func_802A350C before use.
 * Leaked registers (FIDELITY): func_802CFDE8's zone registers here are the
 * stub-model values - t6 the caller's, t7 func_8029F85C's result (the asm
 * expects the C func_80258230 to keep t6/t7), s0 = z and s1 = +4, which in
 * the game func_8029F85C (s0 = s1 = 0) and func_8029E558 have overwritten;
 * likewise f14-f26 after func_8029E558 (f20). */
void func_802CF6A0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fpIn, s32 t6, TriSideOut *f) {
    u8 *v = D_803FC500;
    u8 *buf;
    u64 *b0;
    u64 *b1;
    s32 *s3v;
    s32 t7;

    D_803FC5B4 = model;
    buf = D_80358070;
    D_803FC5B8 = (u64 *) buf;
    D_803FC5BC = (u64 *) (buf + 0x100);
    D_80358070 = buf + 0x200;
    func_802A1388(0xF, 0, (s32) D_803FC5B8, (s32) D_803FC5BC, model);
    func_802A754C(v);
    *(s16 *) (v + 0x52) = 0xAF;
    *(s16 *) (v + 0x54) = 0xFA;
    *(s16 *) (v + 0x56) = -0xAF;
    *(s16 *) (v + 0x58) = 0xFA;
    *(s16 *) (v + 0x5A) = 0xAF;
    *(s16 *) (v + 0x5C) = -0xFA;
    *(s16 *) (v + 0x5E) = 0x140;
    *(s16 *) (v + 0x60) = 0x1F4;
    *(s16 *) (v + 0x62) = -0x140;
    *(s16 *) (v + 0x64) = 0x1F4;
    *(s16 *) (v + 0x66) = 0x140;
    *(s16 *) (v + 0x68) = -0x1F4;
    D_803FC5A8[0] = x;
    D_803FC5A8[1] = y;
    D_803FC5A8[2] = z;
    *(s16 *) (v + 0x4C) = heading;
    *(s16 *) (v + 0x4E) = heading;
    *(s16 *) (v + 0x74) = heading;
    s3v = func_802A992C((s16 *) (v + 0x52), D_803FC5A8[1], x, z, (s32 *) (v + 4), (s32 *) &D_803FC5A8[1],
                        (s16 *) (v + 0x4C), 0xF, fpIn, v, f);
    b0 = D_803FC5B8;
    b1 = D_803FC5BC;
    t7 = func_8029F85C((u32 *) b1, (u32 *) b0, D_803FC200, D_803FC5B4);
    func_802A039C(D_803FC200, 0, 0x64);
    func_802A03D4(D_803FC200, 0, 0);
    func_802A040C(D_803FC200, 0, 0);
    func_802A0480(0.0f, D_803FC200, 0, 0);
    func_802A0290(D_803FC200, 0, 1);
    func_8029E558((u8 *) b0, (u8 *) b1, D_803FC200);
    func_802A0320(0, D_803FC200);
    func_802A0290(D_803FC200, 0, 1);
    func_8029E558((u8 *) b1, (u8 *) b0, D_803FC200);
    *(s16 *) (v + 0x78) = -0xB4;
    *(s16 *) (v + 0x7A) = 0;
    *(s16 *) (v + 0x7C) = 2;
    *(s16 *) (v + 0x7E) = 0;
    *(s16 *) (v + 0x80) = 0x118;
    *(s16 *) (v + 0x82) = 4;
    *(s16 *) (v + 0x84) = 0x118;
    *(s16 *) (v + 0x86) = 0x12C;
    *(s16 *) (v + 0x88) = 5;
    *(s16 *) (v + 0x8A) = 0x12C;
    *(s16 *) (v + 0x8C) = 0x136;
    *(s16 *) (v + 0x8E) = 3;
    *(s16 *) (v + 0x90) = 0x136;
    *(s16 *) (v + 0x92) = 0x140;
    *(s16 *) (v + 0x94) = 2;
    D_803FC5C2 = 0;
    D_803FC5C3 = 0;
    D_803FC5C4 = 0;
    model = D_803FC5B4;
    func_8029C354(0xF, model + *(s32 *) (model + 4), model + *(s32 *) (model + 8), 0x32C8);
    func_80258230(0xF, 0x3C, 0x19, 0x19);
    v[0x9A] = 1;
    func_802CFDE8(t6, t7, z, (s32) (v + 4), (s32) &D_803FC5A8[1], (s32) s3v, (s32) (v + 0x4C), f->cross, f->cz,
                  f->side, f->sideZ, f->dz);
    v[0x9A] = 0;
    model = D_803FC5B4;
    func_802AA838((u8 *) D_803FC5BC, (u8 *) D_803FC5B8, *(s32 *) (model + *(s32 *) (model + 0x18) + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CF6A0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC500[]; /* this vehicle's state block */

/* Enter vehicle type 15: clears the byte at +0x99, D_8036444C/50 = 3000,
 * 1000, then func_802C4310(arg0, 0xCE) (arg0 passes straight through; hd.c
 * calls this with no arguments and func_802C4310 ignores it). The asm also
 * points $gp at D_803FC500 and leaves it there (conventions.txt: clobbers
 * gp); C code doesn't use $gp. Same shape as func_802B76AC (72B80). */
void func_802CFA0C(s32 arg0) {
    D_803FC500[0x99] = 0;
    D_8036444C = 3000;
    D_80364450 = 1000;
    func_802C4310(arg0, 0xCE);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFA0C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 15
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1,
 * else 1 (shape shared with func_802CA140 in 853D0.c). 00000.c declares it
 * void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803FC500[];

s32 func_802CFA58(void) {
    if (D_803FC500[0x96] == 1 || D_803FC500[0x97] == 1 || D_803FC500[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFA58.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC500[];
extern u64 *D_803FC5B8;
extern u64 *D_803FC5BC;

/* Vehicle-type 15 exit: zero the speed (s16 at +0x76), copy 0x100 bytes
 * between the two buffers D_803FC5B8/D_803FC5BC point at (func_802A7764),
 * then func_802C444C. Same shape as func_802B7754 (72B80). */
void func_802CFAB4(void) {
    *(s16 *) (D_803FC500 + 0x76) = 0;
    func_802A7764(D_803FC5B8, D_803FC5BC, 0x100);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFAB4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Re-ground vehicle type 15 (called from 00000.c's func_8024B618): the
 * ground slots (func_802A9A60, key 0xF) at its position, the model matrices
 * (func_802D05D8), then the D_80364460 record (func_802A133C).
 * Register convention: fp and a3 (passed through to func_802A9A60 and
 * func_802D05D8) and the FP pass-through state f12-f26; the C caller declares
 * it `void (void)`, so in the game these are whatever that C code left (the
 * port's caller passes garbage too; update 00000.c's prototype in the port
 * headers). The survey's v1 output (left = y by func_802A133C) isn't used by
 * the C caller. */
void func_802CFB00(s32 fpIn, s32 a3, f32 f12, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26) {
    u8 *v = D_803FC500;
    TriSideOut f;
    Out802A9A60 o9;
    VehMtxOut mo;
    s32 x = D_803FC5A8[0];
    s32 z = D_803FC5A8[2];

    f.pz = f12;
    f.cross = f14;
    f.cz = f20;
    f.side = f22;
    f.sideZ = f24;
    f.dz = f26;
    func_802A9A60((s16 *) (v + 0x52), D_803FC5A8[1], x, z, (s32 *) (v + 4), (s32 *) &D_803FC5A8[1],
                  (s16 *) (v + 0x4C), 0xF, fpIn, v, &f, &o9);
    func_802D05D8(a3, z, (s32) o9.s1, &mo);
    func_802A133C(D_803FC5A8[2], 0xF, D_803FC5A8[0], D_803FC5A8[1], v);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFB00.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803FC5A8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xF at its position
 * D_803FC5A8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CFDE8 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CFC10(ZoneScanRegs *r) {
    return func_802ABD54(0xF, D_803FC5A8[0], D_803FC5A8[1], D_803FC5A8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFC10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Vehicle type 15 on a moving platform (asm caller func_802AB50C): the
 * value pair at its position on triangle `id` (func_802AAD0C) goes to the
 * halves +0x6A/+0x6C and the heading to +0x6E; then the pair at the position
 * plus wheel 0's offset rotated by the heading (func_802A94A4) to +0x70/+0x72.
 * Returns the asm's v1, &D_803FC500[0x52] (func_802AAD0C keeps it).
 * Register convention (conventions.txt): id a3, f24 in through r (chained
 * between the calls), outputs v1 and r's f24/f26; the asm restores t0, t1,
 * t3, t4 (asm caller keeps t0, t1 live) and leaves gp = D_803FC500, s4 and
 * f20-f28 changed. */
s32 func_802CFC54(s32 id, InterpRegs *r) {
    u8 *v = D_803FC500;
    s32 x = D_803FC5A8[0];
    s32 z = D_803FC5A8[2];
    s32 dx;
    s32 dz;

    func_802AAD0C(id, x, z, r);
    *(s16 *) (v + 0x6A) = r->t3;
    *(s16 *) (v + 0x6C) = r->t4;
    *(s16 *) (v + 0x6E) = *(u16 *) (v + 0x4C);
    dx = func_802A94A4(0, (s16 *) (v + 0x52), (s16 *) (v + 0x4C), &dz);
    func_802AAD0C(id, x + dx, z + dz, r);
    *(s16 *) (v + 0x70) = r->t3;
    *(s16 *) (v + 0x72) = r->t4;
    return (s32) (v + 0x52);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFC54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Vehicle type 15 carried by a moving platform (asm caller func_802AB714):
 * new heading from the platform triangle `id` (func_802AB9A4 on the pairs
 * func_802CFC54 stored) into +0x4E and +0x4C, the new position pair
 * (func_802AAE54 at +0x6A/+0x6C), func_802D0784, then ground contact
 * (func_802A8768 with D_803ED40B = 1, divisors 0x1F4/0x15E) at that point,
 * the model matrices (func_802D05D8) and the D_80364460 record.
 * Returns func_802A8768's s3.
 * Register convention (conventions.txt): id a3, f24 in through r; outputs
 * s3 and f24/f26 (func_802A8768's, through r). The asm restores a3, t0-t2
 * (asm caller keeps a3, t0, t1 live) and leaves gp = D_803FC500, s0-s2,
 * s4-s7, fp, f20, f22, f28 changed. func_802AAE54's f12-f26 feed
 * func_802A8768's FP state (f14 as raw bits). func_802D05D8 gets a3 = +0x34,
 * s0 = +0x96, s1 = &z, as the asm leaves them for it. */
s32 func_802CFCE0(s32 id, InterpRegs *r) {
    u8 *v = D_803FC500;
    TriSideOut f;
    Regs802A8768 r8;
    VehMtxOut mo;
    s32 s3;
    s32 h;

    h = func_802AB9A4((s16 *) (v + 0x52), *(s16 *) (v + 0x6A), (u16 *) (v + 0x6E), id, *(s16 *) (v + 0x6C),
                      *(s16 *) (v + 0x70), *(s16 *) (v + 0x72), &s3, r);
    *(s16 *) (v + 0x4E) = h;
    *(s16 *) (v + 0x4C) = h;
    func_802AAE54(id, *(s16 *) (v + 0x6A), *(s16 *) (v + 0x6C), r);
    func_802D0784();
    D_803ED40B = 1;
    f.pz = r->f12;
    *(s32 *) &f.cross = r->f14;
    f.cz = r->f20;
    f.side = r->f22;
    f.sideZ = r->f24;
    f.dz = r->f26;
    r8.s3 = s3;
    func_802A8768(v, 0xF, (s32 *) &D_803FC5A8[0], (s32 *) &D_803FC5A8[1], (s32 *) &D_803FC5A8[2], r->t3, r->t4,
                  0x1F4, 0x15E, (s16 *) (v + 0x4C), v + 0x96, (s16 *) (v + 0x52), (s32 *) (v + 0x28),
                  (s32 *) (v + 0x40), (s32 *) (v + 0x34), (s32 *) (v + 4), &r8, &f);
    func_802D05D8((s32) (v + 0x34), (s32) (v + 0x96), (s32) &D_803FC5A8[2], &mo);
    func_802A133C(D_803FC5A8[2], 0xF, D_803FC5A8[0], D_803FC5A8[1], v);
    r->f24 = f.sideZ;
    r->f26 = f.dz;
    return r8.s3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFCE0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 15 (called from 00000.c's func_8024B7AC
 * and once from func_802CF6A0): zone level (func_802CFC10), state save
 * (func_802A75DC), sound 0x76, effects (func_802D02F8 unless +0x9A), the
 * D_80367BFF hook (func_802CB690), timers (func_802D0784), steering
 * (func_802A7E70 with the rate from func_802D0710), throttle (func_802A785C,
 * skipped while the D_803FC5C4 countdown runs), heading, slope and speed
 * (func_802A7FD8 / 83B8 / 843C, func_802A7070 while D_803FC5C3), the move
 * (func_802A860C), ground contact (func_802A8768, D_803ED40B = 1), animation
 * (func_8029E558 on the current matrix buffer), model matrices
 * (func_802D05D8), the vehicle-state reset and collision passes
 * (func_8029A800, func_8029C52C, func_8029AA10). Then: on a ring hit
 * (D_803A7425) steer along the ring (func_8029A914, func_802A70D8,
 * func_802A71DC with scale D_8030D9D0, func_802A746C, func_802A6FE4) and
 * collide (func_802BE77C); else collide and, if that hit something
 * (D_803A7424), restore the saved state (func_802A768C), start a 5-frame
 * countdown, bounce back at half of at least 50 the other way and redo the
 * matrices. Finally the position/speed/headings go to D_803643E0.. and the
 * D_80364460 record (func_802A133C).
 * Register convention (conventions.txt): the zone registers t6, t7, s0-s4
 * (func_802CFC10 in/out, then func_802D02F8) and the FP pass-through state
 * f14-f26 (func_802A8768; f12 is func_802A83B8's result by then). The C
 * caller declares it `void (void)` (garbage in the game as in the port;
 * update 00000.c's prototype in the port headers). The survey's outputs (v1
 * = y; f12/f14, sine-routine leftovers that func_802CF6A0 doesn't use) are
 * not modelled.
 * Leaked registers (FIDELITY): func_802D05D8's a3/s0/s1 (only read by
 * func_802AA890 for a matrix count of 0) are the stub-model values: a3 =
 * +0x34 (second call: func_802A768C's dst2End), s0 = +0x96, s1 = &z (second
 * call: the first call's s1); in the game func_8029E558, func_8029C52C and
 * func_802BE77C have changed s0/s1 by then. The v1 handed to func_802A746C is
 * the ring distance computed after func_802A6F6C (the asm's dead-looking
 * code feeds it). */
void func_802CFDE8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, f32 f14, f32 f20, f32 f22, f32 f24,
                   f32 f26) {
    u8 *v = D_803FC500;
    ZoneScanRegs zr;
    TriSideOut f;
    Regs802A8768 r8;
    Out802A860C o;
    VehMtxOut mo;
    u64 *end;
    u64 *src;
    u64 *dst2;
    f32 f12;
    s32 x;
    s32 d;
    s32 turn;
    s32 cur;
    s32 tgt;

    zr.t6 = t6;
    zr.t7 = t7;
    zr.s0 = s0;
    zr.s1 = s1;
    zr.s2 = s2;
    zr.s3 = s3;
    zr.s4 = s4;
    func_802CFC10(&zr);
    func_802A75DC(v, D_803FC200, (s32 *) &D_803FC5A8[0], (s32 *) &D_803FC5A8[1], (s32 *) &D_803FC5A8[2]);
    func_802C4724(0x76);
    if (v[0x9A] == 0) {
        func_802D02F8(zr.t6, zr.t7, zr.s0, zr.s1, zr.s2, zr.s3, zr.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(v);
    }
    func_802D0784();
    func_802A7E70(func_802D0710(), (u16 *) (v + 0x4C));
    if (D_803FC5C4 == 0) {
        func_802A785C(v, (s16 *) (v + 0x76), 3, v + 0x96, (s16 *) (v + 0x78), 0x10);
    } else {
        D_803FC5C4--;
    }
    func_802A7FD8(v, (u16 *) (v + 0x74), 0x1F40, (s16 *) (v + 0x76), (u16 *) (v + 0x4C), (u16 *) (v + 0x4E),
                  (s8 *) (v + 0x99), 1);
    f12 = func_802A83B8((s16 *) (v + 0x76), v + 0x96, (s32 *) (v + 4), (f32 *) v);
    func_802A843C(v, (s16 *) (v + 0x76), 0xF, (s8 *) (v + 0x96), (s32 *) (v + 4), 1, 500.0f);
    if (D_803FC5C3 != 0) {
        func_802A7070(v, &D_803FC5C0);
    }
    x = func_802A860C(f12, *(u16 *) (v + 0x4E), (s16 *) (v + 0x76), (s32 *) &D_803FC5A8[0],
                      (s32 *) &D_803FC5A8[2], &o);
    D_803ED40B = 1;
    r8.s3 = o.s3;
    f.pz = f12;
    f.cross = f14;
    f.cz = f20;
    f.side = f22;
    f.sideZ = f24;
    f.dz = f26;
    func_802A8768(v, 0xF, (s32 *) &D_803FC5A8[0], (s32 *) &D_803FC5A8[1], (s32 *) &D_803FC5A8[2], x, o.t1, 0x1F4,
                  0x15E, (s16 *) (v + 0x4C), v + 0x96, (s16 *) (v + 0x52), (s32 *) (v + 0x28), (s32 *) (v + 0x40),
                  (s32 *) (v + 0x34), (s32 *) (v + 4), &r8, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803FC5B8, (u8 *) D_803FC5BC, D_803FC200);
    } else {
        func_8029E558((u8 *) D_803FC5BC, (u8 *) D_803FC5B8, D_803FC200);
    }
    func_802D05D8((s32) (v + 0x34), (s32) (v + 0x96), (s32) &D_803FC5A8[2], &mo);
    func_8029A800(D_803FC5A8[2], (s32) D_80306460, 1, 1, D_803FC5A8[0], D_803FC5A8[1], 7, *(s16 *) (v + 0x76),
                  0x96, 0, 0xF, v);
    func_8029C52C(0xF, v);
    func_8029AA10(0xF);
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803FC200;
        func_802BE77C(0xF, v);
        if (D_803A7424 != 0) {
            D_803FC5C3 = 0;
            if (D_8035805C != 0) {
                src = D_803FC5BC;
                dst2 = D_803FC5B8;
            } else {
                src = D_803FC5B8;
                dst2 = D_803FC5BC;
            }
            func_802A768C(v, D_803FC200, (s32 *) &D_803FC5A8[0], (s32 *) &D_803FC5A8[1], (s32 *) &D_803FC5A8[2],
                          src, dst2, 0x100, &end);
            D_803FC5C4 = 5;
            d = *(s16 *) (v + 0x76);
            if (d >= 0) {
                if (d < 0x32) {
                    d = 0x32;
                }
            } else if (d >= -0x31) {
                d = -0x32;
            }
            d = -d;
            *(s16 *) (v + 0x76) = d >> 1;
            func_802D05D8((s32) end, (s32) (v + 0x96), mo.s1, &mo);
        } else {
            D_803FC5C3 = 0;
        }
    } else {
        func_8029A914(v);
        D_803FC5C3 = 1;
        turn = func_802A6F6C();
        PORT_RING_DIST(d, *(u16 *) (v + 0x4E), turn);
        func_802A70D8(v);
        turn = func_802A71DC(v, *(u16 *) (v + 0x4E), *(u16 *) (v + 0x4C), &cur, D_8030D9D0);
        D_803FC5C0 = cur;
        *(s16 *) (v + 0x4E) = cur;
        *(s16 *) (v + 0x74) = cur;
        func_802A746C(v, turn, d, &tgt);
        func_802A6FE4(v, 0);
        D_803F77D0 = D_803FC200;
        func_802BE77C(0xF, v);
    }
    D_803643E0 = D_803FC5A8[0];
    D_803643E4 = D_803FC5A8[1];
    D_803643E8 = D_803FC5A8[2];
    D_8036443C = *(s16 *) (v + 0x76);
    D_8036443E = *(u16 *) (v + 0x4E);
    D_80364440 = *(u16 *) (v + 0x4C);
    func_802A133C(D_803FC5A8[2], 0xF, D_803FC5A8[0], D_803FC5A8[1], v);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFDE8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC5C2;   /* effect cooldown */
void func_802D0438(void);

/* Per-frame effects for vehicle type 15 ($gp = D_803FC500, read as the
 * global): the tyre trail (func_802D0438); then, if the cooldown
 * D_803FC5C2 is nonzero it just counts down, else when byte +0x99 is set the
 * cooldown becomes 1 and, with fewer than 15 active func_802A6274 records,
 * four are set up (def D_802C2954, type 1 at (15, 1..4, 1), data 0x29810
 * for the first two and 0x1D4C0 for the last two). Finally
 * func_802C4584(|speed| >> 5) with speed the s16 at +0x76.
 * Register convention (conventions.txt): t6, t7, s0-s4 pass through to
 * func_802A6274 (t6 and s1 in its in/out block, chained between the calls);
 * the asm changes s1 and s5-s7. Same shape as func_802B7F98 (72B80). */
void func_802D02F8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 v;

    func_802D0438();
    if (D_803FC5C2 != 0) {
        D_803FC5C2--;
    } else if (D_803FC500[0x99] != 0) {
        D_803FC5C2 = 1;
        if (func_802A5ED0() < 15) {
            io.t6 = t6;
            io.s1 = s1;
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 15, 1, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 15, 2, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 15, 3, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 15, 4, 1, t7, s0, s2, s3, s4, 1);
        }
    }
    v = *(s16 *) (D_803FC500 + 0x76);
    if (v < 0) {
        v = -v;
    }
    func_802C4584((u32) v >> 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D02F8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803FC5A8[]; /* [0], [2]: trail x, z */

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803FC500): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803FC5A8[0], D_803FC5A8[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802D02F8 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802D0438(void) {
    if (D_803FC500[0x99] != 0 && D_803FC500[0x98] != 1 && D_803FC500[0x50] < 3 && D_803FC500[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803FC500 + 0x1C), 250, -400, -400, -400, D_803FC5A8[0], D_803FC5A8[2],
                      *(u16 *) (D_803FC500 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0438.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Model matrices of vehicle type 15: the root matrix (func_802AA764 at its
 * position, scale 0x32C8, rotation y = heading +0x4C only - D_803ED390[0]/[2]
 * are left as they are) into the current matrix buffer (D_8035805C picks
 * D_803FC5B8 or D_803FC5BC) at the offset the model's +0x18 table gives;
 * then the model's part list (+4..+8, func_8029C454, tag 0xF) and point list
 * (+0..+4, func_802ABBEC) through the matrix chain.
 * Register convention (conventions.txt): a3, s0, s1 pass into the matrix
 * chain registers (func_802AA890 reads them only for a count of 0), s2 is the
 * matrix func_802AA764 leaves there; outputs t0 = 0xF (the return value),
 * t2 = end of the point list, s1 = the chain's y', s4 = the buffer. The asm
 * also leaves f12/f14 as the sine routines did (not modelled) and clobbers
 * s2, s5-s7; asm callers keep t6/t7 live (a mixed N64 build would need a
 * thunk). */
s32 func_802D05D8(s32 a3, s32 s0, s32 s1, VehMtxOut *out) {
    u8 *model = D_803FC5B4;
    u8 *base;
    s32 *m;
    MtxChainRegs regs;
    s32 x;
    s32 y;
    s32 z;

    m = (s32 *) (*(s32 *) (model + *(s32 *) (model + 0x18) + 4) +
                 (s32) (D_8035805C != 0 ? D_803FC5B8 : D_803FC5BC));
    D_803ED390[1] = *(u16 *) (D_803FC500 + 0x4C);
    func_802AA764(D_803FC5A8[0], D_803FC5A8[1], D_803FC5A8[2], 0x32C8, m);
    base = (u8 *) (D_8035805C != 0 ? D_803FC5B8 : D_803FC5BC);
    model = D_803FC5B4;
    x = D_803FC5A8[0];
    y = D_803FC5A8[1];
    z = D_803FC5A8[2];
    regs.a3 = a3;
    regs.s1 = s1;
    regs.s2 = (s32) m;
    regs.s0 = s0;
    func_8029C454(x, y, z, 0xF, model + *(s32 *) (model + 4), model + *(s32 *) (model + 8), base, &regs);
    model = D_803FC5B4;
    regs.v1 = y;
    regs.a0 = z;
    func_802ABBEC(0xF, (s16 *) (model + *(s32 *) model), (s16 *) (model + *(s32 *) (model + 4)), base, &regs);
    out->t2 = (s32) (model + *(s32 *) (model + 4));
    out->s1 = regs.s1;
    out->s4 = (s32) base;
    return 0xF;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D05D8.s")
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

extern f32 D_8030D9D4;

/* Speed (s16 at +0x76 of D_803FC500, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9D4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CFDE8 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802D0710(void) {
    f32 div;
    s32 r;

    if (D_803FC500[0x96] == 1 || D_803FC500[0x97] == 1 || D_803FC500[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D9D4;
    }
    CVT_W_S(r, *(s16 *) (D_803FC500 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0710.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CFCE0 on t3, t4, f12, f14; func_802CFDE8 on a0-a3 (mixed N64 build
 * would need a thunk). */

void func_802D0784(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0784.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 16 (from the dispatcher func_802A350C), the shape of
 * func_802CF6A0: model D_803FC984, 0x1400-byte matrix buffers D_803FC988/98C,
 * func_802A1388(0x10, 1, ...), wheel offsets and speed bands, state bytes
 * +0xA1/A2 = 0 and +0xA3 = 1, the ground slots, the animation channels of
 * D_803FC5D0 (channel 0, then channel 1 also started), the parts (scale
 * 0x2134), func_80258230; one frame of func_802D0F98 with +0x9A set; then
 * func_802A7764 copies buffer 98C to 988, the matrix copy func_802AA838, and
 * D_803F7844 (the special-move sound handle) is cleared.
 * Register convention and leaked registers: as func_802CF6A0 (zone registers
 * and f14-f26 passed to func_802D0F98 as the stub model leaves them). */
void func_802D07E0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fpIn, s32 t6, TriSideOut *f) {
    u8 *v = D_803FC8D0;
    u8 *buf;
    u64 *b0;
    u64 *b1;
    s32 *s3v;
    s32 t7;

    D_803FC984 = model;
    buf = D_80358070;
    D_803FC988 = (u64 *) buf;
    D_803FC98C = (u64 *) (buf + 0x1400);
    D_80358070 = buf + 0x2800;
    func_802A1388(0x10, 1, (s32) D_803FC988, (s32) D_803FC98C, model);
    func_802A754C(v);
    *(s16 *) (v + 0x52) = 0;
    *(s16 *) (v + 0x54) = 0;
    *(s16 *) (v + 0x56) = 0;
    *(s16 *) (v + 0x58) = 0;
    *(s16 *) (v + 0x5A) = 0;
    *(s16 *) (v + 0x5C) = 0;
    *(s16 *) (v + 0x5E) = 0x50;
    *(s16 *) (v + 0x60) = 0x50;
    *(s16 *) (v + 0x62) = -0x50;
    *(s16 *) (v + 0x64) = 0x50;
    *(s16 *) (v + 0x66) = 0x50;
    *(s16 *) (v + 0x68) = -0x50;
    D_803FC978[0] = x;
    D_803FC978[1] = y;
    D_803FC978[2] = z;
    *(s16 *) (v + 0x4C) = heading;
    v[0xA1] = 0;
    v[0xA2] = 0;
    *(s16 *) (v + 0x4E) = heading;
    *(s16 *) (v + 0x74) = heading;
    v[0xA3] = 1;
    s3v = func_802A992C((s16 *) (v + 0x52), D_803FC978[1], x, z, (s32 *) (v + 4), (s32 *) &D_803FC978[1],
                        (s16 *) (v + 0x4C), 0x10, fpIn, v, f);
    b0 = D_803FC988;
    b1 = D_803FC98C;
    t7 = func_8029F85C((u32 *) b1, (u32 *) b0, D_803FC5D0, D_803FC984);
    func_802A039C(D_803FC5D0, 0, 0x64);
    func_802A03D4(D_803FC5D0, 0, 0);
    func_802A040C(D_803FC5D0, 0, 0);
    func_802A0480(0.0f, D_803FC5D0, 0, 0);
    func_802A0290(D_803FC5D0, 0, 1);
    func_8029E558((u8 *) b0, (u8 *) b1, D_803FC5D0);
    func_802A0320(0, D_803FC5D0);
    func_802A0290(D_803FC5D0, 0, 1);
    func_8029E558((u8 *) b1, (u8 *) b0, D_803FC5D0);
    *(s16 *) (v + 0x78) = -0x64;
    *(s16 *) (v + 0x7A) = 0;
    *(s16 *) (v + 0x7C) = 4;
    *(s16 *) (v + 0x7E) = 0;
    *(s16 *) (v + 0x80) = 0xA0;
    *(s16 *) (v + 0x82) = 4;
    *(s16 *) (v + 0x84) = 0;
    *(s16 *) (v + 0x86) = 0xA0;
    *(s16 *) (v + 0x88) = 4;
    *(s16 *) (v + 0x8A) = 0;
    *(s16 *) (v + 0x8C) = 0xA0;
    *(s16 *) (v + 0x8E) = 4;
    *(s16 *) (v + 0x90) = 0;
    *(s16 *) (v + 0x92) = 0xA0;
    *(s16 *) (v + 0x94) = 4;
    D_803FC998 = 0;
    D_803FC99A = 0;
    D_803FC999 = 0;
    model = D_803FC984;
    func_8029C354(0x10, model + *(s32 *) (model + 4), model + *(s32 *) (model + 8), 0x2134);
    func_80258230(0x10, 0x64, 0x2D, 0x2D);
    func_802A0360(0.0f, D_803FC5D0, 1, 0);
    func_802A0290(D_803FC5D0, 1, 1);
    v[0x9A] = 1;
    func_802D0F98(t6, t7, z, (s32) (v + 4), (s32) &D_803FC978[1], (s32) s3v, (s32) (v + 0x4C), f->cross, f->cz,
                  f->side, f->sideZ, f->dz);
    v[0x9A] = 0;
    func_802A7764(D_803FC98C, D_803FC988, 0x1400);
    model = D_803FC984;
    func_802AA838((u8 *) D_803FC98C, (u8 *) D_803FC988, *(s32 *) (model + *(s32 *) (model + 0x18) + 4));
    D_803F7844 = NULL;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D07E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 16
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1 or
 * its byte 0xA1 is nonzero, else 1 (shape shared with func_802CA140 in
 * 853D0.c). 00000.c declares it void and ignores the result, but the asm
 * returns 0/1 in v0. */
extern u8 D_803FC8D0[];

s32 func_802D0B90(void) {
    if (D_803FC8D0[0x96] == 1 || D_803FC8D0[0x97] == 1 || D_803FC8D0[0x98] == 1 || D_803FC8D0[0xA1] != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0B90.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC8D0[];
extern u64 *D_803FC988;
extern u64 *D_803FC98C;
extern void *D_803FC990; /* engine sound handle */
extern u8 D_803FC5D0[];  /* this vehicle's animation channel table */

/* Vehicle-type 16 exit: zero the speed (s16 at +0x76), copy 0x1400 bytes
 * between the D_803FC988/D_803FC98C buffers, clear animation channel 0x1F's
 * active flag (func_802A02E4), func_802C444C, then stop the engine sound
 * (func_802608C8(D_803FC990)). The survey lists v1 as an output read by
 * func_8024B188, but that caller is C and v1 is just func_802608C8's
 * leftover. */
void func_802D0BF8(void) {
    *(s16 *) (D_803FC8D0 + 0x76) = 0;
    func_802A7764(D_803FC988, D_803FC98C, 0x1400);
    func_802A02E4(0x1F, D_803FC5D0);
    func_802C444C();
    func_802608C8(D_803FC990);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0BF8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC8D0[]; /* this vehicle's state block */
extern void *D_80367738;
extern void *D_803FC990; /* engine sound handle */
extern u8 D_802C22D0[]; /* key of this vehicle's func_802A06B4 entry */
extern u8 D_803FC5D0[]; /* this vehicle's animation channel table */

/* Vehicle-type 16 setup (called from 00000.c / 17210.c), the shape of
 * func_802B1228: clears byte 0x99 of the state block, D_8036444C/50 = 2000,
 * -1000, starts the engine sound 0x50 (handle in D_803FC990), sets the
 * D_802C22D0 entry's fields (100, 0, 0, -1) and the animation channels 7, 8,
 * 9 (and 1, 5, 6, 3) of D_803FC5D0.
 * The asm also points $gp at D_803FC8D0 and leaves it there (conventions.txt:
 * clobbers gp) and returns with v1 = 0 (the last setter's v1); C callers
 * ignore both. */
void func_802D0C68(void) {
    D_803FC8D0[0x99] = 0;
    D_8036444C = 0x7D0;
    D_80364450 = -0x3E8;
    func_80260650(D_80367738, 0x50, &D_803FC990);
    func_802A05D0((s32) D_802C22D0, 0x64);
    func_802A05F8((s32) D_802C22D0, 0);
    func_802A0620((s32) D_802C22D0, 0);
    func_802A0508((s32) D_802C22D0, -1);
    func_802A039C(D_803FC5D0, 7, 2);
    func_802A040C(D_803FC5D0, 7, 1);
    func_802A0480(0.5f, D_803FC5D0, 7, 1);
    func_802A039C(D_803FC5D0, 8, 2);
    func_802A040C(D_803FC5D0, 8, 1);
    func_802A0480(0.5f, D_803FC5D0, 8, 1);
    func_802A039C(D_803FC5D0, 9, 4);
    func_802A040C(D_803FC5D0, 9, 1);
    func_802A0480(0.5f, D_803FC5D0, 9, 1);
    func_802A0480(0.5f, D_803FC5D0, 1, 1);
    func_802A0480(0.5f, D_803FC5D0, 5, 1);
    func_802A0480(0.5f, D_803FC5D0, 6, 1);
    func_802A0480(0.0f, D_803FC5D0, 3, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0C68.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Re-ground vehicle type 16, as func_802CFB00 (key 0x10, func_802D22F4). No
 * callers are known; same register convention as func_802CFB00. */
void func_802D0E44(s32 fpIn, s32 a3, f32 f12, f32 f14, f32 f20, f32 f22, f32 f24, f32 f26) {
    u8 *v = D_803FC8D0;
    TriSideOut f;
    Out802A9A60 o9;
    VehMtxOut mo;
    s32 x = D_803FC978[0];
    s32 z = D_803FC978[2];

    f.pz = f12;
    f.cross = f14;
    f.cz = f20;
    f.side = f22;
    f.sideZ = f24;
    f.dz = f26;
    func_802A9A60((s16 *) (v + 0x52), D_803FC978[1], x, z, (s32 *) (v + 4), (s32 *) &D_803FC978[1],
                  (s16 *) (v + 0x4C), 0x10, fpIn, v, &f, &o9);
    func_802D22F4(a3, z, (s32) o9.s1, &mo);
    func_802A133C(D_803FC978[2], 0x10, D_803FC978[0], D_803FC978[1], v);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0E44.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803FC978[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0x10 at its position
 * D_803FC978..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802D0F98 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802D0F54(ZoneScanRegs *r) {
    return func_802ABD54(0x10, D_803FC978[0], D_803FC978[1], D_803FC978[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0F54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 16 (from 00000.c's func_8024B7AC and once
 * from func_802D07E0), the shape of func_802CFDE8: zone level
 * (func_802D0F54), the animation state machine (func_802D1360 unless +0x9A),
 * timers (func_802D249C), steering with the rate from func_802D2444,
 * throttle (func_802A785C, delta 12), braking when state +0xA1 is 0
 * (func_802A77D0), heading (rate 0x59D8, no sound), slope and speed (kind
 * 0x10, no clamp, 120.0), func_802A7070 while D_803FC998, the move, ground
 * contact (D_803ED40B = 0, divisors 0x78), animation (func_8029E558 on
 * D_803FC5D0), model matrices (func_802D22F4), the vehicle-state reset, the
 * collision passes and func_802BE77C (always). On a ring hit (D_803A7425):
 * D_803FC998 = 1, D_803FC99A = 1 if the ring distance is under 0x190 (which
 * also makes the v1 handed to func_802A746C 1), and the ring steering with
 * scale 0.25 (the new heading also to D_803FC994); else D_803FC998 and
 * D_803FC99A are cleared. Finally D_803643E0.. and the D_80364460 record.
 * Register convention: as func_802CFDE8 (zone registers t6, t7, s0-s4 into
 * func_802D0F54, whose results are dead here; f14-f26 into func_802A8768).
 * func_8029A800 gets b0 = func_802D22F4's t0 (0x10) and h2 = its t2 (the
 * end of the model's point list), as the asm leaves them - a slip in the
 * original (func_802CFDE8 passes 7 and 0x96 there).
 * Leaked registers (FIDELITY): func_802D1360 (asm: func_8029F9D4) changes
 * f20/f30 in the game before f20 reaches func_802A8768; func_802D22F4's
 * a3/s0/s1 are the stub-model values (+0x34, +0x96, &z) where in the game
 * func_8029E558 has changed s0/s1. */
void func_802D0F98(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, f32 f14, f32 f20, f32 f22, f32 f24,
                   f32 f26) {
    u8 *v = D_803FC8D0;
    ZoneScanRegs zr;
    TriSideOut f;
    Regs802A8768 r8;
    Out802A860C o;
    VehMtxOut mo;
    f32 f12;
    s32 x;
    s32 tag;
    s32 d;
    s32 turn;
    s32 cur;
    s32 tgt;

    zr.t6 = t6;
    zr.t7 = t7;
    zr.s0 = s0;
    zr.s1 = s1;
    zr.s2 = s2;
    zr.s3 = s3;
    zr.s4 = s4;
    func_802D0F54(&zr);
    if (v[0x9A] == 0) {
        func_802D1360();
    }
    func_802D249C();
    func_802A7E70(func_802D2444(), (u16 *) (v + 0x4C));
    func_802A785C(v, (s16 *) (v + 0x76), 3, v + 0x96, (s16 *) (v + 0x78), 0xC);
    if (v[0xA1] == 0) {
        func_802A77D0(v);
    }
    func_802A7FD8(v, (u16 *) (v + 0x74), 0x59D8, (s16 *) (v + 0x76), (u16 *) (v + 0x4C), (u16 *) (v + 0x4E),
                  (s8 *) (v + 0x99), 0);
    f12 = func_802A83B8((s16 *) (v + 0x76), v + 0x96, (s32 *) (v + 4), (f32 *) v);
    func_802A843C(v, (s16 *) (v + 0x76), 0x10, (s8 *) (v + 0x96), (s32 *) (v + 4), 0, 120.0f);
    if (D_803FC998 != 0) {
        func_802A7070(v, &D_803FC994);
    }
    x = func_802A860C(f12, *(u16 *) (v + 0x4E), (s16 *) (v + 0x76), (s32 *) &D_803FC978[0],
                      (s32 *) &D_803FC978[2], &o);
    D_803ED40B = 0;
    r8.s3 = o.s3;
    f.pz = f12;
    f.cross = f14;
    f.cz = f20;
    f.side = f22;
    f.sideZ = f24;
    f.dz = f26;
    func_802A8768(v, 0x10, (s32 *) &D_803FC978[0], (s32 *) &D_803FC978[1], (s32 *) &D_803FC978[2], x, o.t1, 0x78,
                  0x78, (s16 *) (v + 0x4C), v + 0x96, (s16 *) (v + 0x52), (s32 *) (v + 0x28), (s32 *) (v + 0x40),
                  (s32 *) (v + 0x34), (s32 *) (v + 4), &r8, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803FC988, (u8 *) D_803FC98C, D_803FC5D0);
    } else {
        func_8029E558((u8 *) D_803FC98C, (u8 *) D_803FC988, D_803FC5D0);
    }
    tag = func_802D22F4((s32) (v + 0x34), (s32) (v + 0x96), (s32) &D_803FC978[2], &mo);
    func_8029A800(D_803FC978[2], (s32) D_80306470, 0, 0, D_803FC978[0], D_803FC978[1], tag, *(s16 *) (v + 0x76),
                  mo.t2, 0, 0x10, v);
    func_8029C52C(0x10, v);
    func_8029AA10(0x10);
    D_803F77D0 = D_803FC5D0;
    func_802BE77C(0x10, v);
    if (D_803A7425 != 0) {
        func_8029A914(v);
        D_803FC998 = 1;
        D_803FC99A = 0;
        turn = func_802A6F6C();
        PORT_RING_DIST(d, *(u16 *) (v + 0x4E), turn);
        if (d < 0x190) {
            d = 1;
            D_803FC99A = 1;
        }
        func_802A70D8(v);
        turn = func_802A71DC(v, *(u16 *) (v + 0x4E), *(u16 *) (v + 0x4C), &cur, 0.25f);
        D_803FC994 = cur;
        *(s16 *) (v + 0x4E) = cur;
        *(s16 *) (v + 0x74) = cur;
        func_802A746C(v, turn, d, &tgt);
        func_802A6FE4(v, 0);
    } else {
        D_803FC998 = 0;
        D_803FC99A = 0;
    }
    D_803643E0 = D_803FC978[0];
    D_803643E4 = D_803FC978[1];
    D_803643E8 = D_803FC978[2];
    D_8036443C = *(s16 *) (v + 0x76);
    D_8036443E = *(u16 *) (v + 0x4E);
    D_80364440 = *(u16 *) (v + 0x4C);
    func_802A133C(D_803FC978[2], 0x10, D_803FC978[0], D_803FC978[1], v);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0F98.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803FC996;  /* last channel unk13 seen (sound trigger) */
extern u8 D_803FC99A;
extern void *D_803F7844; /* sound handle */
extern f32 D_8030D9E0;

#define VEH16_U8(o) (D_803FC8D0[o])
#define VEH16_SPEED (*(s16 *) (D_803FC8D0 + 0x76))
/* func_802A04BC's results: [0] = unk10 (active), [1] = unk11, [6] = unk13. */
#define CHAN_GET(idx, e) func_802A04BC((idx), D_803FC5D0, (e))

/* Channel 0x1F: unk14 = val, unk11 = unk12 = 0, restart (func_802A0290). */
static void port_802D1360_chan1F(s32 val) {
    func_802A039C(D_803FC5D0, 0x1F, val);
    func_802A03D4(D_803FC5D0, 0x1F, 0);
    func_802A040C(D_803FC5D0, 0x1F, 0);
    func_802A0290(D_803FC5D0, 0x1F, 1);
}

/* Moving, no special state: drive the gear channel chosen by +0xA2 (0, 1, 2
 * -> channel 1, 5, 6). If it isn't active, pick a gear from |speed| (< 60,
 * < 110, else), restart that channel (val 2 if the inactive channel's unk11
 * was set, else 1), set +0xA2 and try again. An active channel gets
 * direction (speed < 0) and rate |speed| / 24. */
static void port_802D1360_gear(void) {
    s32 e[8];
    s32 idx;
    s32 v;

    switch (VEH16_U8(0xA2)) {
        case 0:
            idx = 1;
            break;
        case 1:
            idx = 5;
            break;
        case 2:
            idx = 6;
            break;
        default: /* the asm executes `syscall` here */
            return;
    }
    for (;;) {
        CHAN_GET(idx, e);
        if (e[0] != 0) {
            break;
        }
        v = VEH16_SPEED;
        if (v < 0) {
            v = -v;
        }
        if (v < 60) {
            idx = 1;
            VEH16_U8(0xA2) = 0;
        } else if (v < 110) {
            idx = 5;
            VEH16_U8(0xA2) = 1;
        } else {
            idx = 6;
            VEH16_U8(0xA2) = 2;
        }
        func_802A0290(D_803FC5D0, idx, (e[1] != 0) ? 2 : 1);
        func_802A0360(0.0f, D_803FC5D0, idx, 0);
    }
    v = VEH16_SPEED;
    func_802A03D4(D_803FC5D0, idx, v < 0);
    if (v < 0) {
        v = -v;
    }
    if (v != 0) {
        v = (u32) v / 24;
    }
    func_802A039C(D_803FC5D0, idx, v);
}

/* State 0 (+0xA1 == 0): driving. */
static void port_802D1360_state0(void) {
    s32 e[8];
    s32 r;
    s16 prev;

    /* the unk13 of the first active channel of 1, 5, 6 triggers a sound when
     * it changes to 6 (0x4E) or 2 (0x4F) */
    CHAN_GET(1, e);
    if (e[0] == 1 || (CHAN_GET(5, e), e[0] == 1) || (CHAN_GET(6, e), e[0] == 1)) {
        prev = D_803FC996;
        D_803FC996 = e[6];
        if (e[6] != prev) {
            if (e[6] == 6) {
                func_80260650(D_80367738, 0x4E, NULL);
            } else if (e[6] == 2) {
                func_80260650(D_80367738, 0x4F, NULL);
            }
        }
    }

    if (VEH16_SPEED == 0) {
        /* standing: leave the moving pose once (+0xA3), then idle anims */
        if (VEH16_U8(0xA3) != 0) {
            func_802A0360(0.0f, D_803FC5D0, 7, 0);
            if (CHAN_GET(0x1F, e), e[0] != 0) {
                func_802A02E4(0x1F, D_803FC5D0);
                func_8029F9D4(0x1F, 7, D_803FC5D0);
            } else if (CHAN_GET(1, e), e[0] != 0) {
                func_802A02E4(1, D_803FC5D0);
                func_8029F9D4(1, 7, D_803FC5D0);
            } else if (CHAN_GET(5, e), e[0] != 0) {
                func_802A02E4(5, D_803FC5D0);
                func_8029F9D4(5, 7, D_803FC5D0);
            } else if (CHAN_GET(6, e), e[0] != 0) {
                func_802A02E4(6, D_803FC5D0);
                func_8029F9D4(6, 7, D_803FC5D0);
            } else {
                func_802A0360(0.0f, D_803FC5D0, 1, 0);
                func_8029F9D4(1, 7, D_803FC5D0);
            }
            port_802D1360_chan1F(0x1E);
        }
        VEH16_U8(0xA3) = 0;
        if (CHAN_GET(0x1F, e), e[0] == 1) {
            return;
        }
        if (CHAN_GET(7, e), e[0] == 1) {
            return;
        }
        if (CHAN_GET(8, e), e[0] == 1) {
            return;
        }
        if (CHAN_GET(9, e), e[0] == 1) {
            return;
        }
        if (func_8026A8E0(0, 0x1E) != 0) {
            return;
        }
        r = func_8026A8E0(0, 2);
        if (r == 0) {
            r = 7;
        } else if (r == 1) {
            r = 8;
        } else {
            r = 9;
        }
        func_802A0360(0.0f, D_803FC5D0, r, 0);
        func_802A0290(D_803FC5D0, r, 1);
        return;
    }

    /* moving: enter the moving pose once (+0xA3) */
    if (VEH16_U8(0xA3) != 1) {
        func_802A0360(0.0f, D_803FC5D0, 1, 0);
        if (CHAN_GET(7, e), e[0] != 0) {
            func_8029F9D4(7, 1, D_803FC5D0);
            func_802A02E4(7, D_803FC5D0);
        } else if (CHAN_GET(8, e), e[0] != 0) {
            func_8029F9D4(8, 1, D_803FC5D0);
            func_802A02E4(8, D_803FC5D0);
        } else if (CHAN_GET(9, e), e[0] != 0) {
            func_8029F9D4(9, 1, D_803FC5D0);
            func_802A02E4(9, D_803FC5D0);
        } else {
            func_802A0360(0.0f, D_803FC5D0, 7, 0);
            func_8029F9D4(7, 1, D_803FC5D0);
        }
        port_802D1360_chan1F(0x28);
    }
    VEH16_U8(0xA3) = 1;
    if (CHAN_GET(0x1F, e), e[0] == 1) {
        return;
    }
    D_803F7804 = 0;
    if (((D_80370C35 == 0 && (D_80370C1C != 0 || D_80370C1D != 0)) || D_80370C1A != 0 || D_80370C1B != 0) &&
        VEH16_SPEED >= 0x96) {
        /* start the special move (state 1) from the current gear channel */
        VEH16_U8(0xA1) = 1;
        func_802A0360(0.0f, D_803FC5D0, 2, 0);
        switch (VEH16_U8(0xA2)) {
            case 0:
                r = 1;
                break;
            case 1:
                r = 5;
                break;
            case 2:
                r = 6;
                break;
            default: /* the asm executes `syscall` here */
                return;
        }
        func_8029F9D4(r, 2, D_803FC5D0);
        func_802A02E4(r, D_803FC5D0);
        port_802D1360_chan1F(0x50);
        func_80278EB0(6, D_8030D9E0, 0x64);
        return;
    }
    port_802D1360_gear();
}

/* States 1 and 2: the special move. Hitting something (+0x9C) halves the
 * speed and goes to state 3; +0x9D or D_803FC99A ends it (state 0);
 * otherwise speed = 280 and, in state 1, channel 2 is started (state 2) once
 * channel 0x1F has finished; in state 2 the move ends when channel 2 has. */
static void port_802D1360_state12(s32 state) {
    s32 e[8];

    if (VEH16_U8(0x9C) != 0) {
        VEH16_SPEED = VEH16_SPEED >> 1;
        if (state == 1) {
            func_802794A4();
            VEH16_U8(0xA1) = 3;
            func_802A0360(0.0f, D_803FC5D0, 3, 0);
            func_8029F9D4(0x1F, 3, D_803FC5D0);
        } else {
            VEH16_U8(0xA1) = 3;
            func_802A0360(0.0f, D_803FC5D0, 3, 0);
            func_8029F9D4(2, 3, D_803FC5D0);
            func_802A02E4(2, D_803FC5D0);
        }
        port_802D1360_chan1F(0x21);
        D_803F7804 = 1;
        return;
    }
    if (VEH16_U8(0x9D) == 0 && D_803FC99A == 0) {
        VEH16_SPEED = 0x118;
        if (state == 1) {
            if (CHAN_GET(0x1F, e), e[0] == 1) {
                return;
            }
            func_802A039C(D_803FC5D0, 2, 0xA);
            func_802A03D4(D_803FC5D0, 2, 0);
            func_802A040C(D_803FC5D0, 2, 0);
            func_802A0290(D_803FC5D0, 2, 1);
            VEH16_U8(0xA1) = 2;
            return;
        }
        if (CHAN_GET(2, e), e[0] == 1) {
            return;
        }
    }
    if (state == 1) {
        if (VEH16_SPEED >= 0) {
            VEH16_SPEED = 0x3C;
        }
        func_802C444C();
        func_802A0360(0.0f, D_803FC5D0, 1, 0);
        func_8029F9D4(0x1F, 1, D_803FC5D0);
    } else {
        func_802C444C();
        if (VEH16_SPEED >= 0) {
            VEH16_SPEED = 0x3C;
        }
        func_802A0360(0.0f, D_803FC5D0, 1, 0);
        func_802A02E4(2, D_803FC5D0);
        func_8029F9D4(2, 1, D_803FC5D0);
    }
    port_802D1360_chan1F(0x32);
    func_802794A4();
    VEH16_U8(0xA1) = 0;
}

/* Animation / state machine of vehicle type 16 ($gp = D_803FC8D0, read as
 * the global; channels in D_803FC5D0, an Unk8029DEA0Entry table), on the
 * state byte +0xA1: 0 driving (gears, idle anims, starting the special move
 * with the inputs at speed >= 150), 1 and 2 the special move, 3 the crash
 * (sound 0x4B, func_802BCC10, then channel 3 -> state 4), 4 recovering (back
 * to state 0 when channel 3 has finished). Sound 0x51 (handle D_803F7844) is
 * started on the special move. Out-of-range states/gears hit a `syscall` in
 * the asm; the C just returns (keep +0xA1 in 0..4 and +0xA2 in 0..2).
 * Register note: func_8029F9D4 changes f20 and f30 and the asm leaves them
 * that way; asm caller func_802D0F98 reads f20 afterwards (conventions.txt:
 * clobbers s5, f20, f30), which C can't reproduce. */
void func_802D1360(void) {
    s32 e[8];

    switch (VEH16_U8(0xA1)) {
        case 0:
            port_802D1360_state0();
            break;
        case 1:
            if (D_803F7844 == NULL) {
                func_80260650(D_80367738, 0x51, &D_803F7844);
            }
            port_802D1360_state12(1);
            break;
        case 2:
            port_802D1360_state12(2);
            break;
        case 3:
            if (D_803F7844 != NULL) {
                func_802C444C();
                func_80260650(D_80367738, 0x4B, NULL);
            }
            func_802BCC10();
            if (CHAN_GET(0x1F, e), e[0] == 1) {
                break;
            }
            D_803F7804 = 0;
            func_802794A4();
            func_802A039C(D_803FC5D0, 3, 5);
            func_802A03D4(D_803FC5D0, 3, 0);
            func_802A040C(D_803FC5D0, 3, 0);
            func_802A0290(D_803FC5D0, 3, 1);
            VEH16_U8(0xA1) = 4;
            break;
        case 4:
            func_802BCC10();
            if (CHAN_GET(3, e), e[0] == 1) {
                break;
            }
            D_803F7804 = 1;
            func_802A0360(0.0f, D_803FC5D0, 1, 0);
            func_802A0360(0.0f, D_803FC5D0, 5, 0);
            func_802A0360(0.0f, D_803FC5D0, 6, 0);
            VEH16_U8(0xA1) = 0;
            break;
        default: /* the asm executes `syscall` here */
            break;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D1360.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Model matrices of vehicle type 16, as func_802D05D8 but with scale 0x2134,
 * tag 0x10, buffers D_803FC988/98C, model D_803FC984, and the rotation set to
 * (0, heading + 0x400 wrapped by 0xFFF when >= 0x1000, 0). Same register
 * convention (outputs t0 = 0x10, t2, s1, s4: func_802D0F98 passes t0 and t2
 * on to func_8029A800). */
s32 func_802D22F4(s32 a3, s32 s0, s32 s1, VehMtxOut *out) {
    u8 *model = D_803FC984;
    u8 *base;
    s32 *m;
    MtxChainRegs regs;
    s32 a;
    s32 x;
    s32 y;
    s32 z;

    m = (s32 *) (*(s32 *) (model + *(s32 *) (model + 0x18) + 4) +
                 (s32) (D_8035805C != 0 ? D_803FC988 : D_803FC98C));
    a = *(u16 *) (D_803FC8D0 + 0x4C) + 0x400;
    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    D_803ED390[1] = a;
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    func_802AA764(D_803FC978[0], D_803FC978[1], D_803FC978[2], 0x2134, m);
    base = (u8 *) (D_8035805C != 0 ? D_803FC988 : D_803FC98C);
    model = D_803FC984;
    x = D_803FC978[0];
    y = D_803FC978[1];
    z = D_803FC978[2];
    regs.a3 = a3;
    regs.s1 = s1;
    regs.s2 = (s32) m;
    regs.s0 = s0;
    func_8029C454(x, y, z, 0x10, model + *(s32 *) (model + 4), model + *(s32 *) (model + 8), base, &regs);
    model = D_803FC984;
    regs.v1 = y;
    regs.a0 = z;
    func_802ABBEC(0x10, (s16 *) (model + *(s32 *) model), (s16 *) (model + *(s32 *) (model + 4)), base, &regs);
    out->t2 = (s32) (model + *(s32 *) (model + 4));
    out->s1 = regs.s1;
    out->s4 = (s32) base;
    return 0x10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D22F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0 when the speed (s16 at +0x76 of D_803FC8D0, the asm's $gp) is 0, else 5
 * when the byte at +0xA1 is 1..4, else 110. The asm returns it in s3 (see
 * tools_port/conventions.txt). Its asm caller func_802D0F98 keeps a0-a3 live
 * across the call (a mixed N64 build would need a thunk). Same shape as
 * func_802B28B8 (6C5E0). */
s32 func_802D2444(void) {
    u8 t;

    if (*(s16 *) (D_803FC8D0 + 0x76) == 0) {
        return 0;
    }
    t = D_803FC8D0[0xA1];
    if (t == 2 || t == 1 || t == 3 || t == 4) {
        return 5;
    }
    return 110;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D2444.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm caller func_802D0F98 relies on a0-a3
 * being preserved (mixed N64 build would need a thunk). */

void func_802D249C(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D249C.s")
#endif

/* func_802D24F8/func_802D2524: two-address trampolines into
 * func_802AC7DC/func_802AC85C, same confirmed-unreachable-from-C 8-byte
 * sd-$ra frame as func_802AC284 (hd_code/679E0.c). Permanently
 * GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC8D0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803FC978[]; /* plus these three words */

/* Serialize this vehicle's state (D_803FC8D0[0..0xA5] plus the three words
 * D_803FC978[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802D24F8(u8 *dst) {
    return func_802AC7DC(dst, D_803FC8D0, D_803FC978);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D24F8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802D24F8: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802D2524(void *)`. */
void func_802D2524(void *src) {
    func_802AC85C(src, D_803FC8D0, D_803FC978);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D2524.s")
#endif

/* func_802D2550: `mtc0 $a0, $11` (write COP0 Compare register) wrapped in a
 * dead $ra save/restore frame - same hand-written COP0-leaf-stub character
 * as __osSetSR in init/2330.c. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D2550.s")
