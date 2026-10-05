#include "common.h"
#include <ultra64.h>

extern s16 D_80211A68;
extern u8 D_802153C0[];
extern u8 D_803643D4;
extern s32 D_80367738;
extern u8 D_80208194[];
extern f32 D_80211A70[];
extern f32 D_80208120[];
extern f32 D_802153D4;
extern f32 D_802153DC;
void func_80260A10(void);
void func_80260650(s32, s32, s32);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/00000/func_801E7000.s")

/* Select entry arg0: record it, set the current item from D_802153C0,
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
