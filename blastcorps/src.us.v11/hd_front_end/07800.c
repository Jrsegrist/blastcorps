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
#define D_8020C070 ((MenuEntry *) D_8020C070)
#define D_8020D810 ((FeLevelEntry *) D_8020D810)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F49F4 ((IconInfo *) D_802F49F4)
#define D_803156F8 ((FeDyn *) D_803156F8)
#define D_80364AF0 ((FePlayer *) D_80364AF0)
#ifdef NON_MATCHING
#define D_80358070 (*(s32 *) &D_80358070)
#endif
/* end of views */

/*
 * stats.c (named by its assert): end-of-level results. Grades the time,
 * records best times and units, and sets up the results screen.
 */

/* Pre-2.0I texture rectangle: the RDP-half commands are 0xB3/0xB2 (see hd_code 17E10). */
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


/* front-end view of hd_code's Player record (0x100 bytes, D_80364AF0) */
typedef struct {
    u8 pad0[0xA];
    u16 unkA; /* units */
    u8 unkC;
    u8 padD[7];
    s32 unk14;
    u8 unk18[60]; /* per level: grade */
    u8 unk54[60]; /* per level: bits */
    u8 pad90;
    u8 unk91;
    u8 unk92[60]; /* per level */
    u8 padCE[0x100 - 0xCE];
} FePlayer;

typedef struct {
    u8 pad0[4];
    char *unk4; /* level name */
    u8 pad8[0x10];
    s8 unk18[0x18]; /* -1 terminated */
} FeLevelEntry;     /* 0x30 bytes, one per level */

typedef struct {
    u8 unk0; /* type */
    u8 pad1[0x2F];
    u16 unk30; /* time thresholds, best first */
    u16 unk32;
    u16 unk34;
    u16 unk36;
    u8 pad38[0xC];
} LevelInfo;


typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char *unkC;  /* title */
    void *unk10; /* glyph list */
    u8 unk14;
    u8 pad15[5];
    u8 unk1A;
    u8 pad1B;
} MenuEntry;

typedef struct {
    u8 pad0[4];
    u8 unk4;
    u8 pad5;
    u16 unk6[19];
    u8 unk2C;
    u8 unk2D;
    u8 pad2E[2];
} IconInfo;

extern char D_8036B980[];
extern u16 D_80303B3C[];
extern u16 D_80303B48[];
extern u16 D_80303B58[];
extern u16 D_80303B68[];
#ifndef NON_MATCHING
extern s32 D_80358070; /* heap pointer */
#endif
extern u16 D_802159D0; /* scene angle */
extern u8 *D_802159D4; /* 256x32 IA8 banner */
extern u8 *D_802159D8; /* 40x24 IA8 icon */
extern u16 D_802159DC; /* scene */
extern f32 D_802159E0; /* camera distance */
extern f32 D_802159E4; /* spin speed */

/* ROM bounds of two compressed blobs (the first ends where the second starts) */
extern u8 D_0048F5A0_end[];
extern u8 D_0048F970[];
extern u8 D_0048F970_end[];

int sprintf(char *, const char *, ...);
#ifdef NON_MATCHING
u8 func_801EEDB4(u8 arg0, u8 arg1, u8 arg2);
#else
u8 func_801EEDB4(); /* K&R */
#endif

#define levelno D_802E8BDC
#define DUMMY_LEVELS(l) ((l) == 49 || (l) == 47 || (l) == 38)
#define STATS_ASSERT(EX, line) \
    if (!(EX))                 \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "stats.c", line)

#define PRINT_SCORE(name, s)                                                                                       \
    func_8029A7E4(name " ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n", (s).ip, (s).tc, (s).bd, \
                  (s).cr, (s).rt, (s).coin, (s).bdn)

/* Finish a level: grade it, update units and best times. *arg0 = 1 if a rank was gained. */
u8 func_801EE800(u8 *arg0, u8 arg1, u8 arg2) {
    FePlayer *p;
    LevelInfo *l;
    u32 t;
    u8 ret;
    s32 pad[2];

    p = &D_80364AF0[D_80364AE8];
    l = &D_802E8F94[levelno];
    PRINT_SCORE("new", D_8036EA70);
    PRINT_SCORE("old", D_8036EA60);
    PRINT_SCORE("res", D_8036EA80);
    PRINT_SCORE("rs2", D_8036EA90);
    func_8029A7E4("units %d\n", p->unkA);
    if (D_802E8F94[levelno].unk0 == 1) {
        t = func_802852EC();
        if (arg2) {
            if (arg1) {
                if (t >= 100) {
                    D_8036EA70.coin = 3;
                } else if (t >= 90) {
                    D_8036EA70.coin = 2;
                } else if (t >= 70) {
                    D_8036EA70.coin = 1;
                } else {
                    D_8036EA70.coin = 5;
                }
                if (D_803643D5) {
                    func_8029A7E4("Units up 3\n");
                    p->unkA += 3;
                }
                D_8036EA70.bdn = 1;
            } else {
                D_8036EA70.coin = 0;
            }
        }
        ret = D_8036EA70.coin;
    } else {
        ret = func_801EEDB4(levelno, arg1, arg2);
    }
    sprintf(D_8036B980, "%s", D_8020D810[levelno].unk4);
    *arg0 = 0;
    if (arg1 && arg2) {
        STATS_ASSERT(!DUMMY_LEVELS(levelno), 94);
        if (D_802E8F94[levelno].unk0 == 1) {
            p->unk14 = ((s32) D_803649F0);
        }
        if (p->unkA < 360) {
            func_8029A7E4("UNITS UP %d\n", D_8036EA70.coin % 5 - D_8036EA60.coin % 5);
            p->unkA += D_8036EA70.coin % 5 - D_8036EA60.coin % 5;
        }
        if (p->unkA == 354) {
            p->unkA += 6;
        }
        if (p->unkA / 12 > p->unkC) {
            *arg0 = 1;
            p->unkC++;
        }
        if (!(D_802E8F94[levelno].unk0 & 0x81)) {
            p->unk92[levelno] = D_8036EA70.bdn;
        }
        D_80364EF0[D_80364AE8][D_802E8C44[D_8036EA70.bdn]] = D_8036EA70.tc;
        if (D_803643D5 && D_802E8F94[levelno].unk0 == 1) {
            D_80364EF0[D_80364AE8][D_802E8C44[0]] = D_8036EA70.tc;
        }
        p->unk18[levelno] = ret;
        func_801E8DCC(D_80364AE8);
    }
    return ret;
}

/* results screen banner and its glyph list, by outcome */
char *D_802084D0[] = { "YOUR NEW BEST!", "BEST TO DATE", "YOUR BEST STAYS", "GUEST BEST IS" };
u16 *D_802084E0[] = { D_80303B3C, D_80303B48, D_80303B58, D_80303B68 };

/* Grade a timed level `arg0` and record the best time; returns the grade.
 * K&R in the matching build (its callers pass unmasked ints). */
#ifdef NON_MATCHING
u8 func_801EEDB4(u8 arg0, u8 arg1, u8 arg2)
#else
u8 func_801EEDB4(arg0, arg1, arg2)
    u8 arg0;
    u8 arg1;
    u8 arg2;
#endif
{
    s32 x;
    s32 pad;
    s32 idx;
    FePlayer *p;
    LevelInfo *l;
    IconInfo *e;
    MenuEntry *m;
    char buf[32];
    u16 old;

    p = &D_80364AF0[D_80364AE8];
    l = &D_802E8F94[arg0];
    if (D_80364A98 == 0x08000000 && arg1 && l->unk0 == 2) {
        func_80295A20(func_80286038(D_8036EA70.tc));
    }
    if (arg2 && arg1) {
        if (D_802E8F94[arg0].unk0 == 0x80) {
            D_8036EA70.bdn = 0;
        } else if (D_8036EA70.tc <= D_8036EA60.tc) {
            D_8036EA70.bdn = D_803643D4;
        } else if (D_8036EA70.tc != 0xFFFF &&
                   ((old = D_80364EF0[D_80364AE8][D_802E8C44[D_803643D4]]) == 0 || D_8036EA70.tc < old)) {
            D_80364EF0[D_80364AE8][D_802E8C44[D_803643D4]] = D_8036EA70.tc;
        }
    }
    if (D_8036EA70.tc <= D_8036EA60.tc && arg1) {
        D_8036EA70.coin = func_801EF2BC(D_8036EA70.tc, arg0, D_80364AF0[D_80364AE8].unk91);
    } else {
        D_8036EA70.tc = D_8036EA60.tc;
    }
    if (D_8036EA70.tc < D_8036EA60.tc && !D_803643D5) {
        x = 0x484;
        if (arg2) {
            x = 0x584;
        }
    } else {
        x = 0x480;
    }
    func_80264A34(buf, D_8036EA70.tc, 0);
    sprintf(&D_8036B9A8[0x80], "****%s*", buf);
    if (arg1) {
        m = &D_8020C070[25];
        D_8020C070[25].unk0 = x;
        func_8029A7E4("getting icon %d\n", D_8036EA70.bdn);
        m->unk14 = D_8036EA70.bdn + 0x22;
        e = &D_802F49F4[m->unk14];
        m->unk1A = func_80272C5C(e->unk6, 0, e->unk4, e->unk2C, e->unk2D | 4, 1.0f);
        if (D_80364AE8 != D_80364AEA) {
            idx = 3;
        } else if (D_80364A98 == 0x80 || D_803643D5) {
            idx = 1;
        } else if (D_8036EA70.tc < D_8036EA60.tc) {
            idx = 0;
        } else {
            idx = 2;
        }
        D_8020C070[24].unkC = D_802084D0[idx];
        D_8020C070[24].unk10 = D_802084E0[idx];
    }
    return D_8036EA70.coin;
}

/* Count the current level's entries (at most 2) whose bit is set for the player. */
s8 func_801EF1E0(void) {
    FeLevelEntry *e;
    s32 i;
    s32 count;

    e = &D_8020D810[D_802E8BDC];
    if (e->unk18[0] == -1) {
        return -1;
    }
    for (i = 0, count = 0; e->unk18[i] != -1 && i < 2; i++) {
        if (D_80364AF0[D_80364AE8].unk54[D_802E8BDC] & (1 << i)) {
            count++;
        }
    }
    return count;
}

/* Grade a time against level `arg1`'s thresholds: 4 (best, needs arg2 >= 12) .. 1, else 5. */
#ifdef NON_MATCHING
u8 func_801EF2BC(u16 arg0, u8 arg1, u8 arg2)
#else
u8 func_801EF2BC(arg0, arg1, arg2)
    u16 arg0;
    u8 arg1;
    u8 arg2;
#endif
{
    u8 ret;
    LevelInfo *l;

    l = &D_802E8F94[arg1];
    if (l->unk30 >= arg0 && arg2 >= 12) {
        ret = 4;
    } else if (l->unk32 >= arg0) {
        ret = 3;
    } else if (l->unk34 >= arg0) {
        ret = 2;
    } else if (l->unk36 >= arg0) {
        ret = 1;
    } else {
        ret = 5;
    }
    return ret;
}

/* Results screen init: load scene `arg0` and inflate the two blobs onto the heap. */
void func_801EF380(s32 arg0) {
    s32 size1;
    s32 size2;

    size1 = D_0048F5A0_end - D_0048F5A0;
    size2 = D_0048F970_end - D_0048F970;
    func_801F4E70(arg0);
    if (arg0 == 2) {
        D_802159D0 = 90;
    } else {
        D_802159D0 = 0;
    }
    func_8028B4C4((u32) D_0048F5A0, (u32) ((void *) D_80358070), (u32 *) &size1, 12, 0, 1);
    D_802159D4 = (u8 *) D_80358070;
    D_80358070 += size1;
    func_8028B4C4((u32) D_0048F970, (u32) ((void *) D_80358070), (u32 *) &size2, 12, 0, 1);
    D_802159D8 = (u8 *) D_80358070;
    D_80358070 += size2;
    D_802159DC = arg0;
    D_802159E0 = 0.0f;
    D_802159E4 = 3.0f;
}

/* 8 unreferenced zero bytes sit between the last string and func_801EF4AC's
 * double 7600.0 (0x8020EFA4..AB); probably compiled-out debug strings. */
const u32 D_8020EFA4[2] = { 0, 0 };

/* Results screen frame: builds the whole top-level display list (spinning
 * scene, banner and icon fading in, final fade out). */
void func_801EF4AC(void) {
    FeDyn *dyn;
    Gfx *gdl;
    s32 i;
    s32 xo;
    s32 yo;
    s16 c;

    dyn = &D_803156F8[D_8035805C ^ 1];
    gdl = dyn->dl;
    func_8028A470();
    func_80284E54((u64 *) (D_803156F8[D_8035805C].dl), D_80358078, 1, 1, 0x4D2, 0);
    D_8035805C ^= 1;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetDepthImage(gdl++, D_80358058);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358058);
    gDPSetFillColor(gdl++, 0xFFFCFFFC);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPPipeSync(gdl++);
    if (D_802159DC == 1) {
        if (D_80358060 * 185 / 20 > 185) {
            c = 185;
        } else {
            c = D_80358060 * 185 / 20;
        }
    } else {
        c = 0;
    }
    gDPSetFillColor(gdl++, (GPACK_RGBA5551(c, c, c, 1) << 16) | GPACK_RGBA5551(c, c, c, 1));
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    func_8028A3E4();
    if (D_80358060 == 250) {
        if (D_80364A90 == 0x10) {
            D_80364A98 = 0x20;
        } else {
            D_80364A98 = 0x0400000000000000;
            if (D_802FA268) {
                func_80260650(D_80367738, 0x68, NULL);
            }
        }
    }
    if (D_80358060 < 2) {
        guPerspective(&dyn->unk1240, &D_8035807C, 45.0f, 1.3333334f, 40.0f, 8000.0f, 0.25f);
        if (D_802159DC == 1) {
            guTranslate(&dyn->unk1280, 0.0f, -130.0f, 0.0f);
            guRotate(&dyn->unk12C0, 35.0f, 0.1f, 0.0f, 0.0f);
        } else {
            guTranslate(&dyn->unk1280, 0.0f, 0.0f, 0.0f);
            guRotate(&dyn->unk12C0, -10.0f, 0.1f, 0.0f, 0.0f);
        }
    }
    if (D_80358060 >= 20) {
        if (D_80358060 == 20 && D_802159DC == 1) {
            func_80260650(D_80367738, 0xBA, NULL);
        } else if (D_80358060 == 20 && D_802159DC == 2) {
            func_80260650(D_80367738, 0xBD, NULL);
        }
        if (D_80358060 < 80) {
            D_802159E0 = (80 - D_80358060) * 7600.0 / 60.0 + 400.0;
        }
        if (D_80358060 == 75 && D_802159DC == 2) {
            func_80260650(D_80367738, 0xB8, NULL);
        } else if (D_80358060 == 75 && D_802159DC == 1) {
            func_80260650(D_80367738, 0xBB, NULL);
        }
        guLookAtReflect(&dyn->unk140, &dyn->unk3C00, 1.0f, 0.0f, D_802159E0, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        D_802159D0 += D_802159E4;
        guRotate(&D_802182D0[D_8035805C], D_802159D0 % 360, 0.0f, 1.0f, 0.0f);
        guScale(&dyn->unk1300, 1.5f, 1.5f, 1.5f);
        gDPSetRenderMode(gdl++, G_RM_AA_ZB_OPA_INTER, G_RM_NOOP2);
        gdl = func_801F4FBC(dyn, gdl);
    }
    if (D_80358060 > 80 && D_802159DC == 1) {
        gDPPipeSync(gdl++);
        gSPTexture(gdl++, 0, 0, 0, 0, G_OFF);
        gDPSetTexturePersp(gdl++, G_TP_NONE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombine(gdl++, 0x119623, 0xFF2FFFFF);
        gDPSetPrimColor(gdl++, 0, 0, 0x28, 0, 0xFF, (D_80358060 * 6 - 480 < 255) ? D_80358060 * 6 - 480 : 255);
        /* the banner, in 32-texel-wide strips */
        xo = 26, yo = 42;
        for (i = 0; i < 256; i += 32) {
            gDPLoadTextureTile(gdl++, D_802159D4, G_IM_FMT_IA, G_IM_SIZ_8b, 256, 32, i, 0, i + 31, 31, 0, G_TX_CLAMP,
                               G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangleOld(gdl++, (i + xo) << 2, yo << 2, (i + xo + 32) << 2, (yo + 32) << 2,
                                   G_TX_RENDERTILE, i << 5, 0, 1 << 10, 1 << 10);
        }
        gDPPipeSync(gdl++);
        gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0, 0x28, (D_80358060 * 4 - 320 < 255) ? D_80358060 * 4 - 320 : 255);
        gDPLoadTextureBlock(gdl++, D_802159D8, G_IM_FMT_IA, G_IM_SIZ_8b, 40, 24, 0, G_TX_CLAMP, G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangleOld(gdl++, 256 << 2, 20 << 2, (256 + 40) << 2, (20 + 24) << 2, G_TX_RENDERTILE, 0, 0,
                               1 << 10, 1 << 10);
        gDPSetTexturePersp(gdl++, G_TP_PERSP);
    }
    if (D_80358060 >= 221) {
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, (D_80358060 - 220) * 255 / 30);
        gDPFillRectangle(gdl++, 0, 0, 319, 239);
    }
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    D_80358078 = gdl - dyn->dl;
}
