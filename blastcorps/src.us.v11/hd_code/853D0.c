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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9B90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8AA0[]; /* this vehicle's state block (the asm's $gp) */
extern u8 D_803F87A0[]; /* its animation channel table (Unk8029DEA0Entry, 56040.c) */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802C4310(s32 arg0, s32 arg1);
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);

/* Enter this vehicle (vehicle type 10; called from hd.c and 17210.c): clears
 * the byte at +0x99, sets the pair D_8036444C / D_80364450 to (0xD48, 1000),
 * starts sound 0x94 (func_802C4310; the asm passes its caller's a0 through
 * as the ignored first argument) and sets up channels 1-3 (0 / 0 / 0|1,
 * restart with -1) and 4-5 (8|8 / 0 / 0, 1.0 with 1).
 * The asm points $gp at D_803F8AA0 and leaves it there (conventions.txt:
 * clobbers gp) and leaves v1 = 1 (func_802A0480 preserves it); the C callers
 * declare it void and use neither. Same shape as func_802C5714 (7FB50). */
void func_802C9F54(void) {
    D_803F8AA0[0x99] = 0;
    D_8036444C = 0xD48;
    D_80364450 = 1000;
    func_802C4310(0, 0x94);
    func_802A039C(D_803F87A0, 1, 0);
    func_802A03D4(D_803F87A0, 1, 0);
    func_802A040C(D_803F87A0, 1, 0);
    func_802A0290(D_803F87A0, 1, -1);
    func_802A039C(D_803F87A0, 2, 0);
    func_802A03D4(D_803F87A0, 2, 0);
    func_802A040C(D_803F87A0, 2, 1);
    func_802A0290(D_803F87A0, 2, -1);
    func_802A039C(D_803F87A0, 3, 0);
    func_802A03D4(D_803F87A0, 3, 0);
    func_802A040C(D_803F87A0, 3, 1);
    func_802A0290(D_803F87A0, 3, -1);
    func_802A039C(D_803F87A0, 4, 8);
    func_802A03D4(D_803F87A0, 4, 0);
    func_802A040C(D_803F87A0, 4, 0);
    func_802A0480(1.0f, D_803F87A0, 4, 1);
    func_802A039C(D_803F87A0, 5, 8);
    func_802A03D4(D_803F87A0, 5, 0);
    func_802A040C(D_803F87A0, 5, 0);
    func_802A0480(1.0f, D_803F87A0, 5, 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802C9F54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Vehicle-type 10
 * "can exit" check: 0 if any of the vehicle's bytes 0x96..0x98 equals 1 or
 * D_803F8B7C (= its byte 0xDC) is nonzero, else 1. Same shape as
 * func_802CBB60, func_802CCCD8, func_802CFA58, func_802D0B90. 00000.c
 * declares it void and ignores the result, but the asm returns 0/1 in v0. */
extern u8 D_803F8AA0[];
extern u8 D_803F8B7C;

s32 func_802CA140(void) {
    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1 || D_803F8B7C != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA140.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803F8B58; /* save copy pair (func_802A7764) */
extern u64 *D_803F8B5C;
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

/* Teardown for this vehicle (called from func_8024B188 in hd.c): clears the
 * s16 at +0x76, func_802A7764(D_803F8B58, D_803F8B5C, 0x700) and stops its
 * sounds (func_802C444C). The asm saves and restores $gp. Same shape as
 * func_802C5688 (7FB50). */
void func_802CA1AC(void) {
    *(s16 *) (D_803F8AA0 + 0x76) = 0;
    func_802A7764(D_803F8B58, D_803F8B5C, 0x700);
    func_802C444C();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA1F8.s")

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
extern u32 D_803F8B48[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 0xA at its position
 * D_803F8B48..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802CA4E0 keeps f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802CA308(ZoneScanRegs *r) {
    return func_802ABD54(0xA, D_803F8B48[0], D_803F8B48[1], D_803F8B48[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA308.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA34C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA3D8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CA4E0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef CVT_W_S
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
#endif
#ifndef IO_802A6274_DEFINED
#define IO_802A6274_DEFINED
/* func_802A6274's in/out registers (60F60.c). */
typedef struct {
    s32 a3;
    s32 t6;
    s32 s1;
} Io802A6274;
#endif
extern f32 D_803F8B60; /* engine/tilt animation value (0.5 = centred) */
extern s32 D_803F8B64; /* jump: a (initial speed) */
extern s32 D_803F8B68; /* jump: n (frame) */
extern s32 D_803F8B6C; /* jump: base height */
extern u16 D_803F8B70; /* jump: boost frames */
extern u16 D_803F8B72; /* horn presses left */
extern u8 D_803F8B76;  /* record cooldown */
extern u8 D_803F8B7D;  /* horn cooldown */
extern u8 D_803F8B7E;  /* horn side toggle */
extern u8 D_80370C15;  /* inputs */
extern u8 D_80370C16;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern s8 D_80370C2D;
extern void *D_80367738;
extern f32 D_8030D994;
extern f32 D_8030D998;
extern f32 D_8030D99C;
extern f32 D_8030D9A0;
extern u8 D_802C2954[]; /* definition handed to func_802A6274 */
void *func_80260650(void *arg0, s16 arg1, void *arg2);
s32 func_80292288(s16 speed, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, u8 type, s16 arg8);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
s32 func_802A5ED0(void);
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35);
s32 func_802A7CB0(u8 *veh, s32 range);
s32 func_802ABC88(s32 id, s32 n, u8 **recOut);
void func_802C4584(s32 level);
void func_802CB224(void);
f32 func_802CB3C8(s32 side);

/* a * n + round(-16 * n * n): the jump height after n frames at initial
 * speed a (32-bit products, cvt.w.s rounding; the asm's sum is a trapping
 * `add`). */
#define JUMP_HEIGHT(out, a, n)                    \
    do {                                          \
        s32 _n = (n);                             \
        s32 _q;                                   \
                                                  \
        CVT_W_S(_q, -16.0f * (f32) (_n * _n));    \
        (out) = (a) * _n + _q;                    \
    } while (0)

/* Per-frame effects of vehicle type 10 ($gp = D_803F8AA0, read as the
 * global): with +0x99 set and no cooldown (D_803F8B76), and fewer than 4
 * active D_803C4B70 records (func_802A5ED0), sets up a func_802A6274 record
 * (def D_802C2954, data 0x30D40, type 1 at (10, 1, 1), the caller's t6, t7,
 * s0-s4 passed through); channel 1 gets the speed sign and |speed| / 6, the
 * engine sound |speed| >> 3 (func_802C4584), then the trail (func_802CB224).
 * D_803F8B60 steers towards func_802CB3C8's limit while D_80370C15 /
 * D_80370C16 is held (else back to 0.5) and drives channel 3. The horn
 * (D_80370C1A/1B, D_803F8B72 presses, cooldown D_803F8B7D = 5) alternates
 * channels 4 / 5 and fires func_80292288 from record (10, 3|1) to (10, 4|2)
 * at speed |speed| * 320 / 260 + 640. A jump (D_80370C2D >= 60, not near a
 * band bound per func_802A7CB0) starts or boosts the parabola D_803F8B64 /
 * 68 / 6C; while it is active channel 2 gets height / D_8030D9A0 (at most
 * 1.0), and on landing it bounces (when the impact is >= 61 and no flag
 * +0x96..0x98 is 1) or ends.
 * Register convention (conventions.txt): t6, t7, s0-s4 come in (from the
 * caller's func_802CA308 zone lookup) and only pass through to
 * func_802A6274; the asm changes s1-s7. Integer counters/sums use trapping
 * `addi`/`add`/`sub` in the asm: keep inputs in game range. */
void func_802CAAFC(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 speed;
    s32 cur;
    s32 prev;
    s32 h;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 *rec;
    f32 f;
    f32 lim;

    if (D_803F8B76 != 0) {
        D_803F8B76--;
    } else if (D_803F8AA0[0x99] != 0) {
        D_803F8B76 = 1;
        if (func_802A5ED0() < 4) {
            io.a3 = 0;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2954, 0x30D40, 1, 10, 1, 1, t7, s0, s2, s3, s4, 1);
        }
    }

    speed = *(s16 *) (D_803F8AA0 + 0x76);
    func_802A03D4(D_803F87A0, 1, (speed < 0) ? 1 : 0);
    if (speed < 0) {
        speed = -speed;
    }
    func_802A039C(D_803F87A0, 1, (u32) speed / 6);
    speed = *(s16 *) (D_803F8AA0 + 0x76);
    if (speed < 0) {
        speed = -speed;
    }
    func_802C4584((u32) speed >> 3);
    func_802CB224();

    f = D_803F8B60;
    if (D_80370C15 != 0) {
        f -= D_8030D994;
        lim = func_802CB3C8(0);
        if (f < lim) {
            f = lim;
        }
    } else if (D_80370C16 != 0) {
        f += D_8030D998;
        lim = func_802CB3C8(1);
        if (!(f <= lim)) {
            f = lim;
        }
    } else if (f < 0.5f) {
        f += D_8030D99C;
        if (!(f <= 0.5f)) {
            f = 0.5f;
        }
    } else {
        f -= D_8030D99C;
        if (f < 0.5f) {
            f = 0.5f;
        }
    }
    D_803F8B60 = f;
    func_802A0360(f, D_803F87A0, 3, 0);

    if (D_803F8B7D != 0) {
        D_803F8B7D--;
    } else if ((D_80370C1A != 0 || D_80370C1B != 0) && D_803F8B72 != 0) {
        D_803F8B72--;
        func_80260650(D_80367738, 3, NULL);
        D_803F8B7E ^= 1;
        if (D_803F8B7E != 0) {
            func_802A0360(0.0f, D_803F87A0, 4, 0);
            func_802A0290(D_803F87A0, 4, 1);
            func_802ABC88(10, 3, (u8 **) &rec);
            x1 = rec[0];
            y1 = rec[1];
            z1 = rec[2];
            func_802ABC88(10, 4, (u8 **) &rec);
        } else {
            func_802A0360(0.0f, D_803F87A0, 5, 0);
            func_802A0290(D_803F87A0, 5, 1);
            func_802ABC88(10, 1, (u8 **) &rec);
            x1 = rec[0];
            y1 = rec[1];
            z1 = rec[2];
            func_802ABC88(10, 2, (u8 **) &rec);
        }
        speed = *(s16 *) (D_803F8AA0 + 0x76);
        if (speed < 0) {
            speed = -speed;
        }
        /* the asm passes the whole word; the callee takes an s16 */
        func_80292288((u32) (speed * 0x140) / 0x104 + 0x280, rec[0], rec[1], rec[2], x1, y1, z1, 0, 0x1A4);
        D_803F8B7D = 5;
    }

    if (func_802A7CB0(D_803F8AA0, 10) == 0 && D_80370C2D >= 0x3C) {
        if (D_803F8B7C == 0) {
            /* take off */
            D_803F8B7C = 1;
            D_803F8B6C = 0;
            D_803F8B68 = 1;
            D_803F8B64 = 0x3C;
            D_803F8B70 = 0;
        } else if (D_803F8B70 < 0x17) {
            /* boost: rebase the parabola at the current height */
            D_803F8B70++;
            JUMP_HEIGHT(cur, D_803F8B64, D_803F8B68);
            D_803F8B6C += cur;
            JUMP_HEIGHT(prev, D_803F8B64, D_803F8B68 - 1);
            D_803F8B64 = cur - prev + 0x33;
            D_803F8B68 = 1;
        }
    }

    if (D_803F8B7C != 0) {
        JUMP_HEIGHT(cur, D_803F8B64, D_803F8B68);
        h = D_803F8B6C + cur;
        if (h <= 0) {
            /* landed: bounce with half the impact speed, or stop */
            JUMP_HEIGHT(prev, D_803F8B64, D_803F8B68 - 1);
            prev -= cur;
            if (prev < 0) {
                prev = -prev;
            }
            if (prev < 0x3D || D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1) {
                goto stop;
            }
            D_803F8B6C = 0;
            D_803F8B64 = (u32) prev / 2;
            D_803F8B68 = 1;
            h = 0;
        }
        f = (f32) h / D_8030D9A0;
        if (!(f <= 1.0f)) {
            f = 1.0f;
        }
        func_802A0360(f, D_803F87A0, 2, 0);
        D_803F8B68++;
        return;
    }
stop:
    D_803F8B7C = 0;
    func_802A0360(0.0f, D_803F87A0, 2, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CAAFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803F8B48[]; /* [0], [2]: trail x, z */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

/* Trail (shape of func_802B3C68 in 6E200; $gp = D_803F8AA0): if the byte at
 * +0x99 is set, +0x98 isn't 1, +0x50 < 3 and +0x9B is clear,
 * func_8027BE7C(3, y, 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2], yaw,
 * 5, 40, 0, 1): a single centred track.
 * Register note: the asm saves and restores every integer register; asm
 * caller func_802CAAFC keeps a1-a3 live (a mixed N64 build would need a
 * thunk). */
void func_802CB224(void) {
    if (D_803F8AA0[0x99] != 0 && D_803F8AA0[0x98] != 1 && D_803F8AA0[0x50] < 3 && D_803F8AA0[0x9B] == 0) {
        func_8027BE7C(3, *(s32 *) (D_803F8AA0 + 0x1C), 0, -800, 0, -800, D_803F8B48[0], D_803F8B48[2],
                      *(u16 *) (D_803F8AA0 + 0x4E), 5, 40, 0, 1);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB224.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0.5 +- |speed| / 280 * 0.5 (plus when side != 0, minus otherwise), speed
 * being the s16 at +0x76 of D_803F8AA0 (the asm's $gp). The asm takes side
 * in s2, returns the value in f2 and leaves s3 = 280 (see
 * tools_port/conventions.txt). Its asm caller func_802CAAFC keeps a1-a3
 * live (a mixed N64 build would need a thunk). Same shape as func_802B71DC
 * (71140). */
f32 func_802CB3C8(s32 side) {
    s32 speed = *(s16 *) (D_803F8AA0 + 0x76);
    f32 v;

    if (speed < 0) {
        speed = -speed;
    }
    v = (f32) speed / 280.0f * 0.5f;
    if (side != 0) {
        return v + 0.5f;
    }
    return 0.5f - v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB3C8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB42C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
#ifndef CVT_W_S
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
#endif

extern f32 D_8030D9A4;

/* Speed (s16 at +0x76 of D_803F8AA0, the asm's $gp) / 6.0 when any of the
 * bytes at +0x96/+0x97/+0x98 is 1, else / D_8030D9A4, rounded to nearest.
 * The asm returns it in s3 (see tools_port/conventions.txt). Its asm caller
 * func_802CA4E0 keeps a0-a3 live (a mixed N64 build would need a thunk).
 * Same shape as func_802B0C74 (6B4A0). */
s32 func_802CB564(void) {
    f32 div;
    s32 r;

    if (D_803F8AA0[0x96] == 1 || D_803F8AA0[0x97] == 1 || D_803F8AA0[0x98] == 1) {
        div = 6.0f;
    } else {
        div = D_8030D9A4;
    }
    CVT_W_S(r, *(s16 *) (D_803F8AA0 + 0x76) / div);
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB564.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Functional rewrite (tools_port/eqcheck.py verified). Same "set timers"
 * shape as func_802BAD24 (75490.c). Asm callers rely on preserved registers:
 * func_802CA3D8 on t3, t4, f12, f14; func_802CA4E0 on a0-a3 (mixed N64 build
 * would need a thunk). */
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

void func_802CB5D8(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 60;
    D_803ED3F7 = 2;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB5D8.s")
#endif

/* func_802CB634: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F8AA0[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803F8B48[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803F8AA0[0..0xA5] plus the three words
 * D_803F8B48[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802CB634(u8 *dst) {
    return func_802AC7DC(dst, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB634.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802CB634: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802CB660(void *)`. */
void func_802CB660(void *src) {
    func_802AC85C(src, D_803F8AA0, D_803F8B48);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/853D0/func_802CB660.s")
#endif
