#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gLevelInfo ((u8 *) gLevelInfo)
#define players ((u8 (*)[256]) players)
/* end of views */


/* The attract demo's windows at fixed frames (0x8025C5D0). */
void hud_attract_messages(void) {
    switch (((u32) D_802E8BEC)) {
        case 0:
            if (gGameMode == 2) {
                if (((s32) gFramesInMode) == 0x96) {
                    yoshi_demand(0x803E);
                }
                if (((s32) gFramesInMode) == 0x190) {
                    yoshi_demand(0x8025);
                }
                if (((s32) gFramesInMode) == 0x2BC) {
                    yoshi_demand(0x8026);
                }
            } else {
                if (((s32) gFramesInMode) == 0x64) {
                    yoshi_demand(0x8027);
                }
                if (((s32) gFramesInMode) == 0x12C) {
                    yoshi_demand(0x8028);
                }
                if (((s32) gFramesInMode) == 0x1F4) {
                    yoshi_demand(0x8029);
                }
                if (((s32) gFramesInMode) == 0x2BC) {
                    yoshi_demand(0x802A);
                }
            }
            break;
        case 1:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x802B);
            }
            if (((s32) gFramesInMode) == 0x1D6) {
                yoshi_demand(0x802C);
            }
            break;
        case 2:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x802D);
            }
            break;
        case 3:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x802E);
            }
            break;
        case 4:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x802F);
            }
            break;
        case 5:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x8030);
            }
            break;
        case 6:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x8031);
            }
            if (((s32) gFramesInMode) == 0x1D6) {
                yoshi_demand(0x8032);
            }
            break;
        case 7:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x8033);
            }
            break;
        case 8:
            if (((s32) gFramesInMode) == 0xB4) {
                yoshi_demand(0x8034);
            }
            break;
    }
    if (((s32) gFramesInMode) == 0x82) {
        D_80366A18 = 1;
    }
}

extern void * N64P D_80366BA4;
extern s32 D_80366BA8;
extern u8 D_80366A10;
extern u8 D_80366A11;
extern u32 gAttractFadeTime;
void title_draw_vehicles(Gfx **);
Gfx *title_draw_logo(Gfx *, s32, s32 *);

/* Attract mode: the fade at a demo's end, else the logo (0x8025C878). */
Gfx *title_draw_attract(Gfx *arg0, s32 arg1, u8 arg2, s32 *arg3) {
    void *buf;
    u32 time;
    Gfx *gfx;
    s32 i;

    time = D_803156C4;
    gfx = arg0;
    if (D_802E8BF0 != 0 && (gGameMode & 2)) {
        if (arg2) {
            buf = D_80366BA4;
        } else {
            buf = (u8 *) D_80366BA4 + 0x3c0;
        }
        D_80366BA8 = 0;
        if (((s32) gFramesInMode) == 0) {
            D_80366A10 = 0;
            D_80366A11 = 0;
        }
        if ((u32) ((s32) gFramesInMode) > D_80366A04) {
            if (time < gAttractFadeTime + 0x78) {
                gDPPipeSync(gfx++);
                gSPMatrix(gfx++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
                gSPMatrix(gfx++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
                gDPSetRenderMode(gfx++, 0x00504340, 0);
                gDPSetCombineMode(gfx++, G_CC_SHADE, G_CC_SHADE);
                gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH);
                gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
                gSPVertex(gfx++, (u32) D_802FA8B0[arg2] - 0x80000000, 4, 0);
                gSP1Triangle(gfx++, 0, 1, 2, 0);
                gSP1Triangle(gfx++, 0, 2, 3, 0);
                for (i = 0; i < 4; i++) {
                    D_802FA8B0[arg2][i].v.cn[0] = 0;
                    D_802FA8B0[arg2][i].v.cn[1] = 0;
                    D_802FA8B0[arg2][i].v.cn[2] = 0;
                    D_802FA8B0[arg2][i].v.cn[3] = (time - gAttractFadeTime) * 2.125;
                }
                title_draw_vehicles(&gfx);
            } else {
                gDPPipeSync(gfx++);
                gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
                gDPSetRenderMode(gfx++, 0x0F0A4000, 0);
                gDPSetCycleType(gfx++, G_CYC_FILL);
                gDPSetFillColor(gfx++, 0x00010001);
                gDPFillRectangle(gfx++, 0, 0, 319, 239);
                gDPPipeSync(gfx++);
                gDPSetCycleType(gfx++, G_CYC_1CYCLE);
                title_draw_vehicles(&gfx);
            }
        } else {
            if (D_80366A04 == ((s32) gFramesInMode)) {
                gAttractFadeTime = time;
            }
            title_draw_vehicles(&gfx);
        }
    } else {
        gfx = title_draw_logo(gfx, arg1, arg3);
    }
    *arg3 = *arg3 + (gfx - arg0);
    return gfx;
}

extern void * N64P D_80366BA0;
extern void * N64P D_80366BA4;

void func_8025CE74(void) {
    D_80366BA0 = gHeapPtr;
    gHeapPtr = (u8 *) D_80366BA0 + 0x40;

    *(u16 *) ((u8 *) D_80366BA0 + 0x0) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x2) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x4) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x6) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x8) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0xa) = 0x9e0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0xc) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0xd) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0xe) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0xf) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x10) = 0x14;
    *(u16 *) ((u8 *) D_80366BA0 + 0x12) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x14) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x16) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x18) = 0x280;
    *(u16 *) ((u8 *) D_80366BA0 + 0x1a) = 0x9e0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x1c) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x1d) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x1e) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x1f) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x20) = 0x14;
    *(u16 *) ((u8 *) D_80366BA0 + 0x22) = 0x4f;
    *(u16 *) ((u8 *) D_80366BA0 + 0x24) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x26) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x28) = 0x280;
    *(u16 *) ((u8 *) D_80366BA0 + 0x2a) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x2c) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x2d) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x2e) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x2f) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x30) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x32) = 0x4f;
    *(u16 *) ((u8 *) D_80366BA0 + 0x34) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x36) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x38) = 0;
    *(u16 *) ((u8 *) D_80366BA0 + 0x3a) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x3c) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x3d) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x3e) = 0;
    *(u8 *)  ((u8 *) D_80366BA0 + 0x3f) = 0;

    D_80366BA4 = gHeapPtr;
    gHeapPtr = (u8 *) D_80366BA4 + 0x780;
}

extern u8 D_00487050[];
extern u8 D_00489E70[];
extern u32 gLogoImages[];

/* 0: the 280x64 logo strip, 1: the 156x156 picture (0x8025D0B0). */
void title_load_logo_image(u8 arg0) {
#ifdef NON_MATCHING
    void *rom = NULL; /* arg0 is 0 or 1; any other value reads an indeterminate slot in the original */
#else
    void *rom;
#endif
    s32 size;

    switch (arg0) {
        case 1:
            rom = D_00489E70;
            size = D_0048F5A0 - D_00489E70;
            break;
        case 0:
            rom = D_00487050;
            size = D_00489E70 - D_00487050;
            break;
    }
    func_8028B4C4((u32) rom, (u32) gHeapPtr, (u32 *) &size, 0xC, 0, 1);
    gLogoImages[arg0] = (u32) gHeapPtr & 0x1FFFFFFF;
    gHeapPtr = (u8 *) gHeapPtr + size;
}

extern s16 gLogoY;
extern s16 gLogoPicY;
extern s16 gLogoAlpha;
extern s16 gLogoPicAlpha;

/* The logo state, positions and images for the next mode (0x8025D184). */
void title_setup_logo(void) {
    if (gNextGameMode & 2) {
        title_load_logo_image(1);
        title_load_logo_image(0);
        gLogoPicAlpha = 0xFF;
        gLogoState = 0;
        gLogoY = 0xA5;
        gLogoPicY = 0xD;
    } else if (gNextGameMode & 0x40000) {
        gLogoPicAlpha = 0;
        gLogoState = 4;
        gLogoPicY = 0x2A;
    } else if (gNextGameMode & 0x10000) {
        gLogoPicAlpha = 0;
        gLogoState = 3;
    } else {
        title_load_logo_image(1);
        title_load_logo_image(0);
        gLogoState = 3;
        gLogoAlpha = 0;
        gLogoY = 0x10;
    }
}

/* Pre-2.0I libultra forms of the texture-rectangle macros: the RDP-half
 * commands are G_RDPHALF_2/G_RDPHALF_CONT (0xB3/0xB2), and the scissored
 * variant has no (s16) casts or xl/yl < 0 guard. */
#define OLD_RDPHALF_2 0xB3
#define OLD_RDPHALF_CONT 0xB2

#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                \
    {                                                                                        \
        Gfx *_g = (Gfx *) (pkt);                                                             \
                                                                                             \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));    \
        gImmp1(pkt, OLD_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                \
        gImmp1(pkt, OLD_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));       \
    }

#define gSPScisTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)             \
    {                                                                                        \
        Gfx *_g = (Gfx *) (pkt);                                                             \
                                                                                             \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(MAX(xh, 0), 12, 12) |            \
                        _SHIFTL(MAX(yh, 0), 0, 12));                                         \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(MAX(xl, 0), 12, 12) |                 \
                        _SHIFTL(MAX(yl, 0), 0, 12));                                         \
        gImmp1(pkt, OLD_RDPHALF_2,                                                           \
               (_SHIFTL(((s) - MIN(((xl) * (dsdx)) >> 7, 0)), 16, 16) |                      \
                _SHIFTL(((t) - MIN(((yl) * (dtdy)) >> 7, 0)), 0, 16)));                      \
        gImmp1(pkt, OLD_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));       \
    }

extern s16 gLogoY;
extern s16 gLogoPicY;
extern s16 gLogoAlpha;
extern s16 gLogoPicAlpha;

/* The logo state machine and its texture rectangles (0x8025D2B4).
 * Draws the scrolling title backdrop. Input bits in the u64 gGameMode fade
 * gLogoAlpha up/down, a little state machine on gLogoState (0 -> 1 on
 * gFramesInMode == 0x3C, 1 fades gLogoPicAlpha out, 2 scrolls gLogoY to 0x10,
 * 4 tracks yoshiState) steps the intro, then: a 5x5 grid of 32x32 tiles from
 * gLogoImages[1] (states 0/1/4) and, falling through the switch for every state
 * but 4, a strip of tiles from gLogoImages[0] drawn twice with scissored texture
 * rectangles at a vertical offset derived from gPadDisplayAlpha.
 * The two slti-tested add-and-clamp updates only schedule like the ROM (sh
 * between the sll/sra pair) with the update and the test on separate source
 * lines; the line break changes as1's schedule. */
Gfx *title_draw_logo(Gfx *arg0, s32 arg1, s32 *arg2) {
    Gfx *gfx;
    s32 i;
    s32 j;
    s32 step;
    s16 top;

    gfx = arg0;
    if (gGameMode & 0x08040E2110418002LL) {
        gLogoAlpha += 10;
        if (gLogoAlpha > 0xFF) {
            gLogoAlpha = 0xFF;
        }
    } else if (gGameMode & 0x0188004203160000LL) {
        if ((gLogoAlpha -= 10) <= 0) {
            gLogoAlpha = 0;
        }
    }

    switch (gLogoState) {
        case 0:
            if (((s32) gFramesInMode) == 0x3C) {
                gLogoState = 1;
            }
            break;
        case 1:
            if ((gLogoPicAlpha -= 8) < 0) {
                gLogoPicAlpha = 0;
                gLogoState = 2;
            }
            break;
        case 2:
            gLogoY -= 10;
            if (gLogoY <= 0x10) {
                gLogoY = 0x10;
                gLogoState = 3;
            }
            break;
        case 4:
            if (yoshiState == 2) {
                if (gLogoPicAlpha + 4 > 0x80) {
                    gLogoPicAlpha = 0x80;
                } else {
                    gLogoPicAlpha += 4;
                }
            } else if (gGameMode == 0x200000) {
                if (gLogoPicAlpha - 6 < 0) {
                    gLogoPicAlpha = 0;
                } else {
                    gLogoPicAlpha -= 6;
                }
            }
            break;
    }

    gDPPipeSync(gfx++);
    gSPTexture(gfx++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetTexturePersp(gfx++, G_TP_NONE);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetRenderMode(gfx++, 0x00504240, 0);
    gDPSetCombine(gfx++, 0xFF97FF, 0xFF2CFE7F);

    if (gGameMode == 2 && yoshiState != 1) {
        top = gLogoY - (0xFF - gPadDisplayAlpha) / 4;
        if (top + 0x3F == gLogoY) {
            top = -100;
        }
    } else {
        top = gLogoY;
    }

    switch (gLogoState) {
        case 0:
        case 1:
        case 4:
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, gLogoPicAlpha);
            step = 0x52;
            for (i = 0; i < 0x96; i += 0x1F) {
                for (j = 0; j < 0x96; j += 0x1F) {
                    gDPLoadTextureTile(gfx++, gLogoImages[1], G_IM_FMT_RGBA, G_IM_SIZ_16b, 156, 0, i, j,
                                       i + 31, j + 31, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, G_TX_NOLOD,
                                       G_TX_NOLOD);
                    gSPTextureRectangleOld(gfx++, (i + step) << 2, (j + gLogoPicY) << 2,
                                           (i + step + 0x1F) << 2, (j + gLogoPicY + 0x1F) << 2, 0,
                                           i << 5, j << 5, 1 << 10, 1 << 10);
                }
            }
            if (gLogoState == 4) {
                break;
            }
        default:
            gDPPipeSync(gfx++);
            gDPSetCombine(gfx++, 0xFF97FF, 0xFF2DFEFF);
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, gLogoAlpha / 2);
            step = 0x14;
            for (i = 0; i < 0x110; i += 0x1F) {
                gDPLoadTextureTile(gfx++, gLogoImages[0], G_IM_FMT_RGBA, G_IM_SIZ_16b, 280, 0, i, 0,
                                   i + 31, 63, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
                gDPPipeSync(gfx++);
                gDPSetRenderMode(gfx++, 0x00504240, 0);
                gDPSetCombine(gfx++, 0xFF97FF, 0xFF2DFEFF);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, gLogoAlpha / 2);
                gSPScisTextureRectangleOld(gfx++, (i + step) << 2, (top + 6) << 2,
                                           (i + step + 0x1F) << 2, (top + 0x45) << 2, 0, i << 5, 0,
                                           1 << 10, 1 << 10);
                gDPPipeSync(gfx++);
                if (gLogoAlpha == 0xFF && (gGameMode & 0xC9FD0FE79BFF80B0LL)) {
                    gDPSetRenderMode(gfx++, 0x0F0A7008, 0);
                }
                gDPSetCombine(gfx++, 0xFF97FF, 0xFF2CFE7F);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, gLogoAlpha);
                gSPScisTextureRectangleOld(gfx++, (i + step) << 2, top << 2, (i + step + 0x1F) << 2,
                                           (top + 0x3F) << 2, 0, i << 5, 0, 1 << 10, 1 << 10);
            }
            break;
    }

    gDPPipeSync(gfx++);
    gDPSetTexturePersp(gfx++, G_TP_PERSP);
    *arg2 = *arg2 + (gfx - arg0);
    return gfx;
}


/* The 3D matrices, then the three vehicle layers (0x8025E1E0). */
void title_draw_vehicles(Gfx **arg0) {
    Gfx *gfx = *arg0;

    gSPMatrix(gfx++, &D_02000000[2], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gfx++, &D_02000000[5], G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gfx++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    veh_draw_layer(&gfx, 0);
    veh_draw_layer(&gfx, 1);
    veh_draw_layer(&gfx, 2);
    *arg0 = gfx;
}

extern u8 gLevelDoneSeen;
extern u16 gLevelDoneWindow;
extern u8 D_80366BC4;
extern u32 gEndSeqTime;
extern u32 gLevelDoneDelay[];

/* Level complete: status, window and music, fade out (0x8025E2CC). */
void hud_level_complete(Gfx **arg0, s32 arg1, s32 arg2) {
    Gfx *gfx;

    gfx = *arg0;
    if (gLevelDone != 0 && areWeFading() == 0) {
        if (gLevelDoneSeen == 0) {
            obj_tally(gLevelInfo[levelno * 68] == 0x20 || gLevelInfo[levelno * 68] == 0x80);
            gEndSeqTime = 0;
            if ((players[playerNumber][levelno + 0x18] > 0 &&
                 players[playerNumber][levelno + 0x18] < 6) ? 1 : 0) {
                func_802609F0();
                func_80260A10();
                gLevelDoneWindow = 5;
                D_80366BC4 = 0;
                if (((s32) gLevelKind) != 1) {
                    func_80260E2C();
                    D_80366BC4 = 1;
                }
            } else {
                switch (levelno) {
                    case 0x28:
                        func_80260A10();
                        func_80260DFC();
                        gLevelDoneWindow = 0x1E;
                        D_80366BC4 = 0;
                        break;
                    case 0x32:
                        func_80260A10();
                        func_80260EE0(0x25);
                        gLevelDoneWindow = 0x23;
                        D_80366BC4 = 0;
                        break;
                    default:
                        func_80260A10();
                        gLevelDoneWindow = 5;
                        func_80260E2C();
                        D_80366BC4 = 1;
                        break;
                }
            }
            yoshi_demand(gLevelDoneWindow | 0x8000 | 0x2000);
            gYoshiPendingWindow = -1;
            if (gLevelDoneWindow == 5) {
                func_80260650(D_80367738, func_8026205C(2), NULL);
            }
        }
        gLevelDoneSeen = gLevelDone;
        if (gEndSeqTime == 0) {
            if (yoshiState == 8 && currentYoshiWindow == gLevelDoneWindow) {
                gEndSeqTime = D_803156C4;
                hud_panel_stop();
                hud_panel_start(2, 1, 2, func_8026205C(3));
            }
        } else if (func_802D4E10(D_80367734) == 0 ||
                   (D_80366BC4 != 0 && D_803156C4 - D_80367740 >= 0x1E1) ||
                   (D_80366BC4 == 0 && D_803156C4 - gEndSeqTime > gLevelDoneDelay[(((s32) gLevelKind) & 0x81) ? 1 : 0])) {
            func_80275270(0x08000000, 0.75f);
            gEndSeqTime = 0;
            gLevelDoneSeen = 0;
        }
    }
    *arg0 = gfx;
}

extern u32 gEndSeqTime;
extern u8 gFailSoundPlayed;

/* Mission failed (gCarrierHit): sound, window, fade, next mode (0x8025E67C). */
void hud_mission_failed(Gfx **arg0, s32 arg1, u8 arg2) {
    Gfx *gfx;
    u32 time;
    u32 i;
    u32 j;
    s32 pad;
    u32 alpha;

    gfx = *arg0;
    time = D_803156C4;
    if (gCarrierHit != 0) {
        if (D_803643D8 == 0) {
            func_802609D0();
            obj_tally(gLevelInfo[levelno * 68] == 0x20 || gLevelInfo[levelno * 68] == 0x80);
            switch (levelno) {
                case 0x31:
                    func_80260650(D_80367738, 0x31, NULL);
                    func_80261570(0.0f);
                    break;
                case 0x32:
                    gYoshiPendingWindow = -1;
                    yoshi_demand(0xA00E);
                    func_80261570(0.0f);
                    break;
                default:
                    func_80260650(D_80367738, 0x31, NULL);
                    gPauseRequest = 1;
                    if (currentYoshiWindow != -1 || yoshi_get_demand() != 0) {
                        yoshi_demand(0x4000);
                    }
                    gYoshiPendingWindow = -1;
                    func_80261570(0.0f);
                    break;
            }
            gEndSeqTime = time;
            gFailSoundPlayed = 0;
        }
        i = time - gEndSeqTime;
        if (i >= 0xB4) {
            switch (levelno) {
                case 0x31:
                    if (gFailSoundPlayed == 0) {
                        func_80260650(D_80367738, 0x32, NULL);
                        gFailSoundPlayed = 1;
                    }
                    break;
                case 0x32:
                    if (yoshiState == 1 && areWeFading() == 0) {
                        if ((players[playerNumber][levelno + 0x18] > 0 &&
                             players[playerNumber][levelno + 0x18] < 6) ? 1 : 0) {
                            func_80275390(0x08000000);
                        } else {
                            func_80275390(0x40);
                        }
                    }
                    break;
                default:
                    if (gFailSoundPlayed == 0) {
                        func_80260650(D_80367738, 0x32, NULL);
                        gFailSoundPlayed = 1;
                    }
                    gSPMatrix(gfx++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
                    gSPMatrix(gfx++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
                    gDPPipeSync(gfx++);
                    gDPSetRenderMode(gfx++, 0x00504340, 0);
                    gDPSetCombineMode(gfx++, G_CC_SHADE, G_CC_SHADE);
                    gSPSetGeometryMode(gfx++, G_SHADE | G_SHADING_SMOOTH);
                    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
                    gSPVertex(gfx++, (u32) D_802FA8B0[arg2] - 0x80000000, 4, 0);
                    gSP1Triangle(gfx++, 0, 1, 2, 0);
                    gSP1Triangle(gfx++, 0, 2, 3, 0);
                    gDPPipeSync(gfx++);
                    if (areWeFading() == 0) {
                        if (time - gEndSeqTime - 0xB4 < 0x5A) {
                            alpha = (time - gEndSeqTime - 0xB4) * 2.8333333333333335;
                            for (i = 0; i < 4; i++) {
                                for (j = 0; j < 4; j++) {
                                    D_802FA8B0[arg2][i].v.cn[j] = alpha;
                                }
                            }
                        } else if (time - gEndSeqTime - 0x10E >= 0x2E) {
                            if (gGameMode == 0x100000000000LL) {
                                gNextGameMode = 0x200000000000LL;
                            } else if ((players[playerNumber][levelno + 0x18] > 0 &&
                                        players[playerNumber][levelno + 0x18] < 6) ? 1 : 0) {
                                func_80275390(0x08000000);
                            } else {
                                func_80275390(0x40);
                            }
                        } else {
                            for (i = 0; i < 4; i++) {
                                for (j = 0; j < 4; j++) {
                                    D_802FA8B0[arg2][i].v.cn[j] = 0xFF;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    *arg0 = gfx;
}
