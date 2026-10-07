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
#ifdef NON_MATCHING
#define D_80358070 (*(u32 *) &D_80358070)
#endif
/* end of views */

/* HUD panels drawn over the game: a 40x40 textured dial with a needle
 * (rotated by D_803EE3B1, 0..100 -> 270..450 degrees), and two 32x32 icons
 * with a number printed beside them. Each panel fades in (+10 per frame up
 * to 0xFF) while its mode argument matches and fades out otherwise; the
 * alpha is also capped by D_80367BD6. */

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
extern u32 D_80358070; /* heap pointer */
#endif

/* dial */
extern u32 D_8036EC00; /* texture */
extern Vtx *D_8036EC04; /* 4 dial + 4 needle vertices */
extern Mtx *D_8036EC08; /* needle rotation, one per frame */
extern Mtx *D_8036EC0C; /* needle translation, one per frame */
extern s16 D_8036EC10;  /* alpha */
/* icon 1 */
extern u32 D_8036EC14;
extern Vtx *D_8036EC18;
extern s16 D_8036EC1C;
/* icon 2 */
extern u32 D_8036EC20;
extern Vtx *D_8036EC24;
extern s16 D_8036EC28;


/* Load the dial texture (0x760) and build its vertices */
void func_80286A00(void) {
    D_80364A68 = 1;
    D_8036EC00 = D_80358070;
    func_802A0CC8(0x760, 0);
    D_8036EC08 = (Mtx *) D_80358070;
    D_8036EC0C = (Mtx *) (D_80358070 += 0x80);
    D_8036EC04 = (Vtx *) (D_80358070 += 0x80);
    D_80358070 += 0x80;
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

/* Draw the dial while arg3 == 3 or it is still fading out */
void func_80286C60(Gfx **gfxp, s32 arg1, u8 frame, u8 arg3) {
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
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC10, D_80367BD6));
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
    guRotate(&D_8036EC08[frame], (u32) D_803EE3B1 / 100.0f * 180.0 + 270.0, 0.0f, 0.0f, 1.0f);
    gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(&D_8036EC08[frame]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(&D_8036EC04[4]), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 1, 2, 3, 0);
    gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}

/* Load icon 1 (0x996) and build its quad */
void func_802873AC(void) {
    D_80364A6A = 1;
    D_8036EC14 = D_80358070;
    func_802A0CC8(0x996, 0);
    D_8036EC18 = (Vtx *) D_80358070;
    D_80358070 += 0x40;
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

/* Draw icon 1 and its number (D_803F8B72) while arg3 == 10 or still fading out */
void func_80287530(Gfx **gfxp, Gfx **gfxp2, u8 frame, u8 arg3) {
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
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC1C, D_80367BD6));
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036EC14), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036EC18), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPPipeSync(gdl++);
    func_8026A378(D_803F8B72, (u8 *) buf);
    func_80259DC8(gfxp2, (u8 *) buf, 0, 1, 0, 0x40, 0x91, 15, 15, 1, 0xFF, 0xFF, 0, MIN(D_8036EC1C, D_80367BD6), 0xFF, 0, 0,
                  MIN(D_8036EC1C, D_80367BD6));
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}

/* Load icon 2 (0x998) and build its quad */
void func_80287AE4(void) {
    D_80364A6C = 1;
    D_8036EC20 = D_80358070;
    func_802A0CC8(0x998, 0);
    D_8036EC24 = (Vtx *) D_80358070;
    D_80358070 += 0x40;
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

/* Draw icon 2 and its number (D_803EDC00) while arg3 == 1 or still fading out */
void func_80287C68(Gfx **gfxp, Gfx **gfxp2, u8 frame, u8 arg3) {
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
    gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, MIN(D_8036EC28, D_80367BD6));
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
    gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036EC20), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036EC24), 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPPipeSync(gdl++);
    func_8026A378(D_803EDC00, (u8 *) buf);
    func_80259DC8(gfxp2, (u8 *) buf, 0, 1, 0, 0x44, 0x97, 15, 15, 1, 0xFF, 0xFF, 0, MIN(D_8036EC28, D_80367BD6), 0xFF, 0, 0,
                  MIN(D_8036EC28, D_80367BD6));
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}
