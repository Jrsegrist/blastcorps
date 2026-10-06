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
/* Port-phase shared declarations for the vehicle 3 / vehicle 4 drivers below
 * (func_802B29C0 .. func_802B568C). The asm reaches each vehicle's record
 * through $gp (D_803EE2E0 / D_803EE6C0); the C reads the record directly. */
#define VEH_U8(v, off) (*(u8 *) ((u8 *) (v) + (off)))
#define VEH_S8(v, off) (*(s8 *) ((u8 *) (v) + (off)))
#define VEH_U16(v, off) (*(u16 *) ((u8 *) (v) + (off)))
#define VEH_S16(v, off) (*(s16 *) ((u8 *) (v) + (off)))


extern u8 D_803EE2E0[];  /* vehicle 3 record */
extern u8 D_803EE6C0[];  /* vehicle 4 record */
extern u32 D_803EE38C[]; /* vehicle 3 PortVehPos */
extern u32 D_803EE768[]; /* vehicle 4 PortVehPos */
extern u8 D_803EDFE0[];  /* vehicle 3 animation channels */
extern u8 D_803EE3C0[];  /* vehicle 4 animation channels */
#define VPOS3 ((PortVehPos *) D_803EE38C)
#define VPOS4 ((PortVehPos *) D_803EE768)

extern u8 *D_80358070;   /* heap cursor */
extern u8 *D_803F77D0;
extern u8 D_80305D00[];
extern u8 D_80305D10[];
extern f32 D_8030D8C0;
extern f32 D_8030D8D0;


/* Shared body of func_802B3E40 / func_802B568C: place the vehicle model's
 * parts. The current matrix buffer (D_8035805C ? bufA : bufB) plus the word
 * at model + model[0x18] + 4 is the model matrix m; D_803ED392 = heading
 * (u16 +0x4C), func_802AA764(x, y, z, scale, m); then func_8029C454(x, y, z,
 * id, model + model[4], model + model[8], buffer) and func_802ABBEC(id,
 * model + model[0], model + model[4], buffer). Returns the buffer (the asm's
 * s4). `r` carries func_802AA890's pass-through registers: func_802AA764
 * leaves a3 = 0 (from func_802ACCCC) and s2 = m in the asm, and v1/a0 are
 * still y/z (func_8029C454 preserves them) at func_802ABBEC; s0/s1 come from
 * the caller. They only matter for a zero-count (untransformed) part, whose
 * record then gets s0/s1/s2 >> 11. */
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

/* Shared body of func_802B30F4 / func_802B4818: the value pairs at the
 * vehicle's position and at its front wheel (offset 0 of the wheel table at
 * +0x52 rotated by the heading at +0x4C) on triangle `id` (func_802AAD0C) go
 * to +0x6A/+0x6C and +0x70/+0x72; +0x6E = heading. r's f24 chains through
 * both lookups. Returns veh + 0x52 (the asm's v1, preserved through the
 * callees). */
static s32 port_veh_track(u8 *veh, PortVehPos *p, s32 id, InterpRegs *r) {
    s32 x = p->x;
    s32 z = p->z;
    s32 dx;
    s32 dz;

    func_802AAD0C(id, x, z, r);
    VEH_S16(veh, 0x6A) = r->t3;
    VEH_S16(veh, 0x6C) = r->t4;
    VEH_S16(veh, 0x6E) = VEH_U16(veh, 0x4C);
    dx = func_802A94A4(0, (s16 *) (veh + 0x52), (s16 *) (veh + 0x4C), &dz);
    func_802AAD0C(id, x + dx, z + dz, r);
    VEH_S16(veh, 0x70) = r->t3;
    VEH_S16(veh, 0x72) = r->t4;
    return (s32) (veh + 0x52);
}

/* InterpRegs FP results <-> the TriSideOut func_802A8768 takes (f12 .. f26). */
static void port_interp_to_tri(InterpRegs *r, TriSideOut *f) {
    f->pz = r->f12;
    *(s32 *) &f->cross = r->f14;
    f->cz = r->f20;
    f->side = r->f22;
    f->sideZ = r->f24;
    f->dz = r->f26;
}

static void port_tri_to_interp(TriSideOut *f, InterpRegs *r) {
    r->f12 = f->pz;
    r->f14 = *(s32 *) &f->cross;
    r->f20 = f->cz;
    r->f22 = f->side;
    r->f24 = f->sideZ;
    r->f26 = f->dz;
}

/* Shared body of func_802B3180 / func_802B48A4 (drive along triangle `id`):
 * heading h = func_802AB9A4 from the two value pairs at +0x6A/+0x6C and
 * +0x70/+0x72 (and the target +0x6E) -> +0x4E and +0x4C; the pair at
 * +0x6A/+0x6C again by func_802AAE54 is the position for the ground contact
 * func_802A8768 (vid, divB, divA), whose s3 input is func_802AB9A4's
 * distance and whose FP inputs are func_802AAE54's results; `setup` (the
 * vehicle's setup leaf) runs before it, and D_803ED40B = 1 when `flag40B`.
 * Then parts (`parts`; s0 = flags pointer, s1 = &z in the asm) and the
 * func_802A133C record. Returns func_802A8768's s3. */
static s32 port_veh_follow(u8 *veh, PortVehPos *p, s32 vid, s32 id, InterpRegs *r, void (*setup)(void),
                           s32 flag40B, s32 divB, s32 divA, u8 *(*parts)(MtxChainRegs *)) {
    Regs802A8768 r8;
    TriSideOut f;
    MtxChainRegs mc;
    s32 s3;
    s32 h;

    h = func_802AB9A4((s16 *) (veh + 0x52), VEH_S16(veh, 0x6A), (u16 *) (veh + 0x6E), id, VEH_S16(veh, 0x6C),
                      VEH_S16(veh, 0x70), VEH_S16(veh, 0x72), &s3, r);
    VEH_S16(veh, 0x4E) = h;
    VEH_S16(veh, 0x4C) = h;
    func_802AAE54(id, VEH_S16(veh, 0x6A), VEH_S16(veh, 0x6C), r);
    setup();
    if (flag40B) {
        D_803ED40B = 1;
    }
    r8.s3 = s3;
    port_interp_to_tri(r, &f);
    func_802A8768(veh, vid, &p->x, &p->y, &p->z, r->t3, r->t4, divB, divA, (s16 *) (veh + 0x4C), veh + 0x96,
                  (s16 *) (veh + 0x52), (s32 *) (veh + 0x28), (s32 *) (veh + 0x40), (s32 *) (veh + 0x34),
                  (s32 *) (veh + 4), &r8, &f);
    port_tri_to_interp(&f, r);
    mc.s0 = (s32) (veh + 0x96);
    mc.s1 = (s32) &p->z;
    parts(&mc);
    func_802A133C(p->z, vid, p->x, p->y, veh);
    return r8.s3;
}

/* Shared body of func_802B2FA0 / func_802B46C4 (put the vehicle on the
 * ground): func_802A9A60 (key vid) at the position, with the caller's fp;
 * parts (s0 = z, s1 = func_802A9A60's s1 in the asm); func_802A133C. The
 * asm also hands func_802A9A60 the caller's f12-f26 (a register leak of the
 * C caller that only feeds the FP side results; zero here). */
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

/* First part of the vehicle setup (func_802B29C0 / func_802B4100): model
 * header, two 0x800-byte matrix buffers from the heap, func_802A1388(id, 0,
 * bufA, bufB, model), func_802A754C, the wheel offset table (+0x52, 12
 * halves), position, heading (+0x4C, +0x4E, +0x74). */
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

/* Second part: ground slots (func_802A992C, key id, with the incoming fp; the
 * caller's f12-f26 also go in there in the asm, a register leak that only
 * feeds FP side results: zero here), animation channel 0 reset between the
 * two buffers (func_8029F85C, setters, func_8029E558 both ways), band table
 * (+0x78, 15 halves). */
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
 * func_80258230(id, a1, a2, a2), one run of the per-frame update with +0x9A
 * set (`update`, which then skips its sound/effects step), and the current
 * frame's model matrix copied bufB -> bufA (func_802AA838). */
static void port_veh_init_tail(u8 *veh, PortVehPos *p, s32 id, s32 scale, s32 a1, s32 a2,
                               void (*update)(s32, s32, s32, s32, s32, s32, s32)) {
    u8 *m = p->model;

    func_8029C354(id, m + *(s32 *) (m + 4), m + *(s32 *) (m + 8), scale);
    func_80258230(id, a1, a2, a2);
    VEH_U8(veh, 0x9A) = 1;
    /* The asm enters the update with t6, t7, s0-s4 as func_80258230 (C) and
     * func_8029E558 left them (register leak); with +0x9A set they only pass
     * through func_802ABD54 unused. */
    update(0, 0, 0, 0, 0, 0, 0);
    VEH_U8(veh, 0x9A) = 0;
    m = p->model;
    func_802AA838((u8 *) p->bufB, (u8 *) p->bufA, *(s32 *) (m + *(s32 *) (m + 0x18) + 4));
}

/* Middle of the per-frame updates (func_802B327C, func_802B49AC), after the
 * steering: speed/heading (func_802A7FD8 with `rate`), drive force
 * (func_802A83B8, func_802A843C kind/div), func_802A7070 when `drift` is
 * set, move along the heading (func_802A860C), D_803ED40B = 1, ground
 * contact (func_802A8768 id/divB/divA; s3 in = func_802A860C's s3), the
 * per-frame animation (func_8029E558 on the buffers by D_8035805C), parts
 * (`parts`), func_8029A800 (a1, b2, b0, h2), func_8029C52C, func_8029AA10. */
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
    /* The asm hands func_802A8768 whatever f12-f26 hold here (f12/f14 are
     * func_802A860C's cosine/sine temporaries, f20-f26 the caller's): a
     * register leak that only feeds the scans' FP side results. */
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A8768(veh, id, &p->x, &p->y, &p->z, x, o.t1, divB, divA, (s16 *) (veh + 0x4C), veh + 0x96,
                  (s16 *) (veh + 0x52), (s32 *) (veh + 0x28), (s32 *) (veh + 0x40), (s32 *) (veh + 0x34),
                  (s32 *) (veh + 4), &r8, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) p->bufA, (u8 *) p->bufB, ch);
    } else {
        func_8029E558((u8 *) p->bufB, (u8 *) p->bufA, ch);
    }
    /* s0/s1 here are func_8029E558's leftovers in the asm (register leak,
     * read only for a zero-count part). */
    mc.s0 = 0;
    mc.s1 = 0;
    parts(&mc);
    func_8029A800(p->z, (s32) a1, b2, 1, p->x, p->y, b0, VEH_S16(veh, 0x76), h2, 0, id, veh);
    func_8029C52C(id, veh);
    func_8029AA10(id);
}

/* The turn after a collision report (D_803A7425): the distance d between the
 * reversed target heading (+0x4E - 0x800, wrapped) and the ring midpoint
 * (func_802A6F6C), folded to 0..0x800; then func_802A70D8, the turn
 * func_802A71DC(+0x4E, +0x4C, scale) whose new target goes to *target,
 * +0x4E and +0x74, func_802A746C(turn, d) (d is only the asm's leftover v1
 * there; the C-side callee returns it untouched when heading == target) and
 * func_802A6FE4(0). */
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

/* Restore the saved vehicle state after a collision (func_802A768C; the
 * buffer copy goes from the current buffer into the other one). */
static void port_veh_restore(u8 *veh, PortVehPos *p, u8 *ch) {
    u64 *end;

    if (D_8035805C != 0) {
        func_802A768C(veh, ch, &p->x, &p->y, &p->z, p->bufB, p->bufA, 0x800, &end);
    } else {
        func_802A768C(veh, ch, &p->x, &p->y, &p->z, p->bufA, p->bufB, 0x800, &end);
    }
}

/* Publish the vehicle's position, speed and headings, then its func_802A133C
 * record. */
static void port_veh_publish(u8 *veh, PortVehPos *p, s32 id) {
    D_803643E0 = p->x;
    D_803643E4 = p->y;
    D_803643E8 = p->z;
    D_8036443C = VEH_S16(veh, 0x76);
    D_8036443E = VEH_U16(veh, 0x4E);
    D_80364440 = VEH_U16(veh, 0x4C);
    func_802A133C(p->z, id, p->x, p->y, veh);
}

u8 *func_802B3E40(MtxChainRegs *r);
u8 *func_802B568C(MtxChainRegs *r);
void func_802B3FF0(void);
void func_802B5814(void);

static const s16 sVeh3Wheels[12] = { 0x130, 0x16A, -0x130, 0x16A, 0x130, -0x16A,
                                     0x140, 0x172, -0x140, 0x172, 0x140, -0x172 };
static const s16 sVeh3Bands[15] = { -0xB4, 5, 2, 0, 0x78, 4, 0x78, 0xA0, 5, 0xA0, 0xDC, 2, 0xDC, 0xFA, 2 };
static const s16 sVeh4Wheels[12] = { 0xC8, 0x12C, -0xC8, 0x12C, 0xC8, -0x12C,
                                     0x12C, 0x1C2, -0x12C, 0x1C2, 0x12C, -0x1C2 };
static const s16 sVeh4Bands[15] = { -0xB4, 0, 6, 0, 0x50, 6, 0x50, 0x8C, 4, 0x8C, 0xBE, 2, 0xBE, 0xFA, 2 };
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE3AF;
extern f32 D_803EE3A4;
extern u8 D_803EE3B2;
extern u8 D_803EE3AE;
extern s8 D_803EE3B0;
extern void *D_803EE388;

/* Vehicle 3 setup, called from the asm dispatcher func_802A350C: model
 * header, buffers and record (port_veh_init_head with this vehicle's wheel
 * table), D_803F7840 = 0, level D_803EE3AF = 50, D_803EE3A4 = 0.5, ground
 * slots and animation reset (port_veh_init_anim, band table), func_802A6F00,
 * meters/timers (D_803EE3B1 = 100, D_803EE3B2/AE/B0 = 0, no boost sound
 * D_803EE388), parts (scale 0x2AF8), func_80258230(3, 0x50, 0x1F, 0x1F), one
 * update (func_802B327C with +0x9A set), buffer copy, D_80364A69 = 1.
 * Register convention: model s2, x t7, y s3, z s0, heading s1, and fp (the
 * ground slot flag seed for func_802A992C) (conventions.txt). The asm saves
 * t0-t5 (its caller keeps t1, t2 live). Register leaks not modelled: the
 * caller's f12-f26 go into func_802A992C (FP side results only), and the
 * dispatcher reads s2-s4, fp and f12-f26 afterwards, which hold whatever
 * func_802B327C / func_8029E558 / func_802AA764 left (a mixed N64 build would
 * need a thunk). */
void func_802B29C0(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp) {
    u8 *veh = D_803EE2E0;
    PortVehPos *p = VPOS3;

    port_veh_init_head(veh, p, 3, model, sVeh3Wheels, x, y, z, heading);
    D_803F7840 = 0;
    D_803EE3AF = 50;
    D_803EE3A4 = 0.5f;
    port_veh_init_anim(veh, p, 3, D_803EDFE0, sVeh3Bands, x, z, fp);
    func_802A6F00(veh);
    D_803EE3B1 = 100;
    D_803EE3B2 = 0;
    D_803EE3AE = 0;
    D_803EE3B0 = 0;
    D_803EE388 = NULL;
    port_veh_init_tail(veh, p, 3, 0x2AF8, 0x50, 0x1F, func_802B327C);
    D_80364A69 = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B29C0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE2E0[]; /* vehicle type 3's state block */
extern u8 D_803EDFE0[]; /* its animation channel table */
extern u8 D_802C2314[]; /* key of its func_802A05D0.. sound entry */

/* Vehicle-type 3 setup (called from hd.c / 17210.c): clears byte 0x99 of the
 * state block, resets animation channels 1-3 of D_803EDFE0, resets the
 * D_802C2314 entry (0, 0, 0, -1), D_8036444C/50 = 3400, 1000, then
 * func_802C4310(&D_803EDFE0, 0x8C) (the asm's a0 is still the table).
 * The asm points $gp at D_803EE2E0 and leaves it there (conventions.txt:
 * clobbers gp) and returns with v1 = -1; the C callers use neither. */
void func_802B2D7C(void) {
    s32 i;

    D_803EE2E0[0x99] = 0;
    for (i = 1; i <= 3; i++) {
        func_802A039C(D_803EDFE0, i, 0);
        func_802A03D4(D_803EDFE0, i, 0);
        func_802A040C(D_803EDFE0, i, i == 1 ? 0 : 1);
        func_802A0290(D_803EDFE0, i, -1);
    }
    func_802A05D0((s32) D_802C2314, 0);
    func_802A05F8((s32) D_802C2314, 0);
    func_802A0620((s32) D_802C2314, 0);
    func_802A0508((s32) D_802C2314, -1);
    D_8036444C = 0xD48;
    D_80364450 = 0x3E8;
    func_802C4310((s32) D_803EDFE0, 0x8C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B2D7C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE2E0[]; /* this vehicle's state block */

/* Exit check for vehicle type 3 (called from func_8024B4B8 in hd.c, which
 * declares it void but returns the leftover v0). Returns 1 when none of the
 * bytes at +0x96, +0x97, +0x98 equals 1, else 0. Returns s32: the asm
 * leaves the full 0/1 in v0. Same shape as func_802B45FC (below),
 * func_802B5F04 (71140), func_802B76F8 (72B80); func_802B1150 (6C5E0) adds a
 * +0xA1 test. */
s32 func_802B2EF8(void) {
    if (D_803EE2E0[0x96] == 1 || D_803EE2E0[0x97] == 1 || D_803EE2E0[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B2EF8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EE39C;
extern u64 *D_803EE3A0;

/* Vehicle-type 3 shutdown (called from hd.c's func_8024B188): zeroes the
 * speed (s16 at +0x76), func_802A7764(D_803EE39C, D_803EE3A0, 0x800), then
 * func_802C444C(). */
void func_802B2F54(void) {
    *(s16 *) (D_803EE2E0 + 0x76) = 0;
    func_802A7764(D_803EE39C, D_803EE3A0, 0x800);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B2F54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Put vehicle 3 on the ground (called from C func_8024B618 in 00000.c as
 * `void (void)`): port_veh_ground with key 3, then func_802B3E40.
 * Register convention: fp in (conventions.txt). The C caller doesn't set fp:
 * the asm passes on whatever an outer function left in $fp/$s8 (register
 * leak; it seeds func_802A9A60's D_803ED3F2 flags), and its f12-f26 into the
 * scans. The asm saves every callee-saved register. */
void func_802B2FA0(s32 fp) {
    port_veh_ground(D_803EE2E0, VPOS3, 3, fp, func_802B3E40);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B2FA0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803EE38C[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 3 at its position
 * D_803EE38C..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802B327C keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802B30B0(ZoneScanRegs *r) {
    return func_802ABD54(3, D_803EE38C[0], D_803EE38C[1], D_803EE38C[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B30B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Track points of vehicle 3 on triangle `id` (port_veh_track); called from
 * the asm dispatcher func_802AB50C. Returns veh + 0x52 (the asm's v1).
 * Register convention: id a3, f24 in; v1, f24, f26 out (r); the asm saves t0,
 * t1, t3, t4 (its caller keeps t0, t1 live) and leaves gp = D_803EE2E0, s4 =
 * veh + 0x4C (conventions.txt: clobbers). */
s32 func_802B30F4(s32 id, InterpRegs *r) {
    return port_veh_track(D_803EE2E0, VPOS3, id, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B30F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Drive vehicle 3 along triangle `id` (port_veh_follow: setup leaf
 * func_802B3FF0, ground contact divisors 0x2D4 / 0x260, parts func_802B3E40);
 * called from the asm dispatcher func_802AB714. Returns func_802A8768's s3.
 * Register convention: id a3, f24 in; s3, f24, f26 out (r). The asm saves
 * a3, t0-t2 (its caller keeps them live) and changes the other s-registers,
 * fp and gp (conventions.txt: clobbers). */
s32 func_802B3180(s32 id, InterpRegs *r) {
    return port_veh_follow(D_803EE2E0, VPOS3, 3, id, r, func_802B3FF0, 0, 0x2D4, 0x260, func_802B3E40);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3180.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802B30B0(ZoneScanRegs *r);
void func_802B37B0(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
s32 func_802B3F78(void);
extern s16 D_803EE3AA;
extern s16 D_803EE3A8;

/* Vehicle 3 per-frame update (called from C func_8024B7AC in 00000.c as
 * `void (void)`, and from func_802B29C0):
 * 1. zone level (func_802B30B0) with the incoming scan registers, save state
 *    (func_802A75DC), effects func_802B37B0 (with the scan registers as the
 *    zone lookup left them) unless +0x9A is set, func_802CB690 when
 *    D_80367BFF is set;
 * 2. setup leaf func_802B3FF0, steering func_802A7E70(func_802B3F78()),
 *    speed bands func_802A785C unless the countdown D_803EE3B2 runs;
 * 3. port_veh_move (rate D_803EE3A8, kind 3 / 724.0, drift D_803EE3B0 via
 *    D_803EE3AA, divisors 0x2D4 / 0x260, parts func_802B3E40, func_8029A800
 *    with D_80305D00, 1, 6, 100);
 * 4. no collision report (D_803A7425 == 0): D_803A7424 = 0, collision pass
 *    func_802BE77C (D_803F77D0 = the channel table); if it sets D_803A7424,
 *    restore (port_veh_restore), countdown 5, speed bounced to -max(|v|,
 *    80) / 2 (sign kept, sra) and parts again; else D_803EE3B0 = 0.
 *    Report: func_8029A914, D_803EE3B0 = 1, port_veh_turn (D_8030D8C0,
 *    D_803EE3AA), collision pass. (A bounce test of the turn distance
 *    against 200 follows an unconditional branch in the asm: dead code.)
 * 5. port_veh_publish.
 * Register convention: t6, t7, s0-s4 in (only through func_802ABD54 to
 * func_802B37B0; from the C caller they are whatever it left: register
 * leak). The asm saves all callee-saved registers and leaves v1 = y; asm
 * caller func_802B29C0 reads f12/f14 afterwards (func_802AA764's leftovers,
 * not modelled). */
void func_802B327C(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    u8 *veh = D_803EE2E0;
    PortVehPos *p = VPOS3;
    ZoneScanRegs zs;
    MtxChainRegs mc;
    s32 v;

    zs.t6 = t6;
    zs.t7 = t7;
    zs.s0 = s0;
    zs.s1 = s1;
    zs.s2 = s2;
    zs.s3 = s3;
    zs.s4 = s4;
    func_802B30B0(&zs);
    func_802A75DC(veh, D_803EDFE0, &p->x, &p->y, &p->z);
    if (VEH_S8(veh, 0x9A) == 0) {
        func_802B37B0(zs.t6, zs.t7, zs.s0, zs.s1, zs.s2, zs.s3, zs.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(veh);
    }
    func_802B3FF0();
    func_802A7E70(func_802B3F78(), (u16 *) (veh + 0x4C));
    if (D_803EE3B2 == 0) {
        func_802A785C(veh, (s16 *) (veh + 0x76), 3, veh + 0x96, (s16 *) (veh + 0x78), 0x14);
    } else {
        D_803EE3B2--;
    }
    port_veh_move(veh, p, 3, D_803EDFE0, (u16) D_803EE3A8, 3, 724.0f, D_803EE3B0, &D_803EE3AA, 0x2D4, 0x260,
                  func_802B3E40, D_80305D00, 1, 6, 100);
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803EDFE0;
        func_802BE77C(3, veh);
        if (D_803A7424 != 0) {
            D_803EE3B0 = 0;
            port_veh_restore(veh, p, D_803EDFE0);
            D_803EE3B2 = 5;
            v = VEH_S16(veh, 0x76);
            if (v >= 0) {
                if (v < 0x50) {
                    v = 0x50;
                }
            } else if (v >= -0x4F) {
                v = -0x50;
            }
            VEH_S16(veh, 0x76) = -v >> 1;
            /* s0/s1: leftovers of func_802BE77C / func_8029E558 (leak). */
            mc.s0 = 0;
            mc.s1 = 0;
            func_802B3E40(&mc);
        } else {
            D_803EE3B0 = 0;
        }
    } else {
        func_8029A914(veh);
        D_803EE3B0 = 1;
        port_veh_turn(veh, D_8030D8C0, &D_803EE3AA);
        D_803F77D0 = D_803EDFE0;
        func_802BE77C(3, veh);
    }
    port_veh_publish(veh, p, 3);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B327C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE3AE;  /* func_802A6274 spawn cooldown */
extern u8 D_803EE3AF;  /* 0..100 level for channel 2 */
extern u8 D_803EE3B2;  /* boost countdown */
extern u8 D_803EE3B3;  /* boost sound playing */
extern s16 D_803EE3AC;
extern f32 D_803EE3A4; /* 0..1 level for channel 3 */
extern void *D_803EE388; /* boost sound handle */
extern void *D_80367738;
extern f32 D_8030D8C4;
extern f32 D_8030D8C8;
extern f32 D_8030D8CC;
extern u8 D_802C37C0[];
void func_802B3C68(void);

/* Vehicle type 3 per-frame update ($gp = D_803EE2E0, read as the global):
 * 1. Cooldown D_803EE3AE counts down; at 0 with byte +0x99 set it restarts at
 *    1 and, while fewer than 15 func_802A6274 records are active, sets up two
 *    (def D_802C2954, data 0x29810, type 1 at (3, 2, 1) and (3, 3, 1)).
 * 2. Level D_803EE3A4 (channel 3 of D_803EDFE0): unless func_802A7CB0(10),
 *    falls by D_8030D8C4 to 0 with D_80370C23 held, or rises by D_8030D8C8 to
 *    1 with D_80370C1C held; otherwise it moves by D_8030D8CC toward 0.5.
 * 3. D_803EE3AF steps by -5 (D_80370C15) / +5 (D_80370C16) within 0..100, or
 *    by 10 toward 50; channel 2 gets it / 100.
 * 4. Channel 1: field 03D4 = (speed < 0), 039C = |speed| / 30 (also passed to
 *    func_802C4584).
 * 5. Boost: D_803EE3B2 counts down; at 0 with D_80370C1A or D_80370C1B held
 *    and the meter D_803EE3B1 >= 4: meter -= 4, start sound 0x8D once
 *    (D_803EE3B3), sound entry D_802C2314 on, one more func_802A6274 record
 *    (def D_802C37C0, data 0x186A0, type 1 at (3, 1, 1)) and speed += 40 up
 *    to 400. Otherwise stop the sound, D_803EE3AC = 120, entry off, speed
 *    above 250 drops by 25 (not below 250) and the meter refills by 1 to 100.
 * 6. func_802B3C68 (tyre trail).
 * Register convention: the asm passes t6, t7, s0-s4 through to
 * func_802A6274 (t6 and s1 in its in/out block, carried from one call to the
 * next) and changes s5-s7 (conventions.txt: clobbers). Same shape as
 * func_802B4EF8 (below). */
void func_802B37B0(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    f32 f;
    s32 near;
    s32 v;
    s32 speed;

    io.t6 = t6;
    io.s1 = s1;

    if (D_803EE3AE != 0) {
        D_803EE3AE--;
    } else if (D_803EE2E0[0x99] != 0) {
        D_803EE3AE = 1;
        if (func_802A5ED0() < 15) {
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 3, 2, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 3, 3, 1, t7, s0, s2, s3, s4, 1);
        }
    }

    /* 2. channel 3 level */
    f = D_803EE3A4;
    near = func_802A7CB0(D_803EE2E0, 10);
    if (near == 0 && D_80370C23 != 0) {
        f -= D_8030D8C4;
        if (f < 0.0f) {
            f = 0.0f;
        }
    } else if (near == 0 && D_80370C1C != 0) {
        f += D_8030D8C8;
        if (!(f <= 1.0f)) {
            f = 1.0f;
        }
    } else if (f < 0.5f) {
        f += D_8030D8CC;
        if (!(f <= 0.5f)) {
            f = 0.5f;
        }
    } else {
        f -= D_8030D8CC;
        if (f < 0.5f) {
            f = 0.5f;
        }
    }
    D_803EE3A4 = f;
    func_802A0360(f, D_803EDFE0, 3, 0);

    /* 3. channel 2 level */
    v = D_803EE3AF;
    if (D_80370C15 != 0) {
        v -= 5;
        if (v < 0) {
            v = 0;
        }
    } else if (D_80370C16 != 0) {
        v += 5;
        if (v > 100) {
            v = 100;
        }
    } else if (v >= 50) {
        v -= 10;
        if (v < 50) {
            v = 50;
        }
    } else {
        v += 10;
        if (v > 50) {
            v = 50;
        }
    }
    D_803EE3AF = v;
    func_802A0360((f32) v / 100.0f, D_803EDFE0, 2, 0);

    /* 4. channel 1 from the speed */
    speed = *(s16 *) (D_803EE2E0 + 0x76);
    func_802A03D4(D_803EDFE0, 1, speed < 0);
    if (speed < 0) {
        speed = -speed;
    }
    speed = (u32) speed / 30;
    func_802A039C(D_803EDFE0, 1, speed);
    func_802C4584(speed);

    /* 5. boost */
    if (D_803EE3B2 != 0) {
        D_803EE3B2--;
    } else if ((D_80370C1A != 0 || D_80370C1B != 0) && ((s8) D_803EE3B1) >= 4) {
        D_803EE3B1 -= 4;
        if (D_803EE3B3 == 0) {
            D_803EE3B3 = 1;
            func_80260650(D_80367738, 0x8D, &D_803EE388);
        }
        func_802A05A4(0.0f, (s32) D_802C2314, 1);
        io.a3 = 0;
        func_802A6274(&io, D_802C37C0, 0x186A0, 1, 3, 1, 1, t7, s0, s2, s3, s4, 1);
        speed = *(s16 *) (D_803EE2E0 + 0x76);
        *(s16 *) (D_803EE2E0 + 0x76) = (speed < 400) ? speed + 40 : 400;
        func_802B3C68();
        return;
    }
    if (D_803EE388 != NULL) {
        func_802608C8(D_803EE388);
        D_803EE388 = NULL;
    }
    D_803EE3AC = 120;
    D_803EE3B3 = 0;
    func_802A05A4(0.0f, (s32) D_802C2314, 0);
    speed = *(s16 *) (D_803EE2E0 + 0x76);
    if (speed > 250) {
        speed -= 25;
        if (speed < 250) {
            speed = 250;
        }
        *(s16 *) (D_803EE2E0 + 0x76) = speed;
    }
    v = ((s8) D_803EE3B1) + 1;
    if (v > 100) {
        v = 100;
    }
    D_803EE3B1 = v;

    /* 6. */
    func_802B3C68();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B37B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE2E0[];  /* this vehicle's state block (the asm's $gp here) */
extern u32 D_803EE38C[]; /* [0], [2]: trail x, z */
extern u8 D_803EE3B3;
extern s16 D_803EE3AC;

/* Tyre trail. When the byte at +0x99 is set the trail's a/b value is 50;
 * else, only while D_803EE3B3 is set, D_803EE3AC counts down by 8 to 0
 * (clamped) and its new value is used. Then, unless +0x98 is 1, +0x50 >= 3 or
 * +0x9B is set, adds a trail segment (func_8027BE7C, period 2, wheel offsets
 * (+-400, -300), position D_803EE38C[0]/[2], yaw u16 +0x4E, half-width 4).
 * The asm passes the u16/u8 arguments as full words; the C prototype narrows
 * them (only their low bits are read).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802B37B0 keeps a1-a3 live (a mixed N64 build would need a
 * thunk). Same shape: func_802B54EC (below), func_802B69F8 (71140),
 * func_802B80D8 (72B80), func_802CB224 (853D0), func_802CC56C (86F60),
 * func_802CD660 (88160), func_802D0438 (8AEE0). */
void func_802B3C68(void) {
    s32 col;

    if (D_803EE2E0[0x99] != 0) {
        col = 50;
    } else {
        if (D_803EE3B3 == 0) {
            return;
        }
        col = D_803EE3AC;
        if (col < 8) {
            col = 8;
        }
        col -= 8;
        D_803EE3AC = col;
    }
    if (D_803EE2E0[0x98] != 1 && D_803EE2E0[0x50] < 3 && D_803EE2E0[0x9B] == 0) {
        func_8027BE7C(2, *(s32 *) (D_803EE2E0 + 0x1C), 400, -300, -400, -300, D_803EE38C[0], D_803EE38C[2],
                      *(u16 *) (D_803EE2E0 + 0x4E), 4, col, col, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3C68.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Place vehicle 3's model parts (port_veh_parts, id 3, scale 0x2AF8).
 * Returns the current matrix buffer (the asm's s4).
 * Register convention: s0, s1 in through r (only read for a zero-count part),
 * s1 out (func_802AA890's last y'); s4 = return value; clobbers s1, s2,
 * s4-s7 (conventions.txt). Asm caller func_802B327C keeps t6, t7 live; it
 * and func_802B29C0 also read f12/f14 afterwards (func_802AA764's
 * cosine/sine temporaries, not modelled). */
u8 *func_802B3E40(MtxChainRegs *r) {
    return port_veh_parts(D_803EE2E0, VPOS3, 3, 0x2AF8, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3E40.s")
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

/* Speed (s16 at +0x76 of D_803EE2E0, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / 2.5, rounded to nearest. The asm
 * returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B327C keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802B3F78(void) {
    f32 div;
    s32 r;

    if (D_803EE2E0[0x96] == 1 || D_803EE2E0[0x97] == 1 || D_803EE2E0[0x98] == 1) {
        div = 11.0f;
    } else {
        div = 2.5f;
    }
    CVT_W_S(r, *(s16 *) (D_803EE2E0 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3F78.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803EE3A8;

/* Vehicle-module setup leaf (shape of func_802B5814, below): D_803EBBF4 =
 * D_803EBBF0 * 4, D_803ED3F6/7 = 60, 3, then D_803EE3A8 = 2000 if the button
 * byte D_80370C1C (speed <= 0) or D_80370C23 (speed > 0) is set, else 15000.
 * Speed is the s16 at +0x76 of D_803EE2E0 (the asm's $gp).
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm callers keep
 * registers live across the call (func_802B3180: t3, t4, f12, f14;
 * func_802B327C: a0-a3); a mixed N64 build would need a thunk. */
void func_802B3FF0(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
    if (*(s16 *) (D_803EE2E0 + 0x76) <= 0) {
        D_803EE3A8 = (D_80370C1C == 0) ? 15000 : 2000;
    } else {
        D_803EE3A8 = (D_80370C23 == 0) ? 15000 : 2000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3FF0.s")
#endif

/* func_802B40A8/func_802B40D4: two-address trampolines into
 * func_802AC7DC/func_802AC85C, same confirmed-unreachable-from-C 8-byte
 * sd-$ra frame as func_802AC284 (hd_code/679E0.c). Permanently
 * GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE2E0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EE38C[]; /* plus these three words */

/* Serialize this vehicle's state (D_803EE2E0[0..0xA5] plus the three words
 * D_803EE38C[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B40A8(u8 *dst) {
    return func_802AC7DC(dst, D_803EE2E0, D_803EE38C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B40A8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B40A8: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B40D4(void *)`. */
void func_802B40D4(void *src) {
    func_802AC85C(src, D_803EE2E0, D_803EE38C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B40D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE788;
extern u8 D_803EE789;
extern u8 D_803EE78A;
extern f32 D_803EE780;

/* Vehicle 4 setup, called from the asm dispatcher func_802A350C; as
 * func_802B29C0 with this vehicle's tables: func_802A6F00, then
 * D_803EE789/8A/88 = 0, D_803F7840 = 0, D_803EE780 = 0.5; parts scale
 * 0x2710, func_80258230(4, 0x96, 0x32, 0x32), update func_802B49AC; no
 * final flag. Same register convention and leaks as func_802B29C0. */
void func_802B4100(u8 *model, s32 x, s32 y, s32 z, s32 heading, s32 fp) {
    u8 *veh = D_803EE6C0;
    PortVehPos *p = VPOS4;

    port_veh_init_head(veh, p, 4, model, sVeh4Wheels, x, y, z, heading);
    port_veh_init_anim(veh, p, 4, D_803EE3C0, sVeh4Bands, x, z, fp);
    func_802A6F00(veh);
    D_803EE789 = 0;
    D_803EE78A = 0;
    D_803EE788 = 0;
    D_803F7840 = 0;
    D_803EE780 = 0.5f;
    port_veh_init_tail(veh, p, 4, 0x2710, 0x96, 0x32, func_802B49AC);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4100.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE6C0[]; /* vehicle type 4's state block */
extern u8 D_803EE3C0[]; /* its animation channel table */

/* Vehicle-type 4 setup (called from hd.c / 17210.c), shape of func_802B2D7C:
 * clears byte 0x99 of the state block, resets channel 1 (03D4 only) and
 * channel 2 of D_803EE3C0, resets the sound entries D_802C2190/A4/B8
 * (0, 0, 0, -1 each), D_8036444C/50 = 3000, 1000, then
 * func_802C4310(&D_803EE3C0, 0x0B). Leaves $gp = D_803EE6C0 (clobbers gp)
 * and v1 = -1, unused by the C callers. */
void func_802B448C(void) {
    D_803EE6C0[0x99] = 0;
    func_802A03D4(D_803EE3C0, 1, 0);
    func_802A05D0((s32) D_802C2190, 0);
    func_802A05F8((s32) D_802C2190, 0);
    func_802A0620((s32) D_802C2190, 0);
    func_802A0508((s32) D_802C2190, -1);
    func_802A05D0((s32) D_802C21A4, 0);
    func_802A05F8((s32) D_802C21A4, 0);
    func_802A0620((s32) D_802C21A4, 0);
    func_802A0508((s32) D_802C21A4, -1);
    func_802A05D0((s32) D_802C21B8, 0);
    func_802A05F8((s32) D_802C21B8, 0);
    func_802A0620((s32) D_802C21B8, 0);
    func_802A0508((s32) D_802C21B8, -1);
    func_802A039C(D_803EE3C0, 2, 0);
    func_802A03D4(D_803EE3C0, 2, 0);
    func_802A040C(D_803EE3C0, 2, 1);
    func_802A0290(D_803EE3C0, 2, -1);
    D_8036444C = 0xBB8;
    D_80364450 = 0x3E8;
    func_802C4310((s32) D_803EE3C0, 0x0B);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B448C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE6C0[]; /* this vehicle's state block */

/* Exit check for vehicle type 4, same shape as func_802B2EF8 (above): returns
 * 1 when none of the bytes at +0x96, +0x97, +0x98 equals 1, else 0 (s32; the
 * C caller in hd.c declares it void and returns the leftover v0). */
s32 func_802B45FC(void) {
    if (D_803EE6C0[0x96] == 1 || D_803EE6C0[0x97] == 1 || D_803EE6C0[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B45FC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EE778;
extern u64 *D_803EE77C;

/* Vehicle-type 4 shutdown (called from hd.c's func_8024B188): zeroes the
 * speed (s16 at +0x76), func_802A7764(D_803EE778, D_803EE77C, 0x800),
 * func_802C444C(), then func_802A05D0(key, 0) for D_802C2190 and D_802C21A4.
 * The asm returns with v1 = 0; the C caller ignores it. */
void func_802B4658(void) {
    *(s16 *) (D_803EE6C0 + 0x76) = 0;
    func_802A7764(D_803EE778, D_803EE77C, 0x800);
    func_802C444C();
    func_802A05D0((s32) D_802C2190, 0);
    func_802A05D0((s32) D_802C21A4, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4658.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Put vehicle 4 on the ground; as func_802B2FA0 (key 4, parts
 * func_802B568C), same fp convention and leaks. */
void func_802B46C4(s32 fp) {
    port_veh_ground(D_803EE6C0, VPOS4, 4, fp, func_802B568C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B46C4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803EE768[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 4 at its position
 * D_803EE768..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802B49AC keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802B47D4(ZoneScanRegs *r) {
    return func_802ABD54(4, D_803EE768[0], D_803EE768[1], D_803EE768[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B47D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802B30F4 for vehicle 4. */
s32 func_802B4818(s32 id, InterpRegs *r) {
    return port_veh_track(D_803EE6C0, VPOS4, id, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4818.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802B3180 for vehicle 4: setup leaf func_802B5814, then
 * D_803ED40B = 1, divisors 0x258 / 0x190, parts func_802B568C. */
s32 func_802B48A4(s32 id, InterpRegs *r) {
    return port_veh_follow(D_803EE6C0, VPOS4, 4, id, r, func_802B5814, 1, 0x258, 0x190, func_802B568C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B48A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802B47D4(ZoneScanRegs *r);
void func_802B4EF8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
s32 func_802B57C4(void);
extern s16 D_803EE784;
extern s16 D_803EE786;

/* Vehicle 4 per-frame update (called from C func_8024B7AC as `void (void)`,
 * and from func_802B4100); as func_802B327C with:
 * - after the save, the second looping sound func_802C4724(0x8F);
 *   effects func_802B4EF8;
 * - D_803F7840 = speed before the setup leaf func_802B5814; steering rate
 *   func_802B57C4(); countdown D_803EE78A;
 * - port_veh_move: rate D_803EE784, kind 4 / 600.0, drift D_803EE789 via
 *   D_803EE786, divisors 0x258 / 0x190, parts func_802B568C, func_8029A800
 *   with D_80305D10, 0, 8, 120;
 * - bounce speed: |v| clamped to 62..125 (positive) / 45..90 (negative:
 *   v >= -44 gives -45, v < -90 gives -90), negated (no halving);
 * - func_802BCC10 before the collision pass on the report path.
 * Same register convention and leaks as func_802B327C. */
void func_802B49AC(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    u8 *veh = D_803EE6C0;
    PortVehPos *p = VPOS4;
    ZoneScanRegs zs;
    MtxChainRegs mc;
    s32 v;

    zs.t6 = t6;
    zs.t7 = t7;
    zs.s0 = s0;
    zs.s1 = s1;
    zs.s2 = s2;
    zs.s3 = s3;
    zs.s4 = s4;
    func_802B47D4(&zs);
    func_802A75DC(veh, D_803EE3C0, &p->x, &p->y, &p->z);
    func_802C4724(0x8F);
    if (VEH_S8(veh, 0x9A) == 0) {
        func_802B4EF8(zs.t6, zs.t7, zs.s0, zs.s1, zs.s2, zs.s3, zs.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(veh);
    }
    D_803F7840 = VEH_S16(veh, 0x76);
    func_802B5814();
    func_802A7E70(func_802B57C4(), (u16 *) (veh + 0x4C));
    if (D_803EE78A == 0) {
        func_802A785C(veh, (s16 *) (veh + 0x76), 3, veh + 0x96, (s16 *) (veh + 0x78), 0x14);
    } else {
        D_803EE78A--;
    }
    port_veh_move(veh, p, 4, D_803EE3C0, (u16) D_803EE784, 4, 600.0f, (s8) D_803EE789, &D_803EE786, 0x258,
                  0x190, func_802B568C, D_80305D10, 0, 8, 120);
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803EE3C0;
        func_802BE77C(4, veh);
        if (D_803A7424 != 0) {
            D_803EE789 = 0;
            port_veh_restore(veh, p, D_803EE3C0);
            D_803EE78A = 5;
            v = VEH_S16(veh, 0x76);
            if (v >= 0) {
                if (v < 0x3E) {
                    v = 0x3E;
                } else if (v >= 0x7E) {
                    v = 0x7D;
                }
            } else if (v >= -0x2C) {
                v = -0x2D;
            } else if (v < -0x5A) {
                v = -0x5A;
            }
            VEH_S16(veh, 0x76) = -v;
            /* s0/s1: leftovers of func_802BE77C / func_8029E558 (leak). */
            mc.s0 = 0;
            mc.s1 = 0;
            func_802B568C(&mc);
        } else {
            D_803EE789 = 0;
        }
    } else {
        func_8029A914(veh);
        D_803EE789 = 1;
        port_veh_turn(veh, D_8030D8D0, &D_803EE786);
        func_802BCC10();
        D_803F77D0 = D_803EE3C0;
        func_802BE77C(4, veh);
    }
    port_veh_publish(veh, p, 4);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B49AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE788;  /* func_802A6274 spawn cooldown */
extern u8 D_803EE789;  /* channel 1 restart request */
extern f32 D_803EE780; /* 0..1 level for channel 2 */
extern f32 D_8030D8D4;
extern f32 D_8030D8D8;
extern f32 D_8030D8DC;
void func_802B54EC(void);

/* Vehicle type 4 per-frame update ($gp = D_803EE6C0, read as the global),
 * shape of func_802B37B0 (above):
 * 1. func_802B54EC (tyre trail).
 * 2. Cooldown D_803EE788 counts down; at 0 with byte +0x99 set it restarts
 *    at 1 and, while fewer than 14 func_802A6274 records are active, sets up
 *    one (def D_802C2954, data 0x30D40, type 1 at (4, 1, 1)).
 * 3. With D_803F7840 == 0 and the speed (s16 +0x76) nonzero: sound 10
 *    (func_80260650, the asm saving every register around it) and another
 *    record (data 0x1D4C0 at (4, 2, 1)).
 * 4. Level D_803EE780 (channel 2 of D_803EE3C0): unless func_802A7CB0(30),
 *    falls by D_8030D8D4 to 0 with D_80370C23 held, or rises by D_8030D8D8
 *    to 1 with D_80370C1C held; otherwise it moves by D_8030D8DC toward 0.5.
 * 5. Sound entries D_802C2190/D_802C21A4: moving, unk11 = (speed < 0) on
 *    both, unk14 = |speed| / 2 on both and func_802C4584(|speed| / 8);
 *    standing, unk11 = (stick < 0) / (stick >= 0) and unk14 = |stick|
 *    (D_80370C2C).
 * 6. Entry D_802C21B8: unk13 = (u16 +0x4C in [0x355, 0x8AB) ? it - 0x355 :
 *    0) / 76, unk4 = 0.
 * 7. With D_803EE789 set and channel 1's field 0x10 != 1: channel 1 gets
 *    (9, 1, 2) and sound 1 plays.
 * Register convention: as func_802B37B0, t6, t7, s0-s4 pass through to
 * func_802A6274 (conventions.txt); s5-s7 change. */
void func_802B4EF8(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 ch[8];
    f32 f;
    s32 near;
    s32 speed;
    s32 v;

    io.t6 = t6;
    io.s1 = s1;

    func_802B54EC();

    if (D_803EE788 != 0) {
        D_803EE788--;
    } else if (D_803EE6C0[0x99] != 0) {
        D_803EE788 = 1;
        if (func_802A5ED0() < 14) {
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x30D40, 1, 4, 1, 1, t7, s0, s2, s3, s4, 1);
        }
    }

    if (D_803F7840 == 0 && *(s16 *) (D_803EE6C0 + 0x76) != 0) {
        func_80260650(D_80367738, 10, NULL);
        io.a3 = 1;
        func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 4, 2, 1, t7, s0, s2, s3, s4, 1);
    }

    f = D_803EE780;
    near = func_802A7CB0(D_803EE6C0, 30);
    if (near == 0 && D_80370C23 != 0) {
        f -= D_8030D8D4;
        if (f < 0.0f) {
            f = 0.0f;
        }
    } else if (near == 0 && D_80370C1C != 0) {
        f += D_8030D8D8;
        if (!(f <= 1.0f)) {
            f = 1.0f;
        }
    } else if (f < 0.5f) {
        f += D_8030D8DC;
        if (!(f <= 0.5f)) {
            f = 0.5f;
        }
    } else {
        f -= D_8030D8DC;
        if (f < 0.5f) {
            f = 0.5f;
        }
    }
    D_803EE780 = f;
    func_802A0360(f, D_803EE3C0, 2, 0);

    speed = *(s16 *) (D_803EE6C0 + 0x76);
    if (speed != 0) {
        func_802A05F8((s32) D_802C2190, speed < 0);
        func_802A05F8((s32) D_802C21A4, speed < 0);
        v = (u32) (speed < 0 ? -speed : speed) >> 1;
        func_802A05D0((s32) D_802C2190, v);
        func_802A05D0((s32) D_802C21A4, v);
        func_802C4584((u32) v >> 2);
    } else {
        v = D_80370C2C;
        func_802A05F8((s32) D_802C2190, v < 0);
        func_802A05F8((s32) D_802C21A4, v >= 0);
        if (v < 0) {
            v = -v;
        }
        func_802A05D0((s32) D_802C2190, v);
        func_802A05D0((s32) D_802C21A4, v);
    }

    v = *(u16 *) (D_803EE6C0 + 0x4C);
    if (v >= 0x355 && v < 0x8AB) {
        v -= 0x355;
    } else {
        v = 0;
    }
    func_802A05A4(0.0f, (s32) D_802C21B8, (u32) v / 76);

    if (D_803EE789 != 0) {
        func_802A04BC(1, D_803EE3C0, ch);
        if (ch[0] != 1) {
            func_802A039C(D_803EE3C0, 1, 9);
            func_802A040C(D_803EE3C0, 1, 1);
            func_802A0290(D_803EE3C0, 1, 2);
            func_80260650(D_80367738, 1, NULL);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4EF8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE6C0[];  /* this vehicle's state block (the asm's $gp here) */
extern u32 D_803EE768[]; /* [0], [2]: trail x, z */

/* Tyre trail (shape of func_802B3C68, above): if the byte at +0x99 is set,
 * +0x98 isn't 1, +0x50 < 3 and +0x9B is clear, func_8027BE7C(3, y, 400,
 * -300, -400, -300, D_803EE768[0], D_803EE768[2], yaw, 5, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802B4EF8 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802B54EC(void) {
    if (D_803EE6C0[0x99] != 0 && D_803EE6C0[0x98] != 1 && D_803EE6C0[0x50] < 3 && D_803EE6C0[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803EE6C0 + 0x1C), 400, -300, -400, -300, D_803EE768[0], D_803EE768[2],
                      *(u16 *) (D_803EE6C0 + 0x4E), 5, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B54EC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* As func_802B3E40 for vehicle 4 (id 4, scale 0x2710). */
u8 *func_802B568C(MtxChainRegs *r) {
    return port_veh_parts(D_803EE6C0, VPOS4, 4, 0x2710, r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B568C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 22 when any of the bytes at +0x96/+0x97/+0x98 of D_803EE6C0 (the asm's
 * $gp) is 1, else 75. The asm returns it in s3 (see
 * tools_port/conventions.txt). Its asm caller func_802B49AC keeps a0-a3 live
 * (a mixed N64 build would need a thunk). */
s32 func_802B57C4(void) {
    if (D_803EE6C0[0x96] == 1 || D_803EE6C0[0x97] == 1 || D_803EE6C0[0x98] == 1) {
        return 22;
    }
    return 75;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B57C4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803EE784;

/* Vehicle-module setup leaf (shape of func_802AFBA0 in 69BB0, other
 * constants): D_803EBBF4 = D_803EBBF0 * 2, D_803ED3F6/7 = 110, 4, then
 * D_803EE784 = 2000 if B/Z is held, else 9000.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm callers keep
 * registers live across the call (func_802B48A4: t3, t4, f12, f14;
 * func_802B49AC: a0-a3); a mixed N64 build would need a thunk preserving
 * those, the native port does not. */
void func_802B5814(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 110;
    D_803ED3F7 = 4;
    D_803EE784 = (D_80370C23 == 0) ? 9000 : 2000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B5814.s")
#endif

/* func_802B589C: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE6C0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EE768[]; /* plus these three words */

/* Serialize this vehicle's state (D_803EE6C0[0..0xA5] plus the three words
 * D_803EE768[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B589C(u8 *dst) {
    return func_802AC7DC(dst, D_803EE6C0, D_803EE768);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B589C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B589C: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B58C8(void *)`. */
void func_802B58C8(void *src) {
    func_802AC85C(src, D_803EE6C0, D_803EE768);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B58C8.s")
#endif
