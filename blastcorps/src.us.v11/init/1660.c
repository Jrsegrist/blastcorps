#include "common.h"
#include <ultra64.h>

extern s32 D_80222840;
extern s32 D_802229E0;
extern u8 *D_802229F0;
extern s32 D_802229F4;
extern s32 D_802229FC;
extern s32 D_80222844;
extern s32 D_802228D0[];
extern s32 D_80222A08;
extern s32 D_80222A0C;
extern s32 D_80222A18;
extern s32 D_80222A1C;
extern s32 D_80222A20;

void func_802206D0(void);
s32  gzip_get_method(void);
extern s32 inflate(void);
extern void func_80220714(void *arg0, s32 arg1);


void func_80220360(s32 *arg0, s32 *arg1, s32 arg2) {
    D_802229F0 = *arg0;
    D_802229F4 = *arg1;
    D_802229E0 = arg2;

    func_802206D0();

    if (*(D_802229F0 + D_80222A1C) != 0x1F) {
        D_80222A1C += 1;
    }
    D_80222840 = gzip_get_method();
    if (D_80222840 >= 0) {
        inflate();
        *arg0 += D_80222A1C;
        *arg1 += D_80222A20;
    }
}

#define GZIP_FHCRC 2
#define GZIP_FEXTRA 4
#define GZIP_FNAME 8
#define GZIP_FCOMMENT 0x10

/* gzip_get_method: parses a GZIP member header (RFC 1952) starting 2 bytes
 * into the stream (the caller/func_80220360 already looked at, but did not
 * consume, the first magic byte). Skips the second magic byte, reads the
 * compression method (must be 8/DEFLATE, else reports an error via
 * func_80220714 and returns -1), reads the FLG flags byte, skips the fixed
 * MTIME+XFL+OS fields, then conditionally skips FHCRC/FEXTRA/FNAME/FCOMMENT
 * based on the flag bits - same structure as GNU gzip's own get_method()
 * (gzip.c), but trimmed to pure skip-only logic: no CRC verification, no
 * filename extraction, no encrypted/reserved-flag rejection, since Rare's
 * own compressor guarantees well-formed headers (same pattern confirmed
 * throughout the INFLATE implementation in init/0050.c/init/0E30.c).
 *
 * `*(D_80222A1C + D_802229F0)` vs `D_802229F0[D_80222A1C]` vs
 * `*(D_802229F0 + D_80222A1C)` below are NOT a style inconsistency - each
 * spelling was needed to get IDO to pick the exact operand order the target
 * ROM uses for that specific access's address-add instruction. Addition is
 * commutative so all three compute the same value; don't "clean up" the
 * mixed forms without re-diffing against the target afterward.
 */
s32 gzip_get_method(void) {
    u8 flags;
    u32 xlen;

    D_80222A1C += 2;
    D_80222840 = -1;
    D_802229FC = 0;
    D_80222840 = *(D_80222A1C + D_802229F0);
    D_80222A1C += 1;

    if (D_80222840 != 8) {
        func_80220714(D_802228D0, D_80222840);
        D_80222844 = 1;
        return -1;
    }

    flags = *(D_80222A1C + D_802229F0);
    D_80222A1C += 1;
    D_80222A1C += 6;

    if (flags & GZIP_FHCRC) {
        D_80222A1C += 2;
    }

    if (flags & GZIP_FEXTRA) {
        xlen = *(D_80222A1C + D_802229F0);
        D_80222A1C += 1;
        xlen |= *(D_802229F0 + D_80222A1C) << 8;
        D_80222A1C += 1;
        D_80222A1C += xlen;
    }

    if (flags & GZIP_FNAME) {
        while (D_802229F0[D_80222A1C++] != 0) {
        }
    }

    if (flags & GZIP_FCOMMENT) {
        while (D_802229F0[D_80222A1C++] != 0) {
        }
    }

    D_802229FC = D_80222A1C + 8;
    return D_80222840;
}

/* Reverses the low `arg1` bits of `arg0` (canonical Huffman codes are
 * MSB-first, the bitstream LSB-first). arg2 is a dead parameter reused as the
 * accumulator; hd_code's copy (17A70) is byte-identical. */
u32 reverse_bits(u32 arg0, s32 arg1, u32 arg2) {
    s32 pad;

    arg2 = 0;
    do {
        arg2 |= arg0 & 1;
        arg0 >>= 1, arg2 <<= 1;
        arg1--;
    } while (arg1 > 0);
    return arg2 >> 1;
}

void func_802206D0(void) {
    D_80222A20 = 0;
    D_80222A1C = 0;
    D_80222A18 = 0;
    D_80222A0C = 0;
    D_80222A08 = 0;
}
