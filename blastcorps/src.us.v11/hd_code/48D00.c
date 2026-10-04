#include "common.h"
#include <ultra64.h>

/* D_8039B070: array of D_8039B610 0x48-byte entries. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ u8 unk0E; /* box type (D_802FDB98) */
    /* 0x0F */ u8 pad0F;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 pad25;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ u32 unk2C[4]; /* face textures */
    /* 0x3C */ Vtx *unk3C;   /* 8 corner vertices */
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Entry48D00;
extern Entry48D00 D_8039B070_entries[];

/* D_802FDB98: 0x18-byte box definitions (min/max extents, face textures
 * and a radius). */
typedef struct {
    /* 0x00 */ s16 x0, x1, y0, y1, z0, z1;
    /* 0x0C */ s16 tex[4];
    /* 0x14 */ s16 radius;
    /* 0x16 */ s16 unk16;
} BoxDef48D00;
extern BoxDef48D00 D_802FDB98_boxes[];

extern s32 D_8039B610;
extern s32 D_8039B614;
extern s32 D_8039B618;
extern s32 D_8039B61C;
extern u8 D_8039B620;
extern u8 D_803F932C;
extern Vtx *D_80358070; /* vertex allocator */

s32 func_802CE6F8(s32, s32, s32);
u32 func_802A0CC8(s16, s32);
void func_8028DA5C(Vtx *v, u8 arg1);

/* Load the boxes from level data: 12-byte records (s16 x, y, z; u8 type,
 * u8 timer; s16 unk1A, unk1C). Positions are scaled by 32 and snapped to
 * the ground; each box gets its face textures and 8 vertices. */
void func_8028D4C0(u8 *arg0, u8 *arg1) {
    D_8039B610 = 0;
    D_8039B614 = 0;
    D_8039B618 = 0;
    D_8039B61C = 0;
    D_8039B620 = 0;
    while (arg0 != arg1) {
        D_8039B070_entries[D_8039B610].unk0 = *(s16 *) (arg0 + 0) << 5;
        D_8039B070_entries[D_8039B610].unk4 = *(s16 *) (arg0 + 2) << 5;
        D_8039B070_entries[D_8039B610].unk8 = *(s16 *) (arg0 + 4) << 5;
        D_8039B070_entries[D_8039B610].unk4 = func_802CE6F8(D_8039B070_entries[D_8039B610].unk0,
                                                            D_8039B070_entries[D_8039B610].unk8,
                                                            D_8039B070_entries[D_8039B610].unk4);
        D_8039B070_entries[D_8039B610].unk23 = D_803F932C;
        D_8039B070_entries[D_8039B610].unk0E = arg0[6];
        D_8039B070_entries[D_8039B610].unk10 = arg0[7] * 60;
        D_8039B070_entries[D_8039B610].unk12 = D_8039B070_entries[D_8039B610].unk10;
        D_8039B070_entries[D_8039B610].unk2C[0] =
            func_802A0CC8(D_802FDB98_boxes[D_8039B070_entries[D_8039B610].unk0E].tex[0], 0);
        D_8039B070_entries[D_8039B610].unk2C[1] =
            func_802A0CC8(D_802FDB98_boxes[D_8039B070_entries[D_8039B610].unk0E].tex[1], 0);
        D_8039B070_entries[D_8039B610].unk2C[2] =
            func_802A0CC8(D_802FDB98_boxes[D_8039B070_entries[D_8039B610].unk0E].tex[2], 0);
        D_8039B070_entries[D_8039B610].unk2C[3] =
            func_802A0CC8(D_802FDB98_boxes[D_8039B070_entries[D_8039B610].unk0E].tex[3], 0);
        D_8039B070_entries[D_8039B610].unk0C = 0;
        D_8039B070_entries[D_8039B610].unk14 = 0;
        D_8039B070_entries[D_8039B610].unk1E = 0;
        D_8039B070_entries[D_8039B610].unk19 = 0;
        D_8039B070_entries[D_8039B610].unk1A = *(s16 *) (arg0 + 8);
        D_8039B070_entries[D_8039B610].unk1C = *(s16 *) (arg0 + 10);
        D_8039B070_entries[D_8039B610].unk20 = 0;
        D_8039B070_entries[D_8039B610].unk22 = 0;
        D_8039B070_entries[D_8039B610].unk24 = 0;
        D_8039B070_entries[D_8039B610].unk26 = 0;
        D_8039B070_entries[D_8039B610].unk28 = 0;
        D_8039B070_entries[D_8039B610].unk2A = 0;
        D_8039B070_entries[D_8039B610].unk18 = 1;
        D_8039B070_entries[D_8039B610].unk3C = D_80358070;
        D_80358070 += 8;
        func_8028DA5C(D_8039B070_entries[D_8039B610].unk3C, D_8039B070_entries[D_8039B610].unk0E);
        /* One statement (a for-loop increment, probably). */
        D_8039B610++, arg0 += 12;
    }
}

/* Fill the 8 corner Vtx of box type arg1, with texture coords 0 or 31.0
 * (0x3E0). Corners 2 and 5 need the comma form (probably a macro):
 * separate statements store the two tcs in the wrong order. */
void func_8028DA5C(Vtx *v, u8 arg1) {
    v[0].v.ob[0] = D_802FDB98_boxes[arg1].x1;
    v[0].v.ob[1] = D_802FDB98_boxes[arg1].y0;
    v[0].v.ob[2] = D_802FDB98_boxes[arg1].z0;
    v[0].v.tc[1] = 0;
    v[0].v.tc[0] = 0;
    v[1].v.ob[0] = D_802FDB98_boxes[arg1].x1;
    v[1].v.ob[1] = D_802FDB98_boxes[arg1].y1;
    v[1].v.ob[2] = D_802FDB98_boxes[arg1].z0;
    v[1].v.tc[1] = 0x3E0;
    v[1].v.tc[0] = 0;
    v[2].v.ob[0] = D_802FDB98_boxes[arg1].x1;
    v[2].v.ob[1] = D_802FDB98_boxes[arg1].y1;
    v[2].v.ob[2] = D_802FDB98_boxes[arg1].z1;
    v[2].v.tc[0] = 0x3E0, v[2].v.tc[1] = 0x3E0;
    v[3].v.ob[0] = D_802FDB98_boxes[arg1].x1;
    v[3].v.ob[1] = D_802FDB98_boxes[arg1].y0;
    v[3].v.ob[2] = D_802FDB98_boxes[arg1].z1;
    v[3].v.tc[0] = 0x3E0;
    v[3].v.tc[1] = 0;
    v[4].v.ob[0] = D_802FDB98_boxes[arg1].x0;
    v[4].v.ob[1] = D_802FDB98_boxes[arg1].y0;
    v[4].v.ob[2] = D_802FDB98_boxes[arg1].z0;
    v[4].v.tc[0] = 0x3E0;
    v[4].v.tc[1] = 0;
    v[5].v.ob[0] = D_802FDB98_boxes[arg1].x0;
    v[5].v.ob[1] = D_802FDB98_boxes[arg1].y1;
    v[5].v.ob[2] = D_802FDB98_boxes[arg1].z0;
    v[5].v.tc[0] = 0x3E0, v[5].v.tc[1] = 0x3E0;
    v[6].v.ob[0] = D_802FDB98_boxes[arg1].x0;
    v[6].v.ob[1] = D_802FDB98_boxes[arg1].y1;
    v[6].v.ob[2] = D_802FDB98_boxes[arg1].z1;
    v[6].v.tc[1] = 0x3E0;
    v[6].v.tc[0] = 0;
    v[7].v.ob[0] = D_802FDB98_boxes[arg1].x0;
    v[7].v.ob[1] = D_802FDB98_boxes[arg1].y0;
    v[7].v.ob[2] = D_802FDB98_boxes[arg1].z1;
    v[7].v.tc[1] = 0;
    v[7].v.tc[0] = 0;
}

/* Activate entry arg0. The mix of struct-field and per-field-symbol
 * accesses below is deliberate: it is the spelling whose schedule matches. */
extern u8 D_8039B070;
extern u8 D_8039B088;
extern u8 D_8039B089;
extern u8 D_8039B0B0;
extern u8 D_8039B0B4;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern s32 D_80367738;

void func_802CDA10(s32, s32, s32);
void func_802608C8(s32);
void func_80260650(s32, s32, s32);
Entry48D00 *func_8028DE94(void);

void func_8028DD64(u8 arg0) {
    Entry48D00 *match;

    func_802CDA10(D_8039B070_entries[arg0].unk0, D_8039B070_entries[arg0].unk4, D_8039B070_entries[arg0].unk8);
    *(&D_8039B089 + arg0 * 0x48) = 5;
    D_8039B070_entries[arg0].unk18 = 0;
    D_802E8BE4 = 10;
    D_802E8BE8 = 0x190;
    if (D_8039B070_entries[arg0].unk40 != 0) {
        func_802608C8(D_8039B070_entries[arg0].unk40);
        match = func_8028DE94();
        if (match != NULL) {
            func_80260650(D_80367738, 0x73, (s32) &match->unk40);
        }
    }
    if (*(s32 *) (&D_8039B0B4 + arg0 * 0x48) != 0) {
        func_802608C8(*(s32 *) (&D_8039B0B4 + arg0 * 0x48));
    }
    func_80260650(D_80367738, 0x10, 0);
}

extern u8 D_8039B070;
extern s32 D_8039B610;

/* First entry with both unk18 and unk14 set, or NULL. */
Entry48D00 *func_8028DE94(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk14 != 0) {
            return &D_8039B070_entries[i];
        }
    }
    return NULL;
}

extern u8 D_803F932D;
extern u8 D_803F932E;
extern s32 D_803F9320;
extern s32 D_803F9324;
extern s32 D_802E8BDC;
extern s32 D_80358060;
extern u8 D_803643D9;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern u8 D_80364456;
extern s32 D_803A73F0;
extern s32 D_803A73F4;
extern s32 D_803A73F8;
extern s16 D_803A7410;
extern s16 D_803A7412;
extern u8 D_803ED40C;
extern s16 D_802FDB70[]; /* speed cap per mode */

void func_8026AD30(s32);
s32 func_8029B930(void);
s16 func_802A6F6C(void);
void func_802CDAE8(s16, s16);
s32 func_802CDB70(s16, s16);
u8 func_802CDF94(s16);
s16 func_802CE3B8(s16);
void func_802CE4F0(s32, s32, s32);
void func_802CE5BC(s32, s32, s32, s16, s32, s32);
void func_802CE65C(s32, s32, s16, s16);
void func_802CE880(s32, s32, s32, s32, s32);
void func_802CE90C(s32);
s32 func_802CE958(s32);

/* Per-frame update of the boxes (arg0 = current mode; the box sibling of
 * 4B5E0's func_802906C0). Counts down unk19 (then fires the box's
 * trigger), runs the unk10 fuse and the unk20 throb, pushes moving boxes
 * along (func_802CE65C), resets their speed (unk1E) from the player's
 * movement when hit, handles collision, heading and slow-down, and keeps
 * the box's looping sound (unk44) alive while it moves. The two-in-one
 * assignments are deliberate: separate statements schedule differently. */
void func_8028DF14(u8 arg0) {
    s32 i;
    u8 hit;
    s32 dx;
    s32 dy;
    s32 dz;
    s16 old10;
    s16 old12;
    u8 moved;
    f32 f;
    s16 local;
    u8 drop;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk19 != 0) {
            D_8039B070_entries[i].unk19--;
            if (D_8039B070_entries[i].unk19 == 0) {
                D_803A73F0 = D_8039B070_entries[i].unk0;
                D_803A73F4 = D_8039B070_entries[i].unk4;
                D_803A73F8 = D_8039B070_entries[i].unk8;
                if (D_8039B070_entries[i].unk1A != 0) {
                    func_802CDAE8(D_8039B070_entries[i].unk1C, D_8039B070_entries[i].unk1A);
                }
            }
        }
        if (D_8039B070_entries[i].unk18 != 0) {
            if (D_8039B070_entries[i].unk14 != 0 && D_8039B070_entries[i].unk10 != 0) {
                D_8039B070_entries[i].unk10--;
                if (D_8039B070_entries[i].unk10 <= 0) {
                    func_8028DD64(i);
                }
            }
            if (D_8039B070_entries[i].unk14 != 0 && D_8039B070_entries[i].unk12 != 0) {
                f = (f32) D_8039B070_entries[i].unk10 / (f32) D_8039B070_entries[i].unk12;
                f = 1.0 - f;
                f = f * 50.0;
                if (D_8039B070_entries[i].unk22 != 0) {
                    D_8039B070_entries[i].unk20 -= (s16) f;
                    if (D_8039B070_entries[i].unk20 < 0) {
                        D_8039B070_entries[i].unk20 = -D_8039B070_entries[i].unk20;
                        D_8039B070_entries[i].unk22 = 0;
                    }
                } else {
                    D_8039B070_entries[i].unk20 += (s16) f;
                    if (D_8039B070_entries[i].unk20 >= 0x100) {
                        D_8039B070_entries[i].unk20 = 0x1FE - D_8039B070_entries[i].unk20;
                        D_8039B070_entries[i].unk22 = 1;
                    }
                }
            }
            if (arg0 == 0) {
                D_8039B070_entries[i].unk1E = 0;
            }
            if (D_8039B070_entries[i].unk1E != 0 && func_802CE958(i + 0x100) == 0) {
                func_802CE65C(D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk8, D_8039B070_entries[i].unk1E,
                              D_8039B070_entries[i].unk0C);
                D_8039B070_entries[i].unk0 = D_803F9320;
                D_8039B070_entries[i].unk8 = D_803F9324;
                D_8039B070_entries[i].unk4 = func_802CE6F8(D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk8,
                                                           D_8039B070_entries[i].unk4);
                D_8039B070_entries[i].unk23 = D_803F932C;
            }
            func_802CE4F0(D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk4, D_8039B070_entries[i].unk8);
            hit = func_802CDF94(D_802FDB98_boxes[D_8039B070_entries[i].unk0E].radius);
            if (hit != 0) {
                if (D_803F932D != 0) {
                    D_803643D9 = 1;
                    func_8028DD64(i);
                }
                if (D_803F932E != 0) {
                    D_803ED40C = 1;
                }
                if (D_8039B070_entries[i].unk14 == 0 && arg0 != 0) {
                    if (func_8028DE94() == NULL) {
                        func_80260650(D_80367738, 0x73, (s32) &D_8039B070_entries[i].unk40);
                    }
                    D_8039B070_entries[i].unk14 = D_80358060;
                }
                if (D_8039B620 == arg0) {
                    dx = D_803643E0 - D_8039B614, dy = D_803643E4 - D_8039B618;
                    dz = D_803643E8 - D_8039B61C;
                    D_8039B070_entries[i].unk1E = sqrtf(dx * dx + dy * dy + dz * dz) + 8.0f;
                    if (D_8039B070_entries[i].unk1E > D_802FDB70[arg0] && D_802E8BDC != 0x22) {
                        D_8039B070_entries[i].unk1E = D_802FDB70[arg0];
                    }
                } else {
                    D_8039B070_entries[i].unk1E = 0;
                }
            }
            if (D_8039B070_entries[i].unk1E != 0) {
                old10 = D_803A7410, old12 = D_803A7412;
                moved = 0;
                func_802CE5BC(D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk4, D_8039B070_entries[i].unk8,
                              D_802FDB98_boxes[D_8039B070_entries[i].unk0E].radius, 201, 0);
                if (old10 != D_803A7410 || old12 != D_803A7412) {
                    moved = 1;
                }
                if (D_8039B070_entries[i].unk1A != 0) {
                    local = 0;
                } else {
                    local = D_8039B070_entries[i].unk1C;
                }
                if (func_802CDB70(D_802FDB98_boxes[D_8039B070_entries[i].unk0E].unk16, local) != 0) {
                    func_8028DD64(i);
                }
            } else {
                moved = 0;
            }
            drop = 0;
            if (D_803A7410 != 0 || D_803A7412 != 0xFFF) {
                if (func_8029B930() < 100) {
                    drop = 1;
                } else if (moved) {
                    D_8039B070_entries[i].unk0C = func_802CE3B8(D_8039B070_entries[i].unk0C);
                } else {
                    D_8039B070_entries[i].unk0C = func_802A6F6C();
                }
            }
            if ((drop || arg0 == 0) && D_8039B070_entries[i].unk18 != 0) {
                func_802CE880(i + 0x100, D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk4,
                              D_8039B070_entries[i].unk8, D_802FDB98_boxes[D_8039B070_entries[i].unk0E].radius);
                D_8039B070_entries[i].unk1E = 0;
            } else {
                func_802CE90C(i + 0x100);
            }
            if (D_8039B070_entries[i].unk1E > 0) {
                D_8039B070_entries[i].unk1E -= (D_802E8BDC != 0x2B) ? 8 : 4;
            } else {
                D_8039B070_entries[i].unk1E = 0;
            }
            if (D_8039B070_entries[i].unk1E > 0 && D_8039B070_entries[i].unk44 == 0 &&
                D_8039B070_entries[i].unk18 != 0) {
                func_80260650(D_80367738, 7, (s32) &D_8039B070_entries[i].unk44);
                if (D_80364456 == 4) {
                    func_8026AD30(0x54);
                }
            }
            if (D_8039B070_entries[i].unk44 != 0 && D_8039B070_entries[i].unk1E == 0) {
                func_802608C8(D_8039B070_entries[i].unk44);
            }
        }
    }
    D_8039B614 = D_803643E0;
    D_8039B618 = D_803643E4;
    D_8039B61C = D_803643E8;
    D_8039B620 = arg0;
}

extern u8 D_02000000[]; /* segment 2 base */

/* Per-frame dynamic buffer: one matrix per box at 0x600. */
typedef struct {
    /* 0x000 */ u8 pad[0x600];
    /* 0x600 */ Mtx mtx[1];
} Dyn48D00;

/* Load two 32x32 RGBA16 textures (a into tmem 0, b into 0x100) and set up
 * render tiles 0 and 1 for them (trilinear blend between the two). */
#define BOX_TEX_PAIR(g, a, b)                                                                              \
    gDPSetTextureImage(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (a) - 0x80000000);                              \
    gDPSetTile(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);                  \
    gDPLoadSync(g++);                                                                                      \
    gDPLoadBlock(g++, G_TX_LOADTILE, 0, 0, 0x3FF, 0x100);                                                  \
    gDPSetTextureImage(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (b) - 0x80000000);                              \
    gDPTileSync(g++);                                                                                      \
    gDPSetTile(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);              \
    gDPLoadSync(g++);                                                                                      \
    gDPLoadBlock(g++, G_TX_LOADTILE, 0, 0, 0x3FF, 0x100);                                                  \
    gDPSetTile(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, 0, 0, 0, 5, 0, 0, 5, 0);                              \
    gDPSetTileSize(g++, 0, 2, 2, 0x7E, 0x7E);                                                              \
    gDPSetTile(g++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0x100, 1, 0, 0, 5, 0, 0, 5, 0);                          \
    gDPSetTileSize(g++, 1, 2, 2, 0x7E, 0x7E);

/* Draw the boxes: rotate (unk2A, 0..4095 = one turn) and place each one,
 * then draw its faces in three passes, retexturing the shared 8 vertices
 * with gSPModifyVertex between them; unk20 drives the blend (prim LOD). */
void func_8028E9E4(Gfx **gdl, Dyn48D00 *dyn) {
    Gfx *gfx;
    s32 i;
    f32 rot[4][4];
    f32 trans[4][4];

    gfx = *gdl;
    if (D_8039B610 > 0) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_2CYCLE);
        gDPSetRenderMode(gfx++, G_RM_PASS, G_RM_AA_ZB_OPA_SURF2);
        gDPSetCombineLERP(gfx++, TEXEL1, TEXEL0, PRIM_LOD_FRAC, TEXEL0, TEXEL1, TEXEL0, PRIM_LOD_FRAC, TEXEL0, 0, 0,
                          0, COMBINED, 0, 0, 0, SHADE);
        gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
        gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetTextureLOD(gfx++, G_TL_TILE);
    }
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0) {
            guRotateF(rot, (f32) D_8039B070_entries[i].unk2A / 4095.0 * 360.0, 0.0f, 1.0f, 0.0f);
            guTranslateF(trans, D_8039B070_entries[i].unk0 / 32.0f, D_8039B070_entries[i].unk4 / 32.0f,
                         D_8039B070_entries[i].unk8 / 32.0f);
            guMtxCatF(rot, trans, rot);
            guMtxF2L(rot, &dyn->mtx[i]);
            gSPMatrix(gfx++, i * sizeof(Mtx) + 0x600 + (u32) D_02000000, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
            gSPVertex(gfx++, osVirtualToPhysical(D_8039B070_entries[i].unk3C), 8, 0);
            gDPPipeSync(gfx++);
            BOX_TEX_PAIR(gfx, D_8039B070_entries[i].unk2C[0], D_8039B070_entries[i].unk2C[1])
            gDPSetPrimColor(gfx++, 0, D_8039B070_entries[i].unk20, 0, 0, 0, 0);
            gSP1Triangle(gfx++, 6, 3, 2, 0);
            gSP1Triangle(gfx++, 6, 7, 3, 0);
            gSP1Triangle(gfx++, 4, 1, 0, 0);
            gSP1Triangle(gfx++, 1, 4, 5, 0);
            gSPModifyVertex(gfx++, 0, G_MWO_POINT_ST, 0x03E00000);
            gSPModifyVertex(gfx++, 1, G_MWO_POINT_ST, 0x03E003E0);
            gSPModifyVertex(gfx++, 2, G_MWO_POINT_ST, 0x000003E0);
            gSPModifyVertex(gfx++, 3, G_MWO_POINT_ST, 0);
            gSP1Triangle(gfx++, 0, 1, 2, 0);
            gSP1Triangle(gfx++, 0, 2, 3, 0);
            gSPModifyVertex(gfx++, 7, G_MWO_POINT_ST, 0x03E00000);
            gSPModifyVertex(gfx++, 6, G_MWO_POINT_ST, 0x03E003E0);
            gSPModifyVertex(gfx++, 5, G_MWO_POINT_ST, 0x000003E0);
            gSPModifyVertex(gfx++, 4, G_MWO_POINT_ST, 0);
            gSP1Triangle(gfx++, 7, 5, 4, 0);
            gSP1Triangle(gfx++, 7, 6, 5, 0);
            BOX_TEX_PAIR(gfx, D_8039B070_entries[i].unk2C[2], D_8039B070_entries[i].unk2C[3])
            gSPModifyVertex(gfx++, 2, G_MWO_POINT_ST, 0);
            gSPModifyVertex(gfx++, 1, G_MWO_POINT_ST, 0x03E00000);
            gSPModifyVertex(gfx++, 5, G_MWO_POINT_ST, 0x03E003E0);
            gSPModifyVertex(gfx++, 6, G_MWO_POINT_ST, 0x000003E0);
            gSP1Triangle(gfx++, 5, 2, 1, 0);
            gSP1Triangle(gfx++, 2, 5, 6, 0);
            gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
        }
    }
    gDPSetTextureLOD(gfx++, G_TL_LOD);
    gDPPipeSync(gfx++);
    *gdl = gfx;
}

void func_802AACD4(u8, s32, s32, void *, void *);
extern u8 D_8039B094;

void func_8028F6B4(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk23 == arg0) {
            D_8039B070_entries[i].unk1E = 0;
            *(&D_8039B094 + i * 0x48) = 1;
            func_802AACD4(arg0, D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk8,
                          &D_8039B070_entries[i].unk26, &D_8039B070_entries[i].unk28);
        }
    }
}

extern s32 D_8039B610;
extern u8 D_8039B070;

s32 func_802AAE1C(u8, s16, s16, void *, void *);
s32 func_802CE6F8(s32, s32, s32);
void func_802CE4F0(s32, s32, s32);
s32 func_802CDB70(s16, s16);
void func_8028DD64(u8);

void func_8028F794(u8 arg0) {
    s32 i;
    s16 local;

    for (i = 0; i < D_8039B610; i++) {
        if (*(&D_8039B070 + i * 0x48 + 0x18) != 0 && *(&D_8039B070 + i * 0x48 + 0x24) != 0) {
            func_802AAE1C(
                arg0,
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x26),
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x28),
                (void *) (&D_8039B070 + i * 0x48),
                (void *) (&D_8039B070 + i * 0x48 + 8));

            *(s32 *) (&D_8039B070 + i * 0x48 + 4) = func_802CE6F8(
                *(s32 *) (&D_8039B070 + i * 0x48),
                *(s32 *) (&D_8039B070 + i * 0x48 + 8),
                *(s32 *) (&D_8039B070 + i * 0x48 + 4));

            if (*(s16 *) (&D_8039B070 + i * 0x48 + 0x1A) != 0) {
                local = 0;
            } else {
                local = *(s16 *) (&D_8039B070 + i * 0x48 + 0x1C);
            }

            func_802CE4F0(*(s32 *) (&D_8039B070 + i * 0x48),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 4),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 8));

            if (func_802CDB70(*(s16 *) (0x802FDBAC + *(&D_8039B070 + i * 0x48 + 0xE) * 0x18), local) != 0) {
                func_8028DD64((u8) i);
            }
        }
    }
}

extern u8 D_8039B094;

void func_8028F93C(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        *(&D_8039B094 + i * 0x48) = 0;
    }
}

extern u8 D_803A7424;
s32 func_8026A6F0(s32, s32, s32, s32, s32, s32);

/* Set D_803A7424 if the point (arg0, arg1, arg2) is within any active
 * entry's category radius (all in 1/32 units). */
void func_8028F994(s32 arg0, s32 arg1, s32 arg2) {
    s32 i;
    s32 dist;

    /* One statement (likely a macro in the original): separate
     * statements schedule the three reloads in the opposite order. */
    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5;
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
                                 D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
            if (dist <= (*(s16 *) (0x802FDBAC + D_8039B070_entries[i].unk0E * 0x18) >> 5)) {
                D_803A7424 = 1;
            }
        }
    }
}

extern u8 D_802FDB98[];
void func_8028FAC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i;
    s32 dist;

    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk24 == 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
                                 D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
            if (dist <= (*(s16 *) ((u8 *) D_802FDB98 + 0x14 + D_8039B070_entries[i].unk0E * 0x18) >> 5) + arg3) {
                D_803A7424 = 1;
            }
        }
    }
}

extern OSMesgQueue D_80370BF8;
extern u8 D_802FDBD0;
extern u8 D_802FDBD4;
void func_802DB4D0(OSMesgQueue *);
void func_802DB594(OSContPad *);
u8 func_8028FCD4(void *arg0, u8 *arg1);

/* Boot-time controller check: read the pads (osContStartReadData,
 * osRecvMesg, osContGetReadData), note whether START is held, latch it
 * into D_802FDBD0 if controller 1 is present, and set D_802FDBD4 when
 * START is held but was not latched. */
void func_8028FC10(void) {
    u8 mask;
    u8 start;
    OSContPad pads[4];

    start = 0;
    func_802DB4D0(&D_80370BF8);
    osRecvMesg(&D_80370BF8, 0, 1);
    func_802DB594(pads);
    if (pads[0].button & 0x1000) {
        start = 1;
    }
    if (func_8028FCD4(&D_80370BF8, &mask) == 0 && (mask & 1)) {
        D_802FDBD0 = start;
    }
    D_802FDBD4 = start && !D_802FDBD0;
}

typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Status48D00;


/* Wait for arg0->unk8, then build a bitmask in *arg1 of the 4 status
 * entries with bit 0 of unk2 set and unk3 clear. */
u8 func_8028FCD4(void *arg0, u8 *arg1) {
    Status48D00 status[4];
    s32 i;

    *arg1 = 0;
    osContStartQuery(arg0);
    while (*(s32 *) ((u8 *) arg0 + 8) == 0) {
    }
    osRecvMesg(arg0, 0, 0);
    osContGetQuery(status);
    for (i = 0; i < 4; i++) {
        if ((status[i].unk2 & 1) && status[i].unk3 == 0) {
            *arg1 |= 1 << i;
        }
    }
    return status[0].unk3;
}
