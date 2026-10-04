#include "common.h"
#include <ultra64.h>

extern void *D_80358070;
extern u8 D_006A8DA0[];
extern u8 D_006A9F10[];
extern u8 *D_8039CA90;
extern u8 *D_8039CA94;
extern u8 *D_8039CA98;
extern u8 *D_8039CA9C;
extern s16 D_8039CAA0;
extern u8 D_8039CAA2;
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);

/* Carves four buffers out of the heap and loads ROM asset 0x6A8DA0 into the first. */
void func_80295E50(void) {
    s32 size;

    D_8039CA90 = D_80358070;
    D_8039CA94 = D_8039CA90 + 0x3200;
    D_8039CA98 = D_8039CA94 + 0xF0;
    D_8039CA9C = D_8039CA98 + 0x180;
    size = D_006A9F10 - D_006A8DA0;
    func_8028B4C4(D_006A8DA0, D_8039CA90, &size, 10, 0, 1);
    D_80358070 = (u8 *) D_80358070 + size;
    D_8039CAA0 = 0;
    D_8039CAA2 = 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/51690/func_80295EFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/51690/func_8029700C.s")
