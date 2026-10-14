#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuEntry *) D_8020C070)
#define gYoshiWindows ((MenuPage *) gYoshiWindows)
#define gDynamicBufs ((FeDyn *) gDynamicBufs)
/* end of views */

/* back_loop.c: the front end's per-frame loop (menus behind the yoshi windows) */


typedef struct {
    u8 pad0[4];
    s16 unk4;
    u8 pad6[2];
    s32 unk8;
    u8 padC[2];
    u16 unkE;
    u16 unk10;
    u8 pad12[6];
    u16 unk18;
    u8 pad1A[2];
} MenuPage;

typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char * N64P unkC;  /* title */
    void * N64P unk10; /* glyph list */
    u8 pad14[8];
} MenuEntry;

#ifndef NON_MATCHING
extern s32 D_80367738;
#endif
extern u8 D_8021AB70;
extern char D_8021AB72[];
extern s16 D_8021AB74;
extern s16 D_8021AB76;
extern s32 D_8021AB7C;

int sprintf(char *, const char *, ...);

/* Rare's assert; line numbers are the original back_loop.c's */
#define BACK_ASSERT(EX, line) \
    if (!(EX)) \
    debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "back_loop.c", line)

/* One front-end frame: handles pak replies and the menu (yoshi) selection for
 * the current mode, starts the previous display list and builds this frame's,
 * then picks the next mode once the current one is idle. */
void func_801FE990(void) {
    s32 i;
    Gfx *gdl;
    OSMesg msg;

    if (gGameMode & 0x80010000000) {
        if (D_8021AB7C == 0) {
            func_80260650(D_80367738, 0x73, &D_8021AB7C);
        }
        if (yoshiState == 2) {
            osRecvMesg(&pakToGameMessageQ, &msg, 1);
            BACK_ASSERT(MQ_IS_EMPTY(&pakToGameMessageQ), 62);
            D_8021AB70 = !msg;
            if (gGameMode == 0x10000000) {
                if (D_8021AB70) {
                    func_801EA278();
                }
                gNextGameMode = 0x8000;
            } else {
                gNextGameMode = 0x2000000000;
            }
            func_802608C8((void *) N64_IPTR(D_8021AB7C));
            yoshi_demand(0x4000);
        }
    }
    if (yoshiSelection) {
        debug_printf("yoshiSelection in back_loop is %d %d\n", yoshiSelection, lowest_set_bit(gGameMode));
        switch (gGameMode) {
            case 0x10000:
                if (yoshiSelection == 0xFFFF) {
                    gNextGameMode = 0x8000000000000000;
                    func_801E8EB8(4, 1);
                } else if (gYoshiWindows[10].unk18 - 2 < 4) {
                    gNextGameMode = 0x2000000;
                } else if (D_802154B0 == 4) {
                    D_8021AB70 = 0;
                    D_8039C541 = 1;
                    gNextGameMode = 0x8000;
                } else {
                    gNextGameMode = 0x800000;
                }
                break;
            case 0x100000000:
                if (yoshiSelection != 0xFFFF && func_801F73FC()) {
                    D_8021AB74 = yoshiSelection;
                    gNextGameMode = 0x4000000000;
                }
                break;
            case 0x400000:
                if (yoshiSelection == 0xFFFF) {
                    func_801E8EB8(4, 1);
                    gNextGameMode = 0x200000;
                } else {
                    playerNumber = yoshiSelection - 2;
                    if (playerNumber < 4) {
                        gNextGameMode = 0x100000;
                    } else {
                        gNextGameMode = 0x200000;
                    }
                }
                break;
            case 0x80000:
                if (yoshiSelection == 0xC) {
                    func_801EA108(playerNumber, 0, 0);
                }
                func_801E8EB8(4, 1);
                gNextGameMode = 0x800000;
                break;
            case 0x8000000000:
                if (yoshiSelection == 0xC) {
                    osSendMesg(&D_80219EF8, (OSMesg) ((D_8021AB74 << 16) | 5), 1);
                }
                gNextGameMode = 0x10000000000;
                break;
            case 0x80:
            case 0x8000000:
                if (yoshiSelection == 0xFFFF) {
                    gNextGameMode = 0x4000;
                } else {
                    i = gYoshiWindows[currentYoshiWindow].unk10 + gYoshiWindows[currentYoshiWindow].unkE - yoshiSelection - 1;
                    switch (i) {
                        case 0:
                            gNextGameMode = 0x4000;
                            break;
                        case 1:
                            gNextGameMode = 0x40;
                            break;
                        case 2:
                            if (!(((s32) gLevelKind) & 0x81)) {
                                if (((s32) gLevelKind) == 2 && gGameMode == 0x8000000) {
                                    D_8039CA60 = 1;
                                }
                                gNextGameMode = 0x20000000;
                            } else {
                                gNextGameMode = 0x2000;
                            }
                            break;
                        case 3:
                            gNextGameMode = 0x40000000000;
                            break;
                    }
                }
                break;
            case 0x40000000000:
                if (yoshiSelection == 0xFFFF) {
                    playerNumber = D_80364AE9;
                }
                if (playerNumber == D_80364AE9 || playerNumber == playerNumberAtStart || yoshiSelection == 0xFFFF) {
                    gNextGameMode = D_8021A830;
                } else {
                    gNextGameMode = 0x100000000000000;
                }
                break;
            case 0x4000000000000:
                func_801E8C40(4);
                gNextGameMode = 0x8000000000000;
                break;
            case 0x1000000000000:
                func_80275390(0x20000000000000);
                break;
            case 0x40000000000000:
                if (yoshiSelection == 0xC) {
                    func_801EA108(playerNumber, 0, 1);
                }
                break;
            case 0x100000000000000:
                func_801E8EB8(4, 1);
                break;
            case 0x4000000000000000:
                if (yoshiSelection == 0xC) {
                    func_80275270(0x400000000000000, 0.5f);
                } else {
                    func_80275270(0x4000, 0.5f);
                }
                break;
            default:
                debug_printf("backdrop illegal yoshi selection\n");
                break;
        }
        D_8021AB76 = yoshiSelection;
        yoshiSelection = 0;
    }
    nextdma = 0;
    no_palette_dmas = 0;
    tex_reset_decode_queue();
    rsp_send_task((u64 *) (gDynamicBufs[gBufferIndex].dl), gFrameDlLen, 1, 1, 0x4D2, 0);
    gBufferIndex ^= 1;
    gdl = gDynamicBufs[gBufferIndex].dl;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(&gDynamicBufs[gBufferIndex]));
    gSPSegment(gdl++, 1, osVirtualToPhysical(gStaticSegment));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gSPClipRatio(gdl++, FRUSTRATIO_3);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    if (gGameMode & 0x88000080) {
        gDPSetDepthImage(gdl++, gDepthBufferPhys);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBufferPhys);
        gDPSetFillColor(gdl++, 0xFFFCFFFC);
        gDPFillRectangle(gdl++, 220, 110, 300, 160);
    } else if (gGameMode & 0x40000000) {
        gDPSetDepthImage(gdl++, gDepthBufferPhys);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBufferPhys);
        gDPSetFillColor(gdl++, 0xFFFCFFFC);
        gDPFillRectangle(gdl++, 0, 0, 319, 239);
    }
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameBufferPhys[gBufferIndex]);
    text_begin_frame();
    gdl = func_80200BE0(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    if (gGameMode & 0xC000000000000) {
        gdl = yoshi_draw(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    }
    if (gGameMode == 0x4000000000000) {
        if (gFramesInMode == 0) {
            func_80262008(0x27, 0);
            func_80260EE0(0xF);
        }
        if (gYoshiTypeState == 0 && playerNumber == 4) {
            playerNumber = playerNumberAtStart;
            func_801E8DCC(playerNumber);
            gYoshiWindows[56].unk8 |= 0x20;
        }
    }
    if (gGameMode & 0x818D04001AFB8080) {
        gdl = func_801E9718(gdl, (struct PlayerSelDyn *) &gDynamicBufs[gBufferIndex], 0xC2);
    }
    if (gGameMode & 0x898C0FE313F78002) {
        gdl = title_draw_attract(gdl, (s32) &gDynamicBufs[gBufferIndex], gBufferIndex, &gFrameDlLen);
    }
    if (gGameMode & 0x410000) {
        u8 old = playerNumber;

        playerNumber = gYoshiWindows[10].unk18 - 2;
        if (playerNumber != old) {
            if (old == 4) {
                func_801E8EB8(playerNumber, 0);
            } else {
                func_801E8EB8(playerNumber, 1);
            }
        }
    }
    cont_start_read();
    if (gGameMode & 0x40000000) {
        gdl = func_801ED800(gdl, &gDynamicBufs[gBufferIndex], gBufferIndex, &gFrameDlLen);
    }
    if (gGameMode == 0x1000000000000) {
        gdl = func_80201364((s32) gDynamicBufs, gdl);
    }
    gdl = func_8024C404(gdl, (DynamicBuf *) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if (gGameMode & 0x88000080) {
        gdl = func_801EC770(gdl, &gDynamicBufs[gBufferIndex], &gFrameDlLen);
        if (D_80364A71 != -1 && yoshiState == 2) {
            gdl = func_801F51C8(&gDynamicBufs[gBufferIndex], gdl);
            gSPClearGeometryMode(gdl++, G_ZBUFFER);
        }
    }
    if (gGameMode == 0x80000000) {
        if (D_80364A64) {
            D_80364A64--;
        } else if (gYoshiTypeState != 2 && yoshiState == 2) {
            yoshi_demand(0x4000);
            func_80260650(D_80367738, 0x1C, NULL);
        }
    }
    cont_update();
    if (gGameMode == 0x800000000000) {
        academy_play_state_tune();
    }
    if ((gGameMode & 0x88000080) && D_803156C4 % 20 >= 6 && D_80364A71 != -1 && yoshiState == 2) {
        s32 x;
        s32 y;

        x = 0x118, y = 0x8C;
        sprintf(D_8021AB72, "%d", D_80364A71);
        text_print((Gfx * N64P *) &gDynamicBufs[gBufferIndex], (u8 *) D_8021AB72, 0, 1, 0, x, y, 0x1A, 0x16, 1, 0, 0, 0, gYoshiAlpha);
        text_print_gradient((Gfx * N64P *) &gDynamicBufs[gBufferIndex], (u8 *) D_8021AB72, 0, 1, 0, x + 2, y + 2, 0x12, 0x12, 1, 0xFF, 0xFF, 0,
                      gYoshiAlpha, 0xFF, 0, 0, gYoshiAlpha);
    }
    if (gGameMode & 0x40000000000) {
        func_801F803C();
        gdl = func_801F8440((s32) &gDynamicBufs[gBufferIndex], gdl);
    }
    text_flush_screen(&gdl, (Mtx *) &gDynamicBufs[gBufferIndex]);
    gdl = func_80274BF0((s32) &gDynamicBufs[gBufferIndex], gdl);
    if (!(gGameMode & 0xC000000000000)) {
        gdl = yoshi_draw(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    }
    if (yoshiState != 1 && currentYoshiWindow == 0xB) {
        gdl = func_801EAA7C(gdl, (struct PlayerSelDyn *) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    }
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    gFrameDlLen = gdl - gDynamicBufs[gBufferIndex].dl;
    for (i = 0; i < nextdma; i++) {
        osRecvMesg(&textureDmaMessageQ, NULL, 1);
    }
    for (i = 0; i < nextdma - no_palette_dmas; i++) {
        tex_decode_next();
    }
    if ((gGameMode & 0x81D9836583B28000) && yoshiState == 1 && !func_802753F8() && !areWeFading() &&
        gNextGameMode == 0 && gYoshiPendingWindow == -1) {
        switch (gGameMode) {
            case 0x20000:
                gNextGameMode = 0x40000;
                break;
            case 0x200000:
                gNextGameMode = 0x10000;
                break;
            case 0x10000000000:
                gNextGameMode = 0x100000000;
                break;
            case 0x100000:
                gNextGameMode = 0x80000;
                D_8020C070[9].unkC = D_8020C070[playerNumber + 2].unkC;
                D_8020C070[9].unk10 = NULL;
                D_8020C070[9].unk6 = D_8020C070[9].unk8 = 0x14;
                yoshi_demand(0x800C);
                break;
            case 0x4000000000:
                gNextGameMode = 0x8000000000;
                D_8020C070[9].unkC = D_8020C070[D_8021AB74].unkC;
                D_8020C070[9].unk10 = D_8020C070[D_8021AB74].unk10;
                D_8020C070[9].unk6 = D_8020C070[9].unk8 = 0xF;
                yoshi_demand(0x800C);
                break;
            case 0x800000:
                gNextGameMode = 0x400000;
                break;
            case 0x2000000:
                D_80364AE9 = playerNumber;
                playerNumberAtStart = playerNumber;
                if (D_80365060[playerNumber] == 1) {
                    func_80275390(0x4000);
                } else {
                    gNextGameMode = 0x20000;
                }
                break;
            case 0x8000000000000000:
                func_80275390(0x400000000000000);
                break;
            case 0x1000000:
                osSendMesg(&D_80219EF8, (OSMesg) ((playerNumber << 16) | 7), 1);
                osSendMesg(&D_80219EF8, (OSMesg) ((playerNumber << 16) | 0x15 | 0x01000000), 1);
                seq_prepare(4);
                func_80275390(0x100000000000);
                break;
            case 0x80000000:
                gNextGameMode = 0x40000000;
                break;
            case 0x8000:
                if (D_8039C541) {
                    func_80275390(0x20000000000000);
                } else if (D_8021AB70) {
                    gNextGameMode = 0x10000;
                } else {
                    gNextGameMode = 0x10000000;
                }
                break;
            case 0x2000000000:
                if (D_8021AB70) {
                    gNextGameMode = 0x100000000;
                } else {
                    gNextGameMode = 0x80000000000;
                }
                break;
            case 0x40000000000000:
            case 0x100000000:
                func_80275270(0x10, 0.5f);
                break;
            case 0x800000000000:
                func_80275270(0x2000000000000, 0.75f);
                func_80261570(0.0f);
                break;
            case 0x1000000000000:
                func_80275270(2, 0.5f);
                break;
            case 0x8000000000000:
                playerNumber = playerNumberAtStart;
                func_80275270(0x4000, 0.6f);
                break;
            case 0x80000000000000:
                if (playerNumber == D_80364AE9 || playerNumber == playerNumberAtStart) {
                    gNextGameMode = D_8021A830;
                } else {
                    gNextGameMode = 0x100000000000000;
                }
                break;
            case 0x100000000000000:
                if (D_8021AB76 != 0xC) {
                    gNextGameMode = 0x200000000000000;
                } else {
                    func_80275390(D_8021A830);
                }
                break;
            default:
                debug_printf("illegal yoshi wait game mode %d\n", lowest_set_bit(gGameMode));
                break;
        }
    }
}
