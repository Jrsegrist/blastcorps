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
extern s16 D_803C3248;
extern u8 D_803C2B90[];
extern u8 *D_803C2B88;
extern u16 D_803BE714;
extern u16 D_803BE716;
extern s16 D_803C30A8[];  /* 100 entries, then D_803C3170 */
extern s16 *D_803C3170;
extern s16 D_803C3178[];

/* Body shared by func_802A4510 / func_802A45D4: D_803C3248 = value, reset
 * the D_803C2B88 cursor, fill both index lists with 0..n-1 for n =
 * D_803BE714 * D_803BE716, point D_803C3170 past the D_803C3178 list and
 * terminate the D_803C30A8 list with -1. The asm loops with `!=` on the
 * 32-bit product. The -1 is stored after D_803C3170, so with n == 100 it
 * overwrites D_803C3170's high half, as in the asm. */
#define RESET_INDEX_LISTS(value)                \
    {                                           \
        u32 n_;                                 \
        u32 i_;                                 \
                                                \
        D_803C3248 = (value);                   \
        D_803C2B88 = D_803C2B90;                \
        n_ = (u32) D_803BE714 * D_803BE716;     \
        for (i_ = 0; i_ != n_; i_++) {          \
            D_803C3178[i_] = i_;                \
            D_803C30A8[i_] = i_;                \
        }                                       \
        D_803C3170 = &D_803C3178[n_];           \
        D_803C30A8[n_] = -1;                    \
    }
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* func_802A45D4(0).
 * Asm callers rely on preserved: func_802A1674 keeps t0, f12, f14 (the asm
 * saves t0-t3). */
void func_802A4510(void) {
    RESET_INDEX_LISTS(0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4510.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Sets D_803C3248 and resets the index lists (see RESET_INDEX_LISTS). */
void func_802A45D4(s32 arg0) {
    RESET_INDEX_LISTS(arg0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A45D4.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_802A470C(s32, Gfx *, Vtx *, s32);

/* Skips D_803C3248 frames: while the counter is nonzero it just counts
 * down; at zero it calls func_802A470C with the same four arguments.
 * Register note: func_802A470C clobbers s0-s7 (the asm here saves s0-s7,
 * gp and fp around it), and the asm passes f12/f14 through untouched. A
 * mixed N64 build would need a thunk saving the s-registers around the
 * asm callee; the native port (callee in C) doesn't. */
void func_802A467C(s32 arg0, Gfx *arg1, Vtx *arg2, s32 arg3) {
    if (D_803C3248 == 0) {
        func_802A470C(arg0, arg1, arg2, arg3);
    } else {
        D_803C3248--;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A467C.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A470C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A484C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A49A8.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4A50.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4B0C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4CDC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Writes a gSPDisplayList (0x06000000, physical address) into `gfx` for each
 * display list packed back to back in [dl, end), stepping over each one to
 * just past its gSPEndDisplayList (0xB8000000) word, then a closing
 * gSPEndDisplayList. `end` must sit exactly after one of the lists (the asm
 * compares with `!=`). The asm also leaves gfx advanced in a1, which its only
 * caller does not read. arg0 is unused.
 * Asm callers rely on preserved: func_802A4CDC keeps a0. */
void func_802A4DE8(s32 arg0, u32 *gfx, u32 *dl, u32 *end) {
    u32 w0;

    while (dl != end) {
        gfx[0] = 0x06000000;
        gfx[1] = (u32) dl - 0x80000000;
        gfx += 2;
        do {
            w0 = dl[0];
            dl += 2;
        } while (w0 != 0xB8000000);
    }
    gfx[0] = 0xB8000000;
    gfx[1] = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4DE8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4E4C.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* A variable-size animation record: an 8-byte header plus nFrames words.
 * (period and timer at 0x8/0xA overlap the first frame word.) */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ u8 nFrames;
    /* 0x05 */ u8 frame;
    /* 0x06 */ u8 fade;     /* nonzero: alpha follows the timer */
    /* 0x07 */ u8 alpha;
    /* 0x08 */ u16 period;
    /* 0x0A */ u16 timer;
} AnimRecord;

/* Steps each animation record in the list whose start/end offsets (from
 * `base`) are the words at base+0x2C / base+0x30: timer counts up to
 * period, then wraps to 0 and advances the frame (wrapping at nFrames);
 * a fading record gets alpha = 0xFF * timer / period. The asm loops with
 * `!=`, so the end must be exactly reached. */
void func_802A5020(u8 *base) {
    u8 *p = base + *(s32 *) (base + 0x2C);
    u8 *end = base + *(s32 *) (base + 0x30);

    while (p != end) {
        AnimRecord *r = (AnimRecord *) p;
        u32 t = r->timer + 1;

        if (r->period == t) {
            u32 f = r->frame + 1;

            if (r->nFrames == f) {
                f = 0;
            }
            r->frame = f;
            t = 0;
        }
        if (r->fade != 0) {
            r->alpha = (0xFF * t) / r->period; /* divu; traps on period 0 */
        }
        r->timer = t;
        p += r->nFrames * 4 + 8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A5020.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A50DC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A51FC.s")

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A5334.s")
