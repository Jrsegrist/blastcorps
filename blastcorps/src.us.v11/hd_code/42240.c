#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gHeapPtr
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#ifdef NON_MATCHING
#define gHeapPtr (*(u32 *) &gHeapPtr)
#endif
/* end of views */

/* HUD panels drawn over the game: a 40x40 textured dial with a needle
 * (rotated by gSkyfallBoostMeter, 0..100 -> 270..450 degrees), and two 32x32 icons
 * with a number printed beside them. Each panel fades in (+10 per frame up
 * to 0xFF) while its mode argument matches and fades out otherwise; the
 * alpha is also capped by gHudAlpha. */

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifdef PORT_HOST
/* The quads below are built in fresh heap memory, and their flag and colour
 * bytes (and the needle's texture coordinates) are never written: the RSP
 * shades the dial and the icons with whatever bytes the heap held there (an
 * original slip).  port_vtx_stale (port/src/load/swap.c) first gives the
 * native records the N64's stale bytes, so they draw (and compare) the same. */
void port_vtx_stale(void *v, u32 n);
#endif

#ifndef NON_MATCHING
extern u32 gHeapPtr; /* heap pointer */
#endif

/* dial */
extern u32 D_8036EC00; /* texture */
extern Vtx * N64P D_8036EC04; /* 4 dial + 4 needle vertices */
extern Mtx * N64P D_8036EC08; /* needle rotation, one per frame */
extern Mtx * N64P D_8036EC0C; /* needle translation, one per frame */
extern s16 D_8036EC10;  /* alpha */
/* icon 1 */
extern u32 D_8036EC14;
extern Vtx * N64P D_8036EC18;
extern s16 D_8036EC1C;
/* icon 2 */
extern u32 D_8036EC20;
extern Vtx * N64P D_8036EC24;
extern s16 D_8036EC28;


/* Skyfall's boost dial (0x80286A00).
 * Load the dial texture (0x760) and build its vertices */
void hud_init_boost_dial(void) {
    gHudDialLoaded = 1;
    D_8036EC00 = gHeapPtr;
    tex_get(0x760, 0);
    D_8036EC08 = (Mtx *) gHeapPtr;
    D_8036EC0C = (Mtx *) (gHeapPtr += 0x80);
    D_8036EC04 = (Vtx *) (gHeapPtr += 0x80);
    gHeapPtr += 0x80;
#ifdef PORT_HOST
    port_vtx_stale(D_8036EC04, 8);
#endif
    D_8036EC04[0].v.ob[0] = 38;
    D_8036EC04[0].v.ob[1] = 123;
    D_8036EC04[0].v.ob[2] = -5;
    D_8036EC04[0].v.tc[0] = 0;
    D_8036EC04[0].v.tc[1] = 1248;
    D_8036EC04[1].v.ob[0] = 77;
    D_8036EC04[1].v.ob[1] = 123;
    D_8036EC04[1].v.ob[2] = -5;
    D_8036EC04[1].v.tc[0] = 1248;
    D_8036EC04[1].v.tc[1] = 1248;
    D_8036EC04[2].v.ob[0] = 77;
    D_8036EC04[2].v.ob[1] = 173;
    D_8036EC04[2].v.ob[2] = -5;
    D_8036EC04[2].v.tc[0] = 1248;
    D_8036EC04[2].v.tc[1] = 0;
    D_8036EC04[3].v.ob[0] = 38;
    D_8036EC04[3].v.ob[1] = 173;
    D_8036EC04[3].v.ob[2] = -5;
    D_8036EC04[3].v.tc[0] = 0;
    D_8036EC04[3].v.tc[1] = 0;
    D_8036EC04[4].v.ob[0] = -1;
    D_8036EC04[4].v.ob[1] = 0;
    D_8036EC04[4].v.ob[2] = -5;
    D_8036EC04[5].v.ob[0] = 1;
    D_8036EC04[5].v.ob[1] = 0;
    D_8036EC04[5].v.ob[2] = -5;
    D_8036EC04[6].v.ob[0] = -1;
    D_8036EC04[6].v.ob[1] = -17;
    D_8036EC04[6].v.ob[2] = -5;
    D_8036EC04[7].v.ob[0] = 1;
    D_8036EC04[7].v.ob[1] = -17;
    D_8036EC04[7].v.ob[2] = -5;
    D_8036EC10 = 0;
}

/* The dial's needle from gSkyfallBoostMeter (0x80286C60).
 * Draw the dial while arg3 == 3 or it is still fading out */
void hud_draw_boost_dial(Gfx **gfxp, s32 arg1, u8 frame, u8 arg3) {
    Gfx *gdl = *gfxp;

    if (arg3 != 3 && D_8036EC10 == 0) {
        return;
    }
    if (arg3 == 3 && D_8036EC10 < 0xFF) {
        D_8036EC10 += 10;
        if (D_8036EC10 > 0xFF) {
            D_8036EC10 = 0xFF;
        }
    }
    if (arg3 != 3 && D_8036EC10 != 0) {
        D_8036EC10 -= 10;
        if (D_8036EC10 < 0) {
            D_8036EC10 = 0;
        }
    }
    gSPMatrix(gdl++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC10, gHudAlpha));
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036EC00), G_IM_FMT_RGBA, G_IM_SIZ_16b, 40, 40, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036EC04), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPPipeSync(gdl++);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    guTranslate(&D_8036EC0C[frame], 56.0f, 151.0f, 0.0f);
    gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(&D_8036EC0C[frame]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    guRotate(&D_8036EC08[frame], (u32) gSkyfallBoostMeter / 100.0f * 180.0 + 270.0, 0.0f, 0.0f, 1.0f);
    gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(&D_8036EC08[frame]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(&D_8036EC04[4]), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 1, 2, 3, 0);
    gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}

/* Ballista's missiles icon (0x802873AC).
 * Load icon 1 (0x996) and build its quad */
void hud_init_missile_icon(void) {
    gHudMissileIconLoaded = 1;
    D_8036EC14 = gHeapPtr;
    tex_get(0x996, 0);
    D_8036EC18 = (Vtx *) gHeapPtr;
    gHeapPtr += 0x40;
#ifdef PORT_HOST
    port_vtx_stale(D_8036EC18, 4);
#endif
    D_8036EC18[0].v.ob[0] = 34;
    D_8036EC18[0].v.ob[1] = 135;
    D_8036EC18[0].v.ob[2] = -5;
    D_8036EC18[0].v.tc[0] = 0;
    D_8036EC18[0].v.tc[1] = 992;
    D_8036EC18[1].v.ob[0] = 64;
    D_8036EC18[1].v.ob[1] = 135;
    D_8036EC18[1].v.ob[2] = -5;
    D_8036EC18[1].v.tc[0] = 992;
    D_8036EC18[1].v.tc[1] = 992;
    D_8036EC18[2].v.ob[0] = 64;
    D_8036EC18[2].v.ob[1] = 165;
    D_8036EC18[2].v.ob[2] = -5;
    D_8036EC18[2].v.tc[0] = 992;
    D_8036EC18[2].v.tc[1] = 0;
    D_8036EC18[3].v.ob[0] = 34;
    D_8036EC18[3].v.ob[1] = 165;
    D_8036EC18[3].v.ob[2] = -5;
    D_8036EC18[3].v.tc[0] = 0;
    D_8036EC18[3].v.tc[1] = 0;
    D_8036EC1C = 0;
}

/* The icon and gBallistaMissiles (0x80287530).
 * Draw icon 1 and its number (gBallistaMissiles) while arg3 == 10 or still fading out */
void hud_draw_missile_icon(Gfx **gfxp, Gfx * N64P *gfxp2, u8 frame, u8 arg3) {
    Gfx *gdl = *gfxp;
    char buf[20];

    if (arg3 != 10 && D_8036EC1C == 0) {
        return;
    }
    if (arg3 == 10 && D_8036EC1C < 0xFF) {
        D_8036EC1C += 10;
        if (D_8036EC1C > 0xFF) {
            D_8036EC1C = 0xFF;
        }
    }
    if (arg3 != 10 && D_8036EC1C != 0) {
        D_8036EC1C -= 10;
        if (D_8036EC1C < 0) {
            D_8036EC1C = 0;
        }
    }
    gSPMatrix(gdl++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC1C, gHudAlpha));
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036EC14), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036EC18), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPPipeSync(gdl++);
    util_itoa(gBallistaMissiles, (u8 *) buf);
    text_print_gradient(gfxp2, (u8 *) buf, 0, 1, 0, 0x40, 0x91, 15, 15, 1, 0xFF, 0xFF, 0, MIN(D_8036EC1C, gHudAlpha), 0xFF, 0, 0,
                  MIN(D_8036EC1C, gHudAlpha));
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}

/* Sideswipe's side-ram icon (0x80287AE4).
 * Load icon 2 (0x998) and build its quad */
void hud_init_ram_icon(void) {
    gHudRamIconLoaded = 1;
    D_8036EC20 = gHeapPtr;
    tex_get(0x998, 0);
    D_8036EC24 = (Vtx *) gHeapPtr;
    gHeapPtr += 0x40;
#ifdef PORT_HOST
    port_vtx_stale(D_8036EC24, 4);
#endif
    D_8036EC24[0].v.ob[0] = 34;
    D_8036EC24[0].v.ob[1] = 145;
    D_8036EC24[0].v.ob[2] = -5;
    D_8036EC24[0].v.tc[0] = 0;
    D_8036EC24[0].v.tc[1] = 992;
    D_8036EC24[1].v.ob[0] = 64;
    D_8036EC24[1].v.ob[1] = 145;
    D_8036EC24[1].v.ob[2] = -5;
    D_8036EC24[1].v.tc[0] = 992;
    D_8036EC24[1].v.tc[1] = 992;
    D_8036EC24[2].v.ob[0] = 64;
    D_8036EC24[2].v.ob[1] = 175;
    D_8036EC24[2].v.ob[2] = -5;
    D_8036EC24[2].v.tc[0] = 992;
    D_8036EC24[2].v.tc[1] = 0;
    D_8036EC24[3].v.ob[0] = 34;
    D_8036EC24[3].v.ob[1] = 175;
    D_8036EC24[3].v.ob[2] = -5;
    D_8036EC24[3].v.tc[0] = 0;
    D_8036EC24[3].v.tc[1] = 0;
    D_8036EC28 = 0;
}

/* The icon and gSideswipeRamCharges (0x80287C68).
 * Draw icon 2 and its number (gSideswipeRamCharges) while arg3 == 1 or still fading out */
void hud_draw_ram_icon(Gfx **gfxp, Gfx * N64P *gfxp2, u8 frame, u8 arg3) {
    Gfx *gdl = *gfxp;
    char buf[20];

    if (arg3 != 1 && D_8036EC28 == 0) {
        return;
    }
    if (arg3 == 1 && D_8036EC28 < 0xFF) {
        D_8036EC28 += 10;
        if (D_8036EC28 > 0xFF) {
            D_8036EC28 = 0xFF;
        }
    }
    if (arg3 != 1 && D_8036EC28 != 0) {
        D_8036EC28 -= 10;
        if (D_8036EC28 < 0) {
            D_8036EC28 = 0;
        }
    }
    gSPMatrix(gdl++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC28, gHudAlpha));
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036EC20), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036EC24), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPPipeSync(gdl++);
    util_itoa(gSideswipeRamCharges, (u8 *) buf);
    text_print_gradient(gfxp2, (u8 *) buf, 0, 1, 0, 0x44, 0x97, 15, 15, 1, 0xFF, 0xFF, 0, MIN(D_8036EC28, gHudAlpha), 0xFF, 0, 0,
                  MIN(D_8036EC28, gHudAlpha));
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}
