#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gDynamicBufs ((FeDyn *) gDynamicBufs)
#define gFrameBufferPhys ((void * N64P *) gFrameBufferPhys)
/* end of views */


extern s32 D_802159C0;


void func_801EE390(void) {
}

/* draw one front-end frame: a cleared screen plus the yoshi window `arg0` */
void func_801EE398(s32 arg0) {
    FeDyn *buf;
    Gfx *gdl;
    s32 i;

    buf = &gDynamicBufs[gBufferIndex ^ 1];
    nextdma = 0;
    no_palette_dmas = 0;
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
    rsp_send_task((u64 *) (gDynamicBufs[gBufferIndex].dl), gFrameDlLen, 1, 1, 0x4D2, 0);
    gBufferIndex ^= 1;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(buf));
    gSPSegment(gdl++, 1, osVirtualToPhysical(gStaticSegment));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameBufferPhys[gBufferIndex]);
    gDPPipeSync(gdl++);
    gDPSetFillColor(gdl++, 0x00010001);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gdl = func_8026BBD0(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    gFrameDlLen = gdl - buf->dl;
    for (i = 0; i < nextdma; i++) {
        osRecvMesg(&textureDmaMessageQ, NULL, OS_MESG_BLOCK);
    }
    for (i = 0; i < nextdma - no_palette_dmas; i++) {
        func_802A57AC();
    }
    rsp_wait_task(0x4D2);
    D_802159C0++;
}
