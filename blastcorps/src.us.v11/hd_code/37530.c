#include "common.h"
#include <ultra64.h>

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
    Zone *start;
    Zone *end;
    u8 level;
} ZoneList;

extern ZoneList D_802FC360[11];
extern s32 D_802E8BDC; /* current level */

s32 func_802AC4C4(s32 x, s32 z, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);

/* One trail entry; a and b are countdown timers */
typedef struct {
    u8 pad[0x18];
    u8 a;
    u8 b;
    u8 pad1A[2];
} Trail; /* 0x1C */

extern Trail D_8036D3D0[80]; /* ring, oldest at D_8036DC90, newest at D_8036DC91 */

/* Is (x, y, z) inside one of the current level's zones? (zones span ymin..ymax; the x/z test is func_802AC4C4) */
s32 func_8027BCF0(s16 x, s16 y, s16 z) {
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027BE7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027C4C8.s")

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
