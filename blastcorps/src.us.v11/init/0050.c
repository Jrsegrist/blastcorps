#include "common.h"
#include <ultra64.h>

extern u32 D_802229E4;
extern u32 D_802229E8;
extern u8 *D_802229F0;
extern u8 *D_802229F4;
extern s32 D_80222A1C;
extern s32 D_80222A20;

/*
 * This whole file is Blast Corps' INFLATE (DEFLATE decompression, RFC 1951)
 * implementation, almost certainly adapted from the classic public-domain
 * inflate.c (Mark Adler, used by zlib/gzip/info-zip throughout the 1990s).
 * Confirmed definitively: inflate_fixed()'s fixed Huffman code-length table
 * initialization (144/256/280/288 boundaries with lengths 8/9/7/8, and 30
 * distance codes all length 5) matches RFC 1951 and the reference source
 * exactly, right down to the table addresses lining up with the expected
 * byte sizes (cplens: 31 entries * 2 bytes = 62, rounded to 64; cpdist: 30
 * entries * 2 bytes = 60, exact fit). huft_build's NEEDBITS/DUMPBITS-style
 * `register` bit-buffer convention (see inflate_stored in this same file,
 * and inflate_block in init/0E30.c, which matches exactly) matches the
 * reference source's own `register ulg b; register unsigned k;`
 * declarations verbatim.
 *
 * RFC 1951 fixed Huffman tables (copy lengths/dists + their extra-bit
 * counts), inside the opaque init/3A40 binary blob. Confirmed via
 * inflate_dynamic (init/0E30.c) that these need real symbol_addrs entries
 * rather than raw-address macros: a raw numeric-literal-cast-to-pointer
 * compiles address formation as `ori` (bit construction), while the real
 * target uses `addiu` (the standard %lo(symbol) relocation form) - same
 * tell as mask_bits below.
 */
extern u16 cplens[];
extern u8 cplext[];
extern u16 cpdist[];
extern u8 cpdext[];

/*
 * mask_bits: the reference source's `static ush mask_bits[] = {0x0000,
 * 0x0001,0x0003,0x0007,...}` precomputed (1<<n)-1 table, used throughout
 * inflate_codes/huft_build to mask off exactly N bits from the bit buffer.
 * Unlike cplens/cplext/cpdist/cpdext above, this one's addressed via a real
 * symbol_addrs entry rather than a raw-address macro: the target computes
 * a full base pointer (lui+addiu forming 0x80222810) before indexing, which
 * only happens for a genuine symbol reference, not a numeric-literal cast.
 */
extern u16 mask_bits[];

typedef struct Huft {
    u8 e;
    u8 b;
    u8 pad[2];
    union {
        u16 n;
        struct Huft *t;
    } v;
} Huft;

extern s32 D_802229E0;
extern s32 D_802229EC;

#define BMAX 16
#define N_MAX 288

/* TODO: huft_build - builds a Huffman decode table from a list of code
 * lengths. Confirmed an exact algorithmic match against the reference
 * source's own huft_build() - the bit-length counting, offset/value-table
 * generation, and main table-building loop (including the `h`/`w`/`z`
 * table-level bookkeeping and the backing-up logic) all line up one-to-one.
 * Adapted in one way: the reference mallocs each new sub-table (`hufts`/
 * `D_802229EC` tracks total entries allocated across calls, matching
 * inflate()'s own `if (D_802229EC > sp1C) sp1C = D_802229EC;` high-water
 * tracking) - this build instead bump-allocates from a fixed arena
 * (D_802229E0 + D_802229EC*sizeof(Huft)), so there's no malloc-failure path
 * and no huft_free() needed. Also confirmed (like inflate_codes) that the
 * reference's defensive bounds-check error returns inside the main loop are
 * absent from this build.
 *
 * Extraordinarily close for a function this size (460 instructions): the
 * ENTIRE function matches exactly except the final return statement's
 * register choice. `return y != 0 && g != 1;` (the literal reference form)
 * computes correctly but lands the result in $s6 with an extra `move v0,s6`
 * at the end; restructuring as an explicit two-step boolean (below) gets
 * every instruction's CONTENT right but still ends up computing into a
 * scratch register ($t1/$t9/$t7 depending on the exact phrasing tried)
 * instead of computing directly into $v0 the way the target does from the
 * very first instruction of this tail. Tried five phrasings (direct &&,
 * early-return, explicit intermediate in a nested block, `register`-
 * qualified intermediate, ternary) - the nested-block plain-`s32`
 * version below was the closest (diff score 680, zero structural
 * insertions/deletions, just this one register-choice tail). Whatever
 * determines target's choice of $v0 here isn't reachable through the
 * return expression's own phrasing - worth a fresh angle on why IDO
 * would prefer $v0 unprompted for this specific short-circuit-into-return
 * pattern.
 *
 * s32 huft_build(s32 *b, u32 n, u32 s, u16 *d, u8 *e, Huft **t, s32 *m) {
 *     u32 a;
 *     u32 c[BMAX + 1];
 *     u32 f;
 *     s32 g;
 *     s32 h;
 *     register u32 i;
 *     register u32 j;
 *     register s32 k;
 *     s32 l;
 *     register u32 *p;
 *     register Huft *q;
 *     Huft r;
 *     Huft *u[BMAX];
 *     u32 v[N_MAX];
 *     register s32 w;
 *     u32 x[BMAX + 1];
 *     u32 *xp;
 *     s32 y;
 *     u32 z;
 *
 *     bzero(c, sizeof(c));
 *     p = (u32 *) b;
 *     i = n;
 *     do {
 *         c[*p]++;
 *         p++;
 *     } while (--i);
 *     if (c[0] == n) {
 *         *t = NULL;
 *         *m = 0;
 *         return 0;
 *     }
 *
 *     l = *m;
 *     for (j = 1; j <= BMAX; j++) {
 *         if (c[j]) {
 *             break;
 *         }
 *     }
 *     k = j;
 *     if ((u32) l < j) {
 *         l = j;
 *     }
 *     for (i = BMAX; i; i--) {
 *         if (c[i]) {
 *             break;
 *         }
 *     }
 *     g = i;
 *     if ((u32) l > i) {
 *         l = i;
 *     }
 *     *m = l;
 *
 *     for (y = 1 << j; j < (u32) i; j++, y <<= 1) {
 *         y -= c[j];
 *     }
 *     y -= c[i];
 *     c[i] += y;
 *
 *     x[1] = j = 0;
 *     p = c + 1;
 *     xp = x + 2;
 *     while (--i) {
 *         *xp++ = (j += *p++);
 *     }
 *
 *     p = (u32 *) b;
 *     i = 0;
 *     do {
 *         if ((j = *p++) != 0) {
 *             v[x[j]++] = i;
 *         }
 *     } while (++i < n);
 *
 *     x[0] = i = 0;
 *     p = v;
 *     h = -1;
 *     w = -l;
 *     u[0] = NULL;
 *     q = NULL;
 *     z = 0;
 *
 *     for (; k <= g; k++) {
 *         a = c[k];
 *         while (a--) {
 *             while (k > w + l) {
 *                 h++;
 *                 w += l;
 *
 *                 z = (z = g - w) > (u32) l ? l : z;
 *                 if ((f = 1 << (j = k - w)) > a + 1) {
 *                     f -= a + 1;
 *                     xp = c + k;
 *                     while (++j < z) {
 *                         if ((f <<= 1) <= *++xp) {
 *                             break;
 *                         }
 *                         f -= *xp;
 *                     }
 *                 }
 *                 z = 1 << j;
 *
 *                 q = (Huft *) (D_802229E0 + D_802229EC * 8);
 *                 D_802229EC += z + 1;
 *                 *t = q + 1;
 *                 *(t = (Huft **) &(q->v.t)) = NULL;
 *                 u[h] = ++q;
 *
 *                 if (h) {
 *                     x[h] = i;
 *                     r.b = (u8) l;
 *                     r.e = (u8) (16 + j);
 *                     r.v.t = q;
 *                     j = i >> (w - l);
 *                     u[h - 1][j] = r;
 *                 }
 *             }
 *
 *             r.b = (u8) (k - w);
 *             if (p >= v + n) {
 *                 r.e = 99;
 *             } else if (*p < s) {
 *                 r.e = (u8) (*p < 256 ? 16 : 15);
 *                 r.v.n = (u16) (*p);
 *                 p++;
 *             } else {
 *                 r.e = e[*p - s];
 *                 r.v.n = d[*p++ - s];
 *             }
 *
 *             f = 1 << (k - w);
 *             for (j = i >> w; j < z; j += f) {
 *                 q[j] = r;
 *             }
 *
 *             for (j = 1 << (k - 1); i & j; j >>= 1) {
 *                 i ^= j;
 *             }
 *             i ^= j;
 *
 *             while ((i & ((1 << w) - 1)) != x[h]) {
 *                 h--;
 *                 w -= l;
 *             }
 *         }
 *     }
 *
 *     {
 *         s32 ret;
 *         ret = y != 0;
 *         if (ret) {
 *             ret = g != 1;
 *         }
 *         return ret;
 *     }
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/huft_build.s")

/* TODO: inflate_codes - decodes Huffman-coded literals/lengths (via `tl`)
 * and distances (via `td`) and copies the resulting literal bytes / back-
 * references directly into the output buffer (D_802229F4). Confirmed an
 * exact structural match against the reference source's own
 * inflate_codes(), minus its WSIZE sliding-window wraparound logic
 * (flush_output/circular `slide` buffer) - this build decompresses
 * straight into a single fixed output buffer instead of a circular window,
 * so every "if (w == WSIZE) { flush_output(w); w = 0; }" check and the
 * distance's "& (WSIZE-1)" masking are simply absent here. Also confirmed
 * the reference's defensive "if (e == 99) return 1;" (invalid-code) checks
 * are entirely absent from this build - Rare's own compressor apparently
 * guarantees well-formed tables, so that check was stripped.
 *
 * Extremely close: every instruction content/order matches exactly (down
 * to an instruction-level reordering of the reference's DUMPBITS(e) - this
 * build's b>>=e has to come before k-=e, not after) except ONE single
 * redundant register-to-register `move` the target does right before the
 * inner copy loop (copying the saved copy-count into a fresh register
 * before the loop, rather than just decrementing it in place). Tried: a
 * second named counter variable (both `register` and plain) - always
 * fixed that one instruction but shifted register allocation throughout
 * the EARLIER two-thirds of the function instead (net worse); reusing the
 * already-`register` `e` variable as the counter (matching the reference
 * literally reusing `e` for this) - also rippled backward and regressed.
 * The version below (single `register u32 n2`) is the closest found - one
 * missing 4-byte instruction, otherwise byte-identical modulo registers.
 *
 * s32 inflate_codes(Huft *tl, Huft *td, u32 bl, u32 bd) {
 *     register u32 e;
 *     u32 n;
 *     u32 d;
 *     u32 w;
 *     Huft *t;
 *     u32 ml;
 *     u32 md;
 *     register u32 b;
 *     register u32 k;
 *
 *     b = D_802229E4;
 *     k = D_802229E8;
 *     w = D_80222A20;
 *
 *     ml = mask_bits[bl];
 *     md = mask_bits[bd];
 *
 *     for (;;) {
 *         while (k < bl) {
 *             b |= (u32) D_802229F0[D_80222A1C++] << k;
 *             k += 8;
 *         }
 *         t = tl + (b & ml);
 *         e = t->e;
 *         if (e > 16) {
 *             do {
 *                 k -= t->b;
 *                 b >>= t->b;
 *                 e -= 16;
 *                 while (k < e) {
 *                     b |= (u32) D_802229F0[D_80222A1C++] << k;
 *                     k += 8;
 *                 }
 *                 t = t->v.t + (b & mask_bits[e]);
 *                 e = t->e;
 *             } while (e > 16);
 *         }
 *         k -= t->b;
 *         b >>= t->b;
 *         if (e == 16) {
 *             D_802229F4[w++] = (u8) t->v.n;
 *             continue;
 *         }
 *         if (e == 15) {
 *             break;
 *         }
 *
 *         while (k < e) {
 *             b |= (u32) D_802229F0[D_80222A1C++] << k;
 *             k += 8;
 *         }
 *         n = t->v.n + (b & mask_bits[e]);
 *         k -= e;
 *         b >>= e;
 *
 *         while (k < bd) {
 *             b |= (u32) D_802229F0[D_80222A1C++] << k;
 *             k += 8;
 *         }
 *         t = td + (b & md);
 *         e = t->e;
 *         if (e > 16) {
 *             do {
 *                 k -= t->b;
 *                 b >>= t->b;
 *                 e -= 16;
 *                 while (k < e) {
 *                     b |= (u32) D_802229F0[D_80222A1C++] << k;
 *                     k += 8;
 *                 }
 *                 t = t->v.t + (b & mask_bits[e]);
 *                 e = t->e;
 *             } while (e > 16);
 *         }
 *         k -= t->b;
 *         b >>= t->b;
 *
 *         while (k < e) {
 *             b |= (u32) D_802229F0[D_80222A1C++] << k;
 *             k += 8;
 *         }
 *         d = (w - t->v.n) - (b & mask_bits[e]);
 *         b >>= e;
 *         k -= e;
 *
 *         do {
 *             register u32 n2;
 *             n2 = n;
 *             n -= n2;
 *             do {
 *                 n2 -= 1;
 *                 D_802229F4[w++] = D_802229F4[d++];
 *             } while (n2 != 0);
 *         } while (n != 0);
 *     }
 *
 *     D_80222A20 = w;
 *     D_802229E4 = b;
 *     D_802229E8 = k;
 *     return 0;
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/inflate_codes.s")

/* TODO: inflate_stored (identified via its call site in init/0E30.c's
 * inflate_block - the real public-domain inflate.c's "stored block" handler,
 * which byte-aligns the bit buffer and copies a raw uncompressed block).
 * Confirmed behavior via m2c; got very close to matching with
 * `register u32 a0, a1` mirroring the target's use of $a0/$a1 as persistent
 * values across the whole function (not just call-preserved temps) - this
 * closed almost the entire diff (10269 -> ~900 lines) and correctly
 * reproduced the target's sltiu/srlv instruction choices. The remaining gap
 * is a stack frame size mismatch (target reserves 0x18 bytes, every attempt
 * here lands on 0x10) despite only 8 bytes of that actually being addressed
 * by named locals (sp10/sp14) in either version - likely IDO reserving a
 * standard 16-byte argument-build area that this specific register-heavy,
 * call-free leaf function triggers under some condition not yet identified.
 * Needs more specific IDO knowledge than trial-and-error register/
 * declaration-order tweaks turned up this round. */
#pragma GLOBAL_ASM("asm/nonmatchings/init/0050/inflate_stored.s")

extern s32 huft_build(s32 *b, u32 n, u32 s, u16 *d, u8 *e, Huft **t, s32 *m);
extern s32 inflate_codes(Huft *tl, Huft *td, u32 bl, u32 bd);

s32 inflate_fixed(void) {
    s32 i;
    Huft *tl;
    Huft *td;
    s32 bl;
    s32 bd;
    s32 l[288];

    for (i = 0; i < 144; i++) {
        l[i] = 8;
    }
    for (; i < 256; i++) {
        l[i] = 9;
    }
    for (; i < 280; i++) {
        l[i] = 7;
    }
    for (; i < 288; i++) {
        l[i] = 8;
    }
    bl = 7;
    huft_build(l, 288, 257, cplens, cplext, &tl, &bl);

    for (i = 0; i < 30; i++) {
        l[i] = 5;
    }
    bd = 5;
    huft_build(l, 30, 0, cpdist, cpdext, &td, &bd);

    inflate_codes(tl, td, bl, bd);
    return 0;
}
