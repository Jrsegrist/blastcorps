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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D350.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/37530/func_8027D5AC.s")
