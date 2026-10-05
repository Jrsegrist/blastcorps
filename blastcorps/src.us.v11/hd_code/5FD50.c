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
#ifdef NON_MATCHING
/* Over an h x w block (starting at column x, row z) of a grid of s16 pairs
 * with `stride` pairs per row, returns the minimum of the first halves
 * (starting from 0x7FFF) and stores the maximum of the second halves
 * (starting from -0x8000) in *maxOut. The asm counts w and h down with
 * `!= 0` (both must be >= 1).
 * asm: x/z/w/h/stride/grid in s0/s1/s2/s3/s4/s6, min in v1, max in a0.
 * Asm callers rely on preserved: func_802A484C keeps a1-a3, t3, t8, f12, f14. */
s32 func_802A49A8(s32 x, s32 z, s32 w, s32 h, s32 stride, u8 *grid, s32 *maxOut) {
    u8 *row = grid + ((z * stride + x) << 2);
    u8 *p;
    s32 lo = 0x7FFF;
    s32 hi = -0x8000;
    s32 n;

    do {
        p = row;
        n = w;
        do {
            s32 a = *(s16 *) (p + 0);
            s32 b = *(s16 *) (p + 2);

            if (a < lo) {
                lo = a;
            }
            if (hi < b) {
                hi = b;
            }
            p += 4;
        } while (--n != 0);
        row += stride << 2;
    } while (--h != 0);
    *maxOut = hi;
    return lo;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A49A8.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Writes the positions of the 8 corners of a box into 8 Vtx (0x10 bytes
 * each) at v: x0 = x * xs, x1 = (x + w) * xs, z0 = z * zs, z1 = (z + d) * zs
 * (unsigned 32-bit products), y = y0 / y1; corners 0-3 at y0 and 4-7 at y1,
 * going (x0,z0) (x1,z0) (x1,z1) (x0,z1) and (x0,z0) (x1,z0) (x1,z1) (x0,z1).
 * Only the position halves are written.
 * asm: v in a2, x/z/w/d in s0/s1/s2/s3, xs/zs in s7/t8, y0/y1 in v1/a0; it
 * clobbers t6 and hi/lo.
 * Asm callers rely on preserved: func_802A484C keeps a1-a3, t3, f12, f14. */
void func_802A4A50(Vtx *v, u32 x, u32 z, u32 w, u32 d, u32 xs, u32 zs, s32 y0, s32 y1) {
    s32 x0 = x * xs;
    s32 x1 = (x + w) * xs;
    s32 z0 = z * zs;
    s32 z1 = (z + d) * zs;

    v[0].v.ob[0] = x0, v[0].v.ob[1] = y0, v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x1, v[1].v.ob[1] = y0, v[1].v.ob[2] = z0;
    v[2].v.ob[0] = x1, v[2].v.ob[1] = y0, v[2].v.ob[2] = z1;
    v[3].v.ob[0] = x0, v[3].v.ob[1] = y0, v[3].v.ob[2] = z1;
    v[4].v.ob[0] = x0, v[4].v.ob[1] = y1, v[4].v.ob[2] = z0;
    v[5].v.ob[0] = x1, v[5].v.ob[1] = y1, v[5].v.ob[2] = z0;
    v[6].v.ob[0] = x1, v[6].v.ob[1] = y1, v[6].v.ob[2] = z1;
    v[7].v.ob[0] = x0, v[7].v.ob[1] = y1, v[7].v.ob[2] = z1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4A50.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
extern u32 D_803BE740[];  /* scheduler task: header words, then an OSTask at +0x10 */
extern u8 D_802E6820[];   /* RSP boot ucode, ends at D_802E68F0 */
extern u8 D_802E68F0[];
extern u8 D_802E77B0[];   /* gfx ucode text */
extern u8 D_8030EE60[];   /* gfx ucode data */
extern u8 D_803BE780[];   /* dram stack (0x400) */
extern u8 D_803BEB80[];   /* output buffer */
extern u8 D_803C2B80[];   /* output buffer end */
extern u8 D_8036AFB0[];   /* yield buffer (0x900) */
extern u8 D_803153D8[];
extern u8 D_803156D8[];
extern OSMesgQueue D_80315440;
void func_80285110(s32);

/* Unless the level grid is 1 x 1 (D_803BE714 == D_803BE716 == 1, returns 1),
 * fills the task D_803BE740 (an M_GFXTASK OSTask at +0x10 with data_ptr /
 * data_size from the arguments), writes back `wb` (0x80 bytes) and the task,
 * invalidates the first 0x20 bytes of the output buffer, sends the task to
 * D_80315440, calls func_80285110(0x4D3), and returns 0 if the output
 * buffer's first word is 0xE8000000 (else 1).
 * asm: dataPtr/wb/dataSize in a1/a2/a3, result in t6; it saves a1-a3, t1, t3,
 * s4, s5, t8 and fp. Asm callers rely on preserved: func_802A484C keeps t3. */
s32 func_802A4B0C(void *dataPtr, void *wb, s32 dataSize) {
    u32 *t;

    if (D_803BE714 == 1 && D_803BE716 == 1) {
        return 1;
    }
    t = D_803BE740;
    t[4] = M_GFXTASK;
    t[5] = 0;
    t[6] = (u32) D_802E6820;
    t[8] = (u32) D_802E77B0;
    t[9] = 0x1000;
    t[10] = (u32) D_8030EE60;
    t[11] = 0x800;
    t[12] = (u32) D_803BE780;
    t[13] = 0x400;
    t[14] = (u32) D_803BEB80;
    t[15] = (u32) D_803C2B80;
    t[18] = (u32) D_8036AFB0;
    t[19] = 0x900;
    t[2] = 1;
    t[21] = (u32) D_803153D8;
    t[22] = 0x4D3;
    t[3] = 0;
    t[16] = (u32) dataPtr;
    t[7] = D_802E68F0 - D_802E6820;
    t[17] = dataSize;
    t[20] = (u32) D_803156D8;
    osWritebackDCache(wb, 0x80);
    osWritebackDCache(t, 0x40);
    osInvalDCache(D_803BEB80, 0x20);
    osSendMesg(&D_80315440, (OSMesg) t, OS_MESG_BLOCK);
    func_80285110(0x4D3);
    if (*(u32 *) D_803BEB80 == 0xE8000000) {
        return 0;
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A4B0C.s")
#endif

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
#ifdef NON_MATCHING
extern u32 D_803C3240; /* texture address for the current frame (0 = keep) */
extern u32 D_803C3244; /* texture address for the next frame (0 = keep) */
extern u8 D_803C324A;  /* fade flag */
extern u8 D_803C324B;  /* alpha */

/* Looks up the animation record (see AnimRecord below; frame words from +8)
 * with unk0 == id in the list [base + *(base+0x2C), base + *(base+0x30)).
 * Not found: returns 0. Found: D_803C3240 = frame word `frame` (0 for frame
 * 0), D_803C324A = fade; without fade returns frame != 0; with fade also sets
 * D_803C3244 = frame word frame + 1 (0 when that is nFrames) and D_803C324B
 * = alpha, and returns 1. The asm walks the list with `!=`.
 * asm: base in a0, id in t4, result in gp; it saves v0, v1, a1, a2.
 * Asm callers rely on preserved: func_802A4E4C keeps a0-a3, t4, t8, t9;
 * func_802A5334 keeps a0, a1, t0, t1, t3, t8, t9. */
s32 func_802A50DC(u8 *base, s32 id) {
    u8 *p = base + *(s32 *) (base + 0x2C);
    u8 *end = base + *(s32 *) (base + 0x30);
    u32 frame;

    for (; p != end; p += p[4] * 4 + 8) {
        if (*(s32 *) p == id) {
            frame = p[5];
            if (frame != 0) {
                D_803C3240 = *(u32 *) (p + 8 + frame * 4);
            } else {
                D_803C3240 = 0;
            }
            D_803C324A = p[6];
            if (p[6] == 0) {
                return frame != 0;
            }
            frame++;
            if (p[4] == frame) {
                D_803C3244 = 0;
            } else {
                D_803C3244 = *(u32 *) (p + 8 + frame * 4);
            }
            D_803C324B = p[7];
            return 1;
        }
    }
    return 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A50DC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#ifdef NON_MATCHING
/* Advances `p` past the next command with opcode `op` (no bound: it must
 * exist), leaving that command's first word in w0. */
#define SKIP_TO_CMD(p, op, w0)       \
    do {                             \
        (w0) = (p)[0];               \
        (p) += 2;                    \
    } while (((w0) >> 24) != (op))

/* Copies the display list [base + start, base + end) (8-byte commands) to dst
 * and returns dst advanced past the copy. If `patch` (func_802A50DC's
 * result): sets the address of the first G_SETTIMG (0xFD) to D_803C3240 if
 * nonzero; if D_803C324A, also the second G_SETTIMG's to D_803C3244 if
 * nonzero, and ORs D_803C324B into the first word of the next
 * G_SETPRIMCOLOR (0xFA). The copy loop uses `!=`.
 * asm: base/dst in a0/a1 (a1 returned advanced), start/end in t5/t6, patch
 * in gp (clobbered).
 * Asm callers rely on preserved: func_802A4E4C keeps a0, a2, a3, t4, t7-t9. */
u32 *func_802A51FC(u8 *base, u32 *dst, s32 start, s32 end, s32 patch) {
    u32 *s = (u32 *) (base + start);
    u32 *e = (u32 *) (base + end);
    u32 *d = dst;
    u32 *q;
    u32 w0;

    while (s != e) {
        d[0] = s[0];
        d[1] = s[1];
        s += 2;
        d += 2;
    }
    if (patch != 0) {
        q = dst;
        SKIP_TO_CMD(q, 0xFD, w0);
        if (D_803C3240 != 0) {
            q[-1] = D_803C3240;
        }
        if (D_803C324A != 0) {
            SKIP_TO_CMD(q, 0xFD, w0);
            if (D_803C3244 != 0) {
                q[-1] = D_803C3244;
            }
            SKIP_TO_CMD(q, 0xFA, w0);
            q[-2] = w0 | D_803C324B;
        }
    }
    return d;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A51FC.s")
#endif

/* Uses the sd-$ra frame convention (see hd_code/77E20.c's file-level note and the project skill file) - permanently GLOBAL_ASM. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/5FD50/func_802A5334.s")
