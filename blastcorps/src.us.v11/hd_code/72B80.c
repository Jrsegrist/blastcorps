#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#ifdef NON_MATCHING
#define D_803ED808 (*(s32 *) D_803ED808)
#endif
/* end of views */

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Register blocks of the 62740.c helpers these vehicle functions call (the
 * asm passes them in registers; the C rewrites take pointers). */
extern u8 D_803EEE70[];  /* vehicle 8's state block (the asm's $gp) */
extern u32 D_803EEF18[]; /* vehicle 8's position x, y, z */
extern u8 *D_803EEF24;   /* vehicle 8's model header */
extern u64 *D_803EEF28;  /* vehicle 8's model buffers */
extern u64 *D_803EEF2C;
void func_802B8278(void);
void func_802B8424(void);
void func_802B9B4C(void);
void func_802B8D04(void);
f32 func_802B98E0(void);
void func_802B8C18(s32 t3, s32 fp, TriSideOut *f);
extern u8 D_803EEF40[]; /* the flying vehicle's animation channel table (Unk8029DEA0Entry, 56040.c) */
extern u8 D_803EF240[]; /* the flying vehicle's state block */
extern u8 *D_803EF2F8;  /* its model header */
extern u8 *D_803EF2FC;  /* its model buffers */
extern u8 *D_803EF300;
extern s32 D_803EF31C;
extern s16 D_803EF2E6;
extern s16 D_803EF324;
extern u8 D_803EEB70[]; /* vehicle 8's 0x300-byte animation channel table (also saved/restored) */
extern s16 D_803EEF30;  /* vehicle 8's last ring-steered heading */
extern s8 D_803EEF33;   /* ring steering active */
extern u8 D_803EEF34;   /* throttle lockout frames */
extern u8 D_80305D30[];
extern f32 D_8030D900;
s32 func_802B78B0(ZoneScanRegs *r);
void func_802B7F98(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4);
s32 func_802B83B0(void);

#define VEH8_S16(off) (*(s16 *) (D_803EEE70 + (off)))
#define VEH8_U16(off) (*(u16 *) (D_803EEE70 + (off)))

/* InterpRegs (func_802AAD0C/func_802AAE54 results) -> TriSideOut (the FP
 * registers func_802A8768 and the triangle scans pass through). */
#define INTERP_TO_TRISIDE(f, r)              \
    do {                                     \
        (f).pz = (r).f12;                    \
        (f).cross = *(f32 *) &(r).f14;       \
        (f).cz = (r).f20;                    \
        (f).side = (r).f22;                  \
        (f).sideZ = (r).f24;                 \
        (f).dz = (r).f26;                    \
    } while (0)
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEF32; /* effect cooldown */

/* Sets up vehicle 8 (object 8) from the model header `hdr` at (x, y, z) with
 * heading `heading` (called by the object dispatcher func_802A350C):
 * D_803EEF24 = hdr, two 0x100-byte buffers from the heap D_80358070
 * (D_803EEF28, D_803EEF2C; the cursor moves 0x200), the object record
 * (func_802A1388(8, 0, buf1, buf2, hdr)), the state block D_803EEE70 reset
 * (func_802A754C) with its wheel table +0x52..+0x68 = (+-200, 350) /
 * (+-320, 500) pairs, position D_803EEF18..20, headings +0x4C/+0x4E/+0x74;
 * the initial ground under the wheels (func_802A992C, key 8); the animated
 * model (func_8029F85C(buf2, buf1, D_803EEB70, hdr)) with channel 0 started
 * and animated into each buffer in turn; the throttle bands +0x78..+0x94;
 * D_803EEF32/33/34 = 0; the parts (func_8029C354, tag 8, scale 0x32C8);
 * marker record 8 (func_80258230(8, 0x3C, 25, 25)); one frame of
 * func_802B7A88 with byte +0x9A set (no effects); and the model matrix copied
 * from the second buffer to the first (func_802AA838).
 * Register convention (conventions.txt): hdr s2, x t7, y s3, z s0, heading
 * s1; func_802A992C's fp and f12-f26 inputs are the dispatcher's leftovers
 * (here fp, f). The asm saves t0-t5 (asm caller func_802A350C keeps t1, t2
 * live), points $gp at D_803EEE70 and leaves s0-s7, fp and the FP registers
 * as its callees leave them. Its addi traps on overflow. Fidelity note: the
 * func_802B7A88 call inherits leftover registers in the asm (its zone scan
 * inputs t6, t7, s0-s4); the C version uses its own defaults. */
void func_802B7340(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f) {
    u8 *buf;
    s16 *h;

    D_803EEF24 = hdr;
    buf = D_80358070;
    D_803EEF28 = (u64 *) buf;
    D_803EEF2C = (u64 *) (buf + 0x100);
    D_80358070 = buf + 0x200;
    func_802A1388(8, 0, (s32) D_803EEF28, (s32) D_803EEF2C, hdr);
    func_802A754C(D_803EEE70);
    h = (s16 *) (D_803EEE70 + 0x52);
    h[0] = 0xC8;
    h[1] = 0x15E;
    h[2] = -0xC8;
    h[3] = 0x15E;
    h[4] = 0xC8;
    h[5] = -0x15E;
    h[6] = 0x140;
    h[7] = 0x1F4;
    h[8] = -0x140;
    h[9] = 0x1F4;
    h[10] = 0x140;
    h[11] = -0x1F4;
    D_803EEF18[0] = x;
    D_803EEF18[1] = y;
    D_803EEF18[2] = z;
    VEH8_S16(0x4C) = heading;
    VEH8_S16(0x4E) = heading;
    VEH8_S16(0x74) = heading;
    func_802A992C((s16 *) (D_803EEE70 + 0x52), D_803EEF18[1], x, z, (s32 *) (D_803EEE70 + 4),
                  (s32 *) &D_803EEF18[1], (s16 *) (D_803EEE70 + 0x4C), 8, fp, D_803EEE70, f);
    func_8029F85C((u32 *) D_803EEF2C, (u32 *) D_803EEF28, D_803EEB70, D_803EEF24);
    func_802A039C(D_803EEB70, 0, 0x64);
    func_802A03D4(D_803EEB70, 0, 0);
    func_802A040C(D_803EEB70, 0, 0);
    func_802A0480(0.0f, D_803EEB70, 0, 0);
    func_802A0290(D_803EEB70, 0, 1);
    func_8029E558((u8 *) D_803EEF28, (u8 *) D_803EEF2C, D_803EEB70);
    func_802A0320(0, D_803EEB70);
    func_802A0290(D_803EEB70, 0, 1);
    func_8029E558((u8 *) D_803EEF2C, (u8 *) D_803EEF28, D_803EEB70);
    h = (s16 *) (D_803EEE70 + 0x78);
    h[0] = -0xB4;
    h[1] = 0;
    h[2] = 2;
    h[3] = 0;
    h[4] = 0x50;
    h[5] = 4;
    h[6] = 0x50;
    h[7] = 0xA0;
    h[8] = 5;
    h[9] = 0xA0;
    h[10] = 0xFA;
    h[11] = 3;
    h[12] = 0xFA;
    h[13] = 0x15E;
    h[14] = 2;
    D_803EEF32 = 0;
    D_803EEF33 = 0;
    D_803EEF34 = 0;
    hdr = D_803EEF24;
    func_8029C354(8, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8), 0x32C8);
    func_80258230(8, 0x3C, 0x19, 0x19);
    D_803EEE70[0x9A] = 1;
    func_802B7A88();
    D_803EEE70[0x9A] = 0;
    hdr = D_803EEF24;
    func_802AA838((u8 *) D_803EEF2C, (u8 *) D_803EEF28, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7340.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEE70[]; /* this vehicle's state block */

/* Enter vehicle type 8: clears the byte at +0x99, D_8036444C/50 = 3000,
 * 1000, then func_802C4310(arg0, 0xCE) (arg0 passes straight through; hd.c
 * calls this with no arguments and func_802C4310 ignores it). The asm also
 * points $gp at D_803EEE70 and leaves it there (conventions.txt: clobbers
 * gp); C code doesn't use $gp. Same shape as func_802BBDC8 (772A0),
 * func_802CCC8C (88160), func_802CFA0C (8AEE0). */
void func_802B76AC(s32 arg0) {
    D_803EEE70[0x99] = 0;
    D_8036444C = 3000;
    D_80364450 = 1000;
    func_802C4310(arg0, 0xCE);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B76AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEE70[]; /* this vehicle's state block */

/* Exit check for vehicle type 8, same shape as func_802B2EF8 (6E200): returns
 * 1 when none of the bytes at +0x96, +0x97, +0x98 equals 1, else 0 (s32; the
 * C caller in hd.c declares it void and returns the leftover v0). */
s32 func_802B76F8(void) {
    if (D_803EEE70[0x96] == 1 || D_803EEE70[0x97] == 1 || D_803EEE70[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B76F8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EEF28;
extern u64 *D_803EEF2C;

/* Vehicle-type 8 exit: zero the speed (s16 at +0x76), copy 0x100 bytes
 * between the two buffers D_803EEF28/D_803EEF2C point at (func_802A7764),
 * then func_802C444C. Same shape as func_802CFAB4 (8AEE0). */
void func_802B7754(void) {
    *(s16 *) (D_803EEE70 + 0x76) = 0;
    func_802A7764(D_803EEF28, D_803EEF2C, 0x100);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7754.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Vehicle 8 while not driven (hd.c, func_8024B618): follow the ground under
 * its wheels (func_802A9A60 at D_803EEF18 x/y/z, key 8; heights into +4,
 * middle height into D_803EEF1C), place the model (func_802B8278) and report
 * the position (func_802A133C(z, 8, x, y, D_803EEE70)).
 * Fidelity note: the asm passes func_802A9A60 whatever its C caller left in
 * fp (it ends up as D_803ED3F2[0..2] and the byte +0x50) and in f12-f26 (the
 * triangle scans' FP pass-through); the C passes 0 and zeros. */
void func_802B77A0(void) {
    TriSideOut f;
    Out802A9A60 o;

    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A9A60((s16 *) (D_803EEE70 + 0x52), D_803EEF18[1], D_803EEF18[0], D_803EEF18[2],
                  (s32 *) (D_803EEE70 + 4), (s32 *) &D_803EEF18[1], (s16 *) (D_803EEE70 + 0x4C), 8, 0,
                  D_803EEE70, &f, &o);
    func_802B8278();
    func_802A133C(D_803EEF18[2], 8, D_803EEF18[0], D_803EEF18[1], D_803EEE70);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B77A0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803EEF18[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 8 at its position
 * D_803EEF18..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802B7A88 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802B78B0(ZoneScanRegs *r) {
    return func_802ABD54(8, D_803EEF18[0], D_803EEF18[1], D_803EEF18[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Value pairs on triangle `id` (func_802AAD0C) at vehicle 8's x/z
 * (D_803EEF18[0], [2]) -> s16s at +0x6A/+0x6C, and at that point offset by
 * entry 0 of the wheel table +0x52 rotated by the heading +0x4C
 * (func_802A94A4) -> +0x70/+0x72; +0x6E = the heading. The second
 * func_802AAD0C's f24 input is the first one's f24 result.
 * Register convention (conventions.txt): id a3, f24 in; func_802AAD0C's FP
 * results pass through (here through r). The asm points $gp at D_803EEE70
 * without restoring it, leaves s4 = +0x4C, and saves t0, t1, t3, t4 (asm
 * caller func_802AB50C keeps t0 and t1 live: a mixed N64 build would need a
 * thunk). Same shape as func_802C59B4 (7FB50) plus the second point. The
 * asm's adds trap on overflow. */
void func_802B78F4(s32 id, InterpRegs *r) {
    s32 x = D_803EEF18[0];
    s32 z = D_803EEF18[2];
    s32 dx;
    s32 dz;

    func_802AAD0C(id, x, z, r);
    VEH8_S16(0x6A) = r->t3;
    VEH8_S16(0x6C) = r->t4;
    VEH8_S16(0x6E) = VEH8_U16(0x4C);
    dx = func_802A94A4(0, (s16 *) (D_803EEE70 + 0x52), (s16 *) (D_803EEE70 + 0x4C), &dz);
    func_802AAD0C(id, x + dx, z + dz, r);
    VEH8_S16(0x70) = r->t3;
    VEH8_S16(0x72) = r->t4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B78F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Vehicle 8 driven along a path triangle `id` (dispatched by func_802AB714):
 * heading +0x4E = +0x4C = func_802AB9A4 (steer from the +0x6A/+0x6C point
 * toward the +0x70/+0x72 point, heading +0x6E), then the value pair at
 * (+0x6A, +0x6C) on the triangle (func_802AAE54) is the new x/z for
 * func_802A8768 (ground contact; id 8, divisors 0x2BC/0x190, slot result s3
 * from func_802AB9A4, FP state from func_802AAE54), after func_802B8424 and
 * D_803ED40B = 1. Then the model is placed (func_802B8278) and the position
 * reported (func_802A133C(z, 8, x, y)).
 * Register convention (conventions.txt): id a3; f24 comes in from the
 * dispatcher (whatever the previous vehicle function left) and the f24
 * func_802A8768 leaves goes back out (here through *f24). The asm points $gp
 * at D_803EEE70 and leaves s0-s7, fp, f20-f28 as its callees leave them; it
 * saves a3, t0, t1, t2 (asm caller func_802AB714 keeps a3, t0, t1 live: a
 * mixed N64 build would need a thunk). */
void func_802B7980(s32 id, f32 *f24) {
    InterpRegs r;
    TriSideOut f;
    Regs802A8768 rr;
    s32 s3;
    s32 h;

    r.f24 = *f24;
    h = func_802AB9A4((s16 *) (D_803EEE70 + 0x52), VEH8_S16(0x6A), (u16 *) (D_803EEE70 + 0x6E), id,
                      VEH8_S16(0x6C), VEH8_S16(0x70), VEH8_S16(0x72), &s3, &r);
    VEH8_S16(0x4E) = h;
    VEH8_S16(0x4C) = h;
    func_802AAE54(id, VEH8_S16(0x6A), VEH8_S16(0x6C), &r);
    func_802B8424();
    D_803ED40B = 1;
    INTERP_TO_TRISIDE(f, r);
    rr.s3 = s3;
    func_802A8768(D_803EEE70, 8, (s32 *) &D_803EEF18[0], (s32 *) &D_803EEF18[1], (s32 *) &D_803EEF18[2], r.t3,
                  r.t4, 0x2BC, 0x190, (s16 *) (D_803EEE70 + 0x4C), D_803EEE70 + 0x96, (s16 *) (D_803EEE70 + 0x52),
                  (s32 *) (D_803EEE70 + 0x28), (s32 *) (D_803EEE70 + 0x40), (s32 *) (D_803EEE70 + 0x34),
                  (s32 *) (D_803EEE70 + 4), &rr, &f);
    *f24 = f.sideZ;
    func_802B8278();
    func_802A133C(D_803EEF18[2], 8, D_803EEF18[0], D_803EEF18[1], D_803EEE70);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7980.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Ring-steering branch of func_802B7A88 (D_803A7425 set): func_8029A914,
 * D_803EEF33 = 1, then steer along the ring: func_802A70D8, func_802A71DC
 * (heading +0x4E toward +0x4C, scale D_8030D900; the new index goes to
 * D_803EEF30, +0x4E and +0x74), func_802A746C with that turn, func_802A6FE4
 * (limit 0) and the object collision pass (func_802BE77C).
 * The asm also computes |(+0x4E - 0x800, wrapped) - ring midpoint| (folded
 * to <= 0x800) and then always branches past its use (a `slti 0xC8` test
 * that is never reached); the value only survives in v1 as func_802A746C's
 * pass-through input, which is reproduced. */
static void port_b7a88_ring(void) {
    s32 mid;
    s32 d;
    s32 cur;
    s32 turn;
    s32 target;

    func_8029A914(D_803EEE70);
    D_803EEF33 = 1;
    mid = func_802A6F6C();
    d = VEH8_U16(0x4E) - 0x800;
    if (d < 0) {
        d += 0xFFF;
    }
    d -= mid;
    if (d < 0) {
        d = -d;
    }
    if (d >= 0x801) {
        d = 0xFFF - d;
    }
    func_802A70D8(D_803EEE70);
    turn = func_802A71DC(D_803EEE70, VEH8_U16(0x4E), VEH8_U16(0x4C), &cur, D_8030D900);
    D_803EEF30 = cur;
    VEH8_S16(0x4E) = cur;
    VEH8_S16(0x74) = cur;
    func_802A746C(D_803EEE70, turn, d, &target);
    func_802A6FE4(D_803EEE70, 0);
    D_803F77D0 = D_803EEB70;
    func_802BE77C(8, D_803EEE70);
}

/* Crash branch of func_802B7A88 (the object pass reported a hit): restore
 * the state saved at the start of the frame (func_802A768C, 0x100 bytes of
 * the other model buffer back), lock the throttle for 5 frames, bounce the
 * speed to -max(|v|, 50) / 2 with the sign flipped (arithmetic >> 1), and
 * re-place the model (func_802B8278). */
static void port_b7a88_crash(void) {
    u64 *end;
    s32 v;

    D_803EEF33 = 0;
    if (D_8035805C != 0) {
        func_802A768C(D_803EEE70, D_803EEB70, (s32 *) &D_803EEF18[0], (s32 *) &D_803EEF18[1],
                      (s32 *) &D_803EEF18[2], D_803EEF2C, D_803EEF28, 0x100, &end);
    } else {
        func_802A768C(D_803EEE70, D_803EEB70, (s32 *) &D_803EEF18[0], (s32 *) &D_803EEF18[1],
                      (s32 *) &D_803EEF18[2], D_803EEF28, D_803EEF2C, 0x100, &end);
    }
    D_803EEF34 = 5;
    v = VEH8_S16(0x76);
    if (v >= 0) {
        if (v < 0x32) {
            v = 0x32;
        }
    } else if (!(v < -0x31)) {
        v = -0x32;
    }
    VEH8_S16(0x76) = -v >> 1;
    func_802B8278();
}

/* Per-frame update of vehicle 8 while driven (hd.c's func_8024B7AC, and
 * func_802B7340). Zone lookup (func_802B78B0), save the state for a crash
 * rollback (func_802A75DC), engine sound 0xA4 (func_802C4724); unless byte
 * +0x9A is set, trail and effects (func_802B7F98, fed the zone scan
 * registers); func_802CB690 while D_80367BFF is set; func_802B8424; steering
 * (func_802A7E70 at the rate func_802B83B0 returns) and, unless the throttle
 * lockout D_803EEF34 is counting down, throttle (func_802A785C: mode 3,
 * delta 0x10); heading (func_802A7FD8: rate 6000, with sound), slope
 * (func_802A83B8 into the f32 at +0, then func_802A843C: kind 8, clamp, div
 * 700.0), func_802A7070 while D_803EEF33 is set; move (func_802A860C along
 * +0x4E) and ground contact (func_802A8768, id 8, divisors 0x2BC/0x190, after
 * D_803ED40B = 1); animate (func_8029E558 into the current buffer of
 * D_803EEF28/2C) and place the model (func_802B8278); collision state reset
 * (func_8029A800: b2 = b3 = 1, b0 = 7, h1 = speed, h2 = 0x96, b8 = 8),
 * part contacts (func_8029C52C(8)) and world collision (func_8029AA10(8)).
 * Without a ring hit (D_803A7425 clear): D_803A7424 = 0 and the object pass
 * (func_802BE77C) - a hit there takes the crash branch, else D_803EEF33 = 0;
 * with one: the ring-steering branch. Finally the player position/speed/
 * headings D_803643E0..E8 / D_8036443C..40 are copied from the vehicle and
 * the position reported (func_802A133C(z, 8, x, y)).
 * Fidelity notes: the asm feeds func_802B78B0 its C caller's t6, t7, s0-s4
 * (scan registers, 0 here) and func_802A8768 the FP registers left by
 * func_802A860C's sine routines and earlier code (zeros here). The asm's
 * add/neg trap on overflow. */
void func_802B7A88(void) {
    ZoneScanRegs zr;
    Out802A860C o;
    Regs802A8768 rr;
    TriSideOut f;
    s32 x;
    f32 slope;

    zr.t6 = zr.t7 = zr.s0 = zr.s1 = zr.s2 = zr.s3 = zr.s4 = 0;
    func_802B78B0(&zr);
    func_802A75DC(D_803EEE70, D_803EEB70, (s32 *) &D_803EEF18[0], (s32 *) &D_803EEF18[1],
                  (s32 *) &D_803EEF18[2]);
    func_802C4724(0xA4);
    if ((s8) D_803EEE70[0x9A] == 0) {
        func_802B7F98(zr.t6, zr.t7, zr.s0, zr.s1, zr.s2, zr.s3, zr.s4);
    }
    if (D_80367BFF != 0) {
        func_802CB690(D_803EEE70);
    }
    func_802B8424();
    func_802A7E70(func_802B83B0(), (u16 *) (D_803EEE70 + 0x4C));
    if (D_803EEF34 == 0) {
        func_802A785C(D_803EEE70, (s16 *) (D_803EEE70 + 0x76), 3, D_803EEE70 + 0x96, (s16 *) (D_803EEE70 + 0x78),
                      0x10);
    } else {
        D_803EEF34--;
    }
    func_802A7FD8(D_803EEE70, (u16 *) (D_803EEE70 + 0x74), 0x1770, (s16 *) (D_803EEE70 + 0x76),
                  (u16 *) (D_803EEE70 + 0x4C), (u16 *) (D_803EEE70 + 0x4E), (s8 *) (D_803EEE70 + 0x99), 1);
    slope = func_802A83B8((s16 *) (D_803EEE70 + 0x76), D_803EEE70 + 0x96, (s32 *) (D_803EEE70 + 4),
                          (f32 *) D_803EEE70);
    func_802A843C(D_803EEE70, (s16 *) (D_803EEE70 + 0x76), 8, (s8 *) (D_803EEE70 + 0x96), (s32 *) (D_803EEE70 + 4),
                  1, 700.0f);
    if (D_803EEF33 != 0) {
        func_802A7070(D_803EEE70, &D_803EEF30);
    }
    x = func_802A860C(slope, VEH8_U16(0x4E), (s16 *) (D_803EEE70 + 0x76), (s32 *) &D_803EEF18[0],
                      (s32 *) &D_803EEF18[2], &o);
    D_803ED40B = 1;
    rr.s3 = o.s3;
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802A8768(D_803EEE70, 8, (s32 *) &D_803EEF18[0], (s32 *) &D_803EEF18[1], (s32 *) &D_803EEF18[2], x, o.t1,
                  0x2BC, 0x190, (s16 *) (D_803EEE70 + 0x4C), D_803EEE70 + 0x96, (s16 *) (D_803EEE70 + 0x52),
                  (s32 *) (D_803EEE70 + 0x28), (s32 *) (D_803EEE70 + 0x40), (s32 *) (D_803EEE70 + 0x34),
                  (s32 *) (D_803EEE70 + 4), &rr, &f);
    if (D_8035805C != 0) {
        func_8029E558((u8 *) D_803EEF28, (u8 *) D_803EEF2C, D_803EEB70);
    } else {
        func_8029E558((u8 *) D_803EEF2C, (u8 *) D_803EEF28, D_803EEB70);
    }
    func_802B8278();
    func_8029A800(D_803EEF18[2], (s32) D_80305D30, 1, 1, D_803EEF18[0], D_803EEF18[1], 7, VEH8_S16(0x76), 0x96, 0,
                  8, D_803EEE70);
    func_8029C52C(8, D_803EEE70);
    func_8029AA10(8);
    if (D_803A7425 == 0) {
        D_803A7424 = 0;
        D_803F77D0 = D_803EEB70;
        func_802BE77C(8, D_803EEE70);
        if (D_803A7424 != 0) {
            port_b7a88_crash();
        } else {
            D_803EEF33 = 0;
        }
    } else {
        port_b7a88_ring();
    }
    D_803643E0 = D_803EEF18[0];
    D_803643E4 = D_803EEF18[1];
    D_803643E8 = D_803EEF18[2];
    D_8036443C = VEH8_S16(0x76);
    D_8036443E = VEH8_U16(0x4E);
    D_80364440 = VEH8_U16(0x4C);
    func_802A133C(D_803EEF18[2], 8, D_803EEF18[0], D_803EEF18[1], D_803EEE70);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7A88.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EEF32;   /* effect cooldown */
void func_802B80D8(void);

/* Per-frame effects for vehicle type 8 ($gp = D_803EEE70, read as the
 * global): the tyre trail (func_802B80D8); then, if the cooldown
 * D_803EEF32 is nonzero it just counts down, else when byte +0x99 is set the
 * cooldown becomes 1 and, with fewer than 15 active func_802A6274 records,
 * four are set up (def D_802C2954, type 1 at (8, 1..4, 1), data 0x29810
 * for the first two and 0x1D4C0 for the last two). Finally
 * func_802C4584(|speed| >> 5) with speed the s16 at +0x76.
 * Register convention (conventions.txt): t6, t7, s0-s4 pass through to
 * func_802A6274 (t6 and s1 in its in/out block, chained between the calls);
 * the asm changes s1 and s5-s7. Same shape as func_802D02F8 (8AEE0). */
void func_802B7F98(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 v;

    func_802B80D8();
    if (D_803EEF32 != 0) {
        D_803EEF32--;
    } else if (D_803EEE70[0x99] != 0) {
        D_803EEF32 = 1;
        if (func_802A5ED0() < 15) {
            io.t6 = t6;
            io.s1 = s1;
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 8, 1, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 8, 2, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 8, 3, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x1D4C0, 1, 8, 4, 1, t7, s0, s2, s3, s4, 1);
        }
    }
    v = *(s16 *) (D_803EEE70 + 0x76);
    if (v < 0) {
        v = -v;
    }
    func_802C4584((u32) v >> 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B7F98.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803EEF18[]; /* [0], [2]: trail x, z */

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803EEE70): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803EEF18[0], D_803EEF18[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802B7F98 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802B80D8(void) {
    if (D_803EEE70[0x99] != 0 && D_803EEE70[0x98] != 1 && D_803EEE70[0x50] < 3 && D_803EEE70[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803EEE70 + 0x1C), 250, -400, -400, -400, D_803EEF18[0], D_803EEF18[2],
                      *(u16 *) (D_803EEE70 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B80D8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Places vehicle 8's model ($gp = D_803EEE70, read directly): rotation y =
 * +0x4C (x/z rotation left as they are) and position D_803EEF18..20 at scale
 * 0x32C8 into the matrix at header word [header word 0x18 + 4] of the current
 * buffer (func_802AA764); then its parts (func_8029C454, tag 8, header words
 * 4/8) and points (func_802ABBEC, id 8, header words 0/4), all relative to the
 * header D_803EEF24.
 * func_802AA890's register in/outs (regs; only used for an entry with no
 * matrices): s2 = the matrix (func_802AA764 leaves it there), a3/s1/s0 are
 * the asm callers' leftovers (0 here; fidelity note), and before
 * func_802ABBEC v1/a0 = y/z (func_8029C454 restores them). The asm's add
 * traps on overflow. Asm caller func_802B7A88 keeps t6, t7 live (a mixed N64
 * build would need a thunk). */
void func_802B8278(void) {
    u8 *hdr = D_803EEF24;
    u8 *m = *(u8 **) (hdr + *(s32 *) (hdr + 0x18) + 4);
    MtxChainRegs regs;
    u8 *base;

    m += (s32) (D_8035805C != 0 ? D_803EEF28 : D_803EEF2C);
    D_803ED390[1] = VEH8_U16(0x4C);
    func_802AA764(D_803EEF18[0], D_803EEF18[1], D_803EEF18[2], 0x32C8, (s32 *) m);
    base = (u8 *) (D_8035805C != 0 ? D_803EEF28 : D_803EEF2C);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    func_8029C454(D_803EEF18[0], D_803EEF18[1], D_803EEF18[2], 8, hdr + *(s32 *) (hdr + 4),
                  hdr + *(s32 *) (hdr + 8), base, &regs);
    hdr = D_803EEF24;
    regs.v1 = D_803EEF18[1];
    regs.a0 = D_803EEF18[2];
    func_802ABBEC(8, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), base, &regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8278.s")
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

extern f32 D_8030D904;

/* Speed (s16 at +0x76 of D_803EEE70, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D904, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802B7A88 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802B83B0(void) {
    f32 div;
    s32 r;

    if (D_803EEE70[0x96] == 1 || D_803EEE70[0x97] == 1 || D_803EEE70[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D904;
    }
    CVT_W_S(r, *(s16 *) (D_803EEE70 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B83B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Vehicle-module setup leaf (shape of func_802AFBA0 in 69BB0, other
 * constants): D_803EBBF4 = D_803EBBF0 * 4, D_803ED3F6/7 = 60, 3.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm callers keep
 * registers live across the call (func_802B7980: t3, t4, f12, f14;
 * func_802B7A88: a0-a3); a mixed N64 build would need a thunk preserving
 * those, the native port does not. */
void func_802B8424(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8424.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets up the flying vehicle (object 0xFE) from the model header `hdr`
 * (called by the object dispatcher func_802A3134): D_803EF2F8 = hdr, two
 * 0x800-byte buffers from the heap D_80358070 (D_803EF2FC, D_803EF300; the
 * cursor moves 0x1000), the object record (func_802A1388(0xFE, a1Val, buf1,
 * buf2, hdr)), state block D_803EF240: heading +0x4C/+0x4E = 0, speed +0x76 =
 * 20, D_803EF32A / D_803EF2E6 / D_803EF328 = 0; the animated model
 * (func_8029F85C(buf2, buf1, D_803EEF40, hdr)) and its channels (0: 100, 0,
 * 0, 0.0f/0, started; animated into each buffer in turn with channel 0
 * reset in between; 1: 9 looping; 2, 3 looping; 4: 1, 1, 1 started), state
 * D_803EF32C / D_803EF324 / D_803EF32D / D_803EF31C = 0, marker record 0xFE
 * (func_80258230(0xFE, 0x78, 0x2D, 0x2D)), one frame of func_802B899C, and
 * the model matrix copied from the second buffer to the first
 * (func_802AA838 at header word [header word 0x18 + 4]).
 * Register convention (conventions.txt): a1Val a1, hdr s2. The asm saves
 * t0-t5 (asm caller func_802A3134 keeps t1 live), points $gp at D_803EF240
 * and leaves s0-s7, fp, f20, f30 as its callees leave them. Its addi traps on
 * overflow. Fidelity note: the asm caller's fp, f24 and f26 reach
 * func_802B899C's callees as leftovers (not modelled, see func_802B899C). */
void func_802B8480(s32 a1Val, u8 *hdr) {
    u8 *buf;

    D_803EF2F8 = hdr;
    buf = D_80358070;
    D_803EF2FC = buf;
    D_803EF300 = buf + 0x800;
    D_80358070 = buf + 0x1000;
    func_802A1388(0xFE, a1Val, (s32) D_803EF2FC, (s32) D_803EF300, hdr);
    *(s16 *) (D_803EF240 + 0x4C) = 0;
    *(s16 *) (D_803EF240 + 0x4E) = 0;
    D_803EF32A = 0;
    *(s16 *) (D_803EF240 + 0x76) = 0x14;
    D_803EF2E6 = 0;
    D_803EF328 = 0;
    func_8029F85C((u32 *) D_803EF300, (u32 *) D_803EF2FC, D_803EEF40, D_803EF2F8);
    func_802A039C(D_803EEF40, 0, 0x64);
    func_802A03D4(D_803EEF40, 0, 0);
    func_802A040C(D_803EEF40, 0, 0);
    func_802A0480(0.0f, D_803EEF40, 0, 0);
    func_802A0290(D_803EEF40, 0, 1);
    func_8029E558(D_803EF2FC, D_803EF300, D_803EEF40);
    func_802A0320(0, D_803EEF40);
    func_802A0290(D_803EEF40, 0, 1);
    func_8029E558(D_803EF300, D_803EF2FC, D_803EEF40);
    func_802A039C(D_803EEF40, 1, 9);
    func_802A03D4(D_803EEF40, 1, 0);
    func_802A040C(D_803EEF40, 1, 0);
    func_802A0290(D_803EEF40, 1, -1);
    func_802A0290(D_803EEF40, 2, -1);
    func_802A0290(D_803EEF40, 3, -1);
    func_802A039C(D_803EEF40, 4, 1);
    func_802A03D4(D_803EEF40, 4, 1);
    func_802A040C(D_803EEF40, 4, 1);
    func_802A0290(D_803EEF40, 4, 1);
    D_803EF32C = 0;
    D_803EF324 = 0;
    D_803EF32D = 0;
    D_803EF31C = 0;
    func_80258230(0xFE, 0x78, 0x2D, 0x2D);
    func_802B899C();
    hdr = D_803EF2F8;
    func_802AA838(D_803EF300, D_803EF2FC, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8480.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Rounded 3-D distance from (ax, ay, az) to (bx, by, bz) (62740.c; asm
 * convention in tools_port/conventions.txt: t3-t5, t6, t7, s0 -> s1). */

extern void *D_803EF2E8;  /* this sound's handle, NULL = none */

/* Positional sound 0x13 at D_803EF2EC/F0/F4 (called from hd.c). d is the
 * rounded distance from the player D_803643E0/E4/E8. Beyond 16000 the sound
 * is stopped (func_802608C8) and its handle cleared. Otherwise it is
 * started if it isn't playing, its volume (parameter 8) set to 0x7FFF -
 * max(d - 500, 0) and its pan (parameter 4) to 64 + (player x - source
 * x) / 32, clamped to 0..127. The asm's `sub`/`add` trap on overflow; C
 * doesn't. Register note: the asm saves and restores every register. Same
 * shape as func_802BA148 (75490). */
void func_802B8794(void) {
    s32 dx = D_803643E0 - D_803EF2EC;
    s32 dist;
    s32 pan;

    dist = func_802ABCDC(D_803643E0, D_803643E4, D_803643E8, D_803EF2EC, D_803EF2F0, D_803EF2F4);
    if (dist > 16000) {
        if (D_803EF2E8 != NULL) {
            func_802608C8(D_803EF2E8);
            D_803EF2E8 = NULL;
        }
        return;
    }
    if (D_803EF2E8 == NULL) {
        func_80260650(D_80367738, 0x13, &D_803EF2E8);
    }
    dist -= 500;
    if (dist < 0) {
        dist = 0;
    }
    func_80260AB8(D_803EF2E8, 8, 0x7FFF - dist);
    pan = 0x40 + (dx >> 5);
    if (pan < 0) {
        pan = 0;
    } else if (pan >= 0x80) {
        pan = 0x7F;
    }
    func_80260AB8(D_803EF2E8, 4, pan);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8794.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Per-frame update of the flying vehicle (hd.c's func_802475D8, and
 * func_802B8480): state machine (func_802B8D04), sound/animation channels
 * (func_802B98E0), animate the model into the current buffer
 * (func_8029E558; D_8035805C picks D_803EF2FC or D_803EF300), place it
 * (func_802B9B4C), copy point record 1 of object 0xFE (func_802ABC88) to the
 * marker position D_803EF310/14/18, and drop the marker on the ground
 * (func_802B8C18).
 * func_802B8C18's t3 (the height when the marker is outside the level) is the
 * header pointer D_803EF2F8: func_802B9B4C leaves it in t3 and nothing after
 * changes it - reproduced here. Fidelity note: its fp and f12-f26 inputs are
 * leftovers (fp from func_8029E558's callees, the FP registers from the C
 * caller and the callees above); the C passes 0 and zeros. The asm saves every
 * callee-saved register. */
void func_802B899C(void) {
    TriSideOut f;
    u8 *rec;

    func_802B8D04();
    func_802B98E0();
    if (D_8035805C != 0) {
        func_8029E558(D_803EF2FC, D_803EF300, D_803EEF40);
    } else {
        func_8029E558(D_803EF300, D_803EF2FC, D_803EEF40);
    }
    func_802B9B4C();
    func_802ABC88(0xFE, 1, &rec);
    D_803EF310 = ((s32 *) rec)[0];
    D_803EF314 = ((s32 *) rec)[1];
    D_803EF318 = ((s32 *) rec)[2];
    f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
    func_802B8C18((s32) D_803EF2F8, 0, &f);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B899C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EF240[]; /* this vehicle's state block */
extern s32 D_803EF31C;

/* Copies two halfwords of the vehicle block (+0x4E, +0x76) to D_803EF32A /
 * D_803EF328, and the marker position D_803EF310..1C to D_803ED808..10 and
 * D_80368030. Then, in game mode 0x1000: if D_803EF32C == 5 or
 * D_803EF31C >= D_803EF314, calls func_80275390(0x2000). In any other mode:
 * if D_803EF32C == 4, sets the next game mode to 0x1000.
 * The game-mode compare is a full 64-bit compare in the asm (ld/beq). */
void func_802B8AE4(void) {
    D_803EF32A = *(s16 *) (D_803EF240 + 0x4E);
    D_803EF328 = *(s16 *) (D_803EF240 + 0x76);
    D_803ED808 = D_803EF310;
    D_803ED80C = D_803EF314;
    D_803ED810 = D_803EF318;
    D_80368030 = D_803EF31C;
    if (D_80364A90 == 0x1000) {
        if (D_803EF32C == 5 || !(D_803EF31C < D_803EF314)) {
            func_80275390(0x2000);
        }
    } else if (D_803EF32C == 4) {
        D_80364A98 = 0x1000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8AE4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Shadow/marker 0xFE at the sound source D_803EF2EC/F0/F4 ($gp = D_803EF240,
 * read as the global): when x and z are positive and inside the level
 * (D_803BE732 / D_803BE736 << 5), the ground height under it comes from
 * func_802A9B1C(slot 0, x, z, D_803EF31C, skip 0xFE) and is stored to
 * D_803EF31C; otherwise the height is the incoming t3 (whatever the asm
 * callers left there). Then func_802582C4(0xFE, x, height, z, D_803EF2F0, 0,
 * 0, (s16) +0x4C) and D_803EF326 = +0x4C.
 * Register convention (conventions.txt): t3 and fp (func_802A9B1C's fp input)
 * come in from the asm callers, f12-f26 pass through func_802A9B1C (here
 * through f). The asm's v1 (read by func_802475D8 via func_802B899C) is just
 * func_802582C4's leftover; f12/f14 too after that C call. */
void func_802B8C18(s32 t3, s32 fp, TriSideOut *f) {
    s32 x = D_803EF2EC;
    s32 z = D_803EF2F4;
    s32 y = t3;

    if (x > 0 && z > 0 && x < (D_803BE732 << 5) && z < (D_803BE736 << 5)) {
        y = func_802A9B1C(0, x, z, D_803EF31C, 0xFE, D_803EF240, fp, f);
        D_803EF31C = y;
    }
    func_802582C4(0xFE, D_803EF2EC, y, D_803EF2F4, D_803EF2F0, 0, 0, *(s16 *) (D_803EF240 + 0x4C));
    D_803EF326 = *(s16 *) (D_803EF240 + 0x4C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8C18.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803EF320; /* best distance so far */
extern s16 D_803EF324; /* turn rate */
extern char D_80305D40[];
extern u8 D_803EEF40[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */
s32 func_802B988C(void);

#define VEH8D04_S16(off) (*(s16 *) (D_803EF240 + (off)))
#define VEH8D04_U16(off) (*(u16 *) (D_803EF240 + (off)))
#define ABS_8D04(v) ((v) < 0 ? -(v) : (v))

/* Move cur toward target by at most 0x14 (signed compares). */
static s32 port_approach_14(s32 cur, s32 target) {
    if (cur != target) {
        if (!(target < cur)) {
            cur += 0x14;
            if (target < cur) {
                cur = target;
            }
        } else {
            cur -= 0x14;
            if (cur < target) {
                cur = target;
            }
        }
    }
    return cur;
}

/* One axis of the state-3 approach: pos moves toward target by
 * (|target - pos| << 10) / dist * speed >> 10 (unsigned 32-bit arithmetic,
 * then the sign of target - pos). */
static s32 port_step_axis(s32 pos, s32 target, s32 dist, s32 speed) {
    s32 d = target - pos;
    u32 m = (u32) ABS_8D04(d) << 10;

    m = (m / (u32) dist) * (u32) speed >> 10;
    if (d < 0) {
        m = -m;
    }
    return pos + m;
}

/* State 1 (approaching the target D_803EF308/30C): bx/bz are the asm's
 * leftover registers from func_802B988C (D_803EF2EC/F4, or D_80368048 as bx
 * after the debug warp). Returns 1 to go on to the heading update. */
static s32 port_state1(void) {
    s32 ax = D_803EF308;
    s32 az = D_803EF30C;
    s32 bx = D_803EF2EC;
    s32 bz = D_803EF2F4;
    s32 dist = func_802B988C();
    s32 limit;
    s32 a;
    s32 rx;
    s32 rz;
    s32 d1;
    s32 d2;
    s32 t0;
    s32 t3;
    s32 t2;

    if (D_80364A90 == 0x800) {
        if (D_802E8BDC == 0x1A || D_802E8BDC == 4) {
            limit = 48000;
        } else if (D_802E8BDC == 0x1D || D_802E8BDC == 0x3A || D_802E8BDC == 0xD) {
            limit = 70000;
        } else {
            limit = 30000;
        }
        if (!(limit < dist)) {
            D_80364A98 = 1;
            func_8029A7E4(D_80305D40);
            D_803EF308 = (&D_80368048)[-1];
            bx = D_80368048;
            D_803EF30C = bx;
            func_8026AF6C(0x4000);
        }
    }
    if (dist < 0x7D0) {
        D_803EF32C = 2;
        return 1;
    }
    t0 = VEH8D04_S16(0x76) + 2;
    if (t0 >= 0xA1) {
        t0 = 0xA0;
    }
    VEH8D04_S16(0x76) = t0;
    a = func_802ABB1C(ax, az, 0, dist, bx, bz);
    if (a >= 0x401) {
        a = func_802ABB1C(ax, az, dist, 0, bx, bz) + 0x400;
        if (a >= 0x801) {
            a = func_802ABB1C(ax, az, 0, -dist, bx, bz) + 0x800;
        }
    }
    rx = func_802ACE38(0, dist, a, &rz);
    d1 = ABS_8D04(rx + bx - ax) + ABS_8D04(rz + bz - az);
    rx = func_802ACE38(0, dist, 0xFFF - a, &rz);
    d2 = ABS_8D04(rx + bx - ax) + ABS_8D04(rz + bz - az);
    if (!(d1 < d2)) {
        a = 0xFFF - a;
    }
    t0 = VEH8D04_U16(0x4E) - a;
    t3 = t0;
    if (t3 >= 0) {
        if (t3 >= 0x800) {
            t3 = 0xFFF - t3;
        }
    } else if (t3 < -0x7FF) {
        t3 += 0xFFF;
    } else {
        t3 = -t3;
    }
    t3 = (u32) t3 >> 6;
    if ((t0 > 0) ? (t0 < 0x800) : (t0 < -0x800)) {
        t2 = D_803EF324 - 1;
        if (t2 < -t3) {
            t2 = -t3;
        }
    } else {
        t2 = D_803EF324 + 1;
        if (t3 < t2) {
            t2 = t3;
        }
    }
    D_803EF324 = t2;
    return 1;
}

/* State 3 (homing in): returns 1 when the target is reached (go to state
 * 4), 0 when still closing in. */
static s32 port_state3(void) {
    s32 t = D_803EF324;
    s32 dist;
    s32 speed;

    if (t >= 0) {
        t--;
        if (t < 0) {
            t = 0;
        }
    } else {
        t++;
        if (t > 0) {
            t = 0;
        }
    }
    D_803EF324 = t;
    dist = func_802B988C();
    if (dist == 0) {
        return 1;
    }
    speed = VEH8D04_S16(0x76);
    D_803EF2EC = port_step_axis(D_803EF2EC, D_803EF308, dist, speed);
    D_803EF2F4 = port_step_axis(D_803EF2F4, D_803EF30C, dist, speed);
    dist = func_802B988C();
    if (dist < D_803EF320) {
        D_803EF320 = dist;
        return 0;
    }
    return 1;
}

/* State 4 (landing at D_80368030 + 0xFA0). */
static void port_state4(void) {
    s32 target = D_80368030 + 0xFA0;
    s32 out[8];
    s32 v;
    void *h;

    if (target != D_803EF2F0) {
        D_803EF2F0 = port_approach_14(D_803EF2F0, target);
        return;
    }
    func_802A04BC(4, D_803EEF40, out);
    if (out[0] == 1) {
        return;
    }
    D_803EF32D = 1;
    if (D_80364A90 != 0x1000) {
        v = func_8026A610(D_803643E0, D_803643E0, D_803EF2EC, D_803EF2F4);
        v = 0x88B8 - (v << 1);
        if (v >= 0xFA1) {
            if (v >= 0x8000) {
                v = 0x7FFF;
            }
            h = func_80260650(D_80367738, 0x28, NULL);
            func_80260AB8(h, 8, v);
        }
    }
    D_803EF32C = 5;
    func_802A0290(D_803EEF40, 4, 1);
}

/* Per-frame update of the flying vehicle D_803EF240 (the asm points $gp at
 * it), a state machine on D_803EF32C (any other value hits the asm's
 * `syscall` debug trap and then runs as 6):
 *   6: if animation channel 4 of D_803EEF40 (func_802A04BC) has a nonzero
 *      float, restart it (func_802A03D4, func_802A0290); state 0, then as 0.
 *   0: D_803EF324 one step toward 4; heading update.
 *   1: approach D_803EF308/30C (port_state1); heading update.
 *   2: slow down by 4 to 0x14 (heading update while above), then state 3
 *      with D_803EF320 = distance, then as 3.
 *   3: D_803EF324 one step toward 0, move straight at the target
 *      (port_state3); when reached: position = target, state 4 and channel 4
 *      restarted (func_802A0290).
 *   4: descend (D_803EF2F0 toward D_80368030 + 0xFA0, 0x14 a frame); there,
 *      once channel 4 is done: D_803EF32D = 1, a landing sound outside game
 *      mode 0x1000, state 5, channel 4 restarted.
 *   5: wait for channel 4 (D_803EF32E = 1 when done), climb back to
 *      D_803EF304, then state 0 once both.
 * Heading update: heading +0x4E (and +0x4C) += D_803EF324 (wrapped by 0xFFF)
 * and the position D_803EF2EC/F4 advanced by the speed +0x76 along it
 * (func_802A860C). Finally, outside states 4/5: func_802A5604(D_80358074)
 * and D_803EF2F0 0x14 a frame toward D_803EF304. The asm's add/sub/neg trap
 * on overflow; the state-3 divides trap on a zero distance (not reached).
 * Register convention: no inputs. The asm leaves gp = D_803EF240 and its
 * callees' t6, t7, s0-s4, fp, f12/f14 (listed as read by func_802B899C;
 * not modelled) and clobbers s0-s4, s6, fp (conventions.txt). Unlike the
 * asm, which relies on func_802B988C leaving D_803EF2EC/F4 and the old
 * D_803EF308/30C in t3/t5/t6/s0, the C reads those values itself. */
void func_802B8D04(void) {
    s32 out[8];
    s32 t;
    Out802A860C o;

    switch (D_803EF32C) {
        default: /* syscall */
        case 6:
            func_802A04BC(4, D_803EEF40, out);
            if (!(*(f32 *) &out[7] == 0.0f)) {
                func_802A03D4(D_803EEF40, 4, 1);
                func_802A0290(D_803EEF40, 4, 1);
            }
            D_803EF32C = 0;
            /* fallthrough */
        case 0:
            t = D_803EF324;
            if (t >= 4) {
                t--;
                if (t < 4) {
                    t = 4;
                }
            } else {
                t++;
                if (t >= 5) {
                    t = 4;
                }
            }
            D_803EF324 = t;
            goto heading;
        case 1:
            port_state1();
            goto heading;
        case 2:
            t = VEH8D04_S16(0x76);
            if (t >= 0x15) {
                VEH8D04_S16(0x76) = t - 4;
                goto heading;
            }
            VEH8D04_S16(0x76) = 0x14;
            D_803EF32C = 3;
            D_803EF320 = func_802B988C();
            /* fallthrough */
        case 3:
            if (port_state3()) {
                D_803EF2EC = D_803EF308;
                D_803EF2F4 = D_803EF30C;
                D_803EF32C = 4;
                func_802A0290(D_803EEF40, 4, 1);
            }
            goto tail;
        case 4:
            port_state4();
            goto tail;
        case 5:
            func_802A04BC(4, D_803EEF40, out);
            if (out[0] != 1) {
                D_803EF32E = 1;
            }
            if (D_803EF304 != D_803EF2F0) {
                D_803EF2F0 = port_approach_14(D_803EF2F0, D_803EF304);
            } else if (D_803EF32E != 0) {
                D_803EF32C = 0;
                D_803EF32E = 0;
            }
            goto tail;
    }
heading:
    t = VEH8D04_U16(0x4E) + D_803EF324;
    if (t >= 0x1000) {
        t -= 0xFFF;
    } else if (t < 0) {
        t += 0xFFF;
    }
    VEH8D04_S16(0x4E) = t;
    VEH8D04_S16(0x4C) = t;
    D_803EF2EC = func_802A860C(0.0f, t, &VEH8D04_S16(0x76), &D_803EF2EC, &D_803EF2F4, &o);
    D_803EF2F4 = o.t1;
tail:
    if (D_803EF32C != 4 && D_803EF32C != 5) {
        func_802A5604(D_80358074);
        D_803EF2F0 = port_approach_14(D_803EF2F0, D_803EF304);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B8D04.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Distance (func_802ABCDC, y = 0) from (D_803EF2EC, D_803EF2F4) to
 * (D_803EF308, D_803EF30C), returned (the asm's s1; conventions.txt). The
 * asm also leaves t6 = D_803EF308, t7 = 0, s0 = D_803EF30C and
 * func_802ABCDC's scratch in t3/t5; asm caller func_802B8D04 reloads those
 * before using them, and keeps a0-a3, f12, f14 live (a mixed N64 build
 * would need a thunk; the native port won't). */
s32 func_802B988C(void) {
    return func_802ABCDC(D_803EF2EC, 0, D_803EF2F4, D_803EF308, 0, D_803EF30C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B988C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803EF2E6;  /* last value sent to the sound */
extern s16 D_803EF324;
extern f32 D_8030D910;
extern f32 D_8030D914;
extern u8 D_803EEF40[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */

/* Sound and animation channels for vehicle D_803EF240 (the asm's $gp; s16 at
 * +0x76). v = 160 - that value. When v differs from D_803EF2E6 (which is set
 * to v) and the sound D_803EF2E8 is playing, its parameter 0x10 is set to the
 * float D_8030D910 + v * D_8030D914 (passed as raw bits). Channel 2 of
 * D_803EEF40 gets v / 320.0 (value 0) and channel 3 gets, from
 * t = D_803EF324 (with +-1 treated as 0), (32 - t) / 64.0 for t >= 0 or
 * -t / 64.0 + 0.5 for t < 0 (func_802A0360). Returns v / 320.0 (the asm's
 * f20; conventions.txt). Asm caller func_802B899C keeps a2, a3, t7, f12, f14
 * live (a mixed N64 build would need a thunk; the native port won't). */
f32 func_802B98E0(void) {
    s32 v = 0xA0 - *(s16 *) (D_803EF240 + 0x76);
    s16 last = D_803EF2E6;
    f32 ratio = (f32) v / 320.0f;
    s32 t;

    D_803EF2E6 = v;
    if (last != v && D_803EF2E8 != NULL) {
        f32 p = D_8030D910 + (f32) v * D_8030D914;

        func_80260AB8(D_803EF2E8, 0x10, *(s32 *) &p);
    }
    func_802A0360(ratio, D_803EEF40, 2, 0);
    t = D_803EF324;
    if (t == 1 || t == -1) {
        t = 0;
    }
    if (t >= 0) {
        func_802A0360((f32) (0x20 - t) / 64.0f, D_803EEF40, 3, 0);
    } else {
        func_802A0360((f32) -t / 64.0f + 0.5f, D_803EEF40, 3, 0);
    }
    return ratio;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B98E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803EF2F8; /* the flying vehicle's model header */
extern u8 *D_803EF2FC; /* its model buffers */
extern u8 *D_803EF300;

/* Places the flying vehicle's model ($gp = D_803EF240, read directly):
 * rotation (0, +0x4C, 0) and position D_803EF2EC/F0/F4 at scale 0x5208 into
 * the matrix at header word [header word 0x18 + 4] of the current buffer
 * (func_802AA764), then its points (func_802ABBEC, id 0xFE, header words
 * 0/4, relative to the header D_803EF2F8).
 * func_802AA890's register in/outs (regs; only used for an entry with no
 * matrices): s2 = the matrix (func_802AA764 leaves it there); v1, a0 (as
 * func_802AA764 leaves them), a3, s1, s0 (the asm caller's leftovers) are 0
 * here (fidelity note). The asm's add traps on overflow. */
void func_802B9B4C(void) {
    u8 *hdr = D_803EF2F8;
    u8 *m = *(u8 **) (hdr + *(s32 *) (hdr + 0x18) + 4);
    MtxChainRegs regs;

    m += (s32) (D_8035805C != 0 ? D_803EF2FC : D_803EF300);
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    D_803ED390[1] = *(u16 *) (D_803EF240 + 0x4C);
    func_802AA764(D_803EF2EC, D_803EF2F0, D_803EF2F4, 0x5208, (s32 *) m);
    regs.v1 = 0;
    regs.a0 = 0;
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    hdr = D_803EF2F8;
    func_802ABBEC(0xFE, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)),
                  D_8035805C != 0 ? D_803EF2FC : D_803EF300, &regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/72B80/func_802B9B4C.s")
#endif
