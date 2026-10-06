#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80358070
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuEntry *) D_8020C070)
#ifdef NON_MATCHING
#define D_80358070 (*(s32 *) &D_80358070)
#endif
/* end of views */

/*
 * Front-end picture screen: four 160x120 RGBA16 images inflated from ROM,
 * one drawn in 32x8 tiles with a fading frame (cross-fades via prim/env alpha).
 */

/* Pre-2.0I texture rectangle: the RDP-half commands are 0xB3/0xB2 (see hd_code 17E10). */
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

typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char *unkC;  /* title */
    void *unk10; /* glyph list */
    u8 pad14[8];
} MenuEntry;

extern s16 D_8020E3E0[];      /* sound per picture */
extern char D_8020E3E8[][18]; /* picture titles */
extern void *D_8020E430[];
#ifndef NON_MATCHING
extern s32 D_80358070; /* heap pointer */
#endif
extern s16 D_8036BB1C;

extern u8 *D_8021AB90[4]; /* the four inflated pictures */
extern u8 D_8021ABA0;     /* current picture */
extern u8 D_8021ABA1;     /* env alpha */
extern u8 D_8021ABA2;     /* prim alpha */
extern s32 D_8021ABA4;    /* fade state */
extern s32 D_8021ABA8;


/* Inflate the pictures and select picture `arg0`. */
void func_80201240(s32 arg0) {
    s32 size;
    s32 i;

    size = D_006A32B0 - D_0068B550;
    func_8028B4C4(D_0068B550, (void *) D_80358070, &size, 13, 0, 1);
    for (i = 0; i < 4; i++) {
        D_8021AB90[i] = (u8 *) (i * 160 * 120 * 2 + D_80358070);
    }
    D_80358070 += size;
    D_8021ABA0 = arg0;
    D_8021ABA2 = 0;
    D_8021ABA1 = 0;
    D_8020C070[175].unkC = D_8020E3E8[arg0];
    D_8020C070[175].unk10 = D_8020E430[arg0];
}

/* Draw the current picture and its frame, stepping the fade. */
Gfx *func_80201364(s32 arg0, Gfx *arg1) {
    Gfx *gdl = arg1;
    s32 x;
    s32 y;
    s32 xo;
    s32 yo;

    gDPPipeSync(gdl++);
    gSPTexture(gdl++, 0, 0, 0, 0, G_OFF);
    gDPSetTexturePersp(gdl++, G_TP_NONE);
    gDPSetCycleType(gdl++, G_CYC_2CYCLE);
    gDPSetCombine(gdl++, 0x757E4C, 0xFFFFFA3D);
    switch (D_8036BB1C) {
        case 1:
        case 4:
            D_8021ABA4 = 0;
            break;
        case 2:
            switch (D_8021ABA4) {
                case 0:
                    if (D_8021ABA1 + 3 > 0xFF) {
                        D_8021ABA1 = 0xFF;
                    } else {
                        D_8021ABA1 += 3;
                    }
                    if (D_8021ABA1 < 0x80) {
                        D_8021ABA2 = D_8021ABA1;
                    } else {
                        D_8021ABA2 = 0xFF - D_8021ABA1;
                    }
                    if (D_8021ABA1 == 0xFF) {
                        D_8021ABA4 = 1;
                        func_80260650(D_80367738, D_8020E3E0[D_8021ABA0], NULL);
                    }
                    break;
                case 1:
                    D_8021ABA2 = 0;
                    D_8021ABA1 = 0xFF;
                    if (func_8026A828(0, 40) == 0) {
                        D_8021ABA4 = 2;
                    }
                    break;
                case 2:
                    if (func_8026A828(0, 30) == 0 || D_8021ABA2 == 0xFF) {
                        D_8021ABA4 = 3;
                    }
                    if (D_8021ABA2 + 24 > 0xFF) {
                        D_8021ABA2 = 0xFF;
                    } else {
                        D_8021ABA2 += 24;
                    }
                    D_8021ABA1 = 0xFF - D_8021ABA2;
                    break;
                case 3:
                    if (func_8026A828(0, 50) == 0) {
                        D_8021ABA4 = 2;
                    }
                    if (D_8021ABA2 - 32 < 0) {
                        D_8021ABA2 = 0;
                    } else {
                        D_8021ABA2 -= 32;
                    }
                    D_8021ABA1 = 0xFF - D_8021ABA2;
                    if (D_8021ABA2 == 0) {
                        D_8021ABA4 = 1;
                    }
                    break;
            }
            if (func_801E96F8()) {
                func_8026AF6C(0x4000);
            }
            break;
        case 8:
            if (D_8021ABA1 - 6 < 0) {
                D_8021ABA1 = 0;
            } else {
                D_8021ABA1 -= 6;
            }
            if (D_8021ABA1 < 0x80) {
                D_8021ABA2 = D_8021ABA1;
            } else {
                D_8021ABA2 = 0xFF - D_8021ABA1;
            }
            break;
    }
    if (D_8021ABA2 > 60 && D_8021ABA8 == 0) {
        func_80260650(D_80367738, 0x69, &D_8021ABA8);
    }
    gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8021ABA2);
    gDPSetEnvColor(gdl++, 0, 0, 0, D_8021ABA1);
    gDPSetRenderMode(gdl++, FORCE_BL | GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1),
                     GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1));
    xo = 80;
    for (x = 0; x < 160; x += 32) {
        for (yo = 32, y = 0; y < 120; y += 8) {
            gDPLoadTextureTile(gdl++, D_8021AB90[D_8021ABA0], G_IM_FMT_RGBA, G_IM_SIZ_16b, 160, 120, x, y,
                               x + 31, y + 7, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                               G_TX_NOLOD);
            gSPTextureRectangleOld(gdl++, (x + xo) << 2, (y + yo) << 2, (x + xo + 32) << 2, (y + yo + 8) << 2,
                                   G_TX_RENDERTILE, x << 5, y << 5, 1 << 10, 1 << 10);
        }
    }
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetPrimColor(gdl++, 0, 0, 200, 200, 200, 255);
    gDPSetCombine(gdl++, 0x11B223, 0xFF67FFFF);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_802FAD50), G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                        G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR, G_TX_NOMASK, 5, G_TX_NOLOD, G_TX_NOLOD);
    /* 200x168 frame from a 32x32 texture: dsdx = 0xA3, dtdy = 0xC3 */
    gSPTextureRectangleOld(gdl++, (xo - 16) << 2, (yo - 24) << 2, (xo + 184) << 2, (yo + 144) << 2,
                           G_TX_RENDERTILE, 0, 32 << 5, 0xA3, 0xC3);
    gDPPipeSync(gdl++);
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    return gdl;
}
