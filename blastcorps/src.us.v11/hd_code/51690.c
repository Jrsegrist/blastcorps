#include "common.h"
#include <ultra64.h>

extern void *D_80358070;
extern u8 D_006A8DA0[];
extern u8 D_006A9F10[];
extern u8 *D_8039CA90;
extern u8 *D_8039CA94;
extern u8 *D_8039CA98;
extern u8 *D_8039CA9C;
extern s16 D_8039CAA0;
extern u8 D_8039CAA2;
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/51690/func_80295EFC.s")

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
