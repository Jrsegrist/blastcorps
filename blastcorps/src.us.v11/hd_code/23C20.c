#include "common.h"
#include <ultra64.h>

/* utils2.c (from its assert strings): camera spline paths, triggers, small maths helpers. */

void func_8029A7E4(const char *, ...);
f32 sqrtf(f32);
s32 func_802753C0(void);
s32 func_802AC4C4(s32, s32, s32, s32, s32, s32, s32, s32);
s32 func_802AB3C0(s32);
s32 func_8026AD30(s32);
f32 func_80268D84(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c);
s32 func_8026A610(s32 x1, s32 y1, s32 x2, s32 y2);
s32 func_8026A6F0(s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2);

#define ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "utils2.c", line)

/* 20-byte trigger zones, tested against the player position */
typedef struct {
    u8 id;
    u8 unk1;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Struct802F3C10;

/* spline tension per path */
typedef struct {
    s32 id;
    f32 val;
    u8 unk8;
    u8 pad9[3];
} Struct802F3C24;

/* spline control points */
typedef struct {
    s32 id;
    u8 unk4;
    u8 pad5;
    s16 x;
    s16 y;
    s16 z;
    u8 speed;
    u8 padD[3];
} Struct802F3C48;

typedef struct {
    s32 id;
    u8 unk4;
    u8 unk5;
    u8 pad6[2];
    f32 unk8;
    s16 unkC;
    u8 padE[6];
} Struct802F41E8;

/* vehicles, 0x74 bytes each */
typedef struct {
    u8 pad0[0x5C];
    s32 unk5C;
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
} Struct80364460;

/* players, 0x100 bytes each */
typedef struct {
    u8 pad0[0x18];
    u8 unk18[0xE8];
} Struct80364AF0;

extern s32 D_802E8BDC;
extern Struct802F3C10 D_802F3C10[];
extern Struct802F3C24 D_802F3C24[];
extern Struct802F3C48 D_802F3C48[];
extern Struct802F41E8 D_802F41E8[];
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern u8 D_803643D6;
extern u8 D_803643D7;
extern u8 D_80364456;
extern Struct80364460 D_80364460[];
extern Struct80364460 *D_803649D0;
extern u8 D_803649ED;
extern u64 D_80364A98;
extern u8 D_80364AE8;
extern Struct80364AF0 D_80364AF0[];
extern u8 D_8036B8B0;
extern s32 D_8036B8B4;
extern s32 D_8036B8B8;
extern s32 D_8036B8BC;
extern u8 D_8036B8C0;
extern f32 D_8036B8C8[4][4];
extern s32 D_8036B908;
extern u8 D_8036B90C;
extern f32 D_8036B910[4][4];
extern s32 D_8036B950;
extern u8 D_8036B954;
extern u8 D_8036B955;
extern u8 D_8036B958[4];
extern u8 D_8036B95C;
extern u8 D_8036B964;
extern u8 D_8036B965;
extern u8 D_8036B966;
extern s32 D_8036B968;
extern s32 D_8036B96C;
extern u8 D_8036B970;
extern u8 D_8036B971;
extern s32 D_8036B974;
extern u8 D_8036B978;
extern u8 D_8036B979;
extern u8 D_8036C7CC;
extern u8 D_8036EB98;
extern u8 D_803A7430;
extern u8 D_803ED826;
extern u8 D_803EFECB;
extern s32 D_803FCD60;
extern u8 D_803FCD75;

/* Checks the trigger zones for the current level; sets D_803649ED on a hit. */
void func_802683E0(void) {
    s32 i;
    u8 found;
    u8 hit;
    Struct80364460 *p;

    i = 0;
    found = 0;
    if (D_80364456 || D_803ED826) {
        return;
    }
    while (!found && i < 1) {
        if (D_802F3C10[i].id == D_802E8BDC) {
            p = D_80364460;
            hit = 0;
            while (!hit && D_803649D0 != p) {
                if (D_802F3C10[i].unk1 == p->unk5C) {
                    hit = 1;
                } else {
                    p++;
                }
            }
            if (hit) {
                if (func_802AC4C4(p->unk64 >> 5, p->unk6C >> 5, D_802F3C10[i].unk2, D_802F3C10[i].unk4,
                                  D_802F3C10[i].unk6, D_802F3C10[i].unk8, D_802F3C10[i].unkA, D_802F3C10[i].unkC)
                    || func_802AC4C4(p->unk64 >> 5, p->unk6C >> 5, D_802F3C10[i].unk2, D_802F3C10[i].unk4,
                                     D_802F3C10[i].unkA, D_802F3C10[i].unkC, D_802F3C10[i].unkE, D_802F3C10[i].unk10)) {
                    if (func_8026A6F0(p->unk64, p->unk68, p->unk6C, D_803643E0, D_803643E4, D_803643E8) < D_802F3C10[i].unk12) {
                        D_803649ED = D_802F3C10[i].unk1;
                        found = 1;
                    }
                }
            }
        }
        i++;
    }
}

/* Selects spline path id and builds its cardinal-spline basis matrix in D_8036B8C8. */
void func_80268664(s32 id) {
    f32 s;

    D_8036B8B0 = 0;
    D_8036B8C0 = 0;
    do {
        if (D_802F3C24[D_8036B8C0].id == id) {
            D_8036B8B0 = 1;
        } else {
            D_8036B8C0++;
        }
    } while (D_8036B8C0 < 3 && !D_8036B8B0);
    if (D_8036B8B0) {
        s = D_802F3C24[D_8036B8C0].val;
        D_8036B8C8[0][0] = -s;
        D_8036B8C8[0][1] = s * 2.0;
        D_8036B8C8[0][2] = -s;
        D_8036B8C8[0][3] = 0.0f;
        D_8036B8C8[1][0] = 2.0 - s;
        D_8036B8C8[1][1] = s - 3.0;
        D_8036B8C8[1][2] = 0.0f;
        D_8036B8C8[1][3] = 1.0f;
        D_8036B8C8[2][0] = s - 2.0;
        D_8036B8C8[2][1] = 3.0 - s * 2.0;
        D_8036B8C8[2][2] = s;
        D_8036B8C8[2][3] = 0.0f;
        D_8036B8C8[3][0] = s;
        D_8036B8C8[3][1] = -s;
        D_8036B8C8[3][2] = 0.0f;
        D_8036B8C8[3][3] = 0.0f;
        D_8036B908 = 0;
        D_8036B90C = 1;
    }
}

/* Advances along the path: evaluates the spline at the four current control points. */
void func_802688C4(s32 path) {
    s32 j;
    s32 i;
    u8 pts[4];
    u8 want[4];
    f32 t;
    f32 t2;
    f32 t3;
    s32 pad[4];
    u8 found;

    if (D_8036B8B0) {
        want[0] = D_8036B90C - 1;
        want[1] = D_8036B90C;
        want[2] = D_8036B90C + 1;
        want[3] = D_8036B90C + 2;
        for (j = 0; j < 4; j++) {
            i = 0;
            found = 0;
            while (i < 90 && !found) {
                if (D_802F3C48[i].id == path && D_802F3C48[i].unk4 == want[j]) {
                    found = 1;
                } else {
                    i++;
                }
            }
            ASSERT(found, 181);
            pts[j] = i;
        }
        t = (f32) D_8036B908 / 1000.0;
        t2 = t * t;
        t3 = t2 * t;
        D_8036B8B4 = func_80268D84(D_802F3C48[pts[0]].x << 5, D_802F3C48[pts[1]].x << 5, D_802F3C48[pts[2]].x << 5,
                                   D_802F3C48[pts[3]].x << 5, t, t2, t3);
        D_8036B8B8 = func_80268D84(D_802F3C48[pts[0]].y << 5, D_802F3C48[pts[1]].y << 5, D_802F3C48[pts[2]].y << 5,
                                   D_802F3C48[pts[3]].y << 5, t, t2, t3);
        D_8036B8BC = func_80268D84(D_802F3C48[pts[0]].z << 5, D_802F3C48[pts[1]].z << 5, D_802F3C48[pts[2]].z << 5,
                                   D_802F3C48[pts[3]].z << 5, t, t2, t3);
        if (!D_803643D7 && !D_803643D6 && !func_802753C0()) {
            D_8036B908 += D_802F3C24[D_8036B8C0].unk8 *
                          ((D_802F3C48[pts[2]].speed - D_802F3C48[pts[1]].speed) * t + D_802F3C48[pts[1]].speed);
        }
        if (D_8036B908 >= 1000) {
            D_8036B908 = 0;
            D_8036B90C++;
        }
    }
}

/* Evaluates one spline coordinate with basis D_8036B8C8. */
f32 func_80268D84(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = D_8036B8C8[0][0] * x + D_8036B8C8[1][0] * y + D_8036B8C8[2][0] * z + D_8036B8C8[3][0] * w;
    i1 = D_8036B8C8[0][1] * x + D_8036B8C8[1][1] * y + D_8036B8C8[2][1] * z + D_8036B8C8[3][1] * w;
    i2 = D_8036B8C8[0][2] * x + D_8036B8C8[1][2] * y + D_8036B8C8[2][2] * z + D_8036B8C8[3][2] * w;
    i3 = D_8036B8C8[0][3] * x + D_8036B8C8[1][3] * y + D_8036B8C8[2][3] * z + D_8036B8C8[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}


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

/* Picks the second path set (D_802F41E8) for this level and builds basis D_8036B910. */
void func_80268F54(void) {
    u8 found;
    f32 s;

    found = 0;
    if (D_80364A98 == 0x800) {
        D_8036B966 = 0;
    } else {
        D_8036B966 = 1;
    }
    D_8036B955 = 0;
    while (!found) {
        if (D_802F41E8[D_8036B955].id == D_802E8BDC && D_802F41E8[D_8036B955].unk4 == D_8036B966) {
            found = 1;
        } else {
            D_8036B955++;
        }
    }
    s = D_802F41E8[D_8036B955].unk8;
    D_8036B910[0][0] = -s;
    D_8036B910[0][1] = s * 2.0;
    D_8036B910[0][2] = -s;
    D_8036B910[0][3] = 0.0f;
    D_8036B910[1][0] = 2.0 - s;
    D_8036B910[1][1] = s - 3.0;
    D_8036B910[1][2] = 0.0f;
    D_8036B910[1][3] = 1.0f;
    D_8036B910[2][0] = s - 2.0;
    D_8036B910[2][1] = 3.0 - s * 2.0;
    D_8036B910[2][2] = s;
    D_8036B910[2][3] = 0.0f;
    D_8036B910[3][0] = s;
    D_8036B910[3][1] = -s;
    D_8036B910[3][2] = 0.0f;
    D_8036B910[3][3] = 0.0f;
    D_803FCD60 = D_802F41E8[D_8036B955].unkC << 5;
    D_8036B950 = 0;
    D_8036B954 = 1;
    D_8036B958[0] = 0;
    D_8036B958[1] = 0;
    D_8036B958[2] = 0;
    D_8036B958[3] = 0;
    D_8036B95C = 0;
    D_8036B964 = 0;
    D_8036B965 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/23C20/func_80269258.s")

/* Same as func_80268D84 with basis D_8036B910. */
f32 func_8026A184(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = D_8036B910[0][0] * x + D_8036B910[1][0] * y + D_8036B910[2][0] * z + D_8036B910[3][0] * w;
    i1 = D_8036B910[0][1] * x + D_8036B910[1][1] * y + D_8036B910[2][1] * z + D_8036B910[3][1] * w;
    i2 = D_8036B910[0][2] * x + D_8036B910[1][2] * y + D_8036B910[2][2] * z + D_8036B910[3][2] * w;
    i3 = D_8036B910[0][3] * x + D_8036B910[1][3] * y + D_8036B910[2][3] * z + D_8036B910[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}

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

void func_8026A988(void) {
    D_8036B970 = 0;
    D_8036B971 = 0;
    D_8036B974 = 0;
    D_8036B978 = 0;
    D_8036B979 = 0;
}

/* Per-frame event checks: proximity to vehicles, level-specific triggers (func_8026AD30). */
void func_8026A9B4(void) {
    Struct80364460 *p;
    u8 done;
    s32 minDist;
    s32 d;

    minDist = 999999;
    if (D_803A7430 == 15) {
        func_8026AD30(0x50);
    }
    if (D_802E8BDC == 0x12 && (D_803643E0 >> 5) < 0x578
        && func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, 0x531, 0x1061) < 0x82) {
        func_8026AD30(0x51);
    }
    if (D_80364456 != D_8036B979) {
        D_8036B978 = D_8036B979;
    }
    if (D_80364456) {
        D_8036B979 = D_80364456;
    }
    p = D_80364460;
    done = 0;
    while (!done && D_803649D0 != p) {
        if (p->unk5C != D_8036B978 && p->unk5C != D_80364456 && D_80364456 != 0 && p->unk5C != 0
            && p->unk5C != 0xFE && p->unk5C != 0xFF && p->unk5C != 7 && p->unk5C != 6) {
            d = func_8026A6F0(D_803643E0 >> 5, D_803643E4 >> 5, D_803643E8 >> 5, p->unk64 >> 5, p->unk68 >> 5,
                              p->unk6C >> 5);
            if (d < minDist) {
                minDist = d;
            }
            if (d < 100 && !D_8036B970 && (!func_802AB3C0(p->unk5C) || !D_803EFECB)) {
                func_8026AD30(0x4C);
                D_8036B970 = 1;
                done = 1;
            }
        }
        p++;
    }
    if (D_8036B970 && minDist > 400) {
        D_8036B970 = 0;
    }
    if (D_802E8BDC == 0 && D_8036C7CC >= 2) {
        func_8026AD30(0x4D);
    }
    if (D_802E8BDC == 0 && D_80364456 == 7 && (D_803643E0 >> 5) >= 0x899) {
        func_8026AD30(0x53);
    }
    if (D_8036B971) {
        if (!((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0)
            && !D_8036EB98) {
            if (!func_8026AD30(0x4E)) {
                func_8026AD30(0x4F);
            }
            D_8036B971 = 0;
        }
    }
}
