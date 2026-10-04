#include "common.h"
#include <ultra64.h>

typedef struct {
    /* 0x00 */ u8 unk0[0x78];
    /* 0x78 */ s32 unk78;
    /* 0x7C */ f32 unk7C;
    /* 0x80 */ u8 unk80;
    /* 0x81 */ u8 unk81;
    /* 0x82 */ u8 pad82[2];
} Unk802FE980;

typedef struct {
    /* 0x00 */ u32 time;
    /* 0x04 */ s32 angle;
} SwingState;

extern u8 D_802E8BD0;
extern u32 D_803156C4;
extern Unk802FE980 D_802FE980[];
extern f32 D_8039CA10[4][4];
extern f32 D_8039CA50;
extern f32 D_8039CA54;
extern f32 D_8039CA58;
extern s32 D_803F7664;
extern s32 D_803F7668;
extern s32 D_803F766C;

void func_802936AC(f32 mf[4][4], s32 x, s32 z, s32 arg3, s32 arg4, s16 *rx, s16 *ry, s16 *rz, void *state, s32 idx);
void func_80293F84(f32 mf[4][4], s32 x, s32 z, s16 *rx, s16 *ry, s16 *rz, void *state, s32 idx);
void func_80294B64(f32 mf[4][4], s32 limit, s16 *x, s16 *y, s16 *z, SwingState *state, s32 delay);
void func_80294C50(f32 mf[4][4], s16 *x, s16 *y, s16 *z, s32 *counter, s32 amp);
void func_80294D24(f32 mf[4][4], s16 *x, s16 *y, s16 *z, s32 *angle, s32 speed);

void func_802933A0(s32 x, s32 y, s32 z, s32 type, Mtx *mtx, void *state, Gfx *gfx1, Gfx *gfx2, s32 idx,
                   s32 arg9, s32 arg10, s32 arg11) {
    s16 rx;
    s16 ry;
    s16 rz;
    f32 mf[4][4];
    f32 tmp[4][4];

    rx = 0;
    ry = 0;
    rz = 0;
    switch (type) {
        case 1:
            func_80294C50(mf, &rx, &ry, &rz, (s32 *) state, idx);
            break;
        case 2:
            func_80294D24(mf, &rx, &ry, &rz, (s32 *) state, idx);
            break;
        case 3:
            func_80293F84(mf, x >> 5, z >> 5, &rx, &ry, &rz, state, idx);
            break;
        case 4:
            func_802936AC(mf, x >> 5, z >> 5, arg9 >> 5, arg11 >> 5, &rx, &ry, &rz, state, idx);
            break;
        case 5:
            func_80294B64(mf, 90, &rx, &ry, &rz, (SwingState *) state, idx);
            break;
        default:
            guTranslateF(mf, 0.0f, 0.0f, 0.0f);
            break;
    }
    if (type == 3 && D_802FE980[idx].unk81 == 1) {
        y = 0;
    }
    guTranslateF(tmp, x / 32.0f, y / 32.0f, z / 32.0f);
    guMtxCatF(mf, tmp, mf);
    guMtxF2L(mf, mtx);
    D_803F7664 = (rx << 5) + x;
    D_803F7668 = (ry << 5) + y;
    D_803F766C = (rz << 5) + z;
    gSPMatrix(gfx1++, osVirtualToPhysical(mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(gfx2++, osVirtualToPhysical(mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/4EBE0/func_802936AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/4EBE0/func_80293F84.s")

f32 func_80294840(f32 a, f32 b, f32 c, f32 d) {
    s32 v0;
    s32 v1;
    s32 v2;
    s32 v3;

    v0 = D_8039CA10[0][0] * a + D_8039CA10[1][0] * b + D_8039CA10[2][0] * c + D_8039CA10[3][0] * d;
    v1 = D_8039CA10[0][1] * a + D_8039CA10[1][1] * b + D_8039CA10[2][1] * c + D_8039CA10[3][1] * d;
    v2 = D_8039CA10[0][2] * a + D_8039CA10[1][2] * b + D_8039CA10[2][2] * c + D_8039CA10[3][2] * d;
    v3 = D_8039CA10[0][3] * a + D_8039CA10[1][3] * b + D_8039CA10[2][3] * c + D_8039CA10[3][3] * d;
    return v0 * D_8039CA58 + D_8039CA54 * v1 + D_8039CA50 * v2 + v3;
}

void func_802949B0(s32 idx) {
    f32 s;

    s = D_802FE980[idx].unk7C;
    D_8039CA10[0][0] = -s;
    D_8039CA10[0][1] = s * 2.0;
    D_8039CA10[0][2] = -s;
    D_8039CA10[0][3] = 0.0f;
    D_8039CA10[1][0] = 2.0 - s;
    D_8039CA10[1][1] = s - 3.0;
    D_8039CA10[1][2] = 0.0f;
    D_8039CA10[1][3] = 1.0f;
    D_8039CA10[2][0] = s - 2.0;
    D_8039CA10[2][1] = 3.0 - s * 2.0;
    D_8039CA10[2][2] = s;
    D_8039CA10[2][3] = 0.0f;
    D_8039CA10[3][0] = s;
    D_8039CA10[3][1] = -s;
    D_8039CA10[3][2] = 0.0f;
    D_8039CA10[3][3] = 0.0f;
}

void func_80294B64(f32 mf[4][4], s32 limit, s16 *x, s16 *y, s16 *z, SwingState *state, s32 delay) {
    if (state->time == 0) {
        state->time = delay + D_803156C4;
    }
    if (D_803156C4 > state->time && D_802E8BD0 == 0) {
        state->angle++;
        if (state->angle > limit) {
            state->angle = limit;
        }
    }
    guRotateF(mf, -state->angle, 0.0f, 1.0f, 0.0f);
    *x = 0;
    *y = 0;
    *z = 0;
}

void func_80294C50(f32 mf[4][4], s16 *x, s16 *y, s16 *z, s32 *counter, s32 amp) {
    f32 s;

    if (D_802E8BD0 == 0) {
        (*counter)++;
    }
    s = sinf(*counter / 5.0f);
    *x = 0;
    *z = 0;
    *y = amp * s;
    guTranslateF(mf, 0.0f, *y, 0.0f);
}

void func_80294D24(f32 mf[4][4], s16 *x, s16 *y, s16 *z, s32 *angle, s32 speed) {
    f32 m[4][4];
    f32 ox;
    f32 oy;
    f32 oz;

    if (D_802E8BD0 == 0) {
        *angle += speed;
    }
    guRotateF(m, *angle, 0.2f, 0.7f, 0.1f);
    guMtxXFMF(m, 90.0f, 0.0f, 0.0f, &ox, &oy, &oz);
    *x = ox;
    *y = oy;
    *z = oz;
    guTranslateF(mf, ox, oy, oz);
}
