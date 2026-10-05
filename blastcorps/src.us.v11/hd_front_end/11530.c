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
    /* 0x18 */ u8 unk18[4]; /* 0xFF-terminated */
    /* 0x1C */ u8 unk1C[8]; /* 0xFF-terminated */
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
} LevelInfo; /* size 0x30 */

#define JTEXT(addr) ((u16 *) (addr))

LevelInfo D_8020D810[60] = {
    { 0x64, 1, "SIMIAN ACRES", JTEXT(0x80303700), 0, 0.0f, 0.0f, { 0xFF }, { 0x1C, 0x13, 0x03, 0x37, 0x32, 0xFF } },
    { 0x64, 1, "ANGEL CITY", JTEXT(0x8030370C), 0, -20.0f, -120.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "OUTLAND FARM", JTEXT(0x80303720), 0, 20.0f, 60.0f, { 0x25, 0x1B, 0xFF }, { 0x05, 0x0D, 0x09, 0xFF } },
    { 0x64, 1, "BLACKRIDGE WORKS", JTEXT(0x80303734), 0, -20.0f, -20.0f, { 0x14, 0x06, 0xFF }, { 0x1D, 0x35, 0x0F, 0x0A, 0xFF } },
    { 0x64, 1, "GLORY CROSSING", JTEXT(0x80303748), 0, 70.0f, 180.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "SHUTTLE GULLY", JTEXT(0x8030375C), 0, 50.0f, 20.0f, { 0x29, 0xFF }, { 0x04, 0x21, 0x02, 0xFF } },
    { 0x64, 1, "SALVAGE WHARF", JTEXT(0x8030376C), 0, -30.0f, -10.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "SKYFALL", JTEXT(0x8030377C), 0, 40.0f, 20.0f, { 0xFF }, { 0x05, 0xFF } },
    { 0x64, 1, "TWILIGHT FOUNDRY", JTEXT(0x80303794), 0, 10.0f, 30.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "CRYSTAL RIFT", JTEXT(0x803037A8), 0, 20.0f, 120.0f, { 0x33, 0x1F, 0xFF }, { 0xFF } },
    { 0x64, 1, "ARGENT TOWERS", JTEXT(0x803037B8), 0, -20.0f, 20.0f, { 0x15, 0x22, 0xFF }, { 0x0D, 0x11, 0x10, 0x03, 0xFF } },
    { 0x64, 1, "SKERRIES", JTEXT(0x803037CC), 0, 70.0f, 0.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "DIAMOND SANDS", JTEXT(0x803037D8), 0, -70.0f, 110.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "EBONY COAST", JTEXT(0x803037EC), 0, -20.0f, 60.0f, { 0x20, 0xFF }, { 0x1A, 0x11, 0x02, 0xFF } },
    { 0x64, 1, "OYSTER HARBOR", JTEXT(0x803037FC), 0, -70.0f, -110.0f, { 0x17, 0xFF }, { 0xFF } },
    { 0x64, 1, "CARRICK POINT", JTEXT(0x8030380C), 0, 20.0f, -20.0f, { 0x19, 0x1E, 0xFF }, { 0x21, 0x12, 0x10, 0x03, 0xFF } },
    { 0x64, 1, "HAVOC DISTRICT", JTEXT(0x8030381C), 0, 20.0f, 20.0f, { 0x16, 0x08, 0xFF }, { 0x07, 0x02, 0x0F, 0x0A, 0xFF } },
    { 0x64, 1, "IRONSTONE MINE", JTEXT(0x8030382C), 0, -50.0f, 20.0f, { 0xFF }, { 0x0A, 0x0C, 0x0D, 0x3A, 0xFF } },
    { 0x64, 1, "BEETON TRACKS", JTEXT(0x80303844), 0, 20.0f, -60.0f, { 0x30, 0xFF }, { 0x1D, 0x0F, 0x21, 0x39, 0xFF } },
    { 0x00, 0, "J-BOMB", JTEXT(0x80303854), 0, 10.0f, 10.0f, { 0xFF }, { 0x10, 0xFF } },
    { 0x64, 1, "JADE PLATEAU", JTEXT(0x80303864), 0, -10.0f, -30.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MARINE QUARTER", JTEXT(0x80303874), 0, -30.0f, 10.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "COOTER CREEK", JTEXT(0x80303884), 0, 30.0f, 10.0f, { 0x0B, 0xFF }, { 0xFF } },
    { 0x64, 1, "GIBBON'S GATE", JTEXT(0x80303890), 0, -50.0f, 180.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "BABOON CATACOMB", JTEXT(0x803038A0), 0, 30.0f, -150.0f, { 0xFF }, { 0x25, 0xFF } },
    { 0x64, 1, "SLEEK STREETS", JTEXT(0x803038B0), 0, 10.0f, -30.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "OBSIDIAN MILE", JTEXT(0x803038C0), 0, -20.0f, 120.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "CORVINE BLUFF", JTEXT(0x803038D0), 0, 10.0f, 50.0f, { 0x38, 0xFF }, { 0xFF } },
    { 0x28, 1, "SIDESWIPE", JTEXT(0x803038E0), 0, 10.0f, -10.0f, { 0xFF }, { 0x0F, 0xFF } },
    { 0x64, 1, "ECHO MARCHES", JTEXT(0x803038F8), 0, -20.0f, -60.0f, { 0x2A, 0x27, 0xFF }, { 0x01, 0x3A, 0x12, 0xFF } },
    { 0x64, 1, "KIPLING PLANT", JTEXT(0x80303904), 0, 30.0f, -10.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "FALCHION FIELD", JTEXT(0x80303918), 0, 45.0f, 100.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MORGAN HALL", JTEXT(0x80303930), 0, -10.0f, 50.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "TEMPEST CITY", JTEXT(0x8030393C), 0, 50.0f, -20.0f, { 0x23, 0xFF }, { 0x12, 0x05, 0x04, 0xFF } },
    { 0x64, 1, "ORION PLAZA", JTEXT(0x80303950), 0, -10.0f, 30.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "GLANDER'S RANCH", JTEXT(0x80303960), 0, 50.0f, -70.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "DAGGER PASS", JTEXT(0x80303974), 0, -20.0f, 180.0f, { 0x34, 0xFF }, { 0xFF } },
    { 0x64, 1, "GEODE SQUARE", JTEXT(0x80303980), 0, 50.0f, 180.0f, { 0x3B, 0xFF }, { 0xFF } },
    { 0x64, 1, "SHUTTLE ISLAND", JTEXT(0x80303990), 0, 0.0f, -140.0f, { 0xFF }, { 0x28, 0xFF } },
    { 0x64, 1, "MICA PARK", JTEXT(0x803039A8), 0, -40.0f, -80.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MOON", JTEXT(0x803039B4), 0, 10.0f, -140.0f, { 0xFF }, { 0x2B, 0xFF } },
    { 0x64, 1, "COBALT QUARRY", JTEXT(0x803039BC), 0, 50.0f, 70.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MORAINE CHASE", JTEXT(0x803039CC), 0, -10.0f, -80.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MERCURY", JTEXT(0x803039E0), 0, 20.0f, -140.0f, { 0xFF }, { 0x2C, 0xFF } },
    { 0x64, 1, "VENUS", JTEXT(0x803039F0), 0, 30.0f, -140.0f, { 0xFF }, { 0x2D, 0xFF } },
    { 0x64, 1, "MARS", JTEXT(0x803039FC), 0, 40.0f, -140.0f, { 0xFF }, { 0x2E, 0xFF } },
    { 0x64, 1, "NEPTUNE", JTEXT(0x80303A04), 0, 50.0f, -140.0f, { 0xFF }, { 0xFF } },
    { 0x00, 0, "CMO INTRO", JTEXT(0x80303A14), 0, 70.0f, 0.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "SILVER JUNCTION", JTEXT(0x80303A24), 0, 10.0f, -80.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "END SEQUENCE", JTEXT(0x80303A34), 0, 0.0f, -10.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "SHUTTLE CLEAR", JTEXT(0x80303A48), 0, 0.0f, -15.0f, { 0xFF }, { 0x26, 0xFF } },
    { 0x64, 1, "DARK HEARTLAND", JTEXT(0x80303A58), 0, 0.0f, 90.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "MAGMA PEAK", JTEXT(0x80303A68), 0, 0.0f, -150.0f, { 0x18, 0xFF }, { 0xFF } },
    { 0x0A, 0, "THUNDERFIST", JTEXT(0x80303A74), 0, -40.0f, -20.0f, { 0xFF }, { 0x3A, 0xFF } },
    { 0x64, 1, "SALINE WATCH", JTEXT(0x80303A8C), 0, 0.0f, 150.0f, { 0x24, 0xFF }, { 0xFF } },
    { 0x64, 1, "BACKLASH", JTEXT(0x80303A98), 0, -10.0f, 10.0f, { 0xFF }, { 0x0A, 0xFF } },
    { 0x64, 1, "BISON RIDGE", JTEXT(0x80303AB0), 0, 0.0f, 40.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "EMBER HAMLET", JTEXT(0x80303AC0), 0, 20.0f, -120.0f, { 0xFF }, { 0xFF } },
    { 0x64, 1, "CROMLECH COURT", JTEXT(0x80303AD4), 0, -50.0f, -20.0f, { 0xFF }, { 0x0E, 0x11, 0x1D, 0xFF } },
    { 0x64, 1, "LIZARD ISLAND", JTEXT(0x80303AE8), 0, 30.0f, 150.0f, { 0x36, 0xFF }, { 0xFF } },
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

/* hd_code */
f32 sqrtf(f32);
extern u8 D_8035805C;
extern void *D_80367738;
extern u8 D_80364AE8;
extern u8 D_80364AF0[];
extern u8 D_802E8F94[];
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
extern void *D_8021AB34;
extern f32 D_8021AB4C;
extern f32 D_8021AB50;
extern f32 D_8021AB54;

void func_801FCF38(Vtx *v, f32 x, f32 y, f32 z, u8 w, u8 h, f32 scale, u8 flip);
void func_801FD484(f32 *arg0, f32 *arg1, f32 *arg2, f32 *arg3, f32 *arg4, f32 arg5);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F8530.s")

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
    D_8021AB34 = &D_802E8F94[D_8021A905 * 0x44];
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F8980.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F9258.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F9820.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801F9B84.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FA180.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FA74C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FC5B8.s")

void func_801FCE74(Vtx *v, u8 level, f32 dlat, f32 dlon, u8 w, u8 h, f32 scale, u8 flip) {
    LevelInfo *info;
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FCF38.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FD484.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FD748.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FDCA4.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FE018.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FE238.s")

Gfx *func_801FE5D0(Gfx *arg0, s32 arg1) {
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/11530/func_801FE760.s")
