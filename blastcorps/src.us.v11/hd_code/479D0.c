#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* Collectible boxes: each is an axis-aligned textured cube (8 vertices) built
 * from a per-type table, collected when the player comes within range, then
 * faded out. */

/* Per-type box description, at 0x802FDB40. */
typedef struct {
    s16 x1;
    s16 x0;
    s16 y0;
    s16 y1;
    s16 z0;
    s16 z1;
    s16 tex0Id;
    s16 tex1Id;
    s16 radarSize; /* passed to func_802CE880 */
    s16 radius;    /* collect distance */
    u8 trigger;    /* compared with func_8028C874's argument */
    u8 pad15;
} BoxType; /* size 0x16 */

/* Input placement record. */
typedef struct BoxSpawn {
    s16 x;
    s16 y;
    s16 z;
    s16 type;
} BoxSpawn; /* size 0x8 */

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 type;
    u8 collected;
    s16 alpha;
    s16 padA;
    s32 tex0;
    s32 tex1;
    Vtx *vtx;
} Box; /* size 0x18 */

/* Vtx texture coordinates; a macro in the original (the two stores come out
 * in comma-expression order). */
#define SET_TC(vtx, s, t) ((vtx).v.tc[0] = (s), (vtx).v.tc[1] = (t))

extern BoxType D_802FDB40[];
extern Vtx *D_80358070;
extern s32 D_80367738;
extern Box D_8039AF00[];
extern s32 D_8039B068;
extern s16 D_803F8B72;


void func_8028C41C(Vtx *v, u8 type, s16 x, s16 y, s16 z);

/* Spawn boxes from the records [arg0, arg1). */
void func_8028C190(BoxSpawn *arg0, BoxSpawn *arg1) {
    D_8039B068 = 0;
    while (arg0 != arg1) {
        D_8039AF00[D_8039B068].x = arg0->x;
        D_8039AF00[D_8039B068].y = arg0->y;
        D_8039AF00[D_8039B068].z = arg0->z;
        D_8039AF00[D_8039B068].type = arg0->type;
        D_8039AF00[D_8039B068].collected = 0;
        D_8039AF00[D_8039B068].alpha = 0xFF;
        D_8039AF00[D_8039B068].tex0 = func_802A0CC8(D_802FDB40[D_8039AF00[D_8039B068].type].tex0Id, 0);
        D_8039AF00[D_8039B068].tex1 = func_802A0CC8(D_802FDB40[D_8039AF00[D_8039B068].type].tex1Id, 0);
        D_8039AF00[D_8039B068].vtx = D_80358070;
        D_80358070 += 8;
        func_8028C41C(D_8039AF00[D_8039B068].vtx, D_8039AF00[D_8039B068].type, D_8039AF00[D_8039B068].x,
                      D_8039AF00[D_8039B068].y, D_8039AF00[D_8039B068].z);
        D_8039B068++;
        arg0++;
    }
}

/* Fill the 8 cube vertices of a box of the given type at (x, y, z). */
void func_8028C41C(Vtx *v, u8 type, s16 x, s16 y, s16 z) {
    v[0].v.ob[0] = D_802FDB40[type].x0 + x;
    v[0].v.ob[1] = D_802FDB40[type].y0 + y;
    v[0].v.ob[2] = D_802FDB40[type].z0 + z;
    SET_TC(v[0], 0, 0);
    v[1].v.ob[0] = D_802FDB40[type].x0 + x;
    v[1].v.ob[1] = D_802FDB40[type].y1 + y;
    v[1].v.ob[2] = D_802FDB40[type].z0 + z;
    SET_TC(v[1], 0, 0x1E0);
    v[2].v.ob[0] = D_802FDB40[type].x0 + x;
    v[2].v.ob[1] = D_802FDB40[type].y1 + y;
    v[2].v.ob[2] = D_802FDB40[type].z1 + z;
    SET_TC(v[2], 0x1E0, 0x1E0);
    v[3].v.ob[0] = D_802FDB40[type].x0 + x;
    v[3].v.ob[1] = D_802FDB40[type].y0 + y;
    v[3].v.ob[2] = D_802FDB40[type].z1 + z;
    SET_TC(v[3], 0x1E0, 0);
    v[4].v.ob[0] = D_802FDB40[type].x1 + x;
    v[4].v.ob[1] = D_802FDB40[type].y0 + y;
    v[4].v.ob[2] = D_802FDB40[type].z0 + z;
    SET_TC(v[4], 0x1E0, 0);
    v[5].v.ob[0] = D_802FDB40[type].x1 + x;
    v[5].v.ob[1] = D_802FDB40[type].y1 + y;
    v[5].v.ob[2] = D_802FDB40[type].z0 + z;
    SET_TC(v[5], 0x1E0, 0x1E0);
    v[6].v.ob[0] = D_802FDB40[type].x1 + x;
    v[6].v.ob[1] = D_802FDB40[type].y1 + y;
    v[6].v.ob[2] = D_802FDB40[type].z1 + z;
    SET_TC(v[6], 0, 0x1E0);
    v[7].v.ob[0] = D_802FDB40[type].x1 + x;
    v[7].v.ob[1] = D_802FDB40[type].y0 + y;
    v[7].v.ob[2] = D_802FDB40[type].z1 + z;
    SET_TC(v[7], 0, 0);
}

/* Per-frame update: collect boxes of the given trigger class that are within
 * range, mark the rest on the radar, and fade out collected ones. */
void func_8028C874(u8 arg0) {
    s32 sp34;
    s32 sp30;

    for (sp34 = 0; sp34 < D_8039B068; sp34++) {
        if (D_8039AF00[sp34].collected == 0) {
            if (D_802FDB40[D_8039AF00[sp34].type].trigger == arg0) {
                func_802CE90C(sp34 + 0x10000);
                sp30 = func_8026A6F0(D_803643E0 >> 5, D_803643E4 >> 5, D_803643E8 >> 5, D_8039AF00[sp34].x,
                                     D_8039AF00[sp34].y, D_8039AF00[sp34].z);
                if (sp30 <= D_802FDB40[D_8039AF00[sp34].type].radius) {
                    D_8039AF00[sp34].collected = 1;
                    func_80260650(D_80367738, 0x70, 0);
                    switch (D_8039AF00[sp34].type) {
                        case 0:
                            D_803F8B72 += 10;
                            break;
                        case 1:
                            D_803EDC00 += 10;
                            break;
                    }
                }
            } else {
                func_802CE880(sp34 + 0x10000, D_8039AF00[sp34].x << 5, D_8039AF00[sp34].y << 5,
                              D_8039AF00[sp34].z << 5, D_802FDB40[D_8039AF00[sp34].type].radarSize);
            }
        } else {
            D_8039AF00[sp34].alpha -= 20;
            if (D_8039AF00[sp34].alpha < 0) {
                D_8039AF00[sp34].alpha = 0;
            }
        }
    }
}

/* Draw every box that is uncollected or still fading. */
void func_8028CB30(Gfx **arg0, s32 arg1) {
    Gfx *gfx;
    s32 i;
    u8 renderMode;

    gfx = *arg0;
    renderMode = 0;
    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetCombine(gfx++, 0xFFFFFF, 0xFFFCF67B);
    gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
    gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    for (i = 0; i < D_8039B068; i++) {
        if (D_8039AF00[i].collected == 0 || D_8039AF00[i].alpha != 0) {
            gDPPipeSync(gfx++);
            if (D_8039AF00[i].collected == 0 && (renderMode == 0 || renderMode == 2)) {
                gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
                renderMode = 1;
            }
            if (D_8039AF00[i].collected != 0 && (renderMode == 0 || renderMode == 1)) {
                gDPSetRenderMode(gfx++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
                renderMode = 2;
            }
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_8039AF00[i].alpha);
            gSPVertex(gfx++, osVirtualToPhysical(D_8039AF00[i].vtx), 8, 0);
            gDPLoadTextureBlock(gfx++, (u32) D_8039AF00[i].tex0 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
                                G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSP1Triangle(gfx++, 6, 3, 2, 0);
            gSP1Triangle(gfx++, 6, 7, 3, 0);
            gSP1Triangle(gfx++, 4, 1, 0, 0);
            gSP1Triangle(gfx++, 1, 4, 5, 0);
            gSPModifyVertex(gfx++, 0, G_MWO_POINT_ST, 0x01E00000);
            gSPModifyVertex(gfx++, 1, G_MWO_POINT_ST, 0x01E001E0);
            gSPModifyVertex(gfx++, 2, G_MWO_POINT_ST, 0x000001E0);
            gSPModifyVertex(gfx++, 3, G_MWO_POINT_ST, 0x00000000);
            gSP1Triangle(gfx++, 0, 1, 2, 0);
            gSP1Triangle(gfx++, 0, 2, 3, 0);
            gSPModifyVertex(gfx++, 7, G_MWO_POINT_ST, 0x01E00000);
            gSPModifyVertex(gfx++, 6, G_MWO_POINT_ST, 0x01E001E0);
            gSPModifyVertex(gfx++, 5, G_MWO_POINT_ST, 0x000001E0);
            gSPModifyVertex(gfx++, 4, G_MWO_POINT_ST, 0x00000000);
            gSP1Triangle(gfx++, 7, 5, 4, 0);
            gSP1Triangle(gfx++, 7, 6, 5, 0);
            gDPLoadTextureBlock(gfx++, (u32) D_8039AF00[i].tex1 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
                                G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPModifyVertex(gfx++, 2, G_MWO_POINT_ST, 0x00000000);
            gSPModifyVertex(gfx++, 1, G_MWO_POINT_ST, 0x01E00000);
            gSPModifyVertex(gfx++, 5, G_MWO_POINT_ST, 0x01E001E0);
            gSPModifyVertex(gfx++, 6, G_MWO_POINT_ST, 0x000001E0);
            gSP1Triangle(gfx++, 5, 2, 1, 0);
            gSP1Triangle(gfx++, 2, 5, 6, 0);
        }
    }
    gDPPipeSync(gfx++);
    *arg0 = gfx;
}
