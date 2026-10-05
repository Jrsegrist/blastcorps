#include "common.h"
#include <ultra64.h>

/* One entry per level on the front end's level-select globe. */
typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x04 */ char *name;
    /* 0x08 */ u16 *jname; /* hd_code glyph string (0x0FFE-terminated) */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ s8 unk18[4]; /* level ids, -1-terminated */
    /* 0x1C */ s8 unk1C[8]; /* level ids, -1-terminated */
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
} GlobeLevel; /* size 0x30 */

#define JTEXT(addr) ((u16 *) (addr))

GlobeLevel D_8020D810[60] = {
    { 0x64, 1, "SIMIAN ACRES", JTEXT(0x80303700), 0, 0.0f, 0.0f, { -1 }, { 0x1C, 0x13, 0x03, 0x37, 0x32, -1 } },
    { 0x64, 1, "ANGEL CITY", JTEXT(0x8030370C), 0, -20.0f, -120.0f, { -1 }, { -1 } },
    { 0x64, 1, "OUTLAND FARM", JTEXT(0x80303720), 0, 20.0f, 60.0f, { 0x25, 0x1B, -1 }, { 0x05, 0x0D, 0x09, -1 } },
    { 0x64, 1, "BLACKRIDGE WORKS", JTEXT(0x80303734), 0, -20.0f, -20.0f, { 0x14, 0x06, -1 }, { 0x1D, 0x35, 0x0F, 0x0A, -1 } },
    { 0x64, 1, "GLORY CROSSING", JTEXT(0x80303748), 0, 70.0f, 180.0f, { -1 }, { -1 } },
    { 0x64, 1, "SHUTTLE GULLY", JTEXT(0x8030375C), 0, 50.0f, 20.0f, { 0x29, -1 }, { 0x04, 0x21, 0x02, -1 } },
    { 0x64, 1, "SALVAGE WHARF", JTEXT(0x8030376C), 0, -30.0f, -10.0f, { -1 }, { -1 } },
    { 0x64, 1, "SKYFALL", JTEXT(0x8030377C), 0, 40.0f, 20.0f, { -1 }, { 0x05, -1 } },
    { 0x64, 1, "TWILIGHT FOUNDRY", JTEXT(0x80303794), 0, 10.0f, 30.0f, { -1 }, { -1 } },
    { 0x64, 1, "CRYSTAL RIFT", JTEXT(0x803037A8), 0, 20.0f, 120.0f, { 0x33, 0x1F, -1 }, { -1 } },
    { 0x64, 1, "ARGENT TOWERS", JTEXT(0x803037B8), 0, -20.0f, 20.0f, { 0x15, 0x22, -1 }, { 0x0D, 0x11, 0x10, 0x03, -1 } },
    { 0x64, 1, "SKERRIES", JTEXT(0x803037CC), 0, 70.0f, 0.0f, { -1 }, { -1 } },
    { 0x64, 1, "DIAMOND SANDS", JTEXT(0x803037D8), 0, -70.0f, 110.0f, { -1 }, { -1 } },
    { 0x64, 1, "EBONY COAST", JTEXT(0x803037EC), 0, -20.0f, 60.0f, { 0x20, -1 }, { 0x1A, 0x11, 0x02, -1 } },
    { 0x64, 1, "OYSTER HARBOR", JTEXT(0x803037FC), 0, -70.0f, -110.0f, { 0x17, -1 }, { -1 } },
    { 0x64, 1, "CARRICK POINT", JTEXT(0x8030380C), 0, 20.0f, -20.0f, { 0x19, 0x1E, -1 }, { 0x21, 0x12, 0x10, 0x03, -1 } },
    { 0x64, 1, "HAVOC DISTRICT", JTEXT(0x8030381C), 0, 20.0f, 20.0f, { 0x16, 0x08, -1 }, { 0x07, 0x02, 0x0F, 0x0A, -1 } },
    { 0x64, 1, "IRONSTONE MINE", JTEXT(0x8030382C), 0, -50.0f, 20.0f, { -1 }, { 0x0A, 0x0C, 0x0D, 0x3A, -1 } },
    { 0x64, 1, "BEETON TRACKS", JTEXT(0x80303844), 0, 20.0f, -60.0f, { 0x30, -1 }, { 0x1D, 0x0F, 0x21, 0x39, -1 } },
    { 0x00, 0, "J-BOMB", JTEXT(0x80303854), 0, 10.0f, 10.0f, { -1 }, { 0x10, -1 } },
    { 0x64, 1, "JADE PLATEAU", JTEXT(0x80303864), 0, -10.0f, -30.0f, { -1 }, { -1 } },
    { 0x64, 1, "MARINE QUARTER", JTEXT(0x80303874), 0, -30.0f, 10.0f, { -1 }, { -1 } },
    { 0x64, 1, "COOTER CREEK", JTEXT(0x80303884), 0, 30.0f, 10.0f, { 0x0B, -1 }, { -1 } },
    { 0x64, 1, "GIBBON'S GATE", JTEXT(0x80303890), 0, -50.0f, 180.0f, { -1 }, { -1 } },
    { 0x64, 1, "BABOON CATACOMB", JTEXT(0x803038A0), 0, 30.0f, -150.0f, { -1 }, { 0x25, -1 } },
    { 0x64, 1, "SLEEK STREETS", JTEXT(0x803038B0), 0, 10.0f, -30.0f, { -1 }, { -1 } },
    { 0x64, 1, "OBSIDIAN MILE", JTEXT(0x803038C0), 0, -20.0f, 120.0f, { -1 }, { -1 } },
    { 0x64, 1, "CORVINE BLUFF", JTEXT(0x803038D0), 0, 10.0f, 50.0f, { 0x38, -1 }, { -1 } },
    { 0x28, 1, "SIDESWIPE", JTEXT(0x803038E0), 0, 10.0f, -10.0f, { -1 }, { 0x0F, -1 } },
    { 0x64, 1, "ECHO MARCHES", JTEXT(0x803038F8), 0, -20.0f, -60.0f, { 0x2A, 0x27, -1 }, { 0x01, 0x3A, 0x12, -1 } },
    { 0x64, 1, "KIPLING PLANT", JTEXT(0x80303904), 0, 30.0f, -10.0f, { -1 }, { -1 } },
    { 0x64, 1, "FALCHION FIELD", JTEXT(0x80303918), 0, 45.0f, 100.0f, { -1 }, { -1 } },
    { 0x64, 1, "MORGAN HALL", JTEXT(0x80303930), 0, -10.0f, 50.0f, { -1 }, { -1 } },
    { 0x64, 1, "TEMPEST CITY", JTEXT(0x8030393C), 0, 50.0f, -20.0f, { 0x23, -1 }, { 0x12, 0x05, 0x04, -1 } },
    { 0x64, 1, "ORION PLAZA", JTEXT(0x80303950), 0, -10.0f, 30.0f, { -1 }, { -1 } },
    { 0x64, 1, "GLANDER'S RANCH", JTEXT(0x80303960), 0, 50.0f, -70.0f, { -1 }, { -1 } },
    { 0x64, 1, "DAGGER PASS", JTEXT(0x80303974), 0, -20.0f, 180.0f, { 0x34, -1 }, { -1 } },
    { 0x64, 1, "GEODE SQUARE", JTEXT(0x80303980), 0, 50.0f, 180.0f, { 0x3B, -1 }, { -1 } },
    { 0x64, 1, "SHUTTLE ISLAND", JTEXT(0x80303990), 0, 0.0f, -140.0f, { -1 }, { 0x28, -1 } },
    { 0x64, 1, "MICA PARK", JTEXT(0x803039A8), 0, -40.0f, -80.0f, { -1 }, { -1 } },
    { 0x64, 1, "MOON", JTEXT(0x803039B4), 0, 10.0f, -140.0f, { -1 }, { 0x2B, -1 } },
    { 0x64, 1, "COBALT QUARRY", JTEXT(0x803039BC), 0, 50.0f, 70.0f, { -1 }, { -1 } },
    { 0x64, 1, "MORAINE CHASE", JTEXT(0x803039CC), 0, -10.0f, -80.0f, { -1 }, { -1 } },
    { 0x64, 1, "MERCURY", JTEXT(0x803039E0), 0, 20.0f, -140.0f, { -1 }, { 0x2C, -1 } },
    { 0x64, 1, "VENUS", JTEXT(0x803039F0), 0, 30.0f, -140.0f, { -1 }, { 0x2D, -1 } },
    { 0x64, 1, "MARS", JTEXT(0x803039FC), 0, 40.0f, -140.0f, { -1 }, { 0x2E, -1 } },
    { 0x64, 1, "NEPTUNE", JTEXT(0x80303A04), 0, 50.0f, -140.0f, { -1 }, { -1 } },
    { 0x00, 0, "CMO INTRO", JTEXT(0x80303A14), 0, 70.0f, 0.0f, { -1 }, { -1 } },
    { 0x64, 1, "SILVER JUNCTION", JTEXT(0x80303A24), 0, 10.0f, -80.0f, { -1 }, { -1 } },
    { 0x64, 1, "END SEQUENCE", JTEXT(0x80303A34), 0, 0.0f, -10.0f, { -1 }, { -1 } },
    { 0x64, 1, "SHUTTLE CLEAR", JTEXT(0x80303A48), 0, 0.0f, -15.0f, { -1 }, { 0x26, -1 } },
    { 0x64, 1, "DARK HEARTLAND", JTEXT(0x80303A58), 0, 0.0f, 90.0f, { -1 }, { -1 } },
    { 0x64, 1, "MAGMA PEAK", JTEXT(0x80303A68), 0, 0.0f, -150.0f, { 0x18, -1 }, { -1 } },
    { 0x0A, 0, "THUNDERFIST", JTEXT(0x80303A74), 0, -40.0f, -20.0f, { -1 }, { 0x3A, -1 } },
    { 0x64, 1, "SALINE WATCH", JTEXT(0x80303A8C), 0, 0.0f, 150.0f, { 0x24, -1 }, { -1 } },
    { 0x64, 1, "BACKLASH", JTEXT(0x80303A98), 0, -10.0f, 10.0f, { -1 }, { 0x0A, -1 } },
    { 0x64, 1, "BISON RIDGE", JTEXT(0x80303AB0), 0, 0.0f, 40.0f, { -1 }, { -1 } },
    { 0x64, 1, "EMBER HAMLET", JTEXT(0x80303AC0), 0, 20.0f, -120.0f, { -1 }, { -1 } },
    { 0x64, 1, "CROMLECH COURT", JTEXT(0x80303AD4), 0, -50.0f, -20.0f, { -1 }, { 0x0E, 0x11, 0x1D, -1 } },
    { 0x64, 1, "LIZARD ISLAND", JTEXT(0x80303AE8), 0, 30.0f, 150.0f, { 0x36, -1 }, { -1 } },
};

u16 D_8020E350[38] = {
    0x0000, 0x0000, 0x0571, 0x0570, 0x0918, 0x0917, 0x090C, 0x090B, 0x070A, 0x0709, 0x0772, 0x0713, 0x0000, 0x0000, 0x0000, 0x0000, 0x0912, 0x0911, 0x0916, 0x0915, 0x090A, 0x0909, 0x0000, 0x0000, 0x0000, 0x0000, 0x090E, 0x090D, 0x0908, 0x0907, 0x091A, 0x0919, 0x0712, 0x0711, 0x0000, 0x0000, 0x0000, 0x0000
};

u16 D_8020E39C[4] = { 0x0A98, 0x0A99, 0x0A9A, 0x0A9B };

/* one per frame buffer (D_8035805C) */
Lights1 D_8020E3A8[2] = {
    gdSPDefLights1(0x10, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0, 0, 1),
    gdSPDefLights1(0x10, 0x10, 0x10, 0xFF, 0xFF, 0xFF, 0, 0, 1),
};

f32 D_8020E3D8 = 0.0f;

/* hd_code: one entry per level in D_802E8F94 (0x44 bytes), as in hd_code 1D990.c */
typedef struct {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 unk1; /* needed (save) progress */
    /* 0x02 */ u8 pad2[0x2A];
    /* 0x2C */ u32 unk2C; /* collectables present (bit per D_8020E350 entry) */
    /* 0x30 */ u8 pad30[0x14];
} LevelInfo;

/* hd_code: save/player records (D_80364AF0, 0x100 bytes), as in hd_code 1D990.c */
typedef struct {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u32 unk10; /* collectables found */
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ u8 rank[0x3C]; /* per level: 0 = locked, 1-5 = medal, ... */
    /* 0x54 */ u8 unk54[0x3C]; /* per level: bit n opens unk18[n] */
    /* 0x90 */ u8 pad90;
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 pad92[0x100 - 0x92];
} Player;

/* hd_code D_803156F8: per-frame-buffer dynamic data (0x21498 bytes each) */
typedef struct {
    /* 0x0000 */ u8 pad0[0x80];
    /* 0x0080 */ Mtx persp;
    /* 0x00C0 */ u8 padC0[0x80];
    /* 0x0140 */ Mtx unk140;
    /* 0x0180 */ u8 pad180[0x1280 - 0x180];
    /* 0x1280 */ Mtx translate;
    /* 0x12C0 */ u8 pad12C0[0x15C0 - 0x12C0];
    /* 0x15C0 */ Vtx stars[0x80 * 4];
    /* 0x35C0 */ u8 pad35C0[0x21498 - 0x35C0];
} Dynamic;

extern Gfx D_01000010[];
extern Gfx D_01000038[];
extern Dynamic D_803156F8[];
extern u16 *D_80358050[];
extern u16 *D_80358058;
extern void *D_8035806C;
extern u16 D_8035807C;
extern f32 D_802E8C84[];
extern s32 D_802FA264; /* debug mode */
extern u16 D_80370C28; /* controller buttons held */
void func_8029A7E4(const char *fmt, ...);
Gfx *func_80274BF0(Dynamic *, Gfx *);
f32 sqrtf(f32);
void func_80259450(void);
void func_80259DC8(void *gfxp, char *str, u16 *wstr, s32 align, s32 fit, s32 x, s32 y, s32 w, s32 h, s32 forward,
                   s32 r0, s32 g0, s32 b0, s32 a0, s32 r1, s32 g1, s32 b1, s32 a1);
Gfx *func_8024C404(Gfx *, Dynamic *, s32 *);
void func_80259C24(Gfx **, Dynamic *);
Gfx *func_80272ED8(Gfx *, s32, s32, s32, u32, s32, f32);
Gfx *func_80274868(Gfx *);
Gfx *func_80274AA4(Gfx *);
s16 func_8025B498(s32, s32, char *, u16 *);
s32 func_8025B300(char *);
f32 func_802574F0(f32); /* sinf */
f32 func_80257514(f32); /* cosf */
extern u8 D_8035805C;
extern void *D_80367738;
extern u8 D_80364AE8;
extern Player D_80364AF0[];
extern LevelInfo D_802E8F94[];
s32 func_8026A828(s32 lo, s32 hi);
u8 func_80264BA4(u8 arg0);
void func_80261FB0(u8 arg0);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
u8 func_80272C5C(u16 *ids, s32 arg1, s32 count, s32 frames, s32 flags, f32 scale);

/* front end */
extern s32 D_80217B6C;
extern u8 D_8021A8F0;
extern Gfx *D_8021A8F8;
extern u8 D_8021A905;
extern f32 D_8021A918;
extern s16 D_8021A924;
extern u8 D_802159F0[];
extern Mtx D_80217B70[];
extern u8 D_8021A904;
extern s8 D_8021A906;
extern s8 D_8021A907;
extern u8 D_8021A908;
extern s8 D_8021A909;
u64 D_8021A940[60];
extern f32 D_8021A91C;
extern f32 D_8021A920;
extern f32 D_8021A934;
extern f32 D_8021A938;
extern s16 D_8021A926;
extern s32 D_8021A928[];
extern s8 D_8021A930;
extern u8 D_8021AB20;
extern f32 D_8021AB28;
extern s16 D_8021AB2C;
extern s8 D_8021AB2E;
extern s32 D_8021AB38;
extern f32 D_8021AB40;
extern f32 D_8021AB44;
extern f32 D_8021AB48;
extern s32 D_80358070;
Gfx *func_801F1568(void);
Gfx *func_801F2000(void);
Gfx *func_801F2428(void);
Gfx *func_801F2E20(void);
void func_801F885C(s32 arg0);
f32 func_801FD6B8(f32 a, f32 b, f32 range);
void func_801FDE50(void);
void func_801FDCA4(Vtx *v, s32 idx, s32 z);
Gfx *func_801FE5D0(Gfx *arg0, Dynamic *dyn);
Gfx *func_801FC5B8(Dynamic *dyn, Gfx *gdl, u8 from, u8 to);
void func_801FDE98(void);
Gfx *func_801F3450(Gfx *, Dynamic *);
extern Gfx *D_8021A8F4;
extern Gfx *D_8021A8FC;
extern Gfx *D_8021A900;
extern u8 D_8021AB21;
extern Player *D_8021AB30;
extern LevelInfo *D_8021AB34;
extern f32 D_8021AB4C;
extern f32 D_8021AB50;
extern f32 D_8021AB54;

void func_801FCF38(Vtx *v, f32 x, f32 y, f32 z, u8 w, u8 h, f32 scale, u8 flip);
void func_801FD484(f32 *arg0, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4, f32 arg5);
s32 func_801FE760(); /* K&R */
Gfx *func_801FA180(Gfx *gdl, Dynamic *dyn, f32 arg2, s8 *arg3);
Gfx *func_801FA74C(Dynamic *dyn, Gfx *gdl, s32 from, s32 to, s8 *out, f32 *lon, s32 arg6, s32 arg7, s32 arg8,
                   s32 arg9, s32 arg10, s32 arg11, s32 arg12);

/* Level-select globe: initialise for the given level. */
void func_801F8530(s32 level) {
    Dynamic *d;
    u32 i;
    GlobeLevel *e;
    f32 lat;
    f32 lon;

    D_8021AB30 = &D_80364AF0[D_80364AE8];
    D_8021AB34 = &D_802E8F94[level];
    D_8021A91C = func_801FD6B8(90.0f, D_8020D810[level].unk14, 180.0f);
    D_8021A920 = func_801FD6B8(90.0f, D_8020D810[level].unk10, 180.0f);
    D_8021A8F4 = func_801F1568();
    D_8021A8FC = func_801F2428();
    D_8021A900 = func_801F2000();
    for (i = 0; i < 2; i++) {
        D_8021A928[i] = D_80358070;
        D_80358070 += 0x17000;
    }
    D_8021A8F8 = func_801F2E20();
    for (i = 0; i < 2; i++) {
        d = &D_803156F8[i];
        guPerspective(&d->persp, &D_8035807C, 45.0f, 4.0f / 3.0f, 100.0f, 20000.0f, 1.0f);
        guTranslate(&d->translate, 0.0f, -50.0f, 0.0f);
    }
    D_80217B6C = 6;
    func_801F885C(level);
    D_8021A904 = level;
    D_8021A908 = level;
    D_8021A926 = 1;
    D_8021A930 = 0;
    D_8021A907 = -1;
    D_8021AB21 = 0;
    D_8021A906 = level;
    D_8021AB28 = 0.0f;
    D_8021AB38 = 0;
    D_8021AB2C = 0;
    D_8021AB40 = 0.915f;
    D_8021AB44 = 0.009f;
    D_8021AB48 = 26100.0f;
    func_801FDE50();
    D_8021AB20 = func_80272C5C(D_8020E39C, 0, 4, 1, 1, 1.0f);
    for (i = 0; i < 60; i++) {
        e = &D_8020D810[i];
        lat = e->unk10;
        lon = e->unk14;
        func_801FD484(&lat, &lon, &e->unk24, &e->unk28, &e->unk2C, 250.0f);
    }
    D_8021AB2E = 0;
}

void func_801F885C(s32 arg0) {
    u8 tune;

    tune = func_80264BA4(arg0);
    D_8021A905 = arg0;
    if (tune != D_80217B6C) {
        D_8021A924 = 0;
        D_8021A918 = 6250.0f;
        if (D_80217B6C != 6) {
            if (D_80217B6C == 3) {
                func_80261FB0(0x13);
            } else if (tune == 3) {
                func_80261FB0(0xC);
            }
        }
    }
    if (D_80217B6C != 6) {
        if (tune == 3 && D_80217B6C == 3) {
            func_80260650(D_80367738, 0x1D, 0);
        } else {
            func_80260650(D_80367738, 0x3F, 0);
        }
    }
    D_80217B6C = tune;
    D_8021AB34 = &D_802E8F94[D_8021A905];
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F8980.s")

/* Level-select info screen: level name and the level's collectable icons. */
Gfx *func_801F9258(Gfx *arg0, Dynamic *dyn, s32 *count) {
    Gfx *gdl = arg0;
    s32 padC0;
    s32 i;
    s32 alt;
    s32 y;
    s32 padA0[5];
    s32 size;

    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetFillColor(gdl++, 0x10001);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPPipeSync(gdl++);
    gDPPipelineMode(gdl++, G_PM_1PRIMITIVE);
    gDPSetColorDither(gdl++, 0x80);
    gdl = func_801FE5D0(gdl, dyn);
    gdl = func_801F3450(gdl, dyn);
    func_80259450();
    {
        u32 h;
        s32 n;
        LevelInfo *info;
        s32 x;
        s32 w;

        size = 0x18;
        h = D_8021AB2C / 9;
        func_80259DC8(dyn, D_8020D810[D_8021A908].name, D_8020D810[D_8021A908].jname, 0, 0xA0, 0, (0x1C - h) / 2 + 0x12,
                      size, h, 1, 0xFF, 0xFF, 0xFF, D_8021AB2C, 0, 0, 0xFF, D_8021AB2C);
        gdl = func_8024C404(gdl, dyn, &n);
        func_80259C24(&gdl, dyn);
        info = &D_802E8F94[D_8021A908];
        gdl = func_80274868(gdl);
        y = 0xDA;
        for (i = 0, alt = 0; i < 0x13 && y > 0x28; i++) {
            if ((info->unk2C & (1 << i)) && (info->type == 1 || (D_8021AB30->unk10 & (1 << i))) &&
                D_8020E350[i * 2] != 0) {
                if (alt) {
                    gdl = func_80272ED8(gdl, D_8021A8F0 + i, 0x16 - (0xFF - D_8021AB2C) / 6, y, D_8021AB2C, 0, 0.8125f);
                } else {
                    gdl = func_80272ED8(gdl, D_8021A8F0 + i, (0xFF - D_8021AB2C) / 6 + 0xF6, y -= 0x2C, D_8021AB2C, 0,
                                        0.8125f);
                }
                alt ^= 1;
            }
        }
        x = func_8025B498(0xA0, size, D_8020D810[D_8021A908].name, D_8020D810[D_8021A908].jname);
        w = (s32) (size * D_802E8C84[0]) * func_8025B300(D_8020D810[D_8021A908].name);
    }
    gdl = func_80274AA4(gdl);
    gSPEndDisplayList(gdl++);
    *count = gdl - arg0;
    return gdl;
}

/* Level-select globe: frame setup, camera matrices, then the globe itself. */
Gfx *func_801F9820(Gfx *arg0, Dynamic *dyn, s32 *count) {
    Gfx *gdl = arg0;

    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000010);
    gSPDisplayList(gdl++, D_01000038);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPSetDepthImage(gdl++, D_80358058);
    gDPPipeSync(gdl++);
    gDPSetRenderMode(gdl++, 0x00507048, 0);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    gSPMatrix(gdl++, &dyn->persp, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80217B70[D_8035805C + 12], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80217B70[D_8035805C + 14], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->translate, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gdl = func_801FA180(gdl, dyn, D_8020E3D8, &D_8021A907);
    gSPEndDisplayList(gdl++);
    *count = gdl - arg0;
    return gdl;
}

/* Level-select globe in close-up (route view), with a debug camera tweak on the controller. */
Gfx *func_801F9B84(Gfx *arg0, Dynamic *dyn, s32 *count) {
    Gfx *gdl = arg0;

    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000010);
    gSPDisplayList(gdl++, D_01000038);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPSetDepthImage(gdl++, D_80358058);
    gDPSetRenderMode(gdl++, 0x00504240, 0);
    gSPTexture(gdl++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
    gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8021AB21);
    gDPSetCombine(gdl++, 0xFF97FF, 0xFF2CFE7F);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    gSPMatrix(gdl++, &dyn->persp, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80217B70[D_8035805C + 12], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_80217B70[D_8035805C + 14], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->translate, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gdl = func_801FC5B8(dyn, gdl, D_8021A904, D_8021A905);
    func_801FDE98();
    gdl = func_80274BF0(&dyn[D_8035805C], gdl);
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    if (D_802FA264) {
        if (D_80370C28 & 0x800) {
            D_8021AB40 += 0.01;
        }
        if (D_80370C28 & 0x400) {
            D_8021AB40 -= 0.01;
        }
        if (D_80370C28 & 0xC00) {
            func_8029A7E4("angd %f\n", D_8021AB40);
        }
        if (D_80370C28 & 0x100) {
            D_8021AB44 += 0.001;
        }
        if (D_80370C28 & 0x200) {
            D_8021AB44 -= 0.001;
        }
        if (D_80370C28 & 0x300) {
            func_8029A7E4("dmm %f\n", D_8021AB44);
        }
        if (D_80370C28 & 0x10) {
            D_8021AB48 += 100.0f;
        }
        if (D_80370C28 & 0x20) {
            D_8021AB48 -= 100.0f;
        }
        if (D_80370C28 & 0x30) {
            func_8029A7E4("mmm %f\n", D_8021AB48);
        }
    }
    *count = gdl - arg0;
    return gdl;
}

/* Draw the routes between open levels; pick the route marker nearest the globe's facing longitude. */
Gfx *func_801FA180(Gfx *arg0, Dynamic *dyn, f32 lon0, s8 *selected) {
    GlobeLevel *e;
    Gfx *gdl = arg0;
    f32 lon;
    f32 best;
    s32 i;
    s32 j;
    s8 out;
    s8 sel;

    best = 180.0f;
    D_8021A909 = 0;
    gDPPipeSync(gdl++);
    for (i = 0; i < 60; i++) {
        D_8021A940[i] = 0;
    }
    for (i = 0; i < 60; i++) {
        e = &D_8020D810[i];
        if ((D_80364AF0[D_80364AE8].rank[i] > 0 && D_80364AF0[D_80364AE8].rank[i] < 6) ? 1 : 0) {
            for (j = 0; j < 8 && e->unk1C[j] != -1; j++) {
                gdl = func_801FA74C(dyn, gdl, i, e->unk1C[j], &out, &lon, 0, 0, 0, 0, 0, 0, 0);
            }
        }
    }
    for (i = 0; i < 60; i++) {
        D_8021A940[i] = 0;
    }
    for (i = 0; i < 60; i++) {
        e = &D_8020D810[i];
        if ((D_80364AF0[D_80364AE8].rank[i] > 0 && D_80364AF0[D_80364AE8].rank[i] < 6) ? 1 : 0) {
            for (j = 0; j < 4 && e->unk18[j] != -1; j++) {
                if (D_80364AF0[D_80364AE8].unk54[i] & (1 << j)) {
                    gdl = func_801FA74C(dyn, gdl, i, e->unk18[j], &out, &lon, 1, 0, 0xFF, 0, 0xFF, 0xFF, 0);
                    if (out != -1) {
                        f32 d;

                        d = func_801FD6B8(lon0, lon, 180.0f);
                        d = (d > 0.0f) ? d : -d;
                        if (d < best) {
                            best = d;
                            sel = out;
                        }
                    }
                }
            }
        }
    }
    for (i = 0; i < 60; i++) {
        e = &D_8020D810[i];
        if ((D_80364AF0[D_80364AE8].rank[i] > 0 && D_80364AF0[D_80364AE8].rank[i] < 6) ? 1 : 0) {
            for (j = 0; j < 8 && e->unk1C[j] != -1; j++) {
                gdl = func_801FA74C(dyn, gdl, i, e->unk1C[j], &out, &lon, 1, 0xFF, 0, 0, 0xFF, 0x80, 0x80);
                if (out != -1) {
                    f32 d;

                    d = func_801FD6B8(lon0, lon, 180.0f);
                    d = (d > 0.0f) ? d : -d;
                    if (d < best) {
                        best = d;
                        sel = out;
                    }
                }
            }
        }
    }
    osWritebackDCache((void *) D_8021A928[D_8035805C], 0x17000);
    if (best < 45.0f) {
        *selected = sel;
    } else {
        *selected = -1;
    }
    return gdl;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FA74C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FC5B8.s")

void func_801FCE74(Vtx *v, u8 level, f32 dlat, f32 dlon, u8 w, u8 h, f32 scale, u8 flip) {
    GlobeLevel *info;
    f32 lat;
    f32 lon;
    f32 x;
    f32 y;
    f32 z;

    info = &D_8020D810[level];
    lat = info->unk10 + dlat;
    lon = info->unk14 + dlon;
    func_801FD484(&lat, &lon, &x, &y, &z, 248.75f);
    func_801FCF38(v, x, y, z, w, h, scale, flip);
}

/* A w x h textured quad lying flat on the globe at (x, y, z), rotated 45 degrees about the normal. */
void func_801FCF38(Vtx *v, f32 x, f32 y, f32 z, u8 w, u8 h, f32 scale, u8 flip) {
    f32 m[4][4];
    f32 ux;
    f32 uy;
    f32 uz;
    f32 vx;
    f32 vy;
    f32 vz;
    f32 len1;
    f32 len2;
    f32 x0;
    f32 x1;
    f32 x2;
    f32 x3;
    f32 y0;
    f32 y1;
    f32 y2;
    f32 y3;
    f32 z0;
    f32 z1;
    f32 z2;
    f32 z3;

    ux = -z;
    uy = 0.0f;
    uz = x;
    vx = x * y;
    vy = -(x * x + z * z);
    vz = y * z;
    len1 = sqrtf(ux * ux + uy * uy + uz * uz) / 50.0 * 2.0 / scale;
    if (len1 < 0.1) {
        len1 = 0.1f;
    }
    ux /= len1;
    uy /= len1;
    uz /= len1;
    len2 = sqrtf(vx * vx + vy * vy + vz * vz) / 50.0 * 2.0 / scale;
    if (len2 < 0.1) {
        len2 = 0.1f;
    }
    vx /= len2;
    vy /= len2;
    vz /= len2;
    guRotateF(m, 45.0f, x, y, z);
    x0 = x + ux;
    y0 = y + uy;
    z0 = z + uz;
    x1 = x + vx;
    y1 = y + vy;
    z1 = z + vz;
    x2 = x - ux;
    y2 = y - uy;
    z2 = z - uz;
    x3 = x - vx;
    y3 = y - vy;
    z3 = z - vz;
    guMtxXFMF(m, x0, y0, z0, &x0, &y0, &z0);
    guMtxXFMF(m, x1, y1, z1, &x1, &y1, &z1);
    guMtxXFMF(m, x2, y2, z2, &x2, &y2, &z2);
    guMtxXFMF(m, x3, y3, z3, &x3, &y3, &z3);
    v[0].v.tc[0] = 0;
    v[0].v.tc[1] = (flip ? 0 : h - 1) << 6;
    v[1].v.tc[0] = (w - 1) << 6;
    v[1].v.tc[1] = (flip ? 0 : h - 1) << 6;
    v[2].v.tc[0] = (w - 1) << 6;
    v[2].v.tc[1] = (flip ? h - 1 : 0) << 6;
    v[3].v.tc[0] = 0;
    v[3].v.tc[1] = (flip ? h - 1 : 0) << 6;
    v[0].v.ob[0] = x0;
    v[0].v.ob[1] = y0;
    v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x1;
    v[1].v.ob[1] = y1;
    v[1].v.ob[2] = z1;
    v[2].v.ob[0] = x2;
    v[2].v.ob[1] = y2;
    v[2].v.ob[2] = z2;
    v[3].v.ob[0] = x3;
    v[3].v.ob[1] = y3;
    v[3].v.ob[2] = z3;
}

/* Latitude/longitude (degrees, normalised in place) to a point on a sphere of radius r. */
void func_801FD484(f32 *lat, f32 *lon, f32 *x, f32 *y, f32 *z, f32 r) {
    if (*lat >= 90.0 || *lat < -90.0) {
        if (*lat >= 90.0) {
            *lat = 180.0 - *lat;
        } else {
            *lat = -180.0 - *lat;
        }
        *lon = *lon + 180.0;
    }
    if (*lon >= 180.0) {
        *lon = *lon - 360.0;
    } else if (*lon < -180.0) {
        *lon = *lon + 360.0;
    }
    *x = func_80257514(-*lon * 0.017453292519943295) * func_80257514(*lat * 0.017453292519943295) * r;
    *y = func_802574F0(*lat * 0.017453292519943295) * r;
    *z = func_802574F0(-*lon * 0.017453292519943295) * func_80257514(*lat * 0.017453292519943295) * r;
}

/* Wrap the difference b - a into [-range, range). */
f32 func_801FD6B8(f32 a, f32 b, f32 range) {
    f32 d;

    d = b - a;
    if (d >= -range && d < range) {
        return d;
    }
    if (d >= range) {
        return -2.0f * range + d;
    }
    return 2.0f * range + d;
}

#define ABS(x) ((x) > 0 ? (x) : -(x))

/* Scroll the star field with the globe's spin; respawn stars that wrap or pass the camera. */
void func_801FD748(void) {
    s32 i;
    s32 k;
    s32 n;
    s32 nx;
    s32 ny;
    s32 nz;
    s32 ox;
    s32 oy;
    s32 bigx;
    s32 bigy;
    s32 minx;
    s32 miny;
    s32 respawn;
    s32 pad[2];
    Vtx *cur;
    Vtx *prev;

    cur = D_803156F8[D_8035805C].stars;
    prev = D_803156F8[D_8035805C ^ 1].stars;
    for (i = 0; i < 0x80; i++) {
        minx = miny = 0x40000000;
        for (k = 0, bigx = bigy = 0; k < 4; k++) {
            n = i * 4 + k;
            nx = cur[n].v.ob[0] = prev[n].v.ob[0] - (D_80217B6C == 3 ? D_8021A934 * 40.0f : 0.0f);
            ny = cur[n].v.ob[1] = prev[n].v.ob[1] + (D_80217B6C == 3 ? D_8021A938 * 40.0f : 0.0f);
            nz = cur[n].v.ob[2] = prev[n].v.ob[2] - D_802159F0[i] * D_8021A918 / 925.0;
            if (ABS(nx) > 4000) {
                bigx = nx;
            }
            if (ABS(ny) > 4000) {
                bigy = ny;
            }
            if (ABS(nx) < minx) {
                minx = ABS(nx);
            }
            if (ABS(ny) < miny) {
                miny = ABS(ny);
            }
        }
        respawn = 0;
        if (bigx != 0 || bigy != 0) {
            for (k = 0; k < 4; k++) {
                n = i * 4 + k;
                ox = cur[n].v.ob[0];
                oy = cur[n].v.ob[1];
                if (bigx != 0) {
                    nx = cur[n].v.ob[0] = func_801FD6B8(0.0f, cur[n].v.ob[0], 3890.0f);
                }
                if (bigy != 0) {
                    ny = cur[n].v.ob[1] = func_801FD6B8(0.0f, cur[n].v.ob[1], 3890.0f);
                }
                if (bigx != 0 && ox * nx > 0) {
                    respawn = 1;
                }
                if (bigy != 0 && oy * ny > 0) {
                    respawn = 1;
                }
            }
        }
        if (respawn || nz < 0 || (minx < 231250.0 / D_8021A918 && miny < 231250.0 / D_8021A918)) {
            func_801FDCA4(cur, i, 25000);
        }
    }
}

/* A randomly placed 0x6D-unit square (4 vertices) at depth z. */
void func_801FDCA4(Vtx *v, s32 idx, s32 z) {
    s32 n;
    s32 x;
    s32 y;

    n = idx * 4;
    D_802159F0[idx] = func_8026A828(0x50, 0xC8);
    x = func_8026A828(-4000, 4000);
    y = func_8026A828(-4000, 4000);
    v[n].v.ob[0] = x - 0x37;
    v[n].v.ob[1] = y - 0x37;
    v[n].v.ob[2] = z;
    v[n + 1].v.ob[0] = x - 0x37;
    v[n + 1].v.ob[1] = y + 0x36;
    v[n + 1].v.ob[2] = z;
    v[n + 2].v.ob[0] = x + 0x36;
    v[n + 2].v.ob[1] = y + 0x36;
    v[n + 2].v.ob[2] = z;
    v[n + 3].v.ob[0] = x + 0x36;
    v[n + 3].v.ob[1] = y - 0x37;
    v[n + 3].v.ob[2] = z;
}

void func_801FDE50(void) {
    D_8021A8F0 = func_80272C5C(D_8020E350, 0, 0x13, 2, 1, 1.0f);
}

void func_801FDE98(void) {
    f32 len;
    f32 x;
    f32 y;
    f32 z;

    len = sqrtf(D_8021AB4C * D_8021AB4C + D_8021AB50 * D_8021AB50 + D_8021AB54 * D_8021AB54);
    if (len < 1.0) {
        len = 1.0f;
    }
    x = D_8021AB4C / len * 120.0f;
    y = D_8021AB50 / len * 120.0f;
    z = D_8021AB54 / len * 120.0f;
    D_8020E3A8[D_8035805C].l[0].l.dir[0] = x;
    D_8020E3A8[D_8035805C].l[0].l.dir[1] = y;
    D_8020E3A8[D_8035805C].l[0].l.dir[2] = z;
    osWritebackDCache(&D_8020E3A8[D_8035805C], sizeof(D_8020E3A8));
}

/* Open every level reachable from a completed one (no prototype in scope for func_801FE760). */
void func_801FE018(u8 rank) {
    s32 i;
    s32 j;
    GlobeLevel *e;
    LevelInfo *info;

    D_8021AB30 = &D_80364AF0[D_80364AE8];
    for (i = 0; i < 60; i++) {
        if ((D_80364AF0[D_80364AE8].rank[i] > 0 && D_80364AF0[D_80364AE8].rank[i] < 6) ? 1 : 0) {
            e = &D_8020D810[i];
            info = &D_802E8F94[i];
            for (j = 0; j < 8 && e->unk1C[j] != -1; j++) {
                if (D_8021AB30->rank[e->unk1C[j]] == 0 && func_801FE760(e->unk1C[j]) == 0) {
                    D_8021AB30->rank[e->unk1C[j]] = rank;
                }
            }
            for (j = 0; j < 4 && e->unk18[j] != -1; j++) {
                if (D_8021AB30->unk54[i] & (1 << j)) {
                    if (D_8021AB30->rank[e->unk18[j]] == 0) {
                        D_8021AB30->rank[e->unk18[j]] = rank;
                    }
                }
            }
        }
    }
}

Gfx *func_801FE238(Gfx *arg0, s32 arg1) {
    Gfx *gdl = arg0;

    gDPSetTextureLOD(gdl++, G_TL_LOD);
    gDPSetCycleType(gdl++, G_CYC_2CYCLE);
    gDPSetRenderMode(gdl++, 0x0C182048, 0);
    if (D_80217B6C == 3) {
        gSPSetLights1(gdl++, D_8020E3A8[D_8035805C]);
        gSPSetGeometryMode(gdl++, G_LIGHTING | G_CULL_BACK | G_SHADING_SMOOTH | G_SHADE);
        gDPSetCombine(gdl++, 0x26A024, 0x1FFC93FC);
    } else {
        gSPSetGeometryMode(gdl++, G_LIGHTING | G_CULL_BACK | G_SHADING_SMOOTH | G_SHADE);
        gDPSetCombine(gdl++, 0x26A1FF, 0x1FFC927C);
    }
    gSPDisplayList(gdl++, D_8021A8F4);
    gDPPipeSync(gdl++);
    gSPClearGeometryMode(gdl++, G_LIGHTING);
    gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8021AB21);
    gDPSetEnvColor(gdl++, 0, 0, 0, D_8021AB21 * 2 / 3);
    gSPDisplayList(gdl++, D_8021A8FC);
    gSPDisplayList(gdl++, D_8021A900);
    gSPEndDisplayList(gdl++);
    return gdl;
}

Gfx *func_801FE5D0(Gfx *arg0, Dynamic *dyn) {
    Gfx *gdl = arg0;

    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gDPSetRenderMode(gdl++, 0x0F0A7008, 0);
    gDPSetCombine(gdl++, 0xFFFFFF, 0xFFFCF279);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    gSPDisplayList(gdl++, D_8021A8F8);
    gDPPipeSync(gdl++);
    return gdl;
}

/* Whether a level stays locked (needs more progress, or a prerequisite level unfinished). */
s32 func_801FE760(level, arg1)
    u8 level;
    s32 arg1;
{
    u8 locked = 0;

    if (D_802E8F94[level].unk1 > D_80364AF0[D_80364AE8].unk91 &&
        ((D_802E8F94[level].type & 0x81) || (level >= 0x2B && level < 0x2F))) {
        locked = 1;
    }
    if (level == 0xA) {
        arg1 = (D_80364AF0[D_80364AE8].rank[0x37] > 0 && D_80364AF0[D_80364AE8].rank[0x37] < 6) ? 1 : 0;
        if (!arg1) {
            locked = 1;
        }
    }
    if (level == 0xF) {
        arg1 = (D_80364AF0[D_80364AE8].rank[0x1C] > 0 && D_80364AF0[D_80364AE8].rank[0x1C] < 6) ? 1 : 0;
        if (!arg1) {
            locked = 1;
        }
    }
    if (level == 0x3A) {
        arg1 = (D_80364AF0[D_80364AE8].rank[0x35] > 0 && D_80364AF0[D_80364AE8].rank[0x35] < 6) ? 1 : 0;
        if (!arg1) {
            locked = 1;
        }
    }
    if (level == 5) {
        arg1 = (D_80364AF0[D_80364AE8].rank[0x07] > 0 && D_80364AF0[D_80364AE8].rank[0x07] < 6) ? 1 : 0;
        if (!arg1) {
            locked = 1;
        }
    }
    if (level == 0x10) {
        arg1 = (D_80364AF0[D_80364AE8].rank[0x13] > 0 && D_80364AF0[D_80364AE8].rank[0x13] < 6) ? 1 : 0;
        if (!arg1) {
            locked = 1;
        }
    }
    if (level == 0) {
        locked = 0;
    }
    return locked;
}
