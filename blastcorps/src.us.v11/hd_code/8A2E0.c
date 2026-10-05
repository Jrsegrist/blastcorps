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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEAA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Vtx D_803FBAB0[]; /* 8 vertices per box */
extern u8 D_803FC1F0;    /* current box index */

/* Writes the 8 corner positions of the box x-10..x+10, y..y+20, z-10..z+20
 * into the vertices of box D_803FC1F0 (only ob[] is written). The asm takes
 * x, y, z in t4, t5, t6 (conventions.txt) and saves everything it uses; its
 * asm caller func_802CEAA0 keeps a1-a3, t1-t3, f12 and f14 live across the
 * call (a mixed N64 build would need a thunk, the native port doesn't). */
void func_802CEE14(s32 x, s32 y, s32 z) {
    Vtx *v = &D_803FBAB0[D_803FC1F0 * 8];
    s32 x0 = x - 10;
    s32 x1 = x + 10;
    s32 y1 = y + 20;
    s32 z0 = z - 10;
    s32 z1 = z + 20;

    v[0].v.ob[0] = x0, v[0].v.ob[1] = y, v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x0, v[1].v.ob[1] = y1, v[1].v.ob[2] = z0;
    v[2].v.ob[0] = x1, v[2].v.ob[1] = y, v[2].v.ob[2] = z0;
    v[3].v.ob[0] = x1, v[3].v.ob[1] = y1, v[3].v.ob[2] = z0;
    v[4].v.ob[0] = x0, v[4].v.ob[1] = y, v[4].v.ob[2] = z1;
    v[5].v.ob[0] = x0, v[5].v.ob[1] = y1, v[5].v.ob[2] = z1;
    v[6].v.ob[0] = x1, v[6].v.ob[1] = y, v[6].v.ob[2] = z1;
    v[7].v.ob[0] = x1, v[7].v.ob[1] = y1, v[7].v.ob[2] = z1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEE14.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEEFC.s")

#ifdef NON_MATCHING
/* D_803FBBB0: table of 0x14-byte entries, D_803FC1F0 of them in use: s32
 * x, y, z at +0 and a flag halfword at +0x10 (a bit/flag state, see 409D0's
 * func_80285AB0/func_80285B10). */
typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u8 pad12[2];
} UnkEntry8A2E0; /* size 0x14 */

extern UnkEntry8A2E0 D_803FBBB0[];
extern u8 D_803FBBE0[];

void func_80285AB0(u8 bit);
s32 func_80285B10(u8 bit);
void func_80285B68(s32 arg0);
void func_80285CA0(void);
void func_802A0360(f32 f, void *base, s32 idx, s32 val); /* 56040 */
void func_802CF3E0(s32 *pos);
extern s32 D_803643E0; /* player x, y, z */
extern s32 D_803643E4;
extern s32 D_803643E8;

float sqrtf(float);
#pragma intrinsic(sqrtf)

/* cvt.w.s: float -> s32 rounding to nearest, ties to even (the game's FCSR
 * mode), not truncation like a C cast. */
static s32 port_cvt_w_s(f32 x) {
    s32 t = (s32) x;
    f32 frac = x - (f32) t;

    if (frac > 0.5f || (frac == 0.5f && (t & 1))) {
        t++;
    } else if (frac < -0.5f || (frac == -0.5f && (t & 1))) {
        t--;
    }
    return t;
}
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Pickups: for each of the D_803FC1F0 entries of D_803FBBB0 not yet taken
 * (flag +0x10 == 0) whose position is within 0x4C of the player (both >> 5,
 * squared distance summed in 32 bits, sqrt.s and cvt.w.s rounding; the sum
 * must not overflow, or the asm's cvt.w.s of a NaN would trap), mark it
 * taken, set D_803FBBE0's sound fields 2 and 1 to 0.0 (func_802A0360), call
 * func_80285B68 with the countdown value (D_803FC1F0 for entry 0, down to 1:
 * the same quirk as func_802CF5B0), then func_80285CA0 and the
 * func_802CF3E0 burst at the entry. The player position is read once. The
 * asm saves every register (k1, gp, sp, fp included) around the two C calls
 * only because it keeps its loop state in t0-t4 across them; its only
 * caller is C (func_802475D8), so no asm caller relies on anything. */
void func_802CF1A4(void) {
    s32 px = D_803643E0 >> 5;
    s32 py = D_803643E4 >> 5;
    s32 pz = D_803643E8 >> 5;
    UnkEntry8A2E0 *e = D_803FBBB0;
    s32 n;

    for (n = D_803FC1F0; n != 0; n--, e++) {
        s32 dx;
        s32 dy;
        s32 dz;

        if (e->unk10 != 0) {
            continue;
        }
        dx = (e->pos[0] >> 5) - px;
        dy = (e->pos[1] >> 5) - py;
        dz = (e->pos[2] >> 5) - pz;
        if (port_cvt_w_s(sqrtf((f32) (s32) ((u32) (dx * dx) + (u32) (dy * dy) + (u32) (dz * dz)))) < 0x4C) {
            e->unk10 = 1;
            func_802A0360(0.0f, D_803FBBE0, 2, 0);
            func_802A0360(0.0f, D_803FBBE0, 1, 0);
            func_80285B68(n);
            func_80285CA0();
            func_802CF3E0(e->pos);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF1A4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* The 0x38-byte debris record handed to func_802C04F0 (77E20.c's Unk803F3FF8). */
typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10[6];
    /* 0x28 */ u32 unk28;
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 pad2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 pad32[2];
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 pad36[2];
} Debris8A2E0;

extern Debris8A2E0 D_803F3FF8;
extern void *D_80367738; /* sound player */
void func_802A5E60(void);     /* 60F60 */
void func_802C049C(void);     /* 77E20 */
void func_802C04F0(u32 *src); /* 77E20 */
void *func_80260650(void *arg0, s16 arg1, void *arg2);

/* Burst at pos (s32 x, y, z): func_802A5E60(), frees every D_803F3968 slot
 * (func_802C049C), then 24 debris records (D_803F3FF8, through
 * func_802C04F0) at (x, y, z) << 11, the i-th one 0xA0000 * i higher, with
 * unkC = 50000 + 0x36B0 * i, unk2C = i and kind (byte 0x31) 0x15; finally
 * plays sound 0x5E (func_80260650(D_80367738, 0x5E, NULL)). The asm takes pos
 * in t1 and preserves every register (all of them saved); asm caller
 * func_802CF1A4 keeps t0-t4 live across the call. The asm's trapping `add`s
 * on the height and unkC can't overflow for game coordinates. */
void func_802CF3E0(s32 *pos) {
    Debris8A2E0 *d = &D_803F3FF8;
    s32 x;
    s32 y;
    s32 z;
    s32 w;
    s32 i;

    func_802A5E60();
    func_802C049C();
    x = pos[0] << 11;
    y = pos[1] << 11;
    z = pos[2] << 11;
    for (i = 0, w = 50000; i != 24; i++) {
        d->pos[1] = y;
        y += 0xA0000;
        d->unk28 = 0xFC180000;
        d->unkC = w;
        d->unk2C = i;
        d->pos[0] = x;
        d->pos[2] = z;
        w += 0x36B0;
        d->unk10[0] = 0;
        d->unk10[1] = 0;
        d->unk10[2] = 0;
        d->unk10[3] = 0;
        d->unk10[4] = 0;
        d->unk10[5] = 0;
        d->unk2E = 0;
        d->unk30 = 0;
        d->unk31 = 0x15;
        d->unk34 = 0;
        d->unk35 = 0;
        func_802C04F0((u32 *) d);
    }
    func_80260650(D_80367738, 0x5E, NULL);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF3E0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* For each of the D_803FC1F0 entries whose +0x10 flag is set, call
 * func_80285AB0. Quirk kept from the asm: the argument is the countdown value
 * (D_803FC1F0 for entry 0, down to 1 for the last entry), not the index. */
void func_802CF5B0(void) {
    UnkEntry8A2E0 *e = D_803FBBB0;
    u8 n;

    for (n = D_803FC1F0; n != 0; n--, e++) {
        if (e->unk10 != 0) {
            func_80285AB0(n);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF5B0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Refresh every entry's +0x10 flag from func_80285B10, passing the same
 * countdown value as func_802CF5B0 (stored as a halfword). */
void func_802CF628(void) {
    UnkEntry8A2E0 *e = D_803FBBB0;
    u8 n;

    for (n = D_803FC1F0; n != 0; n--, e++) {
        e->unk10 = func_80285B10(n);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CF628.s")
#endif
