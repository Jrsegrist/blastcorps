#include "common.h"
#include <ultra64.h>
#include "game/game.h"

extern f32 sqrtf(f32);
extern s16 D_80364452;
extern u32 D_803156C4;
extern u16 D_802FCEB0[]; /* 32x32 RGBA16 arrow texture */
extern Vtx D_802FD9B8[];
extern Mtx D_02000000[];

/*
 * Draws the on-screen arrow pointing from the player towards (x, z), tinted
 * green-to-red by the player's distance from (x2, z2) and blinking when within
 * 250 units. Positions are scaled down by 32 first; y and y2 are unused.
 */
void func_80282C80(Gfx **gfxp, Mtx *mtx, s32 x, s32 y, s32 z, s32 x2, s32 y2, s32 z2) {
    Gfx *gfx;
    f32 dist;
    f32 angle;
    u8 r;
    u8 g;
    s16 t;
    f32 mf[4][4];
    f32 mf2[4][4];
    s32 px;
    s32 pz;
    u8 close;

    gfx = *gfxp;
    x >>= 5;
    y >>= 5;
    z >>= 5;
    x2 >>= 5;
    y2 >>= 5;
    z2 >>= 5;
    px = D_803F7670 >> 5;
    pz = D_803F7678 >> 5;
    dist = sqrtf((px - x) * (px - x) + (pz - z) * (pz - z));
    if (dist < 1.0) {
        dist = 1.0f;
    }
    if (px >= x && pz >= z) {
        angle = func_802AD7D4((f32) (px - x) / dist * 65536.0) >> 4;
    }
    if (px >= x && pz < z) {
        angle = (func_802AD7D4((f32) (z - pz) / dist * 65536.0) >> 4) + 0x400;
    }
    if (px < x && pz < z) {
        angle = (func_802AD7D4((f32) (x - px) / dist * 65536.0) >> 4) + 0x800;
    }
    if (px < x && pz >= z) {
        angle = (func_802AD7D4((f32) (pz - z) / dist * 65536.0) >> 4) + 0xC00;
    }
    angle = angle * 0.08791208791208792;
    angle = 360.0 - angle - 45.0;
    angle = angle + (D_80364452 * 360.0 / 4095.0 - 135.0);
    dist = sqrtf((x2 - px) * (x2 - px) + (z2 - pz) * (z2 - pz));
    if (dist > 1500.0f) {
        g = 0xFF;
        r = 0;
    } else if (dist < 500.0f) {
        r = 0xFF;
        g = 0;
    } else {
        t = (dist - 500.0f) / 1000.0f * 511.0f;
        if (t < 0x100) {
            g = t, r = 0xFF;
        } else {
            g = 0xFF, r = 0x1FE - t;
        }
    }
    if (dist < 250.0f) {
        close = 1;
    } else {
        close = 0;
    }
    if ((D_803156C4 % 30 >= 16 || !close) && D_803F7660 != 9999999) {
        guRotateF(mf, 20.0f, 1.0f, 0.0f, 0.0f);
        guRotateF(mf2, -angle, 0.0f, 0.0f, 1.0f);
        guMtxCatF(mf, mf2, mf);
        guTranslateF(mf2, -150.0f, -230.0f, -800.0f);
        guMtxCatF(mf, mf2, mf);
        guMtxF2L(mf, &mtx[0x56]);
        gSPMatrix(gfx++, &D_02000000[2], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gImmp1(gfx++, G_RDPHALF_1, D_8035807C);
        gSPMatrix(gfx++, &D_02000000[0x56], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
        gSPSetGeometryMode(gfx++, 0x61204);
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        if (D_80367BD6 == 0xFF) {
            gDPSetRenderMode(gfx++, 0x00552008, 0);
        } else {
            gDPSetRenderMode(gfx++, 0x005041C8, 0);
        }
        gDPSetCombine(gfx++, 0x119623, 0xFF2FFFFF);
        gDPSetPrimColor(gfx++, 0, 0, r, g, 0, D_80367BD6);
        gSPTexture(gfx++, 0x7C0, 0x7C0, 0, G_TX_RENDERTILE, G_ON);
        gDPLoadTextureBlock(gfx++, OS_K0_TO_PHYSICAL(D_802FCEB0), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                            G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPVertex(gfx++, osVirtualToPhysical(D_802FD9B8), 10, 0);
        gSP1Triangle(gfx++, 0, 1, 2, 0);
        gSP1Triangle(gfx++, 0, 2, 3, 0);
        gSP1Triangle(gfx++, 3, 1, 0, 0);
        gSP1Triangle(gfx++, 1, 4, 2, 0);
        gSP1Triangle(gfx++, 4, 5, 2, 0);
        gSP1Triangle(gfx++, 5, 3, 2, 0);
        gSP1Triangle(gfx++, 6, 7, 8, 0);
        gSP1Triangle(gfx++, 9, 6, 8, 0);
        gSP1Triangle(gfx++, 7, 9, 8, 0);
        gDPPipeSync(gfx++);
    }
    *gfxp = gfx;
}

f32 func_80284ADC(s16 x1, s16 z1, s16 x2, s16 z2);
extern u8 D_802FC5B0[];
extern u8 D_802FC6B0[];
extern u8 D_802FD6B0[];
extern Vtx D_802FD7B0[];
extern Vtx D_802FD7F0[];
extern Vtx D_802FD830[][12]; /* per radar: target blip, player blip, heading arrow quads */
extern f32 D_802FD9B0;
extern Mtx D_8036E5E0[];

/*
 * Draws radar `idx`, centred on (x, z) and rotated with the camera: zoom
 * (scale) and texture shift depend on the distance to (x2, z2); blips for
 * (x2, z2) (blinking when zoomed in) and the player; an arrow pointing toward
 * (D_803F767C, D_803F7680); and a spinning sweep overlay.
 */
void func_8028376C(Gfx **gfxp, Mtx *mtx, u8 idx, s32 x, s32 z, s32 x2, s32 z2) {
    Gfx *gfx;
    s32 dist;
    u8 shift;
    s32 scale;
    s16 sx;
    s16 sy;
    s16 px;
    s16 py;
    Mtx rot;
    f32 mf[4][4];
    f32 mf2[4][4];
    f32 ox;
    f32 oy;
    f32 oz;
    f32 heading;

    gfx = *gfxp;
    dist = func_8026A610(x, z, x2, z2);
    if (dist < 0x55F0) {
        shift = 10;
        scale = 1200;
    }
    if (dist >= 0x55F0 && dist < 0xABE0) {
        shift = 0;
        scale = 2400;
    }
    if (dist >= 0xABE0) {
        shift = 15;
        scale = 4800;
    }
    if (D_803643DB == 0) {
        shift = 10;
        scale = 1200;
    }
    guRotateF(mf, 360.0 - (D_80364414 - 180.0), 0.0f, 1.0f, 0.0f);
    guMtxXFMF(mf, (x - x2) / scale, 0.0f, (z - z2) / scale, &ox, &oy, &oz);
    sx = 58.0f + ox;
    sy = 195.0f + oz;
    D_802FD830[idx][0].v.ob[0] = sx - 2;
    D_802FD830[idx][0].v.ob[1] = sy - 2;
    D_802FD830[idx][1].v.ob[0] = sx - 2;
    D_802FD830[idx][1].v.ob[1] = sy + 1;
    D_802FD830[idx][2].v.ob[0] = sx + 1;
    D_802FD830[idx][2].v.ob[1] = sy + 1;
    D_802FD830[idx][3].v.ob[0] = sx + 1;
    D_802FD830[idx][3].v.ob[1] = sy - 2;
    func_802C1B9C();
    guMtxXFMF(mf, (x - D_803F7670) / scale, 0.0f, (z - D_803F7678) / scale, &ox, &oy, &oz);
    px = 58.0f + ox;
    py = 195.0f + oz;
    D_802FD830[idx][4].v.ob[0] = px - 2;
    D_802FD830[idx][4].v.ob[1] = py - 2;
    D_802FD830[idx][5].v.ob[0] = px - 2;
    D_802FD830[idx][5].v.ob[1] = py + 1;
    D_802FD830[idx][6].v.ob[0] = px + 1;
    D_802FD830[idx][6].v.ob[1] = py + 1;
    D_802FD830[idx][7].v.ob[0] = px + 1;
    D_802FD830[idx][7].v.ob[1] = py - 2;

    gSPMatrix(gfx++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gfx++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
    gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gfx++);
    gDPSetTextureLOD(gfx++, G_TL_TILE);
    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetTextureImage(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(D_802FC5B0));
    gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 0x7F, 0x400);
    gDPSetTextureImage(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(D_802FC6B0));
    gDPTileSync(gfx++);
    gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
    gDPLoadSync(gfx++);
    gDPLoadBlock(gfx++, G_TX_LOADTILE, 0, 0, 0x3FF, 0x200);
    gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_8b, 2, 0, G_TX_RENDERTILE, 0, G_TX_MIRROR, 4, 0, G_TX_MIRROR, 4, 0);
    gDPSetTileSize(gfx++, G_TX_RENDERTILE, 2, 2, 0x3E, 0x3E);
    gDPSetTile(gfx++, G_IM_FMT_IA, G_IM_SIZ_4b, 4, 0x100, 1, 0, G_TX_MIRROR, 6, shift, G_TX_MIRROR, 6, shift);
    gDPSetTileSize(gfx++, 1, 2, 2, 0xFC, 0xFC);
    gDPSetCycleType(gfx++, G_CYC_2CYCLE);
    gDPSetRenderMode(gfx++, 0x0C184340, 0);
    gDPSetCombine(gfx++, 0x3497FF, 0x4FFE7E38);
    gDPSetPrimColor(gfx++, 0, 0, 0xFF, 0xFF, 0xFF, (D_80367BD6 < 0x3F) ? D_80367BD6 : 0x3F);
    gSPVertex(gfx++, osVirtualToPhysical(D_802FD7B0), 4, 0);
    gSP1Triangle(gfx++, 0, 1, 2, 0);
    gSP1Triangle(gfx++, 0, 2, 3, 0);
    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetCombine(gfx++, 0xFFFFFF, 0xFFFE773B);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_80367BD6);
    if (D_803643DB != 0) {
        gDPSetRenderMode(gfx++, 0x005041C8, 0);
        if ((D_803156C4 % 40 > 20 || shift != 10) && sx >= 0x22 && sx < 0x53 && sy >= 0xAB && sy < 0xDC) {
            gSPVertex(gfx++, osVirtualToPhysical(D_802FD830[idx]), 4, 0);
            gSP1Triangle(gfx++, 0, 1, 2, 0);
            gSP1Triangle(gfx++, 0, 2, 3, 0);
        }
        if (px >= 0x22 && px < 0x53 && py >= 0xB0 && py < 0xD7 && D_803F7660 != 9999999) {
            gSPVertex(gfx++, osVirtualToPhysical(&D_802FD830[idx][4]), 4, 0);
            gSP1Triangle(gfx++, 0, 1, 2, 0);
            gSP1Triangle(gfx++, 0, 2, 3, 0);
        }
    }
    heading = func_80284ADC(x >> 5, z >> 5, D_803F767C, D_803F7680);
    guRotateF(mf, -((360.0 - (D_80364414 - 180.0)) + (heading + 180.0)), 0.0f, 0.0f, 1.0f);
    guTranslateF(mf2, 58.0f, 195.0f, 0.0f);
    guMtxCatF(mf, mf2, mf);
    guMtxF2L(mf, &D_8036E5E0[idx]);
    gDPPipeSync(gfx++);
    if (D_80367BD6 == 0xFF) {
        gDPSetRenderMode(gfx++, 0x00552048, 0);
    } else {
        gDPSetRenderMode(gfx++, 0x005041C8, 0);
    }
    gSPMatrix(gfx++, osVirtualToPhysical(&D_8036E5E0[idx]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPVertex(gfx++, osVirtualToPhysical(&D_802FD830[idx][8]), 4, 0);
    gSP1Triangle(gfx++, 0, 1, 2, 0);
    gSP1Triangle(gfx++, 0, 2, 3, 0);
    gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    if (D_803643DB != 0) {
        gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetRenderMode(gfx++, 0x00504340, 0);
        gDPSetCombine(gfx++, 0xFF97FF, 0xFF2E7F3F);
        gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, (D_80367BD6 >= 0x80) ? 0x7F : D_80367BD6);
        gDPLoadTextureBlock(gfx++, OS_K0_TO_PHYSICAL(D_802FD6B0), G_IM_FMT_IA, G_IM_SIZ_8b, 8, 32, 0,
                            G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        guRotate(&mtx[23], D_802FD9B0, 0.0f, 0.0f, 1.0f);
        guTranslate(&rot, 58.0f, 195.0f, 0.0f);
        guMtxCatL(&mtx[23], &rot, &mtx[23]);
        D_802FD9B0 += 4.0;
        gSPMatrix(gfx++, &D_02000000[23], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPVertex(gfx++, osVirtualToPhysical(D_802FD7F0), 4, 0);
        gSP1Triangle(gfx++, 0, 1, 2, 0);
        gSP1Triangle(gfx++, 1, 2, 3, 0);
    }
    gDPPipeSync(gfx++);
    *gfxp = gfx;
}

/*
 * Heading in degrees (0..360) from (x1, z1) to (x2, z2), one quadrant at a time.
 * func_802AD7D4 maps a 0..65535 sine to a 0..0x3FFF binary angle. No return if
 * no quadrant test passes (original falls off the end).
 */
f32 func_80284ADC(s16 x1, s16 z1, s16 x2, s16 z2) {
    f32 dist;

    dist = sqrtf((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1));
    if (dist < 1.0) {
        return 0.0f;
    }
    if (x2 >= x1 && z2 >= z1) {
        return func_802AD7D4((x2 - x1) * 65535.9 / dist) / 65536.0 * 360.0;
    }
    if (x2 >= x1 && z2 < z1) {
        return (func_802AD7D4((z1 - z2) * 65535.9 / dist) + 0x4000) / 65536.0 * 360.0;
    }
    if (x2 < x1 && z2 < z1) {
        return (func_802AD7D4((x1 - x2) * 65535.9 / dist) + 0x8000) / 65536.0 * 360.0;
    }
    if (x2 < x1 && z2 >= z1) {
        return (func_802AD7D4((z2 - z1) * 65535.9 / dist) + 0xC000) / 65536.0 * 360.0;
    }
}
