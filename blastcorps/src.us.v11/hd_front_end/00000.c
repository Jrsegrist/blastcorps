#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#define LEGACY_func_802025D0
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F47B0 ((u8 *) D_802F47B0)
#define D_803156F8 ((FeDyn *) D_803156F8)
#define D_80364AF0 ((Player *) D_80364AF0)
/* end of views */

/* digger_loop.c: the vehicle ("digger") select screen. */

typedef struct {
    u8 pad0[0x2C];
    u32 unk2C; /* mask of vehicles available on this level */
    u8 pad30[0x14];
} LevelInfo; /* 0x44 bytes */

typedef struct {
    u8 pad0[0x10];
    u32 unk10; /* mask of vehicles unlocked */
    u8 pad14[4];
    u8 unk18[0x7A]; /* per level */
    u8 unk92[0x5E]; /* per level: last vehicle used */
    u8 padF0[0x10];
} Player; /* 0x100 bytes */
#ifndef NON_MATCHING
extern s32 D_80367738;
#endif
extern f32 D_802FDAC0[];

extern u16 D_80304954[];

typedef struct {
    u8 pad0[0x14];
    s32 unk14; /* offset of the segment 6 data */
    s32 unk18; /* offset of the matrix table */
} ModelHeader;
extern ModelHeader * N64P D_80210E90[];
extern void * N64P D_80210EE0[][2];
extern Gfx * N64P D_80210F78[][4];
extern Mtx D_802110A8[];
extern Mtx D_80211568[];
extern Mtx D_80211A28;
extern s16 D_80211A68; /* selected slot */
extern s16 D_80211A6A; /* number of slots */
extern f32 D_80211A70[];
extern u8 D_80211AC0[][0x300];
extern u8 D_802153C0[]; /* slot -> vehicle */
extern f32 D_802153D4;
extern f32 D_802153D8;
extern f32 D_802153DC;
extern f32 D_802153E0;
extern u16 D_802153E4;
extern u16 D_802153E6;
extern s32 D_802153E8;
extern s32 D_802153EC;
extern u32 D_802153F0[];

typedef struct {
    u8 unk0;
    f32 unk4;
} Struct80208060;

/* .data, per vehicle (19) */
char * N64P D_80208040 = N64_DPTR("SELECT VEHICLE!");
u16 * N64P D_80208044 = N64_DPTR(D_80304954);
s32 D_80208048 = 0xFFFF0000;
u8 D_8020804C[19] = { 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
Struct80208060 D_80208060[19] = {
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f },
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f },
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f },
};
u8 D_802080F8[19] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 6, 1, 1, 0, 1, 1, 1, 1, 1, 1 };
u8 D_8020810C[19] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 12, 3, 3, 3, 3, 3, 3, 3, 3 };
f32 D_80208120[19] = {
    220.0f, 400.0f, 800.0f, 320.0f, 400.0f, 460.0f, 400.0f, 400.0f, 320.0f, 550.0f,
    320.0f, 320.0f, 500.0f, 320.0f, 320.0f, 320.0f, 320.0f, 320.0f, 320.0f,
};
s16 D_8020816C[19] = {
    -75, -75, -250, -50, -80, -93, -75, -75, -53, -180, -50, -75, -75, -50, -65, -50, -65, -75, -75,
};
u8 D_80208194[19] = {
    0x00, 0x36, 0x50, 0x8C, 0x0B, 0x05, 0x00, 0x20, 0xCE, 0x02, 0x94, 0x72, 0x00, 0xCE, 0x7B, 0xCE, 0x50, 0x72, 0x72,
};

void func_801E74E8(u8);
#ifndef NON_MATCHING
void func_802025D0(u8, u32);
#endif

/* Rare's assert; line numbers are the original digger_loop.c's */
#define DIGGER_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "digger_loop.c", line)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

/* Build the vehicle list for the current level: for each of the 19 vehicles
 * available here and unlocked, set up its model, matrices and slot, then pick
 * the starting slot from the game mode. Returns whether there is a choice. */
s32 func_801E7000(void) {
    s32 i;
    s32 pad;
    s32 x;
    LevelInfo *level;
    u16 unused;
    s32 mid;
    s32 sel;
    s32 slot[19];

    D_80211A68 = 0;
    x = 0;
    level = &D_802E8F94[D_802E8BDC];
    D_80211A6A = 0;
    unused = 0xE73C;
    D_802153E8 = D_803643D4;
    func_8029DEA0();
    for (i = 0; i < 19; i++) {
        slot[i] = -1;
        if ((D_80364AF0[D_80364AEA].unk10 & (1 << i) & level->unk2C) &&
            (D_80364AE8 == D_80364AEA || D_80364EF0[D_80364AEA][D_802E8C44[i]] != 0)) {
            func_80202100(i, (u8 * N64P *) &D_80210E90[D_80211A6A], (u8 * N64P *) D_80210EE0[D_80211A6A], (u8 * N64P *) D_80210F78[D_80211A6A]);
            D_80211A70[i] = x;
            x += 380.0;
            guTranslate(&D_802110A8[D_80211A6A], D_80211A70[i], D_8020816C[i], 0.0f);
            guScale(&D_80211568[D_80211A6A], D_802FDAC0[i], D_802FDAC0[i], D_802FDAC0[i]);
            func_80202270((u8 *) D_80210E90[D_80211A6A], (u32 * N64P *) D_80210EE0[D_80211A6A], D_80211AC0[D_80211A6A]);
            func_802022EC(D_80211AC0[D_80211A6A], D_802080F8[i], D_8020804C[i], D_80208060[i].unk0,
                          D_80208060[i].unk4, D_8020810C[i], 0);
            func_80202380(i);
            D_802153C0[D_80211A6A] = i;
            slot[i] = D_80211A6A;
            D_80211A6A++;
        }
    }
    guRotate(&D_80211A28, 20.0f, 1.0f, 0.0f, 0.0f);
    mid = (D_80211A6A - 1) / 2;
    sel = -1;
    switch (D_80364A90) {
        case 0x80:
            sel = slot[D_80364AF0[D_80364AE8].unk92[D_802E8BDC]];
            break;
        case 4:
        case 0x100:
        case 0x8000000:
            sel = slot[D_803643D4];
            break;
        case 0x4000:
            break;
        default:
            DIGGER_ASSERT(1==0, 139);
            break;
    }
    func_8029A7E4("default %d auto %d\n", mid, sel);
    func_801E74E8(sel == -1 ? mid : sel);
    D_802153D8 = D_802153D4;
    D_802153E0 = D_802153DC * 4.0;
    D_802153E6 = 0;
    D_802153E4 = 0;
    return D_80211A6A > 1;
}

/* Select slot arg0: record it, set the current vehicle from D_802153C0,
 * play its sound and load its two float parameters. */
void func_801E74E8(u8 arg0) {
    D_80211A68 = arg0;
    D_803643D4 = D_802153C0[arg0];
    func_80260A10();
    func_80260650(D_80367738, D_80208194[D_802153C0[arg0]], 0);
    D_802153D4 = D_80211A70[D_802153C0[arg0]];
    D_802153DC = D_80208120[D_802153C0[arg0]];
}

/* Vehicle select frame: reads the stick/buttons (left/right to scroll, A/start
 * to pick, B to go back), starts the previous frame's display list and builds
 * this frame's: title text, the vehicles near the camera (the selected one
 * spinning), and the scroll arrows. */
void func_801E7598(void) {
    u8 sfx[2] = { 0x1D, 0xD0 };
    static s32 D_802081AC = 1;  /* arrow pulse step */
    static s16 D_802081B0 = 0;  /* title pulse */
    static s8 D_802081B4 = 1;   /* title pulse direction */
    s32 i;
    s32 vtx = 0;
    FeDyn *dyn = &D_803156F8[D_8035805C ^ 1];
    Gfx *gdl = dyn->dl;

    func_8028A3E4();
    D_80358080 = 0;
    D_80358084 = 0;
    func_802A5720();
#ifdef NON_MATCHING
    /* The asm takes its param (the texture decoder's type-4/5 table) in $fp,
     * which the game thread's C code never sets: 0 (traced in hd.c's call;
     * this screen needs input, so the demos don't reach it). */
    func_8029E0AC(LEAKED(s8));
#else
    func_8029E0AC();
#endif
    func_8028A470();
    if (D_80358060 >= 11 && !func_802753C0()) {
        if ((D_80370C28 & 0x9000) && !(D_80370C2A & 0x9000)) {
            func_80275390(0x2000);
            func_80260650(D_80367738, 0x1E, 0);
        } else if ((D_80370C28 & 0x4000) && !(D_80370C2A & 0x4000)) {
            func_80260650(D_80367738, 0xDE, 0);
            D_803643D4 = D_802153E8;
            switch (D_80364A88) {
                case 0x80:
                case 0x8000000:
                    D_80364A98 = D_80364A88;
                    break;
                default:
                    D_80364A98 = 0x4000;
                    break;
            }
        }
    }
    if (D_80370C2C < -10 && !func_802753C0()) {
        if (D_80370C2E < -10) {
            if (D_80211A68 > 0) {
                D_802153E4 -= D_80370C2C;
            }
        } else {
            D_802153E4 = 750;
        }
    }
    if ((D_80370C28 & 0x200) && !(D_80370C2A & 0x200) && !func_802753C0()) {
        D_802153E4 = 750;
    }
    if (D_80370C2C > 10 && !func_802753C0()) {
        if (D_80370C2E > 10) {
            if (D_80211A68 < D_80211A6A - 1) {
                D_802153E6 += D_80370C2C;
            }
        } else {
            D_802153E6 = 750;
        }
    }
    if ((D_80370C28 & 0x100) && !(D_80370C2A & 0x100) && !func_802753C0()) {
        D_802153E6 = 750;
    }
    if (D_8035805C == 0 && D_802153E4 >= 750) {
        func_80260650(D_80367738, sfx[D_80211A68 == 0], 0);
        if (D_80211A68 != 0) {
            func_801E74E8(D_80211A68 - 1);
        }
        D_802153E4 -= 750;
    }
    if (D_8035805C == 0 && D_802153E6 >= 750) {
        func_80260650(D_80367738, sfx[D_80211A68 + 1 == D_80211A6A], 0);
        if (D_80211A68 + 1 != D_80211A6A) {
            func_801E74E8(D_80211A68 + 1);
        }
        D_802153E6 -= 750;
    }
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
    func_80259450();
    gdl = func_80200BE0(gdl, (s32) dyn, &D_80358078);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    func_80259CCC((Gfx * N64P *) dyn, (u8 *) D_80208040, D_80208044, 0, 0xA0, 0x50, 0x18, 0x15, 0x15, 1, 0, 0, 0, 0xA0);
    if (PORT_GVI("func_801E7598", D_803156C0) % 20 * 60 / 60 < 16) {
        D_802081B0 += D_802081B4 * 30;
        if (D_802081B0 >= 0x100) {
            D_802081B0 -= 60;
            D_802081B4 = -D_802081B4;
        }
        if (D_802081B0 < 0) {
            D_802081B0 += 60;
            D_802081B4 = -D_802081B4;
        }
        func_80259DC8((Gfx * N64P *) dyn, (u8 *) D_80208040, D_80208044, 0, 0xA0, 0x54, 0x15, 0x15, 0x15, 1, 0xFF, 0xFF - D_802081B0, 0,
                      0xFF, 0xFF, D_802081B0, 0, 0xFF);
    }
    func_80259C24(&gdl, (Mtx *) dyn);
    if (D_80358060 < 2) {
        guPerspective(&dyn->unk1240, &D_8035807C, 45.0f, 1.3333334f, 40.0f, 4000.0f, 1.0f);
    }
    D_802153D8 = D_802153D8 + (D_802153D4 - D_802153D8) * 0.1 * 60.0 / 60.0;
    D_802153E0 = D_802153E0 + (D_802153DC - D_802153E0) * 0.1 * 60.0 / 60.0;
    guLookAtReflect(&dyn->unk140, &dyn->unk3C00, D_802153D8, 1.0f, D_802153E0, D_802153D8, 0.0f, 0.0f, 0.0f, 1.0f,
                    0.0f);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    gSPLookAtX(gdl++, &dyn->unk3C00);
    gSPLookAtY(gdl++, &dyn->unk3C00.l[1]);
    gSPMatrix(gdl++, &dyn->unk1240, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gDPSetEnvColor(gdl++, 0, 0, 0, 0xFF);
    for (i = 0; i < D_80211A6A; i++) {
        u8 flag;
        u8 *seg;
        s32 off;
        Mtx *mtx;

        flag = D_8035805C & (i == D_80211A68);
        if (ABS(D_802153D8 - D_80211A70[D_802153C0[i]]) < 570.0) {
            seg = D_80210E90[i]->unk14 + (u8 *) D_80210E90[i];
            if (D_80211A68 == i) {
                off = *(s32 *) (D_80210E90[i]->unk18 + (u8 *) D_80210E90[i] + 4);
                mtx = (Mtx *) ((u8 *) D_80210EE0[i][flag] + off);
                func_802021FC(D_80211AC0[i], D_80210EE0[i][D_8035805C], D_80210EE0[i][D_8035805C ^ 1]);
                guRotate(mtx, (D_802153F0[i] += 4) % 360, 0.0f, 1.0f, 0.0f);
                osWritebackDCache(mtx, sizeof(Mtx));
                func_802025D0(D_802153C0[i], (f32) ((D_802153F0[i] + 180) % 360) * 11.37778);
            }
            gSPSegment(gdl++, 6, seg);
            gSPSegment(gdl++, 7, D_80210EE0[i][flag]);
            gSPMatrix(gdl++, &D_80211A28, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &D_802110A8[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &D_80211568[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPDisplayList(gdl++, D_80210F78[i][D_8035805C]);
            gSPMatrix(gdl++, &D_80211A28, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &D_802110A8[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &D_80211568[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPDisplayList(gdl++, D_80210F78[i][D_8035805C + 2]);
        }
    }
    D_802153EC += D_802081AC;
    if (D_802081AC < 0) {
        D_802153EC += D_802081AC * 2;
    }
    if (D_802153EC < 0 || D_802153EC >= 16) {
        D_802153EC -= D_802081AC * 2;
        D_802081AC = -D_802081AC;
    }
    if (D_80211A68 + 1 != D_80211A6A) {
        u8 *c = &D_802F47B0[0x98];

        vtx = func_80276130((struct SpriteVtxBuf *) dyn, 2, vtx, D_802153EC + 0x108, 0x20, D_802153EC / 4 + 12, 0x10, c[0], c[1], c[2], c[3],
                            c[4], c[5], c[6], c[7], c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        vtx = func_80276080((struct SpriteVtxBuf *) dyn, 2, vtx, D_802153EC + 0x10C, 0x23, D_802153EC / 4 + 12, 0x10, 0, 0, 0, 0xA0);
        gdl = func_80275DA4(gdl, 0);
        gSPVertex(gdl++, &dyn->unk1E00[0], 8, 0);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    if (D_80211A68 > 0) {
        u8 *c = &D_802F47B0[0x98];

        vtx = func_80276130((struct SpriteVtxBuf *) dyn, 3, vtx, 0x34 - D_802153EC, 0x20, D_802153EC / 4 + 12, 0x10, c[0], c[1], c[2], c[3],
                            c[4], c[5], c[6], c[7], c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        vtx = func_80276080((struct SpriteVtxBuf *) dyn, 3, vtx, 0x30 - D_802153EC, 0x23, D_802153EC / 4 + 12, 0x10, 0, 0, 0, 0xA0);
        gdl = func_80275DA4(gdl, 0);
        gSPVertex(gdl++, &dyn->unk1E00[vtx - 8], 8, 0);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    gdl = func_8026BBD0(gdl, (s32) &D_803156F8[D_8035805C], &D_80358078);
    gdl = func_80274BF0((s32) dyn, gdl);
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    D_80358078 = gdl - dyn->dl;
    for (i = 0; i < D_80358080; i++) {
        osRecvMesg(&D_80315180, NULL, 1);
    }
    for (i = 0; i < D_80358080 - D_80358084; i++) {
        func_802A57AC();
    }
}
