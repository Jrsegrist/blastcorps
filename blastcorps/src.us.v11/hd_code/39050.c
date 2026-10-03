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

/* TODO: func_8028072C - a 7-field vertex/struct setter: 24 `*(s16*)(arg0+N) =
 * argX;` stores at scattered offsets (target: 97 instructions, no frame, all
 * field logic and offsets confirmed correct - this phrasing reaches the
 * exact target length). Remaining gap is pure instruction scheduling, not
 * logic: for the first three stores target interleaves two independent
 * sign-extend pairs (`sll a1;sra a1; sll a2; sll a3;sra a3; sra a2`), i.e.
 * arg3's full truncation gets scheduled between arg2's sll and sra, which
 * this phrasing's plain sequential statement order doesn't reproduce -
 * tried swapping the arg2/arg3 statement order (makes it interleave arg1/
 * arg3 instead, same shape, wrong pair, slightly worse score). A second,
 * apparently independent scheduling difference follows immediately after
 * for the stack-passed arg4/arg5/arg6 reloads (target loads arg5 from its
 * stack slot one store earlier than this phrasing does). Both cascade into
 * register-rename-only differences for the rest of the function (score 953,
 * but entirely 'r' and scheduling markers past this point, no further
 * logic/structural differences) - the original source's exact field-write
 * order likely differs from struct-offset order in a way not recoverable
 * from the offsets alone. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8028072C.s")

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
