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
/* Port-phase rewrites (functional, not matching): the vehicles of types
 * 0x0B, 0x11 and 0x12 run identical asm (three copies differing only in
 * their globals and type id), so the C keeps one implementation driven by a
 * per-vehicle descriptor (sVeh83910) and the asm-named entry points pick the
 * descriptor. The asm points $gp at the vehicle's block; it is a fixed
 * global per copy, so the C takes it from the descriptor. */
#define VEH_U8(v, off) (*(u8 *) ((u8 *) (v) + (off)))
#define VEH_S8(v, off) (*(s8 *) ((u8 *) (v) + (off)))
#define VEH_U16(v, off) (*(u16 *) ((u8 *) (v) + (off)))
#define VEH_S16(v, off) (*(s16 *) ((u8 *) (v) + (off)))
#define VEH_S32(v, off) (*(s32 *) ((u8 *) (v) + (off)))
#define VEH_F32(v, off) (*(f32 *) ((u8 *) (v) + (off)))
/* obj + the word at obj + off (the model header's self-relative offsets) */
#define OBJ_PTR(obj, off) ((u8 *) (obj) + *(s32 *) ((u8 *) (obj) + (off)))

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

typedef struct {
    u8 *veh;             /* the vehicle block (asm $gp) */
    u8 *chan;            /* animation channel table */
    s32 *pos;            /* x, y, z */
    u8 **data;           /* model data */
    u64 **bufA;          /* save-buffer pair (func_802A7764 / func_802AA838) */
    u64 **bufB;
    u8 *flag;            /* set while a hit has been undone */
    s32 type;            /* vehicle type / part tag */
    void (*model)(void); /* this copy's model update */
} VehDesc83910;

extern u8 D_803F8550[];
extern u8 D_803F85F8[];
extern u8 D_803F86A0[];
extern u8 D_803F7C50[];
extern u8 D_803F7F50[];
extern u8 D_803F8250[];
extern s32 D_803F8748[]; /* x, y, z (D_803F8748, D_803F874C, D_803F8750) */
extern s32 D_803F8754[];
extern s32 D_803F8760[];
extern u8 *D_803F876C;
extern u8 *D_803F8770;
extern u8 *D_803F8774;
extern u64 *D_803F8778;
extern u64 *D_803F877C;
extern u64 *D_803F8780;
extern u64 *D_803F8784;
extern u64 *D_803F8788;
extern u64 *D_803F878C;
extern u8 D_803F8790;
extern u8 D_803F8791;
extern u8 D_803F8792;
extern u8 *D_80358070;   /* heap pointer */
extern u8 D_8035805C;    /* which save buffer is current */
extern s16 D_803ED390[]; /* (0, heading, 0) for func_802AA764 */
extern u8 D_803ED40B;
extern u8 D_80306410[];
extern u8 *D_803F77D0;
extern u8 D_803A7424;
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
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2);
void func_802C4724(s32 sfx);
void func_802C4584(s32 level);
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
u64 *func_802A768C(u8 *veh, u8 *dst, s32 *w0, s32 *w1, s32 *w2, u64 *src, u64 *dst2, s32 size,
                   u64 **dst2End);
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs);
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);
s32 func_802AABE4(s32 id, u16 *desc, u8 *base, MtxChainRegs *regs, s16 **vertsOut);
void func_8029D040(s32 val, void *tbl, s32 x, s32 z, s32 id, u8 *model, u8 *mtxBase);
void func_802C8150(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp);
void func_802C8470(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp);
void func_802C8790(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp);
void func_802C8BB8(s32 type);
void func_802C8C90(void);
void func_802C8FA8(void);
void func_802C92C0(void);
void func_802C9624(void);
void func_802C97C0(void);
void func_802C995C(void);
void func_802C95D8(u8 *state);
void func_802C9B30(void);

static const VehDesc83910 sVeh83910[3] = {
    { D_803F8550, D_803F7C50, D_803F8748, &D_803F876C, &D_803F8778, &D_803F877C, &D_803F8790, 0x0B,
      func_802C9624 },
    { D_803F85F8, D_803F7F50, D_803F8754, &D_803F8770, &D_803F8780, &D_803F8784, &D_803F8791, 0x11,
      func_802C97C0 },
    { D_803F86A0, D_803F8250, D_803F8760, &D_803F8774, &D_803F8788, &D_803F878C, &D_803F8792, 0x12,
      func_802C995C },
};

/* Model update (func_802C9624 / 97C0 / 995C): D_803ED390 = (0, heading, 0)
 * and func_802AA764(x, y, z, 0x88B8, m) with m = the current save buffer +
 * the word at (model + header word 0x18) + 4; then the part positions
 * (func_8029C454, model sections 1..2), the matrix-chain records
 * (func_802ABBEC, sections 0..1), the triangles (func_802AABE4, section 2)
 * and the channel models (func_8029D040, section 3), with the current save
 * buffer as matrix base.
 * Fidelity: the asm hands func_802AA890's extra registers through these
 * calls as its caller left them (a3, s1, s0; s2 as func_802AA764 leaves it;
 * read only for a part whose matrix chain is empty). The C starts them at 0
 * (v1 / a0 = y / z, which the asm's callees preserve). */
static void veh83910_model(const VehDesc83910 *d) {
    u8 *veh = d->veh;
    s32 *pos = d->pos;
    u8 *data = *d->data;
    u8 *base;
    s16 *verts;
    MtxChainRegs regs;

    base = (u8 *) (D_8035805C != 0 ? *d->bufA : *d->bufB);
    base += *(s32 *) (OBJ_PTR(data, 0x18) + 4);
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    D_803ED390[1] = VEH_U16(veh, 0x4C);
    func_802AA764(pos[0], pos[1], pos[2], 0x88B8, (s32 *) base);

    base = (u8 *) (D_8035805C != 0 ? *d->bufA : *d->bufB);
    regs.v1 = pos[1];
    regs.a0 = pos[2];
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = 0;
    regs.s0 = 0;
    data = *d->data;
    func_8029C454(pos[0], pos[1], pos[2], d->type, OBJ_PTR(data, 4), OBJ_PTR(data, 8), base, &regs);
    /* the asm's v1 / a0 (y / z) survive func_8029C454 */
    regs.v1 = pos[1];
    regs.a0 = pos[2];
    data = *d->data;
    func_802ABBEC(d->type, (s16 *) OBJ_PTR(data, 0), (s16 *) OBJ_PTR(data, 4), base, &regs);
    data = *d->data;
    func_802AABE4(d->type, (u16 *) OBJ_PTR(data, 8), base, &regs, &verts);
    data = *d->data;
    func_8029D040(VEH_U16(veh, 0x4C), d->chan, pos[0], pos[2], d->type, OBJ_PTR(data, 0xC), base);
}

/* Per-frame update (func_802C8C90 / 8FA8 / 92C0, through func_802C8BB8):
 * save the vehicle (func_802A75DC), engine sound 0x71 (func_802C4724),
 * timers (func_802C9B30), engine level |speed| >> 5 (func_802C4584);
 * throttle (func_802A785C), steering (func_802A7FD8), slope ratio
 * (func_802A83B8) and drag (func_802A843C), move along the heading
 * (func_802A860C), ground contact (func_802A8768); animation channels
 * (func_8029E558 between the two save buffers), model update, collisions
 * (func_8029A800, func_8029C52C, func_8029AA10, func_802BE77C). If those
 * reported a hit (D_803A7424): the first time (flag clear) restore the saved
 * state (func_802A768C), bounce (func_802C95D8), set the flag, update the
 * model again and stop; otherwise (or with no hit, which clears the flag)
 * publish position / speed / headings to D_803643E0.. and func_802A133C.
 * Fidelity: func_802A8768's FP inputs (the asm's f12-f26 at that point:
 * func_802A860C's sine leftovers and the caller's f20-f26) start at 0 here;
 * they only pass through to its FP outputs, which nothing here reads. */
static void veh83910_frame(const VehDesc83910 *d) {
    u8 *veh = d->veh;
    s32 *pos = d->pos;
    s32 s;
    s32 x;
    f32 f;
    Out802A860C o;
    Regs802A8768 r;
    TriSideOut tri;
    u64 *src;
    u64 *dst;
    u64 *end;

    func_802A75DC(veh, d->chan, &pos[0], &pos[1], &pos[2]);
    func_802C4724(0x71);
    func_802C9B30();
    s = VEH_S16(veh, 0x76);
    if (s < 0) {
        s = -s;
    }
    func_802C4584((u32) s >> 5);
    func_802A785C(veh, &VEH_S16(veh, 0x76), 3, &VEH_U8(veh, 0x96), &VEH_S16(veh, 0x78), 6);
    func_802A7FD8(veh, &VEH_U16(veh, 0x74), 0x2328, &VEH_S16(veh, 0x76), &VEH_U16(veh, 0x4C),
                  &VEH_U16(veh, 0x4E), &VEH_S8(veh, 0x99), 0);
    f = func_802A83B8(&VEH_S16(veh, 0x76), &VEH_U8(veh, 0x96), &VEH_S32(veh, 4), &VEH_F32(veh, 0));
    func_802A843C(veh, &VEH_S16(veh, 0x76), d->type, &VEH_S8(veh, 0x96), &VEH_S32(veh, 4), 1, 160.0f);
    x = func_802A860C(f, VEH_U16(veh, 0x4E), &VEH_S16(veh, 0x76), &pos[0], &pos[2], &o);
    D_803ED40B = 0;
    r.s3 = o.s3;
    tri.pz = 0.0f;
    tri.cross = 0.0f;
    tri.cz = 0.0f;
    tri.side = 0.0f;
    tri.sideZ = 0.0f;
    tri.dz = 0.0f;
    func_802A8768(veh, d->type, &pos[0], &pos[1], &pos[2], x, o.t1, 0xA0, 0xA0, &VEH_S16(veh, 0x4C),
                  &VEH_U8(veh, 0x96), &VEH_S16(veh, 0x52), &VEH_S32(veh, 0x28), &VEH_S32(veh, 0x40),
                  &VEH_S32(veh, 0x34), &VEH_S32(veh, 4), &r, &tri);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) *d->bufA, (u8 *) *d->bufB, d->chan);
    } else {
        func_8029E558((u8 *) *d->bufB, (u8 *) *d->bufA, d->chan);
    }
    d->model();
    /* b0 / h2 are the model update's leftover t0 (x) and t2 (the type) */
    func_8029A800(pos[2], (s32) D_80306410, 0, 0, pos[0], pos[1], pos[0], VEH_S16(veh, 0x76), d->type, 0,
                  d->type, veh);
    func_8029C52C(d->type, veh);
    func_8029AA10(d->type);
    D_803F77D0 = d->chan;
    func_802BE77C(d->type, veh);
    if (D_803A7424 != 0) {
        if (*d->flag == 0) {
            if (D_8035805C != 0) {
                src = *d->bufB;
                dst = *d->bufA;
            } else {
                src = *d->bufA;
                dst = *d->bufB;
            }
            func_802A768C(veh, d->chan, &pos[0], &pos[1], &pos[2], src, dst, 0x800, &end);
            func_802C95D8(veh);
            *d->flag = 1;
            d->model();
            return;
        }
    } else {
        *d->flag = 0;
    }
    D_803643E0 = pos[0];
    D_803643E4 = pos[1];
    D_803643E8 = pos[2];
    D_8036443C = VEH_S16(veh, 0x76);
    D_8036443E = VEH_U16(veh, 0x4E);
    D_80364440 = VEH_U16(veh, 0x4C);
    func_802A133C(pos[2], d->type, pos[0], pos[1], veh);
}

/* Spawn (func_802C8150 / 8470 / 8790, through func_802C80D0): model data
 * `data`, two 0x800-byte save buffers from the heap D_80358070, model record
 * (func_802A1388), reset the block (func_802A754C) and its wheel offsets
 * (+0x52..+0x68) and gear bands (+0x78..+0x94), position and headings,
 * ground (func_802A992C), animation channel 0 (func_8029F85C, the setters,
 * func_8029E558 both ways), parts (func_8029C354), func_80258230, then one
 * frame (func_802C8BB8) with byte +0x9A set, and the buffer copy
 * (func_802AA838). The asm's addi on the heap pointer traps on overflow.
 * Fidelity: func_802A992C's FP inputs (the dispatcher's f12-f26) start at 0;
 * they only pass through to its FP outputs. */
static void veh83910_init(const VehDesc83910 *d, s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    u8 *veh = d->veh;
    s32 *pos = d->pos;
    u8 *heap;
    u64 *a;
    u64 *b;
    TriSideOut tri;

    *d->data = data;
    heap = D_80358070;
    *d->bufA = (u64 *) heap;
    *d->bufB = (u64 *) (heap + 0x800);
    D_80358070 = heap + 0x1000;
    func_802A1388(d->type, 0, (s32) *d->bufA, (s32) *d->bufB, data);
    *d->flag = 0;
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
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    VEH_S16(veh, 0x4C) = angle;
    VEH_S16(veh, 0x4E) = angle;
    VEH_S16(veh, 0x74) = angle;
    tri.pz = 0.0f;
    tri.cross = 0.0f;
    tri.cz = 0.0f;
    tri.side = 0.0f;
    tri.sideZ = 0.0f;
    tri.dz = 0.0f;
    func_802A992C(&VEH_S16(veh, 0x52), pos[1], x, z, &VEH_S32(veh, 4), &pos[1], &VEH_S16(veh, 0x4C), d->type,
                  fp, veh, &tri);
    a = *d->bufA;
    b = *d->bufB;
    func_8029F85C((u32 *) b, (u32 *) a, d->chan, *d->data);
    func_802A039C(d->chan, 0, 0x64);
    func_802A03D4(d->chan, 0, 0);
    func_802A040C(d->chan, 0, 0);
    func_802A0480(0.0f, d->chan, 0, 0);
    func_802A0290(d->chan, 0, 1);
    func_8029E558((u8 *) a, (u8 *) b, d->chan);
    func_802A0320(0, d->chan);
    func_802A0290(d->chan, 0, 1);
    func_8029E558((u8 *) b, (u8 *) a, d->chan);
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
    data = *d->data;
    func_8029C354(d->type, OBJ_PTR(data, 4), OBJ_PTR(data, 8), 0x88B8);
    func_80258230(d->type, 0x96, 0x2D, 0x2D);
    VEH_U8(veh, 0x9A) = 1;
    func_802C8BB8(d->type);
    VEH_U8(veh, 0x9A) = 0;
    data = *d->data;
    func_802AA838((u8 *) *d->bufB, (u8 *) *d->bufA, *(s32 *) (OBJ_PTR(data, 0x18) + 4));
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn dispatcher for vehicle types 0x0B / 0x11 / other (0x12), called
 * from func_802A350C's object loop with the type in t3 and the spawn record
 * in t7 (x), s3 (y), s0 (z), s1 (heading), s2 (model data) and fp (the
 * ground-scan byte the previous object left; see func_802A992C). The asm
 * saves t0-t5: asm caller func_802A350C keeps t1 and t2 live (a mixed N64
 * build would need a thunk). It leaves s2-s4, fp and f12-f26 as the spawn
 * leaves them, and func_802A350C hands fp and f12-f26 on to the next
 * object's spawn (not modelled; see the fidelity notes). */
void func_802C80D0(s32 type, s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    if (type == 0x0B) {
        func_802C8150(x, y, z, angle, data, fp);
    } else if (type == 0x11) {
        func_802C8470(x, y, z, angle, data, fp);
    } else {
        func_802C8790(x, y, z, angle, data, fp);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C80D0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 0x0B (see veh83910_init); register inputs as
 * func_802C80D0's (conventions.txt). */
void func_802C8150(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    veh83910_init(&sVeh83910[0], x, y, z, angle, data, fp);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8150.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 0x11 (see veh83910_init). */
void func_802C8470(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    veh83910_init(&sVeh83910[1], x, y, z, angle, data, fp);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8470.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Spawn vehicle type 0x12 (see veh83910_init). */
void func_802C8790(s32 x, s32 y, s32 z, s32 angle, u8 *data, s32 fp) {
    veh83910_init(&sVeh83910[2], x, y, z, angle, data, fp);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8790.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): a0 passes straight
 * through to func_802C4310 with a1 = 0x72. */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

void func_802C8AB0(s32 arg0) {
    D_8036444C = 3000;
    D_80364450 = 0;
    func_802C4310(arg0, 0x72);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8AB0.s")
#endif

/* func_802C8AF0: same dead-$ra-frame-around-`return 1;` as func_802BBE10
 * in 772A0.c - confirmed hand-written, not compiler-reachable. See that
 * file's comment. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified): always returns 1
 * (vehicle-type 11/17/18 "can exit" check). 00000.c declares it void and
 * ignores the result, but the asm returns 1 in v0. */
s32 func_802C8AF0(void) {
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8AF0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803F8778; /* save copy pairs (func_802A7764), one per vehicle */
extern u64 *D_803F877C;
extern u64 *D_803F8780;
extern u64 *D_803F8784;
extern u64 *D_803F8788;
extern u64 *D_803F878C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 0x11 / 0x12 / other (11) (called from hd.c with the
 * type): func_802A7764(pair for that type, 0x800), then stops the looping
 * sounds (func_802C444C). The asm points $gp at that vehicle's block
 * (D_803F85F8 / D_803F86A0 / D_803F8550) around the calls and restores it.
 * The asm compares the whole register; 00000.c declares the parameter u8. */
void func_802C8B0C(s32 type) {
    if (type == 0x11) {
        func_802A7764(D_803F8780, D_803F8784, 0x800);
    } else if (type == 0x12) {
        func_802A7764(D_803F8788, D_803F878C, 0x800);
    } else {
        func_802A7764(D_803F8778, D_803F877C, 0x800);
    }
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8B0C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update dispatcher (hd.c's vehicle switch, and the spawns): type
 * 0x0B -> func_802C8C90, 0x11 -> func_802C8FA8, else func_802C92C0. The asm
 * saves s0-s7, gp, fp and f20-f30 around the call; it compares the whole
 * a0 (00000.c declares the parameter u8). It leaves v1, f12 and f14 as the
 * update leaves them (the survey lists them as read by its callers; nothing
 * there uses them). */
void func_802C8BB8(s32 type) {
    if (type == 0x0B) {
        func_802C8C90();
    } else if (type == 0x11) {
        func_802C8FA8();
    } else {
        func_802C92C0();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8BB8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 0x0B (see veh83910_frame). The asm
 * points $gp at D_803F8550 and leaves it there, and leaves s0-s7, fp and
 * f20-f30 as its callees do (conventions.txt: clobbers). */
void func_802C8C90(void) {
    veh83910_frame(&sVeh83910[0]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8C90.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 0x11 (see veh83910_frame). */
void func_802C8FA8(void) {
    veh83910_frame(&sVeh83910[1]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C8FA8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of vehicle type 0x12 (see veh83910_frame). */
void func_802C92C0(void) {
    veh83910_frame(&sVeh83910[2]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C92C0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Bounce: the speed (s16 at +0x76 of the vehicle block, which the asm takes
 * in $gp: three vehicles share this, see tools_port/conventions.txt) is
 * raised to a magnitude of at least 80 keeping its sign (0 counts as
 * positive), then negated and halved (arithmetic shift). Asm callers
 * func_802C8C90, func_802C8FA8 and func_802C92C0 keep a0-a3, f12 and f14
 * live across the call (a mixed N64 build would need a thunk). */
void func_802C95D8(u8 *state) {
    s32 v = *(s16 *) (state + 0x76);

    if (v >= 0) {
        if (v < 0x50) {
            v = 0x50;
        }
    } else if (v >= -0x4F) {
        v = -0x50;
    }
    *(s16 *) (state + 0x76) = -v >> 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C95D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Model update of vehicle type 0x0B (see veh83910_model). The asm reads
 * the block through $gp (always D_803F8550 here). It leaves t0 = x and
 * t2 = 0x0B for its caller's func_8029A800 call (the C caller passes them
 * itself) and s0-s7, fp, f20-f28 as its callees do (conventions.txt). */
void func_802C9624(void) {
    veh83910_model(&sVeh83910[0]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9624.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Model update of vehicle type 0x11 (see veh83910_model). */
void func_802C97C0(void) {
    veh83910_model(&sVeh83910[1]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C97C0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Model update of vehicle type 0x12 (see veh83910_model). */
void func_802C995C(void) {
    veh83910_model(&sVeh83910[2]);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C995C.s")
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

extern f32 D_8030D980;

/* Speed (s16 at +0x76 of the vehicle block) / D_8030D980, rounded to
 * nearest. The asm takes the block in $gp and returns the value in s3 (see
 * tools_port/conventions.txt); nothing calls it directly, so which of this
 * file's three blocks it gets is unknown, hence the parameter. */
s32 func_802C9AF8(u8 *state) {
    s32 r;

    CVT_W_S(r, *(s16 *) (state + 0x76) / D_8030D980);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9AF8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers func_802C8C90, func_802C8FA8
 * and func_802C92C0 rely on a0-a3, f12, f14 being preserved (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802C9B30(void) {
    D_803EBBF4 = D_803EBBF0 * 2.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/83910/func_802C9B30.s")
#endif
