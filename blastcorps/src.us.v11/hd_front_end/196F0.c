#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* front-end background pictures: inflate a 320x240 RGBA16 image and draw it */

#define OLD_RDPHALF_2 0xB3
#define OLD_RDPHALF_CONT 0xB2

/* pre-2.0I gSPTextureRectangle (B3/B2 halves) */
#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                \
    {                                                                                        \
        Gfx *_g = (Gfx *) (pkt);                                                             \
                                                                                             \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));    \
        gImmp1(pkt, OLD_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                \
        gImmp1(pkt, OLD_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));       \
    }

/* raw combiner words (no 2.0I G_CC_ name for this mode) */
#define gDPSetCombineRaw(pkt, m0, m1)                                     \
    {                                                                     \
        Gfx *_g = (Gfx *) (pkt);                                          \
                                                                          \
        _g->words.w0 = _SHIFTL(G_SETCOMBINE, 24, 8) | _SHIFTL(m0, 0, 24); \
        _g->words.w1 = (unsigned int) (m1);                               \
    }

extern u8 *D_8021AB80;        /* the inflated picture */
extern u8 D_8021AB84;         /* its number */
extern u8 *D_80358070;        /* heap pointer */
extern u8 D_006BF2F0[];
extern u8 D_006D3D30[];


void func_802006F0(void) {
    func_80200714(D_8021AB84);
}

/* inflate background picture `mode` onto the heap and tint it per mode */
void func_80200714(u8 mode) {
    u8 *start;
    u8 *end;
    u32 size;
    u16 *pix;
    u32 i;
    u8 r;
    u8 g;
    u8 b;
    u8 tmp;

    D_8021AB80 = D_80358070;
    D_8021AB84 = mode;
    switch (mode) {
        case 1:
        case 2:
        case 3:
            start = D_006AD3F0;
            end = D_006BF2F0;
            break;
        case 4:
        case 7:
        case 8:
            start = D_006BF2F0;
            end = D_006D3D30;
            break;
        case 5:
        case 6:
        case 9:
            start = D_006D3D30;
            end = D_006E8980;
            break;
        default:
            return;
    }
    size = end - start;
    func_8028B4C4(start, D_80358070, &size, 0xD, 0, 2);
    pix = (u16 *) D_80358070;
    for (i = 0; i < size / 2; i++) {
        r = pix[i] >> 11;
        g = (pix[i] >> 6) & 0x1F;
        b = (pix[i] >> 1) & 0x1F;
        switch (mode) {
            case 3:
            case 7:
            case 9:
                r = (31.0 < (f32) r * 1.25) ? 31.0 : (f32) r * 1.25;
                g = (b < 3) ? b : 3;
                b = b / 4;
                break;
            case 2:
            case 6:
            case 8:
                g = (b < 2) ? b : 2;
                tmp = r;
                r = b / 3;
                b = (f32) tmp * 0.8125;
                break;
        }
        pix[i] = (r << 11) | (g << 6) | (b << 1) | 1;
    }
    D_80358070 += size;
}

void func_80200BD4(s32 arg0) {
    D_8021AB80 = (u8 *) arg0;
}

/* draw the picture as 64x16 texture rectangles (translucent in some modes) */
Gfx *func_80200BE0(Gfx *gfx, s32 arg1, s32 *count) {
    Gfx *gdl = gfx;
    s32 x;
    s32 y;
    u8 alpha;

    gDPPipeSync(gdl++);
    gSPTexture(gdl++, 0, 0, 0, 0, G_OFF);
    gDPSetTexturePersp(gdl++, G_TP_NONE);
    if (D_80364A90 & 0xC000000000000) {
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, FORCE_BL | GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1),
                         GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1));
        gDPSetCombineRaw(gdl++, 0x157E2A, 0xFFFFFDFE);
        switch (D_80364A90) {
            case 0x4000000000000:
            case 0x8000000000000:
                alpha = 0x60;
                break;
            default:
                alpha = 0xFF;
                break;
        }
        gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, alpha);
    } else {
        gDPSetCycleType(gdl++, G_CYC_COPY);
        gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
    }
    for (x = 0; x < 320; x += 64) {
        for (y = 0; y < 240; y += 16) {
            gDPLoadTextureTile(gdl++, D_8021AB80, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, 240, x, y, x + 63, y + 15, 0,
                               G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            if (D_80364A90 & 0xC000000000000) {
                gSPTextureRectangleOld(gdl++, x << 2, y << 2, (x + 64) << 2, (y + 16) << 2, G_TX_RENDERTILE, x << 5,
                                       y << 5, 1 << 10, 1 << 10);
            } else {
                gSPTextureRectangleOld(gdl++, x << 2, y << 2, (x + 63) << 2, (y + 15) << 2, G_TX_RENDERTILE, x << 5,
                                       y << 5, 4 << 10, 1 << 10);
            }
        }
    }
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    *count += gdl - gfx;
    return gdl;
}
