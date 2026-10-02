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

/* TODO: gzip_get_method - parses a GZIP member header (RFC 1952) starting 2
 * bytes into the stream (the caller/func_80220360 already looked at, but
 * did not consume, the first magic byte). Skips the second magic byte,
 * reads the compression method (must be 8/DEFLATE, else reports an error
 * via func_80220714 and returns -1), reads the FLG flags byte, skips the
 * fixed MTIME+XFL+OS fields, then conditionally skips FHCRC/FEXTRA/FNAME/
 * FCOMMENT based on the flag bits - same structure as GNU gzip's own
 * get_method() (gzip.c), but trimmed to pure skip-only logic: no CRC
 * verification, no filename extraction, no encrypted/reserved-flag
 * rejection, since Rare's own compressor guarantees well-formed headers
 * (same pattern already confirmed throughout the INFLATE implementation
 * in this file/init/0050.c/init/0E30.c). Not a reference transcription
 * like those (gzip.c's real get_method is a full CLI-tool implementation
 * with no equivalent simplified form to crib from) - written directly
 * from the disassembly trace instead.
 *
 * Total length is exactly right (confirmed via reverse_bits, immediately
 * after this function, landing on its exact target address). Declaration-
 * order tuning closed most of the gap (diff score 1915 -> 1835): `flags`
 * and `xlen`'s stack slots now match target exactly. What's left is pure
 * instruction-scheduling order in the header prologue (the method=-1 /
 * header_bytes=0 / position-update / method-byte-read sequence) - every
 * instruction target emits also appears here, just interleaved slightly
 * differently; no missing or extra content. Needs the same kind of IDO
 * scheduler insight the other near-misses in this file are waiting on.
 *
 * #define GZIP_FHCRC 2
 * #define GZIP_FEXTRA 4
 * #define GZIP_FNAME 8
 * #define GZIP_FCOMMENT 0x10
 *
 * s32 gzip_get_method(void) {
 *     u8 flags;
 *     u32 xlen;
 *     s32 pos;
 *
 *     pos = D_80222A1C + 2;
 *     D_80222840 = -1;
 *     D_802229FC = 0;
 *     D_80222840 = D_802229F0[pos];
 *     pos += 1;
 *     D_80222A1C = pos;
 *
 *     if (D_80222840 != 8) {
 *         func_80220714(D_802228D0, D_80222840);
 *         D_80222844 = 1;
 *         return -1;
 *     }
 *
 *     flags = D_802229F0[pos];
 *     pos += 1;
 *     D_80222A1C = pos;
 *     pos += 6;
 *     D_80222A1C = pos;
 *
 *     if (flags & GZIP_FHCRC) {
 *         pos += 2;
 *         D_80222A1C = pos;
 *     }
 *
 *     if (flags & GZIP_FEXTRA) {
 *         xlen = D_802229F0[D_80222A1C];
 *         D_80222A1C += 1;
 *         xlen |= D_802229F0[D_80222A1C] << 8;
 *         D_80222A1C += 1;
 *         D_80222A1C += xlen;
 *     }
 *
 *     if (flags & GZIP_FNAME) {
 *         while (D_802229F0[D_80222A1C++] != 0) {
 *         }
 *     }
 *
 *     if (flags & GZIP_FCOMMENT) {
 *         while (D_802229F0[D_80222A1C++] != 0) {
 *         }
 *     }
 *
 *     D_802229FC = D_80222A1C + 8;
 *     return D_80222840;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/1660/gzip_get_method.s")

/* TODO: reverse_bits - reverses the low `arg1` bits of `arg0` (classic
 * bit-reversal, used when emitting/reading canonical Huffman codes MSB-first
 * from an LSB-first bitstream). Fully matched down to a single reordered
 * instruction pair: this form produces byte-identical registers and operand
 * order to the target for every instruction except the arg0-shift (srl) and
 * the accumulator-shift (sll), which the target schedules in the opposite
 * order (sll immediately after the `or`, srl afterward) despite arg0's
 * update statement sitting textually between them below - every statement
 * order/compound-operator/register-hint permutation tried still scheduled
 * srl before sll here. Needs real IDO instruction-scheduler knowledge, not
 * more C-level reordering, to close:
 *
 * u32 reverse_bits(u32 arg0, s32 arg1) {
 *     register u32 phi_a2 = 0;
 *
 *     do {
 *         phi_a2 |= arg0 & 1;
 *         arg0 = arg0 >> 1;
 *         phi_a2 <<= 1;
 *     } while (--arg1 > 0);
 *     return phi_a2 >> 1;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/1660/reverse_bits.s")

void func_802206D0(void) {
    D_80222A20 = 0;
    D_80222A1C = 0;
    D_80222A18 = 0;
    D_80222A0C = 0;
    D_80222A08 = 0;
}
