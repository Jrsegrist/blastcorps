#include "common.h"
#include <ultra64.h>

/* player.c (per its assert strings): save slots, player ranks and the
 * player select / Controller Pak screens */

/* One entry per level in D_802E8F94 (0x44 bytes), as in hd_code 1D990.c */
typedef struct {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 pad1[0x2D];
    /* 0x2E */ u16 times[5]; /* rank times; [4] is the time limit */
    /* 0x38 */ u8 pad38[0xC];
} LevelInfo;

/* One per save slot (D_80364AF0, 0x100 bytes), as in hd_code 26570.c */
typedef struct {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ u8 rank[0x3C]; /* per level: 1-5 when done */
    /* 0x54 */ u8 pad54[0x91 - 0x54];
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 pad92[0x100 - 0x92];
} Player;

/* Menu entries (D_8020C070, 0x1C bytes), as in hd_code 00000.c */
typedef struct {
    /* 0x00 */ u16 unk0; /* flags */
    /* 0x02 */ s16 unk2; /* x */
    /* 0x04 */ u8 pad4[2];
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ char *unkC; /* title */
    /* 0x10 */ void *unk10; /* glyph list */
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 pad1A[2];
} MenuEntry;

/* Per-player matrices for the player select screen (D_803156F8, 0x21498 bytes) */
typedef struct {
    /* 0x000 */ u8 pad0[0x80];
    /* 0x080 */ Mtx persp;
    /* 0x0C0 */ u8 padC0[0xC0];
    /* 0x180 */ Mtx lookAt;
    /* 0x1C0 */ u8 pad1C0[0x400];
    /* 0x5C0 */ Mtx trans;
} PlayerSelDyn;

extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern LevelInfo D_802E8F94[];
extern s32 D_802E8BDC;
extern u64 D_80364A90;
extern u64 D_80364A98;
extern u32 D_80364AA8;
extern u8 D_80364A87;
extern u8 D_803643D5;
extern MenuEntry D_8020C070[];
extern OSMesgQueue D_80219EF8;

extern Vtx D_80208380[];
extern Gfx D_80208400[];
extern Lights2 D_80208448;
extern u16 D_8021591C;
extern s16 D_8021593C;
extern s32 D_80215458;
extern u8 D_80215470[];
extern u8 D_80215915;
extern u8 D_80215916;
extern s16 D_802154B2;
extern s16 D_802154B4;
extern s16 D_802154B6;
extern s16 D_802154B8;
extern s16 D_802154BA;
extern u8 D_802154BC;
extern s16 D_802154BE;
extern s16 D_802154C0;
extern s32 D_802154C4;
extern s32 D_802154C8;
extern s32 D_802154CC;
extern u8 D_802154D0;
extern s16 D_80215918;
extern s16 D_8021591A;
extern s32 D_80215920;
extern u8 D_80215924;
extern char *D_80215928;
extern s16 D_8021592C;

extern s16 D_802154D2;
extern s32 D_802154DC;
extern u8 D_80215900[];
extern u8 D_80215902[];
extern s32 D_80215908[];
extern s16 D_80215910[];
extern u8 D_80215914;
extern u16 D_80215930[];
extern u16 D_802158A8[];
extern u16 D_802E8C94[];
extern u16 D_802E8C98[];
extern u8 D_8021592E;
extern s32 D_802154D8;
extern f32 D_802154E4;
extern s32 D_802154E8;
extern s32 D_802154F0[];
extern u16 D_802082D8[];
extern u16 D_802082E4[];
extern u16 D_802082E8[];

u8 func_80272C5C(u16 *ids, u16 *palIds, s32 count, s32 frames, s32 flags, f32 scale);
Gfx *func_80274868(Gfx *);
Gfx *func_80274AA4(Gfx *);
Gfx *func_80272ED8(Gfx *, s32, s32, s32, s32, s32, f32);
void func_80264A34(u8 *buf, u16 t, s32 arg2);
u8 *func_8025B558(u16 *);
void func_80259CCC(void *gfxp, u8 *str, u16 *wstr, s32 align, s32 fit, s32 x, s32 y, s32 w, s32 h, s32 forward, s32 r,
                   s32 g, s32 b, s32 a);
void func_80259DC8(void *gfxp, u8 *str, u16 *wstr, s32 align, s32 fit, s32 x, s32 y, s32 w, s32 h, s32 forward,
                   s32 r0, s32 g0, s32 b0, s32 a0, s32 r1, s32 g1, s32 b1, s32 a1);
void func_801FE018(s32);
void func_801F8354(u8);
void func_801E8EB8(u8, s32);
u16 func_801E9528(void);
Gfx *func_801EC49C(Gfx *, s32, s32, s32);

/* Highest rank shown: 4 once unk91 reaches 12, else 3 */
#define MAX_RANK() ((D_80364AF0[D_80364AE8].unk91 >= 12) ? 4 : 3)

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8C40.s")

void func_801E8DCC(u8 arg0) {
    s32 i;

    func_801E8EB8(arg0, 0);
    for (i = 0; i < 0x1B; i++) {
        D_802158A8[i] = D_802E8C94[D_8021592E];
    }
    D_802158A8[i] = D_802E8C98[D_8021592E];
    D_802154D8 = 0;
    D_802154E8 = 9999;
    for (i = 1; i < 5; i++) {
        D_802154F0[i] = 9999;
    }
    D_802154E4 = 3.0f;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8EB8.s")

/* Count the player's ranks per grade (D_80215930[1..4]) and the finished
 * levels of the 0x81 types, other than levels 0x26, 0x2F and 0x31 ([3]) */
void func_801E93DC(u8 arg0) {
    s32 i;
    Player *p;

    p = &D_80364AF0[arg0];
    for (i = 0; i < 6; i++) {
        D_80215930[i] = 0;
    }
    for (i = 0; i < 0x3C; i++) {
        if (p->rank[i] > 0 && p->rank[i] < 5) {
            D_80215930[p->rank[i]]++;
        }
        if (((D_80364AF0[arg0].rank[i] > 0 && D_80364AF0[arg0].rank[i] < 6) ? 1 : 0) && (D_802E8F94[i].type & 0x81) &&
            i != 0x31 && i != 0x2F && i != 0x26) {
            D_80215930[3]++;
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E9528.s")

s32 func_801E96F8(void) {
    return D_802154D2 == D_802154DC + 8;
}

/* Player select scene: lit backdrop fading with L/R (D_8021593C), the
 * scrolling vehicle icons and the scrolling name ticker */
Gfx *func_801E9718(Gfx *arg0, PlayerSelDyn *dyn, s32 arg2) {
    Gfx *gdl = arg0;

    gSPMatrix(gdl++, &dyn->persp, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->lookAt, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->trans, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, D_8021591C); /* old gSPPerspNormalize */
    gDPPipeSync(gdl++);
    gSPTexture(gdl++, 0x7C0, 0x7C0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_LIGHTING | G_SHADING_SMOOTH | G_SHADE);
    {
        s32 pad[4]; /* four unused stack slots sit here */

        gSPSetLights2(gdl++, D_80208448);
    }
    gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
    {
        s32 x;
        s32 i;

        if (D_80364A90 & 0x0005040008010080) {
            D_8021593C += 0x10;
            if (D_8021593C >= 0x100) {
                D_8021593C = 0xFF;
            }
        } else if (D_80364A90 & 0x0008000202020000) {
            D_8021593C -= 0x10;
            if (D_8021593C <= 0) {
                D_8021593C = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            D_80208380[i].v.cn[3] = (D_8021593C * 40) / 255;
        }
        for (i = 4; i < 7; i++) {
            D_80208380[i].v.cn[3] = (D_8021593C * 180) / 255;
        }
        osWritebackDCache(D_80208380, 0x80);
        if (D_8021593C != 0) {
            gSPDisplayList(gdl++, D_80208400);
        }
        gDPPipeSync(gdl++);
        gSPClearGeometryMode(gdl++, G_LIGHTING);
        D_802154D8 += D_802154E4;
        if (D_802154D8 >= D_80215458) {
            D_802154D8 -= D_80215458;
            for (i = 1; D_802158A8[i] != D_802E8C98[D_8021592E]; i++) {
                D_802158A8[i - 1] = D_802158A8[i];
            }
            i--;
            D_802158A8[i] = func_801E9528();
            D_802158A8[i + 1] = D_802E8C98[D_8021592E];
        }
        gdl = func_80274868(gdl);
        D_802154E8 += D_802154E4;
        x = 0x136 - D_802154E8;
        if (x >= -0x1F && x < 0x140) {
            gdl = func_80272ED8(gdl, D_80215915, x, 0xC2, D_8021593C, 1, 1.0f);
        }
        for (i = 1; i < 5; i++) {
            D_802154F0[i] += D_802154E4;
            x = 0x136 - D_802154F0[i];
            if (x >= -0x1F && x < 0x140) {
                gdl = func_80272ED8(gdl, D_80215916 + i, x, 0xC2, D_8021593C, 1, 1.0f);
            }
        }
        gdl = func_80274AA4(gdl);
        func_80259CCC(dyn, (D_8021592E == 1) ? NULL : func_8025B558(D_802158A8), (D_8021592E == 1) ? D_802158A8 : NULL,
                      0, 0, (-D_802154D8 % D_80215458) - 3, 0xC9, 0x14, 0x14, 1, 0, 0, 0,
                      (D_8021593C / 2 - 0x1B < 0) ? 0 : D_8021593C / 2 - 0x1B);
        func_80259DC8(dyn, (D_8021592E == 1) ? NULL : func_8025B558(D_802158A8), (D_8021592E == 1) ? D_802158A8 : NULL,
                      0, 0, -D_802154D8 % D_80215458, 0xC7, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF, D_8021593C, 0xFF, 0xFF,
                      0xFF, D_8021593C);
        gDPPipeSync(gdl++);
    }
    return gdl;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA108.s")

void func_801EA268(s32 *arg0) {
    arg0[4] = 0x1063E;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA278.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA4B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA6E8.s")

/* Open the name entry screen (menu entries 7 and 8) on buf */
void func_801EA93C(char *title, u16 *glyphs, u8 arg2, u8 width, char *buf) {
    D_802154B2 = 0x7FFF;
    D_802154B4 = 0x7FFF;
    D_80215918 = 0;
    D_802154B8 = 0;
    D_802154B6 = 0;
    D_802154BA = -1;
    D_802154BC = 0;
    D_802154BE = 0;
    D_802154C0 = 1;
    D_8020C070[7].unk6 = width;
    D_8020C070[7].unk8 = width;
    D_802154CC = width;
    D_802154C8 = (width * 3) / 5;
    D_80215928 = buf;
    *buf = 0;
    D_8020C070[7].unkC = D_80215928;
    D_802154C4 = 0xA0 - D_802154C8 / 2;
    D_8020C070[7].unk2 = D_802154C4;
    D_8020C070[8].unkC = title;
    D_8020C070[8].unk10 = glyphs;
    D_80215924 = arg2;
    D_802154D0 = 1;
    D_8021591A = 0;
    D_80215920 = 1;
    D_8021592C = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EAA7C.s")

void func_801EC288(u8 arg0) {
    s32 i;

    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    for (i = 0; i < 2; i++) {
        D_80215900[i] = 4;
        D_80215908[i] = 0x32;
        D_80215910[i] = 0;
    }
}

void func_801EC30C(u8 arg0) {
    s32 j;
    s32 i;

    D_80215914 = func_80272C5C(D_802082E4, D_802082D8, 4, 2, 0, 1.0f);
    func_80272C5C(D_802082E8, 0, 1, 2, 1, 1.0f);
    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    if (D_802E8F94[D_802E8BDC].type == 1) {
        j = 0;
    } else {
        j = 1;
    }
    for (i = 0; j < 2; j++, i++) {
        D_80215900[j] = 0;
        D_80215908[j] = i * 60 + 0x28;
        D_80215910[j] = 0;
    }
}

void func_801EC464(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        D_80215900[i] = 3;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC49C.s")

/* Draw the save-slot panel: the two slot boxes, or the rank badge and the
 * level's target time for it */
Gfx *func_801EC770(Gfx *start, void *gfxp, s32 *count) {
    Gfx *gdl;
    u8 n;

    gdl = start;
    gdl = func_80274868(gdl);
    if (D_80364AA8 == 1) {
        gdl = func_801EC49C(gdl, 0x5C, 0x68, 0);
        if (D_80215902[1] != 0) {
            gdl = func_801EC49C(gdl, 0xA8, 0x68, 1);
        }
    } else {
        if (D_80215902[1] == 5) {
            n = 1;
        } else {
            n = (MAX_RANK() < D_80215902[1] + 1) ? MAX_RANK() : D_80215902[1] + 1;
        }
        gdl = func_801EC49C(gdl, 0x82, 0x68, 1);
        gdl = func_80272ED8(gdl, n + D_80215914, 0x2E, 0x6C, (D_80215910[1] * 3) / 4, 0, 0.75f);
    }
    gdl = func_80274AA4(gdl);
    if (D_80364AA8 != 1) {
        func_80264A34(D_80215470, D_802E8F94[D_802E8BDC].times[5 - n], 0);
        D_80215470[5] = 0;
        func_80259DC8(gfxp, D_80215470, 0, 0, 0, 0x29, 0x7D, 0x10, 0x10, 1, 0xFF, 0xB4, 0, D_80215910[1], 0xFF, 0x78, 0,
                      D_80215910[1]);
    }
    *count += gdl - start;
    return gdl;
}

/* Level-select mask bit for a level: done, of a 0x81 type, or other */
u64 func_801ECA50(u8 level) {
    u64 mask;

    if ((D_80364AF0[D_80364AE8].rank[level] > 0 && D_80364AF0[D_80364AE8].rank[level] < 6) ? 1 : 0) {
        mask = 0x80;
    } else if (D_802E8F94[level].type & 0x81) {
        mask = 0x800;
    } else {
        mask = 0x20000000;
    }
    return mask;
}

/* Leave a level: queue the result messages for the current level and player */
void func_801ECB18(void) {
    func_801FE018(8);
    D_80364A87 = 0;
    D_803643D5 = 0;
    if ((D_80364AF0[D_80364AE8].rank[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].rank[D_802E8BDC] < 6) ? 1 : 0) {
        if (D_802E8F94[D_802E8BDC].type == 1) {
            osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 0xC | (D_80364AE8 << 16)), OS_MESG_BLOCK);
        }
        osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 8 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
    } else {
        osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 0x16 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
        func_801F8354(D_80364AE8);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECC8C.s")

/* Retype the 0x81-type levels: 0x80 once unk91 >= 11 in mode 0x4000, else 1 */
void func_801ECE9C(void) {
    s32 i;

    for (i = 0; i < 0x3C; i++) {
        if (D_802E8F94[i].type & 0x81) {
            if (D_80364AF0[D_80364AE8].unk91 >= 11 && D_80364A98 == 0x4000) {
                D_802E8F94[i].type = 0x80;
            } else {
                D_802E8F94[i].type = 1;
            }
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECF5C.s")

void func_801ED480(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 0x20; i++) {
        dst[i] = src[i];
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ED4B8.s")
