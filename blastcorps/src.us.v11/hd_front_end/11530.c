#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80217B70
#define LEGACY_D_80358058
#define LEGACY_D_80358070
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_803156F8 ((Dynamic *) D_803156F8)
#define D_80358050 ((u16 * *) D_80358050)
#define D_80364AF0 ((Player *) D_80364AF0)
#ifdef NON_MATCHING
#define D_80217B70 ((Mtx *) D_80217B70)
#define D_80358058 (*(u16 * *) &D_80358058)
#define D_80358070 (*(s32 *) &D_80358070)
#endif
/* end of views */


#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) < (b) ? (b) : (a))
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
    /* 0x35C0 */ u8 pad35C0[0x48B0 - 0x35C0];
    /* 0x48B0 */ Gfx gfx[(0x21498 - 0x48B0) / 8];
    /* 0x21498 */ u8 pad21498[0x21498 - 0x48B0 - ((0x21498 - 0x48B0) / 8) * 8];
} Dynamic;

#ifndef NON_MATCHING
extern u16 *D_80358058;
#endif
f32 sqrtf(f32);

/* front end */
extern Gfx *D_8021A8F8;
extern u8 D_8021A905;
extern u16 D_8021A924;
extern u8 D_802159F0[];
#ifndef NON_MATCHING
extern Mtx D_80217B70[];
#endif
extern u8 D_8021A904;
extern s8 D_8021A906;
extern s8 D_8021A907;
extern u8 D_8021A908;
extern u8 D_8021A909;
u64 D_8021A940[60];
extern f32 D_8021A934;
extern f32 D_8021A938;
extern u16 D_8021A926;
extern s32 D_8021A928[];
extern u8 D_8021A930;
extern u8 D_8021AB20;
extern f32 D_8021AB28;
extern s16 D_8021AB2C;
extern u8 D_8021AB2E;
extern void *D_8021AB38;
extern f32 D_8021AB40;
extern f32 D_8021AB44;
extern f32 D_8021AB48;
#ifndef NON_MATCHING
extern s32 D_80358070;
#endif
void func_801F885C(s32 arg0);
f32 func_801FD6B8(f32 a, f32 b, f32 range);
Gfx *func_801FE5D0(Gfx *arg0, Dynamic *dyn);
Gfx *func_801FC5B8(Dynamic *dyn, Gfx *gdl, u8 from, u8 to);
void func_801FDE98(void);
extern Gfx *D_8021A8F4;
extern Gfx *D_8021A8FC;
extern Gfx *D_8021A900;
extern u8 D_8021AB21;
extern Player *D_8021AB30;
extern LevelInfo *D_8021AB34;
extern f32 D_8021AB4C;
extern f32 D_8021AB50;
extern f32 D_8021AB54;
extern Vtx D_8021A840[2][4];
extern s32 D_8021A8C0; /* flight time */
extern f32 D_8021A8C4; /* flight start/end points */
extern f32 D_8021A8C8;
extern f32 D_8021A8CC;
extern f32 D_8021A8D0;
extern f32 D_8021A8D4;
extern f32 D_8021A8D8;
extern f32 D_8021A8DC; /* distance */
extern f32 D_8021A8E0; /* cos of the arc */
extern f32 D_8021A8E4; /* arc angle */
extern f32 D_8021A8E8; /* flight progress 0..1 */
extern f32 D_8021A8EC; /* altitude scale */
extern f32 D_8021A90C;
extern f32 D_8021A910;
extern f32 D_8021A914;
extern s32 D_8021AB24;
extern s32 D_8021AB58;
extern s32 D_8021AB5C;
extern f32 D_8021AB60;
extern f32 D_8021AB64;
extern Gfx *D_8021AB68;
extern Gfx *D_8021AB6C;
Gfx *func_801F9258(Gfx *, Dynamic *, s32 *);
Gfx *func_801F9820(Gfx *, Dynamic *, s32 *);
Gfx *func_801F9B84(Gfx *, Dynamic *, s32 *);
void func_801FD748(void);

void func_801FCF38(Vtx *v, f32 x, f32 y, f32 z, u8 w, u8 h, f32 scale, u8 flip);
Gfx *func_801FA180(Gfx *gdl, Dynamic *dyn, f32 arg2, s8 *arg3);
Gfx *func_801FA74C(Dynamic *dyn, Gfx *arg1, u8 from, u8 to, s8 *out, f32 *lon, u8 curved, u8 r0, u8 g0, u8 b0,
                   u8 r1, u8 g1, u8 b1);

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

/* Level-select: one frame (input, globe spin, fades, draw). */
void func_801F8980(void) {
    s16 ox;
    s16 oy;
    s32 i;
    u32 dist;
    Dynamic *dyn;

    D_80358080 = 0;
    D_80358084 = 0;
    func_802A5720();
    func_8028A3E4();
    if (((s32) D_80358060)) {
        func_80284E54((u64 *) (D_803156F8[D_8035805C].gfx), D_80358078, 2, 0, 1234, 0);
        func_80284E54((u64 *) D_8021AB68, D_8021AB58, 0, 0, 1234, 0);
        func_80284E54((u64 *) D_8021AB6C, D_8021AB5C, 1, 1, 1234, 0);
    } else {
        func_80284E54((u64 *) (D_803156F8[D_8035805C].gfx), D_80358078, 1, 1, 1234, 0);
    }
    D_8035805C ^= 1;
    dyn = &D_803156F8[D_8035805C];
    func_8028A470();
    if (D_8021AB2E || (!func_802753C0() && D_8021A924 == 1 && (D_80370C28 & 0x9000) && !(D_80370C2A & 0x9000))) {
        if (D_8021A926 == 0) {
            func_80260A10();
            func_80260650(D_80367738, 0x1E, 0);
            D_8021A924 = 2;
            func_8029A7E4("selected level %d\n", D_8021A905);
            D_802E8BDC = D_8021A905;
            func_801ECB18();
            D_8021AB2E = 0;
        } else {
            D_8021AB2E = 1;
        }
    }
    if ((D_80370C28 & 0x4000) && !(D_80370C2A & 0x4000) && !func_802753C0() && D_8021A924 == 1) {
        D_80364A87 = 0;
        func_80260650(D_80367738, 0xDE, 0);
        func_80275390(0x4000000000000000);
        D_802E8BDC = D_8021A905;
    }
    dist = D_80370C2C * D_80370C2C + D_80370C2D * D_80370C2D;
    if (dist < 1500) {
        D_8021A930 = 1;
    }
    if (dist != 0) {
        D_8020E3D8 = func_8028BBF4(0, 0, D_80370C2C, -D_80370C2D);
    }
    if (dist > 1500 && D_8021A930 && D_8021A907 >= 0 && D_8021AB2C == 0xFF && D_8021A924 == 1 && yoshiState == 1) {
        D_8021A930 = 0;
        D_8021A904 = D_8021A905;
        func_801F885C(D_8021A907);
        D_8021A926 = 1;
        D_8021AB28 = 1.0f;
        if (D_8021AB38) {
            func_802608C8(D_8021AB38);
        }
    }
    switch (D_8021A924) {
        case 0:
            if (D_8021AB21 + 10 >= 0x100) {
                D_8021AB21 = 0xFF;
            } else {
                D_8021AB21 += 10;
            }
            D_8021A918 -= 200.0f;
            if (D_8021A918 <= 925.0) {
                D_8021A924 = 1;
                D_8021A918 = 925.0f;
                D_8021AB24 = D_803156C4;
            }
            break;
        case 2:
            D_8021A918 -= 50.0f;
            if (D_8021AB21 - 25 < 0) {
                D_8021AB21 = 0;
            } else {
                D_8021AB21 -= 25;
            }
            if (D_8021AB2C - 64 < 0) {
                D_8021AB2C = 0;
            } else {
                D_8021AB2C -= 64;
            }
            if (D_8021A918 <= 300.0) {
                D_80364A98 = func_80299FE8(D_802E8BDC);
            }
            break;
        default:
            D_8021AB21 = 0xFF;
            if (D_8021A926 == 0) {
                if (D_8021AB2C + 32 >= 0x100) {
                    D_8021AB2C = 0xFF;
                } else {
                    D_8021AB2C += 32;
                }
            }
            break;
    }
    if (D_80217B6C != 3) {
        D_8021AB21 = 0;
    }
    ox = D_8021A90C;
    oy = D_8021A910;
    func_801FD484(&D_8021A920, &D_8021A91C, &D_8021A90C, &D_8021A910, &D_8021A914, 925.0f);
    D_8021A934 = D_8021A90C - ox;
    D_8021A938 = D_8021A910 - oy;
    D_8021AB60 = D_8020D810[D_8021A905].unk14;
    D_8021AB64 = D_8020D810[D_8021A905].unk10;
    D_8021A934 = func_801FD6B8(D_8021AB60, D_8021A91C, 180.0f) * MAX2(dist >> 2, 500) / 10000.0f;
    D_8021A938 = func_801FD6B8(D_8021AB64, D_8021A920, 180.0f) * MAX2(dist >> 2, 500) / 10000.0f;
    D_8021A91C -= D_8021A934;
    D_8021A920 -= D_8021A938;
    D_8021AB68 = func_801F9258(dyn->gfx, dyn, &D_80358078);
    D_8021AB6C = func_801F9820(D_8021AB68, dyn, &D_8021AB58);
    func_801F9B84(D_8021AB6C, dyn, &D_8021AB5C);
    func_801FD748();
    for (i = 0; i < D_80358080; i++) {
        osRecvMesg(&D_80315180, NULL, OS_MESG_BLOCK);
    }
    for (i = 0; i < D_80358080 - D_80358084; i++) {
        func_802A57AC();
    }
}

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
    gdl = func_801F3450(gdl, (u8 *) dyn);
    func_80259450();
    {
        u32 h;
        s32 n;
        LevelInfo *info;
        s32 x;
        s32 w;

        size = 0x18;
        h = D_8021AB2C / 9;
        func_80259DC8((Gfx **) dyn, (u8 *) (D_8020D810[D_8021A908].name), D_8020D810[D_8021A908].jname, 0, 0xA0, 0, (0x1C - h) / 2 + 0x12,
                      size, h, 1, 0xFF, 0xFF, 0xFF, D_8021AB2C, 0, 0, 0xFF, D_8021AB2C);
        gdl = func_8024C404(gdl, (DynamicBuf *) dyn, &n);
        func_80259C24(&gdl, (Mtx *) dyn);
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
        x = func_8025B498(0xA0, size, (u8 *) (D_8020D810[D_8021A908].name), (s32) (D_8020D810[D_8021A908].jname));
        w = (s32) (size * D_802E8C84[0]) * func_8025B300((u8 *) (D_8020D810[D_8021A908].name));
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
    gdl = func_80274BF0((s32) &dyn[D_8035805C], gdl);
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
#ifdef NON_MATCHING
    s8 sel = 0; /* only stored when best < 45, which some entry setting sel guarantees */
#else
    s8 sel;
#endif

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

/*
 * TODO: func_801FA74C (1947 insns) draws one route between two levels: an arc of line segments
 * over the globe (gSPVertex + gSPLineW3D), coloured from (r0,g0,b0) to (r1,g1,b1), brightened
 * towards the camera, with the route-open fade timed by D_8021AB28 and a sound on completion.
 * The draft below is structurally complete (same frame 0x130, same locals, same rodata order)
 * but compiles to 1953 instructions with register-allocation differences throughout: IDO picks
 * other FP registers from the very first FP op (scale = 1.0f gets $f10, target $f4), and in the
 * func_8027690C call (i != 0 branch) x/y stay cached in FP registers, which then defeats the CSE
 * of D_8035805C, &D_80217B70 and dyn between the two Mtx arguments (+3 insns per call site).
 * Tried: statement/declaration-initializer orders, matrix spellings (array/struct/pointer
 * arithmetic), comma/one-line xyz, a pos[3] array, operand orders. Fixed along the way: the
 * MAX with D_8021AB28 is "(AB28 > e) ? AB28 : e", the colour factors are (f32) u8, *lon gets an
 * int ternary, the acos sign goes first (as in func_801FC5B8).
 */
#ifdef NON_MATCHING
/* (The NON_MATCHING build uses this draft; checks in tools_port/checks/fe_11530.txt.) */
/* D_80217B70 seen as a struct (the same address) */
typedef struct {
    Mtx pad[12];
    Mtx view[2];
    Mtx rot[2];
} GlobeMtx;
#define D_80217B70x (*(GlobeMtx *) D_80217B70)

#define FABS(x) ((x) > 0 ? (x) : -(x))

/* One route between two levels: an arc of line segments over the globe, coloured r0..r1 along it. */
Gfx *func_801FA74C(Dynamic *dyn, Gfx *arg1, u8 from, u8 to, s8 *out, f32 *lon, u8 curved, u8 r0, u8 g0, u8 b0,
                   u8 r1, u8 g1, u8 b1) {
    Vtx *v;
    Vtx *vbase;
    Gfx *gdl;
    s32 i;
    s32 n;
    s32 steps;
    u8 unk117;
    u8 found;
    f32 progress;
    GlobeLevel *e1;
    GlobeLevel *e2;
    f32 x;
    f32 y;
    f32 z;
    f32 dist;
    s16 sx1;
    s16 sy1;
    s16 sx0;
    s16 sy0;
    f32 x0;
    f32 x1;
    f32 y0;
    f32 y1;
    f32 z0;
    f32 z1;
    f32 a;
    f32 b;
    f32 cosang;
    f32 angle;
    f32 t;
    f32 scale;
    f32 bright;
    u8 loaded;
    u8 lit;
    u32 extra;

    unk117 = 1;
    lit = 0;
    gdl = arg1;
    scale = 1.0f;
    if (to == D_8021A905 || D_80364AF0[D_80364AE8].rank[from] == 0) {
        u8 tmp;

        tmp = to;
        to = from;
        from = tmp;
    }
    if (D_8021A940[from] & ((u64) 1 << to)) {
        found = 1;
    } else {
        found = 0;
    }
    D_8021A940[from] |= (u64) 1 << to;
    D_8021A940[to] |= (u64) 1 << from;
    *out = -1;
    if (found || func_801FE760(to)) {
        return arg1;
    }
    if (func_80264BA4(from) == 3 || func_80264BA4(to) == 3) {
        lit = 1;
    }
    if (from == D_8021A905) {
        *out = to;
    }
    if (func_80264BA4(from) == 3 && func_80264BA4(to) == 3) {
        if (D_80364A87 &&
            (D_80364AF0[D_80364AE8].rank[to] == 0 ||
             ((D_80364A87 & 1) && from == D_8021A905 &&
              !((D_80364AF0[D_80364AE8].rank[to] > 0 && D_80364AF0[D_80364AE8].rank[to] < 6) ? 1 : 0)))) {
            if (D_8021A924) {
                progress = MIN2(1.0, (D_8021AB28 > (f32) (D_803156C4 - D_8021AB24) * 60.0 / 60.0 / 90.0) ? D_8021AB28 : (f32) (D_803156C4 - D_8021AB24) * 60.0 / 60.0 / 90.0);
                if (progress == 1.0 && D_8021AB38) {
                    func_802608C8(D_8021AB38);
                } else if (!D_8021AB38 && progress != 1.0 && D_80217B6C == 3) {
                    func_80260650(D_80367738, 0x7C, &D_8021AB38);
                }
            } else {
                progress = 0.0f;
            }
        } else {
            progress = 1.0f;
        }
    } else {
        progress = 1.0f;
    }
    e1 = &D_8020D810[from];
    e2 = &D_8020D810[to];
    x0 = e1->unk24;
    y0 = e1->unk28;
    z0 = e1->unk2C;
    x1 = e2->unk24;
    y1 = e2->unk28;
    z1 = e2->unk2C;
    dist = sqrtf((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0));
    steps = MAX2(MIN2(dist / 32.0, 15.0), 3.0);
    n = steps + 1;
    v = vbase = (Vtx *) D_8021A928[D_8035805C] + D_8021A909 * 16;
    cosang = (x0 * x1 + y0 * y1 + z0 * z1) / 250.0 / 250.0;
    angle = 90.0 - ((cosang >= 0.0f) ? 1 : -1) * func_802AD7D4(((cosang > 0.0f) ? cosang : -cosang) * 65535.0) /
                       16.0 / 11.377777;
    if (angle >= 180.0) {
        angle -= 180.0;
    }
    if (angle < -180.0) {
        angle += 180.0;
    }
    angle *= 0.017453292519943295;
    loaded = 0;
    for (i = 0; i < n; i++, v++) {
        t = MIN2(progress, 1.0 / (n - 1) * (f32) i);
        a = func_802574F0((1.0 - t) * angle) / func_802574F0(angle);
        b = func_802574F0(t * angle) / func_802574F0(angle);
        if (curved) {
            scale = func_802574F0(t * 3.141592653) * steps / 64.0 + 1.0;
        }
        x = (a * x0 + b * x1) * scale;
        y = (a * y0 + b * y1) * scale;
        z = (a * z0 + b * z1) * scale;
        if (*out != -1 && i < 2) {
            if (D_80217B6C == 3) {
                if (i != 0) {
                    func_8027690C(dyn, x, y, z, &sx1, &sy1, &D_80217B70x.view[D_8035805C],
                                  &D_80217B70x.rot[D_8035805C], &dyn->translate, 1.0f);
                    *lon = func_8028BBF4(sx0, sy0, sx1, sy1);
                } else {
                    func_8027690C(dyn, x, y, z, &sx0, &sy0, &D_80217B70x.view[D_8035805C],
                                  &D_80217B70x.rot[D_8035805C], &dyn->translate, 1.0f);
                }
            } else if (i != 0) {
                *lon = (from < to) ? 0 : 180;
            }
        }
        v->v.ob[0] = x;
        v->v.ob[1] = y;
        v->v.ob[2] = z;
        bright = MAX2((x * D_8021A90C + y * D_8021A910 + z * D_8021A914) / scale / 250000.0, 0.0);
        if (curved) {
            extra = 1.0 / MAX2(FABS(bright - D_8021AB40), 0.001) * D_8021AB44 * D_8021AB44 * D_8021AB48;
        } else {
            extra = 0;
        }
        v->v.cn[0] = MIN2(255.0, (FABS(t - 0.5) * (f32) r0 + (0.5 - FABS(t - 0.5)) * (f32) r1) * 2.0 + extra);
        v->v.cn[1] = MIN2(255.0, (FABS(t - 0.5) * (f32) g0 + (0.5 - FABS(t - 0.5)) * (f32) g1) * 2.0 * bright + extra);
        v->v.cn[2] = MIN2(255.0, (FABS(t - 0.5) * (f32) b0 + (0.5 - FABS(t - 0.5)) * (f32) b1) * 2.0 * (1.0 - bright) + extra);
        v->v.cn[3] = D_8021AB21 * bright / (2 - curved);
        if (i != 0 && !found && lit && bright > 0.0f) {
            if (!loaded) {
                gSPVertex(gdl++, vbase, n, 0);
                loaded = 1;
            }
            gSPLineW3D(gdl++, i - 1, i, 2.0 - MIN2(2.0, D_8021A918 / 1000.0f / (1.0 + bright / 2.0f)), 0);
        }
    }
    D_8021A909++;
    return gdl;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FA74C.s")
#endif

/* The plane icon flying along the great circle between two levels. */
Gfx *func_801FC5B8(Dynamic *dyn, Gfx *arg1, u8 from, u8 to) {
    f32 a;
    f32 b;
    Gfx *gdl;
    Vtx *v;
    GlobeLevel *e1;
    GlobeLevel *e2;

    gdl = arg1;
    v = D_8021A840[D_8035805C];
    switch (D_8021A926) {
        case 0:
            break;
        case 1:
            e1 = &D_8020D810[from];
            e2 = &D_8020D810[to];
            D_8021A8C4 = e1->unk24;
            D_8021A8CC = e1->unk28;
            D_8021A8D4 = e1->unk2C;
            D_8021A8C8 = e2->unk24;
            D_8021A8D0 = e2->unk28;
            D_8021A8D8 = e2->unk2C;
            D_8021A8DC = sqrtf((D_8021A8C8 - D_8021A8C4) * (D_8021A8C8 - D_8021A8C4) +
                               (D_8021A8D0 - D_8021A8CC) * (D_8021A8D0 - D_8021A8CC) +
                               (D_8021A8D8 - D_8021A8D4) * (D_8021A8D8 - D_8021A8D4));
            D_8021A8C0 = MAX2(MIN2(D_8021A8DC / 32.0, 15.0), 3.0);
            if (from != to) {
                D_8021A926 = 2;
            } else {
                D_8021A926 = 0;
            }
            D_8021A8E8 = 0.0f;
            D_8021A8E0 = (D_8021A8C4 * D_8021A8C8 + D_8021A8CC * D_8021A8D0 + D_8021A8D4 * D_8021A8D8) / 250.0 / 250.0;
            D_8021A8E4 = 90.0 - ((D_8021A8E0 >= 0.0f) ? 1 : -1) * func_802AD7D4(((D_8021A8E0 > 0.0f) ? D_8021A8E0 : -D_8021A8E0) * 65535.0) / 16.0 / 11.377777;
            if (D_8021A8E4 >= 180.0) {
                D_8021A8E4 -= 180.0;
            }
            if (D_8021A8E4 < -180.0) {
                D_8021A8E4 += 180.0;
            }
            D_8021A8E4 *= 0.017453292519943295;
            break;
        case 2:
            D_8021A8E8 += 0.4 / D_8021A8C0;
            if (D_8021A8E8 > 1.0) {
                D_8021A926 = 0;
                D_8021A8E8 = 1.0f;
            }
            D_8021AB2C = ((0.5 - D_8021A8E8 > 0.0) ? 0.5 - D_8021A8E8 : -(0.5 - D_8021A8E8)) * 510.0;
            if (D_8021A8E8 >= 0.5) {
                D_8021A908 = D_8021A905;
            }
            break;
    }
    a = func_802574F0((1.0 - D_8021A8E8) * D_8021A8E4) / func_802574F0(D_8021A8E4);
    b = func_802574F0(D_8021A8E8 * D_8021A8E4) / func_802574F0(D_8021A8E4);
    D_8021A8EC = func_802574F0(D_8021A8E8 * 3.141592653) * D_8021A8C0 / 64.0 + 1.0;
    D_8021AB4C = (a * D_8021A8C4 + b * D_8021A8C8) * D_8021A8EC;
    D_8021AB50 = (a * D_8021A8CC + b * D_8021A8D0) * D_8021A8EC;
    D_8021AB54 = (a * D_8021A8D4 + b * D_8021A8D8) * D_8021A8EC;
    func_801FCF38(v, D_8021AB4C, D_8021AB50, D_8021AB54, 0x20, 0x20, 1.75f, 1);
    osWritebackDCache(v, sizeof(Vtx) * 4);
    gDPLoadTextureBlock(gdl++, D_80215A70[D_803156C4 / 3 % 3], G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gdl++, v, 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 2, 3, 0, 0);
    return gdl;
}

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

/* Whether a level stays locked (needs more progress, or a prerequisite level unfinished).
 * K&R in the matching build, where the unused second parameter is the
 * scratch the checks below reuse (no caller passes it). */
#ifdef NON_MATCHING
s32 func_801FE760(u8 level) {
    s32 arg1;
    u8 locked = 0;
#else
s32 func_801FE760(level, arg1)
    u8 level;
    s32 arg1;
{
    u8 locked = 0;
#endif

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
