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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B0DA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDF10[]; /* this vehicle's state block */

/* Exit check for vehicle type 2 (called from func_8024B4B8 in hd.c, which
 * declares it void but returns the leftover v0). Returns 1 when none of the
 * bytes at +0x96, +0x97, +0x98 equals 1 and the byte at +0xA1 is 0, else 0.
 * Returns s32: the asm leaves the full 0/1 in v0. Same shape as
 * func_802B2EF8/func_802B45FC (6E200), func_802B5F04 (71140) and
 * func_802B76F8 (72B80), which lack the +0xA1 test. */
s32 func_802B1150(void) {
    if (D_803EDF10[0x96] == 1 || D_803EDF10[0x97] == 1 || D_803EDF10[0x98] == 1) {
        return 0;
    }
    if (D_803EDF10[0xA1] != 0) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B1150.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u64 *D_803EDFC8;
extern u64 *D_803EDFCC;
extern void *D_803EDFD0; /* engine sound handle */
extern u8 D_803EDC10[];  /* this vehicle's animation channel table */
void func_802A7764(u64 *a, u64 *b, s32 size);
void func_802A02E4(s32 idx, void *base);
void func_802C444C(void);
void func_802608C8(void *arg0);

/* Vehicle-type 2 shutdown (called from hd.c's func_8024B188): zeroes the
 * speed (s16 at +0x76), func_802A7764(D_803EDFC8, D_803EDFCC, 0x1400), stops
 * channel 31 of D_803EDC10, func_802C444C(), then stops the engine sound
 * D_803EDFD0 (func_802608C8). The asm leaves v1 = &D_803EDC10 only if
 * func_802608C8 (C) happens to keep it; the C caller doesn't use it. */
void func_802B11B8(void) {
    *(s16 *) (D_803EDF10 + 0x76) = 0;
    func_802A7764(D_803EDFC8, D_803EDFCC, 0x1400);
    func_802A02E4(0x1F, D_803EDC10);
    func_802C444C();
    func_802608C8(D_803EDFD0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B11B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDF10[]; /* this vehicle's state block */
extern s16 D_8036444C;
extern s16 D_80364450;
extern void *D_80367738;
extern void *D_803EDFD0; /* engine sound handle */
extern u8 D_802C2308[]; /* key of this vehicle's func_802A06B4 entry */
extern u8 D_803EDC10[]; /* this vehicle's animation channel table */
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_802A039C(void *base, s32 idx, s32 val);
void func_802A040C(void *base, s32 idx, s32 val);
void func_802A0480(f32 f, void *base, s32 idx, s32 val);
void func_802A05D0(s32 key, s32 val);
void func_802A05F8(s32 key, s32 val);
void func_802A0620(s32 key, s32 val);

/* Vehicle-type 2 setup (called from 00000.c / 17210.c): clears byte 0x99 of
 * the state block, D_8036444C/50 = 4200, 3000, starts the engine sound 0x50
 * (handle in D_803EDFD0), sets the D_802C2308 entry's fields (25, 0, 0) and
 * the animation channels 7, 8, 9 (and 1, 5, 3) of D_803EDC10.
 * The asm also points $gp at D_803EDF10 and leaves it there (conventions.txt:
 * clobbers gp) and returns with v1 = 0 (the last setter's v1); C callers
 * ignore both. */
void func_802B1228(void) {
    D_803EDF10[0x99] = 0;
    D_8036444C = 0x1068;
    D_80364450 = 0xBB8;
    func_80260650(D_80367738, 0x50, &D_803EDFD0);
    func_802A05D0((s32) D_802C2308, 0x19);
    func_802A05F8((s32) D_802C2308, 0);
    func_802A0620((s32) D_802C2308, 0);
    func_802A039C(D_803EDC10, 7, 2);
    func_802A040C(D_803EDC10, 7, 1);
    func_802A0480(0.5f, D_803EDC10, 7, 1);
    func_802A039C(D_803EDC10, 8, 2);
    func_802A040C(D_803EDC10, 8, 1);
    func_802A0480(0.5f, D_803EDC10, 8, 1);
    func_802A039C(D_803EDC10, 9, 4);
    func_802A040C(D_803EDC10, 9, 1);
    func_802A0480(0.5f, D_803EDC10, 9, 1);
    func_802A0480(0.5f, D_803EDC10, 1, 1);
    func_802A0480(0.5f, D_803EDC10, 5, 1);
    func_802A0480(0.0f, D_803EDC10, 3, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B1228.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B13D8.s")

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
extern u32 D_803EDFB8[]; /* x, y, z */
/* Zone level lookup (func_802ABD54) for vehicle id 2 at its position
 * D_803EDFB8..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802B152C keeps a0, f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802B14E8(ZoneScanRegs *r) {
    return func_802ABD54(2, D_803EDFB8[0], D_803EDFB8[1], D_803EDFB8[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B14E8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B152C.s")

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
extern s16 D_803EDFD6;    /* last pose byte seen on channel 1/5 */
extern u8 D_803EDFDA;
extern u8 D_803F7804;
extern void *D_803F7844;  /* sound handle */
extern f32 D_8030D8B0;
extern u64 D_803649D8;    /* frame counter */
extern u8 D_80370C1A;
extern u8 D_80370C1B;
extern u8 D_80370C1C;
extern u8 D_80370C1D;
extern u8 D_80370C35;
extern u8 D_802C2984[];   /* func_802A6274 definition */
void func_802A0290(void *base, s32 idx, s32 val);
void func_802A03D4(void *base, s32 idx, s32 val);
void func_802A0360(f32 f, void *base, s32 idx, s32 val);
void func_802A04BC(s32 idx, void *base, s32 *out);
void func_8029F9D4(s32 a, s32 b, void *base);
s32 func_8026A8E0(s32 lo, s32 hi);
void func_80278EB0(s32 n, f32 scale, s32 arg2);
void func_802794A4(void);
void func_802BCC10(void);

#define VEH2_SPEED (*(s16 *) (D_803EDF10 + 0x76))

/* Channel idx's field 0x10 ((s8), func_802A04BC's v1); out gets the rest. */
static s32 veh2_ch_get(s32 idx, s32 *out) {
    func_802A04BC(idx, D_803EDC10, out);
    return out[0];
}

/* Channel 31 (the blend track) starts: 039C = v, 03D4 = 0, 040C = 0, 0290 = 1. */
static void veh2_ch31_start(s32 v) {
    func_802A039C(D_803EDC10, 0x1F, v);
    func_802A03D4(D_803EDC10, 0x1F, 0);
    func_802A040C(D_803EDC10, 0x1F, 0);
    func_802A0290(D_803EDC10, 0x1F, 1);
}

/* Channel ch: 03D4 = (speed < 0), 039C = |speed| / 24. */
static void veh2_ch_speed(s32 ch) {
    s32 speed = VEH2_SPEED;

    func_802A03D4(D_803EDC10, ch, speed < 0);
    if (speed < 0) {
        speed = -speed;
    }
    func_802A039C(D_803EDC10, ch, (u32) speed / 24);
}

/* State 0 (+0xA1 == 0): driving. */
static void veh2_state0(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 o[8];
    s32 r;
    s32 speed;

    /* the pose byte of channel 1 (or else 5) when it's on: 6 -> sound 0x4E, 2 -> 0x4F on a change */
    if (veh2_ch_get(1, o) == 1 || veh2_ch_get(5, o) == 1) {
        r = D_803EDFD6;
        D_803EDFD6 = o[6];
        if (o[6] != r) {
            if (o[6] == 6) {
                func_80260650(D_80367738, 0x4E, NULL);
            } else if (o[6] == 2) {
                func_80260650(D_80367738, 0x4F, NULL);
            }
        }
    }

    if (VEH2_SPEED == 0) {
        /* standing: blend back to an idle pose once (+0xA3), then idle anims at random */
        if (D_803EDF10[0xA3] != 0) {
            func_802A0360(0.0f, D_803EDC10, 7, 0);
            if (veh2_ch_get(0x1F, o) != 0) {
                func_802A02E4(0x1F, D_803EDC10);
                func_8029F9D4(0x1F, 7, D_803EDC10);
            } else if (veh2_ch_get(1, o) != 0) {
                func_802A02E4(1, D_803EDC10);
                func_8029F9D4(1, 7, D_803EDC10);
            } else if (veh2_ch_get(5, o) != 0) {
                func_802A02E4(5, D_803EDC10);
                func_8029F9D4(5, 7, D_803EDC10);
            } else {
                func_802A0360(0.0f, D_803EDC10, 1, 0);
                func_8029F9D4(1, 7, D_803EDC10);
            }
            veh2_ch31_start(0x1E);
        }
        D_803EDF10[0xA3] = 0;
        if (veh2_ch_get(0x1F, o) == 1 || veh2_ch_get(7, o) == 1 || veh2_ch_get(8, o) == 1 ||
            veh2_ch_get(9, o) == 1) {
            return;
        }
        if (func_8026A8E0(0, 30) != 0) {
            return;
        }
        r = func_8026A8E0(0, 2);
        r = (r == 0) ? 7 : (r == 1) ? 8 : 9;
        func_802A0360(0.0f, D_803EDC10, r, 0);
        func_802A0290(D_803EDC10, r, 1);
        return;
    }

    /* moving: blend from the idle pose once (+0xA3 == 1 afterwards) */
    if (D_803EDF10[0xA3] != 1) {
        func_802A0360(0.0f, D_803EDC10, 1, 0);
        if (veh2_ch_get(7, o) != 0) {
            func_8029F9D4(7, 1, D_803EDC10);
            func_802A02E4(7, D_803EDC10);
        } else if (veh2_ch_get(8, o) != 0) {
            func_8029F9D4(8, 1, D_803EDC10);
            func_802A02E4(8, D_803EDC10);
        } else if (veh2_ch_get(9, o) != 0) {
            func_8029F9D4(9, 1, D_803EDC10);
            func_802A02E4(9, D_803EDC10);
        } else {
            func_802A0360(0.0f, D_803EDC10, 7, 0);
            func_8029F9D4(7, 1, D_803EDC10);
        }
        veh2_ch31_start(0x28);
    }
    D_803EDF10[0xA3] = 1;
    if (veh2_ch_get(0x1F, o) == 1) {
        return;
    }
    D_803F7804 = 0;
    if ((D_80370C35 == 0 && (D_80370C1C != 0 || D_80370C1D != 0)) || D_80370C1A != 0 || D_80370C1B != 0) {
        if (VEH2_SPEED >= 0x96) {
            /* fast with a button held: switch to state 1 */
            D_803EDF10[0xA1] = 1;
            func_802A0360(0.0f, D_803EDC10, 2, 0);
            if (D_803EDF10[0xA2] == 0) {
                func_8029F9D4(1, 2, D_803EDC10);
                func_802A02E4(1, D_803EDC10);
            } else if (D_803EDF10[0xA2] == 1) {
                func_8029F9D4(5, 2, D_803EDC10);
                func_802A02E4(5, D_803EDC10);
            } else {
                return; /* asm: syscall (+0xA2 is only ever 0 or 1) */
            }
            veh2_ch31_start(0x23);
            func_80278EB0(6, D_8030D8B0, 100);
            return;
        }
    }

    if ((((u32) D_803649D8 >> 8) & 0x2F) == 0) {
        io.a3 = 1;
        io.t6 = t6;
        io.s1 = s1;
        func_802A6274(&io, D_802C2984, 0xEA60, 1, 2, 2, 0, t7, s0, s2, s3, s4, 1);
    }

    /* drive anim on channel 1 (+0xA2 == 0) or 5 (+0xA2 == 1), switching at |speed| 120 */
    if (D_803EDF10[0xA2] == 0) {
        goto ch1;
    }
    if (D_803EDF10[0xA2] != 1) {
        return; /* asm: syscall */
    }
ch5:
    if (veh2_ch_get(5, o) != 0) {
        veh2_ch_speed(5);
        return;
    }
    goto pick;
ch1:
    if (veh2_ch_get(1, o) != 0) {
        veh2_ch_speed(1);
        return;
    }
pick:
    speed = VEH2_SPEED;
    if (speed < 0) {
        speed = -speed;
    }
    if (speed >= 0x78) {
        func_802A0290(D_803EDC10, 5, o[1] != 0 ? 2 : 1);
        func_802A0360(0.0f, D_803EDC10, 5, 0);
        D_803EDF10[0xA2] = 1;
        goto ch5;
    }
    func_802A0290(D_803EDC10, 1, o[1] != 0 ? 2 : 1);
    func_802A0360(0.0f, D_803EDC10, 1, 0);
    D_803EDF10[0xA2] = 0;
    goto ch1;
}

/* Back to state 0 from state 1/2 (the shared tail of both). */
static void veh2_to_state0_tail(void) {
    veh2_ch31_start(0x32);
    func_802794A4();
    D_803EDF10[0xA1] = 0;
}

/* Vehicle type 2 animation/state update ($gp = D_803EDF10, read as the
 * global), a state machine on the byte at +0xA1:
 * 0: driving (veh2_state0): pose-change sounds, idle/drive blends on the
 *    channels of D_803EDC10, a func_802A6274 record every 0x2F-masked frame,
 *    and the switch to state 1 when fast with a button held.
 * 1: starts sound 0x51 (handle D_803F7844) once; +0x9C set: speed halves and
 *    state 3; else +0x9D or D_803EDFDA set: back to 0; else speed 446 and,
 *    once channel 31 is done, channel 2 plays and state 2.
 * 2: as 1 without the sound and with channel 2 blended away.
 * 3: stops the sound, func_802A6274 (data 0x222E0 at (2, 1, 1)),
 *    func_802BCC10, then once channel 31 is done channel 3 plays, state 4.
 * 4: same record, then once channel 3 is done back to state 0.
 * Other +0xA1 / +0xA2 values hit a `syscall` in the asm (never happens in
 * the game); this returns instead.
 * Register convention: t6, t7, s0-s4 pass through to func_802A6274 (t6 and
 * s1 in its in/out block); s1, s5 change, and f20/f30 are left as
 * func_8029F9D4 leaves them (conventions.txt: clobbers). The asm caller
 * func_802B152C reads f20 afterwards and the asm expects func_80260650 (C)
 * to keep t6/t7 for the following func_802A6274; a mixed N64 build would
 * need a thunk, the native port doesn't. */
void func_802B18F4(s32 t6, s32 t7, s32 s0, s32 s1, s32 s2, s32 s3, s32 s4) {
    Io802A6274 io;
    s32 o[8];

    switch (D_803EDF10[0xA1]) {
        case 0:
            veh2_state0(t6, t7, s0, s1, s2, s3, s4);
            return;

        case 1:
            if (D_803F7844 == NULL) {
                func_80260650(D_80367738, 0x51, &D_803F7844);
            }
            if (D_803EDF10[0x9C] != 0) {
                VEH2_SPEED = VEH2_SPEED >> 1;
                func_802794A4();
                D_803EDF10[0xA1] = 3;
                func_802A0360(0.0f, D_803EDC10, 3, 3);
                func_8029F9D4(0x1F, 3, D_803EDC10);
                veh2_ch31_start(0x21);
                D_803F7804 = 1;
                return;
            }
            if (D_803EDF10[0x9D] != 0 || D_803EDFDA != 0) {
                if (VEH2_SPEED >= 0) {
                    VEH2_SPEED = 0x3C;
                }
                func_802C444C();
                func_802A0360(0.0f, D_803EDC10, 1, 0);
                func_8029F9D4(0x1F, 1, D_803EDC10);
                veh2_to_state0_tail();
                return;
            }
            VEH2_SPEED = 0x1BE;
            if (veh2_ch_get(0x1F, o) == 1) {
                return;
            }
            func_802A039C(D_803EDC10, 2, 10);
            func_802A03D4(D_803EDC10, 2, 0);
            func_802A040C(D_803EDC10, 2, 0);
            func_802A0290(D_803EDC10, 2, 1);
            D_803EDF10[0xA1] = 2;
            return;

        case 2:
            if (D_803EDF10[0x9C] != 0) {
                VEH2_SPEED = VEH2_SPEED >> 1;
                D_803EDF10[0xA1] = 3;
                func_802A0360(0.0f, D_803EDC10, 3, 3);
                func_8029F9D4(2, 3, D_803EDC10);
                func_802A02E4(2, D_803EDC10);
                veh2_ch31_start(0x21);
                D_803F7804 = 1;
                return;
            }
            if (D_803EDF10[0x9D] == 0 && D_803EDFDA == 0) {
                VEH2_SPEED = 0x1BE;
                if (veh2_ch_get(2, o) == 1) {
                    return;
                }
            }
            func_802C444C();
            if (VEH2_SPEED >= 0) {
                VEH2_SPEED = 0x3C;
            }
            func_802A0360(0.0f, D_803EDC10, 1, 0);
            func_802A02E4(2, D_803EDC10);
            func_8029F9D4(2, 1, D_803EDC10);
            veh2_to_state0_tail();
            return;

        case 3:
            if (D_803F7844 != NULL) {
                func_802C444C();
                func_80260650(D_80367738, 0x4B, NULL);
            }
            io.a3 = 1;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2984, 0x222E0, 1, 2, 1, 1, t7, s0, s2, s3, s4, 1);
            func_802BCC10();
            if (veh2_ch_get(0x1F, o) == 1) {
                return;
            }
            D_803F7804 = 0;
            func_802794A4();
            func_802A039C(D_803EDC10, 3, 8);
            func_802A03D4(D_803EDC10, 3, 0);
            func_802A040C(D_803EDC10, 3, 0);
            func_802A0290(D_803EDC10, 3, 1);
            D_803EDF10[0xA1] = 4;
            return;

        case 4:
            io.a3 = 1;
            io.t6 = t6;
            io.s1 = s1;
            func_802A6274(&io, D_802C2984, 0x222E0, 1, 2, 1, 1, t7, s0, s2, s3, s4, 1);
            func_802BCC10();
            if (veh2_ch_get(3, o) == 1) {
                return;
            }
            D_803F7804 = 1;
            func_802A0360(0.0f, D_803EDC10, 1, 0);
            func_802A0360(0.0f, D_803EDC10, 5, 0);
            D_803EDF10[0xA1] = 0;
            return;

        default:
            return; /* asm: syscall */
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B18F4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2768.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* 0 when the speed (s16 at +0x76 of D_803EDF10, the asm's $gp) is 0, else 50
 * when the byte at +0xA1 is 1 or 2, else 120. The asm returns it in s3 (see
 * tools_port/conventions.txt). Its asm caller func_802B152C keeps a0-a3 live
 * across the call (a mixed N64 build would need a thunk). Same shape as
 * func_802D2444 (8AEE0). */
s32 func_802B28B8(void) {
    u8 t;

    if (*(s16 *) (D_803EDF10 + 0x76) == 0) {
        return 0;
    }
    t = D_803EDF10[0xA1];
    if (t == 2 || t == 1) {
        return 50;
    }
    return 120;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B28B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern f32 D_803EBBF0;
extern f32 D_803EBBF4;
extern u8 D_803ED3F6;
extern u8 D_803ED3F7;

/* Vehicle-module setup leaf, identical to func_802AFBA0 (69BB0):
 * D_803EBBF4 = D_803EBBF0 * 4, D_803ED3F6/7 = 40, 3.
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802B152C keeps a0-a3 live across the call; a mixed N64 build would
 * need a thunk preserving those, the native port does not. */
void func_802B2900(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2900.s")
#endif

/* func_802B295C: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803EDF10[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */
extern u32 D_803EDFB8[]; /* plus these three words */
s32 func_802AC7DC(u8 *dst, u8 *src, u32 *words);
void func_802AC85C(u8 *src, u8 *dst, u32 *words);

/* Serialize this vehicle's state (D_803EDF10[0..0xA5] plus the three words
 * D_803EDFB8[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802B295C(u8 *dst) {
    return func_802AC7DC(dst, D_803EDF10, D_803EDFB8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B295C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802B295C: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802B2988(void *)`. */
void func_802B2988(void *src) {
    func_802AC85C(src, D_803EDF10, D_803EDFB8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/6C5E0/func_802B2988.s")
#endif
