#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* FILE-WIDE FINDING: hand-written assembly (sd/ld $ra frames, trapping
 * addi on $sp, every register saved including k0/k1/gp/sp), split out of the
 * old 7D9D0_data bin blob. Like the 56040..8DDB0 block, these stay GLOBAL_ASM
 * permanently in the matching build. Data on both sides (7D9D0_data,
 * 800E0_data) stays bin; both boundaries are 16-aligned. */

#ifdef NON_MATCHING
/* Bit-reader state of the LZSS decompressor (the asm keeps it in a0, t3, t4). */
typedef struct {
    /* 0x0 */ u8 *src; /* a0: next source byte */
    /* 0x4 */ u32 cur; /* t3: current source byte */
    /* 0x8 */ u32 bit; /* t4: mask of the next bit of `cur` (0x80 = load a new byte first) */
} LzBits;

u32 func_802C42CC(u32 mask, LzBits *b);
void func_802C41C0(u8 **srcp, u8 **dstp, u8 *window, s32 bits);

extern u8 *D_803F7830;
extern u8 *D_803F7834;
extern s16 D_803F7840;  /* last |level| set by func_802C4584 (-1 = none yet) */
extern void *D_803F7844; /* looping sound handle */
extern void *D_803F7848; /* second looping sound handle */
extern s16 D_803F784C;  /* its on/off flag; func_802C4724 uses the first byte */
extern void *D_80367738;
extern s32 D_80364AB0;  /* force the next func_802C4584 update */
extern u8 D_80364456;
extern f32 D_8030D940;
extern f32 D_8030D944;
extern u8 D_80370C1A;
extern u8 D_80370C1B;
#endif

/* C-callable (46C20.c func_8028B4C4): saves s0-s7/gp/s8, then
 * func_802C41C0(*a0, *a1) and writes the advanced pointers back to *a0/*a1. */
#ifdef NON_MATCHING
/* Decompress the LZSS stream at *src into *dst (window `work`, `type` offset
 * bits), advancing both pointers. */
void func_802C4070(u32 *src, u32 *dst, u32 work, u8 type) {
    func_802C41C0((u8 **) src, (u8 **) dst, (u8 *) work, type);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4070.s")
#endif

/* Saves s-regs, stores a0/a1 to D_803F7830/D_803F7834 and calls
 * func_8025C230(&D_803F7830, &D_803F7834, a2); returns the updated values in
 * a0/a1 (non-ABI outputs). */
#ifdef NON_MATCHING
/* Register convention: the asm takes and returns the two pointers in a0/a1
 * (here through src/dst; conventions.txt) and restores a2 and every
 * s-register. Asm callers (5CB60's func_802A2A98, func_802A32CC,
 * func_802A396C) keep a2, f12 and f14 live across the call. */
void func_802C4108(u8 **src, u8 **dst, s32 work) {
    D_803F7830 = *src;
    D_803F7834 = *dst;
    func_8025C230(&D_803F7830, &D_803F7834, work);
    *src = D_803F7830;
    *dst = D_803F7834;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4108.s")
#endif

/* Bit-packed LZSS decompressor: a0 = source, a1 = destination, a2 = ring
 * window, a3 = offset width in bits (window = 1 << a3, length field
 * 16 - a3 bits). Flag bits are read MSB-first: 1 = 8-bit literal (to the
 * output and the window), 0 = window offset (0 ends) + length, copying
 * length + 3 bytes. Returns the source end (rounded up to even) in a0 and the
 * destination end in a1. Non-ABI: clobbers s0/s4-s7 (func_802C4070 saves
 * them). */
#ifdef NON_MATCHING
/* Window writes start at index 1. A copy reads window[(off + i) & mask] one
 * byte at a time, so it may read bytes it has just written. `bits` must be
 * 1..15 (game data uses 10 and 12; the asm shifts by it with sllv).
 * Register convention: src/dst in and out in a0/a1 (here through the
 * pointers); clobbers s0, s4-s7 (conventions.txt). */
void func_802C41C0(u8 **srcp, u8 **dstp, u8 *window, s32 bits) {
    LzBits b;
    u8 *dst = *dstp;
    u32 wmask = (1 << bits) - 1;
    u32 offMask = 1 << (bits - 1);
    u32 lenMask = 1 << (15 - bits);
    u32 pos = 1;
    u32 flag;
    u32 off;
    s32 len;
    s32 i;
    u8 c;

    b.src = *srcp;
    b.cur = 0;
    b.bit = 0x80;
    for (;;) {
        if (b.bit == 0x80) {
            b.cur = *b.src++;
        }
        flag = b.cur & b.bit;
        b.bit >>= 1;
        if (b.bit == 0) {
            b.bit = 0x80;
        }
        if (flag != 0) {
            c = func_802C42CC(0x80, &b);
            *dst++ = c;
            window[pos] = c;
            pos = (pos + 1) & wmask;
        } else {
            off = func_802C42CC(offMask, &b);
            if (off == 0) {
                break;
            }
            len = func_802C42CC(lenMask, &b) + 2;
            for (i = 0; i <= len; i++) {
                c = window[(off + i) & wmask];
                *dst++ = c;
                window[pos] = c;
                pos = (pos + 1) & wmask;
            }
        }
    }
    if ((u32) b.src & 1) {
        b.src++;
    }
    *srcp = b.src;
    *dstp = dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C41C0.s")
#endif

/* Bit reader for func_802C41C0: reads bits MSB-first into v0 while the mask
 * a3 shifts right; reader state lives in a0/t3/t4 (s4 = 0x80). */
#ifdef NON_MATCHING
/* Reads one source bit per step while `mask` shifts right to 0 (at least one
 * step): each source bit that is 1 ORs the current mask into the result. The asm reloads when t4 == s4 and steps a0 by s5; its only caller
 * sets s4 = 0x80 and s5 = 1, which are constants here.
 * Register convention: mask a3, state a0/t3/t4 (in/out through b), result
 * v0; clobbers s7. Its asm caller keeps a1, a2, t0, t1, t5 and t6 live. */
u32 func_802C42CC(u32 mask, LzBits *b) {
    u32 v = 0;

    do {
        if (b->bit == 0x80) {
            b->cur = *b->src++;
        }
        if (b->cur & b->bit) {
            v |= mask;
        }
        b->bit >>= 1;
        mask >>= 1;
        if (b->bit == 0) {
            b->bit = 0x80;
        }
    } while (mask != 0);
    return v;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C42CC.s")
#endif

/* Save-everything sound wrapper: resets D_803F7840 to -1, starts the looping
 * effect D_80367738 via func_80260650 (sndPlaySfx) with its handle stored in
 * D_803F7844, then func_802C4584 with s5 = 0. */
#ifdef NON_MATCHING
/* arg0 is ignored; arg1 (a1) is the sound id. The asm also clears the
 * halfword D_803F784C and saves/restores every register (k0/k1/gp/sp too). */
void func_802C4310(s32 arg0, s32 arg1) {
    D_803F7840 = -1;
    D_803F784C = 0;
    func_80260650(D_80367738, arg1, &D_803F7844);
    func_802C4584(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4310.s")
#endif

/* Save-everything sound wrapper: stops the sound handles D_803F7844 and
 * D_803F7848 via func_802608C8 when they are non-null. */
#ifdef NON_MATCHING
/* The asm saves and restores every register; asm callers func_802B18F4 (t7,
 * f12, f14) and func_802D1360 (f12, f14) rely on that (a mixed N64 build
 * would need a thunk; the native port won't). */
void func_802C444C(void) {
    if (D_803F7844 != NULL) {
        func_802608C8(D_803F7844);
    }
    if (D_803F7848 != NULL) {
        func_802608C8(D_803F7848);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C444C.s")
#endif

/* Save-everything sound wrapper: non-ABI input s5 (a signed level). Stores
 * |s5| in D_803F7840 and, when it changed (or D_80364AB0 was set, which it
 * clears), sets the D_803F7844 sound's parameter 0x10 (likely pitch) to
 * 0.5 + |s5| * (D_8030D940, or D_8030D944 when D_80364456 == 1) via
 * func_80260AB8. */
#ifdef NON_MATCHING
/* The change test compares the old halfword (sign-extended) with the full
 * 32-bit |level|. `neg` traps on 0x80000000 (keep levels in game range). The
 * float goes to func_80260AB8 as raw bits in a2.
 * Register convention: level in s5 (conventions.txt); the asm saves and
 * restores every integer register (only f0-f6 change). Asm callers (e.g.
 * func_802B37B0: a0, t6, t7, f12, f14) rely on that (a mixed N64 build would
 * need a thunk; the native port won't). */
void func_802C4584(s32 level) {
    s32 force = D_80364AB0;
    s16 old = D_803F7840;
    union {
        f32 f;
        s32 i;
    } u;

    if (level < 0) {
        level = -level;
    }
    D_803F7840 = level;
    if (force != 0) {
        D_80364AB0 = 0;
    } else if (old == level) {
        return;
    }
    u.f = 0.5f + (f32) level * ((D_80364456 == 1) ? D_8030D944 : D_8030D940);
    func_80260AB8(D_803F7844, 0x10, u.i);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4584.s")
#endif

/* Save-everything sound wrapper: flag = D_80370C1A | D_80370C1B. When it
 * differs from D_803F784C, stores it; on starts the second loop
 * (func_80260650(D_80367738, a1, &D_803F7848)) if not already playing, off
 * stops it (func_802608C8) and clears D_803F7848. */
#ifdef NON_MATCHING
/* The sound id is whatever the caller left in a1 (0x8F, 0xA4, 0x76 ...).
 * Register convention: sfx in a1 (conventions.txt); the asm saves and
 * restores every register. Asm callers (func_802B49AC, func_802B7A88,
 * func_802CBEF0, func_802CD068, func_802CFDE8) keep a0, t6, t7, f12 and f14
 * live (a mixed N64 build would need a thunk; the native port won't). */
void func_802C4724(s32 sfx) {
    u8 flag = D_80370C1A | D_80370C1B;
    void *h = D_803F7848;

    if (*(u8 *) &D_803F784C == flag) {
        return;
    }
    *(u8 *) &D_803F784C = flag;
    if (flag != 0) {
        if (h == NULL) {
            func_80260650(D_80367738, sfx, &D_803F7848);
        }
    } else if (h != NULL) {
        func_802608C8(h);
        D_803F7848 = NULL;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7F8B0/func_802C4724.s")
#endif
