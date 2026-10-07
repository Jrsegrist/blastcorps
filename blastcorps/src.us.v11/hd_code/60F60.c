#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */

/* .data at 0x80305C10 (ROM 0xC1450-0xC1490): the 0-terminated list of effect
 * definitions (in the 7D9D0 tables inside .text) that get a heap texture
 * block (func_802A67C4), then func_802A5FA8's two debug strings. */
u8 * N64P D_80305C10[] = {
    N64_DPTR(D_802C2954), N64_DPTR(D_802C2984), N64_DPTR(D_802C2A5C), N64_DPTR(D_802C37C0),
    N64_DPTR(D_802C3804), N64_DPTR(D_802C382C), N64_DPTR(D_802C3848), N64_DPTR(D_802C386C), NULL,
};
char D_80305C34[] = "Num texture caches=";
char D_80305C48[] = "%d\n";

#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. */

/* One of 16 records at D_803C4B70 (stride 0x3C). */
typedef struct {
    /* 0x00 */ u8 * N64P def;     /* points at a definition whose u16 at +0xE is a count */
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
extern HeapBlock * N64P D_803EB788;  /* first block */
extern HeapBlock * N64P D_803EB78C;  /* end of the blocks */
extern s16 D_803EB790;         /* number of blocks (asm reads it with lh) */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803C4250[];
extern u8 * N64P D_803C4B50;
extern u8 * N64P D_803C4B54;

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
    /* 0x0 */ u8 * N64P data;  /* packed on entry, decoded in place */
    /* 0x4 */ u32 size;  /* packed size in bytes (even) */
    /* 0x8 */ s32 type;  /* decoder: 0..6, anything else = none */
    /* 0xC */ u8 * N64P param; /* u16 table for types 4 and 5 */
} DecodeReq;

extern u64 D_803C3250[]; /* scratch copy of the packed data */
#ifdef PORT_HOST
/* Windows port, byte order (port/src/load/port_textures.c): the packed
 * tokens arrive big-endian (in host order before decoding), the decoded
 * texels leave as big-endian bytes, as the renderer expects them. */
void port_texture_input(u8 *data, u32 size, s32 type);
void port_texture_output(u8 *dst, u32 len, s32 type);
#endif

/* The decoders (non-ABI in the asm: src a0, size a1, dst a3, table t4;
 * they return the advanced dst in a3, see conventions.txt). */
u8 *func_802A5958(s16 *src, s32 len, u8 *dst);
u8 *func_802A5A2C(s16 *src, s32 len, u8 *dst);
u8 *func_802A5AE0(s16 *src, s32 len, u8 *dst);
u8 *func_802A5B90(s16 *src, s32 len, u8 *dst);
u8 *func_802A5C5C(s16 *src, s32 len, u8 *dst, u8 *table);
u8 *func_802A5D34(s16 *src, s32 len, u8 *dst, u8 *table);
u8 *func_802A5E10(u64 *src, u32 len, u8 *dst);
/* The u16 tables of types 4 and 5 are texture-table entries of their own,
 * loaded as stored (type 0) data, so they stay big-endian in the port:
 * read them so (found by port/tools/compare.py: type-5 colours came out
 * with their channels swapped). */
#ifdef PORT_HOST
#define TBL_U16(t, off) ((u16) ((((u8 *) (t))[(off)] << 8) | ((u8 *) (t))[(off) + 1]))
#else
#define TBL_U16(t, off) (*(u16 *) ((u8 *) (t) + (off)))
#endif

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

#ifdef PORT_HOST
    port_texture_input(req->data, req->size, req->type);
#endif
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
#ifdef PORT_HOST
    port_texture_output(dst, end - dst, req->type);
#endif
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

            ((u16 *) dst)[0] = (TBL_U16(table, hi & 0xFE) << 1) | (hi & 1);
            ((u16 *) dst)[1] = (TBL_U16(table, tok & 0xFE) << 1) | (tok & 1);
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
            s32 c = TBL_U16(table, ((u32) tok >> 4) << 1);

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
extern u8 D_8020ED00[]; /* heap end when (D_80364AA8 & 0x20) */
extern u8 D_8021DD00[]; /* heap end otherwise */

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
#ifdef PORT_HOST
        {
            /* trailer bytes 0x1007-0x100F: never written (stale heap) */
            void port_garbage(const void *p, u32 len);

            port_garbage(&b->pad1006[1], 9);
        }
#endif
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5FA8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

extern u8 D_803EB792;

/* Sets up a D_803C4B70 record for definition `def` (s16 id at +0, width and
 * height bytes at +2, +3). Stores io->a3 in D_803EB792, takes the first
 * inactive record (byte 0x33 == 0; none of 16 -> return 0), and gives it
 * def[2] * def[3] free slots of the 16-byte map D_803EB770 (their indices go
 * to rec+0x37.., then each is marked 1; running out returns 0 with the
 * indices written so far). Then: active = 1, +0 = def, +4 = data, +0x34 =
 * type, +0x32 = 0, +0x35 = b35, +0x3B = D_803EB792. Type != 1: words
 * +8/+0xC/+0x10 = x/y/z, +0x20/+0x24/+0x28 = io->t6/w24/w28, +0x14/+0x18/
 * +0x1C = io->s1/w18/w1C, +0x2C = w2C, +0x36 = 0. Type 1 with z != 0: the
 * position comes from record (x, y) of func_802ABC88 (three words << 11),
 * type becomes 0, the other words 0 and +0x2C = 0xFC180000. Type 1 with
 * z == 0: bytes +0x30/+0x31 = x/y. Finally io->t6 = (s16) def[0] and, when
 * that isn't -1, io->s1 = the record's buffer (D_803EB770 - 0x1000 +
 * index * 0x100) and func_802A11C4(id, buffer). Returns 1 (0 on failure).
 * The asm takes its inputs in t0-t5, t6, t7, s0-s5 and a3 and returns in
 * t0 (conventions.txt); a3, t6 and s1 are also outputs (several asm callers
 * read them), and so are f12/f14, left as func_802A11C4's libultra calls
 * leave them, which C can't express. It restores v0, v1, a0-a2, s6, s7. Asm
 * callers keep a0-a2 and t7 live (a mixed N64 build would need a thunk; the
 * native port won't). */
s32 func_802A6274(Io802A6274 *io, u8 *def, s32 data, s32 type, s32 x, s32 y, s32 z, s32 w24, s32 w28,
                  s32 w18, s32 w1C, s32 w2C, s32 b35) {
    u8 *rec = (u8 *) D_803C4B70;
    u8 *slot;
    s32 i;
    s32 k;
    s32 n;

    D_803EB792 = io->a3;
    for (i = 0;; i++) {
        if (i == 16) {
            return 0;
        }
        if (rec[0x33] == 0) {
            break;
        }
        rec += 0x3C;
    }

    n = def[2] * def[3];
    slot = rec + 0x37;
    for (k = 0; n != 0; k++) {
        if (k == 16) {
            return 0;
        }
        io->a3 = D_803EB770[k];
        if (io->a3 == 0) {
            *slot++ = k;
            n--;
        }
    }
    n = def[2] * def[3];
    slot = rec + 0x37;
    while (n != 0) {
        n--;
        D_803EB770[*slot++] = 1;
    }

    rec[0x33] = 1;
    *(u8 * N64P *) (rec + 0x00) = def;
    *(s32 *) (rec + 0x04) = data;
    rec[0x34] = type;
    rec[0x32] = 0;
    rec[0x35] = b35;
    rec[0x3B] = D_803EB792;
    if (type != 1) {
        *(s32 *) (rec + 0x08) = x;
        *(s32 *) (rec + 0x0C) = y;
        *(s32 *) (rec + 0x10) = z;
        *(s32 *) (rec + 0x20) = io->t6;
        *(s32 *) (rec + 0x24) = w24;
        *(s32 *) (rec + 0x28) = w28;
        *(s32 *) (rec + 0x14) = io->s1;
        *(s32 *) (rec + 0x18) = w18;
        *(s32 *) (rec + 0x1C) = w1C;
        *(s32 *) (rec + 0x2C) = w2C;
        rec[0x36] = 0;
    } else if (z != 0) {
        u8 *src;

        func_802ABC88(x, y, &src);
        rec[0x34] = 0;
        *(s32 *) (rec + 0x08) = ((s32 *) src)[0] << 11;
        *(s32 *) (rec + 0x0C) = ((s32 *) src)[1] << 11;
        *(s32 *) (rec + 0x10) = ((s32 *) src)[2] << 11;
        *(s32 *) (rec + 0x20) = 0;
        *(s32 *) (rec + 0x24) = 0;
        *(s32 *) (rec + 0x28) = 0;
        *(s32 *) (rec + 0x14) = 0;
        *(s32 *) (rec + 0x18) = 0;
        *(s32 *) (rec + 0x1C) = 0;
        *(s32 *) (rec + 0x2C) = 0xFC180000;
        rec[0x36] = 0;
    } else {
        rec[0x30] = x;
        rec[0x31] = y;
    }

    io->t6 = *(s16 *) def;
    if (io->t6 != -1) {
        io->s1 = (s32) (D_803EB770 - 0x1000 + i * 0x100);
        func_802A11C4(io->t6, (void *) N64_IPTR(io->s1));
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6274.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Gfx * N64P D_803EB780;
extern Gfx * N64P D_803EB784;
extern s32 D_803C4F70[]; /* 16.16 matrices (0x40 bytes each), frame 0 / 1 */
extern s32 D_803C5370[];
extern Vtx D_803C8770[]; /* quad vertices, frame 0 / 1 */
extern Vtx D_803C9770[];
extern u8 D_803CA770[];  /* fallback texture buffers, frame 0 / 1 */
extern u8 D_803DA770[];
extern u8 D_803EA770[];  /* 0x100-byte load parameter per record */
void func_802A6748(void);
u8 *func_802A67C4(u8 *rec, u8 *idx, u8 *base, u16 **list, u8 *param);
void func_802A68D4(s32 *m, u8 *rec);
void func_802A6C10(s32 w, Vtx *v, u8 *col, s32 x, s32 y, s32 h);
void func_802A6D34(void);
void func_802A6DE8(u8 *obj, Vtx *vtx, void *timg, s32 fmtsiz, s32 width, s32 height, s32 zbuf);
Gfx * N64P *func_802A6EB8(u8 *obj);

/* Draws the 16 D_803C4B70 billboard records into the display lists of this
 * frame's buffer set (D_8035805C picks lists, matrices, vertices and texture
 * buffers): resets both list cursors (D_803EB780/784, then func_802A6D34).
 * For each active record: its matrix (func_802A68D4, matrices advance by
 * 0x40 per drawn record), then def[3] rows of def[2] quads of size w x h
 * (def u16 +0xA, +0xC) centred on the origin, x from (cols * w) >> 1 down by
 * w, y from -((rows * h) >> 1) up by h: each quad's vertices (func_802A6C10,
 * colour from def), its texture (func_802A67C4 with the record's slot bytes
 * at +0x37, the frame's id list at def + 0x10 + 2 * cols * rows * frame and
 * the record's 0x100-byte parameter) and the textured quad (func_802A6DE8,
 * format def u16 +4, z-buffer byte +0x35); then a G_POPMTX in the record's
 * list (func_802A6EB8), and the frame (+0x32) advances, or at def u16 +0xE
 * the record ends (inactive, its map slots in D_803EB770 freed). Finally
 * both lists get a G_ENDDL (cursors not advanced) and the heap blocks age
 * (func_802A6748). The asm saves s0-s7, gp, fp; the survey's inputs (a1, s1,
 * s2) are only saved. The `sub` stepping x traps on overflow (game range). */
void func_802A64A4(void) {
    s32 *mtx;
    Vtx *vtx;
    u8 *texBase;
    u8 *rec = (u8 *) D_803C4B70;
    u8 *param = D_803EA770;
    s32 n;

    if (D_8035805C != 0) {
        D_803EB780 = D_803C6370;
        D_803EB784 = D_803C7B70;
        mtx = D_803C5370;
        vtx = D_803C9770;
        texBase = D_803DA770;
    } else {
        D_803EB780 = D_803C5770;
        D_803EB784 = D_803C6F70;
        mtx = D_803C4F70;
        vtx = D_803C8770;
        texBase = D_803CA770;
    }
    func_802A6D34();
    for (n = 16; n != 0; n--, rec += 0x3C, param += 0x100) {
        u8 *def;
        u8 *idx;
        u16 *list;
        u32 cols;
        u32 rows;
        s32 w;
        s32 h;
        s32 fmtsiz;
        s32 zbuf;
        s32 x0;
        s32 y;
        s32 frame;
        Gfx * N64P *cur;
        Gfx *g;

        if (rec[0x33] == 0) {
            continue;
        }
        func_802A68D4(mtx, rec);
        def = *(u8 * N64P *) rec;
        zbuf = rec[0x35];
        idx = rec + 0x37;
        cols = def[2];
        rows = def[3];
        h = *(u16 *) (def + 0xC);
        fmtsiz = *(u16 *) (def + 4);
        w = *(u16 *) (def + 0xA);
        list = (u16 *) (def + ((cols * rows * rec[0x32]) << 1) + 0x10);
        x0 = (s32) (cols * w) >> 1;
        y = -((s32) (rows * h) >> 1);
        while (rows != 0) {
            s32 x = x0;
            u32 c;

            rows--;
            for (c = cols; c != 0;) {
                u8 *buf;

                c--;
                func_802A6C10(w, vtx, def, x, y, h);
                buf = func_802A67C4(rec, idx, texBase, &list, param);
                idx++;
                func_802A6DE8(rec, vtx, buf, fmtsiz, w, h, zbuf);
                vtx += 4;
                x -= w;
            }
            y += h;
        }
        mtx += 0x10;
        cur = func_802A6EB8(rec);
        g = *cur;
        g->words.w0 = 0xBD000000;
        g->words.w1 = 0;
        *cur = g + 1;
        def = *(u8 * N64P *) rec;
        frame = rec[0x32] + 1;
        if (frame != *(u16 *) (def + 0xE)) {
            rec[0x32] = frame;
        } else {
            u8 *p = rec + 0x37;
            u32 k = def[2] * def[3];

            rec[0x33] = 0;
            while (k != 0) {
                k--;
                D_803EB770[*p++] = 0;
            }
        }
    }
    D_803EB780->words.w0 = 0xB8000000;
    D_803EB780->words.w1 = 0;
    D_803EB784->words.w0 = 0xB8000000;
    D_803EB784->words.w1 = 0;
    func_802A6748();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A64A4.s")
#endif

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
#ifdef NON_MATCHING
/* D_80305C10 (top of this file): 0-terminated list of definitions that get a heap block */

/* Texture buffer for record `rec` (its def word at +0 and byte +0x32): if a
 * heap block (D_803EB788 .. D_803EB78C, walked with `!=`) is in use for that
 * def and byte, resets its age and returns it (nothing loaded). Otherwise, if
 * blocks remain (D_803EB790 != 0) and the def is in the list D_80305C10, takes
 * one (D_803EB790--, first block with inUse == 0, unbounded), tags it with
 * the def and byte, age 0; failing that the buffer is base + (*idx << 12).
 * Either way the next u16 id from *list (advanced by 2) is loaded into the
 * buffer with func_802A1074(id, buffer, param). Returns the buffer.
 * Register convention: asm takes rec, idx, base, list, param in t4, a0, t3,
 * t9, fp and returns the buffer in s1 and the advanced list in t9
 * (conventions.txt); it restores v0-a3 and leaves t6 = the id loaded. Its asm
 * caller func_802A64A4 keeps a0, a1, t0, t1, t3, t4, t5, t7, t8 live. */
u8 *func_802A67C4(u8 *rec, u8 *idx, u8 *base, u16 **list, u8 *param) {
    HeapBlock *b = D_803EB788;
    HeapBlock *end = D_803EB78C;
    s32 key = rec[0x32];
    u32 def = *(u32 *) rec;
    u8 * N64P *l;
    u8 *buf;

    for (; b != end; b++) {
        if (b->inUse == 0 || def != (u32) b->inUse || key != b->pad1006[0]) {
            continue;
        }
        b->age = 0;
        return (u8 *) b;
    }
    if (D_803EB790 == 0) {
        goto fallback;
    }
    for (l = D_80305C10;; l++) {
        if ((u32) *l == def) {
            break;
        }
        if (*l == NULL) {
            goto fallback;
        }
    }
    D_803EB790--;
    for (b = D_803EB788; b->inUse != 0; b++) {
    }
    b->inUse = def;
    b->pad1006[0] = key;
    b->age = 0;
    buf = (u8 *) b;
    goto load;
fallback:
    buf = base + (*idx << 12);
load:
    func_802A1074(**list, buf, param);
    (*list)++;
    return buf;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A67C4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_803C4F30[]; /* scratch 16.16 matrix */

/* Billboard matrix of record rec into the 16.16 matrix m: emits a matrix
 * load of m (gSPMatrix word 0x01040040, physical address) into the record's
 * display list (func_802A6EB8), sets m to the uniform scale rec+4
 * (func_802ACC68), then the position: type 1 (+0x34) records use the words of
 * func_802ABC88(+0x30, +0x31)'s record; others move by velocity (+0x14..+0x1C)
 * times the step byte +0x36 plus +0x20..+0x28 (stored back into +8..+0x10),
 * and once y (+0xC) drops to the floor +0x2C the y speed +0x24 becomes
 * |+0x24 + +0x18 * step| * 3/4 (v - (v >> 2)) and the step restarts (it is
 * stored back + 1); the position is >> 11. The pitch is the arcsine of
 * (player y - y) / distance (func_802ABCDC on the player position >> 11,
 * logical shifts; 64-bit division of dy << 27 by distance << 11, distance 0
 * taken as 1), >> 4 and mirrored (0xFFF - angle) unless the quotient is
 * negative: m *= rotation about x by that (func_802ACBDC, func_802ACCCC),
 * m *= rotation about y by D_80364452, m *= translation (position << 11),
 * then m is converted to the Mtx layout (func_802AC8CC).
 * Register convention: m in t0, rec in t4; the asm saves every integer
 * register (asm caller func_802A64A4 keeps a1, t0, t1, t3, t4, t5 live). The
 * f12/f14 the survey lists as read by func_802A64A4 are dead there. The asm's
 * ddiv traps on a zero divisor (distance << 11 wrapping to 0; game range). */
void func_802A68D4(s32 *m, u8 *rec) {
    Gfx * N64P *cur = func_802A6EB8(rec);
    Gfx *g = *cur;
    s32 scale;
    s32 x;
    s32 y;
    s32 z;
    s32 dist;
    s32 py;
    s64 q;
    s32 angle;

    g->words.w1 = (u32) m & 0x1FFFFFFF;
    g->words.w0 = 0x01040040;
    *cur = g + 1;
    scale = *(s32 *) (rec + 4);
    func_802ACC68(scale, scale, scale, m);
    if (rec[0x34] == 1) {
        u8 *src;

        func_802ABC88(rec[0x30], rec[0x31], &src);
        x = ((s32 *) src)[0];
        y = ((s32 *) src)[1];
        z = ((s32 *) src)[2];
    } else {
        s32 step = rec[0x36];
        s32 y0 = *(s32 *) (rec + 0xC);

        if (*(s32 *) (rec + 0x2C) >= y0) {
            s32 v = *(s32 *) (rec + 0x24) + *(s32 *) (rec + 0x18) * step;

            if (v < 0) {
                v = -v;
            }
            *(s32 *) (rec + 0x24) = v - (v >> 2);
            step = 0;
        }
        rec[0x36] = step + 1;
        x = *(s32 *) (rec + 0x08) + *(s32 *) (rec + 0x14) * step + *(s32 *) (rec + 0x20);
        *(s32 *) (rec + 0x08) = x;
        x >>= 11;
        y = y0 + *(s32 *) (rec + 0x18) * step + *(s32 *) (rec + 0x24);
        *(s32 *) (rec + 0x0C) = y;
        y >>= 11;
        z = *(s32 *) (rec + 0x10) + *(s32 *) (rec + 0x1C) * step + *(s32 *) (rec + 0x28);
        *(s32 *) (rec + 0x10) = z;
        z >>= 11;
    }
    py = (u32) D_803643FC >> 11;
    dist = func_802ABCDC((u32) D_803643F8 >> 11, py, (u32) D_80364400 >> 11, x, y, z);
    if (dist == 0) {
        dist = 1;
    }
    q = ((s64) (py - y) << 27) / (s32) ((u32) dist << 11);
    angle = func_802AD7FC((q >= 0) ? (s32) q : -(s32) q) >> 4;
    if (q >= 0) {
        angle = 0xFFF - angle;
    }
    func_802ACBDC(angle, D_803C4F30);
    func_802ACCCC(D_803C4F30, m);
    func_802ACAC4(((u16) D_80364452), D_803C4F30);
    func_802ACCCC(D_803C4F30, m);
    func_802ACA60(x << 11, y << 11, z << 11, D_803C4F30);
    func_802ACCCC(D_803C4F30, m);
    func_802AC8CC((u16 *) m);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A68D4.s")
#endif

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
extern Gfx * N64P D_803EB780;
extern Gfx * N64P D_803EB784;

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
#ifdef NON_MATCHING
Gfx * N64P *func_802A6EB8(u8 *obj);

/* Draws a textured quad into obj's display-list cursor (func_802A6EB8 picks
 * D_803EB780 or D_803EB784): *cursor = func_802575F4(*cursor, vtx, timg,
 * fmtsiz, width, height, zbuf). The asm takes obj, vtx, timg, fmtsiz, width,
 * height, zbuf in t4, t1, s1, t8, v1, s5, gp (conventions.txt), saves every
 * register it uses except s0 (obj copy) and t2, and passes fmtsiz to
 * func_802575F4 as the full register (the C prototype's s16 narrows it).
 * Asm caller func_802A64A4 keeps a0-a3, t0, t1, t3-t5, t7-t9, f12, f14 live
 * (a mixed N64 build would need a thunk; the native port won't). */
void func_802A6DE8(u8 *obj, Vtx *vtx, void *timg, s32 fmtsiz, s32 width, s32 height, s32 zbuf) {
    Gfx *gfx = func_802575F4(*func_802A6EB8(obj), vtx, timg, fmtsiz, width, height, zbuf);

    *func_802A6EB8(obj) = gfx;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6DE8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Picks a display-list cursor: &D_803EB784 if byte 0x3B of `obj` is set,
 * else &D_803EB780 (obj in t4, result in t2 in the asm). Asm callers keep
 * registers live across the call: func_802A64A4 a0-a3, t0, t1, t3-t5,
 * f12, f14; func_802A68D4 a0-a3, t0, t4, f12, f14; func_802A6DE8 t1, t8
 * (a mixed N64 build would need a thunk, the native port doesn't). */
Gfx * N64P *func_802A6EB8(u8 *obj) {
    if (obj[0x3B] != 0) {
        return &D_803EB784;
    }
    return &D_803EB780;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6EB8.s")
#endif
