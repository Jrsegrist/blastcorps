#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/*
 * Billboard particle emitter: up to 50 textured quads spawned in bursts from
 * one point, flung out in a random cone, pulled down by gravity, bounced off a
 * floor height and animated through a per-state texture sequence. Textures are
 * streamed into a small cache on demand.
 */

typedef struct {
    /* 0x00 */ s16 *seqBirth; /* texture ids; [0] doubles as the frame count */
    /* 0x04 */ s16 *seqLoop;
    /* 0x08 */ s16 *seqDeath;
    /* 0x0C */ s32 size;
    /* 0x10 */ s16 speed;
    /* 0x12 */ s16 speedVar;
    /* 0x14 */ u8 life;
    /* 0x15 */ u8 lifeVar;
    /* 0x16 */ u8 texW;
    /* 0x17 */ u8 texH;
    /* 0x18 */ s8 gravity;
    /* 0x19 */ u8 count;  /* particles per burst */
    /* 0x1A */ u8 bursts;
    /* 0x1B */ u8 spread; /* max tilt from vertical, degrees */
    /* 0x1C */ u8 bounce; /* bounce damping divisor */
    /* 0x1D */ u8 r;
    /* 0x1E */ u8 g;
    /* 0x1F */ u8 b;
    /* 0x20 */ u8 a;
    /* 0x21 */ u8 fmt; /* G_IM_FMT_* */
    /* 0x22 */ u8 siz; /* G_IM_SIZ_* */
} ParticleDef;

typedef struct {
    /* 0x00 */ u8 active;
    /* 0x04 */ s32 pos[3];
    /* 0x10 */ s16 vel[3];
    /* 0x16 */ u8 state; /* 0 birth, 1 loop, 3 death */
    /* 0x17 */ u8 life;
    /* 0x18 */ s16 frame;
    /* 0x1A */ s16 fallTime;
    /* 0x1C */ s16 age;
} Particle; /* size 0x20 */

typedef struct {
    u8 key;  /* particle index */
    s32 val; /* texture address, sort key */
} SortEntry;

extern ParticleDef *D_802C4A20[];
extern Vtx D_802FDA80[4];
extern ParticleDef *D_8036EC30;
extern Particle D_8036EC38[50];
extern Mtx D_8036F278[][50];
extern s32 D_80370B78; /* emitter position */
extern s32 D_80370B7C;
extern s32 D_80370B80;
extern u8 D_80370B84; /* bursts emitted */
extern s32 D_80370B88; /* floor height */
extern u8 D_80370B8C;  /* emitter busy */
extern u8 D_80370B8D;  /* cooldown */
extern u8 *D_80370B90; /* texture cache */
extern s16 D_80370B98[];
extern s32 D_80370BB0; /* textures cached */
extern s32 D_80370BB4; /* bytes per texture */
extern u8 *D_80358070; /* heap pointer */

void func_80289EF4(Gfx **gdl);
u32 func_8028A0A0(s16 id);
void func_8028A1D0(SortEntry *a, s32 n);

/* Old SDK form of the 4-bit dxt helper: no MAX(1, ...) clamp. */
#undef TXL2WORDS_4b
#define TXL2WORDS_4b(txls) ((txls) / 16)

void func_80288220(void) {
    s32 i;

    for (i = 0; i < 50; i++) {
        D_8036EC38[i].active = 0;
    }
    D_80370B8C = 0;
    D_80370B8D = 0;
    D_80370B90 = D_80358070;
    D_80358070 += 0x1400;
}

s32 func_80288284(u8 type, s32 x, s32 y, s32 z, s32 floor) {
    s32 unused;
    s32 bpp;

    if (D_80370B8C == 0 && D_80370B8D == 0) {
        D_80370B78 = x;
        D_80370B7C = y;
        D_80370B80 = z;
        D_80370B88 = floor;
        D_8036EC30 = D_802C4A20[type];
        D_80370B84 = 0;
        D_802FDA80[0].v.ob[0] = -D_8036EC30->size;
        D_802FDA80[0].v.ob[1] = D_8036EC30->size;
        D_802FDA80[0].v.tc[0] = 0;
        D_802FDA80[0].v.tc[1] = 0;
        D_802FDA80[0].v.cn[0] = D_8036EC30->r;
        D_802FDA80[0].v.cn[1] = D_8036EC30->g;
        D_802FDA80[0].v.cn[2] = D_8036EC30->b;
        D_802FDA80[0].v.cn[3] = D_8036EC30->a;
        D_802FDA80[1].v.ob[0] = D_8036EC30->size;
        D_802FDA80[1].v.ob[1] = D_8036EC30->size;
        D_802FDA80[1].v.tc[0] = 0;
        D_802FDA80[1].v.tc[1] = D_8036EC30->texH << 5;
        D_802FDA80[1].v.cn[0] = D_8036EC30->r;
        D_802FDA80[1].v.cn[1] = D_8036EC30->g;
        D_802FDA80[1].v.cn[2] = D_8036EC30->b;
        D_802FDA80[1].v.cn[3] = D_8036EC30->a;
        D_802FDA80[2].v.ob[0] = D_8036EC30->size;
        D_802FDA80[2].v.ob[1] = -D_8036EC30->size;
        D_802FDA80[2].v.tc[0] = D_8036EC30->texW << 5;
        D_802FDA80[2].v.tc[1] = D_8036EC30->texH << 5;
        D_802FDA80[2].v.cn[0] = D_8036EC30->r;
        D_802FDA80[2].v.cn[1] = D_8036EC30->g;
        D_802FDA80[2].v.cn[2] = D_8036EC30->b;
        D_802FDA80[2].v.cn[3] = D_8036EC30->a;
        D_802FDA80[3].v.ob[0] = -D_8036EC30->size;
        D_802FDA80[3].v.ob[1] = -D_8036EC30->size;
        D_802FDA80[3].v.tc[0] = 0;
        D_802FDA80[3].v.tc[1] = D_8036EC30->texH << 5;
        D_802FDA80[3].v.cn[0] = D_8036EC30->r;
        D_802FDA80[3].v.cn[1] = D_8036EC30->g;
        D_802FDA80[3].v.cn[2] = D_8036EC30->b;
        D_802FDA80[3].v.cn[3] = D_8036EC30->a;
        switch (D_8036EC30->siz) {
            case 0:
                bpp = 4;
                break;
            case 1:
                bpp = 8;
                break;
            case 2:
                bpp = 16;
                break;
            case 3:
                bpp = 32;
                break;
        }
        D_80370BB4 = D_8036EC30->texW * D_8036EC30->texH * bpp / 8;
        D_80370BB0 = 0;
        D_80370B8C = 1;
        return 1;
    }
    return 0;
}

void func_802886A0(void) {
    s32 n;
    s32 slot;
    u8 full;
    u8 found;
    s16 dy;
    f32 mf[4][4];
    f32 x;
    f32 y;
    f32 z;
    f32 rx;
    f32 ry;
    f32 rz;
    s32 angle;
    s16 len;

    n = 0;
    slot = 0;
    full = 0;
    if (D_80370B8C == 0) {
        if (D_80370B8D != 0) {
            D_80370B8D--;
        }
    } else {
        D_80370B8C = 0;
        if (D_80370B84 < D_8036EC30->bursts) {
            while (!full && n < D_8036EC30->count) {
                found = 0;
                while (!found && slot < 50) {
                    if (D_8036EC38[slot].active == 0) {
                        found = 1;
                    } else {
                        slot++;
                    }
                }
                if (found) {
                    D_8036EC38[slot].active = 1;
                    D_8036EC38[slot].pos[0] = D_80370B78;
                    D_8036EC38[slot].pos[1] = D_80370B7C;
                    D_8036EC38[slot].pos[2] = D_80370B80;
                    angle = func_8026A828(0, D_8036EC30->spread);
                    guRotateF(mf, angle, 0.0f, 0.0f, 1.0f);
                    guMtxXFMF(mf, 0.0f,
                              (func_8026A828(-D_8036EC30->speedVar, D_8036EC30->speedVar) + D_8036EC30->speed) / 32.0f,
                              0.0f, &x, &y, &z);
                    angle = func_8026A828(0, 359);
                    guRotateF(mf, angle, 0.0f, 1.0f, 0.0f);
                    guMtxXFMF(mf, x, y, z, &rx, &ry, &rz);
                    D_8036EC38[slot].vel[0] = rx * 32.0f;
                    D_8036EC38[slot].vel[1] = ry * 32.0f;
                    D_8036EC38[slot].vel[2] = rz * 32.0f;
                    D_8036EC38[slot].state = 0;
                    D_8036EC38[slot].frame = 0;
                    D_8036EC38[slot].fallTime = 0;
                    D_8036EC38[slot].age = 0;
                    D_8036EC38[slot].life =
                        func_8026A828(-D_8036EC30->lifeVar, D_8036EC30->lifeVar) + D_8036EC30->life;
                    slot++;
                } else {
                    full = 1;
                }
                n++;
            }
            D_80370B84++;
        }
        for (n = 0; n < 50; n++) {
            if (D_8036EC38[n].active) {
                D_80370B8C = 1;
                switch (D_8036EC38[n].state) {
                    case 0:
                        len = *D_8036EC30->seqBirth;
                        if (D_8036EC38[n].frame == len) {
                            D_8036EC38[n].state = 1;
                            D_8036EC38[n].frame = 0;
                        }
                        break;
                    case 1:
                        len = *D_8036EC30->seqLoop;
                        if (D_8036EC38[n].frame == len) {
                            D_8036EC38[n].frame = 0;
                        }
                        break;
                    case 3:
                        len = *D_8036EC30->seqDeath;
                        if (D_8036EC38[n].frame == len) {
                            D_8036EC38[n].active = 0;
                        }
                        break;
                }
                if (D_8036EC38[n].age == D_8036EC38[n].life) {
                    D_8036EC38[n].state = 3;
                    D_8036EC38[n].frame = 0;
                }
            }
        }
        for (n = 0; n < 50; n++) {
            if (D_8036EC38[n].active) {
                D_8036EC38[n].pos[0] += D_8036EC38[n].vel[0];
                D_8036EC38[n].pos[2] += D_8036EC38[n].vel[2];
                dy = D_8036EC38[n].vel[1] + D_8036EC38[n].fallTime * D_8036EC30->gravity;
                D_8036EC38[n].pos[1] += dy;
                if (D_8036EC38[n].pos[1] < D_80370B88) {
                    D_8036EC38[n].pos[1] = D_80370B88;
                    D_8036EC38[n].vel[1] = -(dy * 16 / D_8036EC30->bounce);
                    D_8036EC38[n].fallTime = 0;
                }
            }
            D_8036EC38[n].fallTime++;
            D_8036EC38[n].frame++;
            D_8036EC38[n].age++;
        }
        if (D_80370B8C == 0) {
            D_80370B8D = 3;
        }
    }
}

void func_80288DF0(Gfx **gdl, u8 buf) {
    s32 n;
    s32 i;
    Gfx *gfx;
    f32 mf1[4][4];
    f32 mf2[4][4];
    s32 dist;
    s32 dy;
    s32 ang;
    s32 ratio;
    s32 absRatio;
    s32 texId;
    SortEntry entries[50];
    s32 count;
    s32 lastTex;

    gfx = *gdl;
    count = 0;
    lastTex = -1;
    if (D_80370B8C) {
        func_80289EF4(&gfx);
        for (n = 0; n < 50; n++) {
            if (D_8036EC38[n].active) {
                switch (D_8036EC38[n].state) {
                    case 0:
                        texId = D_8036EC30->seqBirth[D_8036EC38[n].frame];
                        break;
                    case 1:
                        texId = D_8036EC30->seqLoop[D_8036EC38[n].frame];
                        break;
                    case 3:
                        texId = D_8036EC30->seqDeath[D_8036EC38[n].frame];
                        break;
                }
                entries[count].key = n;
                entries[count].val = func_8028A0A0(texId);
                count++;
            }
        }
        /* sort by texture so each one is loaded once */
        func_8028A1D0(entries, count);
        for (i = 0; i < count; i++) {
            n = entries[i].key;
            if (entries[i].val != lastTex) {
                lastTex = entries[i].val;
                gDPPipeSync(gfx++);
                switch (D_8036EC30->siz) {
                    case 0:
                        gDPLoadTextureBlock_4b(gfx++, OS_PHYSICAL_TO_K0(lastTex), D_8036EC30->fmt, D_8036EC30->texW,
                                               D_8036EC30->texH, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK,
                                               G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        break;
                    case 1:
                        gDPLoadTextureBlock(gfx++, OS_PHYSICAL_TO_K0(lastTex), D_8036EC30->fmt, G_IM_SIZ_8b,
                                            D_8036EC30->texW, D_8036EC30->texH, 0, G_TX_CLAMP, G_TX_CLAMP,
                                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        break;
                    case 2:
                        gDPLoadTextureBlock(gfx++, OS_PHYSICAL_TO_K0(lastTex), D_8036EC30->fmt, G_IM_SIZ_16b,
                                            D_8036EC30->texW, D_8036EC30->texH, 0, G_TX_CLAMP, G_TX_CLAMP,
                                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        break;
                    case 3:
                        gDPLoadTextureBlock(gfx++, OS_PHYSICAL_TO_K0(lastTex), D_8036EC30->fmt, G_IM_SIZ_32b,
                                            D_8036EC30->texW, D_8036EC30->texH, 0, G_TX_CLAMP, G_TX_CLAMP,
                                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        break;
                }
            }
            /* tilt the billboard toward the camera's height, then face its yaw */
            dist = func_8026A6F0(D_803643F8 >> 11, D_803643FC >> 11, D_80364400 >> 11, D_8036EC38[n].pos[0],
                                 D_8036EC38[n].pos[1], D_8036EC38[n].pos[2]);
            if (dist == 0) {
                dist = 1;
            }
            dy = (D_803643FC >> 11) - D_8036EC38[n].pos[1];
            dy <<= 16;
            ratio = dy / dist;
            if (ratio < 0) {
                absRatio = -ratio;
            } else {
                absRatio = ratio;
            }
            ang = func_802AD7D4(absRatio);
            if (ratio > 0) {
                ang = -ang;
            }
            guRotateF(mf1, (f32) ang / 65536.0 * 360.0, 1.0f, 0.0f, 0.0f);
            guRotateF(mf2, (f32) D_80364452 / 4095.0 * 360.0, 0.0f, 1.0f, 0.0f);
            guMtxCatF(mf1, mf2, mf1);
            guTranslateF(mf2, D_8036EC38[n].pos[0] / 32.0f, D_8036EC38[n].pos[1] / 32.0f,
                         D_8036EC38[n].pos[2] / 32.0f);
            guMtxCatF(mf1, mf2, mf1);
            guMtxF2L(mf1, &D_8036F278[buf][n]);
            gSPMatrix(gfx++, osVirtualToPhysical(&D_8036F278[buf][n]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gSPVertex(gfx++, osVirtualToPhysical(D_802FDA80), 4, 0);
            gSP1Triangle(gfx++, 0, 1, 2, 0);
            gSP1Triangle(gfx++, 0, 2, 3, 0);
            gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
        }
        *gdl = gfx;
    }
}

void func_80289EF4(Gfx **gdl) {
    Gfx *gfx = *gdl;

    gDPPipeSync(gfx++);
    gSPClearGeometryMode(gfx++, -1);
    gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gDPSetRenderMode(gfx++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
    switch (D_8036EC30->fmt) {
        case G_IM_FMT_RGBA:
            gDPSetCombineMode(gfx++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
            break;
        case G_IM_FMT_IA:
            gDPSetCombineMode(gfx++, G_CC_MODULATEIA, G_CC_MODULATEIA);
            break;
        case G_IM_FMT_I:
            gDPSetCombineMode(gfx++, G_CC_MODULATEI, G_CC_MODULATEI);
            break;
    }
    gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    *gdl = gfx;
}

/* Returns the physical address of texture `id`, loading it into the cache if needed. */
u32 func_8028A0A0(s16 id) {
    u8 found;
    s32 i;
    u8 *p;

    found = 0;
    i = 0;
    while (!found && i < D_80370BB0) {
        if (D_80370B98[i] == id) {
            found = 1;
        } else {
            i++;
        }
    }
    if (found) {
        return osVirtualToPhysical(i * D_80370BB4 + D_80370B90);
    }
    p = D_80370BB0 * D_80370BB4 + D_80370B90;
    D_80370B98[D_80370BB0] = id;
    D_80370BB0++;
    func_802A1040(id, p, 0);
    return osVirtualToPhysical(p);
}

/* Shell sort (Knuth gaps) by val, ascending. */
void func_8028A1D0(SortEntry *a, s32 n) {
    s32 i;
    s32 j;
    s32 h;
    SortEntry tmp;

    for (h = 1; h <= n / 9; h = h * 3 + 1) {
    }
    for (; h > 0; h /= 3) {
        for (i = h; i < n; i++) {
            tmp.key = a[i].key;
            tmp.val = a[i].val;
            j = i;
            while (j >= h && a[j - h].val > tmp.val) {
                a[j].key = a[j - h].key;
                a[j].val = a[j - h].val;
                j -= h;
            }
            a[j].key = tmp.key;
            a[j].val = tmp.val;
        }
    }
}
