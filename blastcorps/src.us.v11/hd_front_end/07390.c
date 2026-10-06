#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_803156F8 ((FeDyn *) D_803156F8)
#define D_80358050 ((void * *) D_80358050)
/* end of views */


extern s32 D_802159C0;


void func_801EE390(void) {
}

/* draw one front-end frame: a cleared screen plus the yoshi window `arg0` */
void func_801EE398(s32 arg0) {
    FeDyn *buf;
    Gfx *gdl;
    s32 i;

    buf = &D_803156F8[D_8035805C ^ 1];
    D_80358080 = 0;
    D_80358084 = 0;
    gdl = buf->dl;
    func_802A5720();
    func_8025B2B8();
    if (currentYoshiWindow != arg0) {
        func_8026AF6C(arg0 | 0x8000 | 0x2000);
        func_80260A10();
        D_802159C0 = 0;
    }
    if (D_802159C0 == 2) {
        osViBlack(0);
    }
    func_80259450();
    func_80284E54(D_803156F8[D_8035805C].dl, D_80358078, 1, 1, 0x4D2, 0);
    D_8035805C ^= 1;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(buf));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPPipeSync(gdl++);
    gDPSetFillColor(gdl++, 0x00010001);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gdl = func_8026BBD0(gdl, &D_803156F8[D_8035805C], &D_80358078);
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    D_80358078 = gdl - buf->dl;
    for (i = 0; i < D_80358080; i++) {
        osRecvMesg(&D_80315180, NULL, OS_MESG_BLOCK);
    }
    for (i = 0; i < D_80358080 - D_80358084; i++) {
        func_802A57AC();
    }
    func_80285110(0x4D2);
    D_802159C0++;
}
