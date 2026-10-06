#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* Spline control point, 0xC bytes */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 rx;
    /* 0x08 */ s16 ry;
    /* 0x0A */ s16 rz;
} PathPoint;

/* Spline path, 0x84 bytes */
typedef struct {
    /* 0x00 */ PathPoint pts[10];
    /* 0x78 */ s32 count;
    /* 0x7C */ f32 tension;
    /* 0x80 */ u8 speed;
    /* 0x81 */ u8 mode; /* 0: y from the spline, 1: y from the ground */
    /* 0x82 */ u8 pad82[2];
} Path;

typedef struct {
    /* 0x00 */ s32 seg;
    /* 0x04 */ s32 t; /* 0..999 within the segment */
} PathState;

typedef struct {
    /* 0x00 */ s32 dir; /* 1, 2, 4, 8 */
    /* 0x04 */ s32 angle; /* degrees */
    /* 0x08 */ s32 last; /* last junction visited */
} WalkState;

/* Junction on a walker's grid, 8 bytes */
typedef struct {
    /* 0x00 */ u8 level;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 exits; /* bitmask of dirs */
    /* 0x07 */ u8 pad7;
} Junction;

typedef struct {
    /* 0x00 */ u32 time;
    /* 0x04 */ s32 angle;
} SwingState;

extern Path D_802FE980[];
extern f32 D_8039CA10[4][4];
extern f32 D_8039CA50;
extern f32 D_8039CA54;
extern f32 D_8039CA58;
extern Junction D_802FEDA0[];

#ifdef NON_MATCHING
void func_802936AC(f32 mf[4][4], s16 x, s16 z, s16 ox, s16 oz, s16 *px, s16 *py, s16 *pz, WalkState *state,
                   s32 speed);
void func_80293F84(f32 mf[4][4], s16 x, s16 z, s16 *px, s16 *py, s16 *pz, PathState *state, s32 idx);
#else
void func_802936AC(); /* K&R */
void func_80293F84(); /* K&R */
#endif
f32 func_80294840(f32 a, f32 b, f32 c, f32 d);
void func_802949B0(s32 idx);
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
    if (type == 3 && D_802FE980[idx].mode == 1) {
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

#define ABS(x) ((x) < 0 ? -(x) : (x))

#ifdef NON_MATCHING
void func_802936AC(f32 mf[4][4], s16 x, s16 z, s16 ox, s16 oz, s16 *px, s16 *py, s16 *pz, WalkState *state,
                   s32 speed)
#else
void func_802936AC(mf, x, z, ox, oz, px, py, pz, state, speed)
    f32 mf[4][4];
    s16 x;
    s16 z;
    s16 ox;
    s16 oz;
    s16 *px;
    s16 *py;
    s16 *pz;
    WalkState *state;
    s32 speed;
#endif
{
    f32 m[4][4];
    s32 i;
    u8 found;
    s16 targetAngle;
    s16 cx;
    s16 cz;
    s16 dx;
    s16 dz;
    s16 adx;
    s16 adz;
    u8 exits;
    u8 want;
    s32 back;
    s16 cur;
    s16 target;
    s16 diff;
    s32 offset;

    i = 0;
    found = 0;
    if (D_8036E4C8 == 0 && D_8036DCD8 != 0) {
        func_802608C8(D_8036DCD8);
    }
    if (D_802E8BD0 == 0) {
        if (((u8) D_8036E4CA)) {
            state->last = -1;
            switch (state->dir) {
                case 1:
                    state->dir = 2;
                    break;
                case 2:
                    state->dir = 1;
                    break;
                case 4:
                    state->dir = 8;
                    break;
                case 8:
                    state->dir = 4;
                    break;
            }
        }
        while (i < 102 && !found) {
            if (D_802FEDA0[i].level == D_802E8BDC && D_802FEDA0[i].x - 5 <= ox && D_802FEDA0[i].z - 5 <= oz &&
                ox < D_802FEDA0[i].x + 5 && oz < D_802FEDA0[i].z + 5 && state->last != i) {
                found = 1;
            } else {
                i++;
            }
        }
        if (found) {
            state->last = i;
            cx = D_803643E0 >> 5, cz = D_803643E8 >> 5;
            if (D_8036E4C8 == 0) {
                dx = cx - ox, dz = cz - oz;
            } else {
                dx = ox - cx;
                dz = oz - cz;
            }
            adx = ABS(dx);
            adz = ABS(dz);
            switch (state->dir) {
                case 1:
                    back = 2;
                    break;
                case 2:
                    back = 1;
                    break;
                case 4:
                    back = 8;
                    break;
                case 8:
                    back = 4;
                    break;
                default:
                    back = 0;
                    break;
            }
            exits = (D_802FEDA0[i].exits & back) ? (D_802FEDA0[i].exits & (back ^ 0xFF)) : D_802FEDA0[i].exits;
            if (dz > 0 && adz >= adx) {
                want = 1;
            }
            if (dz <= 0 && adz >= adx) {
                want = 2;
            }
            if (dx > 0 && adx >= adz) {
                want = 4;
            }
            if (dx <= 0 && adx >= adz) {
                want = 8;
            }
            if (exits & want) {
                state->dir = want;
            } else {
                if (want == 1) {
                    if (exits & 4) {
                        state->dir = 4;
                    } else if (exits & 8) {
                        state->dir = 8;
                    } else {
                        state->dir = 2;
                    }
                }
                if (want == 2) {
                    if (exits & 8) {
                        state->dir = 8;
                    } else if (exits & 4) {
                        state->dir = 4;
                    } else {
                        state->dir = 1;
                    }
                }
                if (want == 4) {
                    if (exits & 1) {
                        state->dir = 1;
                    } else if (exits & 2) {
                        state->dir = 2;
                    } else {
                        state->dir = 8;
                    }
                }
                if (want == 8) {
                    if (exits & 2) {
                        state->dir = 2;
                    } else if (exits & 1) {
                        state->dir = 1;
                    } else {
                        state->dir = 4;
                    }
                }
            }
        }
        if (D_8036E4C8 == 0) {
            offset = speed;
        } else {
            offset = speed * 3 / 2;
        }
        switch (state->dir) {
            case 1:
                *px = ox - x;
                *pz = oz - z + offset;
                targetAngle = 270;
                break;
            case 2:
                *px = ox - x;
                *pz = oz - z - offset;
                targetAngle = 90;
                break;
            case 4:
                *px = ox - x + offset;
                *pz = oz - z;
                targetAngle = 0;
                break;
            case 8:
                *px = ox - x - offset;
                *pz = oz - z;
                targetAngle = 180;
                break;
        }
        *py = 0;
        cur = (state->angle << 16) / 360;
        target = (targetAngle << 16) / 360;
        diff = target - cur;
        if ((diff > 0 ? diff : -diff) < 2000) {
            state->angle = targetAngle;
        } else {
            if (diff > 0) {
                state->angle += 10;
            } else {
                state->angle -= 10;
            }
            if (state->angle >= 360) {
                state->angle -= 360;
            }
            if (state->angle < 0) {
                state->angle += 360;
            }
        }
    } else {
        *px = ox - x;
        *py = 0;
        *pz = oz - z;
    }
    guRotateF(mf, state->angle, 0.0f, 1.0f, 0.0f);
    if (D_8036E4C8 == 0 || (D_803156C4 & 0xF) >= 7) {
        guTranslateF(m, *px, *py, *pz);
    } else {
        guTranslateF(m, 0.0f, 20000.0f, 0.0f);
    }
    guMtxCatF(mf, m, mf);
}

#ifdef NON_MATCHING
void func_80293F84(f32 mf[4][4], s16 x, s16 z, s16 *px, s16 *py, s16 *pz, PathState *state, s32 idx)
#else
void func_80293F84(mf, x, z, px, py, pz, state, idx)
    f32 mf[4][4];
    s16 x;
    s16 z;
    s16 *px;
    s16 *py;
    s16 *pz;
    PathState *state;
    s32 idx;
#endif
{
    s32 seg;
    s32 last;
    f32 rotX;
    f32 rotY;
    f32 rotZ;
    f32 m[4][4];
    u8 k[4];
    f32 p0;
    f32 p1;
    f32 p2;
    f32 p3;

    func_802949B0(idx);
    if (D_802E8BD0 == 0) {
        state->t += D_802FE980[idx].speed;
        if (state->t >= 1000) {
            state->t = 0;
            state->seg++;
            if (state->seg >= D_802FE980[idx].count - 1) {
                state->seg = 0;
            }
        }
    }
    seg = state->seg;
    last = D_802FE980[idx].count - 1;
    if (seg <= 0) {
        k[0] = last + seg - 1;
    } else {
        k[0] = seg - 1;
    }
    k[1] = seg;
    if (seg + 1 >= last) {
        k[2] = seg - last + 1;
    } else {
        k[2] = seg + 1;
    }
    if (seg + 2 >= last) {
        k[3] = seg - last + 2;
    } else {
        k[3] = seg + 2;
    }
    D_8039CA50 = (f32) state->t / 1000.0;
    D_8039CA54 = D_8039CA50 * D_8039CA50;
    D_8039CA58 = D_8039CA54 * D_8039CA50;

    p0 = D_802FE980[idx].pts[k[0]].rx, p1 = D_802FE980[idx].pts[k[1]].rx, p2 = D_802FE980[idx].pts[k[2]].rx,
    p3 = D_802FE980[idx].pts[k[3]].rx;
    func_8026A2E8(p0, &p1);
    func_8026A2E8(p1, &p2);
    func_8026A2E8(p2, &p3);
    rotX = func_80294840(p0, p1, p2, p3);

    p0 = D_802FE980[idx].pts[k[0]].ry, p1 = D_802FE980[idx].pts[k[1]].ry, p2 = D_802FE980[idx].pts[k[2]].ry,
    p3 = D_802FE980[idx].pts[k[3]].ry;
    func_8026A2E8(p0, &p1);
    func_8026A2E8(p1, &p2);
    func_8026A2E8(p2, &p3);
    rotY = func_80294840(p0, p1, p2, p3);

    p0 = D_802FE980[idx].pts[k[0]].rz, p1 = D_802FE980[idx].pts[k[1]].rz, p2 = D_802FE980[idx].pts[k[2]].rz,
    p3 = D_802FE980[idx].pts[k[3]].rz;
    func_8026A2E8(p0, &p1);
    func_8026A2E8(p1, &p2);
    func_8026A2E8(p2, &p3);
    rotZ = func_80294840(p0, p1, p2, p3);

    *px = func_80294840(D_802FE980[idx].pts[k[0]].x, D_802FE980[idx].pts[k[1]].x, D_802FE980[idx].pts[k[2]].x,
                        D_802FE980[idx].pts[k[3]].x);
    *pz = func_80294840(D_802FE980[idx].pts[k[0]].z, D_802FE980[idx].pts[k[1]].z, D_802FE980[idx].pts[k[2]].z,
                        D_802FE980[idx].pts[k[3]].z);
    switch (D_802FE980[idx].mode) {
        case 0:
            *py = func_80294840(D_802FE980[idx].pts[k[0]].y, D_802FE980[idx].pts[k[1]].y,
                                D_802FE980[idx].pts[k[2]].y, D_802FE980[idx].pts[k[3]].y);
            break;
        case 1:
            func_8027EED8(*px + x, *pz + z, py);
            break;
    }
    guRotateF(m, rotX, 1.0f, 0.0f, 0.0f);
    guRotateF(mf, rotY, 0.0f, 1.0f, 0.0f);
    guMtxCatF(m, mf, m);
    guRotateF(mf, rotZ, 0.0f, 0.0f, 1.0f);
    guMtxCatF(m, mf, m);
    guTranslateF(mf, *px, *py, *pz);
    guMtxCatF(m, mf, mf);
}

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

    s = D_802FE980[idx].tension;
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
