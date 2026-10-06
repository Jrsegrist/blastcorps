#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* front-end 3D scene: a small hierarchy of nodes (D_8020BD30[7]) drawn with a
 * float matrix stack, a look-at camera and a depth-sorted display list table */

typedef f32 MtxF[4][4];

typedef struct Node {
    f32 unk0; /* scale */
    f32 unk4;
    f32 unk8; /* spin period (0 = no spin) */
    f32 unkC;
    struct Node *unk10; /* next sibling */
    struct Node *unk14; /* first child */
    u8 unk18[4]; /* rgb */
    f32 unk1C; /* spin angle */
    f32 unk20; /* world position from the matrix stack */
    f32 unk24;
    f32 unk28;
    f32 unk2C;
    f32 unk30;
    f32 unk34;
    f32 unk38; /* depth (draw sort key) */
} Node; /* 0x3C */

typedef struct {
    s32 unk0; /* node index */
    Gfx *unk4;
} DrawEntry;

extern Node D_8020BD30[];
extern f32 D_8020BDE4; /* D_8020BD30[3].unk0 */
extern f32 D_8020BDEC; /* D_8020BD30[3].unk8 */
extern Node D_8020BE98; /* D_8020BD30[6] */
extern Vtx D_80217690[][2][4];
extern MtxF D_80217A10[];
extern s32 D_80217B50; /* matrix stack index into D_80217A10 */
extern f32 D_80217B54; /* camera eye */
extern f32 D_80217B58;
extern f32 D_80217B5C;
extern f32 D_80217B60; /* camera target */
extern f32 D_80217B64;
extern f32 D_80217B68;
extern Mtx D_80217B70[][4];
extern DrawEntry D_80218270[];
extern s16 D_802182A8;

f32 sqrtf(f32);
void func_801F374C(Node *);
Gfx *func_801F3964(Gfx *, u8 *, Node *, f32);
Gfx *func_801F4110(Gfx *, u8 *, Node *, f32);
void func_801F4878(Gfx *, u8 *);
void func_801F4C3C(Node *, f32);
s32 func_801F36B0(DrawEntry *, DrawEntry *);

/* draw the node scene: build the matrices, set the camera, sort and emit the display lists */
Gfx *func_801F3450(Gfx *gdl, u8 *dyn) {
    Gfx *gfx;
    Node *nodes;
    Node *cam;
    u8 found;
    s32 i;

    gfx = gdl;
    nodes = D_8020BD30;
    cam = &nodes[&nodes[D_80217B6C] - nodes];
    gImmp1(gfx++, G_RDPHALF_1, D_8035807C);
    D_80217B50 = 4;
    guMtxIdentF(D_80217A10[4]);
    func_801F374C(&D_8020BE98);
    if (D_802182A8 != 1) {
        D_80217B54 = cam->unk2C;
        D_80217B58 = cam->unk30;
        D_80217B5C = cam->unk34;
        D_80217B60 = cam->unk20;
        D_80217B64 = cam->unk24;
        D_80217B68 = cam->unk28;
    }
    guLookAt((Mtx *) (dyn + 0x140), D_80217B54 + 1.0f, D_80217B58, D_80217B5C, D_80217B60, D_80217B64, D_80217B68,
             0.0f, 1.0f, 0.0f);
    func_801F4878((Gfx *) (dyn + 0xACB0), dyn);
    func_802595E0((u8 *) D_80218270, 7, sizeof(DrawEntry), (s32 (*)(void *, void *)) func_801F36B0);
    for (i = 0, found = 0; i < 7; i++) {
        if (!found || D_80217B6C != 3) {
            gSPDisplayList(gfx++, D_80218270[i].unk4);
        }
        if (D_80218270[i].unk0 == 3) {
            found = 1;
        }
    }
    return gfx;
}

/* sort comparator for D_80218270: by node depth */
s32 func_801F36B0(DrawEntry *a, DrawEntry *b) {
    Node *nodes;
    Node *na;
    Node *nb;

    nodes = D_8020BD30;
    na = &nodes[&nodes[a->unk0] - nodes];
    nb = &nodes[&nodes[b->unk0] - nodes];
    return nb->unk38 - na->unk38;
}

/* build the matrices of a node list and its children (recursive) */
void func_801F374C(Node *node) {
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 rot;
    Mtx *m;
    MtxF mf;

    while (node != NULL) {
        scale = node->unk0 / D_8020BDE4;
        m = D_80217B70[node - D_8020BD30];
        rot = 0.0f;
        if (node->unk8 != 0.0f) {
            node->unk1C += D_8020BDEC * 360.0 / node->unk8 / 1800.0 * 60.0 / 60.0;
        }
        func_801FD484(&rot, &node->unk1C, &x, &y, &z, node->unkC);
        guTranslateF(mf, -x, -y, -z);
        guScale(&m[D_8035805C + 2], scale, scale, scale);
        guMtxCatF(D_80217A10[D_80217B50], mf, D_80217A10[D_80217B50 - 1]);
        D_80217B50--;
        guMtxF2L(D_80217A10[D_80217B50], &m[D_8035805C]);
        osWritebackDCache(m, 0x100);
        func_801F4C3C(node, scale);
        func_801F374C(node->unk14);
        D_80217B50++;
        node = node->unk10;
    }
}

#define ABS(x) ((x) > 0 ? (x) : -(x))

/* a node's glow sprite: a screen-space textured quad (64x64 IA8) at the
 * node's projected position, sized by scale and depth */
Gfx *func_801F3964(Gfx *gdl, u8 *dyn, Node *node, f32 scale) {
    Gfx *gfx;
    f32 size;
    s16 sx;
    s16 sy;
    u8 *tex;
    Vtx *vtx;
    s32 isize;

    gfx = gdl;
    size = node->unk0 * 64.0 / D_8020BDE4;
    vtx = D_80217690[node - D_8020BD30][D_8035805C];
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0xFF, 0xFF, node->unk18[0], node->unk18[1], node->unk18[2], 0xFF);
    gDPSetRenderMode(gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetTextureFilter(gfx++, G_TF_BILERP);
    gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
    gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gfx++, 0x2000, 0x2000, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombine(gfx++, 0x11FE23, 0xFFFFF3F9);
    switch (node - D_8020BD30) {
        case 1:
        case 4:
            tex = D_80215A80;
            break;
        default:
            tex = D_80215A7C;
            break;
    }
    if (node->unk38 > 1.0) {
        size = size * 10000.0 / node->unk38;
    }
    isize = size;
    func_8027690C(dyn, 0.0f, 0.0f, 0.0f, &sx, &sy, &D_80217B70[node - D_8020BD30][D_8035805C],
                  &D_80217B70[node - D_8020BD30][D_8035805C] + 2, (Mtx *) (dyn + 0x1280), 4.0f);
    if (ABS(sx) + size / 2.0 < 4096.0 && ABS(sy) + size / 2.0 < 4096.0) {
        gDPLoadTextureBlock(gfx++, tex, G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0, G_TX_CLAMP, G_TX_MIRROR, G_TX_NOMASK,
                            6, G_TX_NOLOD, G_TX_NOLOD);
        vtx[0].v.ob[0] = sx - isize / 2;
        vtx[0].v.ob[1] = sy - isize / 2;
        vtx[0].v.ob[2] = -10;
        vtx[0].v.tc[0] = 0;
        vtx[0].v.tc[1] = 0;
        vtx[1].v.ob[0] = sx - isize / 2;
        vtx[1].v.ob[1] = sy + isize / 2;
        vtx[1].v.ob[2] = -10;
        vtx[1].v.tc[0] = 0;
        vtx[1].v.tc[1] = 0x3F00;
        vtx[2].v.ob[0] = sx + isize / 2;
        vtx[2].v.ob[1] = sy + isize / 2;
        vtx[2].v.ob[2] = -10;
        vtx[2].v.tc[0] = 0x3F00;
        vtx[2].v.tc[1] = 0x3F00;
        vtx[3].v.ob[0] = sx + isize / 2;
        vtx[3].v.ob[1] = sy - isize / 2;
        vtx[3].v.ob[2] = -10;
        vtx[3].v.tc[0] = 0x3F00;
        vtx[3].v.tc[1] = 0;
        gSPVertex(gfx++, vtx, 4, 0);
        gSP1Triangle(gfx++, 0, 1, 2, 0);
        gSP1Triangle(gfx++, 0, 3, 2, 0);
        gDPPipeSync(gfx++);
    }
    gDPPipeSync(gfx++);
    osWritebackDCache(vtx, 4 * sizeof(Vtx));
    return gfx;
}

/* the same sprite for node 6, with a 32x32 RGBA32 texture */
Gfx *func_801F4110(Gfx *gdl, u8 *dyn, Node *node, f32 scale) {
    Gfx *gfx;
    f32 size;
    s16 sx;
    s16 sy;
    Vtx *vtx;
    s32 isize;

    gfx = gdl;
    size = node->unk0 * 64.0 / D_8020BDE4;
    vtx = D_80217690[node - D_8020BD30][D_8035805C];
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0xFF, 0xFF, node->unk18[0], node->unk18[1], node->unk18[2], 0xFF);
    gDPSetRenderMode(gfx++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetTextureFilter(gfx++, G_TF_BILERP);
    gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
    gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gfx++, 0x1000, 0x1000, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombine(gfx++, 0xFFB3FF, 0xFF64FE7F);
    if (node->unk38 > 1.0) {
        size = size * 10000.0 / node->unk38;
    }
    isize = size;
    func_8027690C(dyn, 0.0f, 0.0f, 0.0f, &sx, &sy, &D_80217B70[node - D_8020BD30][D_8035805C],
                  &D_80217B70[node - D_8020BD30][D_8035805C] + 2, (Mtx *) (dyn + 0x1280), 4.0f);
    if (ABS(sx) + size / 2.0 < 4096.0 && ABS(sy) + size / 2.0 < 4096.0) {
        gDPLoadTextureBlock(gfx++, D_80215A84, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, G_TX_CLAMP, G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        vtx[0].v.ob[0] = sx - isize / 2;
        vtx[0].v.ob[1] = sy - isize / 2;
        vtx[0].v.ob[2] = -10;
        vtx[0].v.tc[0] = 0;
        vtx[0].v.tc[1] = 0;
        vtx[1].v.ob[0] = sx - isize / 2;
        vtx[1].v.ob[1] = sy + isize / 2;
        vtx[1].v.ob[2] = -10;
        vtx[1].v.tc[0] = 0;
        vtx[1].v.tc[1] = 0x3E00;
        vtx[2].v.ob[0] = sx + isize / 2;
        vtx[2].v.ob[1] = sy + isize / 2;
        vtx[2].v.ob[2] = -10;
        vtx[2].v.tc[0] = 0x3E00;
        vtx[2].v.tc[1] = 0x3E00;
        vtx[3].v.ob[0] = sx + isize / 2;
        vtx[3].v.ob[1] = sy - isize / 2;
        vtx[3].v.ob[2] = -10;
        vtx[3].v.tc[0] = 0x3E00;
        vtx[3].v.tc[1] = 0;
        gSPVertex(gfx++, vtx, 4, 0);
        gSP1Triangle(gfx++, 0, 1, 2, 0);
        gSP1Triangle(gfx++, 0, 3, 2, 0);
        gDPPipeSync(gfx++);
    }
    gDPPipeSync(gfx++);
    osWritebackDCache(vtx, 4 * sizeof(Vtx));
    return gfx;
}

/* per node: depth from the camera, draw table entry, matrices and model */
void func_801F4878(Gfx *gdl, u8 *dyn) {
    Gfx *gfx;
    s32 i;
    Node *nodes;
    Node *node;
    Mtx *m;
    f32 scale;
    f32 dx;
    f32 dy;
    f32 dz;

    gfx = gdl;
    for (i = 0; i < 7; i++) {
        nodes = D_8020BD30;
        node = &nodes[&nodes[i] - nodes];
        m = D_80217B70[i];
        scale = node->unk0 / D_8020BDE4;
        dx = node->unk20 - D_80217B54;
        dy = node->unk24 - D_80217B58;
        dz = node->unk28 - D_80217B5C;
        node->unk38 = sqrtf(dx * dx + dy * dy + dz * dz);
        D_80218270[node - D_8020BD30].unk0 = node - D_8020BD30;
        D_80218270[node - D_8020BD30].unk4 = gfx;
        switch (i) {
            case 3:
                gSPMatrix(gfx++, dyn + 0x80, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
                gSPMatrix(gfx++, dyn + 0x140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
                gSPMatrix(gfx++, &m[D_8035805C], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
                gSPMatrix(gfx++, &m[D_8035805C + 2], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
                gSPMatrix(gfx++, dyn + 0x1280, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
                gfx = func_801FE238(gfx, dyn);
                break;
            case 6:
                gSPMatrix(gfx++, dyn + 0x100, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
                gSPMatrix(gfx++, dyn + 0x1C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
                gfx = func_801F4110(gfx, dyn, node, scale);
                break;
            default:
                gSPMatrix(gfx++, dyn + 0x100, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
                gSPMatrix(gfx++, dyn + 0x1C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
                gfx = func_801F3964(gfx, dyn, node, scale);
                break;
        }
        gSPEndDisplayList(gfx++);
    }
}

/* node position from the current matrix, then move it */
void func_801F4C3C(Node *node, f32 scale) {
    f32 w;
    f32 x;
    f32 y;

    x = 0.0f;
    y = 0.0f;
    if (node - D_8020BD30 != 6) {
        w = D_80217A10[D_80217B50][3][3];
        node->unk20 = D_80217A10[D_80217B50][3][0] / w;
        node->unk24 = D_80217A10[D_80217B50][3][1] / w;
        node->unk28 = D_80217A10[D_80217B50][3][2] / w;
    }
    switch (node - D_8020BD30) {
        case 3:
            func_801FD484(&D_8021A920, &D_8021A91C, &node->unk2C, &node->unk30, &node->unk34, scale * D_8021A918);
            node->unk2C += node->unk20;
            node->unk30 += node->unk24;
            node->unk34 += node->unk28;
            break;
        default:
            func_801FD484(&x, &y, &node->unk2C, &node->unk30, &node->unk34, scale * D_8021A918);
            node->unk2C += node->unk20;
            node->unk30 += node->unk24;
            node->unk34 += node->unk28;
            break;
    }
}

/* copy 16 words */
void func_801F4E1C(s32 *arg0, s32 *arg1) {
    s32 *src = arg0;
    s32 *dst = arg1;
    u32 i;

    for (i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}
