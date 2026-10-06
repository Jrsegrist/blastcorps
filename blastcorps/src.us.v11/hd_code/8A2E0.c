#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
#ifdef NON_MATCHING
/* D_803FBBB0: table of 0x14-byte entries, D_803FC1F0 of them in use: s32
 * x, y, z at +0, a word at +0xC (0x280), a flag halfword at +0x10 (a
 * bit/flag state, see 409D0's func_80285AB0/func_80285B10) and the record's
 * halfword +6 at +0x12. */
typedef struct {
    /* 0x00 */ s32 pos[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
} UnkEntry8A2E0; /* size 0x14 */


extern UnkEntry8A2E0 D_803FBBB0[];
extern u8 D_803FBBE0[]; /* channel tables of the boxes' two models */
extern u8 D_803FBEE0[];
extern u8 *D_803FBBD8;  /* the boxes' model data */
extern u8 *D_803FC1E0;  /* four 0x300-byte save copies */
extern u8 *D_803FC1E4;
extern u8 *D_803FC1E8;
extern u8 *D_803FC1EC;
extern u8 D_803FC1F0;   /* number of boxes */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_80358070; /* bump allocator */
void func_802CEE14(s32 x, s32 y, s32 z);

/* Load channels for one model: func_8029F85C(b, a, ch, D_803FBBD8), channel 0
 * = (100, 0, 0, 0.0, 1), run it into both copies (func_8029E558, then
 * func_802A0320 and func_802A0290(1) between them). */
#define BOX_MODEL_CHANNELS(ch, a, b)                                          \
    {                                                                         \
        func_8029F85C((u32 *) (b), (u32 *) (a), (Unk8029DEA0Entry *) (ch), D_803FBBD8); \
        func_802A039C((Unk8029DEA0Entry *) (ch), 0, 100);                     \
        func_802A03D4((Unk8029DEA0Entry *) (ch), 0, 0);                       \
        func_802A040C((Unk8029DEA0Entry *) (ch), 0, 0);                       \
        func_802A0480(0.0f, (Unk8029DEA0Entry *) (ch), 0, 0);                 \
        func_802A0290((Unk8029DEA0Entry *) (ch), 0, 1);                       \
        func_8029E558((a), (b), (Unk8029DEA0Entry *) (ch));                   \
        func_802A0320(0, (Unk8029DEA0Entry *) (ch));                          \
        func_802A0290((Unk8029DEA0Entry *) (ch), 0, 1);                       \
        func_8029E558((b), (a), (Unk8029DEA0Entry *) (ch));                   \
    }

/* Level setup of the boxes (pickups, level object in obj; its list at
 * obj + obj[0x28] .. obj + obj[0x2C], 8-byte records {s16 x, y, z; u16 w}):
 * D_803FC1F0 = 0; if the list isn't empty, load model 0x96 (func_802A396C)
 * into D_803FBBD8, then for each record: entry pos = (x, y, z) << 5, the
 * box vertices (func_802CEE14(x, y, z), unshifted), unkC = 0x280, flag 0,
 * unk12 = w, D_803FC1F0++. If any box exists: four 0x300-byte copies from
 * the D_80358070 bump pointer (D_803FC1E0..EC; trapping adds), model
 * channels D_803FBBE0 (copies 1E0/1E4) with channels 1 and 2 set to (2, 0,
 * 0, -1), and D_803FBEE0 (copies 1E8/1EC).
 * Register convention: obj in t0 (conventions.txt); the asm saves t0 (asm
 * caller func_802A1674 keeps it) and leaves t1/t2 (list end, the model
 * pointer), func_802A396C's s2/s4 and the channel routines' s0-s5, fp,
 * f20, f30 changed; the survey lists t1, t2, s3, f12, f14 as read by
 * func_802A1674 (dead there, not modelled). Its loop uses `!=` (the list
 * must end on a record boundary). */
void func_802CEAA0(u8 *obj) {
    Out802A396C o;
    UnkEntry8A2E0 *e;
    s16 *p;
    s16 *end;
    u32 h;

    D_803FC1F0 = 0;
    p = (s16 *) (obj + *(s32 *) (obj + 0x28));
    end = (s16 *) (obj + *(s32 *) (obj + 0x2C));
    if (p != end) {
        func_802A396C(0x96, &o);
        D_803FBBD8 = o.s2;
        e = D_803FBBB0;
        while (p != end) {
            e->pos[0] = p[0] << 5;
            e->pos[1] = p[1] << 5;
            e->pos[2] = p[2] << 5;
            func_802CEE14(p[0], p[1], p[2]);
            e->unkC = 0x280;
            e->unk10 = 0;
            e->unk12 = p[3];
            D_803FC1F0++;
            e++;
            p += 4;
        }
    }
    if (D_803FC1F0 == 0) {
        return;
    }
    h = D_80358070;
    D_803FC1E0 = (u8 *) h;
    D_803FC1E4 = (u8 *) (h + 0x300);
    D_803FC1E8 = (u8 *) (h + 0x600);
    D_803FC1EC = (u8 *) (h + 0x900);
    D_80358070 = h + 0xC00;
    BOX_MODEL_CHANNELS(D_803FBBE0, D_803FC1E0, D_803FC1E4);
    func_802A039C((Unk8029DEA0Entry *) D_803FBBE0, 1, 2);
    func_802A03D4((Unk8029DEA0Entry *) D_803FBBE0, 1, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803FBBE0, 1, 0);
    func_802A0290((Unk8029DEA0Entry *) D_803FBBE0, 1, -1);
    func_802A039C((Unk8029DEA0Entry *) D_803FBBE0, 2, 2);
    func_802A03D4((Unk8029DEA0Entry *) D_803FBBE0, 2, 0);
    func_802A040C((Unk8029DEA0Entry *) D_803FBBE0, 2, 0);
    func_802A0290((Unk8029DEA0Entry *) D_803FBBE0, 2, -1);
    BOX_MODEL_CHANNELS(D_803FBEE0, D_803FC1E8, D_803FC1EC);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEAA0.s")
#endif

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
#ifdef NON_MATCHING

#define K0_PHYS(p) ((u32) (p) - 0x80000000)

/* Draw the boxes (00000.c: gdl = func_802CEEFC(gdl, D_8035805C, dl2, mtx)):
 * if there are any, gdl gets segment 6 = the model data's part at
 * D_803FBBD8 + [5] (G_MOVEWORD 0xBC001806); then per box i: segment 7 = the
 * box's save copy (taken: D_803FC1E4 / D_803FC1E0, else D_803FC1EC /
 * D_803FC1E8, the second of each pair when `cur` is clear) and a G_DL to
 * dl2; dl2 gets G_VTX of the box's 8 vertices (D_803FBAB0 + 0x80 i), a
 * G_CULLDL 0..8, its translation matrix (func_802ACA60(pos << 11) into
 * mtx, converted by func_802AC8CC, G_MTX 0x01040040), G_DLs to the model's
 * parts [9] and [11], G_POPMTX and G_ENDDL; mtx advances 0x40 per box. Then
 * the box channels run into the two copies (func_8029E558 on D_803FBBE0,
 * current copy first). Returns the advanced gdl; dl2 and mtx aren't handed
 * back (the caller doesn't need them). cur is tested as a whole register\n * (00000.c declares it u8). The asm's pointer adds trap; it saves
 * s0-s7, gp, fp but not the f20/f30 that func_8029E558 changes
 * (conventions.txt: clobbers). */
Gfx *func_802CEEFC(Gfx *gdl, s32 cur, Gfx *dl2, Mtx *mtx) {
    u8 *data = D_803FBBD8;
    UnkEntry8A2E0 *e;
    u8 *seg;
    s32 n;
    s32 i;

    if (D_803FC1F0 == 0) {
        return gdl;
    }
    gdl->words.w0 = 0xBC001806;
    gdl->words.w1 = K0_PHYS(data + *(s32 *) (data + 0x14));
    gdl++;
    e = D_803FBBB0;
    for (n = D_803FC1F0, i = 0; n != 0; n--, e++, i++) {
        if (e->unk10 != 0) {
            seg = cur ? D_803FC1E4 : D_803FC1E0;
        } else {
            seg = cur ? D_803FC1EC : D_803FC1E8;
        }
        gdl[0].words.w0 = 0xBC001C06;
        gdl[0].words.w1 = K0_PHYS(seg);
        gdl[1].words.w0 = 0x06000000;
        gdl[1].words.w1 = K0_PHYS(dl2);
        dl2[0].words.w0 = 0x04700080;
        dl2[0].words.w1 = (u32) ((u8 *) D_803FBAB0 + 0x80 * i);
        dl2[1].words.w0 = 0xBE000000;
        dl2[1].words.w1 = 0x140;
        gdl += 2;
        dl2 += 2;
        func_802ACA60(e->pos[0] << 11, e->pos[1] << 11, e->pos[2] << 11, (s32 *) mtx);
        func_802AC8CC((u16 *) mtx);
        dl2->words.w0 = 0x01040040;
        dl2->words.w1 = K0_PHYS(mtx);
        dl2++;
        data = D_803FBBD8;
        dl2[0].words.w0 = 0x06000000;
        dl2[0].words.w1 = K0_PHYS(data + *(s32 *) (data + 0x24));
        dl2[1].words.w0 = 0x06000000;
        dl2[1].words.w1 = K0_PHYS(data + *(s32 *) (data + 0x2C));
        dl2[2].words.w0 = 0xBD000000;
        dl2[2].words.w1 = 0;
        dl2[3].words.w0 = 0xB8000000;
        dl2[3].words.w1 = 0;
        dl2 += 4;
        mtx++;
    }
    if (cur != 0) {
        func_8029E558(D_803FC1E4, D_803FC1E0, (Unk8029DEA0Entry *) D_803FBBE0);
    } else {
        func_8029E558(D_803FC1E0, D_803FC1E4, (Unk8029DEA0Entry *) D_803FBBE0);
    }
    return gdl;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A2E0/func_802CEEFC.s")
#endif

#ifdef NON_MATCHING
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
