#include "common.h"
#include <ultra64.h>

extern s32 D_80222A1C;
extern s32 D_80222A20;
extern s32 D_802229E4;
extern u32 D_802229E8;
extern u32 D_802229EC;

#pragma GLOBAL_ASM("asm/nonmatchings/init/0E30/func_8021FB30.s")

extern s32 func_8021FB30(void);
s32 func_8022011C(s32 *);

#pragma GLOBAL_ASM("asm/nonmatchings/init/0E30/func_8022011C.s")

s32 func_80220268(void) {
    s32 sp24;
    s32 sp20;
    u32 sp1C;

    D_80222A20 = 0;
    D_802229E8 = 0;
    D_802229E4 = 0;
    sp1C = 0;
    for (;;) {
        D_802229EC = 0;
        sp20 = func_8022011C(&sp24);
        if (sp20 != 0) {
            return sp20;
        }
        if (D_802229EC > sp1C) {
            sp1C = D_802229EC;
        }
        if (sp24 != 0) {
            break;
        }
    }
    if (D_802229E8 >= 8) {
        do {
            D_802229E8 -= 8;
            D_80222A1C -= 1;
        } while (D_802229E8 >= 8);
    }
    D_80222A1C += 8;
    return 0;
}
