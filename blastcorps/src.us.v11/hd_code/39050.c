#include "common.h"
#include <ultra64.h>

#define ABS(x) ((x) >= 0.0f ? (x) : -(x))

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

/* Animated water surface, one entry per level that has one (0x34 bytes). The surface is an
 * (nx + 1) x (nz + 1) vertex grid over x0..x1, z0..z1, displaced by two sine waves. */
typedef struct {
    u8 id;     /* level id */
    u8 nx;     /* 0x01: grid cells along x */
    u8 nz;     /* 0x02: grid cells along z */
    u8 pad3;
    s16 x0;    /* 0x04 */
    s16 z0;    /* 0x06 */
    s16 x1;    /* 0x08 */
    s16 z1;    /* 0x0A */
    s16 y;     /* 0x0C: rest height */
    s16 ampX;  /* 0x0E */
    s16 ampZ;  /* 0x10 */
    s16 pad12;
    f32 rateX; /* 0x14 */
    f32 rateZ; /* 0x18 */
    f32 lenX;  /* 0x1C */
    f32 lenZ;  /* 0x20 */
    u8 unk24;  /* 0x24: nonzero = single texture (tex), else three (D_802FC48C) */
    u8 pad25;
    s16 tex;   /* 0x26 */
    s32 rm0;   /* 0x28: render mode, two-texture case */
    s32 rm1;   /* 0x2C */
    u8 zbuf;   /* 0x30: use G_ZBUFFER, two-texture case */
    u8 unk31;  /* 0x31: draw layer */
    u8 pad32[2];
} Water;

extern u8 D_802E8BD0;
extern Water D_802FC3F0[];
extern s16 D_802FC48C[];
extern s32 D_80358070; /* bump allocator for display memory */
extern Vtx *D_8036DCA0[2]; /* double-buffered water grid */
extern s32 D_8036DCA8[2];
extern s32 D_8036DCB0; /* wave clock */
extern s32 D_8036DCB8[3];
extern s32 D_8036DCC8[2];
extern u8 D_8036DCD0;
extern s16 D_8036DCD2;
extern u8 D_8036DCD4; /* level has water */
extern u8 D_8036DCD5;
extern u8 D_8036DCD6; /* index into D_802FC3F0 */
extern u8 D_8036DCD7;

void func_802802D4(Vtx *v, s32 i0, s32 i1, s32 i2);
void func_8028072C(Vtx *v, s16 x0, s16 y0, s16 z0, s16 x1, s16 y1, s16 z1);

/* Per-level random quad spawner (0x22 bytes); ranges are passed to func_8026A828 (random in range). */
typedef struct {
    u8 id; /* level id */
    u8 pad1;
    s16 x0, z0; /* 0x02 */
    s16 x1, z1; /* 0x06 */
    s16 y0, y1; /* 0x0A */
    u8 count;   /* 0x0E */
    u8 padF;
    s16 r0, r1; /* 0x10 */
    s16 tex;    /* 0x14 */
    u8 tu, tv;  /* 0x16 */
    s16 w0, w1; /* 0x18 */
    s16 d0, d1; /* 0x1C */
    u8 alpha;   /* 0x20 */
    u8 pad21;
} QuadSpawn;

typedef struct {
    s16 unk0;
    s16 x, z;
    s16 w, d; /* half extents */
    s16 angle;
} Quad;

extern QuadSpawn D_802FC494[];
extern Quad D_8036DCE0[];
extern Vtx D_8036DD70[2][12][4];
extern u8 D_8036E370;
extern s32 D_8036E374;
extern s32 D_8036E378;

s16 func_8026A828(s16, s16);
void func_802CE65C(s32, s32, s16, s16);
extern s32 D_803F9320;
extern s32 D_803F9324;

/* Level spawn table for the proximity objects below (one entry). */
typedef struct {
    u8 id; /* level id */
    u8 pad1;
    s16 x, y, z;
} MineSpawn;

/* A placed proximity object (0x50 bytes). */
typedef struct {
    s16 x, y, z;
    s16 pad6;
    Vtx v[4];  /* 0x08 */
    u8 active; /* 0x48 */
} Mine;

extern MineSpawn D_802FC520[];
extern Vtx D_802FC528[];
extern Mine D_8036E380[];
extern s32 D_8036E4C0; /* number of placed objects */
extern s32 D_8036E4C4;
extern s16 D_8036E4C8;
extern s8 D_8036E4CA;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern void *D_80367738;
extern s32 D_8036DCD8;

void func_8026A5CC(void *arg0, void *arg1, s32 arg2);
s32 func_802A0CC8(s32, s32);
s32 func_8026A6F0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void *func_80260650(void *arg0, s16 arg1, void *arg2);

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

/* Where segment p[0]..p[1] (x) / q[0]..q[1] (y) crosses the segment centred on (arg0, arg1)
 * with half-extent (p[1] - p[2], q[1] - q[2]); returns the crossing's fraction along the
 * first segment. Line-intersection maths as in Graphics Gems II (lines_intersect).
 * The a1/b1/c1 line must stay a single source line: IDO schedules it differently otherwise. */
f32 func_8027DD88(s32 arg0, s32 arg1, s32 *p, s32 *q) {
    f32 a1, a2, b1, b2, c1, c2;
    f32 r1, r2, r3, r4;
    f32 denom, offset, num;
    f32 pos;
    f32 t;
    f32 x1, y1, x2, y2, x3, y3, x4, y4;

    x1 = p[0];
    y1 = q[0];
    x2 = p[1];
    y2 = q[1];
    x3 = p[1] - p[2] + arg0;
    y3 = q[1] - q[2] + arg1;
    x4 = arg0 - (p[1] - p[2]);
    y4 = arg1 - (q[1] - q[2]);

    a1 = y2 - y1; b1 = x1 - x2; c1 = x2 * y1 - x1 * y2;
    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;
    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;
    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;
    denom = a1 * b2 - a2 * b1;
    if (denom < 0.0f) {
        offset = -denom / 2.0f;
    } else {
        offset = denom / 2.0f;
    }
    if (ABS(x2 - x1) > ABS(y2 - y1)) {
        num = b1 * c2 - b2 * c1;
        pos = ((num < 0.0f) ? num - offset : num + offset) / denom;
        t = (pos - x1) / (x2 - x1);
    } else {
        num = a2 * c1 - a1 * c2;
        pos = ((num < 0.0f) ? num - offset : num + offset) / denom;
        t = (pos - y1) / (y2 - y1);
    }
    return t;
}

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

/* Set up the water surface for level id: allocate its vertex/display buffers, load its textures,
 * and lay out the grid's x/z positions and texture coordinates. */
void func_8027E344(s32 id) {
    s32 stepX;
    s32 stepZ;
    s32 x;
    s32 z;
    s32 i;
    s32 j;
    s32 n;
    s32 tv;
    s32 tu;
    s32 size;
    u8 found;

    n = 0;
    tv = 0;
    tu = 0;
    found = 0;
    D_8036DCD6 = 0;
    do {
        if (D_802FC3F0[D_8036DCD6].id == id) {
            found = 1;
        } else {
            D_8036DCD6++;
        }
    } while (!found && D_8036DCD6 < 3);
    if (!found) {
        D_8036DCD4 = 0;
        return;
    }
    D_8036DCD4 = 1;
    D_8036DCA0[0] = (Vtx *) D_80358070;
    D_80358070 += (D_802FC3F0[D_8036DCD6].nx + 1) * (D_802FC3F0[D_8036DCD6].nz + 1) * sizeof(Vtx);
    D_8036DCA0[1] = (Vtx *) D_80358070;
    D_80358070 += (D_802FC3F0[D_8036DCD6].nx + 1) * (D_802FC3F0[D_8036DCD6].nz + 1) * sizeof(Vtx);
    D_8036DCA8[0] = D_80358070;
    D_80358070 += 0x12C0;
    D_8036DCA8[1] = D_80358070;
    D_80358070 += 0x12C0;
    size = D_802FC3F0[D_8036DCD6].nx * D_802FC3F0[D_8036DCD6].nz * 2 * 8;
    size += (D_802FC3F0[D_8036DCD6].nx / 8 + 1) * D_802FC3F0[D_8036DCD6].nz * 8;
    size += 0x1C20;
    D_8036DCC8[0] = D_80358070;
    D_80358070 += size;
    D_8036DCC8[1] = D_80358070;
    D_80358070 += size;
    D_8036DCD7 = D_802FC3F0[D_8036DCD6].unk31;
    if ((D_8036DCD5 = D_802FC3F0[D_8036DCD6].unk24) != 0) {
        D_8036DCB8[0] = D_80358070;
        func_802A0CC8(D_802FC3F0[D_8036DCD6].tex, 0);
    } else {
        for (i = 0; i < 3; i++) {
            D_8036DCB8[i] = D_80358070;
            func_802A0CC8(D_802FC48C[i], 0);
        }
    }
    D_8036DCD0 = 0;
    D_8036DCD2 = 0;
    stepX = (D_802FC3F0[D_8036DCD6].x1 - D_802FC3F0[D_8036DCD6].x0) / D_802FC3F0[D_8036DCD6].nx;
    stepZ = (D_802FC3F0[D_8036DCD6].z1 - D_802FC3F0[D_8036DCD6].z0) / D_802FC3F0[D_8036DCD6].nz;
    x = D_802FC3F0[D_8036DCD6].x0;
    z = D_802FC3F0[D_8036DCD6].z0;
    for (i = 0; i <= D_802FC3F0[D_8036DCD6].nz; i++) {
        tu = 0;
        for (j = 0; j <= D_802FC3F0[D_8036DCD6].nx; j++) {
            D_8036DCA0[0][n].v.ob[0] = x;
            D_8036DCA0[0][n].v.ob[2] = z;
            D_8036DCA0[1][n].v.ob[0] = x;
            D_8036DCA0[1][n].v.ob[2] = z;
            D_8036DCA0[0][n].v.tc[0] = tv << 5;
            D_8036DCA0[0][n].v.tc[1] = tu << 5;
            D_8036DCA0[1][n].v.tc[0] = tv << 5;
            D_8036DCA0[1][n].v.tc[1] = tu << 5;
            tu ^= 0x1F;
            n++;
            x += stepX;
        }
        tv ^= 0x1F;
        x = D_802FC3F0[D_8036DCD6].x0;
        z += stepZ;
    }
    D_8036DCB0 = 0;
}

/* Per-frame water update: advance the wave clock, then rebuild the heights of vertex buffer buf. */
void func_8027E9B8(u8 buf) {
    s32 i;
    s32 j;
    s32 stepX;
    s32 stepZ;
    register f32 s;

    if (D_8036DCD4 != 0 && D_802E8BD0 == 0) {
        D_8036DCB0++;
        D_8036DCD2 += 15;
        if (D_8036DCD2 >= 255) {
            D_8036DCD2 = 0;
            if (++D_8036DCD0 >= 3) {
                D_8036DCD0 = 0;
            }
        }
    }
    if (D_8036DCD4 != 0) {
        stepX = (D_802FC3F0[D_8036DCD6].x1 - D_802FC3F0[D_8036DCD6].x0) / D_802FC3F0[D_8036DCD6].nx;
        for (i = 0; i <= D_802FC3F0[D_8036DCD6].nx; i++) {
            for (j = 0; j <= D_802FC3F0[D_8036DCD6].nz; j++) {
                s = sinf(((i + 1) * stepX) / D_802FC3F0[D_8036DCD6].lenX + D_8036DCB0 / D_802FC3F0[D_8036DCD6].rateX);
                D_8036DCA0[buf][i + j * (D_802FC3F0[D_8036DCD6].nz + 1)].v.ob[1] =
                    D_802FC3F0[D_8036DCD6].ampX * s + D_802FC3F0[D_8036DCD6].y;
            }
        }
        stepZ = (D_802FC3F0[D_8036DCD6].z1 - D_802FC3F0[D_8036DCD6].z0) / D_802FC3F0[D_8036DCD6].nz;
        for (i = 0; i <= D_802FC3F0[D_8036DCD6].nz; i++) {
            for (j = 0; j <= D_802FC3F0[D_8036DCD6].nx; j++) {
                s = sinf(((i + 1) * stepZ) / D_802FC3F0[D_8036DCD6].lenZ + D_8036DCB0 / D_802FC3F0[D_8036DCD6].rateZ);
                D_8036DCA0[buf][(D_802FC3F0[D_8036DCD6].nz + 1) * i + j].v.ob[1] =
                    D_802FC3F0[D_8036DCD6].ampZ * s + D_8036DCA0[buf][(D_802FC3F0[D_8036DCD6].nz + 1) * i + j].v.ob[1];
            }
        }
    }
}

/* Water height at (x, z) into *y; returns 1 if (x, z) is over the water grid. */
s32 func_8027EED8(s16 x, s16 z, s16 *y) {
    s32 pad;
    f32 step;
    f32 t;
    register f32 s;

    if (D_8036DCD4 == 0) {
        return 0;
    }
    if (x < D_802FC3F0[D_8036DCD6].x0 || x > D_802FC3F0[D_8036DCD6].x1 || z < D_802FC3F0[D_8036DCD6].z0 ||
        z > D_802FC3F0[D_8036DCD6].z1) {
        *y = D_802FC3F0[D_8036DCD6].y;
        return 0;
    }
    step = (f32) (D_802FC3F0[D_8036DCD6].x1 - D_802FC3F0[D_8036DCD6].x0) / D_802FC3F0[D_8036DCD6].nx;
    t = (x - D_802FC3F0[D_8036DCD6].x0) / step;
    s = sinf(((t + 1.0f) * step) / D_802FC3F0[D_8036DCD6].lenX + D_8036DCB0 / D_802FC3F0[D_8036DCD6].rateX);
    *y = D_802FC3F0[D_8036DCD6].ampX * s + D_802FC3F0[D_8036DCD6].y;
    step = (f32) (D_802FC3F0[D_8036DCD6].z1 - D_802FC3F0[D_8036DCD6].z0) / D_802FC3F0[D_8036DCD6].nz;
    t = (z - D_802FC3F0[D_8036DCD6].z0) / step;
    s = sinf(((t + 1.0f) * step) / D_802FC3F0[D_8036DCD6].lenZ + D_8036DCB0 / D_802FC3F0[D_8036DCD6].rateZ);
    *y = D_802FC3F0[D_8036DCD6].ampZ * s + *y;
    return 1;
}

/* Draw the water surface into gfx when this is its layer. The grid is emitted in strips of up to
 * 8x2 vertices into a sub-display-list (D_8036DCC8[buf]); every 32 vertices the strip batch is
 * closed and given a bounding box (8 verts in D_8036DCA8[buf]) for gSPCullDisplayList. With a
 * single texture the face normals are recomputed for environment mapping, otherwise two textures
 * are blended with LOD fraction D_8036DCD2. */
void func_8027F1F8(Gfx **gfx, u8 buf, u8 layer) {
    Gfx *gdl;
    s32 a;
    s32 b;
    s32 pad; /* unused */
    s32 total;
    s32 cnt;
    s32 col;
    s32 k;
    u8 done;
    s32 m;
    s32 rowlen;
    u8 next;
    s16 minX;
    s16 maxX;
    s16 minY;
    s16 maxY;
    s16 minZ;
    s16 maxZ;
    Gfx *dl;
    Vtx *bbox;
    s32 vcount;

    gdl = *gfx;
    done = 0;
    dl = (Gfx *) D_8036DCC8[buf];
    bbox = (Vtx *) D_8036DCA8[buf];
    if (D_8036DCD4 == 0 || layer != D_8036DCD7) {
        return;
    }
    gDPPipeSync(gdl++);
    if (D_8036DCD5 != 0) {
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LIGHTING | G_TEXTURE_GEN);
        gSPTexture(gdl++, 0x07C0, 0x07C0, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombine(gdl++, 0xFFFFFF, 0xFFFCF67B);
        gDPSetPrimColor(gdl++, 0, 0, 0x00, 0x00, 0x00, 0xAA);
        gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036DCB8[0]), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                            G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    } else {
        gDPSetCycleType(gdl++, G_CYC_2CYCLE);
        gDPSetRenderMode(gdl++, D_802FC3F0[D_8036DCD6].rm0, D_802FC3F0[D_8036DCD6].rm1);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        if (D_802FC3F0[D_8036DCD6].zbuf != 0) {
            gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        } else {
            gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        }
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombine(gdl++, 0x277FFF, 0x1FFCFE3B);
        gDPSetTextureLOD(gdl++, G_TL_TILE);
        if (D_8036DCD0 == 2) {
            next = 0;
        } else {
            next = D_8036DCD0 + 1;
        }
        gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(D_8036DCB8[D_8036DCD0]));
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x000, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPLoadSync(gdl++);
        gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 1023, 256);
        gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, OS_K0_TO_PHYSICAL(D_8036DCB8[next]));
        gDPTileSync(gdl++);
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPLoadSync(gdl++);
        gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 1023, 256);
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x000, G_TX_RENDERTILE, 0, G_TX_CLAMP, 5, G_TX_NOLOD,
                   G_TX_CLAMP, 5, G_TX_NOLOD);
        gDPSetTileSize(gdl++, G_TX_RENDERTILE, 2, 2, 0x7E, 0x7E);
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x100, 1, 0, G_TX_CLAMP, 5, G_TX_NOLOD, G_TX_CLAMP, 5,
                   G_TX_NOLOD);
        gDPSetTileSize(gdl++, 1, 2, 2, 0x7E, 0x7E);
        gDPSetPrimColor(gdl++, 0, D_8036DCD2, 0x00, 0x00, 0x00, 0xAA);
    }
    total = (D_802FC3F0[D_8036DCD6].nx + 1) * (D_802FC3F0[D_8036DCD6].nz + 1);
    a = 0;
    b = D_802FC3F0[D_8036DCD6].nx + 1;
    col = 0;
    vcount = 0;
    while (!done) {
        if (D_802FC3F0[D_8036DCD6].nx - col + 1 >= 9) {
            cnt = 8;
        } else {
            cnt = D_802FC3F0[D_8036DCD6].nx - col + 1;
        }
        if (vcount == 0) {
            gSPDisplayList(gdl++, osVirtualToPhysical(dl));
            gSPVertex(dl++, osVirtualToPhysical(bbox), 8, 0);
            gSPCullDisplayList(dl++, 0, 7);
            minZ = minY = minX = 0x7FFF;
            maxZ = maxY = maxX = -0x8000;
        }
        gSPVertex(dl++, osVirtualToPhysical(&D_8036DCA0[buf][a]), cnt, 0);
        gSPVertex(dl++, osVirtualToPhysical(&D_8036DCA0[buf][b]), cnt, 8);
        for (m = 0; m < cnt; m++) {
            if (D_8036DCA0[buf][a + m].v.ob[0] < minX) {
                minX = D_8036DCA0[buf][a + m].v.ob[0];
            }
            if (D_8036DCA0[buf][a + m].v.ob[1] < minY) {
                minY = D_8036DCA0[buf][a + m].v.ob[1];
            }
            if (D_8036DCA0[buf][a + m].v.ob[2] < minZ) {
                minZ = D_8036DCA0[buf][a + m].v.ob[2];
            }
            if (D_8036DCA0[buf][a + m].v.ob[0] > maxX) {
                maxX = D_8036DCA0[buf][a + m].v.ob[0];
            }
            if (D_8036DCA0[buf][a + m].v.ob[1] > maxY) {
                maxY = D_8036DCA0[buf][a + m].v.ob[1];
            }
            if (D_8036DCA0[buf][a + m].v.ob[2] > maxZ) {
                maxZ = D_8036DCA0[buf][a + m].v.ob[2];
            }
        }
        for (m = 0; m < cnt; m++) {
            if (D_8036DCA0[buf][b + m].v.ob[0] < minX) {
                minX = D_8036DCA0[buf][b + m].v.ob[0];
            }
            if (D_8036DCA0[buf][b + m].v.ob[1] < minY) {
                minY = D_8036DCA0[buf][b + m].v.ob[1];
            }
            if (D_8036DCA0[buf][b + m].v.ob[2] < minZ) {
                minZ = D_8036DCA0[buf][b + m].v.ob[2];
            }
            if (D_8036DCA0[buf][b + m].v.ob[0] > maxX) {
                maxX = D_8036DCA0[buf][b + m].v.ob[0];
            }
            if (D_8036DCA0[buf][b + m].v.ob[1] > maxY) {
                maxY = D_8036DCA0[buf][b + m].v.ob[1];
            }
            if (D_8036DCA0[buf][b + m].v.ob[2] > maxZ) {
                maxZ = D_8036DCA0[buf][b + m].v.ob[2];
            }
        }
        k = 0;
        for (m = 0; m < cnt - 1; m++) {
            gSP1Triangle(dl++, k + 1, k, k + 8, 0);
            if (D_8036DCD5 != 0) {
                func_802802D4(D_8036DCA0[buf], a + k, a + k + 1, b + k);
            }
            gSP1Triangle(dl++, k + 8, k + 9, k + 1, 0);
            if (D_8036DCD5 != 0) {
                func_802802D4(D_8036DCA0[buf], b + k, b + k + 1, a + k + 1);
            }
            k++;
        }
        rowlen = cnt - 1;
        col += rowlen;
        a += rowlen, b += rowlen;
        if (D_802FC3F0[D_8036DCD6].nx == col) {
            col = 0;
            a++, b++;
        }
        vcount += cnt * 2;
        if (vcount >= 32) {
            gSPEndDisplayList(dl++);
            func_8028072C(bbox, minX, minY, minZ, maxX, maxY, maxZ);
            vcount = 0;
            bbox += 8;
        }
        if (b == total) {
            done = 1;
        }
    }
    if (vcount != 0) {
        gSPEndDisplayList(dl++);
        func_8028072C(bbox, minX, minY, minZ, maxX, maxY, maxZ);
        bbox += 8;
    }
    gDPPipeSync(gdl++);
    gDPSetTextureLOD(gdl++, G_TL_LOD);
    gDPPipeSync(gdl++);
    *gfx = gdl;
}

/* Flat-shade triangle (i0, i1, i2) of v: store its face normal, scaled to length 120, in all three vertices. */
void func_802802D4(Vtx *v, s32 i0, s32 i1, s32 i2) {
    f32 e1[3];
    f32 e2[3];
    f32 pad[3];
    f32 scale;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 len;

    e1[0] = v[i2].v.ob[0] - v[i0].v.ob[0];
    e1[1] = v[i2].v.ob[1] - v[i0].v.ob[1];
    e1[2] = v[i2].v.ob[2] - v[i0].v.ob[2];
    e2[0] = v[i1].v.ob[0] - v[i0].v.ob[0];
    e2[1] = v[i1].v.ob[1] - v[i0].v.ob[1];
    e2[2] = v[i1].v.ob[2] - v[i0].v.ob[2];
    nx = e1[1] * e2[2] - e1[2] * e2[1];
    ny = e1[2] * e2[0] - e1[0] * e2[2];
    nz = e1[0] * e2[1] - e1[1] * e2[0];
    len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len < 1.0) {
        len = 1.0f;
    }
    scale = 120.0 / len;
    nx *= scale;
    ny *= scale;
    nz *= scale;
    v[i0].v.cn[0] = (s8) nx;
    v[i0].v.cn[1] = (s8) ny;
    v[i0].v.cn[2] = (s8) nz;
    v[i0].v.cn[3] = 0;
    v[i1].v.cn[0] = (s8) nx;
    v[i1].v.cn[1] = (s8) ny;
    v[i1].v.cn[2] = (s8) nz;
    v[i1].v.cn[3] = 0;
    v[i2].v.cn[0] = (s8) nx;
    v[i2].v.cn[1] = (s8) ny;
    v[i2].v.cn[2] = (s8) nz;
    v[i2].v.cn[3] = 0;
}

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

/* Scatter the level's random textured quads (D_802FC494[n].count of them, up to 12): random position,
 * size and angle within the table's ranges, written into both vertex buffers. */
void func_802807D8(u8 id) {
    s32 i;
    s32 j;
    s16 x;
    s16 y;
    s16 z;
    s16 w;
    s16 d;
    s32 unused; /* set to 0, never read */
    u8 found;

    D_8036E374 = 0;
    unused = 0;
    found = 0;
    D_8036E370 = 0;
    do {
        if (D_802FC494[D_8036E370].id == id) {
            found = 1;
        } else {
            D_8036E370++;
        }
    } while (!found && D_8036E370 < 4);
    if (found) {
        D_8036E374 = D_802FC494[D_8036E370].count;
        D_8036E378 = D_80358070;
        func_802A0CC8(D_802FC494[D_8036E370].tex, 0);
        for (i = 0; i < D_8036E374; i++) {
            x = func_8026A828(D_802FC494[D_8036E370].x0, D_802FC494[D_8036E370].x1);
            y = func_8026A828(D_802FC494[D_8036E370].y0, D_802FC494[D_8036E370].y1);
            z = func_8026A828(D_802FC494[D_8036E370].z0, D_802FC494[D_8036E370].z1);
            w = func_8026A828(D_802FC494[D_8036E370].w0, D_802FC494[D_8036E370].w1);
            d = func_8026A828(D_802FC494[D_8036E370].d0, D_802FC494[D_8036E370].d1);
            w >>= 1, d >>= 1;
            D_8036DCE0[i].w = w;
            D_8036DCE0[i].d = d;
            D_8036DCE0[i].x = x;
            D_8036DCE0[i].z = z;
            D_8036DCE0[i].unk0 = func_8026A828(D_802FC494[D_8036E370].r0, D_802FC494[D_8036E370].r1);
            D_8036DCE0[i].angle = func_8026A828(0, 0xFFF);
            for (j = 0; j < 2; j++) {
                D_8036DD70[j][i][0].v.ob[0] = x - w;
                D_8036DD70[j][i][0].v.ob[1] = y;
                D_8036DD70[j][i][0].v.ob[2] = z - d;
                D_8036DD70[j][i][0].v.tc[0] = 0;
                D_8036DD70[j][i][0].v.tc[1] = 0;
                D_8036DD70[j][i][1].v.ob[0] = x + w;
                D_8036DD70[j][i][1].v.ob[1] = y;
                D_8036DD70[j][i][1].v.ob[2] = z - d;
                D_8036DD70[j][i][1].v.tc[0] = D_802FC494[D_8036E370].tu << 5;
                D_8036DD70[j][i][1].v.tc[1] = 0;
                D_8036DD70[j][i][2].v.ob[0] = x + w;
                D_8036DD70[j][i][2].v.ob[1] = y;
                D_8036DD70[j][i][2].v.ob[2] = z + d;
                D_8036DD70[j][i][2].v.tc[0] = D_802FC494[D_8036E370].tu << 5;
                D_8036DD70[j][i][2].v.tc[1] = D_802FC494[D_8036E370].tv << 5;
                D_8036DD70[j][i][3].v.ob[0] = x - w;
                D_8036DD70[j][i][3].v.ob[1] = y;
                D_8036DD70[j][i][3].v.ob[2] = z + d;
                D_8036DD70[j][i][3].v.tc[0] = 0;
                D_8036DD70[j][i][3].v.tc[1] = D_802FC494[D_8036E370].tv << 5;
            }
        }
    }
}

/* Move the random quads (func_802CE65C steps each one; the result comes back in D_803F9320/4),
 * wrap them around the spawn area, rebuild their corners in vertex buffer buf and draw them. */
void func_80280F34(Gfx **gfx, u8 buf) {
    Gfx *gdl;
    s32 i;
    s16 dx;
    s16 dz;

    gdl = *gfx;
    if (D_8036E374 != 0) {
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, 0x00504340, 0);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
        gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, D_802FC494[D_8036E370].alpha);
        gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036E378), G_IM_FMT_IA, G_IM_SIZ_16b,
                            D_802FC494[D_8036E370].tu, D_802FC494[D_8036E370].tv, 0, G_TX_CLAMP, G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        for (i = 0; i < D_8036E374; i++) {
            func_802CE65C(D_8036DCE0[i].x << 5, D_8036DCE0[i].z << 5, D_8036DCE0[i].unk0, D_8036DCE0[i].angle);
            dx = (D_803F9320 >> 5) - D_8036DCE0[i].x;
            dz = (D_803F9324 >> 5) - D_8036DCE0[i].z;
            D_8036DCE0[i].x = D_803F9320 >> 5;
            D_8036DCE0[i].z = D_803F9324 >> 5;
            /* note: the wrap amounts index the spawn table by i, not D_8036E370 */
            if (D_8036DCE0[i].x > D_802FC494[D_8036E370].x1) {
                D_8036DCE0[i].x = D_802FC494[D_8036E370].x0;
                dx -= D_802FC494[i].x1 - D_802FC494[i].x0;
            }
            if (D_8036DCE0[i].x < D_802FC494[D_8036E370].x0) {
                D_8036DCE0[i].x = D_802FC494[D_8036E370].x1;
                dx += D_802FC494[i].x1 - D_802FC494[i].x0;
            }
            if (D_8036DCE0[i].z > D_802FC494[D_8036E370].z1) {
                D_8036DCE0[i].z = D_802FC494[D_8036E370].z0;
                dz -= D_802FC494[i].z1 - D_802FC494[i].z0;
            }
            if (D_8036DCE0[i].z < D_802FC494[D_8036E370].z0) {
                D_8036DCE0[i].z = D_802FC494[D_8036E370].z1;
                dz += D_802FC494[i].z1 - D_802FC494[i].z0;
            }
            D_8036DD70[buf][i][0].v.ob[0] = D_8036DCE0[i].x - D_8036DCE0[i].w;
            D_8036DD70[buf][i][0].v.ob[2] = D_8036DCE0[i].z - D_8036DCE0[i].d;
            D_8036DD70[buf][i][1].v.ob[0] = D_8036DCE0[i].x + D_8036DCE0[i].w;
            D_8036DD70[buf][i][1].v.ob[2] = D_8036DCE0[i].z - D_8036DCE0[i].d;
            D_8036DD70[buf][i][2].v.ob[0] = D_8036DCE0[i].x + D_8036DCE0[i].w;
            D_8036DD70[buf][i][2].v.ob[2] = D_8036DCE0[i].z + D_8036DCE0[i].d;
            D_8036DD70[buf][i][3].v.ob[0] = D_8036DCE0[i].x - D_8036DCE0[i].w;
            D_8036DD70[buf][i][3].v.ob[2] = D_8036DCE0[i].z + D_8036DCE0[i].d;
            gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_8036DD70[buf][i]), 4, 0);
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
        }
        gDPPipeSync(gdl++);
    }
    *gfx = gdl;
}

void func_80281A70(s32 arg0) {
    s32 i;
    s32 j;

    D_8036E4C0 = 0;
    for (i = 0; i < 1; i++) {
        if (D_802FC520[i].id == arg0) {
            D_8036E380[D_8036E4C0].x = D_802FC520[i].x;
            D_8036E380[D_8036E4C0].y = D_802FC520[i].y;
            D_8036E380[D_8036E4C0].z = D_802FC520[i].z;
            func_8026A5CC(D_8036E380[D_8036E4C0].v, D_802FC528, sizeof(D_8036E380->v));
            for (j = 0; j < 4; j++) {
                D_8036E380[D_8036E4C0].v[j].v.ob[0] += D_8036E380[D_8036E4C0].x;
                D_8036E380[D_8036E4C0].v[j].v.ob[1] += D_8036E380[D_8036E4C0].y;
                D_8036E380[D_8036E4C0].v[j].v.ob[2] += D_8036E380[D_8036E4C0].z;
            }
            D_8036E380[D_8036E4C0].active = 1;
            D_8036E4C0++;
        }
    }
    if (D_8036E4C0 != 0) {
        D_8036E4C4 = func_802A0CC8(0x546, 0);
    }
    D_8036E4C8 = 0;
    D_8036E4CA = 0;
}

void func_80281CE4(void) {
    s32 i;
    s32 dist;

    D_8036E4CA = 0;
    if (D_8036E4C8 != 0) {
        D_8036E4C8--;
    }
    for (i = 0; i < D_8036E4C0; i++) {
        if (D_8036E380[i].active != 0) {
            dist = func_8026A6F0(D_803643E0 >> 5, D_803643E4 >> 5, D_803643E8 >> 5,
                                 D_8036E380[i].x, D_8036E380[i].y, D_8036E380[i].z);
            if (dist < 40) {
                D_8036E4C8 = 400;
                D_8036E4CA = 1;
                D_8036E380[i].active = 0;
                func_80260650(D_80367738, 0xB0, NULL);
                if (D_8036DCD8 == 0) {
                    func_80260650(D_80367738, 0xCF, &D_8036DCD8);
                }
            }
        }
    }
}

void func_80281E44(Gfx **gfx) {
    Gfx *gdl;
    s32 i;

    gdl = *gfx;
    if (D_8036E4C0 != 0) {
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
        gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPLoadTextureBlock(gdl++, D_8036E4C4, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                            G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        for (i = 0; i < D_8036E4C0; i++) {
            if (D_8036E380[i].active != 0) {
                gSPVertex(gdl++, D_8036E380[i].v, 4, 0);
                gSP1Triangle(gdl++, 0, 1, 2, 0);
                gSP1Triangle(gdl++, 0, 2, 3, 0);
            }
        }
        gDPPipeSync(gdl++);
    }
    *gfx = gdl;
}

void func_802A0B00(s32, s32);

extern s32 D_8036E4CC; /* overlay texture */
extern s16 D_8036E4D0; /* overlay alpha */
extern u8 D_8036E4D2;  /* overlay on */
extern Mtx D_02000000[];
extern Vtx D_802FC568[];
extern s16 D_80367BD6;

s32 func_8029DBF0(u8);

void func_802821D0(void) {
    D_8036E4CC = D_80358070;
    func_802A0B00(0xA98, 0);
    D_80358070 += 0x800;
    D_8036E4D0 = 0;
    D_8036E4D2 = 0;
}

/* Fade a textured quad (D_802FC568) in when arg1 is 7, 11, 17 or 18 and func_8029DBF0(arg1) is 0;
 * otherwise fade it out by 10 per frame. */
void func_80282224(Gfx **gfx, u8 arg1) {
    Gfx *gdl;
    u8 flag;
    u8 flag2;

    gdl = *gfx;
    flag = arg1 == 7 || arg1 == 11 || arg1 == 17 || arg1 == 18;
    if (D_8036E4D2 != 0 && !flag) {
        if ((D_8036E4D0 -= 10) < 0) {
            D_8036E4D0 = 0;
        }
    } else {
        if (flag) {
            flag2 = !func_8029DBF0(arg1);
        }
        if (flag && flag2) {
            D_8036E4D0 = 0xFF;
            D_8036E4D2 = 1;
        } else {
            D_8036E4D0 = 0;
        }
    }
    if (D_8036E4D0 == 0) {
        D_8036E4D2 = 0;
    }
    if (D_8036E4D2 != 0) {
        gSPMatrix(gdl++, &D_02000000[3], G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gDPPipeSync(gdl++);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetCombineMode(gdl++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
        gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0xFF, 0xFF, (D_8036E4D0 < D_80367BD6) ? D_8036E4D0 : D_80367BD6);
        gDPLoadTextureBlock(gdl++, OS_K0_TO_PHYSICAL(D_8036E4CC), G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                            G_TX_NOLOD, G_TX_NOLOD);
        gSPVertex(gdl++, OS_K0_TO_PHYSICAL(D_802FC568), 4, 0);
        gSP1Triangle(gdl++, 0, 1, 2, 0);
        gSP1Triangle(gdl++, 0, 2, 3, 0);
        gDPPipeSync(gdl++);
    }
    *gfx = gdl;
}

extern u8 D_8036E4D3;  /* number of active rings (0..2) */
extern u32 D_8036E4D4; /* frame the last ring started */
extern Gfx D_802FFF38[];
extern Gfx D_80300A68[];
extern u32 D_803156C4;
extern u8 D_803643D6;
extern Mtx D_8036E4D8[][2];
extern f32 D_8036E5D8[]; /* ring scales */
extern s32 D_803EF6DC;
extern s32 D_803EF6E0;
extern s32 D_803EF6E4;
extern u8 D_803EF6FF;

void func_802AC1A0(s32);

void func_80282728(void) {
    D_8036E4D3 = 0;
    D_8036E4D4 = 0;
}

/* Expanding-ring effect: tint the screen, then draw up to two growing copies of display list
 * D_80300A68 at (D_803EF6DC, D_803EF6E0, D_803EF6E4) / 32, a new one 10 frames after the last. */
void func_8028273C(Gfx **gfx, u8 arg1) {
    Gfx *gdl;
    f32 mf[4][4];
    f32 scale[4][4];
    s32 i;

    gdl = *gfx;
    if (D_803643D6 != 0) {
        if (D_803EF6FF != 0 && D_8036E4D3 == 0) {
            D_8036E4D3 = 1;
            D_8036E5D8[0] = 0.001f;
            D_8036E4D4 = D_803156C4;
        }
        gDPPipeSync(gdl++);
        gDPSetColorDither(gdl++, G_CD_DISABLE);
        if (D_8036E4D3 != 0) {
            gDPPipeSync(gdl++);
            gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetPrimColor(gdl++, 0, 0, 0x37, 0x00, 0x00, 0x9B);
            gDPFillRectangle(gdl++, 0, 0, 319, 239);
        }
        for (i = 0; i < D_8036E4D3; i++) {
            guTranslateF(mf, D_803EF6DC / 32.0f, D_803EF6E0 / 32.0f, D_803EF6E4 / 32.0f);
            guScaleF(scale, D_8036E5D8[i], D_8036E5D8[i], D_8036E5D8[i]);
            D_8036E5D8[i] += 0.06;
            guMtxCatF(scale, mf, mf);
            guMtxF2L(mf, &D_8036E4D8[arg1][i]);
            gDPPipeSync(gdl++);
            gDPSetPrimColor(gdl++, 0, 0, 0xFF, 0x00, 0x00, 0x64);
            gSPMatrix(gdl++, osVirtualToPhysical(&D_8036E4D8[arg1][i]), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
            gSPSegment(gdl++, 6, osVirtualToPhysical(D_802FFF38));
            gSPDisplayList(gdl++, osVirtualToPhysical(D_80300A68));
            gSPMatrix(gdl++, &D_02000000[7], G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        }
        if (D_8036E4D4 + 10 < D_803156C4 && D_8036E4D3 != 0 && D_8036E4D3 < 2) {
            D_8036E4D4 = D_803156C4;
            D_8036E5D8[D_8036E4D3] = 0.001f;
            D_8036E4D3++;
        }
        gDPPipeSync(gdl++);
        gDPSetColorDither(gdl++, G_CD_MAGICSQ);
        gDPPipeSync(gdl++);
        if (D_8036E4D3 != 0) {
            func_802AC1A0(D_8036E5D8[0] * 283.0f);
        }
    }
    *gfx = gdl;
}
