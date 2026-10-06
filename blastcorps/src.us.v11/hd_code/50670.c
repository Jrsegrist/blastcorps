#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* ghostdigger.c: records the player's path and plays it back as a ghost */

/* One recorded sample, 0x14 bytes */
typedef struct {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ s16 rx;
    /* 0x0E */ s16 ry;
    /* 0x10 */ s16 rz;
    /* 0x12 */ s16 time;
} GhostSample;

typedef struct {
    /* 0x00 */ s32 playCount;
    /* 0x04 */ u32 recCount;
} GhostCounts;

/* Render-mode remap entry, 8 bytes */
typedef struct {
    /* 0x00 */ u32 from;
    /* 0x04 */ u32 to;
} GhostRenderMode;

/* Trigger quad, 0x2C bytes */
typedef struct {
    /* 0x00 */ s32 x[4];
    /* 0x10 */ s32 z[4];
    /* 0x20 */ s8 list[8];
    /* 0x28 */ u8 id;
} GhostQuad;

void func_80295394(s32 *x, s32 *y, s32 *z, s16 *rx, s16 *ry, s16 *rz);
s16 func_80295924(s16 from, s16 to, f32 t);

extern s16 D_803ED390[];
extern s32 D_802FF0D0[];
extern GhostRenderMode D_802FF11C[];
extern GhostQuad D_802FF150[];
extern void *D_803BDB00;
extern void *D_803BDB04;
extern Gfx *D_803BDB08;
extern Mtx D_02000000[];

extern GhostSample *D_8039CA68[];
extern GhostCounts D_8039CA70[];
extern s32 D_8039CA78;
extern u8 D_8039CA7C;
extern u8 D_8039CA7D;
extern s32 D_8039CA80;
extern s32 D_8039CA84;
extern u32 D_8039CA88;
extern u8 D_8039CA8C;

void func_80294E30(void) {
    D_8039CA68[0] = (GhostSample *) 0x80055400;
    D_8039CA68[1] = (GhostSample *) 0x80065400;
    D_8039CA88 = func_80286038(0xFFFF) - 1;
    D_8039CA7D = 0;
}

void func_80294E88(void) {
    D_8039CA70[0].recCount = 0;
    D_8039CA80 = -1;
    D_8039CA62 = 1;
    D_8039CA8C = 0;
}

void func_80294EB8(void) {
    if (D_8039CA7D) {
        D_8039CA61 = 1;
        D_8039CA84 = -1;
        D_8039CA78 = 0;
        D_8039CA7E = D_8039CA7C;
    }
}

void func_80294F00(void) {
    if (D_80364A90 & 0x104) {
        if (D_8039CA80 == -1) {
            D_8039CA80 = ((s32) D_803156C0);
        }
        if (D_8039CA70[0].recCount < 0xCCC) {
            D_8039CA68[1][D_8039CA70[0].recCount].x = D_803643E0;
            D_8039CA68[1][D_8039CA70[0].recCount].y = D_803643E4;
            D_8039CA68[1][D_8039CA70[0].recCount].z = D_803643E8;
            D_8039CA68[1][D_8039CA70[0].recCount].rx = D_803ED390[0];
            D_8039CA68[1][D_8039CA70[0].recCount].ry = D_803ED390[1];
            D_8039CA68[1][D_8039CA70[0].recCount].rz = D_803ED390[2];
            D_8039CA68[1][D_8039CA70[0].recCount].time = ((s32) D_803156C0) - D_8039CA80;
            D_8039CA70[0].recCount++;
        } else {
            D_8039CA8C = 1;
            func_8029A7E4("OVERRUN GHOST DIGGER ARRAY\n");
        }
    }
}

void func_80295120(Gfx **gfxp, Mtx *mtx) {
    Gfx *gdl = *gfxp;
    s32 x;
    s32 y;
    s32 z;
    s16 rx;
    s16 ry;
    s16 rz;

    if (D_8039CA61) {
        func_80295394(&x, &y, &z, &rx, &ry, &rz);
        func_802AA6D0(x, y, z, rx, ry, rz, D_802FF0D0[D_8039CA7C], &mtx[83]);
        gSPSegment(gdl++, 6, osVirtualToPhysical(D_803BDB04));
        gSPSegment(gdl++, 7, osVirtualToPhysical(D_803BDB00));
        gSPMatrix(gdl++, &D_02000000[83], G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
        gDPSetEnvColor(gdl++, 0, 0, 0, 255);
        gDPSetPrimColor(gdl++, 0, 0, 255, 255, 255, 100);
        gSPClearGeometryMode(gdl++, -1);
        gSPDisplayList(gdl++, osVirtualToPhysical(D_803BDB08));
        gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
    }
    *gfxp = gdl;
}

void func_80295394(s32 *x, s32 *y, s32 *z, s16 *rx, s16 *ry, s16 *rz) {
    u8 found;
    s32 t;
    s32 next;
    s32 span;
    s32 elapsed;
    f32 frac;

    t = ((s32) D_803156C0) - D_8039CA84;
    if (D_80364A90 == 0x2000) {
        *x = D_8039CA68[0][0].x;
        *y = D_8039CA68[0][0].y;
        *z = D_8039CA68[0][0].z;
        *rx = D_8039CA68[0][0].rx;
        *ry = D_8039CA68[0][0].ry;
        *rz = D_8039CA68[0][0].rz;
        D_8039CA84 = ((s32) D_803156C0);
    } else {
        found = 0;
        while (D_8039CA78 < D_8039CA70[0].playCount - 2 && !found) {
            if (t >= D_8039CA68[0][D_8039CA78].time && t < D_8039CA68[0][D_8039CA78 + 1].time) {
                found = 1;
            } else {
                D_8039CA78++;
            }
        }
        found = 0;
        next = D_8039CA78 + 1;
        while (next < D_8039CA70[0].playCount - 2 && !found) {
            if (t <= D_8039CA68[0][next].time) {
                found = 1;
            } else {
                next++;
            }
        }
        span = D_8039CA68[0][next].time - D_8039CA68[0][D_8039CA78].time;
        if (next >= D_8039CA70[0].playCount - 2) {
            elapsed = D_8039CA68[0][next].time - D_8039CA68[0][D_8039CA78].time;
        } else {
            elapsed = t - D_8039CA68[0][D_8039CA78].time;
        }
        frac = (f32) elapsed / (f32) span;
        *x = (D_8039CA68[0][next].x - D_8039CA68[0][D_8039CA78].x) * frac + D_8039CA68[0][D_8039CA78].x;
        *y = (D_8039CA68[0][next].y - D_8039CA68[0][D_8039CA78].y) * frac + D_8039CA68[0][D_8039CA78].y;
        *z = (D_8039CA68[0][next].z - D_8039CA68[0][D_8039CA78].z) * frac + D_8039CA68[0][D_8039CA78].z;
        *rx = func_80295924(D_8039CA68[0][D_8039CA78].rx, D_8039CA68[0][next].rx, frac);
        *ry = func_80295924(D_8039CA68[0][D_8039CA78].ry, D_8039CA68[0][next].ry, frac);
        *rz = func_80295924(D_8039CA68[0][D_8039CA78].rz, D_8039CA68[0][next].rz, frac);
    }
}

s16 func_80295924(s16 from, s16 to, f32 t) {
    s16 d = to - from;

    if (d >= -0x800) {
        if (d > 0x800) {
            d -= 0xFFF;
        }
    } else {
        d += 0xFFF;
    }
    t *= d;
    t += from;
    if (t < 0.0) {
        t += 4095.0;
    }
    if (t > 4095.0) {
        t -= 4095.0;
    }
    return t;
}

void func_80295A20(u32 arg0) {
    u8 *dst;
    u8 *src;
    s32 i;

    if (D_8039CA88 >= arg0) {
        if (D_8039CA8C == 0) {
            D_8039CA7D = 1;
        } else {
            D_8039CA7D = 0;
        }
        dst = (u8 *) D_8039CA68[0];
        src = (u8 *) D_8039CA68[1];
        for (i = 0; i < 0x10000; i++) {
            dst[i] = src[i];
        }
        D_8039CA7C = D_803643D4;
        D_8039CA70[0].playCount = D_8039CA70[0].recCount;
        D_8039CA88 = arg0;
    }
}

void func_80295AE0(Gfx *gdl, Gfx *end) {
    s8 cmd;
    u32 shift;
    u8 found;
    s32 i;
    u32 mode;

    while (gdl != end) {
        cmd = gdl->words.w0 >> 24;
        switch ((s8) cmd) {
            case (s8) G_SETCOMBINE:
                {
                    Gfx *_g = (Gfx *) (gdl++);
                    _g->words.w0 = 0xFC119623;
                    _g->words.w1 = 0xFF2FFFFF;
                }
                break;
            case (s8) G_SETOTHERMODE_L:
                shift = (gdl->words.w0 >> 8) & 0xFFFF;
                if (shift == G_MDSFT_RENDERMODE) {
                    mode = gdl->words.w1;
                    i = 0;
                    found = 0;
                    do {
                        if (D_802FF11C[i].from == mode) {
                            found = 1;
                        } else {
                            i++;
                        }
                    } while (!found && i < 6);
                    if (!found) {
                        func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", "found", "ghostdigger.c", 338);
                    }
                    gdl->words.w1 = D_802FF11C[i].to;
                }
                gdl++;
                break;
            default:
                gdl++;
                break;
        }
    }
}

void func_80295C70(u8 id, s32 px, s32 pz) {
    s32 i;
    s32 j;

    for (i = 0; i < 1; i++) {
        if (D_802FF150[i].id == id) {
            if (func_802AC4C4(px, pz, D_802FF150[i].x[0], D_802FF150[i].z[0], D_802FF150[i].x[1],
                              D_802FF150[i].z[1], D_802FF150[i].x[2], D_802FF150[i].z[2]) ||
                func_802AC4C4(px, pz, D_802FF150[i].x[0], D_802FF150[i].z[0], D_802FF150[i].x[2],
                              D_802FF150[i].z[2], D_802FF150[i].x[3], D_802FF150[i].z[3])) {
                j = 0;
                while (D_802FF150[i].list[j] != -1) {
                    D_803C30A8[j] = D_802FF150[i].list[j];
                    j++;
                }
                D_803C30A8[j] = 0xFFFF;
            }
        }
    }
}
