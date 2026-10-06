#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* D_8039C550: D_8039C710 0x38-byte entries (positions in 1/32 units). */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ u8 unk10; /* model index into D_802FDC08 */
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12; /* riding a D_8039C718 lift */
    /* 0x13 */ u8 unk13; /* which D_8039C718 entry */
    /* 0x14 */ s32 unk14; /* bounce velocity */
    /* 0x18 */ s32 unk18; /* bounce time */
    /* 0x1C */ s32 unk1C; /* bounce base height */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 pad21[3];
    /* 0x24 */ s32 unk24; /* distance to target */
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 pad2E[2];
    /* 0x30 */ u32 unk30; /* texture (KSEG0 address) */
    /* 0x34 */ s32 unk34; /* sound handle */
} Entry4B5E0;

/* D_8039C718: D_8039C7F8 0x1C-byte entries. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ u8 unk10; /* model index into D_802FDC08 */
    /* 0x11 */ u8 unk11;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
} Entry4B5E0c;

/* D_8039C800: D_8039C940 0x28-byte entries: a position and a box of
 * 1/32-unit points around it, plus the D_8039C718 index it came from. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 pad24[2];
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 unk27;
} Entry4B5E0b;

/* D_802FDC08: 0x290-byte models: a display list, then texture size and
 * a radius. */
typedef struct {
    /* 0x000 */ Gfx dl[0x50];
    /* 0x280 */ s16 unk280; /* texture id for func_802A0CC8 */
    /* 0x282 */ u8 width;
    /* 0x283 */ u8 height;
    /* 0x284 */ s16 radius;
    /* 0x286 */ s16 unk286; /* pickup range */
    /* 0x288 */ s32 unk288;
    /* 0x28C */ u8 pad28C[4];
} Model4B5E0;

/* Per-frame dynamic buffer: one matrix per entry at 0xB00. */
typedef struct Dyn4B5E0 {
    /* 0x000 */ u8 pad[0xB00];
    /* 0xB00 */ Mtx mtx[1];
} Dyn4B5E0;

extern Entry4B5E0 D_8039C550[];
extern u8 D_8039C579;
extern s32 D_8039C710;
extern Entry4B5E0c D_8039C718[];
extern s32 D_8039C7F8;
extern Entry4B5E0b D_8039C800[];
extern s32 D_8039C944;
extern s32 D_8039C948;
extern s32 D_8039C94C;
extern u8 D_8039C950;
extern s32 D_8039C954;
extern s32 D_8039C958;
extern s32 D_8039C95C;
extern s32 D_803FB8B0;
extern Model4B5E0 D_802FDC08[];
extern u8 D_02000000[]; /* segment 2 base */
extern s16 D_802FDBE0[];
extern s32 D_80367738;

void func_80291724(s32);

/* Load this file's objects from level data: reset the counters, then
 * read D_8039C710 0x38-byte entries (s16 x, y, z, model; positions are
 * stored scaled by 32, height snapped by func_802CE6F8, texture looked
 * up by func_802A0CC8), then D_8039C7F8 0x1C-byte records (s16 x, y, z,
 * u8 model, u8 n, s16 flag, then n 22-byte items for func_802CE9C8).
 * Flagged records also get a D_8039C800 box (+-40 units).
 * NON_MATCHING: the hand-written func_802CE9C8 reads registers this function
 * never sets for it; the NM build passes what they hold here: v0 = the last
 * call's result (func_802CE9A4 leaves &D_803F9330, func_802A0CC8 its result;
 * func_802CE9C8 leaves v0 alone), t6 = &D_8039C718[i], t9 = the old
 * D_803FB8B0, and s1 = the caller's s1 (this function never touches the
 * s-registers), which the NM build takes as an extra parameter from
 * func_802A1674 (5CB60.c). */
#ifdef NON_MATCHING
void func_8028FDA0(u8 *arg0, u8 *arg1, s32 s1) {
    s32 v0 = (s32) D_803F9330;
#else
void func_8028FDA0(u8 *arg0, u8 *arg1) {
#endif
    s32 i;

    D_8039C710 = 0;
    D_8039C7F8 = 0;
    D_8039C940 = 0;
    D_8039C944 = 0;
    D_8039C948 = 0;
    D_8039C94C = 0;
    D_8039C954 = 0;
    D_8039C958 = 0;
    D_8039C95C = 0;
    D_8039C950 = 0;
    D_803ED40C = 0;
    func_802CE9A4();
    if (arg0 != arg1) {
        D_8039C710 = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < D_8039C710; i++) {
            D_8039C550[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            D_8039C550[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            D_8039C550[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            D_8039C550[i].unk10 = *(s16 *) (arg0 + 6);
            arg0 += 8;
            D_8039C550[i].unk4 = func_802CE6F8(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk4);
            D_8039C550[i].unk0E = 0;
            D_8039C550[i].unk0C = 0;
            D_8039C550[i].unk11 = 0;
            D_8039C550[i].unk12 = 0;
            D_8039C550[i].unk20 = 0;
            D_8039C550[i].unk14 = 0;
            D_8039C550[i].unk18 = 0;
            D_8039C550[i].unk29 = 0;
            D_8039C550[i].unk30 = func_802A0CC8(D_802FDC08[D_8039C550[i].unk10].unk280, 0);
#ifdef NON_MATCHING
            v0 = (s32) D_8039C550[i].unk30;
#endif
        }
        D_8039C7F8 = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < D_8039C7F8; i++) {
            D_8039C718[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            D_8039C718[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            D_8039C718[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            D_8039C718[i].unk10 = arg0[6];
            D_8039C718[i].unk0C = D_8039C718[i].unk4 - D_802FDC08[D_8039C718[i].unk10].unk288;
            D_8039C718[i].unk11 = 0;
            D_8039C718[i].unk14 = D_803FB8B0;
#ifdef NON_MATCHING
            func_802CE9C8(arg0 + 10, arg0[7], D_8039C718[i].unk10, v0, (s32) &D_8039C718[i],
                          (s32) D_8039C718[i].unk14, s1);
#else
            func_802CE9C8(arg0 + 10, arg0[7], D_8039C718[i].unk10);
#endif
            D_8039C718[i].unk18 = D_803FB8B0;
            if (*(s16 *) (arg0 + 8) != 0) {
                D_8039C800[D_8039C940].unk0 = D_8039C718[i].unk0;
                D_8039C800[D_8039C940].unk4 = D_8039C718[i].unk4;
                D_8039C800[D_8039C940].unk8 = D_8039C718[i].unk8;
                D_8039C800[D_8039C940].unk0E = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk14 = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk1A = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk20 = D_8039C718[i].unk4 >> 5;
                D_8039C800[D_8039C940].unk0C = (D_8039C718[i].unk0 >> 5) - 40;
                D_8039C800[D_8039C940].unk10 = (D_8039C718[i].unk8 >> 5) - 40;
                D_8039C800[D_8039C940].unk12 = (D_8039C718[i].unk0 >> 5) + 40;
                D_8039C800[D_8039C940].unk16 = (D_8039C718[i].unk8 >> 5) - 40;
                D_8039C800[D_8039C940].unk18 = (D_8039C718[i].unk0 >> 5) - 40;
                D_8039C800[D_8039C940].unk1C = (D_8039C718[i].unk8 >> 5) + 40;
                D_8039C800[D_8039C940].unk1E = (D_8039C718[i].unk0 >> 5) + 40;
                D_8039C800[D_8039C940].unk22 = (D_8039C718[i].unk8 >> 5) + 40;
                D_8039C800[D_8039C940].unk26 = 0;
                D_8039C800[D_8039C940].unk27 = i;
                D_8039C940++;
            }
            arg0 = arg0[7] * 22 + arg0 + 10;
        }
    }
}

/* Per-frame update of the D_8039C550 entries (arg0 = current mode).
 * Free entries with speed (unk0C) are pushed along by func_802CE65C and
 * may latch onto a matching D_8039C718 lift within range; hitting one
 * (func_802CDF94) resets the speed from the player's movement since last
 * frame (capped by D_802FDBE0[arg0]); then collision, a heading update
 * and a slow-down. Latched entries glide toward their lift, then bounce
 * on it (h = v*t - 4t^2) until the bounce dies out. Also keeps a looping
 * sound (unk34) alive while an entry moves. The two-in-one assignments
 * below are deliberate: separate statements schedule differently. */
void func_802906C0(u8 arg0) {
    s32 i;
    s32 j;
    u8 hit;
    s32 dx;
    s32 dy;
    s32 dz;
    s16 old10;
    s16 old12;
    u8 moved;
    s32 dist;
    s32 step;
    s32 h;
    s32 hprev;
    s32 diff;
    u8 drop;
    u8 found;
    s32 tx;
    s32 tz;
    s32 floor;
    u8 near;

    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk11 == 0 && D_8039C550[i].unk12 == 0) {
            if (arg0 == 0) {
                D_8039C550[i].unk0C = 0;
            }
            if (D_8039C550[i].unk0C != 0 && func_802CE958(i) == 0) {
                func_802CE65C(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk0C, D_8039C550[i].unk0E);
                D_8039C550[i].unk0 = D_803F9320;
                D_8039C550[i].unk8 = D_803F9324;
                D_8039C550[i].unk4 = func_802CE6F8(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk4);
                D_8039C550[i].unk28 = D_803F932C;
                found = 0;
                j = 0;
                while (j < D_8039C7F8 && !found) {
                    if (D_8039C550[i].unk10 == D_8039C718[j].unk10 && D_8039C550[i].unk28 == 0 &&
                        D_8039C718[j].unk11 == 0) {
                        dx = (D_8039C550[i].unk0 - D_8039C718[j].unk0) >> 5;
                        dy = (D_8039C550[i].unk4 - D_8039C718[j].unk4) >> 5;
                        dz = (D_8039C550[i].unk8 - D_8039C718[j].unk8) >> 5;
                        dist = sqrtf(dx * dx + dy * dy + dz * dz);
                        if (dist <= D_802FDC08[D_8039C550[i].unk10].unk286) {
                            D_8039C550[i].unk24 = func_8026A610(D_8039C550[i].unk0, D_8039C550[i].unk8,
                                                                D_8039C718[j].unk0, D_8039C718[j].unk8);
                            D_8039C550[i].unk1C = D_8039C550[i].unk4;
                            D_8039C550[i].unk12 = 1;
                            D_8039C550[i].unk13 = j;
                            D_8039C718[j].unk11 = 1;
                            found = 1;
                        }
                    }
                    j++;
                }
            }
            func_802CE4F0(D_8039C550[i].unk0, D_8039C550[i].unk4, D_8039C550[i].unk8);
            hit = func_802CDF94(D_802FDC08[D_8039C550[i].unk10].radius);
            if (hit != 0) {
                if (D_803F932D == 0 && D_803F932E != 0) {
                    D_803ED40C = 1;
                }
                near = D_8036443C != 0 || arg0 == 9;
                if ((D_8039C950 == arg0 && near) || D_803F932D != 0) {
                    if (D_803F932D == 0) {
                        dx = D_803643E0 - D_8039C944,
                        dy = D_803643E4 - D_8039C948,
                        dz = D_803643E8 - D_8039C94C;
                    } else {
                        dx = D_803EF6DC - D_8039C954,
                        dy = D_803EF6E0 - D_8039C958,
                        dz = D_803EF6E4 - D_8039C95C;
                    }
                    D_8039C550[i].unk0C = sqrtf(dx * dx + dy * dy + dz * dz) + 10.0f;
                    if (D_8039C550[i].unk0C > D_802FDBE0[arg0]) {
                        D_8039C550[i].unk0C = D_802FDBE0[arg0];
                    }
                } else {
                    D_8039C550[i].unk0C = 0;
                }
            }
            if (D_8039C550[i].unk0C != 0) {
                old10 = D_803A7410, old12 = D_803A7412;
                moved = 0;
                func_802CE5BC(D_8039C550[i].unk0, D_8039C550[i].unk4, D_8039C550[i].unk8,
                              D_802FDC08[D_8039C550[i].unk10].radius, 200, D_8039C550[i].unk10);
                func_802CDB70(D_802FDC08[D_8039C550[i].unk10].radius, 0);
                if (old10 != D_803A7410 || old12 != D_803A7412) {
                    moved = 1;
                }
            } else {
                moved = 0;
            }
            drop = 0;
            if (D_803A7410 != 0 || D_803A7412 != 0xFFF) {
                if (func_8029B930() < 300) {
                    drop = 1;
                } else if (moved) {
                    D_8039C550[i].unk0E = func_802CE3B8(D_8039C550[i].unk0E);
                } else {
                    D_8039C550[i].unk0E = func_802A6F6C();
                }
            }
            if ((drop || arg0 == 0) && D_8039C550[i].unk12 == 0) {
                func_802CE880(i, D_8039C550[i].unk0, D_8039C550[i].unk4, D_8039C550[i].unk8,
                              D_802FDC08[D_8039C550[i].unk10].radius);
                D_8039C550[i].unk0C = 0;
            } else {
                func_802CE90C(i);
            }
            if (D_8039C550[i].unk0C > 0) {
                D_8039C550[i].unk0C -= 8;
            } else {
                D_8039C550[i].unk0C = 0;
            }
        }
        if (D_8039C550[i].unk12 != 0) {
            tx = D_8039C718[D_8039C550[i].unk13].unk0;
            tz = D_8039C718[D_8039C550[i].unk13].unk8;
            if (D_8039C550[i].unk0 != tx || D_8039C550[i].unk8 != tz) {
                dist = func_8026A610(D_8039C550[i].unk0, D_8039C550[i].unk8, tx, tz);
                if (dist != 0) {
                    dx = tx - D_8039C550[i].unk0;
                    if (dx > 0) {
                        step = dx;
                    } else {
                        step = -dx;
                    }
                    step <<= 10;
                    step /= dist;
                    step *= D_8039C550[i].unk0C;
                    step >>= 10;
                    if (dx < 0) {
                        step = -step;
                    }
                    D_8039C550[i].unk0 += step;
                    dz = tz - D_8039C550[i].unk8;
                    if (dz > 0) {
                        step = dz;
                    } else {
                        step = -dz;
                    }
                    step <<= 10;
                    step /= dist;
                    step *= D_8039C550[i].unk0C;
                    step >>= 10;
                    if (dz < 0) {
                        step = -step;
                    }
                    D_8039C550[i].unk8 += step;
                    dist = func_8026A610(D_8039C550[i].unk0, D_8039C550[i].unk8, tx, tz);
                    if (dist >= D_8039C550[i].unk24) {
                        D_8039C550[i].unk0 = tx;
                        D_8039C550[i].unk8 = tz;
                    } else {
                        D_8039C550[i].unk24 = dist;
                    }
                }
            } else {
                if (D_8039C550[i].unk4 == D_8039C550[i].unk1C) {
                    func_80260650(D_80367738, 0x64, NULL);
                }
                floor = D_8039C718[D_8039C550[i].unk13].unk0C;
                h = D_8039C550[i].unk14 * D_8039C550[i].unk18 + (D_8039C550[i].unk18 * -4) * D_8039C550[i].unk18;
                D_8039C550[i].unk4 = D_8039C550[i].unk1C + h;
                if (D_8039C550[i].unk4 < floor) {
                    D_8039C550[i].unk4 = floor;
                    hprev = D_8039C550[i].unk14 * (D_8039C550[i].unk18 - 1) + ((D_8039C550[i].unk18 - 1) * -4) * (D_8039C550[i].unk18 - 1);
                    diff = h - hprev;
                    if (diff < 0) {
                        diff = -diff;
                    }
                    if (diff < 20) {
                        D_8039C550[i].unk12 = 0;
                        D_8039C550[i].unk11 = 1;
                        func_802CEA68(D_8039C718[D_8039C550[i].unk13].unk14, D_8039C718[D_8039C550[i].unk13].unk18);
                        func_80291724(D_8039C550[i].unk13);
                    } else {
                        D_8039C550[i].unk14 = diff >> 1;
                        D_8039C550[i].unk18 = 0;
                        D_8039C550[i].unk1C = floor;
                        func_80260650(D_80367738, 0xC, NULL);
                    }
                } else {
                    D_8039C550[i].unk18++;
                }
            }
        }
        if (D_8039C550[i].unk0C != 0 && D_8039C550[i].unk12 == 0 && D_8039C550[i].unk11 == 0 &&
            D_8039C550[i].unk34 == 0) {
            func_80260650(D_80367738, 7, &D_8039C550[i].unk34);
        }
        if (D_8039C550[i].unk34 != 0 &&
            (D_8039C550[i].unk0C == 0 || D_8039C550[i].unk12 != 0 || D_8039C550[i].unk11 != 0)) {
            func_802608C8(D_8039C550[i].unk34);
        }
    }
    D_8039C944 = D_803643E0;
    D_8039C948 = D_803643E4;
    D_8039C94C = D_803643E8;
    D_8039C950 = arg0;
    D_8039C954 = D_803EF6DC;
    D_8039C958 = D_803EF6E0;
    D_8039C95C = D_803EF6E4;
}

/* Flag (unk26) the first D_8039C800 entry whose unk27 is arg0. */
void func_80291724(s32 arg0) {
    u8 found;
    s32 i;

    found = 0;
    for (i = 0; i < D_8039C940;) {
        if (D_8039C800[i].unk27 == arg0) {
            D_8039C800[i].unk26 = 1;
            found = 1;
        } else {
            i++;
        }
        if (found) {
            break;
        }
    }
}

/* Draw every entry: a translated, textured copy of its model. */
void func_802917B0(Gfx **gdl, Dyn4B5E0 *dyn) {
    Gfx *gfx;
    s32 i;

    gfx = *gdl;
    if (D_8039C710 > 0) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
        gDPSetCombineMode(gfx++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
        gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    }
    for (i = 0; i < D_8039C710; i++) {
        guTranslate(&dyn->mtx[i], D_8039C550[i].unk0 / 32.0f, D_8039C550[i].unk4 / 32.0f,
                    D_8039C550[i].unk8 / 32.0f);
        gSPMatrix(gfx++, i * sizeof(Mtx) + 0xB00 + (u32) D_02000000, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gDPLoadTextureBlock(gfx++, D_8039C550[i].unk30 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                            D_802FDC08[D_8039C550[i].unk10].width, D_802FDC08[D_8039C550[i].unk10].height, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                            G_TX_NOLOD, G_TX_NOLOD);
        gSPDisplayList(gfx++, osVirtualToPhysical(&D_802FDC08[D_8039C550[i].unk10]));
        gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    }
    gDPPipeSync(gfx++);
    *gdl = gfx;
}

/* For entries tagged arg0 (unk28): clear unk0C, set unk29 and call
 * func_802AACD4 on their position. */
void func_80291ED8(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk28 == arg0) {
            D_8039C550[i].unk0C = 0;
            *(&D_8039C579 + i * 0x38) = 1;
            func_802AACD4(arg0, D_8039C550[i].unk0, D_8039C550[i].unk8, &D_8039C550[i].unk2A,
                          &D_8039C550[i].unk2C);
        }
    }
}

/* Update entries with unk29 set: move them (func_802AAE1C), then snap
 * their height with func_802CE6F8. */
void func_80291FAC(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk29 != 0) {
            func_802AAE1C(arg0, D_8039C550[i].unk2A, D_8039C550[i].unk2C, &D_8039C550[i],
                          &D_8039C550[i].unk8);
            D_8039C550[i].unk4 = func_802CE6F8(D_8039C550[i].unk0, D_8039C550[i].unk8, D_8039C550[i].unk4);
        }
    }
}

/* Clear every entry's unk29. */
void func_80292084(void) {
    s32 i;

    for (i = 0; i < D_8039C710; i++) {
        D_8039C550[i].unk29 = 0;
    }
}

/* Set D_803A7424 if the point (arg0, arg1, arg2) is within arg3 of any
 * idle entry's model radius (all in 1/32 units). */
void func_802920DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i;
    s32 dist;

    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
    for (i = 0; i < D_8039C710; i++) {
        if (D_8039C550[i].unk11 == 0 && D_8039C550[i].unk29 == 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039C550[i].unk0 >> 5, D_8039C550[i].unk4 >> 5,
                                 D_8039C550[i].unk8 >> 5);
            if (dist <= (D_802FDC08[D_8039C550[i].unk10].radius >> 5) + arg3) {
                D_803A7424 = 1;
            }
        }
    }
}
