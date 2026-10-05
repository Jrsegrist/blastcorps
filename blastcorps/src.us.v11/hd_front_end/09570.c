#include "common.h"
#include <ultra64.h>

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
    f32 unk18;
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

typedef struct {
    u8 pad0[0x18];
    s8 unk18[4];
    s8 unk1C[8];
    u8 pad24[0xC];
} LevelInfo; /* 0x30 */

typedef struct {
    u8 pad0[0x18];
    u8 unk18[0x3C]; /* per level */
    u8 unk54[0x3C]; /* per level bit flags */
    u8 pad90[0x70];
} Player; /* 0x100 */

extern Node D_8020BD30[];
extern f32 D_8020BDE4; /* D_8020BD30[3].unk0 */
extern f32 D_8020BDEC; /* D_8020BD30[3].unk8 */
extern Node D_8020BE98; /* D_8020BD30[6] */
extern LevelInfo D_8020D810[];
extern MtxF D_80217A10[];
extern s32 D_80217B50; /* matrix stack index into D_80217A10 */
extern f32 D_80217B54;
extern f32 D_80217B58;
extern f32 D_80217B5C;
extern f32 D_80217B60;
extern f32 D_80217B64;
extern f32 D_80217B68;
extern s32 D_80217B6C; /* camera node */
extern Mtx D_80217B70[][4];
extern DrawEntry D_80218270[];
extern s16 D_802182A8;
extern f32 D_8021A918;
extern f32 D_8021A91C;
extern f32 D_8021A920;
extern u8 D_8035805C; /* which of the two matrix buffers is current */
extern u16 D_8035807C;
extern Player D_80364AF0[];
extern u8 D_80364AE8;

void func_801FD484(f32 *, f32 *, f32 *, f32 *, f32 *, f32);
s32 func_801FE760(s32);
void func_802595E0(u8 *base, s32 n, s32 size, s32 (*cmp)(void *, void *));
s32 func_80264BA4();
void func_801F374C(Node *);
void func_801F4878(u8 *, u8 *);
void func_801F4C3C(Node *, f32);
s32 func_801F36B0(DrawEntry *, DrawEntry *);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F0570.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F1568.s")

/* hd.c's LEVEL_DONE */
#define LEVEL_DONE(l) (D_80364AF0[D_80364AE8].unk18[l] > 0 && D_80364AF0[D_80364AE8].unk18[l] < 6) ? 1 : 0

/* is the level available: done, level 0, or unlocked by a done level */
s32 func_801F1DA8(s32 level) {
    LevelInfo *info;
    s32 i;
    s32 j;

    if (func_801FE760(level)) {
        return 0;
    }
    if (func_80264BA4(level) != 3) {
        return 0;
    }
    if ((LEVEL_DONE(level)) || level == 0) {
        return 1;
    }
    for (i = 0; i < 60; i++) {
        if (LEVEL_DONE(i)) {
            info = &D_8020D810[i];
            for (j = 0; j < 8 && info->unk1C[j] != -1; j++) {
                if (info->unk1C[j] == level) {
                    return 1;
                }
            }
            for (j = 0; j < 4 && info->unk18[j] != -1; j++) {
                if (info->unk18[j] == level && (D_80364AF0[D_80364AE8].unk54[i] & (1 << j))) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F2000.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F2428.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F2E20.s")

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
    func_801F4878(dyn + 0xACB0, dyn);
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F3964.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F4110.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F4878.s")

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
