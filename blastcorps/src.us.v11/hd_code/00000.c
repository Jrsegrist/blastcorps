#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80358070
#define LEGACY_D_80358074
#define LEGACY_D_80364458
#define LEGACY_D_803649D0
#define LEGACY_D_80367734
#define LEGACY_D_80367738
#define LEGACY_D_8036BED8
#define LEGACY_D_8036C794
#define LEGACY_D_8036E694
#define LEGACY_D_803ED808
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuEntry *) D_8020C070)
#define D_80218D30 ((u8 *) &D_80218D30)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F5804 ((MenuItem *) D_802F5804)
#define D_802F8BDC ((MenuPage *) D_802F8BDC)
#define D_80315440 ((u8 *) &D_80315440)
#define D_803156D8 ((u8 *) &D_803156D8)
#define D_80364460 ((Unk74 *) D_80364460)
#define D_80364AF0 ((Player *) D_80364AF0)
#define D_8039C4B8 ((u8 *) D_8039C4B8)
#ifdef NON_MATCHING
#define D_80358070 (*(s32 *) &D_80358070)
#define D_80364458 (*(u32 *) &D_80364458)
#define D_803649D0 (*(Unk74 * *) &D_803649D0)
#define D_8036BED8 (*(Node88 * *) &D_8036BED8)
#define D_8036C794 (*(Box * *) &D_8036C794)
#define D_8036E694 (*(s32 *) &D_8036E694)
#define D_803ED808 (*(s32 *) D_803ED808)
#endif
/* end of views */

/* hd.c (from its assert strings): boot, the main game thread and the
 * frame loop. Owns .rodata at 0x80307B90.
 *
 * Variables defined here (the tell is a shared `lui` on paired stores).
 * Only part of hd.c's .bss is modelled so far: the section is placed at
 * 0x80310D80 by hd_code_bss.us.v11.ld, and the later variables get their
 * real addresses from undefined_syms (absolute symbols win). */
u64 D_80310D80[0x400]; /* main thread stack, filled with a guard pattern */
u64 D_80364A90; /* game mode */
u64 D_80364A98; /* next game mode */
u64 D_803649D8; /* frame time */
u64 D_80364A88; /* previous game mode */
u64 D_80364AA0; /* game mode after next */
u64 D_80364AD0; /* timer start */
u8 D_80364412; /* camera: snap to the new position this frame */

void func_80244870(void *);
void func_80244930(void *);

extern OSThread D_80310820;
extern u64 D_803109D0[]; /* idle thread stack */
extern u8 D_80314D80[];
extern u8 D_80314D98[];
extern u32 D_803649F4;

/* a drawable object, 0x74 bytes */
typedef struct {
    void *unk0; /* segment 6 */
    void *unk4; /* segment 7 */
    void *unk8; /* segment 7 (alternate) */
    void *unkC; /* display lists by LOD */
    void *unk10;
    void *unk14;
    u8 pad18[0x18];
    void *unk30; /* alternate display lists by LOD */
    void *unk34;
    void *unk38;
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
extern s32 D_80358070; /* heap pointer */
#ifndef NON_MATCHING
extern s32 D_8036E694;
#endif
#endif
typedef struct {
    u8 unk0;
    u8 pad1[0x35];
    u16 unk36;
    u8 pad38[0xC];
} LevelInfo;
extern u8 D_80358088[];

void func_802558C8(Gfx *, s32 *);
void func_802559F8(Gfx *, s32 *);

#define TOPLEVEL_DL_SIZE 0xB5E

/* Rare's assert; line numbers are the original hd.c's */
#define HD_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "hd.c", line)

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
extern u64 D_80312D80[];
extern OSMesg D_803153F8[];
extern u8 D_8021ED00[];
extern f32 D_80364418;
extern u8 D_8036441C;
extern u8 D_8036441D;
#define PHYS(x) ((u32)(x) & 0x1FFFFFFF)

u8 func_80255628(void);

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


#define pakToGameMessageQ D_80219F50
extern u8 D_802E8BF4[];
extern s32 D_80364420;
extern u8 D_80364434;
extern f32 D_80364444;
extern f32 D_80364448;
extern s16 D_80364454;
extern f32 D_803649F8;
extern s32 D_80364A5C;
extern u8 D_80364A6F;
extern u8 D_80364A86;
#define pakBusy D_8039C4B0
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
    char *unkC; /* title */
    void *unk10; /* glyph list */
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
void func_802507C8(Mtx *, LookAt *, Mtx *);
#define SEG2(off) ((u8 *) D_02000000 + (off))

#define MS_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "./master_switch.c", line)
#define nss D_8036EA70
extern u16 D_803047A0[];
extern u16 D_803047B4[];
extern u16 D_803047CC[];
extern u16 D_803047DC[];
extern u8 D_80315438;
extern u32 D_80364A54;
extern u8 D_80364A60;
extern s32 D_80364AC4;
extern u32 D_80364AC8;
extern u32 D_80364ACC;
extern u8 D_80365065;
extern u8 D_80365066;
extern u8 D_8039C4F8[];
#define saveIt D_8039C53C
#define playerNumber D_80364AE8
#define levelno D_802E8BDC
#define pakBuffer D_8039C4B8
#define saveLevel D_8039C540
#define record_status func_802C4A40
#define areWeFading func_802753C0
#define LEVEL_SAVE_SIZE 0x40
void func_802475D8(void);
void func_80255AD0(void);
void func_80255D34(void);
#define MODE_PAGE(m) D_802F8BDC[D_802F4868[func_8026F92C(m)]]
#define MODE_ENTRY(m) D_8020C070[MODE_PAGE(m).unkE + MODE_PAGE(m).unk10 - 2]

extern s32 D_80364A78;
extern u8 D_80364A7C;
extern s32 D_80364A80;
extern f32 D_802E8BE0;
extern u8 D_80364A85;
#ifndef NON_MATCHING
extern s32 D_80367734;
#endif
#define textureDmaMessageQ D_80315180
#define nextdma D_80358080
#define no_palette_dmas D_80358084
#define NUM_TEXTURE_DMAS 0x90
#define cmo_hit_request D_803643D9
void func_8024B8F4(Mtx *, Mtx *);
void func_8024AE2C(void);
void func_8024B188(void);
void func_8024B618(void);
void func_8024B7AC(void);
void func_8024A92C(u32);
void func_8024B5E8(void);
void func_8024A348(void);
void func_8024ADD8(void);
Gfx *func_8024C414(DynamicBuf *, s32 *);
void func_8024BDA4(u16 *);
#define LEVEL_DONE(l) (D_80364AF0[D_80364AE8].unk18[l] > 0 && D_80364AF0[D_80364AE8].unk18[l] < 6) ? 1 : 0

/* (end of declarations) */

/* Boot: reads 16 words from PI address 0xFFB000, then starts the idle thread */
void func_802447C0(void) {
    u32 i;
    s32 pad[2];
    u32 addr;
    u32 buf[16];
    s32 pad2[2];

    func_802D4020();
    for (addr = 0xFFB000, i = 0; i < 16; i++, addr += 4) {
        osPiRawReadIo(addr, &buf[i]);
    }
    func_80270AE0((u8 *) buf);
    osCreateThread(&D_80310820, 1, func_80244870, NULL, D_803109D0 + 0x40, 10);
    osStartThread(&D_80310820);
}

/* Idle thread: starts the managers and the main thread, then spins */
void func_80244870(void *arg0) {
    s32 pad;

    func_802D4550(4);
    func_802D4560(0x96, (OSMesgQueue *) D_80314D80, (void **) D_80314D98, 0xC2);
    osCreateThread(&D_80310BD0, 3, func_80244930, arg0, D_80310D80 + 0x400, 10);
    osStartThread(&D_80310BD0);
    if (D_802FA254 == 0) {
        osStartThread(&D_80310BD0);
    }
    osSetThreadPri(NULL, 0);
    while (1) {
    }
}

/* The main thread: runs game mode switches (master_switch.c) and the frame loop */
void func_80244930(void *arg) {
    s32 i;
    s32 start2;
    void *msg;
    u8 newPlayer;
    s32 start;

    func_80255AD0();
    while (1) {
        D_80364A70 = 0;
        do {
            func_8029A7E4("game mode switch from %d to %d\n", func_8026F92C(D_80364A90), func_8026F92C(D_80364A98));
            D_80364AA0 = 0;
            switch (D_80364A98) {
                case 32:
                    func_80255DC8();
                    func_801EF380(2);
                    break;
                case 16:
                    func_80255DC8();
                    func_801EF380(1);
                    start2 = D_803156C4;
                    while ((u32) (D_803156C4 - start2) < 15) {
                        PORT_SPIN();
                    }
                    break;
                case 0x100000000:
                    func_801F6F18();
                    func_8026AF6C(0x8012);
                    D_80364A70 = func_80261A44(D_80364A98);
                    break;
                case 0x80000000000:
                    func_80255DC8();
                    func_8025D184();
                    func_80200714(1);
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000001, 1);
                    func_8026AF6C(0x8011);
                    break;
                case 0x10000:
                    D_80364A70 = func_80261A44(D_80364A98);
                    func_8025D184();
                    func_801EA4B8();
                    func_801E8DCC(D_80364AE8);
                    func_8026AF6C(0x800A);
                    break;
                case 0x10000000:
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000001, 1);
                    func_801E8DCC(4);
                    func_8026AF6C(0x8011);
                    break;
                case 0x20000000000000:
                    func_802609D0();
                    func_80255DC8();
                    func_802A0700();
                    func_8025D184();
                    func_80200714(1);
                    osSendMesg(&D_80219EF8, (OSMesg) 0x0100000F, 1);
                    osRecvMesg(&D_80219F50, &msg, 1);
                    if (msg != NULL || D_8039C541) {
                        D_802E8BF8 = 1;
                    } else {
                        D_802E8BF8 = 0;
                    }
                    D_8039C541 = 0;
                    if (!D_802E8BF8) {
                        func_801E8C40(4);
                        D_80364AA0 = 0x10000000;
                    } else {
                        D_80364AE8 = 0;
                        D_80364AE9 = 0;
                        D_80364AEA = 0, osSendMesg(&D_80219EF8, (OSMesg) 0x01000010, 1);
                        osRecvMesg(&D_80219F50, &D_8039C4B4, 1);
                        if (D_8039C4B4 == NULL) {
                            func_8029A7E4("NO EE PRESENT! - USING DUMMY EE\n");
                        }
                        osSendMesg(&D_80219EF8, (OSMesg) 0x01000006, 1);
                        osRecvMesg(&D_80219F50, &msg, 1);
                        if (msg == NULL) {
                            msg = (void *) func_80201E80();
                        }
                        if (msg != NULL) {
                            func_801EA108(D_80364AE8, 1, 1);
                            D_80364AA0 = 0x40000;
                        } else {
                            D_80365060[D_80364AE8] = 1;
                            D_80364AA0 = 0x4000;
                        }
                    }
                    break;
                case 0x4000000000000:
                    func_80255DC8();
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
                    D_80364AA0 = 2;
                    D_8039C541 = 0;
                    func_801ECE9C();
                    break;
                case 2:
                    switch (D_802E8BEC) {
                        case 1:
                        case 3:
                        case 4:
                        case 6:
                            if (D_80364A90 == 2) {
                                D_80364A98 = 0x1000000000000;
                                func_80255DC8();
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
                        func_80255DC8();
                        func_8025D184();
                        func_80295E50();
                        func_8025B9D0(D_802E8BEC, &D_802E8BDC);
                        if (D_802E8BEC == 0) {
                            func_8029A130();
                        }
                        func_80256A34(0);
                        break;
                    }
                    break;
                case 0x4000:
                    if (D_80364AE8 != D_80364AEA) {
                        D_80364AE9 = D_80364AEA;
                        D_80364AE8 = D_80364AEA;
                    }
                    func_8028B3E0();
                    func_8029A7E4("World screen centred on level %d\n", D_802E8BDC);
                    D_80364AE8 = D_80364AEA;
                    func_801ECE9C();
                    if (!D_80365065) {
                        D_80365065 = 1;
                        D_80364A87 = 0;
                        D_803643D5 = 0;
                        func_801FE018(8);
                        D_802E8BDC = D_80364AF0[D_80364AE8].pad0[8];
                        func_8029A7E4("going to level %d\n", D_802E8BDC);
                    }
                    if ((D_8039CA60 = 0, ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0)) &&
                        D_802E8BDC >= 0x2B && D_802E8BDC < 0x2E) {
                        if (!((D_80364AF0[D_80364AE8].unk18[D_802E8BDC + 1] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC + 1] < 6) ? 1 : 0)) {
                            D_802E8BDC++;
                        }
                    }
                    func_80255DC8();
                    func_801ECC8C();
                    osViBlack(1);
                    func_802A0700();
                    func_801F8530(D_802E8BDC);
                    break;
                case 0x800000000000:
                    D_80364A70 = func_80261A44(D_80364A98);
                    break;
                case 0x400000000:
                    func_8026AF6C(0x8013);
                    break;
                case 0x800000000000000:
                    func_80255DC8();
                    func_80200714(1);
                    func_8025D184();
                    func_8026AF6C(0x8059);
                    break;
                case 0x40000000000:
                    func_80255DC8();
                    func_80200714(3);
                    func_801E8C40(D_80364AE8);
                    func_8025D184();
                    D_8021A830 = D_80364A90;
                    func_8026AF6C(0x8016);
                    break;
                case 0x200000000000000:
                    func_801E8EB8(D_80364AE8, 1);
                    func_801F8228();
                    func_8026AF6C(0x8016);
                    D_80364A98 = 0x40000000000;
                    break;
                case 0x2000:
                    func_80255DC8();
                    if (D_802E8F94[D_802E8BDC].unk0 != 1) {
                        func_80256A34(0);
                        func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
                        func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA80);
                        D_802E8BD8 = 1;
                    } else {
                        if ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) {
                            func_80256A34((s32) D_8039C4B8);
                        } else {
                            func_80256A34(0);
                        }
                        func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
                        func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA80);
                        D_80364AA0 = 4;
                    }
                    break;
                case 4:
                    switch (D_80364A90) {
                        case 0x1000000000:
                            func_8026101C();
                            func_80261E9C(D_80364A98);
                            D_802E8BD4 = 1;
                            break;
                        case 0x100:
                            D_80364412 = 1;
                            break;
                        case 0x2000:
                            if (D_802E8F94[D_802E8BDC].unk0 != 1) {
                                D_80364A70 = func_80261A44(D_80364A98);
                            }
                            D_802E8BD4 = 1;
                            D_802E8BD4 &= !func_8026AD30(0x52);
                            /* one line: the line break changes as1's schedule */
                            D_802E8BD4 &= !func_8026AD30(0x56); if ((((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0)) &&
                                D_802E8F94[D_802E8BDC].unk0 == 1) {
                                D_802E8BD4 &= !func_8026AD30(0x55);
                            } else {
                                D_802E8BD4 &= !func_8026AD30(0x57);
                            }
                            D_802E8BD8 = !D_802E8BD4;
                            func_8029A7E4("Unpause at start = %d\n", D_802E8BD4);
                            D_80364A58 = ((s32) D_803156C0);
                            D_80358064 = 0;
                            func_8025BB38();
                            func_8029A7E4("snew ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA70.ip, D_8036EA70.tc, D_8036EA70.bd, D_8036EA70.cr, D_8036EA70.rt,
                                          D_8036EA70.coin, D_8036EA70.bdn);
                            func_8029A7E4("sold ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA60.ip, D_8036EA60.tc, D_8036EA60.bd, D_8036EA60.cr, D_8036EA60.rt,
                                          D_8036EA60.coin, D_8036EA60.bdn);
                            func_8029A7E4("sres ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA80.ip, D_8036EA80.tc, D_8036EA80.bd, D_8036EA80.cr, D_8036EA80.rt,
                                          D_8036EA80.coin, D_8036EA80.bdn);
                            func_8029A7E4("srs2 ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n",
                                          D_8036EA90.ip, D_8036EA90.tc, D_8036EA90.bd, D_8036EA90.cr, D_8036EA90.rt,
                                          D_8036EA90.coin, D_8036EA90.bdn);
                            break;
                    }
                    break;
                case 0x100:
                    D_80364412 = 1;
                    break;
                case 0x80:
                    newPlayer = 0;
                    switch (D_80364A90) {
                        case 0x100000000000:
                            func_8028B3E0();
                            D_80364A70 = func_80261A44(D_80364A98);
                        case 0x4000:
                            newPlayer = func_80285814();
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (D_80364AE8 != D_80364AE9) {
                                D_80364AE9 = D_80364AE8, func_8029A7E4("switching to new player ...........\n");
                                func_80285814();
                            }
                            break;
                    }
                    func_80255DC8();
                    if (newPlayer) {
                        osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 0xD | (D_80364AE8 << 16)), 1);
                    }
                    MODE_ENTRY(D_80364AA8).unk0 &= ~1;
                    MODE_ENTRY(D_80364AA8).unk0 |= 0x800;
                    func_8026B8F8();
                    func_8026AF6C(D_802F4868[func_8026F92C(D_80364AA8)] | 0x8000);
                    MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 3;
                    MODE_PAGE(D_80364AA8).unk8 &= ~8;
                    func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
                    D_80315438 = func_801EE800(&D_80364A60, 1, 0), func_801E8C40(D_80364AE8);
                    func_801EC30C(D_80315438);
                    func_801EC288(D_80315438);
                    func_80200714(1);
                    D_80364A71 = func_801EF1E0();
                    if (D_80364A71 != -1) {
                        func_801F55D8();
                    }
                    break;
                case 0x800:
                    func_80255DC8();
                    if (D_80364A90 == 0x4000) {
                        osRecvMesg(&D_80219F50, NULL, 1);
                    }
                    osViBlack(1);
                    func_80256A34(0);
                    func_802661EC();
                    if (D_802E8BDC == 0x32) {
                        func_8026AF6C(0x8024);
                    }
                    break;
                case 0x40000000:
                    func_80255DC8();
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
                    func_801E8C40(D_80364AE8);
                    func_801EC30C(D_80315438);
                    break;
                case 0x8000000:
                    switch (D_80364A90) {
                        case 0x40:
                        case 0x400:
                        case 0x20000000:
                            func_80255DC8();
                            func_80200714(1);
                            D_80364A71 = func_801EF1E0();
                            if (D_80364A71 != -1) {
                                func_801F55D8();
                            }
                            func_801E8C40(D_80364AE8);
                            func_80285A78((u8 *) &D_8036EA90, (u8 *) &D_8036EA70);
                            D_80315438 = func_801EE800(&D_80364A60, 1, 0);
                            func_801EC30C(D_80315438);
                            func_801EC288(D_80315438);
                            break;
                        case 0x40000000:
                            D_80364A70 = func_80261A44(D_80364A98);
                            break;
                        case 0x40000000000:
                        case 0x80000000000000:
                        case 0x100000000000000:
                            if (D_80364AE8 != D_80364AE9) {
                                func_8029A7E4("switching to new player ...........\n");
                                D_80364AE9 = D_80364AE8;
                                D_803643D5 = 0;
                                func_80285814();
                            }
                            func_80255DC8();
                            D_80364A71 = func_801EF1E0();
                            if (D_80364A71 != -1) {
                                func_801F55D8();
                            }
                            func_801E8C40(D_80364AE8);
                            D_80315438 = func_801EE800(&D_80364A60, 1, 0);
                            func_80200714(1);
                            func_801EC30C(D_80315438);
                            func_801EC288(D_80315438);
                            break;
                        default:
                            MS_ASSERT(!saveIt[playerNumber] || saveIt[playerNumber]==levelno+1, 488);
                            saveIt[playerNumber] = levelno + 1;
                            if (D_803643D7) {
                                func_802C1DD0(D_802E8F94[D_802E8BDC].unk0 == 0x20 || D_802E8F94[D_802E8BDC].unk0 == 0x80);
                            }
                            D_803643D5 = !((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0);
                            if (D_80364AE8 == D_80364AEA) {
                                D_80364A87 |= D_803643D5;
                            }
                            if (D_80364AA8 == 1 && !D_803643D5) {
                                for (i = 0; i < 0x40; i++) {
                                    D_8039C4F8[i] = D_8039C4B8[i];
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
                                nss.tc = D_802E8F94[D_802E8BDC].unk36 - D_80367BF6;
                            } else {
                                nss.tc = 0xFFFF;
                            }
                            MS_ASSERT(nss.tc, 530);
                            if (nss.tc == 0) {
                                nss.tc = 1;
                            }
                            func_80255DC8();
                            if (D_80365066) {
                                D_8036EA70.bd = D_8036EB92;
                                D_80365066 = 0;
                            }
                            if (D_803643D5) {
                                func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
                            } else {
                                func_80285A78((u8 *) &D_8036EA80, (u8 *) &D_8036EA60);
                            }
                            D_80315438 = func_801EE800(&D_80364A60, 1, 1), func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA90);
                            if (D_80364A60) {
                                D_80364AA0 = 0x40000000;
                                D_80364A64 = 0xDC;
                                D_80364A70 = 0;
                            } else {
                                func_801EC30C(D_80315438);
                                func_80200714(1);
                                D_80364A71 = func_801EF1E0();
                                if (D_80364A71 != -1) {
                                    func_801F55D8();
                                }
                                func_801E8C40(D_80364AE8);
                            }
                            MODE_ENTRY(D_80364AA8).unk0 |= 1;
                            MODE_ENTRY(D_80364AA8).unk0 &= ~0x800;
                            break;
                    }
                    if (D_803643D5 && D_80364AE8 == D_80364AEA) {
                        MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 1;
                    } else {
                        MODE_PAGE(D_80364AA8).unk18 = MODE_PAGE(D_80364AA8).unkE + MODE_PAGE(D_80364AA8).unk10 - 3;
                    }
                    MODE_PAGE(D_80364AA8).unk8 |= 8;
                    func_8026B8F8();
                    if (D_802E8BF8) {
                        D_8020C070[29].unk0 &= ~1;
                    }
                    func_8026AF6C(D_802F4868[func_8026F92C(D_80364AA8)] | 0x8000);
                    break;
                case 0x40:
                    if (D_80364A90 & 0x04000200) {
                        func_8028B3E0();
                        nss.tc = func_8028604C(D_80364A5C), func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
                        func_801EE800(&D_80364A60, 0, 1);
                    }
                    D_802F5804[24].unk0 |= 1;
                    D_802F5804[24].unk0 &= ~0x800;
                    D_802F5804[23].unk0 |= 1;
                    D_802F5804[23].unk0 &= ~0x800;
                    D_802F5804[17].unk0 &= ~1;
                    D_802F5804[17].unk0 |= 0x800;
                    if ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) {
                        D_802F8BDC[6].unk18 = 0x18, D_802F8BDC[7].unk18 = 0x22;
                    } else {
                        D_802F8BDC[6].unk18 = 0x17, D_802F8BDC[7].unk18 = 0x21;
                    }
                    D_80364A50 = 0;
                    D_80364A54 = D_803156C4;
                    func_80255DC8();
                    if (D_802E8F94[D_802E8BDC].unk0 == 1 &&
                        ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) &&
                        !D_803643D5) {
                        func_80256A34((s32) D_8039C4F8);
                    } else {
                        func_80256A34(0);
                    }
                    func_8026B8F8();
                    func_8025BB50();
                    break;
                case 0x20000000:
                    func_80255DC8();
                    if (D_80364A90 == 0x4000) {
                        osRecvMesg(&D_80219F50, NULL, 1);
                    }
                    osViBlack(1);
                    func_802A0700();
                    if (func_801E7000()) {
                        func_80200714(2);
                    } else {
                        D_80364AA0 = 0x2000;
                    }
                    break;
                case 0x40000:
                    osSendMesg(&D_80219EF8, (OSMesg) ((D_80364AE8 << 16) | 0x14 | 0x01000000), 1);
                    osRecvMesg(&D_80219F50, NULL, 1);
                    func_801EA93C("ENTER NAME!", D_803047A0, 7, 0x1E, (char *) &D_80364AF0[D_80364AE8]);
                    func_8026AF6C(0x800B);
                    func_8025D184();
                    D_80364A70 = func_80261A44(D_80364A98);
                    break;
                case 0x4000000000000000:
                    func_80255DC8();
                    func_80200714(1);
                    D_8020C070[9].unkC = "QUIT GAME!", D_8020C070[9].unk10 = D_803047CC, D_8020C070[9].unk6 = D_8020C070[9].unk8 = 22;
                    func_8026AF6C(0x800C);
                    break;
                case 0x1000000000:
                    func_80260EE0(0x18);
                    func_80260B40(0, 0);
                    func_80297ECC();
                    func_8026AF6C(0x8015);
                    D_802E8BD8 = 1;
                    break;
                case 0x100000000000:
                    func_80299C0C();
                    D_80364A70 = func_80261A44(D_80364A98);
                    func_80255DC8();
                    func_80299C20();
                    if (D_80364A90 == 0x4000 || D_802E8BDC == 0x2F) {
                        osRecvMesg(&D_80219F50, NULL, 1);
                    }
                    osViBlack(1);
                    func_80256A34(0);
                    break;
                case 0x40000000000000:
                    func_80255DC8();
                    osSendMesg(&D_80219EF8, (OSMesg) 0x01000010, 1);
                    osRecvMesg(&D_80219F50, &D_8039C4B4, 1);
                    if (D_8039C4B4 != NULL) {
                        func_8025D184();
                        func_80200714(1);
                        D_8020C070[9].unkC = "ERASE SAVED GAME!", D_8020C070[9].unk10 = D_803047B4, D_8020C070[9].unk6 = D_8020C070[9].unk8 = 20;
                        func_8026AF6C(0x800C);
                    } else {
                        D_80364AA0 = 0x10;
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
                    D_802E8BD8 = 1;
                    func_80260E80();
                    break;
            }
            D_80364A88 = D_80364A90;
            D_80364A90 = D_80364A98;
            D_80364A98 = D_80364AA0;
        } while (D_80364A98 != 0);
        if (D_80364A70) {
            func_80261FB0(D_80364A70);
        }
        while (D_80364A98 == 0) {
            D_80364AD0 = osGetTime();
            func_8025B2B8();
            if (D_80364A90 & 0x4000) {
                D_80364ACC = (osGetTime() - D_8036BF38) / 7825;
                func_801F8980();
                D_80364AC8 = (osGetTime() - D_80364AD0) / 7825;
                if (D_8036E68C[2]) {
                    func_80285110(0x4D2);
                }
                if (D_8036E68C[0]) {
                    func_80285110(0x4D2);
                }
                func_80285110(0x4D2);
            } else if (D_80364A90 & 0xC9FD8FE7DBFF8080) {
                D_80364ACC = (osGetTime() - D_8036BF38) / 7825;
                func_801FE990();
                D_80364AC8 = (osGetTime() - D_80364AD0) / 7825;
                func_80285110(0x4D2);
            } else if (D_80364A90 & 0x20000000) {
                func_801E7598();
                func_80285110(0x4D2);
            } else if (D_80364A90 & 0x30) {
                func_801EF4AC();
                func_80285110(0x4D2);
            } else {
                D_80364ACC = (osGetTime() - D_8036BF38) / 7825;
                func_802475D8();
                D_80364AC8 = (osGetTime() - D_80364AD0) / 7825;
                if (D_8036E68C[1]) {
                    func_80285110(0x61F);
                }
                if (D_8036E68C[2]) {
                    func_80285110(0x54D);
                }
                func_80285110(0x4D2);
            }
            func_80255D34();
            D_80358060++;
            D_80358064++;
            if (!D_802E8BD0) {
                D_80358068++;
            }
        }
        if ((D_80364A90 & 0x104) && (D_803643D6 || D_803643D7)) {
            func_8025BBE8(0, 0, 0);
        }
        HD_ASSERT(!areWeFading(), 627);
        switch (D_80364A98) {
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
        if (D_80364AE8 == D_80364AEA && D_80364A98 == 0x4000 && func_8028653C()) {
            func_802860F0();
        }
    }
}

/* Per-frame game update and draw */
void func_802475D8(void) {
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

    if (D_802FA268 && (D_80370C28 & 1) && !(D_80370C2A & 1) && (D_80364A90 & 0x104)) {
        D_803643DA = 1;
        D_803643D9 = 0;
        if (D_80370C28 & 8) {
            if (D_80364AA8 != 1 || !(LEVEL_DONE(D_802E8BDC))) {
                D_803643DA = 0;
                D_803643D9 = 1;
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
        if ((LEVEL_DONE(D_802E8BDC)) && D_80364AA8 == 1 && D_803643DA) {
            D_802E8BD8 = 1;
            func_80275390(0x08000000);
        }
    }
    D_80358080 = 0;
    D_80358084 = 0;
    D_803A7426 = 0;
    if (D_80358060) {
        if (D_8035805C == 0) {
            if (D_80358060 >= 6) {
                func_8024B8F4(&D_803156F8[D_8035805C].unk80, &D_803156F8[D_8035805C].unk180);
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
                          D_803643C8[j].unk1000, (Gfx *) ((D_80364460 + i)->unk54), (D_80364460 + i)->unk0, (D_80364460 + i)->unk4);
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
    if (D_8035805C == 0) {
        func_80279778(D_803643E0, D_803643E4, D_803643E8, D_803643F8, D_803643FC, D_80364400, (void *) (D_80364460[i].unk58),
                      D_80364460[i].unk0, D_80364460[i].unk8, D_80364460[i].unk60);
    } else {
        func_80279778(D_803643E0, D_803643E4, D_803643E8, D_803643F8, D_803643FC, D_80364400, (void *) (D_80364460[i].unk58),
                      D_80364460[i].unk0, D_80364460[i].unk4, D_80364460[i].unk60);
    }
    D_803649D8 = osGetTime();
    func_8028A3E4();
    func_80284E54((u64 *) (D_803156F8[D_8035805C].dl), D_80358078, 3, 1, 0x4D2, 1);
    func_802A5720();
    D_8035805C ^= 1;
    switch (D_802E8BD0) {
        case 0:
            if (D_802E8BD8) {
                D_802E8BD0 = 1;
                D_802E8BD4 = 0;
                D_802E8BD8 = 0;
                D_803156F4 = D_8035805C ^ 1;
                if (D_80364A90 & 0x104) {
                    if (!D_8036EB99) {
                        func_80260B40(0, 0);
                    }
                    func_80261570(0.5f);
                    if (D_80364A90 == 4 && (D_803643DB || D_80364AC1) && currentYoshiWindow == 0) {
                        D_80364A98 = 0x100;
                        func_802A45D4(10);
                        D_80364A78 = 0;
                    }
                }
            }
            break;
        case 1:
            if (D_802E8BD4 && D_8035805C != D_803156F4) {
                D_802E8BD8 = 0;
                D_802E8BD4 = 0;
                D_802E8BD0 = 0;
                if (D_80364A90 & 0x104) {
                    if (D_80358064) {
                        func_8029A7E4("LOCKING KEYS!\n");
                        D_80370C38 = 1;
                    }
                    func_80261E9C(D_80364A90);
                    func_80261570(1.0f);
                    if (D_80364A90 == 0x100 && (D_803643DB || D_80364AC1)) {
                        D_80364A98 = 4;
                        func_802A45D4(10);
                    }
                    if (D_80358068 == 0) {
                        D_80358064 = 0;
                    }
                }
            }
            break;
    }
    if (D_802E8BD0) {
        D_803156F5 = D_803156F4;
    } else {
        D_803156F5 = D_8035805C;
    }
    if (D_803643DB && (D_80364A90 & 0x104)) {
        func_802BA148();
    }
    if (D_803643DC && (D_80364A90 & 0x1905)) {
        func_802B8794();
    }
    func_802683E0();
    D_803649EE = 0;
    if (D_80364456 == 0 && !(D_80364A90 & 0x1801)) {
        func_8024AE2C();
    }
    func_8028A470();
    if (D_80364A90 & 0x104) {
        zoom = 0;
        if ((D_80370C28 & 4) && !(D_80370C28 & 8)) {
            if (D_802E8BE0 < 1.5 && D_80364A90 == 4) {
                zoom = 2;
                if (!(D_80370C2A & 4)) {
                    func_80260650(D_80367738, 0xDC, NULL);
                }
            } else if (!(D_80370C2A & 4) && D_80364A90 != 0x100) {
                zoom = 3;
            }
        }
        if (zoom == 0) {
            if ((D_80370C28 & 8) && !(D_80370C28 & 4)) {
                if (D_802E8BE0 > 1.0 && D_80364A90 == 4 && D_80364A7C == 0) {
                    zoom = 1;
                    if (!(D_80370C2A & 8)) {
                        func_80260650(D_80367738, 0xDD, NULL);
                    }
                } else if (!(D_80370C2A & 8) && D_80364A90 == 0x100) {
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
                D_802E8BE0 = D_802E8BE0 + 0.05;
                if (D_802E8BE0 > 1.5) {
                    D_802E8BE0 = 1.5f;
                }
                break;
            case 1:
                D_802E8BE0 = D_802E8BE0 - 0.05;
                if (D_802E8BE0 < 1.0) {
                    D_802E8BE0 = 1.0f;
                }
                break;
            case 3:
                if (D_803643DB || D_80364AC1) {
                    func_802A45D4(10);
                    D_80364A98 = 0x100;
                    D_80364A78 = 0;
                    func_80260650(D_80367738, 0xDF, NULL);
                }
                break;
            case 4:
                D_80364A7C = 1;
                if (D_803643DB || D_80364AC1) {
                    func_802A45D4(10);
                    D_80364A98 = 4;
                    func_80260650(D_80367738, 0xDF, NULL);
                }
                break;
        }
    }
    if (D_80364A90 == 0x100 && D_802E8BD0 && yoshiState == 2 && D_803643DB && currentYoshiWindow == 0) {
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
    if (D_80370C22 && !D_802E8BD0 && D_80364456 && !D_8036443C && (D_80364AA8 == 1 || D_80364AA8 == 0x80)) {
        func_8024B188();
    }
    func_8024B618();
    if ((D_80370C28 & 0x4000) && !(D_80370C2A & 0x4000) && !func_802753C0() && D_80364A98 == 0) {
        switch (D_80364A90) {
            case 0x100000000000:
                if (D_802E8BDC != 0x2F && D_802E8BDC != 0x31 && (D_802E8BDC != 0x26 || D_80364A88 == 0x4000)) {
                    func_80260650(D_80367738, 0xDE, NULL);
                    D_80364A98 = 0x4000;
                }
                break;
            case 0x40:
            case 0x400:
                func_80260650(D_80367738, 0xDE, NULL);
                if (LEVEL_DONE(D_802E8BDC)) {
                    D_80364A98 = 0x08000000;
                } else {
                    D_80364A98 = 0x4000;
                }
                break;
            case 0x800:
            case 0x1000:
                if (D_802E8BDC != 0x32 || D_80364A88 == 0x4000) {
                    func_80260650(D_80367738, 0xDE, NULL);
                    D_80364A98 = 0x4000;
                }
                break;
        }
    }
    if (D_803643DC && (!D_802E8BD0 || (D_80364A90 & 0x1801))) {
        func_802B899C();
        if (D_80364A90 & 0x1801) {
            if (!func_802753C0()) {
                if (D_80364A98 == 0 || (D_80364A98 & 0x1801)) {
                    func_802AEEC8();
                }
                func_802B8AE4();
            }
        } else {
            func_8026510C();
        }
    }
    if (!D_802E8BD0 && !D_80364A84) {
        func_8024B7AC();
    }
    if (D_8039CA62 && !D_802E8BD0) {
        func_80294F00();
    }
    if (D_803643DB && (D_80364AA8 & 0x81)) {
        if (!(D_80364A90 & 0x1801)) {
            func_802BA354();
        }
        if ((D_80364A90 & 0x104) && !func_8026B10C() && !D_802E8BD0 && !D_803643D7 && !D_803643D6) {
            func_8024A92C(vis = func_802C1B9C());
        }
    }
    if (D_80364AC1 && !D_802E8BD0) {
        func_802D291C();
        if (D_802E8BDC == 0x32 && yoshiState == 1 && D_80358060 > 50 && (D_80364A90 & 0x1801) && !func_802753C0()) {
            func_80275390(0x2000);
        }
    }
    func_8029DDC8();
    if (!D_802E8BD0) {
        D_803ED40C = 0;
        D_803F7806 = func_802C1AA0();
        func_802BC5E0();
        func_8024B5E8();
        func_8028F794(D_80364456);
        func_80291FAC(D_80364456);
        func_802688C4(D_802E8BDC);
        if (!(D_80364A90 & 0x1801)) {
            func_8026FEC4();
        }
        func_80281CE4();
        if (D_80364A90 & 0x104) {
            func_80297804(D_803643E0, D_803643E4, D_803643E8);
        }
#ifdef NON_MATCHING
        /* The asm takes its param (the texture decoder's type-4/5 table) in
         * $fp: 0 here in every level (traced). */
        func_8029E0AC(LEAKED(s8));
#else
        func_8029E0AC();
#endif
        if ((D_80364A90 & 0x104) && (D_80364AA8 & 1)) {
            func_8024A348();
            func_8024ADD8();
        }
        if (D_80364AA8 != 1 && D_80367C00 && (u32) (((s32) D_803156C0) - D_80364A58) > 90) {
            D_80367C00 = 0;
            func_802794A4();
        }
    }
    if ((D_803643D6 || D_803643D7 || (D_802E8BDC == 0x32 && D_8036EB98 && !(LEVEL_DONE(0x32)))) && D_80364AA8 == 1 && D_80364A5C == 0) {
        func_8029A7E4("TIME IN LEVEL=%d\n", D_80364A5C = ((s32) D_803156C0) - D_80364A58);
    }
    func_802A5510(D_80358074);
    if (D_80364A90 & 0x1801) {
        func_802A45D4(2);
    }
    if (currentYoshiWindow == 0x58 && yoshiState != 8 && !D_80364410) {
        if (!D_80364A85) {
            func_802A45D4(10);
        }
    } else if (D_80364A85) {
        func_802A45D4(10);
    }
    func_802BD1F8((u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x1E0]), (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x578]),
                  (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x910]), (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x112B0]),
                  (s32) &D_803156F8[D_8035805C], (s32) (&D_803156F8[D_8035805C].unk2C0[0x2D]), (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x17070]),
                  (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x170D8]));
    if (D_80364A90 == 4) {
        func_80295C70(D_802E8BDC, D_803643E0, D_803643E8);
    }
    func_802A4CDC((u32 *) D_80358030[D_8035805C], (u32 *) D_80358038[D_8035805C], (u32 *) D_80358040[D_8035805C], (u32 *) D_80358048[D_8035805C],
                  (u32 *) (&D_803156F8[D_8035805C].unkA3A0[0x140]));
    func_8027E9B8(D_8035805C);
    if (!D_802E8BD0 || (D_803643DB && D_803643D6) || D_8036EB99) {
        func_802C0574();
        func_802BCA2C();
    }
    if (!D_802E8BD0) {
        func_8028DF14(D_80364456);
        func_802906C0(D_80364456);
        func_80292830();
        func_8028C874(D_80364456);
    }
    if (D_80364AA8 != 1) {
        if (D_80364A90 & 0x40) {
            func_8026420C();
        }
        if (D_80364A90 & 0x04002104) {
            func_80262BF4();
        }
    } else if (D_803BE738) {
        if (!(LEVEL_DONE(D_802E8BDC))) {
            D_803643D9 = 1;
        } else if (!func_802753C0()) {
            D_803643DA = 1;
            func_80260650(D_80367738, 0x2B, NULL);
            if (D_80364A90 == 0x40) {
                func_80275390(0x40);
            } else {
                func_80275390(0x08000000);
            }
        }
    }
    func_802A64A4();
    func_802886A0();
    if (!D_802E8BD0) {
        func_802CF1A4();
    }
    func_80279514(D_803643E0, D_803643E4, D_803643E8, D_803643F8, D_803643FC, D_80364400);
    func_80277620(D_80358060);
    func_8027D810(D_802E8BDC);
    func_802C2054();
    if (!D_802E8BD0) {
        func_8026A9B4();
    }
    gdl = func_8024C414(&D_803156F8[D_8035805C], &D_80358078);
    if ((D_80364A90 & 0x04002444) && D_80364AA8 != 1) {
        gdl = func_802639B4(gdl, (Gfx **) &D_803156F8[D_8035805C], (u32 *) &D_80358078);
    }
    if (D_80364A90 & 0x100000000002) {
        if (D_80364A90 == 2) {
            gdl = func_8025C878(gdl, (s32) &D_803156F8[D_8035805C], D_8035805C, &D_80358078);
        }
        if (D_802E8BF0 || D_80366A12 == 3 || D_802E8BEC >= 9) {
            if ((yoshiState == 1 || yoshiState == 8 || D_802E8BEC >= 9) && (!D_802E8BF0 || D_80358060 <= D_80366A04)) {
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
            gdl = func_80295EFC((s32) &D_803156F8[D_8035805C], gdl, D_8039CAA0 / 2 - 103,
                                0x55 - ((D_80364A90 == 0x100000000000) << 5), D_8039CAA0);
        }
        if (D_802E8BEC == 0 && D_80366A12 == 3 && D_80364A90 == 2) {
            gdl = func_8029A1A8((s32) D_803156F8, gdl);
        }
        if (D_80364A90 == 2 || D_802E8BDC == 0x2F) {
            func_8025C5D0();
        }
    }
    if (D_80364A90 & 0x1801) {
        if (D_80358060 == 150 && D_802E8BDC != 0x32) {
            func_8026AF6C(0x8040);
        }
        if (!D_80364AF0[D_80364AE8].unk91 && D_80358060 == 10) {
            func_8026AF6C(0x803F);
        }
    }
    gdl = func_8024C404(gdl, &D_803156F8[D_8035805C], &D_80358078);
    if ((D_80364A90 & 0x440) && D_803156C4 - D_80364A54 > 160 && !D_80364A50) {
        D_80364A50 = 1;
        if (yoshiState == 1) {
            if (!(LEVEL_DONE(D_802E8BDC)) || D_80364AA8 == 1) {
                func_8026AF6C(D_802F4870[func_8026F92C(D_80364AA8)] | 0x8000);
            }
        }
    }
    if (D_80364A90 == 8 && !func_802753C0() && (!func_802D4E10(D_80367734) || D_803156C4 - D_80367740 > 300)) {
        func_80275390(0x08000000);
    }
    if (D_802E8BDC == 0x31 && yoshiState == 1) {
        gdl = func_8029A518((Gfx **) &D_803156F8[D_8035805C], gdl);
    }
    func_80259C24(&gdl, (Mtx *) &D_803156F8[D_8035805C]);
    if (D_80364A90 != 2) {
        gdl = func_80274BF0((s32) &D_803156F8[D_8035805C], gdl);
    }
    if (D_80364A90 == 0x2000000000000000) {
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
        !func_802753C0() && D_8036BB1A == -1 && D_80364A98 == 0 && !D_803643D9 && !D_803643DA && currentYoshiWindow != 0) {
        if ((func_8026B10C() & 0x8000) && (func_8026B10C() & 1)) {
            func_8026B10C();
        }
        switch (D_80364A90) {
            case 1:
            case 0x800:
            case 0x1000:
                if (D_802E8BDC != 0x32 || D_80364A88 == 0x4000) {
                    D_80364A98 = 0x2000;
                    func_80260650(D_80367738, 0x1E, NULL);
                }
                break;
            case 0x40:
            case 0x400:
                if ((LEVEL_DONE(D_802E8BDC)) && D_80364AA8 != 1) {
                    D_80364A98 = 0x08000000;
                    func_80260650(D_80367738, 0x1E, NULL);
                } else if (yoshiState == 1) {
                    func_8026AF6C(D_802F4870[func_8026F92C(D_80364AA8)] | 0x8000);
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
                    D_80364A98 = 0x400000000000;
                }
                break;
            case 4:
            case 0x100:
                if ((D_80370C28 & 0x1000) && !D_802E8BD0) {
                    if ((LEVEL_DONE(D_802E8BDC)) && D_80364AA8 == 1) {
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
    gdl = func_8026BBD0(gdl, (s32) &D_803156F8[D_8035805C], &D_80358078);
    if (D_8036BB16 && !D_803643D9 && !D_803643DA) {
        func_8024BDA4(&D_8036BB16);
    }
    if ((D_80364A90 & 0x444) && D_8036BB1A == -1 && D_80367BC8 == 0 &&
        !((currentYoshiWindow != -1) ? D_802F8BDC[currentYoshiWindow].unk8 & 0x01000000 : 0)) {
        alpha = (D_80364A90 & 0x440) ? 0xB4 : 0xFF;
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
    if (D_80364A90 == 2) {
        gdl = func_80274BF0((s32) &D_803156F8[D_8035805C], gdl);
    }
    func_802559F8(gdl, &D_80358078);
    for (i = 0; i < D_80358080; i++) {
        osRecvMesg(&D_80315180, NULL, 1);
    }
    HD_ASSERT(MQ_IS_EMPTY(&textureDmaMessageQ), 1509);
    HD_ASSERT(nextdma-no_palette_dmas<NUM_TEXTURE_DMAS, 1510);
    for (i = 0; i < D_80358080 - D_80358084; i++) {
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
        } else if (D_803643D9) {
            D_803643D6 = 1;
        }
        D_803643DA = 0;
        D_803643D9 = 0;
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

    switch (D_802E8BDC) {
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
            func_8029A7E4("TOO FAR AWAY FROM CMO\n");
        }
        switch (D_80364A6F) {
            case 1:
                switch (D_803153F0) {
                    case 0:
                        func_80260DFC();
                    case 2:
                        func_802608C8((void *) D_803156E8);
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
                func_802608C8((void *) D_803156E8);
                if (D_802E8F94[D_802E8BDC].unk0 != 0x80 || !func_802C1AA0()) {
                    func_8029A7E4("popTuneImmediate();\n");
                    func_80261040();
                } else {
                    func_8029A7E4("WILL THIS FIX IT!!?\n");
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
                if (!D_802E8BD0 && D_80358060 >= 11 &&
                    (D_80364456 == 8 || D_80364456 == 15 || D_80364456 == 13 || D_80364456 == 14)) {
                    func_8026AD30(0x4A);
                }
            }
            D_80364AF0[D_80364AE8].unk10 |= 1 << D_80364456;
            D_803649E8 = 1;
            D_803649EC = 1;
            D_803649EE = 1;
        } else if (!func_80260634((void *) D_803156F0)) {
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
    if (ok && (D_80364A90 & 0x104)) {
        func_8025BBE8((D_80364AF0[D_80364AE8].unkF0 & (1 << id)) ? 0x80 : 0x40, 0, 0);
    }
    if (ok) {
        func_8029A7E4("changing to digger %d\n", id);
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
        if (D_80364A90 & 0x104) {
            func_8025BBE8((D_80364AF0[D_80364AE8].unkF0 & 1) ? 0x80 : 0x40, 0, 0);
        }
    }
    if (res == 0 || (res != 2 && !ok)) {
        if (!func_80260634((void *) D_803156EC)) {
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
 * func_8029E0AC in func_802475D8), which this file declares `void (void)`
 * (traced in the original ROM with mupen64plus breakpoints over attract demos
 * 0-8, nine levels; see the port notes, trace_values.md):
 *  - $fp ($s8): 0 at func_8024B618, func_8024B7AC and func_8029E0AC in every
 *    level (func_802475D8 and the IDO code above it never set $s8, and the
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
                        func_8029A7E4("MOVEABLE GEOMETRY MOVE ROUTINE NOT WRITTEN YET\n");
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

/* Builds and runs a small task drawing an 8-vertex box (12 triangles) */
void func_8024B8F4(Mtx *proj, Mtx *view) {
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
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000010);
    gSPMatrix(gdl++, PHYS(proj), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
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
    func_802A467C((s32) D_80358074, dl, v, (s32) (gdl - dl) * sizeof(Gfx));
}

/* Handles a Yoshi (menu) selection *sel for the current game mode */
void func_8024BDA4(u16 *sel) {
    switch (D_80364A90) {
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
                    D_80364A98 = 4;
                    break;
                case 2:
                    func_80285EF4(D_80364A58);
                    break;
                case 7:
                    func_8028B240();
                    break;
                case 3:
                    if (D_802E8F94[D_802E8BDC].unk0 & 0x81) {
                        func_80275270(0x2000, 0.5f);
                    } else {
                        func_80275270(0x20000000, 0.5f);
                    }
                    break;
                case 8:
                    D_80364A98 = 0x2000000000000000;
                    func_8026AF6C(0x805E);
                    D_80364412 = 1;
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
                        func_8029A7E4("selected controller mode yes\n");
                        D_80364AF0[D_80364AE8].unkF0 |= 1 << D_80364456;
                        func_8025BBE8(0x80, 0, 0);
                    } else {
                        func_8029A7E4("selected controller mode no\n");
                        D_80364AF0[D_80364AE8].unkF0 &= ~(1 << D_80364456);
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
                            func_8029A7E4("TESTING PAUSE2 %d %d %d\n", D_802E8BD0, D_802E8BD8, D_802E8BD4);
                            func_8026AF6C(0x8000);
                            if (D_803643DB || D_80364AC1) {
                                D_80364A98 = 0x100;
                                func_802A45D4(10);
                            }
                            break;
                        case 13:
                        case 0x58:
                            func_8029A7E4("TESTING PAUSE3 %d %d %d\n", D_802E8BD0, D_802E8BD8, D_802E8BD4);
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
            D_80364A98 = 4;
            break;
        case 0x40:
        case 0x400:
            if ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) {
                if (*sel == 0x18 || *sel == 0x22 || *sel == 0xFFFF) {
                    D_80364A98 = 0x08000000;
                } else {
                    D_80364A98 = 0x2000;
                }
            } else {
                if (*sel == 0x18 || *sel == 0x22 || *sel == 0xFFFF) {
                    D_80364A98 = 0x4000;
                } else {
                    D_80364A98 = 0x2000;
                }
            }
            break;
        case 0x2000000000000000:
            if (*sel == 0xFFFF) {
                func_8029A7E4("TESTING PAUSE %d %d %d\n", D_802E8BD0, D_802E8BD8, D_802E8BD4);
                func_8026AF6C(0x8001);
            }
            D_80364A98 = 4;
            break;
        default:
            func_8029A7E4("Yoshi selection in illegal game mode\n");
            break;
    }
    *sel = 0;
}

Gfx *func_8024C404(Gfx *arg0, DynamicBuf *arg1, s32 *arg2) {
    *arg2 = 0;
    return arg0;
}

/* Builds the frame's top-level display list */
Gfx *func_8024C414(DynamicBuf *dyn, s32 *len) {
    Gfx *gdl = dyn->dl;
    char money[16];
    char bonus[16];
    s32 height;

    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(dyn));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gSPClearGeometryMode(gdl++, G_ZBUFFER);
    gDPSetDepthImage(gdl++, D_80358058);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358058);
    gDPSetFillColor(gdl++, 0xFFFCFFFC);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    guTranslate(&dyn->unk1C0, 0.0f, 0.0f, 0.0f);
    guOrtho(&dyn->unkC0, 0.0f, 319.0f, 239.0f, 0.0f, -20000.0f, 20000.0f, 1.0f);
    guOrtho(&dyn->unk100, 0.0f, 1279.0f, 959.0f, 0.0f, -20000.0f, 20000.0f, 1.0f);
    func_802507C8(&dyn->unk140, &dyn->unk3C00, &dyn->unk180);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gdl = func_80271FD0(gdl, (u32) dyn, D_802E8BDC, D_80364452, D_80364454, &height);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    switch (D_802E8BDC) {
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
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    switch (D_80364A90) {
        case 0x40:
        case 0x400:
            guPerspective(&dyn->unk80, &D_8035807C, D_80364438, 1.3333334f, 10.0f, 20000.0f, 1.0f);
            break;
        case 1:
        case 0x800:
        case 0x1000:
            guPerspective(&dyn->unk80, &D_8035807C, D_80364438, 1.3333334f, 10.0f, 10000.0f, 1.0f);
            break;
        default:
            guPerspective(&dyn->unk80, &D_8035807C, D_80364438, 1.3333334f, 10.0f, 10000.0f, 1.0f);
            break;
    }
    gSPSetOtherMode(gdl++, G_SETOTHERMODE_H, 6, 2, 0);
    gSPClipRatio(gdl++, FRUSTRATIO_3);
    func_8027F1F8(&gdl, D_8035805C, 0);
    gSPSegment(gdl++, 8, PHYS(D_80364458));
    gSPClearGeometryMode(gdl++, -1);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) D_80358030[D_8035805C]));
    gSPDisplayList(gdl++, D_803BE6E0);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) D_80358038[D_8035805C]));
    gSPDisplayList(gdl++, D_803BE6E4);
    if (!D_803643D6 && !D_803643D7) {
        gSPClearGeometryMode(gdl++, -1);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
        gSPDisplayList(gdl++, SEG2(0x21410));
        gDPPipeSync(gdl++);
    }
    switch (D_8035805C) {
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
    if (!(D_80364A90 & 0x440)) {
        func_8027C4C8(&gdl, (struct TrailBuf *) dyn);
    }
    func_802502EC();
    func_8024E4F4(&gdl, dyn, 0);
    func_80258B78(&gdl, (struct VtxBuf *) dyn);
    if (!D_802E8BD0 && D_80364AA8 != 0x40 && !(D_80364A90 & 0x440) && !D_80364A84) {
        func_80279EE8(&gdl, (s32) dyn, D_8035805C);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LOD);
    gSPDisplayList(gdl++, SEG2(0x0A580));
    gDPPipeSync(gdl++);
    gdl = func_802CEEFC(gdl, D_8035805C, (Gfx *) dyn->unkA3A0, &dyn->unk2C0[0x3C]);
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
    switch (D_8035805C) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C5770));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C6370));
            break;
    }
    func_80288DF0(&gdl, D_8035805C);
    if (!(D_80364A90 & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 0);
    }
    func_802976E8(&gdl);
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) D_80358040[D_8035805C]));
    gSPDisplayList(gdl++, D_803BE6E8);
    if (!(D_80364A90 & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 1);
    }
    gSPClearGeometryMode(gdl++, -1);
    gSPSetGeometryMode(gdl++, G_ZBUFFER);
    gSPDisplayList(gdl++, osVirtualToPhysical((void *) D_80358048[D_8035805C]));
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
    if (!(D_80364A90 & 2) || !D_802E8BF0) {
        func_8024FC2C(&gdl, 2);
    }
    func_80295120(&gdl, (Mtx *) dyn);
    if (D_80364A90 & 0x2000001000003905) {
        func_80280F34(&gdl, D_8035805C);
    }
    func_8027F1F8(&gdl, D_8035805C, 1);
    func_8024E4F4(&gdl, dyn, 2);
    if (D_803643DC) {
        func_80266248(&gdl, (struct DynBuf *) dyn);
    }
    switch (D_8035805C) {
        case 0:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C6F70));
            break;
        case 1:
            gSPDisplayList(gdl++, osVirtualToPhysical(D_803C7B70));
            break;
    }
    if (D_803643DB) {
        func_8028273C(&gdl, D_8035805C);
    }
    if (D_80364A68 && (D_80364A90 & 0x104)) {
        func_80286C60(&gdl, (s32) dyn, D_8035805C, D_80364456);
    }
    if (D_80364A90 & 0x200000000400220C) {
        func_80278324(&gdl, (s32) dyn, D_8035805C);
    }
    if (D_80364A90 == 0x100 && D_803643DB) {
        func_80276E50(&gdl, dyn, D_8035805C, D_803643E0, D_803643E4, D_803643E8);
    }
    func_80259450();
    if (D_80364A6A && (D_80364A90 & 0x104)) {
        func_80287530(&gdl, (Gfx **) dyn, D_8035805C, D_80364456);
    }
    if (D_80364A6C && (D_80364A90 & 0x104)) {
        func_80287C68(&gdl, (Gfx **) dyn, D_8035805C, D_80364456);
    }
    if (D_80364A90 & 0x104) {
        func_80282224(&gdl, D_80364456);
    }
    if ((D_80364AA8 & 1) && (D_80364A90 & 0x0400030C)) {
        func_8026A378(D_803649F4, (u8 *) &money[1]);
        money[0] = '$';
        func_80259CCC((Gfx **) dyn, (u8 *) money, 0, 1, 0, 0x118, 0x12, 0x14, 0x14, 0, 0xFF, 0xFF, 0xFF, D_80367BD6);
        if (D_80364A44 && !D_802E8BD0) {
            func_8026A378(D_80364A44, (u8 *) &bonus[1]);
            bonus[0] = '$';
            func_80259CCC((Gfx **) dyn, (u8 *) bonus, 0, 1, 0, D_80364A4A, D_80364A4C, 0x23, 0x23, D_80364A4E, 0xFF, 0xFF, 0xFF,
                          D_80364A48);
        }
    }
    if ((D_80364A90 & 0x2000000000000104) && (!(D_80364AA8 & 0x81) || D_803643DB || D_80364AC1)) {
        func_80275478((struct SpriteVtxBuf *) dyn, &gdl, (D_80364A90 & 0x100) || currentYoshiWindow == 0x4D || currentYoshiWindow == 0x49);
    }
    if (D_80364A98 == 0 && !func_802753C0()) {
        if (!(D_80364A90 & 0x200000100400230C) && D_803156C4 % 50 * 60 / 60 >= 21 && yoshiState == 1 &&
            (!(D_80364A90 & 2) || (D_802E8BEC && (D_80366A12 == 3 || D_802E8BEC == 1))) &&
            (D_80364A90 != 0x100000000000 || D_803A6B04) &&
            (!(D_80364A90 & 0x1801) || D_80364AF0[D_80364AE8].unk91) && D_802E8BDC != 0x2F) {
            func_80259CCC((Gfx **) dyn, (u8 *) ("PRESS START"), 0, 1, 0, 0x5C, 0xC4, 0x1A, 0x1A, 1, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        if (D_803156C4 % 40 * 60 / 60 >= 16) {
            if (D_802E8BD0) {
                if (D_80364A90 == 0x2000000000000000 && yoshiState == 2) {
                    func_80259CCC((Gfx **) dyn, (u8 *) ("USE Z/R TO TURN PAGES"), D_8030491C, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                } else if (D_80364A90 == 0x100 && currentYoshiWindow == 0 && yoshiState == 2 && D_803643DB &&
                           !(D_80370C28 & 0x2010)) {
                    func_80259CCC((Gfx **) dyn, (u8 *) ("USE Z/R TO MOVE MAP"), D_80304938, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF,
                                  0xFF, 0xFF);
                }
            } else if (D_80364A90 == 0x100) {
                if (D_80364AC1) {
                    func_80259CCC((Gfx **) dyn, (u8 *) ("SHUTTLE VIEW"), D_80304904, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                } else {
                    func_80259CCC((Gfx **) dyn, (u8 *) ("MISSILE VIEW"), D_80304910, 0, 0, 0x18, 0x14, 0xF, 0xF, 1, 0xFF, 0xFF, 0xFF,
                                  0xFF);
                }
            }
        }
    }
    if (((D_80364A90 & 0x440) || D_802E8BDC == 0x26) && yoshiState == 1 && !func_802753C0()) {
        if ((D_80364A90 & 0x440) && D_80364AA8 != 1) {
            func_80274B40(&gdl, (s32) dyn, D_80365580, 0x108, 0x12);
        } else {
            func_80274B40(&gdl, (s32) dyn, D_80365580, 0x18, 0x12);
        }
    }
    if ((D_80364A90 & 0x104) && D_80364410) {
        func_80274B40(&gdl, (s32) dyn, D_80364A86, 0x108, 0xBE);
    }
    if ((D_80364A90 & 0x104) && D_80364AA8 == 1 && !D_802E8BD0) {
        func_80285CC0();
    }
    if (D_803643DB && D_80364A90 == 4) {
        func_80282C80(&gdl, (Mtx *) dyn, D_803643E0, D_803643E4, D_803643E8, D_803EF6DC, D_803EF6E0, D_803EF6E4);
    }
    if ((D_802E8F94[D_802E8BDC].unk0 & 0x81) && D_80364A90 == 4) {
        if (D_802E8BDC != 0x32 ||
            ((D_80364AF0[D_80364AE8].unk18[0x32] > 0 && D_80364AF0[D_80364AE8].unk18[0x32] < 6) ? 1 : 0)) {
            func_8028376C(&gdl, (Mtx *) dyn, D_8035805C, D_803643E0, D_803643E8, D_803EF6DC, D_803EF6E4);
        }
    }
    if (D_803643DB || D_80364AC1) {
        switch (D_80364A90) {
            case 4:
            case 0x100:
            case 0x200:
            case 0x100000000000:
                func_8025E2CC(&gdl, (s32) dyn, D_8035805C);
                func_8025E67C(&gdl, (s32) dyn, D_8035805C);
            case 0x40:
            case 0x400:
                if ((D_803643D6 || D_803643D7 || D_803643D9 || D_803643DA) && (D_80364A90 & 0x144)) {
                    func_802A45D4(0x32);
                    if (D_80364A90 == 0x40) {
                        D_80364A98 = 0x400;
                    } else {
                        D_80364A98 = 0x200;
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
    all = !D_803649E8 && (D_803649EC || (D_80364A90 & 0x1801));
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
    gSPMatrix(gdl++, &D_02000000[9], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(gdl++, &D_02000000[10], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPVertex(gdl++, &D_02000000[0x63], 4, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
    *gfx = gdl;
}

/* Emits the display lists of every object in D_80364460 */
void func_8024FC2C(Gfx **gfx, u8 lod) {
    Gfx *gdl = *gfx;
    s32 i;
    u8 unused;
    u8 alt;
    u8 all;

    all = !D_803649E8 && (D_803649EC || (D_80364A90 & 0x1801));
    i = 0;
    while (&D_80364460[i] != D_803649D0) {
        if ((D_80364460[i].unk5C != 0 || all) && (D_80364460[i].unk5C != 0xFF || !D_803EF6FF) &&
            (D_80364460[i].unk5C == 0xFD || !D_80364A84 || !D_80364AC1)) {
            gSPSegment(gdl++, 6, osVirtualToPhysical(D_80364460[i].unk0));
            if ((D_80364A90 & 0x1801) && (D_80364460[i].unk5C == 0xFE || D_80364460[i].unk5C == 0)) {
                alt = D_8035805C;
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
        guMtxF2L(mf, &D_803156F8[D_8035805C].unk1500);
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
        guMtxF2L(mf, &D_803156F8[D_8035805C].unk1540);
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
extern u8 D_8030F660; /* target easing settled */
extern s32 D_8030F664; /* target height offset */
extern u8 D_8030F668;
extern u8 D_8030F669;
extern u8 D_8030F66A;
extern s32 D_803643EC; /* wanted eye x, y, z (<< 11) */
extern s32 D_803643F0;
extern s32 D_803643F4;
extern s16 D_80364A74;
extern f32 D_80365078; /* wanted target x, y, z */
extern f32 D_8036507C;
extern f32 D_80365080;
extern f32 D_80365084; /* target x, y, z */
extern f32 D_80365088;
extern f32 D_8036508C;
extern f32 D_80365090; /* target easing deltas */
extern f32 D_80365094;
extern f32 D_80365098;
extern u16 D_8036509C; /* eased heading */
extern s16 D_8036509E; /* camera type */
extern s16 D_803650A0; /* previous camera type */
#ifndef NON_MATCHING
extern Box *D_8036C794;
#endif
f32 func_80254E54(f32, f32, f32, f32, f32, f32);
void func_80255034(s32, f32, s32 *, s32 *);
void func_80255190(void);

#define SINE(a) func_802574F0((f32) (s16) (a) / 16384.0f * 1.5708)
#define COSINE(a) func_80257514((f32) (s16) (a) / 16384.0f * 1.5708)

/* Camera: chooses the eye and target for the current mode and builds the view matrices */
void func_802507C8(Mtx *mtx, LookAt *lookAt, Mtx *view) {
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
    func_80255190();
    if (D_8036B965 && (D_80364A90 & 0x600)) {
        D_8036B965 = 0;
    }
    inShuttle = D_80364AC1 && ((D_80364A90 & 0x940) || D_8036B965);
    if (D_80364A90 == 8 || (currentYoshiWindow == 0x4B && yoshiState == 2 && (u16) D_802F8BDC[currentYoshiWindow].unk18 == 0x14D)) {
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
    if (currentYoshiWindow == 0x58 && yoshiState != 8 && !D_80364410) {
        if (!D_80364A85) {
            D_80364412 = changed = 1;
        }
        D_80364A85 = 1;
    } else {
        if (D_80364A85) {
            D_80364412 = changed = 1;
        }
        D_80364A85 = 0;
    }
    D_80364412 |= inShuttle || D_80364A90 == 0x100;
    D_80364412 |= D_80358060 < 2;
    if (D_803643D6 && (D_80364A90 & 0x100000000600)) {
        rateX = 0.2f;
        rateY = 0.1f;
        rateZ = 0.2f;
    } else if ((yoshiState == 2 && (currentYoshiWindow == 0x4D || currentYoshiWindow == 0x4B || currentYoshiWindow == 0x49)) ||
               (D_803643D7 && (D_80364A90 & 0x100000000600)) || D_80364A90 == 8) {
        rateX = 0.08f;
        rateY = 0.03f;
        rateZ = 0.08f;
    } else if (D_80364A90 & 0x1801) {
        rateX = 0.06f;
        rateY = 0.03f;
        rateZ = 0.03f;
    } else if ((D_80364AA8 == 0x10 || D_80364AA8 == 0x40) && D_80364A90 == 0x40) {
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
    if (D_80364A90 == 0x40 && D_802E8BDC == 0x3B) {
        rateX = 1.0f;
        rateY = 1.0f;
        rateZ = 1.0f;
        dampX = 1.0f;
        dampY = 1.0f;
        dampZ = 1.0f;
    }
    switch (D_80364A90) {
        case 0x100:
        case 0x200:
        case 0x400:
            if (D_80364AC1) {
                D_8036509E = 0xFD;
            } else {
                D_8036509E = 0xFF;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (D_80364AC1) {
                D_8036509E = 0xFD;
            } else {
                D_8036509E = 0xFE;
            }
            break;
        default:
            D_8036509E = D_80364456;
            break;
    }
    tilt = D_8036443C;
    if (!D_803649EC) {
        D_8030F664 = func_802A56C4();
    }
    if (((D_80364A90 & 2) && D_802E8BF0) || !D_803649EC) {
        radius = 0;
        tiltRate = 0.0f;
        turnRate = 0.0f;
    } else {
        switch (D_8036509E) {
            case 7:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xEB;
                D_80364A74 = -0xA5;
                break;
            case 10:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xF5;
                D_80364A74 = -0x87;
                break;
            case 11:
            case 17:
            case 18:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xEB;
                D_80364A74 = -0xA5;
                break;
            case 8:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0x14F;
                D_80364A74 = -0xA5;
                break;
            case 15:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0x131;
                D_80364A74 = -0xA5;
                break;
            case 13:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0x145;
                D_80364A74 = -0xA5;
                break;
            case 14:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0x13B;
                D_80364A74 = -0xA5;
                break;
            case 9:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 1000;
                D_80364A72 = 0x87;
                D_80364A74 = -0x55;
                break;
            case 6:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 5000;
                D_80364A72 = 0;
                D_80364A74 = 0;
                break;
            case 5:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xCD;
                D_80364A74 = -0x69;
                break;
            case 4:
                radius = 9;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xEB;
                D_80364A74 = -0xA5;
                break;
            case 3:
                radius = 7;
                tiltRate = 3.0f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xEB;
                D_80364A74 = -0xA5;
                break;
            case 2:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 2000;
                D_80364A72 = 0x91;
                D_80364A74 = -0x55;
                break;
            case 16:
                radius = 2;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 600;
                D_80364A72 = 0x91;
                D_80364A74 = -0x55;
                break;
            case 1:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                D_80364A72 = 0xCD;
                D_80364A74 = -0xA5;
                break;
            case 0:
                radius = 20;
                tiltRate = 0.25f;
                turnRate = 15.0f;
                D_8030F664 = 0;
                D_80364A72 = 0x55;
                D_80364A74 = -0x19;
                break;
            case 0xFF:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                tilt = tilt / 2;
                D_80364A72 = 0;
                D_80364A74 = 0;
                break;
            case 0xFE:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                tilt = D_803EF328;
                D_8030F664 = 0;
                D_8036443E = D_803EF32A;
                D_80364A72 = 0;
                D_80364A74 = 0;
                break;
            case 0xFD:
                radius = 11;
                tiltRate = 0.75f;
                turnRate = 30.0f;
                D_8030F664 = 0;
                tilt = tilt / 2;
                D_8036443E = D_8036443E / 2;
                D_80364A72 = 0;
                D_80364A74 = 0;
                break;
        }
    }
    if (D_80364A90 == 0x40) {
        radius = radius / 2;
    }
    if (tilt > D_80364A72) {
        tilt = D_80364A72;
    }
    if (tilt < D_80364A74) {
        tilt = D_80364A74;
    }
    if ((tilt > 0 && D_80370C1C && !D_802E8BD0 && (D_80364A90 & 0x106)) ||
        (tilt < 0 && D_80370C1D && !D_802E8BD0 && (D_80364A90 & 0x106))) {
        if (D_80364444 < tilt) {
            D_80364444 += tiltRate;
        }
        if (D_80364444 > tilt) {
            D_80364444 -= tiltRate;
        }
    } else {
        if (D_80364444 < tilt && tilt <= 0) {
            D_80364444 += tiltRate * 10.0f;
            if (tilt < D_80364444) {
                D_80364444 = tilt;
            }
        }
        if (D_80364444 > tilt && tilt >= 0) {
            D_80364444 = D_80364444 - tiltRate * 10.0f;
            if (D_80364444 < tilt) {
                D_80364444 = tilt;
            }
        }
    }
    diff = D_8036443E - D_80364448;
    if (diff < 0.0) {
        diff = 0.0 - diff;
    }
    if (diff > 2048.0) {
        if (D_80364448 > D_8036443E) {
            D_80364448 = D_80364448 + turnRate;
            if (D_80364448 > 4095.0) {
                D_80364448 = D_80364448 - 4095.0;
                if (D_8036443E < D_80364448) {
                    D_80364448 = D_8036443E;
                }
            }
        } else {
            D_80364448 = D_80364448 - turnRate;
            if (D_80364448 < 0.0) {
                D_80364448 = D_80364448 + 4095.0;
                if (D_80364448 < D_8036443E) {
                    D_80364448 = D_8036443E;
                }
            }
        }
    } else {
        if (D_80364448 > D_8036443E) {
            D_80364448 = D_80364448 - turnRate;
            if (D_80364448 < D_8036443E) {
                D_80364448 = D_8036443E;
            }
        }
        if (D_80364448 < D_8036443E) {
            D_80364448 = D_80364448 + turnRate;
            if (D_8036443E < D_80364448) {
                D_80364448 = D_8036443E;
            }
        }
    }
    ang = (s16) D_80364448 % 1024;
    cs = func_80257514((f32) ang / 1024.0f * 1.5708) * ((f32) radius * D_80364444);
    sn = func_802574F0((f32) ang / 1024.0f * 1.5708) * ((f32) radius * D_80364444);
    if (D_80364448 >= 0.0f && D_80364448 < 1024.0f) {
        offX = sn;
        offZ = cs;
    }
    if (D_80364448 >= 1024.0f && D_80364448 < 2048.0f) {
        offX = cs;
        offZ = -sn;
    }
    if (D_80364448 >= 2048.0f && D_80364448 < 3072.0f) {
        offX = -sn, offZ = -cs;
    }
    if (D_80364448 >= 3072.0f && D_80364448 < 4096.0f) {
        offX = -cs;
        offZ = sn;
    }
    switch (D_80364A90) {
        case 0x100:
            if (D_80364AC1) {
                if (D_80364412) {
                    D_8036509C = D_803FCD68 * 16;
                } else {
                    D_8036509C = (s16) ((u16) D_803FCD68 * 16 - D_8036509C) / 10 + (s16) D_8036509C;
                }
                D_803643EC = (s32) (D_803FCD48 - SINE(D_8036509C) * 8000.0f + offX) << 11;
                D_803643F0 = (D_803FCD4C + 0xDAC) << 11;
                D_803643F4 = (s32) (D_803FCD50 - COSINE(D_8036509C) * 8000.0f + offZ) << 11;
            } else {
                D_803643EC = (D_803EF6DC + 4000) << 11;
                D_803643F0 = (D_803EF6E0 + 25000) << 11;
                D_803643F4 = (D_803EF6E4 + D_80364A78 + 7000) << 11;
            }
            break;
        case 0x40:
            switch (D_80364AA8) {
                case 1:
                case 0x80:
                    if (D_803643DB) {
                        D_803643EC = (D_803EF6DC + offX) << 11;
                        D_803643F0 = (D_803EF6E0 + 2000) << 11;
                        D_803643F4 = (D_803EF6E4 + offZ) << 11;
                    }
                    if (D_80364AC1) {
                        if (D_80358060 < 2) {
                            D_8036509C = D_803FCD68 * 16;
                        } else {
                            D_8036509C = (s16) ((u16) D_803FCD68 * 16 - D_8036509C) / 10 + (s16) D_8036509C;
                        }
                        D_803643EC = (s32) (D_803FCD48 - SINE(D_8036509C) * 8000.0f + offX) << 11;
                        D_803643F0 = (D_803FCD4C + 0xDAC) << 11;
                        D_803643F4 = (s32) (D_803FCD50 - COSINE(D_8036509C) * 8000.0f + offZ) << 11;
                    }
                    if (D_803643DB || D_80364AC1) {
                        break;
                    }
                case 4:
                case 8:
                    if (!D_80364412) {
                        D_8036509C = (s16) ((u16) D_8036443E * 16 - D_8036509C) / 10 + (s16) D_8036509C;
                    } else {
                        D_8036509C = D_8036443E * 16;
                    }
                    D_803643EC = (s32) (D_803643E0 - SINE(D_8036509C) * (D_8036444C * 2.0f) + offX) << 11;
                    D_803643F0 = (D_8036444E + D_803643E4) << 11;
                    D_803643F4 = (s32) (D_803643E8 - COSINE(D_8036509C) * (D_8036444C * 2.0f) + offZ) << 11;
                    break;
                case 2:
                case 0x20:
                    if (!D_80364412) {
                        D_8036509C = (s16) ((u16) D_8036443E * 16 - D_8036509C) / 10 + (s16) D_8036509C;
                    } else {
                        D_8036509C = D_8036443E * 16;
                    }
                    D_803643EC = (s32) (SINE(D_8036509C) * (D_8036444C * 2.0f) + D_803643E0 + offX) << 11;
                    D_803643F0 = (D_8036444E + D_803643E4 + D_80370C2D * 50) << 11;
                    D_803643F4 = (s32) (COSINE(D_8036509C) * (D_8036444C * 2.0f) + D_803643E8 + offZ) << 11;
                    break;
                case 0x10:
                case 0x40:
                    D_803643EC = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk0 << 5) + offX) << 11;
                    D_803643F0 = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk2 << 5) + 2000) << 11;
                    D_803643F4 = ((D_8036BED8[D_8036BBB0[D_8036EA7C]].unk4 << 5) + offZ) << 11;
                    break;
            }
            break;
        case 0x200:
            if (D_80364AC1) {
                D_803643EC = (D_803FCD48 + offX - 6000) << 11;
                D_803643F0 = (D_803FCD4C + 0xD48) << 11;
                D_803643F4 = (D_803FCD50 + offZ + 6000) << 11;
            } else if (D_803643D6) {
                D_803643EC = (D_803EF6DC + offX + 16000) << 11;
                D_803643F0 = (D_803EF6E0 + 15000) << 11;
                D_803643F4 = (D_803EF6E4 + offZ - 3000) << 11;
            } else {
                D_803643EC = (D_803EF6DC + offX + 4000) << 11;
                D_803643F0 = (D_803EF6E0 + 2400) << 11;
                if (D_802E8BDC == 0) {
                    D_803643F0 += 0x5DC000;
                }
                D_803643F4 = (D_803EF6E4 + offZ - 4000) << 11;
            }
            break;
        case 0x400:
            if (D_80364AC1) {
                D_803643EC = (D_803FCD48 + offX + 6000) << 11;
                D_803643F0 = (D_803FCD4C + 0xD48) << 11;
                D_803643F4 = (D_803FCD50 + offZ - 6000) << 11;
            } else if (D_803643D6) {
                D_803643EC = (D_803EF6DC + offX - 16000) << 11;
                D_803643F0 = (D_803EF6E0 + 15000) << 11;
                D_803643F4 = (D_803EF6E4 - offZ + 3000) << 11;
            } else {
                D_803643EC = (D_803EF6DC + offX - 4000) << 11;
                D_803643F0 = (D_803EF6E0 + 2400) << 11;
                if (D_802E8BDC == 0) {
                    D_803643F0 += 0x5DC000;
                }
                D_803643F4 = (D_803EF6E4 + offZ + 4000) << 11;
            }
            break;
        case 1:
        case 0x800:
            if (D_80364AC1) {
                if (D_80358060 < 2) {
                    D_8036509C = D_803FCD68 * 16;
                } else {
                    D_8036509C = (s16) ((u16) D_803FCD68 * 16 - D_8036509C) / 10 + (s16) D_8036509C;
                }
                D_803643EC = (s32) (D_803FCD48 - SINE(D_8036509C) * 8000.0f + offX) << 11;
                D_803643F0 = (D_803FCD4C + 0xDAC) << 11;
                D_803643F4 = (s32) (D_803FCD50 - COSINE(D_8036509C) * 8000.0f + offZ) << 11;
            } else {
                D_803643EC = (D_803643E0 + offX - 500) << 11;
                D_803643F0 = (D_803643E4 + 6000) << 11;
                D_803643F4 = (D_803643E8 + offZ + 500) << 11;
            }
            break;
        case 0x1000:
            D_803643EC = (D_803643E0 + offX - 4000) << 11;
            D_803643F0 = (D_803643E4 + 5000) << 11;
            D_803643F4 = (D_803643E8 + offZ + 4000) << 11;
            break;
        default:
            if (D_80364410) {
                D_803643EC = D_80364404;
                D_803643F0 = D_80364408;
                D_803643F4 = D_8036440C;
            } else {
                if (D_80364AA8 == 0x40) {
                    D_8036444C = 9000;
                    D_8036444E = 11000;
                    D_80364420 = 11000;
                }
                if (D_80364AC1 && D_8036B8B0) {
                    D_803643EC = (D_803FCD48 + D_8036B8B4) << 11;
                    D_803643F0 = (D_803FCD4C + D_8036B8B8) << 11;
                    D_803643F4 = (D_803FCD50 + D_8036B8BC) << 11;
                } else if (D_803643DB && D_8036B8B0) {
                    D_803643EC = (D_803EF6DC + D_8036B8B4) << 11;
                    D_803643F0 = (D_803EF6E0 + D_8036B8B8) << 11;
                    D_803643F4 = (D_803EF6E4 + D_8036B8BC) << 11;
                } else if (D_8030F668) {
                    func_80255034(2500, D_80364414, &px, &pz);
                    D_803643EC = ((D_803F767C << 5) + px) << 11;
                    D_803643F0 = ((D_803F767E << 5) + 5000) << 11;
                    D_803643F4 = ((D_803F7680 << 5) + pz) << 11;
                } else if (D_8030F66A) {
                    func_80255034(2500, D_80364414, &px, &pz);
                    D_803643EC = (px + D_803F7670) << 11;
                    D_803643F0 = (D_803F7674 + 5000) << 11;
                    D_803643F4 = (pz + D_803F7678) << 11;
                } else if (D_8030F669) {
                    if (D_8036C794 != NULL) {
                        func_80255034(2500, D_80364414, &px, &pz);
                        D_803643EC = ((D_8036C794->unk12 + D_8036C794->unk0) / 2 << 16) + (px << 11);
                        D_803643F0 = ((D_8036C794->unk14 + D_8036C794->unk2) / 2 << 16) + 0x9C4000;
                        D_803643F4 = ((D_8036C794->unk16 + D_8036C794->unk4) / 2 << 16) + (pz << 11);
                    }
                } else {
                    if (D_80364A85) {
                        angle = (f32) (D_8036443E * 360 / 4096) - D_80364414 + 180.0f;
                        if (angle < 0) {
                            angle += 360;
                        }
                        if (angle >= 360) {
                            angle %= 360;
                        }
                        if (D_80364456 == 2 || D_80364456 == 9) {
                            func_80255034(D_8036444C, angle, &px, &pz);
                        } else {
                            func_80255034(D_8036444C / 2, angle, &px, &pz);
                        }
                        D_803643F0 = (D_80364420 * 3 / 5 + D_803643E4) << 11;
                        offX = 0;
                        offZ = 0;
                    } else {
                        func_80255034(D_8036444C, D_80364414, &px, &pz);
                        if (D_80364411 && D_80364414 >= 134.0 && D_80364414 <= 136.0) {
                            D_803643F0 = (D_803643E4 + 0x3908) << 11;
                        } else {
                            D_803643F0 = (D_803643E4 + D_80364420) << 11;
                        }
                    }
                    D_803643EC = (D_803643E0 + px + offX) << 11;
                    D_803643F4 = (D_803643E8 + pz + offZ) << 11;
                }
            }
            break;
    }
    switch (D_80364A90) {
        case 0x100:
            if (D_80364AC1) {
                D_80365078 = D_803FCD48 / 32.0f;
                D_8036507C = (D_803FCD4C + 2000) / 32.0f;
                D_80365080 = D_803FCD50 / 32.0f;
            } else {
                D_80365078 = D_803EF6DC / 32.0f;
                D_8036507C = D_803EF6E0 / 32.0f;
                D_80365080 = (D_803EF6E4 + D_80364A78 + 11000) / 32.0f;
            }
            break;
        case 0x200:
        case 0x400:
            if (D_80364AC1) {
                D_80365078 = (D_803FCD48 + offX) / 32.0f;
                D_8036507C = (D_803FCD4C + 2000) / 32.0f;
                D_80365080 = (D_803FCD50 + offZ) / 32.0f;
            } else {
                D_80365078 = (D_803EF6DC + offX) / 32.0f;
                D_8036507C = D_803EF6E0 / 32.0f;
                D_80365080 = (D_803EF6E4 + offZ) / 32.0f;
            }
            break;
        case 1:
        case 0x800:
        case 0x1000:
            if (D_80364AC1) {
                D_80365078 = D_803FCD48 / 32.0f;
                D_8036507C = (D_803FCD4C + 2000) / 32.0f;
                D_80365080 = D_803FCD50 / 32.0f;
            } else {
                D_80365078 = (D_803EF2EC + offX) / 32.0f;
                D_8036507C = D_803EF314 / 32.0f;
                D_80365080 = (D_803EF2F4 + offZ) / 32.0f;
            }
            break;
        case 0x40:
            if (D_80364AC1) {
                D_80365078 = D_803FCD48 / 32.0f;
                D_8036507C = (D_803FCD4C + 2000) / 32.0f;
                D_80365080 = D_803FCD50 / 32.0f;
                break;
            }
        default:
            if (D_80364AC1 && D_8036B8B0) {
                D_80365078 = D_803FCD48 / 32.0f;
                D_8036507C = (D_803FCD4C + 6000) / 32.0f;
                D_80365080 = D_803FCD50 / 32.0f;
            } else if (D_803643DB && D_8036B8B0) {
                D_80365078 = D_803EF6DC / 32.0f;
                D_8036507C = D_803EF6E0 / 32.0f;
                D_80365080 = D_803EF6E4 / 32.0f;
            } else if (D_8030F668) {
                D_80365078 = D_803F767C;
                D_8036507C = D_803F767E;
                D_80365080 = D_803F7680;
            } else if (D_8030F66A) {
                D_80365078 = D_803F7670 / 32.0f;
                D_8036507C = D_803F7674 / 32.0f;
                D_80365080 = D_803F7678 / 32.0f;
            } else if (D_8030F669) {
                if (D_8036C794 != NULL) {
                    D_80365078 = (f32) (D_8036C794->unk12 + D_8036C794->unk0) / 2.0;
                    D_8036507C = (f32) (D_8036C794->unk14 + D_8036C794->unk2) / 2.0;
                    D_80365080 = (f32) (D_8036C794->unk16 + D_8036C794->unk4) / 2.0;
                }
            } else {
                D_80365078 = (D_803643E0 + offX) / 32.0f;
                D_8036507C = (D_803643E4 + D_8030F664) / 32.0f;
                D_80365080 = (D_803643E8 + offZ) / 32.0f;
            }
            break;
    }
    if (D_803643D6 && D_802E8BDC == 0x31) {
        D_803643EC = (D_803EF6DC + offX + 16000) << 11;
        D_803643F0 = (D_803EF6E0 + 15000) << 11;
        D_803643F4 = (D_803EF6E4 + offZ - 3000) << 11;
    }
    if (D_80364AC1 && D_8036B965) {
        if (D_80364412) {
            D_8036509C = D_803FCD68 * 16;
        } else {
            D_8036509C = (s16) ((u16) D_803FCD68 * 16 - D_8036509C) / 10 + (s16) D_8036509C;
        }
        D_803643EC = (s32) (D_803FCD48 - SINE(D_8036509C) * 8000.0f + offX) << 11;
        D_803643F0 = (D_803FCD4C + 0xDAC) << 11;
        D_803643F4 = (s32) (D_803FCD50 - COSINE(D_8036509C) * 8000.0f + offZ) << 11;
        D_80365078 = D_803FCD48 / 32.0f;
        D_8036507C = (D_803FCD4C + 2000) / 32.0f;
        D_80365080 = D_803FCD50 / 32.0f;
    }
    if (D_80364A90 == 0x40 && D_802E8BDC == 0x3B) {
        D_803643EC = (D_803643E0 + 0x1900) << 11;
        D_803643F0 = (D_803643E4 + 0x3070) << 11;
        D_803643F4 = (D_803643E8 - 0x1900) << 11;
        D_80365078 = D_803643E0 / 32.0f;
        D_8036507C = D_803643E4 / 32.0f;
        D_80365080 = D_803643E8 / 32.0f;
    }
    if ((D_80364A90 & 0x2000000000002104) && !D_8036B8B0 && !inShuttle && D_80364A90 != 0x100 && D_802E8BDC != 0x23 &&
        !D_80364A85) {
        tx = D_80365078 * 65536.0;
        ty = D_8036507C * 65536.0;
        tz = D_80365080 * 65536.0;
        if (D_802E8BDC == 0x10 && D_80365078 > 8718.0f && D_80365078 < 8919.0f && D_80365080 > 9051.0f &&
            D_80365080 < 9251.0f) {
            D_802E8BE0 = 1.5f;
            rateY = 0.1f;
        }
        D_803643EC = (D_803643EC - tx) * D_802E8BE0 + tx;
        D_803643F0 = (D_803643F0 - ty) * D_802E8BE0 + ty;
        D_803643F4 = (D_803643F4 - tz) * D_802E8BE0 + tz;
    }
    if (!D_80364412) {
        D_803643F8 = (D_803643EC - D_803643F8) * rateX + D_803643F8;
        D_803643FC = (D_803643F0 - D_803643FC) * rateY + D_803643FC;
        D_80364400 = (D_803643F4 - D_80364400) * rateZ + D_80364400;
    } else {
        D_803643F8 = D_803643EC;
        D_803643FC = D_803643F0;
        D_80364400 = D_803643F4;
        D_803650A0 = !D_8036509E;
    }
    if (D_80364424 && (D_80364414 < 134.0 || D_80364414 > 136.0)) {
        func_802CE4F0(D_803643F8 >> 11, D_803643FC >> 11, D_80364400 >> 11);
        func_802CE5BC(D_803643F8 >> 11, D_803643FC >> 11, D_80364400 >> 11,
                      D_80364434 ? D_80364430 : D_80364430 + 1600, 0xBE, 0);
        if (D_80364434) {
            if (D_803A7424) {
                D_80364420 += D_8036442C;
                if (D_80364420 > D_80364428) {
                    D_80364420 = D_80364428;
                }
                D_80364434 = 1;
            } else {
                D_80364434 = 0;
            }
        } else if (D_803A7424) {
            D_80364434 = 1;
        } else {
            D_80364420 -= D_8036442C;
            if (D_80364420 < D_8036444E) {
                D_80364420 = D_8036444E;
            }
            D_80364434 = 0;
        }
    }
    if (!D_80364424) {
        D_80364420 = D_8036444E;
    } else if (D_80364414 >= 134.0 && D_80364414 <= 136.0) {
        D_80364420 -= D_8036442C;
        if (D_80364420 < D_8036444E) {
            D_80364420 = D_8036444E;
        }
    }
    if (D_8036509E != D_803650A0 || changed) {
        D_80365090 = D_80365084 - D_80365078;
        D_80365094 = D_80365088 - D_8036507C;
        D_80365098 = D_8036508C - D_80365080;
        D_803650A0 = D_8036509E;
        D_8030F660 = 0;
    }
    if (!D_8030F660 && !D_80364412) {
        D_80365090 = D_80365090 * 0.95;
        D_80365084 = D_80365090 + D_80365078;
        D_80365094 = D_80365094 * 0.97;
        D_80365088 = D_80365094 + D_8036507C;
        D_80365098 = D_80365098 * 0.95;
        D_8036508C = D_80365098 + D_80365080;
        if (D_80365090 < 0.1 && D_80365090 > -0.1 && D_80365098 < 0.1 && D_80365098 > -0.1 && D_80365094 < 0.1 &&
            D_80365094 > -0.1) {
            D_8030F660 = 1;
        }
    } else {
        D_80365084 = D_80365078;
        D_80365088 = D_8036507C;
        D_8036508C = D_80365080;
        D_8030F660 = 1;
    }
    if (D_802E8BE8 == 0) {
        D_802E8BE8 = 1;
    }
    if (D_802E8BE4 && (D_80364A90 & 0x2000100000000206)) {
        shakeX = D_803649D8 % D_802E8BE8 - (D_802E8BE8 >> 1);
        shakeY = D_803649D8 / 100 % D_802E8BE8 - (D_802E8BE8 >> 1);
        shakeZ = D_803649D8 / 10000 % D_802E8BE8 - (D_802E8BE8 >> 1);
    } else {
        shakeX = 0;
        shakeY = 0;
        shakeZ = 0;
    }
    if (D_802E8BE4) {
        D_802E8BE4--;
        D_802E8BE8 = D_802E8BE8 - D_802E8BE8 / 6;
        D_802E8BE8 = D_802E8BE8 + 1;
    }
    eyeX = (f32) D_803643F8 / 65536.0;
    eyeY = (f32) D_803643FC / 65536.0;
    eyeZ = (f32) D_80364400 / 65536.0;
    if ((D_80364A90 & 2) && D_802E8BF0 && D_80358060 > D_80366A04) {
        eyeX = (eyeX - D_80365084) * D_803649F8 + D_80365084;
        eyeY = (eyeY - D_80365088) * D_803649F8 + D_80365088;
        eyeZ = (eyeZ - D_8036508C) * D_803649F8 + D_8036508C;
        if (D_803649F8 > 0.4) {
            D_803649F8 = D_803649F8 - 0.01;
        }
    }
    atX = (D_80365084 * 32.0f + shakeX) / 32.0f;
    atZ = (D_8036508C * 32.0f + shakeZ) / 32.0f;
    adx = eyeX - atX;
    if (adx < 0.0) {
        adx = 0.0 - adx;
    }
    adz = eyeZ - atZ;
    if (adz < 0.0) {
        adz = 0.0 - adz;
    }
    if (adx > 0.5 || adz > 0.5) {
        guLookAtReflect(mtx, lookAt, eyeX, eyeY, eyeZ, (D_80365084 * 32.0f + shakeX) / 32.0f,
                        (D_80365088 * 32.0f + shakeY) / 32.0f, (D_8036508C * 32.0f + shakeZ) / 32.0f, 0.0f, 1.0f, 0.0f);
    } else {
        guLookAtReflect(mtx, lookAt, eyeX + 2.0, eyeY, eyeZ + 2.0, (D_80365084 * 32.0f + shakeX) / 32.0f,
                        (D_80365088 * 32.0f + shakeY) / 32.0f, (D_8036508C * 32.0f + shakeZ) / 32.0f, 0.0f, 1.0f, 0.0f);
    }
    dist = sqrtf((eyeX - D_80365084) * (eyeX - D_80365084) + (eyeZ - D_8036508C) * (eyeZ - D_8036508C));
    if (dist < 1.0) {
        dist = 1.0f;
    }
    if (eyeX >= D_80365084 && eyeZ >= D_8036508C) {
        D_80364452 = func_802AD7D4((eyeX - D_80365084) * 65535.9 / dist) >> 4;
        quadrant = 0;
    }
    if (eyeX >= D_80365084 && eyeZ < D_8036508C) {
        D_80364452 = (func_802AD7D4((D_8036508C - eyeZ) * 65535.9 / dist) >> 4) + 0x400; quadrant = 1;
    }
    if (eyeX < D_80365084 && eyeZ < D_8036508C) {
        D_80364452 = (func_802AD7D4((D_80365084 - eyeX) * 65535.9 / dist) >> 4) + 0x800; quadrant = 2;
    }
    if (eyeX < D_80365084 && eyeZ >= D_8036508C) {
        D_80364452 = (func_802AD7D4((eyeZ - D_8036508C) * 65535.9 / dist) >> 4) + 0xC00; quadrant = 3;
    }
    dist = sqrtf((eyeX - D_80365084) * (eyeX - D_80365084) + (eyeZ - D_8036508C) * (eyeZ - D_8036508C) +
                 (eyeY - D_80365088) * (eyeY - D_80365088));
    if (eyeY >= D_80365088) {
        pitch1 = func_802AD7D4((eyeY - D_80365088) * 65535.9 / dist) >> 3;
    } else {
        pitch1 = 0x1000 - (func_802AD7D4((D_80365088 - eyeY) * 65535.9 / dist) >> 3);
    }
    if (eyeY >= D_80365088) {
        pitch2 = func_802AD7D4((65535.0 < (eyeY - D_80365088) * 65535.9 / 20000.0) ? 65535.0 : (eyeY - D_80365088) * 65535.9 / 20000.0) >> 3;
    } else {
        pitch2 = 0x1000 - (func_802AD7D4((65535.0 < (D_80365088 - eyeY) * 65535.9 / 20000.0) ? 65535.0 : (D_80365088 - eyeY) * 65535.9 / 20000.0) >> 3);
    }
    D_80364454 = pitch1 - pitch2;
    scale = func_80254E54(eyeX, eyeY, eyeZ, D_80365084, D_80365088, D_8036508C);
    posX = (eyeX - D_80365084) * scale + D_80365084;
    posY = (eyeY - D_80365088) * scale + D_80365088;
    posZ = (eyeZ - D_8036508C) * scale + D_8036508C;
    if (adx > 0.5 || adz > 0.5) {
        guLookAt(view, posX, posY, posZ, D_80365084, D_80365088, D_8036508C, 0.0f, 1.0f, 0.0f);
    } else {
        guLookAt(view, posX + 2.0, posY, posZ + 2.0, D_80365084, D_80365088, D_8036508C, 0.0f, 1.0f, 0.0f);
    }
    D_80364412 = 0;
}

/* Speed limit for the current vehicle, scaled down past 10000 units of travel */
f32 func_80254E54(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1) {
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

/* Polar to cartesian: point at angle (degrees) on a circle of radius r*sqrt(2) */
void func_80255034(s32 r, f32 angle, s32 *outX, s32 *outY) {
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

/* Map screen: Z/R rotate the map 45 degrees at 3 degrees a frame */
void func_80255190(void) {
    u8 ok;

    if (D_802E8BD0 == 0) {
        ok = func_80255628();
        if (ok && ((D_80370C21 && !D_80370C27) || (D_80370C1E && !D_80370C24)) && (D_80364A90 & 0x104)) {
            func_80260650(D_80367738, 0xD0, NULL);
        }
        if (D_80370C21 && !D_80370C27 && !D_8036441C && !D_8036441D && !ok && D_80364A90 != 0x2000) {
            D_80364418 = D_80364414 + 45.0;
            if (D_80364418 >= 360.0) {
                D_80364418 = D_80364418 - 360.0;
            }
            D_8036441C = 1;
            if (D_80364A90 != 0x40) {
                func_80260650(D_80367738, 0xDD, NULL);
            }
        }
        if (D_8036441C) {
            if (D_80364414 < D_80364418) {
                D_80364414 = D_80364414 + 3.0;
                if (D_80364414 >= D_80364418) {
                    D_8036441C = 0;
                    D_80364414 = D_80364418;
                }
            } else {
                D_80364414 = D_80364414 + 3.0;
                if (D_80364414 >= 360.0) {
                    D_80364414 = D_80364414 - 360.0;
                    if (D_80364414 >= D_80364418) {
                        D_8036441C = 0;
                        D_80364414 = D_80364418;
                    }
                }
            }
        }
        if (D_80370C1E && !D_80370C24 && !D_8036441C && !D_8036441D && !ok && D_80364A90 != 0x2000) {
            D_80364418 = D_80364414 - 45.0;
            if (D_80364418 < 0.0) {
                D_80364418 = D_80364418 + 360.0;
            }
            D_8036441D = 1;
            if (D_80364A90 != 0x40) {
                func_80260650(D_80367738, 0xDC, NULL);
            }
        }
        if (D_8036441D) {
            if (D_80364414 > D_80364418) {
                D_80364414 = D_80364414 - 3.0;
                if (D_80364414 <= D_80364418) {
                    D_8036441D = 0;
                    D_80364414 = D_80364418;
                }
            } else {
                D_80364414 = D_80364414 - 3.0;
                if (D_80364414 < 0.0) {
                    D_80364414 = D_80364414 + 360.0;
                    if (D_80364414 <= D_80364418) {
                        D_8036441D = 0;
                        D_80364414 = D_80364418;
                    }
                }
            }
        }
        if (D_8036441D || D_8036441C) {
            func_802A45D4(2);
        }
    }
}

u8 func_80255628(void) {
    u8 ok = 0;

    if (D_80364A90 == 0x100000000000 || D_80364A90 == 2) {
        ok = 1;
    } else {
        switch (D_802E8BDC) {
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
        if (D_80364418 < 134.0 || D_80364418 > 136.0) {
            D_8036441D = 0;
            D_8036441C = 0;
            D_80364418 = 135.0f;
            if (D_80364418 < D_80364414) {
                if (D_80364414 - D_80364418 > 180.0) {
                    D_8036441C = 1;
                } else {
                    D_8036441D = 1;
                }
            } else {
                D_8036441C = 1;
            }
        }
    }
    return ok;
}

/* Clears the current frame buffer with a fill rectangle */
void func_802558C8(Gfx *arg0, s32 *len) {
    Gfx *gdl = arg0;

    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPSetFillColor(gdl++, 0x00010001);
    gDPPipeSync(gdl++);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    *len += gdl - arg0;
}

/* Closes the top-level display list and returns its length in *length */
void func_802559F8(Gfx *arg0, s32 *length) {
    Gfx *gdl = arg0;

    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    *length = gdl - D_803156F8[D_8035805C].dl;
    HD_ASSERT(*length<TOPLEVEL_DL_SIZE, 3665);
}

void func_80255AD0(void) {
    s32 i;
    f32 one;
    s32 pad[4];
    u8 status;

    one = 1.0f;
    D_80364A90 = 32;
    osCreateMesgQueue(&D_803150A0, D_803150B8, 50);
    osCreateMesgQueue(&D_80315180, D_80315198, 0x90);
    func_80270D20((struct BcSched *) D_80315440, D_80312D80 + 0x400, 13, (D_80000300 != 1) ? 0x10 : 2, 1);
    osCreateMesgQueue(&D_803153D8, D_803153F8, 16);
    func_80270E50((struct BcSched *) D_80315440, (struct BcScClient *) D_803156D8, &D_803153D8, 1, 1);
    status = func_8028A370();
    func_80261588();
    func_8029A7E4("audio inited\n");
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
    D_80358050[0] = PHYS(D_80000400[0]);
    D_80358050[1] = PHYS(D_80000400[1]);
    D_80358058 = PHYS(D_8021ED00);
    func_80284DB0();
    func_802D6710();
    func_8028FC10();
    if (!(status & 1)) {
        D_80364A98 = 0x0800000000000000;
    } else if (D_802FDBD0) {
        D_80364A98 = 0x0000080000000000;
    } else if (D_802FDBD4) {
        D_80364A98 = 0x0040000000000000;
    } else {
        D_80364A98 = 0x10;
    }
    for (i = 0x1FF; i >= 0; i--) {
        D_80310D80[i] = 0x1122334455667788LL;
    }
}

/* Checks the main thread's stack guard pattern from the top down */
void func_80255D34(void) {
    u8 bad;
    s32 i;

    bad = 0;
    for (i = 0x1FF; i >= 0 && !bad; i--) {
        if (D_80310D80[i] != 0x1122334455667788LL) {
            bad = 1;
            func_8029A7E4("stack end =%d\n", i);
        }
    }
}

/* One-time game init: memory, display lists, heap, subsystems */
void func_80255DC8(void) {
    u32 end;
    s32 pad;
    u32 size;
    s32 old;

    size = D_00788000 - D_00787F40;
    osViBlack(1);
    D_80364A70 = func_80261A44(D_80364A98);
    func_802D6710();
    osInvalDCache((void *) 0x80000000, 0x400000);
    D_803649F4 = 0;
    D_80358068 = 0;
    D_80358064 = 0;
    D_80358060 = 0;
    D_8035805C = 0;
    D_80367BD6 = 0;
    func_8026A8BC();
    func_8026A974();
    func_8028AE88();
    func_8028B720();
    D_8035806C = D_803FF600;
    func_8028B4C4((u32) D_00787F40, (u32) D_803FF600, &size, 10, 0, 2);
    end = (u32) (D_00788000 - D_00787F40) + (u32) D_803FF600;
    func_8029A7E4("Static end = 0x%x, space=0x%x (%d) bytes\n", end, 0x80400000 - end, 0x80400000 - end);
    D_80358078 = 0;
    func_802558C8(D_803156F8[D_8035805C].dl, &D_80358078);
    func_802559F8(D_803156F8[D_8035805C].dl, &D_80358078);
    D_80358070 = 0x8004B400;
    func_80257490(&D_80358070, 0x10);
    D_8036E694 = D_80358070;
    D_80358070 += 0xA000;
    if (D_802E8F94[D_802E8BDC].unk0 == 2 && !(D_80364A98 & 0x100000000002)) {
        func_8029A7E4("Allocating ghost buffer memory\n");
        D_80358070 += 0x20000;
    }
    D_803B9888 = 0;
    func_802A0700();
    D_803643C8 = (Vehicle *) (((u32) D_80358088 + 0x40) & ~0x3F);
    func_80278E3C();
    D_803643D9 = 0;
    D_803643DA = 0;
    D_803643D8 = 0;
    D_803643D6 = 0;
    D_803643D7 = 0;
    D_802E8BD8 = 0;
    D_802E8BD4 = 0;
    D_802E8BD0 = 0;
    D_8036EB99 = 0;
    D_803669B4 = 0;
    if (D_80364A98 & 0xC9FD8FE7FBFFC0B0) {
        func_8028B3E0();
    }
    func_80297530(D_802E8BDC);
    func_80272C50();
    if (D_80364A98 == 0x40000000000) {
        func_801F7850();
    }
    old = D_80358070;
    func_8026B118(0);
    func_8029A7E4("Yoshi windows allocated %d bytes, %x\n", D_80358070 - old, D_80358070);
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
    func_8028B4C4((u32) start, (u32) ((void *) dest), (u32 *) size, 12, 10, 1);
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
    D_80364AA8 = D_802E8F94[D_802E8BDC].unk0;
    D_802E8BE4 = 0;
    D_802E8BE8 = 0;
    D_803643E0 = 0;
    D_803643E4 = 0;
    D_803643E8 = 0;
    D_8036443C = 0;
    D_80364414 = 135.0f;
    D_80364418 = 135.0f;
    D_8036441C = 0;
    D_8036441D = 0;
    D_80364420 = 3000;
    D_80364434 = 1;
    D_803A7430 = 0;
    D_803649EE = 0;
    D_80364A84 = 0;
    D_80364456 = 0;
    if (D_80370C50) {
        HD_ASSERT(!pakBusy, 4162);
        HD_ASSERT(MQ_IS_EMPTY(&pakToGameMessageQ), 4163);
        func_80270ECC((struct BcSched *) D_80315440, (struct BcScClient *) D_80218EE0);
        func_802D67F0((OSThread *) D_80218D30);
    }
    D_8039CA62 = 0;
    D_8039CA61 = 0;
    if (D_80364AA8 == 2 && D_80364A98 == 0x2000) {
        if (!D_8039CA60) {
            func_80294E30();
        }
        func_80294E88();
        func_80294EB8();
    }
    func_8025615C(D_802E8BDC, D_80358070, &size);
    D_80358074 = (u8 *) D_80358070;
    D_80358070 += size;
    func_80257490(&D_80358070, 0x10);
    func_80285190();
    func_80275430();
    func_802621DC(D_802E8BDC);
    func_80262238(D_802E8BDC);
    func_80262150(D_802E8BDC);
    D_80367BFF = 0;
    func_802CE840();
    func_8029A7E4("enter initlevel game_mode=%d loop_done=%d\n", func_8026F92C(D_80364A90), func_8026F92C(D_80364A98));
    old = D_80358070;
    func_802A1674(D_80358074, arg0);
    func_8029A7E4("exit initlevel allocated %d bytes, %x\n", D_80358070 - old, D_80358070);
    func_80257234();
    if (D_80364A98 != 2) {
        if ((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) {
            func_802CF628();
        }
    }
    func_802C1DD0(D_80364AA8 == 0x20 || D_80364AA8 == 0x80);
    func_80262320(D_802E8BDC);
    if (D_80364410) {
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
    func_80281A70(D_802E8BDC);
    if (D_802E8F94[D_802E8BDC].unk0 == 1) {
        func_80264C20(arg0);
    }
    func_80288220();
    func_8027BE4C();
    func_80292240();
    func_8027E344(D_802E8BDC);
    func_802807D8(D_802E8BDC);
    func_80268664(D_802E8BDC);
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
        func_80258544(v, v->unk1004, v->unk1010, v->unk100C, v->unk1000, (Gfx *) (D_80364460[i].unk54), D_80364460[i].unk0,
                      D_80364460[i].unk4);
        func_80285110(0x61F);
        v++;
    }
    func_802729F0(D_80364A98, D_802E8BDC);
    D_80364452 = 0x2000;
    D_80364454 = 0x2000;
    D_80364456 = 0;
    D_803649ED = 0;
    D_803643F8 = 0;
    D_803643FC = 0;
    D_80364400 = 0;
    D_80364444 = 0.0f;
    D_80364448 = 0.0f;
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
        D_803649F0 = D_8036EA70.ip;
    } else {
        D_803649F0 = D_80364AF0[D_80364AE8].unk14;
    }
    D_80364A5C = 0;
    D_80364A58 = 0;
    if (D_803669B4) {
        func_8025BD98();
    }
    func_8029A7E4("Level %d: mem_pool=0x%x, code seg=0x%x, space=%d bytes\n", D_802E8BDC, D_80358070, 0x802447C0,
                  0x8021ED00 - D_80358070);
    if (D_8039CAB7) {
        func_802979E0(D_802E8BDC);
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

    switch (D_802E8BDC) {
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
    D_80358030[0] = D_80358070;
    D_80358070 += a * 8;
    D_80358038[0] = D_80358070;
    D_80358070 += b * 8;
    D_80358040[0] = D_80358070;
    D_80358070 += c * 8;
    D_80358048[0] = D_80358070;
    D_80358070 += d * 8;
    D_80358030[1] = D_80358070;
    D_80358070 += a * 8;
    D_80358038[1] = D_80358070;
    D_80358070 += b * 8;
    D_80358040[1] = D_80358070;
    D_80358070 += c * 8;
    D_80358048[1] = D_80358070;
    D_80358070 += d * 8;
}

/* Rounds *arg0 up to a multiple of arg1 */
void func_80257490(s32 *arg0, s32 arg1) {
    s32 r;

    r = *arg0 % arg1;
    if (r != 0) {
        r = arg1 - r;
    }
    *arg0 += r;
}


f32 func_802574F0(f32 arg0) {
    return sinf(arg0);
}


f32 func_80257514(f32 arg0) {
    return cosf(arg0);
}
