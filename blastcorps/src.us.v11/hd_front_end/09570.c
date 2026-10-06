#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* front-end level select: level availability, the level icon display list
 * and the starfield background */

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
    u8 unk90;
    u8 pad91[0x6F];
} Player; /* 0x100 */

typedef struct {
    u8 unk0; /* level */
    u8 pad1[7];
} Entry8;

typedef struct {
    u8 unk0; /* flags */
    u8 pad1[0x43];
} LevelFlags; /* 0x44 */

typedef struct {
    Gfx *dl[9];
} DlTable;

extern u8 D_02000000[]; /* segment 2 base */
extern Gfx D_8020BC88[];
extern DlTable D_8020BD08; /* per-category display lists */
extern LevelInfo D_8020D810[];
extern u16 D_80217288; /* perspNorm */
extern u16 *D_8021728C; /* star textures, 16x16 RGBA16 each */
extern Entry8 D_802E8F38[];
extern LevelFlags D_802E8F94[];
extern u8 D_803156F8[];
extern Vtx D_80215A88[]; /* globe vertices, 6 faces x 64 */
extern s32 D_80217290[64]; /* face grid x */
extern s32 D_80217390[64]; /* face grid y */
extern s32 D_80217490[64]; /* face grid s */
extern s32 D_80217590[64]; /* face grid t */
extern u8 *D_80358070; /* heap pointer */
extern Player D_80364AF0[];
extern u8 D_80364AE8;

s32 func_801F1DA8(s32);

#pragma intrinsic (sqrtf)

/* globe vertices: an 8x8 grid on each of the six cube faces, pushed out to
 * radius 250 (positions, texture coords, normals) */
void func_801F0570(void) {
    s32 i;
    s32 j;
    s32 k;
    s32 face;
    f32 half;
    f32 step;
    f32 len;
    f32 px;
    f32 py;
    f32 pz;
    s32 unused[2];

    half = 250.0f;
    step = half * 2.0 / 7.0;
    for (j = 0; j < 8; j++) {
        for (i = 0; i < 8; i++) {
            D_80217290[i + j * 8] = i * step - half;
            D_80217390[i + j * 8] = j * step - half;
            D_80217490[i + j * 8] = (i * 32) << 5;
            D_80217590[i + j * 8] = (j * 32) << 5;
        }
    }
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[k].v.ob[0] = px / len * 250.0;
        D_80215A88[k].v.ob[1] = py / len * 250.0;
        D_80215A88[k].v.ob[2] = pz / len * 250.0;
        D_80215A88[k].v.tc[0] = D_80217490[k];
        D_80215A88[k].v.tc[1] = D_80217590[k];
        D_80215A88[k].n.n[0] = px / len * 127.0f;
        D_80215A88[k].n.n[1] = py / len * 127.0f;
        D_80215A88[k].n.n[2] = pz / len * 127.0f;
        D_80215A88[k].n.a = 0xFF;
    }
    face = 0x40;
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[face + k].v.ob[0] = px / len * 250.0;
        D_80215A88[face + k].v.ob[1] = py / len * 250.0;
        D_80215A88[face + k].v.ob[2] = -pz / len * 250.0;
        D_80215A88[face + k].v.tc[0] = D_80217490[k];
        D_80215A88[face + k].v.tc[1] = D_80217590[k];
        D_80215A88[face + k].n.n[0] = px / len * 127.0f;
        D_80215A88[face + k].n.n[1] = py / len * 127.0f;
        D_80215A88[face + k].n.n[2] = -pz / len * 127.0f;
        D_80215A88[face + k].n.a = 0xFF;
    }
    face = 0x80;
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[face + k].v.ob[0] = px / len * 250.0;
        D_80215A88[face + k].v.ob[1] = pz / len * 250.0;
        D_80215A88[face + k].v.ob[2] = py / len * 250.0;
        D_80215A88[face + k].v.tc[0] = D_80217490[k];
        D_80215A88[face + k].v.tc[1] = D_80217590[k];
        D_80215A88[face + k].n.n[0] = px / len * 127.0f;
        D_80215A88[face + k].n.n[1] = pz / len * 127.0f;
        D_80215A88[face + k].n.n[2] = py / len * 127.0f;
        D_80215A88[face + k].n.a = 0xFF;
    }
    face = 0xC0;
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[face + k].v.ob[0] = px / len * 250.0;
        D_80215A88[face + k].v.ob[1] = -pz / len * 250.0;
        D_80215A88[face + k].v.ob[2] = py / len * 250.0;
        D_80215A88[face + k].v.tc[0] = D_80217490[k];
        D_80215A88[face + k].v.tc[1] = D_80217590[k];
        D_80215A88[face + k].n.n[0] = px / len * 127.0f;
        D_80215A88[face + k].n.n[1] = -pz / len * 127.0f;
        D_80215A88[face + k].n.n[2] = py / len * 127.0f;
        D_80215A88[face + k].n.a = 0xFF;
    }
    face = 0x100;
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[face + k].v.ob[0] = pz / len * 250.0;
        D_80215A88[face + k].v.ob[1] = px / len * 250.0;
        D_80215A88[face + k].v.ob[2] = py / len * 250.0;
        D_80215A88[face + k].v.tc[0] = D_80217490[k];
        D_80215A88[face + k].v.tc[1] = D_80217590[k];
        D_80215A88[face + k].n.n[0] = pz / len * 127.0f;
        D_80215A88[face + k].n.n[1] = px / len * 127.0f;
        D_80215A88[face + k].n.n[2] = py / len * 127.0f;
        D_80215A88[face + k].n.a = 0xFF;
    }
    face = 0x140;
    for (k = 0; k < 64; k++) {
        px = D_80217290[k];
        py = D_80217390[k];
        pz = 250.0f;
        len = sqrtf(px * px + py * py + pz * pz);
        D_80215A88[face + k].v.ob[0] = -pz / len * 250.0;
        D_80215A88[face + k].v.ob[1] = px / len * 250.0;
        D_80215A88[face + k].v.ob[2] = py / len * 250.0;
        D_80215A88[face + k].v.tc[0] = D_80217490[k];
        D_80215A88[face + k].v.tc[1] = D_80217590[k];
        D_80215A88[face + k].n.n[0] = -pz / len * 127.0f;
        D_80215A88[face + k].n.n[1] = px / len * 127.0f;
        D_80215A88[face + k].n.n[2] = py / len * 127.0f;
        D_80215A88[face + k].n.a = 0xFF;
    }
}

/*
 * TODO: func_801F1568 builds the globe display list: inflates worldtextures.raw
 * (ROM D_0066C900..D_0068B550) to the heap, carves the texture pointers out of
 * its tail, builds the vertices (func_801F0570) and emits, per face and grid cell,
 * five mip levels of a 17x17..2x2 RGBA16 texture plus two triangles.
 * Near-miss (528/528 insns, same length): everything matches up to the end of the
 * lod loop, where the target loads texoff, ww, lod, tmem, step (registers
 * t4 t6 t0 t3 t5) and this C loads lod first; that one allocation-order
 * difference renames registers in the rest of the function. Statement order,
 * comma forms, for/do-while forms and moving the increments into the loop
 * header were tried (IDO gives identical code for most), plus a 25-minute
 * permuter run, without a match.
 *
 * The NON_MATCHING build uses that draft (completed with the triangle commands
 * read off the asm), checked with tools_port/checks/fe_09570.txt.
 */
#ifdef NON_MATCHING
typedef struct {
    u16 tex[49][0x1A4];
} Globe; /* one face's 7x7 cells, five mip levels each */

#define D_0066C900 ((u8 *) 0x0066C900) /* ROM: worldtextures.raw (compressed) */
extern u8 D_0068B550[];                /* ... its end */
extern u16 *D_80215A70[3];
extern u8 *D_80215A7C;
extern u8 *D_80215A80;
extern u8 *D_80215A84;

Gfx *func_801F1568(void) {
    u8 *base;
    s32 size;
    s32 i;
    s32 j;
    s32 idx;
    s32 k;
    s32 faceOff;
    s32 flip;
    s32 lod;
    s32 texoff;
    s32 tmem;
    u32 step;
    s32 w;
    u32 line;
    u32 ww;
    Gfx *gfx;
    Gfx *dl;

    base = D_80358070;
    size = D_0068B550 - D_0066C900;
    func_8028B4C4((u32) D_0066C900, (u32) D_80358070, (u32 *) &size, 0xD, 0, 1);
    D_80358070 += size;
    D_8021728C = (u16 *) (D_80358070 - 0x6600);
    for (i = 0; i < 3; i++) {
        D_80215A70[i] = (u16 *) D_80358070 - (3 - i) * 0x800 - 0x1800;
    }
    D_80215A7C = D_80358070 - 0x3000;
    D_80215A80 = D_80358070 - 0x2000;
    D_80215A84 = D_80358070 - 0x1000;
    gfx = (Gfx *) D_80358070;
    dl = gfx;
    func_801F0570();
    gSPTexture(gfx++, 0x8000, 0x8000, 4, G_TX_RENDERTILE, G_ON);
    for (k = 0; k < 6; k++) {
        faceOff = k << 6;
        gSPVertex(gfx++, &D_80215A88[faceOff], 8, 0);
        flip = 8;
        for (i = 0; i < 7; i++) {
            gSPVertex(gfx++, &D_80215A88[faceOff + i * 8 + 8], 8, flip);
            for (j = 0; j < 7; j++) {
                lod = 0;
                idx = i * 7 + j;
                tmem = 0;
                texoff = 0;
                do {
                    w = (16 >> lod) + 1;
                    line = (w + 3) >> 2;
                    step = w * line;
                    ww = w * w;
                    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, w, &((Globe *) base)[k].tex[idx][texoff]);
                    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, line, tmem, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
                    gDPLoadTile(gfx++, G_TX_LOADTILE, 0, 0, w << 2, w << 2);
                    gDPSetTile(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, line, tmem, lod, 0, 0, 0, lod, 0, 0, lod);
                    gDPSetTileSize(gfx++, lod, ((j * 16) << 2) >> lod, ((i * 16) << 2) >> lod,
                                   ((j * 16 + 16) << 2) >> lod, ((i * 16 + 16) << 2) >> lod);
                    tmem += step;
                    texoff += ww + 3;
                } while (++lod < 5);
                /* faces 1, 2 and 5 wind the other way */
                if (k == 1 || k == 2 || k == 5) {
                    if (flip == 8) {
                        gSP1Triangle(gfx++, j, j + 8, j + 9, 0);
                        gSP1Triangle(gfx++, j, j + 9, j + 1, 0);
                    } else {
                        gSP1Triangle(gfx++, j + 8, j, j + 1, 0);
                        gSP1Triangle(gfx++, j + 8, j + 1, j + 9, 0);
                    }
                } else if (flip == 8) {
                    gSP1Triangle(gfx++, j, j + 9, j + 8, 0);
                    gSP1Triangle(gfx++, j, j + 1, j + 9, 0);
                } else {
                    gSP1Triangle(gfx++, j + 8, j + 1, j, 0);
                    gSP1Triangle(gfx++, j + 8, j + 9, j + 1, 0);
                }
            }
            flip ^= 8;
        }
    }
    gSPEndDisplayList(gfx++);
    D_80358070 += (gfx - dl) * sizeof(Gfx);
    return dl;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/09570/func_801F1568.s")
#endif

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

/* display list of the level icons (one quad per level with its bit set in
 * the player's unk90), drawn in two passes */
Gfx *func_801F2000(void) {
    Vtx *vtx;
    u8 *heap;
    Gfx *dl;
    Gfx *gfx;
    s32 i;
    s32 pass;

    vtx = (Vtx *) D_80358070;
    heap = D_80358070;
    D_80358070 += 0x300;
    gfx = (Gfx *) D_80358070;
    dl = gfx;
    gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
    gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
    gSPTexture(gfx++, 0x8000, 0x8000, 5, G_TX_RENDERTILE, G_ON);
    gDPSetTextureLOD(gfx++, G_TL_LOD);
    gDPSetCycleType(gfx++, G_CYC_2CYCLE);
    gDPSetRenderMode(gfx++, G_RM_PASS, G_RM_XLU_SURF2);
    for (pass = 0; pass < 2; pass++) {
        gDPPipeSync(gfx++);
        if (pass != 0) {
            gDPSetCombine(gfx++, 0x26A00A, 0x1F0C93FF);
        } else {
            gSPDisplayList(gfx++, D_8020BC88);
            gDPSetCombine(gfx++, 0x26A1FF, 0x1F1492FF);
        }
        for (i = 0; i < 60; i++) {
            s32 j;
            s32 found;

            for (j = 0, found = 0; j < 6 && !found; j++) {
                if (D_802E8F38[j].unk0 == i) {
                    found = 1;
                }
            }
            /* j is one past the match here, so bit (j + 31) & 31 = j - 1 */
            if (found && (D_80364AF0[D_80364AE8].unk90 & (1 << (j + 31)))) {
                func_801FCE74(vtx, i, pass * 1.5 + 2.5, pass * 1.5 + 4.0, 32, 32, 2.25f, 1);
                gSPVertex(gfx++, vtx, 4, 0);
                gSP1Triangle(gfx++, 0, 1, 2, 0);
                gSP1Triangle(gfx++, 2, 3, 0, 0);
                vtx += 4;
            }
        }
    }
    gDPPipeSync(gfx++);
    gSPEndDisplayList(gfx++);
    D_80358070 = (u8 *) gfx;
    osWritebackDCache(heap, 0x300);
    return dl;
}

/* display list of the level select icons, one category (colour/combiner) at a time,
 * flushing vertices 16 at a time */
Gfx *func_801F2428(void) {
    LevelInfo *info;
    Vtx *vtx;
    Gfx *gfx;
    Gfx *dl;
    s32 lvl;
    s32 j;
    s32 n;
    s32 flushed;
    s32 pending;
    u8 found;
    s8 cat;
    u8 avail;
    f32 scale;
    DlTable dls; /* initialised from D_8020BD08 */
    Gfx *cur;
    Gfx *prev;
    u8 r;
    u8 g;
    u8 b;
    u8 a;

    n = 0;
    flushed = 0;
    vtx = (Vtx *) D_80358070;
    dls = D_8020BD08;
    prev = NULL;
    D_80358070 += 0x1E00;
    gfx = (Gfx *) D_80358070;
    dl = gfx;
    gDPPipeSync(gfx++);
    gDPSetTextureLOD(gfx++, G_TL_LOD);
    gDPSetCycleType(gfx++, G_CYC_2CYCLE);
    gDPSetRenderMode(gfx++, G_RM_PASS, G_RM_XLU_SURF2);
    gSPTexture(gfx++, 0x8000, 0x8000, 5, G_TX_RENDERTILE, G_ON);
    for (cat = 8; cat >= 0; cat--) {
        cur = dls.dl[cat];
        if (cur != prev) {
            gSPDisplayList(gfx++, cur);
        }
        prev = cur;
        gDPPipeSync(gfx++);
        switch (cat) {
            case 0:
            case 8:
                gDPSetCombine(gfx++, 0x26A1FF, 0x1F14933F);
                r = 0, g = 0, b = 0, a = 0xFF;
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                gDPSetCombine(gfx++, 0x26A00A, 0x1F0C93FF);
                r = 0, g = 0, b = 0, a = 0xFF;
                break;
            case 5:
                gDPSetCombine(gfx++, 0x26A1FF, 0x1F0C933F);
                r = 0x50, g = 0x50, b = 0x50, a = 0xFF;
                break;
            case 6:
                gDPSetCombine(gfx++, 0x26A1FF, 0x1F0C933F);
                r = 0xFF, g = 0, b = 0, a = 0xFF;
                break;
            case 7:
                gDPSetCombine(gfx++, 0x26A1FF, 0x1F0C933F);
                r = 0, g = 0xFF, b = 0, a = 0xFF;
                break;
        }
        for (lvl = 0; lvl < 60; lvl++) {
            avail = (cat == 6 || cat == 7) && (LEVEL_DONE(lvl)) && D_80364AF0[D_80364AE8].unk18[lvl] != 4;
            if (D_80364AF0[D_80364AE8].unk18[lvl] == cat || avail) {
                info = &D_8020D810[lvl];
                if (func_801F1DA8(lvl)) {
                    if (avail) {
                        for (j = 0, found = 0; j < 4 && info->unk18[j] != -1 && !found; j++) {
                            if (!(D_80364AF0[D_80364AE8].unk54[lvl] & (1 << j))) {
                                found = 1;
                            }
                        }
                        if ((found && cat == 6) || (!found && cat == 7)) {
                            continue;
                        }
                    }
                    if (D_802E8F94[lvl].unk0 & 0x81) {
                        scale = 1.75f;
                    } else {
                        scale = 1.0f;
                    }
                    if (avail) {
                        scale += 0.3;
                    }
                    func_801FCE74(&vtx[n], lvl, 0.0f, 0.0f, 32, 32, scale, 0);
                    for (j = 0; j < 4; j++) {
                        vtx[n + j].v.cn[0] = r;
                        vtx[n + j].v.cn[1] = g;
                        vtx[n + j].v.cn[2] = b;
                        vtx[n + j].v.cn[3] = a;
                    }
                    n += 4;
                    pending = n - flushed;
                    if (!(pending & 0xF)) {
                        Vtx *v = &vtx[n - 16];

                        gSPVertex(gfx++, v, 16, 0);
                        for (j = 0; j < 16; j += 4) {
                            gSP1Triangle(gfx++, j, j + 1, j + 2, 0);
                            gSP1Triangle(gfx++, j + 2, j + 3, j, 0);
                        }
                    }
                }
            }
        }
        pending = n - flushed;
        if (pending % 16) {
            Vtx *v = &vtx[n - pending % 16];

            gSPVertex(gfx++, v, pending % 16, 0);
            for (j = 0; j < pending % 16; j += 4) {
                gSP1Triangle(gfx++, j, j + 1, j + 2, 0);
                gSP1Triangle(gfx++, j + 2, j + 3, j, 0);
            }
            flushed = n;
        }
    }
    gDPPipeSync(gfx++);
    gSPEndDisplayList(gfx++);
    D_80358070 = (u8 *) gfx;
    osWritebackDCache(vtx, 0x1E00);
    return dl;
}

/* starfield: projection/view matrices, then 0x200 random textured star quads
 * (vertices at segment 2 offset 0x15C0, copied to the other buffer) */
Gfx *func_801F2E20(void) {
    Vtx *verts;
    u8 *copy;
    Mtx *mtx;
    Gfx *gfx;
    Gfx *dl;
    s32 i;
    s32 j;
    s32 unused[4];
    s32 k;
    s32 unused2[7];

    verts = (Vtx *) (D_803156F8 + 0x15C0);
    copy = D_803156F8 + 0x22A58;
    mtx = (Mtx *) D_80358070;
    D_80358070 += 0x80;
    gfx = (Gfx *) D_80358070;
    dl = gfx;
    guPerspective(mtx, &D_80217288, 45.0f, 4.0f / 3.0f, 100.0f, 25000.0f, 1.0f);
    guLookAt(mtx + 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    gImmp1(gfx++, G_RDPHALF_1, D_80217288);
    gSPMatrix(gfx++, mtx, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gfx++, mtx + 1, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPTexture(gfx++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
    k = 0;
    for (i = 0; i < 0x200; i += 16) {
        gSPVertex(gfx++, (Vtx *) (i * 16 + 0x15C0 + (u32) D_02000000), 16, 0);
        gDPPipeSync(gfx++);
        for (j = 0; j < 16; j += 4) {
            if (((i + j) >> 2) % 43 == 0) {
                gDPLoadTextureBlock(gfx++, D_8021728C + k * 256, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
                                    G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                k++;
            }
            func_801FDCA4(verts, (i + j) / 4, func_8026A828(0, 25000));
            verts[i + j].v.tc[0] = 0;
            verts[i + j].v.tc[1] = 0;
            verts[i + j + 1].v.tc[0] = 0;
            verts[i + j + 1].v.tc[1] = 0x3C0;
            verts[i + j + 2].v.tc[0] = 0x3C0;
            verts[i + j + 2].v.tc[1] = 0x3C0;
            verts[i + j + 3].v.tc[0] = 0x3C0;
            verts[i + j + 3].v.tc[1] = 0;
            gSP1Triangle(gfx++, j, j + 1, j + 2, 0);
            gSP1Triangle(gfx++, j, j + 2, j + 3, 0);
        }
    }
    for (i = 0; i < 0x200 * sizeof(Vtx); i++) {
        copy[i] = ((u8 *) verts)[i];
    }
    gDPPipeSync(gfx++);
    gSPEndDisplayList(gfx++);
    D_80358070 += (gfx - dl) * sizeof(Gfx);
    osWritebackDCache(verts, 0x200 * sizeof(Vtx));
    return dl;
}
