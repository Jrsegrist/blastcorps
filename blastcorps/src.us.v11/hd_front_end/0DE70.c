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
#define D_803156F8 ((FeDyn *) D_803156F8)
#ifdef NON_MATCHING
#define D_80358070 (*(s32 *) &D_80358070)
#endif
/* end of views */

/*
 * Front-end 3D scene setup: loads one of three compressed scene blobs, and
 * builds the display lists that draw it (two per-frame-buffer camera setups).
 */


/* header of an inflated scene blob: offsets relative to its start */
typedef struct {
    s32 unk0;
    s32 unk4;
    u8 pad8[0xC];
    s32 unk14; /* segment 6 base */
    s32 unk18;
    s32 unk1C; /* display list */
    s32 unk20;
} FeHdr;

#ifndef NON_MATCHING
extern s32 D_80358070; /* heap pointer */
#endif

/* ROM addresses of the compressed scene blobs */
extern u8 D_006EA850[];
extern u8 D_006EAB90[];
extern u8 D_006EC4C0[];

extern u32 D_802182C0;
extern u8 * N64P D_802182C4;
extern Gfx * N64P D_802182C8;
extern void * N64P D_802182CC;
extern u8 * N64P D_80218350;
extern void * N64P D_80218358[2];
extern Gfx * N64P D_80218360[2][2];
extern Mtx D_80218370;
extern Mtx D_802183B0;
extern Mtx D_802183F0;
extern u8 D_80218430[];
extern u32 D_80218730;
extern u16 D_80218734;


/* Inflate scene blob `arg0` onto the heap and record its parts. */
void func_801F4E70(u8 arg0) {
    FeHdr *hdr;
    void *p;

    func_802A0700();
    switch (arg0) {
        case 0:
            D_802182C4 = D_006E8980;
            D_802182C0 = D_006EA850 - D_006E8980;
            break;
        case 1:
            D_802182C4 = D_006EA850;
            D_802182C0 = D_006EAB90 - D_006EA850;
            break;
        case 2:
            D_802182C4 = D_006EAB90;
            D_802182C0 = D_006EC4C0 - D_006EAB90;
            break;
    }
    func_8028B4C4((u32) D_802182C4, (u32) ((void *) N64_IPTR(D_80358070)), &D_802182C0, 12, 10, 1);
    hdr = (FeHdr *) N64_IPTR(D_80358070);
    D_80358070 += D_802182C0;
    D_802182C8 = (Gfx *) (hdr->unk1C + (u32) hdr);
    p = (void *) (hdr->unk20 + (u32) hdr);
    D_802182CC = (void *) (hdr->unk14 + (u32) hdr);
    func_802A08B4((u32 *) D_802182C8, p);
}

/* Draw the scene loaded by func_801F4E70. */
Gfx *func_801F4FBC(FeDyn *dyn, Gfx *arg1) {
    Gfx *gdl = arg1;

    gSPSegment(gdl++, 6, D_802182CC);
    gSPSegment(gdl++, 7, &D_802182D0[D_8035805C]);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    gSPLookAtX(gdl++, &dyn->unk3C00);
    gSPLookAtY(gdl++, (u8 *) &dyn->unk3C00 + 0x10);
    gSPMatrix(gdl++, &dyn->unk1240, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk12C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk1280, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk1300, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPDisplayList(gdl++, D_802182C8);
    return gdl;
}

/* Draw the spinning model set up by func_801F55D8 (turns 3 degrees a frame). */
Gfx *func_801F51C8(FeDyn *dyn, Gfx *arg1) {
    Gfx *gdl = arg1;
    void *seg;

    gImmp1(gdl++, G_RDPHALF_1, D_80218734);
    gSPLookAtX(gdl++, &dyn->unk3C00);
    gSPLookAtY(gdl++, (u8 *) &dyn->unk3C00 + 0x10);
    gSPMatrix(gdl++, &dyn->unk240, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gDPSetEnvColor(gdl++, 0, 0, 0, D_8036BB20);
    {
        s32 base;
        Mtx *m;

        base = ((FeHdr *) (((FeHdr *) D_80218350)->unk18 + D_80218350))->unk4;
        m = (Mtx *) ((u32) D_80218358[D_8035805C] + base);
        func_802021FC(D_80218430, D_80218358[D_8035805C], D_80218358[D_8035805C ^ 1]);
        D_80218730 += 3;
        guRotate(m, D_80218730 % 360, 0.0f, 1.0f, 0.0f);
        osWritebackDCache(m, sizeof(Mtx));
    }
    seg = (void *) (((FeHdr *) D_80218350)->unk14 + D_80218350);
    gSPSegment(gdl++, 6, seg);
    gSPSegment(gdl++, 7, D_80218358[D_8035805C]);
    gSPMatrix(gdl++, &D_802183F0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80218370, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(gdl++, &D_802183B0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(gdl++, D_80218360[0][D_8035805C]);
    gSPMatrix(gdl++, &D_802183F0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80218370, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(gdl++, &D_802183B0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(gdl++, D_80218360[1][D_8035805C]);
    return gdl;
}

/* Load the spinning model and set up both frame buffers' cameras. */
void func_801F55D8(void) {
    s32 i;
    FeDyn *dyn;

    func_80202100(0x96, &D_80218350, (u8 * N64P *) D_80218358, (u8 * N64P *) D_80218360[0]);
    guTranslate(&D_80218370, 150.0f, -45.0f, 0.0f);
    guScale(&D_802183B0, 1.5f, 1.5f, 1.5f);
    func_80202270(D_80218350, (u32 * N64P *) D_80218358, D_80218430);
    func_802022EC(D_80218430, 1, 0, 0, 0.0f, 2, 0);
    func_802022EC(D_80218430, 2, 0, 0, 0.0f, 1, 1);
    guRotate(&D_802183F0, 20.0f, 1.0f, 0.0f, 0.0f);
    for (i = 0; i < 2; i++) {
        dyn = &D_803156F8[i];
        guPerspective(&dyn->unk240, &D_80218734, 45.0f, 1.3333334f, 40.0f, 4000.0f, 1.0f);
        guLookAtReflect(&dyn->unk140, &dyn->unk3C00, 0.0f, 1.0f, 400.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }
}
