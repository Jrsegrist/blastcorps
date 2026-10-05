#include "common.h"
#include <ultra64.h>

#ifdef NON_MATCHING
/* Register-block types shared by this file's rewrites (each mirrors the
 * callee's definition in 62740.c / 60F60.c; later per-function copies are
 * guarded out). */
#define TRI_SCAN_TYPES_DEFINED
/* func_802AC0BC's / the ground functions' FP register state (62740.c). */
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
/* func_802A860C's results besides t0 (62740.c). */
typedef struct {
    s32 t1; /* new z */
    s32 s3; /* *px as read */
    s32 fp; /* the cosine */
} Out802A860C;
/* func_802A8768's register results besides the FP state (62740.c). */
typedef struct {
    s32 s3;  /* in/out: the slot functions' result */
    s16 *s4; /* out */
    s32 fp;  /* out: the second tilt */
} Regs802A8768;
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
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* Registers func_802AA890 reads and writes besides its arguments (62740.c). */
typedef struct {
    s32 v1; /* out: y' >> 11 */
    s32 a0; /* out: z' >> 11 */
    s32 a3; /* in/out: the last matrix used */
    s32 s1; /* in/out: y' */
    s32 s2; /* in/out: z' */
    s32 s0; /* in: only read when count == 0 */
} MtxChainRegs;
#endif
extern s32 D_80358070;   /* matrix buffer allocator */
extern u8 *D_803EF704;   /* the 0xFF object's two 0xC00-byte matrix buffers */
extern u8 *D_803EF708;
extern u8 *D_803EF70C;   /* its model header */
extern u8 D_803EF630[];  /* its state block */
extern u8 D_803EF330[];  /* its animation channels */
extern s32 D_803EF6DC;   /* position x, y, z */
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
extern s32 D_803EF6F0;
extern s32 D_803EF6F4;   /* start point x, z */
extern s32 D_803EF6F8;
extern s16 D_803EF6FC;   /* target speed */
extern s16 D_803EF6D6;
extern u8 D_803EF6FE;
extern u8 D_803EF700;
extern u8 D_803EF701;
extern u8 D_802C236C[];  /* its func_802A06B4 entry key */
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);
void func_802A754C(u8 *veh);
void func_802BA074(void);
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
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
s32 func_802BABEC(MtxChainRegs *regs);
void func_802BA354(void);
void func_802AA838(u8 *src, u8 *dst, s32 off);
void func_802A0508(s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);

/* Level setup of the type-0xFF object (asm caller func_802A303C): model
 * header -> D_803EF70C, two 0xC00-byte matrix buffers from D_80358070
 * (D_803EF704 / D_803EF708; the allocator advances by 0x1800),
 * func_802A1388(0xFF, 0, buffers, model); position x/z (and the start point
 * D_803EF6F4/F8), target speed D_803EF6FC, D_803EF6F0 = limit, D_803EF6D6 =
 * 0; func_802A754C on D_803EF630, heading +0x4E/+0x4C; effect counters
 * D_803EF6FE = 5, D_803EF700 = D_803EF701 = 0; func_802BA074 (level spawn
 * point); wheel-slot offsets (+-0x190 / 0x4B0, +-0x1A4 / 0x4D8) at
 * +0x52..+0x68; the ground slots (func_802A992C at y 0x7FFF, writing y to
 * D_803EF6E0, key 0xFF); the model channels (func_8029F85C on D_803EF330,
 * channel 0 set up, both buffers animated); parts (func_8029C354, tag 0xFF,
 * scale 0x59D8); func_80258230(0xFF, 0x96, 0x3C, 0x3C); the model rebuild
 * func_802BABEC; one update with +0x9A set (func_802BA354); the root matrix
 * copied from buffer B to A (func_802AA838); channels 1 and 2 reset and
 * restarted with -1; the D_802C236C entry set (0x64, 0, 1) and restarted.
 * The asm's addi/add trap on overflow.
 * Register convention (conventions.txt): model s2, x t4, z t5, target speed
 * s1, limit t7, heading t6, and fp, which the asm hands to func_802A992C
 * (reaching D_803ED3F2 when a slot's grid scan finds no triangle: the
 * dispatcher's leftover fp, FIDELITY AUDIT). func_802A992C's FP inputs (the
 * dispatcher's, passed through) and func_802BABEC's chain inputs (a3 left by
 * the C func_80258230, s1 / s0 = 0 from func_8029F85C; only read for a
 * zero-matrix point) are 0 here. The asm restores t0-t5 and points $gp at
 * D_803EF630 (clobbers). */
void func_802B9C50(u8 *model, s32 x, s32 z, s32 speed, s32 limit, s32 heading, s32 fp) {
    TriSideOut f;
    MtxChainRegs regs;
    s32 buf;
    u8 *hdr;

    D_803EF70C = model;
    buf = D_80358070;
    D_803EF704 = (u8 *) buf;
    D_803EF708 = (u8 *) (buf + 0xC00);
    D_80358070 = buf + 0x1800;
    func_802A1388(0xFF, 0, (s32) D_803EF704, (s32) D_803EF708, model);
    D_803EF6DC = x;
    D_803EF6F4 = x;
    D_803EF6E4 = z;
    D_803EF6F8 = z;
    D_803EF6FC = speed;
    D_803EF6D6 = 0;
    D_803EF6F0 = limit;
    func_802A754C(D_803EF630);
    *(s16 *) (D_803EF630 + 0x4E) = heading;
    *(s16 *) (D_803EF630 + 0x4C) = heading;
    D_803EF6FE = 5;
    D_803EF700 = 0;
    D_803EF701 = 0;
    func_802BA074();
    /* wheel slot offsets */
    *(s16 *) (D_803EF630 + 0x52) = 0x190;
    *(s16 *) (D_803EF630 + 0x54) = 0x4B0;
    *(s16 *) (D_803EF630 + 0x56) = -0x190;
    *(s16 *) (D_803EF630 + 0x58) = 0x4B0;
    *(s16 *) (D_803EF630 + 0x5A) = 0x190;
    *(s16 *) (D_803EF630 + 0x5C) = -0x4B0;
    *(s16 *) (D_803EF630 + 0x5E) = 0x1A4;
    *(s16 *) (D_803EF630 + 0x60) = 0x4D8;
    *(s16 *) (D_803EF630 + 0x62) = -0x1A4;
    *(s16 *) (D_803EF630 + 0x64) = 0x4D8;
    *(s16 *) (D_803EF630 + 0x66) = 0x1A4;
    *(s16 *) (D_803EF630 + 0x68) = -0x4D8;
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A992C((s16 *) (D_803EF630 + 0x52), 0x7FFF, D_803EF6DC, D_803EF6E4, (s32 *) (D_803EF630 + 4),
                  &D_803EF6E0, (s16 *) (D_803EF630 + 0x4C), 0xFF, fp, D_803EF630, &f);
    hdr = D_803EF70C;
    func_8029F85C((u32 *) D_803EF708, (u32 *) D_803EF704, D_803EF330, hdr);
    func_802A039C(D_803EF330, 0, 0x64);
    func_802A03D4(D_803EF330, 0, 0);
    func_802A040C(D_803EF330, 0, 0);
    func_802A0480(0.0f, D_803EF330, 0, 0);
    func_802A0290(D_803EF330, 0, 1);
    func_8029E558(D_803EF704, D_803EF708, D_803EF330);
    func_802A0320(0, D_803EF330);
    func_802A0290(D_803EF330, 0, 1);
    func_8029E558(D_803EF708, D_803EF704, D_803EF330);
    hdr = D_803EF70C;
    func_8029C354(0xFF, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8), 0x59D8);
    func_80258230(0xFF, 0x96, 0x3C, 0x3C);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    func_802BABEC(&regs);
    D_803EF630[0x9A] = 1;
    func_802BA354();
    D_803EF630[0x9A] = 0;
    hdr = D_803EF70C;
    func_802AA838(D_803EF708, D_803EF704, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
    func_802A039C(D_803EF330, 1, 0);
    func_802A03D4(D_803EF330, 1, 0);
    func_802A040C(D_803EF330, 1, 0);
    func_802A0290(D_803EF330, 1, -1);
    func_802A039C(D_803EF330, 2, 0);
    func_802A03D4(D_803EF330, 2, 0);
    func_802A040C(D_803EF330, 2, 1);
    func_802A0290(D_803EF330, 2, -1);
    func_802A05D0((s32) D_802C236C, 0x64);
    func_802A05F8((s32) D_802C236C, 0);
    func_802A0620((s32) D_802C236C, 1);
    func_802A0508((s32) D_802C236C, -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802B9C50.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Looks up the current
 * level in a table of {level, x, z} records terminated by a negative level
 * and sets the spawn position (x, z in 1/32 units) from the matching record;
 * position stays 0 if the level isn't listed. The asm also saves/restores
 * v0, v1, a0; its asm caller func_802B9C50 relies on a1, a3, f12, f14 being
 * preserved (a mixed N64 build would need a thunk; the native port won't). */
typedef struct {
    s16 level;
    u16 x;
    u16 z;
} LevelPos;
extern s32 D_802E8BDC; /* current level */
extern LevelPos D_80305D74[];
extern s32 D_803EF6E8;
extern s32 D_803EF6EC;
extern u8 D_803EF710;
extern u8 D_803EF711;

void func_802BA074(void) {
    LevelPos *e;
    s32 level;

    D_803EF6E8 = 0;
    D_803EF710 = 0;
    D_803EF6EC = 0;
    D_803EF711 = 0;
    level = D_802E8BDC;
    for (e = D_80305D74; e->level >= 0; e++) {
        if (e->level == level) {
            D_803EF6E8 = e->x << 5;
            D_803EF6EC = e->z << 5;
            return;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA074.s")
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
extern s32 D_803EF6DC;
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
/* Zone level lookup (func_802ABD54) for vehicle id 0xFF at its position
 * D_803EF6DC..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BA354 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BA104(ZoneScanRegs *r) {
    return func_802ABD54(0xFF, D_803EF6DC, D_803EF6E0, D_803EF6E4, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA104.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rounded 3-D distance from (ax, ay, az) to (bx, by, bz) (62740.c; asm
 * convention in tools_port/conventions.txt: t3-t5, t6, t7, s0 -> s1). */
s32 func_802ABCDC(s32 ax, s32 ay, s32 az, s32 bx, s32 by, s32 bz);

extern s32 D_803643E0; /* player x, y, z */
extern s32 D_803643E4;
extern s32 D_803643E8;
extern void *D_80367738;  /* sound player */
extern void *D_803EF6D8;  /* this sound's handle, NULL = none */
extern s32 D_803EF6DC;    /* sound source x, y, z */
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);

/* Positional sound 0x75 at D_803EF6DC/E0/E4 (called from hd.c); shape of
 * func_802B8794 (72B80). d is the rounded distance from the player
 * D_803643E0/E4/E8. Beyond 16000 the sound is stopped (func_802608C8) and
 * its handle cleared. Otherwise it is started if it isn't playing, its
 * volume (parameter 8) set to 0x7FFF - 2 * max(d - 4000, 0) and its pan
 * (parameter 4) to 64 + (player x - source x) / 32, clamped to 0..127. The
 * asm's `sub`/`add` trap on overflow; C doesn't. Register note: the asm
 * saves and restores every register. */
void func_802BA148(void) {
    s32 dx = D_803643E0 - D_803EF6DC;
    s32 dist;
    s32 pan;

    dist = func_802ABCDC(D_803643E0, D_803643E4, D_803643E8, D_803EF6DC, D_803EF6E0, D_803EF6E4);
    if (dist > 16000) {
        if (D_803EF6D8 != NULL) {
            func_802608C8(D_803EF6D8);
            D_803EF6D8 = NULL;
        }
        return;
    }
    if (D_803EF6D8 == NULL) {
        func_80260650(D_80367738, 0x75, &D_803EF6D8);
    }
    dist -= 4000;
    if (dist < 0) {
        dist = 0;
    }
    func_80260AB8(D_803EF6D8, 8, 0x7FFF - (dist << 1));
    pan = 0x40 + (dx >> 5);
    if (pan < 0) {
        pan = 0;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    func_80260AB8(D_803EF6D8, 4, pan);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA148.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* Registers func_802AA890 reads and writes besides its arguments (62740.c). */
typedef struct {
    s32 v1; /* out: y' >> 11 */
    s32 a0; /* out: z' >> 11 */
    s32 a3; /* in/out: the last matrix used */
    s32 s1; /* in/out: y' */
    s32 s2; /* in/out: z' */
    s32 s0; /* in: only read when count == 0 */
} MtxChainRegs;
#endif
extern u8 D_803EF630[];
extern u8 D_803EF330[];
extern u8 D_803643D6;
extern u8 D_802E8BD0;
extern u8 D_8035805C;
extern u8 D_803ED40B;
extern s16 D_803EF6FC;
extern u8 *D_803EF704;
extern u8 *D_803EF708;
s32 func_802BA5A4(void);
s32 func_802BA6AC(void);
s32 func_802BA638(s32 *termZ, s32 *lastLevel, s32 *level);
void func_802BAD24(void);
s32 func_802BABEC(MtxChainRegs *regs);
void func_802BA9A0(s32 h2);
void func_802BA91C(void);
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f);
void func_8029E558(u8 *base, u8 *other, void *ch);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);

#define VEH_BA354(off, T) (*(T *) (D_803EF630 + (off)))

/* Per-frame update of the type-0xFF object D_803EF630 (called from hd.c's
 * func_802475D8 and from func_802B9C50): zone lookup (func_802BA104), the
 * distance flags (func_802BA5A4); then, while byte +0x9A is clear, the engine
 * sound/animation (func_802BA6AC), skipping the movement when D_803643D6 or
 * D_802E8BD0 is set. Movement: the zone value scan (func_802BA638, which may
 * set the target speed D_803EF6FC); the speed (s16 +0x76) rises by 8 towards
 * D_803EF6FC (never above it) when below it; func_802BAD24; the drive step
 * f = func_802A83B8, func_802A843C(kind 0xFF, divisor 2400.0); the new
 * position func_802A860C(f, heading u16 +0x4E, speed, x, z); D_803ED40B = 1;
 * ground contact func_802A8768 (id 0xFF, divisors 0x960 / 0x320). Always:
 * animate the model (func_8029E558 on the current buffer pair), rebuild it
 * (func_802BABEC), the collision/effect step func_802BA9A0 while +0x9A is
 * clear, func_802BA91C and func_802A133C(z, 0xFF, x, y, veh).
 * Leaked registers (FIDELITY AUDIT, see port_followups): the zone scan's
 * t6/t7/s0-s4 inputs (the caller's registers; only passed through when no
 * zone is looked at, and dead afterwards), func_802BA638's a2 input and its
 * a0-a3 results (dead: func_802A860C doesn't read a0-a3), the chain
 * registers a3/s1/s0 going into func_802BABEC (only read for a zero-matrix
 * point, which the model data doesn't have) and func_802A8768's FP inputs
 * other than f12 (f14, f20-f26: the caller's or earlier callees' leftovers,
 * passed through) are 0 here. The asm's `addi` traps on overflow. The asm
 * saves every s- and FP register it changes; it returns with f12/f14/v1 as
 * its last callees leave them (read by func_802B9C50; not modelled). */
void func_802BA354(void) {
    ZoneScanRegs zr;
    MtxChainRegs regs;
    Regs802A8768 r;
    TriSideOut f;
    Out802A860C out;
    s32 termZ;
    s32 lastLevel;
    s32 level;
    s32 speed;
    s32 x;
    f32 step;
    s32 end;

    zr.t6 = 0;
    zr.t7 = 0;
    zr.s0 = 0;
    zr.s1 = 0;
    zr.s2 = 0;
    zr.s3 = 0;
    zr.s4 = 0;
    func_802BA104(&zr);
    func_802BA5A4();
    if ((s8) D_803EF630[0x9A] == 0) {
        func_802BA6AC();
        if ((s8) D_803643D6 != 0 || (s8) D_802E8BD0 != 0) {
            goto animate;
        }
    }
    lastLevel = 0;
    func_802BA638(&termZ, &lastLevel, &level);
    speed = VEH_BA354(0x76, s16);
    if (speed < D_803EF6FC) {
        speed += 8;
        if (D_803EF6FC < speed) {
            speed = D_803EF6FC;
        }
    }
    VEH_BA354(0x76, s16) = speed;
    func_802BAD24();
    step = func_802A83B8(&VEH_BA354(0x76, s16), D_803EF630 + 0x96, &VEH_BA354(4, s32), (f32 *) D_803EF630);
    func_802A843C(D_803EF630, &VEH_BA354(0x76, s16), 0xFF, (s8 *) (D_803EF630 + 0x96), &VEH_BA354(4, s32), 0,
                  2400.0f);
    x = func_802A860C(step, VEH_BA354(0x4E, u16), &VEH_BA354(0x76, s16), &D_803EF6DC, &D_803EF6E4, &out);
    D_803ED40B = 1;
    r.s3 = out.s3;
    f.pz = step;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A8768(D_803EF630, 0xFF, &D_803EF6DC, &D_803EF6E0, &D_803EF6E4, x, out.t1, 0x960, 0x320,
                  &VEH_BA354(0x4C, s16), D_803EF630 + 0x96, &VEH_BA354(0x52, s16), &VEH_BA354(0x28, s32),
                  &VEH_BA354(0x40, s32), &VEH_BA354(0x34, s32), &VEH_BA354(4, s32), &r, &f);

animate:
    if (D_8035805C != 0) {
        func_8029E558(D_803EF704, D_803EF708, D_803EF330);
    } else {
        func_8029E558(D_803EF708, D_803EF704, D_803EF330);
    }
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    end = func_802BABEC(&regs);
    if ((s8) D_803EF630[0x9A] == 0) {
        func_802BA9A0(end);
    }
    func_802BA91C();
    func_802A133C(D_803EF6E4, 0xFF, D_803EF6DC, D_803EF6E0, D_803EF630);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA354.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF6F4; /* second point x, z (y = 0) */
extern s32 D_803EF6F8;

/* Distance (func_802ABCDC, y = 0) from (D_803EF6DC, D_803EF6E4) to
 * (D_803EF6F4, D_803EF6F8); sets D_803EF710 when it is >= D_803EF6E8 and
 * D_803EF711 when it is >= D_803EF6EC (signed). Returns the distance (the
 * asm leaves it in s1; conventions.txt). The asm also leaves t7 = 0 and
 * s0 = D_803EF6F8; asm caller func_802BA354 keeps a2, a3, f12, f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BA5A4(void) {
    s32 dist = func_802ABCDC(D_803EF6DC, 0, D_803EF6E4, D_803EF6F4, 0, D_803EF6F8);

    if (dist >= D_803EF6E8) {
        D_803EF710 = 1;
    }
    if (dist >= D_803EF6EC) {
        D_803EF711 = 1;
    }
    return dist;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA5A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    s16 z;    /* negative ends the table */
    u8 level;
    u8 value;
} ZoneEntry;
extern ZoneEntry D_80305D62[];
extern s16 D_803EF6FC;

/* Scans D_80305D62 (terminated by a negative z) for entries of the current
 * level (D_802E8BDC) whose z <= player z / 32 (D_803EF6E4 >> 5); the last
 * one's value wins (0 if none). A nonzero result is stored in D_803EF6FC.
 * The asm leaves its scan registers behind and asm caller func_802BA354
 * doesn't overwrite them before func_802A860C reads a0-a3, so they are all
 * outputs (see tools_port/conventions.txt): result (a0), the terminator z
 * (a1), the last level byte read (a2, unchanged if the table is empty) and
 * the current level (a3). */
s32 func_802BA638(s32 *termZ, s32 *lastLevel, s32 *level) {
    s32 pz = D_803EF6E4 >> 5;
    s32 lv = D_802E8BDC;
    s32 result = 0;
    ZoneEntry *e;
    s32 z;

    for (e = D_80305D62; (z = e->z) >= 0; e++) {
        *lastLevel = e->level;
        if (e->level == lv && !(pz < z)) {
            result = e->value;
        }
    }
    if (result != 0) {
        D_803EF6FC = result;
    }
    *termZ = z;
    *level = lv;
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA638.s")
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

extern u8 D_803EF630[];  /* this vehicle's state block (the asm's $gp) */
extern s16 D_803EF6D6;   /* last speed sent to the engine sound */
extern f32 D_8030D920;
extern f32 D_8030D924;
extern f32 D_8030D928;
extern u16 D_80364452;   /* camera / player angle (0..0xFFF) */
extern u8 D_802E8BD0;
extern u8 D_803EF330[];  /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);

/* Engine sound and animation channels for vehicle D_803EF630 (speed = s16 at
 * +0x76). When the speed changed since the last call (D_803EF6D6) and the
 * sound D_803EF6D8 is playing, its parameter 0x10 is set to the float
 * 1.5 + speed * D_8030D920 (passed as raw bits). Then a = (D_80364452 +
 * 0x800) wrapped by -0xFFF at 0x1000; channel 2 gets value a / 0x555
 * (0, 1, else 2) and fraction (a % 0x555) / D_8030D924 (func_802A0360);
 * channel 1 gets round(speed * D_8030D928) (0 while D_802E8BD0 is set,
 * func_802A039C). Returns a / 0x555 (the asm's s4; conventions.txt). The asm
 * also clobbers s5; asm caller func_802BA354 keeps a2, a3, t7, f12, f14 live
 * (a mixed N64 build would need a thunk; the native port won't). The asm's
 * `addi` traps on overflow, which can't happen for a u16. */
s32 func_802BA6AC(void) {
    s32 speed = *(s16 *) (D_803EF630 + 0x76);
    s16 last = D_803EF6D6;
    u32 a;
    u32 q;
    s32 v;
    f32 frac;

    D_803EF6D6 = speed;
    if (last != speed && D_803EF6D8 != NULL) {
        f32 p = 1.5f + (f32) speed * D_8030D920;

        func_80260AB8(D_803EF6D8, 0x10, *(s32 *) &p);
    }
    a = D_80364452 + 0x800;
    if ((s32) a >= 0x1000) {
        a -= 0xFFF;
    }
    q = a / 0x555;
    frac = (f32) (s32) (a % 0x555) / D_8030D924;
    if (q == 0) {
        func_802A0360(frac, D_803EF330, 2, 0);
    } else if (q == 1) {
        func_802A0360(frac, D_803EF330, 2, 1);
    } else {
        func_802A0360(frac, D_803EF330, 2, 2);
    }
    v = D_802E8BD0 != 0 ? 0 : *(s16 *) (D_803EF630 + 0x76);
    CVT_W_S(v, (f32) v * D_8030D928);
    func_802A039C(D_803EF330, 1, v);
    return q;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA6AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF6F0;
extern u8 D_803643DA;
extern u8 D_802E8BD8;

/* Distance (func_802ABCDC, y = 0) from (D_803EF6DC, D_803EF6E4) to
 * (D_803EF6F4, D_803EF6F8); when it is >= D_803EF6F0 (signed) sets
 * D_803643DA and D_802E8BD8 to 1. The asm leaves the distance in s1 and
 * D_803EF6F8 in s0 (conventions.txt: clobbers s0, s1); asm caller
 * func_802BA354 keeps a2, a3, f12, f14 live (a mixed N64 build would need a
 * thunk; the native port won't). */
void func_802BA91C(void) {
    s32 dist = func_802ABCDC(D_803EF6DC, 0, D_803EF6E4, D_803EF6F4, 0, D_803EF6F8);

    if (dist >= D_803EF6F0) {
        D_803643DA = 1;
        D_802E8BD8 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA91C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef IO_802A6274_DEFINED
#define IO_802A6274_DEFINED
/* The three registers func_802A6274 takes and may hand back changed (60F60.c). */
typedef struct {
    s32 a3; /* in: tag byte for D_803EB792; out: last D_803EB770 byte scanned */
    s32 t6; /* in: word for rec+0x20; out: (s16) def[0] */
    s32 s1; /* in: word for rec+0x14; out: the record's buffer */
} Io802A6274;
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
#endif
extern u8 D_803643D6;
extern u8 D_803643D8;
extern u8 D_803643D9;
extern u8 D_80305D60[];
extern u8 D_802C3B44[]; /* func_802A6274 definition */
extern u8 D_803A7424;
extern u8 D_803A7426;
extern void *D_803F77D0;
extern u8 D_803EF6FE;   /* countdown before the effect */
extern u8 D_803EF6FF;   /* effect finished */
extern u8 D_803EF700;   /* effect frame count */
extern u8 D_803EF701;   /* effect started */
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);
void func_8029C52C(s32 tag, u8 *veh);
void func_8029AA10(s32 kind);
void func_802BE77C(s32 id, u8 *vehicle);
void func_80278318(void);
void func_802A02E4(s32 idx, void *base);
void func_802A5E60(void);
void func_802C049C(void);

#define ABS_BA9A0(v) ((v) < 0 ? -(v) : (v))

/* Collision/effect step of the type-0xFF object (D_803EF630, the asm's $gp).
 * Unless D_803643D6 is set: func_8029A800((x, y, z) = D_803EF6DC/E0/E4, table
 * D_80305D60, 0, 0, b0, speed (s16 +0x76), h2, 0, 0xFF), func_8029C52C(0xFF, D_803EF630);
 * then, unless D_803A7426 is set, D_803A7424 = 0, func_8029AA10(0xFF) when
 * D_803EF710 is set and func_802BE77C(0xFF, veh) (with D_803F77D0 =
 * &D_803EF330) when D_803EF711 is set; if D_803A7424 is then still 0 and the
 * three height pairs at +4/+8, +0x10/+0x14, +0x1C/+0x20 each differ by at most
 * 10000, it returns. Otherwise D_803643D9 = D_802E8BD8 = 1. Then, unless
 * D_803643D8 is set: func_80278318, func_802A02E4(1, &D_803EF330),
 * func_802A5E60, func_802C049C. Finally the effect: D_803EF6FE counts down
 * first; then, until D_803EF6FF is set, the first frame sets up a
 * func_802A6274 record (def D_802C3B44, data 0x7A120, type 1 at (0xFF, 2, 0))
 * and sets D_803EF701, and later frames count D_803EF700 up to 0x12, which
 * sets D_803EF6FF.
 * Leaked registers (kept): b0 is the asm's t0 = &D_803643D6 (so D_803A7428
 * gets 0xD6) and h2 its t2, func_802BABEC's leftover point-list end pointer
 * (its low half goes to D_803A7422), passed in here as `h2`. The
 * func_802A6274 call (type 1, z = 0) ignores the asm's other leftover
 * registers (t6, t7, s0-s4, the a3 tag aside), so they are 0 here. The asm's
 * `sub`/`addi` trap on overflow.
 * Register convention: h2 in t2 (conventions.txt); the asm reads the height
 * records through s7 = veh + 4 (always, from its one caller func_802BA354)
 * and leaves its callees' registers behind (clobbers). */
void func_802BA9A0(s32 h2) {
    s32 *ys = (s32 *) (D_803EF630 + 4);
    Io802A6274 io;
    s32 n;

    if (D_803643D6 == 0) {
        func_8029A800(D_803EF6E4, (s32) D_80305D60, 0, 0, D_803EF6DC, D_803EF6E0, (s32) &D_803643D6,
                      *(s16 *) (D_803EF630 + 0x76), h2, 0, 0xFF, D_803EF630);
        func_8029C52C(0xFF, D_803EF630);
        if (D_803A7426 == 0) {
            D_803A7424 = 0;
            if (D_803EF710 != 0) {
                func_8029AA10(0xFF);
            }
            if (D_803EF711 != 0) {
                D_803F77D0 = D_803EF330;
                func_802BE77C(0xFF, D_803EF630);
            }
            if (D_803A7424 == 0 && ABS_BA9A0(ys[1] - ys[0]) < 0x2711 && ABS_BA9A0(ys[4] - ys[3]) < 0x2711 &&
                ABS_BA9A0(ys[7] - ys[6]) < 0x2711) {
                return;
            }
        }
        D_803643D9 = 1;
        D_802E8BD8 = 1;
    }
    if (D_803643D8 == 0) {
        func_80278318();
        func_802A02E4(1, D_803EF330);
        func_802A5E60();
        func_802C049C();
    }
    if (D_803EF6FE != 0) {
        D_803EF6FE--;
        return;
    }
    if (D_803EF6FF != 0) {
        return;
    }
    if (D_803EF701 == 0) {
        io.a3 = 1;
        io.t6 = 0;
        io.s1 = 0;
        func_802A6274(&io, D_802C3B44, 0x7A120, 1, 0xFF, 2, 0, 0, 0, 0, 0, 0, 0);
        D_803EF701 = 1;
        return;
    }
    n = D_803EF700 + 1;
    D_803EF700 = n;
    if (n == 0x12) {
        D_803EF6FF = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BA9A0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef MTX_CHAIN_REGS_DEFINED
#define MTX_CHAIN_REGS_DEFINED
/* Registers func_802AA890 reads and writes besides its arguments (62740.c). */
typedef struct {
    s32 v1; /* out: y' >> 11 */
    s32 a0; /* out: z' >> 11 */
    s32 a3; /* in/out: the last matrix used */
    s32 s1; /* in/out: y' */
    s32 s2; /* in/out: z' */
    s32 s0; /* in: only read when count == 0 */
} MtxChainRegs;
#endif
extern u8 D_8035805C;  /* selects which of the two matrix buffers is current */
extern s16 D_803ED390; /* model rotation x, y, z (0x803ED390/92/94) */
extern s16 D_803ED392;
extern u8 *D_803EF704; /* matrix buffer pair */
extern u8 *D_803EF708;
extern u8 *D_803EF70C; /* model header */
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);

/* Rebuilds the type-0xFF object's model matrices and points, as
 * func_802B7030 (71140) does for vehicle 5: header D_803EF70C, buffers
 * D_803EF704 (D_8035805C set) / D_803EF708, rotation y = u16 +0x4C of
 * D_803EF630 (the asm's $gp; both callers point it there) in D_803ED392, root
 * matrix at D_803EF6DC/E0/E4 with scale 0x59D8 (func_802AA764), parts tag
 * 0xFF (func_8029C454) and points id 0xFF (func_802ABBEC).
 * Register convention: func_802AA890's chain registers pass through regs
 * (a3, s1, s0 in; v1, a0, a3, s1, s2 out), s2 = the root matrix and v1/a0 =
 * position y/z going into func_802ABBEC as in the asm. Returns the asm's t2,
 * the point list's end (func_802ABBEC preserves it), which caller
 * func_802BA354 hands on to func_802BA9A0 (conventions.txt). The asm also
 * leaves s4 = the buffer, s5/s6 = position y/z, s7 = 0x59D8 and
 * func_802AA764's f12/f14 scratch (clobbers; not read). Asm callers keep t6,
 * t7 live. */
s32 func_802BABEC(MtxChainRegs *regs) {
    u8 *hdr = D_803EF70C;
    u8 *buf = D_8035805C != 0 ? D_803EF704 : D_803EF708;
    s32 *m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) + buf);
    u8 *end;

    D_803ED392 = *(u16 *) (D_803EF630 + 0x4C);
    func_802AA764(D_803EF6DC, D_803EF6E0, D_803EF6E4, 0x59D8, m);
    regs->s2 = (s32) m;
    buf = D_8035805C != 0 ? D_803EF704 : D_803EF708;
    hdr = D_803EF70C;
    func_8029C454(D_803EF6DC, D_803EF6E0, D_803EF6E4, 0xFF, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8),
                  buf, regs);
    regs->v1 = D_803EF6E0;
    regs->a0 = D_803EF6E4;
    hdr = D_803EF70C;
    end = hdr + *(s32 *) (hdr + 4);
    func_802ABBEC(0xFF, (s16 *) (hdr + *(s32 *) hdr), (s16 *) end, buf, regs);
    return (s32) end;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BABEC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle "set timers"
 * leaf, the shape shared by func_802BC578, func_802C9B30, func_802CB5D8,
 * func_802CC8B8, func_802CD9AC, func_802D0784 and func_802D249C (only the
 * scale and the two byte values differ). Asm caller func_802BA354 relies on
 * a0-a3 being preserved (mixed N64 build would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802BAD24(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD24.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80358070;    /* matrix buffer allocator */
extern u8 *D_803EFAD4;    /* vehicle 6's model header */
extern u64 *D_803EFAE4;   /* its two 0x800-byte matrix buffers */
extern u64 *D_803EFAE8;
extern u8 D_803EFA20[];
extern u32 D_803EFAC8[];
extern u8 D_803EF720[];
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
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);
void func_802BB274(void);
void func_802AA838(u8 *src, u8 *dst, s32 off);

/* Level setup of vehicle 6 (asm caller func_802A350C, the level's object
 * list): model header `model` -> D_803EFAD4; two 0x800-byte matrix buffers
 * from the allocator D_80358070 (D_803EFAE4, D_803EFAE8; the allocator
 * advances by 0x1000); func_802A1388(6, 0, buffers, model); $gp record
 * D_803EFA20 reset (func_802A754C); wheel-slot offsets (+-0x50, +-0x5A) at
 * +0x52..+0x68; position D_803EFAC8 = (x, y, z), heading +0x4C/+0x4E/+0x74
 * = `heading`; the three ground slots (func_802A992C, height records +4, y
 * written back to D_803EFACC, key 6); the model channels (func_8029F85C,
 * channel 0 of D_803EF720 set up and both buffers animated); +0xA1 = +0xA2 =
 * 0; parts registered (func_8029C354, tag 6, scale 0x1B58);
 * func_80258230(6, 0x96, 0x2D, 0x2D); one update with +0x9A set
 * (func_802BB274); and the root matrix copied from buffer B to A
 * (func_802AA838). The asm's add/addi trap on overflow.
 * Register convention (conventions.txt): model s2, x t7, y s3, z s0,
 * heading s1, and fp, which the asm hands to func_802A992C (its value
 * reaches D_803ED3F2 when a slot's grid scan finds no triangle: the
 * dispatcher's leftover fp, FIDELITY AUDIT). func_802A992C's FP inputs
 * (the dispatcher's f12-f26, passed through) are 0 here. The asm restores
 * t0-t5 (func_802A350C keeps t1/t2 live), points $gp at D_803EFA20 and
 * leaves s0-s7, fp and f20-f28 as its callees leave them (clobbers). */
void func_802BAD80(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp) {
    TriSideOut f;
    s32 buf;
    u8 *hdr;

    D_803EFAD4 = model;
    buf = D_80358070;
    D_803EFAE4 = (u64 *) buf;
    D_803EFAE8 = (u64 *) (buf + 0x800);
    D_80358070 = buf + 0x1000;
    func_802A1388(6, 0, (s32) D_803EFAE4, (s32) D_803EFAE8, model);
    func_802A754C(D_803EFA20);
    /* wheel slot offsets (x, z pairs and heights) */
    *(s16 *) (D_803EFA20 + 0x52) = 0x50;
    *(s16 *) (D_803EFA20 + 0x54) = 0x50;
    *(s16 *) (D_803EFA20 + 0x56) = -0x50;
    *(s16 *) (D_803EFA20 + 0x58) = 0x50;
    *(s16 *) (D_803EFA20 + 0x5A) = 0x50;
    *(s16 *) (D_803EFA20 + 0x5C) = -0x50;
    *(s16 *) (D_803EFA20 + 0x5E) = 0x5A;
    *(s16 *) (D_803EFA20 + 0x60) = 0x5A;
    *(s16 *) (D_803EFA20 + 0x62) = -0x5A;
    *(s16 *) (D_803EFA20 + 0x64) = 0x5A;
    *(s16 *) (D_803EFA20 + 0x66) = 0x5A;
    *(s16 *) (D_803EFA20 + 0x68) = -0x5A;
    D_803EFAC8[0] = x;
    D_803EFAC8[1] = y;
    D_803EFAC8[2] = z;
    *(s16 *) (D_803EFA20 + 0x4C) = heading;
    *(s16 *) (D_803EFA20 + 0x4E) = heading;
    *(s16 *) (D_803EFA20 + 0x74) = heading;
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A992C((s16 *) (D_803EFA20 + 0x52), D_803EFAC8[1], x, z, (s32 *) (D_803EFA20 + 4),
                  (s32 *) &D_803EFAC8[1], (s16 *) (D_803EFA20 + 0x4C), 6, fp, D_803EFA20, &f);
    hdr = D_803EFAD4;
    func_8029F85C((u32 *) D_803EFAE8, (u32 *) D_803EFAE4, D_803EF720, hdr);
    func_802A039C(D_803EF720, 0, 0x64);
    func_802A03D4(D_803EF720, 0, 0);
    func_802A040C(D_803EF720, 0, 0);
    func_802A0480(0.0f, D_803EF720, 0, 0);
    func_802A0290(D_803EF720, 0, 1);
    func_8029E558((u8 *) D_803EFAE4, (u8 *) D_803EFAE8, D_803EF720);
    func_802A0320(0, D_803EF720);
    func_802A0290(D_803EF720, 0, 1);
    func_8029E558((u8 *) D_803EFAE8, (u8 *) D_803EFAE4, D_803EF720);
    D_803EFA20[0xA1] = 0;
    D_803EFA20[0xA2] = 0;
    hdr = D_803EFAD4;
    func_8029C354(6, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8), 0x1B58);
    func_80258230(6, 0x96, 0x2D, 0x2D);
    D_803EFA20[0x9A] = 1;
    func_802BB274();
    D_803EFA20[0x9A] = 0;
    hdr = D_803EFAD4;
    func_802AA838((u8 *) D_803EFAE8, (u8 *) D_803EFAE4, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BAD80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_8036444C;
extern s16 D_80364450;
extern u8 D_803EF720[];  /* animation channel table (Unk8029DEA0Entry, 56040.c) */
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);

/* Enter vehicle type 6 (called from hd.c): D_8036444C/50 = 6000, 9000, then
 * sets fields 0x14/0x11/0x12 of channels 2, 4, 5 of D_803EF720 and starts
 * channels 4 and 5 with value -1 (func_802A0290). The asm also points $gp at
 * D_803EFA20 and leaves it there (conventions.txt: clobbers gp) and leaves
 * v1 = -1; the C caller uses neither. */
void func_802BB054(void) {
    D_8036444C = 6000;
    D_80364450 = 9000;
    func_802A039C(D_803EF720, 2, 1);
    func_802A03D4(D_803EF720, 2, 0);
    func_802A040C(D_803EF720, 2, 1);
    func_802A039C(D_803EF720, 4, 0);
    func_802A03D4(D_803EF720, 4, 0);
    func_802A040C(D_803EF720, 4, 0);
    func_802A0290(D_803EF720, 4, -1);
    func_802A039C(D_803EF720, 5, 0);
    func_802A03D4(D_803EF720, 5, 0);
    func_802A040C(D_803EF720, 5, 1);
    func_802A0290(D_803EF720, 5, -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB054.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EFA20[]; /* this vehicle's state block */

/* Exit check for vehicle type 6 (called from func_8024B4B8 in hd.c, which
 * declares it void and returns the leftover v0): 1 when the byte at +0xA1 is
 * 0, else 0. Returns s32 (the asm's v0). The asm also points $gp at
 * D_803EFA20 and leaves it there (conventions.txt: clobbers gp), and leaves
 * the byte in v1; C callers use neither. */
s32 func_802BB170(void) {
    return D_803EFA20[0xA1] == 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB170.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EFAE4;  /* save copy pair (func_802A7764) */
extern u64 *D_803EFAE8;
extern void *D_803EFAD8; /* sound handles, NULL = none */
extern void *D_803EFADC;
extern void *D_803EFAE0;
void func_802A7764(u64 *a, u64 *b, s32 size);

/* Leave vehicle type 6 (called from hd.c): func_802A7764(D_803EFAE4,
 * D_803EFAE8, 0x800), then stops the sounds D_803EFADC, D_803EFAD8 and
 * D_803EFAE0 that are playing (func_802608C8); the handles are left as they
 * are. The asm saves and restores $gp. */
void func_802BB1A0(void) {
    func_802A7764(D_803EFAE4, D_803EFAE8, 0x800);
    if (D_803EFADC != NULL) {
        func_802608C8(D_803EFADC);
    }
    if (D_803EFAD8 != NULL) {
        func_802608C8(D_803EFAD8);
    }
    if (D_803EFAE0 != NULL) {
        func_802608C8(D_803EFAE0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB1A0.s")
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
extern u32 D_803EFAC8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 6 at its position
 * D_803EFAC8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802BB274 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802BB230(ZoneScanRegs *r) {
    return func_802ABD54(6, D_803EFAC8[0], D_803EFAC8[1], D_803EFAC8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB230.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80305DF0[];
extern s16 D_8036443C; /* player speed, heading, angle */
extern s16 D_8036443E;
extern s16 D_80364440;
void func_802BB4C0(void);
s32 func_802BB8B8(MtxChainRegs *regs, s32 *t2Out);
s32 func_802A92C8(s32 x, s32 z, s16 *tbl, s16 *angle, s32 *ys, s32 key, u8 *veh, s32 fpIn, TriSideOut *f);
s32 func_8029C5EC(s32 s0, s32 s1, s32 s2, s32 s3);

/* Per-frame update of vehicle 6 (D_803EFA20; called from hd.c's vehicle
 * switch and from func_802BAD80): zone lookup (func_802BB230); the
 * channels/sounds func_802BB4C0 while byte +0x9A is clear; ground heights
 * of the three wheel slots (func_802A92C8 at D_803EFAC8[0]/[2], key 6);
 * animate the model (func_8029E558 on the current buffer pair, channels
 * D_803EF720); rebuild it (func_802BB8B8); clear +0xA2; vehicle-state reset
 * func_8029A800 (table D_80305DF0, kind 6), func_8029C52C(6), the part
 * lookup func_8029C5EC, the collision pass func_802BE77C(6) (with
 * D_803F77D0 = D_803EF720) and func_8029AA10(6); +0xA2 = 1 if D_803A7424 got
 * set; finally the player globals D_803643E0/E4/E8 = the position,
 * D_8036443C = 0, D_8036443E / D_80364440 = u16 +0x4E / +0x4C.
 * func_8029A800's b0 and h2 are what func_802BB8B8 leaves in t0 / t2 (x and
 * 6), kept here.
 * Leaked registers (FIDELITY AUDIT): func_802A92C8's fp and f12-f26 inputs
 * are what func_802BB4C0 / func_802BB868 (or, when +0x9A is set, the
 * caller) left behind. fp only reaches D_803ED3F2 when func_802A9DC0 finds
 * no triangle and the h1 path still wins (impossible for game heights: it
 * needs |h3 - y| > ~0x5F5E0FF) and f12-f26 only pass through, so this
 * passes 0. Likewise 0 for the zone scan's t6/t7/s0-s4 inputs (dead
 * afterwards), func_802BB8B8's chain inputs a3/s1/s0 (only read for a
 * zero-matrix point) and func_8029C5EC's s0-s3 (only used when its lookup
 * succeeds, which reloads them). The asm saves every s- and FP register it
 * changes; it returns with v1/f12/f14 as its callees leave them (read by
 * func_802BAD80; not modelled). */
void func_802BB274(void) {
    ZoneScanRegs zr;
    TriSideOut f;
    MtxChainRegs regs;
    s32 x;
    s32 h2;

    zr.t6 = 0;
    zr.t7 = 0;
    zr.s0 = 0;
    zr.s1 = 0;
    zr.s2 = 0;
    zr.s3 = 0;
    zr.s4 = 0;
    func_802BB230(&zr);
    if ((s8) D_803EFA20[0x9A] == 0) {
        func_802BB4C0();
    }
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A92C8(D_803EFAC8[0], D_803EFAC8[2], (s16 *) (D_803EFA20 + 0x5E), (s16 *) (D_803EFA20 + 0x4C),
                  (s32 *) (D_803EFA20 + 4), 6, D_803EFA20, 0, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803EFAE4, (u8 *) D_803EFAE8, D_803EF720);
    } else {
        func_8029E558((u8 *) D_803EFAE8, (u8 *) D_803EFAE4, D_803EF720);
    }
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s0 = 0;
    x = func_802BB8B8(&regs, &h2);
    D_803EFA20[0xA2] = 0;
    func_8029A800(D_803EFAC8[2], (s32) D_80305DF0, 0, 0, D_803EFAC8[0], D_803EFAC8[1], x, 0, h2, 0, 6,
                  D_803EFA20);
    func_8029C52C(6, D_803EFA20);
    func_8029C5EC(0, 0, 0, 0);
    D_803F77D0 = D_803EF720;
    func_802BE77C(6, D_803EFA20);
    func_8029AA10(6);
    if (D_803A7424 != 0) {
        D_803EFA20[0xA2] = 1;
    }
    D_803643E0 = D_803EFAC8[0];
    D_803643E4 = D_803EFAC8[1];
    D_803643E8 = D_803EFAC8[2];
    D_8036443C = 0;
    D_8036443E = *(u16 *) (D_803EFA20 + 0x4E);
    D_80364440 = *(u16 *) (D_803EFA20 + 0x4C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB274.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80370C15; /* input flags */
extern u8 D_80370C16;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C1D;
extern f32 D_8030D930; /* channel 5 limits */
extern f32 D_8030D934;
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802A04BC(s32 idx, void *base, s32 *out);
s32 func_802BB868(void);

/* Animation channels and sounds of vehicle type 6 ($gp = D_803EFA20, read as
 * the global; channels in D_803EF720, an Unk8029DEA0Entry table).
 * - Byte +0xA2 set: channels 5, 4 off (unk14 = 0), channel 2 unk11 = 0 and
 *   restarted (func_802A0290), +0xA1 = 1.
 * - Otherwise channel 5 follows input D_80370C1C while its unk4 <= D_8030D930
 *   (direction 0) or D_80370C1D while !(unk4 < D_8030D934) (direction 1),
 *   starting sound 0x6F (handle D_803EFAD8) if needed, else it is switched
 *   off and that sound stopped; channel 4 likewise follows D_80370C15 /
 *   D_80370C16 with sound 0x6E (D_803EFADC). Then, when func_802BB868 reports
 *   ground under the level-0x11 part, channel 2 is restarted with
 *   direction 0 and +0xA1 = 1 unless it is already running in direction 1
 *   at unk4 * 100 == 100 (cvt.w.s, nearest); with no ground, if +0xA1 is 0
 *   and input D_80370C1A or D_80370C1B is set, sound 0x6D (D_803EFAE0) is
 *   started if needed, channel 2 restarted and +0xA1 = 1, and nothing else
 *   happens.
 * - Finally (all other paths): when channel 2 isn't active (unk10 != 1),
 *   +0xA1 = 0 and sound D_803EFAE0 is stopped if playing (handle kept).
 * Register note: the asm leaves func_802BB868's registers behind (s0-s7, fp,
 * f20-f28, t7 per the survey; conventions.txt: clobbers); the asm caller
 * func_802BB274 passes some of them on to func_802A92C8 unchanged, which the
 * C can't reproduce (they are func_802BB868's scratch). */
void func_802BB4C0(void) {
    s32 e[8];
    s32 r;

    if (D_803EFA20[0xA2] != 0) {
        func_802A039C(D_803EF720, 5, 0);
        func_802A039C(D_803EF720, 4, 0);
        func_802A03D4(D_803EF720, 2, 0);
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
        goto check2;
    }

    if (D_80370C1C != 0 && (func_802A04BC(5, D_803EF720, e), ((f32 *) e)[7] <= D_8030D930)) {
        func_802A03D4(D_803EF720, 5, 0);
        func_802A039C(D_803EF720, 5, 1);
        goto start5;
    }
    if (D_80370C1D != 0 && (func_802A04BC(5, D_803EF720, e), !(((f32 *) e)[7] < D_8030D934))) {
        func_802A03D4(D_803EF720, 5, 1);
        func_802A039C(D_803EF720, 5, 1);
        goto start5;
    }
    func_802A039C(D_803EF720, 5, 0);
    if (D_803EFAD8 != NULL) {
        func_802608C8(D_803EFAD8);
    }
    goto chan4;
start5:
    if (D_803EFAD8 == NULL) {
        func_80260650(D_80367738, 0x6F, &D_803EFAD8);
    }

chan4:
    if (D_80370C15 != 0) {
        func_802A03D4(D_803EF720, 4, 0);
        func_802A039C(D_803EF720, 4, 1);
    } else if (D_80370C16 != 0) {
        func_802A03D4(D_803EF720, 4, 1);
        func_802A039C(D_803EF720, 4, 1);
    } else {
        func_802A039C(D_803EF720, 4, 0);
        if (D_803EFADC != NULL) {
            func_802608C8(D_803EFADC);
        }
        goto ground;
    }
    if (D_803EFADC == NULL) {
        func_80260650(D_80367738, 0x6E, &D_803EFADC);
    }

ground:
    if (func_802BB868() != 0) {
        func_802A04BC(2, D_803EF720, e);
        if (e[6] == 1) {
            CVT_W_S(r, ((f32 *) e)[7] * 100.0f);
            if (r == 100) {
                goto check2;
            }
        }
        func_802A03D4(D_803EF720, 2, 0);
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
    } else {
        if (D_803EFA20[0xA1] != 0) {
            goto check2;
        }
        if (D_80370C1A == 0 && D_80370C1B == 0) {
            goto check2;
        }
        if (D_803EFAE0 == NULL) {
            func_80260650(D_80367738, 0x6D, &D_803EFAE0);
        }
        func_802A0290(D_803EF720, 2, 1);
        D_803EFA20[0xA1] = 1;
        return;
    }

check2:
    func_802A04BC(2, D_803EF720, e);
    if (e[0] != 1) {
        D_803EFA20[0xA1] = 0;
        if (D_803EFAE0 != NULL) {
            func_802608C8(D_803EFAE0);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB4C0.s")
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
/* func_8029C6E4's register results (56040.c). */
typedef struct {
    s32 s0;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4; /* 1 = found */
    u8 *t6;
    s32 t7;
} Unk8029C6E4Out;
void func_8029C6E4(Unk8029C6E4Out *o);
extern s32 D_802E8BDC; /* current level */

/* On level 0x11 only: finds the kind-6 / id-0x3BD part (func_8029C6E4) and
 * returns whether func_802AC0BC finds ground under its position (words 0, 8
 * as x, z; word 4 as y); 0 otherwise.
 * Register convention: result in v0 (ABI). The asm leaves both callees'
 * registers behind (s0-s4, t6, t7, fp, f12-f26) and its caller
 * func_802BB4C0 only tests v0, so they're not modelled (conventions.txt:
 * clobbers). Its s0-s3 inputs only pass through func_8029C6E4 when nothing is
 * found, and func_802AC0BC's pass-through inputs a3 / f12-f26 don't affect
 * the found flag, so the C starts them at 0. */
s32 func_802BB868(void) {
    Unk8029C6E4Out o;
    TriSideOut f;
    TriScanRegs r;

    if (D_802E8BDC != 0x11) {
        return 0;
    }
    o.s0 = 0;
    o.s1 = 0;
    o.s2 = 0;
    o.s3 = 0;
    func_8029C6E4(&o);
    if (o.s4 == 0) {
        return 0;
    }
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    r.a1 = 0;
    r.a3 = 0;
    r.t6 = (s32) o.t6;
    r.t7 = o.t7;
    r.fp = 0;
    r.s1 = o.s1;
    r.s2 = o.s2;
    r.s3 = o.s3;
    r.s4 = o.s4;
    func_802AC0BC(o.s0, o.s2, o.s1, &f, &r);
    return r.a1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB868.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803EFAD4; /* model header */
s32 func_802AABE4(s32 id, u16 *desc, u8 *base, MtxChainRegs *regs, s16 **vertsOut);
void func_8029D040(s32 val, void *tbl, s32 x, s32 z, s32 id, u8 *model, u8 *mtxBase);

/* Rebuilds vehicle 6's model (D_803EFA20, the asm's $gp; header
 * D_803EFAD4, buffers D_803EFAE4 (D_8035805C set) / D_803EFAE8): rotation
 * D_803ED390 = 0, y = heading (u16 +0x4C), z = 0; root matrix at
 * D_803EFAC8[0..2] with scale 0x1B58 (func_802AA764); parts tag 6
 * (func_8029C454, header +4 .. +8), points id 6 (func_802ABBEC, +0 .. +4),
 * triangles id 6 (func_802AABE4, header +8) and the collision setup
 * func_8029D040(heading, D_803EF720, x, z, 6, header + [+0xC], buffer).
 * Register convention: func_802AA890's chain registers pass through regs
 * (a3, s1, s0 in) as in func_802B7030 (71140). The asm returns with t0 = x
 * (D_803EFAC8[0]) and t2 = 6, which asm caller func_802BB274 passes on to
 * func_8029A800: the return value and *t2Out here (conventions.txt).
 * func_8029D040 leaves s0-s7, fp, f20-f28 (and the chain outputs) changed
 * (clobbers; func_802BB274 only hands s0-s3 to func_8029C5EC, which reads
 * them only when its lookup fails and then doesn't use them). */
s32 func_802BB8B8(MtxChainRegs *regs, s32 *t2Out) {
    u8 *hdr = D_803EFAD4;
    u8 *buf = D_8035805C != 0 ? (u8 *) D_803EFAE4 : (u8 *) D_803EFAE8;
    s32 *m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) + buf);
    s16 *verts;

    D_803ED390 = 0;
    (&D_803ED390)[2] = 0;
    D_803ED392 = *(u16 *) (D_803EFA20 + 0x4C);
    func_802AA764(D_803EFAC8[0], D_803EFAC8[1], D_803EFAC8[2], 0x1B58, m);
    regs->s2 = (s32) m;
    buf = D_8035805C != 0 ? (u8 *) D_803EFAE4 : (u8 *) D_803EFAE8;
    hdr = D_803EFAD4;
    func_8029C454(D_803EFAC8[0], D_803EFAC8[1], D_803EFAC8[2], 6, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8),
                  buf, regs);
    regs->v1 = D_803EFAC8[1];
    regs->a0 = D_803EFAC8[2];
    hdr = D_803EFAD4;
    func_802ABBEC(6, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), buf, regs);
    hdr = D_803EFAD4;
    func_802AABE4(6, (u16 *) (hdr + *(s32 *) (hdr + 8)), buf, regs, &verts);
    hdr = D_803EFAD4;
    func_8029D040(*(u16 *) (D_803EFA20 + 0x4C), D_803EF720, D_803EFAC8[0], D_803EFAC8[2], 6,
                  hdr + *(s32 *) (hdr + 0xC), buf);
    *t2Out = 6;
    return D_803EFAC8[0];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/75490/func_802BB8B8.s")
#endif
