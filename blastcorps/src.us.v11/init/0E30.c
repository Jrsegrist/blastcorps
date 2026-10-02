#include "common.h"
#include <ultra64.h>

extern s32 D_80222A1C;
extern s32 D_80222A20;
extern s32 D_802229E4;
extern u32 D_802229E8;
extern u32 D_802229EC;
extern u8 *D_802229F0;

/* TODO: inflate_dynamic (dynamic-Huffman-table block handler) - not yet
 * attempted, see huft_build/inflate_codes/inflate_fixed in init/0050.c for
 * the rest of this INFLATE implementation. */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0E30/inflate_dynamic.s")

extern s32 inflate_dynamic(void);
extern s32 inflate_stored(void);
extern s32 inflate_fixed(void);

/*
 * inflate_block: reads a 3-bit block header (1-bit "last block" flag + 2-bit
 * block type) from the same bit-buffer/bit-count globals used throughout
 * this file's INFLATE implementation, then dispatches to the handler for
 * that block type. Confirmed an exact structural match (including the
 * t==2/t==0/t==1 dispatch order and the "return 2" bad-type fallthrough)
 * against the classic public-domain inflate.c's own inflate_block(): this is
 * byte-for-byte the same function, just compiled by IDO instead of a PC
 * compiler. inflate_dynamic and inflate_stored identified here via their
 * call sites (inflate_stored's TODO comment in init/0050.c predates this
 * identification); inflate_fixed already identified/named separately.
 */
s32 inflate_block(s32 *arg0) {
    s32 type;
    register u32 k;
    register u32 b;

    k = D_802229E8;
    b = D_802229E4;

    while (k == 0) {
        b |= (u32) D_802229F0[D_80222A1C++] << k;
        k += 8;
    }
    *arg0 = b & 1;
    b = b >> 1;
    k -= 1;

    while (k < 2) {
        b |= (u32) D_802229F0[D_80222A1C++] << k;
        k += 8;
    }
    type = b & 3;
    b = b >> 2;
    k -= 2;

    D_802229E4 = b;
    D_802229E8 = k;

    if (type == 2) {
        return inflate_dynamic();
    }
    if (type == 0) {
        return inflate_stored();
    }
    if (type == 1) {
        return inflate_fixed();
    }
    return 2;
}

s32 inflate(void) {
    s32 sp24;
    s32 sp20;
    u32 sp1C;

    D_80222A20 = 0;
    D_802229E8 = 0;
    D_802229E4 = 0;
    sp1C = 0;
    for (;;) {
        D_802229EC = 0;
        sp20 = inflate_block(&sp24);
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
