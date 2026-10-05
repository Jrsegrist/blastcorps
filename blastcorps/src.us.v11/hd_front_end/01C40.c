#include "common.h"
#include <ultra64.h>

/* player.c (per its assert strings): save slots, player ranks and the
 * player select / Controller Pak screens */

/* One entry per level in D_802E8F94 (0x44 bytes), as in hd_code 1D990.c */
typedef struct {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 pad1[0x43];
} LevelInfo;

/* One per save slot (D_80364AF0, 0x100 bytes), as in hd_code 26570.c */
typedef struct {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ u8 rank[0x3C]; /* per level: 1-5 when done */
    /* 0x54 */ u8 pad54[0x91 - 0x54];
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 pad92[0x100 - 0x92];
} Player;

extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern LevelInfo D_802E8F94[];
extern s32 D_802E8BDC;
extern u64 D_80364A98;

extern s16 D_802154D2;
extern s32 D_802154DC;
extern u8 D_80215900[];
extern u8 D_80215902[];
extern s32 D_80215908[];
extern s16 D_80215910[];
extern u8 D_80215914;
extern u16 D_80215930[];
extern u16 D_802158A8[];
extern u16 D_802E8C94[];
extern u16 D_802E8C98[];
extern u8 D_8021592E;
extern s32 D_802154D8;
extern f32 D_802154E4;
extern s32 D_802154E8;
extern s32 D_802154F0[];
extern u16 D_802082D8[];
extern u16 D_802082E4[];
extern u16 D_802082E8[];

u8 func_80272C5C(u16 *ids, u16 *palIds, s32 count, s32 frames, s32 flags, f32 scale);
void func_801E8EB8(u8, s32);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8C40.s")

void func_801E8DCC(u8 arg0) {
    s32 i;

    func_801E8EB8(arg0, 0);
    for (i = 0; i < 0x1B; i++) {
        D_802158A8[i] = D_802E8C94[D_8021592E];
    }
    D_802158A8[i] = D_802E8C98[D_8021592E];
    D_802154D8 = 0;
    D_802154E8 = 9999;
    for (i = 1; i < 5; i++) {
        D_802154F0[i] = 9999;
    }
    D_802154E4 = 3.0f;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E8EB8.s")

/* Count the player's ranks per grade (D_80215930[1..4]) and the finished
 * levels of the 0x81 types, other than levels 0x26, 0x2F and 0x31 ([3]) */
void func_801E93DC(u8 arg0) {
    s32 i;
    Player *p;

    p = &D_80364AF0[arg0];
    for (i = 0; i < 6; i++) {
        D_80215930[i] = 0;
    }
    for (i = 0; i < 0x3C; i++) {
        if (p->rank[i] > 0 && p->rank[i] < 5) {
            D_80215930[p->rank[i]]++;
        }
        if (((D_80364AF0[arg0].rank[i] > 0 && D_80364AF0[arg0].rank[i] < 6) ? 1 : 0) && (D_802E8F94[i].type & 0x81) &&
            i != 0x31 && i != 0x2F && i != 0x26) {
            D_80215930[3]++;
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E9528.s")

s32 func_801E96F8(void) {
    return D_802154D2 == D_802154DC + 8;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801E9718.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA108.s")

void func_801EA268(s32 *arg0) {
    arg0[4] = 0x1063E;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA278.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA4B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA6E8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EA93C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EAA7C.s")

void func_801EC288(u8 arg0) {
    s32 i;

    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    for (i = 0; i < 2; i++) {
        D_80215900[i] = 4;
        D_80215908[i] = 0x32;
        D_80215910[i] = 0;
    }
}

void func_801EC30C(u8 arg0) {
    s32 j;
    s32 i;

    D_80215914 = func_80272C5C(D_802082E4, D_802082D8, 4, 2, 0, 1.0f);
    func_80272C5C(D_802082E8, 0, 1, 2, 1, 1.0f);
    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    if (D_802E8F94[D_802E8BDC].type == 1) {
        j = 0;
    } else {
        j = 1;
    }
    for (i = 0; j < 2; j++, i++) {
        D_80215900[j] = 0;
        D_80215908[j] = i * 60 + 0x28;
        D_80215910[j] = 0;
    }
}

void func_801EC464(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        D_80215900[i] = 3;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC49C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801EC770.s")

/* Level-select mask bit for a level: done, of a 0x81 type, or other */
u64 func_801ECA50(u8 level) {
    u64 mask;

    if ((D_80364AF0[D_80364AE8].rank[level] > 0 && D_80364AF0[D_80364AE8].rank[level] < 6) ? 1 : 0) {
        mask = 0x80;
    } else if (D_802E8F94[level].type & 0x81) {
        mask = 0x800;
    } else {
        mask = 0x20000000;
    }
    return mask;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECB18.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECC8C.s")

/* Retype the 0x81-type levels: 0x80 once unk91 >= 11 in mode 0x4000, else 1 */
void func_801ECE9C(void) {
    s32 i;

    for (i = 0; i < 0x3C; i++) {
        if (D_802E8F94[i].type & 0x81) {
            if (D_80364AF0[D_80364AE8].unk91 >= 11 && D_80364A98 == 0x4000) {
                D_802E8F94[i].type = 0x80;
            } else {
                D_802E8F94[i].type = 1;
            }
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ECF5C.s")

void func_801ED480(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 0x20; i++) {
        dst[i] = src[i];
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/01C40/func_801ED4B8.s")
