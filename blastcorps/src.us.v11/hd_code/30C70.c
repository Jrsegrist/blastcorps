#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* This file's .bss starts at 0x8036C790 */
extern s16 *D_8036C790;
extern s16 *D_8036C794;
extern s32 D_8036C798;
extern s16 *D_8036C7A0[10]; /* point lists to keep on screen */
extern Vtx D_8036C7D0[][4];   /* off-screen marker quads */
extern Mtx D_8036C850[];      /* off-screen marker matrices */
extern s16 D_8036443E;        /* camera yaw, 4095 = 360 degrees */
extern u32 D_80364AA8;
extern s32 D_802FAD40; /* target bounce offset */
extern s32 D_802FAD44; /* target bounce count */
extern u8 D_802FAD48;  /* target bounce direction */
extern u8 D_02000000[]; /* segment 2 base */

typedef struct SpriteVtxBuf {
    u8 pad[0x1E00];
    Vtx vtx[1]; /* sprite quads */
} SpriteVtxBuf;

void func_80276D1C(Mtx *m, f32 x, f32 y, f32 z, f32 w, f32 *ox, f32 *oy, f32 *oz, f32 *ow);
s32 func_802768A8(void);
void func_8027656C(void *arg0);

void func_80275430(void) {
    s32 i;

    for (i = 0; i < 10; i++) {
        D_8036C7A0[i] = 0;
    }
    D_8036C794 = 0;
    D_8036C7CC = 0;
}

/* Picks the on-screen target marker, colours it by time left, bounces it and draws its four arrows */
void func_80275478(SpriteVtxBuf *arg0, Gfx **gfxp, u8 arg2) {
    s16 i;
    s16 k;
    s16 vtxIdx = 16;
    Gfx *gdl = *gfxp;
    s16 sx;
    s16 sy;
    s16 px;
    s16 py;
    s16 minX;
    s16 maxX;
    s16 minY;
    s16 maxY;
    u8 r;
    u8 g;
    s16 t;
    s16 *pts;
    s32 ret;

    minX = 0x7FFF, maxX = -0x8000, minY = 0x7FFF, maxY = -0x8000;
    ret = func_802BCE40();
    pts = D_8036C790;
    D_8036C7CC = 0;
    if (!D_8036EB98 && !ret && D_80364AA8 == 1 && !D_802E8BD0) {
        if (!func_8026AD30(0x4B)) {
            func_8026AF6C(0x803D);
            func_80277EDC(3, 1, 2, 0x82);
        }
        D_8036EB98 = 1;
    }
    if (D_8036C790 != NULL && (!func_802768A8() || arg2) && D_8036C794 == NULL) {
        for (i = 0; i < 4; i++) {
            func_8027690C(arg0, D_8036C790[0], D_8036C790[1], D_8036C790[2], &sx, &sy, NULL, NULL, NULL, 1.0f);
            D_8036C790 += 3;
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
        D_8036C7CC = 0;
        for (k = 0; k < 4; k++) {
            switch (k) {
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
                D_8036C794 = pts;
                D_8036C798 = ret;
                D_803F7809 = D_803F7808;
                D_802FAD44 = 0;
            }
        }
    }
    if (D_8036C794 != NULL) {
        func_802BD10C(D_8036C798);
        if (D_8036C7C8 >= 1501 || !D_803643DB) {
            g = 0xFF;
            r = 0;
        } else if (D_8036C7C8 < 500) {
            r = 0xFF;
            g = 0;
        } else {
            t = (D_8036C7C8 - 500) / 1000.0f * 511.0f;
            if (t < 256) {
                g = t, r = 0xFF;
            } else {
                g = 0xFF, r = 0x1FE - t;
            }
        }
        pts = D_8036C794;
        for (i = 0; i < 4; i++) {
            func_8027690C(arg0, pts[0], pts[1], pts[2], &sx, &sy, NULL, NULL, NULL, 1.0f);
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
        for (k = 0; k < 4; k++) {
            switch (k) {
                case 0:
                    px = (maxX - minX) / 2 + minX, py = minY - D_802FAD40;
                    break;
                case 1:
                    px = (maxX - minX) / 2 + minX, py = maxY + D_802FAD40;
                    break;
                case 2:
                    px = minX - D_802FAD40;
                    py = (maxY - minY) / 2 + minY;
                    break;
                case 3:
                    px = maxX + D_802FAD40;
                    py = (maxY - minY) / 2 + minY;
                    break;
            }
            if (px < 310 && px >= 11 && py < 230 && py >= 11) {
                D_8036C7CC++;
            }
            vtxIdx = func_80276080(arg0, k, vtxIdx, px, py, 8, 8, r, g, 0, 0xFF);
        }
        if (!D_802FAD48) {
            D_802FAD40++;
            if (D_802FAD40 == 10) {
                D_802FAD48 = 1;
                D_802FAD44++;
            }
        } else {
            D_802FAD40--;
            if (D_802FAD40 == 0) {
                D_802FAD48 = 0;
                D_802FAD44++;
            }
        }
        if (D_802FAD44 == 5) {
            D_8036C794 = NULL;
        }
        gdl = func_80275DA4(gdl, 0);
        gSPVertex(gdl++, (u32) D_02000000 + 0x1F00, 16, 0);
        vtxIdx = 0;
        for (k = 0; k < 4; k++) {
            gSP1Triangle(gdl++, vtxIdx, vtxIdx + 1, vtxIdx + 2, 0);
            gSP1Triangle(gdl++, vtxIdx, vtxIdx + 2, vtxIdx + 3, 0);
            vtxIdx += 4;
        }
    }
    func_8027656C(arg0);
    *gfxp = gdl;
}

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

s32 func_80276080(SpriteVtxBuf *arg0, u8 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u8 r, u8 g, u8 b, u8 a) {
    return func_80276130(arg0, arg1, arg2, arg3, arg4, arg5, arg6, r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a);
}

/* Writes a quad of 4 Vtx centred on (x, y), half size (w, h); arg1 picks the texture orientation */
s32 func_80276130(SpriteVtxBuf *arg0, u8 arg1, s32 arg2, s32 x, s32 y, s32 w, s32 h,
                  u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1,
                  u8 r2, u8 g2, u8 b2, u8 a2, u8 r3, u8 g3, u8 b3, u8 a3) {
    s32 pad;

    switch (arg1) {
        case 0:
            arg0->vtx[arg2].v.tc[0] = 0, arg0->vtx[arg2].v.tc[1] = 0;
            arg0->vtx[arg2 + 1].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 1].v.tc[1] = 0;
            arg0->vtx[arg2 + 2].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 2].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 3].v.tc[0] = 0, arg0->vtx[arg2 + 3].v.tc[1] = 0x3E0;
            break;
        case 1:
            arg0->vtx[arg2].v.tc[0] = 0, arg0->vtx[arg2].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 1].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 1].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 2].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 2].v.tc[1] = 0;
            arg0->vtx[arg2 + 3].v.tc[0] = 0, arg0->vtx[arg2 + 3].v.tc[1] = 0;
            break;
        case 2:
            arg0->vtx[arg2].v.tc[0] = 0, arg0->vtx[arg2].v.tc[1] = 0;
            arg0->vtx[arg2 + 1].v.tc[0] = 0, arg0->vtx[arg2 + 1].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 2].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 2].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 3].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 3].v.tc[1] = 0;
            break;
        case 3:
            arg0->vtx[arg2].v.tc[0] = 0, arg0->vtx[arg2].v.tc[1] = 0x3E0;
            arg0->vtx[arg2 + 1].v.tc[0] = 0, arg0->vtx[arg2 + 1].v.tc[1] = 0;
            arg0->vtx[arg2 + 2].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 2].v.tc[1] = 0;
            arg0->vtx[arg2 + 3].v.tc[0] = 0x3E0, arg0->vtx[arg2 + 3].v.tc[1] = 0x3E0;
            break;
    }
    arg0->vtx[arg2].v.ob[0] = x - w;
    arg0->vtx[arg2].v.ob[1] = y - h;
    arg0->vtx[arg2].v.ob[2] = -10;
    arg0->vtx[arg2].v.flag = 0;
    arg0->vtx[arg2].v.cn[0] = r0;
    arg0->vtx[arg2].v.cn[1] = g0;
    arg0->vtx[arg2].v.cn[2] = b0;
    arg0->vtx[arg2].v.cn[3] = a0;
    arg2++;
    arg0->vtx[arg2].v.ob[0] = x + w;
    arg0->vtx[arg2].v.ob[1] = y - h;
    arg0->vtx[arg2].v.ob[2] = -10;
    arg0->vtx[arg2].v.flag = 0;
    arg0->vtx[arg2].v.cn[0] = r1;
    arg0->vtx[arg2].v.cn[1] = g1;
    arg0->vtx[arg2].v.cn[2] = b1;
    arg0->vtx[arg2].v.cn[3] = a1;
    arg2++;
    arg0->vtx[arg2].v.ob[0] = x + w;
    arg0->vtx[arg2].v.ob[1] = y + h;
    arg0->vtx[arg2].v.ob[2] = -10;
    arg0->vtx[arg2].v.flag = 0;
    arg0->vtx[arg2].v.cn[0] = r2;
    arg0->vtx[arg2].v.cn[1] = g2;
    arg0->vtx[arg2].v.cn[2] = b2;
    arg0->vtx[arg2].v.cn[3] = a2;
    arg2++;
    arg0->vtx[arg2].v.ob[0] = x - w;
    arg0->vtx[arg2].v.ob[1] = y + h;
    arg0->vtx[arg2].v.ob[2] = -10;
    arg0->vtx[arg2].v.flag = 0;
    arg0->vtx[arg2].v.cn[0] = r3;
    arg0->vtx[arg2].v.cn[1] = g3;
    arg0->vtx[arg2].v.cn[2] = b3;
    arg0->vtx[arg2].v.cn[3] = a3;
    arg2++;
    return arg2;
}

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

/* Projects a point to screen coordinates, clamped to +-0x4000 (0x4000 when behind the camera) */
void func_8027690C(void *arg0, f32 x, f32 y, f32 z, s16 *sx, s16 *sy, Mtx *arg6, Mtx *arg7, Mtx *arg8, f32 arg9) {
    f32 w = 1.0f;

    if (arg8 != NULL) {
        func_80276D1C(arg8, x, y, z, w, &x, &y, &z, &w);
    }
    if (arg7 != NULL) {
        func_80276D1C(arg7, x, y, z, w, &x, &y, &z, &w);
    }
    if (arg6 != NULL) {
        func_80276D1C(arg6, x, y, z, w, &x, &y, &z, &w);
    }
    func_80276D1C((Mtx *) ((u8 *) arg0 + 0x140), x, y, z, w, &x, &y, &z, &w);
    if (z >= 0.0) {
        *sx = 0x4000;
        *sy = 0x4000;
        return;
    }
    func_80276D1C((Mtx *) ((u8 *) arg0 + 0x80), x, y, z, w, &x, &y, &z, &w);
    x = x * (D_8035807C / 65535.0);
    y = y * (D_8035807C / 65535.0);
    w = w * (D_8035807C / 65535.0);
    x = x / w;
    y = y / w;
    x = ((320.0f * arg9) / 2.0f) * x;
    y = ((240.0f * arg9) / 2.0f) * y;
    x = ((320.0f * arg9) / 2.0f) + x;
    y = ((240.0f * arg9) / 2.0f) + y;
    y = (240.0f * arg9) - y;
    if (((x > 0.0f) ? x : -x) >= 16384.0f) {
        x = ((x >= 0.0f) ? 1 : -1) << 14;
    }
    if (((y > 0.0f) ? y : -y) >= 16384.0f) {
        y = ((y >= 0.0f) ? 1 : -1) << 14;
    }
    *sx = x;
    *sy = y;
}

void func_80276D1C(Mtx *m, f32 x, f32 y, f32 z, f32 w, f32 *ox, f32 *oy, f32 *oz, f32 *ow) {
    f32 mf[4][4];

    guMtxL2F(mf, m);
    *ox = mf[0][0] * x + mf[1][0] * y + mf[2][0] * z + mf[3][0];
    *oy = mf[0][1] * x + mf[1][1] * y + mf[2][1] * z + mf[3][1];
    *oz = mf[0][2] * x + mf[1][2] * y + mf[2][2] * z + mf[3][2];
    *ow = mf[0][3] * x + mf[1][3] * y + mf[2][3] * z + mf[3][3];
}

/* Draws a marker at the screen edge pointing towards an off-screen point (x, y, z in 1/32 units) */
void func_80276E50(Gfx **gfxp, void *view, u8 idx, s32 x, s32 y, s32 z) {
    s16 sx;
    s16 sy;
    Gfx *gdl = *gfxp;
    f32 mf[4][4];
    f32 rot[4][4];

    func_8027690C(view, x >> 5, y >> 5, z >> 5, &sx, &sy, NULL, NULL, NULL, 1.0f);
    if (sx < 30 || sx >= 291 || sy < 25 || sy >= 216) {
        if (sx < 30) {
            sx = 30;
        }
        if (sy < 25) {
            sy = 25;
        }
        if (sx >= 291) {
            sx = 290;
        }
        if (sy >= 216) {
            sy = 215;
        }
        D_8036C7D0[idx][0].v.ob[0] = -15;
        D_8036C7D0[idx][0].v.ob[1] = -15;
        D_8036C7D0[idx][0].v.ob[2] = -10;
        D_8036C7D0[idx][0].v.tc[0] = 0;
        D_8036C7D0[idx][0].v.tc[1] = 0;
        D_8036C7D0[idx][1].v.ob[0] = 15;
        D_8036C7D0[idx][1].v.ob[1] = -15;
        D_8036C7D0[idx][1].v.ob[2] = -10;
        D_8036C7D0[idx][1].v.tc[0] = 0x3E0;
        D_8036C7D0[idx][1].v.tc[1] = 0;
        D_8036C7D0[idx][2].v.ob[0] = 15;
        D_8036C7D0[idx][2].v.ob[1] = 15;
        D_8036C7D0[idx][2].v.ob[2] = -10;
        D_8036C7D0[idx][2].v.tc[0] = 0x3E0;
        D_8036C7D0[idx][2].v.tc[1] = 0x3E0;
        D_8036C7D0[idx][3].v.ob[0] = -15;
        D_8036C7D0[idx][3].v.ob[1] = 15;
        D_8036C7D0[idx][3].v.ob[2] = -10;
        D_8036C7D0[idx][3].v.tc[0] = 0;
        D_8036C7D0[idx][3].v.tc[1] = 0x3E0;
        guTranslateF(mf, sx, sy, 0.0f);
        guRotateF(rot, 135.0 - (D_8036443E / 4095.0) * 360.0, 0.0f, 0.0f, 1.0f);
        guMtxCatF(rot, mf, mf);
        guMtxF2L(mf, &D_8036C850[idx]);
        gSPMatrix(gdl++, (u32) D_02000000 + 0xC0, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(gdl++, &D_8036C850[idx], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombine(gdl++, 0x119623, 0xFF2FFFFF);
        gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0x00, 0xFF);
        gDPLoadTextureBlock(gdl++, (u32) D_802FA940 - 0x80000000, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0, G_TX_CLAMP,
                            G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPVertex(gdl++, (u32) D_8036C7D0[idx] - 0x80000000, 4, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
        gDPPipeSync(gdl++);
    }
    *gfxp = gdl;
}
