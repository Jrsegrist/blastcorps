#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* The digger code (this file's .bss starts at 0x8036C8D0) */
extern u8 D_80364456;      /* current vehicle */
extern void *D_80358070;

/* 50-entry ring buffer of digger samples, indexed D_8036CB28..D_8036CB29 */
typedef struct {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 unk4;   /* digger state when sampled */
    /* 0x08 */ s32 time;
} DigEntry;

/* Timed digger events, 8 bytes */
typedef struct {
    /* 0x00 */ s16 time;
    /* 0x02 */ u8 a;
    /* 0x03 */ u8 b;
    /* 0x04 */ u8 c;
    /* 0x05 */ u8 d;
    /* 0x06 */ u8 done;
} DigTrigger;

extern DigTrigger *D_803BE6FC; /* first */
extern DigTrigger *D_803BE700; /* end */
extern u8 D_803643D6;
extern u8 D_803643DB;
extern s32 D_803EF6E4;
extern u8 D_8036CB2F;
extern u64 D_80364A90;
extern u32 D_80364AA8;
extern u8 D_8036CB35;
extern u8 D_8036CB36;
extern u8 D_8036CB37;
extern u8 D_8036CB38;
extern u8 D_8036CB39;
extern u8 D_8036CB3A;
extern u8 D_8036CB3B;
extern u8 D_8036CB3C;
extern s16 *D_8036CB40; /* event animation frame ids */
extern u8 D_8036CB44;
extern u8 D_8036CB50;
extern u8 D_8036CB51;      /* event panel alpha */
extern u8 D_802FAD50[];    /* 32x32 RGBA32 panel frame */
extern u8 D_02000000[];    /* segment 2 base */
extern s16 D_802FBDD0[];
extern s16 D_802FBDEC[];
extern s16 D_802FBE18[];
extern s16 D_802FBE44[];
extern s16 D_802FBE80[];
extern Vtx D_802FBD50[8];
extern void *D_80367738;

extern DigEntry D_8036C8D0[50];
extern u8 D_8036CB32;
extern void *D_8036CB48[2];
extern u8 D_8036CB28;
extern u8 D_8036CB29;
extern s16 D_8036CB2A;
extern s16 D_8036CB2C;
extern u8 D_8036CB2E;
extern u8 D_8036CB30;
extern u8 D_8036CB31;
extern u8 D_8036CB33;
extern u8 D_8036CB34;

s32 func_80277D34(void);
s32 func_80277E08(void);
void func_80277C20(void);
void func_802778FC(void);
void func_80277AE0(void);
void func_80277B84(void);

void func_802775C0(void) {
    D_8036CB34 = 0;
    D_8036CB48[0] = D_80358070;
    D_8036CB48[1] = D_80358070 = (u8 *) D_80358070 + 0xC80;
    D_80358070 = (u8 *) D_80358070 + 0xC80;
    D_8036CB28 = 0;
    D_8036CB29 = 0;
}

void func_80277620(s32 now) {
    u8 done = 0;
    DigTrigger *p = D_803BE6FC;
    s16 limit;

    while (!done && D_8036CB28 != D_8036CB29) {
        if (now - D_8036C8D0[D_8036CB28].time > 1000) {
            if (++D_8036CB28 == 50) {
                D_8036CB28 = 0;
            }
        } else {
            done = 1;
        }
    }
    if (D_8036CB2F) {
        if (D_8036CB29 + 1 != D_8036CB28 && !(D_8036CB29 == 49 && D_8036CB28 == 0)) {
            D_8036C8D0[D_8036CB29].unk0 = func_80277D34();
            D_8036C8D0[D_8036CB29].unk2 = func_80277E08();
            D_8036C8D0[D_8036CB29].unk4 = D_8036CB2E;
            D_8036C8D0[D_8036CB29].time = now;
            if (++D_8036CB29 == 50) {
                D_8036CB29 = 0;
            }
        }
    }
    if (D_8036CB2F) {
        func_80277C20();
        func_802778FC();
        func_80277AE0();
        func_80277B84();
    }
    if (now == 50 && !D_803643D6) {
        func_80277EDC(2, 1, 2, func_8026205C(1));
    }
    if (D_803643DB) {
        limit = D_803EF6E4 >> 5;
        while (p < D_803BE700) {
            if (!p->done && limit > p->time) {
                func_80277EDC(p->a, p->b, p->c, p->d);
                p->done = 1;
            }
            p++;
        }
    }
    D_8036CB2F = 0;
}

void func_802778FC(void) {
    switch (D_80364456) {
        case 5:
            if (D_8036CB31 >= 16 && D_8036CB30 >= 16) {
                func_80277EDC(3, 1, 1, 0x63);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 4:
            if (D_8036CB31 >= 41 && D_8036CB30 >= 41) {
                func_80277EDC(4, 1, 1, func_8026205C(4));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 3:
            if (D_8036CB31 >= 11 && D_8036CB30 >= 11) {
                func_80277EDC(0, 1, 3, 0xB2);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 9:
            if (D_8036CB31 >= 26 && D_8036CB30 >= 26) {
                func_80277EDC(4, 1, 1, func_8026205C(4));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
    }
}

void func_80277AE0(void) {
    switch (D_80364456) {
        case 3:
        case 4:
        case 5:
        case 9:
            if (D_8036CB31 >= 6 && D_8036CB30 < 2) {
                func_80277EDC(1, 1, 3, 0xB3);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("USING WRONG DIGGER\n");
            }
            break;
    }
}

void func_80277B84(void) {
    switch (D_80364456) {
        case 3:
        case 4:
        case 5:
            if (D_8036CB33 >= 31 && D_8036CB30 < 6) {
                func_80277EDC(2, 1, 2, 0x58);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("USING DIGGER INCORRECTLY\n");
            }
            break;
    }
}

void func_80277C20(void) {
    u8 i;

    D_8036CB30 = 0;
    i = D_8036CB28;
    D_8036CB31 = 0;
    D_8036CB32 = 0;
    D_8036CB33 = 0;
    while (i != D_8036CB29) {
        if (!D_8036C8D0[i].unk2) {
            D_8036CB30++;
        } else {
            D_8036CB32++;
        }
        if (!D_8036C8D0[i].unk0) {
            D_8036CB31++;
        } else {
            D_8036CB33++;
        }
        if (++i == 50) {
            i = 0;
        }
    }
}

s32 func_80277D34(void) {
    switch (D_8036CB2E) {
        case 5:
            if (D_8036CB2A > 400) {
                return 0;
            }
            return 1;
        case 4:
            if (D_8036CB2A > 400) {
                return 0;
            }
            return 1;
        case 9:
            if (D_8036CB2A >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (D_8036CB2A >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

s32 func_80277E08(void) {
    switch (D_8036CB2E) {
        case 5:
            if (D_8036CB2C > 80) {
                return 0;
            }
            return 1;
        case 4:
            if (D_8036CB2C > 80) {
                return 0;
            }
            return 1;
        case 9:
            if (D_8036CB2C >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (D_8036CB2C >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

/* Starts a digger event. A K&R definition: callers pass plain ints, and the
 * u8 parameters read their low bytes */
#ifdef NON_MATCHING
void func_80277EDC(u8 type, u8 arg1, s32 arg2, u8 sound)
#else
void func_80277EDC(type, arg1, arg2, sound)
    u8 type;
    u8 arg1;
    s32 arg2;
    u8 sound;
#endif
{
    u8 pos;

    if ((D_80364A90 & 0x200000000400220C) && !D_8036CB34) {
        if (sound) {
            func_80260650(D_80367738, sound, NULL);
        }
        D_8036CB34 = 1;
        D_8036CB35 = arg2;
        D_8036CB36 = 0;
        D_8036CB37 = 0;
        D_8036CB39 = 0;
        D_8036CB3A = 0;
        D_8036CB38 = arg1;
        D_8036CB44 = type;
        D_8036CB50 = 0;
        switch (type) {
            case 0:
                pos = 0;
                D_8036CB3C = 13;
                D_8036CB40 = D_802FBDD0;
                D_8036CB3B = 1;
                break;
            case 1:
                pos = 1;
                D_8036CB3C = 21;
                D_8036CB40 = D_802FBDEC;
                D_8036CB3B = 1;
                break;
            case 2:
                pos = 0;
                D_8036CB3C = 21;
                D_8036CB40 = D_802FBE18;
                D_8036CB3B = 2;
                break;
            case 3:
                pos = 1;
                D_8036CB3C = 30;
                D_8036CB40 = D_802FBE44;
                D_8036CB3B = 1;
                break;
            case 4:
                pos = 0;
                D_8036CB3C = 34;
                D_8036CB40 = D_802FBE80;
                D_8036CB3B = 1;
                break;
        }
        if (D_80364AA8 != 1) {
            pos = 1;
        }
        switch (pos) {
            case 0:
                D_802FBD50[0].v.ob[0] = 32, D_802FBD50[0].v.ob[1] = 68;
                D_802FBD50[1].v.ob[0] = 78, D_802FBD50[1].v.ob[1] = 68;
                D_802FBD50[2].v.ob[0] = 78, D_802FBD50[2].v.ob[1] = 23;
                D_802FBD50[3].v.ob[0] = 32, D_802FBD50[3].v.ob[1] = 23;
                D_802FBD50[4].v.ob[0] = 26, D_802FBD50[4].v.ob[1] = 73;
                D_802FBD50[5].v.ob[0] = 84, D_802FBD50[5].v.ob[1] = 73;
                D_802FBD50[6].v.ob[0] = 84, D_802FBD50[6].v.ob[1] = 13;
                D_802FBD50[7].v.ob[0] = 26, D_802FBD50[7].v.ob[1] = 13;
                break;
            case 1:
                D_802FBD50[0].v.ob[0] = 238, D_802FBD50[0].v.ob[1] = 222;
                D_802FBD50[1].v.ob[0] = 284, D_802FBD50[1].v.ob[1] = 222;
                D_802FBD50[2].v.ob[0] = 284, D_802FBD50[2].v.ob[1] = 177;
                D_802FBD50[3].v.ob[0] = 238, D_802FBD50[3].v.ob[1] = 177;
                D_802FBD50[4].v.ob[0] = 232, D_802FBD50[4].v.ob[1] = 227;
                D_802FBD50[5].v.ob[0] = 290, D_802FBD50[5].v.ob[1] = 227;
                D_802FBD50[6].v.ob[0] = 290, D_802FBD50[6].v.ob[1] = 167;
                D_802FBD50[7].v.ob[0] = 232, D_802FBD50[7].v.ob[1] = 167;
                break;
        }
    }
}

void func_80278318(void) {
    D_8036CB34 = 0;
}

/* Animates and draws the digger event panel */
void func_80278324(Gfx **gfx, s32 arg1, u8 buf) {
    Gfx *gdl = *gfx;

    if (D_8036CB34) {
        func_802A1040(D_8036CB40[D_8036CB39], D_8036CB48[buf], 0);
        switch (D_8036CB38) {
            case 1:
                if (!D_8036CB37) {
                    D_8036CB3A++;
                    if (D_8036CB3A == D_8036CB3B) {
                        D_8036CB3A = 0;
                        D_8036CB39++;
                        if (D_8036CB39 + 1 == D_8036CB3C) {
                            D_8036CB37 = 1;
                            D_8036CB36++;
                        }
                    }
                } else {
                    D_8036CB3A++;
                    if (D_8036CB3A == D_8036CB3B) {
                        D_8036CB3A = 0;
                        D_8036CB39--;
                        if (D_8036CB39 == 0) {
                            D_8036CB37 = 0;
                            D_8036CB36++;
                        }
                    }
                }
                break;
            case 0:
                D_8036CB3A++;
                if (D_8036CB3A == D_8036CB3B) {
                    D_8036CB3A = 0;
                    D_8036CB39++;
                    if (D_8036CB39 == D_8036CB3C) {
                        D_8036CB39 = 0;
                        D_8036CB36++;
                    }
                }
                break;
        }
        if (D_8036CB36 == D_8036CB35) {
            D_8036CB34 = 0;
        }
        if (D_8036CB36 == 0 && D_8036CB39 < 5) {
            if (D_8036CB39 == 0) {
                func_80260650(D_80367738, 0x69, NULL);
            }
            D_8036CB51 = 0x50;
        } else if (D_8036CB50) {
            D_8036CB51 -= 15;
            D_8036CB50--;
        } else if (!func_8026A828(0, 20)) {
            D_8036CB50 = 5;
            D_8036CB51 = 0x50;
            func_80260650(D_80367738, 0x69, NULL);
        } else {
            D_8036CB51 = 0;
        }
        gSPMatrix(gdl++, (u32) D_02000000 + 0xC0, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(gdl++, (u32) D_02000000 + 0x1C0, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_2CYCLE);
        gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombine(gdl++, 0xFC757E44, 0xFFFFF83C);
        gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8036CB51);
        gDPLoadTextureBlock(gdl++, (u32) D_8036CB48[buf] - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 40, 40, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSPVertex(gdl++, (u32) D_802FBD50 - 0x80000000, 8, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
        gDPLoadTextureBlock(gdl++, (u32) D_802FAD50 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gDPPipeSync(gdl++);
    }
    *gfx = gdl;
}
