#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027D810.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027D8F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027DA10.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027DB5C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027DD88.s")

s32 func_802AC4C4(s32, s32, s32, s32, s32, s32, s32, s32);

s32 func_8027E164(s32 arg0, s32 arg1, void *arg2, void *arg3) {
    if (func_802AC4C4(arg0, arg1, *(s32 *)((u8 *) arg2 + 0x0), *(s32 *)((u8 *) arg3 + 0x0),
                       *(s32 *)((u8 *) arg2 + 0x4), *(s32 *)((u8 *) arg3 + 0x4),
                       *(s32 *)((u8 *) arg2 + 0x8), *(s32 *)((u8 *) arg3 + 0x8)) != 0) {
        return 1;
    }
    if (func_802AC4C4(arg0, arg1, *(s32 *)((u8 *) arg2 + 0x0), *(s32 *)((u8 *) arg3 + 0x0),
                       *(s32 *)((u8 *) arg2 + 0x8), *(s32 *)((u8 *) arg3 + 0x8),
                       *(s32 *)((u8 *) arg2 + 0xC), *(s32 *)((u8 *) arg3 + 0xC)) != 0) {
        return 1;
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027E228.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027E344.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027E9B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027EED8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027F1F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_802802D4.s")

/* Set the 8 corner positions of the box (x0..x1, y0..y1, z0..z1). */
void func_8028072C(Vtx *v, s16 x0, s16 y0, s16 z0, s16 x1, s16 y1, s16 z1) {
    v[0].v.ob[0] = x0;
    v[0].v.ob[1] = y0;
    v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x0;
    v[1].v.ob[1] = y1;
    v[1].v.ob[2] = z0;
    v[2].v.ob[0] = x1;
    v[2].v.ob[1] = y0;
    v[2].v.ob[2] = z0;
    v[3].v.ob[0] = x1;
    v[3].v.ob[1] = y1;
    v[3].v.ob[2] = z0;
    v[4].v.ob[0] = x0;
    v[4].v.ob[1] = y0;
    v[4].v.ob[2] = z1;
    v[5].v.ob[0] = x0;
    v[5].v.ob[1] = y1;
    v[5].v.ob[2] = z1;
    v[6].v.ob[0] = x1;
    v[6].v.ob[1] = y0;
    v[6].v.ob[2] = z1;
    v[7].v.ob[0] = x1;
    v[7].v.ob[1] = y1;
    v[7].v.ob[2] = z1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_802807D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80280F34.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281A70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281CE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281E44.s")

void func_802A0B00(s32, s32);

extern s32 D_80358070;
extern s32 D_8036E4CC;
extern s16 D_8036E4D0;
extern s8 D_8036E4D2;

void func_802821D0(void) {
    D_8036E4CC = D_80358070;
    func_802A0B00(0xA98, 0);
    D_80358070 += 0x800;
    D_8036E4D0 = 0;
    D_8036E4D2 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80282224.s")

extern u8 D_8036E4D3;
extern s32 D_8036E4D4;

void func_80282728(void) {
    D_8036E4D3 = 0;
    D_8036E4D4 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8028273C.s")
