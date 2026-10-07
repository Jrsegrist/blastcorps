#include "common.h"
#include <ultra64.h>
#include "game/game.h"

#define OLD_RDPHALF_2 0xB3
#define OLD_RDPHALF_CONT 0xB2

#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                \
    {                                                                                        \
        Gfx *_g = (Gfx *) (pkt);                                                             \
                                                                                             \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));    \
        gImmp1(pkt, OLD_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                \
        gImmp1(pkt, OLD_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));       \
    }

extern u8 D_0048FA70[];
extern u8 D_0048FE90[];
extern void * N64P D_803A6B10;
extern s16 D_803A6B14;

/* Load the 256x16 IA8 overlay texture from ROM into the heap */
void func_8029A130(void) {
    s32 size = D_0048FE90 - D_0048FA70;

    func_8028B4C4((u32) D_0048FA70, (u32) D_80358070, (u32 *) &size, 0xC, 0, 1);
    D_803A6B10 = D_80358070;
    D_80358070 = (u8 *) D_80358070 + size;
    D_803A6B14 = 0;
}

/* Draw it as a textured rectangle, fading in by 0x40 per frame */
Gfx *func_8029A1A8(s32 arg0, Gfx *arg1) {
    Gfx *gfx = arg1;

    gDPPipeSync(gfx++);
    gSPTexture(gfx++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetTexturePersp(gfx++, G_TP_NONE);
    gDPSetRenderMode(gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gfx++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    if (D_803A6B14 + 0x40 >= 0x100) {
        D_803A6B14 = 0xFF;
    } else {
        D_803A6B14 += 0x40;
    }
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, D_803A6B14);
    gDPLoadTextureBlock(gfx++, D_803A6B10, G_IM_FMT_IA, G_IM_SIZ_8b, 256, 16, 0, G_TX_CLAMP, G_TX_CLAMP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPTextureRectangleOld(gfx++, 32 << 2, 208 << 2, 288 << 2, 224 << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    gDPPipeSync(gfx++);
    gDPSetTexturePersp(gfx++, G_TP_PERSP);
    return gfx;
}
