#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* kiunzip: a second copy of the gzip front end in init/1660.c, used by the
 * game to inflate assets at run time (inflate itself is hd_inflate). */

extern s32 D_803669CC;  /* header bytes */
extern s32 D_802E8CC0;  /* compression method */
extern s32 D_802E8CC4;  /* exit code */

void kiunzip_clear_bufs(void);
s32 kiunzip_get_method(void);

extern s32 D_803669E8;
extern s32 D_803669DC;
extern s32 D_803669D8;

#define GZIP_FHCRC 2
#define GZIP_FEXTRA 4
#define GZIP_FNAME 8
#define GZIP_FCOMMENT 0x10

/* Inflates the gzip member at *src into *dst (window work), then advances both pointers past the input read
 * and the output written; the run-time copy of init's gzip front end (0x8025C230).
 * Inflates the gzip stream at *arg0 into *arg1, then advances both
 * pointers past the consumed input and the produced output. */
void kiunzip(u8 * N64P *arg0, u8 * N64P *arg1, s32 arg2) {
    D_803669C0 = *arg0;
    D_803669C4 = *arg1;
    D_8039CAE0 = arg2;

    kiunzip_clear_bufs();

    if (*(D_803669C0 + D_803669EC) != 0x1F) {
        D_803669EC += 1;
    }
    D_802E8CC0 = kiunzip_get_method();
    if (D_802E8CC0 >= 0) {
        hd_inflate();
        *arg0 += D_803669EC;
        *arg1 += D_803669F0;
    }
}

/* gzip get_method: skips the magic, checks method 8 ("kiunzip: unknown method %d"), skips the flags, mtime,
 * crc, extra, name and comment fields (0x8025C30C).
 * get_method: parses the gzip member header and returns the method (8). */
s32 kiunzip_get_method(void) {
    u8 flags;
    u32 xlen;

    D_803669EC += 2;
    D_802E8CC0 = -1;
    D_803669CC = 0;
    D_802E8CC0 = *(D_803669EC + D_803669C0);
    D_803669EC += 1;

    if (D_802E8CC0 != 8) {
        debug_printf("kiunzip: unknown method %d -- get newer version of gzip\n", D_802E8CC0);
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

/* Reverses the low n bits of code (gzip's bi_reverse); no callers (0x8025C56C).
 * Reverses the low arg1 bits of arg0. */
u32 kiunzip_reverse_bits(u32 arg0, s32 arg1, u32 arg2) {
    s32 pad;

    arg2 = 0;
    do {
        arg2 |= arg0 & 1;
        arg0 >>= 1, arg2 <<= 1;
        arg1--;
    } while (arg1 > 0);
    return arg2 >> 1;
}

/* gzip clear_bufs: zeroes the input and output positions and the byte counters (0x8025C5A0). */
void kiunzip_clear_bufs(void) {
    D_803669F0 = 0;
    D_803669EC = 0;
    D_803669E8 = 0;
    D_803669DC = 0;
    D_803669D8 = 0;
}
