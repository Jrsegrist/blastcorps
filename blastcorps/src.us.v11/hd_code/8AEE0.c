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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CF6A0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803FC500[]; /* this vehicle's state block */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);

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
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFB00.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFC54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFCE0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802CFDE8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef IO802A6274_DEFINED
#define IO802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
#endif
extern u8 D_803FC5C2;   /* effect cooldown */
extern u8 D_802C2954[]; /* definition handed to func_802A6274 */
void func_802D0438(void);
s32 func_802A5ED0(void);
void func_802C4584(s32 level);

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
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D05D8.s")

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
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802D0784(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0784.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D07E0.s")

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
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802A02E4(s32 idx, void *base);
void func_802C444C(void);
void func_802608C8(void *arg0);

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
extern s16 D_8036444C;
extern s16 D_80364450;
extern void *D_80367738;
extern void *D_803FC990; /* engine sound handle */
extern u8 D_802C22D0[]; /* key of this vehicle's func_802A06B4 entry */
extern u8 D_803FC5D0[]; /* this vehicle's animation channel table */
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_802A0508(s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0E44.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D0F98.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s16 D_803FC996;  /* last channel unk13 seen (sound trigger) */
extern u8 D_803FC99A;
extern u8 D_803F7804;
extern void *D_803F7844; /* sound handle */
extern u8 D_80370C1A;    /* input flags */
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C1D;
extern u8 D_80370C35;
extern f32 D_8030D9E0;
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A04BC(s32 idx, void *base, s32 *out);
void func_8029F9D4(s32 a, s32 b, void *base);
s32 func_8026A8E0(s32 lo, s32 hi);
void func_80278EB0(s32 n, f32 scale, s32 arg2);
void func_802794A4(void);
void func_802BCC10(void);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8AEE0/func_802D22F4.s")

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
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

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
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

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
