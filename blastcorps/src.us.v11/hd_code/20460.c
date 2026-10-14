#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gSfxBank
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_02000000 ((u8 *) D_02000000)
/* end of views */

/* The people: a pool of 20 little people who run out of a destroyed object
 * (people_spawn, from obj_spawn_people), wait inside a box, walk away from
 * the player and are picked up by the BCT chopper (type 0xFE; drawn only
 * while gHaveChopper); its record type was called Debris before. The audio
 * manager that follows is a separate file, 22EE0.c (audio.c). */

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 unk6; /* run-out target x */
    /* 0x08 */ s16 unk8; /* run-out target z */
    /* 0x0A */ s16 unkA; /* home box centre x */
    /* 0x0C */ s16 unkC; /* home box centre z */
    /* 0x0E */ s16 unkE; /* home box half size */
    /* 0x10 */ s16 unk10; /* half size of the area where the player makes them flee */
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 state; /* 0 free, 5 running out, 1 idle, 4 walking away, 2 chosen for pickup, 3 boarding */
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18; /* heading, 0..0xFFF */
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C; /* speed */
    /* 0x1D */ u8 pad1D[3];
    /* 0x20 */ f32 unk20; /* last distance to target */
} Person; /* size 0x24 */

extern Person gPeople[20];
extern s16 gPeopleStepX;
extern s16 gPeopleStepZ;
extern s32 gChopperPickupTimer;
extern s32 gChopperPickupIndex;
extern s32 gChopperRetargetX;
#ifndef NON_MATCHING
extern s32 gSfxBank;
#endif

s32 people_player_near(s32 arg0);
void people_step(s32 arg0);

/* Empties the people pool; the released count from the status or 0 (0x80264C20).
 * Clear the pool and reset the timers */
void people_init(s32 arg0) {
    s32 i;

    for (i = 0; i < 20; i++) {
        gPeople[i].state = 0;
    }
    gRandSeed = osGetCount();
    gChopperPickupTimer = 99999999;
    if (arg0) {
        D_8036EA79 = gPeopleRestored;
    } else {
        D_8036EA79 = 0;
    }
}

/* N people run out of a destroyed object at (x, y, z) (0x80264CB4).
 * Spawn arg5 people around (arg0, arg2) inside a box of half size arg3 */
void people_spawn(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, s32 arg5) {
    s32 pad;
    s32 n;
    s32 i;
    u8 found;
    s16 r;

    i = 0;
    n = 0;
    D_8036EA79 += arg5;
    if (arg5 != 0 && gPaused == 0) {
        yoshi_show_hint(0x48);
    }
    if (arg3 > 200) {
        arg3 = 200;
    }
    while (n < arg5 && i < 20) {
        if (n == 2) {
            snd_play(gSfxBank, SFX_PEOPLE_SCREAM, 0);
        }
        found = 0;
        while (i < 20 && !found) {
            if (gPeople[i].state == 0) {
                found = 1;
            } else {
                i++;
            }
        }
        if (found) {
            PORT_ISPAWN(PORT_ITAG_PEOPLE, i);
            gPeople[i].x = arg0;
            gPeople[i].z = arg2;
            gPeople[i].y = arg1;
            r = arg3 / 5;
            gPeople[i].unkA = util_rand2_range(-r, r) + (arg0 - arg3);
            gPeople[i].unkC = util_rand2_range(-r, r) + (arg2 + arg3);
            gPeople[i].unk10 = arg3 * 3 / 2;
            gPeople[i].unkE = arg3;
            gPeople[i].unk6 = gPeople[i].unkA + util_rand2_range(-arg3, arg3);
            gPeople[i].unk8 = gPeople[i].unkC + util_rand2_range(-arg3, arg3);
            gPeople[i].unk13 = 0;
            gPeople[i].unk14 = 0;
            gPeople[i].unk16 = util_rand2_range(-500, 500);
            gPeople[i].unk12 = arg4;
            gPeople[i].unk1C = 0;
            gPeople[i].state = 5;
            gPeople[i].unk18 = 4000;
            gPeople[i].unk20 = 100000000.0f;
            gPeople[i].unk1B = 0;
        }
        i++, n++;
    }
}

void people_flee(void);
void people_run_out(void);
void people_dispatch_chopper(void);

/* Per frame: flee, run out, chopper pickup (0x8026510C). */
void people_update(void) {
    people_flee();
    people_run_out();
    people_dispatch_chopper();
}

/* People running out (state 5) reach their spot, then idle (0x8026513C).
 * Move homing debris (state 5) 5 units a frame toward its target; it lands
 * (state 1) once it arrives or starts moving away */
void people_run_out(void) {
    s16 dx;
    s16 dz;
    s16 adx;
    s16 adz;
    f32 dist;
    s16 sx;
    s16 sz;
    s32 i;

    for (i = 0; i < 20; i++) {
        if (gPeople[i].state == 5) {
            dx = gPeople[i].unk6 - gPeople[i].x;
            if (dx >= 0) {
                adx = dx;
            } else {
                adx = -dx;
            }
            dz = gPeople[i].unk8 - gPeople[i].z;
            if (dz >= 0) {
                adz = dz;
            } else {
                adz = -dz;
            }
            dist = util_dist2d(gPeople[i].unk6, gPeople[i].unk8, gPeople[i].x, gPeople[i].z);
            if (dist < 1.0 || gPeople[i].unk20 < dist) {
                gPeople[i].state = 1;
            } else {
                gPeople[i].unk20 = dist;
                sx = adx / dist * 5.0f;
                sz = adz / dist * 5.0f;
                if (dx >= 0) {
                    gPeople[i].x = gPeople[i].x + sx;
                } else {
                    gPeople[i].x -= sx;
                }
                if (dz >= 0) {
                    gPeople[i].z += sz;
                } else {
                    gPeople[i].z -= sz;
                }
            }
        }
    }
}

/* While the player is near, a person walks away from the player (0x80265428).
 * Wander: step each active piece along its heading, clamped to its box,
 * preferring the direction away from the player */
void people_flee(void) {
    s32 i;
    s32 pad;
    s32 x;
    s32 z;
    s32 x2;
    s32 z2;
    s32 d1;
    s32 d2;

    for (i = 0; i < 20; i++) {
        if ((gPeople[i].state == 1 || gPeople[i].state == 4 || gPeople[i].state == 2) && people_player_near(i)) {
            people_step(i);
            x = gPeople[i].x + gPeopleStepX;
            z = gPeople[i].z + gPeopleStepZ;
            x2 = gPeople[i].x - gPeopleStepX;
            z2 = gPeople[i].z - gPeopleStepZ;
            if (x < gPeople[i].unkA - gPeople[i].unkE) {
                x = gPeople[i].unkA - gPeople[i].unkE;
            }
            if (z < gPeople[i].unkC - gPeople[i].unkE) {
                z = gPeople[i].unkC - gPeople[i].unkE;
            }
            if (x >= gPeople[i].unkA + gPeople[i].unkE) {
                x = gPeople[i].unkA + gPeople[i].unkE - 1;
            }
            if (z >= gPeople[i].unkC + gPeople[i].unkE) {
                z = gPeople[i].unkC + gPeople[i].unkE - 1;
            }
            if (x2 < gPeople[i].unkA - gPeople[i].unkE) {
                x2 = gPeople[i].unkA - gPeople[i].unkE;
            }
            if (z2 < gPeople[i].unkC - gPeople[i].unkE) {
                z2 = gPeople[i].unkC - gPeople[i].unkE;
            }
            if (x2 >= gPeople[i].unkA + gPeople[i].unkE) {
                x2 = gPeople[i].unkA + gPeople[i].unkE - 1;
            }
            if (z2 >= gPeople[i].unkC + gPeople[i].unkE) {
                z2 = gPeople[i].unkC + gPeople[i].unkE - 1;
            }
            d1 = util_dist2d(gPlayerX >> 5, gPlayerZ >> 5, x, z);
            d2 = util_dist2d(gPlayerX >> 5, gPlayerZ >> 5, x2, z2);
            if (gPeople[i].unk1B) {
                if (gPeople[i].unk1A) {
                    d2 = 0;
                } else {
                    d1 = 0;
                }
                gPeople[i].unk1B--;
            }
            if (d1 < d2) {
                gPeople[i].x = x2;
                gPeople[i].z = z2;
                gPeople[i].unk18 += 0x800;
                if (gPeople[i].unk18 >= 0x1000) {
                    gPeople[i].unk18 -= 0xFFF;
                }
                if (gPeople[i].unk1A) {
                    gPeople[i].unk1B = 5;
                }
                gPeople[i].unk1A = 0;
            } else {
                gPeople[i].x = x;
                gPeople[i].z = z;
                if (!gPeople[i].unk1A) {
                    gPeople[i].unk1B = 5;
                }
                gPeople[i].unk1A = 1;
            }
            if (i == 1 && gPeople[i].state != 4) {
                snd_play(gSfxBank, SFX_PEOPLE_SCREAM, 0);
            }
            if (gPeople[i].state == 2) {
                gChopperMode = 6;
            }
            gPeople[i].state = 4;
        } else if (gPeople[i].state == 4) {
            gPeople[i].state = 1;
        }
    }
}

/* 1 if the player is inside person i's area (0x80265A0C).
 * Is the player (gPlayerX/Z, >> 5) inside piece arg0's area? */
s32 people_player_near(s32 arg0) {
    s16 b3;
    s16 b2;
    s16 b1;
    s16 b0;

    b1 = gPeople[arg0].unkA - gPeople[arg0].unk10;
    b0 = gPeople[arg0].unkC - gPeople[arg0].unk10;
    b3 = gPlayerX >> 5, b2 = gPlayerZ >> 5;
    if (b3 < b1 || b2 < b0) {
        return 0;
    }
    b1 = gPeople[arg0].unkA + gPeople[arg0].unk10;
    b0 = gPeople[arg0].unkC + gPeople[arg0].unk10;
    if (b3 >= b1 || b2 >= b0) {
        return 0;
    }
    return 1;
}

/* Person i's heading and this frame's step (0x80265B7C).
 * Turn piece arg0 and work out this frame's step (gPeopleStepX/Z) from its
 * heading and a speed that ramps toward D_8036443C / 40 */
void people_step(s32 arg0) {
    s16 ang;
    s16 t;
    s16 s;
    s16 c;
    s16 vx;
    s16 vz;
    s32 lim;

    lim = D_8036443C / 40;
    if (lim < 0) {
        lim = -lim;
    }
    if (D_8036443C != 0 && lim == 0) {
        lim = 2;
    }
    ang = gPeople[arg0].unk16 + D_8036443E + 0x400;
    if (ang >= 0x1000) {
        ang -= 0xFFF;
    }
    gPeople[arg0].unk18 = ang;
    t = ang % 0x400;
    s = sins(t * 0xFFFF / 0xFFF);
    c = coss(t * 0xFFFF / 0xFFF);
    if (gPeople[arg0].unk1C < lim) {
        gPeople[arg0].unk1C++;
    }
    if (gPeople[arg0].unk1C > lim) {
        gPeople[arg0].unk1C--;
    }
    vx = gPeople[arg0].unk1C * s >> 15;
    vz = gPeople[arg0].unk1C * c >> 15;
    if (ang < 0x400) {
        gPeopleStepX = vx;
        gPeopleStepZ = vz;
    }
    if (ang >= 0x400 && ang < 0x800) {
        gPeopleStepX = vz;
        gPeopleStepZ = -vx;
    }
    if (ang >= 0x800 && ang < 0xC00) {
        gPeopleStepX = -vx;
        gPeopleStepZ = -vz;
    }
    if (ang >= 0xC00) {
        gPeopleStepX = -vz;
        gPeopleStepZ = vx;
    }
}

/* Chopper pickup: boarding, and the next person to fetch (0x80265E48).
 * When the chopper has landed (gChopperTouchdown) the chosen person boards; when it
 * is free (gChopperMode == 0) it is sent to the idle person nearest it, with a
 * sound. Rare's slip: the "farthest person" choice for `forced` is dead code,
 * since the timeout that sets forced also sets gChopperMode = 6 in the same call.
 * The first three initialisers share a line (as1 schedules by source line). */
void people_dispatch_chopper(void) {
    s32 i;
    s32 minIdx;
    s32 minDist;
    s32 maxIdx;
    s32 maxDist;
    s32 d;
    u8 found;
    u8 forced;
    s32 vol;

    minIdx = -1; minDist = 99999999; maxIdx = -1;
    maxDist = 0;
    forced = 0;
    if (gChopperTouchdown) {
        i = 0;
        found = 0;
        while (!found) {
            if (gPeople[i].state == 2) {
                gPeople[i].state = 3;
                gPeople[i].unk13 = 0;
                found = 1;
            } else {
                i++;
            }
        }
        gChopperTouchdown = 0;
        gChopperPickupTimer = 99999999;
    } else {
        gChopperPickupTimer--;
        if (gChopperPickupTimer == 0) {
            gChopperMode = 6;
            gPeople[gChopperPickupIndex].state = 1;
            forced = 1;
        }
    }
    if (gChopperMode == 0) {
        for (i = 0; i < 20; i++) {
            if (gPeople[i].state == 3) {
                gPeople[i].state = 0;
            }
        }
        for (i = 0; i < 20; i++) {
            if (gPeople[i].state == 1) {
                d = util_dist2d(gChopperX, gChopperZ, gPeople[i].x << 5, gPeople[i].z << 5);
                if (d < minDist) {
                    minIdx = i;
                    minDist = d;
                }
                if (d > maxDist) {
                    maxIdx = i;
                    maxDist = d;
                }
            }
        }
        if (minIdx != -1) {
            if (forced) {
                i = maxIdx;
            } else {
                i = minIdx;
            }
            gChopperTargetX = gPeople[i].x << 5;
            gChopperTargetZ = gPeople[i].z << 5;
            gChopperLandY = gPeople[i].y << 5;
            gPeople[i].state = 2;
            gChopperPickupTimer = 600;
            gChopperMode = 1;
            gChopperPickupIndex = i;
            if (35000 - util_dist2d(gPlayerX, gPlayerZ, gChopperTargetX, gChopperTargetZ) * 2 >= 0x8000) {
                vol = 0x7FFF;
            } else {
                vol = 35000 - util_dist2d(gPlayerX, gPlayerZ, gChopperTargetX, gChopperTargetZ) * 2;
            }
            if (vol > 4000) {
                snd_modify(snd_play(gSfxBank, SFX_CHOPPER_DISPATCH, 0), 8, vol);
            }
        }
    }
}

/* Mode 0x800 start: the chopper heads for the carrier (0x802661EC).
 * The chopper's target becomes the carrier's x/z; the player's x/z are kept
 * for the retarget (72B80.c). */
void chopper_fly_to_carrier(void) {
    gChopperTargetX = gCarrierX;
    gChopperTargetZ = gCarrierZ;
    gChopperRetargetX = gPlayerX;
    gChopperRetargetZ = gPlayerZ;
    gChopperMode = 1;
    gChopperPickupTimer = 2000;
}

/* Draw the people: billboarded 20x32 RGBA16 sprites, animated and picked by
 * heading, plus one marker at D_803EF310 */
typedef struct DynBuf {
    Mtx mtx[100];
    Vtx vtx[1];
} DynBuf;

extern Person gPeople[20];
extern Vtx D_802E9FB0[4];
extern u16 D_802E9FF0[];
extern u16 D_802EA4F0[], D_802EA9F0[], D_802EAEF0[], D_802EB3F0[], D_802EB8F0[], D_802EBDF0[];
extern u16 D_802EC2F0[], D_802EC7F0[], D_802ECCF0[], D_802ED1F0[], D_802ED6F0[], D_802EDBF0[], D_802EE0F0[], D_802EE5F0[];
extern u16 D_802EEAF0[], D_802EEFF0[], D_802EF4F0[], D_802EF9F0[], D_802EFEF0[], D_802F03F0[], D_802F08F0[], D_802F0DF0[];
extern u16 D_802F12F0[], D_802F17F0[], D_802F1CF0[], D_802F21F0[], D_802F26F0[], D_802F2BF0[], D_802F30F0[], D_802F35F0[];

s32 people_is_visible();

#define LOAD_TEX(ptr) \
    gDPPipeSync(gdl++); \
    gDPLoadTextureBlock(gdl++, ptr, G_IM_FMT_RGBA, G_IM_SIZ_16b, 20, 32, 0, G_TX_CLAMP, G_TX_CLAMP, \
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD)

/* The people as 20x32 billboards, and the one boarding (0x80266248). */
void people_draw(Gfx **gdlp, DynBuf *buf) {
    Gfx *gdl;
    s32 n;
    s32 i;
    s32 k;
    s32 j;
    u8 found;
    u8 frame;
    u16 *tex;
    u32 phys;
    u8 flip;
    f32 mf[4][4];
    f32 fx[4];
    f32 fy[4];
    f32 fz[4];
    s16 ang;

    gdl = *gdlp;
    n = 0;
    guRotateF(mf, gCamHeading - 135.0, 0.0f, 1.0f, 0.0f);
    for (k = 0; k < 4; k++) {
        guMtxXFMF(mf, D_802E9FB0[k].v.ob[0], D_802E9FB0[k].v.ob[1], D_802E9FB0[k].v.ob[2], &fx[k], &fy[k], &fz[k]);
    }
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    for (i = 0; i < 20; i++) {
        if (people_is_visible(&gPeople[i]) &&
            (gPeople[i].state == 1 || gPeople[i].state == 2 || (gPeople[i].state == 4 && gPeople[i].unk1C == 0))) {
            gPeople[i].unk14++;
            if (gPeople[i].unk14 >= 6) {
                gPeople[i].unk13++;
                gPeople[i].unk14 = 0;
            }
            if (gPeople[i].unk13 >= 6) {
                gPeople[i].unk13 = 0;
            }
            switch (gPeople[i].unk13) {
                case 0: tex = D_802EA4F0; break;
                case 1: tex = D_802EA9F0; break;
                case 2: tex = D_802EAEF0; break;
                case 3: tex = D_802EB3F0; break;
                case 4: tex = D_802EB8F0; break;
                case 5: tex = D_802EBDF0; break;
            }
            phys = osVirtualToPhysical(tex);
            PORT_ITAG(gdl, PORT_ITAG_PEOPLE, i);
            LOAD_TEX(phys);
            util_copy64((u64 *) (&buf->vtx[n]), (u64 *) D_802E9FB0, sizeof(Vtx) * 4);
            for (j = 0; j < 4; j++) {
                buf->vtx[n + j].v.ob[0] = fx[j] + gPeople[i].x;
                buf->vtx[n + j].v.ob[1] = fy[j] + gPeople[i].y;
                buf->vtx[n + j].v.ob[2] = fz[j] + gPeople[i].z;
            }
            gSPVertex(gdl++, n * sizeof(Vtx) + 0x1900 + (u32) D_02000000, 4, 0);
            n += 4;
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
        }
    }
    for (i = 0; i < 20; i++) {
        if (((gPeople[i].state == 4 && gPeople[i].unk1C != 0) || gPeople[i].state == 5) &&
            people_is_visible(&gPeople[i])) {
            gPeople[i].unk14 += gPeople[i].unk1C;
            if (gPeople[i].unk14 >= 11) {
                gPeople[i].unk13++;
                gPeople[i].unk14 = 0;
            }
            if (gPeople[i].unk13 >= 8) {
                gPeople[i].unk13 = 0;
            }
            frame = gPeople[i].unk13;
            flip = 0;
            ang = gPeople[i].unk18 - (gCamHeading - 135.0) / 360.0 * 4095.0;
            if (ang < 0) {
                ang += 0xFFF;
            }
            if (ang >= 0xC00) {
                switch (frame) {
                    case 0: tex = D_802EEAF0; break;
                    case 1: tex = D_802EEFF0; break;
                    case 2: tex = D_802EF4F0; break;
                    case 3: tex = D_802EF9F0; break;
                    case 4: tex = D_802EFEF0; break;
                    case 5: tex = D_802F03F0; break;
                    case 6: tex = D_802F08F0; break;
                    case 7: tex = D_802F0DF0; break;
                }
            }
            if (ang >= 0x800 && ang < 0xC00) {
                flip = 1;
                switch (frame) {
                    case 0: tex = D_802F12F0; break;
                    case 1: tex = D_802F17F0; break;
                    case 2: tex = D_802F1CF0; break;
                    case 3: tex = D_802F21F0; break;
                    case 4: tex = D_802F26F0; break;
                    case 5: tex = D_802F2BF0; break;
                    case 6: tex = D_802F30F0; break;
                    case 7: tex = D_802F35F0; break;
                }
            }
            if (ang >= 0x400 && ang < 0x800) {
                switch (frame) {
                    case 0: tex = D_802EC2F0; break;
                    case 1: tex = D_802EC7F0; break;
                    case 2: tex = D_802ECCF0; break;
                    case 3: tex = D_802ED1F0; break;
                    case 4: tex = D_802ED6F0; break;
                    case 5: tex = D_802EDBF0; break;
                    case 6: tex = D_802EE0F0; break;
                    case 7: tex = D_802EE5F0; break;
                }
            }
            if (ang < 0x400) {
                switch (frame) {
                    case 0: tex = D_802F12F0; break;
                    case 1: tex = D_802F17F0; break;
                    case 2: tex = D_802F1CF0; break;
                    case 3: tex = D_802F21F0; break;
                    case 4: tex = D_802F26F0; break;
                    case 5: tex = D_802F2BF0; break;
                    case 6: tex = D_802F30F0; break;
                    case 7: tex = D_802F35F0; break;
                }
            }
            phys = osVirtualToPhysical(tex);
            PORT_ITAG(gdl, PORT_ITAG_PEOPLE, i);
            LOAD_TEX(phys);
            util_copy64((u64 *) (&buf->vtx[n]), (u64 *) D_802E9FB0, sizeof(Vtx) * 4);
            if (flip) {
                buf->vtx[n].v.tc[0] = 0x260;
                buf->vtx[n + 1].v.tc[0] = 0;
                buf->vtx[n + 2].v.tc[0] = 0;
                buf->vtx[n + 3].v.tc[0] = 0x260;
            }
            for (j = 0; j < 4; j++) {
                buf->vtx[n + j].v.ob[0] = fx[j] + gPeople[i].x;
                buf->vtx[n + j].v.ob[1] = fy[j] + gPeople[i].y;
                buf->vtx[n + j].v.ob[2] = fz[j] + gPeople[i].z;
            }
            gSPVertex(gdl++, n * sizeof(Vtx) + 0x1900 + (u32) D_02000000, 4, 0);
            n += 4;
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
        }
    }
    if (D_803EF32E == 0) {
        i = 0;
        found = 0;
        while (i < 20 && !found) {
            if (gPeople[i].state == 3) {
                found = 1;
                PORT_ITAG(gdl, PORT_ITAG_MARKER, 0);
                LOAD_TEX(osVirtualToPhysical(D_802E9FF0));
                util_copy64((u64 *) (&buf->vtx[n]), (u64 *) D_802E9FB0, sizeof(Vtx) * 4);
                for (j = 0; j < 4; j++) {
                    buf->vtx[n + j].v.ob[0] = fx[j];
                    buf->vtx[n + j].v.ob[1] = fy[j];
                    buf->vtx[n + j].v.ob[2] = fz[j];
                }
                guTranslate(&buf->mtx[8], D_803EF310 / 32.0f, D_803EF314 / 32.0f, D_803EF318 / 32.0f);
                gSPMatrix(gdl++, 8 * sizeof(Mtx) + (u32) D_02000000, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
                gSPVertex(gdl++, n * sizeof(Vtx) + 0x1900 + (u32) D_02000000, 4, 0);
                n += 4;
                gSP1Triangle(gdl++, 0, 1, 2, 0);
                gSP1Triangle(gdl++, 0, 2, 3, 0);
                gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
            } else {
                i++;
            }
        }
    }
    PORT_ITAG(gdl, PORT_ITAG_NONE, 0);
    gDPPipeSync(gdl++);
    *gdlp = gdl;
}

typedef struct {
    u8 pad[0x12];
    u8 cell;
} Node12;

/* 1 if the person's grid cell is in gVisibleCells (0x80267614).
 * Is the node's cell in the 0xFFFF-terminated list gVisibleCells? (same as
 * 26570's rdu_is_visible with the cell at 0x12) */
s32 people_is_visible(Node12 *node, s32 arg1) {
    s32 i;
    u8 v;
    s32 pad;

    i = 0;
    v = node->cell;
    for (; gVisibleCells[i] != 0xFFFF; ) {
        arg1 = gVisibleCells[i++] == v;
        if (arg1) {
            return 1;
        }
    }
    return 0;
}
