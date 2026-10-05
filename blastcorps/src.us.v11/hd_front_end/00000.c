#include "common.h"
#include <ultra64.h>

/* digger_loop.c: the vehicle ("digger") select screen. */

typedef struct {
    u8 pad0[0x2C];
    u32 unk2C; /* mask of vehicles available on this level */
    u8 pad30[0x14];
} LevelInfo; /* 0x44 bytes */
extern LevelInfo D_802E8F94[];

typedef struct {
    u8 pad0[0x10];
    u32 unk10; /* mask of vehicles unlocked */
    u8 pad14[4];
    u8 unk18[0x7A]; /* per level */
    u8 unk92[0x5E]; /* per level: last vehicle used */
    u8 padF0[0x10];
} Player; /* 0x100 bytes */
extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern u8 D_80364AEA;
extern u64 D_80364A90;
extern s32 D_802E8BDC;
extern u8 D_802E8C44[];
extern u16 D_80364EF0[][16];
extern u8 D_803643D4;
extern s32 D_80367738;
extern f32 D_802FDAC0[];

typedef struct {
    u8 unk0;
    f32 unk4;
} Struct80208060;
extern u8 D_8020804C[];
extern Struct80208060 D_80208060[];
extern u8 D_802080F8[];
extern u8 D_8020810C[];
extern s16 D_8020816C[];
extern u8 D_80208194[];
extern f32 D_80208120[];
extern s32 D_80210E90[];
extern u8 D_80210EE0[][8];
extern u8 D_80210F78[][16];
extern Mtx D_802110A8[];
extern Mtx D_80211568[];
extern Mtx D_80211A28;
extern s16 D_80211A68;
extern s16 D_80211A6A;
extern f32 D_80211A70[];
extern u8 D_80211AC0[][0x300];
extern u8 D_802153C0[];
extern f32 D_802153D4;
extern f32 D_802153D8;
extern f32 D_802153DC;
extern f32 D_802153E0;
extern s16 D_802153E4;
extern s16 D_802153E6;
extern s32 D_802153E8;

void func_8029A7E4(const char *, ...);
void func_8029DEA0(void);
void func_80202100(s32, void *, void *, void *);
void func_80202270(s32, void *, void *);
void func_802022EC(void *, u8, u8, u8, f32, u8, s32);
void func_80202380(s32);
void func_801E74E8(u8);
void func_80260A10(void);
void func_80260650(s32, s32, s32);

/* Rare's assert; line numbers are the original digger_loop.c's */
#define DIGGER_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "digger_loop.c", line)

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
            func_80202100(i, &D_80210E90[D_80211A6A], D_80210EE0[D_80211A6A], D_80210F78[D_80211A6A]);
            D_80211A70[i] = x;
            x += 380.0;
            guTranslate(&D_802110A8[D_80211A6A], D_80211A70[i], D_8020816C[i], 0.0f);
            guScale(&D_80211568[D_80211A6A], D_802FDAC0[i], D_802FDAC0[i], D_802FDAC0[i]);
            func_80202270(D_80210E90[D_80211A6A], D_80210EE0[D_80211A6A], D_80211AC0[D_80211A6A]);
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/00000/func_801E7598.s")
