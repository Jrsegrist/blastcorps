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
#ifdef NON_MATCHING
/* Shared declarations for the NON_MATCHING (port) rewrites below. */
extern u32 * N64P D_803B8D44;         /* ROM offsets (from 0x4CE0) of the entries, 8 bytes apart */
extern u8 * N64P D_803B8D40;
extern u8 D_803B8570[];
/* Entry n of the D_803B8D44 table spans ROM [0x4CE0 + w[2n], 0x4CE0 + w[2n + 2]). */
#define TABLE_ROM_BASE 0x4CE0

/* Entry n of D_803B8D44 (8 bytes): ROM offset, then the unpacked size and the
 * decoder type as u16s (the next entry's offset ends it). */
typedef struct {
    /* 0x0 */ u32 rom;
    /* 0x4 */ u16 size;
    /* 0x6 */ u16 type;
} TexTableEntry;

/* The 16-byte decode request handed to func_802A57DC (60F60.c's DecodeReq). */
typedef struct {
    /* 0x0 */ u8 * N64P data;
    /* 0x4 */ u32 size;
    /* 0x8 */ s32 type;
    /* 0xC */ u8 * N64P param;
} DecodeReq;
extern DecodeReq D_803C4B58;

/* D_803B8570..D_803B8D40: the loaded-texture cache, 8-byte {id, physical
 * address} pairs; D_803B8D40 is the first free pair. */

/* Loads table entry `id` at the heap pointer D_80358070 (into `buf`):
 * invalidates 0x1000 bytes there, DMAs the packed entry in and waits, decodes
 * it in place through D_803C4B58 (func_802A57DC; the caller sets .param where
 * the asm does) and advances the heap by the decoded size. */
#define LOAD_TABLE_ENTRY(id, buf)                                                                 \
    {                                                                                             \
        TexTableEntry *e_ = (TexTableEntry *) ((u8 *) D_803B8D44 + (id) * 8);                     \
        s32 n_;                                                                                   \
                                                                                                  \
        (buf) = D_80358070;                                                                       \
        osInvalDCache(buf, 0x1000);                                                               \
        D_803C4B58.size = e_->size;                                                               \
        D_803C4B58.type = e_->type;                                                               \
        D_803C4B58.data = (buf);                                                                  \
        osPiStartDma(&D_80370C58, OS_MESG_PRI_NORMAL, OS_READ, TABLE_ROM_BASE + e_->rom, buf,     \
                     ((TexTableEntry *) ((u8 *) e_ + 8))->rom - e_->rom, &D_80315180);            \
        osRecvMesg(&D_80315180, NULL, OS_MESG_BLOCK);                                             \
        n_ = func_802A57DC((u8 *) &D_803C4B58);                                                   \
        D_80358070 += n_;                                                                         \
    }
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
#ifdef NON_MATCHING

/* C-callable entry: func_802A08E4(dl, end) (dl/end moved to s0/s1, which it
 * saves). The asm passes its caller's s4 through as func_802A08E4's in/out
 * s4 and doesn't restore s2-s6 (func_802A08E4's outputs/scratch); nothing
 * calls it directly (no jal to it in the ROM), so `r` is just a local here. */
void func_802A08B4(u32 *dl, u32 *end) {
    Unk802A08E4Regs r;

    func_802A08E4(dl, end, &r);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A08B4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Walks the display list [dl, end) (8-byte commands) and, for each
 * G_SETTIMG (0xFD), replaces its address word (a table id) with the physical
 * address of that texture: from the cache pairs if the id is there, else it
 * appends a pair, loads the entry at the heap (LOAD_TABLE_ENTRY, .param left
 * as it is) and caches its physical address (heap - 0x80000000). Stores the
 * new cache end in D_803B8D40.
 * Register convention: dl in s0, end in s1; outputs s2, s3, s4 through `r`
 * (conventions.txt). The asm saves every other register except s0 (= end
 * after), s5, s6 (scratch); its add/sub trap. Asm callers keep a1-a3, t1-t4
 * live; they also rely on f12/f14 surviving the libultra calls. */
void func_802A08E4(u32 *dl, u32 *end, Unk802A08E4Regs *r) {
    u8 *cur = D_803B8D40;
    u8 *p;
    u8 *buf;
    u32 id;

    while (dl != end) {
        dl += 2;
        if (((dl[-2] & 0xFF000000) >> 24) != 0xFD) {
            continue;
        }
        id = dl[-1];
        for (p = D_803B8570; p != cur; p += 8) {
            if (*(u32 *) p == id) {
                break;
            }
        }
        r->s4 = p;
        if (p == cur) {
            *(u32 *) cur = id;
            LOAD_TABLE_ENTRY(id, buf);
            ((u32 *) cur)[1] = (u32) buf - 0x80000000;
            cur += 8;
        }
        dl[-1] = ((u32 *) p)[1];
    }
    D_803B8D40 = cur;
    r->s2 = &D_803B8D40;
    r->s3 = cur;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A08E4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802A0B34(s32 id, u8 *param);

/* C-callable entry: func_802A0B34(id, param) (moved to t6/fp). The asm
 * returns its caller's s0 in v0 (a leftover); C callers declare it void.
 * The whole id register is used (2E490.c/2D810.c declare u16). */
void func_802A0B00(s32 id, u8 *param) {
    func_802A0B34(id, param);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0B00.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Loads table entry `id` at the heap (LOAD_TABLE_ENTRY with .param = param),
 * uncached. Register convention: id in t6, param in fp (conventions.txt); the
 * asm saves every register (it relies on the libultra calls keeping f12/f14,
 * which C needn't). */
void func_802A0B34(s32 id, u8 *param) {
    u8 *buf;

    D_803C4B58.param = param;
    LOAD_TABLE_ENTRY(id, buf);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0B34.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING

/* C-callable entry: returns func_802A0CFC(id, param), the texture's physical
 * address (the asm moves id/param to t6/fp and returns s0 in v0). The whole
 * id register is used (callers declare s16 or s32). */
u32 func_802A0CC8(s32 id, u8 *param) {
    return func_802A0CFC(id, param);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0CC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Physical address of texture `id`: from the cache pairs D_803B8570.. if
 * present, else appends a pair, loads the entry at the heap
 * (LOAD_TABLE_ENTRY with .param = param) and caches heap - 0x80000000.
 * Register convention: id in t6, param in fp, result in s0
 * (conventions.txt); the asm saves every other register. Asm callers keep
 * t1, t2, t4-t7 live and rely on f12/f14 surviving the libultra calls. */
u32 func_802A0CFC(s32 id, u8 *param) {
    u8 *cur = D_803B8D40;
    u8 *p;
    u8 *buf;
    u32 phys;

    for (p = D_803B8570; p != cur; p += 8) {
        if (*(s32 *) p == id) {
            return ((u32 *) p)[1];
        }
    }
    *(s32 *) cur = id;
    D_803B8D40 = cur + 8;
    D_803C4B58.param = param;
    LOAD_TABLE_ENTRY(id, buf);
    phys = (u32) buf - 0x80000000;
    ((u32 *) cur)[1] = phys;
    return phys;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A0CFC.s")
#endif

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
#ifdef NON_MATCHING

/* C-callable entry: func_802A1074(id, dest, param) (moved to t6/s1/fp). The
 * whole id register is used (168B0.c declares u16, 32E00.c/43A60.c s16). */
void func_802A1040(s32 id, u8 *dest, u8 *param) {
    func_802A1074(id, dest, param);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A1040.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B8D48[];

/* Queued load of table entry `id` into `dest`: invalidates 0x1000 bytes,
 * starts the DMA with the next 0x14-byte OSIoMesg slot of D_803B8D48
 * (D_80358080++) without waiting, and queues the decode request
 * (dest, size, type, param) with func_802A5764.
 * Register convention: id in t6, dest in s1, param in fp (conventions.txt);
 * the asm saves every register it uses. Asm caller func_802A67C4 keeps t9. */
void func_802A1074(s32 id, u8 *dest, u8 *param) {
    TexTableEntry *e;
    s32 slot;

    osInvalDCache(dest, 0x1000);
    slot = D_80358080++;
    e = (TexTableEntry *) ((u8 *) D_803B8D44 + id * 8);
    osPiStartDma((OSIoMesg *) (D_803B8D48 + slot * 0x14), OS_MESG_PRI_NORMAL, OS_READ, TABLE_ROM_BASE + e->rom,
                 dest, e[1].rom - e->rom, &D_80315180);
    func_802A5764((s32) dest, e->size, e->type, (s32) param);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5BF40/func_802A1074.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
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
