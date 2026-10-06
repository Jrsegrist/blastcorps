#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/*
 * Ballistic shells: up to four lobbed projectiles fired from one point through
 * another, flying on a fixed-gravity arc, pitched along their path, and
 * exploding when they leave the map, hit the ground or touch something.
 */

typedef struct {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ s32 prevY;
    /* 0x10 */ s16 vx;
    /* 0x12 */ s16 vz;
    /* 0x14 */ s16 yaw;
    /* 0x16 */ s16 pitch;
    /* 0x18 */ u8 type;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s32 y0; /* launch height */
    /* 0x20 */ s32 vy; /* launch vertical speed */
    /* 0x24 */ s32 t;  /* frames since launch */
    /* 0x28 */ u8 active;
    /* 0x29 */ u8 alpha;
} Shell; /* size 0x2C */

/* Per-type shell data: model display list, collision id, explosion and trail parameters. */
typedef struct {
    /* 0x000 */ Gfx dl[180];
    /* 0x5A0 */ s16 unk5A0;
    /* 0x5A4 */ s32 unk5A4;
    /* 0x5A8 */ u8 unk5A8;
    /* 0x5A9 */ u8 unk5A9;
    /* 0x5AC */ s32 unk5AC;
    /* 0x5B0 */ u8 unk5B0;
} ShellType; /* size 0x5B8 */

/* Per-frame dynamic buffer (segment 2); the shell matrices sit at 0xD00. */
typedef struct Dyn {
    u8 pad[0xD00];
    Mtx mtx[4];
} Dyn;

extern s8 D_802E8BE4;
extern ShellType D_802FE3C0[];
extern s8 D_803643D9;
extern void *D_80367738;
extern Shell D_8039C960[4];
extern s16 D_803A7410;
extern s16 D_803A7412;
extern u16 D_803BE714;
extern Mtx D_02000000[];

f32 sqrtf(f32);
void func_80292DDC(s32 i);

void func_80292240(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        D_8039C960[i].active = 0;
    }
}

s32 func_80292288(s16 speed, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, u8 type, s16 arg8) {
    s32 i;
    u8 found;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;

    i = 0;
    found = 0;
    while (!found && i < 4) {
        if (D_8039C960[i].active) {
            i++;
        } else {
            found = 1;
        }
    }
    if (!found) {
        return 0;
    }
    D_8039C960[i].active = 1;
    D_8039C960[i].type = type;
    D_8039C960[i].unk1A = arg8;
    D_8039C960[i].x = x1;
    D_8039C960[i].y = y1;
    D_8039C960[i].z = z1;
    D_8039C960[i].y0 = y1;
    D_8039C960[i].t = 0;
    D_8039C960[i].alpha = 255;
    dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
    guNormalize(&dx, &dy, &dz);
    D_8039C960[i].vx = speed * dx;
    D_8039C960[i].vz = speed * dz;
    D_8039C960[i].vy = speed * dy;
    dist = sqrtf((x1 - x0) * (x1 - x0) + (z1 - z0) * (z1 - z0));
    if (dist < 1.0) {
        dist = 1.0f;
    }
    if (x1 >= x0 && z1 >= z0) {
        D_8039C960[i].yaw = func_802AD7D4((x1 - x0) * 65535.0 / dist) >> 4;
    }
    if (x1 >= x0 && z1 < z0) {
        D_8039C960[i].yaw = (func_802AD7D4((z0 - z1) * 65535.0 / dist) >> 4) + 0x400;
    }
    if (x1 < x0 && z1 < z0) {
        D_8039C960[i].yaw = (func_802AD7D4((x0 - x1) * 65535.0 / dist) >> 4) + 0x800;
    }
    if (x1 < x0 && z1 >= z0) {
        D_8039C960[i].yaw = (func_802AD7D4((z1 - z0) * 65535.0 / dist) >> 4) + 0xC00;
    }
    return 1;
}

void func_80292830(void) {
    s32 i;
    s16 pitch;
    s16 savedA;
    s16 savedB;
    s32 limX;
    s32 limZ;

    limX = D_803BE714 * D_803BE70C;
    limZ = D_803BE716 * D_803BE710;
    for (i = 0; i < 4; i++) {
        if (D_8039C960[i].active) {
            D_8039C960[i].x += D_8039C960[i].vx;
            D_8039C960[i].z += D_8039C960[i].vz;
            D_8039C960[i].y = (D_8039C960[i].y0 + D_8039C960[i].vy * D_8039C960[i].t) +
                              -12.0 * D_8039C960[i].t * D_8039C960[i].t;
            pitch = D_8039C960[i].prevY - D_8039C960[i].y;
            if (pitch >= 0x400) {
                pitch = 0x3FF;
            }
            if (pitch < -0x3FF) {
                pitch = -0x3FF;
            }
            D_8039C960[i].pitch = pitch;
            if (D_8039C960[i].x >= limX || D_8039C960[i].z >= limZ || D_8039C960[i].x < 0 ||
                D_8039C960[i].z < 0 || D_8039C960[i].y < 0) {
                func_80292DDC(i);
            } else {
                func_802CE4F0(D_8039C960[i].x, D_8039C960[i].y, D_8039C960[i].z);
                func_802CDF94(D_802FE3C0[D_8039C960[i].type].unk5A0);
                if (D_803F932D) {
                    D_803643D9 = 1;
                }
                savedA = D_803A7410, savedB = D_803A7412;
                func_802CE5BC(D_8039C960[i].x, D_8039C960[i].y, D_8039C960[i].z,
                              D_802FE3C0[D_8039C960[i].type].unk5A0, 0xCA, 0);
                func_802CDB70(D_802FE3C0[D_8039C960[i].type].unk5A0, D_8039C960[i].unk1A);
                if (savedA != D_803A7410 || savedB != D_803A7412) {
                    func_80292DDC(i);
                }
                if (D_8039C960[i].y < func_802CE6F8(D_8039C960[i].x, D_8039C960[i].z, D_8039C960[i].y)) {
                    func_80292DDC(i);
                }
                if (D_802FE3C0[D_8039C960[i].type].unk5A9) {
                    func_802AC61C(D_8039C960[i].x, D_8039C960[i].y, D_8039C960[i].z,
                                  D_802FE3C0[D_8039C960[i].type].unk5B0, D_802FE3C0[D_8039C960[i].type].unk5AC);
                }
                D_8039C960[i].prevY = D_8039C960[i].y;
                D_8039C960[i].alpha -= 25;
                if (D_8039C960[i].alpha < 100) {
                    D_8039C960[i].alpha = 255;
                }
                D_8039C960[i].t++;
            }
        }
    }
}

void func_80292DDC(s32 i) {
    D_8039C960[i].active = 0;
    func_802AC61C(D_8039C960[i].x, D_8039C960[i].y, D_8039C960[i].z, D_802FE3C0[D_8039C960[i].type].unk5A8,
                  D_802FE3C0[D_8039C960[i].type].unk5A4);
    D_802E8BE4 = 10;
    D_802E8BE8 = 400;
    func_80260650(D_80367738, 16, 0);
}

void func_80292EB8(Gfx **gdl, Dyn *dyn) {
    Gfx *gfx = *gdl;
    s32 i;
    f32 mf1[4][4];
    f32 mf2[4][4];

    gDPPipeSync(gfx++);
    gDPSetCycleType(gfx++, G_CYC_1CYCLE);
    gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineMode(gfx++, G_CC_SHADE, G_CC_SHADE);
    gSPClearGeometryMode(gfx++, -1);
    gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    for (i = 0; i < 4; i++) {
        if (D_8039C960[i].active && D_8039C960[i].t >= 2) {
            guScaleF(mf1, 0.5f, 0.5f, 0.35f);
            guRotateF(mf2, D_8039C960[i].pitch / 4095.0 * 360.0, 1.0f, 0.0f, 0.0f);
            guMtxCatF(mf1, mf2, mf1);
            guRotateF(mf2, D_8039C960[i].yaw / 4095.0 * 360.0, 0.0f, 1.0f, 0.0f);
            guMtxCatF(mf1, mf2, mf1);
            guTranslateF(mf2, D_8039C960[i].x / 32.0f, D_8039C960[i].y / 32.0f, D_8039C960[i].z / 32.0f);
            guMtxCatF(mf1, mf2, mf1);
            guMtxF2L(mf1, &dyn->mtx[i]);
            gSPMatrix(gfx++, &D_02000000[i + 52], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gDPPipeSync(gfx++);
            gDPSetPrimColor(gfx++, 0, 0, D_8039C960[i].alpha, 0, 0, 255);
            gSPDisplayList(gfx++, osVirtualToPhysical(D_802FE3C0[D_8039C960[i].type].dl));
            gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
        }
    }
    gDPPipeSync(gfx++);
    *gdl = gfx;
}
