#include "common.h"
#include <ultra64.h>

typedef struct {
    u16 key;
    s32 vtx;
    u8 *tex;
} TextQuad; /* size 0xC */

extern u64 D_80364A98;
extern s32 D_802E8BDC;
extern s32 D_80365350;
extern u8 *D_80358070;
extern Vtx *D_80365348[2];
extern TextQuad *D_80365340;
extern s32 D_802E8C70;
extern s32 D_802E8C74;
extern s32 D_802E8C78;
extern u8 D_8035805C;

void func_8025B070(void);
void func_8025946C(Gfx **gdlp, s32 arg1);
void func_80259824(Gfx **gdlp, s32 arg1);
void func_802595E0(u8 *base, s32 n, s32 size, s32 (*cmp)(void *, void *));
void func_802597D8(u8 *dst, u8 *src, s32 n);
s32 func_80259814(u16 *arg0, u16 *arg1);

void func_802592F0(void) {
    s32 i;

    if ((D_80364A98 & 0xC9FD8FE7DBFF8080) || D_80364A98 == 0x100000000000 || D_80364A98 == 2 ||
        D_802E8BDC == 0x28 || D_802E8BDC == 0x32) {
        D_80365350 = 0x200;
    } else if (D_80364A98 == 0x40) {
        D_80365350 = 0x9C;
    } else {
        D_80365350 = 0xAC;
    }
    for (i = 0; i < 2; i++) {
        D_80365348[i] = D_80358070;
        D_80358070 += D_80365350 * 16 * 4;
    }
    D_80365340 = D_80358070;
    D_80358070 += D_80365350 * 12;
    func_8025B070();
}

void func_80259450(void) {
    D_802E8C70 = 0;
    D_802E8C74 = 0;
    D_802E8C78 = 0;
}

void func_8025946C(Gfx **gdlp, s32 arg1) {
    Gfx *gdl = *gdlp;

    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    *gdlp = gdl;
}

/* Shell sort (h = 3h + 1) of n elements of the given size. */
void func_802595E0(u8 *base, s32 n, s32 size, s32 (*cmp)(void *, void *)) {
    s32 i;
    s32 j;
    s32 h;
    u8 *b;
    u8 tmp[256];

    b = base;
    for (h = 1; h <= n / 9; h = 3 * h + 1) {
    }
    for (; h > 0; h /= 3) {
        for (i = h; i < n; i++) {
            func_802597D8(tmp, b + size * i, size);
            j = i;
            while (j >= h && cmp(b + (j - h) * size, tmp) > 0) {
                func_802597D8(b + size * j, b + (j - h) * size, size);
                j -= h;
            }
            func_802597D8(b + size * j, tmp, size);
        }
    }
}

void func_802597D8(u8 *dst, u8 *src, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

s32 func_80259814(u16 *arg0, u16 *arg1) {
    return *arg0 - *arg1;
}

void func_80259824(Gfx **gdlp, s32 arg1) {
    u8 *tex = NULL;
    Gfx *gdl = *gdlp;
    s32 i;

    func_802595E0((u8 *) &D_80365340[D_802E8C70], D_802E8C74 - D_802E8C70, sizeof(TextQuad), func_80259814);
    for (i = D_802E8C70; i < D_802E8C74; i++) {
        if (D_80365340[i].tex != tex) {
            tex = D_80365340[i].tex;
            if (i == D_802E8C70) {
                gDPLoadTextureBlock_4b(gdl++, OS_K0_TO_PHYSICAL(tex), G_IM_FMT_I, 32, 32, 0, G_TX_CLAMP, G_TX_CLAMP,
                                       G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPSetTextureImage(gdl++, G_IM_FMT_I, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(tex));
                gDPLoadSync(gdl++);
                gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 0xFF, 0x400);
            }
        }
        gSPVertex(gdl++, &D_80365348[D_8035805C][D_80365340[i].vtx * 4], 4, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    osWritebackDCache(&D_80365348[D_8035805C][D_802E8C70 * 4], (D_802E8C74 - D_802E8C70) * 64);
    D_802E8C70 = D_802E8C74;
    *gdlp = gdl;
}

void func_80259BD4(Gfx **gdlp, s32 arg1) {
    Gfx *gdl = *gdlp;

    func_8025946C(&gdl, arg1);
    func_80259824(&gdl, arg1);
    *gdlp = gdl;
}

void func_80259C24(Gfx **gdlp, Mtx *arg1) {
    Gfx *gdl = *gdlp;

    gSPMatrix(gdl++, &arg1[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &arg1[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    func_8025946C(&gdl, (s32) arg1);
    func_80259824(&gdl, (s32) arg1);
    *gdlp = gdl;
}

void func_80259EC4(s32 arg0, s32 arg1, s32 arg2, u8 arg3, s32 arg4, f32 arg5, s32 arg6, f32 arg7, s32 arg8, u8 arg9,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1, u8 r2, u8 g2, u8 b2, u8 a2,
                   u8 r3, u8 g3, u8 b3, u8 a3);

void func_80259CCC(s32 arg0, s32 arg1, s32 arg2, u8 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, u8 arg9,
                   u8 r, u8 g, u8 b, u8 a) {
    func_80259EC4(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, r, g, b, a, r, g, b, a, r, g, b, a, r,
                  g, b, a);
}

void func_80259DC8(s32 arg0, s32 arg1, s32 arg2, u8 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, u8 arg9,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1) {
    func_80259EC4(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, r0, g0, b0, a0, r0, g0, b0, a0, r1,
                  g1, b1, a1, r1, g1, b1, a1);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/14B30/func_80259EC4.s")
