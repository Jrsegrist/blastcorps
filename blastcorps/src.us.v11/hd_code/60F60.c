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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5764.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A57DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5958.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5A2C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5AE0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5B90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5C5C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5D34.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5E10.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A5ED0.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6C10.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/60F60/func_802A6EB8.s")
