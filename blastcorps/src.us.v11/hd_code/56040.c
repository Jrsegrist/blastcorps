#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: this file's functions save $ra via the 64-bit `sd`/`ld`
 * doubleword form, not the normal 32-bit `sw`/`lw` pair - the same signature
 * documented in full in hd_code/77E20.c's top-of-file note and the project
 * skill file, confirming (with no exceptions found) that this file is part
 * of the same large hand-written-assembly module. Any pragma below without
 * a more specific comment follows this convention; a few have their own
 * more specific non-ABI explanation where one was already worked out. */
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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029B5B8.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D1D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D210.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D24C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029D534.s")

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
extern struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} D_803B7FC8[120];
extern void *D_803B8568; /* next-free pointer into D_803B7FC8 (sits right after it) */

/* Resets the 120-entry pool D_803B7FC8: the next-free pointer goes back to
 * the start and words 0 and 4 of every 12-byte entry are cleared.
 * Register note: the asm saves/restores v0 and v1 (clobbers at); asm caller
 * func_802A1674 keeps t0, f12 and f14 live across the call (a mixed N64
 * build would need a thunk; the native port doesn't). */
void func_8029DC80(void) {
    s32 i;

    D_803B8568 = D_803B7FC8;
    for (i = 0; i < 120; i++) {
        D_803B7FC8[i].unk0 = 0;
        D_803B7FC8[i].unk4 = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DC80.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DCD4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DD54.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DDC8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029DE50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s32 id;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 pad15[3];
} Unk8029DEA0Entry; /* size 0x18 */

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FF2C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_8029FFA0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0118.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0290.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A02E4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0320.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0360.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A039C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A03D4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A040C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0444.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A0480.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A04BC.s")

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
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/56040/func_802A06B4.s")
