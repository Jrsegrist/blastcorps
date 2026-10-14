#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gHeapPtr
#define LEGACY_gLevelData
#define LEGACY_gLevelSegment8
#define LEGACY_gVehiclesEnd
#define LEGACY_gMusicPlayer
#define LEGACY_gSfxBank
#define LEGACY_gRdus
#define LEGACY_gMarkedTargetPts
#define LEGACY_gRdpOutputBuffer
#define LEGACY_gDriverPos
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gYoshiFeItems ((MenuEntry *) gYoshiFeItems)
#define gPakThread ((u8 *) &gPakThread)
#define gLevelInfo ((LevelInfo *) gLevelInfo)
#define gYoshiGameItems ((MenuItem *) gYoshiGameItems)
#define gYoshiWindows ((MenuPage *) gYoshiWindows)
#define gScheduler ((u8 *) &gScheduler)
#define gMainSchedClient ((u8 *) &gMainSchedClient)
#define gVehicles ((VehRecord *) gVehicles)
#define players ((Player *) players)
#define pakBuffer ((u8 *) pakBuffer)
#ifdef NON_MATCHING
#define gHeapPtr (*(s32 *) &gHeapPtr)
#define gLevelSegment8 (*(u32 *) &gLevelSegment8)
#define gVehiclesEnd (*(VehRecord * N64P *) &gVehiclesEnd)
#define gRdus (*(Node88 * N64P *) &gRdus)
#define gMarkedTargetPts (*(Box * N64P *) &gMarkedTargetPts)
#define gRdpOutputBuffer (*(s32 *) &gRdpOutputBuffer)
#define gDriverPos (*(s32 *) gDriverPos)
#endif
/* end of views */

/* hd.c (from its assert strings): boot, the main game thread and the
 * frame loop. Owns .rodata at 0x80307B90.
 *
 * Variables defined here (the tell is a shared `lui` on paired stores).
 * Only part of hd.c's .bss is modelled so far: the section is placed at
 * 0x80310D80 by hd_code_bss.us.v11.ld, and the later variables get their
 * real addresses from undefined_syms (absolute symbols win).
 *
 * The game mode (gGameMode, one bit of a u64; hd.c's debug output calls it
 * game_mode and the next one loop_done). What the code shows of the bits:
 *   0x2 attract mode (the demo recordings, "PRESS START"); 0x4 a level in
 *   play; 0x100 its map view ("MISSILE VIEW" / "SHUTTLE VIEW"); 0x10, 0x20
 *   the front end's boot screens; 0x2000 a level starts (its file is loaded,
 *   then 0x4); 0x4000 the world screen ("World screen centred on level");
 *   0x40000 name entry ("ENTER NAME!"); 0x8000000 after a level (results,
 *   saving); 0x20000000 the vehicle select screen; 0x100000000000 the
 *   globe's sequences; 0x20000000000000 the boot-time EEPROM / pak check
 *   ("NO EE PRESENT"); 0x40000000000000 "ERASE SAVED GAME!";
 *   0x100000000000000 "BECOME GUEST PLAYER:"; 0x400000000000000 the start
 *   of the attract sequence; 0x2000000000000000 the Yoshi window pages
 *   ("USE Z/R TO TURN PAGES"); 0x4000000000000000 "QUIT GAME!". */
u64 gMainThreadStack[0x400]; /* 0x80310D80: the main thread's stack, its low half filled with a guard pattern */
u64 gGameMode; /* 0x80364A90: the current game mode */
u64 gNextGameMode; /* 0x80364A98: the mode to switch to; nonzero ends the frame loop */
u64 D_803649D8; /* osGetTime() at the frame's draw (seeds the screen shake and part jitter) */
u64 gPrevGameMode; /* 0x80364A88 */
u64 gGameModeAfterNext; /* 0x80364AA0: set during a switch; becomes gNextGameMode after it */
u64 gFrameStartTime; /* 0x80364AD0: osGetTime() at the start of the frame */
u8 gCamSnap; /* 0x80364412: the camera jumps to the wanted eye and target this frame */

void hd_idle_thread(void *);
void hd_main_thread(void *);

extern OSThread gIdleThread;
extern u64 gIdleThreadStack[]; /* idle thread stack */
extern u8 D_80314D80[];
extern u8 D_80314D98[];
extern u32 D_803649F4;

/* A gVehicles record: one per loaded vehicle model (veh_add_record), 0x74 bytes */
typedef struct {
    void * N64P unk0; /* segment 6 */
    void * N64P unk4; /* segment 7 */
    void * N64P unk8; /* segment 7 (alternate) */
    void * N64P unkC; /* display lists by LOD */
    void * N64P unk10;
    void * N64P unk14;
    u8 pad18[0x18];
    void * N64P unk30; /* alternate display lists by LOD */
    void * N64P unk34;
    void * N64P unk38;
    u8 pad3C[0x18];
    s32 unk54;
    s32 unk58;
    s32 type; /* the vehicle type (docs/blast_corps_vehicles.txt) */
    u32 alpha; /* env alpha */
    u8 pad64[0xC]; /* x, y, z (veh_set_record) */
    s32 onMovingPart; /* resting on a moving level part: veh_update_others re-grounds it */
} VehRecord;
STATIC_ASSERT_SIZE(VehRecord, 0x74);
#ifndef NON_MATCHING
extern VehRecord *gVehiclesEnd;
#endif


extern u8 D_00787F40[]; /* static code segment bounds (ROM) */
extern u8 D_00788000[];
extern u8 gPendingTune;
extern u8 D_803FF600[];
#ifndef NON_MATCHING
extern s32 gHeapPtr; /* heap pointer */
#ifndef NON_MATCHING
extern s32 gRdpOutputBuffer;
#endif
#endif
typedef struct {
    u8 unk0;
    u8 pad1[0x35];
    u16 unk36;
    u8 pad38[0xC];
} LevelInfo;
extern u8 D_80358088[];

void hd_clear_framebuffer(Gfx *, s32 *);
void hd_close_display_list(Gfx *, s32 *);

#define TOPLEVEL_DL_SIZE 0xB5E

/* Rare's assert; line numbers are the original hd.c's */
#define HD_ASSERT(EX, line) \
    if (!(EX)) \
    debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "hd.c", line)

typedef struct {
    u8 pad0[0x10];
    u32 unk10;
    s32 unk14;
    u8 unk18[0x79]; /* per level */
    u8 unk91;
    u8 pad92[0x5E];
    u32 unkF0; /* vehicle flags */
    u8 padF4[0xC];
} Player; /* 0x100 bytes */
extern u8 D_803649EC;
extern s32 gEnterRefusedSound;
#ifndef NON_MATCHING
extern s32 gSfxBank;
#endif
s32 veh_enter(s32);

extern OSMesg D_803150B8[];
extern OSMesg D_80315198[];
extern u64 gSchedStack[];
extern OSMesg D_803153F8[];
extern u8 D_8021ED00[];
extern f32 gCamHeadingGoal;
extern u8 gCamTurnPos;
extern u8 gCamTurnNeg;
#define PHYS(x) ((u32)(x) & 0x1FFFFFFF)

u8 cam_fixed_heading_area(void);

#ifndef NON_MATCHING
extern s32 gLevelData;
#endif

extern f32 D_80364AB4;
extern f32 D_80364AB8;
extern f32 D_80364ABC;
extern u8 D_80364AC0;

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8;
    u8 unk9;
} Message; /* 0xC bytes */
extern Message D_80364A00[5];
extern u8 D_80364A3C;
extern u8 D_80364A3D;
extern s32 D_80364A44;
extern u8 D_80364A48;
extern s16 D_80364A4A;
extern s16 D_80364A4C;
extern u8 D_80364A4E;

extern u8 D_803156F5;


extern u16 D_802E8BF4[]; /* sprite ids (spr_load) */
extern s32 gCamEyeHeight;
extern u8 D_80364434;
extern f32 gCamTilt;
extern f32 gCamTurn;
extern s16 gCamPitch;
extern f32 D_803649F8;
extern s32 D_80364A5C;
extern u8 D_80364A6F;
extern u8 D_80364A86;
#ifndef NON_MATCHING
extern s32 gDriverPos;
#endif
void lvl_load_file(s32, s32, s32 *);
void lvl_alloc_dl_buffers(void);


extern s32 gLevelDl0[2];
extern s32 gLevelDl1[2];
extern s32 gLevelDl2[2];
extern s32 gLevelDl3[2];
extern s32 gLeaveRefusedSound;
u8 veh_exit_check(void);
s32 veh_is_loaded(u8);

extern u8 D_803153F0;
extern s32 gCarrierAlarmSound;

/* menu tables, 0x1C-byte entries */
typedef struct {
    u16 unk0; /* flags */
    u8 pad2[0x16];
    u8 unk18;
    u8 pad19[3];
} MenuItem;
typedef struct {
    u8 pad0[4];
    s16 unk4;
    u8 pad6[2];
    s32 unk8;
    u8 padC[2];
    u16 unkE;
    u16 unk10;
    u8 pad12[6];
    s16 unk18;
    u8 pad1A[2];
} MenuPage;
typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char * N64P unkC; /* title */
    void * N64P unk10; /* glyph list */
    u8 pad14[8];
} MenuEntry;

extern u8 D_004A5660[]; /* lagp */
extern u8 D_004ACC10[]; /* chimp */
extern u8 D_004B8960[]; /* valley */
extern u8 D_004BFD60[]; /* fact */
extern u8 D_004C3AC0[]; /* dip */
extern u8 D_004D5F90[]; /* beetle */
extern u8 D_004E2F70[]; /* bonus1 */
extern u8 D_004E4E80[]; /* bonus2 */
extern u8 D_004E7C00[]; /* bonus3 */
extern u8 D_004E8F70[]; /* level9 */
extern u8 D_004F5C10[]; /* level10 */
extern u8 D_00500520[]; /* level11 */
extern u8 D_00507E80[]; /* level12 */
extern u8 D_00511340[]; /* level13 */
extern u8 D_00523080[]; /* level14 */
extern u8 D_0052CD00[]; /* level15 */
extern u8 D_00532700[]; /* level16 */
extern u8 D_0053E9B0[]; /* level17 */
extern u8 D_0054A820[]; /* level18 */
extern u8 D_00552DE0[]; /* level19 */
extern u8 D_00555000[]; /* level20 */
extern u8 D_00560E90[]; /* level21 */
extern u8 D_005652D0[]; /* level22 */
extern u8 D_0056F3F0[]; /* level23 */
extern u8 D_005721E0[]; /* level24 */
extern u8 D_005736E0[]; /* level25 */
extern u8 D_0057A2C0[]; /* level26 */
extern u8 D_00580B60[]; /* level27 */
extern u8 D_00588CE0[]; /* level28 */
extern u8 D_0058BE80[]; /* level29 */
extern u8 D_00597B80[]; /* level30 */
extern u8 D_0059B7D0[]; /* level31 */
extern u8 D_005A5840[]; /* level32 */
extern u8 D_005B0B10[]; /* level33 */
extern u8 D_005B5A30[]; /* level34 */
extern u8 D_005B8BB0[]; /* level35 */
extern u8 D_005C4C80[]; /* level36 */
extern u8 D_005CA9C0[]; /* level37 */
extern u8 D_005CCF50[]; /* level38 */
extern u8 D_005D1060[]; /* level39 */
extern u8 D_005DC830[]; /* level40 */
extern u8 D_005E6EE0[]; /* level41 */
extern u8 D_005EC800[]; /* level42 */
extern u8 D_005F3A80[]; /* level43 */
extern u8 D_006014B0[]; /* level44 */
extern u8 D_0060A710[]; /* level45 */
extern u8 D_00613AA0[]; /* level46 */
extern u8 D_0061DD70[]; /* level47 */
extern u8 D_00621AF0[]; /* level48 */
extern u8 D_006269E0[]; /* level49 */
extern u8 D_00630C30[]; /* level50 */
extern u8 D_00635700[]; /* level51 */
extern u8 D_0063CA10[]; /* level52 */
extern u8 D_00641F30[]; /* level53 */
extern u8 D_00644810[]; /* level54 */
extern u8 D_00646080[]; /* level55 */
extern u8 D_00647550[]; /* level56 */
extern u8 D_00654FC0[]; /* level57 */
extern u8 D_00660950[]; /* level58 */
extern u8 D_00665F80[]; /* level59 */

extern s32 D_8036506C;

#ifndef NON_MATCHING
extern u32 gLevelSegment8;
#endif
extern u16 D_80304904[]; /* glyph lists */
extern u16 D_80304910[];
extern u16 D_8030491C[];
extern u16 D_80304938[];
void veh_draw_shadows(Gfx **, DynamicBuf *, u8);
void veh_draw_chopper_shadow(Gfx **, DynamicBuf *);
void func_802502EC(void);
void cam_update(Mtx *, LookAt *, Mtx *);
#define SEG2(off) ((u8 *) D_02000000 + (off))

#define MS_ASSERT(EX, line) \
    if (!(EX)) \
    debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "./master_switch.c", line)
extern u16 D_803047A0[];
extern u16 D_803047B4[];
extern u16 D_803047CC[];
extern u16 D_803047DC[];
extern u8 D_80315438;
extern u32 D_80364A54;
extern u8 D_80364A60;
extern s32 D_80364AC4;
extern u32 gFrameCpuTime;
extern u32 gFrameStartLag;
extern u8 D_80365065;
extern u8 D_80365066;
extern u8 D_8039C4F8[];
#define LEVEL_SAVE_SIZE 0x40
void hd_game_frame(void);
void hd_init(void);
void hd_check_stack(void);
#define MODE_PAGE(m) gYoshiWindows[D_802F4868[lowest_set_bit(m)]]
#define MODE_ENTRY(m) gYoshiFeItems[MODE_PAGE(m).unkE + MODE_PAGE(m).unk10 - 2]

extern s32 D_80364A78;
extern u8 D_80364A7C;
extern s32 D_80364A80;
extern f32 gCamZoom;
extern u8 D_80364A85;
#ifndef NON_MATCHING
extern s32 gMusicPlayer;
#endif
#define NUM_TEXTURE_DMAS 0x90
void cull_start(Mtx *, Mtx *);
void veh_apply_switch(void);
void veh_leave(void);
void veh_update_others(void);
void veh_update_player(void);
void func_8024A92C(u32);
void veh_update_riders(void);
void func_8024A348(void);
void func_8024ADD8(void);
Gfx *hd_build_frame_dl(DynamicBuf *, s32 *);
void func_8024BDA4(u16 *);
#define LEVEL_DONE(l) (players[playerNumber].unk18[l] > 0 && players[playerNumber].unk18[l] < 6) ? 1 : 0

/* (end of declarations) */

/* hd_code's entry (init jumps here): osInitialize, the debug command line
 * (16 words at PI address 0xFFB000, parse_cmdline), then the idle thread
 * (0x802447C0). */
void hd_boot(void) {
    u32 i;
    s32 pad[2];
    u32 addr;
    u32 buf[16];
    s32 pad2[2];

    func_802D4020();
    for (addr = 0xFFB000, i = 0; i < 16; i++, addr += 4) {
        osPiRawReadIo(addr, &buf[i]);
    }
    parse_cmdline((u8 *) buf);
    osCreateThread(&gIdleThread, 1, hd_idle_thread, NULL, gIdleThreadStack + 0x40, 10);
    osStartThread(&gIdleThread);
}

/* The idle thread: starts the PI manager and the main thread (twice unless
 * -d), drops to priority 0 and spins (0x80244870). */
void hd_idle_thread(void *arg0) {
    s32 pad;

    func_802D4550(4);
    func_802D4560(0x96, (OSMesgQueue *) D_80314D80, (void * N64P *) D_80314D98, 0xC2);
    osCreateThread(&gMainThread, 3, hd_main_thread, arg0, gMainThreadStack + 0x400, 10);
    osStartThread(&gMainThread);
    if (D_802FA254 == 0) {
        osStartThread(&gMainThread);
    }
    osSetThreadPri(NULL, 0);
    while (1) {
    }
}

/* The main thread: hd_init, then forever: switch to gNextGameMode (the
 * set-up of each mode, master_switch.c's asserts; a switch can queue the next
 * one in gGameModeAfterNext), then run frames of the mode (hd_game_frame or
 * the front end's) until a new mode is asked for (0x80244930). */
void hd_main_thread(void *arg) {
    s32 i;
    s32 start2;
    void * N64P msg;
    u8 newPlayer;
    s32 start;

    hd_init();
    while (1) {
        gPendingTune = 0;
        do {
            debug_printf("game mode switch from %d to %d\n", lowest_set_bit(gGameMode), lowest_set_bit(gNextGameMode));
            gGameModeAfterNext = 0;
            switch (gNextGameMode) {
                case 32:
                    hd_mode_setup();
                    fe_boot_screen_init(2);
                    break;
                case 16:
                    hd_mode_setup();
                    fe_boot_screen_init(1);
                    start2 = D_803156C4;
                    while ((u32) (D_803156C4 - start2) < 15) {
                        PORT_SPIN();
                    }
                    break;
                case 0x100000000:
                    fe_pak_list_files();
                    yoshi_demand(0x8012);
                    gPendingTune = music_tune_for_mode(gNextGameMode);
                    break;
                case 0x80000000000:
                    hd_mode_setup();
                    title_setup_logo();
                    fe_bg_load(1);
                    osSendMesg(&gPakRequestQueue, (OSMesg) 0x01000001, 1);
                    yoshi_demand(0x8011);
                    break;
                case 0x10000:
                    gPendingTune = music_tune_for_mode(gNextGameMode);
                    title_setup_logo();
                    fe_player_build_load_menu();
                    fe_player_ticker_reset(playerNumber);
                    yoshi_demand(0x800A);
                    break;
                case 0x10000000:
                    osSendMesg(&gPakRequestQueue, (OSMesg) 0x01000001, 1);
                    fe_player_ticker_reset(4);
                    yoshi_demand(0x8011);
                    break;
                case 0x20000000000000:
                    snd_stop_all();
                    hd_mode_setup();
                    tex_load_table();
                    title_setup_logo();
                    fe_bg_load(1);
                    osSendMesg(&gPakRequestQueue, (OSMesg) 0x0100000F, 1);
                    osRecvMesg(&pakToGameMessageQ, &msg, 1);
                    if (msg != NULL || gIgnorePak) {
                        gUseEeprom = 1;
                    } else {
                        gUseEeprom = 0;
                    }
                    gIgnorePak = 0;
                    if (!gUseEeprom) {
                        fe_player_panel_init(4);
                        gGameModeAfterNext = 0x10000000;
                    } else {
                        playerNumber = 0;
                        D_80364AE9 = 0;
                        playerNumberAtStart = 0, osSendMesg(&gPakRequestQueue, (OSMesg) 0x01000010, 1);
                        osRecvMesg(&pakToGameMessageQ, &gEepromPresent, 1);
                        if (gEepromPresent == NULL) {
                            debug_printf("NO EE PRESENT! - USING DUMMY EE\n");
                        }
                        osSendMesg(&gPakRequestQueue, (OSMesg) 0x01000006, 1);
                        osRecvMesg(&pakToGameMessageQ, &msg, 1);
                        if (msg == NULL) {
                            msg = (void *) N64_IPTR(fe_pak_restore_from_eeprom());
                        }
                        if (msg != NULL) {
                            fe_player_new_game(playerNumber, 1, 1);
                            gGameModeAfterNext = MODE_NAME_ENTRY;
                        } else {
                            gSlotState[playerNumber] = 1;
                            gGameModeAfterNext = MODE_WORLD;
                        }
                    }
                    break;
                case 0x4000000000000:
                    hd_mode_setup();
                    tex_load_table();
                    title_setup_logo();
                    fe_bg_load(4);
                    fe_player_panel_init(4);
                    yoshi_demand(0x8038);
                    break;
                case 0x400000000000000:
                    D_802E8BEC = -1;
                    D_802E8BF0 = 1;
                    D_80365065 = 0;
                    gGameModeAfterNext = 2;
                    gIgnorePak = 0;
                    fe_player_update_level_kinds();
                    break;
                case 2:
                    switch (D_802E8BEC) {
                        case 1:
                        case 3:
                        case 4:
                        case 6:
                            if (gGameMode == 2) {
                                gNextGameMode = MODE_ATTRACT_PICTURE;
                                hd_mode_setup();
                                fe_bg_load(7);
                                fe_attract_pic_load(D_80364AC4 & 3);
                                yoshi_demand(0x8035);
                                fe_player_panel_init(D_80364AC4 & 3);
                                D_80364AC4++;
                                break;
                            }
                        default:
                        D_802E8BEC++;
                        if (D_802E8BEC == 9) {
                            D_802E8BEC = 0;
                            D_802E8BF0 = 0;
                        } else {
                            D_802E8BF0 ^= 1;
                        }
                        gPadDisplayAlpha = 0;
                        hd_mode_setup();
                        title_setup_logo();
                        hud_load_pad_image();
                        rec_start_playback(D_802E8BEC, &levelno);
                        if (D_802E8BEC == 0) {
                            hud_load_demo_banner();
                        }
                        hd_setup_level(0);
                        break;
                    }
                    break;
                case 0x4000:
                    if (playerNumber != playerNumberAtStart) {
                        D_80364AE9 = playerNumberAtStart;
                        playerNumber = playerNumberAtStart;
                    }
                    func_8028B3E0();
                    debug_printf("World screen centred on level %d\n", levelno);
                    playerNumber = playerNumberAtStart;
                    fe_player_update_level_kinds();
                    if (!D_80365065) {
                        D_80365065 = 1;
                        D_80364A87 = 0;
                        D_803643D5 = 0;
                        fe_open_reachable_levels(8);
                        levelno = players[playerNumber].pad0[8];
                        debug_printf("going to level %d\n", levelno);
                    }
                    if ((D_8039CA60 = 0, ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0)) &&
                        levelno >= 0x2B && levelno < 0x2E) {
                        if (!((players[playerNumber].unk18[levelno + 1] > 0 && players[playerNumber].unk18[levelno + 1] < 6) ? 1 : 0)) {
                            levelno++;
                        }
                    }
                    hd_mode_setup();
                    fe_player_save_pending();
                    osViBlack(1);
                    tex_load_table();
                    fe_globe_init(levelno);
                    break;
                case 0x800000000000:
                    gPendingTune = music_tune_for_mode(gNextGameMode);
                    break;
                case 0x400000000:
                    yoshi_demand(0x8013);
                    break;
                case 0x800000000000000:
                    hd_mode_setup();
                    fe_bg_load(1);
                    title_setup_logo();
                    yoshi_demand(0x8059);
                    break;
                case 0x40000000000:
                    hd_mode_setup();
                    fe_bg_load(3);
                    fe_player_panel_init(playerNumber);
                    title_setup_logo();
                    gBestTimesReturnMode = gGameMode;
                    yoshi_demand(0x8016);
                    break;
                case 0x200000000000000:
                    fe_player_ticker_set_text(playerNumber, 1);
                    fe_best_color_items();
                    yoshi_demand(0x8016);
                    gNextGameMode = MODE_BEST_TIMES;
                    break;
                case 0x2000:
                    hd_mode_setup();
                    if (gLevelInfo[levelno].unk0 != 1) {
                        hd_setup_level(0);
                        stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
                        stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA80);
                        gPauseRequest = 1;
                    } else {
                        if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
                            hd_setup_level((s32) pakBuffer);
                        } else {
                            hd_setup_level(0);
                        }
                        stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
                        stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA80);
                        gGameModeAfterNext = 4;
                    }
                    break;
                case 4:
                    switch (gGameMode) {
                        case 0x1000000000:
                            music_pop_tune();
                            snd_set_mode_slot_volumes(gNextGameMode);
                            gUnpauseRequest = 1;
                            break;
                        case 0x100:
                            gCamSnap = 1;
                            break;
                        case 0x2000:
                            if (gLevelInfo[levelno].unk0 != 1) {
                                gPendingTune = music_tune_for_mode(gNextGameMode);
                            }
                            gUnpauseRequest = 1;
                            gUnpauseRequest &= !yoshi_show_hint(0x52);
                            /* one line: the line break changes as1's schedule */
                            gUnpauseRequest &= !yoshi_show_hint(0x56); if ((((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0)) &&
                                gLevelInfo[levelno].unk0 == 1) {
                                gUnpauseRequest &= !yoshi_show_hint(0x55);
                            } else {
                                gUnpauseRequest &= !yoshi_show_hint(0x57);
                            }
                            gPauseRequest = !gUnpauseRequest;
                            debug_printf("Unpause at start = %d\n", gUnpauseRequest);
                            D_80364A58 = ((s32) PORT_GVI("hd_main_thread:4A58", D_803156C0));
                            D_80358064 = 0;
                            func_8025BB38();
                            debug_printf("snew ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          nss.ip, nss.tc, nss.bd, nss.cr, nss.rt,
                                          nss.coin, nss.bdn);
                            debug_printf("sold ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA60.ip, D_8036EA60.tc, D_8036EA60.bd, D_8036EA60.cr, D_8036EA60.rt,
                                          D_8036EA60.coin, D_8036EA60.bdn);
                            debug_printf("sres ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA80.ip, D_8036EA80.tc, D_8036EA80.bd, D_8036EA80.cr, D_8036EA80.rt,
                                          D_8036EA80.coin, D_8036EA80.bdn);
                            debug_printf("srs2 ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA90.ip, D_8036EA90.tc, D_8036EA90.bd, D_8036EA90.cr, D_8036EA90.rt,
                                          D_8036EA90.coin, D_8036EA90.bdn);
                            break;
                    }
                    break;
                case 0x100:
                    gCamSnap = 1;
                    break;
                case 0x80:
                    newPlayer = 0;
                    switch (gGameMode) {
                        case 0x100000000000:
                            func_8028B3E0();
                            gPendingTune = music_tune_for_mode(gNextGameMode);
                        case 0x4000:
                            newPlayer = stats_save_status();
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (playerNumber != D_80364AE9) {
                                D_80364AE9 = playerNumber, debug_printf("switching to new player ...........\n");
                                stats_save_status();
                            }
                            break;
                    }
                    hd_mode_setup();
                    if (newPlayer) {
                        osSendMesg(&gPakRequestQueue, (OSMesg) ((levelno << 8) | 0xD | (playerNumber << 16)), 1);
                    }
                    MODE_ENTRY(gLevelKind).unk0 &= ~1;
                    MODE_ENTRY(gLevelKind).unk0 |= 0x800;
                    yoshi_set_target_icons();
                    yoshi_demand(D_802F4868[lowest_set_bit(gLevelKind)] | 0x8000);
                    MODE_PAGE(gLevelKind).unk18 = MODE_PAGE(gLevelKind).unkE + MODE_PAGE(gLevelKind).unk10 - 3;
                    MODE_PAGE(gLevelKind).unk8 &= ~8;
                    stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
                    D_80315438 = fe_stats_grade_level(&D_80364A60, 1, 0), fe_player_panel_init(playerNumber);
                    fe_player_medals_init(D_80315438);
                    fe_player_medals_show_quiet(D_80315438);
                    fe_bg_load(1);
                    D_80364A71 = fe_stats_count_unlocks();
                    if (D_80364A71 != -1) {
                        fe_comm_point_load();
                    }
                    break;
                case 0x800:
                    hd_mode_setup();
                    if (gGameMode == MODE_WORLD) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    hd_setup_level(0);
                    chopper_fly_to_carrier();
                    if (levelno == 0x32) {
                        yoshi_demand(0x8024);
                    }
                    break;
                case 0x40000000:
                    hd_mode_setup();
                    start = D_803156C4;
                    while ((u32) (D_803156C4 - start) < 15) {
                        PORT_SPIN();
                    }
                    fe_promo_init();
                    fe_bg_load(3);
                    D_80364A71 = fe_stats_count_unlocks();
                    if (D_80364A71 != -1) {
                        fe_comm_point_load();
                    }
                    fe_player_panel_init(playerNumber);
                    fe_player_medals_init(D_80315438);
                    break;
                case 0x8000000:
                    switch (gGameMode) {
                        case 0x40:
                        case 0x400:
                        case 0x20000000:
                            hd_mode_setup();
                            fe_bg_load(1);
                            D_80364A71 = fe_stats_count_unlocks();
                            if (D_80364A71 != -1) {
                                fe_comm_point_load();
                            }
                            fe_player_panel_init(playerNumber);
                            stats_copy_result((u8 *) &D_8036EA90, (u8 *) &nss);
                            D_80315438 = fe_stats_grade_level(&D_80364A60, 1, 0);
                            fe_player_medals_init(D_80315438);
                            fe_player_medals_show_quiet(D_80315438);
                            break;
                        case 0x40000000:
                            gPendingTune = music_tune_for_mode(gNextGameMode);
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (playerNumber != D_80364AE9) {
                                debug_printf("switching to new player ...........\n");
                                D_80364AE9 = playerNumber;
                                D_803643D5 = 0;
                                stats_save_status();
                            }
                            hd_mode_setup();
                            D_80364A71 = fe_stats_count_unlocks();
                            if (D_80364A71 != -1) {
                                fe_comm_point_load();
                            }
                            fe_player_panel_init(playerNumber);
                            D_80315438 = fe_stats_grade_level(&D_80364A60, 1, 0);
                            fe_bg_load(1);
                            fe_player_medals_init(D_80315438);
                            fe_player_medals_show_quiet(D_80315438);
                            break;
                        default:
                            MS_ASSERT(!saveIt[playerNumber] || saveIt[playerNumber]==levelno+1, 488);
                            saveIt[playerNumber] = levelno + 1;
                            if (gLevelDone) {
                                obj_tally(gLevelInfo[levelno].unk0 == 0x20 || gLevelInfo[levelno].unk0 == 0x80);
                            }
                            D_803643D5 = !((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0);
                            if (playerNumber == playerNumberAtStart) {
                                D_80364A87 |= D_803643D5;
                            }
                            if (gLevelKind == 1 && !D_803643D5) {
                                for (i = 0; i < 0x40; i++) {
                                    D_8039C4F8[i] = pakBuffer[i];
                                }
                            }
                            commpt_save_flags();
                            academy_save_markers();
                            if (gLevelKind == 1) {
                                MS_ASSERT(record_status(pakBuffer)<=LEVEL_SAVE_SIZE-4, 513);
                                MS_ASSERT(!saveLevel || saveLevel==levelno+1, 515);
                                saveLevel = levelno + 1;
                                if ((u32) (stats_frames_to_tenths(D_80364A5C) + nss.tc) >= 60000) {
                                    nss.tc = 59999;
                                } else {
                                    nss.tc = stats_frames_to_tenths(D_80364A5C) + nss.tc;
                                }
                            } else if (gLevelDone) {
                                hud_race_time_left();
                                nss.tc = gLevelInfo[levelno].unk36 - gTimeLeft;
                            } else {
                                nss.tc = 0xFFFF;
                            }
                            MS_ASSERT(nss.tc, 530);
                            if (nss.tc == 0) {
                                nss.tc = 1;
                            }
                            hd_mode_setup();
                            if (D_80365066) {
                                nss.bd = D_8036EB92;
                                D_80365066 = 0;
                            }
                            if (D_803643D5) {
                                stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
                            } else {
                                stats_copy_result((u8 *) &D_8036EA80, (u8 *) &D_8036EA60);
                            }
                            D_80315438 = fe_stats_grade_level(&D_80364A60, 1, 1), stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA90);
                            if (D_80364A60) {
                                gGameModeAfterNext = MODE_PROMOTION;
                                D_80364A64 = 0xDC;
                                gPendingTune = 0;
                            } else {
                                fe_player_medals_init(D_80315438);
                                fe_bg_load(1);
                                D_80364A71 = fe_stats_count_unlocks();
                                if (D_80364A71 != -1) {
                                    fe_comm_point_load();
                                }
                                fe_player_panel_init(playerNumber);
                            }
                            MODE_ENTRY(gLevelKind).unk0 |= 1;
                            MODE_ENTRY(gLevelKind).unk0 &= ~0x800;
                            break;
                    }
                    if (D_803643D5 && playerNumber == playerNumberAtStart) {
                        MODE_PAGE(gLevelKind).unk18 = MODE_PAGE(gLevelKind).unkE + MODE_PAGE(gLevelKind).unk10 - 1;
                    } else {
                        MODE_PAGE(gLevelKind).unk18 = MODE_PAGE(gLevelKind).unkE + MODE_PAGE(gLevelKind).unk10 - 3;
                    }
                    MODE_PAGE(gLevelKind).unk8 |= 8;
                    yoshi_set_target_icons();
                    if (gUseEeprom) {
                        gYoshiFeItems[29].unk0 &= ~1;
                    }
                    yoshi_demand(D_802F4868[lowest_set_bit(gLevelKind)] | 0x8000);
                    break;
                case 0x40:
                    if (gGameMode & 0x04000200) {
                        func_8028B3E0();
                        nss.tc = stats_frames_to_tenths(D_80364A5C), stats_copy_result((u8 *) &nss, (u8 *) &D_8036EA60);
                        fe_stats_grade_level(&D_80364A60, 0, 1);
                    }
                    gYoshiGameItems[24].unk0 |= 1;
                    gYoshiGameItems[24].unk0 &= ~0x800;
                    gYoshiGameItems[23].unk0 |= 1;
                    gYoshiGameItems[23].unk0 &= ~0x800;
                    gYoshiGameItems[17].unk0 &= ~1;
                    gYoshiGameItems[17].unk0 |= 0x800;
                    if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
                        gYoshiWindows[6].unk18 = 0x18, gYoshiWindows[7].unk18 = 0x22;
                    } else {
                        gYoshiWindows[6].unk18 = 0x17, gYoshiWindows[7].unk18 = 0x21;
                    }
                    D_80364A50 = 0;
                    D_80364A54 = PORT_GVI("hd_main_thread:4A54", D_803156C4);
                    hd_mode_setup();
                    if (gLevelInfo[levelno].unk0 == 1 &&
                        ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) &&
                        !D_803643D5) {
                        hd_setup_level((s32) D_8039C4F8);
                    } else {
                        hd_setup_level(0);
                    }
                    yoshi_set_target_icons();
                    func_8025BB50();
                    break;
                case 0x20000000:
                    hd_mode_setup();
                    if (gGameMode == MODE_WORLD) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    tex_load_table();
                    if (fe_digger_init()) {
                        fe_bg_load(2);
                    } else {
                        gGameModeAfterNext = MODE_LEVEL_START;
                    }
                    break;
                case 0x40000:
                    osSendMesg(&gPakRequestQueue, (OSMesg) ((playerNumber << 16) | 0x14 | 0x01000000), 1);
                    osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    fe_player_name_entry_open("ENTER NAME!", D_803047A0, 7, 0x1E, (char *) &players[playerNumber]);
                    yoshi_demand(0x800B);
                    title_setup_logo();
                    gPendingTune = music_tune_for_mode(gNextGameMode);
                    break;
                case 0x4000000000000000:
                    hd_mode_setup();
                    fe_bg_load(1);
                    gYoshiFeItems[9].unkC = "QUIT GAME!", gYoshiFeItems[9].unk10 = D_803047CC, gYoshiFeItems[9].unk6 = gYoshiFeItems[9].unk8 = 22;
                    yoshi_demand(0x800C);
                    break;
                case 0x1000000000:
                    music_push_tune(0x18);
                    snd_set_slot_volume(0, 0);
                    func_80297ECC();
                    yoshi_demand(0x8015);
                    gPauseRequest = 1;
                    break;
                case 0x100000000000:
                    seq_enter_start_level();
                    gPendingTune = music_tune_for_mode(gNextGameMode);
                    hd_mode_setup();
                    seq_start_level();
                    if (gGameMode == MODE_WORLD || levelno == 0x2F) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    hd_setup_level(0);
                    break;
                case 0x40000000000000:
                    hd_mode_setup();
                    osSendMesg(&gPakRequestQueue, (OSMesg) 0x01000010, 1);
                    osRecvMesg(&pakToGameMessageQ, &gEepromPresent, 1);
                    if (gEepromPresent != NULL) {
                        title_setup_logo();
                        fe_bg_load(1);
                        gYoshiFeItems[9].unkC = "ERASE SAVED GAME!", gYoshiFeItems[9].unk10 = D_803047B4, gYoshiFeItems[9].unk6 = gYoshiFeItems[9].unk8 = 20;
                        yoshi_demand(0x800C);
                    } else {
                        gGameModeAfterNext = MODE_BOOT_SCREEN1;
                    }
                    break;
                case 0x100000000000000:
                    gYoshiFeItems[9].unkC = "BECOME GUEST PLAYER:", gYoshiFeItems[9].unk10 = D_803047DC, gYoshiFeItems[9].unk6 = gYoshiFeItems[9].unk8 = 20;
                    yoshi_demand(0x800C);
                    break;
                case 0x400000:
                    fe_player_build_erase_menu();
                    yoshi_demand(0x800A);
                    break;
                case 8:
                    gPauseRequest = 1;
                    music_play_clear_tune();
                    break;
            }
            gPrevGameMode = gGameMode;
            gGameMode = gNextGameMode;
            gNextGameMode = gGameModeAfterNext;
        } while (gNextGameMode != 0);
        if (gPendingTune) {
            music_start_tune(gPendingTune);
        }
        while (gNextGameMode == 0) {
            gFrameStartTime = osGetTime();
            font_age_cache();
            if (gGameMode & MODE_WORLD) {
                gFrameStartLag = (osGetTime() - gLastRetraceTime) / 7825;
                fe_globe_frame();
                gFrameCpuTime = (osGetTime() - gFrameStartTime) / 7825;
                if (gRspSlotBusy[2]) {
                    rsp_wait_task(0x4D2);
                }
                if (gRspSlotBusy[0]) {
                    rsp_wait_task(0x4D2);
                }
                rsp_wait_task(0x4D2);
            } else if (gGameMode & 0xC9FD8FE7DBFF8080) {
                gFrameStartLag = (osGetTime() - gLastRetraceTime) / 7825;
                fe_back_loop();
                gFrameCpuTime = (osGetTime() - gFrameStartTime) / 7825;
                rsp_wait_task(0x4D2);
            } else if (gGameMode & MODE_VEHICLE_SELECT) {
                fe_digger_frame();
                rsp_wait_task(0x4D2);
            } else if (gGameMode & (MODE_BOOT_SCREEN1 | MODE_BOOT_SCREEN2)) {
                fe_boot_screen_frame();
                rsp_wait_task(0x4D2);
            } else {
                gFrameStartLag = (osGetTime() - gLastRetraceTime) / 7825;
                hd_game_frame();
                gFrameCpuTime = (osGetTime() - gFrameStartTime) / 7825;
                if (gRspSlotBusy[1]) {
                    rsp_wait_task(0x61F);
                }
                if (gRspSlotBusy[2]) {
                    rsp_wait_task(0x54D);
                }
                rsp_wait_task(0x4D2);
            }
            hd_check_stack();
            gFramesInMode++;
            D_80358064++;
            if (!gPaused) {
                gUnpausedFrames++;
            }
        }
        if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) && (gCarrierHit || gLevelDone)) {
            rec_record(0, 0, 0);
        }
        HD_ASSERT(!areWeFading(), 627);
        switch (gNextGameMode) {
            case 0x400000000000:
                seq_finish(1);
                break;
            case 0x200000000000:
                seq_finish(0);
                break;
            case 0x2000000000000:
                academy_next_mode();
                break;
        }
        if (playerNumber == playerNumberAtStart && gNextGameMode == MODE_WORLD && academy_advance_state()) {
            academy_announce_state();
        }
    }
}

/* One frame of the in-game modes: hands the frame built last time to the RSP
 * (rsp_send_task), pause handling, the updates (camera zoom, vehicles,
 * objects, mission state), then this frame's display list (hd_build_frame_dl
 * and the overlays) (0x802475D8). */
void hd_game_frame(void) {
    u32 vis;
    s32 i;
    s32 j;
    u8 found;
    Gfx *gdl;
    u8 zoom;
    s32 pos;
    s32 lo;
    s32 hi;
    s16 alpha;

    if (gDebugCheats && (gContButtons & 1) && !(gContPrevButtons & 1) && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        gLevelDoneRequest = 1;
        cmo_hit_request = 0;
        if (gContButtons & 8) {
            if (gLevelKind != 1 || !(LEVEL_DONE(levelno))) {
                gLevelDoneRequest = 0;
                cmo_hit_request = 1;
            }
        }
        if ((gContButtons & 0x2000) && gLevelDoneRequest) {
            gCheatAllDestroyed = 1;
            D_8036EA7C = gRduCount;
            D_8036EA79 = D_8036EB93;
            D_80365066 = 1;
            stats_set_level_flag(2);
            stats_set_level_flag(1);
        }
        if ((LEVEL_DONE(levelno)) && gLevelKind == 1 && gLevelDoneRequest) {
            gPauseRequest = 1;
            func_80275390(0x08000000);
        }
    }
    nextdma = 0;
    no_palette_dmas = 0;
    gCarrierContact = 0;
    if (gFramesInMode) {
        if (gBufferIndex == 0) {
            if (gFramesInMode >= 6) {
                cull_start(&gDynamicBufs[gBufferIndex].persp, &gDynamicBufs[gBufferIndex].cullView);
            }
        } else {
            i = 0;
            found = 0;
            while (!found) {
                if (gVehicles[i].type == gCurVehicle) {
                    found = 1;
                } else {
                    i++;
                }
            }
            j = 0;
            found = 0;
            while (!found) {
                if (gVehicleShadows[j].type == gCurVehicle) {
                    found = 1;
                } else {
                    j++;
                }
            }
            veh_render_shadow_tex(gVehicleShadows[j].unk0, gVehicleShadows[j].unk1004, gVehicleShadows[j].unk1010, gVehicleShadows[j].unk100C,
                          gVehicleShadows[j].unk1000, (Gfx *) N64_IPTR(((gVehicles + i)->unk54)), (gVehicles + i)->unk0, (gVehicles + i)->unk4);
        }
    }
    i = 0;
    found = 0;
    while (!found) {
        if (gVehicles[i].type == gCurVehicle) {
            found = 1;
        } else {
            i++;
        }
    }
    if (gBufferIndex == 0) {
        mb_render_view(gPlayerX, gPlayerY, gPlayerZ, gCamEyeX, gCamEyeY, gCamEyeZ, (void *) N64_IPTR((gVehicles[i].unk58)),
                      gVehicles[i].unk0, gVehicles[i].unk8, gVehicles[i].alpha);
    } else {
        mb_render_view(gPlayerX, gPlayerY, gPlayerZ, gCamEyeX, gCamEyeY, gCamEyeZ, (void *) N64_IPTR((gVehicles[i].unk58)),
                      gVehicles[i].unk0, gVehicles[i].unk4, gVehicles[i].alpha);
    }
    D_803649D8 = osGetTime();
    cont_start_read();
    rsp_send_task((u64 *) (gDynamicBufs[gBufferIndex].dl), gFrameDlLen, 3, 1, 0x4D2, 1);
    tex_reset_decode_queue();
    gBufferIndex ^= 1;
    switch (gPaused) {
        case 0:
            if (gPauseRequest) {
                gPaused = 1;
                gUnpauseRequest = 0;
                gPauseRequest = 0;
                D_803156F4 = gBufferIndex ^ 1;
                if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
                    if (!D_8036EB99) {
                        snd_set_slot_volume(0, 0);
                    }
                    music_fade_to(0.5f);
                    if (gGameMode == 4 && (gHaveCarrier || gHaveShuttle) && currentYoshiWindow == 0) {
                        gNextGameMode = MODE_MAP_VIEW;
                        cull_restart(10);
                        D_80364A78 = 0;
                    }
                }
            }
            break;
        case 1:
            if (gUnpauseRequest && gBufferIndex != D_803156F4) {
                gPauseRequest = 0;
                gUnpauseRequest = 0;
                gPaused = 0;
                if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
                    if (D_80358064) {
                        debug_printf("LOCKING KEYS!\n");
                        gSwallowB = 1;
                    }
                    snd_set_mode_slot_volumes(gGameMode);
                    music_fade_to(1.0f);
                    if (gGameMode == MODE_MAP_VIEW && (gHaveCarrier || gHaveShuttle)) {
                        gNextGameMode = 4;
                        cull_restart(10);
                    }
                    if (gUnpausedFrames == 0) {
                        D_80358064 = 0;
                    }
                }
            }
            break;
    }
    if (gPaused) {
        D_803156F5 = D_803156F4;
    } else {
        D_803156F5 = gBufferIndex;
    }
    if (gHaveCarrier && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        carrier_update_sound();
    }
    if (gHaveChopper && (gGameMode & 0x1905)) {
        chopper_update_sound();
    }
    util_check_board_zone();
    D_803649EE = 0;
    if (gCurVehicle == 0 && !(gGameMode & 0x1801)) {
        veh_apply_switch();
    }
    cont_update();
    if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
        zoom = 0;
        if ((gContButtons & 4) && !(gContButtons & 8)) {
            if (gCamZoom < 1.5 && gGameMode == 4) {
                zoom = 2;
                if (!(gContPrevButtons & 4)) {
                    snd_play(gSfxBank, 0xDC, NULL);
                }
            } else if (!(gContPrevButtons & 4) && gGameMode != MODE_MAP_VIEW) {
                zoom = 3;
            }
        }
        if (zoom == 0) {
            if ((gContButtons & 8) && !(gContButtons & 4)) {
                if (gCamZoom > 1.0 && gGameMode == 4 && D_80364A7C == 0) {
                    zoom = 1;
                    if (!(gContPrevButtons & 8)) {
                        snd_play(gSfxBank, 0xDD, NULL);
                    }
                } else if (!(gContPrevButtons & 8) && gGameMode == MODE_MAP_VIEW) {
                    zoom = 4;
                }
            } else {
                D_80364A7C = 0;
            }
        }
        switch (zoom) {
            case 0:
                break;
            case 2:
                gCamZoom = gCamZoom + 0.05;
                if (gCamZoom > 1.5) {
                    gCamZoom = 1.5f;
                }
                break;
            case 1:
                gCamZoom = gCamZoom - 0.05;
                if (gCamZoom < 1.0) {
                    gCamZoom = 1.0f;
                }
                break;
            case 3:
                if (gHaveCarrier || gHaveShuttle) {
                    cull_restart(10);
                    gNextGameMode = MODE_MAP_VIEW;
                    D_80364A78 = 0;
                    snd_play(gSfxBank, 0xDF, NULL);
                }
                break;
            case 4:
                D_80364A7C = 1;
                if (gHaveCarrier || gHaveShuttle) {
                    cull_restart(10);
                    gNextGameMode = 4;
                    snd_play(gSfxBank, 0xDF, NULL);
                }
                break;
        }
    }
    if (gGameMode == MODE_MAP_VIEW && gPaused && yoshiState == 2 && gHaveCarrier && currentYoshiWindow == 0) {
        if (!(gContButtons & 0x2000) && !(gContButtons & 0x10)) {
            D_80364A80 = 0;
        } else {
            D_80364A80 += 100;
            if (D_80364A80 > 800) {
                D_80364A80 = 800;
            }
            if (gContButtons & 0x2000) {
                D_80364A78 += D_80364A80;
            }
            if (gContButtons & 0x10) {
                D_80364A78 -= D_80364A80;
            }
        }
        pos = gCarrierZ + D_80364A78;
        lo = gCarrierStartZ - 400;
        hi = gCarrierStartZ + gCarrierRouteLen;
        if (pos > hi) {
            D_80364A78 = hi - gCarrierZ;
        }
        if (pos < lo) {
            D_80364A78 = lo - gCarrierZ;
        }
        if (gContButtons & 0x2010) {
            gYoshiWindows[0].unk4 = (gYoshiWindows[0].unk4 - 0x18 < -0x100) ? -0x100 : gYoshiWindows[0].unk4 - 0x18;
            gYoshiScale = (gYoshiScale * 0.8 > 0.1) ? gYoshiScale * 0.8 : 0.1;
            gYoshiWindows[0].unk8 &= ~0x20;
        } else {
            gYoshiWindows[0].unk4 = (gYoshiWindows[0].unk4 + 0x20 > 0x20) ? 0x20 : gYoshiWindows[0].unk4 + 0x20;
            gYoshiScale = (gYoshiScale / 0.8 < 1.0) ? gYoshiScale / 0.8 : 1.0;
            if (gYoshiScale == 1.0) {
                gYoshiWindows[0].unk8 |= 0x20;
            }
        }
    } else {
        D_80364A80 = 0;
        gYoshiWindows[0].unk4 = (gYoshiWindows[0].unk4 + 0x20 > 0x20) ? 0x20 : gYoshiWindows[0].unk4 + 0x20;
        gYoshiScale = (gYoshiScale / 0.8 < 1.0) ? gYoshiScale / 0.8 : 1.0;
        if (gYoshiScale == 1.0) {
            gYoshiWindows[0].unk8 |= 0x20;
        }
    }
    if (gHeldZ && !gPaused && gCurVehicle && !D_8036443C && (gLevelKind == 1 || gLevelKind == 0x80)) {
        veh_leave();
    }
    veh_update_others();
    if ((gContButtons & 0x4000) && !(gContPrevButtons & 0x4000) && !areWeFading() && gNextGameMode == 0) {
        switch (gGameMode) {
            case 0x100000000000:
                if (levelno != 0x2F && levelno != 0x31 && (levelno != 0x26 || gPrevGameMode == MODE_WORLD)) {
                    snd_play(gSfxBank, 0xDE, NULL);
                    gNextGameMode = MODE_WORLD;
                }
                break;
            case 0x40:
            case 0x400:
                snd_play(gSfxBank, 0xDE, NULL);
                if (LEVEL_DONE(levelno)) {
                    gNextGameMode = MODE_RESULTS;
                } else {
                    gNextGameMode = MODE_WORLD;
                }
                break;
            case 0x800:
            case 0x1000:
                if (levelno != 0x32 || gPrevGameMode == MODE_WORLD) {
                    snd_play(gSfxBank, 0xDE, NULL);
                    gNextGameMode = MODE_WORLD;
                }
                break;
        }
    }
    if (gHaveChopper && (!gPaused || (gGameMode & 0x1801))) {
        chopper_update();
        if (gGameMode & 0x1801) {
            if (!areWeFading()) {
                if (gNextGameMode == 0 || (gNextGameMode & 0x1801)) {
                    driver_update();
                }
                chopper_update_mode();
            }
        } else {
            people_update();
        }
    }
    if (!gPaused && !D_80364A84) {
        veh_update_player();
    }
    if (gGhostRecording && !gPaused) {
        ghost_record();
    }
    if (gHaveCarrier && (gLevelKind & 0x81)) {
        if (!(gGameMode & 0x1801)) {
            carrier_update();
        }
        if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) && !yoshi_get_demand() && !gPaused && !gLevelDone && !gCarrierHit) {
            func_8024A92C(vis = obj_find_carrier_target());
        }
    }
    if (gHaveShuttle && !gPaused) {
        shuttle_update();
        if (levelno == 0x32 && yoshiState == 1 && gFramesInMode > 50 && (gGameMode & 0x1801) && !areWeFading()) {
            func_80275390(0x2000);
        }
    }
    anim_run_mtx_copies();
    if (!gPaused) {
        gVehSlowed = 0;
        gPathBuildingsDestroyed = obj_path_buildings_destroyed();
        obj_check_level_end();
        veh_update_riders();
        tnt_ride_vehicle(gCurVehicle);
        block_ride_vehicle(gCurVehicle);
        util_cam_path_step(levelno);
        if (!(gGameMode & 0x1801)) {
            rdu_update();
        }
        fx_update_power_pill();
        if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
            academy_check_marker(gPlayerX, gPlayerY, gPlayerZ);
        }
#ifdef NON_MATCHING
        /* The asm takes its param (the texture decoder's type-4/5 table) in
         * $fp: 0 here in every level (traced). */
        anim_tex_update_all(LEAKED(s8));
#else
        anim_tex_update_all();
#endif
        if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) && (gLevelKind & 1)) {
            func_8024A348();
            func_8024ADD8();
        }
        if (gLevelKind != 1 && gTurboActive && (u32) (((s32) D_803156C0) - D_80364A58) > 90) {
            gTurboActive = 0;
            mb_stop();
        }
    }
    if ((gCarrierHit || gLevelDone || (levelno == 0x32 && gTargetsCleared && !(LEVEL_DONE(0x32)))) && gLevelKind == 1 && D_80364A5C == 0) {
        debug_printf("TIME IN LEVEL=%d\n", D_80364A5C = ((s32) D_803156C0) - D_80364A58);
    }
    cam_apply_height_zone(gLevelData);
    if (gGameMode & 0x1801) {
        cull_restart(2);
    }
    if (currentYoshiWindow == 0x58 && yoshiState != 8 && !gCamFixedEye) {
        if (!D_80364A85) {
            cull_restart(10);
        }
    } else if (D_80364A85) {
        cull_restart(10);
    }
    obj_draw_all((u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x1E0]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x578]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x910]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x112B0]),
                  (s32) &gDynamicBufs[gBufferIndex], (s32) (&gDynamicBufs[gBufferIndex].unk2C0[0x2D]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x17070]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x170D8]));
    if (gGameMode == 4) {
        cull_override_visible_cells(levelno, gPlayerX, gPlayerZ);
    }
    lvl_build_visible_dls((u32 *) N64_IPTR(gLevelDl0[gBufferIndex]), (u32 *) N64_IPTR(gLevelDl1[gBufferIndex]), (u32 *) N64_IPTR(gLevelDl2[gBufferIndex]), (u32 *) N64_IPTR(gLevelDl3[gBufferIndex]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x140]));
    water_update(gBufferIndex);
    if (!gPaused || (gHaveCarrier && gCarrierHit) || D_8036EB99) {
        obj_run_effect_queue();
        obj_update_delayed_damage();
    }
    if (!gPaused) {
        tnt_update(gCurVehicle);
        block_update(gCurVehicle);
        shell_update_all();
        crate_update(gCurVehicle);
    }
    if (gLevelKind != 1) {
        if (gGameMode & 0x40) {
            hud_replay_turbo();
        }
        if (gGameMode & 0x04002104) {
            hud_update_objective();
        }
    } else if (gFailTrigger) {
        if (!(LEVEL_DONE(levelno))) {
            cmo_hit_request = 1;
        } else if (!areWeFading()) {
            gLevelDoneRequest = 1;
            snd_play(gSfxBank, 0x2B, NULL);
            if (gGameMode == 0x40) {
                func_80275390(0x40);
            } else {
                func_80275390(0x08000000);
            }
        }
    }
    fx_draw_all();
    ptcl_update();
    if (!gPaused) {
        commpt_update();
    }
    mb_push_sample(gPlayerX, gPlayerY, gPlayerZ, gCamEyeX, gCamEyeY, gCamEyeZ);
    dig_coach_update(gFramesInMode);
    bridge_update_levels(levelno);
    lvl_groups_update_tris();
    if (!gPaused) {
        util_check_hint_events();
    }
    gdl = hd_build_frame_dl(&gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if ((gGameMode & 0x04002444) && gLevelKind != 1) {
        gdl = hud_draw_objective(gdl, (Gfx * N64P *) &gDynamicBufs[gBufferIndex], (u32 *) &gFrameDlLen);
    }
    if (gGameMode & (MODE_ATTRACT | MODE_GLOBE_SEQ)) {
        if (gGameMode == 2) {
            PORT_ITAG(gdl, PORT_ITAG_NONE, 0); /* the attract mode's bobbing logo: blended */
            gdl = title_draw_attract(gdl, (s32) &gDynamicBufs[gBufferIndex], gBufferIndex, &gFrameDlLen);
            PORT_ITAG(gdl, PORT_ITAG_HUD, 0);
        }
        if (D_802E8BF0 || gLogoState == 3 || D_802E8BEC >= 9) {
            if ((yoshiState == 1 || yoshiState == 8 || D_802E8BEC >= 9) && (!D_802E8BF0 || gFramesInMode <= D_80366A04)) {
                if (gPadDisplayAlpha + 20 > 0xFF) {
                    gPadDisplayAlpha = 0xFF;
                } else {
                    gPadDisplayAlpha += 20;
                }
            } else if (gPadDisplayAlpha - 20 < 0) {
                gPadDisplayAlpha = 0;
            } else {
                gPadDisplayAlpha -= 20;
            }
        }
        if (gPadDisplayAlpha && D_802E8BEC && gPadDisplayLoaded) {
            PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
            gdl = hud_draw_pad((s32) &gDynamicBufs[gBufferIndex], gdl, gPadDisplayAlpha / 2 - 103,
                                0x55 - ((gGameMode == MODE_GLOBE_SEQ) << 5), gPadDisplayAlpha);
        }
        if (D_802E8BEC == 0 && gLogoState == 3 && gGameMode == 2) {
            PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0);
            gdl = hud_draw_demo_banner((s32) gDynamicBufs, gdl);
        }
        if (gGameMode == 2 || levelno == 0x2F) {
            hud_attract_messages();
        }
    }
    if (gGameMode & 0x1801) {
        if (gFramesInMode == 150 && levelno != 0x32) {
            yoshi_demand(0x8040);
        }
        if (!players[playerNumber].unk91 && gFramesInMode == 10) {
            yoshi_demand(0x803F);
        }
    }
    PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0);
    gdl = func_8024C404(gdl, &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if ((gGameMode & 0x440) && D_803156C4 - D_80364A54 > 160 && !D_80364A50) {
        D_80364A50 = 1;
        if (yoshiState == 1) {
            if (!(LEVEL_DONE(levelno)) || gLevelKind == 1) {
                yoshi_demand(D_802F4870[lowest_set_bit(gLevelKind)] | 0x8000);
            }
        }
    }
    if (gGameMode == 8 && !areWeFading() && (!func_802D4E10(gMusicPlayer) || D_803156C4 - gDoneTuneStartTime > 300)) {
        func_80275390(0x08000000);
    }
    if (levelno == 0x31 && yoshiState == 1) {
        gdl = credits_draw((Gfx * N64P *) &gDynamicBufs[gBufferIndex], gdl);
    }
    PORT_WTAG(gdl, PORT_WTAG_TEXT_SIDES, 0, 0);
    text_flush_screen(&gdl, (Mtx *) &gDynamicBufs[gBufferIndex]);
    PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0);
    if (gGameMode != 2) {
        gdl = func_80274BF0((s32) &gDynamicBufs[gBufferIndex], gdl);
    }
    if (gGameMode == MODE_YOSHI_PAGES) {
        if ((gContButtons & 0x10) && !(gContPrevButtons & 0x10)) {
            if (currentYoshiWindow < 0x6B && yoshiState != 8 && yoshiState != 1) {
                yoshi_demand((currentYoshiWindow + 1) | 0x8000);
                snd_play(gSfxBank, 0x1D, NULL);
            } else {
                snd_play(gSfxBank, 0xD0, NULL);
            }
        }
        if ((gContButtons & 0x2000) && !(gContPrevButtons & 0x2000)) {
            if (currentYoshiWindow >= 0x5F && yoshiState != 8 && yoshiState != 1) {
                yoshi_demand((currentYoshiWindow - 1) | 0x8000);
                snd_play(gSfxBank, 0x1D, NULL);
            } else {
                snd_play(gSfxBank, 0xD0, NULL);
            }
        }
    }
    if ((((gContButtons & 0x1000) && !(gContPrevButtons & 0x1000)) || ((gContButtons & 0x8000) && !(gContPrevButtons & 0x8000))) &&
        !areWeFading() && gYoshiPendingWindow == -1 && gNextGameMode == 0 && !cmo_hit_request && !gLevelDoneRequest && currentYoshiWindow != 0) {
        if ((yoshi_get_demand() & 0x8000) && (yoshi_get_demand() & 1)) {
            yoshi_get_demand();
        }
        switch (gGameMode) {
            case 1:
            case 0x800:
            case 0x1000:
                if (levelno != 0x32 || gPrevGameMode == MODE_WORLD) {
                    gNextGameMode = MODE_LEVEL_START;
                    snd_play(gSfxBank, 0x1E, NULL);
                }
                break;
            case 0x40:
            case 0x400:
                if ((LEVEL_DONE(levelno)) && gLevelKind != 1) {
                    gNextGameMode = MODE_RESULTS;
                    snd_play(gSfxBank, 0x1E, NULL);
                } else if (yoshiState == 1) {
                    yoshi_demand(D_802F4870[lowest_set_bit(gLevelKind)] | 0x8000);
                }
                break;
            case 2:
                if (D_80366A18) {
                    snd_play(gSfxBank, 0x1E, NULL);
                    snd_set_slot_volume(0, 0);
                    snd_set_slot_volume(5, 0);
                    func_80275390(0x20000000000000);
                }
                break;
            case 0x100000000000:
                if (D_803A6B04) {
                    gNextGameMode = 0x400000000000;
                }
                break;
            case 4:
            case 0x100:
                if ((gContButtons & 0x1000) && !gPaused) {
                    if ((LEVEL_DONE(levelno)) && gLevelKind == 1) {
                        gYoshiGameItems[2].unk0 |= 1;
                        gYoshiGameItems[2].unk18 = 7;
                    } else {
                        gYoshiGameItems[2].unk0 &= ~1;
                        gYoshiGameItems[2].unk18 = 8;
                    }
                    yoshi_demand(0x8000);
                }
                break;
        }
    }
    gdl = yoshi_draw(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if (yoshiSelection && !cmo_hit_request && !gLevelDoneRequest) {
        func_8024BDA4(&yoshiSelection);
    }
    if ((gGameMode & 0x444) && gYoshiPendingWindow == -1 && gTurboStartState == 0 &&
        !((currentYoshiWindow != -1) ? gYoshiWindows[currentYoshiWindow].unk8 & 0x01000000 : 0)) {
        alpha = (gGameMode & 0x440) ? 0xB4 : 0xFF;
        if (gHudAlpha < alpha) {
            gHudAlpha = (s32) ((alpha < gHudAlpha + 16) ? alpha : gHudAlpha + 16);
        } else {
            gHudAlpha = alpha;
        }
    } else if (gHudAlpha >= 0) {
        gHudAlpha = (s32) ((gHudAlpha - 16 < 0) ? 0 : gHudAlpha - 16);
    } else {
        gHudAlpha = 0;
    }
    if (gGameMode == 2) {
        gdl = func_80274BF0((s32) &gDynamicBufs[gBufferIndex], gdl);
    }
    hd_close_display_list(gdl, &gFrameDlLen);
    for (i = 0; i < nextdma; i++) {
        osRecvMesg(&textureDmaMessageQ, NULL, 1);
    }
    HD_ASSERT(MQ_IS_EMPTY(&textureDmaMessageQ), 1509);
    HD_ASSERT(nextdma-no_palette_dmas<NUM_TEXTURE_DMAS, 1510);
    for (i = 0; i < nextdma - no_palette_dmas; i++) {
        tex_decode_next();
    }
    D_803649E0 = D_803649D8 % 40 - 20;
    D_803649E2 = D_803649D8 / 100 % 40 - 20;
    D_803649E4 = D_803649D8 / 10000 % 40 - 20;
    D_803643D8 = gCarrierHit;
    if (!gLevelDone && !gCarrierHit) {
        if (gLevelDoneRequest) {
            HD_ASSERT(!cmo_hit_request, 1529);
            gLevelDone = 1;
        } else if (cmo_hit_request) {
            gCarrierHit = 1;
        }
        gLevelDoneRequest = 0;
        cmo_hit_request = 0;
    }
}

/* On-screen message queue: posts D_80364A40 and fades the current one */
void func_8024A348(void) {
    s32 pad;

    if (D_80364A40 != 0) {
        if (D_80364A3D + 1 != D_80364A3C && (D_80364A3D != 4 || D_80364A3C != 0)) {
            D_80364A00[D_80364A3D].unk0 = D_80364A40;
            D_80364A00[D_80364A3D].unk9 = 30;
            if (D_8036443E >= 0xE00 || D_8036443E < 0x200) {
                if (gLevelKind == 8) {
                    D_80364A00[D_80364A3D].unk4 = 30;
                    D_80364A00[D_80364A3D].unk6 = 60;
                    D_80364A00[D_80364A3D].unk8 = 1;
                } else {
                    D_80364A00[D_80364A3D].unk4 = 30;
                    D_80364A00[D_80364A3D].unk6 = 30;
                    D_80364A00[D_80364A3D].unk8 = 1;
                }
            }
            if (D_8036443E >= 0x200 && D_8036443E < 0x600) {
                D_80364A00[D_80364A3D].unk4 = 30;
                D_80364A00[D_80364A3D].unk6 = 160;
                D_80364A00[D_80364A3D].unk8 = 1;
            }
            if (D_8036443E >= 0x600 && D_8036443E < 0xA00) {
                D_80364A00[D_80364A3D].unk4 = 250;
                D_80364A00[D_80364A3D].unk6 = 160;
                D_80364A00[D_80364A3D].unk8 = 0;
            }
            if (D_8036443E >= 0xA00 && D_8036443E < 0xE00) {
                D_80364A00[D_80364A3D].unk4 = 250;
                D_80364A00[D_80364A3D].unk6 = 37;
                D_80364A00[D_80364A3D].unk8 = 0;
            }
            D_80364A3D++;
            if (D_80364A3D == 5) {
                D_80364A3D = 0;
            }
        }
        D_80364A40 = 0;
    }
    if (D_80364A3C != D_80364A3D) {
        D_80364A44 = D_80364A00[D_80364A3C].unk0;
        D_80364A4A = D_80364A00[D_80364A3C].unk4;
        D_80364A4C = D_80364A00[D_80364A3C].unk6;
        D_80364A4E = D_80364A00[D_80364A3C].unk8;
        if (D_80364A00[D_80364A3C].unk9 >= 16) {
            D_80364A48 = 255.0f - (f32) (D_80364A00[D_80364A3C].unk9 - 15) / 15.0f * 255.0f;
        } else {
            D_80364A48 = (f32) D_80364A00[D_80364A3C].unk9 / 15.0f * 255.0f;
        }
        if (!(D_80364A00[D_80364A3C].unk9--)) {
            D_80364A3C++;
            if (D_80364A3C == 5) {
                D_80364A3C = 0;
            }
        }
    } else {
        D_80364A44 = 0;
    }
}

/* Picks the music intensity from the distance to the carrier (CMO) */
void func_8024A92C(u32 dist) {
    u32 far;
    u32 near;

    switch (levelno) {
        case 29:
            far = 22000;
            near = 14000;
            break;
        case 12:
            far = 26000;
            near = 20000;
            break;
        case 58:
            far = 36000;
            near = 30000;
            break;
        case 18:
            far = 36000;
            near = 28000;
            break;
        case 16:
            far = 33000;
            near = 23000;
            break;
        case 15:
            far = 24000;
            near = 18000;
            break;
        case 13:
            far = 55000;
            near = 35000;
            break;
        case 14:
            if (gCarrierZ > 177600) {
                far = 25000;
                near = 20000;
            } else if (gCarrierZ > 80000) {
                far = 22000;
                near = 17500;
            } else {
                far = 14000;
                near = 8000;
            }
            break;
        case 9:
            if (gCarrierZ > 100320) {
                far = 36000;
                near = 18000;
            } else {
                far = 12000;
                near = 6000;
            }
            break;
        default:
            far = 12000;
            near = 6000;
            break;
    }
    D_803153F0 = D_80364A6F;
    if (dist < far) {
        if (dist < near) {
            D_80364A6F = 2;
        } else {
            D_80364A6F = 1;
        }
    } else {
        D_80364A6F = 0;
    }
    if (D_80364A6F != D_803153F0) {
        if (D_803153F0 == 0 && util_dist2d(gCarrierX, gCarrierZ, gPlayerX, gPlayerZ) > 20000) {
            hud_panel_start(1, 1, 7, 0x6C);
            debug_printf("TOO FAR AWAY FROM CMO\n");
        }
        switch (D_80364A6F) {
            case 1:
                switch (D_803153F0) {
                    case 0:
                        music_push_danger_tune();
                    case 2:
                        snd_stop((void *) N64_IPTR(gCarrierAlarmSound));
                        break;
                }
                break;
            case 2:
                switch (D_803153F0) {
                    case 0:
                        music_push_danger_tune();
                    case 1:
                        snd_play(gSfxBank, 0x26, &gCarrierAlarmSound);
                        break;
                }
                break;
            case 0:
                snd_stop((void *) N64_IPTR(gCarrierAlarmSound));
                if (gLevelInfo[levelno].unk0 != 0x80 || !obj_path_buildings_destroyed()) {
                    debug_printf("popTuneImmediate();\n");
                    popTuneImmediate();
                } else {
                    debug_printf("WILL THIS FIX IT!!?\n");
                }
                if (D_803153F0 == 2) {
                    snd_play(gSfxBank, 0x68, NULL);
                }
                break;
        }
    }
    switch (D_80364A6F) {
        case 1:
            if (!yoshi_show_hint(0x49) && currentYoshiWindow != 3) {
                yoshi_demand(0x8003);
            }
            break;
        case 2:
            if (currentYoshiWindow != 2) {
                yoshi_demand(0x8002);
            }
            break;
        case 0:
            if ((yoshiState == 2 || yoshiState == 4) && !yoshi_get_demand() &&
                (currentYoshiWindow == 3 || currentYoshiWindow == 2 || currentYoshiWindow == 0x49)) {
                yoshi_demand(0x4000);
            }
            break;
    }
}

/* Eases D_803649F4 towards D_803649F0 by a tenth of the gap plus 10 */
void func_8024ADD8(void) {
    u32 d;

    d = D_803649F0 - D_803649F4;
    D_803649F4 = d / 10 + D_803649F4;
    D_803649F4 += 10;
    if (D_803649F0 < D_803649F4) {
        D_803649F4 = D_803649F0;
    }
}

/* Applies a pending vehicle switch (gNextVehicle): the type's enter routine
 * (veh_enter), the player's flag for the type (0x8024AE2C). */
void veh_apply_switch(void) {
    s32 pad;

    if (gNextVehicle != 0 && gNextVehicle != 0xFF) {
        if (veh_has_partial_rider(gNextVehicle) == 0) {
            veh_attach_riders(gNextVehicle);
            tnt_attach_to_vehicle(gNextVehicle);
            block_attach_to_vehicle(gNextVehicle);
            func_8028B720();
            mb_cancel();
            if (veh_enter(gNextVehicle)) {
                gCurVehicle = gNextVehicle;
                driver_stop_blend_anim();
                if (!gPaused && gFramesInMode >= 11 &&
                    (gCurVehicle == 8 || gCurVehicle == 15 || gCurVehicle == 13 || gCurVehicle == 14)) {
                    yoshi_show_hint(0x4A);
                }
            }
            players[playerNumber].unk10 |= 1 << gCurVehicle;
            D_803649E8 = 1;
            D_803649EC = 1;
            D_803649EE = 1;
        } else if (!snd_get_state((void *) N64_IPTR(gEnterRefusedSound))) {
            snd_play(gSfxBank, 0x2B, &gEnterRefusedSound);
        }
    }
}

/* Calls the type's enter routine ("changing to digger %d": the game's word
 * for its vehicles); returns 0 if id is not a vehicle (0x8024AFA8). */
s32 veh_enter(s32 id) {
    u8 ok = 1;

    switch (id) {
        case 1:
            sideswipe_enter();
            break;
        case 2:
            thunderfist_enter();
            break;
        case 3:
            skyfall_enter();
            break;
        case 4:
            ramdozer_enter();
            break;
        case 5:
            backlash_enter();
            break;
        case 6:
            crane_enter();
            break;
        case 7:
#ifdef NON_MATCHING
            train_enter(LEAKED(a0));
#else
            train_enter();
#endif
            break;
        case 8:
#ifdef NON_MATCHING
            amdream_enter(LEAKED(a0));
#else
            amdream_enter();
#endif
            break;
        case 9:
            jbomb_enter();
            break;
        case 10:
            ballista_enter();
            break;
        case 11:
        case 17:
        case 18:
#ifdef NON_MATCHING
            barge_enter(LEAKED(a0));
#else
            barge_enter();
#endif
            break;
        case 13:
#ifdef NON_MATCHING
            police_enter(LEAKED(a0));
#else
            police_enter();
#endif
            break;
        case 14:
#ifdef NON_MATCHING
            ateam_enter(LEAKED(a0));
#else
            ateam_enter();
#endif
            break;
        case 15:
#ifdef NON_MATCHING
            starski_enter(LEAKED(a0));
#else
            starski_enter();
#endif
            break;
        case 16:
            cyclone_enter();
            break;
        default:
            ok = 0;
            break;
    }
    if (ok && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        rec_record((players[playerNumber].unkF0 & (1 << id)) ? 0x80 : 0x40, 0, 0);
    }
    if (ok) {
        debug_printf("changing to digger %d\n", id);
    }
    return ok;
}

#if defined(NON_MATCHING) && defined(PORT_HOST)
/* Windows port, byte order: veh_leave reads the word table that starts
 * at the u8 EEPROM flag gUseEeprom (set at boot), and on the N64 that flag is
 * word 0's most significant byte.  The image keeps word 0 big-endian
 * (port/data/image_overrides.txt), so word 0 is assembled from its bytes;
 * the others are host-order words.  Checked by port/tools/loadref.py (T5). */
s32 port_802E8BF8_word(s32 i) {
    u8 *t = &gUseEeprom;

    if (i == 0) {
        return (t[0] << 24) | (t[1] << 16) | (t[2] << 8) | t[3];
    }
    return ((s32 *) t)[i];
}
#endif

/* Leaves the current vehicle (0x8024B188). */
void veh_leave(void) {
    u8 res = 0;
    u8 ok = 0;

    res = veh_exit_check();
    if (res == 1) {
#if defined(NON_MATCHING) && defined(PORT_HOST)
        ok = driver_try_exit(port_802E8BF8_word(gCurVehicle));
#else
        ok = driver_try_exit(((s32 *) &gUseEeprom)[gCurVehicle]);
#endif
    }
    if (ok) {
        D_803649E8 = 0;
        mb_cancel();
        tnt_detach_all();
        block_detach_all();
        func_8028B720();
        switch (gCurVehicle) {
            case 4:
                ramdozer_leave();
                break;
            case 3:
                skyfall_leave();
                break;
            case 5:
                backlash_leave();
                break;
            case 2:
                thunderfist_leave();
                break;
            case 1:
                sideswipe_leave();
                break;
            case 7:
                train_leave();
                break;
            case 8:
                amdream_leave();
                break;
            case 9:
                jbomb_leave();
                break;
            case 10:
                ballista_leave();
                break;
            case 11:
            case 17:
            case 18:
                barge_leave(gCurVehicle);
                break;
            case 13:
                police_leave();
                break;
            case 14:
                ateam_leave();
                break;
            case 6:
                crane_leave();
                if (veh_is_loaded(4)) {
                    ramdozer_leave();
                }
                if (veh_is_loaded(13)) {
                    police_leave();
                }
                break;
            case 15:
                starski_leave();
                break;
            case 16:
                cyclone_leave();
                break;
        }
        gCurVehicle = 0;
        if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
            rec_record((players[playerNumber].unkF0 & 1) ? 0x80 : 0x40, 0, 0);
        }
    }
    if (res == 0 || (res != 2 && !ok)) {
        if (!snd_get_state((void *) N64_IPTR(gLeaveRefusedSound))) {
            snd_play(gSfxBank, 0x2B, &gLeaveRefusedSound);
        }
    }
}

/* 1 if some gVehicles record has type id (0x8024B418).
 * Is any entry of gVehicles (up to gVehiclesEnd) tagged with id? */
s32 veh_is_loaded(u8 id) {
    s32 i;

    i = 0;
    while (&gVehicles[i] != gVehiclesEnd) {
        if (gVehicles[i].type == id) {
            return 1;
        }
        i++;
    }
    return 0;
}

/* Per-vehicle exit check for the current vehicle (gCurVehicle). Declared u8
 * but has no return statement: callers read the handler's leftover v0. */
#ifdef NON_MATCHING
/* The current type's exit check (0 no, 1 yes, 2 silently no) (0x8024B4B8). */
u8 veh_exit_check(void) {
    /* An id without a handler returns whatever v0 held in the original
     * (indeterminate; only vehicle ids reach this). */
    s32 ret = 0;

    switch (gCurVehicle) {
        case 4:
            ret = ramdozer_exit_check();
            break;
        case 3:
            ret = skyfall_exit_check();
            break;
        case 5:
            ret = backlash_exit_check();
            break;
        case 2:
            ret = thunderfist_exit_check();
            break;
        case 1:
            ret = sideswipe_exit_check();
            break;
        case 6:
            ret = crane_exit_check();
            break;
        case 7:
            ret = train_exit_check();
            break;
        case 8:
            ret = amdream_exit_check();
            break;
        case 9:
            ret = jbomb_exit_check();
            break;
        case 10:
            ret = ballista_exit_check();
            break;
        case 11:
        case 17:
        case 18:
            ret = barge_exit_check();
            break;
        case 13:
            ret = police_exit_check();
            break;
        case 14:
            ret = ateam_exit_check();
            break;
        case 15:
            ret = starski_exit_check();
            break;
        case 16:
            ret = cyclone_exit_check();
            break;
    }
    return ret;
}
#else
u8 veh_exit_check(void) {
    switch (gCurVehicle) {
        case 4:
            ramdozer_exit_check();
            break;
        case 3:
            skyfall_exit_check();
            break;
        case 5:
            backlash_exit_check();
            break;
        case 2:
            thunderfist_exit_check();
            break;
        case 1:
            sideswipe_exit_check();
            break;
        case 6:
            crane_exit_check();
            break;
        case 7:
            train_exit_check();
            break;
        case 8:
            amdream_exit_check();
            break;
        case 9:
            jbomb_exit_check();
            break;
        case 10:
            ballista_exit_check();
            break;
        case 11:
        case 17:
        case 18:
            barge_exit_check();
            break;
        case 13:
            police_exit_check();
            break;
        case 14:
            ateam_exit_check();
            break;
        case 15:
            starski_exit_check();
            break;
        case 16:
            cyclone_exit_check();
            break;
    }
}
#endif


/* Vehicles standing on the player's vehicle follow it (0x8024B5E8). */
void veh_update_riders(void) {
    gRiderPass = 1;
    veh_move_riders(gCurVehicle);
}

#ifdef NON_MATCHING
/* Leaked-register inputs of the hand-asm vehicle functions called below (and
 * anim_tex_update_all in hd_game_frame), which this file declares `void (void)`
 * (traced in the original ROM with mupen64plus breakpoints over attract demos
 * 0-8, nine levels; see the port notes, trace_values.md):
 *  - $fp ($s8): 0 at veh_update_others, veh_update_player and anim_tex_update_all in every
 *    level (hd_game_frame and the IDO code above it never set $s8, and the
 *    hand asm they call before these points restores it). The re-ground
 *    functions of veh_update_others save and restore fp themselves, so every one
 *    of them gets 0. It reaches state: veh_reground stores it into
 *    gSlotSurface[0..2] and veh+0x50 = their average (the ground byte), and
 *    anim_tex_update_all hands it to the texture decoder as the type-4/5 table.
 *  - starski_reground's a3 (the previous loop iteration's leftover; only read by
 *    starski_build_model for a zero-matrix point) and its FP pass-through state
 *    f12-f26 (only FP side results of the triangle scans): 0. Not reached in
 *    the demos.
 *  - the per-frame updates' zone registers t6, t7, s0-s4 (and f14-f26 for
 *    thunderfist_update / starski_update / cyclone_update). Measured: t6 = this
 *    switch's case label (the jump-table target, e.g. 0x8024B7F8), t7 = the
 *    level's leftover small integer (2..7, also in t8), s0 = 0, s1 = 1 / 0x30
 *    / 0x3A, s2/s3/s4 = 0x803B8D40 / 0x803B8578 / 0x803B8570. They only pass
 *    through veh_zone_alpha (unchanged when the level has no zones, as in 8 of
 *    the 9 demo levels) into the type-1 fx_spawn effect records, which
 *    never store them (and the FP state only feeds FP side results): dead.
 *    0, as the other updates (driver_update, backlash_update, ...) use. */
#endif

/* The per-frame routine of every placed vehicle but the player's (by type)
 * (0x8024B618). */
void veh_update_others(void) {
    s32 i;
    s32 id;

    i = 0;
    while (&gVehicles[i] != gVehiclesEnd) {
        if (gVehicles[i].onMovingPart != 0) {
            id = gVehicles[i].type;
            if (id != gCurVehicle && id != 0xFF && id != 0xFE && id != 0) {
                switch (id) {
                    case 5:
                        backlash_reground();
                        break;
                    case 1:
#ifdef NON_MATCHING
                        sideswipe_reground(LEAKED(fp));
#else
                        sideswipe_reground();
#endif
                        break;
                    case 4:
#ifdef NON_MATCHING
                        ramdozer_reground(LEAKED(fp));
#else
                        ramdozer_reground();
#endif
                        break;
                    case 8:
                        amdream_reground();
                        break;
                    case 13:
                        police_reground();
                        break;
                    case 14:
                        ateam_reground();
                        break;
                    case 15:
#ifdef NON_MATCHING
                        starski_reground(LEAKED(fp), LEAKED(a3), LEAKED(f12), LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24),
                                      LEAKED(f26));
#else
                        starski_reground();
#endif
                        break;
                    case 9:
#ifdef NON_MATCHING
                        jbomb_reground(LEAKED(fp));
#else
                        jbomb_reground();
#endif
                        break;
                    case 3:
#ifdef NON_MATCHING
                        skyfall_reground(LEAKED(fp));
#else
                        skyfall_reground();
#endif
                        break;
                    default:
                        debug_printf("MOVEABLE GEOMETRY MOVE ROUTINE NOT WRITTEN YET\n");
                        break;
                }
            }
        }
        i++;
    }
}

/* The per-frame routine of the player's vehicle (by gCurVehicle; 0: on
 * foot) (0x8024B7AC). */
void veh_update_player(void) {
    gRiderPass = 0;
    switch (gCurVehicle) {
        case 0:
            gNextVehicle = 0;
            driver_update();
            break;
        case 1:
#ifdef NON_MATCHING
            sideswipe_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            sideswipe_update();
#endif
            break;
        case 2:
#ifdef NON_MATCHING
            thunderfist_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            thunderfist_update();
#endif
            break;
        case 3:
#ifdef NON_MATCHING
            skyfall_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            skyfall_update();
#endif
            break;
        case 4:
#ifdef NON_MATCHING
            ramdozer_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            ramdozer_update();
#endif
            break;
        case 5:
            backlash_update();
            break;
        case 6:
            crane_update();
            break;
        case 7:
            train_update();
            break;
        case 8:
            amdream_update();
            break;
        case 9:
            jbomb_update();
            break;
        case 10:
            ballista_update();
            break;
        case 11:
        case 17:
        case 18:
            barge_update(gCurVehicle);
            break;
        case 13:
            police_update();
            break;
        case 14:
            ateam_update();
            break;
        case 15:
#ifdef NON_MATCHING
            starski_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            starski_update();
#endif
            break;
        case 16:
#ifdef NON_MATCHING
            cyclone_update(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            cyclone_update();
#endif
            break;
    }
}

/* Each frame (hd_game_frame): the visibility test's display list, a box of 8
 * vertices (12 triangles) under this frame's projection and culling view;
 * cull_step_delayed fills in the box per block of level cells (0x8024B8F4). */
void cull_start(Mtx *proj, Mtx *view) {
    Gfx dl[50];
    Gfx *gdl = dl;
    Vtx v[8];
    s32 i;

    for (i = 0; i < 8; i++) {
        v[i].v.flag = 0;
        v[i].v.tc[0] = 0;
        v[i].v.tc[1] = 0;
        v[i].v.cn[0] = 0;
        v[i].v.cn[1] = 0;
        v[i].v.cn[2] = 0;
        v[i].v.cn[3] = 0;
    }
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 1, osVirtualToPhysical(gStaticSegment));
    gSPDisplayList(gdl++, D_01000010);
    gSPMatrix(gdl++, PHYS(proj), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, gPerspNorm);
    gSPMatrix(gdl++, PHYS(view), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPVertex(gdl++, PHYS(v), 8, 0);
    gSP1Triangle(gdl++, 0, 1, 4, 0);
    gSP1Triangle(gdl++, 1, 4, 5, 0);
    gSP1Triangle(gdl++, 0, 3, 4, 0);
    gSP1Triangle(gdl++, 3, 4, 7, 0);
    gSP1Triangle(gdl++, 2, 3, 7, 0);
    gSP1Triangle(gdl++, 2, 6, 7, 0);
    gSP1Triangle(gdl++, 1, 2, 5, 0);
    gSP1Triangle(gdl++, 2, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 6, 7, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPTileSync(gdl++);
    gSPEndDisplayList(gdl++);
    osWritebackDCache(dl, (s32) (gdl - dl) * sizeof(Gfx));
    osWritebackDCache(proj, 0x40);
    osWritebackDCache(view, 0x40);
    cull_step_delayed((s32) gLevelData, dl, v, (s32) (gdl - dl) * sizeof(Gfx));
}

/* Handles a Yoshi (menu) selection *sel for the current game mode */
void func_8024BDA4(u16 *sel) {
    switch (gGameMode) {
        case 4:
        case 0x100:
            switch (*sel) {
                case 1:
                    if (gCurVehicle == 6 || gCurVehicle == 11 || gCurVehicle == 17 || gCurVehicle == 18) {
                        gYoshiGameItems[7].unk0 &= ~1;
                        gYoshiGameItems[7].unk18 = 8;
                    } else {
                        gYoshiGameItems[7].unk0 |= 1;
                        gYoshiGameItems[7].unk18 = 7;
                    }
                    yoshi_demand(0x8001);
                    gNextGameMode = 4;
                    break;
                case 2:
                    stats_finish_level(D_80364A58);
                    break;
                case 7:
                    cont_show_method_option();
                    break;
                case 3:
                    if (gLevelInfo[levelno].unk0 & 0x81) {
                        func_80275270(0x2000, 0.5f);
                    } else {
                        func_80275270(0x20000000, 0.5f);
                    }
                    break;
                case 8:
                    gNextGameMode = MODE_YOSHI_PAGES;
                    yoshi_demand(0x805E);
                    gCamSnap = 1;
                    break;
                case 4:
                    func_80275270(0x4000, 0.5f);
                    break;
                case 9:
                    if (music_get_master_volume() == 1.0) {
                        gYoshiWindows[13].unk18 = 40;
                    } else {
                        gYoshiWindows[13].unk18 = 39;
                    }
                    yoshi_demand(0x800D);
                    break;
                case 0x1A7:
                case 0x1A8:
                    if (((*sel - 0x1A7) << gCurVehicle) ^ ((1 << gCurVehicle) & 0x10205)) {
                        debug_printf("selected controller mode yes\n");
                        players[playerNumber].unkF0 |= 1 << gCurVehicle;
                        rec_record(0x80, 0, 0);
                    } else {
                        debug_printf("selected controller mode no\n");
                        players[playerNumber].unkF0 &= ~(1 << gCurVehicle);
                        rec_record(0x40, 0, 0);
                    }
                    break;
                case 17:
                    gYoshiWindows[6].unk8 |= 0x80;
                    break;
                case 0xFFFF:
                    switch (currentYoshiWindow) {
                        case 1:
                        case 6:
                            debug_printf("TESTING PAUSE2 %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                            yoshi_demand(0x8000);
                            if (gHaveCarrier || gHaveShuttle) {
                                gNextGameMode = MODE_MAP_VIEW;
                                cull_restart(10);
                            }
                            break;
                        case 13:
                        case 0x58:
                            debug_printf("TESTING PAUSE3 %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                            yoshi_demand(0x8001);
                            break;
                    }
                    break;
                case 40:
                    music_set_master_volume(1.0f);
                    break;
                case 39:
                    music_set_master_volume(0.7f);
                    break;
            }
            break;
        case 0x1000000000:
            gNextGameMode = 4;
            break;
        case 0x40:
        case 0x400:
            if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
                if (*sel == 0x18 || *sel == 0x22 || *sel == 0xFFFF) {
                    gNextGameMode = MODE_RESULTS;
                } else {
                    gNextGameMode = MODE_LEVEL_START;
                }
            } else {
                if (*sel == 0x18 || *sel == 0x22 || *sel == 0xFFFF) {
                    gNextGameMode = MODE_WORLD;
                } else {
                    gNextGameMode = MODE_LEVEL_START;
                }
            }
            break;
        case 0x2000000000000000:
            if (*sel == 0xFFFF) {
                debug_printf("TESTING PAUSE %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                yoshi_demand(0x8001);
            }
            gNextGameMode = 4;
            break;
        default:
            debug_printf("Yoshi selection in illegal game mode\n");
            break;
    }
    *sel = 0;
}

/* An empty hook after the frame's list (probably a compiled-out debug
 * overlay): zeroes *arg2 (gFrameDlLen; hd_close_display_list sets it again). */
Gfx *func_8024C404(Gfx *arg0, DynamicBuf *arg1, s32 *arg2) {
    *arg2 = 0;
    return arg0;
}

/* Builds the level frame's top-level display list in dyn: segments, depth
 * clear, the matrices (ortho, cam_update's views, guPerspective with
 * gCamFovy), the sky, the level's display lists, vehicles, objects, effects
 * and the HUD; *len = its length (0x8024C414). */
Gfx *hd_build_frame_dl(DynamicBuf *dyn, s32 *len) {
    Gfx *gdl = dyn->dl;
    char money[16];
    char bonus[16];
    s32 height;

    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(gStaticSegment));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gSPClearGeometryMode(gdl++, G_ZBUFFER);
    gDPSetDepthImage(gdl++, gDepthBufferPhys);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gDepthBufferPhys);
    gDPSetFillColor(gdl++, 0xFFFCFFFC);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    guTranslate(&dyn->identity, 0.0f, 0.0f, 0.0f);
    guOrtho(&dyn->ortho, 0.0f, 319.0f, 239.0f, 0.0f, -20000.0f, 20000.0f, 1.0f);
    guOrtho(&dyn->orthoHi, 0.0f, 1279.0f, 959.0f, 0.0f, -20000.0f, 20000.0f, 1.0f);
    cam_update(&dyn->view, &dyn->lookAt, &dyn->cullView);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameBufferPhys[gBufferIndex]);
    gdl = backdrop_draw(gdl, (u32) dyn, levelno, gCamYaw, gCamPitch, &height);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    switch (levelno) {
        case 13:
        case 14:
        case 16:
        case 52:
            gDPSetFillColor(gdl++, 0xD55FD55F);
            break;
        case 15:
            gDPSetFillColor(gdl++, 0x10511051);
            break;
        default:
            gDPSetFillColor(gdl++, 0x00010001);
            break;
    }
    gDPPipeSync(gdl++);
    gDPFillRectangle(gdl++, 0, (height <= 0) ? 0 : height - 1, 319, 239);
    gDPPipeSync(gdl++);
    gSPLookAtX(gdl++, &D_02000000[0xF0]);
    gSPLookAtY(gdl++, (u8 *) &D_02000000[0xF0] + 0x10);
    gSPMatrix(gdl++, &D_02000000[2], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_02000000[5], G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, gPerspNorm);
    switch (gGameMode) {
        case 0x40:
        case 0x400:
            guPerspective(&dyn->persp, &gPerspNorm, gCamFovy, 1.3333334f, 10.0f, 20000.0f, 1.0f);
            break;
        case 1:
        case 0x800:
        case 0x1000:
            guPerspective(&dyn->persp, &gPerspNorm, gCamFovy, 1.3333334f, 10.0f, 10000.0f, 1.0f);
            break;
        default:
            guPerspective(&dyn->persp, &gPerspNorm, gCamFovy, 1.3333334f, 10.0f, 10000.0f, 1.0f);
            break;
    }
    gSPSetOtherMode(gdl++, G_SETOTHERMODE_H, 6, 2, 0);
    gSPClipRatio(gdl++, FRUSTRATIO_3);
    water_draw(&gdl, gBufferIndex, 0);
    gSPSegment(gdl++, 8, PHYS(gLevelSegment8));
    gSPClearGeometryMode(gdl++, -1);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(gLevelDl0[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E0);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(gLevelDl1[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E4);
    if (!gCarrierHit && !gLevelDone) {
        gSPClearGeometryMode(gdl++, -1);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
        gSPDisplayList(gdl++, SEG2(0x21410));
        gDPPipeSync(gdl++);
    }
    switch (gBufferIndex) {
        case 0:
            gSPSegment(gdl++, 10, osVirtualToPhysical(D_803F7820));
            break;
        case 1:
            gSPSegment(gdl++, 10, osVirtualToPhysical(D_803F7824));
            break;
    }
    gDPPipeSync(gdl++);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
    gSPDisplayList(gdl++, SEG2(0x0A4E0));
    if (!(gGameMode & 0x440)) {
        trail_draw(&gdl, (struct TrailBuf *) dyn);
    }
    func_802502EC();
    veh_draw_shadows(&gdl, dyn, 0);
    obj_draw_ground_shadows(&gdl, (struct VtxBuf *) dyn);
    if (!gPaused && gLevelKind != 0x40 && !(gGameMode & 0x440) && !D_80364A84) {
        mb_draw(&gdl, (s32) dyn, gBufferIndex);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPDisplayList(gdl++, SEG2(0x0A580));
    gDPPipeSync(gdl++);
    gdl = commpt_draw(gdl, gBufferIndex, (Gfx *) dyn->unkA3A0, &dyn->unk2C0[0x3C]);
    veh_draw_shadows(&gdl, dyn, 1);
    if (gHaveChopper) {
        veh_draw_chopper_shadow(&gdl, dyn);
    }
    rdu_draw(&gdl, (s32) dyn);
    fx_draw_power_pill(&gdl);
    tnt_draw(&gdl, (struct Dyn48D00 *) dyn);
    block_draw(&gdl, (struct Dyn4B5E0 *) dyn);
    shell_draw_all(&gdl, (struct Dyn *) dyn);
    crate_draw(&gdl, (s32) dyn);
    switch (gBufferIndex) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(gFxDlA0));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(gFxDlA1));
            break;
    }
    ptcl_draw(&gdl, gBufferIndex);
    if (!(gGameMode & 2) || !D_802E8BF0) {
        veh_draw_layer(&gdl, 0);
    }
    academy_draw_marker(&gdl);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(gLevelDl2[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E8);
    if (!(gGameMode & 2) || !D_802E8BF0) {
        veh_draw_layer(&gdl, 1);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(gLevelDl3[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6EC);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPDisplayList(gdl++, SEG2(0x0A918));
    gDPPipeSync(gdl++);
    if (!gCarrierHit && !gLevelDone) {
        gSPClearGeometryMode(gdl++, -1);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
        gSPDisplayList(gdl++, SEG2(0x21478));
        gDPPipeSync(gdl++);
    }
    if (!(gGameMode & 2) || !D_802E8BF0) {
        veh_draw_layer(&gdl, 2);
    }
    ghost_draw(&gdl, (Mtx *) dyn);
    if (gGameMode & 0x2000001000003905) {
        fx_draw_drift_quads(&gdl, gBufferIndex);
    }
    water_draw(&gdl, gBufferIndex, 1);
    veh_draw_shadows(&gdl, dyn, 2);
    if (gHaveChopper) {
        people_draw(&gdl, (struct DynBuf *) dyn);
    }
    switch (gBufferIndex) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(gFxDlB0));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(gFxDlB1));
            break;
    }
    if (gHaveCarrier) {
        fx_draw_carrier_blast(&gdl, gBufferIndex);
    }
    PORT_ITAG(gdl, PORT_ITAG_HUD, 0);
    if (gHudDialLoaded && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0); /* widescreen HUD sides: see port.h */
        hud_draw_boost_dial(&gdl, (s32) dyn, gBufferIndex, gCurVehicle);
    }
    if (gGameMode & 0x200000000400220C) {
        hud_panel_draw(&gdl, (s32) dyn, gBufferIndex);
    }
    if (gGameMode == MODE_MAP_VIEW && gHaveCarrier) {
        PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0); /* follows the player on screen */
        hud_draw_player_locator(&gdl, dyn, gBufferIndex, gPlayerX, gPlayerY, gPlayerZ);
    }
    text_begin_frame();
    if (gHudMissileIconLoaded && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
        hud_draw_missile_icon(&gdl, (Gfx * N64P *) dyn, gBufferIndex, gCurVehicle);
    }
    if (gHudRamIconLoaded && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
        PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
        hud_draw_ram_icon(&gdl, (Gfx * N64P *) dyn, gBufferIndex, gCurVehicle);
    }
    if (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) {
        hud_draw_veh_overlay(&gdl, gCurVehicle);
    }
    if ((gLevelKind & 1) && (gGameMode & 0x0400030C)) {
        util_itoa(D_803649F4, (u8 *) &money[1]);
        money[0] = '$';
        text_print((Gfx * N64P *) dyn, (u8 *) money, 0, 1, 0, 0x118, 0x12, 0x14, 0x14, 0, 0xFF, 0xFF, 0xFF, gHudAlpha);
        if (D_80364A44 && !gPaused) {
            util_itoa(D_80364A44, (u8 *) &bonus[1]);
            bonus[0] = '$';
            text_print((Gfx * N64P *) dyn, (u8 *) bonus, 0, 1, 0, D_80364A4A, D_80364A4C, 0x23, 0x23, D_80364A4E, 0xFF, 0xFF, 0xFF,
                          D_80364A48);
        }
    }
    if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW | MODE_YOSHI_PAGES)) && (!(gLevelKind & 0x81) || gHaveCarrier || gHaveShuttle)) {
        PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0); /* follows the target in the 3D view */
        hud_draw_target_marker((struct SpriteVtxBuf *) dyn, &gdl, (gGameMode & MODE_MAP_VIEW) || currentYoshiWindow == 0x4D || currentYoshiWindow == 0x49);
    }
    if (gNextGameMode == 0 && !areWeFading()) {
        if (!(gGameMode & 0x200000100400230C) && PORT_GVI("hd_build_frame_dl", D_803156C4) % 50 * 60 / 60 >= 21 && yoshiState == 1 &&
            (!(gGameMode & 2) || (D_802E8BEC && (gLogoState == 3 || D_802E8BEC == 1))) &&
            (gGameMode != MODE_GLOBE_SEQ || D_803A6B04) &&
            (!(gGameMode & 0x1801) || players[playerNumber].unk91) && levelno != 0x2F) {
            text_print((Gfx * N64P *) dyn, (u8 *) ("PRESS START"), 0, 1, 0, 0x5C, 0xC4, 0x1A, 0x1A, 1, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (PORT_GVI("hd_build_frame_dl", D_803156C4) % 40 * 60 / 60 >= 16) {
            if (gPaused) {
                if (gGameMode == MODE_YOSHI_PAGES && yoshiState == 2) {
                    text_print((Gfx * N64P *) dyn, (u8 *) ("USE Z/R TO TURN PAGES"), D_8030491C, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                } else if (gGameMode == MODE_MAP_VIEW && currentYoshiWindow == 0 && yoshiState == 2 && gHaveCarrier &&
                           !(gContButtons & 0x2010)) {
                    text_print((Gfx * N64P *) dyn, (u8 *) ("USE Z/R TO MOVE MAP"), D_80304938, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                }
            } else if (gGameMode == MODE_MAP_VIEW) {
                if (gHaveShuttle) {
                    text_print((Gfx * N64P *) dyn, (u8 *) ("SHUTTLE VIEW"), D_80304904, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                } else {
                    text_print((Gfx * N64P *) dyn, (u8 *) ("MISSILE VIEW"), D_80304910, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                }
            }
        }
    }
    if (((gGameMode & 0x440) || levelno == 0x26) && yoshiState == 1 && !areWeFading()) {
        if ((gGameMode & 0x440) && gLevelKind != 1) {
            PORT_WTAG(gdl, PORT_WTAG_RIGHT, 0, 0);
            spr_draw_blinking(&gdl, (s32) dyn, D_80365580, 0x108, 0x12);
        } else {
            PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
            spr_draw_blinking(&gdl, (s32) dyn, D_80365580, 0x18, 0x12);
        }
    }
    if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) && gCamFixedEye) {
        PORT_WTAG(gdl, PORT_WTAG_RIGHT, 0, 0);
        spr_draw_blinking(&gdl, (s32) dyn, D_80364A86, 0x108, 0xBE);
    }
    if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW)) && gLevelKind == 1 && !gPaused) {
        stats_award_bonuses();
    }
    if (gHaveCarrier && gGameMode == 4) {
        PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
        hud_draw_target_arrow(&gdl, (Mtx *) dyn, gPlayerX, gPlayerY, gPlayerZ, gCarrierX, gCarrierY, gCarrierZ);
    }
    if ((gLevelInfo[levelno].unk0 & 0x81) && gGameMode == 4) {
        if (levelno != 0x32 ||
            ((players[playerNumber].unk18[0x32] > 0 && players[playerNumber].unk18[0x32] < 6) ? 1 : 0)) {
            PORT_WTAG(gdl, PORT_WTAG_LEFT, 0, 0);
            hud_draw_radar(&gdl, (Mtx *) dyn, gBufferIndex, gPlayerX, gPlayerZ, gCarrierX, gCarrierZ);
        }
    }
    PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0); /* full-screen fades, the text's own sides */
    if (gHaveCarrier || gHaveShuttle) {
        switch (gGameMode) {
            case 4:
            case 0x100:
            case 0x200:
            case 0x100000000000:
                hud_level_complete(&gdl, (s32) dyn, gBufferIndex);
                hud_mission_failed(&gdl, (s32) dyn, gBufferIndex);
            case 0x40:
            case 0x400:
                if ((gCarrierHit || gLevelDone || cmo_hit_request || gLevelDoneRequest) && (gGameMode & 0x144)) {
                    cull_restart(0x32);
                    if (gGameMode == 0x40) {
                        gNextGameMode = 0x400;
                    } else {
                        gNextGameMode = 0x200;
                    }
                }
                break;
        }
    }
    PORT_WTAG(gdl, PORT_WTAG_TEXT_SIDES, 0, 0);
    text_flush_screen(&gdl, (Mtx *) dyn);
    PORT_WTAG(gdl, PORT_WTAG_NONE, 0, 0);
    *len = gdl - dyn->dl;
    return gdl;
}

/* Draws the shadow quads of every vehicle on the given layer (0x8024E4F4). */
void veh_draw_shadows(Gfx **gfx, DynamicBuf *dyn, u8 layer) {
    Gfx *gdl = *gfx;
    s32 i;
    s16 unused;
    s16 x;
    s16 y;
    s16 z;
    s16 w;
    s16 d;
    u8 all;
    f32 mf[4][4];
    f32 tmp[4][4];
    s16 th;
    s16 tw;
    u8 spin;

    if (layer == 0) {
        D_8036506C = 0;
    }
    gDPPipeSync(gdl++);
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    gDPSetRenderMode(gdl++, 0x504340, 0);
    all = !D_803649E8 && (D_803649EC || (gGameMode & 0x1801));
    i = 0;
    while (&gVehicleShadows[i] != gVehicleShadowsEnd) {
        if ((gVehicleShadows[i].type != 0 || all) && gVehicleShadows[i].type != 0xFE &&
            (gVehicleShadows[i].type != 0xFF || !gCarrierExplodeDone) &&
            (gVehicleShadows[i].type == 0xFD || !D_80364A84 || !gHaveShuttle) && gVehicleShadows[i].layer == layer) {
            x = gVehicleShadows[i].unk1004 >> 5;
            y = gVehicleShadows[i].unk1008 >> 5;
            z = gVehicleShadows[i].unk100C >> 5;
            w = gVehicleShadows[i].halfWidth;
            d = gVehicleShadows[i].halfDepth;
            spin = gVehicleShadows[i].type == gCurVehicle && gSurfaceMax == 0x65;
            if (spin) {
                guRotateF(tmp, (f32) D_80364440 * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            }
            guRotateF(mf, (f32) -gVehicleShadows[i].unk101E * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            if (spin) {
                guMtxCatF(tmp, mf, mf);
            }
            guRotateF(tmp, (f32) gVehicleShadows[i].unk101C * 360.0 / 4096.0, 1.0f, 0.0f, 0.0f);
            guMtxCatF(mf, tmp, tmp);
            guRotateF(mf, (f32) gVehicleShadows[i].unk1020 * 360.0 / 4096.0, 0.0f, 0.0f, 1.0f);
            guMtxCatF(tmp, mf, mf);
            guRotateF(tmp, (f32) gVehicleShadows[i].unk101E * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            guMtxCatF(mf, tmp, mf);
            guTranslateF(tmp, x, y, z);
            guMtxCatF(mf, tmp, mf);
            guMtxF2L(mf, &dyn->unk2C0[i]);
            if (spin) {
                gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_802FA940), G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                                    G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                th = 32, tw = 32;
            } else {
                gDPLoadTextureBlock(gdl++, PHYS((u32) gVehicleShadows + i * sizeof(VehShadow)), G_IM_FMT_IA, G_IM_SIZ_8b,
                                    64, 64, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                    G_TX_NOLOD);
                th = 64, tw = 64;
            }
            dyn->unk15C0[D_8036506C].v.ob[0] = -w;
            dyn->unk15C0[D_8036506C].v.ob[1] = 0;
            dyn->unk15C0[D_8036506C].v.ob[2] = d;
            dyn->unk15C0[D_8036506C].v.flag = 0;
            dyn->unk15C0[D_8036506C].v.tc[0] = (th - 1) << 5;
            dyn->unk15C0[D_8036506C].v.tc[1] = (tw - 1) << 5;
            dyn->unk15C0[D_8036506C].v.cn[0] = 10;
            dyn->unk15C0[D_8036506C].v.cn[1] = 10;
            dyn->unk15C0[D_8036506C].v.cn[2] = 10;
            dyn->unk15C0[D_8036506C].v.cn[3] = 0x8C;
            D_8036506C++;
            dyn->unk15C0[D_8036506C].v.ob[0] = w;
            dyn->unk15C0[D_8036506C].v.ob[1] = 0;
            dyn->unk15C0[D_8036506C].v.ob[2] = d;
            dyn->unk15C0[D_8036506C].v.flag = 0;
            dyn->unk15C0[D_8036506C].v.tc[0] = 0;
            dyn->unk15C0[D_8036506C].v.tc[1] = (tw - 1) << 5;
            dyn->unk15C0[D_8036506C].v.cn[0] = 10;
            dyn->unk15C0[D_8036506C].v.cn[1] = 10;
            dyn->unk15C0[D_8036506C].v.cn[2] = 10;
            dyn->unk15C0[D_8036506C].v.cn[3] = 0x8C;
            D_8036506C++;
            dyn->unk15C0[D_8036506C].v.ob[0] = w;
            dyn->unk15C0[D_8036506C].v.ob[1] = 0;
            dyn->unk15C0[D_8036506C].v.ob[2] = -d;
            dyn->unk15C0[D_8036506C].v.flag = 0;
            dyn->unk15C0[D_8036506C].v.tc[0] = 0;
            dyn->unk15C0[D_8036506C].v.tc[1] = 0;
            dyn->unk15C0[D_8036506C].v.cn[0] = 10;
            dyn->unk15C0[D_8036506C].v.cn[1] = 10;
            dyn->unk15C0[D_8036506C].v.cn[2] = 10;
            dyn->unk15C0[D_8036506C].v.cn[3] = 0x8C;
            D_8036506C++;
            dyn->unk15C0[D_8036506C].v.ob[0] = -w;
            dyn->unk15C0[D_8036506C].v.ob[1] = 0;
            dyn->unk15C0[D_8036506C].v.ob[2] = -d;
            dyn->unk15C0[D_8036506C].v.flag = 0;
            dyn->unk15C0[D_8036506C].v.tc[0] = (th - 1) << 5;
            dyn->unk15C0[D_8036506C].v.tc[1] = 0;
            dyn->unk15C0[D_8036506C].v.cn[0] = 10;
            dyn->unk15C0[D_8036506C].v.cn[1] = 10;
            dyn->unk15C0[D_8036506C].v.cn[2] = 10;
            dyn->unk15C0[D_8036506C].v.cn[3] = 0x8C;
            D_8036506C++;
            PORT_ITAG(gdl, PORT_ITAG_SHADOW, i);
            if (gCarrierHit && !(gLevelKind & 0x81) && gVehicleShadows[i].type == gCurVehicle) {
                gSPMatrix(gdl++, &D_02000000[0x55], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            }
            gSPMatrix(gdl++, &D_02000000[i + 11], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gSPVertex(gdl++, (Vtx *) D_02000000 + D_8036506C + 0x158, 4, 0);
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
            gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            if (gCarrierHit && !(gLevelKind & 0x81) && gVehicleShadows[i].type == gCurVehicle) {
                gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            }
            gDPPipeSync(gdl++);
        }
        i++;
    }
    PORT_ITAG(gdl, PORT_ITAG_NONE, 0);
    *gfx = gdl;
}

/* Draws the shadow quad under vehicle 0xFE, the BCT chopper (0x8024F520). */
void veh_draw_chopper_shadow(Gfx **gfx, DynamicBuf *dyn) {
    Gfx *gdl = *gfx;
    s32 i;
    u8 found;
    s16 x;
    s16 y;
    s16 z;
    s16 w;
    s16 d;

    gDPPipeSync(gdl++);
    gDPSetTexturePersp(gdl++, G_TP_PERSP);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    gDPSetRenderMode(gdl++, 0x504340, 0);
    found = 0;
    i = 0;
    while (!found) {
        if (gVehicleShadows[i].type == 0xFE) {
            found = 1;
        } else {
            i++;
        }
    }
    gDPLoadTextureBlock(gdl++, PHYS((u32) gVehicleShadows + i * sizeof(VehShadow)), G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    x = gVehicleShadows[i].unk1004 >> 5;
    y = gVehicleShadows[i].unk1008 >> 5;
    z = gVehicleShadows[i].unk100C >> 5;
    w = gVehicleShadows[i].halfWidth;
    d = gVehicleShadows[i].halfDepth;
    dyn->unk18C0[0].v.ob[0] = -w;
    dyn->unk18C0[0].v.ob[1] = 0;
    dyn->unk18C0[0].v.ob[2] = d;
    dyn->unk18C0[0].v.flag = 0;
    dyn->unk18C0[0].v.tc[0] = 0x7E0;
    dyn->unk18C0[0].v.tc[1] = 0x7E0;
    dyn->unk18C0[0].v.cn[0] = 10;
    dyn->unk18C0[0].v.cn[1] = 10;
    dyn->unk18C0[0].v.cn[2] = 10;
    dyn->unk18C0[0].v.cn[3] = 0x8C;
    dyn->unk18C0[1].v.ob[0] = w;
    dyn->unk18C0[1].v.ob[1] = 0;
    dyn->unk18C0[1].v.ob[2] = d;
    dyn->unk18C0[1].v.flag = 0;
    dyn->unk18C0[1].v.tc[0] = 0;
    dyn->unk18C0[1].v.tc[1] = 0x7E0;
    dyn->unk18C0[1].v.cn[0] = 10;
    dyn->unk18C0[1].v.cn[1] = 10;
    dyn->unk18C0[1].v.cn[2] = 10;
    dyn->unk18C0[1].v.cn[3] = 0x8C;
    dyn->unk18C0[2].v.ob[0] = w;
    dyn->unk18C0[2].v.ob[1] = 0;
    dyn->unk18C0[2].v.ob[2] = -d;
    dyn->unk18C0[2].v.flag = 0;
    dyn->unk18C0[2].v.tc[0] = 0;
    dyn->unk18C0[2].v.tc[1] = 0;
    dyn->unk18C0[2].v.cn[0] = 10;
    dyn->unk18C0[2].v.cn[1] = 10;
    dyn->unk18C0[2].v.cn[2] = 10;
    dyn->unk18C0[2].v.cn[3] = 0x8C;
    dyn->unk18C0[3].v.ob[0] = -w;
    dyn->unk18C0[3].v.ob[1] = 0;
    dyn->unk18C0[3].v.ob[2] = -d;
    dyn->unk18C0[3].v.flag = 0;
    dyn->unk18C0[3].v.tc[0] = 0x7E0;
    dyn->unk18C0[3].v.tc[1] = 0;
    dyn->unk18C0[3].v.cn[0] = 10;
    dyn->unk18C0[3].v.cn[1] = 10;
    dyn->unk18C0[3].v.cn[2] = 10;
    dyn->unk18C0[3].v.cn[3] = 0x8C;
    guTranslate(&dyn->unk240, x, y, z);
    guRotate(&dyn->unk280, (f32) D_803EF326 * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
    PORT_ITAG(gdl, PORT_ITAG_SHADOWFE, 0);
    gSPMatrix(gdl++, &D_02000000[9], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(gdl++, &D_02000000[10], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPVertex(gdl++, &D_02000000[0x63], 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
    PORT_ITAG(gdl, PORT_ITAG_NONE, 0);
    *gfx = gdl;
}

/* Emits the display lists of every vehicle record (gVehicles) for one of the
 * frame's three layers (0x8024FC2C). */
void veh_draw_layer(Gfx **gfx, u8 lod) {
    Gfx *gdl = *gfx;
    s32 i;
    u8 unused;
    u8 alt;
    u8 all;

    all = !D_803649E8 && (D_803649EC || (gGameMode & 0x1801));
    i = 0;
    while (&gVehicles[i] != gVehiclesEnd) {
        if ((gVehicles[i].type != 0 || all) && (gVehicles[i].type != 0xFF || !gCarrierExplodeDone) &&
            (gVehicles[i].type == 0xFD || !D_80364A84 || !gHaveShuttle)) {
            PORT_ITAG(gdl, PORT_ITAG_OBJ, i | (lod << 8));
            gSPSegment(gdl++, 6, osVirtualToPhysical(gVehicles[i].unk0));
            if ((gGameMode & 0x1801) && (gVehicles[i].type == 0xFE || gVehicles[i].type == 0)) {
                alt = gBufferIndex;
            } else {
                alt = D_803156F5;
            }
            if (alt) {
                gSPSegment(gdl++, 7, osVirtualToPhysical(gVehicles[i].unk4));
            } else {
                gSPSegment(gdl++, 7, osVirtualToPhysical(gVehicles[i].unk8));
            }
            gDPPipeSync(gdl++);
            gDPSetEnvColor(gdl++, 0, 0, 0, gVehicles[i].alpha);
            gSPClearGeometryMode(gdl++, -1);
            if (gCarrierHit && !(gLevelKind & 0x81) && gVehicles[i].type == gCurVehicle) {
                gSPMatrix(gdl++, &D_02000000[0x54], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            }
            if (alt) {
                switch (lod) {
                    case 0:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unkC));
                        break;
                    case 1:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unk10));
                        break;
                    case 2:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unk14));
                        break;
                }
            } else {
                switch (lod) {
                    case 0:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unk30));
                        break;
                    case 1:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unk34));
                        break;
                    case 2:
                        gSPDisplayList(gdl++, osVirtualToPhysical(gVehicles[i].unk38));
                        break;
                }
            }
            PORT_ITAG(gdl, PORT_ITAG_NONE, 0);
            gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        }
        i++;
    }
    *gfx = gdl;
}

/* Spins and shrinks the selected vehicle's matrices (vehicle eject effect?) */
void func_802502EC(void) {
    f32 mf[4][4];
    f32 tmp[4][4];
    s32 i;
    s16 x;
    s16 y;
    s16 z;

    if (gCarrierHit != 0 && !(gLevelKind & 0x81)) {
        guTranslateF(mf, -(f32) gPlayerX / 32.0, -(f32) gPlayerY / 32.0, -(f32) gPlayerZ / 32.0);
        guScaleF(tmp, D_80364ABC, D_80364ABC, D_80364ABC);
        guMtxCatF(mf, tmp, mf);
        guRotateF(tmp, D_80364AB4, 0.0f, 1.0f, 0.0f);
        guMtxCatF(mf, tmp, mf);
        guTranslateF(tmp, (f32) gPlayerX / 32.0, (f32) gPlayerY / 32.0, (f32) gPlayerZ / 32.0);
        guMtxCatF(mf, tmp, mf);
        guMtxF2L(mf, &gDynamicBufs[gBufferIndex].unk1500);
        i = 0;
        while (gVehicleShadows[i].type != gCurVehicle) {
            i++;
        }
        x = gVehicleShadows[i].unk1004 >> 5;
        y = gVehicleShadows[i].unk1008 >> 5;
        z = gVehicleShadows[i].unk100C >> 5;
        guTranslateF(mf, -x, -y, -z);
        guScaleF(tmp, D_80364ABC, D_80364ABC, D_80364ABC);
        guMtxCatF(mf, tmp, mf);
        guRotateF(tmp, D_80364AB4, 0.0f, 1.0f, 0.0f);
        guMtxCatF(mf, tmp, mf);
        guTranslateF(tmp, x, y, z);
        guMtxCatF(mf, tmp, mf);
        guMtxF2L(mf, &gDynamicBufs[gBufferIndex].unk1540);
        if (D_80364AC0 == 0) {
            D_80364ABC = D_80364ABC - 0.04;
            if (D_80364ABC < 0.0) {
                D_80364ABC = 0.0f;
            }
        }
        if (D_80364ABC == 0.0 && D_80364AC0 == 0) {
            fx_spawn_at(gPlayerX, gPlayerY, gPlayerZ, 0x13, 400000);
            D_80364AC0 = 1;
        }
        D_80364AB4 += D_80364AB8;
        D_80364AB8 = D_80364AB8 + 3.0;
    }
}

/* camera */
typedef struct {
    s16 unk0; /* min x, y, z */
    s16 unk2;
    s16 unk4;
    u8 pad6[0xC];
    s16 unk12; /* max x, y, z */
    s16 unk14;
    s16 unk16;
} Box;
typedef struct {
    s16 unk0; /* position x, y, z */
    s16 unk2;
    s16 unk4;
    u8 pad6[0x82];
} Node88; /* 0x88 bytes */
#ifndef NON_MATCHING
extern Node88 *gRdus;
#endif
extern u8 gCamAtSettled; /* target easing settled */
extern s32 gCamAtHeight; /* target height offset */
extern u8 D_8030F668;
extern u8 D_8030F669;
extern u8 D_8030F66A;
extern s32 gCamEyeGoalX; /* wanted eye x, y, z (<< 11) */
extern s32 gCamEyeGoalY;
extern s32 gCamEyeGoalZ;
extern s16 gCamTiltMin;
extern f32 gCamAtGoalX; /* wanted target x, y, z */
extern f32 gCamAtGoalY;
extern f32 gCamAtGoalZ;
extern f32 gCamAtX; /* target x, y, z */
extern f32 gCamAtY;
extern f32 gCamAtZ;
extern f32 gCamAtEaseX; /* target easing deltas */
extern f32 gCamAtEaseY;
extern f32 gCamAtEaseZ;
extern u16 gCamEasedHeading; /* eased heading */
extern s16 gCamType; /* camera type */
extern s16 gCamPrevType; /* previous camera type */
#ifndef NON_MATCHING
extern Box *gMarkedTargetPts;
#endif
f32 cam_eye_scale(f32, f32, f32, f32, f32, f32);
void cam_polar_offset(s32, f32, s32 *, s32 *);
void cam_turn_heading(void);

#define SINE(a) hd_sinf((f32) (s16) (a) / 16384.0f * 1.5708)
#define COSINE(a) hd_cosf((f32) (s16) (a) / 16384.0f * 1.5708)

/* The camera: chooses the wanted eye and target for the mode and the
 * vehicle (gCamType), eases toward them (or snaps, gCamSnap), adds the screen
 * shake, and builds the drawing view (mtx, lookAt: guLookAtReflect) and the
 * culling view (view: the eye pulled in by cam_eye_scale) (0x802507C8). */
void cam_update(Mtx *mtx, LookAt *lookAt, Mtx *view) {
    f32 rateX;
    f32 rateY;
    f32 rateZ;
    f32 dampX;
    f32 dampY;
    f32 dampZ;
    s32 pad;
    s16 ang;
    s16 sn;
    s16 cs;
#ifdef NON_MATCHING
    s32 offX = 0; /* camera modes without a case leave it indeterminate in the original */
#else
    s32 offX;
#endif
#ifdef NON_MATCHING
    s32 offZ = 0; /* camera modes without a case leave it indeterminate in the original */
#else
    s32 offZ;
#endif
    f32 diff;
#ifdef NON_MATCHING
    s32 radius = 0; /* camera ids without a case leave it indeterminate in the original */
#else
    s32 radius;
#endif
#ifdef NON_MATCHING
    f32 tiltRate = 0.0f; /* camera ids without a case leave it indeterminate in the original */
#else
    f32 tiltRate;
#endif
#ifdef NON_MATCHING
    f32 turnRate = 0.0f; /* camera ids without a case leave it indeterminate in the original */
#else
    f32 turnRate;
#endif
    s32 shakeX;
    s32 shakeY;
    s32 shakeZ;
    f32 eyeX;
    f32 eyeY;
    f32 eyeZ;
    f32 posX;
    f32 posY;
    f32 posZ;
    f32 scale;
    f32 dist;
    s16 tilt;
    u8 inShuttle;
    s32 px;
    s32 pz;
    u8 changed;
    f32 adx;
    f32 adz;
    f32 atX;
    f32 atZ;
    s16 angle;
    s32 tx;
    s32 ty;
    s32 tz;
    s8 quadrant;
    s16 pitch2;
    s16 pitch1;

    changed = 0;
    cam_turn_heading();
    if (D_8036B965 && (gGameMode & 0x600)) {
        D_8036B965 = 0;
    }
    inShuttle = gHaveShuttle && ((gGameMode & 0x940) || D_8036B965);
    if (gGameMode == 8 || (currentYoshiWindow == 0x4B && yoshiState == 2 && (u16) gYoshiWindows[currentYoshiWindow].unk18 == 0x14D)) {
        if (!D_8030F668) {
            changed = 1;
        }
        D_8030F668 = 1;
    } else {
        if (D_8030F668) {
            changed = 1;
        }
        D_8030F668 = 0;
    }
    if (currentYoshiWindow == 0x49 && yoshiState == 2 && (u16) gYoshiWindows[currentYoshiWindow].unk18 == 0x130) {
        if (!D_8030F66A) {
            changed = 1;
        }
        D_8030F66A = 1;
    } else {
        if (D_8030F66A) {
            changed = 1;
        }
        D_8030F66A = 0;
    }
    if (currentYoshiWindow == 0x4D && yoshiState == 2 && (u16) gYoshiWindows[currentYoshiWindow].unk18 != 0x164) {
        if (!D_8030F669) {
            changed = 1;
        }
        D_8030F669 = 1;
    } else {
        if (D_8030F669) {
            changed = 1;
        }
        D_8030F669 = 0;
    }
    if (currentYoshiWindow == 0x58 && yoshiState != 8 && !gCamFixedEye) {
        if (!D_80364A85) {
            gCamSnap = changed = 1;
        }
        D_80364A85 = 1;
    } else {
        if (D_80364A85) {
            gCamSnap = changed = 1;
        }
        D_80364A85 = 0;
    }
    gCamSnap |= inShuttle || gGameMode == MODE_MAP_VIEW;
    gCamSnap |= gFramesInMode < 2;
    if (gCarrierHit && (gGameMode & 0x100000000600)) {
        rateX = 0.2f;
        rateY = 0.1f;
        rateZ = 0.2f;
    } else if ((yoshiState == 2 && (currentYoshiWindow == 0x4D || currentYoshiWindow == 0x4B || currentYoshiWindow == 0x49)) ||
               (gLevelDone && (gGameMode & 0x100000000600)) || gGameMode == 8) {
        rateX = 0.08f;
        rateY = 0.03f;
        rateZ = 0.08f;
    } else if (gGameMode & 0x1801) {
        rateX = 0.06f;
        rateY = 0.03f;
        rateZ = 0.03f;
    } else if ((gLevelKind == 0x10 || gLevelKind == 0x40) && gGameMode == 0x40) {
        rateX = 0.1f;
        rateY = 0.03f;
        rateZ = 0.1f;
    } else if (gCurVehicle == 9) {
        rateX = 0.5f;
        rateY = 0.06f;
        rateZ = 0.5f;
    } else {
        rateX = 0.5f;
        rateY = 0.03f;
        rateZ = 0.5f;
    }
    dampX = 0.95f;
    dampY = 0.97f;
    dampZ = 0.95f;
    if (gGameMode == 0x40 && levelno == 0x3B) {
        rateX = 1.0f;
        rateY = 1.0f;
        rateZ = 1.0f;
        dampX = 1.0f;
        dampY = 1.0f;
        dampZ = 1.0f;
    }
    switch (gGameMode) {
        case 0x100:
        case 0x200:
        case 0x400:
            if (gHaveShuttle) {
                gCamType = 0xFD;
            } else {
                gCamType = 0xFF;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (gHaveShuttle) {
                gCamType = 0xFD;
            } else {
                gCamType = 0xFE;
            }
            break;
        default:
            gCamType = gCurVehicle;
            break;
    }
    tilt = D_8036443C;
    if (!D_803649EC) {
        gCamAtHeight = cam_load_level_defaults();
    }
    if (((gGameMode & 2) && D_802E8BF0) || !D_803649EC) {
        radius = 0;
        tiltRate = 0.0f;
        turnRate = 0.0f;
    } else {
        switch (gCamType) {
            case 7:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xEB;
                gCamTiltMin = -0xA5;
                break;
            case 10:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xF5;
                gCamTiltMin = -0x87;
                break;
            case 11:
            case 17:
            case 18:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xEB;
                gCamTiltMin = -0xA5;
                break;
            case 8:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0x14F;
                gCamTiltMin = -0xA5;
                break;
            case 15:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0x131;
                gCamTiltMin = -0xA5;
                break;
            case 13:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0x145;
                gCamTiltMin = -0xA5;
                break;
            case 14:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0x13B;
                gCamTiltMin = -0xA5;
                break;
            case 9:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 1000;
                gCamTiltMax = 0x87;
                gCamTiltMin = -0x55;
                break;
            case 6:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 5000;
                gCamTiltMax = 0;
                gCamTiltMin = 0;
                break;
            case 5:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xCD;
                gCamTiltMin = -0x69;
                break;
            case 4:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xEB;
                gCamTiltMin = -0xA5;
                break;
            case 3:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xEB;
                gCamTiltMin = -0xA5;
                break;
            case 2:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 2000;
                gCamTiltMax = 0x91;
                gCamTiltMin = -0x55;
                break;
            case 16:
                radius = 2;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 600;
                gCamTiltMax = 0x91;
                gCamTiltMin = -0x55;
                break;
            case 1:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0xCD;
                gCamTiltMin = -0xA5;
                break;
            case 0:
                radius = 20;
                tiltRate = 0.25f;
                turnRate = 15.0f;
                gCamAtHeight = 0;
                gCamTiltMax = 0x55;
                gCamTiltMin = -0x19;
                break;
            case 0xFF:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                tilt = tilt / 2;
                gCamTiltMax = 0;
                gCamTiltMin = 0;
                break;
            case 0xFE:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                tilt = D_803EF328;
                gCamAtHeight = 0;
                D_8036443E = D_803EF32A;
                gCamTiltMax = 0;
                gCamTiltMin = 0;
                break;
            case 0xFD:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                gCamAtHeight = 0;
                tilt = tilt / 2;
                D_8036443E = D_8036443E / 2;
                gCamTiltMax = 0;
                gCamTiltMin = 0;
                break;
        }
    }
    if (gGameMode == 0x40) {
        radius = radius / 2;
    }
    if (tilt > gCamTiltMax) {
        tilt = gCamTiltMax;
    }
    if (tilt < gCamTiltMin) {
        tilt = gCamTiltMin;
    }
    if ((tilt > 0 && gHeldA && !gPaused && (gGameMode & (MODE_ATTRACT | MODE_LEVEL | MODE_MAP_VIEW))) ||
        (tilt < 0 && gHeldB && !gPaused && (gGameMode & (MODE_ATTRACT | MODE_LEVEL | MODE_MAP_VIEW)))) {
        if (gCamTilt < tilt) {
            gCamTilt += tiltRate;
        }
        if (gCamTilt > tilt) {
            gCamTilt -= tiltRate;
        }
    } else {
        if (gCamTilt < tilt && tilt <= 0) {
            gCamTilt += tiltRate * 10.0f;
            if (tilt < gCamTilt) {
                gCamTilt = tilt;
            }
        }
        if (gCamTilt > tilt && tilt >= 0) {
            gCamTilt = gCamTilt - tiltRate * 10.0f;
            if (gCamTilt < tilt) {
                gCamTilt = tilt;
            }
        }
    }
    diff = D_8036443E - gCamTurn;
    if (diff < 0.0) {
        diff = 0.0 - diff;
    }
    if (diff > 2048.0) {
        if (gCamTurn > D_8036443E) {
            gCamTurn = gCamTurn + turnRate;
            if (gCamTurn > 4095.0) {
                gCamTurn = gCamTurn - 4095.0;
                if (D_8036443E < gCamTurn) {
                    gCamTurn = D_8036443E;
                }
            }
        } else {
            gCamTurn = gCamTurn - turnRate;
            if (gCamTurn < 0.0) {
                gCamTurn = gCamTurn + 4095.0;
                if (gCamTurn < D_8036443E) {
                    gCamTurn = D_8036443E;
                }
            }
        }
    } else {
        if (gCamTurn > D_8036443E) {
            gCamTurn = gCamTurn - turnRate;
            if (gCamTurn < D_8036443E) {
                gCamTurn = D_8036443E;
            }
        }
        if (gCamTurn < D_8036443E) {
            gCamTurn = gCamTurn + turnRate;
            if (D_8036443E < gCamTurn) {
                gCamTurn = D_8036443E;
            }
        }
    }
    ang = (s16) gCamTurn % 1024;
    cs = hd_cosf((f32) ang / 1024.0f * 1.5708) * ((f32) radius * gCamTilt);
    sn = hd_sinf((f32) ang / 1024.0f * 1.5708) * ((f32) radius * gCamTilt);
    if (gCamTurn >= 0.0f && gCamTurn < 1024.0f) {
        offX = sn;
        offZ = cs;
    }
    if (gCamTurn >= 1024.0f && gCamTurn < 2048.0f) {
        offX = cs;
        offZ = -sn;
    }
    if (gCamTurn >= 2048.0f && gCamTurn < 3072.0f) {
        offX = -sn, offZ = -cs;
    }
    if (gCamTurn >= 3072.0f && gCamTurn < 4096.0f) {
        offX = -cs;
        offZ = sn;
    }
    switch (gGameMode) {
        case 0x100:
            if (gHaveShuttle) {
                if (gCamSnap) {
                    gCamEasedHeading = D_803FCD68 * 16;
                } else {
                    gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                }
                gCamEyeGoalX = (s32) (gShuttlePos - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
            } else {
                gCamEyeGoalX = (gCarrierX + 4000) << 11;
                gCamEyeGoalY = (gCarrierY + 25000) << 11;
                gCamEyeGoalZ = (gCarrierZ + D_80364A78 + 7000) << 11;
            }
            break;
        case 0x40:
            switch (gLevelKind) {
                case 1:
                case 0x80:
                    if (gHaveCarrier) {
                        gCamEyeGoalX = (gCarrierX + offX) << 11;
                        gCamEyeGoalY = (gCarrierY + 2000) << 11;
                        gCamEyeGoalZ = (gCarrierZ + offZ) << 11;
                    }
                    if (gHaveShuttle) {
                        if (gFramesInMode < 2) {
                            gCamEasedHeading = D_803FCD68 * 16;
                        } else {
                            gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                        }
                        gCamEyeGoalX = (s32) (gShuttlePos - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                        gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                        gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
                    }
                    if (gHaveCarrier || gHaveShuttle) {
                        break;
                    }
                case 4:
                case 8:
                    if (!gCamSnap) {
                        gCamEasedHeading = (s16) ((u16) D_8036443E * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                    } else {
                        gCamEasedHeading = D_8036443E * 16;
                    }
                    gCamEyeGoalX = (s32) (gPlayerX - SINE(gCamEasedHeading) * (gCamDistance * 2.0f) + offX) << 11;
                    gCamEyeGoalY = (gCamHeight + gPlayerY) << 11;
                    gCamEyeGoalZ = (s32) (gPlayerZ - COSINE(gCamEasedHeading) * (gCamDistance * 2.0f) + offZ) << 11;
                    break;
                case 2:
                case 0x20:
                    if (!gCamSnap) {
                        gCamEasedHeading = (s16) ((u16) D_8036443E * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                    } else {
                        gCamEasedHeading = D_8036443E * 16;
                    }
                    gCamEyeGoalX = (s32) (SINE(gCamEasedHeading) * (gCamDistance * 2.0f) + gPlayerX + offX) << 11;
                    gCamEyeGoalY = (gCamHeight + gPlayerY + gPadStickY * 50) << 11;
                    gCamEyeGoalZ = (s32) (COSINE(gCamEasedHeading) * (gCamDistance * 2.0f) + gPlayerZ + offZ) << 11;
                    break;
                case 0x10:
                case 0x40:
                    gCamEyeGoalX = ((gRdus[gRduLitOrder[D_8036EA7C]].unk0 << 5) + offX) << 11;
                    gCamEyeGoalY = ((gRdus[gRduLitOrder[D_8036EA7C]].unk2 << 5) + 2000) << 11;
                    gCamEyeGoalZ = ((gRdus[gRduLitOrder[D_8036EA7C]].unk4 << 5) + offZ) << 11;
                    break;
            }
            break;
        case 0x200:
            if (gHaveShuttle) {
                gCamEyeGoalX = (gShuttlePos + offX - 6000) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xD48) << 11;
                gCamEyeGoalZ = (D_803FCD50 + offZ + 6000) << 11;
            } else if (gCarrierHit) {
                gCamEyeGoalX = (gCarrierX + offX + 16000) << 11;
                gCamEyeGoalY = (gCarrierY + 15000) << 11;
                gCamEyeGoalZ = (gCarrierZ + offZ - 3000) << 11;
            } else {
                gCamEyeGoalX = (gCarrierX + offX + 4000) << 11;
                gCamEyeGoalY = (gCarrierY + 2400) << 11;
                if (levelno == 0) {
                    gCamEyeGoalY += 0x5DC000;
                }
                gCamEyeGoalZ = (gCarrierZ + offZ - 4000) << 11;
            }
            break;
        case 0x400:
            if (gHaveShuttle) {
                gCamEyeGoalX = (gShuttlePos + offX + 6000) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xD48) << 11;
                gCamEyeGoalZ = (D_803FCD50 + offZ - 6000) << 11;
            } else if (gCarrierHit) {
                gCamEyeGoalX = (gCarrierX + offX - 16000) << 11;
                gCamEyeGoalY = (gCarrierY + 15000) << 11;
                gCamEyeGoalZ = (gCarrierZ - offZ + 3000) << 11;
            } else {
                gCamEyeGoalX = (gCarrierX + offX - 4000) << 11;
                gCamEyeGoalY = (gCarrierY + 2400) << 11;
                if (levelno == 0) {
                    gCamEyeGoalY += 0x5DC000;
                }
                gCamEyeGoalZ = (gCarrierZ + offZ + 4000) << 11;
            }
            break;
        case 1:
        case 0x800:
            if (gHaveShuttle) {
                if (gFramesInMode < 2) {
                    gCamEasedHeading = D_803FCD68 * 16;
                } else {
                    gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                }
                gCamEyeGoalX = (s32) (gShuttlePos - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
            } else {
                gCamEyeGoalX = (gPlayerX + offX - 500) << 11;
                gCamEyeGoalY = (gPlayerY + 6000) << 11;
                gCamEyeGoalZ = (gPlayerZ + offZ + 500) << 11;
            }
            break;
        case 0x1000:
            gCamEyeGoalX = (gPlayerX + offX - 4000) << 11;
            gCamEyeGoalY = (gPlayerY + 5000) << 11;
            gCamEyeGoalZ = (gPlayerZ + offZ + 4000) << 11;
            break;
        default:
            if (gCamFixedEye) {
                gCamEyeGoalX = gCamFixedEyeX;
                gCamEyeGoalY = gCamFixedEyeY;
                gCamEyeGoalZ = gCamFixedEyeZ;
            } else {
                if (gLevelKind == 0x40) {
                    gCamDistance = 9000;
                    gCamHeight = 11000;
                    gCamEyeHeight = 11000;
                }
                if (gHaveShuttle && gCamPathActive) {
                    gCamEyeGoalX = (gShuttlePos + gCamPathX) << 11;
                    gCamEyeGoalY = (D_803FCD4C + gCamPathY) << 11;
                    gCamEyeGoalZ = (D_803FCD50 + gCamPathZ) << 11;
                } else if (gHaveCarrier && gCamPathActive) {
                    gCamEyeGoalX = (gCarrierX + gCamPathX) << 11;
                    gCamEyeGoalY = (gCarrierY + gCamPathY) << 11;
                    gCamEyeGoalZ = (gCarrierZ + gCamPathZ) << 11;
                } else if (D_8030F668) {
                    cam_polar_offset(2500, gCamHeading, &px, &pz);
                    gCamEyeGoalX = ((gEndObjectX << 5) + px) << 11;
                    gCamEyeGoalY = ((gEndObjectY << 5) + 5000) << 11;
                    gCamEyeGoalZ = ((gEndObjectZ << 5) + pz) << 11;
                } else if (D_8030F66A) {
                    cam_polar_offset(2500, gCamHeading, &px, &pz);
                    gCamEyeGoalX = (px + gCarrierTargetX) << 11;
                    gCamEyeGoalY = (gCarrierTargetY + 5000) << 11;
                    gCamEyeGoalZ = (pz + gCarrierTargetZ) << 11;
                } else if (D_8030F669) {
                    if (gMarkedTargetPts != NULL) {
                        cam_polar_offset(2500, gCamHeading, &px, &pz);
                        gCamEyeGoalX = ((gMarkedTargetPts->unk12 + gMarkedTargetPts->unk0) / 2 << 16) + (px << 11);
                        gCamEyeGoalY = ((gMarkedTargetPts->unk14 + gMarkedTargetPts->unk2) / 2 << 16) + 0x9C4000;
                        gCamEyeGoalZ = ((gMarkedTargetPts->unk16 + gMarkedTargetPts->unk4) / 2 << 16) + (pz << 11);
                    }
                } else {
                    if (D_80364A85) {
                        angle = (f32) (D_8036443E * 360 / 4096) - gCamHeading + 180.0f;
                        if (angle < 0) {
                            angle += 360;
                        }
                        if (angle >= 360) {
                            angle %= 360;
                        }
                        if (gCurVehicle == 2 || gCurVehicle == 9) {
                            cam_polar_offset(gCamDistance, angle, &px, &pz);
                        } else {
                            cam_polar_offset(gCamDistance / 2, angle, &px, &pz);
                        }
                        gCamEyeGoalY = (gCamEyeHeight * 3 / 5 + gPlayerY) << 11;
                        offX = 0;
                        offZ = 0;
                    } else {
                        cam_polar_offset(gCamDistance, gCamHeading, &px, &pz);
                        if (D_80364411 && gCamHeading >= 134.0 && gCamHeading <= 136.0) {
                            gCamEyeGoalY = (gPlayerY + 0x3908) << 11;
                        } else {
                            gCamEyeGoalY = (gPlayerY + gCamEyeHeight) << 11;
                        }
                    }
                    gCamEyeGoalX = (gPlayerX + px + offX) << 11;
                    gCamEyeGoalZ = (gPlayerZ + pz + offZ) << 11;
                }
            }
            break;
    }
    switch (gGameMode) {
        case 0x100:
            if (gHaveShuttle) {
                gCamAtGoalX = gShuttlePos / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else {
                gCamAtGoalX = gCarrierX / 32.0f;
                gCamAtGoalY = gCarrierY / 32.0f;
                gCamAtGoalZ = (gCarrierZ + D_80364A78 + 11000) / 32.0f;
            }
            break;
        case 0x200:
        case 0x400:
            if (gHaveShuttle) {
                gCamAtGoalX = (gShuttlePos + offX) / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = (D_803FCD50 + offZ) / 32.0f;
            } else {
                gCamAtGoalX = (gCarrierX + offX) / 32.0f;
                gCamAtGoalY = gCarrierY / 32.0f;
                gCamAtGoalZ = (gCarrierZ + offZ) / 32.0f;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (gHaveShuttle) {
                gCamAtGoalX = gShuttlePos / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else {
                gCamAtGoalX = (gChopperX + offX) / 32.0f;
                gCamAtGoalY = D_803EF314 / 32.0f;
                gCamAtGoalZ = (gChopperZ + offZ) / 32.0f;
            }
            break;
        case 0x40:
            if (gHaveShuttle) {
                gCamAtGoalX = gShuttlePos / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
                break;
            }
        default:
            if (gHaveShuttle && gCamPathActive) {
                gCamAtGoalX = gShuttlePos / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 6000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else if (gHaveCarrier && gCamPathActive) {
                gCamAtGoalX = gCarrierX / 32.0f;
                gCamAtGoalY = gCarrierY / 32.0f;
                gCamAtGoalZ = gCarrierZ / 32.0f;
            } else if (D_8030F668) {
                gCamAtGoalX = gEndObjectX;
                gCamAtGoalY = gEndObjectY;
                gCamAtGoalZ = gEndObjectZ;
            } else if (D_8030F66A) {
                gCamAtGoalX = gCarrierTargetX / 32.0f;
                gCamAtGoalY = gCarrierTargetY / 32.0f;
                gCamAtGoalZ = gCarrierTargetZ / 32.0f;
            } else if (D_8030F669) {
                if (gMarkedTargetPts != NULL) {
                    gCamAtGoalX = (f32) (gMarkedTargetPts->unk12 + gMarkedTargetPts->unk0) / 2.0;
                    gCamAtGoalY = (f32) (gMarkedTargetPts->unk14 + gMarkedTargetPts->unk2) / 2.0;
                    gCamAtGoalZ = (f32) (gMarkedTargetPts->unk16 + gMarkedTargetPts->unk4) / 2.0;
                }
            } else {
                gCamAtGoalX = (gPlayerX + offX) / 32.0f;
                gCamAtGoalY = (gPlayerY + gCamAtHeight) / 32.0f;
                gCamAtGoalZ = (gPlayerZ + offZ) / 32.0f;
            }
            break;
    }
    if (gCarrierHit && levelno == 0x31) {
        gCamEyeGoalX = (gCarrierX + offX + 16000) << 11;
        gCamEyeGoalY = (gCarrierY + 15000) << 11;
        gCamEyeGoalZ = (gCarrierZ + offZ - 3000) << 11;
    }
    if (gHaveShuttle && D_8036B965) {
        if (gCamSnap) {
            gCamEasedHeading = D_803FCD68 * 16;
        } else {
            gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
        }
        gCamEyeGoalX = (s32) (gShuttlePos - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
        gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
        gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
        gCamAtGoalX = gShuttlePos / 32.0f;
        gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
        gCamAtGoalZ = D_803FCD50 / 32.0f;
    }
    if (gGameMode == 0x40 && levelno == 0x3B) {
        gCamEyeGoalX = (gPlayerX + 0x1900) << 11;
        gCamEyeGoalY = (gPlayerY + 0x3070) << 11;
        gCamEyeGoalZ = (gPlayerZ - 0x1900) << 11;
        gCamAtGoalX = gPlayerX / 32.0f;
        gCamAtGoalY = gPlayerY / 32.0f;
        gCamAtGoalZ = gPlayerZ / 32.0f;
    }
    if ((gGameMode & (MODE_LEVEL | MODE_MAP_VIEW | MODE_LEVEL_START | MODE_YOSHI_PAGES)) && !gCamPathActive && !inShuttle && gGameMode != MODE_MAP_VIEW && levelno != 0x23 &&
        !D_80364A85) {
        tx = gCamAtGoalX * 65536.0;
        ty = gCamAtGoalY * 65536.0;
        tz = gCamAtGoalZ * 65536.0;
        if (levelno == 0x10 && gCamAtGoalX > 8718.0f && gCamAtGoalX < 8919.0f && gCamAtGoalZ > 9051.0f &&
            gCamAtGoalZ < 9251.0f) {
            gCamZoom = 1.5f;
            rateY = 0.1f;
        }
        gCamEyeGoalX = (gCamEyeGoalX - tx) * gCamZoom + tx;
        gCamEyeGoalY = (gCamEyeGoalY - ty) * gCamZoom + ty;
        gCamEyeGoalZ = (gCamEyeGoalZ - tz) * gCamZoom + tz;
    }
    if (!gCamSnap) {
        gCamEyeX = (gCamEyeGoalX - gCamEyeX) * rateX + gCamEyeX;
        gCamEyeY = (gCamEyeGoalY - gCamEyeY) * rateY + gCamEyeY;
        gCamEyeZ = (gCamEyeGoalZ - gCamEyeZ) * rateZ + gCamEyeZ;
    } else {
        gCamEyeX = gCamEyeGoalX;
        gCamEyeY = gCamEyeGoalY;
        gCamEyeZ = gCamEyeGoalZ;
        gCamPrevType = !gCamType;
    }
    if (D_80364424 && (gCamHeading < 134.0 || gCamHeading > 136.0)) {
        coll_begin_probe(gCamEyeX >> 11, gCamEyeY >> 11, gCamEyeZ >> 11);
        coll_probe_tris(gCamEyeX >> 11, gCamEyeY >> 11, gCamEyeZ >> 11,
                      D_80364434 ? D_80364430 : D_80364430 + 1600, 0xBE, 0);
        if (D_80364434) {
            if (gCollHit) {
                gCamEyeHeight += D_8036442C;
                if (gCamEyeHeight > D_80364428) {
                    gCamEyeHeight = D_80364428;
                }
                D_80364434 = 1;
            } else {
                D_80364434 = 0;
            }
        } else if (gCollHit) {
            D_80364434 = 1;
        } else {
            gCamEyeHeight -= D_8036442C;
            if (gCamEyeHeight < gCamHeight) {
                gCamEyeHeight = gCamHeight;
            }
            D_80364434 = 0;
        }
    }
    if (!D_80364424) {
        gCamEyeHeight = gCamHeight;
    } else if (gCamHeading >= 134.0 && gCamHeading <= 136.0) {
        gCamEyeHeight -= D_8036442C;
        if (gCamEyeHeight < gCamHeight) {
            gCamEyeHeight = gCamHeight;
        }
    }
    if (gCamType != gCamPrevType || changed) {
        gCamAtEaseX = gCamAtX - gCamAtGoalX;
        gCamAtEaseY = gCamAtY - gCamAtGoalY;
        gCamAtEaseZ = gCamAtZ - gCamAtGoalZ;
        gCamPrevType = gCamType;
        gCamAtSettled = 0;
    }
    if (!gCamAtSettled && !gCamSnap) {
        gCamAtEaseX = gCamAtEaseX * 0.95;
        gCamAtX = gCamAtEaseX + gCamAtGoalX;
        gCamAtEaseY = gCamAtEaseY * 0.97;
        gCamAtY = gCamAtEaseY + gCamAtGoalY;
        gCamAtEaseZ = gCamAtEaseZ * 0.95;
        gCamAtZ = gCamAtEaseZ + gCamAtGoalZ;
        if (gCamAtEaseX < 0.1 && gCamAtEaseX > -0.1 && gCamAtEaseZ < 0.1 && gCamAtEaseZ > -0.1 && gCamAtEaseY < 0.1 &&
            gCamAtEaseY > -0.1) {
            gCamAtSettled = 1;
        }
    } else {
        gCamAtX = gCamAtGoalX;
        gCamAtY = gCamAtGoalY;
        gCamAtZ = gCamAtGoalZ;
        gCamAtSettled = 1;
    }
    if (gShakeSize == 0) {
        gShakeSize = 1;
    }
    if (gShakeTime && (gGameMode & 0x2000100000000206)) {
        shakeX = D_803649D8 % gShakeSize - (gShakeSize >> 1);
        shakeY = D_803649D8 / 100 % gShakeSize - (gShakeSize >> 1);
        shakeZ = D_803649D8 / 10000 % gShakeSize - (gShakeSize >> 1);
    } else {
        shakeX = 0;
        shakeY = 0;
        shakeZ = 0;
    }
    if (gShakeTime) {
        gShakeTime--;
        gShakeSize = gShakeSize - gShakeSize / 6;
        gShakeSize = gShakeSize + 1;
    }
    eyeX = (f32) gCamEyeX / 65536.0;
    eyeY = (f32) gCamEyeY / 65536.0;
    eyeZ = (f32) gCamEyeZ / 65536.0;
    if ((gGameMode & 2) && D_802E8BF0 && gFramesInMode > D_80366A04) {
        eyeX = (eyeX - gCamAtX) * D_803649F8 + gCamAtX;
        eyeY = (eyeY - gCamAtY) * D_803649F8 + gCamAtY;
        eyeZ = (eyeZ - gCamAtZ) * D_803649F8 + gCamAtZ;
        if (D_803649F8 > 0.4) {
            D_803649F8 = D_803649F8 - 0.01;
        }
    }
    atX = (gCamAtX * 32.0f + shakeX) / 32.0f;
    atZ = (gCamAtZ * 32.0f + shakeZ) / 32.0f;
    adx = eyeX - atX;
    if (adx < 0.0) {
        adx = 0.0 - adx;
    }
    adz = eyeZ - atZ;
    if (adz < 0.0) {
        adz = 0.0 - adz;
    }
    if (adx > 0.5 || adz > 0.5) {
        guLookAtReflect(mtx, lookAt, eyeX, eyeY, eyeZ, (gCamAtX * 32.0f + shakeX) / 32.0f,
                        (gCamAtY * 32.0f + shakeY) / 32.0f, (gCamAtZ * 32.0f + shakeZ) / 32.0f, 0.0f, 1.0f, 0.0f);
    } else {
        guLookAtReflect(mtx, lookAt, eyeX + 2.0, eyeY, eyeZ + 2.0, (gCamAtX * 32.0f + shakeX) / 32.0f,
                        (gCamAtY * 32.0f + shakeY) / 32.0f, (gCamAtZ * 32.0f + shakeZ) / 32.0f, 0.0f, 1.0f, 0.0f);
    }
    dist = sqrtf((eyeX - gCamAtX) * (eyeX - gCamAtX) + (eyeZ - gCamAtZ) * (eyeZ - gCamAtZ));
    if (dist < 1.0) {
        dist = 1.0f;
    }
    if (eyeX >= gCamAtX && eyeZ >= gCamAtZ) {
        gCamYaw = math_asin_c((eyeX - gCamAtX) * 65535.9 / dist) >> 4;
        quadrant = 0;
    }
    if (eyeX >= gCamAtX && eyeZ < gCamAtZ) {
        gCamYaw = (math_asin_c((gCamAtZ - eyeZ) * 65535.9 / dist) >> 4) + 0x400; quadrant = 1;
    }
    if (eyeX < gCamAtX && eyeZ < gCamAtZ) {
        gCamYaw = (math_asin_c((gCamAtX - eyeX) * 65535.9 / dist) >> 4) + 0x800; quadrant = 2;
    }
    if (eyeX < gCamAtX && eyeZ >= gCamAtZ) {
        gCamYaw = (math_asin_c((eyeZ - gCamAtZ) * 65535.9 / dist) >> 4) + 0xC00; quadrant = 3;
    }
    dist = sqrtf((eyeX - gCamAtX) * (eyeX - gCamAtX) + (eyeZ - gCamAtZ) * (eyeZ - gCamAtZ) +
                 (eyeY - gCamAtY) * (eyeY - gCamAtY));
    if (eyeY >= gCamAtY) {
        pitch1 = math_asin_c((eyeY - gCamAtY) * 65535.9 / dist) >> 3;
    } else {
        pitch1 = 0x1000 - (math_asin_c((gCamAtY - eyeY) * 65535.9 / dist) >> 3);
    }
    if (eyeY >= gCamAtY) {
        pitch2 = math_asin_c((65535.0 < (eyeY - gCamAtY) * 65535.9 / 20000.0) ? 65535.0 : (eyeY - gCamAtY) * 65535.9 / 20000.0) >> 3;
    } else {
        pitch2 = 0x1000 - (math_asin_c((65535.0 < (gCamAtY - eyeY) * 65535.9 / 20000.0) ? 65535.0 : (gCamAtY - eyeY) * 65535.9 / 20000.0) >> 3);
    }
    gCamPitch = pitch1 - pitch2;
    scale = cam_eye_scale(eyeX, eyeY, eyeZ, gCamAtX, gCamAtY, gCamAtZ);
    posX = (eyeX - gCamAtX) * scale + gCamAtX;
    posY = (eyeY - gCamAtY) * scale + gCamAtY;
    posZ = (eyeZ - gCamAtZ) * scale + gCamAtZ;
    if (adx > 0.5 || adz > 0.5) {
        guLookAt(view, posX, posY, posZ, gCamAtX, gCamAtY, gCamAtZ, 0.0f, 1.0f, 0.0f);
    } else {
        guLookAt(view, posX + 2.0, posY, posZ + 2.0, gCamAtX, gCamAtY, gCamAtZ, 0.0f, 1.0f, 0.0f);
    }
    PORT_ICUT_IF(gCamSnap);
    gCamSnap = 0;
}

/* The culling view's eye factor for the current vehicle type: the eye is
 * moved to target + (eye - target) * factor, the factor reduced so that the
 * distance stays within 10000 (0x80254E54). */
f32 cam_eye_scale(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1) {
#ifdef NON_MATCHING
    f32 speed = 0.0f; /* ids without a case leave the original's stack slot as it was (indeterminate) */
#else
    f32 speed;
#endif
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;

    switch (gCurVehicle) {
        case 0:
            speed = 3.5f;
            break;
        case 1:
            speed = 1.4f;
            break;
        case 2:
            speed = 1.6f;
            break;
        case 3:
            speed = 4.8f;
            break;
        case 4:
            speed = 5.0f;
            break;
        case 5:
            speed = 5.0f;
            break;
        case 6:
            speed = 1.6f;
            break;
        case 7:
            speed = 1.6f;
            break;
        case 8:
            speed = 4.8f;
            break;
        case 9:
            speed = 1.6f;
            break;
        case 10:
            speed = 5.0f;
            break;
        case 11:
        case 17:
        case 18:
            speed = 1.6f;
            break;
        case 13:
            speed = 4.8f;
            break;
        case 14:
            speed = 4.8f;
            break;
        case 15:
            speed = 4.8f;
            break;
        case 16:
            speed = 1.6f;
            break;
    }
    dx = x0 - x1;
    dx = dx * dx;
    dy = y0 - y1;
    dy = dy * dy;
    dz = z0 - z1;
    dz = dz * dz;
    dist = sqrtf(dx + dy + dz);
    if (dist * speed > 10000.0f) {
        speed = speed - (dist * speed - 10000.0f) / dist;
    }
    return speed;
}

/* Polar to cartesian: point at angle (degrees) on a circle of radius r*sqrt(2)
 * (0x80255034). */
void cam_polar_offset(s32 r, f32 angle, s32 *outX, s32 *outY) {
    f32 len;
    f32 r2;
    s32 x;
    s32 y;
    f32 a;

    r2 = r * r;
    len = sqrtf(r2 + r2);
    a = angle;
    a = a / 360.0;
    a = a * 6.28318;
    x = sinf(a) * len;
    y = sqrtf(len * len - x * x);
    if (angle >= 90.0 && angle < 270.0) {
        y = 0.0 - y;
    }
    if (y >= -100 && y <= 100) {
        y = 0;
    }
    *outX = x;
    *outY = y;
}

/* C-right / C-left turn gCamHeading 45 degrees, 3 degrees a frame (with a
 * sound); not where cam_fixed_heading_area holds it (0x80255190). */
void cam_turn_heading(void) {
    u8 ok;

    if (gPaused == 0) {
        ok = cam_fixed_heading_area();
        if (ok && ((gHeldCRight && !gPrevHeldCRight) || (gHeldCLeft && !gPrevHeldCLeft)) && (gGameMode & (MODE_LEVEL | MODE_MAP_VIEW))) {
            snd_play(gSfxBank, 0xD0, NULL);
        }
        if (gHeldCRight && !gPrevHeldCRight && !gCamTurnPos && !gCamTurnNeg && !ok && gGameMode != MODE_LEVEL_START) {
            gCamHeadingGoal = gCamHeading + 45.0;
            if (gCamHeadingGoal >= 360.0) {
                gCamHeadingGoal = gCamHeadingGoal - 360.0;
            }
            gCamTurnPos = 1;
            if (gGameMode != 0x40) {
                snd_play(gSfxBank, 0xDD, NULL);
            }
        }
        if (gCamTurnPos) {
            if (gCamHeading < gCamHeadingGoal) {
                gCamHeading = gCamHeading + 3.0;
                if (gCamHeading >= gCamHeadingGoal) {
                    gCamTurnPos = 0;
                    gCamHeading = gCamHeadingGoal;
                }
            } else {
                gCamHeading = gCamHeading + 3.0;
                if (gCamHeading >= 360.0) {
                    gCamHeading = gCamHeading - 360.0;
                    if (gCamHeading >= gCamHeadingGoal) {
                        gCamTurnPos = 0;
                        gCamHeading = gCamHeadingGoal;
                    }
                }
            }
        }
        if (gHeldCLeft && !gPrevHeldCLeft && !gCamTurnPos && !gCamTurnNeg && !ok && gGameMode != MODE_LEVEL_START) {
            gCamHeadingGoal = gCamHeading - 45.0;
            if (gCamHeadingGoal < 0.0) {
                gCamHeadingGoal = gCamHeadingGoal + 360.0;
            }
            gCamTurnNeg = 1;
            if (gGameMode != 0x40) {
                snd_play(gSfxBank, 0xDC, NULL);
            }
        }
        if (gCamTurnNeg) {
            if (gCamHeading > gCamHeadingGoal) {
                gCamHeading = gCamHeading - 3.0;
                if (gCamHeading <= gCamHeadingGoal) {
                    gCamTurnNeg = 0;
                    gCamHeading = gCamHeadingGoal;
                }
            } else {
                gCamHeading = gCamHeading - 3.0;
                if (gCamHeading < 0.0) {
                    gCamHeading = gCamHeading + 360.0;
                    if (gCamHeading <= gCamHeadingGoal) {
                        gCamTurnNeg = 0;
                        gCamHeading = gCamHeadingGoal;
                    }
                }
            }
        }
        if (gCamTurnNeg || gCamTurnPos) {
            cull_restart(2);
        }
    }
}

/* 1 where the heading is held at 135 degrees (and turned back to it): the
 * attract mode, the sequences and some areas of levels 4, 13, 16 and 59
 * (0x80255628). */
u8 cam_fixed_heading_area(void) {
    u8 ok = 0;

    if (gGameMode == MODE_GLOBE_SEQ || gGameMode == 2) {
        ok = 1;
    } else {
        switch (levelno) {
            case 4:
                if (ground_in_triangle(gPlayerX >> 5, gPlayerZ >> 5, 0xE56, 0x8EC, 0xBB8, 0x6A4, 0x1068, 0x1F4) ||
                    ground_in_triangle(gPlayerX >> 5, gPlayerZ >> 5, 0xE56, 0x8EC, 0x1068, 0x1F4, 0x1324, 0x4B0)) {
                    ok = 1;
                }
                break;
            case 16:
                if (gPlayerX > 0x46500 && gPlayerZ > 0x3E800) {
                    ok = 1;
                }
                break;
            case 13:
                if (gPlayerX > 0x42680 && gPlayerZ > 0x46500) {
                    ok = 1;
                }
                break;
            case 0x3B:
                ok = 1;
                break;
        }
    }
    if (ok) {
        if (gCamHeadingGoal < 134.0 || gCamHeadingGoal > 136.0) {
            gCamTurnNeg = 0;
            gCamTurnPos = 0;
            gCamHeadingGoal = 135.0f;
            if (gCamHeadingGoal < gCamHeading) {
                if (gCamHeading - gCamHeadingGoal > 180.0) {
                    gCamTurnPos = 1;
                } else {
                    gCamTurnNeg = 1;
                }
            } else {
                gCamTurnPos = 1;
            }
        }
    }
    return ok;
}

/* Clears the current frame buffer with a fill rectangle (0x802558C8). */
void hd_clear_framebuffer(Gfx *arg0, s32 *len) {
    Gfx *gdl = arg0;

    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, gFrameBufferPhys[gBufferIndex]);
    gDPSetFillColor(gdl++, 0x00010001);
    gDPPipeSync(gdl++);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    *len += gdl - arg0;
}

/* Closes the top-level display list and returns its length in *length
 * (0x802559F8). */
void hd_close_display_list(Gfx *arg0, s32 *length) {
    Gfx *gdl = arg0;

    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    *length = gdl - gDynamicBufs[gBufferIndex].dl;
    HD_ASSERT(*length<TOPLEVEL_DL_SIZE, 3665);
}

/* Once, from the main thread: the DMA and texture-DMA queues, the scheduler
 * and the main thread's client, the controllers, audio, VI features, the frame
 * buffer addresses, the first game mode (from the controller pattern and the
 * boot flags) and the stack guard pattern (0x80255AD0). */
void hd_init(void) {
    s32 i;
    f32 one;
    s32 pad[4];
    u8 status;

    one = 1.0f;
    gGameMode = 32;
    osCreateMesgQueue(&gDmaMesgQueue, D_803150B8, 50);
    osCreateMesgQueue(&textureDmaMessageQ, D_80315198, 0x90);
    sched_create((struct BcSched *) gScheduler, gSchedStack + 0x400, 13, (D_80000300 != 1) ? 0x10 : 2, 1);
    osCreateMesgQueue(&gTaskDoneQueue, D_803153F8, 16);
    sched_add_client((struct BcSched *) gScheduler, (struct BcScClient *) gMainSchedClient, &gTaskDoneQueue, 1, 1);
    status = cont_init();
    music_init_audio();
    debug_printf("audio inited\n");
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
    gFrameBufferPhys[0] = PHYS(gFrameBuffers[0]);
    gFrameBufferPhys[1] = PHYS(gFrameBuffers[1]);
    gDepthBufferPhys = PHYS(D_8021ED00);
    rsp_init_ucode_table();
    func_802D6710();
    cont_boot_check_start();
    if (!(status & 1)) {
        gNextGameMode = 0x0800000000000000;
    } else if (D_802FDBD0) {
        gNextGameMode = 0x0000080000000000;
    } else if (D_802FDBD4) {
        gNextGameMode = MODE_ERASE_SAVE;
    } else {
        gNextGameMode = MODE_BOOT_SCREEN1;
    }
    for (i = 0x1FF; i >= 0; i--) {
        gMainThreadStack[i] = 0x1122334455667788LL;
    }
}

/* After every frame: checks the main thread's stack guard pattern from the
 * top down and prints where it ends (0x80255D34). */
void hd_check_stack(void) {
    u8 bad;
    s32 i;

    bad = 0;
    for (i = 0x1FF; i >= 0 && !bad; i--) {
        if (gMainThreadStack[i] != 0x1122334455667788LL) {
            bad = 1;
            debug_printf("stack end =%d\n", i);
        }
    }
}

/* Called by most mode switches before the new mode starts: screen off,
 * frame counters zeroed, the static segment loaded, the heap reset (RDP
 * output buffer, ghost buffer, vehicle shadow records, Yoshi windows) and the
 * per-mode subsystems reset (0x80255DC8). */
void hd_mode_setup(void) {
    u32 end;
    s32 pad;
    u32 size;
    s32 old;

    size = D_00788000 - D_00787F40;
    osViBlack(1);
    gPendingTune = music_tune_for_mode(gNextGameMode);
    func_802D6710();
    osInvalDCache((void *) 0x80000000, 0x400000);
    D_803649F4 = 0;
    gUnpausedFrames = 0;
    D_80358064 = 0;
    gFramesInMode = 0;
    gBufferIndex = 0;
    gHudAlpha = 0;
    util_seed_rand();
    util_seed_rand2();
    cont_clear();
    func_8028B720();
    gStaticSegment = D_803FF600;
    func_8028B4C4((u32) D_00787F40, (u32) D_803FF600, &size, 10, 0, 2);
    end = (u32) (D_00788000 - D_00787F40) + (u32) D_803FF600;
    debug_printf("Static end = 0x%x, space=0x%x (%d) bytes\n", end, 0x80400000 - end, 0x80400000 - end);
    gFrameDlLen = 0;
    hd_clear_framebuffer(gDynamicBufs[gBufferIndex].dl, &gFrameDlLen);
    hd_close_display_list(gDynamicBufs[gBufferIndex].dl, &gFrameDlLen);
    gHeapPtr = 0x8004B400;
    align_up(&gHeapPtr, 0x10);
    gRdpOutputBuffer = gHeapPtr;
    gHeapPtr += 0xA000;
    if (gLevelInfo[levelno].unk0 == 2 && !(gNextGameMode & (MODE_ATTRACT | MODE_GLOBE_SEQ))) {
        debug_printf("Allocating ghost buffer memory\n");
        gHeapPtr += 0x20000;
    }
    gTexTableLoaded = 0;
    tex_load_table();
    gVehicleShadows = (VehShadow *) (((u32) D_80358088 + 0x40) & ~0x3F);
    mb_init();
    cmo_hit_request = 0;
    gLevelDoneRequest = 0;
    D_803643D8 = 0;
    gCarrierHit = 0;
    gLevelDone = 0;
    gPauseRequest = 0;
    gUnpauseRequest = 0;
    gPaused = 0;
    D_8036EB99 = 0;
    D_803669B4 = 0;
    if (gNextGameMode & 0xC9FD8FE7FBFFC0B0) {
        func_8028B3E0();
    }
    academy_place_marker(levelno);
    spr_reset();
    if (gNextGameMode == MODE_BEST_TIMES) {
        fe_best_build_menu();
    }
    old = gHeapPtr;
    yoshi_init(0);
    debug_printf("Yoshi windows allocated %d bytes, %x\n", gHeapPtr - old, gHeapPtr);
    gVehiclesEnd = gVehicles;
    cont_wait_read();
    text_init();
    gPadDisplayLoaded = 0;
}

/* Inflates level's data file (lagp, chimp, valley, ..., level59: the ROM symbols
 * below; docs/blast_corps_levels.txt) to dest; *size gets its size (0x8025615C). */
void lvl_load_file(s32 level, s32 dest, s32 *size) {
    u8 *start;

    switch (level) {
        case 1:
            start = D_004A5660;
            *size = D_004ACC10 - D_004A5660;
            break;
        case 0:
            start = D_004ACC10;
            *size = D_004B8960 - D_004ACC10;
            break;
        case 2:
            start = D_004B8960;
            *size = D_004BFD60 - D_004B8960;
            break;
        case 3:
            start = D_004BFD60;
            *size = D_004C3AC0 - D_004BFD60;
            break;
        case 4:
            start = D_004C3AC0;
            *size = D_004D5F90 - D_004C3AC0;
            break;
        case 5:
            start = D_004D5F90;
            *size = D_004E2F70 - D_004D5F90;
            break;
        case 6:
            start = D_004E2F70;
            *size = D_004E4E80 - D_004E2F70;
            break;
        case 7:
            start = D_004E4E80;
            *size = D_004E7C00 - D_004E4E80;
            break;
        case 8:
            start = D_004E7C00;
            *size = D_004E8F70 - D_004E7C00;
            break;
        case 9:
            start = D_004E8F70;
            *size = D_004F5C10 - D_004E8F70;
            break;
        case 10:
            start = D_004F5C10;
            *size = D_00500520 - D_004F5C10;
            break;
        case 11:
            start = D_00500520;
            *size = D_00507E80 - D_00500520;
            break;
        case 12:
            start = D_00507E80;
            *size = D_00511340 - D_00507E80;
            break;
        case 13:
            start = D_00511340;
            *size = D_00523080 - D_00511340;
            break;
        case 14:
            start = D_00523080;
            *size = D_0052CD00 - D_00523080;
            break;
        case 15:
            start = D_0052CD00;
            *size = D_00532700 - D_0052CD00;
            break;
        case 16:
            start = D_00532700;
            *size = D_0053E9B0 - D_00532700;
            break;
        case 17:
            start = D_0053E9B0;
            *size = D_0054A820 - D_0053E9B0;
            break;
        case 18:
            start = D_0054A820;
            *size = D_00552DE0 - D_0054A820;
            break;
        case 19:
            start = D_00552DE0;
            *size = D_00555000 - D_00552DE0;
            break;
        case 20:
            start = D_00555000;
            *size = D_00560E90 - D_00555000;
            break;
        case 21:
            start = D_00560E90;
            *size = D_005652D0 - D_00560E90;
            break;
        case 22:
            start = D_005652D0;
            *size = D_0056F3F0 - D_005652D0;
            break;
        case 23:
            start = D_0056F3F0;
            *size = D_005721E0 - D_0056F3F0;
            break;
        case 24:
            start = D_005721E0;
            *size = D_005736E0 - D_005721E0;
            break;
        case 25:
            start = D_005736E0;
            *size = D_0057A2C0 - D_005736E0;
            break;
        case 26:
            start = D_0057A2C0;
            *size = D_00580B60 - D_0057A2C0;
            break;
        case 27:
            start = D_00580B60;
            *size = D_00588CE0 - D_00580B60;
            break;
        case 28:
            start = D_00588CE0;
            *size = D_0058BE80 - D_00588CE0;
            break;
        case 29:
            start = D_0058BE80;
            *size = D_00597B80 - D_0058BE80;
            break;
        case 30:
            start = D_00597B80;
            *size = D_0059B7D0 - D_00597B80;
            break;
        case 31:
            start = D_0059B7D0;
            *size = D_005A5840 - D_0059B7D0;
            break;
        case 32:
            start = D_005A5840;
            *size = D_005B0B10 - D_005A5840;
            break;
        case 33:
            start = D_005B0B10;
            *size = D_005B5A30 - D_005B0B10;
            break;
        case 34:
            start = D_005B5A30;
            *size = D_005B8BB0 - D_005B5A30;
            break;
        case 35:
            start = D_005B8BB0;
            *size = D_005C4C80 - D_005B8BB0;
            break;
        case 36:
            start = D_005C4C80;
            *size = D_005CA9C0 - D_005C4C80;
            break;
        case 37:
            start = D_005CA9C0;
            *size = D_005CCF50 - D_005CA9C0;
            break;
        case 38:
            start = D_005CCF50;
            *size = D_005D1060 - D_005CCF50;
            break;
        case 39:
            start = D_005D1060;
            *size = D_005DC830 - D_005D1060;
            break;
        case 40:
            start = D_005DC830;
            *size = D_005E6EE0 - D_005DC830;
            break;
        case 41:
            start = D_005E6EE0;
            *size = D_005EC800 - D_005E6EE0;
            break;
        case 42:
            start = D_005EC800;
            *size = D_005F3A80 - D_005EC800;
            break;
        case 43:
            start = D_005F3A80;
            *size = D_006014B0 - D_005F3A80;
            break;
        case 44:
            start = D_006014B0;
            *size = D_0060A710 - D_006014B0;
            break;
        case 45:
            start = D_0060A710;
            *size = D_00613AA0 - D_0060A710;
            break;
        case 46:
            start = D_00613AA0;
            *size = D_0061DD70 - D_00613AA0;
            break;
        case 47:
            start = D_0061DD70;
            *size = D_00621AF0 - D_0061DD70;
            break;
        case 48:
            start = D_00621AF0;
            *size = D_006269E0 - D_00621AF0;
            break;
        case 49:
            start = D_006269E0;
            *size = D_00630C30 - D_006269E0;
            break;
        case 50:
            start = D_00630C30;
            *size = D_00635700 - D_00630C30;
            break;
        case 51:
            start = D_00635700;
            *size = D_0063CA10 - D_00635700;
            break;
        case 52:
            start = D_0063CA10;
            *size = D_00641F30 - D_0063CA10;
            break;
        case 53:
            start = D_00641F30;
            *size = D_00644810 - D_00641F30;
            break;
        case 54:
            start = D_00644810;
            *size = D_00646080 - D_00644810;
            break;
        case 55:
            start = D_00646080;
            *size = D_00647550 - D_00646080;
            break;
        case 56:
            start = D_00647550;
            *size = D_00654FC0 - D_00647550;
            break;
        case 57:
            start = D_00654FC0;
            *size = D_00660950 - D_00654FC0;
            break;
        case 58:
            start = D_00660950;
            *size = D_00665F80 - D_00660950;
            break;
        case 59:
            start = D_00665F80;
            *size = D_0066C900 - D_00665F80;
            break;
    }
    func_8028B4C4((u32) start, (u32) ((void *) N64_IPTR(dest)), (u32 *) size, 12, 10, 1);
}

/* hd.c's level setup (arg0: a saved status stream or 0): the level's state,
 * its file on the heap (lvl_load_file), initlevel, the display list buffers,
 * the vehicles' shadows, HUD and camera start values (0x80256A34). */
void hd_setup_level(s32 arg0) {
    s32 i;
    s32 size;
    VehShadow *v;
    u8 found;
    s32 old;

    gHudDialLoaded = 0;
    gHaveSkyfall = 0;
    gHudMissileIconLoaded = 0;
    D_80364A6B = 0;
    gHudRamIconLoaded = 0;
    gHaveSideswipe = 0;
    D_803649E8 = 0;
    D_803649EC = 0;
    gVehicleShadowsEnd = gVehicleShadows;
    gLevelKind = gLevelInfo[levelno].unk0;
    gShakeTime = 0;
    gShakeSize = 0;
    gPlayerX = 0;
    gPlayerY = 0;
    gPlayerZ = 0;
    D_8036443C = 0;
    gCamHeading = 135.0f;
    gCamHeadingGoal = 135.0f;
    gCamTurnPos = 0;
    gCamTurnNeg = 0;
    gCamEyeHeight = 3000;
    D_80364434 = 1;
    gTrainContactFrames = 0;
    D_803649EE = 0;
    D_80364A84 = 0;
    gCurVehicle = 0;
    if (frontEndPresent) {
        HD_ASSERT(!pakBusy, 4162);
        HD_ASSERT(MQ_IS_EMPTY(&pakToGameMessageQ), 4163);
        sched_remove_client((struct BcSched *) gScheduler, (struct BcScClient *) gPakSchedClient);
        func_802D67F0((OSThread *) gPakThread);
    }
    gGhostRecording = 0;
    gGhostPlaying = 0;
    if (gLevelKind == 2 && gNextGameMode == MODE_LEVEL_START) {
        if (!D_8039CA60) {
            ghost_init();
        }
        ghost_start_recording();
        ghost_start_playback();
    }
    lvl_load_file(levelno, gHeapPtr, &size);
    gLevelData = (u8 *) N64_IPTR(gHeapPtr);
    gHeapPtr += size;
    align_up(&gHeapPtr, 0x10);
    stats_level_start();
    hud_init_target_marker();
    lvl_init_chopper_pos(levelno);
    func_80262238(levelno);
    cam_set_level_fovy(levelno);
    gTurboPending = 0;
    radar_clear_markers();
    debug_printf("enter initlevel game_mode=%d loop_done=%d\n", lowest_set_bit(gGameMode), lowest_set_bit(gNextGameMode));
    old = gHeapPtr;
    initlevel(gLevelData, arg0);
    debug_printf("exit initlevel allocated %d bytes, %x\n", gHeapPtr - old, gHeapPtr);
    lvl_alloc_dl_buffers();
    if (gNextGameMode != 2) {
        if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
            commpt_load_flags();
        }
    }
    obj_tally(gLevelKind == 0x20 || gLevelKind == 0x80);
    hud_init_objective(levelno);
    if (gCamFixedEye) {
        D_80364A86 = spr_load((u16 *) D_802E8BF4, 0, 1, 1, 1, 1.0f);
    }
    hud_panel_init();
    if (gHaveSkyfall) {
        hud_init_boost_dial();
    }
    if (D_80364A6B) {
        hud_init_missile_icon();
    }
    if (gHaveSideswipe) {
        hud_init_ram_icon();
    }
    hud_init_veh_overlay();
    fx_init_carrier_blast();
    fx_init_power_pill(levelno);
    if (gLevelInfo[levelno].unk0 == 1) {
        people_init(arg0);
    }
    ptcl_init();
    trail_init();
    shell_init();
    water_init(levelno);
    fx_init_drift_quads(levelno);
    util_cam_path_init(levelno);
    util_reset_hint_events();
    v = gVehicleShadows;
    while (v != gVehicleShadowsEnd) {
        i = 0;
        found = 0;
        while (!found) {
            if (gVehicles[i].type == v->type) {
                found = 1;
            } else {
                i++;
            }
        }
        veh_render_shadow_tex(v, v->unk1004, v->unk1010, v->unk100C, v->unk1000, (Gfx *) N64_IPTR((gVehicles[i].unk54)), gVehicles[i].unk0,
                      gVehicles[i].unk4);
        rsp_wait_task(0x61F);
        v++;
    }
    backdrop_init(gNextGameMode, levelno);
    gCamYaw = 0x2000;
    gCamPitch = 0x2000;
    gCurVehicle = 0;
    gNextVehicle = 0;
    gCamEyeX = 0;
    gCamEyeY = 0;
    gCamEyeZ = 0;
    gCamTilt = 0.0f;
    gCamTurn = 0.0f;
    D_803649F8 = 1.0f;
    D_80364A3C = 0;
    D_80364A3D = 0;
    D_80364A40 = 0;
    D_80364A44 = 0;
    D_80364A6F = 0;
    D_80364AB4 = 0.0f;
    D_80364AB8 = 1.0f;
    D_80364ABC = 1.0f;
    D_80364AC0 = 0;
    gPlayerX = gDriverPos;
    gPlayerY = D_803ED80C;
    gPlayerZ = D_803ED810;
    D_802FDB14 = 0;
    if (gUseEeprom) {
        D_803649F0 = nss.ip;
    } else {
        D_803649F0 = players[playerNumber].unk14;
    }
    D_80364A5C = 0;
    D_80364A58 = 0;
    if (D_803669B4) {
        func_8025BD98();
    }
    debug_printf("Level %d: mem_pool=0x%x, code seg=0x%x, space=%d bytes\n", levelno, gHeapPtr, 0x802447C0,
                  0x8021ED00 - gHeapPtr);
    if (gAcademyMarkerOn) {
        func_802979E0(levelno);
    }
    cam_load_level_defaults();
    fx_alloc_texture_caches();
}

/* Allocates the per-level display list buffers (two of each) from the heap
 * (0x80257234). */
void lvl_alloc_dl_buffers(void) {
#ifdef NON_MATCHING
    s32 a = 0; /* levels without a case leave it indeterminate in the original */
#else
    s32 a;
#endif
#ifdef NON_MATCHING
    s32 b = 0; /* levels without a case leave it indeterminate in the original */
#else
    s32 b;
#endif
#ifdef NON_MATCHING
    s32 c = 0; /* levels without a case leave it indeterminate in the original */
#else
    s32 c;
#endif
#ifdef NON_MATCHING
    s32 d = 0; /* levels without a case leave it indeterminate in the original */
#else
    s32 d;
#endif

    switch (levelno) {
        case 16:
        case 29:
            a = 50;
            b = 50;
            c = 50;
            d = 200;
            break;
        case 17:
            a = 50;
            b = 50;
            c = 50;
            d = 50;
            break;
        case 11:
            a = 3000;
            b = 50;
            c = 50;
            d = 200;
            break;
        case 10:
            a = 50;
            b = 50;
            c = 50;
            d = 2000;
            break;
        default:
            switch (D_803BE739) {
                case 0:
                    a = 6000;
                    b = 5000;
                    c = 100;
                    d = 2000;
                    break;
                case 1:
                    a = 1000;
                    b = 1000;
                    c = 100;
                    d = 2000;
                    break;
            }
            break;
    }
    gLevelDl0[0] = gHeapPtr;
    gHeapPtr += a * 8;
    gLevelDl1[0] = gHeapPtr;
    gHeapPtr += b * 8;
    gLevelDl2[0] = gHeapPtr;
    gHeapPtr += c * 8;
    gLevelDl3[0] = gHeapPtr;
    gHeapPtr += d * 8;
    gLevelDl0[1] = gHeapPtr;
    gHeapPtr += a * 8;
    gLevelDl1[1] = gHeapPtr;
    gHeapPtr += b * 8;
    gLevelDl2[1] = gHeapPtr;
    gHeapPtr += c * 8;
    gLevelDl3[1] = gHeapPtr;
    gHeapPtr += d * 8;
}

/* Rounds *arg0 up to a multiple of arg1 (the heap pointer) (0x80257490). */
void align_up(s32 *arg0, s32 arg1) {
    s32 r;

    r = *arg0 % arg1;
    if (r != 0) {
        r = arg1 - r;
    }
    *arg0 += r;
}


/* sinf (0x802574F0); the camera's SINE() */
f32 hd_sinf(f32 arg0) {
    return sinf(arg0);
}


/* cosf (0x80257514); the camera's COSINE() */
f32 hd_cosf(f32 arg0) {
    return cosf(arg0);
}
