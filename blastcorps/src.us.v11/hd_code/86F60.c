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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CB720.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8E80[]; /* this vehicle's state block */
extern s16 D_8036444C;
extern s16 D_80364450;
extern u8 D_802C2324[]; /* channel keys (7D9D0 text blob) */
extern u8 D_802C2348[];
void func_802A0508(s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);
void func_802C4310(s32 arg0, s32 arg1);

/* Enter vehicle type 13: clears the byte at +0x99, D_8036444C/50 = 3000,
 * 1000, resets the two channels keyed D_802C2324 and D_802C2348 (unk14,
 * unk11, unk12 = 0, then func_802A0508 with -1), then func_802C4310(arg0,
 * 0xCE) (arg0 passes straight through; hd.c calls this with no arguments
 * and func_802C4310 ignores it). The asm also points $gp at D_803F8E80 and
 * leaves it there (conventions.txt: clobbers gp), and leaves v1 = -1; C code
 * uses neither. */
void func_802CBA94(s32 arg0) {
    D_803F8E80[0x99] = 0;
    D_8036444C = 3000;
    D_80364450 = 1000;
    func_802A05D0((s32) D_802C2324, 0);
    func_802A05F8((s32) D_802C2324, 0);
    func_802A0620((s32) D_802C2324, 0);
    func_802A0508((s32) D_802C2324, -1);
    func_802A05D0((s32) D_802C2348, 0);
    func_802A05F8((s32) D_802C2348, 0);
    func_802A0620((s32) D_802C2348, 0);
    func_802A0508((s32) D_802C2348, -1);
    func_802C4310(arg0, 0xCE);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBA94.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 13
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1,
 * else 1 (shape shared with func_802CA140 in 853D0.c). 00000.c declares it
 * void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F8E80[];

s32 func_802CBB60(void) {
    if (D_803F8E80[0x96] == 1 || D_803F8E80[0x97] == 1 || D_803F8E80[0x98] == 1) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBB60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8E80[];
extern u64 *D_803F8F38; /* save copy pair (func_802A7764) */
extern u64 *D_803F8F3C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Leave vehicle type 13 (called from hd.c): clears the speed (s16 at +0x76),
 * func_802A7764(D_803F8F38, D_803F8F3C, 0x100), then stops the looping
 * sounds (func_802C444C). The asm points $gp at D_803F8E80 around the calls
 * and restores it. */
void func_802CBBBC(void) {
    *(s16 *) (D_803F8E80 + 0x76) = 0;
    func_802A7764(D_803F8F38, D_803F8F3C, 0x100);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBBBC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBC08.s")

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
extern u32 D_803F8F28[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xD at its position
 * D_803F8F28..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CBEF0 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CBD18(ZoneScanRegs *r) {
    return func_802ABD54(0xD, D_803F8F28[0], D_803F8F28[1], D_803F8F28[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBD18.s")
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
void func_802AAD0C(s32 id, s32 x, s32 z, InterpRegs *r); /* 62740 */
s32 func_802A94A4(s32 index, s16 *tbl, s16 *angle, s32 *dz); /* 62740 */

/* Value pairs on the D_803EBDB0 triangle `id` (func_802AAD0C) at this
 * vehicle's x/z (D_803F8F28[0], [2]) -> s16s at +0x6A/+0x6C, and at that
 * point plus the offset tbl[0] (+0x52) rotated by the heading (func_802A94A4,
 * angle at +0x4C) -> +0x70/+0x72; the heading is copied to +0x6E. Returns
 * &D_803F8E80[0x52] (the asm's v1). The offset is added with trapping adds.
 * Register convention: id in a3; func_802AAD0C's FP results (f12-f26, f24
 * also in) pass through to the caller, here through r (conventions.txt). The
 * asm saves and restores t0, t1, t3, t4 (asm caller func_802AB50C keeps t0
 * and t1 live: a mixed N64 build would need a thunk), points $gp at
 * D_803F8E80 and leaves s4 = $gp + 0x4C. */
s32 func_802CBD5C(s32 id, InterpRegs *r) {
    s32 x = D_803F8F28[0];
    s32 z = D_803F8F28[2];
    s32 dx;
    s32 dz;

    func_802AAD0C(id, x, z, r);
    *(s16 *) (D_803F8E80 + 0x6A) = r->t3;
    *(s16 *) (D_803F8E80 + 0x6C) = r->t4;
    *(u16 *) (D_803F8E80 + 0x6E) = *(u16 *) (D_803F8E80 + 0x4C);
    dx = func_802A94A4(0, (s16 *) (D_803F8E80 + 0x52), (s16 *) (D_803F8E80 + 0x4C), &dz);
    func_802AAD0C(id, x + dx, z + dz, r);
    *(s16 *) (D_803F8E80 + 0x70) = r->t3;
    *(s16 *) (D_803F8E80 + 0x72) = r->t4;
    return (s32) (D_803F8E80 + 0x52);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBD5C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBDE8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CBEF0.s")

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
extern u8 D_80370C1A;   /* flags (either one set animates) */
extern u8 D_80370C1B;
extern u8 D_803F8F45;   /* animation phase 0, 2..13 */
extern u8 D_803F8F42;   /* countdown */
extern u8 D_802C2954[]; /* definition handed to func_802A6274 (7D9D0 text blob) */
extern u8 D_802C2324[]; /* channel keys (7D9D0 text blob) */
extern u8 D_802C2348[];
void func_802A05A4(f32 f, s32 key, s32 val);
s32 func_802A5ED0(void);
void func_802C4584(s32 level);
void func_802CC56C(void);

/* When D_80370C1A or D_80370C1B is set, the phase D_803F8F45 steps by one,
 * wrapping 14 to 2, else it is reset to 0; both channels keyed D_802C2324 /
 * D_802C2348 get unk13 = phase / 2 and unk4 = 0.0f (func_802A05A4). Then
 * the tyre trail (func_802CC56C) and the countdown D_803F8F42: when nonzero
 * it just counts down; at zero, if the byte at +0x99 is set, it restarts at
 * 1 and, when fewer than 15 D_803C4B70 records are active (func_802A5ED0),
 * two func_802A6274 records are set up (def D_802C2954, data 0x29810, tag 1,
 * type 1 at (0xD, 1, 1) and (0xD, 2, 1)). Then always
 * func_802C4584(|speed| >> 5) (speed = s16 at +0x76).
 * Register convention: the asm passes t6, t7, s0-s3 through to
 * func_802A6274 (t6 and s1 in its in/out block; its s4 input is the phase /
 * 2 computed here); $gp (= D_803F8E80) is read as the global. It leaves
 * func_802A6274's s1 and changes s4-s7 (conventions.txt: clobbers). */
void func_802CC400(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3) {
    Io802A6274 io;
    s32 s4;
    s32 v;

    if (D_80370C1A != 0 || D_80370C1B != 0) {
        s4 = D_803F8F45 + 1;
        if (s4 == 14) {
            s4 = 2;
        }
        D_803F8F45 = s4;
        s4 = (u32) s4 >> 1;
    } else {
        s4 = 0;
        D_803F8F45 = 0;
    }
    func_802A05A4(0.0f, (s32) D_802C2324, s4);
    func_802A05A4(0.0f, (s32) D_802C2348, s4);
    func_802CC56C();
    if (D_803F8F42 != 0) {
        D_803F8F42--;
    } else if (D_803F8E80[0x99] != 0) {
        D_803F8F42 = 1;
        if (func_802A5ED0() < 15) {
            io.a3 = 1;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 0xD, 1, 1, t7, s0, s2, s3, s4, 1);
            io.a3 = 1;
            func_802A6274(&io, D_802C2954, 0x29810, 1, 0xD, 2, 1, t7, s0, s2, s3, s4, 1);
        }
    }
    v = *(s16 *) (D_803F8E80 + 0x76);
    if (v < 0) {
        v = -v;
    }
    func_802C4584((u32) v >> 5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC400.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F8F28[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Tyre trail (shape of func_802B3C68 in 6E200; $gp = D_803F8E80): if the
 * byte at +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 250, -400, -400, -400, D_803F8F28[0], D_803F8F28[2],
 * yaw, 3, 50, 50, 0).
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CC400 keeps a0-a3, t6, t7 live (and reads f12/f14 after
 * the call, which the asm doesn't touch but func_8027BE7C may); a mixed N64
 * build would need a thunk. */
void func_802CC56C(void) {
    if (D_803F8E80[0x99] != 0 && D_803F8E80[0x98] != 1 && D_803F8E80[0x50] < 3 && D_803F8E80[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F8E80 + 0x1C), 250, -400, -400, -400, D_803F8F28[0], D_803F8F28[2],
                      *(u16 *) (D_803F8E80 + 0x4E), 3, 50, 50, 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC56C.s")
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
extern u8 D_8035805C;     /* which of the two save copies is current */
extern u8 *D_803F8F34;    /* model header: word offsets to the part lists */
extern s16 D_803ED390[3]; /* rotation angles x, y, z for func_802AA764 */
void func_802AA764(s32 x, s32 y, s32 z, s32 scale, s32 *m);                                     /* 62740 */
void func_8029C454(s32 x, s32 y, s32 z, s32 tag, u8 *p, u8 *end, u8 *base, MtxChainRegs *regs); /* 56040 */
void func_802ABBEC(s32 id, s16 *p, s16 *end, u8 *base, MtxChainRegs *regs);                     /* 62740 */

/* Place vehicle 13's model: m = the word at +4 of the header entry at
 * hdr + hdr[6], plus the current save copy (D_803F8F38 when D_8035805C is
 * set, else D_803F8F3C); y rotation D_803ED392 = the heading (u16 at +0x4C);
 * func_802AA764(position D_803F8F28..30, scale 0x4268, m). Then the
 * vehicle's parts (func_8029C454, tag 0xD, list hdr + hdr[1] .. hdr +
 * hdr[2]) and points (func_802ABBEC, id 0xD, hdr + hdr[0] .. hdr + hdr[1])
 * relative to the current save copy. The header offsets are added with
 * trapping adds.
 * Register notes: the asm's $gp (= D_803F8E80) is read as the global. It
 * hands func_8029C454 whatever a3, s0, s1 held (they only matter for a point
 * entry with no matrices; 0 here) and s2 = m (as func_802AA764 leaves it),
 * and func_802ABBEC v1 = y, a0 = z (func_8029C454 restores them). It leaves
 * func_802ABBEC's s1, the save copy in s4 and func_802AA764's f12/f14,
 * which the survey lists as read by func_802CBEF0 (its C rewrite doesn't).
 * Clobbers s1, s2, s4-s7 (conventions.txt); asm caller func_802CBEF0 keeps
 * t6, t7 live (a mixed N64 build would need a thunk). */
void func_802CC70C(void) {
    MtxChainRegs regs;
    u8 *hdr = D_803F8F34;
    u8 *base;
    s32 *m;

    m = (s32 *) (*(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4) +
                 (s32) (D_8035805C ? (u8 *) D_803F8F38 : (u8 *) D_803F8F3C));
    D_803ED390[1] = *(u16 *) (D_803F8E80 + 0x4C);
    func_802AA764(D_803F8F28[0], D_803F8F28[1], D_803F8F28[2], 0x4268, m);
    base = D_8035805C ? (u8 *) D_803F8F38 : (u8 *) D_803F8F3C;
    hdr = D_803F8F34;
    regs.v1 = 0;
    regs.a0 = 0;
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    func_8029C454(D_803F8F28[0], D_803F8F28[1], D_803F8F28[2], 0xD, hdr + *(s32 *) (hdr + 4),
                  hdr + *(s32 *) (hdr + 8), base, &regs);
    hdr = D_803F8F34;
    regs.v1 = D_803F8F28[1];
    regs.a0 = D_803F8F28[2];
    func_802ABBEC(0xD, (s16 *) (hdr + *(s32 *) hdr), (s16 *) (hdr + *(s32 *) (hdr + 4)), base, &regs);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC70C.s")
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

extern f32 D_8030D9B4;

/* Speed (s16 at +0x76 of D_803F8E80, the asm's $gp) / 11.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9B4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CBEF0 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CC844(void) {
    f32 div;
    s32 r;

    if (D_803F8E80[0x96] == 1 || D_803F8E80[0x97] == 1 || D_803F8E80[0x98] == 1) {
        div = 11.0f;
    } else {
        div = D_8030D9B4;
    }
    CVT_W_S(r, *(s16 *) (D_803F8E80 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC844.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CBDE8 on t3, t4, f12, f14; func_802CBEF0 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CC8B8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 150;
    D_803ED3F7 = 6;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/86F60/func_802CC8B8.s")
#endif
