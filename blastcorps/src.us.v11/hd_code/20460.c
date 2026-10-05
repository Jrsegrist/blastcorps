#include "common.h"
#include <ultra64.h>

/* Falling debris / bouncing objects: a pool of 20 that are spawned around a
 * point, home in on a target, wander inside a box and play sounds. Also the
 * audio manager (audio.c, from 0x802676A0), a Rare-modified copy of the SDK
 * demos' audiomgr.c. */

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 unk6; /* target x */
    /* 0x08 */ s16 unk8; /* target z */
    /* 0x0A */ s16 unkA; /* box centre x */
    /* 0x0C */ s16 unkC; /* box centre z */
    /* 0x0E */ s16 unkE; /* box half size */
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15; /* state: 0 free, 1/2/3/4 active, 5 homing */
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18; /* heading, 0..0xFFF */
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C; /* speed */
    /* 0x1D */ u8 pad1D[3];
    /* 0x20 */ f32 unk20; /* last distance to target */
} Debris; /* size 0x24 */

extern Debris D_80367D60[20];
extern u32 D_8036B968;
extern s32 D_80368030;
extern s16 D_80368034;
extern s16 D_80368036;
extern s32 D_80368038;
extern s32 D_8036803C;
extern s32 D_80368040;
extern s32 D_80368044;
extern s32 D_80368048;
extern u8 D_8036EA79;
extern u8 D_802E8BD0;
extern s32 D_80367738;
extern s32 D_803643E0;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern s16 D_8036443E;
extern s32 D_803EF2EC;
extern s32 D_803EF2F4;
extern s32 D_803EF308;
extern s32 D_803EF30C;
extern u8 D_803EF32C;
extern u8 D_803EF32D;
extern s32 D_803EF6DC;
extern s32 D_803EF6E4;
extern u16 D_803C30A8[];

s32 func_80260650(s32, s32, s32);
void func_80260AB8(s32, s32, s32);
s32 func_8026A610(s32, s32, s32, s32);
s32 func_8026A8E0(s16, s16);
void func_8026AD30(s32);
s32 func_80265A0C(s32 arg0);
void func_80265B7C(s32 arg0);

/* Clear the debris pool and reset the timers */
void func_80264C20(s32 arg0) {
    s32 i;

    for (i = 0; i < 20; i++) {
        D_80367D60[i].unk15 = 0;
    }
    D_8036B968 = osGetCount();
    D_80368038 = 99999999;
    if (arg0) {
        D_8036EA79 = D_80368040;
    } else {
        D_8036EA79 = 0;
    }
}

/* TODO: needs this file's .rodata split in the yaml (the 100000000.0f
 * constant at 0x80309640). Matches in probe:
void func_80264CB4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, s32 arg5) {
    s32 pad;
    s32 n;
    s32 i;
    u8 found;
    s16 r;

    i = 0;
    n = 0;
    D_8036EA79 += arg5;
    if (arg5 != 0 && D_802E8BD0 == 0) {
        func_8026AD30(0x48);
    }
    if (arg3 > 200) {
        arg3 = 200;
    }
    while (n < arg5 && i < 20) {
        if (n == 2) {
            func_80260650(D_80367738, 0x24, 0);
        }
        found = 0;
        while (i < 20 && !found) {
            if (D_80367D60[i].unk15 == 0) {
                found = 1;
            } else {
                i++;
            }
        }
        if (found) {
            D_80367D60[i].x = arg0;
            D_80367D60[i].z = arg2;
            D_80367D60[i].y = arg1;
            r = arg3 / 5;
            D_80367D60[i].unkA = func_8026A8E0(-r, r) + (arg0 - arg3);
            D_80367D60[i].unkC = func_8026A8E0(-r, r) + (arg2 + arg3);
            D_80367D60[i].unk10 = arg3 * 3 / 2;
            D_80367D60[i].unkE = arg3;
            D_80367D60[i].unk6 = D_80367D60[i].unkA + func_8026A8E0(-arg3, arg3);
            D_80367D60[i].unk8 = D_80367D60[i].unkC + func_8026A8E0(-arg3, arg3);
            D_80367D60[i].unk13 = 0;
            D_80367D60[i].unk14 = 0;
            D_80367D60[i].unk16 = func_8026A8E0(-500, 500);
            D_80367D60[i].unk12 = arg4;
            D_80367D60[i].unk1C = 0;
            D_80367D60[i].unk15 = 5;
            D_80367D60[i].unk18 = 4000;
            D_80367D60[i].unk20 = 100000000.0f;
            D_80367D60[i].unk1B = 0;
        }
        i++, n++;
    }
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80264CB4.s")

void func_80265428(void);
void func_8026513C(void);
void func_80265E48(void);

void func_8026510C(void) {
    func_80265428();
    func_8026513C();
    func_80265E48();
}

/* Move homing debris (state 5) 5 units a frame toward its target; it lands
 * (state 1) once it arrives or starts moving away */
void func_8026513C(void) {
    s16 dx;
    s16 dz;
    s16 adx;
    s16 adz;
    f32 dist;
    s16 sx;
    s16 sz;
    s32 i;

    for (i = 0; i < 20; i++) {
        if (D_80367D60[i].unk15 == 5) {
            dx = D_80367D60[i].unk6 - D_80367D60[i].x;
            if (dx >= 0) {
                adx = dx;
            } else {
                adx = -dx;
            }
            dz = D_80367D60[i].unk8 - D_80367D60[i].z;
            if (dz >= 0) {
                adz = dz;
            } else {
                adz = -dz;
            }
            dist = func_8026A610(D_80367D60[i].unk6, D_80367D60[i].unk8, D_80367D60[i].x, D_80367D60[i].z);
            if (dist < 1.0 || D_80367D60[i].unk20 < dist) {
                D_80367D60[i].unk15 = 1;
            } else {
                D_80367D60[i].unk20 = dist;
                sx = adx / dist * 5.0f;
                sz = adz / dist * 5.0f;
                if (dx >= 0) {
                    D_80367D60[i].x = D_80367D60[i].x + sx;
                } else {
                    D_80367D60[i].x -= sx;
                }
                if (dz >= 0) {
                    D_80367D60[i].z += sz;
                } else {
                    D_80367D60[i].z -= sz;
                }
            }
        }
    }
}

/* Wander: step each active piece along its heading, clamped to its box,
 * preferring the direction away from the player */
void func_80265428(void) {
    s32 i;
    s32 pad;
    s32 x;
    s32 z;
    s32 x2;
    s32 z2;
    s32 d1;
    s32 d2;

    for (i = 0; i < 20; i++) {
        if ((D_80367D60[i].unk15 == 1 || D_80367D60[i].unk15 == 4 || D_80367D60[i].unk15 == 2) && func_80265A0C(i)) {
            func_80265B7C(i);
            x = D_80367D60[i].x + D_80368034;
            z = D_80367D60[i].z + D_80368036;
            x2 = D_80367D60[i].x - D_80368034;
            z2 = D_80367D60[i].z - D_80368036;
            if (x < D_80367D60[i].unkA - D_80367D60[i].unkE) {
                x = D_80367D60[i].unkA - D_80367D60[i].unkE;
            }
            if (z < D_80367D60[i].unkC - D_80367D60[i].unkE) {
                z = D_80367D60[i].unkC - D_80367D60[i].unkE;
            }
            if (x >= D_80367D60[i].unkA + D_80367D60[i].unkE) {
                x = D_80367D60[i].unkA + D_80367D60[i].unkE - 1;
            }
            if (z >= D_80367D60[i].unkC + D_80367D60[i].unkE) {
                z = D_80367D60[i].unkC + D_80367D60[i].unkE - 1;
            }
            if (x2 < D_80367D60[i].unkA - D_80367D60[i].unkE) {
                x2 = D_80367D60[i].unkA - D_80367D60[i].unkE;
            }
            if (z2 < D_80367D60[i].unkC - D_80367D60[i].unkE) {
                z2 = D_80367D60[i].unkC - D_80367D60[i].unkE;
            }
            if (x2 >= D_80367D60[i].unkA + D_80367D60[i].unkE) {
                x2 = D_80367D60[i].unkA + D_80367D60[i].unkE - 1;
            }
            if (z2 >= D_80367D60[i].unkC + D_80367D60[i].unkE) {
                z2 = D_80367D60[i].unkC + D_80367D60[i].unkE - 1;
            }
            d1 = func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, x, z);
            d2 = func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, x2, z2);
            if (D_80367D60[i].unk1B) {
                if (D_80367D60[i].unk1A) {
                    d2 = 0;
                } else {
                    d1 = 0;
                }
                D_80367D60[i].unk1B--;
            }
            if (d1 < d2) {
                D_80367D60[i].x = x2;
                D_80367D60[i].z = z2;
                D_80367D60[i].unk18 += 0x800;
                if (D_80367D60[i].unk18 >= 0x1000) {
                    D_80367D60[i].unk18 -= 0xFFF;
                }
                if (D_80367D60[i].unk1A) {
                    D_80367D60[i].unk1B = 5;
                }
                D_80367D60[i].unk1A = 0;
            } else {
                D_80367D60[i].x = x;
                D_80367D60[i].z = z;
                if (!D_80367D60[i].unk1A) {
                    D_80367D60[i].unk1B = 5;
                }
                D_80367D60[i].unk1A = 1;
            }
            if (i == 1 && D_80367D60[i].unk15 != 4) {
                func_80260650(D_80367738, 0x24, 0);
            }
            if (D_80367D60[i].unk15 == 2) {
                D_803EF32C = 6;
            }
            D_80367D60[i].unk15 = 4;
        } else if (D_80367D60[i].unk15 == 4) {
            D_80367D60[i].unk15 = 1;
        }
    }
}

/* Is the player (D_803643E0/E8, >> 5) inside piece arg0's area? */
s32 func_80265A0C(s32 arg0) {
    s16 b3;
    s16 b2;
    s16 b1;
    s16 b0;

    b1 = D_80367D60[arg0].unkA - D_80367D60[arg0].unk10;
    b0 = D_80367D60[arg0].unkC - D_80367D60[arg0].unk10;
    b3 = D_803643E0 >> 5, b2 = D_803643E8 >> 5;
    if (b3 < b1 || b2 < b0) {
        return 0;
    }
    b1 = D_80367D60[arg0].unkA + D_80367D60[arg0].unk10;
    b0 = D_80367D60[arg0].unkC + D_80367D60[arg0].unk10;
    if (b3 >= b1 || b2 >= b0) {
        return 0;
    }
    return 1;
}

/* Turn piece arg0 and work out this frame's step (D_80368034/36) from its
 * heading and a speed that ramps toward D_8036443C / 40 */
void func_80265B7C(s32 arg0) {
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
    ang = D_80367D60[arg0].unk16 + D_8036443E + 0x400;
    if (ang >= 0x1000) {
        ang -= 0xFFF;
    }
    D_80367D60[arg0].unk18 = ang;
    t = ang % 0x400;
    s = sins(t * 0xFFFF / 0xFFF);
    c = coss(t * 0xFFFF / 0xFFF);
    if (D_80367D60[arg0].unk1C < lim) {
        D_80367D60[arg0].unk1C++;
    }
    if (D_80367D60[arg0].unk1C > lim) {
        D_80367D60[arg0].unk1C--;
    }
    vx = D_80367D60[arg0].unk1C * s >> 15;
    vz = D_80367D60[arg0].unk1C * c >> 15;
    if (ang < 0x400) {
        D_80368034 = vx;
        D_80368036 = vz;
    }
    if (ang >= 0x400 && ang < 0x800) {
        D_80368034 = vz;
        D_80368036 = -vx;
    }
    if (ang >= 0x800 && ang < 0xC00) {
        D_80368034 = -vx;
        D_80368036 = -vz;
    }
    if (ang >= 0xC00) {
        D_80368034 = -vz;
        D_80368036 = vx;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80265E48.s")

/* Point the camera target at the player */
void func_802661EC(void) {
    D_803EF308 = D_803EF6DC;
    D_803EF30C = D_803EF6E4;
    D_80368044 = D_803643E0;
    D_80368048 = D_803643E8;
    D_803EF32C = 1;
    D_80368038 = 2000;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80266248.s")

typedef struct {
    u8 pad[0x12];
    u8 cell;
} Node12;

/* Is the node's cell in the 0xFFFF-terminated list D_803C30A8? (same as
 * 26570's func_80270A54 with the cell at 0x12) */
s32 func_80267614(Node12 *node, s32 arg1) {
    s32 i;
    u8 v;
    s32 pad;

    i = 0;
    v = node->cell;
    for (; D_803C30A8[i] != 0xFFFF; ) {
        arg1 = D_803C30A8[i++] == v;
        if (arg1) {
            return 1;
        }
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_802676A0.s")

extern u8 D_80368070[];

void func_80267A74(void) {
    osStartThread(D_80368070 + 0x18);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267A9C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267CDC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267F88.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267FE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80268254.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_802682A4.s")
