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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B29C0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE2E0[]; /* vehicle type 3's state block */
extern u8 D_803EDFE0[]; /* its animation channel table */
extern u8 D_802C2314[]; /* key of its func_802A05D0.. sound entry */
extern s16 D_8036444C;
extern s16 D_80364450;
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0508(s32 key, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);
void func_802C4310(s32 arg0, s32 arg1);

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
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802C444C(void);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B2FA0.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B30F4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3180.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B327C.s")

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
extern u8 D_803EE3AE;  /* func_802A6274 spawn cooldown */
extern u8 D_803EE3AF;  /* 0..100 level for channel 2 */
extern s8 D_803EE3B1;  /* 0..100 boost meter */
extern u8 D_803EE3B2;  /* boost countdown */
extern u8 D_803EE3B3;  /* boost sound playing */
extern s16 D_803EE3AC;
extern f32 D_803EE3A4; /* 0..1 level for channel 3 */
extern void *D_803EE388; /* boost sound handle */
extern void *D_80367738;
extern f32 D_8030D8C4;
extern f32 D_8030D8C8;
extern f32 D_8030D8CC;
extern u8 D_80370C15;
extern u8 D_80370C16;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C23;
extern u8 D_802C2954[]; /* func_802A6274 definitions */
extern u8 D_802C37C0[];
s32 func_802A5ED0(void);
s32 func_802A7CB0(u8 *veh, s32 range);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A05A4(f32 f, s32 key, s32 val);
void func_802C4584(s32 level);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802608C8(void *arg0);
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
    } else if ((D_80370C1A != 0 || D_80370C1B != 0) && D_803EE3B1 >= 4) {
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
    v = D_803EE3B1 + 1;
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
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B3E40.s")

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
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;
extern u8 D_80370C1C;
extern u8 D_80370C23; /* B or Z held */
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
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4100.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EE6C0[]; /* vehicle type 4's state block */
extern u8 D_803EE3C0[]; /* its animation channel table */
extern u8 D_802C2190[]; /* keys of its three sound entries */
extern u8 D_802C21A4[];
extern u8 D_802C21B8[];

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B46C4.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4818.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B48A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B49AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B4EF8.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6E200/func_802B568C.s")

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
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;
extern u8 D_80370C23; /* B or Z held */
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
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

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
