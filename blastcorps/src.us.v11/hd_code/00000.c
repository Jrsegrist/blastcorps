#include "common.h"
#include <ultra64.h>

/* hd.c (from its assert strings): boot, the main game thread and the
 * frame loop. Owns .rodata at 0x80307B90.
 *
 * Variables defined here (the tell is a shared `lui` on paired stores).
 * Only part of hd.c's .bss is modelled so far: the section is placed at
 * 0x80310D80 by hd_code_bss.us.v11.ld, and the later variables get their
 * real addresses from undefined_syms (absolute symbols win). */
u64 D_80310D80[0x400]; /* main thread stack, filled with a guard pattern */
u64 D_80364A90;
u64 D_80364A98; /* game mode flags */

void func_8029A7E4(char *, ...); /* debug printf */
void func_802D4020(void);
void func_80270AE0(u32 *);
void func_802D4550(s32);
void func_802D4560(s32, void *, void *, s32);
void func_80244870(void *);
void func_80244930(void *);

extern OSThread D_80310820;
extern u64 D_803109D0[]; /* idle thread stack */
extern OSThread D_80310BD0;
extern u8 D_80314D80[];
extern u8 D_80314D98[];
extern s32 D_802FA254;
extern u32 D_803649F0;
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
    u8 pad58[4];
    s32 unk5C; /* id */
    u32 unk60; /* alpha */
    u8 pad64[0xC];
    s32 unk70; /* active */
} Unk74;
extern Unk74 D_80364460[];
extern Unk74 *D_803649D0;

/* one per frame buffer, 0x21498 bytes */
typedef struct {
    u8 pad0[0x240];
    Mtx unk240;
    Mtx unk280;
    u8 pad2C0[0x1500 - 0x2C0];
    Mtx unk1500;
    Mtx unk1540;
    u8 pad1580[0x18C0 - 0x1580];
    Vtx unk18C0[4]; /* shadow quad */
    u8 pad1900[0x48B0 - 0x1900];
    Gfx dl[0xB5E]; /* TOPLEVEL_DL_SIZE */
    u8 padA3A0[0x21498 - 0xA3A0];
} DynamicBuf;
extern DynamicBuf D_803156F8[];
extern u8 D_8035805C; /* current frame buffer index */

extern u8 D_00787F40[]; /* static code segment bounds (ROM) */
extern u8 D_00788000[];
extern u8 D_80364A70;
extern s32 D_80358068;
extern s32 D_80358064;
extern u32 D_80358060;
extern s16 D_80367BD6;
extern u8 D_803FF600[];
extern u8 *D_8035806C;
extern s32 D_80358078;
extern s32 D_80358070; /* heap pointer */
extern s32 D_802E8BDC; /* current level */
extern s32 D_8036E694;
typedef struct {
    u8 unk0;
    u8 pad1[0x43];
} LevelInfo;
extern LevelInfo D_802E8F94[];
extern u8 D_803B9888;
extern u8 D_80358088[];
typedef struct {
    u8 unk0[0x1000]; /* shadow texture, 64x64 IA8 */
    f32 unk1000;
    s32 unk1004; /* position, 1/32 units */
    s32 unk1008;
    s32 unk100C;
    s32 unk1010;
    u8 pad1014[4];
    s16 unk1018; /* shadow half-width */
    s16 unk101A; /* shadow half-depth */
    u8 pad101C[6];
    u8 unk1022; /* id */
    u8 pad1023[0x1D];
} Vehicle; /* 0x1040 bytes */
extern Vehicle *D_803643C8;
extern u8 D_803643D9;
extern u8 D_803643DA;
extern u8 D_803643D8;
extern u8 D_803643D6;
extern u8 D_803643D7;
extern u8 D_802E8BD8;
extern u8 D_802E8BD4;
extern u8 D_802E8BD0;
extern u8 D_8036EB99;
extern s32 D_803669B4;
extern u8 D_8039CAA2;

u8 func_80261A44(u64);
void func_802D6710(void);
void func_8026A8BC(void);
void func_8026A974(void);
void func_8028AE88(void);
void func_8028B720(void);
void func_8028B4C4(void *, void *, u32 *, s32, s32, s32);
void func_802558C8(Gfx *, s32 *);
void func_802559F8(Gfx *, s32 *);
void func_80257490(s32 *, s32);
void func_802A0700(void);
void func_80278E3C(void);
void func_8028B3E0(void);
void func_80297530(s32);
void func_80272C50(void);
void func_801F7850(void);
void func_8026B118(s32);
void func_8028A42C(void);
void func_802592F0(void);

#define TOPLEVEL_DL_SIZE 0xB5E

/* Rare's assert; line numbers are the original hd.c's */
#define HD_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "hd.c", line)

extern s32 D_80000300; /* osTvType */
extern u32 D_80358050[]; /* frame buffer physical addresses */
typedef struct {
    u8 pad0[0x10];
    u32 unk10;
    s32 unk14;
    u8 unk18[0xD8]; /* per level */
    u32 unkF0; /* vehicle flags */
    u8 padF4[0xC];
} Player; /* 0x100 bytes */
extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern u8 D_803649ED;
extern u8 D_80364456;
extern s32 D_803649E8;
extern u8 D_803649EC;
extern u8 D_803649EE;
extern s32 D_803156F0;
extern s32 D_80367738;
s32 func_802AB878(u8);
void func_802AB478(u8);
void func_8028F6B4(u8);
void func_80291ED8(u8);
void func_802794E4(void);
s32 func_8024AFA8(s32);
void func_802AE860(void);
s32 func_8026AD30(s32);
s32 func_80260634(s32);
void func_80260650(s32, s32, s32 *);

extern OSMesgQueue D_803150A0;
extern OSMesg D_803150B8[];
extern OSMesgQueue D_80315180;
extern OSMesg D_80315198[];
extern u64 D_80312D80[];
extern u8 D_80315440[];
extern OSMesgQueue D_803153D8;
extern OSMesg D_803153F8[];
extern u8 D_803156D8[];
extern u16 D_80000400[][320 * 240];
extern u8 D_8021ED00[];
extern u32 D_80358058;
extern u8 D_802FDBD0;
extern u8 D_802FDBD4;
void func_80270D20(void *, void *, s32, s32, s32);
void func_80270E50(void *, void *, OSMesgQueue *, s32, s32);
u8 func_8028A370(void);
void func_80261588(void);
void func_80284DB0(void);
void func_8028FC10(void);
extern s32 D_803643E0;
extern s32 D_803643E8;
extern f32 D_80364418;
extern f32 D_80364414;
extern u8 D_8036441C;
extern u8 D_8036441D;
s32 func_802AC4C4(s32, s32, s32, s32, s32, s32, s32, s32);
#define PHYS(x) ((u32)(x) & 0x1FFFFFFF)

extern u8 D_80370C1E;
extern u8 D_80370C21;
extern u8 D_80370C24;
extern u8 D_80370C27;
u8 func_80255628(void);
void func_802A45D4(s32);

extern u16 D_8035807C;
extern s32 D_80358074;
extern Gfx D_01000010[]; /* segment 1 */
void func_802A467C(s32, Gfx *, Vtx *, s32);

extern s32 D_803643E4;
extern s32 D_80364AA8;
extern f32 D_80364AB4;
extern f32 D_80364AB8;
extern f32 D_80364ABC;
extern u8 D_80364AC0;
void func_802AC61C(s32, s32, s32, s32, s32);

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8;
    u8 unk9;
} Message; /* 0xC bytes */
extern Message D_80364A00[5];
extern s16 D_8036443E;
extern u8 D_80364A3C;
extern u8 D_80364A3D;
extern s32 D_80364A40;
extern s32 D_80364A44;
extern u8 D_80364A48;
extern s16 D_80364A4A;
extern s16 D_80364A4C;
extern u8 D_80364A4E;

extern u8 D_803156F5;
extern u8 D_80364A84;
extern u8 D_80364AC1;
extern u8 D_803EF6FF;
extern Mtx D_02000000[];

extern s16 D_803EF326;

extern Vehicle *D_803643CC;
extern u8 D_80218D30[];
extern u8 D_80218EE0[];
extern OSMesgQueue D_80219F50;
#define pakToGameMessageQ D_80219F50
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern u8 D_802E8BF4[];
extern u8 D_802E8BF8;
extern u8 D_802FDB14;
extern s32 D_803643F8;
extern s32 D_803643FC;
extern s32 D_80364400;
extern u8 D_80364410;
extern s32 D_80364420;
extern u8 D_80364434;
extern s16 D_8036443C;
extern f32 D_80364444;
extern f32 D_80364448;
extern s16 D_80364452;
extern s16 D_80364454;
extern f32 D_803649F8;
extern s32 D_80364A58;
extern s32 D_80364A5C;
extern u8 D_80364A68;
extern u8 D_80364A69;
extern u8 D_80364A6A;
extern u8 D_80364A6B;
extern u8 D_80364A6C;
extern u8 D_80364A6D;
extern u8 D_80364A6F;
extern u8 D_80364A86;
extern u8 D_80367BFF;
extern s32 D_8036EA70;
extern u8 D_80370C50;
extern u8 D_8039C4B0;
#define pakBusy D_8039C4B0
extern u8 D_8039CA60;
extern u8 D_8039CA61;
extern u8 D_8039CA62;
extern u8 D_8039CAB7;
extern u8 D_803A7430;
extern s32 D_803ED808;
extern s32 D_803ED80C;
extern s32 D_803ED810;
void func_80270ECC(void *, void *);
void func_802D67F0(void *);
void func_80294E30(void);
void func_80294E88(void);
void func_80294EB8(void);
void func_8025615C(s32, s32, s32 *);
void func_80285190(void);
void func_80275430(void);
void func_802621DC(s32);
void func_80262238(s32);
void func_80262150(s32);
void func_802CE840(void);
s32 func_8026F92C(u64);
void func_802A1674(s32, s32);
void func_80257234(void);
void func_802CF628(void);
void func_802C1DD0(s32);
void func_80262320(s32);
u8 func_80272C5C(void *, s32, s32, s32, s32, f32);
void func_802775C0(void);
void func_80286A00(void);
void func_802873AC(void);
void func_80287AE4(void);
void func_802821D0(void);
void func_80282728(void);
void func_80281A70(s32);
void func_80264C20(s32);
void func_80288220(void);
void func_8027BE4C(void);
void func_80292240(void);
void func_8027E344(s32);
void func_802807D8(s32);
void func_80268664(s32);
void func_8026A988(void);
void func_80258544(Vehicle *, s32, s32, s32, f32, s32, void *, void *);
void func_80285110(s32);
void func_802729F0(s32, s32);
void func_8025BD98(void);
void func_802979E0(s32);
void func_802A56C4(void);
void func_802A5FA8(void);

extern u8 D_803ED3F5;
void func_802B01DC(void);
void func_802B1150(void);
void func_802B2EF8(void);
void func_802B45FC(void);
void func_802B5F04(void);
void func_802BB170(void);
void func_802BBE10(void);
void func_802B76F8(void);
void func_802C5508(void);
void func_802CA140(void);
void func_802C8AF0(void);
void func_802CBB60(void);
void func_802CCCD8(void);
void func_802CFA58(void);
void func_802D0B90(void);
void func_802B5FAC(void);
void func_802B02A0(void);
void func_802B46C4(void);
void func_802B77A0(void);
void func_802CBC08(void);
void func_802CCD80(void);
void func_802CFB00(void);
void func_802C5860(void);
void func_802B2FA0(void);
void func_802AEEC8(void);
void func_802B03F4(void);
void func_802B152C(void);
void func_802B327C(void);
void func_802B49AC(void);
void func_802B6294(void);
void func_802BB274(void);
void func_802BBEB8(void);
void func_802B7A88(void);
void func_802C5AFC(void);
void func_802CA4E0(void);
void func_802C8BB8(u8);
void func_802CBEF0(void);
void func_802CD068(void);
void func_802CFDE8(void);
void func_802D0F98(void);

void func_802AFFD4(void);
void func_802B1228(void);
void func_802B2D7C(void);
void func_802B448C(void);
void func_802B5CD8(void);
void func_802BB054(void);
void func_802BBDC8(void);
void func_802B76AC(void);
void func_802C5714(void);
void func_802C9F54(void);
void func_802C8AB0(void);
void func_802CBA94(void);
void func_802CCC8C(void);
void func_802CFA0C(void);
void func_802D0C68(void);
void func_8025BBE8(s32, s32, s32);

extern u8 D_803BE739;
extern s32 D_80358030[2];
extern s32 D_80358038[2];
extern s32 D_80358040[2];
extern s32 D_80358048[2];
extern s32 D_803156EC;
u8 func_8024B4B8(void);
s32 func_8024B418(u8);
u8 func_802AE888(s32);
void func_8028F93C(void);
void func_80292084(void);
void func_802B4658(void);
void func_802B2F54(void);
void func_802B5F60(void);
void func_802B11B8(void);
void func_802B0254(void);
void func_802BBE2C(void);
void func_802B7754(void);
void func_802C5688(void);
void func_802CA1AC(void);
void func_802C8B0C(u8);
void func_802CBBBC(void);
void func_802CCD34(void);
void func_802BB1A0(void);
void func_802CFAB4(void);
void func_802D0BF8(void);

extern u8 D_803153F0;
extern s32 D_803156E8;
extern s32 D_803EF6DC;
extern s32 D_803EF6E4;
extern s16 currentYoshiWindow;
extern s16 yoshiState;
void func_802608C8(s32);
void func_80260DFC(void);
void func_80261040(void);
s32 func_8026A610(s32, s32, s32, s32);
void func_8026AF6C(s32);
s32 func_8026B10C(void);
void func_80277EDC(s32, s32, s32, s32);
s32 func_802C1AA0(void);

extern u8 D_803643DB;
extern u8 D_80364412;
typedef struct {
    u8 pad0[0xC4];
    u16 unkC4;
    u8 padC6[0x16];
    u8 unkDC;
} Unk802F5804;
extern Unk802F5804 D_802F5804[];
typedef struct {
    u8 pad0[0xB0];
    s32 unkB0;
    u8 padB4[0xD0];
    s16 unk184;
} Unk802F8BDC;
extern Unk802F8BDC D_802F8BDC[];
void func_80260D7C(f32);
f32 func_80260DF0(void);
void func_80275270(u64, f32);
void func_80285EF4(s32);
void func_8028B240(void);

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
extern u8 D_0066C900[]; /* worldtextures */

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
    func_80270AE0(buf);
    osCreateThread(&D_80310820, 1, func_80244870, NULL, D_803109D0 + 0x40, 10);
    osStartThread(&D_80310820);
}

/* Idle thread: starts the managers and the main thread, then spins */
void func_80244870(void *arg0) {
    s32 pad;

    func_802D4550(4);
    func_802D4560(0x96, D_80314D80, D_80314D98, 0xC2);
    osCreateThread(&D_80310BD0, 3, func_80244930, arg0, D_80310D80 + 0x400, 10);
    osStartThread(&D_80310BD0);
    if (D_802FA254 == 0) {
        osStartThread(&D_80310BD0);
    }
    osSetThreadPri(NULL, 0);
    while (1) {
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80244930.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802475D8.s")

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
                        func_802608C8(D_803156E8);
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
                func_802608C8(D_803156E8);
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
        } else if (!func_80260634(D_803156F0)) {
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
            func_802BBDC8();
            break;
        case 8:
            func_802B76AC();
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
            func_802C8AB0();
            break;
        case 13:
            func_802CBA94();
            break;
        case 14:
            func_802CCC8C();
            break;
        case 15:
            func_802CFA0C();
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

/* Leaves the current vehicle */
void func_8024B188(void) {
    u8 res = 0;
    u8 ok = 0;

    res = func_8024B4B8();
    if (res == 1) {
        ok = func_802AE888(((s32 *) &D_802E8BF8)[D_80364456]);
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
        if (!func_80260634(D_803156EC)) {
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

void func_802AB670(s32 arg0);
extern u8 D_803ED3F5;
extern u8 D_80364456;

void func_8024B5E8(void) {
    D_803ED3F5 = 1;
    func_802AB670(D_80364456);
}

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
                        func_802B02A0();
                        break;
                    case 4:
                        func_802B46C4();
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
                        func_802CFB00();
                        break;
                    case 9:
                        func_802C5860();
                        break;
                    case 3:
                        func_802B2FA0();
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
            func_802B03F4();
            break;
        case 2:
            func_802B152C();
            break;
        case 3:
            func_802B327C();
            break;
        case 4:
            func_802B49AC();
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
            func_802CFDE8();
            break;
        case 16:
            func_802D0F98();
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
    func_802A467C(D_80358074, dl, v, (s32) (gdl - dl) * sizeof(Gfx));
}

/* Handles a Yoshi (menu) selection *sel for the current game mode */
void func_8024BDA4(u16 *sel) {
    switch (D_80364A90) {
        case 4:
        case 0x100:
            switch (*sel) {
                case 1:
                    if (D_80364456 == 6 || D_80364456 == 11 || D_80364456 == 17 || D_80364456 == 18) {
                        D_802F5804[0].unkC4 &= ~1;
                        D_802F5804[0].unkDC = 8;
                    } else {
                        D_802F5804[0].unkC4 |= 1;
                        D_802F5804[0].unkDC = 7;
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
                        D_802F8BDC[0].unk184 = 40;
                    } else {
                        D_802F8BDC[0].unk184 = 39;
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
                    D_802F8BDC[0].unkB0 |= 0x80;
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

/* HUD text drawn by func_8024C414 (still asm, which loads these by address) */
const char D_8030821C[] = "PRESS START";
const char D_80308228[] = "USE Z/R TO TURN PAGES";
const char D_80308240[] = "USE Z/R TO MOVE MAP";
const char D_80308254[] = "SHUTTLE VIEW";
const char D_80308264[] = "MISSILE VIEW";

void *func_8024C404(void *arg0, s32 arg1, s32 *arg2) {
    *arg2 = 0;
    return arg0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024C414.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024E4F4.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802507C8.s")

/* Speed limit for the current vehicle, scaled down past 10000 units of travel */
f32 func_80254E54(f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1) {
    f32 speed;
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
    func_80270D20(D_80315440, D_80312D80 + 0x400, 13, (D_80000300 != 1) ? 0x10 : 2, 1);
    osCreateMesgQueue(&D_803153D8, D_803153F8, 16);
    func_80270E50(D_80315440, D_803156D8, &D_803153D8, 1, 1);
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
    func_8028B4C4(D_00787F40, D_803FF600, &size, 10, 0, 2);
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
    func_8028B4C4(start, (void *) dest, (u32 *) size, 12, 10, 1);
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
        func_80270ECC(D_80315440, D_80218EE0);
        func_802D67F0(D_80218D30);
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
    D_80358074 = D_80358070;
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
        D_80364A86 = func_80272C5C(D_802E8BF4, 0, 1, 1, 1, 1.0f);
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
        func_80258544(v, v->unk1004, v->unk1010, v->unk100C, v->unk1000, D_80364460[i].unk54, D_80364460[i].unk0,
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
        D_803649F0 = D_8036EA70;
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
    s32 a;
    s32 b;
    s32 c;
    s32 d;

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


void func_802574F0(f32 arg0) {
    sinf(arg0);
}


void func_80257514(f32 arg0) {
    cosf(arg0);
}
