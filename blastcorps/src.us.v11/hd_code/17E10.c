#include "common.h"
#include <ultra64.h>

extern u32 D_802E8BEC;
extern s32 D_80358060;
extern u64 D_80364A90;
extern u8 D_80366A18;
void func_8026AF6C(s32);

void func_8025C5D0(void) {
    switch (D_802E8BEC) {
        case 0:
            if (D_80364A90 == 2) {
                if (D_80358060 == 0x96) {
                    func_8026AF6C(0x803E);
                }
                if (D_80358060 == 0x190) {
                    func_8026AF6C(0x8025);
                }
                if (D_80358060 == 0x2BC) {
                    func_8026AF6C(0x8026);
                }
            } else {
                if (D_80358060 == 0x64) {
                    func_8026AF6C(0x8027);
                }
                if (D_80358060 == 0x12C) {
                    func_8026AF6C(0x8028);
                }
                if (D_80358060 == 0x1F4) {
                    func_8026AF6C(0x8029);
                }
                if (D_80358060 == 0x2BC) {
                    func_8026AF6C(0x802A);
                }
            }
            break;
        case 1:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x802B);
            }
            if (D_80358060 == 0x1D6) {
                func_8026AF6C(0x802C);
            }
            break;
        case 2:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x802D);
            }
            break;
        case 3:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x802E);
            }
            break;
        case 4:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x802F);
            }
            break;
        case 5:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x8030);
            }
            break;
        case 6:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x8031);
            }
            if (D_80358060 == 0x1D6) {
                func_8026AF6C(0x8032);
            }
            break;
        case 7:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x8033);
            }
            break;
        case 8:
            if (D_80358060 == 0xB4) {
                func_8026AF6C(0x8034);
            }
            break;
    }
    if (D_80358060 == 0x82) {
        D_80366A18 = 1;
    }
}

extern u8 D_802E8BF0;
extern u64 D_80364A90;
extern u32 D_803156C4;
extern void *D_80366BA4;
extern s32 D_80366BA8;
extern s32 D_80358060;
extern u8 D_80366A10;
extern u8 D_80366A11;
extern s16 D_80366A04;
extern u32 D_80366BBC;
extern Vtx D_802FA8B0[][4];
extern Mtx D_02000000[];
void func_8025E1E0(Gfx **);
Gfx *func_8025D2B4(Gfx *, s32, s32 *);

Gfx *func_8025C878(Gfx *arg0, s32 arg1, u8 arg2, s32 *arg3) {
    void *buf;
    u32 time;
    Gfx *gfx;
    s32 i;

    time = D_803156C4;
    gfx = arg0;
    if (D_802E8BF0 != 0 && (D_80364A90 & 2)) {
        if (arg2) {
            buf = D_80366BA4;
        } else {
            buf = (u8 *) D_80366BA4 + 0x3c0;
        }
        D_80366BA8 = 0;
        if (D_80358060 == 0) {
            D_80366A10 = 0;
            D_80366A11 = 0;
        }
        if ((u32) D_80358060 > D_80366A04) {
            if (time < D_80366BBC + 0x78) {
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
                    D_802FA8B0[arg2][i].v.cn[3] = (time - D_80366BBC) * 2.125;
                }
                func_8025E1E0(&gfx);
            } else {
                gDPPipeSync(gfx++);
                gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
                gDPSetRenderMode(gfx++, 0x0F0A4000, 0);
                gDPSetCycleType(gfx++, G_CYC_FILL);
                gDPSetFillColor(gfx++, 0x00010001);
                gDPFillRectangle(gfx++, 0, 0, 319, 239);
                gDPPipeSync(gfx++);
                gDPSetCycleType(gfx++, G_CYC_1CYCLE);
                func_8025E1E0(&gfx);
            }
        } else {
            if (D_80366A04 == D_80358060) {
                D_80366BBC = time;
            }
            func_8025E1E0(&gfx);
        }
    } else {
        gfx = func_8025D2B4(gfx, arg1, arg3);
    }
    *arg3 = *arg3 + (gfx - arg0);
    return gfx;
}

extern void *D_80358070;
extern void *D_80366BA0;
extern void *D_80366BA4;

void func_8025CE74(void) {
    D_80366BA0 = D_80358070;
    D_80358070 = (u8 *) D_80366BA0 + 0x40;

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

    D_80366BA4 = D_80358070;
    D_80358070 = (u8 *) D_80366BA4 + 0x780;
}

extern u8 D_00487050[];
extern u8 D_00489E70[];
extern u8 D_0048F5A0[];
extern u32 D_80366BB0[];
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);

void func_8025D0B0(u8 arg0) {
    void *rom;
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
    func_8028B4C4(rom, D_80358070, &size, 0xC, 0, 1);
    D_80366BB0[arg0] = (u32) D_80358070 & 0x1FFFFFFF;
    D_80358070 = (u8 *) D_80358070 + size;
}

extern u64 D_80364A98;
extern s16 D_80366A00;
extern s16 D_80366A02;
extern u16 D_80366A12;
extern s16 D_80366A14;
extern s16 D_80366A16;

void func_8025D184(void) {
    if (D_80364A98 & 2) {
        func_8025D0B0(1);
        func_8025D0B0(0);
        D_80366A16 = 0xFF;
        D_80366A12 = 0;
        D_80366A00 = 0xA5;
        D_80366A02 = 0xD;
    } else if (D_80364A98 & 0x40000) {
        D_80366A16 = 0;
        D_80366A12 = 4;
        D_80366A02 = 0x2A;
    } else if (D_80364A98 & 0x10000) {
        D_80366A16 = 0;
        D_80366A12 = 3;
    } else {
        func_8025D0B0(1);
        func_8025D0B0(0);
        D_80366A12 = 3;
        D_80366A14 = 0;
        D_80366A00 = 0x10;
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

extern s16 D_80366A00;
extern s16 D_80366A02;
extern u16 D_80366A12;
extern s16 D_80366A14;
extern s16 D_80366A16;
extern s16 D_8039CAA0;
extern s16 yoshiState;

/* Draws the scrolling title backdrop. Input bits in the u64 D_80364A90 fade
 * D_80366A14 up/down, a little state machine on D_80366A12 (0 -> 1 on
 * D_80358060 == 0x3C, 1 fades D_80366A16 out, 2 scrolls D_80366A00 to 0x10,
 * 4 tracks yoshiState) steps the intro, then: a 5x5 grid of 32x32 tiles from
 * D_80366BB0[1] (states 0/1/4) and, falling through the switch for every state
 * but 4, a strip of tiles from D_80366BB0[0] drawn twice with scissored texture
 * rectangles at a vertical offset derived from D_8039CAA0.
 * The two slti-tested add-and-clamp updates only schedule like the ROM (sh
 * between the sll/sra pair) with the update and the test on separate source
 * lines; the line break changes as1's schedule. */
Gfx *func_8025D2B4(Gfx *arg0, s32 arg1, s32 *arg2) {
    Gfx *gfx;
    s32 i;
    s32 j;
    s32 step;
    s16 top;

    gfx = arg0;
    if (D_80364A90 & 0x08040E2110418002LL) {
        D_80366A14 += 10;
        if (D_80366A14 > 0xFF) {
            D_80366A14 = 0xFF;
        }
    } else if (D_80364A90 & 0x0188004203160000LL) {
        if ((D_80366A14 -= 10) <= 0) {
            D_80366A14 = 0;
        }
    }

    switch (D_80366A12) {
        case 0:
            if (D_80358060 == 0x3C) {
                D_80366A12 = 1;
            }
            break;
        case 1:
            if ((D_80366A16 -= 8) < 0) {
                D_80366A16 = 0;
                D_80366A12 = 2;
            }
            break;
        case 2:
            D_80366A00 -= 10;
            if (D_80366A00 <= 0x10) {
                D_80366A00 = 0x10;
                D_80366A12 = 3;
            }
            break;
        case 4:
            if (yoshiState == 2) {
                if (D_80366A16 + 4 > 0x80) {
                    D_80366A16 = 0x80;
                } else {
                    D_80366A16 += 4;
                }
            } else if (D_80364A90 == 0x200000) {
                if (D_80366A16 - 6 < 0) {
                    D_80366A16 = 0;
                } else {
                    D_80366A16 -= 6;
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

    if (D_80364A90 == 2 && yoshiState != 1) {
        top = D_80366A00 - (0xFF - D_8039CAA0) / 4;
        if (top + 0x3F == D_80366A00) {
            top = -100;
        }
    } else {
        top = D_80366A00;
    }

    switch (D_80366A12) {
        case 0:
        case 1:
        case 4:
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_80366A16);
            step = 0x52;
            for (i = 0; i < 0x96; i += 0x1F) {
                for (j = 0; j < 0x96; j += 0x1F) {
                    gDPLoadTextureTile(gfx++, D_80366BB0[1], G_IM_FMT_RGBA, G_IM_SIZ_16b, 156, 0, i, j,
                                       i + 31, j + 31, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, G_TX_NOLOD,
                                       G_TX_NOLOD);
                    gSPTextureRectangleOld(gfx++, (i + step) << 2, (j + D_80366A02) << 2,
                                           (i + step + 0x1F) << 2, (j + D_80366A02 + 0x1F) << 2, 0,
                                           i << 5, j << 5, 1 << 10, 1 << 10);
                }
            }
            if (D_80366A12 == 4) {
                break;
            }
        default:
            gDPPipeSync(gfx++);
            gDPSetCombine(gfx++, 0xFF97FF, 0xFF2DFEFF);
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_80366A14 / 2);
            step = 0x14;
            for (i = 0; i < 0x110; i += 0x1F) {
                gDPLoadTextureTile(gfx++, D_80366BB0[0], G_IM_FMT_RGBA, G_IM_SIZ_16b, 280, 0, i, 0,
                                   i + 31, 63, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
                gDPPipeSync(gfx++);
                gDPSetRenderMode(gfx++, 0x00504240, 0);
                gDPSetCombine(gfx++, 0xFF97FF, 0xFF2DFEFF);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_80366A14 / 2);
                gSPScisTextureRectangleOld(gfx++, (i + step) << 2, (top + 6) << 2,
                                           (i + step + 0x1F) << 2, (top + 0x45) << 2, 0, i << 5, 0,
                                           1 << 10, 1 << 10);
                gDPPipeSync(gfx++);
                if (D_80366A14 == 0xFF && (D_80364A90 & 0xC9FD0FE79BFF80B0LL)) {
                    gDPSetRenderMode(gfx++, 0x0F0A7008, 0);
                }
                gDPSetCombine(gfx++, 0xFF97FF, 0xFF2CFE7F);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, D_80366A14);
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

extern Mtx D_02000000[];
void func_8024FC2C(Gfx **, s32);

void func_8025E1E0(Gfx **arg0) {
    Gfx *gfx = *arg0;

    gSPMatrix(gfx++, &D_02000000[2], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gfx++, &D_02000000[5], G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gfx++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    func_8024FC2C(&gfx, 0);
    func_8024FC2C(&gfx, 1);
    func_8024FC2C(&gfx, 2);
    *arg0 = gfx;
}

extern u8 D_803643D7;
extern u8 D_80366BC0;
extern u16 D_80366BC2;
extern u8 D_80366BC4;
extern s32 D_80364AA8;
extern s32 D_802E8BDC;
extern u8 D_802E8F94[];
extern u8 D_80364AE8;
extern u8 D_80364AF0[][256];
extern u32 D_80366BB8;
extern s16 currentYoshiWindow;
extern s16 D_8036BB1A;
extern s16 yoshiState;
extern void *D_80367734;
extern void *D_80367738;
extern u32 D_80367740;
extern u32 D_802E8CD0[];
s32 func_802753C0(void);
void func_802C1DD0(s32);
void func_802609F0(void);
void func_80260A10(void);
void func_80260E2C(void);
void func_80260DFC(void);
void func_80260EE0(s32);
void func_8026AF6C(s32);
s32 func_8026205C(s32);
void *func_80260650(void *, s16, void *);
void func_80278318(void);
void func_80277EDC(s32, s32, s32, s32);
s32 func_802D4E10(void *);
void func_80275270(u64, f32);

void func_8025E2CC(Gfx **arg0, s32 arg1, s32 arg2) {
    Gfx *gfx;

    gfx = *arg0;
    if (D_803643D7 != 0 && func_802753C0() == 0) {
        if (D_80366BC0 == 0) {
            func_802C1DD0(D_802E8F94[D_802E8BDC * 68] == 0x20 || D_802E8F94[D_802E8BDC * 68] == 0x80);
            D_80366BB8 = 0;
            if ((D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] > 0 &&
                 D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] < 6) ? 1 : 0) {
                func_802609F0();
                func_80260A10();
                D_80366BC2 = 5;
                D_80366BC4 = 0;
                if (D_80364AA8 != 1) {
                    func_80260E2C();
                    D_80366BC4 = 1;
                }
            } else {
                switch (D_802E8BDC) {
                    case 0x28:
                        func_80260A10();
                        func_80260DFC();
                        D_80366BC2 = 0x1E;
                        D_80366BC4 = 0;
                        break;
                    case 0x32:
                        func_80260A10();
                        func_80260EE0(0x25);
                        D_80366BC2 = 0x23;
                        D_80366BC4 = 0;
                        break;
                    default:
                        func_80260A10();
                        D_80366BC2 = 5;
                        func_80260E2C();
                        D_80366BC4 = 1;
                        break;
                }
            }
            func_8026AF6C(D_80366BC2 | 0x8000 | 0x2000);
            D_8036BB1A = -1;
            if (D_80366BC2 == 5) {
                func_80260650(D_80367738, func_8026205C(2), NULL);
            }
        }
        D_80366BC0 = D_803643D7;
        if (D_80366BB8 == 0) {
            if (yoshiState == 8 && currentYoshiWindow == D_80366BC2) {
                D_80366BB8 = D_803156C4;
                func_80278318();
                func_80277EDC(2, 1, 2, func_8026205C(3));
            }
        } else if (func_802D4E10(D_80367734) == 0 ||
                   (D_80366BC4 != 0 && D_803156C4 - D_80367740 >= 0x1E1) ||
                   (D_80366BC4 == 0 && D_803156C4 - D_80366BB8 > D_802E8CD0[(D_80364AA8 & 0x81) ? 1 : 0])) {
            func_80275270(0x08000000, 0.75f);
            D_80366BB8 = 0;
            D_80366BC0 = 0;
        }
    }
    *arg0 = gfx;
}

extern u8 D_803643D6;
extern u8 D_803643D8;
extern s32 D_802E8BDC;
extern u8 D_802E8F94[];
extern void *D_80367738;
extern s16 currentYoshiWindow;
extern s16 D_8036BB1A;
extern s16 yoshiState;
extern u8 D_802E8BD8;
extern u32 D_80366BB8;
extern u8 D_80366BC5;
extern u8 D_80364AE8;
extern u8 D_80364AF0[][256];
extern u64 D_80364A98;
void func_802609D0(void);
void func_802C1DD0(s32);
void *func_80260650(void *, s16, void *);
void func_80261570(f32);
void func_8026AF6C(s32);
s32 func_8026B10C(void);
s32 func_802753C0(void);
void func_80275390(u64);

void func_8025E67C(Gfx **arg0, s32 arg1, u8 arg2) {
    Gfx *gfx;
    u32 time;
    u32 i;
    u32 j;
    s32 pad;
    u32 alpha;

    gfx = *arg0;
    time = D_803156C4;
    if (D_803643D6 != 0) {
        if (D_803643D8 == 0) {
            func_802609D0();
            func_802C1DD0(D_802E8F94[D_802E8BDC * 68] == 0x20 || D_802E8F94[D_802E8BDC * 68] == 0x80);
            switch (D_802E8BDC) {
                case 0x31:
                    func_80260650(D_80367738, 0x31, NULL);
                    func_80261570(0.0f);
                    break;
                case 0x32:
                    D_8036BB1A = -1;
                    func_8026AF6C(0xA00E);
                    func_80261570(0.0f);
                    break;
                default:
                    func_80260650(D_80367738, 0x31, NULL);
                    D_802E8BD8 = 1;
                    if (currentYoshiWindow != -1 || func_8026B10C() != 0) {
                        func_8026AF6C(0x4000);
                    }
                    D_8036BB1A = -1;
                    func_80261570(0.0f);
                    break;
            }
            D_80366BB8 = time;
            D_80366BC5 = 0;
        }
        i = time - D_80366BB8;
        if (i >= 0xB4) {
            switch (D_802E8BDC) {
                case 0x31:
                    if (D_80366BC5 == 0) {
                        func_80260650(D_80367738, 0x32, NULL);
                        D_80366BC5 = 1;
                    }
                    break;
                case 0x32:
                    if (yoshiState == 1 && func_802753C0() == 0) {
                        if ((D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] > 0 &&
                             D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] < 6) ? 1 : 0) {
                            func_80275390(0x08000000);
                        } else {
                            func_80275390(0x40);
                        }
                    }
                    break;
                default:
                    if (D_80366BC5 == 0) {
                        func_80260650(D_80367738, 0x32, NULL);
                        D_80366BC5 = 1;
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
                    if (func_802753C0() == 0) {
                        if (time - D_80366BB8 - 0xB4 < 0x5A) {
                            alpha = (time - D_80366BB8 - 0xB4) * 2.8333333333333335;
                            for (i = 0; i < 4; i++) {
                                for (j = 0; j < 4; j++) {
                                    D_802FA8B0[arg2][i].v.cn[j] = alpha;
                                }
                            }
                        } else if (time - D_80366BB8 - 0x10E >= 0x2E) {
                            if (D_80364A90 == 0x100000000000LL) {
                                D_80364A98 = 0x200000000000LL;
                            } else if ((D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] > 0 &&
                                        D_80364AF0[D_80364AE8][D_802E8BDC + 0x18] < 6) ? 1 : 0) {
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
