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
extern u8 *D_80358070;          /* heap pointer */
extern OSIoMesg D_80370C58;
extern OSMesgQueue D_80315180;
extern u8 D_803B9888;           /* the table below is loaded */
extern u32 *D_803B8D44;         /* ROM offsets (from 0x4CE0) of the entries, 8 bytes apart */
extern u8 *D_803B8D40;
extern u8 D_803B8570[];
/* Entry n of the D_803B8D44 table spans ROM [0x4CE0 + w[2n], 0x4CE0 + w[2n + 2]). */
#define TABLE_ROM_BASE 0x4CE0
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Once (until D_803B9888 is set): takes 0x8000 heap bytes for the table
 * D_803B8D44, DMAs ROM 0x4CE0..+0x8000 into it and waits, then points
 * D_803B8D40 at D_803B8570.
 * The asm saves and restores every register; func_802A1674 keeps t0, t6, t9
 * across it. (Its survey outputs f12/f14 are what the libultra calls leave.) */
void func_802A0700(void) {
    u8 *buf;

    if (D_803B9888 == 0) {
        buf = D_80358070;
        D_803B8D44 = (u32 *) buf;
        D_80358070 = buf + 0x8000;
        osInvalDCache(buf, 0x8000);
        osPiStartDma(&D_80370C58, OS_MESG_PRI_NORMAL, OS_READ, TABLE_ROM_BASE, buf, 0x8000, &D_80315180);
        osRecvMesg(&D_80315180, NULL, OS_MESG_BLOCK);
        D_803B8D40 = D_803B8570;
        D_803B9888 = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0700.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A08B4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A08E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0B00.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0B34.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0CC8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0CFC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802A0F0C(s32 id, void *dest);

/* func_802A0F0C(id, dest) (the asm moves them to t6/s1 and saves s1).
 * 2E490.c declares it (u16 id, void *dest); the asm uses the whole register. */
void func_802A0EE0(s32 id, void *dest) {
    func_802A0F0C(id, dest);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0EE0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* DMAs entry `id` of the D_803B8D44 table (ROM 0x4CE0 + w[2id] up to the next
 * entry's start) into the 0x100-byte buffer `dest` and waits for it.
 * asm: id in t6, dest in s1; it saves every register but s0/s1 (unchanged). */
void func_802A0F0C(s32 id, void *dest) {
    u32 *e;

    osInvalDCache(dest, 0x100);
    e = D_803B8D44 + id * 2;
    osPiStartDma(&D_80370C58, OS_MESG_PRI_NORMAL, OS_READ, TABLE_ROM_BASE + e[0], dest, e[2] - e[0],
                 &D_80315180);
    osRecvMesg(&D_80315180, NULL, OS_MESG_BLOCK);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0F0C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A1040.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A1074.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80358080; /* queued-DMA counter (next OSIoMesg slot) */
extern s32 D_80358084;
extern u8 D_803B8D48[]; /* OSIoMesg slots, 0x14 bytes each */

/* Like func_802A0F0C but doesn't wait: counts D_80358084 up and issues the
 * DMA with the next 0x14-byte OSIoMesg slot of D_803B8D48 (D_80358080++).
 * asm: id in t6, dest in s1; it saves every register but s0/s1 (unchanged).
 * Asm callers rely on preserved: func_802A6274 keeps a3, t6. (Its survey
 * outputs f12/f14 are what the libultra calls leave.) */
void func_802A11C4(s32 id, void *dest) {
    u32 *e;
    s32 slot;

    osInvalDCache(dest, 0x100);
    D_80358084++;
    slot = D_80358080++;
    e = D_803B8D44 + id * 2;
    osPiStartDma((OSIoMesg *) (D_803B8D48 + slot * 0x14), OS_MESG_PRI_NORMAL, OS_READ,
                 TABLE_ROM_BASE + e[0], dest, e[2] - e[0], &D_80315180);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A11C4.s")
#endif
