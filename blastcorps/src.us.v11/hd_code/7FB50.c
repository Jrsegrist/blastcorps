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
/* D_803F4030: array of 0xFC-byte records (55 slots); D_803F7654 points one
 * past the last record in use. Only the fields below are known. */
typedef struct {
    /* 0x00 */ u8 *unk0;    /* points at something whose byte +4 is a kind (1 = excluded) */
    /* 0x04 */ u8 pad4[0x2C];
    /* 0x30 */ s32 unk30;   /* 0x38 = excluded */
    /* 0x34 */ u8 pad34[0xB5];
    /* 0xE9 */ u8 numParts; /* number of per-part bytes at +0xEC */
    /* 0xEA */ u8 padEA;
    /* 0xEB */ u8 unkEB;    /* nonzero = already done */
    /* 0xEC */ u8 parts[0x10]; /* 100 = part destroyed */
} Rec7FB50; /* size 0xFC */

extern Rec7FB50 D_803F4030[];
extern Rec7FB50 *D_803F7654;
extern u16 D_8036EB90;
extern u8 D_803063F0[];

u8 func_8026FE6C(s32 arg0);

/* Bit writer used by the status packers below: MSB-first into whole bytes.
 * (Macros rather than functions: the originals are single asm leaves.) */
typedef struct {
    u8 *out;
    u32 bits;
    s32 count;
} BitWriter7FB50;

#define BITW_PUT(w, bit)                \
    {                                   \
        (w).bits = ((w).bits << 1) | (bit); \
        (w).count++;                    \
        if ((w).count == 8) {           \
            *(w).out++ = (w).bits;      \
            (w).bits = 0;               \
            (w).count = 0;              \
        }                               \
    }

/* Flush a partial byte, left-aligned. */
#define BITW_FLUSH(w)                                  \
    {                                                  \
        if ((w).count != 0) {                          \
            *(w).out++ = (w).bits << (8 - (w).count);  \
        }                                              \
    }
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* record_status: pack into `out`, one bit each, MSB-first, (1) for every part
 * of every record in use whether it is 100, then (2) func_8026FE6C(i) for
 * i = 0 .. D_8036EB90-1. Each section is padded to a whole byte. Returns the
 * number of bytes written. */
s32 func_802C4A40(u8 *out) {
    BitWriter7FB50 w;
    Rec7FB50 *r;
    u8 *part;
    s32 count;
    s32 i;
    s32 n;

    w.out = out;
    w.bits = 0;
    w.count = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        part = r->parts;
        for (n = r->numParts; n != 0; n--) {
            BITW_PUT(w,*part++ == 100);
        }
    }
    BITW_FLUSH(w);

    w.bits = 0;
    w.count = 0;
    count = D_8036EB90; /* read once, before any call */
    for (i = 0; i < count; i++) {
        BITW_PUT(w,func_8026FE6C(i));
    }
    BITW_FLUSH(w);

    return w.out - out;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4A40.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4BF0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* A record counts toward a random status unless it is excluded. */
#define REC7FB50_ELIGIBLE(r) ((r)->unk30 != 0x38 && (r)->unk0[4] != 1)

/* create_status: build a status bitstream in the format func_802C4A40 writes,
 * for difficulty/percentage level `level` (1-based index into D_803063F0, a
 * table of percentages).
 * Section 1: for every record in use, numParts copies of one bit. The bit is
 * 1 for eligible records that are already done (unkEB), plus enough further
 * eligible records, in order, to make D_803063F0[level-1] percent of the
 * eligible ones, plus one (none at all when level == 5).
 * Section 2: D_8036EB90 bits, the first D_803063F0[level-1] percent of them 1.
 * Each section is padded to a whole byte. Returns the number of bytes written. */
u32 func_802C4E58(void *outp, u8 level) {
    BitWriter7FB50 w;
    Rec7FB50 *r;
    u32 pct;
    u32 eligible;
    s32 done;
    s32 extra;
    u32 ones;
    s32 count;
    u32 bit;
    s32 n;

    pct = D_803063F0[level - 1];

    eligible = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        if (REC7FB50_ELIGIBLE(r)) {
            eligible++;
        }
    }
    /* Note: unlike `eligible`, this counts done records whether eligible or not. */
    done = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        if (r->unkEB != 0) {
            done++;
        }
    }
    extra = (pct * eligible) / 100 + 1;
    if (level - 1 == 4) {
        extra = 0;
    } else {
        extra -= done;
        if (extra < 0) {
            extra = 0;
        }
    }

    w.out = outp;
    w.bits = 0;
    w.count = 0;
    for (r = D_803F4030; r != D_803F7654; r++) {
        bit = 0;
        if (REC7FB50_ELIGIBLE(r)) {
            if (r->unkEB != 0) {
                bit = 1;
            } else if (extra != 0) {
                extra--;
                bit = 1;
            }
        }
        for (n = r->numParts; n != 0; n--) {
            BITW_PUT(w,bit);
        }
    }
    BITW_FLUSH(w);

    count = D_8036EB90;
    ones = (pct * D_8036EB90) / 100;
    w.bits = 0;
    w.count = 0;
    for (; count != 0; count--) {
        bit = 0;
        if (ones != 0) {
            bit = 1;
            ones--;
        }
        BITW_PUT(w,bit);
    }
    BITW_FLUSH(w);

    return w.out - (u8 *) outp;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C4E58.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5120.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5508.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5688.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5714.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5860.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5970.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C59B4.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5A14.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C5AFC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C617C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C61F0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6DAC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6ECC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C6FD8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C70E8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C71FC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7354.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7410.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7544.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F7C44; /* phase 0 (odd = waiting for input 0 to be released) */
extern u8 D_803F7C45; /* timer 0 */
extern u8 D_803F7C46; /* phase 1 */
extern u8 D_803F7C47; /* timer 1 */
extern u8 D_80370C1A; /* input 0 */
extern u8 D_80370C1B; /* input 1 */

/* One step of a press/release tracker: when the timer has run out the phase
 * is reset; otherwise the phase advances (and the timer restarts at 4) once
 * the input reaches the state the phase is waiting for (pressed on even
 * phases, released on odd ones), and the timer counts down while it hasn't.
 * (A macro, not a function: the original is a single asm leaf.) */
#define PRESS_TRACKER_STEP(phase, timer, input)                     \
    {                                                               \
        if ((timer) == 0) {                                         \
            (phase) = 0;                                            \
            (timer) = 4;                                            \
        } else if (((phase) & 1) ? ((input) == 0) : ((input) != 0)) { \
            (timer) = 4;                                            \
            (phase)++;                                              \
        } else {                                                    \
            (timer)--;                                              \
        }                                                           \
    }

/* Update the two press/release trackers (D_803F7C44/45 on D_80370C1A and
 * D_803F7C46/47 on D_80370C1B).
 * Register note: the asm touches only at/v0/v1/a0; its caller func_802C61F0
 * keeps a1-a3, t6, t7, f12 and f14 live across the call (a mixed N64 build
 * would need a thunk, the native port does not). */
void func_802C770C(void) {
    PRESS_TRACKER_STEP(D_803F7C44, D_803F7C45, D_80370C1A);
    PRESS_TRACKER_STEP(D_803F7C46, D_803F7C47, D_80370C1B);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C770C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7864.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7BC0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7C1C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7CB0.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7DFC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7ECC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C7F28.s")

/* func_802C8074: two-address trampoline into func_802AC7DC, same
 * confirmed-unreachable-from-C 8-byte sd-$ra frame as func_802AC284
 * (hd_code/679E0.c). Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C8074.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/7FB50/func_802C80A0.s")
