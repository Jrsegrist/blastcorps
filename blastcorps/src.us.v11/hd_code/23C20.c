#include "common.h"
#include <ultra64.h>

f32 sqrtf(f32);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_802683E0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_80268664.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_802688C4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_80268D84.s")

typedef struct {
    s32 id;
    u8 unk4;
    u8 unk5;
    u8 pad6[14];
} Struct802F41E8;

extern Struct802F41E8 D_802F41E8[];
extern u8 D_803FCD75;

s32 func_80268EE8(s32 id) {
    s32 i;

    i = 0;
    do {
        if (D_802F41E8[i].id == id) {
            D_803FCD75 = D_802F41E8[i].unk5;
            return 1;
        }
        i++;
    } while (i < 3);
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_80268F54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_80269258.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_8026A184.s")

/* Wraps *angle into (ref - 180, ref + 180]. */
void func_8026A2E8(f32 ref, f32 *angle) {
    f32 d = *angle - ref;

    if (d > 180.0) {
        *angle -= 360.0;
    } else if (d < -180.0) {
        *angle += 360.0;
    }
}

/* Writes n as decimal digits (no leading zeros, at most 9 digits) plus a NUL. */
void func_8026A378(s32 n, u8 *buf) {
    s32 div;
    u8 digit;
    u8 printed;
    u8 started;

    div = 100000000;
    printed = 0;
    started = 0;
    do {
        digit = n / div;
        if (started || digit) {
            *buf++ = digit + '0';
            started = printed = 1;
        }
        n -= digit * div;
        div /= 10;
    } while (div != 0);
    if (!printed) {
        *buf++ = '0';
    }
    *buf = 0;
}

/* Rotation about the point (x, y, z): z axis by rx, then y axis by ry (units of 1/11.375 degree). */
void func_8026A454(s16 x, s16 y, s16 z, s16 rx, s16 ry, Mtx *m) {
    f32 a[4][4];
    f32 b[4][4];

    guTranslateF(a, -x, -y, -z);
    guRotateF(b, (f32) rx / 11.375, 0.0f, 0.0f, 1.0f);
    guMtxCatF(a, b, a);
    guRotateF(b, (f32) ry / 11.375, 0.0f, 1.0f, 0.0f);
    guMtxCatF(a, b, a);
    guTranslateF(b, x, y, z);
    guMtxCatF(a, b, a);
    guMtxF2L(a, m);
}

/* Copies size bytes in 8-byte units. */
void func_8026A5CC(u64 *dst, u64 *src, s32 size) {
    s32 unused;

    size >>= 3;
    while (size--) {
        *dst++ = *src++;
    }
}

/* 2D distance, squared in 64 bits. */
s32 func_8026A610(s32 x1, s32 y1, s32 x2, s32 y2) {
    s64 dx;
    s64 dy;

    dx = x1 - x2, dy = y1 - y2;
    dx = dx * dx;
    dy = dy * dy;
    return sqrtf(dx + dy);
}

/* 3D distance, squared in 64 bits. */
s32 func_8026A6F0(s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2) {
    s64 dx;
    s64 dy;
    s64 dz;

    dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;
    return sqrtf(dx + dy + dz);
}

extern s32 D_8036B968;
extern s32 D_8036B96C;

/* LCG random integer in [lo, hi], rounded. */
s32 func_8026A828(s32 lo, s32 hi) {
    f32 f;

    D_8036B968 = D_8036B968 * 0x41C64E6D + 0x3039;
    f = (D_8036B968 & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

void func_8026A8BC(void) {
    D_8036B968 = osGetCount();
}

/* Same with the second seed. */
s32 func_8026A8E0(s32 lo, s32 hi) {
    f32 f;

    D_8036B96C = D_8036B96C * 0x41C64E6D + 0x3039;
    f = (D_8036B96C & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

void func_8026A974(void) {
    D_8036B96C = 0x9BA0D;
}

extern u8 D_8036B970;
extern u8 D_8036B971;
extern s32 D_8036B974;
extern u8 D_8036B978;
extern u8 D_8036B979;

void func_8026A988(void) {
    D_8036B970 = 0;
    D_8036B971 = 0;
    D_8036B974 = 0;
    D_8036B978 = 0;
    D_8036B979 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_8026A9B4.s")
