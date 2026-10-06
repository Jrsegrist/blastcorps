#include "common.h"
#include <ultra64.h>
#include "game/regs.h"

#ifdef NON_MATCHING
/* Register-block types shared by this file's rewrites (each mirrors the
 * callee's definition in 62740.c / 60F60.c; later per-function copies are
 * guarded out). */
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
extern u8 D_803EEA90[];  /* vehicle 5's state block (the asm's $gp) */
extern u32 D_803EEB38[]; /* its position x, y, z */
extern u64 *D_803EEB48;  /* its two matrix buffers */
extern u64 *D_803EEB4C;
extern u8 *D_803EEB44;   /* its model header */
extern u8 D_803ED40B;
extern u8 D_8035805C;    /* selects which of the two matrix buffers is current */
void func_802B7030(MtxChainRegs *regs);
void func_802B7240(void);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
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
extern s32 D_80358070;   /* matrix buffer allocator */
extern u8 D_803EE790[];  /* vehicle 5's animation channels */
extern f32 D_80364414;   /* camera yaw (degrees) */
extern s8 D_803EEB60;
extern u8 D_803EEB5E;
extern u8 D_803EEB5F;
extern f32 D_803EEB50;
extern f32 D_803EEB54;
extern s16 D_803EEB5C;
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_802A754C(u8 *veh);
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh,
                   TriSideOut *f);
s32 func_8029F85C(u32 *bufA, u32 *bufB, void *ch, u8 *hdr);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0320(s32 idx, void *base);
void func_8029E558(u8 *base, u8 *other, void *ch);
void func_802A6F00(u8 *veh);
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
void func_802B6294(void);
void func_802AA838(u8 *src, u8 *dst, s32 off);

/* cvt.w.s under the FCSR's default rounding (nearest, ties to even). */
#define CVT_W_S_5900(out, x)                                            \
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

/* Level setup of vehicle 5 (asm caller func_802A350C, the level's object
 * list), the shape of func_802BAD80 (75490): model header -> D_803EEB44,
 * two 0x800-byte matrix buffers from D_80358070 (D_803EEB48 / D_803EEB4C;
 * the allocator advances by 0x1000), func_802A1388(5, 1, buffers, model),
 * func_802A754C on D_803EEA90, wheel-slot offsets (+-0x168, +-0x17C) at
 * +0x52..+0x68, position D_803EEB38 = (x, y, z), heading +0x4C/+0x4E/+0x74,
 * the ground slots (func_802A992C, key 5, y written back to D_803EEB3C), the
 * model channels (func_8029F85C on D_803EE790, channel 0 set up, both
 * buffers animated), the speed band table +0x78..+0x94 (-0x78, 0, 2, 0,
 * 0x50, 6, 0x50, 0x64, 4, 0x64, 0x8C, 2, 0x8C, 0xDC, 2), func_802A6F00;
 * D_803EEB60 = D_803EEB5E = 0, level D_803EEB5F = 50, D_803EEB50 =
 * D_803EEB54 = 0.5; parts (func_8029C354, tag 5, scale 0x4268);
 * func_80258230(5, 0x64, 0x2D, 0x2D); one update with +0x9A set
 * (func_802B6294); the root matrix copied from buffer B to A
 * (func_802AA838); finally D_803EEB5C = heading + round(camera yaw *
 * 4096 / 360), less 0xFFF when >= 0x1000. The asm's add/addi trap on
 * overflow.
 * Register convention (conventions.txt): model s2, x t7, y s3, z s0,
 * heading s1, and fp, which the asm hands to func_802A992C (reaching
 * D_803ED3F2 when a slot's grid scan finds no triangle: the dispatcher's
 * leftover fp, FIDELITY AUDIT); func_802A992C's FP inputs (passed through)
 * are 0 here. The asm restores t0-t5 (func_802A350C keeps t1/t2 live),
 * points $gp at D_803EEA90 and leaves s0-s7, fp and f20-f28 as its callees
 * leave them (clobbers). */
void func_802B5900(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp) {
    TriSideOut f;
    s32 buf;
    u8 *hdr;
    s32 yaw;
    s32 a;

    D_803EEB44 = model;
    buf = D_80358070;
    D_803EEB48 = (u64 *) buf;
    D_803EEB4C = (u64 *) (buf + 0x800);
    D_80358070 = buf + 0x1000;
    func_802A1388(5, 1, (s32) D_803EEB48, (s32) D_803EEB4C, model);
    func_802A754C(D_803EEA90);
    /* wheel slot offsets */
    *(s16 *) (D_803EEA90 + 0x52) = 0x168;
    *(s16 *) (D_803EEA90 + 0x54) = 0x168;
    *(s16 *) (D_803EEA90 + 0x56) = -0x168;
    *(s16 *) (D_803EEA90 + 0x58) = 0x168;
    *(s16 *) (D_803EEA90 + 0x5A) = 0x168;
    *(s16 *) (D_803EEA90 + 0x5C) = -0x168;
    *(s16 *) (D_803EEA90 + 0x5E) = 0x17C;
    *(s16 *) (D_803EEA90 + 0x60) = 0x17C;
    *(s16 *) (D_803EEA90 + 0x62) = -0x17C;
    *(s16 *) (D_803EEA90 + 0x64) = 0x17C;
    *(s16 *) (D_803EEA90 + 0x66) = 0x17C;
    *(s16 *) (D_803EEA90 + 0x68) = -0x17C;
    D_803EEB38[0] = x;
    D_803EEB38[1] = y;
    D_803EEB38[2] = z;
    *(s16 *) (D_803EEA90 + 0x4C) = heading;
    *(s16 *) (D_803EEA90 + 0x4E) = heading;
    *(s16 *) (D_803EEA90 + 0x74) = heading;
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A992C((s16 *) (D_803EEA90 + 0x52), D_803EEB38[1], x, z, (s32 *) (D_803EEA90 + 4),
                  (s32 *) &D_803EEB38[1], (s16 *) (D_803EEA90 + 0x4C), 5, fp, D_803EEA90, &f);
    hdr = D_803EEB44;
    func_8029F85C((u32 *) D_803EEB4C, (u32 *) D_803EEB48, D_803EE790, hdr);
    func_802A039C(D_803EE790, 0, 0x64);
    func_802A03D4(D_803EE790, 0, 0);
    func_802A040C(D_803EE790, 0, 0);
    func_802A0480(0.0f, D_803EE790, 0, 0);
    func_802A0290(D_803EE790, 0, 1);
    func_8029E558((u8 *) D_803EEB48, (u8 *) D_803EEB4C, D_803EE790);
    func_802A0320(0, D_803EE790);
    func_802A0290(D_803EE790, 0, 1);
    func_8029E558((u8 *) D_803EEB4C, (u8 *) D_803EEB48, D_803EE790);
    /* speed bands */
    *(s16 *) (D_803EEA90 + 0x78) = -0x78;
    *(s16 *) (D_803EEA90 + 0x7A) = 0;
    *(s16 *) (D_803EEA90 + 0x7C) = 2;
    *(s16 *) (D_803EEA90 + 0x7E) = 0;
    *(s16 *) (D_803EEA90 + 0x80) = 0x50;
    *(s16 *) (D_803EEA90 + 0x82) = 6;
    *(s16 *) (D_803EEA90 + 0x84) = 0x50;
    *(s16 *) (D_803EEA90 + 0x86) = 0x64;
    *(s16 *) (D_803EEA90 + 0x88) = 4;
    *(s16 *) (D_803EEA90 + 0x8A) = 0x64;
    *(s16 *) (D_803EEA90 + 0x8C) = 0x8C;
    *(s16 *) (D_803EEA90 + 0x8E) = 2;
    *(s16 *) (D_803EEA90 + 0x90) = 0x8C;
    *(s16 *) (D_803EEA90 + 0x92) = 0xDC;
    *(s16 *) (D_803EEA90 + 0x94) = 2;
    func_802A6F00(D_803EEA90);
    D_803EEB60 = 0;
    D_803EEB5E = 0;
    D_803EEB5F = 0x32;
    D_803EEB50 = 0.5f;
    D_803EEB54 = 0.5f;
    hdr = D_803EEB44;
    func_8029C354(5, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8), 0x4268);
    func_80258230(5, 0x64, 0x2D, 0x2D);
    D_803EEA90[0x9A] = 1;
    func_802B6294();
    D_803EEA90[0x9A] = 0;
    hdr = D_803EEB44;
    func_802AA838((u8 *) D_803EEB4C, (u8 *) D_803EEB48, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
    CVT_W_S_5900(yaw, D_80364414 * 4096.0f / 360.0f);
    a = *(u16 *) (D_803EEA90 + 0x4C) + yaw;
    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    D_803EEB5C = a;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5900.s")
#endif

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
#ifdef NON_MATCHING
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh,
                  TriSideOut *f, Out802A9A60 *out);

/* Place vehicle 5 on the ground at its position (called from hd.c's
 * func_8024B618 when the player switches to it): func_802A9A60 fills the
 * three wheel slots' height records (veh + 4) and sets the y position
 * D_803EEB38[1] (its `mid`) from them, key 5; then the model is rebuilt
 * (func_802B7030) and func_802A133C(z, 5, x, y, veh) called.
 * Leaked registers (FIDELITY AUDIT): func_802A9A60 also takes fp (stored
 * into D_803ED3F2[0..2], whose average goes to veh + 0x50) and f12-f26; the
 * asm passes on whatever its C caller (hd.c func_8024B618) has in $fp
 * ($s8) and the FP registers. This passes 0 for fp (veh + 0x50 = 0) and
 * the FP state, and 0 for func_802B7030's chain inputs (a3 is the caller's,
 * s1 / s0 func_802A9A60's dst + 9 and the z position; only read for a
 * zero-matrix point). The asm saves every s- and FP register; it returns
 * with v1 = func_802A133C's (preserved) y, unused by its C caller. */
void func_802B5FAC(void) {
    TriSideOut f;
    Out802A9A60 out;
    MtxChainRegs regs;

    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A9A60((s16 *) (D_803EEA90 + 0x52), D_803EEB38[1], D_803EEB38[0], D_803EEB38[2],
                  (s32 *) (D_803EEA90 + 4), (s32 *) &D_803EEB38[1], (s16 *) (D_803EEA90 + 0x4C), 5, 0, D_803EEA90,
                  &f, &out);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    func_802B7030(&regs);
    func_802A133C(D_803EEB38[2], 5, D_803EEB38[0], D_803EEB38[1], D_803EEA90);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B5FAC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
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
#ifdef NON_MATCHING
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r);
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r);
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz);
s32 func_802AB9A4(s16 *tbl, s32 x1, u16 *angle, s32 id, s32 z1, s32 x2, s32 z2, s32 *s3Out, InterpRegs *r);

/* Path-following setup for vehicle 5 (asm caller func_802AB50C, record
 * kind 5): the value pair at its position on triangle `id` (func_802AAD0C)
 * goes to s16 +0x6A/+0x6C of the state block, the heading (u16 +0x4C) to
 * +0x6E, and the pair at the position plus wheel slot 0's offset
 * (func_802A94A4 on veh + 0x52, rotated by the heading) to +0x70/+0x72.
 * Register convention (conventions.txt): id in a3; func_802AAD0C's FP results
 * (f24 also in, chained through both calls) pass through r. The asm saves and
 * restores t0, t1, t3, t4 (asm caller func_802AB50C keeps t0 and t1 live),
 * points $gp at D_803EEA90 and leaves s4 = veh + 0x4C (clobbers), and returns
 * with v1 = veh + 0x52 and t5/t6 = the slot offset; func_802AB50C doesn't
 * read them (the next record's function gets them as scratch). The asm's
 * `add`s trap on overflow. Same shape as func_802C59B4 (7FB50) plus the
 * second sample. */
void func_802B6100(s32 id, InterpRegs *r) {
    s32 dx;
    s32 dz;

    func_802AAD0C(id, D_803EEB38[0], D_803EEB38[2], r);
    *(s16 *) (D_803EEA90 + 0x6A) = r->t3;
    *(s16 *) (D_803EEA90 + 0x6C) = r->t4;
    *(s16 *) (D_803EEA90 + 0x6E) = *(u16 *) (D_803EEA90 + 0x4C);
    dx = func_802A94A4(0, (s16 *) (D_803EEA90 + 0x52), (s16 *) (D_803EEA90 + 0x4C), &dz);
    func_802AAD0C(id, D_803EEB38[0] + dx, D_803EEB38[2] + dz, r);
    *(s16 *) (D_803EEA90 + 0x70) = r->t3;
    *(s16 *) (D_803EEA90 + 0x72) = r->t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6100.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Path-following step of vehicle 5 (asm caller func_802AB714, record kind
 * 5): steer from the stored pair (s16 +0x6A/+0x6C) towards (+0x70/+0x72)
 * on triangle `id` with heading u16 +0x6E (func_802AB9A4); the result
 * becomes the heading (+0x4E and +0x4C). The pair at (+0x6A, +0x6C) is
 * re-sampled (func_802AAE54) and becomes the ground-contact position:
 * func_802B7240, D_803ED40B = 1, func_802A8768 (id 5, divisors 0x2D0 /
 * 0x2D0, s3 = func_802AB9A4's distance), the model rebuild func_802B7030 and
 * func_802A133C(z, 5, x, y, veh).
 * Register convention (conventions.txt): id in a3, f24 in, f24/f26 out (f),
 * result = func_802A8768's s3 (the asm's s3). The FP state chains as in the
 * asm: f24 -> func_802AB9A4 -> func_802AAE54 (f12-f26 out) ->
 * func_802A8768 (in/out). The asm saves and restores a3, t0, t1, t2 (asm
 * caller func_802AB714 keeps them live), points $gp at D_803EEA90 and leaves
 * s0-s2, s4-s7, fp, f20, f22 changed (clobbers). func_802B7030's chain inputs
 * (func_802A8768's leftover a3 / s1 / s0) are 0 here: only read for a
 * zero-matrix point. */
s32 func_802B618C(s32 id, TriSideOut *f) {
    InterpRegs ir;
    TriSideOut tf;
    Regs802A8768 r;
    MtxChainRegs regs;
    s32 s3;
    s32 h;

    ir.f24 = f->sideZ;
    h = func_802AB9A4((s16 *) (D_803EEA90 + 0x52), *(s16 *) (D_803EEA90 + 0x6A), (u16 *) (D_803EEA90 + 0x6E), id,
                      *(s16 *) (D_803EEA90 + 0x6C), *(s16 *) (D_803EEA90 + 0x70), *(s16 *) (D_803EEA90 + 0x72), &s3,
                      &ir);
    *(s16 *) (D_803EEA90 + 0x4E) = h;
    *(s16 *) (D_803EEA90 + 0x4C) = h;
    func_802AAE54(id, *(s16 *) (D_803EEA90 + 0x6A), *(s16 *) (D_803EEA90 + 0x6C), &ir);
    func_802B7240();
    D_803ED40B = 1;
    tf.pz = ir.f12;
    tf.cross = *(f32 *) &ir.f14;
    tf.cz = ir.f20;
    tf.side = ir.f22;
    tf.sideZ = ir.f24;
    tf.dz = ir.f26;
    r.s3 = s3;
    func_802A8768(D_803EEA90, 5, (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1], (s32 *) &D_803EEB38[2], ir.t3,
                  ir.t4, 0x2D0, 0x2D0, (s16 *) (D_803EEA90 + 0x4C), D_803EEA90 + 0x96, (s16 *) (D_803EEA90 + 0x52),
                  (s32 *) (D_803EEA90 + 0x28), (s32 *) (D_803EEA90 + 0x40), (s32 *) (D_803EEA90 + 0x34),
                  (s32 *) (D_803EEA90 + 4), &r, &tf);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    func_802B7030(&regs);
    func_802A133C(D_803EEB38[2], 5, D_803EEB38[0], D_803EEB38[1], D_803EEA90);
    f->sideZ = tf.sideZ;
    f->dz = tf.dz;
    return r.s3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B618C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80367BFF;
extern void *D_80367738;  /* sound player */
extern s16 D_803EEB58;    /* engine rate */
extern s16 D_803EEB5A;    /* heading kept while bouncing off */
extern u8 D_803EEB5E;     /* spawn cooldown */
extern s8 D_803EEB60;     /* collision flag */
extern s8 D_803EEB61;     /* sound to start this frame (-1 = none) */
extern s32 D_80364AA8;
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 D_80305D20[];
extern u8 D_802C2954[];   /* func_802A6274 definition */
extern f32 D_8030D8E0;
extern void *D_803F77D0;
extern s32 D_803643E0;    /* player position, speed, heading, angle */
extern s32 D_803643E4;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern s16 D_8036443E;
extern s16 D_80364440;
s32 func_802B60BC(ZoneScanRegs *r);
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2);
void func_802B6C28(void);
void func_802CB690(u8 *state);
s32 func_802B7168(void);
void func_802A7834(s32 rate, u16 *angle, u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag, s32 sound);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
void func_802A7070(u8 *veh, s16 *angle);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_8029E558(u8 *base, u8 *other, void *ch);
u8 func_802794F0(void);
void func_80278EB0(s32 n, f32 scale, s32 arg2);
void func_802794A4(void);
void func_802B69F8(void);
s32 func_802A5ED0(void);
void func_802AC284(s32 *px, s32 *py, s32 *pz);
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);
void func_8029C52C(s32 tag, u8 *veh);
void func_8029AA10(s32 kind);
void func_8029A914(u8 *veh);
s32 func_802A6F6C(void);
void func_802BE77C(s32 id, u8 *vehicle);
u64 *func_802A768C(u8 *veh, u8 *dst, s32 *w0, s32 *w1, s32 *w2, u64 *src, u64 *dst2, s32 size, u64 **dst2End);
void func_802A70D8(u8 *veh);
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale);
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut);
void func_802A6FE4(u8 *veh, s32 limit);
void func_802AC2A4(s32 z, s32 a1, s32 x, s32 y, s32 b0, s32 id, u8 *vehicle);
void *func_80260650(void *arg0, s16 arg1, void *arg2);

#define VEH5(off, T) (*(T *) (D_803EEA90 + (off)))

/* Per-frame update of vehicle 5 (D_803EEA90; called from hd.c's vehicle
 * switch and from func_802B5900):
 * 1. Zone lookup (func_802B60BC), func_802A75DC (position from D_803EE790),
 *    the animation/sound update func_802B6C28 while byte +0x9A is clear,
 *    func_802CB690 when D_80367BFF is set, func_802B7240.
 * 2. Driving: func_802A7834(rate = func_802B7168, heading +0x4C, speed +0x76,
 *    mode 3, flags +0x96, bands +0x78, 8), func_802A7FD8(+0x74, rate
 *    D_803EEB58, +0x4C -> +0x4E, flag +0x99, sound 1), f = func_802A83B8,
 *    func_802A843C(kind 5, clamp 1, 720.0), func_802A7070 when D_803EEB60 is
 *    set, the new position func_802A860C(f, +0x4E, ...), D_803ED40B = 1 and
 *    the ground contact func_802A8768 (id 5, divisors 0x2D0 / 0x2D0).
 * 3. Model: func_8029E558 on the current buffer pair; while +0x99 is set
 *    func_80278EB0(6, 0.25, 100) unless func_802794F0, else func_802794A4;
 *    the tyre trail func_802B69F8; the spawn cooldown D_803EEB5E (at 0 with
 *    +0x99 set: restart at 1 and, with fewer than 4 func_802A6274 records,
 *    one more: def D_802C2954, data 0x30D40, type 1 at (5, 1, 1));
 *    func_802AC284, the rebuild func_802B7030.
 * 4. Collision: func_8029A800 (table D_80305D20, 1, 1, b0 8, speed, 100, 0,
 *    kind 5), func_8029C52C(5), func_8029AA10(5); D_803EEB61 = 12. Without a
 *    hit (D_803A7425 == 0): D_803A7424 = 0, D_803EEB61 = -1 and, unless
 *    D_80364AA8 == 0x40, func_802BE77C(5) (D_803F77D0 = D_803EE790); if that
 *    set D_803A7424 the vehicle bounces: D_803EEB60 = 0, func_802A768C
 *    (restore the saved state, 0x800 bytes), speed = -max(|v|, 80) / 2 with
 *    v's sign (0 counts as positive) and the model rebuilt; else D_803EEB60
 *    = 0. With a hit: func_8029A914, D_803EEB60 = 1, then the heading
 *    correction: d = |((+0x4E - 0x800) wrapped by +0xFFF) - func_802A6F6C()|
 *    folded to <= 0x800, func_802A70D8, func_802A71DC(+0x4E -> +0x4C,
 *    D_8030D8E0) whose new heading goes to D_803EEB5A, +0x4E and +0x74,
 *    func_802A746C(its turn, d), func_802A6FE4(0) and, unless D_80364AA8 ==
 *    0x40, func_802BE77C(5).
 * 5. Player globals D_803643E0/E4/E8 = the position, D_8036443C/3E/40 =
 *    speed, +0x4E, +0x4C; func_802A133C(z, 5, x, y, veh); func_802AC2A4(z,
 *    D_80305D20, x, y, b0, 5, veh) with D_803F77D0 = D_803EE790; sound
 *    D_803EEB61 when >= 0 (func_80260650).
 * Leaked registers (FIDELITY AUDIT): func_802AC2A4's b0 is the asm's t0 =
 * &D_803643E8 (kept). Passed as 0 here: the zone scan's t6/t7/s0-s4 inputs
 * (the caller's registers, dead afterwards); func_802A8768's f14-f26 inputs
 * (pass-through); func_802B7030's chain inputs (only read for a
 * zero-matrix point); func_802A6274's t6/s1/w24/w28/w18/w1C/w2C (the asm
 * hands over t7 = &D_803EEB38 and func_8029E558's leftover s0/s2-s4 - after
 * the C callees func_802794F0 / func_80278EB0 / func_802794A4, which may
 * change t7; a type-1 record with z != 0 ignores them all). The asm's
 * `addi`/`sub` trap on overflow. It saves every s- and FP register it
 * changes and returns with f12/f14/v1 as its last callees leave them (read
 * by func_802B5900; not modelled). */
void func_802B6294(void) {
    ZoneScanRegs zr;
    MtxChainRegs regs;
    Regs802A8768 r;
    TriSideOut f;
    Out802A860C out;
    Io802A6274 io;
    u64 *end;
    f32 step;
    s32 x;
    s32 v;
    s32 d;
    s32 turn;
    s32 cur;
    s32 tgt;

    zr.t6 = 0;
    zr.t7 = 0;
    zr.s0 = 0;
    zr.s1 = 0;
    zr.s2 = 0;
    zr.s3 = 0;
    zr.s4 = 0;
    func_802B60BC(&zr);
    func_802A75DC(D_803EEA90, D_803EE790, (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1], (s32 *) &D_803EEB38[2]);
    if ((s8) D_803EEA90[0x9A] == 0) {
        func_802B6C28();
    }
    if (D_80367BFF != 0) {
        func_802CB690(D_803EEA90);
    }
    func_802B7240();

    /* 2. */
    func_802A7834(func_802B7168(), &VEH5(0x4C, u16), D_803EEA90, &VEH5(0x76, s16), 3, D_803EEA90 + 0x96,
                  &VEH5(0x78, s16), 8);
    func_802A7FD8(D_803EEA90, &VEH5(0x74, u16), (u16) D_803EEB58, &VEH5(0x76, s16), &VEH5(0x4C, u16),
                  &VEH5(0x4E, u16), (s8 *) (D_803EEA90 + 0x99), 1);
    step = func_802A83B8(&VEH5(0x76, s16), D_803EEA90 + 0x96, &VEH5(4, s32), (f32 *) D_803EEA90);
    func_802A843C(D_803EEA90, &VEH5(0x76, s16), 5, (s8 *) (D_803EEA90 + 0x96), &VEH5(4, s32), 1, 720.0f);
    if (D_803EEB60 != 0) {
        func_802A7070(D_803EEA90, &D_803EEB5A);
    }
    x = func_802A860C(step, VEH5(0x4E, u16), &VEH5(0x76, s16), (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[2],
                      &out);
    D_803ED40B = 1;
    r.s3 = out.s3;
    f.pz = step;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A8768(D_803EEA90, 5, (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1], (s32 *) &D_803EEB38[2], x, out.t1,
                  0x2D0, 0x2D0, &VEH5(0x4C, s16), D_803EEA90 + 0x96, &VEH5(0x52, s16), &VEH5(0x28, s32),
                  &VEH5(0x40, s32), &VEH5(0x34, s32), &VEH5(4, s32), &r, &f);

    /* 3. */
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803EEB48, (u8 *) D_803EEB4C, D_803EE790);
    } else {
        func_8029E558((u8 *) D_803EEB4C, (u8 *) D_803EEB48, D_803EE790);
    }
    if (D_803EEA90[0x99] != 0) {
        if (func_802794F0() == 0) {
            func_80278EB0(6, 0.25f, 100);
        }
    } else {
        func_802794A4();
    }
    func_802B69F8();
    if (D_803EEB5E != 0) {
        D_803EEB5E--;
    } else if (D_803EEA90[0x99] != 0) {
        D_803EEB5E = 1;
        if (func_802A5ED0() < 4) {
            io.a3 = 1;
            io.t6 = 0;
            io.s1 = 0;
            func_802A6274(&io, D_802C2954, 0x30D40, 1, 5, 1, 1, 0, 0, 0, 0, 0, 1);
        }
    }
    func_802AC284((s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1], (s32 *) &D_803EEB38[2]);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    func_802B7030(&regs);

    /* 4. */
    func_8029A800(D_803EEB38[2], (s32) D_80305D20, 1, 1, D_803EEB38[0], D_803EEB38[1], 8, VEH5(0x76, s16), 0x64, 0,
                  5, D_803EEA90);
    func_8029C52C(5, D_803EEA90);
    func_8029AA10(5);
    D_803EEB61 = 12;
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803EEB61 = -1;
        if (D_80364AA8 == 0x40) {
            goto no_hit;
        }
        D_803F77D0 = D_803EE790;
        func_802BE77C(5, D_803EEA90);
        if (D_803A7424 == 0) {
            goto no_hit;
        }
        D_803EEB60 = 0;
        if (D_8035805C != 0) {
            func_802A768C(D_803EEA90, D_803EE790, (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1],
                          (s32 *) &D_803EEB38[2], D_803EEB4C, D_803EEB48, 0x800, &end);
        } else {
            func_802A768C(D_803EEA90, D_803EE790, (s32 *) &D_803EEB38[0], (s32 *) &D_803EEB38[1],
                          (s32 *) &D_803EEB38[2], D_803EEB48, D_803EEB4C, 0x800, &end);
        }
        v = VEH5(0x76, s16);
        if (v < 0) {
            if (v > -0x50) {
                v = -0x50;
            }
        } else if (v < 0x50) {
            v = 0x50;
        }
        VEH5(0x76, s16) = -v >> 1;
        func_802B7030(&regs);
        goto globals;
    }
    func_8029A914(D_803EEA90);
    D_803EEB60 = 1;
    d = VEH5(0x4E, u16) - 0x800;
    if (d < 0) {
        d += 0xFFF;
    }
    d -= func_802A6F6C();
    if (d < 0) {
        d = -d;
    }
    if (d >= 0x801) {
        d = 0xFFF - d;
    }
    func_802A70D8(D_803EEA90);
    turn = func_802A71DC(D_803EEA90, VEH5(0x4E, u16), VEH5(0x4C, u16), &cur, D_8030D8E0);
    D_803EEB5A = cur;
    VEH5(0x4E, s16) = cur;
    VEH5(0x74, s16) = cur;
    func_802A746C(D_803EEA90, turn, d, &tgt);
    func_802A6FE4(D_803EEA90, 0);
    if (D_80364AA8 != 0x40) {
        D_803F77D0 = D_803EE790;
        func_802BE77C(5, D_803EEA90);
    }
    goto globals;
no_hit:
    D_803EEB60 = 0;

globals:
    /* 5. */
    D_803643E0 = D_803EEB38[0];
    D_803643E4 = D_803EEB38[1];
    D_803643E8 = D_803EEB38[2];
    D_8036443C = VEH5(0x76, s16);
    D_8036443E = VEH5(0x4E, u16);
    D_80364440 = VEH5(0x4C, u16);
    func_802A133C(D_803EEB38[2], 5, D_803EEB38[0], D_803EEB38[1], D_803EEA90);
    D_803F77D0 = D_803EE790;
    func_802AC2A4(D_803EEB38[2], (s32) D_80305D20, D_803EEB38[0], D_803EEB38[1], (s32) &D_803643E8, 5, D_803EEA90);
    if (D_803EEB61 >= 0) {
        func_80260650(D_80367738, D_803EEB61, NULL);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6294.s")
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
#ifdef NON_MATCHING
extern f32 D_80364414;   /* camera yaw (degrees) */
extern s16 D_803EEB5C;   /* angle offset */
extern f32 D_803EEB50;   /* channel-4 position, 0.5 = rest */
extern u8 D_803EEB5F;    /* level 0..100 */
extern f32 D_8030D8E4;   /* balance / position steps */
extern f32 D_8030D8E8;
extern f32 D_8030D8EC;
extern f32 D_8030D8F0;
extern f32 D_8030D8F4;
extern f32 D_8030D8F8;
extern u8 D_80370C15;    /* button bytes */
extern u8 D_80370C16;
extern u8 D_80370C1C;
extern u8 D_80370C23;
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A05A4(f32 f, s32 key, s32 val);
s32 func_802A7CB0(u8 *veh, s32 range);
f32 func_802B71DC(s32 side);
void func_802C4584(s32 level);

/* Moves f by step towards 0.5, stopping at 0.5 (NaN stays NaN). */
#define TOWARDS_HALF(f, step)       \
    if ((f) < 0.5f) {               \
        (f) += (step);              \
        if (!((f) <= 0.5f)) {       \
            (f) = 0.5f;             \
        }                           \
    } else {                        \
        (f) -= (step);              \
        if ((f) < 0.5f) {           \
            (f) = 0.5f;             \
        }                           \
    }

/* Per-frame animation/sound update of vehicle 5 (asm caller func_802B6294;
 * the asm reads the state block through $gp = D_803EEA90, read directly
 * here). The heading (+0x4C) plus the camera yaw in 4096ths, minus
 * D_803EEB5C, folded into 0..0x7FF, / 0x55 goes to the D_802C2208 and
 * D_802C226C entries (func_802A05A4). The balance D_803EEB54 moves by
 * D_8030D8E4 down to func_802B71DC(0) with D_80370C15, by D_8030D8E8 up to
 * func_802B71DC(1) with D_80370C16, else by D_8030D8EC towards 0.5, and goes to
 * channel 5. D_803EEB50 moves, unless func_802A7CB0(veh, 10) (near a gear
 * bound), by D_8030D8F0 down to 0 with D_80370C23 or by D_8030D8F4 up to 1
 * with D_80370C1C, otherwise by D_8030D8F8 towards 0.5, and goes to channel
 * 4. Channel 1 gets the direction (speed < 0) and |speed| / 14 (also passed to
 * func_802C4584); the level D_803EEB5F moves by 5 (down to 0 / up to 100) or
 * by 10 towards 50 and goes to channel 2 as level / 100.
 * The asm clobbers s2, s3, s5, s6 (conventions.txt). */
void func_802B6C28(void) {
    u8 *base = D_803EE790;
    s32 yaw;
    s32 a;
    f32 f;
    f32 lim;
    s32 speed;
    s32 level;
    s32 near;

    CVT_W_S(yaw, D_80364414 * 4096.0f / 360.0f);
    a = *(u16 *) (D_803EEA90 + 0x4C) + yaw;
    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    a -= D_803EEB5C;
    if (a < 0) {
        a += 0xFFF;
    }
    if (a >= 0x800) {
        a -= 0x800;
    }
    a = (u32) a / 0x55;
    func_802A05A4(0.0f, (s32) D_802C2208, a);
    func_802A05A4(0.0f, (s32) D_802C226C, a);

    f = D_803EEB54;
    if (D_80370C15 != 0) {
        f -= D_8030D8E4;
        lim = func_802B71DC(0);
        if (f < lim) {
            f = lim;
        }
    } else if (D_80370C16 != 0) {
        f += D_8030D8E8;
        lim = func_802B71DC(1);
        if (!(f <= lim)) {
            f = lim;
        }
    } else {
        TOWARDS_HALF(f, D_8030D8EC);
    }
    D_803EEB54 = f;
    func_802A0360(f, base, 5, 0);

    f = D_803EEB50;
    near = func_802A7CB0(D_803EEA90, 10);
    if (near == 0 && D_80370C23 != 0) {
        f -= D_8030D8F0;
        if (f < 0.0f) {
            f = 0.0f;
        }
    } else if (near == 0 && D_80370C1C != 0) {
        f += D_8030D8F4;
        if (!(f <= 1.0f)) {
            f = 1.0f;
        }
    } else {
        TOWARDS_HALF(f, D_8030D8F8);
    }
    D_803EEB50 = f;
    func_802A0360(f, base, 4, 0);

    speed = *(s16 *) (D_803EEA90 + 0x76);
    if (speed < 0) {
        func_802A03D4(base, 1, 1);
        speed = -speed;
    } else {
        func_802A03D4(base, 1, 0);
    }
    if (speed != 0) {
        speed = (u32) speed / 14;
    }
    func_802A039C(base, 1, speed);
    func_802C4584(speed);

    level = D_803EEB5F;
    if (D_80370C15 != 0) {
        level -= 5;
        if (level < 0) {
            level = 0;
        }
    } else if (D_80370C16 != 0) {
        level += 5;
        if (level >= 0x65) {
            level = 0x64;
        }
    } else if (level < 0x32) {
        level += 10;
        if (level >= 0x33) {
            level = 0x32;
        }
    } else {
        level -= 10;
        if (level < 0x32) {
            level = 0x32;
        }
    }
    D_803EEB5F = level;
    func_802A0360((f32) level / 100.0f, base, 2, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B6C28.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8035805C;   /* selects which of the two matrix buffers is current */
extern s16 D_803ED392;  /* model rotation y (62740.c) */
extern u8 *D_803EEB44;  /* this vehicle's model header */
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);

/* Rebuilds vehicle 5's model matrices and collision points. The model
 * header D_803EEB44 holds offsets (from the header) to: the part records at
 * +4 .. +8, the point list at +0 .. +4 and, at +0x18, a word whose +4 is the
 * root matrix's offset into the current matrix buffer (D_803EEB48 when
 * D_8035805C is set, else D_803EEB4C). With rotation y = the heading (u16
 * +0x4C of D_803EEA90, the asm's $gp) in D_803ED392, the root matrix is built
 * at position D_803EEB38[0..2] with scale 0x4268 (func_802AA764); then the
 * parts are placed (func_8029C454, tag 5) and the points transformed
 * (func_802ABBEC, id 5) through the current buffer.
 * Register convention: func_802AA890's chain registers pass through regs
 * (a3, s1, s0 in; v1, a0, a3, s1, s2 out; conventions.txt). As in the asm,
 * s2 = the root matrix (func_802AA764 leaves it there) and, for
 * func_802ABBEC, v1/a0 = position y/z (func_8029C454 preserves them). The
 * asm also leaves s4 = the buffer, s5/s6 = position y/z, s7 = 0x4268 and
 * func_802AA764's f12/f14 scratch (conventions.txt: clobbers); its asm
 * callers don't read them (the survey lists their later saves/passes).
 * Asm callers keep t6, t7 live. */
void func_802B7030(MtxChainRegs *regs) {
    u8 *hdr = D_803EEB44;
    u8 *buf = D_8035805C != 0 ? (u8 *) D_803EEB48 : (u8 *) D_803EEB4C;
    s32 *m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) + buf);

    D_803ED392 = *(u16 *) (D_803EEA90 + 0x4C);
    func_802AA764(D_803EEB38[0], D_803EEB38[1], D_803EEB38[2], 0x4268, m);
    regs->s2 = (s32) m;
    buf = D_8035805C != 0 ? (u8 *) D_803EEB48 : (u8 *) D_803EEB4C;
    hdr = D_803EEB44;
    func_8029C454(D_803EEB38[0], D_803EEB38[1], D_803EEB38[2], 5, hdr + *(s32 *) (hdr + 4),
                  hdr + *(s32 *) (hdr + 8), buf, regs);
    regs->v1 = D_803EEB38[1];
    regs->a0 = D_803EEB38[2];
    hdr = D_803EEB44;
    func_802ABBEC(5, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), buf, regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/71140/func_802B7030.s")
#endif

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
