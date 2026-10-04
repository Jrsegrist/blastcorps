#include "common.h"
#include <ultra64.h>

/* D_8039C550: D_8039C710 0x38-byte entries (positions in 1/32 units). */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ u8 unk10; /* model index into D_802FDC08 */
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 pad13;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ u8 pad1C[4];
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 pad21[0x28 - 0x21];
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 pad2E[2];
    /* 0x30 */ u32 unk30; /* texture (KSEG0 address) */
    /* 0x34 */ u8 pad34[4];
} Entry4B5E0;

/* D_8039C718: D_8039C7F8 0x1C-byte entries. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ u8 unk10; /* model index into D_802FDC08 */
    /* 0x11 */ u8 unk11;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
} Entry4B5E0c;

/* D_8039C800: D_8039C940 0x28-byte entries: a position and a box of
 * 1/32-unit points around it, plus the D_8039C718 index it came from. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 pad24[2];
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 unk27;
} Entry4B5E0b;

/* D_802FDC08: 0x290-byte models: a display list, then texture size and
 * a radius. */
typedef struct {
    /* 0x000 */ Gfx dl[0x50];
    /* 0x280 */ s16 unk280; /* texture id for func_802A0CC8 */
    /* 0x282 */ u8 width;
    /* 0x283 */ u8 height;
    /* 0x284 */ s16 radius;
    /* 0x286 */ u8 pad286[2];
    /* 0x288 */ s32 unk288;
    /* 0x28C */ u8 pad28C[4];
} Model4B5E0;

/* Per-frame dynamic buffer: one matrix per entry at 0xB00. */
typedef struct {
    /* 0x000 */ u8 pad[0xB00];
    /* 0xB00 */ Mtx mtx[1];
} Dyn4B5E0;

extern Entry4B5E0 D_8039C550[];
extern u8 D_8039C579;
extern s32 D_8039C710;
extern Entry4B5E0c D_8039C718[];
extern s32 D_8039C7F8;
extern Entry4B5E0b D_8039C800[];
extern u8 D_8039C940;
extern s32 D_8039C944;
extern s32 D_8039C948;
extern s32 D_8039C94C;
extern u8 D_8039C950;
extern s32 D_8039C954;
extern s32 D_8039C958;
extern s32 D_8039C95C;
extern u8 D_803ED40C;
extern s32 D_803FB8B0;
extern Model4B5E0 D_802FDC08[];
extern u8 D_803A7424;
extern u8 D_02000000[]; /* segment 2 base */

void func_802AACD4(u8, s32, s32, void *, void *);
s32 func_802AAE1C(u8, s16, s16, void *, void *);
s32 func_802CE6F8(s32, s32, s32);
s32 func_8026A6F0(s32, s32, s32, s32, s32, s32);
void func_802CE9A4(void);
void func_802CE9C8(u8 *, u8, u8);
u32 func_802A0CC8(s16, s32);

/* Load this file's objects from level data: reset the counters, then
 * read D_8039C710 0x38-byte entries (s16 x, y, z, model; positions are
 * stored scaled by 32, height snapped by func_802CE6F8, texture looked
 * up by func_802A0CC8), then D_8039C7F8 0x1C-byte records (s16 x, y, z,
 * u8 model, u8 n, s16 flag, then n 22-byte items for func_802CE9C8).
 * Flagged records also get a D_8039C800 box (+-40 units). */
void func_8028FDA0(u8 *arg0, u8 *arg1) {
    s32 i;

    D_8039C710 = 0;
    D_8039C7F8 = 0;
    D_8039C940 = 0;
    D_8039C944 = 0;
    D_8039C948 = 0;
    D_8039C94C = 0;
    D_8039C954 = 0;
    D_8039C958 = 0;
    D_8039C95C = 0;
    D_8039C950 = 0;
    D_803ED40C = 0;
    func_802CE9A4();
    if (arg0 != arg1) {
        D_8039C710 = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < D_8039C710; i++) {
            D_8039C550[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            D_8039C550[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            D_8039C550[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            D_8039C550[i].unk10 = *(s16 *) (arg0 + 6);
            arg0 += 8;
            D_8039C550[i].unk4 = func_802CE6F8(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk4);
            D_8039C550[i].unk0E = 0;
            D_8039C550[i].unk0C = 0;
            D_8039C550[i].unk11 = 0;
            D_8039C550[i].unk12 = 0;
            D_8039C550[i].unk20 = 0;
            D_8039C550[i].unk14 = 0;
            D_8039C550[i].unk18 = 0;
            D_8039C550[i].unk29 = 0;
            D_8039C550[i].unk30 = func_802A0CC8(D_802FDC08[D_8039C550[i].unk10].unk280, 0);
        }
        D_8039C7F8 = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < D_8039C7F8; i++) {
            D_8039C718[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            D_8039C718[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            D_8039C718[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            D_8039C718[i].unk10 = arg0[6];
            D_8039C718[i].unk0C = D_8039C718[i].unk4 - D_802FDC08[D_8039C718[i].unk10].unk288;
            D_8039C718[i].unk11 = 0;
            D_8039C718[i].unk14 = D_803FB8B0;
            func_802CE9C8(arg0 + 10, arg0[7], D_8039C718[i].unk10);
            D_8039C718[i].unk18 = D_803FB8B0;
            if (*(s16 *) (arg0 + 8) != 0) {
                D_8039C800[D_8039C940].unk0 = D_8039C718[i].unk0;
                D_8039C800[D_8039C940].unk4 = D_8039C718[i].unk4;
                D_8039C800[D_8039C940].unk8 = D_8039C718[i].unk8;
                D_8039C800[D_8039C940].unk0E = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk14 = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk1A = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk20 = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk0C = (D_8039C718[i].unk0 >> 5) - 40;
                D_8039C800[D_8039C940].unk10 = (D_8039C718[i].unk8 >> 5) - 40;
                D_8039C800[D_8039C940].unk12 = (D_8039C718[i].unk0 >> 5) + 40;
                D_8039C800[D_8039C940].unk16 = (D_8039C718[i].unk8 >> 5) - 40;
                D_8039C800[D_8039C940].unk18 = (D_8039C718[i].unk0 >> 5) - 40;
                D_8039C800[D_8039C940].unk1C = (D_8039C718[i].unk8 >> 5) + 40;
                D_8039C800[D_8039C940].unk1E = (D_8039C718[i].unk0 >> 5) + 40;
                D_8039C800[D_8039C940].unk22 = (D_8039C718[i].unk8 >> 5) + 40;
                D_8039C800[D_8039C940].unk26 = 0;
                D_8039C800[D_8039C940].unk27 = i;
                D_8039C940++;
            }
            arg0 = arg0[7] * 22 + arg0 + 10;
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/4B5E0/func_802906C0.s")

/* Flag (unk26) the first D_8039C800 entry whose unk27 is arg0. */
void func_80291724(s32 arg0) {
    u8 found;
    s32 i;

    found = 0;
    for (i = 0; i < D_8039C940;) {
        if (D_8039C800[i].unk27 == arg0) {
            D_8039C800[i].unk26 = 1;
            found = 1;
        } else {
            i++;
        }
        if (found) {
            break;
        }
    }
}

/* Draw every entry: a translated, textured copy of its model. */
void func_802917B0(Gfx **gdl, Dyn4B5E0 *dyn) {
    Gfx *gfx;
    s32 i;

    gfx = *gdl;
    if (D_8039C710 > 0) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
        gDPSetCombineMode(gfx++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
        gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    }
    for (i = 0; i < D_8039C710; i++) {
        guTranslate(&dyn->mtx[i], D_8039C550[i].unk0 / 32.0f, D_8039C550[i].unk4 / 32.0f,
                    D_8039C550[i].unk8 / 32.0f);
        gSPMatrix(gfx++, i * sizeof(Mtx) + 0xB00 + (u32) D_02000000, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gDPLoadTextureBlock(gfx++, D_8039C550[i].unk30 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                            D_802FDC08[D_8039C550[i].unk10].width, D_802FDC08[D_8039C550[i].unk10].height, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                            G_TX_NOLOD, G_TX_NOLOD);
        gSPDisplayList(gfx++, osVirtualToPhysical(&D_802FDC08[D_8039C550[i].unk10]));
        gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    }
    gDPPipeSync(gfx++);
    *gdl = gfx;
}

/* For entries tagged arg0 (unk28): clear unk0C, set unk29 and call
 * func_802AACD4 on their position. */
void func_80291ED8(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk28 == arg0) {
            D_8039C550[i].unk0C = 0;
            *(&D_8039C579 + i * 0x38) = 1;
            func_802AACD4(arg0, D_8039C550[i].unk0, D_8039C550[i].unk8, &D_8039C550[i].unk2A,
                          &D_8039C550[i].unk2C);
        }
    }
}

/* Update entries with unk29 set: move them (func_802AAE1C), then snap
 * their height with func_802CE6F8. */
void func_80291FAC(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk29 != 0) {
            func_802AAE1C(arg0, D_8039C550[i].unk2A, D_8039C550[i].unk2C, &D_8039C550[i],
                          &D_8039C550[i].unk8);
            D_8039C550[i].unk4 = func_802CE6F8(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk4);
        }
    }
}

/* Clear every entry's unk29. */
void func_80292084(void) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        D_8039C550[i].unk29 = 0;
    }
}

/* Set D_803A7424 if the point (arg0, arg1, arg2) is within arg3 of any
 * idle entry's model radius (all in 1/32 units). */
void func_802920DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i;
    s32 dist;

    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk11 == 0 && D_8039C550[i].unk29 == 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039C550[i].unk0 >> 5, D_8039C550[i].unk4 >> 5,
                                 D_8039C550[i].unk8 >> 5);
            if (dist <= (D_802FDC08[D_8039C550[i].unk10].radius >> 5) + arg3) {
                D_803A7424 = 1;
            }
        }
    }
}
