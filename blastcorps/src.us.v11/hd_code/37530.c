#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_02000000 ((u8 *) D_02000000)
/* end of views */

/* A zone: an x/z shape (six values for func_802AC4C4) spanning ymin..ymax */
typedef struct {
    s16 a;
    s16 b;
    s16 c;
    s16 d;
    s16 e;
    s16 f;
    s16 ymin;
    s16 ymax;
} Zone;

/* Zones per level */
typedef struct {
    Zone * N64P start;
    Zone * N64P end;
    u8 level;
} ZoneList;

extern ZoneList D_802FC360[11];


/* One trail segment: a quad (4 corners x/y/z), two countdown timers, an
 * "open" flag and a style byte */
typedef struct {
    s16 v[12];
    u8 a;
    u8 b;
    u8 c;
    u8 d;
} Trail; /* 0x1C */

extern Trail D_8036D3D0[80]; /* ring, oldest at D_8036DC90, newest at D_8036DC91 */

/* The trail's vertex and display-list buffer (seen through segment 2) */
typedef struct TrailBuf {
    u8 pad[0x2000];
    Vtx vtx[451];
    Gfx dl[1];
} TrailBuf;

void func_8027D5AC(void);
void func_8027D350(s16 x0, s16 y0, s16 z0, s16 x1, s16 y1, s16 z1, Vtx *v, s32 i);

/* Is (x, y, z) inside one of the current level's zones? (zones span ymin..ymax; the x/z test is func_802AC4C4) */
/* K&R: the caller passes unconverted ints */
#ifdef NON_MATCHING
s32 func_8027BCF0(s16 x, s16 y, s16 z)
#else
s32 func_8027BCF0(x, y, z)
    s16 x;
    s16 y;
    s16 z;
#endif
{
    s32 i = 0;
    u8 found = 0;
    Zone *p;
    Zone *end;

    do {
        if (D_802FC360[i].level == D_802E8BDC) {
            found = 1;
        } else {
            i++;
        }
    } while (i < 11 && !found);
    if (!found) {
        return 0;
    }
    p = D_802FC360[i].start;
    end = D_802FC360[i].end;
    while (p != end) {
        if (y >= p->ymin && y <= p->ymax) {
            if (func_802AC4C4(x, z, p->a, p->b, p->c, p->d, p->e, p->f)) {
                return 1;
            }
        }
        p++;
    }
    return 0;
}

extern u8 D_8036DC90;
extern u8 D_8036DC91;
extern u8 D_8036DC92;
extern s32 D_8036DC94;

/* The dead mid-function `addiu sp` pair is an unused local (-O1 gives
 * every declared local a frame slot, even one never touched). */
void func_8027BE4C(void) {
    s32 unused;

    D_8036DC90 = 0;
    D_8036DC91 = 0;
    D_8036DC92 = 0;
    D_8036DC94 = -1;
}

/* Adds a trail segment at (x, y, z) (1/32 units), rotated by yaw; closes it every period calls or after a skipped frame */
void func_8027BE7C(u8 period, s32 y, s16 x1, s16 z1, s16 x2, s16 z2, s32 x, s32 z, s16 yaw, u8 halfw, u8 a,
                   u8 b, u8 d) {
    f32 mf[4][4];
    f32 p1x;
    f32 p1z;
    f32 p2x;
    f32 p2z;
    f32 py;
    f32 hx;
    f32 hz;
    s16 c1x;
    s16 c1z;
    s16 c2x;
    s16 c2z;

    if (func_8027BCF0(x >> 5, y >> 5, z >> 5)) {
        return;
    }
    guRotateF(mf, (f32) yaw / 4095.0 * 360.0, 0.0f, 1.0f, 0.0f);
    guMtxXFMF(mf, x1, 0.0f, z1, &p1x, &py, &p1z);
    guMtxXFMF(mf, x2, 0.0f, z2, &p2x, &py, &p2z);
    c1x = (s32) ((f32) x + p1x) >> 5;
    c1z = (s32) ((f32) z + p1z) >> 5;
    c2x = (s32) ((f32) x + p2x) >> 5;
    c2z = (s32) ((f32) z + p2z) >> 5;
    y >>= 5;
    guMtxXFMF(mf, halfw, 0.0f, 0.0f, &hx, &py, &hz);
    if (D_8036DC94 + 1 != ((s32) D_80358060) || D_8036DC94 == -1) {
        D_8036D3D0[D_8036DC91].c = 1;
        D_8036DC91++;
        if (D_8036DC91 == 80) {
            D_8036DC91 = 0;
        }
        if (D_8036DC91 == D_8036DC90) {
            D_8036DC90++;
            if (D_8036DC90 == 80) {
                D_8036DC90 = 0;
            }
        }
    }
    D_8036D3D0[D_8036DC91].d = d;
    D_8036D3D0[D_8036DC91].v[0] = c1x + (s16) hx;
    D_8036D3D0[D_8036DC91].v[1] = y;
    D_8036D3D0[D_8036DC91].v[2] = c1z + (s16) hz;
    D_8036D3D0[D_8036DC91].v[3] = c1x - (s16) hx;
    D_8036D3D0[D_8036DC91].v[4] = y;
    D_8036D3D0[D_8036DC91].v[5] = c1z - (s16) hz;
    D_8036D3D0[D_8036DC91].v[6] = c2x + (s16) hx;
    D_8036D3D0[D_8036DC91].v[7] = y;
    D_8036D3D0[D_8036DC91].v[8] = c2z + (s16) hz;
    D_8036D3D0[D_8036DC91].v[9] = c2x - (s16) hx;
    D_8036D3D0[D_8036DC91].v[10] = y;
    D_8036D3D0[D_8036DC91].v[11] = c2z - (s16) hz;
    D_8036D3D0[D_8036DC91].a = a;
    D_8036D3D0[D_8036DC91].b = b;
    D_8036DC92++;
    if (D_8036DC92 >= period) {
        D_8036DC92 = 0;
        D_8036D3D0[D_8036DC91].c = 0;
        D_8036DC91++;
        if (D_8036DC91 == 80) {
            D_8036DC91 = 0;
        }
        if (D_8036DC91 == D_8036DC90) {
            D_8036DC90++;
            if (D_8036DC90 == 80) {
                D_8036DC90 = 0;
            }
        }
    }
    D_8036DC94 = ((s32) D_80358060);
}

/* Draws the trail: each run of open segments becomes triangle strips in a sub display list, with a cull box for long runs */
void func_8027C4C8(Gfx **gfxp, TrailBuf *buf) {
    Gfx *gdl = *gfxp;
    u8 cur = D_8036DC90;
    u8 start;
    u8 end;
    u8 j;
    u8 done = 0;
    Gfx *dl = buf->dl;
    s32 dlIdx = 0;
    s32 vbase = 0;
    s32 k;
    s16 minX;
    s16 minY;
    s16 minZ;
    s16 maxX;
    s16 maxY;
    s16 maxZ;
    u8 t;
    u8 nv;
    s32 n;
    s32 m;
    u8 style;

    func_8027D5AC();
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, 0x5041C8, 0);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCombine(gdl++, 0xFFFFFF, 0xFFFDF8FC);
    gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, 0);
    while (!done) {
        start = cur;
        style = D_8036D3D0[cur].d;
        while (cur != D_8036DC91 && !D_8036D3D0[cur].c) {
            cur++;
            if (cur == 80) {
                cur = 0;
            }
        }
        end = cur;
        if (cur == D_8036DC91) {
            done = 1;
        }
        if (start != end) {
            gSPDisplayList(gdl++, dlIdx * sizeof(Gfx) + 0x3C30 + (u32) D_02000000);
            minZ = minY = minX = 0x7FFF;
            maxZ = maxY = maxX = -0x8000;
            k = vbase;
            j = start;
            while (j != end) {
                buf->vtx[k].v.ob[0] = D_8036D3D0[j].v[0];
                buf->vtx[k].v.ob[1] = D_8036D3D0[j].v[1];
                buf->vtx[k].v.ob[2] = D_8036D3D0[j].v[2];
                buf->vtx[k].v.cn[3] = D_8036D3D0[j].a;
                k++;
                buf->vtx[k].v.ob[0] = D_8036D3D0[j].v[3];
                buf->vtx[k].v.ob[1] = D_8036D3D0[j].v[4];
                buf->vtx[k].v.ob[2] = D_8036D3D0[j].v[5];
                buf->vtx[k].v.cn[3] = D_8036D3D0[j].a;
                k++;
                if (D_8036D3D0[j].v[0] < minX) {
                    minX = D_8036D3D0[j].v[0];
                }
                if (D_8036D3D0[j].v[1] < minY) {
                    minY = D_8036D3D0[j].v[1];
                }
                if (D_8036D3D0[j].v[2] < minZ) {
                    minZ = D_8036D3D0[j].v[2];
                }
                if (D_8036D3D0[j].v[0] > maxX) {
                    maxX = D_8036D3D0[j].v[0];
                }
                if (D_8036D3D0[j].v[1] > maxY) {
                    maxY = D_8036D3D0[j].v[1];
                }
                if (D_8036D3D0[j].v[2] > maxZ) {
                    maxZ = D_8036D3D0[j].v[2];
                }
                if (D_8036D3D0[j].v[3] < minX) {
                    minX = D_8036D3D0[j].v[3];
                }
                if (D_8036D3D0[j].v[4] < minY) {
                    minY = D_8036D3D0[j].v[4];
                }
                if (D_8036D3D0[j].v[5] < minZ) {
                    minZ = D_8036D3D0[j].v[5];
                }
                if (D_8036D3D0[j].v[3] > maxX) {
                    maxX = D_8036D3D0[j].v[3];
                }
                if (D_8036D3D0[j].v[4] > maxY) {
                    maxY = D_8036D3D0[j].v[4];
                }
                if (D_8036D3D0[j].v[5] > maxZ) {
                    maxZ = D_8036D3D0[j].v[5];
                }
                j++;
                if (j == 80) {
                    j = 0;
                }
            }
            if (!style) {
                j = start;
                while (j != end) {
                    buf->vtx[k].v.ob[0] = D_8036D3D0[j].v[6];
                    buf->vtx[k].v.ob[1] = D_8036D3D0[j].v[7];
                    buf->vtx[k].v.ob[2] = D_8036D3D0[j].v[8];
                    buf->vtx[k].v.cn[3] = D_8036D3D0[j].b;
                    k++;
                    buf->vtx[k].v.ob[0] = D_8036D3D0[j].v[9];
                    buf->vtx[k].v.ob[1] = D_8036D3D0[j].v[10];
                    buf->vtx[k].v.ob[2] = D_8036D3D0[j].v[11];
                    buf->vtx[k].v.cn[3] = D_8036D3D0[j].b;
                    k++;
                    /* (sic) both corners test v[6..8] for the minimum and v[9..11] for the maximum */
                    if (D_8036D3D0[j].v[6] < minX) {
                        minX = D_8036D3D0[j].v[6];
                    }
                    if (D_8036D3D0[j].v[7] < minY) {
                        minY = D_8036D3D0[j].v[7];
                    }
                    if (D_8036D3D0[j].v[8] < minZ) {
                        minZ = D_8036D3D0[j].v[8];
                    }
                    if (D_8036D3D0[j].v[9] > maxX) {
                        maxX = D_8036D3D0[j].v[9];
                    }
                    if (D_8036D3D0[j].v[10] > maxY) {
                        maxY = D_8036D3D0[j].v[10];
                    }
                    if (D_8036D3D0[j].v[11] > maxZ) {
                        maxZ = D_8036D3D0[j].v[11];
                    }
                    if (D_8036D3D0[j].v[6] < minX) {
                        minX = D_8036D3D0[j].v[6];
                    }
                    if (D_8036D3D0[j].v[7] < minY) {
                        minY = D_8036D3D0[j].v[7];
                    }
                    if (D_8036D3D0[j].v[8] < minZ) {
                        minZ = D_8036D3D0[j].v[8];
                    }
                    if (D_8036D3D0[j].v[9] > maxX) {
                        maxX = D_8036D3D0[j].v[9];
                    }
                    if (D_8036D3D0[j].v[10] > maxY) {
                        maxY = D_8036D3D0[j].v[10];
                    }
                    if (D_8036D3D0[j].v[11] > maxZ) {
                        maxZ = D_8036D3D0[j].v[11];
                    }
                    j++;
                    if (j == 80) {
                        j = 0;
                    }
                }
            }
            n = k - vbase;
            if (style) {
                m = 0;
            } else {
                m = n >> 1;
            }
            if (n >= 41) {
                func_8027D350(minX, minY, minZ, maxX, maxY, maxZ, buf->vtx, k);
                gSPVertex(dl++, k * sizeof(Vtx) + 0x2000 + (u32) D_02000000, 8, 0);
                gSPCullDisplayList(dl++, 0, 7);
                dlIdx += 2;
                k += 8;
            }
            if (n >= 3) {
                do {
                    if (n >= 17) {
                        nv = 16;
                    } else {
                        nv = n;
                    }
                    gSPVertex(dl++, vbase * sizeof(Vtx) + 0x2000 + (u32) D_02000000, nv, 0);
                    t = 0;
                    vbase += nv - 2;
                    dlIdx++;
                    for (; t < nv - 2;) {
                        if (m != 2) {
                            gSP1Triangle(dl++, t, t + 1, t + 2, 0);
                            gSP1Triangle(dl++, t + 1, t + 2, t + 3, 0);
                            dlIdx += 2;
                            m -= 2;
                            t += 2;
                        } else {
                            m = 0;
                            t += 4;
                        }
                    }
                    n = n - nv + 2;
                } while (n >= 3);
            }
            vbase = k;
            gSPEndDisplayList(dl++);
            dlIdx++;
        }
        cur++;
        if (cur == 80) {
            cur = 0;
        }
    }
    gDPPipeSync(gdl++);
    *gfxp = gdl;
}

/* Writes the 8 corners of the box (x0..x1, y0..y1, z0..z1) into v[i..i+7] */
void func_8027D350(s16 x0, s16 y0, s16 z0, s16 x1, s16 y1, s16 z1, Vtx *v, s32 i) {
    v[i].v.ob[0] = x0;
    v[i].v.ob[1] = y0;
    v[i].v.ob[2] = z0;
    i++;
    v[i].v.ob[0] = x0;
    v[i].v.ob[1] = y1;
    v[i].v.ob[2] = z0;
    i++;
    v[i].v.ob[0] = x1;
    v[i].v.ob[1] = y0;
    v[i].v.ob[2] = z0;
    i++;
    v[i].v.ob[0] = x1;
    v[i].v.ob[1] = y1;
    v[i].v.ob[2] = z0;
    i++;
    v[i].v.ob[0] = x0;
    v[i].v.ob[1] = y0;
    v[i].v.ob[2] = z1;
    i++;
    v[i].v.ob[0] = x0;
    v[i].v.ob[1] = y1;
    v[i].v.ob[2] = z1;
    i++;
    v[i].v.ob[0] = x1;
    v[i].v.ob[1] = y0;
    v[i].v.ob[2] = z1;
    i++;
    v[i].v.ob[0] = x1;
    v[i].v.ob[1] = y1;
    v[i].v.ob[2] = z1;
}

/* Once the 80-entry trail ring is nearly full (71+), counts down the oldest entries' two timers and drops them when both reach 0 */
void func_8027D5AC(void) {
    s32 n;
    u8 j;

    if (D_8036DC91 >= D_8036DC90) {
        n = D_8036DC91 - D_8036DC90;
    } else {
        n = D_8036DC91 - D_8036DC90 + 80;
    }
    if (n >= 71) {
        if (D_8036D3D0[D_8036DC90].a <= 0) {
            D_8036D3D0[D_8036DC90].a = 0;
        } else {
            D_8036D3D0[D_8036DC90].a--;
        }
        if (D_8036D3D0[D_8036DC90].b <= 0) {
            D_8036D3D0[D_8036DC90].b = 0;
        } else {
            D_8036D3D0[D_8036DC90].b--;
        }
        if (D_8036D3D0[D_8036DC90].a == 0 && D_8036D3D0[D_8036DC90].b == 0) {
            j = D_8036DC90 + 1;
            if (j == 80) {
                j = 0;
            }
            if (D_8036D3D0[j].a <= 0) {
                D_8036D3D0[j].a = 0;
            } else {
                D_8036D3D0[j].a--;
            }
            if (D_8036D3D0[j].b <= 0) {
                D_8036D3D0[j].b = 0;
            } else {
                D_8036D3D0[j].b--;
            }
            if (D_8036D3D0[j].a == 0 && D_8036D3D0[j].b == 0) {
                D_8036DC90 = j;
            }
        }
    }
}
