#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gSfxBank
#define LEGACY_func_802025D0
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gLevelInfo ((LevelInfo *) gLevelInfo)
#define D_802F47B0 ((u8 *) D_802F47B0)
#define gDynamicBufs ((FeDyn *) gDynamicBufs)
#define players ((Player *) players)
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
extern s32 gSfxBank;
#endif
extern f32 D_802FDAC0[];

extern u16 D_80304954[];

typedef struct {
    u8 pad0[0x14];
    s32 unk14; /* offset of the segment 6 data */
    s32 unk18; /* offset of the matrix table */
} ModelHeader;
extern ModelHeader * N64P gDiggerModels[];
extern void * N64P gDiggerMtxBufs[][2];
extern Gfx * N64P gDiggerDls[][4];
extern Mtx gDiggerTransMtx[];
extern Mtx gDiggerScaleMtx[];
extern Mtx gDiggerTiltMtx;
extern s16 gDiggerSel; /* selected slot */
extern s16 gDiggerCount; /* number of slots */
extern f32 gDiggerVehicleX[];
extern u8 D_80211AC0[][0x300];
extern u8 gDiggerSlotVehicle[]; /* slot -> vehicle */
extern f32 gDiggerCamXGoal;
extern f32 gDiggerCamX;
extern f32 gDiggerCamZGoal;
extern f32 gDiggerCamZ;
extern u16 gDiggerScrollLeft;
extern u16 gDiggerScrollRight;
extern s32 gDiggerPrevVehicle;
extern s32 gDiggerArrowPulse;
extern u32 gDiggerSpin[];

typedef struct {
    u8 unk0;
    f32 unk4;
} Struct80208060;

/* .data, per vehicle (19) */
char * N64P gDiggerTitle = N64_DPTR("SELECT VEHICLE!");
u16 * N64P gDiggerTitleGlyphs = N64_DPTR(D_80304954);
s32 D_80208048 = 0xFFFF0000;
u8 D_8020804C[19] = { 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
Struct80208060 D_80208060[19] = {
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f },
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f },
    { 0, 0.0f }, { 0, 0.0f }, { 1, 0.5f }, { 0, 0.0f }, { 0, 0.0f },
};
u8 D_802080F8[19] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 6, 1, 1, 0, 1, 1, 1, 1, 1, 1 };
u8 D_8020810C[19] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 12, 3, 3, 3, 3, 3, 3, 3, 3 };
f32 gDiggerViewDist[19] = {
    220.0f, 400.0f, 800.0f, 320.0f, 400.0f, 460.0f, 400.0f, 400.0f, 320.0f, 550.0f,
    320.0f, 320.0f, 500.0f, 320.0f, 320.0f, 320.0f, 320.0f, 320.0f, 320.0f,
};
s16 gDiggerModelY[19] = {
    -75, -75, -250, -50, -80, -93, -75, -75, -53, -180, -50, -75, -75, -50, -65, -50, -65, -75, -75,
};
u8 gDiggerSelectSound[19] = {
    0x00, 0x36, 0x50, 0x8C, 0x0B, 0x05, 0x00, 0x20, 0xCE, 0x02, 0x94, 0x72, 0x00, 0xCE, 0x7B, 0xCE, 0x50, 0x72, 0x72,
};

void fe_digger_select(u8);
#ifndef NON_MATCHING
void func_802025D0(u8, u32);
#endif

/* Rare's assert; line numbers are the original digger_loop.c's */
#define DIGGER_ASSERT(EX, line) \
    if (!(EX)) \
    debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "digger_loop.c", line)

#define ABS(x) ((x) > 0.0f ? (x) : -(x))

/* The level's vehicle-select row; 1 if there is a choice (0x801E7000).
 * Build the vehicle list for the current level: for each of the 19 vehicles
 * available here and unlocked, set up its model, matrices and slot, then pick
 * the starting slot from the game mode. Returns whether there is a choice. */
s32 fe_digger_init(void) {
    s32 i;
    s32 pad;
    s32 x;
    LevelInfo *level;
    u16 unused;
    s32 mid;
    s32 sel;
    s32 slot[19];

    gDiggerSel = 0;
    x = 0;
    level = &gLevelInfo[levelno];
    gDiggerCount = 0;
    unused = 0xE73C;
    gDiggerPrevVehicle = D_803643D4;
    anim_tex_init();
    for (i = 0; i < 19; i++) {
        slot[i] = -1;
        if ((players[playerNumberAtStart].unk10 & (1 << i) & level->unk2C) &&
            (playerNumber == playerNumberAtStart || gBestTimes[playerNumberAtStart][D_802E8C44[i]] != 0)) {
            fe_load_model(i, (u8 * N64P *) &gDiggerModels[gDiggerCount], (u8 * N64P *) gDiggerMtxBufs[gDiggerCount], (u8 * N64P *) gDiggerDls[gDiggerCount]);
            gDiggerVehicleX[i] = x;
            x += 380.0;
            guTranslate(&gDiggerTransMtx[gDiggerCount], gDiggerVehicleX[i], gDiggerModelY[i], 0.0f);
            guScale(&gDiggerScaleMtx[gDiggerCount], D_802FDAC0[i], D_802FDAC0[i], D_802FDAC0[i]);
            func_80202270((u8 *) gDiggerModels[gDiggerCount], (u32 * N64P *) gDiggerMtxBufs[gDiggerCount], D_80211AC0[gDiggerCount]);
            func_802022EC(D_80211AC0[gDiggerCount], D_802080F8[i], D_8020804C[i], D_80208060[i].unk0,
                          D_80208060[i].unk4, D_8020810C[i], 0);
            func_80202380(i);
            gDiggerSlotVehicle[gDiggerCount] = i;
            slot[i] = gDiggerCount;
            gDiggerCount++;
        }
    }
    guRotate(&gDiggerTiltMtx, 20.0f, 1.0f, 0.0f, 0.0f);
    mid = (gDiggerCount - 1) / 2;
    sel = -1;
    switch (gGameMode) {
        case 0x80:
            sel = slot[players[playerNumber].unk92[levelno]];
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
    debug_printf("default %d auto %d\n", mid, sel);
    fe_digger_select(sel == -1 ? mid : sel);
    gDiggerCamX = gDiggerCamXGoal;
    gDiggerCamZ = gDiggerCamZGoal * 4.0;
    gDiggerScrollRight = 0;
    gDiggerScrollLeft = 0;
    return gDiggerCount > 1;
}

/* Selects a slot: the chosen vehicle, its sound, camera goals (0x801E74E8).
 * Select slot arg0: record it, set the current vehicle from gDiggerSlotVehicle,
 * play its sound and load its two float parameters. */
void fe_digger_select(u8 arg0) {
    gDiggerSel = arg0;
    D_803643D4 = gDiggerSlotVehicle[arg0];
    snd_stop_all_looping();
    snd_play(gSfxBank, gDiggerSelectSound[gDiggerSlotVehicle[arg0]], 0);
    gDiggerCamXGoal = gDiggerVehicleX[gDiggerSlotVehicle[arg0]];
    gDiggerCamZGoal = gDiggerViewDist[gDiggerSlotVehicle[arg0]];
}

/* One vehicle-select frame: input, then its display list (0x801E7598).
 * Vehicle select frame: reads the stick/buttons (left/right to scroll, A/start
 * to pick, B to go back), starts the previous frame's display list and builds
 * this frame's: title text, the vehicles near the camera (the selected one
 * spinning), and the scroll arrows. */
void fe_digger_frame(void) {
    u8 sfx[2] = { 0x1D, 0xD0 };
    static s32 D_802081AC = 1;  /* arrow pulse step */
    static s16 D_802081B0 = 0;  /* title pulse */
    static s8 D_802081B4 = 1;   /* title pulse direction */
    s32 i;
    s32 vtx = 0;
    FeDyn *dyn = &gDynamicBufs[gBufferIndex ^ 1];
    Gfx *gdl = dyn->dl;

    cont_start_read();
    nextdma = 0;
    no_palette_dmas = 0;
    tex_reset_decode_queue();
#ifdef NON_MATCHING
    /* The asm takes its param (the texture decoder's type-4/5 table) in $fp,
     * which the game thread's C code never sets: 0 (traced in hd.c's call;
     * this screen needs input, so the demos don't reach it). */
    anim_tex_update_all(LEAKED(s8));
#else
    anim_tex_update_all();
#endif
    cont_update();
    if (gFramesInMode >= 11 && !areWeFading()) {
        if ((gContButtons & 0x9000) && !(gContPrevButtons & 0x9000)) {
            func_80275390(0x2000);
            snd_play(gSfxBank, 0x1E, 0);
        } else if ((gContButtons & 0x4000) && !(gContPrevButtons & 0x4000)) {
            snd_play(gSfxBank, 0xDE, 0);
            D_803643D4 = gDiggerPrevVehicle;
            switch (gPrevGameMode) {
                case 0x80:
                case 0x8000000:
                    gNextGameMode = gPrevGameMode;
                    break;
                default:
                    gNextGameMode = 0x4000;
                    break;
            }
        }
    }
    if (gPadStickX < -10 && !areWeFading()) {
        if (gPrevPadStickX < -10) {
            if (gDiggerSel > 0) {
                gDiggerScrollLeft -= gPadStickX;
            }
        } else {
            gDiggerScrollLeft = 750;
        }
    }
    if ((gContButtons & 0x200) && !(gContPrevButtons & 0x200) && !areWeFading()) {
        gDiggerScrollLeft = 750;
    }
    if (gPadStickX > 10 && !areWeFading()) {
        if (gPrevPadStickX > 10) {
            if (gDiggerSel < gDiggerCount - 1) {
                gDiggerScrollRight += gPadStickX;
            }
        } else {
            gDiggerScrollRight = 750;
        }
    }
    if ((gContButtons & 0x100) && !(gContPrevButtons & 0x100) && !areWeFading()) {
        gDiggerScrollRight = 750;
    }
    if (gBufferIndex == 0 && gDiggerScrollLeft >= 750) {
        snd_play(gSfxBank, sfx[gDiggerSel == 0], 0);
        if (gDiggerSel != 0) {
            fe_digger_select(gDiggerSel - 1);
        }
        gDiggerScrollLeft -= 750;
    }
    if (gBufferIndex == 0 && gDiggerScrollRight >= 750) {
        snd_play(gSfxBank, sfx[gDiggerSel + 1 == gDiggerCount], 0);
        if (gDiggerSel + 1 != gDiggerCount) {
            fe_digger_select(gDiggerSel + 1);
        }
        gDiggerScrollRight -= 750;
    }
    rsp_send_task((u64 *) (gDynamicBufs[gBufferIndex].dl), gFrameDlLen, 1, 1, 0x4D2, 0);
    gBufferIndex ^= 1;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(gStaticSegment));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetDepthImage(gdl++, gDepthBufferPhys);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBufferPhys);
    gDPSetFillColor(gdl++, 0xFFFCFFFC);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameBufferPhys[gBufferIndex]);
    text_begin_frame();
    gdl = fe_bg_draw(gdl, (s32) dyn, &gFrameDlLen);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    text_print((Gfx * N64P *) dyn, (u8 *) gDiggerTitle, gDiggerTitleGlyphs, 0, 0xA0, 0x50, 0x18, 0x15, 0x15, 1, 0, 0, 0, 0xA0);
    if (PORT_GVI("fe_digger_frame", D_803156C0) % 20 * 60 / 60 < 16) {
        D_802081B0 += D_802081B4 * 30;
        if (D_802081B0 >= 0x100) {
            D_802081B0 -= 60;
            D_802081B4 = -D_802081B4;
        }
        if (D_802081B0 < 0) {
            D_802081B0 += 60;
            D_802081B4 = -D_802081B4;
        }
        text_print_gradient((Gfx * N64P *) dyn, (u8 *) gDiggerTitle, gDiggerTitleGlyphs, 0, 0xA0, 0x54, 0x15, 0x15, 0x15, 1, 0xFF, 0xFF - D_802081B0, 0,
                      0xFF, 0xFF, D_802081B0, 0, 0xFF);
    }
    text_flush_screen(&gdl, (Mtx *) dyn);
    if (gFramesInMode < 2) {
        guPerspective(&dyn->unk1240, &gPerspNorm, 45.0f, 1.3333334f, 40.0f, 4000.0f, 1.0f);
    }
    gDiggerCamX = gDiggerCamX + (gDiggerCamXGoal - gDiggerCamX) * 0.1 * 60.0 / 60.0;
    gDiggerCamZ = gDiggerCamZ + (gDiggerCamZGoal - gDiggerCamZ) * 0.1 * 60.0 / 60.0;
    guLookAtReflect(&dyn->unk140, &dyn->unk3C00, gDiggerCamX, 1.0f, gDiggerCamZ, gDiggerCamX, 0.0f, 0.0f, 0.0f, 1.0f,
                    0.0f);
    gImmp1(gdl++, G_RDPHALF_1, gPerspNorm);
    gSPLookAtX(gdl++, &dyn->unk3C00);
    gSPLookAtY(gdl++, &dyn->unk3C00.l[1]);
    gSPMatrix(gdl++, &dyn->unk1240, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->unk140, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gDPSetEnvColor(gdl++, 0, 0, 0, 0xFF);
    for (i = 0; i < gDiggerCount; i++) {
        u8 flag;
        u8 *seg;
        s32 off;
        Mtx *mtx;

        flag = gBufferIndex & (i == gDiggerSel);
        if (ABS(gDiggerCamX - gDiggerVehicleX[gDiggerSlotVehicle[i]]) < 570.0) {
            seg = gDiggerModels[i]->unk14 + (u8 *) gDiggerModels[i];
            if (gDiggerSel == i) {
                off = *(s32 *) (gDiggerModels[i]->unk18 + (u8 *) gDiggerModels[i] + 4);
                mtx = (Mtx *) ((u8 *) gDiggerMtxBufs[i][flag] + off);
                func_802021FC(D_80211AC0[i], gDiggerMtxBufs[i][gBufferIndex], gDiggerMtxBufs[i][gBufferIndex ^ 1]);
                guRotate(mtx, (gDiggerSpin[i] += 4) % 360, 0.0f, 1.0f, 0.0f);
                osWritebackDCache(mtx, sizeof(Mtx));
                func_802025D0(gDiggerSlotVehicle[i], (f32) ((gDiggerSpin[i] + 180) % 360) * 11.37778);
            }
            gSPSegment(gdl++, 6, seg);
            gSPSegment(gdl++, 7, gDiggerMtxBufs[i][flag]);
            gSPMatrix(gdl++, &gDiggerTiltMtx, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &gDiggerTransMtx[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &gDiggerScaleMtx[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPDisplayList(gdl++, gDiggerDls[i][gBufferIndex]);
            gSPMatrix(gdl++, &gDiggerTiltMtx, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &gDiggerTransMtx[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPMatrix(gdl++, &gDiggerScaleMtx[i], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
            gSPDisplayList(gdl++, gDiggerDls[i][gBufferIndex + 2]);
        }
    }
    gDiggerArrowPulse += D_802081AC;
    if (D_802081AC < 0) {
        gDiggerArrowPulse += D_802081AC * 2;
    }
    if (gDiggerArrowPulse < 0 || gDiggerArrowPulse >= 16) {
        gDiggerArrowPulse -= D_802081AC * 2;
        D_802081AC = -D_802081AC;
    }
    if (gDiggerSel + 1 != gDiggerCount) {
        u8 *c = &D_802F47B0[0x98];

        vtx = hud_sprite_quad_colors((struct SpriteVtxBuf *) dyn, 2, vtx, gDiggerArrowPulse + 0x108, 0x20, gDiggerArrowPulse / 4 + 12, 0x10, c[0], c[1], c[2], c[3],
                            c[4], c[5], c[6], c[7], c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        vtx = hud_sprite_quad((struct SpriteVtxBuf *) dyn, 2, vtx, gDiggerArrowPulse + 0x10C, 0x23, gDiggerArrowPulse / 4 + 12, 0x10, 0, 0, 0, 0xA0);
        gdl = hud_sprite_setup(gdl, 0);
        gSPVertex(gdl++, &dyn->unk1E00[0], 8, 0);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    if (gDiggerSel > 0) {
        u8 *c = &D_802F47B0[0x98];

        vtx = hud_sprite_quad_colors((struct SpriteVtxBuf *) dyn, 3, vtx, 0x34 - gDiggerArrowPulse, 0x20, gDiggerArrowPulse / 4 + 12, 0x10, c[0], c[1], c[2], c[3],
                            c[4], c[5], c[6], c[7], c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
        vtx = hud_sprite_quad((struct SpriteVtxBuf *) dyn, 3, vtx, 0x30 - gDiggerArrowPulse, 0x23, gDiggerArrowPulse / 4 + 12, 0x10, 0, 0, 0, 0xA0);
        gdl = hud_sprite_setup(gdl, 0);
        gSPVertex(gdl++, &dyn->unk1E00[vtx - 8], 8, 0);
        gSP1Triangle(gdl++, 4, 5, 6, 0);
        gSP1Triangle(gdl++, 4, 6, 7, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
    }
    gdl = yoshi_draw(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    gdl = func_80274BF0((s32) dyn, gdl);
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    gFrameDlLen = gdl - dyn->dl;
    for (i = 0; i < nextdma; i++) {
        osRecvMesg(&textureDmaMessageQ, NULL, 1);
    }
    for (i = 0; i < nextdma - no_palette_dmas; i++) {
        tex_decode_next();
    }
}
