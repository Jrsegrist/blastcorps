#include "common.h"
#include <ultra64.h>

/* This file's .bss starts at 0x8036C790 */
extern s16 *D_8036C790;
extern s16 *D_8036C794;
extern s16 *D_8036C7A0[10]; /* point lists to keep on screen */
extern u8 D_8036C7CC;
extern u8 D_02000000[]; /* segment 2 base */
extern u8 D_802FA940[]; /* 32x32 IA8 texture */

void func_80276130(s32 arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1,
                   u8 r2, u8 g2, u8 b2, u8 a2, u8 r3, u8 g3, u8 b3, u8 a3);
void func_8027690C(void *arg0, f32 x, f32 y, f32 z, s16 *sx, s16 *sy, s32 arg6, s32 arg7, s32 arg8, f32 arg9);

void func_80275430(void) {
    s32 i;

    for (i = 0; i < 10; i++) {
        D_8036C7A0[i] = 0;
    }
    D_8036C794 = 0;
    D_8036C7CC = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80275478.s")

Gfx *func_80275DA4(Gfx *gfx, u8 arg1) {
    Gfx *gdl = gfx;

    if (!arg1) {
        gSPMatrix(gdl++, (u32) D_02000000 + 0xC0, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(gdl++, (u32) D_02000000 + 0x1C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    }
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gdl++, (u32) D_802FA940 - 0x80000000, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0, G_TX_CLAMP,
                        G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    return gdl;
}

void func_80276080(s32 arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u8 r, u8 g, u8 b, u8 a) {
    func_80276130(arg0, arg1, arg2, arg3, arg4, arg5, arg6, r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80276130.s")

void func_8027656C(void *arg0) {
    s32 i;
    s32 j;
    s16 *pts;
    s16 sx;
    s16 sy;
    s16 px;
    s16 py;
    s16 minX;
    s16 maxX;
    s16 minY;
    s16 maxY;
    u8 found;

    for (i = 0; i < 10; i++) {
        if (D_8036C7A0[i]) {
            minX = 0x7FFF, maxX = -0x8000;
            minY = 0x7FFF, maxY = -0x8000;
            pts = D_8036C7A0[i];
            for (j = 0; j < 4; j++) {
                func_8027690C(arg0, pts[0], pts[1], pts[2], &sx, &sy, 0, 0, 0, 1.0f);
                pts += 3;
                if (sx < minX) {
                    minX = sx;
                }
                if (sx > maxX) {
                    maxX = sx;
                }
                if (sy < minY) {
                    minY = sy;
                }
                if (sy > maxY) {
                    maxY = sy;
                }
            }
            j = 0;
            found = 0;
            while (j < 4 && !found) {
                switch (j) {
                    case 0:
                        px = (maxX - minX) / 2 + minX;
                        py = minY;
                        break;
                    case 1:
                        px = (maxX - minX) / 2 + minX;
                        py = maxY;
                        break;
                    case 2:
                        px = minX;
                        py = (maxY - minY) / 2 + minY;
                        break;
                    case 3:
                        px = maxX;
                        py = (maxY - minY) / 2 + minY;
                        break;
                }
                if (px < 310 && px >= 11 && py < 230 && py >= 11) {
                    found = 1;
                }
                j++;
            }
            if (!found) {
                D_8036C7A0[i] = NULL;
            }
        }
    }
}

void func_8027684C(void) {
    s32 i;

    i = 0;
    while (i < 10) {
        if (!D_8036C7A0[i]) {
            D_8036C7A0[i] = D_8036C794;
            return;
        }
        i++;
    }
}

s32 func_802768A8(void) {
    s32 i;

    i = 0;
    while (i < 10) {
        if (D_8036C7A0[i] && D_8036C7A0[i] == D_8036C790) {
            return 1;
        }
        i++;
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_8027690C.s")

void func_80276D1C(Mtx *m, f32 x, f32 y, f32 z, s32 arg4, f32 *ox, f32 *oy, f32 *oz, f32 *ow) {
    f32 mf[4][4];

    guMtxL2F(mf, m);
    *ox = mf[0][0] * x + mf[1][0] * y + mf[2][0] * z + mf[3][0];
    *oy = mf[0][1] * x + mf[1][1] * y + mf[2][1] * z + mf[3][1];
    *oz = mf[0][2] * x + mf[1][2] * y + mf[2][2] * z + mf[3][2];
    *ow = mf[0][3] * x + mf[1][3] * y + mf[2][3] * z + mf[3][3];
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30C70/func_80276E50.s")
