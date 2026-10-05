#include "common.h"
#include <ultra64.h>

/* FILE-WIDE FINDING: hand-written assembly (sd/ld $ra frames, results
 * returned in $s8 or $f0), split out of the old 68810 bin blob. Like the
 * 56040..8DDB0 block, these stay GLOBAL_ASM permanently in the matching build.
 *
 * ROM 0x690C0-0x69944 holds the arcsine tables D_802AD880 / D_802AE084 (see
 * 69000.c). Code units must start on a 16-byte boundary, so this file begins
 * 0x14 bytes early, at 0x802AE0F0, and carries the last five words of
 * D_802AE084 as data below. */

/* Last 5 words of the u16 table D_802AE084 (data, not code). */
GLOBAL_ASM(
glabel D_802AE0F0
.word 0x3F513F5B, 0x3F663F71, 0x3F7E3F8B, 0x3F9B3FAD, 0x3FC53FFF
)

#ifdef NON_MATCHING
extern f32 D_80305C70; /* 2*pi */
extern f32 D_80305C74; /* pi */
extern f32 D_80305C78; /* pi/2 */
extern f32 D_80305C7C; /* 1/2! */
extern f32 D_80305C80; /* 1/3! */
extern f32 D_80305C84; /* 1/4! */
extern f32 D_80305C88; /* 1/5! */
extern f32 D_80305C8C; /* 1/6! */
extern f32 D_80305C90; /* 1/7! */
extern f32 D_80305C94; /* 1/8! */
extern f32 D_80305C9C; /* 65536.0 */
extern f32 D_80305CA0; /* 2*pi / 4096 */

f32 func_802AE1BC(f32 x);
f32 func_802AE290(f32 x);

/* cvt.w.s: float -> s32 rounding to nearest, ties to even (the FCSR mode the
 * game runs with), not truncation like a C cast. */
static s32 port_cvt_w_s(f32 x) {
    s32 t = (s32) x;
    f32 frac = x - (f32) t;

    if (frac > 0.5f || (frac == 0.5f && (t & 1))) {
        t++;
    } else if (frac < -0.5f || (frac == -0.5f && (t & 1))) {
        t--;
    }
    return t;
}
#endif

/* Fixed-point cosine: in v1 = angle (low 12 bits, 0x1000 = 360 degrees),
 * out s8 = (s32)(cos * scale) via func_802AE1BC. Preserves v1/a0. */
#ifdef NON_MATCHING
/* Convention: in v1=a0 ; out fp=ret. Asm callers rely on it preserving v1/a0
 * (it saves them) and t0/t3/t4/t5/t7, a1-a3 (untouched); a mixed build needs a
 * thunk. The asm also leaves f12 = 65536.0 and the core's f8/f14 temporaries;
 * the survey lists those as read by callers, but none of the callers reads
 * them before writing them (checked by hand), so they are not modelled. */
s32 func_802AE104(s32 angle) {
    return port_cvt_w_s(func_802AE1BC((f32) (angle & 0xFFF) * D_80305CA0) * D_80305C9C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE104.s")
#endif

/* Fixed-point sine: as func_802AE104 but via func_802AE290. */
#ifdef NON_MATCHING
/* Convention: in v1=a0 ; out fp=ret. Same preserved registers and leftover
 * f12/f14 as func_802AE104. */
s32 func_802AE160(s32 angle) {
    return port_cvt_w_s(func_802AE290((f32) (angle & 0xFFF) * D_80305CA0) * D_80305C9C);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE160.s")
#endif

/* Float cosine core: f12 = radians, returns f0 (range reduction + even
 * polynomial). Clobbers a0, f2-f16. */
#ifdef NON_MATCHING
/* Plain o32 (f12 in, f0 out). The asm's only caller, func_802AE104, reads
 * nothing but f0. An infinite input loops forever in the asm too. */
f32 func_802AE1BC(f32 x) {
    union {
        f32 f;
        u32 u;
    } bits;
    f32 x2, x4, x6, x8, r;
    s32 neg;

    bits.f = x;
    bits.u &= 0x7FFFFFFF; /* abs.s, also for -0.0 / NaN */
    x = bits.f;
    while (D_80305C70 < x) {
        x -= D_80305C70;
    }
    if (D_80305C74 < x) {
        x = D_80305C70 - x;
    }
    neg = 0;
    if (D_80305C78 < x) {
        x = D_80305C74 - x;
        neg = 1;
    }
    x2 = x * x;
    x4 = x2 * x2;
    x6 = x4 * x2;
    x8 = x4 * x4;
    r = 1.0f - x2 * D_80305C7C;
    r = r + x4 * D_80305C84;
    r = r - x6 * D_80305C8C;
    r = r + x8 * D_80305C94;
    if (neg) {
        r = -r;
    }
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE1BC.s")
#endif

/* Float sine core: f12 = radians, returns f0 (odd polynomial). */
#ifdef NON_MATCHING
/* Plain o32 (f12 in, f0 out); its only caller func_802AE160 reads only f0. */
f32 func_802AE290(f32 x) {
    f32 x2, x4, x6, r;
    s32 neg = 0;

    if (x < 0.0f) {
        x = -x; /* abs.s, only for x < 0 */
        neg = 1;
    }
    while (D_80305C70 < x) {
        x -= D_80305C70;
    }
    if (D_80305C74 < x) {
        x = D_80305C70 - x;
        neg ^= 1;
    }
    if (D_80305C78 < x) {
        x = D_80305C74 - x;
    }
    x2 = x * x;
    x4 = x2 * x2;
    x6 = x4 * x2;
    r = 1.0f - x2 * D_80305C80;
    r = r + x4 * D_80305C88;
    r = r - x6 * D_80305C90;
    r = r * x;
    if (neg) {
        r = -r;
    }
    return r;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/69930/func_802AE290.s")
#endif
