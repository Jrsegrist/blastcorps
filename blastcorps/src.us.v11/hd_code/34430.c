#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* mb.c (this file's .bss starts at 0x8036CB60) */

/* KSEG0 address to physical, as the game spells it */
#define PHYS(x) ((u32) (x) & 0x1FFFFFFF)

#define MB_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "mb.c", line)
/* The file's second assert string has a bell */
#define MB_ASSERT_BELL(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "mb.c", line)

/* One recorded sample: three 16.16 values and a position in 1/32 units */
typedef struct {
    f32 a;
    f32 b;
    f32 c;
    f32 x;
    f32 y;
    f32 z;
} MbSample;

/* SetOtherMode_L values swapped when copying a display list */
typedef struct {
    u32 from;
    u32 to;
} MbRemap;

extern u8 *D_80358070;         /* heap pointer */
extern MbSample D_8036CB60[11]; /* ring of samples */
extern s32 D_8036CC68;         /* ring head */
extern s32 D_8036CC6C;         /* ring tail */
extern Mtx D_8036CC70[][10];   /* view matrices per slot */
extern u8 *D_8036D170;         /* 0x5460-byte buffer */
extern u8 D_8036D178;
extern s32 D_8036D180;
extern u8 D_80367C00;
extern MbRemap D_802FC060[3];
extern f32 D_8036D174;         /* view angle, degrees */
extern s32 D_8036D17C;
extern f32 D_8036D184;
extern Vtx D_802FBEE0[];       /* 6 x 4 grid of view points */
extern Gfx D_8036D188[40];     /* display list for the view */
extern Mtx D_8036D2C8;         /* its projection */
extern Mtx D_8036D388;         /* its look-at */
extern Vp D_802FBED0;
extern void *D_80358058;       /* depth buffer */

s32 func_802796D8(s32 n, s32 *a, s32 *b);
void func_8027A7DC(Gfx **gfxp, s32 offset, s32 v);
s32 func_8027B87C(f32 out[4][4], f32 in[4][4]);

/* Copies a display list to the heap, dropping G_ENDDLs and remapping render modes */
void func_80278BF0(Gfx *src, Gfx *end, Gfx **dstp) {
    Gfx *dst;
    s8 c;
    u32 v;
    u8 found;
    s32 i;
    u32 w1;

    *dstp = (Gfx *) D_80358070;
    dst = *dstp;
    D_80358070 += (end - src) * sizeof(Gfx) - 16;
    while (src != end) {
        switch (c = src->words.w0 >> 24) {
            case (s8) G_ENDDL:
                src++;
                break;
            case (s8) G_SETOTHERMODE_L:
                if ((v = (src->words.w0 >> 8) & 0xFFFF) == 3) {
                    dst->words.w0 = src->words.w0;
                    w1 = src->words.w1;
                    i = 0;
                    found = 0;
                    do {
                        if (D_802FC060[i].from == w1) {
                            found = 1;
                        } else {
                            i++;
                        }
                    } while (!found && i < 3);
                    MB_ASSERT(found, 157);
                    dst->words.w1 = D_802FC060[i].to;
                    dst++, src++;
                } else {
                    *dst++ = *src++;
                }
                break;
            default:
                *dst++ = *src++;
                break;
        }
    }
    gSPEndDisplayList(dst++);
}

/* Allocates the buffers and resets the state */
void func_80278E3C(void) {
    func_80257490(D_80358070, 0x40);
    D_8036D170 = D_80358070;
    func_80257490(D_80358070 = D_80358070 + 0x5460, 8);
    D_8036D178 = 0;
    D_8036CC68 = 0;
    D_8036CC6C = 0;
    D_8036D180 = 0;
    D_80367C00 = 0;
}

/* Sets up the view: count n (at most 10), scale (at most 1), and a view angle from the latest sample (25 degrees if none); fills a 6x4 grid of points 100 units out */
void func_80278EB0(s32 n, f32 scale, s32 arg2) {
    f32 mf1[4][4];
    f32 mf2[4][4];
    f32 p[3];
    f32 q[3];
    f32 ang;
    s32 i;
    s32 k = 0;
    f32 step;
    f32 yaw;
    u8 ok;
    s32 i0;
    s32 i1;
    s32 r;
    s32 pad;
    s32 v;

    if (n >= 11) {
        n = 10;
    }
    if (scale > 1.0f) {
        scale = 1.0f;
    }
    D_8036D17C = n;
    D_8036D184 = scale;
    D_8036D180 = 0;
    D_8036D178 = 1;
    ok = func_802796D8(1, &i0, &i1);
    if (ok) {
        r = func_8026A6F0(D_8036CB60[i0].a, D_8036CB60[i0].b, D_8036CB60[i0].c, D_8036CB60[i0].x,
                          D_8036CB60[i0].y, D_8036CB60[i0].z);
        if (r < 1.0) {
            r = 1;
        }
        v = func_802ACF3C((arg2 <<= 16) / r);
        D_8036D174 = (f32) v / 65536.0 * 360.0;
    } else {
        D_8036D174 = 25.0f;
    }
    ang = D_8036D174 / 2.0;
    step = D_8036D174 / 6.0;
    yaw = ang * 1.3333334f;
    for (i = 0; i < 6; i++) {
        guRotateF(mf1, ang, 1.0f, 0.0f, 0.0f);
        guMtxXFMF(mf1, 0.0f, 0.0f, -100.0f, &p[0], &p[1], &p[2]);
        guRotateF(mf2, yaw, 0.0f, 1.0f, 0.0f);
        guMtxXFMF(mf2, p[0], p[1], p[2], &q[0], &q[1], &q[2]);
        D_802FBEE0[k].v.ob[0] = q[0];
        D_802FBEE0[k].v.ob[1] = q[1];
        D_802FBEE0[k].v.ob[2] = q[2];
        guRotateF(mf2, -yaw, 0.0f, 1.0f, 0.0f);
        guMtxXFMF(mf2, p[0], p[1], p[2], &q[0], &q[1], &q[2]);
        D_802FBEE0[k + 3].v.ob[0] = q[0];
        D_802FBEE0[k + 3].v.ob[1] = q[1];
        D_802FBEE0[k + 3].v.ob[2] = q[2];
        guRotateF(mf1, ang - step, 1.0f, 0.0f, 0.0f);
        guMtxXFMF(mf1, 0.0f, 0.0f, -100.0f, &p[0], &p[1], &p[2]);
        guRotateF(mf2, yaw, 0.0f, 1.0f, 0.0f);
        guMtxXFMF(mf2, p[0], p[1], p[2], &q[0], &q[1], &q[2]);
        D_802FBEE0[k + 1].v.ob[0] = q[0];
        D_802FBEE0[k + 1].v.ob[1] = q[1];
        D_802FBEE0[k + 1].v.ob[2] = q[2];
        guRotateF(mf2, -yaw, 0.0f, 1.0f, 0.0f);
        guMtxXFMF(mf2, p[0], p[1], p[2], &q[0], &q[1], &q[2]);
        D_802FBEE0[k + 2].v.ob[0] = q[0];
        D_802FBEE0[k + 2].v.ob[1] = q[1];
        D_802FBEE0[k + 2].v.ob[2] = q[2];
        k += 4;
        ang -= step;
    }
}

void func_802794A4(void) {
    if (!D_80367C00) {
        if (D_8036D180 < 2) {
            D_8036D178 = 0;
        } else {
            D_8036D178 = 2;
        }
    }
}

void func_802794E4(void) {
    D_8036D178 = 0;
}

u8 func_802794F0(void) {
    s32 unused;

    return !(D_8036D178 == 0);
}

/* Pushes a sample onto the ring, dropping the oldest when full */
void func_80279514(s32 x, s32 y, s32 z, s32 a, s32 b, s32 c) {
    D_8036CB60[D_8036CC68].a = (f32) a / 65536.0;
    D_8036CB60[D_8036CC68].b = (f32) b / 65536.0;
    D_8036CB60[D_8036CC68].c = (f32) c / 65536.0;
    D_8036CB60[D_8036CC68].x = x / 32.0f;
    D_8036CB60[D_8036CC68].y = y / 32.0f;
    D_8036CB60[D_8036CC68].z = z / 32.0f;
    D_8036CC68++;
    if (D_8036CC68 == 11) {
        D_8036CC68 = 0;
    }
    if (D_8036CC68 == D_8036CC6C) {
        D_8036CC6C++;
        if (D_8036CC6C == 11) {
            D_8036CC6C = 0;
        }
    }
}

/* Fetches the ring indices n and n + 1 steps back from the head; 0 if not there */
s32 func_802796D8(s32 n, s32 *a, s32 *b) {
    s32 idx = D_8036CC68;

    while (n--) {
        if (idx) {
            idx--;
        } else {
            idx = 10;
        }
        if (idx == D_8036CC6C) {
            return 0;
        }
    }
    *a = idx;
    if (idx) {
        idx--;
    } else {
        idx = 10;
    }
    *b = idx;
    return 1;
}

/* Renders the 120x90 view into the buffer: clears it, sets the projection from the view angle and looks from (a, b, c) (16.16) at (x, y, z) (1/32), then calls dl */
void func_80279778(s32 x, s32 y, s32 z, s32 a, s32 b, s32 c, void *dl, void *seg6, void *seg7, s32 alpha) {
    Gfx *gdl;
    u16 perspNorm;
    s32 pad;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 ex;
    f32 ey;
    f32 ez;
    s32 pad2[8];
    f32 dx;
    f32 dz;

    if (D_8036D178) {
        gdl = D_8036D188;
        gSPViewport(gdl++, PHYS(&D_802FBED0));
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSegment(gdl++, 0, 0);
        gSPSegment(gdl++, 6, PHYS(seg6));
        gSPSegment(gdl++, 7, PHYS(seg7));
        gDPPipeSync(gdl++);
        gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, 120, 90);
        gDPSetColorDither(gdl++, G_CD_DISABLE);
        gDPSetCycleType(gdl++, G_CYC_FILL);
        gSPClearGeometryMode(gdl++, G_ZBUFFER);
        gDPSetDepthImage(gdl++, D_80358058);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 120, D_80358058);
        gDPSetFillColor(gdl++, 0xFFFCFFFC);
        gDPFillRectangle(gdl++, 0, 0, 119, 89);
        gDPPipeSync(gdl++);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 120, PHYS(D_8036D170));
        gDPSetFillColor(gdl++, 0);
        gDPFillRectangle(gdl++, 0, 0, 119, 89);
        gDPPipeSync(gdl++);
        guPerspective(&D_8036D2C8, &perspNorm, D_8036D174, 1.3333334f, 100.0f, 5000.0f, 1.0f);
        gSPMatrix(gdl++, PHYS(&D_8036D2C8), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gImmp1(gdl++, G_RDPHALF_1, perspNorm); /* old F3D gSPPerspNormalize */
        tx = x / 32.0f;
        ty = y / 32.0f;
        tz = z / 32.0f;
        ex = (f32) a / 65536.0;
        ey = (f32) b / 65536.0;
        ez = (f32) c / 65536.0;
        dx = ex - tx;
        if (dx < 0.0) {
            dx = 0.0 - dx;
        }
        dz = ez - tz;
        if (dz < 0.0) {
            dz = 0.0 - dz;
        }
        if (dx > 0.5 || dz > 0.5) {
            guLookAt(&D_8036D388, ex, ey, ez, tx, ty, tz, 0.0f, 1.0f, 0.0f);
        } else {
            guLookAt(&D_8036D388, ex + 2.0, ey, ez + 2.0, tx, ty, tz, 0.0f, 1.0f, 0.0f);
        }
        gSPMatrix(gdl++, PHYS(&D_8036D388), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPSetEnvColor(gdl++, 0, 0, 0, alpha);
        gSPDisplayList(gdl++, PHYS(dl));
        gSPEndDisplayList(gdl++);
        func_80284E54(D_8036D188, gdl - D_8036D188, 2, 0, 0x54D, 0);
    }
}

/* Draws up to D_8036D180 trailing copies of the view, fading out, along the recorded path */
void func_80279EE8(Gfx **gfxp, s32 arg1, u8 slot) {
    Gfx *gdl = *gfxp;
    u8 flag;
    s32 off = 0;
    s32 vi = 0;
    f32 mf[4][4];
    f32 inv[4][4];
    s32 i;
    u8 b;
    u8 a;
    s32 n = 1;
    s32 i0;
    s32 i1;
    f32 t = 0.0f;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 ea;
    f32 eb;
    f32 ec;
    f32 dx;
    f32 dz;

    switch (D_8036D178) {
        case 1:
            D_8036D180++;
            if (D_8036D180 == D_8036D17C) {
                D_8036D178 = 3;
            }
            break;
        case 2:
            D_8036D180--;
            if (D_8036D180 == 0) {
                D_8036D178 = 0;
            }
            break;
    }
    if (D_8036D178) {
        gDPPipeSync(gdl++);
        gDPSetColorDither(gdl++, G_CD_NOISE);
        gDPSetAlphaDither(gdl++, G_AD_NOTPATTERN);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombine(gdl++, 0x119623, 0xFF2FFFFF);
        gDPSetTextureFilter(gdl++, G_TF_BILERP);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        a = 30 / (D_8036D17C - 1);
        b = 40 - a * (D_8036D17C - D_8036D180);
        for (i = 0; i < D_8036D180; i++) {
            gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, b);
            flag = func_802796D8(n, &i0, &i1);
            if (!flag) {
                break;
            }
            tx = (D_8036CB60[i1].x - D_8036CB60[i0].x) * t + D_8036CB60[i0].x;
            ty = (D_8036CB60[i1].y - D_8036CB60[i0].y) * t + D_8036CB60[i0].y;
            tz = (D_8036CB60[i1].z - D_8036CB60[i0].z) * t + D_8036CB60[i0].z;
            ea = (D_8036CB60[i1].a - D_8036CB60[i0].a) * t + D_8036CB60[i0].a;
            eb = (D_8036CB60[i1].b - D_8036CB60[i0].b) * t + D_8036CB60[i0].b;
            ec = (D_8036CB60[i1].c - D_8036CB60[i0].c) * t + D_8036CB60[i0].c;
            t += D_8036D184;
            if (t >= 1.0) {
                n++;
                t = 0.0f;
            }
            dx = ea - tx;
            if (dx < 0.0) {
                dx = 0.0 - dx;
            }
            dz = ec - tz;
            if (dz < 0.0) {
                dz = 0.0 - dz;
            }
            if (dx > 0.5 || dz > 0.5) {
                guLookAtF(mf, ea, eb, ec, tx, ty, tz, 0.0f, 1.0f, 0.0f);
            } else {
                guLookAtF(mf, ea + 2.0, eb, ec + 2.0, tx, ty, tz, 0.0f, 1.0f, 0.0f);
            }
            flag = func_8027B87C(inv, mf);
            MB_ASSERT_BELL(flag, 547);
            guMtxF2L(inv, &D_8036CC70[slot][i]);
            gSPMatrix(gdl++, PHYS(&D_8036CC70[slot][i]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gSPVertex(gdl++, PHYS(D_802FBEE0), 16, 0);
            vi = 0;
            off = 0;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            gSPVertex(gdl++, PHYS(&D_802FBEE0[16]), 8, 0);
            vi = 0;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            func_8027A7DC(&gdl, off, vi);
            off += 0xE10, vi += 4;
            gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            b -= a;
        }
        gDPPipeSync(gdl++);
        gDPSetColorDither(gdl++, G_CD_MAGICSQ);
        gDPPipeSync(gdl++);
        *gfxp = gdl;
    }
}

/* Draws a 120x15 RGBA16 strip from the buffer at offset as two triangles from vertex v */
void func_8027A7DC(Gfx **gfxp, s32 offset, s32 v) {
    Gfx *gdl = *gfxp;
    s32 pad;

    gDPLoadTextureBlock(gdl++, D_8036D170 + offset - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 120, 15, 0, G_TX_CLAMP,
                        G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Triangle(gdl++, v, v + 1, v + 2, 0);
    gSP1Triangle(gdl++, v, v + 2, v + 3, 0);
    *gfxp = gdl;
}

/* Swaps rows of a 3x3 matrix so the axes come out in order */
void func_8027AA04(f32 m[][3], s32 a, s32 b, s32 c) {
    s32 i;
    s32 t;
    f32 tmp;

    if (a != 0) {
        if (b == 0) {
            for (i = 0; i < 3; i++) {
                tmp = m[b][i];
                m[b][i] = m[a][i];
                m[a][i] = tmp;
            }
            t = b;
            b = a;
            a = t;
        } else {
            for (i = 0; i < 3; i++) {
                tmp = m[c][i];
                m[c][i] = m[a][i];
                m[a][i] = tmp;
            }
            t = c;
            c = a;
            a = t;
        }
    }
    if (b != 1) {
        for (i = 0; i < 3; i++) {
            tmp = m[c][i];
            m[c][i] = m[b][i];
            m[b][i] = tmp;
        }
        t = c;
        c = b;
        b = t;
    }
}

/* Finishes the 3x3 inversion for columns 1 and 2 (pivot rows q and r), then reorders the rows; 0 if singular */
s32 func_8027AC00(f32 m[3][3], f32 out[3][3], s32 p) {
    s32 q;
    s32 r;
    s32 a;
    s32 b;
    s32 i;
    f32 s;
    f32 aa;
    f32 ab;

    if (p == 0) {
        a = 1, b = 2;
    } else if (p == 1) {
        a = 0, b = 2;
    } else {
        a = 0, b = 1;
    }
    if ((aa = m[a][1]) < 0.0f) {
        aa = -aa;
    }
    if ((ab = m[b][1]) < 0.0f) {
        ab = -ab;
    }
    if (aa > ab) {
        q = a;
    } else {
        q = b;
    }
    if (q == a) {
        r = b;
    } else {
        r = a;
    }
    if (m[q][1] < 1e-8 && m[q][1] > -1e-8) {
        return 0;
    }
    s = 1.0 / m[q][1];
    m[q][1] = 1.0f;
    m[q][2] *= s;
    out[q][q] = s;
    out[q][p] *= s;
    for (i = 0; i < 3; i++) {
        if (i != q) {
            s = -m[i][1];
            m[i][1] = 0.0f;
            m[i][2] += s * m[q][2];
            out[i][q] = out[q][q] * s;
            out[i][p] += s * out[q][p];
        }
    }
    if (m[r][2] < 1e-8 && m[r][2] > -1e-8) {
        return 0;
    }
    s = 1.0 / m[r][2];
    m[r][2] = 1.0f;
    out[r][r] = s;
    out[r][p] *= s;
    out[r][q] *= s;
    for (i = 0; i < 3; i++) {
        if (i != r) {
            s = -m[i][2];
            m[i][2] = 0.0f;
            out[i][p] += s * out[r][p];
            out[i][q] += s * out[r][q];
            out[i][r] += s * out[r][r];
        }
    }
    func_8027AA04(out, p, q, r);
    return 1;
}

/* Inverts a 3x3 matrix into out by elimination: pivots on the largest column-0 entry, then hands over to func_8027AC00; 0 if singular */
s32 func_8027B200(f32 m[3][3], f32 out[3][3]) {
    s32 i;
    s32 p;
    f32 s;
    f32 ax;
    f32 ay;
    f32 az;

    out[0][0] = out[1][1] = out[2][2] = 1.0f;
    out[0][1] = out[0][2] = out[1][0] = out[1][2] = out[2][0] = out[2][1] = 0.0f;
    if ((ax = m[0][0]) < 0.0f) {
        ax = -ax;
    }
    if ((ay = m[1][0]) < 0.0f) {
        ay = -ay;
    }
    if ((az = m[2][0]) < 0.0f) {
        az = -az;
    }
    if (ax > ay) {
        if (ax > az) {
            p = 0;
        } else {
            p = 2;
        }
    } else if (ay > az) {
        p = 1;
    } else {
        p = 2;
    }
    if (m[p][0] < 1e-8 && m[p][0] > -1e-8) {
        return 0;
    }
    s = 1.0 / m[p][0];
    m[p][0] = 1.0f;
    m[p][1] *= s;
    m[p][2] *= s;
    out[p][p] = s;
    for (i = 0; i < 3; i++) {
        if (i != p) {
            s = -m[i][0];
            m[i][0] = 0.0f;
            m[i][1] += s * m[p][1];
            m[i][2] += s * m[p][2];
            out[i][p] = out[p][p] * s;
        }
    }
    if (!func_8027AC00(m, out, p)) {
        return 0;
    }
    return 1;
}

/* Splits a 4x4 matrix into translation v, scale s and the rest (pivoting rows if m[3][3] is ~0) */
s32 func_8027B5D0(f32 m[4][4], f32 *v, f32 *s, s32 *row) {
    s32 i;
    s32 j;
    f32 tmp;
    f32 max;

    *row = -1;
    if (((m[3][3] > 0.0f) ? m[3][3] : -m[3][3]) < 1e-8) {
        max = 0.0f;
        for (i = 0; i < 4; i++) {
            if (m[i][3] > max) {
                *row = i;
                max = m[*row][3];
            } else if (m[i][3] < -max) {
                *row = i;
                max = -m[*row][3];
            }
        }
        if (*row < 0) {
            return 0;
        }
        for (j = 0; j < 4; j++) {
            tmp = m[3][j];
            m[3][j] = m[*row][j];
            m[*row][j] = tmp;
        }
    }
    v[0] = -m[0][3];
    v[1] = -m[1][3];
    v[2] = -m[2][3];
    *s = 1.0 / m[3][3];
    m[0][3] = m[1][3] = m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    m[3][0] *= *s;
    m[3][1] *= *s;
    m[3][2] *= *s;
    for (i = 0; i < 3; i++) {
        m[0][i] += v[0] * m[3][i];
        m[1][i] += v[1] * m[3][i];
        m[2][i] += v[2] * m[3][i];
    }
    return 1;
}

/* Inverts a 4x4 matrix (general or affine); 0 if singular */
s32 func_8027B87C(f32 out[4][4], f32 in[4][4]) {
    f32 m[4][4];
    s32 i;
    s32 j;
    s32 affine;
    f32 a[3][3];
    f32 inv[3][3];
    f32 s;
    f32 tmp;
    f32 v[4];
    f32 t[4];
    s32 row;
    s32 pad;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m[i][j] = in[i][j];
            out[i][j] = 0.0f;
        }
    }
    out[0][0] = 1.0f;
    out[1][1] = 1.0f;
    out[2][2] = 1.0f;
    out[3][3] = 1.0f;
    affine = m[0][3] == 0.0 && m[1][3] == 0.0 && m[2][3] == 0.0 && m[3][3] == 1.0;
    if (!affine) {
        if (!func_8027B5D0(m, v, &s, &row)) {
            return 0;
        }
    }
    t[0] = m[3][0];
    t[1] = m[3][1];
    t[2] = m[3][2];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            a[i][j] = m[i][j];
        }
    }
    if (!func_8027B200(a, inv)) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            out[i][j] = inv[i][j];
        }
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            out[3][i] -= t[j] * inv[j][i];
        }
    }
    if (!affine) {
        for (i = 0; i < 4; i++) {
            out[i][3] += v[0] * out[i][0] + v[1] * out[i][1] + v[2] * out[i][2];
            out[i][3] *= s;
        }
        if (row >= 0) {
            for (i = 0; i < 4; i++) {
                tmp = out[i][3];
                out[i][3] = out[i][row];
                out[i][row] = tmp;
            }
        }
    }
    return 1;
}
