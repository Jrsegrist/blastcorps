#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_803649D0
#define LEGACY_D_8036B974
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802F5804 ((PathNode *) D_802F5804)
#define D_802F8BDC ((TextArg *) D_802F8BDC)
#define D_80364460 ((Struct80364460 *) D_80364460)
#define D_80364AF0 ((Struct80364AF0 *) D_80364AF0)
#ifdef NON_MATCHING
#define D_803649D0 (*(Struct80364460 * *) &D_803649D0)
#define D_8036B974 (*(s32 *) &D_8036B974)
#endif
/* end of views */

/* utils2.c (from its assert strings): camera spline paths, triggers, small maths helpers. */

f32 sqrtf(f32);
f32 func_8026A184(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c);
f32 func_80268D84(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c);

#define ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "utils2.c", line)

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

/* Pathfinding node, 0x1C bytes (see 45BB0.c) */
typedef struct {
    /* 0x00 */ u8 unk0[2];
    /* 0x02 */ u8 unk2[4];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ char *text;
    /* 0x10 */ u16 *jtext;
    /* 0x14 */ u8 unk14[8];
} PathNode;

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

extern Struct802F3C10 D_802F3C10[];
extern Struct802F3C24 D_802F3C24[];
extern Struct802F3C48 D_802F3C48[];
extern Struct802F41E8 D_802F41E8[];
extern Struct802F4224 D_802F4224[];
extern u16 D_80303AF4[];
extern u16 D_80303B00[];
extern u16 D_80303B10[];
extern u16 D_80303B24[];
#ifndef NON_MATCHING
extern Struct80364460 *D_803649D0;
#endif
extern u8 D_8036B8C0;
extern f32 D_8036B8C8[4][4];
extern s32 D_8036B908;
extern u8 D_8036B90C;
extern f32 D_8036B910[4][4];
extern s32 D_8036B950;
extern u8 D_8036B954;
extern u8 D_8036B955;
extern u8 D_8036B958[4];
extern u8 D_8036B95C;
extern u8 D_8036B960[4];
extern u8 D_8036B966;
extern s32 D_8036B96C;
extern u8 D_8036B970;
#ifndef NON_MATCHING
extern s32 D_8036B974;
#endif
extern u8 D_8036B978;
extern u8 D_8036B979;

/* Checks the trigger zones for the current level; sets D_803649ED on a hit. */
void func_802683E0(void) {
    s32 i;
    u8 found;
    u8 hit;
    Struct80364460 *p;

    i = 0;
    found = 0;
    if (D_80364456 || D_803ED826) {
        return;
    }
    while (!found && i < 1) {
        if (D_802F3C10[i].id == D_802E8BDC) {
            p = D_80364460;
            hit = 0;
            while (!hit && D_803649D0 != p) {
                if (D_802F3C10[i].unk1 == p->unk5C) {
                    hit = 1;
                } else {
                    p++;
                }
            }
            if (hit) {
                if (func_802AC4C4(p->unk64 >> 5, p->unk6C >> 5, D_802F3C10[i].unk2, D_802F3C10[i].unk4,
                                  D_802F3C10[i].unk6, D_802F3C10[i].unk8, D_802F3C10[i].unkA, D_802F3C10[i].unkC)
                    || func_802AC4C4(p->unk64 >> 5, p->unk6C >> 5, D_802F3C10[i].unk2, D_802F3C10[i].unk4,
                                     D_802F3C10[i].unkA, D_802F3C10[i].unkC, D_802F3C10[i].unkE, D_802F3C10[i].unk10)) {
                    if (func_8026A6F0(p->unk64, p->unk68, p->unk6C, D_803643E0, D_803643E4, D_803643E8) < D_802F3C10[i].unk12) {
                        D_803649ED = D_802F3C10[i].unk1;
                        found = 1;
                    }
                }
            }
        }
        i++;
    }
}

/* Selects spline path id and builds its cardinal-spline basis matrix in D_8036B8C8. */
void func_80268664(s32 id) {
    f32 s;

    D_8036B8B0 = 0;
    D_8036B8C0 = 0;
    do {
        if (D_802F3C24[D_8036B8C0].id == id) {
            D_8036B8B0 = 1;
        } else {
            D_8036B8C0++;
        }
    } while (D_8036B8C0 < 3 && !D_8036B8B0);
    if (D_8036B8B0) {
        s = D_802F3C24[D_8036B8C0].val;
        D_8036B8C8[0][0] = -s;
        D_8036B8C8[0][1] = s * 2.0;
        D_8036B8C8[0][2] = -s;
        D_8036B8C8[0][3] = 0.0f;
        D_8036B8C8[1][0] = 2.0 - s;
        D_8036B8C8[1][1] = s - 3.0;
        D_8036B8C8[1][2] = 0.0f;
        D_8036B8C8[1][3] = 1.0f;
        D_8036B8C8[2][0] = s - 2.0;
        D_8036B8C8[2][1] = 3.0 - s * 2.0;
        D_8036B8C8[2][2] = s;
        D_8036B8C8[2][3] = 0.0f;
        D_8036B8C8[3][0] = s;
        D_8036B8C8[3][1] = -s;
        D_8036B8C8[3][2] = 0.0f;
        D_8036B8C8[3][3] = 0.0f;
        D_8036B908 = 0;
        D_8036B90C = 1;
    }
}

/* Advances along the path: evaluates the spline at the four current control points. */
void func_802688C4(s32 path) {
    s32 j;
    s32 i;
    u8 pts[4];
    u8 want[4];
    f32 t;
    f32 t2;
    f32 t3;
    s32 pad[4];
    u8 found;

    if (D_8036B8B0) {
        want[0] = D_8036B90C - 1;
        want[1] = D_8036B90C;
        want[2] = D_8036B90C + 1;
        want[3] = D_8036B90C + 2;
        for (j = 0; j < 4; j++) {
            i = 0;
            found = 0;
            while (i < 90 && !found) {
                if (D_802F3C48[i].id == path && D_802F3C48[i].unk4 == want[j]) {
                    found = 1;
                } else {
                    i++;
                }
            }
            ASSERT(found, 181);
            pts[j] = i;
        }
        t = (f32) D_8036B908 / 1000.0;
        t2 = t * t;
        t3 = t2 * t;
        D_8036B8B4 = func_80268D84(D_802F3C48[pts[0]].x << 5, D_802F3C48[pts[1]].x << 5, D_802F3C48[pts[2]].x << 5,
                                   D_802F3C48[pts[3]].x << 5, t, t2, t3);
        D_8036B8B8 = func_80268D84(D_802F3C48[pts[0]].y << 5, D_802F3C48[pts[1]].y << 5, D_802F3C48[pts[2]].y << 5,
                                   D_802F3C48[pts[3]].y << 5, t, t2, t3);
        D_8036B8BC = func_80268D84(D_802F3C48[pts[0]].z << 5, D_802F3C48[pts[1]].z << 5, D_802F3C48[pts[2]].z << 5,
                                   D_802F3C48[pts[3]].z << 5, t, t2, t3);
        if (!D_803643D7 && !D_803643D6 && !func_802753C0()) {
            D_8036B908 += D_802F3C24[D_8036B8C0].unk8 *
                          ((D_802F3C48[pts[2]].speed - D_802F3C48[pts[1]].speed) * t + D_802F3C48[pts[1]].speed);
        }
        if (D_8036B908 >= 1000) {
            D_8036B908 = 0;
            D_8036B90C++;
        }
    }
}

/* Evaluates one spline coordinate with basis D_8036B8C8. */
f32 func_80268D84(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = D_8036B8C8[0][0] * x + D_8036B8C8[1][0] * y + D_8036B8C8[2][0] * z + D_8036B8C8[3][0] * w;
    i1 = D_8036B8C8[0][1] * x + D_8036B8C8[1][1] * y + D_8036B8C8[2][1] * z + D_8036B8C8[3][1] * w;
    i2 = D_8036B8C8[0][2] * x + D_8036B8C8[1][2] * y + D_8036B8C8[2][2] * z + D_8036B8C8[3][2] * w;
    i3 = D_8036B8C8[0][3] * x + D_8036B8C8[1][3] * y + D_8036B8C8[2][3] * z + D_8036B8C8[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}


s32 func_80268EE8(s32 id) {
    s32 i;

    i = 0;
    do {
        if (D_802F41E8[i].id == id) {
            D_803FCD75 = D_802F41E8[i].unk5;
            return 1;
        }
        i++;
    } while (i < 3);
    return 0;
}

/* Picks the second path set (D_802F41E8) for this level and builds basis D_8036B910. */
void func_80268F54(void) {
    u8 found;
    f32 s;

    found = 0;
    if (D_80364A98 == 0x800) {
        D_8036B966 = 0;
    } else {
        D_8036B966 = 1;
    }
    D_8036B955 = 0;
    while (!found) {
        if (D_802F41E8[D_8036B955].id == D_802E8BDC && D_802F41E8[D_8036B955].unk4 == D_8036B966) {
            found = 1;
        } else {
            D_8036B955++;
        }
    }
    s = D_802F41E8[D_8036B955].unk8;
    D_8036B910[0][0] = -s;
    D_8036B910[0][1] = s * 2.0;
    D_8036B910[0][2] = -s;
    D_8036B910[0][3] = 0.0f;
    D_8036B910[1][0] = 2.0 - s;
    D_8036B910[1][1] = s - 3.0;
    D_8036B910[1][2] = 0.0f;
    D_8036B910[1][3] = 1.0f;
    D_8036B910[2][0] = s - 2.0;
    D_8036B910[2][1] = 3.0 - s * 2.0;
    D_8036B910[2][2] = s;
    D_8036B910[2][3] = 0.0f;
    D_8036B910[3][0] = s;
    D_8036B910[3][1] = -s;
    D_8036B910[3][2] = 0.0f;
    D_8036B910[3][3] = 0.0f;
    D_803FCD60 = D_802F41E8[D_8036B955].unkC << 5;
    D_8036B950 = 0;
    D_8036B954 = 1;
    D_8036B958[0] = 0;
    D_8036B958[1] = 0;
    D_8036B958[2] = 0;
    D_8036B958[3] = 0;
    D_8036B95C = 0;
    D_8036B964 = 0;
    D_8036B965 = 0;
}

/* Camera/aircraft path follower on the D_802F4224 spline set: position, angles, and the landing messages. */
void func_80269258(void) {
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

    D_8036B960[0] = D_8036B954 - 1;
    D_8036B960[1] = D_8036B954;
    D_8036B960[2] = D_8036B954 + 1;
    D_8036B960[3] = D_8036B954 + 2;
    for (j = 0; j < 4; j++) {
        i = 0;
        found = 0;
        while (i < 49 && !found) {
            if (D_802F4224[i].id == D_802E8BDC && D_802F4224[i].unk4 == D_8036B966
                && D_802F4224[i].idx == D_8036B960[j] && D_802F4224[i].unk13 == D_8036B95C) {
                found = 1;
            } else {
                i++;
            }
        }
        if (!found) {
            i = 0;
            while (i < 49 && !found) {
                if (D_802F4224[i].id == D_802E8BDC && D_802F4224[i].unk4 == D_8036B966
                    && D_802F4224[i].idx == D_8036B960[j] && D_802F4224[i].unk13 == D_8036B958[j]) {
                    found = 1;
                } else {
                    i++;
                }
            }
        }
        ASSERT(found, 387);
        pts[j] = i;
    }
    t = (f32) D_8036B950 / 1000.0;
    t2 = t * t;
    t3 = t2 * t;

    a0 = D_802F4224[pts[0]].rot[0];
    a1 = D_802F4224[pts[1]].rot[0];
    a2 = D_802F4224[pts[2]].rot[0];
    a3 = D_802F4224[pts[3]].rot[0];
    func_8026A2E8(a0, &a1);
    func_8026A2E8(a1, &a2);
    func_8026A2E8(a2, &a3);
    r0 = func_8026A184(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6A = r0 / 360.0 * 4095.0;

    a0 = D_802F4224[pts[0]].rot[1];
    a1 = D_802F4224[pts[1]].rot[1];
    a2 = D_802F4224[pts[2]].rot[1];
    a3 = D_802F4224[pts[3]].rot[1];
    func_8026A2E8(a0, &a1);
    func_8026A2E8(a1, &a2);
    func_8026A2E8(a2, &a3);
    r1 = func_8026A184(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6C = r1 / 360.0 * 4095.0;

    a0 = D_802F4224[pts[0]].rot[2];
    a1 = D_802F4224[pts[1]].rot[2];
    a2 = D_802F4224[pts[2]].rot[2];
    a3 = D_802F4224[pts[3]].rot[2];
    func_8026A2E8(a0, &a1);
    func_8026A2E8(a1, &a2);
    func_8026A2E8(a2, &a3);
    r2 = func_8026A184(a0, a1, a2, a3, t, t2, t3);
    D_803FCD6E = r2 / 360.0 * 4095.0;

    D_803FCD48 = func_8026A184(D_802F4224[pts[0]].x << 5, D_802F4224[pts[1]].x << 5, D_802F4224[pts[2]].x << 5,
                               D_802F4224[pts[3]].x << 5, t, t2, t3);
    D_803FCD4C = func_8026A184(D_802F4224[pts[0]].y << 5, D_802F4224[pts[1]].y << 5, D_802F4224[pts[2]].y << 5,
                               D_802F4224[pts[3]].y << 5, t, t2, t3);
    D_803FCD50 = func_8026A184(D_802F4224[pts[0]].z << 5, D_802F4224[pts[1]].z << 5, D_802F4224[pts[2]].z << 5,
                               D_802F4224[pts[3]].z << 5, t, t2, t3);
    if (D_802F41E8[D_8036B955].unkE != -1 && D_803FCD4C < D_802F41E8[D_8036B955].unkE << 5) {
        D_803FCD4C = D_802F41E8[D_8036B955].unkE << 5;
    }
    D_803FCD70 = 0;
    if (!D_803643D7 && !D_803643D6 && !func_802753C0()) {
        D_8036B950 += D_802F41E8[D_8036B955].unk10 *
                      ((D_802F4224[pts[2]].speed - D_802F4224[pts[1]].speed) * t + D_802F4224[pts[1]].speed);
    }
    if (D_8036B950 >= 1000) {
        D_8036B950 = 0;
        D_8036B954++;
        switch (D_8036B95C) {
            case 0:
                lim = D_802F41E8[D_8036B955].unk11;
                break;
            case 1:
                lim = D_802F41E8[D_8036B955].unk12;
                break;
            case 2:
                lim = D_802F41E8[D_8036B955].unk13;
                break;
        }
        if (D_8036B954 >= lim) {
            switch (D_8036B95C) {
                case 0:
                    if (D_803FCD75 == 1) {
                        func_80275270(0x200000000000, 0.5f);
                    } else {
                        D_803643DA = 1;
                        D_802E8BD8 = 1;
                    }
                    break;
                case 1:
                    D_803643D9 = 1;
                    D_802E8BD8 = 1;
                    break;
            }
        }
        switch (D_802F4224[pts[2]].event) {
            case 0:
                break;
            case 1:
                if (((s32) D_80364AA8) == 0x80) {
                    D_8036B95C = 1;
                } else {
                    func_802C1DD0(0);
                    if (D_8036EA78 < D_8036EB92) {
                        D_8036B964 = 1;
                        D_8036B95C = 1;
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
                D_803FCD70 = D_802F4224[pts[2]].event;
                break;
            case 5:
                D_802F5804[42].text = "LANDING ABORTED!";
                D_802F5804[42].jtext = D_80303AF4;
                D_802F8BDC[23].unk12 = 0xDA;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 6:
                D_802F5804[42].text = "DITCHING IN SEA!";
                D_802F5804[42].jtext = D_80303B00;
                D_802F8BDC[23].unk12 = 0x77;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 4:
                D_802F5804[42].text = "ON FINAL APPROACH!";
                D_802F5804[42].jtext = D_80303B10;
                D_802F8BDC[23].unk12 = 0xD7;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 7:
                D_802F5804[42].text = "SUCCESSFUL LANDING!";
                D_802F5804[42].jtext = D_80303B24;
                D_802F8BDC[23].unk12 = 0x82;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 8:
                D_802F5804[42].text = "3000 FT!";
                D_802F5804[42].jtext = NULL;
                D_802F8BDC[23].unk12 = 0xD5;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 9:
                D_802F5804[42].text = "2000 FT!";
                D_802F5804[42].jtext = NULL;
                D_802F8BDC[23].unk12 = 0xD3;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                }
                break;
            case 10:
                D_802F5804[42].text = "1000 FT!";
                D_802F5804[42].jtext = NULL;
                D_802F8BDC[23].unk12 = 0xD1;
                if (D_80364A90 & 0x104) {
                    func_8026AF6C(0x8017);
                    func_80260DFC();
                }
                break;
        }
    }
}

/* Same as func_80268D84 with basis D_8036B910. */
f32 func_8026A184(f32 x, f32 y, f32 z, f32 w, f32 a, f32 b, f32 c) {
    s32 i0;
    s32 i1;
    s32 i2;
    s32 i3;

    i0 = D_8036B910[0][0] * x + D_8036B910[1][0] * y + D_8036B910[2][0] * z + D_8036B910[3][0] * w;
    i1 = D_8036B910[0][1] * x + D_8036B910[1][1] * y + D_8036B910[2][1] * z + D_8036B910[3][1] * w;
    i2 = D_8036B910[0][2] * x + D_8036B910[1][2] * y + D_8036B910[2][2] * z + D_8036B910[3][2] * w;
    i3 = D_8036B910[0][3] * x + D_8036B910[1][3] * y + D_8036B910[2][3] * z + D_8036B910[3][3] * w;
    return i0 * c + b * i1 + a * i2 + i3;
}

/* Wraps *angle into (ref - 180, ref + 180]. */
void func_8026A2E8(f32 ref, f32 *angle) {
    f32 d = *angle - ref;

    if (d > 180.0) {
        *angle -= 360.0;
    } else if (d < -180.0) {
        *angle += 360.0;
    }
}

/* Writes n as decimal digits (no leading zeros, at most 9 digits) plus a NUL. */
void func_8026A378(s32 n, u8 *buf) {
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

/* Rotation about the point (x, y, z): z axis by rx, then y axis by ry (units of 1/11.375 degree). */
void func_8026A454(s16 x, s16 y, s16 z, s16 rx, s16 ry, Mtx *m) {
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

/* Copies size bytes in 8-byte units. */
void func_8026A5CC(u64 *dst, u64 *src, s32 size) {
    s32 unused;

    size >>= 3;
    while (size--) {
        *dst++ = *src++;
    }
}

/* 2D distance, squared in 64 bits. */
s32 func_8026A610(s32 x1, s32 y1, s32 x2, s32 y2) {
    s64 dx;
    s64 dy;

    dx = x1 - x2, dy = y1 - y2;
    dx = dx * dx;
    dy = dy * dy;
    return sqrtf(dx + dy);
}

/* 3D distance, squared in 64 bits. */
s32 func_8026A6F0(s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2) {
    s64 dx;
    s64 dy;
    s64 dz;

    dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;
    return sqrtf(dx + dy + dz);
}

/* LCG random integer in [lo, hi], rounded. */
s32 func_8026A828(s32 lo, s32 hi) {
    f32 f;

    D_8036B968 = D_8036B968 * 0x41C64E6D + 0x3039;
    f = (D_8036B968 & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

void func_8026A8BC(void) {
    D_8036B968 = osGetCount();
}

/* Same with the second seed. */
s32 func_8026A8E0(s32 lo, s32 hi) {
    f32 f;

    D_8036B96C = D_8036B96C * 0x41C64E6D + 0x3039;
    f = (D_8036B96C & 0x7FFFFFFF) / 2147483648.0f;
    f = (hi - lo) * f + lo;
    return f + 0.5;
}

void func_8026A974(void) {
    D_8036B96C = 0x9BA0D;
}

void func_8026A988(void) {
    D_8036B970 = 0;
    D_8036B971 = 0;
    D_8036B974 = 0;
    D_8036B978 = 0;
    D_8036B979 = 0;
}

/* Per-frame event checks: proximity to vehicles, level-specific triggers (func_8026AD30). */
void func_8026A9B4(void) {
    Struct80364460 *p;
    u8 done;
    s32 minDist;
    s32 d;

    minDist = 999999;
    if (D_803A7430 == 15) {
        func_8026AD30(0x50);
    }
    if (D_802E8BDC == 0x12 && (D_803643E0 >> 5) < 0x578
        && func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, 0x531, 0x1061) < 0x82) {
        func_8026AD30(0x51);
    }
    if (D_80364456 != D_8036B979) {
        D_8036B978 = D_8036B979;
    }
    if (D_80364456) {
        D_8036B979 = D_80364456;
    }
    p = D_80364460;
    done = 0;
    while (!done && D_803649D0 != p) {
        if (p->unk5C != D_8036B978 && p->unk5C != D_80364456 && D_80364456 != 0 && p->unk5C != 0
            && p->unk5C != 0xFE && p->unk5C != 0xFF && p->unk5C != 7 && p->unk5C != 6) {
            d = func_8026A6F0(D_803643E0 >> 5, D_803643E4 >> 5, D_803643E8 >> 5, p->unk64 >> 5, p->unk68 >> 5,
                              p->unk6C >> 5);
            if (d < minDist) {
                minDist = d;
            }
            if (d < 100 && !D_8036B970 && (!func_802AB3C0(p->unk5C) || !D_803EFECB)) {
                func_8026AD30(0x4C);
                D_8036B970 = 1;
                done = 1;
            }
        }
        p++;
    }
    if (D_8036B970 && minDist > 400) {
        D_8036B970 = 0;
    }
    if (D_802E8BDC == 0 && D_8036C7CC >= 2) {
        func_8026AD30(0x4D);
    }
    if (D_802E8BDC == 0 && D_80364456 == 7 && (D_803643E0 >> 5) >= 0x899) {
        func_8026AD30(0x53);
    }
    if (D_8036B971) {
        if (!((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0)
            && !D_8036EB98) {
            if (!func_8026AD30(0x4E)) {
                func_8026AD30(0x4F);
            }
            D_8036B971 = 0;
        }
    }
}
