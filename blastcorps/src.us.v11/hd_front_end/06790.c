#include "common.h"
#include <ultra64.h>

/* front-end view of hd_code's Player record (0x100 bytes, D_80364AF0) */
typedef struct {
    u8 pad0[0xC];
    u8 unkC;
    u8 padD[0x100 - 0xD];
} FePlayer;

extern FePlayer D_80364AF0[];
extern u8 D_80364AE8; /* current player */
extern s32 D_80215960;
extern s32 D_80215964;
extern f32 D_80215968;
extern f32 D_8021596C;
extern s16 D_80215974;
extern s16 D_80215976;
extern s32 D_80215978;

void func_801F4E70();

/* Promotion screen init: load scene 0 and reset the screen's state. */
void func_801ED790(void) {
    func_801F4E70(0);
    D_80215960 = 0;
    D_80215964 = 0;
    D_80215968 = D_8021596C = 0.0f;
    D_80215974 = D_80215976 = 0;
    D_80215978 = D_80364AF0[D_80364AE8].unkC;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/06790/func_801ED800.s")
