#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_gVehiclesEnd
#define LEGACY_D_8036B974
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gYoshiGameItems ((YoshiItem *) gYoshiGameItems)
#define gYoshiWindows ((TextArg *) gYoshiWindows)
#define gVehicles ((Struct80364460 *) gVehicles)
#define players ((Struct80364AF0 *) players)
#ifdef NON_MATCHING
#define gVehiclesEnd (*(Struct80364460 * N64P *) &gVehiclesEnd)
#define D_8036B974 (*(s32 *) &D_8036B974)
#endif
/* end of views */

/* utils2.c (from its assert strings): camera spline paths, triggers, small maths helpers. */

f32 sqrtf(f32);
f32 util_shuttle_path_eval(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c);
f32 util_cam_path_eval(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c);

#define ASSERT(EX, line) \
    if (!(EX)) debug_printf("\n --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "utils2.c", line)

/* 20-byte trigger zones, tested against the player position */
typedef struct {
    u8 id;
    u8 unk1;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Struct802F3C10;

/* spline tension per path */
typedef struct {
    s32 id;
    f32 val;
    u8 unk8;
    u8 pad9[3];
} Struct802F3C24;

/* spline control points */
typedef struct {
    s32 id;
    u8 unk4;
    u8 pad5;
    s16 x;
    s16 y;
    s16 z;
    u8 speed;
    u8 padD[3];
} Struct802F3C48;

typedef struct {
    s32 id;
    u8 unk4;
    u8 unk5;
    u8 pad6[2];
    f32 unk8;
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
} Struct802F41E8;

/* second set of spline control points (with angles and events) */
typedef struct {
    s32 id;
    u8 unk4;
    u8 idx;
    s16 x;
    s16 y;
    s16 z;
    s16 rot[3];
    u8 speed;
    u8 unk13;
    u8 event;
    u8 pad15[3];
} Struct802F4224;

/* A text-window item, 0x1C bytes (yoshi.c's YoshiItem; this file writes the landing messages into item 42) */
typedef struct {
    /* 0x00 */ u8 unk0[2];
    /* 0x02 */ u8 unk2[4];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ char * N64P text;
    /* 0x10 */ u16 * N64P jtext;
    /* 0x14 */ u8 unk14[8];
} YoshiItem;

/* 0x1C bytes (45BB0.c's YoshiArg) */
typedef struct {
    /* 0x00 */ u8 unk0[0x12];
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14[8];
} TextArg;

/* vehicles, 0x74 bytes each */
typedef struct {
    u8 pad0[0x5C];
    s32 unk5C;
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
} Struct80364460;

/* players, 0x100 bytes each */
typedef struct {
    u8 pad0[0x18];
    u8 unk18[0xE8];
} Struct80364AF0;

extern Struct802F3C10 gBoardZones[];
extern Struct802F3C24 gCamPaths[];
extern Struct802F3C48 gCamPathPoints[];
extern Struct802F41E8 gShuttlePaths[];
extern Struct802F4224 gShuttlePathPoints[];
extern u16 D_80303AF4[];
extern u16 D_80303B00[];
extern u16 D_80303B10[];
extern u16 D_80303B24[];
#ifndef NON_MATCHING
extern Struct80364460 *gVehiclesEnd;
#endif
extern u8 gCamPathIndex;
extern f32 gCamPathBasis[4][4];
extern s32 gCamPathT;
extern u8 gCamPathPoint;
extern f32 gShuttlePathBasis[4][4];
extern s32 gShuttlePathT;
extern u8 gShuttlePathPoint;
extern u8 gShuttlePathIndex;
extern u8 D_8036B958[4];
extern u8 gShuttlePathBranch;
extern u8 D_8036B960[4];
extern u8 D_8036B966;
extern s32 gRandSeed2;
extern u8 D_8036B970;
#ifndef NON_MATCHING
extern s32 D_8036B974;
#endif
extern u8 D_8036B978;
extern u8 D_8036B979;

/* On foot near a zone's vehicle: switch to it (0x802683E0).
 * Checks the trigger zones for the current level; sets gNextVehicle on a hit. */
void util_check_board_zone(void) {
    s32 i;
    u8 found;
    u8 hit;
    Struct80364460 *p;

    i = 0;
    found = 0;
    if (gCurVehicle || gDriverExitWalk) {
        return;
    }
    while (!found && i < 1) {
        if (gBoardZones[i].id == levelno) {
            p = gVehicles;
            hit = 0;
            while (!hit && gVehiclesEnd != p) {
                if (gBoardZones[i].unk1 == p->unk5C) {
                    hit = 1;
                } else {
                    p++;
                }
            }
            if (hit) {
                if (ground_in_triangle(p->unk64 >> 5, p->unk6C >> 5, gBoardZones[i].unk2, gBoardZones[i].unk4,
                                  gBoardZones[i].unk6, gBoardZones[i].unk8, gBoardZones[i].unkA, gBoardZones[i].unkC)
                    || ground_in_triangle(p->unk64 >> 5, p->unk6C >> 5, gBoardZones[i].unk2, gBoardZones[i].unk4,
                                     gBoardZones[i].unkA, gBoardZones[i].unkC, gBoardZones[i].unkE, gBoardZones[i].unk10)) {
                    if (util_dist3d(p->unk64, p->unk68, p->unk6C, gPlayerX, gPlayerY, gPlayerZ) < gBoardZones[i].unk12) {
                        gNextVehicle = gBoardZones[i].unk1;
                        found = 1;
                    }
                }
            }
        }
        i++;
    }
}

/* The level's camera path, its spline basis (0x80268664).
 * Selects spline path id and builds its cardinal-spline basis matrix in gCamPathBasis. */
void util_cam_path_init(s32 id) {
    f32 s;

    gCamPathActive = 0;
    gCamPathIndex = 0;
    do {
        if (gCamPaths[gCamPathIndex].id == id) {
            gCamPathActive = 1;
        } else {
            gCamPathIndex++;
        }
    } while (gCamPathIndex < 3 && !gCamPathActive);
    if (gCamPathActive) {
        s = gCamPaths[gCamPathIndex].val;
        gCamPathBasis[0][0] = -s;
        gCamPathBasis[0][1] = s * 2.0;
        gCamPathBasis[0][2] = -s;
        gCamPathBasis[0][3] = 0.0f;
        gCamPathBasis[1][0] = 2.0 - s;
        gCamPathBasis[1][1] = s - 3.0;
        gCamPathBasis[1][2] = 0.0f;
        gCamPathBasis[1][3] = 1.0f;
        gCamPathBasis[2][0] = s - 2.0;
        gCamPathBasis[2][1] = 3.0 - s * 2.0;
        gCamPathBasis[2][2] = s;
        gCamPathBasis[2][3] = 0.0f;
        gCamPathBasis[3][0] = s;
        gCamPathBasis[3][1] = -s;
        gCamPathBasis[3][2] = 0.0f;
        gCamPathBasis[3][3] = 0.0f;
        gCamPathT = 0;
        gCamPathPoint = 1;
    }
}

/* GCamPathX/Y/Z along the path (0x802688C4).
 * Advances along the path: evaluates the spline at the four current control points. */
void util_cam_path_step(s32 path) {
    s32 j;
    s32 i;
    u8 pts[4];
    u8 want[4];
    f32 t;
    f32 t2;
    f32 t3;
    s32 pad[4];
    u8 found;

    if (gCamPathActive) {
        want[0] = gCamPathPoint - 1;
        want[1] = gCamPathPoint;
        want[2] = gCamPathPoint + 1;
        want[3] = gCamPathPoint + 2;
        for (j = 0; j < 4; j++) {
            i = 0;
            found = 0;
            while (i < 90 && !found) {
                if (gCamPathPoints[i].id == path && gCamPathPoints[i].unk4 == want[j]) {
                    found = 1;
                } else {
                    i++;
                }
            }
            ASSERT(found, 181);
            pts[j] = i;
        }
        t = (f32) gCamPathT / 1000.0;
        t2 = t * t;
        t3 = t2 * t;
        gCamPathX = util_cam_path_eval(gCamPathPoints[pts[0]].x << 5, gCamPathPoints[pts[1]].x << 5, gCamPathPoints[pts[2]].x << 5,
                                   gCamPathPoints[pts[3]].x << 5, t, t2, t3);
        gCamPathY = util_cam_path_eval(gCamPathPoints[pts[0]].y << 5, gCamPathPoints[pts[1]].y << 5, gCamPathPoints[pts[2]].y << 5,
                                   gCamPathPoints[pts[3]].y << 5, t, t2, t3);
        gCamPathZ = util_cam_path_eval(gCamPathPoints[pts[0]].z << 5, gCamPathPoints[pts[1]].z << 5, gCamPathPoints[pts[2]].z << 5,
                                   gCamPathPoints[pts[3]].z << 5, t, t2, t3);
        if (!gLevelDone && !gCarrierHit && !areWeFading()) {
            gCamPathT += gCamPaths[gCamPathIndex].unk8 *
                          ((gCamPathPoints[pts[2]].speed - gCamPathPoints[pts[1]].speed) * t + gCamPathPoints[pts[1]].speed);
        }
        if (gCamPathT >= 1000) {
            gCamPathT = 0;
            gCamPathPoint++;
        }
    }
}

/* Evaluates the camera path (0x80268D84).
 * Evaluates one spline coordinate with basis gCamPathBasis. */
f32 util_cam_path_eval(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = gCamPathBasis[0][0] * x + gCamPathBasis[1][0] * y + gCamPathBasis[2][0] * z + gCamPathBasis[3][0] * w;
    i1 = gCamPathBasis[0][1] * x + gCamPathBasis[1][1] * y + gCamPathBasis[2][1] * z + gCamPathBasis[3][1] * w;
    i2 = gCamPathBasis[0][2] * x + gCamPathBasis[1][2] * y + gCamPathBasis[2][2] * z + gCamPathBasis[3][2] * w;
    i3 = gCamPathBasis[0][3] * x + gCamPathBasis[1][3] * y + gCamPathBasis[2][3] * z + gCamPathBasis[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}


/* 1 if the level has a shuttle path; its kind (0x80268EE8). */
s32 util_find_shuttle_path(s32 id) {
    s32 i;

    i = 0;
    do {
        if (gShuttlePaths[i].id == id) {
            D_803FCD75 = gShuttlePaths[i].unk5;
            return 1;
        }
        i++;
    } while (i < 3);
    return 0;
}

/* The shuttle's flight path (0x80268F54).
 * Picks the second path set (gShuttlePaths) for this level and builds basis gShuttlePathBasis. */
void util_shuttle_path_init(void) {
    u8 found;
    f32 s;

    found = 0;
    if (gNextGameMode == 0x800) {
        D_8036B966 = 0;
    } else {
        D_8036B966 = 1;
    }
    gShuttlePathIndex = 0;
    while (!found) {
        if (gShuttlePaths[gShuttlePathIndex].id == levelno && gShuttlePaths[gShuttlePathIndex].unk4 == D_8036B966) {
            found = 1;
        } else {
            gShuttlePathIndex++;
        }
    }
    s = gShuttlePaths[gShuttlePathIndex].unk8;
    gShuttlePathBasis[0][0] = -s;
    gShuttlePathBasis[0][1] = s * 2.0;
    gShuttlePathBasis[0][2] = -s;
    gShuttlePathBasis[0][3] = 0.0f;
    gShuttlePathBasis[1][0] = 2.0 - s;
    gShuttlePathBasis[1][1] = s - 3.0;
    gShuttlePathBasis[1][2] = 0.0f;
    gShuttlePathBasis[1][3] = 1.0f;
    gShuttlePathBasis[2][0] = s - 2.0;
    gShuttlePathBasis[2][1] = 3.0 - s * 2.0;
    gShuttlePathBasis[2][2] = s;
    gShuttlePathBasis[2][3] = 0.0f;
    gShuttlePathBasis[3][0] = s;
    gShuttlePathBasis[3][1] = -s;
    gShuttlePathBasis[3][2] = 0.0f;
    gShuttlePathBasis[3][3] = 0.0f;
    D_803FCD60 = gShuttlePaths[gShuttlePathIndex].unkC << 5;
    gShuttlePathT = 0;
    gShuttlePathPoint = 1;
    D_8036B958[0] = 0;
    D_8036B958[1] = 0;
    D_8036B958[2] = 0;
    D_8036B958[3] = 0;
    gShuttlePathBranch = 0;
    D_8036B964 = 0;
    D_8036B965 = 0;
}

/* The shuttle along its path: position, angles, landing events (0x80269258).
 * Camera/aircraft path follower on the gShuttlePathPoints spline set: position, angles, and the landing messages. */
void util_shuttle_path_step(void) {
    s32 j;
    s32 i;
    u8 pts[4];
    f32 t;
    f32 t2;
    f32 t3;
    f32 a0;
    f32 a1;
    f32 a2;
    f32 a3;
    f32 r0;
    f32 r1;
    f32 r2;
    u8 found;
    u8 lim;

    D_8036B960[0] = gShuttlePathPoint - 1;
    D_8036B960[1] = gShuttlePathPoint;
    D_8036B960[2] = gShuttlePathPoint + 1;
    D_8036B960[3] = gShuttlePathPoint + 2;
    for (j = 0; j < 4; j++) {
        i = 0;
        found = 0;
        while (i < 49 && !found) {
            if (gShuttlePathPoints[i].id == levelno && gShuttlePathPoints[i].unk4 == D_8036B966
                && gShuttlePathPoints[i].idx == D_8036B960[j] && gShuttlePathPoints[i].unk13 == gShuttlePathBranch) {
                found = 1;
            } else {
                i++;
            }
        }
        if (!found) {
            i = 0;
            while (i < 49 && !found) {
                if (gShuttlePathPoints[i].id == levelno && gShuttlePathPoints[i].unk4 == D_8036B966
                    && gShuttlePathPoints[i].idx == D_8036B960[j] && gShuttlePathPoints[i].unk13 == D_8036B958[j]) {
                    found = 1;
                } else {
                    i++;
                }
            }
        }
        ASSERT(found, 387);
        pts[j] = i;
    }
    t = (f32) gShuttlePathT / 1000.0;
    t2 = t * t;
    t3 = t2 * t;

    a0 = gShuttlePathPoints[pts[0]].rot[0];
    a1 = gShuttlePathPoints[pts[1]].rot[0];
    a2 = gShuttlePathPoints[pts[2]].rot[0];
    a3 = gShuttlePathPoints[pts[3]].rot[0];
    util_unwrap_angle(a0, &a1);
    util_unwrap_angle(a1, &a2);
    util_unwrap_angle(a2, &a3);
    r0 = util_shuttle_path_eval(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6A = r0 / 360.0 * 4095.0;

    a0 = gShuttlePathPoints[pts[0]].rot[1];
    a1 = gShuttlePathPoints[pts[1]].rot[1];
    a2 = gShuttlePathPoints[pts[2]].rot[1];
    a3 = gShuttlePathPoints[pts[3]].rot[1];
    util_unwrap_angle(a0, &a1);
    util_unwrap_angle(a1, &a2);
    util_unwrap_angle(a2, &a3);
    r1 = util_shuttle_path_eval(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6C = r1 / 360.0 * 4095.0;

    a0 = gShuttlePathPoints[pts[0]].rot[2];
    a1 = gShuttlePathPoints[pts[1]].rot[2];
    a2 = gShuttlePathPoints[pts[2]].rot[2];
    a3 = gShuttlePathPoints[pts[3]].rot[2];
    util_unwrap_angle(a0, &a1);
    util_unwrap_angle(a1, &a2);
    util_unwrap_angle(a2, &a3);
    r2 = util_shuttle_path_eval(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6E = r2 / 360.0 * 4095.0;

    gShuttlePos = util_shuttle_path_eval(gShuttlePathPoints[pts[0]].x << 5, gShuttlePathPoints[pts[1]].x << 5, gShuttlePathPoints[pts[2]].x << 5,
                               gShuttlePathPoints[pts[3]].x << 5, t, t2, t3);
    D_803FCD4C = util_shuttle_path_eval(gShuttlePathPoints[pts[0]].y << 5, gShuttlePathPoints[pts[1]].y << 5, gShuttlePathPoints[pts[2]].y << 5,
                               gShuttlePathPoints[pts[3]].y << 5, t, t2, t3);
    D_803FCD50 = util_shuttle_path_eval(gShuttlePathPoints[pts[0]].z << 5, gShuttlePathPoints[pts[1]].z << 5, gShuttlePathPoints[pts[2]].z << 5,
                               gShuttlePathPoints[pts[3]].z << 5, t, t2, t3);
    if (gShuttlePaths[gShuttlePathIndex].unkE != -1 && D_803FCD4C < gShuttlePaths[gShuttlePathIndex].unkE << 5) {
        D_803FCD4C = gShuttlePaths[gShuttlePathIndex].unkE << 5;
    }
    gShuttlePathEvent = 0;
    if (!gLevelDone && !gCarrierHit && !areWeFading()) {
        gShuttlePathT += gShuttlePaths[gShuttlePathIndex].unk10 *
                      ((gShuttlePathPoints[pts[2]].speed - gShuttlePathPoints[pts[1]].speed) * t + gShuttlePathPoints[pts[1]].speed);
    }
    if (gShuttlePathT >= 1000) {
        gShuttlePathT = 0;
        gShuttlePathPoint++;
        switch (gShuttlePathBranch) {
            case 0:
                lim = gShuttlePaths[gShuttlePathIndex].unk11;
                break;
            case 1:
                lim = gShuttlePaths[gShuttlePathIndex].unk12;
                break;
            case 2:
                lim = gShuttlePaths[gShuttlePathIndex].unk13;
                break;
        }
        if (gShuttlePathPoint >= lim) {
            switch (gShuttlePathBranch) {
                case 0:
                    if (D_803FCD75 == 1) {
                        func_80275270(0x200000000000, 0.5f);
                    } else {
                        gLevelDoneRequest = 1;
                        gPauseRequest = 1;
                    }
                    break;
                case 1:
                    cmo_hit_request = 1;
                    gPauseRequest = 1;
                    break;
            }
        }
        switch (gShuttlePathPoints[pts[2]].event) {
            case 0:
                break;
            case 1:
                if (((s32) gLevelKind) == 0x80) {
                    gShuttlePathBranch = 1;
                } else {
                    obj_tally(0);
                    if (D_8036EA78 < D_8036EB92) {
                        D_8036B964 = 1;
                        gShuttlePathBranch = 1;
                    }
                    D_8036B965 = 1;
                    D_80364A84 = 1;
                }
                break;
            case 2:
            case 3:
            case 11:
            case 12:
            case 13:
                gShuttlePathEvent = gShuttlePathPoints[pts[2]].event;
                break;
            case 5:
                gYoshiGameItems[42].text = "LANDING ABORTED!";
                gYoshiGameItems[42].jtext = D_80303AF4;
                gYoshiWindows[23].unk12 = 0xDA;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 6:
                gYoshiGameItems[42].text = "DITCHING IN SEA!";
                gYoshiGameItems[42].jtext = D_80303B00;
                gYoshiWindows[23].unk12 = 0x77;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 4:
                gYoshiGameItems[42].text = "ON FINAL APPROACH!";
                gYoshiGameItems[42].jtext = D_80303B10;
                gYoshiWindows[23].unk12 = 0xD7;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 7:
                gYoshiGameItems[42].text = "SUCCESSFUL LANDING!";
                gYoshiGameItems[42].jtext = D_80303B24;
                gYoshiWindows[23].unk12 = 0x82;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 8:
                gYoshiGameItems[42].text = "3000 FT!";
                gYoshiGameItems[42].jtext = NULL;
                gYoshiWindows[23].unk12 = 0xD5;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 9:
                gYoshiGameItems[42].text = "2000 FT!";
                gYoshiGameItems[42].jtext = NULL;
                gYoshiWindows[23].unk12 = 0xD3;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                }
                break;
            case 10:
                gYoshiGameItems[42].text = "1000 FT!";
                gYoshiGameItems[42].jtext = NULL;
                gYoshiWindows[23].unk12 = 0xD1;
                if (gGameMode & 0x104) {
                    yoshi_demand(0x8017);
                    music_push_danger_tune();
                }
                break;
        }
    }
}

/* Evaluates the shuttle path (0x8026A184).
 * Same as util_cam_path_eval with basis gShuttlePathBasis. */
f32 util_shuttle_path_eval(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = gShuttlePathBasis[0][0] * x + gShuttlePathBasis[1][0] * y + gShuttlePathBasis[2][0] * z + gShuttlePathBasis[3][0] * w;
    i1 = gShuttlePathBasis[0][1] * x + gShuttlePathBasis[1][1] * y + gShuttlePathBasis[2][1] * z + gShuttlePathBasis[3][1] * w;
    i2 = gShuttlePathBasis[0][2] * x + gShuttlePathBasis[1][2] * y + gShuttlePathBasis[2][2] * z + gShuttlePathBasis[3][2] * w;
    i3 = gShuttlePathBasis[0][3] * x + gShuttlePathBasis[1][3] * y + gShuttlePathBasis[2][3] * z + gShuttlePathBasis[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}

/* *angle into (ref - 180, ref + 180] (0x8026A2E8).
 * Wraps *angle into (ref - 180, ref + 180]. */
void util_unwrap_angle(f32 ref, f32 *angle) {
    f32 d = *angle - ref;

    if (d > 180.0) {
        *angle -= 360.0;
    } else if (d < -180.0) {
        *angle += 360.0;
    }
}

/* Decimal string, no leading zeros (0x8026A378).
 * Writes n as decimal digits (no leading zeros, at most 9 digits) plus a NUL. */
void util_itoa(s32 n, u8 *buf) {
    s32 div;
    u8 digit;
    u8 printed;
    u8 started;

    div = 100000000;
    printed = 0;
    started = 0;
    do {
        digit = n / div;
        if (started || digit) {
            *buf++ = digit + '0';
            started = printed = 1;
        }
        n -= digit * div;
        div /= 10;
    } while (div != 0);
    if (!printed) {
        *buf++ = '0';
    }
    *buf = 0;
}

/* A matrix: z rotation, then y, about (x, y, z) (0x8026A454).
 * Rotation about the point (x, y, z): z axis by rx, then y axis by ry (units of 1/11.375 degree). */
void util_rotate_about_point(s16 x, s16 y, s16 z, s16 rx, s16 ry, Mtx *m) {
    f32 a[4][4];
    f32 b[4][4];

    guTranslateF(a, -x, -y, -z);
    guRotateF(b, (f32) rx / 11.375, 0.0f, 0.0f, 1.0f);
    guMtxCatF(a, b, a);
    guRotateF(b, (f32) ry / 11.375, 0.0f, 1.0f, 0.0f);
    guMtxCatF(a, b, a);
    guTranslateF(b, x, y, z);
    guMtxCatF(a, b, a);
    guMtxF2L(a, m);
}

/* Copies size bytes as u64s (0x8026A5CC).
 * Copies size bytes in 8-byte units. */
void util_copy64(u64 *dst, u64 *src, s32 size) {
    s32 unused;

    size >>= 3;
    while (size--) {
        *dst++ = *src++;
    }
}

/* 2-D distance (64-bit squares, then sqrtf) (0x8026A610).
 * 2D distance (the squares summed in 64 bits, then sqrtf). */
s32 util_dist2d(s32 x1, s32 y1, s32 x2, s32 y2) {
    s64 dx;
    s64 dy;

    dx = x1 - x2, dy = y1 - y2;
    dx = dx * dx;
    dy = dy * dy;
    return sqrtf(dx + dy);
}

/* 3-D distance (64-bit squares, then sqrtf) (0x8026A6F0).
 * 3D distance (the squares summed in 64 bits, then sqrtf). */
s32 util_dist3d(s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2) {
    s64 dx;
    s64 dy;
    s64 dz;

    dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;
    return sqrtf(dx + dy + dz);
}

/* LCG random number in [lo, hi] (0x8026A828).
 * LCG random integer in [lo, hi], rounded. */
s32 util_rand_range(s32 lo, s32 hi) {
    f32 f;

    gRandSeed = PORT_WRAP_ADD(PORT_WRAP_MUL(gRandSeed, 0x41C64E6D), 0x3039);
    f = (gRandSeed & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

/* Seeds util_rand_range from the CPU counter (0x8026A8BC). */
void util_seed_rand(void) {
    gRandSeed = osGetCount();
}

/* The second, fixed-seed generator (0x8026A8E0).
 * Same with the second seed. */
s32 util_rand2_range(s32 lo, s32 hi) {
    f32 f;

    gRandSeed2 = PORT_WRAP_ADD(PORT_WRAP_MUL(gRandSeed2, 0x41C64E6D), 0x3039);
    f = (gRandSeed2 & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

/* The fixed seed of util_rand2_range (0x8026A974). */
void util_seed_rand2(void) {
    gRandSeed2 = 0x9BA0D;
}

/* Clears the hint-event latches (0x8026A988). */
void util_reset_hint_events(void) {
    D_8036B970 = 0;
    D_8036B971 = 0;
    D_8036B974 = 0;
    D_8036B978 = 0;
    D_8036B979 = 0;
}

/* Per frame: hint windows near other vehicles and at level spots (0x8026A9B4).
 * Per-frame event checks: proximity to vehicles, level-specific triggers (yoshi_show_hint). */
void util_check_hint_events(void) {
    Struct80364460 *p;
    u8 done;
    s32 minDist;
    s32 d;

    minDist = 999999;
    if (gTrainContactFrames == 15) {
        yoshi_show_hint(0x50);
    }
    if (levelno == 0x12 && (gPlayerX >> 5) < 0x578
        && util_dist2d(gPlayerX >> 5, gPlayerZ >> 5, 0x531, 0x1061) < 0x82) {
        yoshi_show_hint(0x51);
    }
    if (gCurVehicle != D_8036B979) {
        D_8036B978 = D_8036B979;
    }
    if (gCurVehicle) {
        D_8036B979 = gCurVehicle;
    }
    p = gVehicles;
    done = 0;
    while (!done && gVehiclesEnd != p) {
        if (p->unk5C != D_8036B978 && p->unk5C != gCurVehicle && gCurVehicle != 0 && p->unk5C != 0
            && p->unk5C != 0xFE && p->unk5C != 0xFF && p->unk5C != 7 && p->unk5C != 6) {
            d = util_dist3d(gPlayerX >> 5, gPlayerY >> 5, gPlayerZ >> 5, p->unk64 >> 5, p->unk68 >> 5,
                              p->unk6C >> 5);
            if (d < minDist) {
                minDist = d;
            }
            if (d < 100 && !D_8036B970 && (!veh_is_riding(p->unk5C) || !D_803EFECB)) {
                yoshi_show_hint(0x4C);
                D_8036B970 = 1;
                done = 1;
            }
        }
        p++;
    }
    if (D_8036B970 && minDist > 400) {
        D_8036B970 = 0;
    }
    if (levelno == 0 && gMarkerArrowsOnScreen >= 2) {
        yoshi_show_hint(0x4D);
    }
    if (levelno == 0 && gCurVehicle == 7 && (gPlayerX >> 5) >= 0x899) {
        yoshi_show_hint(0x53);
    }
    if (D_8036B971) {
        if (!((players[playerNumber].unk18[levelno] > 0 && players[playerNumber].unk18[levelno] < 6) ? 1 : 0)
            && !gTargetsCleared) {
            if (!yoshi_show_hint(0x4E)) {
                yoshi_show_hint(0x4F);
            }
            D_8036B971 = 0;
        }
    }
}
