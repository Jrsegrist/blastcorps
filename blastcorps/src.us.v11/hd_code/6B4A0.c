#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Port-phase shared declarations for the vehicle 1 driver below
 * (func_802AFC60, func_802B02A0, func_802B03F4, func_802B0B3C). The asm
 * reaches the record through $gp (D_803EDB40); the C reads it directly.
 * The helpers are copies of 6E200.c's (vehicles 3 and 4), trimmed to what
 * vehicle 1 uses (dedupe into a shared port header later). */
#define VEH_U8(v, off) (*(u8 *) ((u8 *) (v) + (off)))
#define VEH_S8(v, off) (*(s8 *) ((u8 *) (v) + (off)))
#define VEH_U16(v, off) (*(u16 *) ((u8 *) (v) + (off)))
#define VEH_S16(v, off) (*(s16 *) ((u8 *) (v) + (off)))


extern u8 D_803EDB40[];  /* vehicle 1 record */
extern u32 D_803EDBE8[]; /* vehicle 1 PortVehPos */
extern u8 D_803ED840[];  /* vehicle 1 animation channels */
#define VPOS1 ((PortVehPos *) D_803EDBE8)

extern u8 *D_80358070; /* heap cursor */
extern u8 *D_803F77D0;
extern u8 D_80305CE0[];
extern f32 D_8030D8A0;


/* Place the vehicle model's parts (as 6E200.c's port_veh_parts): the current
 * matrix buffer (D_8035805C ? bufA : bufB) plus the word at model +
 * model[0x18] + 4 is the model matrix m; D_803ED392 = heading (u16 +0x4C),
 * func_802AA764(x, y, z, scale, m); then func_8029C454(x, y, z, id, model +
 * model[4], model + model[8], buffer) and func_802ABBEC(id, model +
 * model[0], model + model[4], buffer). Returns the buffer (the asm's s4).
 * `r` carries func_802AA890's pass-through registers: func_802AA764 leaves
 * a3 = 0 (from func_802ACCCC) and s2 = m in the asm, v1/a0 are still y/z at
 * func_802ABBEC; s0/s1 come from the caller. They only matter for a
 * zero-count (untransformed) part, whose record then gets s0/s1/s2 >> 11. */
static u8 *port_veh_parts(u8 *veh, PortVehPos *p, s32 id, s32 scale, MtxChainRegs *r) {
    u8 *m = p->model;
    s32 off = *(s32 *) (m + *(s32 *) (m + 0x18) + 4);
    s32 *mtx;
    u8 *base;

    mtx = (s32 *) ((D_8035805C != 0 ? (u8 *) p->bufA : (u8 *) p->bufB) + off);
    D_803ED392 = VEH_U16(veh, 0x4C);
    func_802AA764(p->x, p->y, p->z, scale, mtx);
    r->a3 = 0;
    r->s2 = (s32) mtx;
    base = (D_8035805C != 0) ? (u8 *) p->bufA : (u8 *) p->bufB;
    m = p->model;
    func_8029C454(p->x, p->y, p->z, id, m + *(s32 *) (m + 4), m + *(s32 *) (m + 8), base, r);
    r->v1 = p->y;
    r->a0 = p->z;
    m = p->model;
    func_802ABBEC(id, (s16 *) (m + *(s32 *) m), (s16 *) (m + *(s32 *) (m + 4)), base, r);
    return base;
}

/* Put the vehicle on the ground (as 6E200.c's port_veh_ground):
 * func_802A9A60 (key vid) at the position with the caller's fp, parts (s0 =
 * z, s1 = func_802A9A60's s1 in the asm), func_802A133C. The asm also hands
 * func_802A9A60 the caller's f12-f26 (register leak; FP side results only,
 * zero here). */
static void port_veh_ground(u8 *veh, PortVehPos *p, s32 vid, s32 fp, u8 *(*parts)(MtxChainRegs *)) {
    TriSideOut f;
    Out802A9A60 o;
    MtxChainRegs mc;
    s32 z = p->z;

    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A9A60((s16 *) (veh + 0x52), p->y, p->x, z, (s32 *) (veh + 4), &p->y, (s16 *) (veh + 0x4C), vid, fp,
                  veh, &f, &o);
    mc.s0 = z;
    mc.s1 = (s32) o.s1;
    parts(&mc);
    func_802A133C(p->z, vid, p->x, p->y, veh);
}

/* Vehicle setup, first part (as 6E200.c): model header, two 0x800-byte
 * matrix buffers from the heap, func_802A1388(id, 0, bufA, bufB, model),
 * func_802A754C, wheel offset table (+0x52, 12 halves), position, heading
 * (+0x4C, +0x4E, +0x74). */
static void port_veh_init_head(u8 *veh, PortVehPos *p, s32 id, u8 *model, const s16 *wheels, s32 x, s32 y,
                               s32 z, s32 heading) {
    u8 *heap;
    s32 i;

    p->model = model;
    heap = D_80358070;
    p->bufA = (u64 *) heap;
    heap += 0x800;
    p->bufB = (u64 *) heap;
    heap += 0x800;
    D_80358070 = heap;
    func_802A1388(id, 0, (s32) p->bufA, (s32) p->bufB, model);
    func_802A754C(veh);
    for (i = 0; i < 12; i++) {
        VEH_S16(veh, 0x52 + i * 2) = wheels[i];
    }
    p->x = x;
    p->y = y;
    p->z = z;
    VEH_S16(veh, 0x4C) = heading;
    VEH_S16(veh, 0x4E) = heading;
    VEH_S16(veh, 0x74) = heading;
}

/* Second part: ground slots (func_802A992C, key id, with the incoming fp;
 * the caller's f12-f26 also go in there in the asm: register leak, FP side
 * results only, zero here), animation channel 0 reset between the two
 * buffers, band table (+0x78, 15 halves). */
static void port_veh_init_anim(u8 *veh, PortVehPos *p, s32 id, u8 *ch, const s16 *bands, s32 x, s32 z, s32 fp) {
    TriSideOut f;
    u64 *a;
    u64 *b;
    s32 i;

    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A992C((s16 *) (veh + 0x52), p->y, x, z, (s32 *) (veh + 4), &p->y, (s16 *) (veh + 0x4C), id, fp,
                  veh, &f);
    a = p->bufA;
    b = p->bufB;
    func_8029F85C((u32 *) b, (u32 *) a, ch, p->model);
    func_802A039C(ch, 0, 100);
    func_802A03D4(ch, 0, 0);
    func_802A040C(ch, 0, 0);
    func_802A0480(0.0f, ch, 0, 0);
    func_802A0290(ch, 0, 1);
    func_8029E558((u8 *) a, (u8 *) b, ch);
    func_802A0320(0, ch);
    func_802A0290(ch, 0, 1);
    func_8029E558((u8 *) b, (u8 *) a, ch);
    for (i = 0; i < 15; i++) {
        VEH_S16(veh, 0x78 + i * 2) = bands[i];
    }
}

/* Last part: func_8029C354(id, model + model[4], model + model[8], scale),
 * func_80258230(id, a1, a2, a2), one update with +0x9A set, and the model
 * matrix copied bufB -> bufA (func_802AA838). */
static void port_veh_init_tail(u8 *veh, PortVehPos *p, s32 id, s32 scale, s32 a1, s32 a2,
                               void (*update)(s32, s32, s32, s32, s32, s32, s32)) {
    u8 *m = p->model;

    func_8029C354(id, m + *(s32 *) (m + 4), m + *(s32 *) (m + 8), scale);
    func_80258230(id, a1, a2, a2);
    VEH_U8(veh, 0x9A) = 1;
    /* The asm enters the update with t6, t7, s0-s4 as func_80258230 (C) and
     * func_8029E558 left them (register leak); they only pass through
     * func_802ABD54 unused. */
    update(0, 0, 0, 0, 0, 0, 0);
    VEH_U8(veh, 0x9A) = 0;
    m = p->model;
    func_802AA838((u8 *) p->bufB, (u8 *) p->bufA, *(s32 *) (m + *(s32 *) (m + 0x18) + 4));
}

/* Middle of the per-frame update (as 6E200.c's port_veh_move): speed/heading
 * (func_802A7FD8 with `rate`), drive force (func_802A83B8, func_802A843C
 * kind/div), func_802A7070 when `drift`, move along the heading
 * (func_802A860C), D_803ED40B = 1, ground contact (func_802A8768), animation
 * (func_8029E558), parts, func_8029A800, func_8029C52C, func_8029AA10. */
static void port_veh_move(u8 *veh, PortVehPos *p, s32 id, u8 *ch, s32 rate, s32 kind, f32 div, s32 drift,
                          s16 *driftAngle, s32 divB, s32 divA, u8 *(*parts)(MtxChainRegs *), u8 *a1, s32 b2,
                          s32 b0, s32 h2) {
    Out802A860C o;
    Regs802A8768 r8;
    TriSideOut f;
    MtxChainRegs mc;
    f32 fl;
    s32 x;

    func_802A7FD8(veh, (u16 *) (veh + 0x74), rate, (s16 *) (veh + 0x76), (u16 *) (veh + 0x4C),
                  (u16 *) (veh + 0x4E), (s8 *) (veh + 0x99), 1);
    fl = func_802A83B8((s16 *) (veh + 0x76), veh + 0x96, (s32 *) (veh + 4), (f32 *) veh);
    func_802A843C(veh, (s16 *) (veh + 0x76), kind, (s8 *) (veh + 0x96), (s32 *) (veh + 4), 1, div);
    if (drift != 0) {
        func_802A7070(veh, driftAngle);
    }
    x = func_802A860C(fl, VEH_U16(veh, 0x4E), (s16 *) (veh + 0x76), &p->x, &p->z, &o);
    D_803ED40B = 1;
    r8.s3 = o.s3;
    /* f12-f26 here: func_802A860C's temporaries and the caller's registers
     * (register leak, FP side results only). */
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A8768(veh, id, &p->x, &p->y, &p->z, x, o.t1, divB, divA, (s16 *) (veh + 0x4C), veh + 0x96,
                  (s16 *) (veh + 0x52), (s32 *) (veh + 0x28), (s32 *) (veh + 0x40), (s32 *) (veh + 0x34),
                  (s32 *) (veh + 4), &r8, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) p->bufA, (u8 *) p->bufB, ch);
    } else {
        func_8029E558((u8 *) p->bufB, (u8 *) p->bufA, ch);
    }
    /* s0/s1: func_8029E558's leftovers in the asm (register leak). */
    mc.s0 = 0;
    mc.s1 = 0;
    parts(&mc);
    func_8029A800(p->z, (s32) a1, b2, 1, p->x, p->y, b0, VEH_S16(veh, 0x76), h2, 0, id, veh);
    func_8029C52C(id, veh);
    func_8029AA10(id);
}

/* The turn after a collision report (as 6E200.c's port_veh_turn). */
static void port_veh_turn(u8 *veh, f32 scale, s16 *target) {
    s32 d;
    s32 cur;
    s32 turn;
    s32 tmp;

    d = VEH_U16(veh, 0x4E) - 0x800;
    if (d < 0) {
        d += 0xFFF;
    }
    d -= func_802A6F6C();
    if (d < 0) {
        d = -d;
    }
    if (d > 0x800) {
        d = 0xFFF - d;
    }
    func_802A70D8(veh);
    turn = func_802A71DC(veh, VEH_U16(veh, 0x4E), VEH_U16(veh, 0x4C), &cur, scale);
    *target = cur;
    VEH_S16(veh, 0x4E) = cur;
    VEH_S16(veh, 0x74) = cur;
    func_802A746C(veh, turn, d, &tmp);
    func_802A6FE4(veh, 0);
}

/* Publish position, speed, headings, then the func_802A133C record. */
static void port_veh_publish(u8 *veh, PortVehPos *p, s32 id) {
    D_803643E0 = p->x;
    D_803643E4 = p->y;
    D_803643E8 = p->z;
    D_8036443C = VEH_S16(veh, 0x76);
    D_8036443E = VEH_U16(veh, 0x4E);
    D_80364440 = VEH_U16(veh, 0x4C);
    func_802A133C(p->z, id, p->x, p->y, veh);
}

u8 *func_802B0B3C(MtxChainRegs *r);

static const s16 sVeh1Wheels[12] = { 0x168, 0x168, -0x168, 0x168, 0x168, -0x168,
                                     0x17C, 0x17C, -0x17C, 0x17C, 0x17C, -0x17C };
static const s16 sVeh1Bands[15] = { -0xB4, 0, 5, 0, 0x3C, 5, 0x3C, 0x64, 4, 0x64, 0xB4, 3, 0xB4, 0xDC, 2 };
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDC05;
extern u8 D_803EDC06;
extern s8 D_803EDC04;

/* Vehicle 1 setup, called from the asm dispatcher func_802A350C: model
 * header, buffers and record (port_veh_init_head with this vehicle's wheel
 * table), level D_803EDC05 = 50, horn uses D_803EDC00 = 0, cooldown
 * D_803EDC06 = 0, D_803EDC04 = 0, ground slots and animation reset
 * (port_veh_init_anim, band table), parts (scale 0x1B58),
 * func_80258230(1, 0x96, 0x3C, 0x3C), one update (func_802B03F4 with +0x9A
 * set), buffer copy, D_80364A6D = 1. (No func_802A6F00, unlike vehicles 3
 * and 4.)
 * Register convention: model s2, x t7, y s3, z s0, heading s1, fp (the ground
 * slot flag seed for func_802A992C) (conventions.txt). The asm saves t0-t5
 * (its caller keeps t1, t2 live). Register leaks not modelled: the caller's
 * f12-f26 go into func_802A992C (FP side results only), and the dispatcher
 * reads s2-s4, fp and f12-f26 afterwards, which hold whatever func_802B03F4 /
 * func_8029E558 / func_802AA764 left (a mixed N64 build would need a
 * thunk). */
void func_802AFC60(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp) {
    u8 *veh = D_803EDB40;
    PortVehPos *p = VPOS1;

    port_veh_init_head(veh, p, 1, model, sVeh1Wheels, x, y, z, heading);
    D_803EDC05 = 50;
    D_803EDC00 = 0;
    D_803EDC06 = 0;
    D_803EDC04 = 0;
    port_veh_init_anim(veh, p, 1, D_803ED840, sVeh1Bands, x, z, fp);
    port_veh_init_tail(veh, p, 1, 0x1B58, 0x96, 0x3C, func_802B03F4);
    D_80364A6D = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFC60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDB40[]; /* this vehicle's state block */
extern u8 D_803ED840[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
extern u8 D_802C2390[]; /* key of this vehicle's func_802A06B4 entry */

/* Enter vehicle type 1 (called from 00000.c / 17210.c): clears byte 0x99 of
 * the state block, sets fields 0x14 / 0x11 / 0x12 of animation channels 1, 2,
 * 5, 3, 2 of D_803ED840 and restarts all but 5 with -1 (func_802A0290), sets
 * the D_802C2390 entry's fields (100, 0, 1) and restarts it, D_8036444C/50 =
 * 3400, 300, then func_802C4310(a0, 0x36) with a0 = D_803ED840 left over from
 * the setters (func_802C4310 ignores it). The asm points $gp at D_803EDB40 and
 * leaves it there (conventions.txt: clobbers gp) and returns with v1 = -1
 * (the last setter's v1); C callers ignore both. */
void func_802AFFD4(void) {
    D_803EDB40[0x99] = 0;
    func_802A039C(D_803ED840, 1, 0);
    func_802A03D4(D_803ED840, 1, 0);
    func_802A040C(D_803ED840, 1, 0);
    func_802A0290(D_803ED840, 1, -1);
    func_802A039C(D_803ED840, 2, 0);
    func_802A03D4(D_803ED840, 2, 0);
    func_802A040C(D_803ED840, 2, 0);
    func_802A0290(D_803ED840, 2, -1);
    func_802A039C(D_803ED840, 5, 7);
    func_802A03D4(D_803ED840, 5, 0);
    func_802A040C(D_803ED840, 5, 1);
    func_802A039C(D_803ED840, 3, 0);
    func_802A03D4(D_803ED840, 3, 0);
    func_802A040C(D_803ED840, 3, 1);
    func_802A0290(D_803ED840, 3, -1);
    func_802A039C(D_803ED840, 2, 0);
    func_802A03D4(D_803ED840, 2, 0);
    func_802A040C(D_803ED840, 2, 1);
    func_802A0290(D_803ED840, 2, -1);
    func_802A05D0((s32) D_802C2390, 100);
    func_802A05F8((s32) D_802C2390, 0);
    func_802A0620((s32) D_802C2390, 1);
    func_802A0508((s32) D_802C2390, -1);
    D_8036444C = 0xD48;
    D_80364450 = 0x12C;
    func_802C4310((s32) D_803ED840, 0x36);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802AFFD4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDB40[]; /* this vehicle's state block */
extern u8 D_803ED840[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
/* Reads channel idx of base into out[0..7] (56040.c; asm convention in
 * tools_port/conventions.txt): out[0] = (s8) field 0x10, out[3] = field
 * 0x14, out[4] = (u16) field 0xC, ... */

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
#ifdef NON_MATCHING
extern u64 *D_803EDBF8; /* save copy pair (func_802A7764) */
extern u64 *D_803EDBFC;

/* Leave vehicle type 1 (called from hd.c): zeroes the speed (s16 at +0x76 of
 * the state block), func_802A7764(D_803EDBF8, D_803EDBFC, 0x800), then
 * func_802C444C(). The asm saves and restores $gp. */
void func_802B0254(void) {
    *(s16 *) (D_803EDB40 + 0x76) = 0;
    func_802A7764(D_803EDBF8, D_803EDBFC, 0x800);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0254.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Put vehicle 1 on the ground (called from C func_8024B618 in 00000.c as
 * `void (void)`): port_veh_ground with key 1, then func_802B0B3C.
 * Register convention: fp in (conventions.txt). The C caller doesn't set fp:
 * the asm passes on whatever an outer function left in $fp/$s8 (register
 * leak; it seeds func_802A9A60's D_803ED3F2 flags), and its f12-f26 into the
 * scans. The asm saves every callee-saved register. */
void func_802B02A0(s32 fp) {
    port_veh_ground(D_803EDB40, VPOS1, 1, fp, func_802B0B3C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B02A0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
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
#ifdef NON_MATCHING
s32 func_802B03B0(ZoneScanRegs *r);
void func_802B07DC(void);
void func_802B0CE8(void);
s32 func_802B0C74(void);
s32 func_802B0AAC(void);
extern s8 D_803EDC04;
extern s16 D_803EDC02;

/* Vehicle 1 per-frame update (called from C func_8024B7AC in 00000.c as
 * `void (void)`, and from func_802AFC60):
 * 1. zone level (func_802B03B0) with the incoming scan registers; the
 *    animation/sound update func_802B07DC unless +0x9A is set;
 * 2. setup leaf func_802B0CE8, steering + speed bands
 *    func_802A7834(func_802B0C74(), ...) (no countdown, no state save, no
 *    func_802CB690, unlike vehicles 3 and 4);
 * 3. port_veh_move (rate 0x4650, kind 1 / 720.0, drift D_803EDC04 via
 *    D_803EDC02, divisors 0x2D0 / 0x2D0, parts func_802B0B3C, func_8029A800
 *    with D_80305CE0, 0, 7, 100);
 * 4. collision pass func_802BE77C (D_803F77D0 = the channel table), the
 *    horn channel check func_802B0AAC; with a collision report
 *    (D_803A7425): func_8029A914, D_803EDC04 = 1 and port_veh_turn
 *    (D_8030D8A0, D_803EDC02); else D_803EDC04 = 0. (A bounce of the speed
 *    follows an unconditional branch in the asm: dead code.)
 * 5. port_veh_publish.
 * Register convention: t6, t7, s0-s4 in (only through func_802ABD54; from
 * the C caller they are whatever it left: register leak). The asm saves all
 * callee-saved registers and leaves v1 = y; asm caller func_802AFC60 reads
 * f12/f14 afterwards (func_802AA764's leftovers, not modelled). */
void func_802B03F4(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    u8 *veh = D_803EDB40;
    PortVehPos *p = VPOS1;
    ZoneScanRegs zs;

    zs.t6 = t6;
    zs.t7 = t7;
    zs.s0 = s0;
    zs.s1 = s1;
    zs.s2 = s2;
    zs.s3 = s3;
    zs.s4 = s4;
    func_802B03B0(&zs);
    if (VEH_S8(veh, 0x9A) == 0) {
        func_802B07DC();
    }
    func_802B0CE8();
    func_802A7834(func_802B0C74(), (u16 *) (veh + 0x4C), veh, (s16 *) (veh + 0x76), 3, veh + 0x96,
                  (s16 *) (veh + 0x78), 0x14);
    port_veh_move(veh, p, 1, D_803ED840, 0x4650, 1, 720.0f, D_803EDC04, &D_803EDC02, 0x2D0, 0x2D0,
                  func_802B0B3C, D_80305CE0, 0, 7, 100);
    D_803F77D0 = D_803ED840;
    func_802BE77C(1, veh);
    func_802B0AAC();
    if (D_803A7425 == 0) {
        D_803EDC04 = 0;
    } else {
        func_8029A914(veh);
        D_803EDC04 = 1;
        port_veh_turn(veh, D_8030D8A0, &D_803EDC02);
    }
    port_veh_publish(veh, p, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B03F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_8030D8A4;
extern u8 D_803EDC06;    /* horn cooldown (frames) */
extern u8 D_803EDC05;    /* level 0..100 */
extern void *D_80367738;

/* Per-frame animation/sound update of vehicle 1 (asm caller func_802B03F4;
 * the asm reads the state block through $gp = D_803EDB40, read directly
 * here). Channel 2 of D_803ED840 gets the position within one of three
 * 0x555-wide thirds of the angle D_80364452 + 0x800 (wrapped by 0xFFF) and
 * the third (0, 1, else 2). When the cooldown D_803EDC06 is 0, channel 5 isn't
 * running (field 0x10 != 1), D_80370C1A or D_80370C1B is set and D_803EDC00 is
 * nonzero: one use is taken, channel 5 restarts with 2, the cooldown is set
 * to 10 and sound 0x37 starts; otherwise a nonzero cooldown counts down. The
 * level D_803EDC05 moves by 5 (down to 0 with D_80370C15, up to 100 with
 * D_80370C16, else towards 50) and goes to channel 3 as level / 100. Channel
 * 1 gets direction (speed < 0) and |speed| / 14, which also goes to
 * func_802C4584. The asm clobbers s4 and s5 (conventions.txt). */
void func_802B07DC(void) {
    s32 a = ((u16) D_80364452) + 0x800;
    s32 third;
    f32 pos;
    s32 ch[8];
    s32 level;
    s32 speed;

    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    pos = (f32) (s32) ((u32) a % 0x555) / D_8030D8A4;
    third = (u32) a / 0x555;
    if (third == 0) {
        func_802A0360(pos, D_803ED840, 2, 0);
    } else if (third == 1) {
        func_802A0360(pos, D_803ED840, 2, 1);
    } else {
        func_802A0360(pos, D_803ED840, 2, 2);
    }

    if (D_803EDC06 != 0) {
        D_803EDC06--;
    } else {
        func_802A04BC(5, D_803ED840, ch);
        if (ch[0] != 1 && (D_80370C1A != 0 || D_80370C1B != 0) && D_803EDC00 != 0) {
            D_803EDC00--;
            func_802A0290(D_803ED840, 5, 2);
            D_803EDC06 = 10;
            func_80260650(D_80367738, 0x37, NULL);
        }
    }

    level = D_803EDC05;
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
        level += 5;
        if (level >= 0x33) {
            level = 0x32;
        }
    } else {
        level -= 5;
        if (level < 0x32) {
            level = 0x32;
        }
    }
    D_803EDC05 = level;
    func_802A0360((f32) level / 100.0f, D_803ED840, 3, 0);

    speed = *(s16 *) (D_803EDB40 + 0x76);
    if (speed < 0) {
        func_802A03D4(D_803ED840, 1, 1);
        speed = -speed;
    } else {
        func_802A03D4(D_803ED840, 1, 0);
    }
    if (speed != 0) {
        speed = (u32) speed / 14;
    }
    func_802A039C(D_803ED840, 1, speed);
    func_802C4584(speed);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B07DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

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
#ifdef NON_MATCHING
/* Place vehicle 1's model parts (port_veh_parts, id 1, scale 0x1B58).
 * Returns the current matrix buffer (the asm's s4).
 * Register convention: s0, s1 in through r (only read for a zero-count part),
 * s1 out; s4 = return value; clobbers s1, s2, s4-s7 (conventions.txt). Asm
 * caller func_802B03F4 keeps t6, t7 live and reads f12/f14 afterwards
 * (func_802AA764's temporaries, not modelled). */
u8 *func_802B0B3C(MtxChainRegs *r) {
    return port_veh_parts(D_803EDB40, VPOS1, 1, 0x1B58, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6B4A0/func_802B0B3C.s")
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
