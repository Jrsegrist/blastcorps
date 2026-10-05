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
/* Shared types for the NON_MATCHING (port) rewrites below. Register-keyed
 * functions take their register inputs as ordinary parameters; the mapping is
 * recorded per function in tools_port/conventions.txt. */

/* 0x18-byte animation-channel entry (the -1-terminated table D_803B35F8 and
 * the 0x20-entry blocks built by func_8029F85C). */
typedef struct {
    /* 0x00 */ s32 id;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s8 unk10;
    /* 0x11 */ s8 unk11;
    /* 0x12 */ s8 unk12;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[2];
} Unk8029DEA0Entry; /* size 0x18 */

/* 12-byte pending-copy slot of the 120-entry pool D_803B7FC8. */
typedef struct {
    /* 0x00 */ u8 *src;  /* 0 = free */
    /* 0x04 */ u8 *dst;
    /* 0x08 */ s32 parity;
} Unk8029DCD4Slot;

extern Unk8029DEA0Entry D_803B35F8[];
extern u8 D_803B9890[]; /* 0x60-byte records */
extern u8 *D_803BD300;  /* end of the records in use */
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A800.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029A914.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AA10.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029AB88.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B02C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B514.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803A742B;
extern u8 D_803A7430;

/* If the record's byte 0x4F is 7: sets D_803A742B and counts D_803A7430 up
 * (while it is <= 200).
 * Register convention: asm takes rec in s0 and restores v0 (conventions.txt);
 * its asm callers keep a0-a3, t3-t6, t8, f12, f14 live. */
void func_8029B5B8(u8 *rec) {
    if (rec[0x4F] == 7) {
        D_803A742B = 1;
        if (D_803A7430 < 0xC9) {
            D_803A7430 = D_803A7430 + 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B5B8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B614.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */
extern s32 D_80358064;
extern u8 D_803A742C;
extern u8 D_803A742D;
extern u8 D_803A742E;
extern u8 D_803A742F;
s32 func_8029B930(void);

/* Widens the ring span [D_803A7410, D_803A7412] to take in indices a and b
 * (each first wrapped into 0..0xFFE by one +-0xFFF step, as in the asm).
 * If the span is still the "empty" 0/0xFFF pair it is simply set to a/b.
 * Otherwise B moves back to b when b is at or before B, and A moves on to a
 * when a is after A, comparing 12-bit ring differences (<<20, signed).
 * If the span got longer (func_8029B930 before vs after) and D_80358064 is
 * set, D_803A742F is set, and if D_803A742C is set and D_803A742E clear,
 * D_803A742D steps 1 -> 8, otherwise +1, and D_803A742E is set.
 * Asm callers rely on preserved: func_8029B614 keeps f12, f14;
 * func_8029CB04 keeps t1; func_802CE204 keeps t1, t3-t6, t8, f12, f14
 * (the asm saves t0). The native port doesn't need a thunk. */
void func_8029B7CC(s32 a, s32 b) {
    s32 before;
    u32 ca;
    u32 cb;

    if (a < 0) {
        a += 0xFFF;
    }
    if (a >= 0x1000) {
        a -= 0xFFF;
    }
    if (b < 0) {
        b += 0xFFF;
    }
    if (b >= 0x1000) {
        b -= 0xFFF;
    }
    before = func_8029B930();
    ca = D_803A7410;
    cb = D_803A7412;
    if (ca == 0 && cb == 0xFFF) {
        D_803A7410 = a;
        D_803A7412 = b;
        return;
    }
    ca <<= 20;
    cb <<= 20;
    if ((s32) (((u32) b << 20) - cb) <= 0) {
        cb = (u32) b << 20;
    }
    if ((s32) (((u32) a << 20) - ca) > 0) {
        ca = (u32) a << 20;
    }
    D_803A7410 = ca >> 20;
    D_803A7412 = cb >> 20;
    if (before < func_8029B930() && D_80358064 != 0) {
        D_803A742F = 1;
        if (D_803A742C != 0 && D_803A742E == 0) {
            D_803A742D = (D_803A742D == 1) ? 8 : D_803A742D + 1;
            D_803A742E = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B7CC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u16 D_803A7410; /* ring index A (12-bit, 0..0xFFF) */
extern u16 D_803A7412; /* ring index B */

/* Distance from index A forward to index B in the 0x1000-entry ring, with
 * the asm's quirks kept: the wrapped case uses 0xFFF (not 0x1000), and a
 * non-wrapped distance above 0x800 has 0x800 subtracted. Called from C
 * (48D00, 4B5E0) and from asm func_8029B7CC.
 * Register note: the asm saves/restores v1 and a0 and touches nothing but
 * v0/at; func_8029B7CC keeps a0, a1 and t0 live across the call. This C
 * version is plain o32, so a mixed N64 build would need a thunk preserving
 * those for the asm caller; the native port does not. */
s32 func_8029B930(void) {
    s32 a = D_803A7410;
    s32 b = D_803A7412;

    if (b < a) {
        return b + (0xFFF - a);
    }
    b -= a;
    if (b > 0x800) {
        b -= 0x800;
    }
    return b;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B930.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B994.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BB28.s")

/* func_8029BD0C: a real distance/parametric-closest-point calculation
 * (sum-of-squares via `dmult`, `sqrt.s`, divide, range-check) - no $ra
 * save needed (true leaf), but uses raw `dmtc1`/`dmfc1`/`cvt.s.l`/`cvt.l.s`
 * 64-bit FPU moves directly, the same confirmed-hand-written s64<->float
 * signature documented in the project skill file (IDO only ever emits a
 * runtime helper call for this cast, never these raw instructions).
 * Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BD0C.s")

/* func_8029BEE4: same raw dmtc1/dmfc1/cvt.s.l/cvt.l.s signature as
 * func_8029BD0C above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BEE4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029BF64.s")

/* func_8029C0DC: reads $s0 as a hidden input never set within the
 * function itself (a switch on `$s0->unk4E`, returning other $s0 fields) -
 * same non-ABI hidden-register-input family as func_8029DBF0/func_802A06B4
 * documented elsewhere in this file, just a different register. Permanently
 * GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C0DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C160.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C284.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C354.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C454.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C52C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C5EC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C6E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C748.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C828.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C914.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029C9D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CB04.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CD54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF04.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CF54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029CFA4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D040.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D120.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Finds the first 0x60-byte record from D_803B9890 with byte 0x4F == id and
 * byte 0x50 == sub (no bound: it must exist), clears its byte 0x51 and returns
 * it; *last gets the last byte compared (== sub).
 * Register convention: asm takes sub in t6 and id in t7 and leaves the record
 * in s0 and the byte in s1 (conventions.txt). Its asm caller func_8029D040
 * keeps a1-a3, t0-t2, t4-t7, f12, f14 live. */
u8 *func_8029D1D4(s32 sub, s32 id, s32 *last) {
    u8 *rec = D_803B9890;

    while (rec[0x4F] != id || rec[0x50] != sub) {
        rec += 0x60;
    }
    rec[0x51] = 0;
    *last = rec[0x50];
    return rec;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D1D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns byte 0x51 of the first 0x60-byte record from D_803B9890 whose byte
 * 0x50 == sub (no bound: it must exist).
 * Register convention: asm takes sub in t4 and returns in t5, saving s0/s1
 * (conventions.txt); asm callers keep t0, t3, t6, t7 live. */
s32 func_8029D210(s32 sub) {
    u8 *rec = D_803B9890;

    while (rec[0x50] != sub) {
        rec += 0x60;
    }
    return rec[0x51];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D210.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D24C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* rec[0x55] = (kind != 7).
 * Register convention: asm takes kind in t2 and rec in s0, restoring v0
 * (conventions.txt); asm callers keep a3, t2, t6, t7, f12, f14 live. */
void func_8029D534(s32 kind, u8 *rec) {
    rec[0x55] = (kind == 7) ? 0 : 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D534.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D56C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D90C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DA90.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DB7C.s")

/* TODO: func_8029DBF0 - wrapper around func_8029DC14, but that callee
 * reads its real input out of $v0 (set via a delay-slot `or v0,a0,zero`
 * right before the `jal`, not through the normal a0-a3 argument
 * registers) and returns its result in $v1 instead of $v0 - a hand-tuned
 * non-ABI register convention between these two specific functions, not
 * expressible as a normal C function call. Logic (looking at
 * func_8029DC14 directly): search a fixed-stride struct array starting
 * at D_803B9890 up to D_803BD300's current pointer value for a byte field
 * matching the caller's input, returning found/not-found. Needs either
 * inline asm or leaving as GLOBAL_ASM. (The NON_MATCHING port below uses
 * plain o32 for both; eqcheck maps the v0/v1 convention.) */
#ifdef NON_MATCHING
s32 func_8029DC14(s32 id);

/* Wrapper: returns func_8029DC14(id). 39050.c declares it taking a u8; the
 * asm passes all of a0 through, so this takes s32. */
s32 func_8029DBF0(s32 id) {
    return func_8029DC14(id);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DBF0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803B9890[]; /* 0x60-byte records */
extern u8 *D_803BD300;  /* end of the records in use */

/* Returns 0 if a record in D_803B9890..D_803BD300 (0x60 bytes each) has
 * byte 0x4F == id and byte 0x51 == 0, else 1. The scan stops only at exactly
 * D_803BD300.
 * Register convention: the asm takes id in v0 and returns the result in v1
 * (tools_port/conventions.txt); this C is plain o32. Its asm caller
 * func_802BC3D0 relies on a3, t0, t2, t4, t6, t7, f12 and f14 surviving the
 * call (a mixed N64 build would need a thunk; the native port doesn't). */
s32 func_8029DC14(s32 id) {
    u8 *rec;

    for (rec = D_803B9890; rec != D_803BD300; rec += 0x60) {
        if (rec[0x4F] == id && rec[0x51] == 0) {
            return 0;
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC14.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern Unk8029DCD4Slot D_803B7FC8[120];
extern Unk8029DCD4Slot *D_803B8568; /* last slot in use (sits right after D_803B7FC8) */

/* Resets the 120-entry pool D_803B7FC8: the next-free pointer goes back to
 * the start and words 0 and 4 of every 12-byte entry are cleared.
 * Register note: the asm saves/restores v0 and v1 (clobbers at); asm caller
 * func_802A1674 keeps t0, f12 and f14 live across the call (a mixed N64
 * build would need a thunk; the native port doesn't). */
void func_8029DC80(void) {
    s32 i;

    D_803B8568 = D_803B7FC8;
    for (i = 0; i < 120; i++) {
        D_803B7FC8[i].src = NULL;
        D_803B7FC8[i].dst = NULL;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_8035805C;

/* Queues a 64-byte copy src -> dst in the first free slot (src == 0) of the
 * 120-entry pool D_803B7FC8, tagged with !D_8035805C (low bit flipped), and
 * raises the last-used pointer D_803B8568 to that slot if it is below it
 * (signed compare). Does nothing if the pool is full.
 * Register convention: asm takes dst in t5 and src in s2 (conventions.txt).
 * Asm callers keep a0-a3, t2, t4, t7, f8, f12, f14 live (the asm saves v0, v1,
 * a0); a mixed N64 build would need a thunk, the native port doesn't. */
void func_8029DCD4(u8 *dst, u8 *src) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    s32 n = 120;

    while (n != 0) {
        n--;
        if (slot->src == NULL) {
            slot->src = src;
            slot->dst = dst;
            slot->parity = D_8035805C ^ 1;
            if ((s32) D_803B8568 < (s32) slot) {
                D_803B8568 = slot;
            }
            return;
        }
        slot++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DCD4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Cancels every queued copy whose dst is `dst` (clears src and dst).
 * Quirk kept from the asm: the scan bound is the ADDRESS of D_803B8568 (which
 * sits right after the 120 slots), not its value, so all 120 slots plus a
 * 121st "slot" overlaying D_803B8568 itself are scanned. If that pseudo-slot
 * matches (word D_803B8568+4 == dst), D_803B8568 and the following word are
 * zeroed and D_803B8568 is then set to the last real slot.
 * Register convention: asm takes dst in s2 (conventions.txt); the asm saves
 * v0, v1, a0, and its asm callers keep a0-a3, t2, t4, t7, f8, f12, f14 live. */
void func_8029DD54(u8 *dst) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    Unk8029DCD4Slot *end = (Unk8029DCD4Slot *) &D_803B8568;

    while ((s32) slot <= (s32) end) {
        if (slot->dst == dst) {
            slot->src = NULL;
            slot->dst = NULL;
            if (slot == end) {
                D_803B8568 = slot - 1;
            }
        }
        slot++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DD54.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_8029DE50(u64 *dst, u64 *src);

/* Runs the queued copies: for each used slot of D_803B7FC8 up to D_803B8568
 * (signed compare) whose parity equals D_8035805C, copies 64 bytes src -> dst
 * (func_8029DE50) and frees the slot. Slots with the other parity stay; the
 * last of those becomes the new D_803B8568 (or the pool start if none). */
void func_8029DDC8(void) {
    Unk8029DCD4Slot *slot = D_803B7FC8;
    Unk8029DCD4Slot *last = D_803B7FC8;
    Unk8029DCD4Slot *end = D_803B8568;
    s32 parity = D_8035805C;

    while ((s32) slot <= (s32) end) {
        if (slot->src != NULL) {
            if (slot->parity != parity) {
                last = slot;
            } else {
                func_8029DE50((u64 *) slot->dst, (u64 *) slot->src);
                slot->src = NULL;
                slot->dst = NULL;
            }
        }
        slot++;
    }
    D_803B8568 = last;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DDC8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies 64 bytes (8 doublewords) src -> dst.
 * Register convention: asm takes dst in t5 and src in s2, and restores both
 * plus v0/v1 (conventions.txt). Its asm caller func_8029DDC8 keeps a0 and a2
 * live; the native port needs no thunk. */
void func_8029DE50(u64 *dst, u64 *src) {
    s32 i;

    for (i = 0; i < 8; i++) {
        dst[i] = src[i];
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DE50.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Unk8029DEA0Entry is defined in the file-level NON_MATCHING block at the top. */
extern u8 D_803A7440[12][0x1010];
extern void *D_803B35F0;
extern u8 D_803B3500[];
extern Unk8029DEA0Entry D_803B35F8[];
/* List in the 7D9D0 data blob, ended by -1. In the ROM its 12 entries are
 * addresses inside that blob (0x802C2190...), so `id` is really a pointer. */
extern s32 D_802C23B4[];

/* Clears byte 6 of each of the 12 0x1010-byte blocks at D_803A7440, points
 * D_803B35F0 at D_803B3500, then builds D_803B35F8 from the -1-terminated id
 * list D_802C23B4: one 0x18-byte entry per id with every other field (except
 * unk8) zeroed. The terminating -1 is also stored, as the id of the entry
 * after the last.
 * Register note: the asm saves/restores v0, v1 and a0-a3 (clobbers at and
 * f0); asm caller func_802A1674 keeps t0, t6, t9, f12 and f14 live. */
void func_8029DEA0(void) {
    s32 i;
    s32 *src;
    Unk8029DEA0Entry *dst;
    s32 id;

    for (i = 0; i < 12; i++) {
        D_803A7440[i][6] = 0;
    }
    D_803B35F0 = D_803B3500;

    src = D_802C23B4;
    dst = D_803B35F8;
    for (;;) {
        id = *src;
        dst->id = id;
        if (id == -1) {
            break;
        }
        src++;
        dst->unk4 = 0.0f;
        dst->unkC = 0;
        dst->unkE = 0;
        dst->unk10 = 0;
        dst->unk11 = 0;
        dst->unk12 = 0;
        dst->unk13 = 0;
        dst->unk14 = 0;
        dst++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DEA0.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DF78.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E0AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E21C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E47C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E4E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E558.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E5AC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E730.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E878.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029E938.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EA48.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EB58.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EC68.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EDEC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029EF80.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F060.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F110.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F1BC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F3D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F4B8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F560.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F608.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F6B0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F760.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F85C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029F9D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FC74.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Copies the ten halfwords just before dst (dst[-10..-1]) to dst[0..9] and
 * returns dst + 10.
 * Register convention: asm takes and returns the pointer in t1, restoring v0
 * (conventions.txt); its asm caller keeps a0, t0, t3, t8, f12, f14 live. */
s16 *func_8029FF2C(s16 *dst) {
    s32 i;

    for (i = 0; i < 10; i++) {
        dst[i] = dst[i - 10];
    }
    return dst + 10;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FF2C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FFA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0118.s")

/* func_802A0290 .. func_802A04BC: field setters/getter for entry `idx` of an
 * Unk8029DEA0Entry table (entry address = base + idx * 0x18, a signed 32-bit
 * multiply as in the asm). Register convention (conventions.txt): idx in v0,
 * the base in a0 or v1, the value in v1 and a float in f0. All of them restore
 * every register they use; asm callers keep various t-regs and f12/f14 live,
 * which the native port doesn't need a thunk for. */
#ifdef NON_MATCHING
#define ENTRY_AT(base, idx) ((Unk8029DEA0Entry *) ((u8 *) (base) + (idx) * 0x18))

/* unk10 = 1, unkE = val, unkC = 0 (base in a0, idx in v0, val in v1). */
void func_802A0290(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk10 = 1;
    e->unkE = val;
    e->unkC = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0290.s")
#endif

#ifdef NON_MATCHING
/* unk10 = 0 (idx in v0, base in v1). */
void func_802A02E4(s32 idx, Unk8029DEA0Entry *base) {
    ENTRY_AT(base, idx)->unk10 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A02E4.s")
#endif

#ifdef NON_MATCHING
/* unk13 = 0, unk4 = 0.0f (idx in v0, base in v1). */
void func_802A0320(s32 idx, Unk8029DEA0Entry *base) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk13 = 0;
    e->unk4 = 0.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0320.s")
#endif

#ifdef NON_MATCHING
/* unk13 = val, unk4 = f (f in f0, base in a0, idx in v0, val in v1). */
void func_802A0360(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk13 = val;
    e->unk4 = f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0360.s")
#endif

#ifdef NON_MATCHING
/* unk14 = val (base in a0, idx in v0, val in v1). */
void func_802A039C(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk14 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A039C.s")
#endif

#ifdef NON_MATCHING
/* unk11 = val (base in a0, idx in v0, val in v1). */
void func_802A03D4(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk11 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A03D4.s")
#endif

#ifdef NON_MATCHING
/* unk12 = val (base in a0, idx in v0, val in v1). */
void func_802A040C(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    ENTRY_AT(base, idx)->unk12 = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A040C.s")
#endif

#ifdef NON_MATCHING
/* unkC = 0, unkE = val (base in a0, idx in v0, val in v1). */
void func_802A0444(Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unkC = 0;
    e->unkE = val;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0444.s")
#endif

#ifdef NON_MATCHING
/* unk15 = val, unk8 = f (f in f0, base in a0, idx in v0, val in v1). */
void func_802A0480(f32 f, Unk8029DEA0Entry *base, s32 idx, s32 val) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    e->unk15 = val;
    e->unk8 = f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0480.s")
#endif

#ifdef NON_MATCHING
/* Reads entry idx (idx in v0, base in v1) into out[]: the asm returns
 * v1 = (s8) unk10, a0 = (s8) unk11, a1 = (s8) unk12, a2 = unk14,
 * a3 = (u16) unkC, t0 = (u16) unkE, t1 = (s8) unk13, f0 = unk4; here
 * out[0..6] get those integers in that order and out[7] the float's bits. */
void func_802A04BC(s32 idx, Unk8029DEA0Entry *base, s32 *out) {
    Unk8029DEA0Entry *e = ENTRY_AT(base, idx);

    out[0] = e->unk10;
    out[1] = e->unk11;
    out[2] = e->unk12;
    out[3] = e->unk14;
    out[4] = (u16) e->unkC;
    out[5] = (u16) e->unkE;
    out[6] = e->unk13;
    ((f32 *) out)[7] = e->unk4;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A04BC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0508.s")

/* func_802A0540: also part of the func_802A06B4 hidden-$v0-search-key
 * family documented below - saves the incoming $v0 across the call
 * (forwarding it one layer further) and returns that original value,
 * not the callee's. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0540.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0570.s")

/* func_802A05A4/func_802A05D0/func_802A05F8/func_802A0620/func_802A0648
 * (and likely more below): all call func_802A06B4 with no visible
 * arguments, then immediately use BOTH $v0 and $v1 from the return.
 * func_802A06B4 itself reads $v0 as an INPUT (compares it against a
 * table it walks via $v1, with no instruction anywhere setting $v0
 * first) and returns the matching entry's address in both $v0 and $v1 -
 * meaning these wrappers don't actually get two distinct values back,
 * they're each transparently forwarding a search key that some much
 * earlier caller stuffed into $v0, unchanged, through every layer in
 * between. Classic hand-tuned non-ABI register threading for a hot
 * dispatch-table lookup, not expressible as a normal C function call at
 * any layer - would need either inline asm or a hand-maintained
 * register-correct wrapper, not a straight decomp. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05A4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05D0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A05F8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0620.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0648.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0674.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Returns the D_803B35F8 entry whose id equals `id`. The scan has no bound
 * (the asm doesn't stop at the -1 terminator), so the id must be present.
 * Register convention: asm takes id in v0 and returns the entry in v0
 * (it restores v1 and a0); conventions.txt. */
Unk8029DEA0Entry *func_802A06B4(s32 id) {
    Unk8029DEA0Entry *e = D_803B35F8;

    while (e->id != id) {
        e++;
    }
    return e;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A06B4.s")
#endif
