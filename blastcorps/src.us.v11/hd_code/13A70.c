#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_803643C8
#define LEGACY_D_803643CC
#define LEGACY_D_803F7654
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_02000000 (*(VtxBuf *) D_02000000)
#define D_803F4030 ((Object *) D_803F4030)
#ifdef NON_MATCHING
#define D_803643C8 (*(Struct13A70 * *) &D_803643C8)
#define D_803643CC (*(Struct13A70 * *) &D_803643CC)
#define D_803F7654 (*(Object * *) &D_803F7654)
#endif
/* end of views */

/* Shadow/marker records (0x1040-byte entries with a 4 KB buffer each), the
 * 64x64 off-screen render that fills one, and the ground-shadow quads drawn
 * under objects of type 0xBA..0xBC. */

#define PHYS(x) ((u32)(x) & 0x1FFFFFFF)

/* XLU surfaces with CVG_DST_SAVE where the 2.0I headers have CVG_DST_FULL */
#define RM_ZB_XLU_SAVE(clk) (RM_ZB_XLU_SURF(clk) | CVG_DST_SAVE)
#define RM_XLU_SAVE(clk) (RM_XLU_SURF(clk) | CVG_DST_SAVE)

typedef struct {
    u8 pad[0x1000];
    f32 unk1000;
    s32 unk1004;
    s32 unk1008;
    s32 unk100C;
    s32 unk1010;
    s32 unk1014;
    s16 unk1018;
    s16 unk101A;
    s16 unk101C;
    s16 unk101E;
    s16 unk1020;
    u8 unk1022;
    u8 unk1023;
    u8 pad1024[0x1C];
} Struct13A70; /* size 0x1040 */

typedef struct {
    u8 pad0[0x10];
    s32 x;
    s32 y;
    s32 z;
    u8 pad1C[0x14];
    s32 type;
    u8 pad34[0x10];
    s32 unk44;
    u8 pad48[0xA2];
    u8 unkEA;
    u8 padEB[0x11];
} Object; /* size 0xFC */

typedef struct VtxBuf {
    u8 pad[0x3900];
    Vtx vtx[1];
} VtxBuf;

#ifndef NON_MATCHING
extern Struct13A70 *D_803643C8;
#ifndef NON_MATCHING
extern Struct13A70 *D_803643CC;
#endif
#endif
extern Gfx D_803650B0[];
extern Vp D_802E8C60;
extern Mtx D_803651F0;
extern Mtx D_80365230;
extern Mtx D_80365270;
extern Mtx D_803652B0;
extern Mtx D_803652F0;
#ifndef NON_MATCHING
extern Object *D_803F7654;
#endif


void func_80258230(u8 id, s32 arg1, s16 arg2, s16 arg3) {
    D_803643CC->unk1022 = id;
    D_803643CC->unk1023 = 0;
    D_803643CC->unk1000 = arg1;
    D_803643CC->unk1018 = arg2;
    D_803643CC->unk101A = arg3;
    D_803643CC->unk101C = 0;
    D_803643CC->unk101E = 0;
    D_803643CC->unk1020 = 0;
    D_803643CC++;
}

void func_802582C4(u8 id, s32 x, s32 y, s32 z, s32 arg4, s32 arg5, s32 arg6, s32 arg7) {
    u8 found = 0;
    Struct13A70 *p;
    u8 hit = 0;
    u8 mode = 0;
    u8 ok;
    s16 h;
    s32 d1;
    s32 d2;
    s32 d3;

    if (id == 9) {
        hit = func_802ABEDC(x, arg4, z);
        if (hit) {
            mode = 1;
        }
        ok = func_8027EED8(x >> 5, z >> 5, &h);
        if (ok) {
            d1 = arg4 - (h << 5);
            d3 = arg4 - y;
            if (hit) {
                d2 = arg4 - D_803EBBF8;
            } else {
                d2 = 9999999;
            }
            if (d1 > 0 && d1 < d2 && d1 < d3) {
                D_803EBBF8 = h << 5;
                mode = 2;
            }
        }
    }
    for (p = D_803643C8; !found;) {
        if (p->unk1022 == id) {
            p->unk1004 = x;
            if (mode) {
                p->unk1008 = D_803EBBF8;
            } else {
                p->unk1008 = y;
            }
            p->unk100C = z;
            p->unk1014 = y;
            p->unk101C = arg5;
            p->unk101E = arg7;
            p->unk1020 = arg6;
            p->unk1010 = arg4;
            p->unk1023 = mode;
            found = 1;
        } else {
            p++;
        }
    }
}

s32 func_802584BC(u8 id) {
    Struct13A70 *p = D_803643C8;

    while (TRUE) {
        if (p->unk1022 == id) {
            return p->unk1008;
        }
        p++;
    }
}

s32 func_80258500(u8 id) {
    Struct13A70 *p = D_803643C8;

    while (TRUE) {
        if (p->unk1022 == id) {
            return p->unk1014;
        }
        p++;
    }
}

void func_80258544(void *cimg, s32 x, s32 y, s32 z, f32 dist, Gfx *dl, void *seg6, void *seg7) {
    Gfx *gdl = D_803650B0;
    u16 perspNorm;
    s32 pad;

    gSPViewport(gdl++, PHYS(&D_802E8C60));
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 6, PHYS(seg6));
    gSPSegment(gdl++, 7, PHYS(seg7));
    gDPPipeSync(gdl++);
    gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, 63, 63);
    gDPSetPrimColor(gdl++, 0, 0, 255, 255, 255, 255);
    gDPSetColorDither(gdl++, G_CD_DISABLE);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_8b, 64, PHYS(cimg));
    gDPSetFillColor(gdl++, 0x00010001);
    gDPFillRectangle(gdl++, 0, 0, 63, 63);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1), GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1));
    gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    guPerspective(&D_803651F0, &perspNorm, 45.0f, 1.0f, 1.0f, 1000.0f, 1.0f);
    gSPMatrix(gdl++, PHYS(&D_803651F0), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, perspNorm);
    guTranslate(&D_803652F0, 0.0f, 0.0f, -dist);
    gSPMatrix(gdl++, PHYS(&D_803652F0), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    guRotate(&D_80365270, -90.0f, 1.0f, 0.0f, 0.0f);
    gSPMatrix(gdl++, PHYS(&D_80365270), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    guRotate(&D_803652B0, 180.0f, 0.0f, 1.0f, 0.0f);
    gSPMatrix(gdl++, PHYS(&D_803652B0), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    guTranslate(&D_80365230, -(x / 32.0f), -(y / 32.0f), -(z / 32.0f));
    gSPMatrix(gdl++, PHYS(&D_80365230), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPDisplayList(gdl++, PHYS(dl));
    gSPEndDisplayList(gdl++);
    osWritebackDCache(&D_803651F0, 0x140);
    func_80284E54(D_803650B0, gdl - D_803650B0, 1, 0, 0x61F, 0);
}

void func_80258B78(Gfx **gdlp, VtxBuf *buf) {
    Gfx *gdl = *gdlp;
    s32 i = 0;
    s32 n = 0;
    s32 h;
    s16 x;
    s16 y;
    s16 z;
    s16 loaded = 0;

    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombineMode(gdl++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(gdl++, 0, 0, 200, 200, 200, 200);
    while (&D_803F4030[i] != D_803F7654) {
        if (D_803F4030[i].unkEA == 0) {
            switch (D_803F4030[i].type) {
                case 0xBA:
                case 0xBB:
                case 0xBC:
                    h = 60 - (D_803F4030[i].y - D_803F4030[i].unk44) / 800;
                    if (h < 0) {
                        h = 0;
                    }
                    x = D_803F4030[i].x >> 5;
                    y = D_803F4030[i].unk44 >> 5;
                    z = D_803F4030[i].z >> 5;
                    buf->vtx[n].v.ob[0] = x - h;
                    buf->vtx[n].v.ob[1] = y;
                    buf->vtx[n].v.ob[2] = z - h;
                    buf->vtx[n].v.tc[0] = 0;
                    buf->vtx[n].v.tc[1] = 0;
                    n++;
                    buf->vtx[n].v.ob[0] = x + h;
                    buf->vtx[n].v.ob[1] = y;
                    buf->vtx[n].v.ob[2] = z - h;
                    buf->vtx[n].v.tc[0] = 0x7E0;
                    buf->vtx[n].v.tc[1] = 0;
                    n++;
                    buf->vtx[n].v.ob[0] = x + h;
                    buf->vtx[n].v.ob[1] = y;
                    buf->vtx[n].v.ob[2] = z + h;
                    buf->vtx[n].v.tc[0] = 0x7E0;
                    buf->vtx[n].v.tc[1] = 0x7E0;
                    n++;
                    buf->vtx[n].v.ob[0] = x - h;
                    buf->vtx[n].v.ob[1] = y;
                    buf->vtx[n].v.ob[2] = z + h;
                    buf->vtx[n].v.tc[0] = 0;
                    buf->vtx[n].v.tc[1] = 0x7E0;
                    n++;
                    if (!loaded) {
                        gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_80365330), G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0,
                                            G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        loaded = 1;
                    }
                    gDPPipeSync(gdl++);
                    if (D_802E8BDC == 0x10 && x == 0x19B7 && z == 0xF99) {
                        gSPSetGeometryMode(gdl++, G_ZBUFFER);
                        gDPSetRenderMode(gdl++, RM_ZB_XLU_SAVE(1), RM_ZB_XLU_SAVE(2));
                    } else {
                        gSPClearGeometryMode(gdl++, G_ZBUFFER);
                        gDPSetRenderMode(gdl++, RM_XLU_SAVE(1), RM_XLU_SAVE(2));
                    }
                    gSPVertex(gdl++, &D_02000000.vtx[n - 4], 4, 0);
                    gSP1Triangle(gdl++, 0, 1, 3, 0);
                    gSP1Triangle(gdl++, 1, 2, 3, 0);
                    break;
            }
        }
        i++;
    }
    gDPPipeSync(gdl++);
    *gdlp = gdl;
}
