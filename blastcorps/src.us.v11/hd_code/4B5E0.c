#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#define LEGACY_D_803FB8B0
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_02000000 ((u8 *) D_02000000)
#define gBoxTargets ((Entry4B5E0b *) gBoxTargets)
#ifdef NON_MATCHING
#define D_803FB8B0 (*(s32 *) &D_803FB8B0)
#endif
/* end of views */

/* gBlocks: gBlockCount 0x38-byte entries (positions in 1/32 units). */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s16 unk0C;
    /* 0x0E */ s16 unk0E;
    /* 0x10 */ u8 unk10; /* model index into D_802FDC08 */
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12; /* riding a gBlockSlots lift */
    /* 0x13 */ u8 unk13; /* which gBlockSlots entry */
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

/* gBlockSlots: gBlockSlotCount 0x1C-byte entries. */
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

/* gBoxTargets: gBoxTargetCount 0x28-byte entries: a position and a box of
 * 1/32-unit points around it, plus the gBlockSlots index it came from. */
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
    /* 0x280 */ s16 unk280; /* texture id for tex_get */
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

extern Entry4B5E0 gBlocks[];
extern u8 D_8039C579;
extern s32 gBlockCount;
extern Entry4B5E0c gBlockSlots[];
extern s32 gBlockSlotCount;
extern s32 D_8039C944;
extern s32 D_8039C948;
extern s32 D_8039C94C;
extern u8 D_8039C950;
extern s32 D_8039C954;
extern s32 D_8039C958;
extern s32 D_8039C95C;
#ifndef NON_MATCHING
extern s32 D_803FB8B0;
#endif
extern Model4B5E0 D_802FDC08[];
extern s16 D_802FDBE0[];
#ifndef NON_MATCHING
extern s32 D_80367738;
#endif

void block_fill_target(s32);

/* Load this file's objects from level data: reset the counters, then
 * read gBlockCount 0x38-byte entries (s16 x, y, z, model; positions are
 * stored scaled by 32, height snapped by coll_ground_height, texture looked
 * up by tex_get), then gBlockSlotCount 0x1C-byte records (s16 x, y, z,
 * u8 model, u8 n, s16 flag, then n 22-byte items for coll_add_dyn_tris).
 * Flagged records also get a gBoxTargets box (+-40 units).
 * NON_MATCHING: the hand-written coll_add_dyn_tris reads registers this function
 * never sets for it; the NM build passes what they hold here: v0 = the last
 * call's result (coll_reset_dyn_tris leaves &D_803F9330, tex_get its result;
 * coll_add_dyn_tris leaves v0 alone), t6 = &gBlockSlots[i], t9 = the old
 * D_803FB8B0, and s1 = the caller's s1 (this function never touches the
 * s-registers), which the NM build takes as an extra parameter from
 * initlevel (5CB60.c). */
#ifdef NON_MATCHING
/* The push blocks, their slots, the target boxes (0x8028FDA0). */
void block_spawn_all(u8 *arg0, u8 *arg1, s32 s1) {
    s32 v0 = (s32) D_803F9330;
#else
void block_spawn_all(u8 *arg0, u8 *arg1) {
#endif
    s32 i;

    gBlockCount = 0;
    gBlockSlotCount = 0;
    gBoxTargetCount = 0;
    D_8039C944 = 0;
    D_8039C948 = 0;
    D_8039C94C = 0;
    D_8039C954 = 0;
    D_8039C958 = 0;
    D_8039C95C = 0;
    D_8039C950 = 0;
    gVehSlowed = 0;
    coll_reset_dyn_tris();
    if (arg0 != arg1) {
        gBlockCount = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < gBlockCount; i++) {
            gBlocks[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            gBlocks[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            gBlocks[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            gBlocks[i].unk10 = *(s16 *) (arg0 + 6);
            arg0 += 8;
            gBlocks[i].unk4 = coll_ground_height(gBlocks[i].unk0, gBlocks[i].unk8, gBlocks[i].unk4);
            gBlocks[i].unk0E = 0;
            gBlocks[i].unk0C = 0;
            gBlocks[i].unk11 = 0;
            gBlocks[i].unk12 = 0;
            gBlocks[i].unk20 = 0;
            gBlocks[i].unk14 = 0;
            gBlocks[i].unk18 = 0;
            gBlocks[i].unk29 = 0;
            gBlocks[i].unk30 = tex_get(D_802FDC08[gBlocks[i].unk10].unk280, 0);
#ifdef NON_MATCHING
            v0 = (s32) gBlocks[i].unk30;
#endif
        }
        gBlockSlotCount = *(s16 *) arg0;
        arg0 += 2;
        for (i = 0; i < gBlockSlotCount; i++) {
            gBlockSlots[i].unk0 = *(s16 *) (arg0 + 0) << 5;
            gBlockSlots[i].unk4 = *(s16 *) (arg0 + 2) << 5;
            gBlockSlots[i].unk8 = *(s16 *) (arg0 + 4) << 5;
            gBlockSlots[i].unk10 = arg0[6];
            gBlockSlots[i].unk0C = gBlockSlots[i].unk4 - D_802FDC08[gBlockSlots[i].unk10].unk288;
            gBlockSlots[i].unk11 = 0;
            gBlockSlots[i].unk14 = D_803FB8B0;
#ifdef NON_MATCHING
            coll_add_dyn_tris(arg0 + 10, arg0[7], gBlockSlots[i].unk10, v0, (s32) &gBlockSlots[i],
                          (s32) gBlockSlots[i].unk14, s1);
#else
            coll_add_dyn_tris(arg0 + 10, arg0[7], gBlockSlots[i].unk10);
#endif
            gBlockSlots[i].unk18 = D_803FB8B0;
            if (*(s16 *) (arg0 + 8) != 0) {
                gBoxTargets[gBoxTargetCount].unk0 = gBlockSlots[i].unk0;
                gBoxTargets[gBoxTargetCount].unk4 = gBlockSlots[i].unk4;
                gBoxTargets[gBoxTargetCount].unk8 = gBlockSlots[i].unk8;
                gBoxTargets[gBoxTargetCount].unk0E = gBlockSlots[i].unk4 >> 5;
                gBoxTargets[gBoxTargetCount].unk14 = gBlockSlots[i].unk4 >> 5;
                gBoxTargets[gBoxTargetCount].unk1A = gBlockSlots[i].unk4 >> 5;
                gBoxTargets[gBoxTargetCount].unk20 = gBlockSlots[i].unk4 >> 5;
                gBoxTargets[gBoxTargetCount].unk0C = (gBlockSlots[i].unk0 >> 5) - 40;
                gBoxTargets[gBoxTargetCount].unk10 = (gBlockSlots[i].unk8 >> 5) - 40;
                gBoxTargets[gBoxTargetCount].unk12 = (gBlockSlots[i].unk0 >> 5) + 40;
                gBoxTargets[gBoxTargetCount].unk16 = (gBlockSlots[i].unk8 >> 5) - 40;
                gBoxTargets[gBoxTargetCount].unk18 = (gBlockSlots[i].unk0 >> 5) - 40;
                gBoxTargets[gBoxTargetCount].unk1C = (gBlockSlots[i].unk8 >> 5) + 40;
                gBoxTargets[gBoxTargetCount].unk1E = (gBlockSlots[i].unk0 >> 5) + 40;
                gBoxTargets[gBoxTargetCount].unk22 = (gBlockSlots[i].unk8 >> 5) + 40;
                gBoxTargets[gBoxTargetCount].unk26 = 0;
                gBoxTargets[gBoxTargetCount].unk27 = i;
                gBoxTargetCount++;
            }
            arg0 = arg0[7] * 22 + arg0 + 10;
        }
    }
}

/* Push, settle into a slot, bounce (0x802906C0).
 * Per-frame update of the gBlocks entries (arg0 = current mode).
 * Free entries with speed (unk0C) are pushed along by coll_step_xz and
 * may latch onto a matching gBlockSlots lift within range; hitting one
 * (coll_probe_vehicles) resets the speed from the player's movement since last
 * frame (capped by D_802FDBE0[arg0]); then collision, a heading update
 * and a slow-down. Latched entries glide toward their lift, then bounce
 * on it (h = v*t - 4t^2) until the bounce dies out. Also keeps a looping
 * sound (unk34) alive while an entry moves. The two-in-one assignments
 * below are deliberate: separate statements schedule differently. */
void block_update(u8 arg0) {
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

    for (i = 0; i < gBlockCount; i++) {
        if (gBlocks[i].unk11 == 0 && gBlocks[i].unk12 == 0) {
            if (arg0 == 0) {
                gBlocks[i].unk0C = 0;
            }
            if (gBlocks[i].unk0C != 0 && radar_has_marker(i) == 0) {
                coll_step_xz(gBlocks[i].unk0, gBlocks[i].unk8, gBlocks[i].unk0C, gBlocks[i].unk0E);
                gBlocks[i].unk0 = D_803F9320;
                gBlocks[i].unk8 = D_803F9324;
                gBlocks[i].unk4 = coll_ground_height(gBlocks[i].unk0, gBlocks[i].unk8, gBlocks[i].unk4);
                gBlocks[i].unk28 = gGroundTriId;
                found = 0;
                j = 0;
                while (j < gBlockSlotCount && !found) {
                    if (gBlocks[i].unk10 == gBlockSlots[j].unk10 && gBlocks[i].unk28 == 0 &&
                        gBlockSlots[j].unk11 == 0) {
                        dx = (gBlocks[i].unk0 - gBlockSlots[j].unk0) >> 5;
                        dy = (gBlocks[i].unk4 - gBlockSlots[j].unk4) >> 5;
                        dz = (gBlocks[i].unk8 - gBlockSlots[j].unk8) >> 5;
                        dist = sqrtf(dx * dx + dy * dy + dz * dz);
                        if (dist <= D_802FDC08[gBlocks[i].unk10].unk286) {
                            gBlocks[i].unk24 = util_dist2d(gBlocks[i].unk0, gBlocks[i].unk8,
                                                                gBlockSlots[j].unk0, gBlockSlots[j].unk8);
                            gBlocks[i].unk1C = gBlocks[i].unk4;
                            gBlocks[i].unk12 = 1;
                            gBlocks[i].unk13 = j;
                            gBlockSlots[j].unk11 = 1;
                            found = 1;
                        }
                    }
                    j++;
                }
            }
            coll_begin_probe(gBlocks[i].unk0, gBlocks[i].unk4, gBlocks[i].unk8);
            hit = coll_probe_vehicles(D_802FDC08[gBlocks[i].unk10].radius);
            if (hit != 0) {
                if (gProbeHitCarrier == 0 && gProbeHitPlayer != 0) {
                    gVehSlowed = 1;
                }
                near = D_8036443C != 0 || arg0 == 9;
                if ((D_8039C950 == arg0 && near) || gProbeHitCarrier != 0) {
                    if (gProbeHitCarrier == 0) {
                        dx = gPlayerX - D_8039C944,
                        dy = gPlayerY - D_8039C948,
                        dz = gPlayerZ - D_8039C94C;
                    } else {
                        dx = gCarrierX - D_8039C954,
                        dy = gCarrierY - D_8039C958,
                        dz = gCarrierZ - D_8039C95C;
                    }
                    gBlocks[i].unk0C = sqrtf(dx * dx + dy * dy + dz * dz) + 10.0f;
                    if (gBlocks[i].unk0C > D_802FDBE0[arg0]) {
                        gBlocks[i].unk0C = D_802FDBE0[arg0];
                    }
                } else {
                    gBlocks[i].unk0C = 0;
                }
            }
            if (gBlocks[i].unk0C != 0) {
                old10 = gFreeArcStart, old12 = gFreeArcEnd;
                moved = 0;
                coll_probe_tris(gBlocks[i].unk0, gBlocks[i].unk4, gBlocks[i].unk8,
                              D_802FDC08[gBlocks[i].unk10].radius, 200, gBlocks[i].unk10);
                coll_probe_objects(D_802FDC08[gBlocks[i].unk10].radius, 0);
                if (old10 != gFreeArcStart || old12 != gFreeArcEnd) {
                    moved = 1;
                }
            } else {
                moved = 0;
            }
            drop = 0;
            if (gFreeArcStart != 0 || gFreeArcEnd != 0xFFF) {
                if (veh_free_arc_length() < 300) {
                    drop = 1;
                } else if (moved) {
                    gBlocks[i].unk0E = coll_heading_in_ring(gBlocks[i].unk0E);
                } else {
                    gBlocks[i].unk0E = veh_free_arc_mid();
                }
            }
            if ((drop || arg0 == 0) && gBlocks[i].unk12 == 0) {
                radar_set_marker(i, gBlocks[i].unk0, gBlocks[i].unk4, gBlocks[i].unk8,
                              D_802FDC08[gBlocks[i].unk10].radius);
                gBlocks[i].unk0C = 0;
            } else {
                radar_remove_marker(i);
            }
            if (gBlocks[i].unk0C > 0) {
                gBlocks[i].unk0C -= 8;
            } else {
                gBlocks[i].unk0C = 0;
            }
        }
        if (gBlocks[i].unk12 != 0) {
            tx = gBlockSlots[gBlocks[i].unk13].unk0;
            tz = gBlockSlots[gBlocks[i].unk13].unk8;
            if (gBlocks[i].unk0 != tx || gBlocks[i].unk8 != tz) {
                dist = util_dist2d(gBlocks[i].unk0, gBlocks[i].unk8, tx, tz);
                if (dist != 0) {
                    dx = tx - gBlocks[i].unk0;
                    if (dx > 0) {
                        step = dx;
                    } else {
                        step = -dx;
                    }
                    step <<= 10;
                    step /= dist;
                    step *= gBlocks[i].unk0C;
                    step >>= 10;
                    if (dx < 0) {
                        step = -step;
                    }
                    gBlocks[i].unk0 += step;
                    dz = tz - gBlocks[i].unk8;
                    if (dz > 0) {
                        step = dz;
                    } else {
                        step = -dz;
                    }
                    step <<= 10;
                    step /= dist;
                    step *= gBlocks[i].unk0C;
                    step >>= 10;
                    if (dz < 0) {
                        step = -step;
                    }
                    gBlocks[i].unk8 += step;
                    dist = util_dist2d(gBlocks[i].unk0, gBlocks[i].unk8, tx, tz);
                    if (dist >= gBlocks[i].unk24) {
                        gBlocks[i].unk0 = tx;
                        gBlocks[i].unk8 = tz;
                    } else {
                        gBlocks[i].unk24 = dist;
                    }
                }
            } else {
                if (gBlocks[i].unk4 == gBlocks[i].unk1C) {
                    func_80260650(D_80367738, 0x64, NULL);
                }
                floor = gBlockSlots[gBlocks[i].unk13].unk0C;
                h = gBlocks[i].unk14 * gBlocks[i].unk18 + (gBlocks[i].unk18 * -4) * gBlocks[i].unk18;
                gBlocks[i].unk4 = gBlocks[i].unk1C + h;
                if (gBlocks[i].unk4 < floor) {
                    gBlocks[i].unk4 = floor;
                    hprev = gBlocks[i].unk14 * (gBlocks[i].unk18 - 1) + ((gBlocks[i].unk18 - 1) * -4) * (gBlocks[i].unk18 - 1);
                    diff = h - hprev;
                    if (diff < 0) {
                        diff = -diff;
                    }
                    if (diff < 20) {
                        gBlocks[i].unk12 = 0;
                        gBlocks[i].unk11 = 1;
                        coll_disable_tris((u8 *) N64_IPTR((gBlockSlots[gBlocks[i].unk13].unk14)), (u8 *) N64_IPTR((gBlockSlots[gBlocks[i].unk13].unk18)));
                        block_fill_target(gBlocks[i].unk13);
                    } else {
                        gBlocks[i].unk14 = diff >> 1;
                        gBlocks[i].unk18 = 0;
                        gBlocks[i].unk1C = floor;
                        func_80260650(D_80367738, 0xC, NULL);
                    }
                } else {
                    gBlocks[i].unk18++;
                }
            }
        }
        if (gBlocks[i].unk0C != 0 && gBlocks[i].unk12 == 0 && gBlocks[i].unk11 == 0 &&
            gBlocks[i].unk34 == 0) {
            func_80260650(D_80367738, 7, &gBlocks[i].unk34);
        }
        if (gBlocks[i].unk34 != 0 &&
            (gBlocks[i].unk0C == 0 || gBlocks[i].unk12 != 0 || gBlocks[i].unk11 != 0)) {
            func_802608C8((void *) N64_IPTR((gBlocks[i].unk34)));
        }
    }
    D_8039C944 = gPlayerX;
    D_8039C948 = gPlayerY;
    D_8039C94C = gPlayerZ;
    D_8039C950 = arg0;
    D_8039C954 = gCarrierX;
    D_8039C958 = gCarrierY;
    D_8039C95C = gCarrierZ;
}

/* Marks the slot's target box done (0x80291724).
 * Flag (unk26) the first gBoxTargets entry whose unk27 is arg0. */
void block_fill_target(s32 arg0) {
    u8 found;
    s32 i;

    found = 0;
    for (i = 0; i < gBoxTargetCount;) {
        if (gBoxTargets[i].unk27 == arg0) {
            gBoxTargets[i].unk26 = 1;
            found = 1;
        } else {
            i++;
        }
        if (found) {
            break;
        }
    }
}

/* Draws the push blocks (0x802917B0).
 * Draw every entry: a translated, textured copy of its model. */
void block_draw(Gfx **gdl, Dyn4B5E0 *dyn) {
    Gfx *gfx;
    s32 i;

    gfx = *gdl;
    if (gBlockCount > 0) {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_1CYCLE);
        gDPSetRenderMode(gfx++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
        gDPSetCombineMode(gfx++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gSPClearGeometryMode(gfx++, 0xFFFFFFFF);
        gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK);
        gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    }
    for (i = 0; i < gBlockCount; i++) {
        guTranslate(&dyn->mtx[i], gBlocks[i].unk0 / 32.0f, gBlocks[i].unk4 / 32.0f,
                    gBlocks[i].unk8 / 32.0f);
        gSPMatrix(gfx++, i * sizeof(Mtx) + 0xB00 + (u32) D_02000000, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gDPLoadTextureBlock(gfx++, gBlocks[i].unk30 - 0x80000000, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                            D_802FDC08[gBlocks[i].unk10].width, D_802FDC08[gBlocks[i].unk10].height, 0,
                            G_TX_MIRROR | G_TX_CLAMP, G_TX_MIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                            G_TX_NOLOD, G_TX_NOLOD);
        gSPDisplayList(gfx++, osVirtualToPhysical(&D_802FDC08[gBlocks[i].unk10]));
        gSPPopMatrix(gfx++, G_MTX_MODELVIEW);
    }
    gDPPipeSync(gfx++);
    *gdl = gfx;
}

/* Blocks standing on a vehicle ride on it (0x80291ED8).
 * For entries tagged arg0 (unk28): clear unk0C, set unk29 and call
 * veh_deck_to_local_c on their position. */
void block_attach_to_vehicle(u8 arg0) {
    s32 i;

    for (i = 0; i < gBlockCount; i++) {
        if (gBlocks[i].unk28 == arg0) {
            gBlocks[i].unk0C = 0;
            *(&D_8039C579 + i * 0x38) = 1;
            veh_deck_to_local_c(arg0, gBlocks[i].unk0, gBlocks[i].unk8, &gBlocks[i].unk2A,
                          &gBlocks[i].unk2C);
        }
    }
}

/* Carried blocks move with the vehicle (0x80291FAC).
 * Update entries with unk29 set: move them (veh_deck_to_world_c), then snap
 * their height with coll_ground_height. */
void block_ride_vehicle(u8 arg0) {
    s32 i;

    for (i = 0; i < gBlockCount; i++) {
        if (gBlocks[i].unk29 != 0) {
            veh_deck_to_world_c(arg0, gBlocks[i].unk2A, gBlocks[i].unk2C, (s32 *) &gBlocks[i],
                          &gBlocks[i].unk8);
            gBlocks[i].unk4 = coll_ground_height(gBlocks[i].unk0, gBlocks[i].unk8, gBlocks[i].unk4);
        }
    }
}

/* Unpins all blocks (0x80292084).
 * Clear every entry's unk29. */
void block_detach_all(void) {
    s32 i;

    for (i = 0; i < gBlockCount; i++) {
        gBlocks[i].unk29 = 0;
    }
}

/* GCollHit if a sphere touches a block (0x802920DC).
 * Set gCollHit if the point (arg0, arg1, arg2) is within arg3 of any
 * idle entry's model radius (all in 1/32 units). */
void block_test_sphere(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i;
    s32 dist;

    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
    for (i = 0; i < gBlockCount; i++) {
        if (gBlocks[i].unk11 == 0 && gBlocks[i].unk29 == 0) {
            dist = util_dist3d(arg0, arg1, arg2, gBlocks[i].unk0 >> 5, gBlocks[i].unk4 >> 5,
                                 gBlocks[i].unk8 >> 5);
            if (dist <= (D_802FDC08[gBlocks[i].unk10].radius >> 5) + arg3) {
                gCollHit = 1;
            }
        }
    }
}
