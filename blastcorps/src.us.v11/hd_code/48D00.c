#include "common.h"
#include <ultra64.h>

/* D_8039B070: array of D_8039B610 0x48-byte entries. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ u8 pad0C[0x0E - 0x0C];
    /* 0x0E */ u8 unk0E;
    /* 0x0F */ u8 pad0F[0x14 - 0x0F];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19[0x1E - 0x19];
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u8 pad20[0x23 - 0x20];
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 pad25;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} Entry48D00;
extern Entry48D00 D_8039B070_entries[];

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028D4C0.s")

/* TODO: func_8028DA5C - initialize an 8x 0x10-byte sub-entry struct at *a0
 * (likely per-corner/wheel contact data) from a 0x18-stride lookup table at
 * D_802FDB98 indexed by arg1, cycling through 3 of
 * {D_802FDB98,9A,9C,9E,A0,A2} (always the field at +0 and +4 relative to
 * the one skipped) plus a fixed {0, 0x3e0} constant pair per sub-entry.
 * Logic, every field offset, and the full group-by-group table-field
 * selection are all confirmed correct (diff score down to 20, zero inserts/
 * deletes, frame-free leaf matches exactly) - the only residue is 2 sites
 * (4 instructions total) where two back-to-back `li $tN, 0x3e0` loads of
 * the literally same redundant constant land in the opposite two temp
 * registers from target (t5<->t6, t7<->t8), confirmed via decomp-workbench
 * as a genuine instruction-bit difference (not a relocation/cosmetic
 * artifact) and flagged by its own field guide as "register-permutation,
 * owning pass unknown" - resolving it needs an instrumented IDO uopt trace
 * to tell UOPT-reservation from UGEN-demand, not a source-level lever; a
 * `lever 15`-style discarded-expression probe before the pair had zero
 * effect (fully eliminated, no FIFO rotation). Documented rather than
 * guessing further. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DA5C.s")

/* TODO: func_8028DD64 - for the D_8039B610 array entry at index arg0 (0x48
 * stride): forward its first 3 words into func_802CDA10, set its unk19 byte
 * to 5 and unk18 byte to 0, then set D_802E8BE4=10/D_802E8BE8=0x190 and, if
 * its unk40 word is nonzero, notify func_802608C8 and - if func_8028DE94()
 * (a no-arg linear search, see below) finds a match - forward it into
 * func_80260650(D_80367738, 0x73, match+0x40); finally, if unk44 is nonzero,
 * notify func_802608C8 of it too, then unconditionally call
 * func_80260650(D_80367738, 0x10, 0). Logic, every field offset, and the
 * overall control flow are all confirmed correct (diff score down to 300,
 * zero inserts/deletes - every single instruction present and in the right
 * place except a 4-instruction window). The one remaining gap: while
 * clearing the unk18 byte, target interleaves the tail of that address
 * calculation with the *start* of the next statement's (the unk40 lookup's)
 * address calculation one instruction later than every phrasing tried
 * produces - a pure instruction-scheduling-window artifact between two
 * independent, back-to-back statements, not a logic or layout gap. Tried:
 * several different statement orderings/interleavings of the two preceding
 * global stores (D_802E8BE4/D_802E8BE8) relative to the array writes, all
 * of which only made the score worse (710-1010) by disturbing other,
 * already-matching regions - this phrasing is the local optimum found. */
extern u8 D_8039B070;
extern u8 D_8039B088;
extern u8 D_8039B089;
extern u8 D_8039B0B0;
extern u8 D_8039B0B4;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern s32 D_80367738;

void func_802CDA10(s32, s32, s32);
void func_802608C8(s32);
void func_80260650(s32, s32, s32);
Entry48D00 *func_8028DE94(void);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DD64.s")

extern u8 D_8039B070;
extern s32 D_8039B610;

/* First entry with both unk18 and unk14 set, or NULL. */
Entry48D00 *func_8028DE94(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk14 != 0) {
            return &D_8039B070_entries[i];
        }
    }
    return NULL;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DF14.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028E9E4.s")

void func_802AACD4(u8, s32, s32, void *, void *);
extern u8 D_8039B094;

void func_8028F6B4(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk23 == arg0) {
            D_8039B070_entries[i].unk1E = 0;
            *(&D_8039B094 + i * 0x48) = 1;
            func_802AACD4(arg0, D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk8,
                          &D_8039B070_entries[i].unk26, &D_8039B070_entries[i].unk28);
        }
    }
}

extern s32 D_8039B610;
extern u8 D_8039B070;

s32 func_802AAE1C(u8, s16, s16, void *, void *);
s32 func_802CE6F8(s32, s32, s32);
void func_802CE4F0(s32, s32, s32);
s32 func_802CDB70(s16, s16);
void func_8028DD64(u8);

void func_8028F794(u8 arg0) {
    s32 i;
    s16 local;

    for (i = 0; i < D_8039B610; i++) {
        if (*(&D_8039B070 + i * 0x48 + 0x18) != 0 && *(&D_8039B070 + i * 0x48 + 0x24) != 0) {
            func_802AAE1C(
                arg0,
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x26),
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x28),
                (void *) (&D_8039B070 + i * 0x48),
                (void *) (&D_8039B070 + i * 0x48 + 8));

            *(s32 *) (&D_8039B070 + i * 0x48 + 4) = func_802CE6F8(
                *(s32 *) (&D_8039B070 + i * 0x48),
                *(s32 *) (&D_8039B070 + i * 0x48 + 8),
                *(s32 *) (&D_8039B070 + i * 0x48 + 4));

            if (*(s16 *) (&D_8039B070 + i * 0x48 + 0x1A) != 0) {
                local = 0;
            } else {
                local = *(s16 *) (&D_8039B070 + i * 0x48 + 0x1C);
            }

            func_802CE4F0(*(s32 *) (&D_8039B070 + i * 0x48),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 4),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 8));

            if (func_802CDB70(*(s16 *) (0x802FDBAC + *(&D_8039B070 + i * 0x48 + 0xE) * 0x18), local) != 0) {
                func_8028DD64((u8) i);
            }
        }
    }
}

extern u8 D_8039B094;

void func_8028F93C(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        *(&D_8039B094 + i * 0x48) = 0;
    }
}

extern u8 D_803A7424;
s32 func_8026A6F0(s32, s32, s32, s32, s32, s32);

/* Set D_803A7424 if the point (arg0, arg1, arg2) is within any active
 * entry's category radius (all in 1/32 units). */
void func_8028F994(s32 arg0, s32 arg1, s32 arg2) {
    s32 i;
    s32 dist;

    /* One statement (likely a macro in the original): separate
     * statements schedule the three reloads in the opposite order. */
    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5;
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
                                 D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
            if (dist <= (*(s16 *) (0x802FDBAC + D_8039B070_entries[i].unk0E * 0x18) >> 5)) {
                D_803A7424 = 1;
            }
        }
    }
}

/* TODO: func_8028FAC0 - same as func_8028F994, skipping entries with
 * unk24 set and widening each radius by arg3 (also 1/32 units). The
 * draft below is exact except one adjacent pair: target reloads arg3
 * (`lw t8,0x34(sp)`) one slot before the `sw v0` that spills dist; this
 * draft emits them the other way round. Tried: both operand orders of
 * the sum and the comparison, `!(dist > ...)`, a subtract-and-test
 * form, and the call inline in the condition (grows the frame by 8).
 *
 * void func_8028FAC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
 *     s32 i;
 *     s32 dist;
 *
 *     arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
 *     for (i = 0; i < D_8039B610; i++) {
 *         if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk24 == 0) {
 *             dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
 *                                  D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
 *             if (dist <= (*(s16 *) (0x802FDBAC + D_8039B070_entries[i].unk0E * 0x18) >> 5) + arg3) {
 *                 D_803A7424 = 1;
 *             }
 *         }
 *     }
 * }
 */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FAC0.s")

/* TODO: func_8028FC10 - set up D_80370BF8 via func_802DB4D0/func_802D4910,
 * read a u16 status word via func_802DB594 and set a local flag if bit
 * 0x1000 is set, call func_8028FCD4 and conditionally latch the flag into
 * D_802FDBD0, then derive D_802FDBD4 as "flag was set AND D_802FDBD0 was
 * zero" (target: 0x40-byte frame, no loops). Logic and call sequence
 * confirmed correct; declaring the final boolean `register` (to match
 * target's use of the callee-saved $s0 across the whole function, visible
 * directly in the target disassembly) dropped the score from 1706 to 740
 * and fixed the $s0 save/restore and register-class markers throughout.
 * Remaining gap: target's frame is 0x40 bytes but every phrasing tried
 * only needs 0x28-0x30 for the same three locals - something about the
 * original source uses roughly 24 more bytes of stack than this
 * reconstruction does (not a padding/alignment artifact; the three local
 * variables' own offsets shift to fill whatever space is allocated, so
 * this isn't simply "declare one more unused local" without knowing what
 * it should be). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028FC10.s")

typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Status48D00;

void func_802DB850(void *);
void func_802D4910(void *, s32, s32);
void func_802DB8D4(Status48D00 *);

/* Wait for arg0->unk8, then build a bitmask in *arg1 of the 4 status
 * entries with bit 0 of unk2 set and unk3 clear. */
u8 func_8028FCD4(void *arg0, u8 *arg1) {
    Status48D00 status[4];
    s32 i;

    *arg1 = 0;
    func_802DB850(arg0);
    while (*(s32 *) ((u8 *) arg0 + 8) == 0) {
    }
    func_802D4910(arg0, 0, 0);
    func_802DB8D4(status);
    for (i = 0; i < 4; i++) {
        if ((status[i].unk2 & 1) && status[i].unk3 == 0) {
            *arg1 |= 1 << i;
        }
    }
    return status[0].unk3;
}
