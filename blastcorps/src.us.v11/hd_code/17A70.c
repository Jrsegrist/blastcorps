#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* kiunzip: a second copy of the gzip front end in init/1660.c, used by the
 * game to inflate assets at run time (inflate itself is func_802994F8). */

extern s32 D_803669CC;  /* header bytes */
extern s32 D_802E8CC0;  /* compression method */
extern s32 D_802E8CC4;  /* exit code */

void func_8025C5A0(void);
s32 func_8025C30C(void);

extern s32 D_803669E8;
extern s32 D_803669DC;
extern s32 D_803669D8;

#define GZIP_FHCRC 2
#define GZIP_FEXTRA 4
#define GZIP_FNAME 8
#define GZIP_FCOMMENT 0x10

/* Inflates the gzip stream at *arg0 into *arg1, then advances both
 * pointers past the consumed input and the produced output. */
void func_8025C230(u8 * N64P *arg0, u8 * N64P *arg1, s32 arg2) {
    D_803669C0 = *arg0;
    D_803669C4 = *arg1;
    D_8039CAE0 = arg2;

    func_8025C5A0();

    if (*(D_803669C0 + D_803669EC) != 0x1F) {
        D_803669EC += 1;
    }
    D_802E8CC0 = func_8025C30C();
    if (D_802E8CC0 >= 0) {
        func_802994F8();
        *arg0 += D_803669EC;
        *arg1 += D_803669F0;
    }
}

/* get_method: parses the gzip member header and returns the method (8). */
s32 func_8025C30C(void) {
    u8 flags;
    u32 xlen;

    D_803669EC += 2;
    D_802E8CC0 = -1;
    D_803669CC = 0;
    D_802E8CC0 = *(D_803669EC + D_803669C0);
    D_803669EC += 1;

    if (D_802E8CC0 != 8) {
        func_8029A7E4("kiunzip: unknown method %d -- get newer version of gzip\n", D_802E8CC0);
        D_802E8CC4 = 1;
        return -1;
    }

    flags = *(D_803669EC + D_803669C0);
    D_803669EC += 1;
    D_803669EC += 6;

    if (flags & GZIP_FHCRC) {
        D_803669EC += 2;
    }

    if (flags & GZIP_FEXTRA) {
        xlen = *(D_803669EC + D_803669C0);
        D_803669EC += 1;
        xlen |= *(D_803669C0 + D_803669EC) << 8;
        D_803669EC += 1;
        D_803669EC += xlen;
    }

    /* !! (not plain != 0) gives the target's materialized boolean in $s0 */
    if (flags & GZIP_FNAME) {
        while (!!D_803669C0[D_803669EC++]) {
        }
    }

    if (flags & GZIP_FCOMMENT) {
        while (!!D_803669C0[D_803669EC++]) {
        }
    }

    D_803669CC = D_803669EC + 8;
    return D_802E8CC0;
}

/* Reverses the low arg1 bits of arg0. */
u32 func_8025C56C(u32 arg0, s32 arg1, u32 arg2) {
    s32 pad;

    arg2 = 0;
    do {
        arg2 |= arg0 & 1;
        arg0 >>= 1, arg2 <<= 1;
        arg1--;
    } while (arg1 > 0);
    return arg2 >> 1;
}

void func_8025C5A0(void) {
    D_803669F0 = 0;
    D_803669EC = 0;
    D_803669E8 = 0;
    D_803669DC = 0;
    D_803669D8 = 0;
}
