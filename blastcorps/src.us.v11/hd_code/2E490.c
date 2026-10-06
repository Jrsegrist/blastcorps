#include "common.h"
#include <ultra64.h>
#include "game/game.h"

extern u8 D_8036C360;

/* Sprite slots (this file's .bss, 0x8036BFE0) */
extern void *D_8036BFE0[64][2];    /* frame images */
extern u8 D_8036C1E0[64];          /* frame count */
extern u8 D_8036C220[64];          /* flags */
extern f32 D_8036C260[64];         /* scale */
extern Vtx *D_8036C368[2][64][2]; /* per-buffer quads, two orientations */


Gfx *func_802742D8(Gfx *gdl, u8 slot, s16 x, s16 y, s32 flip, s32 size, s32 half, f32 scale, u8 frame);

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

/* Pre-2.0I texture-rectangle form (G_RDPHALF_2/CONT = 0xB3/0xB2), as in 17E10.c */
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define OLD_RDPHALF_2 0xB3
#define OLD_RDPHALF_CONT 0xB2

#define gSPScisTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)             \
    {                                                                                        \
        Gfx *_g = (Gfx *) (pkt);                                                             \
                                                                                             \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(MAX(xh, 0), 12, 12) |            \
                        _SHIFTL(MAX(yh, 0), 0, 12));                                         \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(MAX(xl, 0), 12, 12) |                 \
                        _SHIFTL(MAX(yl, 0), 0, 12));                                         \
        gImmp1(pkt, OLD_RDPHALF_2,                                                           \
               (_SHIFTL(((s) - MIN(((xl) * (dsdx)) >> 7, 0)), 16, 16) |                      \
                _SHIFTL(((t) - MIN(((yl) * (dtdy)) >> 7, 0)), 0, 16)));                      \
        gImmp1(pkt, OLD_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));       \
    }


/* Draw sprite slot `slot` at (x, y): one 32-pixel-tall texture strip per frame column,
 * either through func_802742D8 (flag 4: vertex quads) or as scaled texture rectangles.
 * mode selects an optional tinted shadow pass and the main pass. */
Gfx *func_80272ED8(Gfx *arg0, u8 slot, s16 x, s16 y, u8 alpha, u8 mode, f32 scale) {
    s32 i;
    Gfx *gdl;
    u8 a;
    s32 xl;
    s32 yl;
    s32 half;
    s32 n;

    gdl = arg0;
    a = alpha;
    scale *= D_8036C260[slot];
    n = D_8036C1E0[slot];
    if ((mode & 8) && D_803156C4 % 20 < 7) {
        a >>= 1;
    }
    gDPPipeSync(gdl++);
    if (D_8036C220[slot] & 4) {
        if (scale > 1.0) {
            gDPSetTextureFilter(gdl++, G_TF_BILERP);
        } else {
            gDPSetTextureFilter(gdl++, G_TF_POINT);
        }
    }
    for (i = 0; i < n - ((D_8036C220[slot] & 8) ? 1 : 0); i++) {
        if (D_8036C220[slot] & 2) {
            gDPLoadTextureBlock(gdl++, D_8036BFE0[slot][i], G_IM_FMT_RGBA, G_IM_SIZ_32b, n << 5, 32, 0,
                                G_TX_CLAMP, G_TX_MIRROR, G_TX_NOMASK, n + 4, G_TX_NOLOD, G_TX_NOLOD);
        } else {
            gDPLoadTextureBlock(gdl++, D_8036BFE0[slot][i], G_IM_FMT_RGBA, G_IM_SIZ_16b, n << 5, 32, 0,
                                G_TX_CLAMP, G_TX_MIRROR, G_TX_NOMASK, n + 4, G_TX_NOLOD, G_TX_NOLOD);
        }
        if (mode) {
            gDPPipeSync(gdl++);
            if (mode & 4) {
                gDPSetPrimColor(gdl++, 0, 0, alpha, alpha, alpha, 0xFF);
            } else {
                gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, alpha / 2);
            }
            half = n + 2.0f * scale;
            gDPSetRenderMode(gdl++, 0x00504240, 0);
            gDPSetCombine(gdl++, 0xFF97FF, 0xFF2DFEFF);
            if (D_8036C220[slot] & 4) {
                gdl = func_802742D8(gdl, slot, x, y, i, n, half, scale, 1);
            } else {
                xl = (x - half) * 4;
                yl = (y + half) * 4 + (i << 5) * 4 * scale;
                gSPScisTextureRectangleOld(gdl++, xl, yl, x * 4 + (((n << 5) - half - 1) << 2) * scale,
                                           (y + half) * 4 + (((i << 5) + 32) << 2) * scale, 0, 0,
                                           (D_8036C220[slot] & 1) ? (n << 5) << 5 : 0,
                                           (s32) (1024.0f / scale), (s32) (1024.0f / scale));
            }
        }
        if (!(mode & 6)) {
            if (!(mode & 1)) {
                half = n + 2.0f * scale;
            } else {
                half = 0;
            }
            gDPPipeSync(gdl++);
            gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0, 0, a);
            if ((D_8036C220[slot] & 2) || a != 0xFF || !(D_80364A90 & 0xC9FD0FE79BFF80B0LL)) {
                gDPSetRenderMode(gdl++, 0x00504240, 0);
            } else {
                gDPSetRenderMode(gdl++, 0x0F0A7008, 0);
            }
            gDPSetCombine(gdl++, 0x15162A, 0xFF2FFFFF);
            if (D_8036C220[slot] & 4) {
                gdl = func_802742D8(gdl, slot, x, y, i, n, half, scale, 0);
            } else {
                xl = (x - half) * 4;
                yl = (y + half) * 4 + (i << 5) * 4 * scale;
                gSPScisTextureRectangleOld(gdl++, xl, yl, x * 4 + (((n << 5) - half - 1) << 2) * scale,
                                           (y + half) * 4 + (((i << 5) + 32) << 2) * scale, 0, 0,
                                           (D_8036C220[slot] & 1) ? (n << 5) << 5 : 0,
                                           (s32) (1024.0f / scale), (s32) (1024.0f / scale));
            }
        }
    }
    gDPPipeSync(gdl++);
    return gdl;
}

Gfx *func_802742D8(Gfx *gdl, u8 slot, s16 x, s16 y, s32 flip, s32 size, s32 half, f32 scale, u8 frame) {
    Vtx *vtx = D_8036C368[D_8035805C][slot][frame];
    s32 x0 = x - half;
    s32 y0 = y + half;
    s32 t;

    if (D_8036C220[slot] & 1) {
        t = 0;
    } else {
        t = size << 5;
    }
    if (flip) {
        vtx += 4;
        vtx[0].v.ob[0] = x0;
        vtx[0].v.ob[1] = y0 + 64.0f * scale - 1.0f;
        vtx[0].v.ob[2] = -10;
        vtx[0].v.tc[0] = 0;
        vtx[0].v.tc[1] = t << 5;
        vtx[1].v.ob[0] = x0 + (size << 5) * scale - 1.0f;
        vtx[1].v.ob[1] = y0 + 64.0f * scale - 1.0f;
        vtx[1].v.ob[2] = -10;
        vtx[1].v.tc[0] = ((size << 5) - 1) << 5;
        vtx[1].v.tc[1] = t << 5;
        vtx[2].v.ob[0] = x0 + (size << 5) * scale - 1.0f;
        vtx[2].v.ob[1] = y0 + 32.0f * scale - 1.0f;
        vtx[2].v.ob[2] = -10;
        vtx[2].v.tc[0] = ((size << 5) - 1) << 5;
        vtx[2].v.tc[1] = (t + 31) << 5;
        vtx[3].v.ob[0] = x0;
        vtx[3].v.ob[1] = y0 + 32.0f * scale - 1.0f;
        vtx[3].v.ob[2] = -10;
        vtx[3].v.tc[0] = 0;
        vtx[3].v.tc[1] = (t + 31) << 5;
    } else {
        vtx[0].v.ob[0] = x0;
        vtx[0].v.ob[1] = y0;
        vtx[0].v.ob[2] = -10;
        vtx[0].v.tc[0] = 0;
        vtx[0].v.tc[1] = (t + 31) << 5;
        vtx[1].v.ob[0] = x0 + (size << 5) * scale - 1.0f;
        vtx[1].v.ob[1] = y0;
        vtx[1].v.ob[2] = -10;
        vtx[1].v.tc[0] = ((size << 5) - 1) << 5;
        vtx[1].v.tc[1] = (t + 31) << 5;
        vtx[2].v.ob[0] = x0 + (size << 5) * scale - 1.0f;
        vtx[2].v.ob[1] = y0 + 32.0f * scale - 1.0f;
        vtx[2].v.ob[2] = -10;
        vtx[2].v.tc[0] = ((size << 5) - 1) << 5;
        vtx[2].v.tc[1] = t << 5;
        vtx[3].v.ob[0] = x0;
        vtx[3].v.ob[1] = y0 + 32.0f * scale - 1.0f;
        vtx[3].v.ob[2] = -10;
        vtx[3].v.tc[0] = 0;
        vtx[3].v.tc[1] = t << 5;
    }
    gSPVertex(gdl++, vtx, 4, 0);
    osWritebackDCache(vtx, sizeof(Vtx) * 4);
    gSP1Triangle(gdl++, 0, 3, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 1, 0);
    return gdl;
}

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
