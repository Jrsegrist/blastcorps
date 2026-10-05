#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Port-phase rewrites (functional, not matching) of vehicle type 7. Its asm
 * points $gp at the block D_803EFDF0 (a fixed global here, so the C reads
 * it directly). Spawn, per-frame and model update follow the same pattern as
 * the three vehicles of 83910.c. */
#define VEH_U8(v, off) (*(u8 *) ((u8 *) (v) + (off)))
#define VEH_S8(v, off) (*(s8 *) ((u8 *) (v) + (off)))
#define VEH_U16(v, off) (*(u16 *) ((u8 *) (v) + (off)))
#define VEH_S16(v, off) (*(s16 *) ((u8 *) (v) + (off)))
#define VEH_S32(v, off) (*(s32 *) ((u8 *) (v) + (off)))
#define VEH_F32(v, off) (*(f32 *) ((u8 *) (v) + (off)))
/* obj + the word at obj + off (the model header's self-relative offsets) */
#define OBJ_PTR(obj, off) ((u8 *) (obj) + *(s32 *) ((u8 *) (obj) + (off)))

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
/* Registers func_802AA890 reads and writes besides its arguments (62740.c). */
typedef struct {
    s32 v1;
    s32 a0;
    s32 a3;
    s32 s1;
    s32 s2;
    s32 s0;
} MtxChainRegs;
/* FP results of the triangle scans (62740.c): f12, f14, f20, f22, f24, f26. */
typedef struct {
    f32 pz;
    f32 cross;
    f32 cz;
    f32 side;
    f32 sideZ;
    f32 dz;
} TriSideOut;
/* func_802A860C's results besides its return value (62740.c). */
typedef struct {
    s32 t1;
    s32 s3;
    s32 fp;
} Out802A860C;
/* func_802A8768's register results besides the FP state (62740.c). */
typedef struct {
    s32 s3;
    s16 *s4;
    s32 fp;
} Regs802A8768;

extern u8 D_803EFDF0[];  /* this vehicle's block (the asm's $gp) */
extern u8 D_803EFAF0[];  /* its animation channel table */
extern s32 D_803EFE98;   /* x */
extern s32 D_803EFE9C;   /* y */
extern s32 D_803EFEA0;   /* z */
extern u8 *D_803EFEA4;   /* model data */
extern u64 *D_803EFEA8;  /* save-buffer pair */
extern u64 *D_803EFEAC;
extern s32 D_803EFEB0;   /* x interpolation: x0, x1 */
extern s32 D_803EFEB4;   /* z interpolation: z0, z1 */
extern s32 D_803EFEB8;
extern s32 D_803EFEBC;
extern s32 D_803EFEC0;   /* frame counters */
extern s32 D_803EFEC4;
extern u8 D_803EFEC8;    /* x follows z along the line (D_803EFEB0..) */
extern u8 D_803EFEC9;
extern u8 D_803EFECA;
extern u8 D_803EFECB;
extern u8 *D_80358070;   /* heap pointer */
extern u8 D_8035805C;    /* which save buffer is current */
extern s16 D_803ED390[]; /* (0, heading, 0) for func_802AA764 */
extern u8 D_803ED40B;
extern u8 D_80305E00[];
extern u8 *D_803F77D0;
extern u8 D_803A7425;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern u16 D_8036443E;
extern u16 D_80364440;

void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_802A754C(u8 *veh);
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh,
                   TriSideOut *f);
s32 func_8029F85C(u32 *bufA, u32 *bufB, void *ch, u8 *hdr);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0320(s32 idx, void *base);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_8029E558(u8 *base, u8 *other, void *ch);
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
void func_802AA838(u8 *src, u8 *dst, s32 off);
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag, s32 sound);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);
void func_8029C52C(s32 tag, u8 *veh);
void func_8029AA10(s32 kind);
void func_802BE77C(s32 id, u8 *vehicle);
s32 func_802BCD80(s32 value);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);
s32 func_802AABE4(s32 id, u16 *desc, u8 *base, MtxChainRegs *regs, s16 **vertsOut);
void func_8029D040(s32 val, void *tbl, s32 x, s32 z, s32 id, u8 *model, u8 *mtxBase);
s32 func_8029DC14(s32 id);
s32 func_802BBE74(ZoneScanRegs *r);
void func_802BBEB8(void);
void func_802BC2C8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
void func_802BC3D0(void);
void func_802BC578(void);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 7, called from func_802A350C's object loop with the
 * spawn record in t7 (x), s3 (y), s0 (z), s1 (heading), s2 (model data) and
 * fp (the ground-scan byte the previous object left; see func_802A992C):
 * as 83910.c's veh83910_init (save buffers from the heap, model record,
 * block reset, wheel offsets, gear bands, ground, animation channel 0, parts
 * with scale 0x3A98, func_80258230, one frame (func_802BBEB8) with byte
 * +0x9A set, buffer copy), and in addition D_803EFEC9 = D_803EFECA = 0 and
 * both frame counters D_803EFEC0 / D_803EFEC4 = 999999.
 * The asm saves t0-t5: asm caller func_802A350C keeps t1 and t2 live (a
 * mixed N64 build would need a thunk). It leaves s2-s4, fp and f12-f26 as
 * its callees leave them, and func_802A350C hands fp and f12-f26 on to the
 * next object's spawn (not modelled; see the fidelity notes).
 * Fidelity: func_802A992C's FP inputs (the dispatcher's f12-f26) start at 0;
 * they only pass through to its FP outputs. */
void func_802BBA60(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    u8 *veh = D_803EFDF0;
    u8 *heap;
    u64 *a;
    u64 *b;
    TriSideOut tri;

    D_803EFEA4 = data;
    heap = D_80358070;
    D_803EFEA8 = (u64 *) heap;
    D_803EFEAC = (u64 *) (heap + 0x800);
    D_80358070 = heap + 0x1000;
    func_802A1388(7, 0, (s32) D_803EFEA8, (s32) D_803EFEAC, data);
    func_802A754C(veh);
    VEH_S16(veh, 0x52) = 0x50;
    VEH_S16(veh, 0x54) = 0x50;
    VEH_S16(veh, 0x56) = -0x50;
    VEH_S16(veh, 0x58) = 0x50;
    VEH_S16(veh, 0x5A) = 0x50;
    VEH_S16(veh, 0x5C) = -0x50;
    VEH_S16(veh, 0x5E) = 0x5A;
    VEH_S16(veh, 0x60) = 0x5A;
    VEH_S16(veh, 0x62) = -0x5A;
    VEH_S16(veh, 0x64) = 0x5A;
    VEH_S16(veh, 0x66) = 0x5A;
    VEH_S16(veh, 0x68) = -0x5A;
    D_803EFE98 = x;
    D_803EFE9C = y;
    D_803EFEA0 = z;
    VEH_S16(veh, 0x4C) = angle;
    VEH_S16(veh, 0x4E) = angle;
    VEH_S16(veh, 0x74) = angle;
    tri.pz = 0.0f;
    tri.cross = 0.0f;
    tri.cz = 0.0f;
    tri.side = 0.0f;
    tri.sideZ = 0.0f;
    tri.dz = 0.0f;
    func_802A992C(&VEH_S16(veh, 0x52), D_803EFE9C, x, z, &VEH_S32(veh, 4), &D_803EFE9C, &VEH_S16(veh, 0x4C), 7,
                  fp, veh, &tri);
    a = D_803EFEA8;
    b = D_803EFEAC;
    func_8029F85C((u32 *) b, (u32 *) a, D_803EFAF0, D_803EFEA4);
    func_802A039C(D_803EFAF0, 0, 0x64);
    func_802A03D4(D_803EFAF0, 0, 0);
    func_802A040C(D_803EFAF0, 0, 0);
    func_802A0480(0.0f, D_803EFAF0, 0, 0);
    func_802A0290(D_803EFAF0, 0, 1);
    func_8029E558((u8 *) a, (u8 *) b, D_803EFAF0);
    func_802A0320(0, D_803EFAF0);
    func_802A0290(D_803EFAF0, 0, 1);
    func_8029E558((u8 *) b, (u8 *) a, D_803EFAF0);
    VEH_S16(veh, 0x78) = -0xB4;
    VEH_S16(veh, 0x7A) = 0;
    VEH_S16(veh, 0x7C) = 1;
    VEH_S16(veh, 0x7E) = 0;
    VEH_S16(veh, 0x80) = 0x50;
    VEH_S16(veh, 0x82) = 1;
    VEH_S16(veh, 0x84) = 0x50;
    VEH_S16(veh, 0x86) = 0x8C;
    VEH_S16(veh, 0x88) = 1;
    VEH_S16(veh, 0x8A) = 0x8C;
    VEH_S16(veh, 0x8C) = 0xBE;
    VEH_S16(veh, 0x8E) = 1;
    VEH_S16(veh, 0x90) = 0xBE;
    VEH_S16(veh, 0x92) = 0xFA;
    VEH_S16(veh, 0x94) = 1;
    D_803EFEC9 = 0;
    D_803EFECA = 0;
    D_803EFEC0 = 999999;
    D_803EFEC4 = 999999;
    data = D_803EFEA4;
    func_8029C354(7, OBJ_PTR(data, 4), OBJ_PTR(data, 8), 0x3A98);
    func_80258230(7, 0x96, 0x2D, 0x2D);
    VEH_U8(veh, 0x9A) = 1;
    func_802BBEB8();
    VEH_U8(veh, 0x9A) = 0;
    data = D_803EFEA4;
    func_802AA838((u8 *) D_803EFEAC, (u8 *) D_803EFEA8, *(s32 *) (OBJ_PTR(data, 0x18) + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBA60.s")
#endif

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
/* (D_803EFE98 / 9C / A0 = x, y, z are declared at the top of the file.) */
/* Zone level lookup (func_802ABD54) for vehicle id 7 at its position
 * D_803EFE98..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BBEB8 keeps a0, f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BBE74(ZoneScanRegs *r) {
    return func_802ABD54(7, D_803EFE98, D_803EFE9C, D_803EFEA0, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBE74.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 7 (hd.c's vehicle switch, and the spawn):
 * zone lookup (func_802BBE74) whose scan registers feed the dust effect
 * func_802BC2C8 (only while byte +0x9A is clear), timers (func_802BC578);
 * throttle (func_802A785C), steering (func_802A7FD8), slope ratio
 * (func_802A83B8), drag (func_802A843C, kind 7), move along the heading
 * (func_802A860C). With D_803EFEC8 set, x is then pinned to the line from
 * (D_803EFEB0, D_803EFEB4) to (D_803EFEB8, D_803EFEBC) at the current z:
 * x = x0 + round((x1 - x0) * (z - z0) / (z1 - z0)) (the asm also rounds
 * 100 * the ratio into a register it then overwrites; z1 == z0 would make
 * that cvt.w.s fault). Ground contact (func_802A8768), animation channels
 * (func_8029E558), model update (func_802BC3D0), both frame counters + 1.
 * While byte +0x9A is clear: collisions (func_8029A800, func_8029C52C,
 * func_8029AA10, func_802BE77C) and, if they flagged D_803A7425, the bumper
 * check: with contact 3 (func_802BCD80) the speed becomes 0 if contact 2 is
 * also set, else D_803EFEC0 = 0 and -40 once D_803EFEC4 >= 6 (else 0); with
 * only contact 2, D_803EFEC4 = 0 and +40 once D_803EFEC0 >= 6 (else 0).
 * Finally position / speed / headings go to D_803643E0.. and func_802A133C.
 * The asm saves s0-s7, gp, fp and f20-f30. The asm's adds/subs trap on
 * overflow (game range).
 * Fidelity: func_802BBE74's scan registers are passed through from the
 * caller (t6, t7, s0-s4 as hd.c's compiled caller or the spawn leave them)
 * and reach func_802BC2C8's func_802A6274 records only when no zone is looked
 * at (empty zone list); the C starts them at 0. func_802A8768's FP inputs
 * (f12/f14 as func_802A860C's sine leaves them, the caller's f20-f26) start
 * at 0; they only reach its FP outputs, which nothing here reads. */
void func_802BBEB8(void) {
    u8 *veh = D_803EFDF0;
    ZoneScanRegs zr;
    Regs802A8768 r;
    Out802A860C o;
    TriSideOut tri;
    s32 x;
    s32 x0;
    s32 z0;
    s32 c2;
    s32 c3;
    f32 f;
    f32 k;

    zr.t6 = 0;
    zr.t7 = 0;
    zr.s0 = 0;
    zr.s1 = 0;
    zr.s2 = 0;
    zr.s3 = 0;
    zr.s4 = 0;
    func_802BBE74(&zr);
    if (VEH_S8(veh, 0x9A) == 0) {
        func_802BC2C8(zr.t6, zr.t7, zr.s0, zr.s1, zr.s2, zr.s3, zr.s4);
    }
    func_802BC578();
    func_802A785C(veh, &VEH_S16(veh, 0x76), 3, &VEH_U8(veh, 0x96), &VEH_S16(veh, 0x78), 6);
    func_802A7FD8(veh, &VEH_U16(veh, 0x74), 0x2328, &VEH_S16(veh, 0x76), &VEH_U16(veh, 0x4C),
                  &VEH_U16(veh, 0x4E), &VEH_S8(veh, 0x99), 0);
    f = func_802A83B8(&VEH_S16(veh, 0x76), &VEH_U8(veh, 0x96), &VEH_S32(veh, 4), &VEH_F32(veh, 0));
    func_802A843C(veh, &VEH_S16(veh, 0x76), 7, &VEH_S8(veh, 0x96), &VEH_S32(veh, 4), 1, 160.0f);
    x = func_802A860C(f, VEH_U16(veh, 0x4E), &VEH_S16(veh, 0x76), &D_803EFE98, &D_803EFEA0, &o);
    if (D_803EFEC8 != 0) {
        z0 = D_803EFEB4;
        k = (f32) (D_803EFEA0 - z0) / (f32) (D_803EFEBC - z0);
        x0 = D_803EFEB0;
        CVT_W_S(x, (f32) (D_803EFEB8 - x0) * k);
        x += x0;
        D_803EFE98 = x;
    }
    D_803ED40B = 0;
    r.s3 = o.s3;
    tri.pz = 0.0f;
    tri.cross = 0.0f;
    tri.cz = 0.0f;
    tri.side = 0.0f;
    tri.sideZ = 0.0f;
    tri.dz = 0.0f;
    func_802A8768(veh, 7, &D_803EFE98, &D_803EFE9C, &D_803EFEA0, x, o.t1, 0xA0, 0xA0, &VEH_S16(veh, 0x4C),
                  &VEH_U8(veh, 0x96), &VEH_S16(veh, 0x52), &VEH_S32(veh, 0x28), &VEH_S32(veh, 0x40),
                  &VEH_S32(veh, 0x34), &VEH_S32(veh, 4), &r, &tri);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803EFEA8, (u8 *) D_803EFEAC, D_803EFAF0);
    } else {
        func_8029E558((u8 *) D_803EFEAC, (u8 *) D_803EFEA8, D_803EFAF0);
    }
    func_802BC3D0();
    D_803EFEC0++;
    D_803EFEC4++;
    if (VEH_S8(veh, 0x9A) == 0) {
        /* b0 / h2 are the model update's leftover t0 (x) and t2 (7) */
        func_8029A800(D_803EFEA0, (s32) D_80305E00, 0, 0, D_803EFE98, D_803EFE9C, D_803EFE98,
                      VEH_S16(veh, 0x76), 7, 0, 7, veh);
        func_8029C52C(7, veh);
        func_8029AA10(7);
        D_803F77D0 = D_803EFAF0;
        func_802BE77C(7, veh);
        if (D_803A7425 != 0) {
            c2 = func_802BCD80(2);
            c3 = func_802BCD80(3);
            if (c3 != 0) {
                if (c2 != 0) {
                    VEH_S16(veh, 0x76) = 0;
                } else {
                    D_803EFEC0 = 0;
                    VEH_S16(veh, 0x76) = (D_803EFEC4 >= 6) ? -0x28 : 0;
                }
            } else if (c2 != 0) {
                D_803EFEC4 = 0;
                VEH_S16(veh, 0x76) = (D_803EFEC0 >= 6) ? 0x28 : 0;
            }
        }
    }
    D_803643E0 = D_803EFE98;
    D_803643E4 = D_803EFE9C;
    D_803643E8 = D_803EFEA0;
    D_8036443C = VEH_S16(veh, 0x76);
    D_8036443E = VEH_U16(veh, 0x4E);
    D_80364440 = VEH_U16(veh, 0x4C);
    func_802A133C(D_803EFEA0, 7, D_803EFE98, D_803EFE9C, veh);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BBEB8.s")
#endif

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
#ifdef NON_MATCHING
/* Model update of vehicle type 7: as 83910.c's veh83910_model (scale
 * 0x3A98, tag 7), then D_803EFECB = func_8029DC14(7). The asm reads the
 * block through $gp (always D_803EFDF0). It leaves t0 = x and t2 = 7 for
 * func_802BBEB8's func_8029A800 call (the C caller passes them itself) and
 * s0-s7, fp, f20-f28 as its callees do (conventions.txt: clobbers).
 * Fidelity: func_802AA890's extra registers (a3, s1, s2, s0 as the caller
 * and func_802AA764 leave them; read only for a part with an empty matrix
 * chain) start at 0 here. */
void func_802BC3D0(void) {
    u8 *veh = D_803EFDF0;
    u8 *data = D_803EFEA4;
    u8 *base;
    s16 *verts;
    MtxChainRegs regs;

    base = (u8 *) (D_8035805C != 0 ? D_803EFEA8 : D_803EFEAC);
    base += *(s32 *) (OBJ_PTR(data, 0x18) + 4);
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    D_803ED390[1] = VEH_U16(veh, 0x4C);
    func_802AA764(D_803EFE98, D_803EFE9C, D_803EFEA0, 0x3A98, (s32 *) base);

    base = (u8 *) (D_8035805C != 0 ? D_803EFEA8 : D_803EFEAC);
    regs.v1 = D_803EFE9C;
    regs.a0 = D_803EFEA0;
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = 0;
    regs.s0 = 0;
    data = D_803EFEA4;
    func_8029C454(D_803EFE98, D_803EFE9C, D_803EFEA0, 7, OBJ_PTR(data, 4), OBJ_PTR(data, 8), base, &regs);
    /* the asm's v1 / a0 (y / z) survive func_8029C454 */
    regs.v1 = D_803EFE9C;
    regs.a0 = D_803EFEA0;
    data = D_803EFEA4;
    func_802ABBEC(7, (s16 *) OBJ_PTR(data, 0), (s16 *) OBJ_PTR(data, 4), base, &regs);
    data = D_803EFEA4;
    func_802AABE4(7, (u16 *) OBJ_PTR(data, 8), base, &regs, &verts);
    data = D_803EFEA4;
    func_8029D040(VEH_U16(veh, 0x4C), D_803EFAF0, D_803EFE98, D_803EFEA0, 7, OBJ_PTR(data, 0xC), base);
    D_803EFECB = func_8029DC14(7);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/772A0/func_802BC3D0.s")
#endif

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
