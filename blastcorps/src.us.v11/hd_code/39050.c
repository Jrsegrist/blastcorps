#include "common.h"
#include <ultra64.h>

/* One entry of the digger/object table at D_80364460 (0x74 bytes each);
 * D_803649D0 points one past the last live entry. */
typedef struct {
    u8 pad0[0x5C];
    s32 type;  /* 0x5C: digger weight class, see func_8027E228 */
    s32 pad60;
    s32 unk64; /* 0x64 */
    s32 pad68;
    s32 unk6C; /* 0x6C */
    s32 unk70; /* 0x70 */
} Digger;

extern Digger D_80364460[];
extern Digger *D_803649D0;
extern s32 D_803649E8;
extern s32 D_802FC51C;

f32 sinf(f32);
void func_802C1F30(s32, s32, s32, s32, s32);
s16 *func_802C1EE0(s32);
void func_8029A7E4(const char *fmt, ...);
s32 func_8027E164(s32 arg0, s32 arg1, void *arg2, void *arg3);
f32 func_8027DD88(s32, s32, s32 *, s32 *);
f32 func_8027E228();
f32 func_8027DB5C(s32 *a, s32 *b, s32 arg2);
void func_8027DA10(s32 arg0, s32 arg1, s32 arg2);

void func_8027D810(s32 arg0) {
    switch (arg0) {
    case 0:
        func_8027DA10(1, 7, 0x320000);
        break;
    case 4:
        func_8027DA10(1, 7, 0xA00000);
        break;
    case 16:
        func_8027DA10(1, 7, 0x3C0000);
        func_8027DA10(2, 7, 0x3C0000);
        break;
    case 20:
        func_8027DA10(1, 7, 0x820000);
        func_8027DA10(2, 7, 0x820000);
        break;
    case 15:
        func_8027DA10(1, 7, 0x410000);
        func_8027DA10(2, 7, 0x410000);
        break;
    }
}

void func_8027D8F4(s32 arg0, s32 arg1, s32 arg2) {
    s32 i;
    s32 j;
    f32 step;
    f32 s;
    f32 angle;

    j = 1;
    step = 6.28318 / (arg1 + 1);
    angle = D_802FC51C / 20.0;
    for (i = D_802FC51C; i < arg1 + D_802FC51C; i++) {
        s = sinf(angle);
        func_802C1F30(arg0, j++, 0, -s * arg2, 0);
        angle += step;
    }
    D_802FC51C++;
}

void func_8027DA10(s32 arg0, s32 arg1, s32 arg2) {
    f32 r;
    f32 s;
    f32 step;
    s32 i;
    s16 *p;
    s32 pad[5];
    s32 x[4];
    s32 z[4];

    p = func_802C1EE0(arg0);
    for (i = 0; i < 4; i++) {
        x[i] = p[i * 2] << 5;
        z[i] = p[i * 2 + 1] << 5;
    }
    r = func_8027DB5C(x, z, arg2);
    step = 3.14159 / (arg1 + 1);
    for (i = 0; i < arg1; i++) {
        s = sinf((i + 1) * step);
        func_802C1F30(arg0, i + 1, 0, -s * r, 0);
    }
}

f32 func_8027DB5C(s32 *a, s32 *b, s32 arg2) {
    s32 i;
    f32 max;
    f32 v;
    f32 w;
    f32 t;
    s32 type;

    i = 0;
    max = 0.0f;
    while (&D_80364460[i] != D_803649D0) {
        if ((type = D_80364460[i].type) != 0xFE && (type != 0 || D_803649E8 == 0)) {
            if (func_8027E164(D_80364460[i].unk64, D_80364460[i].unk6C, a, b) != 0) {
                if (D_80364460[i].unk70 != 0) {
                    t = func_8027DD88(D_80364460[i].unk64, D_80364460[i].unk6C, a, b);
                    if (t <= 0.5) {
                        t = t * 2.0;
                    } else {
                        t = (1.0 - t) * 2.0;
                    }
                    w = func_8027E228(D_80364460[i].type);
                    v = t * w;
                    if (max < v) {
                        max = v;
                    }
                }
            }
        }
        i++;
    }
    return arg2 * max;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027DD88.s")

s32 func_802AC4C4(s32, s32, s32, s32, s32, s32, s32, s32);

s32 func_8027E164(s32 arg0, s32 arg1, void *arg2, void *arg3) {
    if (func_802AC4C4(arg0, arg1, *(s32 *)((u8 *) arg2 + 0x0), *(s32 *)((u8 *) arg3 + 0x0),
                       *(s32 *)((u8 *) arg2 + 0x4), *(s32 *)((u8 *) arg3 + 0x4),
                       *(s32 *)((u8 *) arg2 + 0x8), *(s32 *)((u8 *) arg3 + 0x8)) != 0) {
        return 1;
    }
    if (func_802AC4C4(arg0, arg1, *(s32 *)((u8 *) arg2 + 0x0), *(s32 *)((u8 *) arg3 + 0x0),
                       *(s32 *)((u8 *) arg2 + 0x8), *(s32 *)((u8 *) arg3 + 0x8),
                       *(s32 *)((u8 *) arg2 + 0xC), *(s32 *)((u8 *) arg3 + 0xC)) != 0) {
        return 1;
    }
    return 0;
}

f32 func_8027E228(type)
    u8 type;
{
    switch (type) {
    case 0:
        return 0.3f;
    case 1:
        return 0.8f;
    case 5:
        return 0.8f;
    case 4:
        return 0.8f;
    case 2:
        return 0.8f;
    case 16:
        return 0.4f;
    case 3:
        return 0.8f;
    case 8:
        return 0.6f;
    case 10:
        return 0.6f;
    case 13:
        return 0.8f;
    case 14:
        return 0.6f;
    case 15:
        return 0.6f;
    case 9:
        return 0.0f;
    case 0xFF:
        return 0.8f;
    default:
        func_8029A7E4("DIGGER WEIGHT NOT SET\n");
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027E344.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027E9B8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027EED8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8027F1F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_802802D4.s")

/* Set the 8 corner positions of the box (x0..x1, y0..y1, z0..z1). */
void func_8028072C(Vtx *v, s16 x0, s16 y0, s16 z0, s16 x1, s16 y1, s16 z1) {
    v[0].v.ob[0] = x0;
    v[0].v.ob[1] = y0;
    v[0].v.ob[2] = z0;
    v[1].v.ob[0] = x0;
    v[1].v.ob[1] = y1;
    v[1].v.ob[2] = z0;
    v[2].v.ob[0] = x1;
    v[2].v.ob[1] = y0;
    v[2].v.ob[2] = z0;
    v[3].v.ob[0] = x1;
    v[3].v.ob[1] = y1;
    v[3].v.ob[2] = z0;
    v[4].v.ob[0] = x0;
    v[4].v.ob[1] = y0;
    v[4].v.ob[2] = z1;
    v[5].v.ob[0] = x0;
    v[5].v.ob[1] = y1;
    v[5].v.ob[2] = z1;
    v[6].v.ob[0] = x1;
    v[6].v.ob[1] = y0;
    v[6].v.ob[2] = z1;
    v[7].v.ob[0] = x1;
    v[7].v.ob[1] = y1;
    v[7].v.ob[2] = z1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_802807D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80280F34.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281A70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281CE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80281E44.s")

void func_802A0B00(s32, s32);

extern s32 D_80358070;
extern s32 D_8036E4CC;
extern s16 D_8036E4D0;
extern s8 D_8036E4D2;

void func_802821D0(void) {
    D_8036E4CC = D_80358070;
    func_802A0B00(0xA98, 0);
    D_80358070 += 0x800;
    D_8036E4D0 = 0;
    D_8036E4D2 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_80282224.s")

extern u8 D_8036E4D3;
extern s32 D_8036E4D4;

void func_80282728(void) {
    D_8036E4D3 = 0;
    D_8036E4D4 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/39050/func_8028273C.s")
