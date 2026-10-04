#include "common.h"
#include <ultra64.h>

extern u8 D_8036C360;
extern u32 D_803156C4;
extern u8 D_803B9888;
extern void *D_80358070;

/* Sprite slots (this file's .bss, 0x8036BFE0) */
extern void *D_8036BFE0[64][2];    /* frame images */
extern u8 D_8036C1E0[64];          /* frame count */
extern u8 D_8036C220[64];          /* flags */
extern f32 D_8036C260[64];         /* scale */
extern void *D_8036C368[2][64][2];

void func_802A0700(void);
void func_80257490(void **heap, s32 align);
void func_802A0EE0(u16 id, void *dest);
void func_802A0B00(u16 id, void *pal);

Gfx *func_80272ED8(Gfx *gdl, u8 arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5, f32 arg6);

void func_80272C50(void) {
    D_8036C360 = 0;
}

u8 func_80272C5C(u16 *ids, u16 *palIds, u8 count, u8 frames, u8 flags, f32 scale) {
    void *pal;
    s32 slot;
    s32 j;
    s32 m;
    s32 k;
    s32 start = D_8036C360;

    if (!D_803B9888) {
        func_802A0700();
    }
    k = 0;
    for (slot = start; slot < count + start; slot++, k += palIds ? 0 : 1) {
        if (palIds) {
            func_80257490(&D_80358070, 16);
            func_802A0EE0(palIds[slot - start], pal = D_80358070);
            D_80358070 = (u8 *) D_80358070 + 0x80;
        } else {
            func_80257490(&D_80358070, 16);
            pal = NULL;
        }
        for (j = 0; j < frames; j++) {
            D_8036BFE0[slot][j] = D_80358070;
            func_802A0B00(ids[frames * k + j], pal);
        }
        D_8036C1E0[slot] = frames;
        D_8036C220[slot] = flags;
        if (flags & 4) {
            for (j = 0; j < 2; j++) {
                for (m = 0; m < 2; m++) {
                    D_8036C368[j][slot][m] = D_80358070;
                    D_80358070 = (u8 *) D_80358070 + 0x80;
                }
            }
        }
        D_8036C260[slot] = scale;
    }
    D_8036C360 = slot;
    return start;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2E490/func_80272ED8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2E490/func_802742D8.s")

Gfx *func_80274868(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPTexture(gdl++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetTexturePersp(gdl++, G_TP_NONE);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    return gdl;
}

Gfx *func_80274998(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    return gdl;
}

Gfx *func_80274AA4(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    return gdl;
}

Gfx *func_80274B08(Gfx *gfx) {
    Gfx *gdl = gfx;

    gDPPipeSync(gdl++);
    return gdl;
}

void func_80274B40(Gfx **gfx, s32 arg1, u8 arg2, s16 arg3, s16 arg4) {
    Gfx *gdl = *gfx;

    if (D_803156C4 % 40 < 28) {
        gdl = func_80274868(gdl);
        gdl = func_80272ED8(gdl, arg2, arg3, arg4, 0xFF, 1, 1.0f);
        gdl = func_80274AA4(gdl);
    }
    *gfx = gdl;
}
