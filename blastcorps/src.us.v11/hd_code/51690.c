#include "common.h"
#include <ultra64.h>
#include "game/game.h"

extern u8 *D_8039CA90;
extern u8 *D_8039CA94;
extern u8 *D_8039CA98;
extern u8 *D_8039CA9C;

/* Carves four buffers out of the heap and loads ROM asset 0x6A8DA0 into the first. */
void func_80295E50(void) {
    s32 size;

    D_8039CA90 = D_80358070;
    D_8039CA94 = D_8039CA90 + 0x3200;
    D_8039CA98 = D_8039CA94 + 0xF0;
    D_8039CA9C = D_8039CA98 + 0x180;
    size = D_006A9F10 - D_006A8DA0;
    func_8028B4C4(D_006A8DA0, D_8039CA90, &size, 10, 0, 1);
    D_80358070 = (u8 *) D_80358070 + size;
    D_8039CAA0 = 0;
    D_8039CAA2 = 1;
}

/* Pre-2.0I scissored texture rectangle (see 17E10.c): G_RDPHALF_2/G_RDPHALF_CONT
 * are 0xB3/0xB2 and there are no (s16) casts. */
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

Gfx *func_8029700C(Gfx *gfx, s16 x, s16 y);

/*
 * Draws the on-screen controller at (x, y) with translucency alpha: the
 * pad image (with a drop shadow), Z and L/R trigger highlights, coloured
 * A/B/Start and D-pad left/right button highlights, and the stick position.
 */
Gfx *func_80295EFC(s32 arg0, Gfx *arg1, s16 x, s16 y, u8 alpha) {
    Gfx *gfx;
    s32 i;

    gfx = arg1;
    gDPPipeSync(gfx++);
    gSPTexture(gfx++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetTexturePersp(gfx++, G_TP_NONE);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetRenderMode(gfx++, 0x00404240, 0);
    gDPSetCombine(gfx++, 0xFF97FF, 0xFF2DFEFF);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, alpha / 2);
    gfx = func_8029700C(gfx, x - 4, y + 4);
    gDPPipeSync(gfx++);
    gDPSetCombine(gfx++, 0xFF97FF, 0xFF2CFE7F);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, alpha);
    gfx = func_8029700C(gfx, x, y);
    if (D_80370C30 & 0x30) {
        gDPLoadTextureBlock(gfx++, D_8039CA98, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 6, 0, G_TX_MIRROR,
                            G_TX_CLAMP, 5, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        if (D_80370C30 & 0x10) {
            gSPScisTextureRectangleOld(gfx++, (x + 0x30) << 2, (y + 8) << 2, (x + 0x4F) << 2,
                                       (y + 0xD) << 2, 0, 0, 0, 1 << 10, 1 << 10);
        }
        if (D_80370C30 & 0x20) {
            gSPScisTextureRectangleOld(gfx++, x << 2, (y + 8) << 2, (x + 0x1F) << 2, (y + 0xD) << 2, 0,
                                       32 << 5, 0, 1 << 10, 1 << 10);
        }
    }
    if (D_80370C30 & 0x2000) {
        gDPLoadTextureBlock(gfx++, D_8039CA9C, G_IM_FMT_RGBA, G_IM_SIZ_16b, 24, 24, 0, G_TX_CLAMP,
                            G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPScisTextureRectangleOld(gfx++, (x + 9) << 2, (y + 0x33) << 2, (x + 0x20) << 2,
                                   (y + 0x4A) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    }
    gDPPipeSync(gfx++);
    gDPSetCombine(gfx++, 0x119623, 0xFF2FFFFF);
    gDPLoadTextureBlock(gfx++, D_8039CA94, G_IM_FMT_IA, G_IM_SIZ_16b, 12, 10, 0, G_TX_CLAMP,
                        G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (i = 0; i < 16; i++) {
        u8 draw;
        u16 ox;
        u16 oy;

        draw = 1;
        if (D_80370C30 & (1 << i)) {
            gDPPipeSync(gfx++);
            switch (1 << i) {
                case 0x8000:
                    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0xFF, alpha);
                    ox = 0x39, oy = 0x1E;
                    break;
                case 0x4000:
                    gDPSetPrimColor(gfx++, 0, 0, 0, 0xFF, 0, alpha);
                    ox = 0x34, oy = 0x19;
                    break;
                case 0x1000:
                    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0, 0, alpha);
                    ox = 0x23, oy = 0x1B;
                    break;
                case 0x200:
                    gDPSetPrimColor(gfx++, 0, 0, 0x50, 0x50, 0x50, alpha);
                    ox = 4, oy = 0x16;
                    break;
                case 0x100:
                    gDPSetPrimColor(gfx++, 0, 0, 0x50, 0x50, 0x50, alpha);
                    ox = 0x10, oy = 0x16;
                    break;
                default:
                    draw = 0;
                    break;
            }
            if (draw) {
                gSPScisTextureRectangleOld(gfx++, (x + ox) << 2, (y + oy) << 2, (x + ox + 11) << 2,
                                           (y + oy + 9) << 2, 0, 0, 0, 1 << 10, 1 << 10);
            }
        }
    }
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0xDC, 0xDC, 0xDC, alpha);
    gSPScisTextureRectangleOld(gfx++, (x + D_80370C32 / 18 + 0x23) << 2, (y - D_80370C33 / 18 + 0x28) << 2,
                               (x + D_80370C32 / 18 + 0x2E) << 2, (y - D_80370C33 / 18 + 0x31) << 2, 0, 0,
                               0, 1 << 10, 1 << 10);
    gDPPipeSync(gfx++);
    gDPSetTexturePersp(gfx++, G_TP_PERSP);
    return gfx;
}

/* Clamp a tile's last texel to the 80x80 image. */
#define LIM79(a) ((a) > 79 ? 79 : (a))

/* Draws the 80x80 RGBA16 image in D_8039CA90 at (x, y) as 32x32 tiles. */
Gfx *func_8029700C(Gfx *gfx, s16 x, s16 y) {
    s32 i;
    s32 j;

    for (i = 0; i < 80; i += 31) {
        for (j = 0; j < 80; j += 31) {
            gDPLoadTextureTile(gfx++, D_8039CA90, G_IM_FMT_RGBA, G_IM_SIZ_16b, 80, 0, i, j,
                               LIM79(i + 31), LIM79(j + 31), 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0,
                               G_TX_NOLOD, G_TX_NOLOD);
            gSPScisTextureRectangleOld(gfx++, (x + i) << 2, (y + j) << 2,
                                       (LIM79(i + 31) + x) << 2, (LIM79(j + 31) + y) << 2, 0,
                                       i << 5, j << 5, 1 << 10, 1 << 10);
        }
    }
    return gfx;
}
