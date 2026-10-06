#include "common.h"
#include <ultra64.h>
#include "game/regs.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
#ifdef NON_MATCHING
/* Shared by the vehicle-14 setup / update functions below (C rewrites; same
 * code as vehicle 13's in 86F60.c with this vehicle's data and constants). */
typedef struct Unk8029DEA0Entry Unk8029DEA0Entry; /* channel table entry (56040.c) */


extern u8 D_803F9250[];  /* this vehicle's state block (the asm's $gp) */
extern u8 D_803F8F50[];  /* its channel table / save area */
extern u32 D_803F92F8[]; /* x, y, z */
extern u8 *D_803F9304;   /* model header */
extern u64 *D_803F9308;  /* save copy pair (func_802A7764) */
extern u64 *D_803F930C;
extern u8 D_8035805C;    /* which save copy is current */

void func_802CD068(void);
void func_802CD800(void);
void func_8029E558(u8 *base, u8 *other, Unk8029DEA0Entry *ch);                                 /* 56040 */
void func_802A133C(s32 a0Val, s32 id, s32 v0Val, s32 v1Val, u8 *obj);                          /* 5CB60 */
void func_802A8768(u8 *veh, s32 id, s32 *px, s32 *py, s32 *pz, s32 x, s32 z, s32 divB, s32 divA, s16 *angle,
                   u8 *flags, s16 *tbl, s32 *a, s32 *b, s32 *c, s32 *ys, Regs802A8768 *r, TriSideOut *f); /* 62740 */

#define VEH14_S16(off) (*(s16 *) (D_803F9250 + (off)))
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_80358070; /* bump allocator for the save copies */
extern u8 D_803F9312;
extern u8 D_803F9313;
extern u8 D_803F9314;
void func_802A1388(s32 a0Val, s32 a1Val, s32 v0Val, s32 v1Val, u8 *hdr);                            /* 5CB60 */
void func_802A754C(u8 *veh);                                                                     /* 62740 */
s32 *func_802A992C(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fpIn, u8 *veh,
                   TriSideOut *f);                                                               /* 62740 */
s32 func_8029F85C(u32 *bufA, u32 *bufB, Unk8029DEA0Entry *ch, u8 *hdr);                           /* 56040 */
void func_802A0290(Unk8029DEA0Entry *base, s32 idx, s32 val);                                    /* 56040 */
void func_802A0320(s32 idx, Unk8029DEA0Entry *base);                                             /* 56040 */
void func_802A039C(Unk8029DEA0Entry *base, s32 idx, s32 val);                                    /* 56040 */
void func_802A03D4(Unk8029DEA0Entry *base, s32 idx, s32 val);                                    /* 56040 */
void func_802A040C(Unk8029DEA0Entry *base, s32 idx, s32 val);                                    /* 56040 */
void func_802A0480(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val);                             /* 56040 */
void func_8029C354(s32 tag, u8 *p, u8 *end, u32 scale);                                          /* 56040 */
void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3);                                         /* 13A70 */
void func_802AA838(u8 *src, u8 *dst, s32 off);                                                   /* 62740 */

/* Set up vehicle 14 (the level-object dispatcher func_802A350C's case 0xE):
 * as func_802CB720 (86F60) with this vehicle's data (header D_803F9304, save
 * copies D_803F9308/930C from D_80358070, block D_803F9250, channels
 * D_803F8F50, position D_803F92F8..9300), id 0xE, wheel offsets
 * +-250/400 and +-320/500, its speed bands, scale 0x55F0, and only
 * D_803F9312..14 cleared (there is no phase byte). One update with +0x9A set
 * (func_802CD068), then the model matrix copy (func_802AA838).
 * Register convention as func_802CB720: hdr s2, x t7, y s3, z s0, heading
 * s1, fp, FP state f12-f26 in, f22-f26 out through f (conventions.txt). The
 * asm saves t0-t5 and leaves $gp = D_803F9250; fp/f20 (as func_8029E558
 * leaves them) and f12/f14 (as func_802CD068 leaves them), which its caller
 * hands to the next object's setup, aren't modelled. Asm caller
 * func_802A350C keeps t1, t2 live (a mixed N64 build would need a thunk). */
void func_802CC920(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f) {
    u32 p;
    u8 *h;
    u8 *copyA;
    u8 *copyB;

    D_803F9304 = hdr;
    p = D_80358070;
    D_803F9308 = (u64 *) p;
    D_803F930C = (u64 *) (p + 0x100);
    D_80358070 = p + 0x200;
    func_802A1388(0xE, 0, (s32) D_803F9308, (s32) D_803F930C, hdr);
    func_802A754C(D_803F9250);
    VEH14_S16(0x52) = 250;
    VEH14_S16(0x54) = 400;
    VEH14_S16(0x56) = -250;
    VEH14_S16(0x58) = 400;
    VEH14_S16(0x5A) = 250;
    VEH14_S16(0x5C) = -400;
    VEH14_S16(0x5E) = 320;
    VEH14_S16(0x60) = 500;
    VEH14_S16(0x62) = -320;
    VEH14_S16(0x64) = 500;
    VEH14_S16(0x66) = 320;
    VEH14_S16(0x68) = -500;
    D_803F92F8[0] = x;
    D_803F92F8[1] = y;
    D_803F92F8[2] = z;
    VEH14_S16(0x4C) = heading;
    VEH14_S16(0x4E) = heading;
    VEH14_S16(0x74) = heading;
    func_802A992C((s16 *) (D_803F9250 + 0x52), D_803F92F8[1], x, z, (s32 *) (D_803F9250 + 4),
                  (s32 *) &D_803F92F8[1], (s16 *) (D_803F9250 + 0x4C), 0xE, fp, D_803F9250, f);
    copyA = (u8 *) D_803F9308;
    copyB = (u8 *) D_803F930C;
    func_8029F85C((u32 *) copyB, (u32 *) copyA, (Unk8029DEA0Entry *) D_803F8F50, D_803F9304);
    func_802A039C((Unk8029DEA0Entry *) D_803F8F50, 0, 100);
    func_802A03D4((Unk8029DEA0Entry *) D_803F8F50, 0, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803F8F50, 0, 0);
    func_802A0480(0.0f, (Unk8029DEA0Entry *) D_803F8F50, 0, 0);
    func_802A0290((Unk8029DEA0Entry *) D_803F8F50, 0, 1);
    func_8029E558(copyA, copyB, (Unk8029DEA0Entry *) D_803F8F50);
    func_802A0320(0, (Unk8029DEA0Entry *) D_803F8F50);
    func_802A0290((Unk8029DEA0Entry *) D_803F8F50, 0, 1);
    func_8029E558(copyB, copyA, (Unk8029DEA0Entry *) D_803F8F50);
    VEH14_S16(0x78) = -180;
    VEH14_S16(0x7A) = 0;
    VEH14_S16(0x7C) = 8;
    VEH14_S16(0x7E) = 0;
    VEH14_S16(0x80) = 80;
    VEH14_S16(0x82) = 8;
    VEH14_S16(0x84) = 80;
    VEH14_S16(0x86) = 160;
    VEH14_S16(0x88) = 8;
    VEH14_S16(0x8A) = 160;
    VEH14_S16(0x8C) = 250;
    VEH14_S16(0x8E) = 8;
    VEH14_S16(0x90) = 250;
    VEH14_S16(0x92) = 330;
    VEH14_S16(0x94) = 4;
    D_803F9312 = 0;
    D_803F9313 = 0;
    D_803F9314 = 0;
    h = D_803F9304;
    func_8029C354(0xE, h + *(s32 *) (h + 4), h + *(s32 *) (h + 8), 0x55F0);
    func_80258230(0xE, 60, 25, 25);
    D_803F9250[0x9A] = 1;
    func_802CD068();
    D_803F9250[0x9A] = 0;
    h = D_803F9304;
    h += *(s32 *) (h + 0x18);
    func_802AA838((u8 *) D_803F930C, (u8 *) D_803F9308, *(s32 *) (h + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CC920.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F9250[]; /* this vehicle's state block */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

/* Enter vehicle type 14: clears the byte at +0x99, D_8036444C/50 = 3000,
 * 1000, then func_802C4310(arg0, 0x7B) (arg0 passes straight through; hd.c
 * calls this with no arguments and func_802C4310 ignores it). The asm also
 * points $gp at D_803F9250 and leaves it there (conventions.txt: clobbers
 * gp); C code doesn't use $gp. Same shape as func_802B76AC (72B80). */
void func_802CCC8C(s32 arg0) {
    D_803F9250[0x99] = 0;
    D_8036444C = 3000;
    D_80364450 = 1000;
    func_802C4310(arg0, 0x7B);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCC8C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 14
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1,
 * else 1 (shape shared with func_802CA140 in 853D0.c). 00000.c declares it
 * void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F9250[];

s32 func_802CCCD8(void) {
    if (D_803F9250[0x96] == 1 || D_803F9250[0x97] == 1 || D_803F9250[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCCD8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F9250[];
extern u64 *D_803F9308; /* save copy pair (func_802A7764) */
extern u64 *D_803F930C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 14 (called from hd.c): clears the speed (s16 at +0x76),
 * func_802A7764(D_803F9308, D_803F930C, 0x100), then stops the looping
 * sounds (func_802C444C). The asm points $gp at D_803F9250 around the calls
 * and restores it. */
void func_802CCD34(void) {
    *(s16 *) (D_803F9250 + 0x76) = 0;
    func_802A7764(D_803F9308, D_803F930C, 0x100);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCD34.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A9A60(s16 *tbl, s32 y, s32 x, s32 z, s32 *dst, s32 *mid, s16 *angle, s32 key, s32 fp, u8 *veh,
                  TriSideOut *f, Out802A9A60 *out); /* 62740 */

/* Re-seat vehicle 14 on the ground (hd.c calls it): as func_802CBC08
 * (86F60): wheel heights (func_802A9A60, key 0xE), model placement
 * (func_802CD800), func_802A133C(z, 0xE, x, y, block). The asm hands
 * func_802A9A60 its caller's fp (which reaches D_803ED3F2[0..2] and +0x50)
 * and FP registers; the C passes 0 (see the port notes: fidelity). */
void func_802CCD80(void) {
    TriSideOut f;
    Out802A9A60 out;

    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    func_802A9A60((s16 *) (D_803F9250 + 0x52), D_803F92F8[1], D_803F92F8[0], D_803F92F8[2],
                  (s32 *) (D_803F9250 + 4), (s32 *) &D_803F92F8[1], (s16 *) (D_803F9250 + 0x4C), 0xE, 0,
                  D_803F9250, &f, &out);
    func_802CD800();
    func_802A133C(D_803F92F8[2], 0xE, D_803F92F8[0], D_803F92F8[1], D_803F9250);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCD80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802ABD54(s32 id, s32 x, s32 y, s32 z, ZoneScanRegs *r);
extern u32 D_803F92F8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xE at its position
 * D_803F92F8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CD068 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CCE90(ZoneScanRegs *r) {
    return func_802ABD54(0xE, D_803F92F8[0], D_803F92F8[1], D_803F92F8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCE90.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r); /* 62740 */
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz); /* 62740 */

/* As func_802CBD5C (86F60) for vehicle 14 (D_803F9250, position
 * D_803F92F8..9300): value pairs on triangle `id` at the position -> +0x6A/
 * +0x6C, heading -> +0x6E, and at the position plus tbl[0] (+0x52) rotated by
 * the heading -> +0x70/+0x72; returns &D_803F9250[0x52] (the asm's v1).
 * Register convention: id in a3; func_802AAD0C's FP results pass through
 * r (conventions.txt). The asm saves t0, t1, t3, t4 (asm caller
 * func_802AB50C keeps t0 and t1 live), points $gp at D_803F9250 and leaves
 * s4 = $gp + 0x4C. */
s32 func_802CCED4(s32 id, InterpRegs *r) {
    s32 x = D_803F92F8[0];
    s32 z = D_803F92F8[2];
    s32 dx;
    s32 dz;

    func_802AAD0C(id, x, z, r);
    *(s16 *) (D_803F9250 + 0x6A) = r->t3;
    *(s16 *) (D_803F9250 + 0x6C) = r->t4;
    *(u16 *) (D_803F9250 + 0x6E) = *(u16 *) (D_803F9250 + 0x4C);
    dx = func_802A94A4(0, (s16 *) (D_803F9250 + 0x52), (s16 *) (D_803F9250 + 0x4C), &dz);
    func_802AAD0C(id, x + dx, z + dz, r);
    *(s16 *) (D_803F9250 + 0x70) = r->t3;
    *(s16 *) (D_803F9250 + 0x72) = r->t4;
    return (s32) (D_803F9250 + 0x52);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCED4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED40B;
s32 func_802AB9A4(s16 *tbl, s32 x1, u16 *angle, s32 id, s32 z1, s32 x2, s32 z2, s32 *s3Out, InterpRegs *r); /* 62740 */
void func_802AAE54(s32 id, s32 x, s32 z, InterpRegs *r);                                                 /* 62740 */
void func_802CD9AC(void);

/* Steer vehicle 14 along triangle `id` (asm caller func_802AB714): as
 * func_802CBDE8 (86F60) with this vehicle's block D_803F9250, position
 * D_803F92F8.., timers func_802CD9AC, ground-contact id 0xE and divisors
 * 800/500, model func_802CD800.
 * Register convention: id a3, f24 in; func_802A8768's s3 (return value) and
 * FP results (f) out (conventions.txt). The asm saves a3, t0-t2 (asm caller
 * func_802AB714 keeps a3, t0, t1 live) and leaves $gp = D_803F9250. */
s32 func_802CCF60(s32 id, TriSideOut *f) {
    InterpRegs r;
    Regs802A8768 rr;
    s32 heading;
    s32 dist;

    r.f24 = f->sideZ;
    heading = func_802AB9A4((s16 *) (D_803F9250 + 0x52), VEH14_S16(0x6A), (u16 *) (D_803F9250 + 0x6E), id,
                            VEH14_S16(0x6C), VEH14_S16(0x70), VEH14_S16(0x72), &dist, &r);
    VEH14_S16(0x4E) = heading;
    VEH14_S16(0x4C) = heading;
    func_802AAE54(id, VEH14_S16(0x6A), VEH14_S16(0x6C), &r);
    func_802CD9AC();
    D_803ED40B = 1;
    f->pz = r.f12;
    *(s32 *) &f->cross = r.f14;
    f->cz = r.f20;
    f->side = r.f22;
    f->sideZ = r.f24;
    f->dz = r.f26;
    rr.s3 = dist;
    func_802A8768(D_803F9250, 0xE, (s32 *) &D_803F92F8[0], (s32 *) &D_803F92F8[1], (s32 *) &D_803F92F8[2], r.t3,
                  r.t4, 800, 500, (s16 *) (D_803F9250 + 0x4C), D_803F9250 + 0x96, (s16 *) (D_803F9250 + 0x52),
                  (s32 *) (D_803F9250 + 0x28), (s32 *) (D_803F9250 + 0x40), (s32 *) (D_803F9250 + 0x34),
                  (s32 *) (D_803F9250 + 4), &rr, f);
    func_802CD800();
    func_802A133C(D_803F92F8[2], 0xE, D_803F92F8[0], D_803F92F8[1], D_803F9250);
    return rr.s3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CCF60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80367BFF;
extern s16 D_803F9310; /* steering target */
extern u8 D_803A7424;
extern u8 D_803A7425;
extern u8 *D_803F77D0;
extern f32 D_8030D9C0;
extern u8 D_80306440[];
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern u16 D_8036443E;
extern u16 D_80364440;
void func_802A75DC(u8 *veh, u8 *src, s32 *w0, s32 *w1, s32 *w2);                                /* 62740 */
void func_802C4724(s32 sfx);                                                                     /* 7F8B0 */
void func_802CD578(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
void func_802CB690(u8 *state);                                                                   /* 86ED0 */
s32 func_802CD938(void);
void func_802A7E70(s32 rate, u16 *angle);                                                        /* 62740 */
void func_802A785C(u8 *veh, s16 *speed, s32 mode, u8 *flags, s16 *bands, s32 delta);             /* 62740 */
void func_802A7FD8(u8 *veh, u16 *heading, s32 rate, s16 *speedp, u16 *target, u16 *out, s8 *flag,
                   s32 sound);                                                                   /* 62740 */
f32 func_802A83B8(s16 *div, u8 *f, s32 *p, f32 *out);                                            /* 62740 */
void func_802A843C(u8 *veh, s16 *speed, s32 kind, s8 *f, s32 *p, s32 clamp, f32 div);            /* 62740 */
void func_802A7070(u8 *veh, s16 *angle);                                                         /* 62740 */
s32 func_802A860C(f32 f, s32 angle, s16 *len, s32 *px, s32 *pz, Out802A860C *out);               /* 62740 */
void func_8029A800(s32 z, s32 a1, s32 b2, s32 b3, s32 x, s32 y, s32 b0, s32 h1, s32 h2, s32 b4, s32 b8,
                   u8 *veh);                                                                     /* 56040 */
void func_8029C52C(s32 tag, u8 *veh);                                                                     /* 56040 */
void func_8029AA10(s32 kind);                                                                    /* 56040 */
void func_802BE77C(s32 id, u8 *vehicle);                                                         /* 77E20 */
void func_8029A914(u8 *veh);                                                                        /* 56040 */
s32 func_802A6F6C(void);                                                                         /* 62740 */
u64 *func_802A768C(u8 *veh, u8 *dst, s32 *w0, s32 *w1, s32 *w2, u64 *src, u64 *dst2, s32 size,
                   u64 **dst2End);                                                               /* 62740 */
void func_802A70D8(u8 *veh);                                                                     /* 62740 */
s32 func_802A71DC(u8 *veh, s32 cur, s32 target, s32 *curOut, f32 scale);                         /* 62740 */
s32 func_802A746C(u8 *veh, s32 delta, s32 v1, s32 *targetOut);                                   /* 62740 */
void func_802A6FE4(u8 *veh, s32 limit);                                                          /* 62740 */

/* Per-frame update of vehicle 14 (hd.c, and once from func_802CC920): as
 * func_802CBEF0 (86F60) with this vehicle's data and constants: zone lookup
 * func_802CCE90, engine sound 0x54, effects func_802CD578 (it also takes the
 * zone scan's s4), timers func_802CD9AC, turn rate func_802CD938, speed band
 * delta 0x19 (countdown D_803F9314), acceleration rate 0x4E20, top speed
 * 800.0, reverse flag D_803F9313 / target D_803F9310, ground-contact
 * divisors 800/500, model func_802CD800, collision sphere table D_80306440,
 * steering scale D_8030D9C0, id 0xE.
 * Register notes as there: the zone scan registers and the FP state handed
 * to func_802A8768 are the caller's leftovers in the asm (0 here); f12/f14
 * and v1 are left as the callees leave them (not modelled). The asm saves
 * every callee-saved register. */
void func_802CD068(void) {
    ZoneScanRegs zr;
    TriSideOut f;
    Out802A860C o;
    Regs802A8768 rr;
    u64 *end;
    f32 scale;
    s32 x;
    s32 d;
    s32 v;
    s32 cur;
    s32 turn;
    s32 target;

    zr.t6 = 0;
    zr.t7 = 0;
    zr.s0 = 0;
    zr.s1 = 0;
    zr.s2 = 0;
    zr.s3 = 0;
    zr.s4 = 0;
    func_802CCE90(&zr);
    func_802A75DC(D_803F9250, D_803F8F50, (s32 *) &D_803F92F8[0], (s32 *) &D_803F92F8[1], (s32 *) &D_803F92F8[2]);
    func_802C4724(0x54);
    if ((s8) D_803F9250[0x9A] == 0) {
        func_802CD578(zr.t6, zr.t7, zr.s0, zr.s1, zr.s2, zr.s3, zr.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(D_803F9250);
    }
    func_802CD9AC();
    func_802A7E70(func_802CD938(), (u16 *) (D_803F9250 + 0x4C));
    if (D_803F9314 == 0) {
        func_802A785C(D_803F9250, (s16 *) (D_803F9250 + 0x76), 3, D_803F9250 + 0x96, (s16 *) (D_803F9250 + 0x78),
                      0x19);
    } else {
        D_803F9314--;
    }
    func_802A7FD8(D_803F9250, (u16 *) (D_803F9250 + 0x74), 0x4E20, (s16 *) (D_803F9250 + 0x76),
                  (u16 *) (D_803F9250 + 0x4C), (u16 *) (D_803F9250 + 0x4E), (s8 *) (D_803F9250 + 0x99), 1);
    scale = func_802A83B8((s16 *) (D_803F9250 + 0x76), D_803F9250 + 0x96, (s32 *) (D_803F9250 + 4),
                          (f32 *) D_803F9250);
    func_802A843C(D_803F9250, (s16 *) (D_803F9250 + 0x76), 0xE, (s8 *) (D_803F9250 + 0x96),
                  (s32 *) (D_803F9250 + 4), 1, 800.0f);
    if ((s8) D_803F9313 != 0) {
        func_802A7070(D_803F9250, &D_803F9310);
    }
    x = func_802A860C(scale, *(u16 *) (D_803F9250 + 0x4E), (s16 *) (D_803F9250 + 0x76), (s32 *) &D_803F92F8[0],
                      (s32 *) &D_803F92F8[2], &o);
    D_803ED40B = 1;
    f.pz = 0.0f;
    f.cross = 0.0f;
    f.cz = 0.0f;
    f.side = 0.0f;
    f.sideZ = 0.0f;
    f.dz = 0.0f;
    rr.s3 = o.s3;
    func_802A8768(D_803F9250, 0xE, (s32 *) &D_803F92F8[0], (s32 *) &D_803F92F8[1], (s32 *) &D_803F92F8[2], x, o.t1,
                  800, 500, (s16 *) (D_803F9250 + 0x4C), D_803F9250 + 0x96, (s16 *) (D_803F9250 + 0x52),
                  (s32 *) (D_803F9250 + 0x28), (s32 *) (D_803F9250 + 0x40), (s32 *) (D_803F9250 + 0x34),
                  (s32 *) (D_803F9250 + 4), &rr, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803F9308, (u8 *) D_803F930C, (Unk8029DEA0Entry *) D_803F8F50);
    } else {
        func_8029E558((u8 *) D_803F930C, (u8 *) D_803F9308, (Unk8029DEA0Entry *) D_803F8F50);
    }
    func_802CD800();
    func_8029A800(D_803F92F8[2], (s32) D_80306440, 1, 1, D_803F92F8[0], D_803F92F8[1], 7, VEH14_S16(0x76), 0x96, 0,
                  0xE, D_803F9250);
    func_8029C52C(0xE, D_803F9250);
    func_8029AA10(0xE);
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803F8F50;
        func_802BE77C(0xE, D_803F9250);
        if (D_803A7424 == 0) {
            D_803F9313 = 0;
            goto publish;
        }
        D_803F9313 = 0;
        if (D_8035805C != 0) {
            func_802A768C(D_803F9250, D_803F8F50, (s32 *) &D_803F92F8[0], (s32 *) &D_803F92F8[1],
                          (s32 *) &D_803F92F8[2], D_803F930C, D_803F9308, 0x100, &end);
        } else {
            func_802A768C(D_803F9250, D_803F8F50, (s32 *) &D_803F92F8[0], (s32 *) &D_803F92F8[1],
                          (s32 *) &D_803F92F8[2], D_803F9308, D_803F930C, 0x100, &end);
        }
        D_803F9314 = 5;
        v = VEH14_S16(0x76);
        if (v >= 0) {
            if (v < 50) {
                v = 50;
            }
        } else if (!(v < -49)) {
            v = -50;
        }
        VEH14_S16(0x76) = -v >> 1;
        func_802CD800();
        goto publish;
    }
    func_8029A914(D_803F9250);
    D_803F9313 = 1;
    d = *(u16 *) (D_803F9250 + 0x4E) - 0x800;
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
    func_802A70D8(D_803F9250);
    turn = func_802A71DC(D_803F9250, *(u16 *) (D_803F9250 + 0x4E), *(u16 *) (D_803F9250 + 0x4C), &cur, D_8030D9C0);
    D_803F9310 = cur;
    VEH14_S16(0x4E) = cur;
    VEH14_S16(0x74) = cur;
    func_802A746C(D_803F9250, turn, d, &target);
    func_802A6FE4(D_803F9250, 0);
    D_803F77D0 = D_803F8F50;
    func_802BE77C(0xE, D_803F9250);
publish:
    D_803643E0 = D_803F92F8[0];
    D_803643E4 = D_803F92F8[1];
    D_803643E8 = D_803F92F8[2];
    D_8036443C = VEH14_S16(0x76);
    D_8036443E = *(u16 *) (D_803F9250 + 0x4E);
    D_80364440 = *(u16 *) (D_803F9250 + 0x4C);
    func_802A133C(D_803F92F8[2], 0xE, D_803F92F8[0], D_803F92F8[1], D_803F9250);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD068.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
extern u8 D_803F9312;   /* countdown */
extern u8 D_802C2954[]; /* definition handed to func_802A6274 (7D9D0 text blob) */
s32 func_802A5ED0(void);
void func_802C4584(s32 level);
void func_802CD660(void);

/* Tyre trail (func_802CD660), then the countdown D_803F9312: when nonzero
 * it just counts down; at zero, if the byte at +0x99 is set, it restarts at
 * 1 and, when fewer than 15 D_803C4B70 records are active (func_802A5ED0),
 * two func_802A6274 records are set up (def D_802C2954, data 0x29810, tag 1,
 * type 1 at (0xE, 1, 1) and (0xE, 2, 1)). Then always func_802C4584(|speed|
 * >> 5) (speed = s16 at +0x76).
 * Register convention: the asm passes t6, t7, s0-s4 through to
 * func_802A6274 (t6 and s1 in its in/out block); $gp (= D_803F9250) is read
 * as the global. It leaves func_802A6274's s1 and changes s5-s7
 * (conventions.txt: clobbers). */
void func_802CD578(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 v;

    func_802CD660();
    if (D_803F9312 != 0) {
        D_803F9312--;
    } else if (D_803F9250[0x99] != 0) {
        D_803F9312 = 1;
        if (func_802A5ED0() < 15) {
            io.a3 = 1;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 0xE, 1, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 0xE, 2, 1, t7, s0, s2, s3, s4, 1);
        }
    }
    v = *(s16 *) (D_803F9250 + 0x76);
    if (v < 0) {
        v = -v;
    }
    func_802C4584((u32) v >> 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD578.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F92F8[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803F9250): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803F92F8[0], D_803F92F8[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CD578 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802CD660(void) {
    if (D_803F9250[0x99] != 0 && D_803F9250[0x98] != 1 && D_803F9250[0x50] < 3 && D_803F9250[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F9250 + 0x1C), 250, -400, -400, -400, D_803F92F8[0], D_803F92F8[2],
                      *(u16 *) (D_803F9250 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD660.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8035805C;     /* which of the two save copies is current */
extern u8 *D_803F9304;    /* model header: word offsets to the part lists */
extern s16 D_803ED390[3]; /* rotation angles x, y, z for func_802AA764 */
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);                                     /* 62740 */
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs); /* 56040 */
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);                     /* 62740 */

/* Place vehicle 14's model: as func_802CC70C (86F60) with this vehicle's
 * header D_803F9304, save copies D_803F9308/930C, heading at +0x4C of
 * D_803F9250, position D_803F92F8..9300, scale 0x55F0 and id 0xE.
 * Register notes as there: the asm's $gp (= D_803F9250) is read as the
 * global; the a3/s0/s1 leftovers handed to func_8029C454 / func_802ABBEC
 * (only used by a point entry with no matrices) are 0 here; its s1, s4,
 * f12, f14 results (survey: read by func_802CD068) aren't modelled.
 * Clobbers s1, s2, s4-s7 (conventions.txt). */
void func_802CD800(void) {
    MtxChainRegs regs;
    u8 *hdr = D_803F9304;
    u8 *base;
    s32 *m;

    m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) +
                 (s32) (D_8035805C ? (u8 *) D_803F9308 : (u8 *) D_803F930C));
    D_803ED390[1] = *(u16 *) (D_803F9250 + 0x4C);
    func_802AA764(D_803F92F8[0], D_803F92F8[1], D_803F92F8[2], 0x55F0, m);
    base = D_8035805C ? (u8 *) D_803F9308 : (u8 *) D_803F930C;
    hdr = D_803F9304;
    regs.v1 = 0;
    regs.a0 = 0;
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    func_8029C454(D_803F92F8[0], D_803F92F8[1], D_803F92F8[2], 0xE, hdr + *(s32 *) (hdr + 4),
                  hdr + *(s32 *) (hdr + 8), base, &regs);
    hdr = D_803F9304;
    regs.v1 = D_803F92F8[1];
    regs.a0 = D_803F92F8[2];
    func_802ABBEC(0xE, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), base, &regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD800.s")
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

extern f32 D_8030D9C4;

/* Speed (s16 at +0x76 of D_803F9250, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9C4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CD068 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CD938(void) {
    f32 div;
    s32 r;

    if (D_803F9250[0x96] == 1 || D_803F9250[0x97] == 1 || D_803F9250[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D9C4;
    }
    CVT_W_S(r, *(s16 *) (D_803F9250 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD938.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CCF60 on t3, t4, f12, f14; func_802CD068 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CD9AC(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/88160/func_802CD9AC.s")
#endif
