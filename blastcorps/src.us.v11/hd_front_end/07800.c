#include "common.h"
#include <ultra64.h>

/* front-end view of hd_code's Player record (0x100 bytes, D_80364AF0) */
typedef struct {
    u8 pad0[0xC];
    u8 unkC;
    u8 padD[0x54 - 0xD];
    u8 unk54[0x3C]; /* per level */
    u8 pad90[0x100 - 0x90];
} FePlayer;

typedef struct {
    u8 pad0[0x18];
    s8 unk18[0x18]; /* -1 terminated */
} FeLevelEntry;     /* 0x30 bytes, one per level */

typedef struct {
    u8 pad0[0x30];
    u16 unk30; /* time thresholds, best first */
    u16 unk32;
    u16 unk34;
    u16 unk36;
    u8 pad38[0xC];
} LevelInfo;

extern FePlayer D_80364AF0[];
extern u8 D_80364AE8;  /* current player */
extern s32 D_802E8BDC; /* current level */
extern FeLevelEntry D_8020D810[];
extern LevelInfo D_802E8F94[];
extern s32 D_80358070; /* heap pointer */
extern s16 D_802159D0;
extern s32 D_802159D4;
extern s32 D_802159D8;
extern s16 D_802159DC;
extern f32 D_802159E0;
extern f32 D_802159E4;

/* ROM bounds of two compressed blobs (the first ends where the second starts) */
extern u8 D_0048F5A0[];
extern u8 D_0048F5A0_end[];
extern u8 D_0048F970[];
extern u8 D_0048F970_end[];

void func_801F4E70();
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/07800/func_801EE800.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/07800/func_801EEDB4.s")

/* Count the current level's entries (at most 2) whose bit is set for the player. */
s8 func_801EF1E0(void) {
    FeLevelEntry *e;
    s32 i;
    s32 count;

    e = &D_8020D810[D_802E8BDC];
    if (e->unk18[0] == -1) {
        return -1;
    }
    for (i = 0, count = 0; e->unk18[i] != -1 && i < 2; i++) {
        if (D_80364AF0[D_80364AE8].unk54[D_802E8BDC] & (1 << i)) {
            count++;
        }
    }
    return count;
}

/* Grade a time against level `arg1`'s thresholds: 4 (best, needs arg2 >= 12) .. 1, else 5. */
u8 func_801EF2BC(u16 arg0, u8 arg1, u8 arg2) {
    u8 ret;
    LevelInfo *l;

    l = &D_802E8F94[arg1];
    if (l->unk30 >= arg0 && arg2 >= 12) {
        ret = 4;
    } else if (l->unk32 >= arg0) {
        ret = 3;
    } else if (l->unk34 >= arg0) {
        ret = 2;
    } else if (l->unk36 >= arg0) {
        ret = 1;
    } else {
        ret = 5;
    }
    return ret;
}

/* Results screen init: load scene `arg0` and inflate the two blobs onto the heap. */
void func_801EF380(s32 arg0) {
    s32 size1;
    s32 size2;

    size1 = D_0048F5A0_end - D_0048F5A0;
    size2 = D_0048F970_end - D_0048F970;
    func_801F4E70(arg0);
    if (arg0 == 2) {
        D_802159D0 = 90;
    } else {
        D_802159D0 = 0;
    }
    func_8028B4C4(D_0048F5A0, (void *) D_80358070, &size1, 12, 0, 1);
    D_802159D4 = D_80358070;
    D_80358070 += size1;
    func_8028B4C4(D_0048F970, (void *) D_80358070, &size2, 12, 0, 1);
    D_802159D8 = D_80358070;
    D_80358070 += size2;
    D_802159DC = arg0;
    D_802159E0 = 0.0f;
    D_802159E4 = 3.0f;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/07800/func_801EF4AC.s")
