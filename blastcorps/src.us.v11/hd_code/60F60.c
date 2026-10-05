#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. */

/* One of 16 records at D_803C4B70 (stride 0x3C). */
typedef struct {
    /* 0x00 */ u8 *def;     /* points at a definition whose u16 at +0xE is a count */
    /* 0x04 */ u8 pad4[0x2E];
    /* 0x32 */ u8 last;     /* def's count - 1 */
    /* 0x33 */ u8 active;
    /* 0x34 */ u8 pad34[8];
} Rec3C; /* size 0x3C */

/* A 0x1010-byte block carved from the heap by func_802A5FA8. Only the
 * trailing words are touched here. */
typedef struct {
    /* 0x0000 */ u8 data[0x1000];
    /* 0x1000 */ s32 inUse;
    /* 0x1004 */ u16 age;
    /* 0x1006 */ u8 pad1006[0xA];
} HeapBlock; /* size 0x1010 */

extern Rec3C D_803C4B70[16];
extern u8 D_803EB770[16];
extern HeapBlock *D_803EB788;  /* first block */
extern HeapBlock *D_803EB78C;  /* end of the blocks */
extern s16 D_803EB790;         /* number of blocks (asm reads it with lh) */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803C4250[];
extern u8 *D_803C4B50;
extern u8 *D_803C4B54;

/* Resets the two cursors D_803C4B50 / D_803C4B54 to the start of the
 * buffer D_803C4250. */
void func_802A5720(void) {
    D_803C4B50 = D_803C4250;
    D_803C4B54 = D_803C4250;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5720.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Appends the four words (a, b, c, d) at the cursor D_803C4B50 and advances
 * it by 0x10. The asm takes them in s1, s2, s3 and fp (conventions.txt) and
 * saves v0/v1; its asm caller func_802A1074 keeps f12 and f14 live across
 * the call (a mixed N64 build would need a thunk, the native port doesn't). */
void func_802A5764(s32 a, s32 b, s32 c, s32 d) {
    s32 *p = (s32 *) D_803C4B50;

    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    D_803C4B50 = (u8 *) (p + 4);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5764.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
s32 func_802A57DC(u8 *rec);

/* Processes the next 16-byte record at the cursor D_803C4B54 with
 * func_802A57DC and advances the cursor by 0x10. The asm reuses the cursor
 * value it passed (func_802A57DC preserves a0) rather than rereading it, and
 * leaves func_802A57DC's v0 in v0 (the C caller declares it void). */
void func_802A57AC(void) {
    u8 *rec = D_803C4B54;

    func_802A57DC(rec);
    D_803C4B54 = rec + 0x10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A57AC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* A 16-byte decode request (the records func_802A57AC walks). */
typedef struct {
    /* 0x0 */ u8 *data;  /* packed on entry, decoded in place */
    /* 0x4 */ u32 size;  /* packed size in bytes (even) */
    /* 0x8 */ s32 type;  /* decoder: 0..6, anything else = none */
    /* 0xC */ u8 *param; /* u16 table for types 4 and 5 */
} DecodeReq;

extern u64 D_803C3250[]; /* scratch copy of the packed data */

/* The decoders (non-ABI in the asm: src a0, size a1, dst a3, table t4;
 * they return the advanced dst in a3, see conventions.txt). */
u8 *func_802A5958(s16 *src, s32 len, u8 *dst);
u8 *func_802A5A2C(s16 *src, s32 len, u8 *dst);
u8 *func_802A5AE0(s16 *src, s32 len, u8 *dst);
u8 *func_802A5B90(s16 *src, s32 len, u8 *dst);
u8 *func_802A5C5C(s16 *src, s32 len, u8 *dst, u8 *table);
u8 *func_802A5D34(s16 *src, s32 len, u8 *dst, u8 *table);
u8 *func_802A5E10(u64 *src, u32 len, u8 *dst);

/* Decodes a request in place: copies `size` packed bytes to D_803C3250
 * (8-byte units, then halfwords; the asm loops on `!= 0`, so size must be
 * even), runs the decoder for `type` from there back into `data`, and
 * returns the number of bytes written (0 for an unknown type).
 * Register note: the asm saves a0-a3, t0 and t4; asm caller func_802A57AC
 * reuses a0 and v1 after the call, func_802A08E4/func_802A0CFC keep f12 and
 * f14 (a mixed N64 build would need a thunk, the native port doesn't). */
s32 func_802A57DC(u8 *rec) {
    DecodeReq *req = (DecodeReq *) rec;
    u32 len = req->size;
    u32 n = (len >> 3) << 3;
    u64 *s8 = (u64 *) req->data;
    u64 *d8 = D_803C3250;
    u16 *s2;
    u16 *d2;
    u8 *dst;
    u8 *end;

    len -= n;
    for (; n != 0; n -= 8) {
        *d8++ = *s8++;
    }
    s2 = (u16 *) s8;
    d2 = (u16 *) d8;
    for (; len != 0; len -= 2) {
        *d2++ = *s2++;
    }

    dst = req->data;
    switch (req->type) {
        case 0:
            end = func_802A5E10(D_803C3250, req->size, dst);
            break;
        case 1:
            end = func_802A5AE0((s16 *) D_803C3250, req->size, dst);
            break;
        case 2:
            end = func_802A5B90((s16 *) D_803C3250, req->size, dst);
            break;
        case 4:
            end = func_802A5C5C((s16 *) D_803C3250, req->size, dst, req->param);
            break;
        case 5:
            end = func_802A5D34((s16 *) D_803C3250, req->size, dst, req->param);
            break;
        case 3:
            end = func_802A5A2C((s16 *) D_803C3250, req->size, dst);
            break;
        case 6:
            end = func_802A5958((s16 *) D_803C3250, req->size, dst);
            break;
        default:
            end = dst;
            break;
    }
    return end - dst;
}

/* The decoders read s16 tokens until `len` bytes are consumed (the asm loops
 * on `len != 0`, so len must be even). A token with bit 15 clear is a literal
 * that expands to 2 or 4 output bytes; one with bit 15 set is a
 * back-reference: copy (tok & 0x1F) output units, one at a time and
 * overlap-safe, starting the given number of bytes back from dst. Each
 * returns the advanced dst (a3 in the asm). The asm saves every register it
 * uses except a3; its only caller is func_802A57DC. */

/* Back-reference of halfword units, offset (tok & 0x7FFF) >> 5 bytes. */
#define DECODE_BACKREF16(dst, tok)                                \
    {                                                             \
        s32 _n = (tok) & 0x1F;                                    \
        u16 *_from = (u16 *) ((dst) - (((tok) & 0x7FFF) >> 5));   \
                                                                  \
        while (_n != 0) {                                         \
            *(u16 *) (dst) = *_from++;                            \
            (dst) += 2;                                           \
            _n--;                                                 \
        }                                                         \
    }

/* Back-reference of word units, offset (tok & 0x7FE0) >> 4 bytes. */
#define DECODE_BACKREF32(dst, tok)                                \
    {                                                             \
        s32 _n = (tok) & 0x1F;                                    \
        u32 *_from = (u32 *) ((dst) - (((tok) & 0x7FE0) >> 4));   \
                                                                  \
        while (_n != 0) {                                         \
            *(u32 *) (dst) = *_from++;                            \
            (dst) += 4;                                           \
            _n--;                                                 \
        }                                                         \
    }
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A57DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: two bytes, each 0bxxaaabbb -> 0baaa0bbb0 (bits 3-5 to 5-7,
 * bits 0-2 to 1-3). */
u8 *func_802A5958(s16 *src, s32 len, u8 *dst) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            s32 hi = (u32) tok >> 8;
            s32 lo = tok & 0xFF;

            dst[0] = ((hi & 0x38) << 2) | ((hi & 7) << 1);
            dst[1] = ((lo & 0x38) << 2) | ((lo & 7) << 1);
            dst += 2;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF16(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5958.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: two bytes, each shifted left by one. */
u8 *func_802A5A2C(s16 *src, s32 len, u8 *dst) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            dst[0] = ((u32) tok >> 8) << 1;
            dst[1] = (tok & 0xFF) << 1;
            dst += 2;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF16(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5A2C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: one halfword, bits 6-15 shifted up by one (bit 6 becomes 0). */
u8 *func_802A5AE0(s16 *src, s32 len, u8 *dst) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            *(u16 *) dst = ((tok & 0xFFC0) << 1) | (tok & 0x3F);
            dst += 2;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF16(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5AE0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: one word of four 4-bit fields (bits 11-14, 7-10, 3-6, 0-2)
 * expanded to the top of each byte. */
u8 *func_802A5B90(s16 *src, s32 len, u8 *dst) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            *(u32 *) dst = ((tok & 0x7800) << 17) | ((tok & 0x780) << 13) | ((tok & 0x78) << 9) |
                           ((tok & 7) << 5);
            dst += 4;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF32(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5B90.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: two halfwords; each byte b of the token gives
 * (table[b >> 1] << 1) | (b & 1), table being u16 entries. */
u8 *func_802A5C5C(s16 *src, s32 len, u8 *dst, u8 *table) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            s32 hi = (u32) tok >> 8;

            ((u16 *) dst)[0] = (*(u16 *) (table + (hi & 0xFE)) << 1) | (hi & 1);
            ((u16 *) dst)[1] = (*(u16 *) (table + (tok & 0xFE)) << 1) | (tok & 1);
            dst += 4;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF32(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5C5C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Literal: one word from the 5-5-5 colour table[tok >> 4] (u16 entries), its
 * channels moved to the top of bytes 0-2, with (tok & 0xF) << 4 in byte 3. */
u8 *func_802A5D34(s16 *src, s32 len, u8 *dst, u8 *table) {
    while (len != 0) {
        s32 tok = *src++;

        if (tok >= 0) {
            s32 c = *(u16 *) (table + (((u32) tok >> 4) << 1));

            *(u32 *) dst = ((c & 0x7C00) << 17) | ((c & 0x3E0) << 14) | ((c & 0x1F) << 11) |
                           ((tok & 0xF) << 4);
            dst += 4;
            len -= 2;
        } else {
            len -= 2;
            DECODE_BACKREF32(dst, tok);
        }
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5D34.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Type 0, stored: copies len >> 3 doublewords (a trailing len & 7 is
 * dropped). */
u8 *func_802A5E10(u64 *src, u32 len, u8 *dst) {
    u32 n = len >> 3;

    while (n != 0) {
        *(u64 *) dst = *src++;
        dst += 8;
        n--;
    }
    return dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5E10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* For each active record of D_803C4B70, sets `last` to the definition's
 * count (u16 at +0xE) minus 1, truncated to a byte.
 * Asm callers rely on preserved: func_802BA9A0 keeps a0, a2, a3, t6, t7,
 * f12, f14; func_802CF3E0 keeps t1. */
void func_802A5E60(void) {
    s32 i;

    for (i = 0; i < 16; i++) {
        if (D_803C4B70[i].active != 0) {
            D_803C4B70[i].last = *(u16 *) (D_803C4B70[i].def + 0xE) - 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5E60.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns the number of active records of D_803C4B70 (t0 in the asm).
 * Asm callers (func_802B37B0, func_802B4EF8, func_802B6294, func_802B7F98,
 * func_802CAAFC, func_802CC400, func_802CD578, func_802D02F8) keep a0-a3,
 * t6, t7, f12 and f14 live across the call (a mixed N64 build would need a
 * thunk, the native port doesn't). */
s32 func_802A5ED0(void) {
    s32 i;
    s32 n = 0;

    for (i = 0; i < 16; i++) {
        if (D_803C4B70[i].active != 0) {
            n++;
        }
    }
    return n;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5ED0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Clears the heap-block range and count, deactivates all 16 records of
 * D_803C4B70 and clears the 16 bytes of D_803EB770.
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14. */
void func_802A5F30(void) {
    s32 i;

    D_803EB788 = NULL;
    D_803EB78C = NULL;
    D_803EB790 = 0;
    for (i = 0; i < 16; i++) {
        D_803C4B70[i].active = 0;
        D_803EB770[i] = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5F30.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_80364AA8;  /* game mode flags */
extern u8 *D_80358070;  /* heap pointer */
extern u8 D_8020ED00[]; /* heap end when (D_80364AA8 & 0x20) */
extern u8 D_8021DD00[]; /* heap end otherwise */
extern char D_80305C34[];
extern char D_80305C48[];
void func_8029A7E4(char *, ...); /* debug printf */

/* Carves as many 0x1010-byte HeapBlocks as fit between the heap pointer and
 * the heap end, records the range in D_803EB788 / D_803EB78C and the count
 * in D_803EB790, advances the heap pointer past them, prints two debug
 * messages and clears each block's inUse word. */
void func_802A5FA8(void) {
    u8 *start = D_80358070;
    s32 span = ((D_80364AA8 & 0x20) ? D_8020ED00 : D_8021DD00) - start;
    u32 n;
    HeapBlock *b;
    HeapBlock *end;

    if (span < 0) {
        span = 0;
    }
    n = (u32) span / sizeof(HeapBlock);
    D_803EB788 = (HeapBlock *) start;
    D_803EB790 = n;
    func_8029A7E4(D_80305C34);
    func_8029A7E4(D_80305C48, n);
    end = (HeapBlock *) (start + n * sizeof(HeapBlock));
    D_803EB78C = end;
    D_80358070 = (u8 *) end;
    for (b = (HeapBlock *) start; b != end; b++) {
        b->inUse = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5FA8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6274.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A64A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Ages every in-use HeapBlock by one; a block whose age reaches 0x3D (61)
 * is freed (inUse = 0). Adds the number freed to D_803EB790. */
void func_802A6748(void) {
    HeapBlock *b;
    s32 freed = 0;

    for (b = D_803EB788; b != D_803EB78C; b++) {
        s32 age;

        if (b->inUse == 0) {
            continue;
        }
        age = b->age + 1; /* u16 + 1, compared as a full int */
        b->age = age;
        if (age >= 0x3D) {
            freed++;
            b->inUse = 0;
        }
    }
    D_803EB790 += freed;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6748.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A67C4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A68D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Fills a 4-vertex quad: corners (x, y), (x - w, y), (x, y + h),
 * (x - w, y + h) at z = 0, texture coordinates (0 or w << 5, 0 or h << 5),
 * and every vertex's colour from bytes 6-9 of `col`. The asm takes
 * w, v, col, x, y, h in v1, t1, t7, s2, s3, s5 (conventions.txt) and saves
 * everything it uses; asm caller func_802A64A4 keeps a0-a3, t0, t1, t3-t5,
 * t7-t9, f12 and f14 live across the call (a mixed N64 build would need a
 * thunk, the native port doesn't). */
void func_802A6C10(s32 w, Vtx *v, u8 *col, s32 x, s32 y, s32 h) {
    s32 x1 = x - w;
    s32 y1 = y + h;
    s32 s = w << 5;
    s32 t = h << 5;
    s32 i;

    v[0].v.ob[0] = x;
    v[0].v.ob[1] = y;
    v[0].v.ob[2] = 0;
    v[0].v.flag = 0;
    v[0].v.tc[0] = 0;
    v[0].v.tc[1] = 0;
    v[1].v.ob[0] = x1;
    v[1].v.ob[1] = y;
    v[1].v.ob[2] = 0;
    v[1].v.flag = 0;
    v[1].v.tc[0] = s;
    v[1].v.tc[1] = 0;
    v[2].v.ob[0] = x;
    v[2].v.ob[1] = y1;
    v[2].v.ob[2] = 0;
    v[2].v.flag = 0;
    v[2].v.tc[0] = 0;
    v[2].v.tc[1] = t;
    v[3].v.ob[0] = x1;
    v[3].v.ob[1] = y1;
    v[3].v.ob[2] = 0;
    v[3].v.flag = 0;
    v[3].v.tc[0] = s;
    v[3].v.tc[1] = t;
    for (i = 0; i < 4; i++) {
        v[i].v.cn[0] = col[6];
        v[i].v.cn[1] = col[7];
        v[i].v.cn[2] = col[8];
        v[i].v.cn[3] = col[9];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6C10.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Gfx *D_803EB780;
extern Gfx *D_803EB784;
Gfx *func_80257540(Gfx *gfx);

/* Runs func_80257540 on the display-list cursors D_803EB780 and D_803EB784,
 * in that order, storing the advanced cursors back.
 * Asm callers rely on preserved: func_802A64A4 keeps a0-a3, t0, t1, t3-t5,
 * f12, f14 (the asm saves v0, v1, a0-a3, t0, t1, t3-t9 around the calls). */
void func_802A6D34(void) {
    D_803EB780 = func_80257540(D_803EB780);
    D_803EB784 = func_80257540(D_803EB784);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6D34.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6DE8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Picks a display-list cursor: &D_803EB784 if byte 0x3B of `obj` is set,
 * else &D_803EB780 (obj in t4, result in t2 in the asm). Asm callers keep
 * registers live across the call: func_802A64A4 a0-a3, t0, t1, t3-t5,
 * f12, f14; func_802A68D4 a0-a3, t0, t4, f12, f14; func_802A6DE8 t1, t8
 * (a mixed N64 build would need a thunk, the native port doesn't). */
Gfx **func_802A6EB8(u8 *obj) {
    if (obj[0x3B] != 0) {
        return &D_803EB784;
    }
    return &D_803EB780;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6EB8.s")
#endif
