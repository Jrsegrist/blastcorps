#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#ifdef NON_MATCHING
#define D_802E8BDC (*(u8 *) &D_802E8BDC)
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
extern u8 D_803ED760[];  /* vehicle 0's state block (the asm's $gp) */
extern u8 D_803ED460[];  /* vehicle 0's animation channel table (Unk8029DEA0Entry, 56040.c) */
extern s32 D_803ED81C;   /* drive-in ground height */
extern u8 D_803ED827;    /* drive-in being set up */
extern u8 D_803ED828;    /* drive-in direction: 0 +z, 1 -z, 2 +x, else -x */
extern s32 D_803ED814;   /* drive-in stop coordinate */
extern u8 D_80305CB0[];
extern u8 D_80305CB1[];  /* drive-in table: 5-byte records {level, vehicle, dir, mul, zone}, level -1 ends */
s32 func_802AEB9C(s32 *mode);
s32 func_802AEC3C(s32 dist, s32 key, s32 *fp, TriSideOut *f);
s32 func_8029AA10_fp(s32 kind, s32 fp); /* 56040.c: func_8029AA10 with the asm's fp out */
u8 *func_802AFA64(void);
extern u8 *D_803ED82C;  /* vehicle 0's model buffers */
extern u8 *D_803ED830;
extern s16 D_803ED822;  /* vehicle 0's last ring-steered heading */
extern s8 D_803ED824;   /* ring steering active */
void func_802AF4BC(void);
void func_802AF340(s32 key, s32 fp, TriSideOut *f);
void func_802AFBA0(void);
s32 func_802AFB84(void);
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803ED818; /* vehicle 0's model header */

/* Sets up vehicle 0 (object 0) from the model header `hdr` at (x, y, z) with
 * heading `heading` (called by the object dispatcher func_802A350C):
 * D_803ED818 = hdr, two 0xC80-byte buffers from the heap D_80358070
 * (D_803ED82C, D_803ED830; the cursor moves 0x1900), the object record
 * (func_802A1388(0, 0, buf1, buf2, hdr)), the state block D_803ED760 reset
 * (func_802A754C) with its wheel table +0x52..+0x68 = (+-30, 30) / (+-35, 35)
 * pairs, position D_803ED808..810, headings +0x4C/+0x4E/+0x74, drive-in flags
 * D_803ED825 = 1, D_803ED826/827 = 0, D_803ED824 = 0, +0xA1 = 0; the initial
 * ground under the wheels (func_802A992C, key 0); the animated model
 * (func_8029F85C(buf2, buf1, D_803ED460, hdr)) with channel 0 started and
 * animated into each buffer in turn; the throttle bands +0x78..+0x94; the
 * parts (func_8029C354, tag 0, scale 0x4E20); marker record 0
 * (func_80258230(0, 0x28, 15, 15)); one frame of func_802AEEC8 with byte
 * +0x9A set (no driving); the model matrix copied from the second buffer to
 * the first (func_802AA838); channels 1-4 set up (1: 0, 0, 0 / 0.5, 1; 2-4: 2,
 * 0, 1 / 0.5, 1) and D_8036444C/50 = 0xD48, 0.
 * Register convention (conventions.txt): hdr s2, x t7, y s3, z s0, heading
 * s1; func_802A992C's fp and f12-f26 inputs are the dispatcher's leftovers
 * (here fp, f). The asm saves t0-t5 (asm caller func_802A350C keeps t1, t2
 * live), points $gp at D_803ED760 and leaves s0-s7, fp and the FP registers
 * as its callees leave them. Its addi traps on overflow. Fidelity note: the
 * func_802AEEC8 call inherits leftovers in the asm (fp from func_8029E558's
 * callees etc.); the C version uses its own defaults. */
void func_802AE370(u8 *hdr, s32 x, s32 y, s32 z, s32 heading, s32 fp, TriSideOut *f) {
    s32 *pos = (s32 *) D_803ED808;
    u8 *buf;
    s16 *h;

    D_803ED818 = hdr;
    buf = D_80358070;
    D_803ED82C = buf;
    D_803ED830 = buf + 0xC80;
    D_80358070 = buf + 0x1900;
    func_802A1388(0, 0, (s32) D_803ED82C, (s32) D_803ED830, hdr);
    func_802A754C(D_803ED760);
    h = (s16 *) (D_803ED760 + 0x52);
    h[0] = 30;
    h[1] = 30;
    h[2] = -30;
    h[3] = 30;
    h[4] = 30;
    h[5] = -30;
    h[6] = 35;
    h[7] = 35;
    h[8] = -35;
    h[9] = 35;
    h[10] = 35;
    h[11] = -35;
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    *(s16 *) (D_803ED760 + 0x4C) = heading;
    *(s16 *) (D_803ED760 + 0x4E) = heading;
    *(s16 *) (D_803ED760 + 0x74) = heading;
    D_803ED825 = 1;
    D_803ED826 = 0;
    D_803ED827 = 0;
    D_803ED824 = 0;
    D_803ED760[0xA1] = 0;
    func_802A992C((s16 *) (D_803ED760 + 0x52), pos[1], x, z, (s32 *) (D_803ED760 + 4), &pos[1],
                  (s16 *) (D_803ED760 + 0x4C), 0, fp, D_803ED760, f);
    func_8029F85C((u32 *) D_803ED830, (u32 *) D_803ED82C, (Unk8029DEA0Entry *) D_803ED460, D_803ED818);
    func_802A039C((Unk8029DEA0Entry *) D_803ED460, 0, 0x64);
    func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 0, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803ED460, 0, 0);
    func_802A0480(0.0f, (Unk8029DEA0Entry *) D_803ED460, 0, 0);
    func_802A0290((Unk8029DEA0Entry *) D_803ED460, 0, 1);
    func_8029E558(D_803ED82C, D_803ED830, (Unk8029DEA0Entry *) D_803ED460);
    func_802A0320(0, (Unk8029DEA0Entry *) D_803ED460);
    func_802A0290((Unk8029DEA0Entry *) D_803ED460, 0, 1);
    func_8029E558(D_803ED830, D_803ED82C, (Unk8029DEA0Entry *) D_803ED460);
    h = (s16 *) (D_803ED760 + 0x78);
    h[0] = -0x28;
    h[1] = 0;
    h[2] = 2;
    h[3] = 0;
    h[4] = 0x28;
    h[5] = 2;
    h[6] = 0x28;
    h[7] = 0x3C;
    h[8] = 2;
    h[9] = 0x3C;
    h[10] = 0x50;
    h[11] = 2;
    h[12] = 0x50;
    h[13] = 0x64;
    h[14] = 2;
    hdr = D_803ED818;
    func_8029C354(0, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8), 0x4E20);
    func_80258230(0, 0x28, 0xF, 0xF);
    D_803ED760[0x9A] = 1;
    func_802AEEC8();
    D_803ED760[0x9A] = 0;
    hdr = D_803ED818;
    func_802AA838(D_803ED830, D_803ED82C, *(s32 *) (hdr + *(s32 *) (hdr + 0x18) + 4));
    func_802A039C((Unk8029DEA0Entry *) D_803ED460, 1, 0);
    func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 1, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803ED460, 1, 0);
    func_802A0480(0.5f, (Unk8029DEA0Entry *) D_803ED460, 1, 1);
    func_802A039C((Unk8029DEA0Entry *) D_803ED460, 2, 2);
    func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 2, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803ED460, 2, 1);
    func_802A0480(0.5f, (Unk8029DEA0Entry *) D_803ED460, 2, 1);
    func_802A039C((Unk8029DEA0Entry *) D_803ED460, 3, 2);
    func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 3, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803ED460, 3, 1);
    func_802A0480(0.5f, (Unk8029DEA0Entry *) D_803ED460, 3, 1);
    func_802A039C((Unk8029DEA0Entry *) D_803ED460, 4, 2);
    func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 4, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803ED460, 4, 1);
    func_802A0480(0.5f, (Unk8029DEA0Entry *) D_803ED460, 4, 1);
    D_8036444C = 0xD48;
    D_80364450 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE370.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED460[]; /* animation channel table (Unk8029DEA0Entry, 56040.c) */

/* Stops channel 31 of D_803ED460 (func_802A02E4; called from hd.c's vehicle
 * switch). The asm leaves v0 = 31 and v1 = &D_803ED460; the C caller uses
 * neither. */
void func_802AE860(void) {
    func_802A02E4(0x1F, (Unk8029DEA0Entry *) D_803ED460);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE860.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Drive-in setup check for vehicle 0, called from hd.c (func_8024B188) with
 * the vehicle's distance `dist`; returns 1 when the drive-in starts, else 0
 * (the C caller declares u8; the asm returns the full v0).
 * Unless D_803ED825 is clear (then 0 at once), the first drive-in record of
 * D_80305CB1 (5 bytes: level (s8, -1 ends; compared with the whole word
 * D_802E8BDC), vehicle type, direction, multiplier, zone mode) for this level
 * and D_80364456 - with zone mode 0 or func_802AEB9C(zone mode) true - gives
 * the direction (default 0) and multiplier (default 1). The path from the
 * player along that direction is checked every 100 units up to dist * mul
 * and at dist * mul itself (func_802AEC3C, which also positions the vehicle
 * at each point); any blocked point returns 0. Otherwise the vehicle is
 * reset (flags +0x96..+0x99 and the speed cleared, heading +0x4C/+0x4E/+0x74
 * = 0, 0x800, 0x400 or 0xC00 for direction 0, 1, 2, else), D_803ED814 = the
 * last checked z (directions 0/1) or x, the position = the player's (y + 500
 * for vehicle types 0xB, 0x11, 0x12), D_803ED826 = 1, channel 1 restarted
 * looping, D_8036444C/50 = 0xD48, 0. D_803ED827 is 1 while this runs and
 * D_803F7812 is 1 during the call.
 * Fidelity note: func_802AEC3C's func_802A9A60 inputs come from leftover
 * registers in the asm: key t8 = D_80364456 << 2 (what the C caller
 * func_8024B188 leaves in t8 at the call), reproduced here; fp is the C
 * caller's (0: the game thread's fp) on the first check and then what the
 * previous check's func_8029AA10 left (its part count when vehicle 0's sphere
 * touched the world, else unchanged), threaded through `fp` here; it ends up
 * in D_803ED3F2[0..2] and +0x50. Traced in the original (Oct 2026, level 0,
 * vehicle 4): t8 = 0x10 on every check, fp 0 on the first and 0 or 2 on the
 * later ones. f12-f26 are pass-through FP state (zeros here). The asm's addi
 * traps on overflow. */
s32 func_802AE888(s32 dist) {
    s32 *pos = (s32 *) D_803ED808;
    u8 *rec;
    s32 dir = 0;
    s32 mul = 1;
    s32 mode;
    s32 off;
    s32 key;
    s32 fp = 0;
    s32 ret = 0;
    s32 t;
    TriSideOut f;

    D_803F7812 = 1;
    if (D_803ED825 != 0) {
        for (rec = D_80305CB1; (s8) rec[0] != -1; rec += 5) {
            if ((s8) rec[0] != *(s32 *) &D_802E8BDC || rec[1] != D_80364456) {
                continue;
            }
            if (rec[4] != 0) {
                mode = rec[4];
                if (func_802AEB9C(&mode) == 0) {
                    continue;
                }
            }
            dir = rec[2];
            mul = rec[3];
            break;
        }
        dist = (u32) dist * (u32) mul;
        D_803ED828 = dir;
        D_803ED827 = 1;
        D_803ED81C = D_803643E4;
        key = D_80364456 << 2;
        f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
        for (off = 0; !(dist < off); off += 100) {
            if (func_802AEC3C(off, key, &fp, &f) == 0) {
                goto done;
            }
        }
        if (func_802AEC3C(dist, key, &fp, &f) == 0) {
            goto done;
        }
        D_803ED760[0x96] = 0;
        D_803ED760[0x97] = 0;
        D_803ED760[0x98] = 0;
        D_803ED760[0x99] = 0;
        *(s16 *) (D_803ED760 + 0x76) = 0;
        switch (D_803ED828) {
            case 0:
                t = 0;
                break;
            case 1:
                t = 0x800;
                break;
            case 2:
                t = 0x400;
                break;
            default:
                t = 0xC00;
                break;
        }
        *(s16 *) (D_803ED760 + 0x4E) = t;
        *(s16 *) (D_803ED760 + 0x4C) = t;
        *(s16 *) (D_803ED760 + 0x74) = t;
        if (D_803ED828 == 0 || D_803ED828 == 1) {
            D_803ED814 = pos[2];
        } else {
            D_803ED814 = pos[0];
        }
        pos[0] = D_803643E0;
        t = D_803643E4;
        if (D_80364456 == 0xB || D_80364456 == 0x11 || D_80364456 == 0x12) {
            t += 500;
        }
        pos[1] = t;
        pos[2] = D_803643E8;
        D_803ED826 = 1;
        func_802A039C((Unk8029DEA0Entry *) D_803ED460, 1, 0);
        func_802A03D4((Unk8029DEA0Entry *) D_803ED460, 1, 0);
        func_802A040C((Unk8029DEA0Entry *) D_803ED460, 1, 0);
        func_802A0290((Unk8029DEA0Entry *) D_803ED460, 1, -1);
        D_8036444C = 0xD48;
        D_80364450 = 0;
        ret = 1;
    }
done:
    D_803ED827 = 0;
    D_803F7812 = 0;
    return ret;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AE888.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Zone test for mode *mode (asm: a3 in and out, result in a1; see
 * tools_port/conventions.txt). Mode 1: 1 when z/32 is in [0xCCD, 0xDB5).
 * Mode 2: 1 when x/32 is in [0x834, 0x961) and z/32 in [0x4B0, 0x5DD).
 * Otherwise 0. The asm leaves the last value it compared (z/32, or x/32 when
 * the x test fails) in a3, so *mode gets that; other modes leave it alone.
 * Register note: asm caller func_802AE888 keeps a0, a2, t0-t3, f12 and f14
 * live across the call (a mixed N64 build would need a thunk). */
s32 func_802AEB9C(s32 *mode) {
    s32 v;

    if (*mode == 1) {
        *mode = v = D_803643E8 >> 5;
        return v >= 0xCCD && v < 0xDB5;
    }
    if (*mode == 2) {
        *mode = v = D_803643E0 >> 5;
        if (v < 0x834 || v >= 0x961) {
            return 0;
        }
        *mode = v = D_803643E8 >> 5;
        return v >= 0x4B0 && v < 0x5DD;
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEB9C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* One drive-in path check for vehicle 0 ($gp = D_803ED760, read directly):
 * puts the vehicle `dist` from the player in direction D_803ED828 (D_803ED808
 * / D_803ED810; 0: z + dist, 1: z - dist, 2: x + dist, else x - dist), follows
 * the ground there (func_802A9A60 at height D_803ED81C; middle height into
 * D_803ED80C), places the model (func_802AFA64), sets D_803ED81C to the mean
 * of the wheel heights +0x10/+0x1C (logical >> 1), resets the collision state
 * (func_8029A800 with b0 = 0 and h2 = the part-list end, as func_802AFA64
 * leaves them in t0/t2), runs the world and object collision passes
 * (func_8029AA10(0), D_803F77D0 = D_803ED460, func_802BE77C(0)) and, outside
 * vehicle type 0xB, func_8028F994 at the position. Returns 1 when the spot is
 * free (D_803A7424 clear), else 0.
 * Register convention (conventions.txt): dist a2, result a3; func_802A9A60's
 * leftover inputs key t8, fp, f12-f26 come in from the asm caller (here key,
 * *fp, f). The asm doesn't save fp: it leaves what func_8029AA10 left there
 * (the part count on a world hit, else unchanged; func_802BE77C's chain
 * restores it), which the caller's next check reads, so *fp is in/out. The
 * asm saves v0-a0, a2, t0-t9 and gp, and also leaves a1 = D_803A7424
 * (unread). The asm's add/sub trap on overflow. */
s32 func_802AEC3C(s32 dist, s32 key, s32 *fp, TriSideOut *f) {
    s32 *pos = (s32 *) D_803ED808;
    s32 x;
    s32 z;
    u8 *end;
    Out802A9A60 o;

    switch (D_803ED828) {
        case 0:
            x = D_803643E0;
            z = D_803643E8 + dist;
            break;
        case 1:
            x = D_803643E0;
            z = D_803643E8 - dist;
            break;
        case 2:
            x = D_803643E0 + dist;
            z = D_803643E8;
            break;
        default:
            x = D_803643E0 - dist;
            z = D_803643E8;
            break;
    }
    pos[0] = x;
    pos[2] = z;
    func_802A9A60((s16 *) (D_803ED760 + 0x52), D_803ED81C, x, z, (s32 *) (D_803ED760 + 4), &pos[1],
                  (s16 *) (D_803ED760 + 0x4C), key, *fp, D_803ED760, f, &o);
    end = func_802AFA64();
    D_803ED81C = (u32) (*(s32 *) (D_803ED760 + 0x10) + *(s32 *) (D_803ED760 + 0x1C)) >> 1;
    func_8029A800(pos[2], (s32) D_80305CB0, 0, 0, pos[0], pos[1], 0, 0, (s32) end, 0, 0, D_803ED760);
    *fp = func_8029AA10_fp(0, *fp);
    D_803F77D0 = D_803ED460;
    func_802BE77C(0, D_803ED760);
    if (D_80364456 != 0xB) {
        func_8028F994(pos[0], pos[1], pos[2]);
    }
    return D_803A7424 == 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEC3C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Zone level lookup (func_802ABD54) for vehicle id 0 at its position
 * D_803ED808..+8; returns func_802ABD54's v1 (the zone list end).
 * Register convention: the asm passes func_802ABD54's scan registers t6, t7,
 * s0-s4 through (in and out; here through r), leaves a3 = id and clobbers
 * s5, s6 (conventions.txt). Asm caller func_802AEEC8 keeps t8, f12 and f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
s32 func_802AEE84(ZoneScanRegs *r) {
    return func_802ABD54(0, D_803ED808[0], D_803ED808[1], D_803ED808[2], r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEE84.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Ring-steering branch of func_802AEEC8 (D_803A7425 set): func_8029A914,
 * D_803ED824 = 1, func_802A70D8, func_802A71DC (heading +0x4E toward +0x4C,
 * scale 1.0; the new index goes to D_803ED822, +0x4E and +0x74),
 * func_802A746C with that turn and func_802A6FE4 (limit 0).
 * The asm also computes |(+0x4E - 0x800, wrapped) - ring midpoint| (folded
 * to <= 0x800) and then always branches past its use; the value only
 * survives in v1 as func_802A746C's pass-through input, reproduced here. */
static void port_aeec8_ring(void) {
    s32 mid;
    s32 d;
    s32 cur;
    s32 turn;
    s32 target;

    func_8029A914(D_803ED760);
    D_803ED824 = 1;
    mid = func_802A6F6C();
    d = *(u16 *) (D_803ED760 + 0x4E) - 0x800;
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
    func_802A70D8(D_803ED760);
    turn = func_802A71DC(D_803ED760, *(u16 *) (D_803ED760 + 0x4E), *(u16 *) (D_803ED760 + 0x4C), &cur, 1.0f);
    D_803ED822 = cur;
    *(s16 *) (D_803ED760 + 0x4E) = cur;
    *(s16 *) (D_803ED760 + 0x74) = cur;
    func_802A746C(D_803ED760, turn, d, &target);
    func_802A6FE4(D_803ED760, 0);
}

/* Per-frame update of vehicle 0 (hd.c's func_802475D8 / func_8024B7AC, and
 * func_802AE370; state block D_803ED760, the asm's $gp). Zone lookup
 * (func_802AEE84). Unless byte +0x9A is set: animation update
 * (func_802AF4BC) and, while the drive-in D_803ED826 runs, the drive-in step
 * (func_802AF340) instead of driving. Driving: func_802AFBA0; outside game
 * modes 0x800 / 1 / 0x1000 (the next mode D_80364A98 if set, else
 * D_80364A90; 64-bit compares): steering and throttle (func_802A7834: rate
 * func_802AFB84() = 0x8C, mode 3, delta 6), braking when +0xA1 == 1
 * (func_802A77D0), heading (func_802A7FD8: rate 9000, no sound), slope
 * (func_802A83B8 into the f32 at +0, func_802A843C: kind 0, clamp, div
 * 60.0), func_802A7070 while D_803ED824 is set, move (func_802A860C) and
 * ground contact (func_802A8768: id 0, divisors 0x3C/0x3C, after
 * D_803ED40B = 0). Then: animate the model into the current buffer
 * (func_8029E558), place it (func_802AFA64); unless the drive-in runs: in
 * game modes with any of bits 0x1801 D_803ED824 = 0; otherwise collision
 * state reset (func_8029A800: b0 = 0 and h2 = the part-list end as
 * func_802AFA64 leaves them in t0/t2, h1 = speed), part contacts
 * (func_8029C52C(0)), world collision (func_8029AA10(0)), the object pass
 * (func_802BE77C(0)), and then the ring-steering branch when D_803A7425 is
 * set, else D_803ED824 = 0. Finally the player position/speed/headings
 * D_803643E0..E8 / D_8036443C..40 are copied from the vehicle and the
 * position reported (func_802A133C(z, 0, x, y)).
 * Fidelity notes: the asm feeds func_802AEE84 its C caller's t6, t7, s0-s4
 * (0 here; the results are unused), func_802AF340 its C caller's t8 as the
 * triangle-skip key and fp / f12-f26 leftovers (0 and zeros here; fp becomes
 * D_803ED3F2[0..2] and +0x50). Traced in the original (Oct 2026, gameplay,
 * level 0): fp 0 always (the game thread's); t8 = 0x13 from func_8024B7AC
 * (left by func_8026510C's callees), 0 from func_802475D8's mode-0x1801 path,
 * heap pointers during the level intro, and func_8026A8E0's seed
 * (0x47FF37D3) on frames where func_802AF4BC took its random-idle path. The
 * key only makes func_802A9F24 skip D_803EBDB0 triangles whose u16 id equals
 * it (and func_802A9CAC test 0xFF); those ids are vehicle types 6, 7, 0xB,
 * 0x11, 0x12 (func_802AABE4's callers), which neither 0 nor any traced
 * leftover equals, so 0 gives the same ground. And
 * func_802A8768 the FP registers left by func_802A860C's sine routines
 * (zeros here). The asm saves every callee-saved register. */
void func_802AEEC8(void) {
    s32 *pos = (s32 *) D_803ED808;
    ZoneScanRegs zr;
    TriSideOut f;
    Out802A860C o;
    Regs802A8768 rr;
    u64 mode;
    u8 *end;
    s32 rate;
    s32 x;
    f32 slope;

    zr.t6 = zr.t7 = zr.s0 = zr.s1 = zr.s2 = zr.s3 = zr.s4 = 0;
    func_802AEE84(&zr);
    if ((s8) D_803ED760[0x9A] == 0) {
        func_802AF4BC();
        if (D_803ED826 != 0) {
            f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
            func_802AF340(0, 0, &f);
            goto animate;
        }
    }
    func_802AFBA0();
    rate = func_802AFB84();
    mode = D_80364A98;
    if (mode == 0) {
        mode = D_80364A90;
    }
    if (mode != 0x800 && mode != 1 && mode != 0x1000) {
        func_802A7834(rate, (u16 *) (D_803ED760 + 0x4C), D_803ED760, (s16 *) (D_803ED760 + 0x76), 3,
                      D_803ED760 + 0x96, (s16 *) (D_803ED760 + 0x78), 6);
        if (D_803ED760[0xA1] == 1) {
            func_802A77D0(D_803ED760);
        }
        func_802A7FD8(D_803ED760, (u16 *) (D_803ED760 + 0x74), 0x2328, (s16 *) (D_803ED760 + 0x76),
                      (u16 *) (D_803ED760 + 0x4C), (u16 *) (D_803ED760 + 0x4E), (s8 *) (D_803ED760 + 0x99), 0);
        slope = func_802A83B8((s16 *) (D_803ED760 + 0x76), D_803ED760 + 0x96, (s32 *) (D_803ED760 + 4),
                              (f32 *) D_803ED760);
        func_802A843C(D_803ED760, (s16 *) (D_803ED760 + 0x76), 0, (s8 *) (D_803ED760 + 0x96),
                      (s32 *) (D_803ED760 + 4), 1, 60.0f);
        if (D_803ED824 != 0) {
            func_802A7070(D_803ED760, &D_803ED822);
        }
        x = func_802A860C(slope, *(u16 *) (D_803ED760 + 0x4E), (s16 *) (D_803ED760 + 0x76), &pos[0], &pos[2], &o);
        D_803ED40B = 0;
        rr.s3 = o.s3;
        f.pz = f.cross = f.cz = f.side = f.sideZ = f.dz = 0.0f;
        func_802A8768(D_803ED760, 0, &pos[0], &pos[1], &pos[2], x, o.t1, 0x3C, 0x3C, (s16 *) (D_803ED760 + 0x4C),
                      D_803ED760 + 0x96, (s16 *) (D_803ED760 + 0x52), (s32 *) (D_803ED760 + 0x28),
                      (s32 *) (D_803ED760 + 0x40), (s32 *) (D_803ED760 + 0x34), (s32 *) (D_803ED760 + 4), &rr, &f);
    }
animate:
    if (D_8035805C != 0) {
        func_8029E558(D_803ED82C, D_803ED830, (Unk8029DEA0Entry *) D_803ED460);
    } else {
        func_8029E558(D_803ED830, D_803ED82C, (Unk8029DEA0Entry *) D_803ED460);
    }
    end = func_802AFA64();
    if (D_803ED826 == 0) {
        mode = D_80364A98;
        if (mode == 0) {
            mode = D_80364A90;
        }
        if (mode & 0x1801) {
            D_803ED824 = 0;
        } else {
            func_8029A800(pos[2], (s32) D_80305CB0, 0, 0, pos[0], pos[1], 0, *(s16 *) (D_803ED760 + 0x76),
                          (s32) end, 0, 0, D_803ED760);
            func_8029C52C(0, D_803ED760);
            func_8029AA10(0);
            D_803F77D0 = D_803ED460;
            func_802BE77C(0, D_803ED760);
            if (D_803A7425 != 0) {
                port_aeec8_ring();
            } else {
                D_803ED824 = 0;
            }
        }
    }
    D_803643E0 = pos[0];
    D_803643E4 = pos[1];
    D_803643E8 = pos[2];
    D_8036443C = *(s16 *) (D_803ED760 + 0x76);
    D_8036443E = *(u16 *) (D_803ED760 + 0x4E);
    D_80364440 = *(u16 *) (D_803ED760 + 0x4C);
    func_802A133C(pos[2], 0, pos[0], pos[1], D_803ED760);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AEEC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* vehicle 0's state block (the asm's $gp) */
extern u8 D_803ED828;    /* drive-in direction: 0 +z, 1 -z, 2 +x, else -x */
extern s32 D_803ED814;   /* drive-in stop coordinate */

/* Drive-in step of vehicle 0 ($gp = D_803ED760, read directly): speed +0x76 =
 * 80, the position D_803ED808 (x) / D_803ED810 (z) moves 100 in the direction
 * D_803ED828, the ground is followed with func_802A9A60 (wheel heights into
 * +4, middle height into D_803ED80C), and once the position reaches (or
 * passes) D_803ED814 the drive-in ends (D_803ED826 = 0, speed 0). Then the
 * shadow/marker record 0 is updated: func_802582C4(0, x, (D_803ED3A8[1] +
 * [2]) >> 1 (logical), z, y, ...).
 * Register convention (conventions.txt): func_802A9A60's leftover inputs come
 * from the asm caller: key t8, fp and f12-f26 (here key, fp, f). Fidelity
 * notes: the asm's key is whatever t8 held in func_802AEEC8 (its C callers'
 * leftover, or func_8026A8E0's seed on func_802AF4BC's random-idle path;
 * traced 0x13 / 0x47FF37D3, see func_802AEEC8: no triangle id can equal it,
 * so the C's 0 scans the same ground); fp (traced: always 0, as here) ends up
 * as D_803ED3F2[0..2] and +0x50. The asm passes only five arguments to
 * func_802582C4: arg5 is a stale stack word (0 here), arg6/arg7 are the high
 * and low words of its own saved $ra (-1 and 0x802AEF50, the return address in
 * func_802AEEC8; func_802582C4 keeps them as s16s), reproduced here. The asm's
 * addi traps on overflow; it changes s0-s4 (and func_802A9A60's s5, s6). */
void func_802AF340(s32 key, s32 fp, TriSideOut *f) {
    s32 *pos = (s32 *) D_803ED808;
    s32 x;
    s32 z;
    s32 t;
    s32 stop;
    Out802A9A60 o;

    *(s16 *) (D_803ED760 + 0x76) = 0x50;
    x = pos[0];
    z = pos[2];
    switch (D_803ED828) {
        case 0:
            z += 100;
            break;
        case 1:
            z -= 100;
            break;
        case 2:
            x += 100;
            break;
        default:
            x -= 100;
            break;
    }
    pos[0] = x;
    pos[2] = z;
    func_802A9A60((s16 *) (D_803ED760 + 0x52), pos[1], x, z, (s32 *) (D_803ED760 + 4), &pos[1],
                  (s16 *) (D_803ED760 + 0x4C), key, fp, D_803ED760, f, &o);
    t = D_803ED814;
    switch (D_803ED828) {
        case 0:
            stop = !(pos[2] < t);
            break;
        case 1:
            stop = !(t < pos[2]);
            break;
        case 2:
            stop = !(pos[0] < t);
            break;
        default:
            stop = !(t < pos[0]);
            break;
    }
    if (stop) {
        D_803ED826 = 0;
        *(s16 *) (D_803ED760 + 0x76) = 0;
    }
    func_802582C4(0, pos[0], (u32) (D_803ED3A8[1] + D_803ED3A8[2]) >> 1, pos[2], pos[1], 0, -1,
                  (s32) 0x802AEF50);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF340.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* this vehicle's state block (the asm's $gp) */
extern s16 D_803ED820;   /* last channel-1 mode */

/* Field 0x10 of channel idx of D_803ED460 (1 = running). */
static s32 AnimChannelState(s32 idx) {
    s32 ch[8];

    func_802A04BC(idx, (Unk8029DEA0Entry *) D_803ED460, ch);
    return ch[0];
}

/* Per-frame animation update of vehicle 0 (asm caller func_802AEEC8; the asm
 * reads the state block through $gp = D_803ED760, read directly here). Byte
 * +0xA1 records whether the vehicle was moving last frame.
 * Stopped (speed +0x76 == 0): on the first stopped frame channel 1 is stopped
 * and channel 31 restarted with 70; then, when none of channels 31, 2, 3, 4 is
 * running and func_8026A8E0(0, 20) is 0, one of channels 3 / 4 / 2 (by
 * func_8026A8E0(0, 2) = 0 / 1 / else) is started.
 * Moving: on the first moving frame channel 1 is set to mode 2 and the running
 * one of channels 2, 3, 4 (else 3, after resetting it) is handed over via
 * func_8029F9D4 and channel 31 restarted with 40. Then, unless channel 31 is
 * running, channel 1 gets the direction (speed < 0) and |speed| / 11 and is
 * restarted, and (outside levels 0x31 and 0x26) a change of channel 1's mode
 * to 2 or 6 starts sound 0x14 / 0x15.
 * Register note: the asm clobbers s5 (and f20/f30 through func_8029F9D4);
 * conventions.txt. Asm caller func_802AEEC8 reads t6, t8, f12 and f14 after
 * the call, which the asm passes through except where its C callees change
 * them (func_8026A8E0 leaves its seeds in t6/t8): a mixed N64 build would need
 * a thunk, the native port won't. */
void func_802AF4BC(void) {
    u8 *base = D_803ED460;
    s32 speed = *(s16 *) (D_803ED760 + 0x76);
    s32 r;
    s32 ch[8];

    if (speed == 0) {
        if (D_803ED760[0xA1] != 0) {
            func_802A02E4(1, (Unk8029DEA0Entry *) base);
            func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 3, 0);
            func_8029F9D4(1, 3, (Unk8029DEA0Entry *) base);
            func_802A039C((Unk8029DEA0Entry *) base, 0x1F, 0x46);
            func_802A03D4((Unk8029DEA0Entry *) base, 0x1F, 0);
            func_802A040C((Unk8029DEA0Entry *) base, 0x1F, 0);
            func_802A0290((Unk8029DEA0Entry *) base, 0x1F, 1);
        }
        D_803ED760[0xA1] = 0;
        if (AnimChannelState(0x1F) == 1 || AnimChannelState(2) == 1 || AnimChannelState(3) == 1 ||
            AnimChannelState(4) == 1) {
            return;
        }
        if (func_8026A8E0(0, 0x14) != 0) {
            return;
        }
        r = func_8026A8E0(0, 2);
        if (r == 0) {
            func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 3, 0);
            func_802A0290((Unk8029DEA0Entry *) base, 3, 1);
        } else if (r == 1) {
            func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 4, 0);
            func_802A0290((Unk8029DEA0Entry *) base, 4, 1);
        } else {
            func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 2, 0);
            func_802A0290((Unk8029DEA0Entry *) base, 2, 1);
        }
        return;
    }

    if (D_803ED760[0xA1] != 1) {
        func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 1, 2);
        if (AnimChannelState(2) != 0) {
            func_8029F9D4(2, 1, (Unk8029DEA0Entry *) base);
            func_802A02E4(2, (Unk8029DEA0Entry *) base);
        } else if (AnimChannelState(3) != 0) {
            func_8029F9D4(3, 1, (Unk8029DEA0Entry *) base);
            func_802A02E4(3, (Unk8029DEA0Entry *) base);
        } else if (AnimChannelState(4) != 0) {
            func_8029F9D4(4, 1, (Unk8029DEA0Entry *) base);
            func_802A02E4(4, (Unk8029DEA0Entry *) base);
        } else {
            func_802A0360(0.0f, (Unk8029DEA0Entry *) base, 3, 0);
            func_8029F9D4(3, 1, (Unk8029DEA0Entry *) base);
        }
        func_802A039C((Unk8029DEA0Entry *) base, 0x1F, 0x28);
        func_802A03D4((Unk8029DEA0Entry *) base, 0x1F, 0);
        func_802A040C((Unk8029DEA0Entry *) base, 0x1F, 0);
        func_802A0290((Unk8029DEA0Entry *) base, 0x1F, 1);
    }
    if (AnimChannelState(0x1F) != 1) {
        speed = *(s16 *) (D_803ED760 + 0x76);
        if (speed < 0) {
            func_802A03D4((Unk8029DEA0Entry *) base, 1, 1);
        } else {
            func_802A03D4((Unk8029DEA0Entry *) base, 1, 0);
        }
        if (D_802E8BDC != 0x31 && D_802E8BDC != 0x26) {
            s32 old = D_803ED820;
            s32 mode;

            func_802A04BC(1, (Unk8029DEA0Entry *) base, ch);
            mode = ch[6];
            D_803ED820 = mode;
            if (mode != old) {
                if (mode == 2) {
                    func_80260650(D_80367738, 0x14, NULL);
                } else if (mode == 6) {
                    func_80260650(D_80367738, 0x15, NULL);
                }
            }
        }
        if (speed < 0) {
            speed = -speed;
        }
        func_802A039C((Unk8029DEA0Entry *) base, 1, (u32) speed / 11);
        func_802A0290((Unk8029DEA0Entry *) base, 1, -1);
    }
    D_803ED760[0xA1] = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AF4BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 *D_803ED818;  /* vehicle 0's model header */
extern u8 *D_803ED82C;  /* vehicle 0's model buffers */
extern u8 *D_803ED830;

/* Places vehicle 0's model ($gp = D_803ED760, read directly): rotation
 * (0, +0x4C, 0) and position D_803ED808..810 at scale 0x4E20 into the matrix
 * at header word [header word 0x18 + 4] of the current buffer (func_802AA764),
 * then its parts (func_8029C454, tag 0; header words 4 and 8 give the part
 * list, relative to the header).
 * func_8029C454's register in/outs (only used for a part with no matrices):
 * s2 = the matrix (func_802AA764 leaves it there); a3, s1, s0 are whatever the
 * asm callers had (0 here; fidelity note). The asm's add traps on overflow.
 * Returns the end of the part list (the asm leaves it in t2, and t0 = 0; both
 * asm callers pass them on to func_8029A800 as its h2/b0 inputs;
 * conventions.txt). Asm callers keep t6 (func_802AEC3C) and t7
 * (func_802AEEC8) live; a mixed N64 build would need a thunk. */
u8 *func_802AFA64(void) {
    u8 *hdr = D_803ED818;
    s32 *pos = (s32 *) D_803ED808;
    u8 *m = *(u8 **) (hdr + *(s32 *) (hdr + 0x18) + 4);
    MtxChainRegs regs;

    m += (s32) (D_8035805C != 0 ? D_803ED82C : D_803ED830);
    D_803ED390[0] = 0;
    D_803ED390[2] = 0;
    D_803ED390[1] = *(u16 *) (D_803ED760 + 0x4C);
    func_802AA764(pos[0], pos[1], pos[2], 0x4E20, (s32 *) m);
    regs.a3 = 0;
    regs.s1 = 0;
    regs.s2 = (s32) m;
    regs.s0 = 0;
    func_8029C454(pos[0], pos[1], pos[2], 0, hdr + *(s32 *) (hdr + 4), hdr + *(s32 *) (hdr + 8),
                  D_8035805C != 0 ? D_803ED82C : D_803ED830, &regs);
    return hdr + *(s32 *) (hdr + 8);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFA64.s")
#endif

/* func_802AFB84: sets $s3 = 0x8c directly (`addiu $s3, $zero, 0x8c`)
 * inside the same dead 8-byte `sd $ra` frame as func_802BBE10/
 * func_802C8AF0/func_802CE9A4/func_802AC284 - same confirmed hand-
 * written family, and doubly so here since C has no way to pin a value
 * to a specific callee-saved register by name regardless. Value is
 * never read or returned; likely a vestigial/debug leftover in the
 * original assembly. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns 0x8C (asm: in s3, which asm caller func_802AEEC8 reads; see
 * tools_port/conventions.txt). That caller keeps a0-a3, f12 and f14 live
 * across the call (a mixed N64 build would need a thunk). */
s32 func_802AFB84(void) {
    return 0x8C;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFB84.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* Vehicle-module setup leaf: D_803EBBF4 = D_803EBBF0 * 4, then the byte pair
 * D_803ED3F6/7 = 40, 3. Same shape as func_802B0CE8 (6B4A0), func_802B2900
 * (6C5E0) and, with other constants, func_802B8424 (72B80), func_802B5814
 * (6E200), func_802B7240 (71140).
 * Register note: the asm touches only at/v0/v1/f0/f2. Its asm caller
 * func_802AEEC8 keeps a0-a3, f12 and f14 live across the call; a mixed N64
 * build would need a thunk preserving those, the native port does not. */
void func_802AFBA0(void) {
    D_803EBBF4 = D_803EBBF0 * 4.0f;
    D_803ED3F6 = 40;
    D_803ED3F7 = 3;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBA0.s")
#endif

/* func_802AFBFC: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803ED760[];  /* this vehicle's state block: 0xA6 bytes are (de)serialized */

/* Serialize this vehicle's state (D_803ED760[0..0xA5] plus the three words
 * D_803ED808[0..2]) into `dst` via func_802AC7DC. Returns the callee's result
 * (0xB2, the byte count): the asm leaves the callee's v0 untouched. */
s32 func_802AFBFC(u8 *dst) {
    return func_802AC7DC(dst, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFBFC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Inverse of func_802AFBFC: restore this vehicle's state from `src` via
 * func_802AC85C. Called from 17210.c as `void func_802AFC28(void *)`. */
void func_802AFC28(void *src) {
    func_802AC85C(src, D_803ED760, D_803ED808);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69BB0/func_802AFC28.s")
#endif
