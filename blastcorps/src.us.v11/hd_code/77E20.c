#include "common.h"
#include <ultra64.h>

extern void *D_80358074;

/* FILE-WIDE FINDING: every one of this file's 68 functions saves $ra (and
 * any other preserved registers) via the 64-bit `sd`/`ld` doubleword
 * instructions, never the normal 32-bit `sw`/`lw` pair IDO emits for every
 * o32 call frame (confirmed mechanically: grep for the first `*ra,` save in
 * each of the 68 .s files here - 68/68 are `sd`, 0/68 are `sw`). Per this
 * project's probe-confirmed research (see the hd_code "phantom dead frame"
 * notes in the project skill file), the `sd $ra`/`ld $ra` doubleword save
 * never comes out of this compiler for ANY real C, including genuine
 * function calls - so this signature, applied file-wide with no exceptions,
 * means the entire file is hand-written MIPS assembly, not reachable from C
 * at all. Spot-checked across four different internal shapes (a trivial
 * single-global-write stub, a ~30-instruction loop, a full all-registers
 * save/restore wrapper preserving even $sp/$fp/$k0/$k1 as data, and a
 * pointer-chain walk with a real call inside that still uses `sd $ra`) -
 * all confirm the same convention regardless of internal complexity. 33 of
 * the 68 additionally follow a "preserve caller registers across scratch
 * reuse" sub-convention (see the block comment above func_802BC840 below);
 * the rest don't need that extra layer to already be unreachable from C on
 * frame-shape grounds alone. Every pragma in this file is permanently
 * GLOBAL_ASM; don't attempt a C translation for any of them. */

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC5E0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC714.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC7DC.s")

/* Confirmed hand-written assembly: a "preserve caller registers via explicit
 * stack save/restore" convention, distinct from every other hand-written-asm
 * family documented elsewhere in this project. Each function in this family
 * (33 of this file's 68) opens by `sd`-saving some subset of registers
 * (always including $v0/$v1; the largest, func_802BC888, saves literally
 * every register in the file including $at/$gp/$sp/$fp/$k0/$k1 as raw data)
 * to its own stack frame, is then free to clobber those same register
 * numbers as ordinary scratch for its real body, and `ld`-restores every
 * saved slot byte-for-byte before returning - the restored value is always
 * exactly what was saved, never read or influenced by the body in between.
 * No compiler emits this: IDO never saves/restores a register it doesn't
 * believe is a live local, and no C semantics could want $sp itself
 * round-tripped through memory as inert data (seen in func_802BC888). This
 * is a deliberate low-level convention - likely a shared boilerplate/macro
 * the original engine used for a family of dispatch-table callback/handler
 * functions that must leave the caller's register state untouched regardless
 * of their own internal register needs. Permanently GLOBAL_ASM; don't
 * attempt a C translation for any function flagged with this comment.
 * (Port phase: functions of this family with an `#ifdef NON_MATCHING` C
 * rewrite below are functional, non-matching equivalents for the native port.) */
#ifdef NON_MATCHING
extern u8 D_803F7690[40][8];

/* Clears byte 7 of each of the 40 8-byte entries of D_803F7690.
 * Register note: the asm saves/restores v0 and v1; asm caller func_802A1674
 * keeps t0, f12 and f14 live across the call (a mixed N64 build would need a
 * thunk; the native port doesn't). */
void func_802BC840(void) {
    s32 i;

    for (i = 0; i < 40; i++) {
        D_803F7690[i][7] = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC840.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BC888.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCA2C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Two small byte buffers, each with a write pointer stored just before it:
 * buffer A is D_803F77D8 (write pointer D_803F77D4), buffer B is D_803F77E8
 * (write pointer D_803F77E4). Used by func_802BCBD8/802BCC10/802BCC48. */
extern u8 *D_803F77D4;
extern u8 D_803F77D8[];
extern u8 *D_803F77E4;
extern u8 D_803F77E8[];

/* Resets buffer A: its write pointer goes back to the buffer start.
 * Register note: the asm saves/restores v0 and v1 and touches nothing else. */
void func_802BCBD8(void) {
    D_803F77D4 = D_803F77D8;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCBD8.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Resets buffer B: its write pointer goes back to the buffer start.
 * Register note: the asm saves/restores v0 and v1; asm callers keep
 * registers live across the call (func_8029A800: a2, t8; func_802B49AC: a0,
 * a3, t6, f12, f14), so a mixed N64 build would need a thunk. */
void func_802BCC10(void) {
    D_803F77E4 = D_803F77E8;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC10.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern s32 D_80358060;

/* If D_80358060 is set, copies buffer A (D_803F77D8 up to its write pointer)
 * into buffer B byte by byte, front to back; then sets B's write pointer to
 * just past the copied bytes (to B's start when nothing was copied). The asm
 * loops with `!=`, so A's write pointer must not be below D_803F77D8.
 * Register note: the asm saves/restores v0, v1, a0 and a1; asm caller
 * func_802BE77C keeps a0, a1, t6, f12 and f14 live across the call. */
void func_802BCC48(void) {
    u8 *dst = D_803F77E8;

    if (D_80358060 != 0) {
        u8 *src = D_803F77D8;
        u8 *end = D_803F77D4;

        while (src != end) {
            *dst++ = *src++;
        }
    }
    D_803F77E4 = dst;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCC48.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCCD4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD20.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCD80.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCDE0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BCE40.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD064.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD10C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD1F8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD85C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD8C8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BD99C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BDDB4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE228.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE3C8.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE77C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE944.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern void *D_803F3960;
extern u8 D_803F3910[];

/* Resets the pointer D_803F3960 to the start of D_803F3910.
 * Register note: the asm saves/restores v0 and v1; asm caller func_802BE77C
 * keeps a0, a1, t6, t8, f12 and f14 live across the call. */
void func_802BE9F8(void) {
    D_803F3960 = D_803F3910;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BE9F8.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA30.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEA70.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEADC.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEBB0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEF9C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_80364456;   /* mode: the update is skipped in modes 0,1,2,6,7,9,0xB,0x10..0x12 */
extern s16 D_803F77FC;  /* signed input whose sign is tracked */
extern s32 D_803F77F4;  /* frame stamp: run counting only if it is D_80358068 - 1 */
extern s32 D_80358068;
extern u8 D_80370C2C;   /* flag sampled each counted frame */
extern s8 D_803F780A;   /* last sign of D_803F77FC (-1, 0, 1) */
extern u8 D_803F780B;   /* run length, 0..0x14 */
extern u8 D_803F780C;   /* result: set for one call when a run of 0x14 completes with a change */
extern u8 D_803F780D;   /* "the flag changed during this run" */
extern u8 D_803F780E;   /* flag value at the start of the run */
extern u8 D_803F780F;   /* once-per-frame latch, set here */

/* Once-per-frame run detector. Unless the mode excludes it or it already ran
 * (latch D_803F780F), it counts consecutive frames (D_803F77F4 + 1 ==
 * D_80358068) in which D_803F77FC's sign doesn't repeat a nonzero previous
 * sign, recording whether D_80370C2C's truth value changed since the run
 * started. After 0x14 such frames the count restarts and D_803F780C is set if
 * a change was seen. Any other frame resets the run. The asm's `addi` on
 * D_803F77F4 traps on 0x7FFFFFFF; the C wraps instead.
 * Register note: the asm saves/restores v0, v1, a0 and a1 (and clobbers at);
 * asm caller func_802BEBB0 keeps a0, a1, t3, t8, t9, f12 and f14 live. */
void func_802BEFF4(void) {
    u8 mode = D_80364456;
    s32 sign;
    s32 flag;
    s32 count;

    if (mode == 0 || mode == 1 || mode == 2 || mode == 6 || mode == 7 || mode == 9 || mode == 0xB ||
        mode == 0x10 || mode == 0x11 || mode == 0x12) {
        return;
    }
    if (D_803F77FC < 0) {
        sign = -1;
    } else if (D_803F77FC != 0) {
        sign = 1;
    } else {
        sign = 0;
    }
    if (D_803F780F != 0) {
        return;
    }
    D_803F780F = 1;

    if (D_803F77F4 + 1 == D_80358068 && (sign == 0 || D_803F780A != sign)) {
        count = D_803F780B;
        flag = (D_80370C2C != 0) ? 1 : 0;
        if (count == 0) {
            D_803F780E = flag;
            D_803F780D = 0;
        } else if (flag == 0 || flag != D_803F780E) {
            D_803F780D = 1;
        }
        count++; /* full width: 0xFF + 1 stores 0 but is compared as 0x100 */
        D_803F780B = count;
        D_803F780A = sign;
        D_803F780C = 0;
        if (count == 0x14) {
            D_803F780B = 0;
            if (D_803F780D != 0) {
                D_803F780C = 1;
            }
        }
    } else {
        D_803F780B = 0;
        D_803F780A = sign;
        D_803F780C = 0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BEFF4.s")
#endif

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF1F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF264.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF384.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF534.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF668.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF898.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BF978.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFB50.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFBF4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFD1C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFDAC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFEE4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802BFF6C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0284.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C038C.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u8 D_803F3968[30][0x38];

/* Sets byte 0x34 of each of the 30 0x38-byte entries of D_803F3968 to 1.
 * Register note: the asm saves/restores v0, v1 and a0; asm callers keep
 * registers live across the call (func_802A1674: a2, a3, t0, t1, t2, f12,
 * f14; func_802BA9A0: a0, a2, a3, t6, t7, f12, f14; func_802CF3E0: t1). */
void func_802C049C(void) {
    s32 i;

    for (i = 0; i < 30; i++) {
        D_803F3968[i][0x34] = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C049C.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C04F0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0574.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C08C4.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C09B8.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0C64.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0CBC.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C0E8C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1214.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C12E0.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1438.s")

/* Same preserve-caller-registers convention as func_802BC840 above - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C18D4.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1A28.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1AA0.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B1C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1B9C.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u8 type;   /* 0xFF = ignored, 1 = not counted */
    /* 0x05 */ u8 pad5[3];
    /* 0x08 */ s32 value; /* summed */
} Unk802C1DD0Info;

typedef struct {
    /* 0x00 */ Unk802C1DD0Info *info;
    /* 0x04 */ u8 pad4[0x2C];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ u8 pad34[0xB6];
    /* 0xEA */ u8 unkEA;
    /* 0xEB */ u8 unkEB;
    /* 0xEC */ u8 padEC[0x10];
} Unk802C1DD0Entry; /* size 0xFC */

extern Unk802C1DD0Entry D_803F4030[];
extern Unk802C1DD0Entry *D_803F7654; /* end of the used part of D_803F4030 */
extern s32 D_803F7684;
extern s32 D_803F7688;
extern u8 D_8036EB92;
extern struct {
    /* 0x00 */ s32 total;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 active;
} D_8036EA70;

/* Tallies the entries D_803F4030..D_803F7654 (0xFC bytes each). With
 * onlyFlagged set, only entries whose unkEB is nonzero count, and each of
 * those stores its unk30 into D_803F7684 (so the last one wins). Of the
 * counted entries whose info->type isn't 0xFF: those with type != 1 are
 * counted into D_8036EB92; if the entry's unkEA or D_803F7688 is nonzero,
 * type != 1 entries are also counted into D_8036EA70.active and every such
 * entry's info->value is summed into D_8036EA70.total. Clears D_803F7688.
 * The asm loops with `!=`, so D_803F7654 must be D_803F4030 + n entries.
 * The asm keeps its counters in s0-s2 but saves and restores s0-s7. */
void func_802C1DD0(s32 onlyFlagged) {
    Unk802C1DD0Entry *e;
    Unk802C1DD0Entry *end = D_803F7654;
    s32 counted = 0;
    s32 active = 0;
    s32 total = 0;

    for (e = D_803F4030; e != end; e++) {
        Unk802C1DD0Info *info = e->info;

        if (onlyFlagged != 0) {
            if (e->unkEB == 0) {
                continue;
            }
            D_803F7684 = e->unk30;
        }
        if (info->type == 0xFF) {
            continue;
        }
        if (info->type != 1) {
            counted++;
        }
        if ((e->unkEA | D_803F7688) == 0) {
            continue;
        }
        if (info->type != 1) {
            active++;
        }
        total += info->value;
    }
    D_8036EB92 = counted;
    D_8036EA70.active = active;
    D_8036EA70.total = total;
    D_803F7688 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1DD0.s")
#endif

/* TODO: func_802C1EE0 - walk a u16-offset chain rooted at
 * D_80358074 + D_80358074->unk74: `u8 *t1 = (u8*)D_80358074 + *(s32*)((u8*)D_80358074 + 0x74);
 * u16 *t2 = (u16*)(t1 + 4); for (arg0--; arg0 != 0; arg0--) t2 = (u16*)(t1 + *t2);
 * return (u8*)t2 + 2;` (target: 11 instructions, a pure leaf with zero calls).
 * The target saves/restores $ra via an 8-byte `sd $ra`/`ld $ra` frame despite
 * never calling anything - this is the already-documented dead-frame shape
 * that no C phrasing can produce even for a genuine call, so definitely not
 * for a callless leaf (see the hd_code "phantom dead frame" notes in the
 * project skill file). Every phrasing tried (plain locals, register-qualified
 * locals) either adds extra spills on top or drops the frame to nothing -
 * never this exact shape. Logic confirmed correct via direct diff read. */
#ifdef NON_MATCHING
/* Returns record `index` (1-based) of a chain of variable-length records.
 * The chain starts at level + *(s32 *)(level + 0x74), where level is
 * D_80358074. Each record is {u16 offsetOfNextRecord, s16 data[]}, with
 * offsets relative to the chain start, and the first record sits at chain + 4.
 * The result points at the record's data, just past its link. index 0 makes
 * the asm loop ~2^32 times, so callers must pass index >= 1. */
s16 *func_802C1EE0(s32 index) {
    u8 *chain = (u8 *) D_80358074 + *(s32 *) ((u8 *) D_80358074 + 0x74);
    u16 *record = (u16 *) (chain + 4);

    for (index--; index != 0; index--) {
        record = (u16 *) (chain + *record);
    }
    return (s16 *) (record + 1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1EE0.s")
#endif

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C1F30.s")

/* Uses the sd-$ra frame convention - see the file-level note at the top of this file. Permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
typedef struct {
    /* 0x00 */ s16 next;  /* -1 = last group */
    /* 0x02 */ u8 pad2[0x26];
    /* 0x28 */ s32 count; /* records following this header */
} Unk802C2054Group;       /* size 0x2C */

typedef struct {
    /* 0x00 */ u16 index[9];
    /* 0x12 */ u16 pad12;
    struct {
        /* 0x00 */ s32 unk0;
        /* 0x04 */ s32 base[3];
    } /* 0x14 */ part[3];
} Unk802C2054Record;      /* size 0x44 */

extern s32 *D_803F7828;

/* Walks the level's group list (at D_80358074 + *(s32 *)(D_80358074 + 0x74);
 * an initial zero word means "empty") and, for each record, writes nine words
 * (index[k] << 5) + part[k / 3].base[k % 3] to the output array D_803F7828,
 * 10 words (0x28 bytes) per record; word 9 of each output slot is left
 * untouched. Groups are stored back to back: a 0x2C-byte header, then
 * `count` 0x44-byte records, then the next header. The walk stops after a
 * group whose header's `next` is -1. The asm also computes chain + next but
 * never uses it (dead), so `next` acts only as an end marker here.
 * `count` is counted down with `!=` and must not be negative. */
void func_802C2054(void) {
    u8 *level = D_80358074;
    u8 *chain = level + *(s32 *) (level + 0x74);
    s32 *out = D_803F7828;
    u8 *p;
    s16 next;
    s32 n;
    s32 k;
    Unk802C2054Record *r;

    if (*(s32 *) chain == 0) {
        return;
    }
    p = chain + 4;
    do {
        next = ((Unk802C2054Group *) p)->next;
        n = ((Unk802C2054Group *) p)->count;
        r = (Unk802C2054Record *) (p + sizeof(Unk802C2054Group));
        while (n != 0) {
            n--;
            for (k = 0; k < 9; k++) {
                out[k] = (r->index[k] << 5) + r->part[k / 3].base[k % 3];
            }
            out += 10;
            r++;
        }
        p = (u8 *) r;
    } while (next != -1);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/77E20/func_802C2054.s")
#endif
