#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gHeapPtr
#define LEGACY_D_80358074
#define LEGACY_D_80364458
#define LEGACY_D_803649D0
#define LEGACY_D_80367734
#define LEGACY_D_80367738
#define LEGACY_D_8036BED8
#define LEGACY_D_8036C794
#define LEGACY_gRdpOutputBuffer
#define LEGACY_D_803ED808
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuEntry *) D_8020C070)
#define D_80218D30 ((u8 *) &D_80218D30)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F5804 ((MenuItem *) D_802F5804)
#define D_802F8BDC ((MenuPage *) D_802F8BDC)
#define gScheduler ((u8 *) &gScheduler)
#define gMainSchedClient ((u8 *) &gMainSchedClient)
#define D_80364460 ((Unk74 *) D_80364460)
#define players ((Player *) players)
#define pakBuffer ((u8 *) pakBuffer)
#ifdef NON_MATCHING
#define gHeapPtr (*(s32 *) &gHeapPtr)
#define D_80364458 (*(u32 *) &D_80364458)
#define D_803649D0 (*(Unk74 * N64P *) &D_803649D0)
#define D_8036BED8 (*(Node88 * N64P *) &D_8036BED8)
#define D_8036C794 (*(Box * N64P *) &D_8036C794)
#define gRdpOutputBuffer (*(s32 *) &gRdpOutputBuffer)
#define D_803ED808 (*(s32 *) D_803ED808)
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

/* a drawable object, 0x74 bytes */
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
    s32 unk5C; /* id */
    u32 unk60; /* alpha */
    u8 pad64[0xC];
    s32 unk70; /* active */
} Unk74;
#ifndef NON_MATCHING
extern Unk74 *D_803649D0;
#endif


extern u8 D_00787F40[]; /* static code segment bounds (ROM) */
extern u8 D_00788000[];
extern u8 D_80364A70;
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
extern s32 D_803156F0;
#ifndef NON_MATCHING
extern s32 D_80367738;
#endif
s32 func_8024AFA8(s32);

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
extern s32 D_80358074;
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


extern u16 D_802E8BF4[]; /* sprite ids (func_80272C5C) */
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
extern s32 D_803ED808;
#endif
void func_8025615C(s32, s32, s32 *);
void func_80257234(void);


extern s32 D_80358030[2];
extern s32 D_80358038[2];
extern s32 D_80358040[2];
extern s32 D_80358048[2];
extern s32 D_803156EC;
u8 func_8024B4B8(void);
s32 func_8024B418(u8);

extern u8 D_803153F0;
extern s32 D_803156E8;

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
extern u32 D_80364458;
#endif
extern u16 D_80304904[]; /* glyph lists */
extern u16 D_80304910[];
extern u16 D_8030491C[];
extern u16 D_80304938[];
void func_8024E4F4(Gfx **, DynamicBuf *, u8);
void func_8024F520(Gfx **, DynamicBuf *);
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
#define MODE_PAGE(m) D_802F8BDC[D_802F4868[lowest_set_bit(m)]]
#define MODE_ENTRY(m) D_8020C070[MODE_PAGE(m).unkE + MODE_PAGE(m).unk10 - 2]

extern s32 D_80364A78;
extern u8 D_80364A7C;
extern s32 D_80364A80;
extern f32 gCamZoom;
extern u8 D_80364A85;
#ifndef NON_MATCHING
extern s32 D_80367734;
#endif
#define NUM_TEXTURE_DMAS 0x90
void cull_start(Mtx *, Mtx *);
void func_8024AE2C(void);
void func_8024B188(void);
void func_8024B618(void);
void func_8024B7AC(void);
void func_8024A92C(u32);
void func_8024B5E8(void);
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
        D_80364A70 = 0;
        do {
            debug_printf("game mode switch from %d to %d\n", lowest_set_bit(gGameMode), lowest_set_bit(gNextGameMode));
            gGameModeAfterNext = 0;
            switch (gNextGameMode) {
                case 32:
                    hd_mode_setup();
                    func_801EF380(2);
                    break;
                case 16:
                    hd_mode_setup();
                    func_801EF380(1);
                    start2 = D_803156C4;
                    while ((u32) (D_803156C4 - start2) < 15) {
                        PORT_SPIN();
                    }
                    break;
                case 0x100000000:
                    func_801F6F18();
                    func_8026AF6C(0x8012);
                    D_80364A70 = func_80261A44(gNextGameMode);
                    break;
                case 0x80000000000:
                    hd_mode_setup();
                    func_8025D184();
                    func_80200714(1);
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000001, 1);
                    func_8026AF6C(0x8011);
                    break;
                case 0x10000:
                    D_80364A70 = func_80261A44(gNextGameMode);
                    func_8025D184();
                    func_801EA4B8();
                    func_801E8DCC(playerNumber);
                    func_8026AF6C(0x800A);
                    break;
                case 0x10000000:
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000001, 1);
                    func_801E8DCC(4);
                    func_8026AF6C(0x8011);
                    break;
                case 0x20000000000000:
                    func_802609D0();
                    hd_mode_setup();
                    func_802A0700();
                    func_8025D184();
                    func_80200714(1);
                    osSendMesg(&D_80219EF8, (OSMesg) 0x0100000F, 1);
                    osRecvMesg(&pakToGameMessageQ, &msg, 1);
                    if (msg != NULL || D_8039C541) {
                        D_802E8BF8 = 1;
                    } else {
                        D_802E8BF8 = 0;
                    }
                    D_8039C541 = 0;
                    if (!D_802E8BF8) {
                        func_801E8C40(4);
                        gGameModeAfterNext = 0x10000000;
                    } else {
                        playerNumber = 0;
                        D_80364AE9 = 0;
                        playerNumberAtStart = 0, osSendMesg(&D_80219EF8, (OSMesg) 0x01000010, 1);
                        osRecvMesg(&pakToGameMessageQ, &D_8039C4B4, 1);
                        if (D_8039C4B4 == NULL) {
                            debug_printf("NO EE PRESENT! - USING DUMMY EE\n");
                        }
                        osSendMesg(&D_80219EF8, (OSMesg) 0x01000006, 1);
                        osRecvMesg(&pakToGameMessageQ, &msg, 1);
                        if (msg == NULL) {
                            msg = (void *) N64_IPTR(func_80201E80());
                        }
                        if (msg != NULL) {
                            func_801EA108(playerNumber, 1, 1);
                            gGameModeAfterNext = 0x40000;
                        } else {
                            D_80365060[playerNumber] = 1;
                            gGameModeAfterNext = 0x4000;
                        }
                    }
                    break;
                case 0x4000000000000:
                    hd_mode_setup();
                    func_802A0700();
                    func_8025D184();
                    func_80200714(4);
                    func_801E8C40(4);
                    func_8026AF6C(0x8038);
                    break;
                case 0x400000000000000:
                    D_802E8BEC = -1;
                    D_802E8BF0 = 1;
                    D_80365065 = 0;
                    gGameModeAfterNext = 2;
                    D_8039C541 = 0;
                    func_801ECE9C();
                    break;
                case 2:
                    switch (D_802E8BEC) {
                        case 1:
                        case 3:
                        case 4:
                        case 6:
                            if (gGameMode == 2) {
                                gNextGameMode = 0x1000000000000;
                                hd_mode_setup();
                                func_80200714(7);
                                func_80201240(D_80364AC4 & 3);
                                func_8026AF6C(0x8035);
                                func_801E8C40(D_80364AC4 & 3);
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
                        D_8039CAA0 = 0;
                        hd_mode_setup();
                        func_8025D184();
                        func_80295E50();
                        func_8025B9D0(D_802E8BEC, &levelno);
                        if (D_802E8BEC == 0) {
                            func_8029A130();
                        }
                        func_80256A34(0);
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
                    func_801ECE9C();
                    if (!D_80365065) {
                        D_80365065 = 1;
                        D_80364A87 = 0;
                        D_803643D5 = 0;
                        func_801FE018(8);
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
                    func_801ECC8C();
                    osViBlack(1);
                    func_802A0700();
                    func_801F8530(levelno);
                    break;
                case 0x800000000000:
                    D_80364A70 = func_80261A44(gNextGameMode);
                    break;
                case 0x400000000:
                    func_8026AF6C(0x8013);
                    break;
                case 0x800000000000000:
                    hd_mode_setup();
                    func_80200714(1);
                    func_8025D184();
                    func_8026AF6C(0x8059);
                    break;
                case 0x40000000000:
                    hd_mode_setup();
                    func_80200714(3);
                    func_801E8C40(playerNumber);
                    func_8025D184();
                    D_8021A830 = gGameMode;
                    func_8026AF6C(0x8016);
                    break;
                case 0x200000000000000:
                    func_801E8EB8(playerNumber, 1);
                    func_801F8228();
                    func_8026AF6C(0x8016);
                    gNextGameMode = 0x40000000000;
                    break;
                case 0x2000:
                    hd_mode_setup();
                    if (D_802E8F94[levelno].unk0 != 1) {
                        func_80256A34(0);
                        func_80285A78((u8 *) &nss, (u8 *) &D_8036EA60);
                        func_80285A78((u8 *) &nss, (u8 *) &D_8036EA80);
                        gPauseRequest = 1;
                    } else {
                        if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
                            func_80256A34((s32) pakBuffer);
                        } else {
                            func_80256A34(0);
                        }
                        func_80285A78((u8 *) &nss, (u8 *) &D_8036EA60);
                        func_80285A78((u8 *) &nss, (u8 *) &D_8036EA80);
                        gGameModeAfterNext = 4;
                    }
                    break;
                case 4:
                    switch (gGameMode) {
                        case 0x1000000000:
                            func_8026101C();
                            func_80261E9C(gNextGameMode);
                            gUnpauseRequest = 1;
                            break;
                        case 0x100:
                            gCamSnap = 1;
                            break;
                        case 0x2000:
                            if (D_802E8F94[levelno].unk0 != 1) {
                                D_80364A70 = func_80261A44(gNextGameMode);
                            }
                            gUnpauseRequest = 1;
                            gUnpauseRequest &= !func_8026AD30(0x52);
                            /* one line: the line break changes as1's schedule */
                            gUnpauseRequest &= !func_8026AD30(0x56); if ((((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0)) &&
                                D_802E8F94[levelno].unk0 == 1) {
                                gUnpauseRequest &= !func_8026AD30(0x55);
                            } else {
                                gUnpauseRequest &= !func_8026AD30(0x57);
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
                            D_80364A70 = func_80261A44(gNextGameMode);
                        case 0x4000:
                            newPlayer = func_80285814();
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (playerNumber != D_80364AE9) {
                                D_80364AE9 = playerNumber, debug_printf("switching to new player ...........\n");
                                func_80285814();
                            }
                            break;
                    }
                    hd_mode_setup();
                    if (newPlayer) {
                        osSendMesg(&D_80219EF8, (OSMesg) ((levelno << 8) | 0xD | (playerNumber << 16)), 1);
                    }
                    MODE_ENTRY(D_80364AA8).unk0 &= ~1;
                    MODE_ENTRY(D_80364AA8).unk0 |= 0x800;
                    func_8026B8F8();
                    func_8026AF6C(D_802F4868[lowest_set_bit(D_80364AA8)] | 0x8000);
                    MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 3;
                    MODE_PAGE(D_80364AA8).unk8 &= ~8;
                    func_80285A78((u8 *) &nss, (u8 *) &D_8036EA60);
                    D_80315438 = func_801EE800(&D_80364A60, 1, 0), func_801E8C40(playerNumber);
                    func_801EC30C(D_80315438);
                    func_801EC288(D_80315438);
                    func_80200714(1);
                    D_80364A71 = func_801EF1E0();
                    if (D_80364A71 != -1) {
                        func_801F55D8();
                    }
                    break;
                case 0x800:
                    hd_mode_setup();
                    if (gGameMode == 0x4000) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    func_80256A34(0);
                    func_802661EC();
                    if (levelno == 0x32) {
                        func_8026AF6C(0x8024);
                    }
                    break;
                case 0x40000000:
                    hd_mode_setup();
                    start = D_803156C4;
                    while ((u32) (D_803156C4 - start) < 15) {
                        PORT_SPIN();
                    }
                    func_801ED790();
                    func_80200714(3);
                    D_80364A71 = func_801EF1E0();
                    if (D_80364A71 != -1) {
                        func_801F55D8();
                    }
                    func_801E8C40(playerNumber);
                    func_801EC30C(D_80315438);
                    break;
                case 0x8000000:
                    switch (gGameMode) {
                        case 0x40:
                        case 0x400:
                        case 0x20000000:
                            hd_mode_setup();
                            func_80200714(1);
                            D_80364A71 = func_801EF1E0();
                            if (D_80364A71 != -1) {
                                func_801F55D8();
                            }
                            func_801E8C40(playerNumber);
                            func_80285A78((u8 *) &D_8036EA90, (u8 *) &nss);
                            D_80315438 = func_801EE800(&D_80364A60, 1, 0);
                            func_801EC30C(D_80315438);
                            func_801EC288(D_80315438);
                            break;
                        case 0x40000000:
                            D_80364A70 = func_80261A44(gNextGameMode);
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (playerNumber != D_80364AE9) {
                                debug_printf("switching to new player ...........\n");
                                D_80364AE9 = playerNumber;
                                D_803643D5 = 0;
                                func_80285814();
                            }
                            hd_mode_setup();
                            D_80364A71 = func_801EF1E0();
                            if (D_80364A71 != -1) {
                                func_801F55D8();
                            }
                            func_801E8C40(playerNumber);
                            D_80315438 = func_801EE800(&D_80364A60, 1, 0);
                            func_80200714(1);
                            func_801EC30C(D_80315438);
                            func_801EC288(D_80315438);
                            break;
                        default:
                            MS_ASSERT(!saveIt[playerNumber] || saveIt[playerNumber]==levelno+1, 488);
                            saveIt[playerNumber] = levelno + 1;
                            if (D_803643D7) {
                                func_802C1DD0(D_802E8F94[levelno].unk0 == 0x20 || D_802E8F94[levelno].unk0 == 0x80);
                            }
                            D_803643D5 = !((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0);
                            if (playerNumber == playerNumberAtStart) {
                                D_80364A87 |= D_803643D5;
                            }
                            if (D_80364AA8 == 1 && !D_803643D5) {
                                for (i = 0; i < 0x40; i++) {
                                    D_8039C4F8[i] = pakBuffer[i];
                                }
                            }
                            func_802CF5B0();
                            func_80297960();
                            if (D_80364AA8 == 1) {
                                MS_ASSERT(record_status(pakBuffer)<=LEVEL_SAVE_SIZE-4, 513);
                                MS_ASSERT(!saveLevel || saveLevel==levelno+1, 515);
                                saveLevel = levelno + 1;
                                if ((u32) (func_8028604C(D_80364A5C) + nss.tc) >= 60000) {
                                    nss.tc = 59999;
                                } else {
                                    nss.tc = func_8028604C(D_80364A5C) + nss.tc;
                                }
                            } else if (D_803643D7) {
                                func_80264AEC();
                                nss.tc = D_802E8F94[levelno].unk36 - D_80367BF6;
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
                                func_80285A78((u8 *) &nss, (u8 *) &D_8036EA60);
                            } else {
                                func_80285A78((u8 *) &D_8036EA80, (u8 *) &D_8036EA60);
                            }
                            D_80315438 = func_801EE800(&D_80364A60, 1, 1), func_80285A78((u8 *) &nss, (u8 *) &D_8036EA90);
                            if (D_80364A60) {
                                gGameModeAfterNext = 0x40000000;
                                D_80364A64 = 0xDC;
                                D_80364A70 = 0;
                            } else {
                                func_801EC30C(D_80315438);
                                func_80200714(1);
                                D_80364A71 = func_801EF1E0();
                                if (D_80364A71 != -1) {
                                    func_801F55D8();
                                }
                                func_801E8C40(playerNumber);
                            }
                            MODE_ENTRY(D_80364AA8).unk0 |= 1;
                            MODE_ENTRY(D_80364AA8).unk0 &= ~0x800;
                            break;
                    }
                    if (D_803643D5 && playerNumber == playerNumberAtStart) {
                        MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 1;
                    } else {
                        MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 3;
                    }
                    MODE_PAGE(D_80364AA8).unk8 |= 8;
                    func_8026B8F8();
                    if (D_802E8BF8) {
                        D_8020C070[29].unk0 &= ~1;
                    }
                    func_8026AF6C(D_802F4868[lowest_set_bit(D_80364AA8)] | 0x8000);
                    break;
                case 0x40:
                    if (gGameMode & 0x04000200) {
                        func_8028B3E0();
                        nss.tc = func_8028604C(D_80364A5C), func_80285A78((u8 *) &nss, (u8 *) &D_8036EA60);
                        func_801EE800(&D_80364A60, 0, 1);
                    }
                    D_802F5804[24].unk0 |= 1;
                    D_802F5804[24].unk0 &= ~0x800;
                    D_802F5804[23].unk0 |= 1;
                    D_802F5804[23].unk0 &= ~0x800;
                    D_802F5804[17].unk0 &= ~1;
                    D_802F5804[17].unk0 |= 0x800;
                    if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
                        D_802F8BDC[6].unk18 = 0x18, D_802F8BDC[7].unk18 = 0x22;
                    } else {
                        D_802F8BDC[6].unk18 = 0x17, D_802F8BDC[7].unk18 = 0x21;
                    }
                    D_80364A50 = 0;
                    D_80364A54 = PORT_GVI("hd_main_thread:4A54", D_803156C4);
                    hd_mode_setup();
                    if (D_802E8F94[levelno].unk0 == 1 &&
                        ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) &&
                        !D_803643D5) {
                        func_80256A34((s32) D_8039C4F8);
                    } else {
                        func_80256A34(0);
                    }
                    func_8026B8F8();
                    func_8025BB50();
                    break;
                case 0x20000000:
                    hd_mode_setup();
                    if (gGameMode == 0x4000) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    func_802A0700();
                    if (func_801E7000()) {
                        func_80200714(2);
                    } else {
                        gGameModeAfterNext = 0x2000;
                    }
                    break;
                case 0x40000:
                    osSendMesg(&D_80219EF8, (OSMesg) ((playerNumber << 16) | 0x14 | 0x01000000), 1);
                    osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    func_801EA93C("ENTER NAME!", D_803047A0, 7, 0x1E, (char *) &players[playerNumber]);
                    func_8026AF6C(0x800B);
                    func_8025D184();
                    D_80364A70 = func_80261A44(gNextGameMode);
                    break;
                case 0x4000000000000000:
                    hd_mode_setup();
                    func_80200714(1);
                    D_8020C070[9].unkC = "QUIT GAME!", D_8020C070[9].unk10 = D_803047CC, D_8020C070[9].unk6 = D_8020C070[9].unk8 = 22;
                    func_8026AF6C(0x800C);
                    break;
                case 0x1000000000:
                    func_80260EE0(0x18);
                    func_80260B40(0, 0);
                    func_80297ECC();
                    func_8026AF6C(0x8015);
                    gPauseRequest = 1;
                    break;
                case 0x100000000000:
                    func_80299C0C();
                    D_80364A70 = func_80261A44(gNextGameMode);
                    hd_mode_setup();
                    func_80299C20();
                    if (gGameMode == 0x4000 || levelno == 0x2F) {
                        osRecvMesg(&pakToGameMessageQ, NULL, 1);
                    }
                    osViBlack(1);
                    func_80256A34(0);
                    break;
                case 0x40000000000000:
                    hd_mode_setup();
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000010, 1);
                    osRecvMesg(&pakToGameMessageQ, &D_8039C4B4, 1);
                    if (D_8039C4B4 != NULL) {
                        func_8025D184();
                        func_80200714(1);
                        D_8020C070[9].unkC = "ERASE SAVED GAME!", D_8020C070[9].unk10 = D_803047B4, D_8020C070[9].unk6 = D_8020C070[9].unk8 = 20;
                        func_8026AF6C(0x800C);
                    } else {
                        gGameModeAfterNext = 0x10;
                    }
                    break;
                case 0x100000000000000:
                    D_8020C070[9].unkC = "BECOME GUEST PLAYER:", D_8020C070[9].unk10 = D_803047DC, D_8020C070[9].unk6 = D_8020C070[9].unk8 = 20;
                    func_8026AF6C(0x800C);
                    break;
                case 0x400000:
                    func_801EA6E8();
                    func_8026AF6C(0x800A);
                    break;
                case 8:
                    gPauseRequest = 1;
                    func_80260E80();
                    break;
            }
            gPrevGameMode = gGameMode;
            gGameMode = gNextGameMode;
            gNextGameMode = gGameModeAfterNext;
        } while (gNextGameMode != 0);
        if (D_80364A70) {
            func_80261FB0(D_80364A70);
        }
        while (gNextGameMode == 0) {
            gFrameStartTime = osGetTime();
            func_8025B2B8();
            if (gGameMode & 0x4000) {
                gFrameStartLag = (osGetTime() - gLastRetraceTime) / 7825;
                func_801F8980();
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
                func_801FE990();
                gFrameCpuTime = (osGetTime() - gFrameStartTime) / 7825;
                rsp_wait_task(0x4D2);
            } else if (gGameMode & 0x20000000) {
                func_801E7598();
                rsp_wait_task(0x4D2);
            } else if (gGameMode & 0x30) {
                func_801EF4AC();
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
        if ((gGameMode & 0x104) && (D_803643D6 || D_803643D7)) {
            func_8025BBE8(0, 0, 0);
        }
        HD_ASSERT(!areWeFading(), 627);
        switch (gNextGameMode) {
            case 0x400000000000:
                func_80299E10(1);
                break;
            case 0x200000000000:
                func_80299E10(0);
                break;
            case 0x2000000000000:
                func_80286330();
                break;
        }
        if (playerNumber == playerNumberAtStart && gNextGameMode == 0x4000 && func_8028653C()) {
            func_802860F0();
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

    if (gDebugCheats && (D_80370C28 & 1) && !(D_80370C2A & 1) && (gGameMode & 0x104)) {
        D_803643DA = 1;
        cmo_hit_request = 0;
        if (D_80370C28 & 8) {
            if (D_80364AA8 != 1 || !(LEVEL_DONE(levelno))) {
                D_803643DA = 0;
                cmo_hit_request = 1;
            }
        }
        if ((D_80370C28 & 0x2000) && D_803643DA) {
            D_803F7688 = 1;
            D_8036EA7C = D_8036EB90;
            D_8036EA79 = D_8036EB93;
            D_80365066 = 1;
            func_80285AB0(2);
            func_80285AB0(1);
        }
        if ((LEVEL_DONE(levelno)) && D_80364AA8 == 1 && D_803643DA) {
            gPauseRequest = 1;
            func_80275390(0x08000000);
        }
    }
    nextdma = 0;
    no_palette_dmas = 0;
    D_803A7426 = 0;
    if (gFramesInMode) {
        if (gBufferIndex == 0) {
            if (gFramesInMode >= 6) {
                cull_start(&gDynamicBufs[gBufferIndex].persp, &gDynamicBufs[gBufferIndex].cullView);
            }
        } else {
            i = 0;
            found = 0;
            while (!found) {
                if (D_80364460[i].unk5C == D_80364456) {
                    found = 1;
                } else {
                    i++;
                }
            }
            j = 0;
            found = 0;
            while (!found) {
                if (D_803643C8[j].unk1022 == D_80364456) {
                    found = 1;
                } else {
                    j++;
                }
            }
            func_80258544(D_803643C8[j].unk0, D_803643C8[j].unk1004, D_803643C8[j].unk1010, D_803643C8[j].unk100C,
                          D_803643C8[j].unk1000, (Gfx *) N64_IPTR(((D_80364460 + i)->unk54)), (D_80364460 + i)->unk0, (D_80364460 + i)->unk4);
        }
    }
    i = 0;
    found = 0;
    while (!found) {
        if (D_80364460[i].unk5C == D_80364456) {
            found = 1;
        } else {
            i++;
        }
    }
    if (gBufferIndex == 0) {
        func_80279778(D_803643E0, D_803643E4, D_803643E8, gCamEyeX, gCamEyeY, gCamEyeZ, (void *) N64_IPTR((D_80364460[i].unk58)),
                      D_80364460[i].unk0, D_80364460[i].unk8, D_80364460[i].unk60);
    } else {
        func_80279778(D_803643E0, D_803643E4, D_803643E8, gCamEyeX, gCamEyeY, gCamEyeZ, (void *) N64_IPTR((D_80364460[i].unk58)),
                      D_80364460[i].unk0, D_80364460[i].unk4, D_80364460[i].unk60);
    }
    D_803649D8 = osGetTime();
    func_8028A3E4();
    rsp_send_task((u64 *) (gDynamicBufs[gBufferIndex].dl), gFrameDlLen, 3, 1, 0x4D2, 1);
    func_802A5720();
    gBufferIndex ^= 1;
    switch (gPaused) {
        case 0:
            if (gPauseRequest) {
                gPaused = 1;
                gUnpauseRequest = 0;
                gPauseRequest = 0;
                D_803156F4 = gBufferIndex ^ 1;
                if (gGameMode & 0x104) {
                    if (!D_8036EB99) {
                        func_80260B40(0, 0);
                    }
                    func_80261570(0.5f);
                    if (gGameMode == 4 && (D_803643DB || D_80364AC1) && currentYoshiWindow == 0) {
                        gNextGameMode = 0x100;
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
                if (gGameMode & 0x104) {
                    if (D_80358064) {
                        debug_printf("LOCKING KEYS!\n");
                        D_80370C38 = 1;
                    }
                    func_80261E9C(gGameMode);
                    func_80261570(1.0f);
                    if (gGameMode == 0x100 && (D_803643DB || D_80364AC1)) {
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
    if (D_803643DB && (gGameMode & 0x104)) {
        func_802BA148();
    }
    if (D_803643DC && (gGameMode & 0x1905)) {
        func_802B8794();
    }
    func_802683E0();
    D_803649EE = 0;
    if (D_80364456 == 0 && !(gGameMode & 0x1801)) {
        func_8024AE2C();
    }
    func_8028A470();
    if (gGameMode & 0x104) {
        zoom = 0;
        if ((D_80370C28 & 4) && !(D_80370C28 & 8)) {
            if (gCamZoom < 1.5 && gGameMode == 4) {
                zoom = 2;
                if (!(D_80370C2A & 4)) {
                    func_80260650(D_80367738, 0xDC, NULL);
                }
            } else if (!(D_80370C2A & 4) && gGameMode != 0x100) {
                zoom = 3;
            }
        }
        if (zoom == 0) {
            if ((D_80370C28 & 8) && !(D_80370C28 & 4)) {
                if (gCamZoom > 1.0 && gGameMode == 4 && D_80364A7C == 0) {
                    zoom = 1;
                    if (!(D_80370C2A & 8)) {
                        func_80260650(D_80367738, 0xDD, NULL);
                    }
                } else if (!(D_80370C2A & 8) && gGameMode == 0x100) {
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
                if (D_803643DB || D_80364AC1) {
                    cull_restart(10);
                    gNextGameMode = 0x100;
                    D_80364A78 = 0;
                    func_80260650(D_80367738, 0xDF, NULL);
                }
                break;
            case 4:
                D_80364A7C = 1;
                if (D_803643DB || D_80364AC1) {
                    cull_restart(10);
                    gNextGameMode = 4;
                    func_80260650(D_80367738, 0xDF, NULL);
                }
                break;
        }
    }
    if (gGameMode == 0x100 && gPaused && yoshiState == 2 && D_803643DB && currentYoshiWindow == 0) {
        if (!(D_80370C28 & 0x2000) && !(D_80370C28 & 0x10)) {
            D_80364A80 = 0;
        } else {
            D_80364A80 += 100;
            if (D_80364A80 > 800) {
                D_80364A80 = 800;
            }
            if (D_80370C28 & 0x2000) {
                D_80364A78 += D_80364A80;
            }
            if (D_80370C28 & 0x10) {
                D_80364A78 -= D_80364A80;
            }
        }
        pos = D_803EF6E4 + D_80364A78;
        lo = D_803EF6F8 - 400;
        hi = D_803EF6F8 + D_803EF6F0;
        if (pos > hi) {
            D_80364A78 = hi - D_803EF6E4;
        }
        if (pos < lo) {
            D_80364A78 = lo - D_803EF6E4;
        }
        if (D_80370C28 & 0x2010) {
            D_802F8BDC[0].unk4 = (D_802F8BDC[0].unk4 - 0x18 < -0x100) ? -0x100 : D_802F8BDC[0].unk4 - 0x18;
            D_8036BB34 = (D_8036BB34 * 0.8 > 0.1) ? D_8036BB34 * 0.8 : 0.1;
            D_802F8BDC[0].unk8 &= ~0x20;
        } else {
            D_802F8BDC[0].unk4 = (D_802F8BDC[0].unk4 + 0x20 > 0x20) ? 0x20 : D_802F8BDC[0].unk4 + 0x20;
            D_8036BB34 = (D_8036BB34 / 0.8 < 1.0) ? D_8036BB34 / 0.8 : 1.0;
            if (D_8036BB34 == 1.0) {
                D_802F8BDC[0].unk8 |= 0x20;
            }
        }
    } else {
        D_80364A80 = 0;
        D_802F8BDC[0].unk4 = (D_802F8BDC[0].unk4 + 0x20 > 0x20) ? 0x20 : D_802F8BDC[0].unk4 + 0x20;
        D_8036BB34 = (D_8036BB34 / 0.8 < 1.0) ? D_8036BB34 / 0.8 : 1.0;
        if (D_8036BB34 == 1.0) {
            D_802F8BDC[0].unk8 |= 0x20;
        }
    }
    if (D_80370C22 && !gPaused && D_80364456 && !D_8036443C && (D_80364AA8 == 1 || D_80364AA8 == 0x80)) {
        func_8024B188();
    }
    func_8024B618();
    if ((D_80370C28 & 0x4000) && !(D_80370C2A & 0x4000) && !areWeFading() && gNextGameMode == 0) {
        switch (gGameMode) {
            case 0x100000000000:
                if (levelno != 0x2F && levelno != 0x31 && (levelno != 0x26 || gPrevGameMode == 0x4000)) {
                    func_80260650(D_80367738, 0xDE, NULL);
                    gNextGameMode = 0x4000;
                }
                break;
            case 0x40:
            case 0x400:
                func_80260650(D_80367738, 0xDE, NULL);
                if (LEVEL_DONE(levelno)) {
                    gNextGameMode = 0x08000000;
                } else {
                    gNextGameMode = 0x4000;
                }
                break;
            case 0x800:
            case 0x1000:
                if (levelno != 0x32 || gPrevGameMode == 0x4000) {
                    func_80260650(D_80367738, 0xDE, NULL);
                    gNextGameMode = 0x4000;
                }
                break;
        }
    }
    if (D_803643DC && (!gPaused || (gGameMode & 0x1801))) {
        func_802B899C();
        if (gGameMode & 0x1801) {
            if (!areWeFading()) {
                if (gNextGameMode == 0 || (gNextGameMode & 0x1801)) {
                    func_802AEEC8();
                }
                func_802B8AE4();
            }
        } else {
            func_8026510C();
        }
    }
    if (!gPaused && !D_80364A84) {
        func_8024B7AC();
    }
    if (D_8039CA62 && !gPaused) {
        func_80294F00();
    }
    if (D_803643DB && (D_80364AA8 & 0x81)) {
        if (!(gGameMode & 0x1801)) {
            func_802BA354();
        }
        if ((gGameMode & 0x104) && !func_8026B10C() && !gPaused && !D_803643D7 && !D_803643D6) {
            func_8024A92C(vis = func_802C1B9C());
        }
    }
    if (D_80364AC1 && !gPaused) {
        func_802D291C();
        if (levelno == 0x32 && yoshiState == 1 && gFramesInMode > 50 && (gGameMode & 0x1801) && !areWeFading()) {
            func_80275390(0x2000);
        }
    }
    func_8029DDC8();
    if (!gPaused) {
        D_803ED40C = 0;
        D_803F7806 = func_802C1AA0();
        func_802BC5E0();
        func_8024B5E8();
        func_8028F794(D_80364456);
        func_80291FAC(D_80364456);
        func_802688C4(levelno);
        if (!(gGameMode & 0x1801)) {
            func_8026FEC4();
        }
        func_80281CE4();
        if (gGameMode & 0x104) {
            func_80297804(D_803643E0, D_803643E4, D_803643E8);
        }
#ifdef NON_MATCHING
        /* The asm takes its param (the texture decoder's type-4/5 table) in
         * $fp: 0 here in every level (traced). */
        func_8029E0AC(LEAKED(s8));
#else
        func_8029E0AC();
#endif
        if ((gGameMode & 0x104) && (D_80364AA8 & 1)) {
            func_8024A348();
            func_8024ADD8();
        }
        if (D_80364AA8 != 1 && D_80367C00 && (u32) (((s32) D_803156C0) - D_80364A58) > 90) {
            D_80367C00 = 0;
            func_802794A4();
        }
    }
    if ((D_803643D6 || D_803643D7 || (levelno == 0x32 && D_8036EB98 && !(LEVEL_DONE(0x32)))) && D_80364AA8 == 1 && D_80364A5C == 0) {
        debug_printf("TIME IN LEVEL=%d\n", D_80364A5C = ((s32) D_803156C0) - D_80364A58);
    }
    func_802A5510(D_80358074);
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
    func_802BD1F8((u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x1E0]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x578]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x910]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x112B0]),
                  (s32) &gDynamicBufs[gBufferIndex], (s32) (&gDynamicBufs[gBufferIndex].unk2C0[0x2D]), (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x17070]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x170D8]));
    if (gGameMode == 4) {
        func_80295C70(levelno, D_803643E0, D_803643E8);
    }
    func_802A4CDC((u32 *) N64_IPTR(D_80358030[gBufferIndex]), (u32 *) N64_IPTR(D_80358038[gBufferIndex]), (u32 *) N64_IPTR(D_80358040[gBufferIndex]), (u32 *) N64_IPTR(D_80358048[gBufferIndex]),
                  (u32 *) (&gDynamicBufs[gBufferIndex].unkA3A0[0x140]));
    func_8027E9B8(gBufferIndex);
    if (!gPaused || (D_803643DB && D_803643D6) || D_8036EB99) {
        func_802C0574();
        func_802BCA2C();
    }
    if (!gPaused) {
        func_8028DF14(D_80364456);
        func_802906C0(D_80364456);
        func_80292830();
        func_8028C874(D_80364456);
    }
    if (D_80364AA8 != 1) {
        if (gGameMode & 0x40) {
            func_8026420C();
        }
        if (gGameMode & 0x04002104) {
            func_80262BF4();
        }
    } else if (D_803BE738) {
        if (!(LEVEL_DONE(levelno))) {
            cmo_hit_request = 1;
        } else if (!areWeFading()) {
            D_803643DA = 1;
            func_80260650(D_80367738, 0x2B, NULL);
            if (gGameMode == 0x40) {
                func_80275390(0x40);
            } else {
                func_80275390(0x08000000);
            }
        }
    }
    func_802A64A4();
    func_802886A0();
    if (!gPaused) {
        func_802CF1A4();
    }
    func_80279514(D_803643E0, D_803643E4, D_803643E8, gCamEyeX, gCamEyeY, gCamEyeZ);
    func_80277620(gFramesInMode);
    func_8027D810(levelno);
    func_802C2054();
    if (!gPaused) {
        func_8026A9B4();
    }
    gdl = hd_build_frame_dl(&gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if ((gGameMode & 0x04002444) && D_80364AA8 != 1) {
        gdl = func_802639B4(gdl, (Gfx * N64P *) &gDynamicBufs[gBufferIndex], (u32 *) &gFrameDlLen);
    }
    if (gGameMode & 0x100000000002) {
        if (gGameMode == 2) {
            PORT_ITAG(gdl, PORT_ITAG_NONE, 0); /* the attract mode's bobbing logo: blended */
            gdl = func_8025C878(gdl, (s32) &gDynamicBufs[gBufferIndex], gBufferIndex, &gFrameDlLen);
            PORT_ITAG(gdl, PORT_ITAG_HUD, 0);
        }
        if (D_802E8BF0 || D_80366A12 == 3 || D_802E8BEC >= 9) {
            if ((yoshiState == 1 || yoshiState == 8 || D_802E8BEC >= 9) && (!D_802E8BF0 || gFramesInMode <= D_80366A04)) {
                if (D_8039CAA0 + 20 > 0xFF) {
                    D_8039CAA0 = 0xFF;
                } else {
                    D_8039CAA0 += 20;
                }
            } else if (D_8039CAA0 - 20 < 0) {
                D_8039CAA0 = 0;
            } else {
                D_8039CAA0 -= 20;
            }
        }
        if (D_8039CAA0 && D_802E8BEC && D_8039CAA2) {
            gdl = func_80295EFC((s32) &gDynamicBufs[gBufferIndex], gdl, D_8039CAA0 / 2 - 103,
                                0x55 - ((gGameMode == 0x100000000000) << 5), D_8039CAA0);
        }
        if (D_802E8BEC == 0 && D_80366A12 == 3 && gGameMode == 2) {
            gdl = func_8029A1A8((s32) gDynamicBufs, gdl);
        }
        if (gGameMode == 2 || levelno == 0x2F) {
            func_8025C5D0();
        }
    }
    if (gGameMode & 0x1801) {
        if (gFramesInMode == 150 && levelno != 0x32) {
            func_8026AF6C(0x8040);
        }
        if (!players[playerNumber].unk91 && gFramesInMode == 10) {
            func_8026AF6C(0x803F);
        }
    }
    gdl = func_8024C404(gdl, &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if ((gGameMode & 0x440) && D_803156C4 - D_80364A54 > 160 && !D_80364A50) {
        D_80364A50 = 1;
        if (yoshiState == 1) {
            if (!(LEVEL_DONE(levelno)) || D_80364AA8 == 1) {
                func_8026AF6C(D_802F4870[lowest_set_bit(D_80364AA8)] | 0x8000);
            }
        }
    }
    if (gGameMode == 8 && !areWeFading() && (!func_802D4E10(D_80367734) || D_803156C4 - D_80367740 > 300)) {
        func_80275390(0x08000000);
    }
    if (levelno == 0x31 && yoshiState == 1) {
        gdl = func_8029A518((Gfx * N64P *) &gDynamicBufs[gBufferIndex], gdl);
    }
    func_80259C24(&gdl, (Mtx *) &gDynamicBufs[gBufferIndex]);
    if (gGameMode != 2) {
        gdl = func_80274BF0((s32) &gDynamicBufs[gBufferIndex], gdl);
    }
    if (gGameMode == 0x2000000000000000) {
        if ((D_80370C28 & 0x10) && !(D_80370C2A & 0x10)) {
            if (currentYoshiWindow < 0x6B && yoshiState != 8 && yoshiState != 1) {
                func_8026AF6C((currentYoshiWindow + 1) | 0x8000);
                func_80260650(D_80367738, 0x1D, NULL);
            } else {
                func_80260650(D_80367738, 0xD0, NULL);
            }
        }
        if ((D_80370C28 & 0x2000) && !(D_80370C2A & 0x2000)) {
            if (currentYoshiWindow >= 0x5F && yoshiState != 8 && yoshiState != 1) {
                func_8026AF6C((currentYoshiWindow - 1) | 0x8000);
                func_80260650(D_80367738, 0x1D, NULL);
            } else {
                func_80260650(D_80367738, 0xD0, NULL);
            }
        }
    }
    if ((((D_80370C28 & 0x1000) && !(D_80370C2A & 0x1000)) || ((D_80370C28 & 0x8000) && !(D_80370C2A & 0x8000))) &&
        !areWeFading() && D_8036BB1A == -1 && gNextGameMode == 0 && !cmo_hit_request && !D_803643DA && currentYoshiWindow != 0) {
        if ((func_8026B10C() & 0x8000) && (func_8026B10C() & 1)) {
            func_8026B10C();
        }
        switch (gGameMode) {
            case 1:
            case 0x800:
            case 0x1000:
                if (levelno != 0x32 || gPrevGameMode == 0x4000) {
                    gNextGameMode = 0x2000;
                    func_80260650(D_80367738, 0x1E, NULL);
                }
                break;
            case 0x40:
            case 0x400:
                if ((LEVEL_DONE(levelno)) && D_80364AA8 != 1) {
                    gNextGameMode = 0x08000000;
                    func_80260650(D_80367738, 0x1E, NULL);
                } else if (yoshiState == 1) {
                    func_8026AF6C(D_802F4870[lowest_set_bit(D_80364AA8)] | 0x8000);
                }
                break;
            case 2:
                if (D_80366A18) {
                    func_80260650(D_80367738, 0x1E, NULL);
                    func_80260B40(0, 0);
                    func_80260B40(5, 0);
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
                if ((D_80370C28 & 0x1000) && !gPaused) {
                    if ((LEVEL_DONE(levelno)) && D_80364AA8 == 1) {
                        D_802F5804[2].unk0 |= 1;
                        D_802F5804[2].unk18 = 7;
                    } else {
                        D_802F5804[2].unk0 &= ~1;
                        D_802F5804[2].unk18 = 8;
                    }
                    func_8026AF6C(0x8000);
                }
                break;
        }
    }
    gdl = func_8026BBD0(gdl, (s32) &gDynamicBufs[gBufferIndex], &gFrameDlLen);
    if (D_8036BB16 && !cmo_hit_request && !D_803643DA) {
        func_8024BDA4(&D_8036BB16);
    }
    if ((gGameMode & 0x444) && D_8036BB1A == -1 && D_80367BC8 == 0 &&
        !((currentYoshiWindow != -1) ? D_802F8BDC[currentYoshiWindow].unk8 & 0x01000000 : 0)) {
        alpha = (gGameMode & 0x440) ? 0xB4 : 0xFF;
        if (D_80367BD6 < alpha) {
            D_80367BD6 = (s32) ((alpha < D_80367BD6 + 16) ? alpha : D_80367BD6 + 16);
        } else {
            D_80367BD6 = alpha;
        }
    } else if (D_80367BD6 >= 0) {
        D_80367BD6 = (s32) ((D_80367BD6 - 16 < 0) ? 0 : D_80367BD6 - 16);
    } else {
        D_80367BD6 = 0;
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
        func_802A57AC();
    }
    D_803649E0 = D_803649D8 % 40 - 20;
    D_803649E2 = D_803649D8 / 100 % 40 - 20;
    D_803649E4 = D_803649D8 / 10000 % 40 - 20;
    D_803643D8 = D_803643D6;
    if (!D_803643D7 && !D_803643D6) {
        if (D_803643DA) {
            HD_ASSERT(!cmo_hit_request, 1529);
            D_803643D7 = 1;
        } else if (cmo_hit_request) {
            D_803643D6 = 1;
        }
        D_803643DA = 0;
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
                if (D_80364AA8 == 8) {
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
            if (D_803EF6E4 > 177600) {
                far = 25000;
                near = 20000;
            } else if (D_803EF6E4 > 80000) {
                far = 22000;
                near = 17500;
            } else {
                far = 14000;
                near = 8000;
            }
            break;
        case 9:
            if (D_803EF6E4 > 100320) {
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
        if (D_803153F0 == 0 && func_8026A610(D_803EF6DC, D_803EF6E4, D_803643E0, D_803643E8) > 20000) {
            func_80277EDC(1, 1, 7, 0x6C);
            debug_printf("TOO FAR AWAY FROM CMO\n");
        }
        switch (D_80364A6F) {
            case 1:
                switch (D_803153F0) {
                    case 0:
                        func_80260DFC();
                    case 2:
                        func_802608C8((void *) N64_IPTR(D_803156E8));
                        break;
                }
                break;
            case 2:
                switch (D_803153F0) {
                    case 0:
                        func_80260DFC();
                    case 1:
                        func_80260650(D_80367738, 0x26, &D_803156E8);
                        break;
                }
                break;
            case 0:
                func_802608C8((void *) N64_IPTR(D_803156E8));
                if (D_802E8F94[levelno].unk0 != 0x80 || !func_802C1AA0()) {
                    debug_printf("popTuneImmediate();\n");
                    func_80261040();
                } else {
                    debug_printf("WILL THIS FIX IT!!?\n");
                }
                if (D_803153F0 == 2) {
                    func_80260650(D_80367738, 0x68, NULL);
                }
                break;
        }
    }
    switch (D_80364A6F) {
        case 1:
            if (!func_8026AD30(0x49) && currentYoshiWindow != 3) {
                func_8026AF6C(0x8003);
            }
            break;
        case 2:
            if (currentYoshiWindow != 2) {
                func_8026AF6C(0x8002);
            }
            break;
        case 0:
            if ((yoshiState == 2 || yoshiState == 4) && !func_8026B10C() &&
                (currentYoshiWindow == 3 || currentYoshiWindow == 2 || currentYoshiWindow == 0x49)) {
                func_8026AF6C(0x4000);
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

/* Applies a pending vehicle switch (D_803649ED) */
void func_8024AE2C(void) {
    s32 pad;

    if (D_803649ED != 0 && D_803649ED != 0xFF) {
        if (func_802AB878(D_803649ED) == 0) {
            func_802AB478(D_803649ED);
            func_8028F6B4(D_803649ED);
            func_80291ED8(D_803649ED);
            func_8028B720();
            func_802794E4();
            if (func_8024AFA8(D_803649ED)) {
                D_80364456 = D_803649ED;
                func_802AE860();
                if (!gPaused && gFramesInMode >= 11 &&
                    (D_80364456 == 8 || D_80364456 == 15 || D_80364456 == 13 || D_80364456 == 14)) {
                    func_8026AD30(0x4A);
                }
            }
            players[playerNumber].unk10 |= 1 << D_80364456;
            D_803649E8 = 1;
            D_803649EC = 1;
            D_803649EE = 1;
        } else if (!func_80260634((void *) N64_IPTR(D_803156F0))) {
            func_80260650(D_80367738, 0x2B, &D_803156F0);
        }
    }
}

/* Vehicle enter handler; returns whether the id is a vehicle */
s32 func_8024AFA8(s32 id) {
    u8 ok = 1;

    switch (id) {
        case 1:
            func_802AFFD4();
            break;
        case 2:
            func_802B1228();
            break;
        case 3:
            func_802B2D7C();
            break;
        case 4:
            func_802B448C();
            break;
        case 5:
            func_802B5CD8();
            break;
        case 6:
            func_802BB054();
            break;
        case 7:
#ifdef NON_MATCHING
            func_802BBDC8(LEAKED(a0));
#else
            func_802BBDC8();
#endif
            break;
        case 8:
#ifdef NON_MATCHING
            func_802B76AC(LEAKED(a0));
#else
            func_802B76AC();
#endif
            break;
        case 9:
            func_802C5714();
            break;
        case 10:
            func_802C9F54();
            break;
        case 11:
        case 17:
        case 18:
#ifdef NON_MATCHING
            func_802C8AB0(LEAKED(a0));
#else
            func_802C8AB0();
#endif
            break;
        case 13:
#ifdef NON_MATCHING
            func_802CBA94(LEAKED(a0));
#else
            func_802CBA94();
#endif
            break;
        case 14:
#ifdef NON_MATCHING
            func_802CCC8C(LEAKED(a0));
#else
            func_802CCC8C();
#endif
            break;
        case 15:
#ifdef NON_MATCHING
            func_802CFA0C(LEAKED(a0));
#else
            func_802CFA0C();
#endif
            break;
        case 16:
            func_802D0C68();
            break;
        default:
            ok = 0;
            break;
    }
    if (ok && (gGameMode & 0x104)) {
        func_8025BBE8((players[playerNumber].unkF0 & (1 << id)) ? 0x80 : 0x40, 0, 0);
    }
    if (ok) {
        debug_printf("changing to digger %d\n", id);
    }
    return ok;
}

#if defined(NON_MATCHING) && defined(PORT_HOST)
/* Windows port, byte order: func_8024B188 reads the word table that starts
 * at the u8 EEPROM flag D_802E8BF8 (set at boot), and on the N64 that flag is
 * word 0's most significant byte.  The image keeps word 0 big-endian
 * (port/data/image_overrides.txt), so word 0 is assembled from its bytes;
 * the others are host-order words.  Checked by port/tools/loadref.py (T5). */
s32 port_802E8BF8_word(s32 i) {
    u8 *t = &D_802E8BF8;

    if (i == 0) {
        return (t[0] << 24) | (t[1] << 16) | (t[2] << 8) | t[3];
    }
    return ((s32 *) t)[i];
}
#endif

/* Leaves the current vehicle */
void func_8024B188(void) {
    u8 res = 0;
    u8 ok = 0;

    res = func_8024B4B8();
    if (res == 1) {
#if defined(NON_MATCHING) && defined(PORT_HOST)
        ok = func_802AE888(port_802E8BF8_word(D_80364456));
#else
        ok = func_802AE888(((s32 *) &D_802E8BF8)[D_80364456]);
#endif
    }
    if (ok) {
        D_803649E8 = 0;
        func_802794E4();
        func_8028F93C();
        func_80292084();
        func_8028B720();
        switch (D_80364456) {
            case 4:
                func_802B4658();
                break;
            case 3:
                func_802B2F54();
                break;
            case 5:
                func_802B5F60();
                break;
            case 2:
                func_802B11B8();
                break;
            case 1:
                func_802B0254();
                break;
            case 7:
                func_802BBE2C();
                break;
            case 8:
                func_802B7754();
                break;
            case 9:
                func_802C5688();
                break;
            case 10:
                func_802CA1AC();
                break;
            case 11:
            case 17:
            case 18:
                func_802C8B0C(D_80364456);
                break;
            case 13:
                func_802CBBBC();
                break;
            case 14:
                func_802CCD34();
                break;
            case 6:
                func_802BB1A0();
                if (func_8024B418(4)) {
                    func_802B4658();
                }
                if (func_8024B418(13)) {
                    func_802CBBBC();
                }
                break;
            case 15:
                func_802CFAB4();
                break;
            case 16:
                func_802D0BF8();
                break;
        }
        D_80364456 = 0;
        if (gGameMode & 0x104) {
            func_8025BBE8((players[playerNumber].unkF0 & 1) ? 0x80 : 0x40, 0, 0);
        }
    }
    if (res == 0 || (res != 2 && !ok)) {
        if (!func_80260634((void *) N64_IPTR(D_803156EC))) {
            func_80260650(D_80367738, 0x2B, &D_803156EC);
        }
    }
}

/* Is any entry of D_80364460 (up to D_803649D0) tagged with id? */
s32 func_8024B418(u8 id) {
    s32 i;

    i = 0;
    while (&D_80364460[i] != D_803649D0) {
        if (D_80364460[i].unk5C == id) {
            return 1;
        }
        i++;
    }
    return 0;
}

/* Per-vehicle exit check for the current vehicle (D_80364456). Declared u8
 * but has no return statement: callers read the handler's leftover v0. */
#ifdef NON_MATCHING
u8 func_8024B4B8(void) {
    /* An id without a handler returns whatever v0 held in the original
     * (indeterminate; only vehicle ids reach this). */
    s32 ret = 0;

    switch (D_80364456) {
        case 4:
            ret = func_802B45FC();
            break;
        case 3:
            ret = func_802B2EF8();
            break;
        case 5:
            ret = func_802B5F04();
            break;
        case 2:
            ret = func_802B1150();
            break;
        case 1:
            ret = func_802B01DC();
            break;
        case 6:
            ret = func_802BB170();
            break;
        case 7:
            ret = func_802BBE10();
            break;
        case 8:
            ret = func_802B76F8();
            break;
        case 9:
            ret = func_802C5508();
            break;
        case 10:
            ret = func_802CA140();
            break;
        case 11:
        case 17:
        case 18:
            ret = func_802C8AF0();
            break;
        case 13:
            ret = func_802CBB60();
            break;
        case 14:
            ret = func_802CCCD8();
            break;
        case 15:
            ret = func_802CFA58();
            break;
        case 16:
            ret = func_802D0B90();
            break;
    }
    return ret;
}
#else
u8 func_8024B4B8(void) {
    switch (D_80364456) {
        case 4:
            func_802B45FC();
            break;
        case 3:
            func_802B2EF8();
            break;
        case 5:
            func_802B5F04();
            break;
        case 2:
            func_802B1150();
            break;
        case 1:
            func_802B01DC();
            break;
        case 6:
            func_802BB170();
            break;
        case 7:
            func_802BBE10();
            break;
        case 8:
            func_802B76F8();
            break;
        case 9:
            func_802C5508();
            break;
        case 10:
            func_802CA140();
            break;
        case 11:
        case 17:
        case 18:
            func_802C8AF0();
            break;
        case 13:
            func_802CBB60();
            break;
        case 14:
            func_802CCCD8();
            break;
        case 15:
            func_802CFA58();
            break;
        case 16:
            func_802D0B90();
            break;
    }
}
#endif


void func_8024B5E8(void) {
    D_803ED3F5 = 1;
    func_802AB670(D_80364456);
}

#ifdef NON_MATCHING
/* Leaked-register inputs of the hand-asm vehicle functions called below (and
 * func_8029E0AC in hd_game_frame), which this file declares `void (void)`
 * (traced in the original ROM with mupen64plus breakpoints over attract demos
 * 0-8, nine levels; see the port notes, trace_values.md):
 *  - $fp ($s8): 0 at func_8024B618, func_8024B7AC and func_8029E0AC in every
 *    level (hd_game_frame and the IDO code above it never set $s8, and the
 *    hand asm they call before these points restores it). The re-ground
 *    functions of func_8024B618 save and restore fp themselves, so every one
 *    of them gets 0. It reaches state: func_802A9A60 stores it into
 *    D_803ED3F2[0..2] and veh+0x50 = their average (the ground byte), and
 *    func_8029E0AC hands it to the texture decoder as the type-4/5 table.
 *  - func_802CFB00's a3 (the previous loop iteration's leftover; only read by
 *    func_802D05D8 for a zero-matrix point) and its FP pass-through state
 *    f12-f26 (only FP side results of the triangle scans): 0. Not reached in
 *    the demos.
 *  - the per-frame updates' zone registers t6, t7, s0-s4 (and f14-f26 for
 *    func_802B152C / func_802CFDE8 / func_802D0F98). Measured: t6 = this
 *    switch's case label (the jump-table target, e.g. 0x8024B7F8), t7 = the
 *    level's leftover small integer (2..7, also in t8), s0 = 0, s1 = 1 / 0x30
 *    / 0x3A, s2/s3/s4 = 0x803B8D40 / 0x803B8578 / 0x803B8570. They only pass
 *    through func_802ABD54 (unchanged when the level has no zones, as in 8 of
 *    the 9 demo levels) into the type-1 func_802A6274 effect records, which
 *    never store them (and the FP state only feeds FP side results): dead.
 *    0, as the other updates (func_802AEEC8, func_802B6294, ...) use. */
#endif

/* Moves every placed vehicle other than the current one */
void func_8024B618(void) {
    s32 i;
    s32 id;

    i = 0;
    while (&D_80364460[i] != D_803649D0) {
        if (D_80364460[i].unk70 != 0) {
            id = D_80364460[i].unk5C;
            if (id != D_80364456 && id != 0xFF && id != 0xFE && id != 0) {
                switch (id) {
                    case 5:
                        func_802B5FAC();
                        break;
                    case 1:
#ifdef NON_MATCHING
                        func_802B02A0(LEAKED(fp));
#else
                        func_802B02A0();
#endif
                        break;
                    case 4:
#ifdef NON_MATCHING
                        func_802B46C4(LEAKED(fp));
#else
                        func_802B46C4();
#endif
                        break;
                    case 8:
                        func_802B77A0();
                        break;
                    case 13:
                        func_802CBC08();
                        break;
                    case 14:
                        func_802CCD80();
                        break;
                    case 15:
#ifdef NON_MATCHING
                        func_802CFB00(LEAKED(fp), LEAKED(a3), LEAKED(f12), LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24),
                                      LEAKED(f26));
#else
                        func_802CFB00();
#endif
                        break;
                    case 9:
#ifdef NON_MATCHING
                        func_802C5860(LEAKED(fp));
#else
                        func_802C5860();
#endif
                        break;
                    case 3:
#ifdef NON_MATCHING
                        func_802B2FA0(LEAKED(fp));
#else
                        func_802B2FA0();
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

/* Vehicle exit handler for the current vehicle */
void func_8024B7AC(void) {
    D_803ED3F5 = 0;
    switch (D_80364456) {
        case 0:
            D_803649ED = 0;
            func_802AEEC8();
            break;
        case 1:
#ifdef NON_MATCHING
            func_802B03F4(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            func_802B03F4();
#endif
            break;
        case 2:
#ifdef NON_MATCHING
            func_802B152C(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            func_802B152C();
#endif
            break;
        case 3:
#ifdef NON_MATCHING
            func_802B327C(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            func_802B327C();
#endif
            break;
        case 4:
#ifdef NON_MATCHING
            func_802B49AC(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4));
#else
            func_802B49AC();
#endif
            break;
        case 5:
            func_802B6294();
            break;
        case 6:
            func_802BB274();
            break;
        case 7:
            func_802BBEB8();
            break;
        case 8:
            func_802B7A88();
            break;
        case 9:
            func_802C5AFC();
            break;
        case 10:
            func_802CA4E0();
            break;
        case 11:
        case 17:
        case 18:
            func_802C8BB8(D_80364456);
            break;
        case 13:
            func_802CBEF0();
            break;
        case 14:
            func_802CD068();
            break;
        case 15:
#ifdef NON_MATCHING
            func_802CFDE8(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            func_802CFDE8();
#endif
            break;
        case 16:
#ifdef NON_MATCHING
            func_802D0F98(LEAKED(t6), LEAKED(t7), LEAKED(s0), LEAKED(s1), LEAKED(s2), LEAKED(s3), LEAKED(s4),
                          LEAKED(f14), LEAKED(f20), LEAKED(f22), LEAKED(f24), LEAKED(f26));
#else
            func_802D0F98();
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
    cull_step_delayed((s32) D_80358074, dl, v, (s32) (gdl - dl) * sizeof(Gfx));
}

/* Handles a Yoshi (menu) selection *sel for the current game mode */
void func_8024BDA4(u16 *sel) {
    switch (gGameMode) {
        case 4:
        case 0x100:
            switch (*sel) {
                case 1:
                    if (D_80364456 == 6 || D_80364456 == 11 || D_80364456 == 17 || D_80364456 == 18) {
                        D_802F5804[7].unk0 &= ~1;
                        D_802F5804[7].unk18 = 8;
                    } else {
                        D_802F5804[7].unk0 |= 1;
                        D_802F5804[7].unk18 = 7;
                    }
                    func_8026AF6C(0x8001);
                    gNextGameMode = 4;
                    break;
                case 2:
                    func_80285EF4(D_80364A58);
                    break;
                case 7:
                    func_8028B240();
                    break;
                case 3:
                    if (D_802E8F94[levelno].unk0 & 0x81) {
                        func_80275270(0x2000, 0.5f);
                    } else {
                        func_80275270(0x20000000, 0.5f);
                    }
                    break;
                case 8:
                    gNextGameMode = 0x2000000000000000;
                    func_8026AF6C(0x805E);
                    gCamSnap = 1;
                    break;
                case 4:
                    func_80275270(0x4000, 0.5f);
                    break;
                case 9:
                    if (func_80260DF0() == 1.0) {
                        D_802F8BDC[13].unk18 = 40;
                    } else {
                        D_802F8BDC[13].unk18 = 39;
                    }
                    func_8026AF6C(0x800D);
                    break;
                case 0x1A7:
                case 0x1A8:
                    if (((*sel - 0x1A7) << D_80364456) ^ ((1 << D_80364456) & 0x10205)) {
                        debug_printf("selected controller mode yes\n");
                        players[playerNumber].unkF0 |= 1 << D_80364456;
                        func_8025BBE8(0x80, 0, 0);
                    } else {
                        debug_printf("selected controller mode no\n");
                        players[playerNumber].unkF0 &= ~(1 << D_80364456);
                        func_8025BBE8(0x40, 0, 0);
                    }
                    break;
                case 17:
                    D_802F8BDC[6].unk8 |= 0x80;
                    break;
                case 0xFFFF:
                    switch (currentYoshiWindow) {
                        case 1:
                        case 6:
                            debug_printf("TESTING PAUSE2 %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                            func_8026AF6C(0x8000);
                            if (D_803643DB || D_80364AC1) {
                                gNextGameMode = 0x100;
                                cull_restart(10);
                            }
                            break;
                        case 13:
                        case 0x58:
                            debug_printf("TESTING PAUSE3 %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                            func_8026AF6C(0x8001);
                            break;
                    }
                    break;
                case 40:
                    func_80260D7C(1.0f);
                    break;
                case 39:
                    func_80260D7C(0.7f);
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
                    gNextGameMode = 0x08000000;
                } else {
                    gNextGameMode = 0x2000;
                }
            } else {
                if (*sel == 0x18 || *sel == 0x22 || *sel == 0xFFFF) {
                    gNextGameMode = 0x4000;
                } else {
                    gNextGameMode = 0x2000;
                }
            }
            break;
        case 0x2000000000000000:
            if (*sel == 0xFFFF) {
                debug_printf("TESTING PAUSE %d %d %d\n", gPaused, gPauseRequest, gUnpauseRequest);
                func_8026AF6C(0x8001);
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
    gdl = func_80271FD0(gdl, (u32) dyn, levelno, gCamYaw, gCamPitch, &height);
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
    func_8027F1F8(&gdl, gBufferIndex, 0);
    gSPSegment(gdl++, 8, PHYS(D_80364458));
    gSPClearGeometryMode(gdl++, -1);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(D_80358030[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E0);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(D_80358038[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E4);
    if (!D_803643D6 && !D_803643D7) {
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
        func_8027C4C8(&gdl, (struct TrailBuf *) dyn);
    }
    func_802502EC();
    func_8024E4F4(&gdl, dyn, 0);
    func_80258B78(&gdl, (struct VtxBuf *) dyn);
    if (!gPaused && D_80364AA8 != 0x40 && !(gGameMode & 0x440) && !D_80364A84) {
        func_80279EE8(&gdl, (s32) dyn, gBufferIndex);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPDisplayList(gdl++, SEG2(0x0A580));
    gDPPipeSync(gdl++);
    gdl = func_802CEEFC(gdl, gBufferIndex, (Gfx *) dyn->unkA3A0, &dyn->unk2C0[0x3C]);
    func_8024E4F4(&gdl, dyn, 1);
    if (D_803643DC) {
        func_8024F520(&gdl, dyn);
    }
    func_802701A8(&gdl, (s32) dyn);
    func_80281E44(&gdl);
    func_8028E9E4(&gdl, (struct Dyn48D00 *) dyn);
    func_802917B0(&gdl, (struct Dyn4B5E0 *) dyn);
    func_80292EB8(&gdl, (struct Dyn *) dyn);
    func_8028CB30(&gdl, (s32) dyn);
    switch (gBufferIndex) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C5770));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C6370));
            break;
    }
    func_80288DF0(&gdl, gBufferIndex);
    if (!(gGameMode & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 0);
    }
    func_802976E8(&gdl);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(D_80358040[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6E8);
    if (!(gGameMode & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 1);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) N64_IPTR(D_80358048[gBufferIndex])));
    gSPDisplayList(gdl++, D_803BE6EC);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPDisplayList(gdl++, SEG2(0x0A918));
    gDPPipeSync(gdl++);
    if (!D_803643D6 && !D_803643D7) {
        gSPClearGeometryMode(gdl++, -1);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
        gSPDisplayList(gdl++, SEG2(0x21478));
        gDPPipeSync(gdl++);
    }
    if (!(gGameMode & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 2);
    }
    func_80295120(&gdl, (Mtx *) dyn);
    if (gGameMode & 0x2000001000003905) {
        func_80280F34(&gdl, gBufferIndex);
    }
    func_8027F1F8(&gdl, gBufferIndex, 1);
    func_8024E4F4(&gdl, dyn, 2);
    if (D_803643DC) {
        func_80266248(&gdl, (struct DynBuf *) dyn);
    }
    switch (gBufferIndex) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C6F70));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C7B70));
            break;
    }
    if (D_803643DB) {
        func_8028273C(&gdl, gBufferIndex);
    }
    PORT_ITAG(gdl, PORT_ITAG_HUD, 0);
    if (D_80364A68 && (gGameMode & 0x104)) {
        func_80286C60(&gdl, (s32) dyn, gBufferIndex, D_80364456);
    }
    if (gGameMode & 0x200000000400220C) {
        func_80278324(&gdl, (s32) dyn, gBufferIndex);
    }
    if (gGameMode == 0x100 && D_803643DB) {
        func_80276E50(&gdl, dyn, gBufferIndex, D_803643E0, D_803643E4, D_803643E8);
    }
    func_80259450();
    if (D_80364A6A && (gGameMode & 0x104)) {
        func_80287530(&gdl, (Gfx * N64P *) dyn, gBufferIndex, D_80364456);
    }
    if (D_80364A6C && (gGameMode & 0x104)) {
        func_80287C68(&gdl, (Gfx * N64P *) dyn, gBufferIndex, D_80364456);
    }
    if (gGameMode & 0x104) {
        func_80282224(&gdl, D_80364456);
    }
    if ((D_80364AA8 & 1) && (gGameMode & 0x0400030C)) {
        func_8026A378(D_803649F4, (u8 *) &money[1]);
        money[0] = '$';
        func_80259CCC((Gfx * N64P *) dyn, (u8 *) money, 0, 1, 0, 0x118, 0x12, 0x14, 0x14, 0, 0xFF, 0xFF, 0xFF, D_80367BD6);
        if (D_80364A44 && !gPaused) {
            func_8026A378(D_80364A44, (u8 *) &bonus[1]);
            bonus[0] = '$';
            func_80259CCC((Gfx * N64P *) dyn, (u8 *) bonus, 0, 1, 0, D_80364A4A, D_80364A4C, 0x23, 0x23, D_80364A4E, 0xFF, 0xFF, 0xFF,
                          D_80364A48);
        }
    }
    if ((gGameMode & 0x2000000000000104) && (!(D_80364AA8 & 0x81) || D_803643DB || D_80364AC1)) {
        func_80275478((struct SpriteVtxBuf *) dyn, &gdl, (gGameMode & 0x100) || currentYoshiWindow == 0x4D || currentYoshiWindow == 0x49);
    }
    if (gNextGameMode == 0 && !areWeFading()) {
        if (!(gGameMode & 0x200000100400230C) && PORT_GVI("hd_build_frame_dl", D_803156C4) % 50 * 60 / 60 >= 21 && yoshiState == 1 &&
            (!(gGameMode & 2) || (D_802E8BEC && (D_80366A12 == 3 || D_802E8BEC == 1))) &&
            (gGameMode != 0x100000000000 || D_803A6B04) &&
            (!(gGameMode & 0x1801) || players[playerNumber].unk91) && levelno != 0x2F) {
            func_80259CCC((Gfx * N64P *) dyn, (u8 *) ("PRESS START"), 0, 1, 0, 0x5C, 0xC4, 0x1A, 0x1A, 1, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (PORT_GVI("hd_build_frame_dl", D_803156C4) % 40 * 60 / 60 >= 16) {
            if (gPaused) {
                if (gGameMode == 0x2000000000000000 && yoshiState == 2) {
                    func_80259CCC((Gfx * N64P *) dyn, (u8 *) ("USE Z/R TO TURN PAGES"), D_8030491C, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                } else if (gGameMode == 0x100 && currentYoshiWindow == 0 && yoshiState == 2 && D_803643DB &&
                           !(D_80370C28 & 0x2010)) {
                    func_80259CCC((Gfx * N64P *) dyn, (u8 *) ("USE Z/R TO MOVE MAP"), D_80304938, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                }
            } else if (gGameMode == 0x100) {
                if (D_80364AC1) {
                    func_80259CCC((Gfx * N64P *) dyn, (u8 *) ("SHUTTLE VIEW"), D_80304904, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                } else {
                    func_80259CCC((Gfx * N64P *) dyn, (u8 *) ("MISSILE VIEW"), D_80304910, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                }
            }
        }
    }
    if (((gGameMode & 0x440) || levelno == 0x26) && yoshiState == 1 && !areWeFading()) {
        if ((gGameMode & 0x440) && D_80364AA8 != 1) {
            func_80274B40(&gdl, (s32) dyn, D_80365580, 0x108, 0x12);
        } else {
            func_80274B40(&gdl, (s32) dyn, D_80365580, 0x18, 0x12);
        }
    }
    if ((gGameMode & 0x104) && gCamFixedEye) {
        func_80274B40(&gdl, (s32) dyn, D_80364A86, 0x108, 0xBE);
    }
    if ((gGameMode & 0x104) && D_80364AA8 == 1 && !gPaused) {
        func_80285CC0();
    }
    if (D_803643DB && gGameMode == 4) {
        func_80282C80(&gdl, (Mtx *) dyn, D_803643E0, D_803643E4, D_803643E8, D_803EF6DC, D_803EF6E0, D_803EF6E4);
    }
    if ((D_802E8F94[levelno].unk0 & 0x81) && gGameMode == 4) {
        if (levelno != 0x32 ||
            ((players[playerNumber].unk18[0x32] > 0 && players[playerNumber].unk18[0x32] < 6) ? 1 : 0)) {
            func_8028376C(&gdl, (Mtx *) dyn, gBufferIndex, D_803643E0, D_803643E8, D_803EF6DC, D_803EF6E4);
        }
    }
    if (D_803643DB || D_80364AC1) {
        switch (gGameMode) {
            case 4:
            case 0x100:
            case 0x200:
            case 0x100000000000:
                func_8025E2CC(&gdl, (s32) dyn, gBufferIndex);
                func_8025E67C(&gdl, (s32) dyn, gBufferIndex);
            case 0x40:
            case 0x400:
                if ((D_803643D6 || D_803643D7 || cmo_hit_request || D_803643DA) && (gGameMode & 0x144)) {
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
    func_80259C24(&gdl, (Mtx *) dyn);
    *len = gdl - dyn->dl;
    return gdl;
}

/* Draws the shadow quads of every vehicle on the given layer */
void func_8024E4F4(Gfx **gfx, DynamicBuf *dyn, u8 layer) {
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
    while (&D_803643C8[i] != D_803643CC) {
        if ((D_803643C8[i].unk1022 != 0 || all) && D_803643C8[i].unk1022 != 0xFE &&
            (D_803643C8[i].unk1022 != 0xFF || !D_803EF6FF) &&
            (D_803643C8[i].unk1022 == 0xFD || !D_80364A84 || !D_80364AC1) && D_803643C8[i].unk1023 == layer) {
            x = D_803643C8[i].unk1004 >> 5;
            y = D_803643C8[i].unk1008 >> 5;
            z = D_803643C8[i].unk100C >> 5;
            w = D_803643C8[i].unk1018;
            d = D_803643C8[i].unk101A;
            spin = D_803643C8[i].unk1022 == D_80364456 && D_803ED40D == 0x65;
            if (spin) {
                guRotateF(tmp, (f32) D_80364440 * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            }
            guRotateF(mf, (f32) -D_803643C8[i].unk101E * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            if (spin) {
                guMtxCatF(tmp, mf, mf);
            }
            guRotateF(tmp, (f32) D_803643C8[i].unk101C * 360.0 / 4096.0, 1.0f, 0.0f, 0.0f);
            guMtxCatF(mf, tmp, tmp);
            guRotateF(mf, (f32) D_803643C8[i].unk1020 * 360.0 / 4096.0, 0.0f, 0.0f, 1.0f);
            guMtxCatF(tmp, mf, mf);
            guRotateF(tmp, (f32) D_803643C8[i].unk101E * 360.0 / 4096.0, 0.0f, 1.0f, 0.0f);
            guMtxCatF(mf, tmp, mf);
            guTranslateF(tmp, x, y, z);
            guMtxCatF(mf, tmp, mf);
            guMtxF2L(mf, &dyn->unk2C0[i]);
            if (spin) {
                gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_802FA940), G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                                    G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                th = 32, tw = 32;
            } else {
                gDPLoadTextureBlock(gdl++, PHYS((u32) D_803643C8 + i * sizeof(Vehicle)), G_IM_FMT_IA, G_IM_SIZ_8b,
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
            if (D_803643D6 && !(D_80364AA8 & 0x81) && D_803643C8[i].unk1022 == D_80364456) {
                gSPMatrix(gdl++, &D_02000000[0x55], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            }
            gSPMatrix(gdl++, &D_02000000[i + 11], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gSPVertex(gdl++, (Vtx *) D_02000000 + D_8036506C + 0x158, 4, 0);
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
            gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            if (D_803643D6 && !(D_80364AA8 & 0x81) && D_803643C8[i].unk1022 == D_80364456) {
                gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            }
            gDPPipeSync(gdl++);
        }
        i++;
    }
    PORT_ITAG(gdl, PORT_ITAG_NONE, 0);
    *gfx = gdl;
}

/* Draws the shadow quad under vehicle 0xFE */
void func_8024F520(Gfx **gfx, DynamicBuf *dyn) {
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
        if (D_803643C8[i].unk1022 == 0xFE) {
            found = 1;
        } else {
            i++;
        }
    }
    gDPLoadTextureBlock(gdl++, PHYS((u32) D_803643C8 + i * sizeof(Vehicle)), G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0, G_TX_CLAMP, G_TX_CLAMP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    x = D_803643C8[i].unk1004 >> 5;
    y = D_803643C8[i].unk1008 >> 5;
    z = D_803643C8[i].unk100C >> 5;
    w = D_803643C8[i].unk1018;
    d = D_803643C8[i].unk101A;
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

/* Emits the display lists of every object in D_80364460 */
void func_8024FC2C(Gfx **gfx, u8 lod) {
    Gfx *gdl = *gfx;
    s32 i;
    u8 unused;
    u8 alt;
    u8 all;

    all = !D_803649E8 && (D_803649EC || (gGameMode & 0x1801));
    i = 0;
    while (&D_80364460[i] != D_803649D0) {
        if ((D_80364460[i].unk5C != 0 || all) && (D_80364460[i].unk5C != 0xFF || !D_803EF6FF) &&
            (D_80364460[i].unk5C == 0xFD || !D_80364A84 || !D_80364AC1)) {
            PORT_ITAG(gdl, PORT_ITAG_OBJ, i | (lod << 8));
            gSPSegment(gdl++, 6, osVirtualToPhysical(D_80364460[i].unk0));
            if ((gGameMode & 0x1801) && (D_80364460[i].unk5C == 0xFE || D_80364460[i].unk5C == 0)) {
                alt = gBufferIndex;
            } else {
                alt = D_803156F5;
            }
            if (alt) {
                gSPSegment(gdl++, 7, osVirtualToPhysical(D_80364460[i].unk4));
            } else {
                gSPSegment(gdl++, 7, osVirtualToPhysical(D_80364460[i].unk8));
            }
            gDPPipeSync(gdl++);
            gDPSetEnvColor(gdl++, 0, 0, 0, D_80364460[i].unk60);
            gSPClearGeometryMode(gdl++, -1);
            if (D_803643D6 && !(D_80364AA8 & 0x81) && D_80364460[i].unk5C == D_80364456) {
                gSPMatrix(gdl++, &D_02000000[0x54], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            }
            if (alt) {
                switch (lod) {
                    case 0:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unkC));
                        break;
                    case 1:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unk10));
                        break;
                    case 2:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unk14));
                        break;
                }
            } else {
                switch (lod) {
                    case 0:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unk30));
                        break;
                    case 1:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unk34));
                        break;
                    case 2:
                        gSPDisplayList(gdl++, osVirtualToPhysical(D_80364460[i].unk38));
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

    if (D_803643D6 != 0 && !(D_80364AA8 & 0x81)) {
        guTranslateF(mf, -(f32) D_803643E0 / 32.0, -(f32) D_803643E4 / 32.0, -(f32) D_803643E8 / 32.0);
        guScaleF(tmp, D_80364ABC, D_80364ABC, D_80364ABC);
        guMtxCatF(mf, tmp, mf);
        guRotateF(tmp, D_80364AB4, 0.0f, 1.0f, 0.0f);
        guMtxCatF(mf, tmp, mf);
        guTranslateF(tmp, (f32) D_803643E0 / 32.0, (f32) D_803643E4 / 32.0, (f32) D_803643E8 / 32.0);
        guMtxCatF(mf, tmp, mf);
        guMtxF2L(mf, &gDynamicBufs[gBufferIndex].unk1500);
        i = 0;
        while (D_803643C8[i].unk1022 != D_80364456) {
            i++;
        }
        x = D_803643C8[i].unk1004 >> 5;
        y = D_803643C8[i].unk1008 >> 5;
        z = D_803643C8[i].unk100C >> 5;
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
            func_802AC61C(D_803643E0, D_803643E4, D_803643E8, 0x13, 400000);
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
extern Node88 *D_8036BED8;
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
extern Box *D_8036C794;
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
    inShuttle = D_80364AC1 && ((gGameMode & 0x940) || D_8036B965);
    if (gGameMode == 8 || (currentYoshiWindow == 0x4B && yoshiState == 2 && (u16) D_802F8BDC[currentYoshiWindow].unk18 == 0x14D)) {
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
    if (currentYoshiWindow == 0x49 && yoshiState == 2 && (u16) D_802F8BDC[currentYoshiWindow].unk18 == 0x130) {
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
    if (currentYoshiWindow == 0x4D && yoshiState == 2 && (u16) D_802F8BDC[currentYoshiWindow].unk18 != 0x164) {
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
    gCamSnap |= inShuttle || gGameMode == 0x100;
    gCamSnap |= gFramesInMode < 2;
    if (D_803643D6 && (gGameMode & 0x100000000600)) {
        rateX = 0.2f;
        rateY = 0.1f;
        rateZ = 0.2f;
    } else if ((yoshiState == 2 && (currentYoshiWindow == 0x4D || currentYoshiWindow == 0x4B || currentYoshiWindow == 0x49)) ||
               (D_803643D7 && (gGameMode & 0x100000000600)) || gGameMode == 8) {
        rateX = 0.08f;
        rateY = 0.03f;
        rateZ = 0.08f;
    } else if (gGameMode & 0x1801) {
        rateX = 0.06f;
        rateY = 0.03f;
        rateZ = 0.03f;
    } else if ((D_80364AA8 == 0x10 || D_80364AA8 == 0x40) && gGameMode == 0x40) {
        rateX = 0.1f;
        rateY = 0.03f;
        rateZ = 0.1f;
    } else if (D_80364456 == 9) {
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
            if (D_80364AC1) {
                gCamType = 0xFD;
            } else {
                gCamType = 0xFF;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (D_80364AC1) {
                gCamType = 0xFD;
            } else {
                gCamType = 0xFE;
            }
            break;
        default:
            gCamType = D_80364456;
            break;
    }
    tilt = D_8036443C;
    if (!D_803649EC) {
        gCamAtHeight = func_802A56C4();
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
    if ((tilt > 0 && D_80370C1C && !gPaused && (gGameMode & 0x106)) ||
        (tilt < 0 && D_80370C1D && !gPaused && (gGameMode & 0x106))) {
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
            if (D_80364AC1) {
                if (gCamSnap) {
                    gCamEasedHeading = D_803FCD68 * 16;
                } else {
                    gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                }
                gCamEyeGoalX = (s32) (D_803FCD48 - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
            } else {
                gCamEyeGoalX = (D_803EF6DC + 4000) << 11;
                gCamEyeGoalY = (D_803EF6E0 + 25000) << 11;
                gCamEyeGoalZ = (D_803EF6E4 + D_80364A78 + 7000) << 11;
            }
            break;
        case 0x40:
            switch (D_80364AA8) {
                case 1:
                case 0x80:
                    if (D_803643DB) {
                        gCamEyeGoalX = (D_803EF6DC + offX) << 11;
                        gCamEyeGoalY = (D_803EF6E0 + 2000) << 11;
                        gCamEyeGoalZ = (D_803EF6E4 + offZ) << 11;
                    }
                    if (D_80364AC1) {
                        if (gFramesInMode < 2) {
                            gCamEasedHeading = D_803FCD68 * 16;
                        } else {
                            gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                        }
                        gCamEyeGoalX = (s32) (D_803FCD48 - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                        gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                        gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
                    }
                    if (D_803643DB || D_80364AC1) {
                        break;
                    }
                case 4:
                case 8:
                    if (!gCamSnap) {
                        gCamEasedHeading = (s16) ((u16) D_8036443E * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                    } else {
                        gCamEasedHeading = D_8036443E * 16;
                    }
                    gCamEyeGoalX = (s32) (D_803643E0 - SINE(gCamEasedHeading) * (gCamDistance * 2.0f) + offX) << 11;
                    gCamEyeGoalY = (gCamHeight + D_803643E4) << 11;
                    gCamEyeGoalZ = (s32) (D_803643E8 - COSINE(gCamEasedHeading) * (gCamDistance * 2.0f) + offZ) << 11;
                    break;
                case 2:
                case 0x20:
                    if (!gCamSnap) {
                        gCamEasedHeading = (s16) ((u16) D_8036443E * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                    } else {
                        gCamEasedHeading = D_8036443E * 16;
                    }
                    gCamEyeGoalX = (s32) (SINE(gCamEasedHeading) * (gCamDistance * 2.0f) + D_803643E0 + offX) << 11;
                    gCamEyeGoalY = (gCamHeight + D_803643E4 + D_80370C2D * 50) << 11;
                    gCamEyeGoalZ = (s32) (COSINE(gCamEasedHeading) * (gCamDistance * 2.0f) + D_803643E8 + offZ) << 11;
                    break;
                case 0x10:
                case 0x40:
                    gCamEyeGoalX = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk0 << 5) + offX) << 11;
                    gCamEyeGoalY = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk2 << 5) + 2000) << 11;
                    gCamEyeGoalZ = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk4 << 5) + offZ) << 11;
                    break;
            }
            break;
        case 0x200:
            if (D_80364AC1) {
                gCamEyeGoalX = (D_803FCD48 + offX - 6000) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xD48) << 11;
                gCamEyeGoalZ = (D_803FCD50 + offZ + 6000) << 11;
            } else if (D_803643D6) {
                gCamEyeGoalX = (D_803EF6DC + offX + 16000) << 11;
                gCamEyeGoalY = (D_803EF6E0 + 15000) << 11;
                gCamEyeGoalZ = (D_803EF6E4 + offZ - 3000) << 11;
            } else {
                gCamEyeGoalX = (D_803EF6DC + offX + 4000) << 11;
                gCamEyeGoalY = (D_803EF6E0 + 2400) << 11;
                if (levelno == 0) {
                    gCamEyeGoalY += 0x5DC000;
                }
                gCamEyeGoalZ = (D_803EF6E4 + offZ - 4000) << 11;
            }
            break;
        case 0x400:
            if (D_80364AC1) {
                gCamEyeGoalX = (D_803FCD48 + offX + 6000) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xD48) << 11;
                gCamEyeGoalZ = (D_803FCD50 + offZ - 6000) << 11;
            } else if (D_803643D6) {
                gCamEyeGoalX = (D_803EF6DC + offX - 16000) << 11;
                gCamEyeGoalY = (D_803EF6E0 + 15000) << 11;
                gCamEyeGoalZ = (D_803EF6E4 - offZ + 3000) << 11;
            } else {
                gCamEyeGoalX = (D_803EF6DC + offX - 4000) << 11;
                gCamEyeGoalY = (D_803EF6E0 + 2400) << 11;
                if (levelno == 0) {
                    gCamEyeGoalY += 0x5DC000;
                }
                gCamEyeGoalZ = (D_803EF6E4 + offZ + 4000) << 11;
            }
            break;
        case 1:
        case 0x800:
            if (D_80364AC1) {
                if (gFramesInMode < 2) {
                    gCamEasedHeading = D_803FCD68 * 16;
                } else {
                    gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
                }
                gCamEyeGoalX = (s32) (D_803FCD48 - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
                gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
                gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
            } else {
                gCamEyeGoalX = (D_803643E0 + offX - 500) << 11;
                gCamEyeGoalY = (D_803643E4 + 6000) << 11;
                gCamEyeGoalZ = (D_803643E8 + offZ + 500) << 11;
            }
            break;
        case 0x1000:
            gCamEyeGoalX = (D_803643E0 + offX - 4000) << 11;
            gCamEyeGoalY = (D_803643E4 + 5000) << 11;
            gCamEyeGoalZ = (D_803643E8 + offZ + 4000) << 11;
            break;
        default:
            if (gCamFixedEye) {
                gCamEyeGoalX = gCamFixedEyeX;
                gCamEyeGoalY = gCamFixedEyeY;
                gCamEyeGoalZ = gCamFixedEyeZ;
            } else {
                if (D_80364AA8 == 0x40) {
                    gCamDistance = 9000;
                    gCamHeight = 11000;
                    gCamEyeHeight = 11000;
                }
                if (D_80364AC1 && D_8036B8B0) {
                    gCamEyeGoalX = (D_803FCD48 + D_8036B8B4) << 11;
                    gCamEyeGoalY = (D_803FCD4C + D_8036B8B8) << 11;
                    gCamEyeGoalZ = (D_803FCD50 + D_8036B8BC) << 11;
                } else if (D_803643DB && D_8036B8B0) {
                    gCamEyeGoalX = (D_803EF6DC + D_8036B8B4) << 11;
                    gCamEyeGoalY = (D_803EF6E0 + D_8036B8B8) << 11;
                    gCamEyeGoalZ = (D_803EF6E4 + D_8036B8BC) << 11;
                } else if (D_8030F668) {
                    cam_polar_offset(2500, gCamHeading, &px, &pz);
                    gCamEyeGoalX = ((D_803F767C << 5) + px) << 11;
                    gCamEyeGoalY = ((D_803F767E << 5) + 5000) << 11;
                    gCamEyeGoalZ = ((D_803F7680 << 5) + pz) << 11;
                } else if (D_8030F66A) {
                    cam_polar_offset(2500, gCamHeading, &px, &pz);
                    gCamEyeGoalX = (px + D_803F7670) << 11;
                    gCamEyeGoalY = (D_803F7674 + 5000) << 11;
                    gCamEyeGoalZ = (pz + D_803F7678) << 11;
                } else if (D_8030F669) {
                    if (D_8036C794 != NULL) {
                        cam_polar_offset(2500, gCamHeading, &px, &pz);
                        gCamEyeGoalX = ((D_8036C794->unk12 + D_8036C794->unk0) / 2 << 16) + (px << 11);
                        gCamEyeGoalY = ((D_8036C794->unk14 + D_8036C794->unk2) / 2 << 16) + 0x9C4000;
                        gCamEyeGoalZ = ((D_8036C794->unk16 + D_8036C794->unk4) / 2 << 16) + (pz << 11);
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
                        if (D_80364456 == 2 || D_80364456 == 9) {
                            cam_polar_offset(gCamDistance, angle, &px, &pz);
                        } else {
                            cam_polar_offset(gCamDistance / 2, angle, &px, &pz);
                        }
                        gCamEyeGoalY = (gCamEyeHeight * 3 / 5 + D_803643E4) << 11;
                        offX = 0;
                        offZ = 0;
                    } else {
                        cam_polar_offset(gCamDistance, gCamHeading, &px, &pz);
                        if (D_80364411 && gCamHeading >= 134.0 && gCamHeading <= 136.0) {
                            gCamEyeGoalY = (D_803643E4 + 0x3908) << 11;
                        } else {
                            gCamEyeGoalY = (D_803643E4 + gCamEyeHeight) << 11;
                        }
                    }
                    gCamEyeGoalX = (D_803643E0 + px + offX) << 11;
                    gCamEyeGoalZ = (D_803643E8 + pz + offZ) << 11;
                }
            }
            break;
    }
    switch (gGameMode) {
        case 0x100:
            if (D_80364AC1) {
                gCamAtGoalX = D_803FCD48 / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else {
                gCamAtGoalX = D_803EF6DC / 32.0f;
                gCamAtGoalY = D_803EF6E0 / 32.0f;
                gCamAtGoalZ = (D_803EF6E4 + D_80364A78 + 11000) / 32.0f;
            }
            break;
        case 0x200:
        case 0x400:
            if (D_80364AC1) {
                gCamAtGoalX = (D_803FCD48 + offX) / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = (D_803FCD50 + offZ) / 32.0f;
            } else {
                gCamAtGoalX = (D_803EF6DC + offX) / 32.0f;
                gCamAtGoalY = D_803EF6E0 / 32.0f;
                gCamAtGoalZ = (D_803EF6E4 + offZ) / 32.0f;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (D_80364AC1) {
                gCamAtGoalX = D_803FCD48 / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else {
                gCamAtGoalX = (D_803EF2EC + offX) / 32.0f;
                gCamAtGoalY = D_803EF314 / 32.0f;
                gCamAtGoalZ = (D_803EF2F4 + offZ) / 32.0f;
            }
            break;
        case 0x40:
            if (D_80364AC1) {
                gCamAtGoalX = D_803FCD48 / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
                break;
            }
        default:
            if (D_80364AC1 && D_8036B8B0) {
                gCamAtGoalX = D_803FCD48 / 32.0f;
                gCamAtGoalY = (D_803FCD4C + 6000) / 32.0f;
                gCamAtGoalZ = D_803FCD50 / 32.0f;
            } else if (D_803643DB && D_8036B8B0) {
                gCamAtGoalX = D_803EF6DC / 32.0f;
                gCamAtGoalY = D_803EF6E0 / 32.0f;
                gCamAtGoalZ = D_803EF6E4 / 32.0f;
            } else if (D_8030F668) {
                gCamAtGoalX = D_803F767C;
                gCamAtGoalY = D_803F767E;
                gCamAtGoalZ = D_803F7680;
            } else if (D_8030F66A) {
                gCamAtGoalX = D_803F7670 / 32.0f;
                gCamAtGoalY = D_803F7674 / 32.0f;
                gCamAtGoalZ = D_803F7678 / 32.0f;
            } else if (D_8030F669) {
                if (D_8036C794 != NULL) {
                    gCamAtGoalX = (f32) (D_8036C794->unk12 + D_8036C794->unk0) / 2.0;
                    gCamAtGoalY = (f32) (D_8036C794->unk14 + D_8036C794->unk2) / 2.0;
                    gCamAtGoalZ = (f32) (D_8036C794->unk16 + D_8036C794->unk4) / 2.0;
                }
            } else {
                gCamAtGoalX = (D_803643E0 + offX) / 32.0f;
                gCamAtGoalY = (D_803643E4 + gCamAtHeight) / 32.0f;
                gCamAtGoalZ = (D_803643E8 + offZ) / 32.0f;
            }
            break;
    }
    if (D_803643D6 && levelno == 0x31) {
        gCamEyeGoalX = (D_803EF6DC + offX + 16000) << 11;
        gCamEyeGoalY = (D_803EF6E0 + 15000) << 11;
        gCamEyeGoalZ = (D_803EF6E4 + offZ - 3000) << 11;
    }
    if (D_80364AC1 && D_8036B965) {
        if (gCamSnap) {
            gCamEasedHeading = D_803FCD68 * 16;
        } else {
            gCamEasedHeading = (s16) ((u16) D_803FCD68 * 16 - gCamEasedHeading) / 10 + (s16) gCamEasedHeading;
        }
        gCamEyeGoalX = (s32) (D_803FCD48 - SINE(gCamEasedHeading) * 8000.0f + offX) << 11;
        gCamEyeGoalY = (D_803FCD4C + 0xDAC) << 11;
        gCamEyeGoalZ = (s32) (D_803FCD50 - COSINE(gCamEasedHeading) * 8000.0f + offZ) << 11;
        gCamAtGoalX = D_803FCD48 / 32.0f;
        gCamAtGoalY = (D_803FCD4C + 2000) / 32.0f;
        gCamAtGoalZ = D_803FCD50 / 32.0f;
    }
    if (gGameMode == 0x40 && levelno == 0x3B) {
        gCamEyeGoalX = (D_803643E0 + 0x1900) << 11;
        gCamEyeGoalY = (D_803643E4 + 0x3070) << 11;
        gCamEyeGoalZ = (D_803643E8 - 0x1900) << 11;
        gCamAtGoalX = D_803643E0 / 32.0f;
        gCamAtGoalY = D_803643E4 / 32.0f;
        gCamAtGoalZ = D_803643E8 / 32.0f;
    }
    if ((gGameMode & 0x2000000000002104) && !D_8036B8B0 && !inShuttle && gGameMode != 0x100 && levelno != 0x23 &&
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
        func_802CE4F0(gCamEyeX >> 11, gCamEyeY >> 11, gCamEyeZ >> 11);
        func_802CE5BC(gCamEyeX >> 11, gCamEyeY >> 11, gCamEyeZ >> 11,
                      D_80364434 ? D_80364430 : D_80364430 + 1600, 0xBE, 0);
        if (D_80364434) {
            if (D_803A7424) {
                gCamEyeHeight += D_8036442C;
                if (gCamEyeHeight > D_80364428) {
                    gCamEyeHeight = D_80364428;
                }
                D_80364434 = 1;
            } else {
                D_80364434 = 0;
            }
        } else if (D_803A7424) {
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
        gCamYaw = func_802AD7D4((eyeX - gCamAtX) * 65535.9 / dist) >> 4;
        quadrant = 0;
    }
    if (eyeX >= gCamAtX && eyeZ < gCamAtZ) {
        gCamYaw = (func_802AD7D4((gCamAtZ - eyeZ) * 65535.9 / dist) >> 4) + 0x400; quadrant = 1;
    }
    if (eyeX < gCamAtX && eyeZ < gCamAtZ) {
        gCamYaw = (func_802AD7D4((gCamAtX - eyeX) * 65535.9 / dist) >> 4) + 0x800; quadrant = 2;
    }
    if (eyeX < gCamAtX && eyeZ >= gCamAtZ) {
        gCamYaw = (func_802AD7D4((eyeZ - gCamAtZ) * 65535.9 / dist) >> 4) + 0xC00; quadrant = 3;
    }
    dist = sqrtf((eyeX - gCamAtX) * (eyeX - gCamAtX) + (eyeZ - gCamAtZ) * (eyeZ - gCamAtZ) +
                 (eyeY - gCamAtY) * (eyeY - gCamAtY));
    if (eyeY >= gCamAtY) {
        pitch1 = func_802AD7D4((eyeY - gCamAtY) * 65535.9 / dist) >> 3;
    } else {
        pitch1 = 0x1000 - (func_802AD7D4((gCamAtY - eyeY) * 65535.9 / dist) >> 3);
    }
    if (eyeY >= gCamAtY) {
        pitch2 = func_802AD7D4((65535.0 < (eyeY - gCamAtY) * 65535.9 / 20000.0) ? 65535.0 : (eyeY - gCamAtY) * 65535.9 / 20000.0) >> 3;
    } else {
        pitch2 = 0x1000 - (func_802AD7D4((65535.0 < (gCamAtY - eyeY) * 65535.9 / 20000.0) ? 65535.0 : (gCamAtY - eyeY) * 65535.9 / 20000.0) >> 3);
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

    switch (D_80364456) {
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
        if (ok && ((D_80370C21 && !D_80370C27) || (D_80370C1E && !D_80370C24)) && (gGameMode & 0x104)) {
            func_80260650(D_80367738, 0xD0, NULL);
        }
        if (D_80370C21 && !D_80370C27 && !gCamTurnPos && !gCamTurnNeg && !ok && gGameMode != 0x2000) {
            gCamHeadingGoal = gCamHeading + 45.0;
            if (gCamHeadingGoal >= 360.0) {
                gCamHeadingGoal = gCamHeadingGoal - 360.0;
            }
            gCamTurnPos = 1;
            if (gGameMode != 0x40) {
                func_80260650(D_80367738, 0xDD, NULL);
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
        if (D_80370C1E && !D_80370C24 && !gCamTurnPos && !gCamTurnNeg && !ok && gGameMode != 0x2000) {
            gCamHeadingGoal = gCamHeading - 45.0;
            if (gCamHeadingGoal < 0.0) {
                gCamHeadingGoal = gCamHeadingGoal + 360.0;
            }
            gCamTurnNeg = 1;
            if (gGameMode != 0x40) {
                func_80260650(D_80367738, 0xDC, NULL);
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

    if (gGameMode == 0x100000000000 || gGameMode == 2) {
        ok = 1;
    } else {
        switch (levelno) {
            case 4:
                if (func_802AC4C4(D_803643E0 >> 5, D_803643E8 >> 5, 0xE56, 0x8EC, 0xBB8, 0x6A4, 0x1068, 0x1F4) ||
                    func_802AC4C4(D_803643E0 >> 5, D_803643E8 >> 5, 0xE56, 0x8EC, 0x1068, 0x1F4, 0x1324, 0x4B0)) {
                    ok = 1;
                }
                break;
            case 16:
                if (D_803643E0 > 0x46500 && D_803643E8 > 0x3E800) {
                    ok = 1;
                }
                break;
            case 13:
                if (D_803643E0 > 0x42680 && D_803643E8 > 0x46500) {
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
    status = func_8028A370();
    func_80261588();
    debug_printf("audio inited\n");
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
    gFrameBufferPhys[0] = PHYS(gFrameBuffers[0]);
    gFrameBufferPhys[1] = PHYS(gFrameBuffers[1]);
    gDepthBufferPhys = PHYS(D_8021ED00);
    rsp_init_ucode_table();
    func_802D6710();
    func_8028FC10();
    if (!(status & 1)) {
        gNextGameMode = 0x0800000000000000;
    } else if (D_802FDBD0) {
        gNextGameMode = 0x0000080000000000;
    } else if (D_802FDBD4) {
        gNextGameMode = 0x0040000000000000;
    } else {
        gNextGameMode = 0x10;
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
    D_80364A70 = func_80261A44(gNextGameMode);
    func_802D6710();
    osInvalDCache((void *) 0x80000000, 0x400000);
    D_803649F4 = 0;
    gUnpausedFrames = 0;
    D_80358064 = 0;
    gFramesInMode = 0;
    gBufferIndex = 0;
    D_80367BD6 = 0;
    func_8026A8BC();
    func_8026A974();
    func_8028AE88();
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
    if (D_802E8F94[levelno].unk0 == 2 && !(gNextGameMode & 0x100000000002)) {
        debug_printf("Allocating ghost buffer memory\n");
        gHeapPtr += 0x20000;
    }
    D_803B9888 = 0;
    func_802A0700();
    D_803643C8 = (Vehicle *) (((u32) D_80358088 + 0x40) & ~0x3F);
    func_80278E3C();
    cmo_hit_request = 0;
    D_803643DA = 0;
    D_803643D8 = 0;
    D_803643D6 = 0;
    D_803643D7 = 0;
    gPauseRequest = 0;
    gUnpauseRequest = 0;
    gPaused = 0;
    D_8036EB99 = 0;
    D_803669B4 = 0;
    if (gNextGameMode & 0xC9FD8FE7FBFFC0B0) {
        func_8028B3E0();
    }
    func_80297530(levelno);
    func_80272C50();
    if (gNextGameMode == 0x40000000000) {
        func_801F7850();
    }
    old = gHeapPtr;
    func_8026B118(0);
    debug_printf("Yoshi windows allocated %d bytes, %x\n", gHeapPtr - old, gHeapPtr);
    D_803649D0 = D_80364460;
    func_8028A42C();
    func_802592F0();
    D_8039CAA2 = 0;
}

/* Loads level data file (lagp, chimp, valley, ..., level59) to dest; *size gets its size */
void func_8025615C(s32 level, s32 dest, s32 *size) {
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

/* Level init */
void func_80256A34(s32 arg0) {
    s32 i;
    s32 size;
    Vehicle *v;
    u8 found;
    s32 old;

    D_80364A68 = 0;
    D_80364A69 = 0;
    D_80364A6A = 0;
    D_80364A6B = 0;
    D_80364A6C = 0;
    D_80364A6D = 0;
    D_803649E8 = 0;
    D_803649EC = 0;
    D_803643CC = D_803643C8;
    D_80364AA8 = D_802E8F94[levelno].unk0;
    gShakeTime = 0;
    gShakeSize = 0;
    D_803643E0 = 0;
    D_803643E4 = 0;
    D_803643E8 = 0;
    D_8036443C = 0;
    gCamHeading = 135.0f;
    gCamHeadingGoal = 135.0f;
    gCamTurnPos = 0;
    gCamTurnNeg = 0;
    gCamEyeHeight = 3000;
    D_80364434 = 1;
    D_803A7430 = 0;
    D_803649EE = 0;
    D_80364A84 = 0;
    D_80364456 = 0;
    if (frontEndPresent) {
        HD_ASSERT(!pakBusy, 4162);
        HD_ASSERT(MQ_IS_EMPTY(&pakToGameMessageQ), 4163);
        sched_remove_client((struct BcSched *) gScheduler, (struct BcScClient *) D_80218EE0);
        func_802D67F0((OSThread *) D_80218D30);
    }
    D_8039CA62 = 0;
    D_8039CA61 = 0;
    if (D_80364AA8 == 2 && gNextGameMode == 0x2000) {
        if (!D_8039CA60) {
            func_80294E30();
        }
        func_80294E88();
        func_80294EB8();
    }
    func_8025615C(levelno, gHeapPtr, &size);
    D_80358074 = (u8 *) N64_IPTR(gHeapPtr);
    gHeapPtr += size;
    align_up(&gHeapPtr, 0x10);
    func_80285190();
    func_80275430();
    func_802621DC(levelno);
    func_80262238(levelno);
    cam_set_level_fovy(levelno);
    D_80367BFF = 0;
    func_802CE840();
    debug_printf("enter initlevel game_mode=%d loop_done=%d\n", lowest_set_bit(gGameMode), lowest_set_bit(gNextGameMode));
    old = gHeapPtr;
    func_802A1674(D_80358074, arg0);
    debug_printf("exit initlevel allocated %d bytes, %x\n", gHeapPtr - old, gHeapPtr);
    func_80257234();
    if (gNextGameMode != 2) {
        if ((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0) {
            func_802CF628();
        }
    }
    func_802C1DD0(D_80364AA8 == 0x20 || D_80364AA8 == 0x80);
    func_80262320(levelno);
    if (gCamFixedEye) {
        D_80364A86 = func_80272C5C((u16 *) D_802E8BF4, 0, 1, 1, 1, 1.0f);
    }
    func_802775C0();
    if (D_80364A69) {
        func_80286A00();
    }
    if (D_80364A6B) {
        func_802873AC();
    }
    if (D_80364A6D) {
        func_80287AE4();
    }
    func_802821D0();
    func_80282728();
    func_80281A70(levelno);
    if (D_802E8F94[levelno].unk0 == 1) {
        func_80264C20(arg0);
    }
    func_80288220();
    func_8027BE4C();
    func_80292240();
    func_8027E344(levelno);
    func_802807D8(levelno);
    func_80268664(levelno);
    func_8026A988();
    v = D_803643C8;
    while (v != D_803643CC) {
        i = 0;
        found = 0;
        while (!found) {
            if (D_80364460[i].unk5C == v->unk1022) {
                found = 1;
            } else {
                i++;
            }
        }
        func_80258544(v, v->unk1004, v->unk1010, v->unk100C, v->unk1000, (Gfx *) N64_IPTR((D_80364460[i].unk54)), D_80364460[i].unk0,
                      D_80364460[i].unk4);
        rsp_wait_task(0x61F);
        v++;
    }
    func_802729F0(gNextGameMode, levelno);
    gCamYaw = 0x2000;
    gCamPitch = 0x2000;
    D_80364456 = 0;
    D_803649ED = 0;
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
    D_803643E0 = D_803ED808;
    D_803643E4 = D_803ED80C;
    D_803643E8 = D_803ED810;
    D_802FDB14 = 0;
    if (D_802E8BF8) {
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
    if (D_8039CAB7) {
        func_802979E0(levelno);
    }
    func_802A56C4();
    func_802A5FA8();
}

/* Allocates the per-level display list buffers (two of each) from the heap */
void func_80257234(void) {
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
    D_80358030[0] = gHeapPtr;
    gHeapPtr += a * 8;
    D_80358038[0] = gHeapPtr;
    gHeapPtr += b * 8;
    D_80358040[0] = gHeapPtr;
    gHeapPtr += c * 8;
    D_80358048[0] = gHeapPtr;
    gHeapPtr += d * 8;
    D_80358030[1] = gHeapPtr;
    gHeapPtr += a * 8;
    D_80358038[1] = gHeapPtr;
    gHeapPtr += b * 8;
    D_80358040[1] = gHeapPtr;
    gHeapPtr += c * 8;
    D_80358048[1] = gHeapPtr;
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
