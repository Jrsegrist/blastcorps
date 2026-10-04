#include "common.h"
#include <ultra64.h>

/* mb.c (this file's .bss starts at 0x8036CB60) */

#define MB_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "mb.c", line)

/* One recorded sample: three 16.16 values and a position in 1/32 units */
typedef struct {
    f32 a;
    f32 b;
    f32 c;
    f32 x;
    f32 y;
    f32 z;
} MbSample;

/* SetOtherMode_L values swapped when copying a display list */
typedef struct {
    u32 from;
    u32 to;
} MbRemap;

extern u8 *D_80358070;         /* heap pointer */
extern MbSample D_8036CB60[11]; /* ring of samples */
extern s32 D_8036CC68;         /* ring head */
extern s32 D_8036CC6C;         /* ring tail */
extern u8 *D_8036D170;         /* 0x5460-byte buffer */
extern u8 D_8036D178;
extern s32 D_8036D180;
extern u8 D_80367C00;
extern MbRemap D_802FC060[3];

void func_8029A7E4(const char *fmt, ...);
void func_80257490(void *arg0, s32 arg1);

/* Copies a display list to the heap, dropping G_ENDDLs and remapping render modes */
void func_80278BF0(Gfx *src, Gfx *end, Gfx **dstp) {
    Gfx *dst;
    s8 c;
    u32 v;
    u8 found;
    s32 i;
    u32 w1;

    *dstp = (Gfx *) D_80358070;
    dst = *dstp;
    D_80358070 += (end - src) * sizeof(Gfx) - 16;
    while (src != end) {
        switch (c = src->words.w0 >> 24) {
            case (s8) G_ENDDL:
                src++;
                break;
            case (s8) G_SETOTHERMODE_L:
                if ((v = (src->words.w0 >> 8) & 0xFFFF) == 3) {
                    dst->words.w0 = src->words.w0;
                    w1 = src->words.w1;
                    i = 0;
                    found = 0;
                    do {
                        if (D_802FC060[i].from == w1) {
                            found = 1;
                        } else {
                            i++;
                        }
                    } while (!found && i < 3);
                    MB_ASSERT(found, 157);
                    dst->words.w1 = D_802FC060[i].to;
                    dst++, src++;
                } else {
                    *dst++ = *src++;
                }
                break;
            default:
                *dst++ = *src++;
                break;
        }
    }
    gSPEndDisplayList(dst++);
}

/* Allocates the buffers and resets the state */
void func_80278E3C(void) {
    func_80257490(D_80358070, 0x40);
    D_8036D170 = D_80358070;
    func_80257490(D_80358070 = D_80358070 + 0x5460, 8);
    D_8036D178 = 0;
    D_8036CC68 = 0;
    D_8036CC6C = 0;
    D_8036D180 = 0;
    D_80367C00 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80278EB0.s")

void func_802794A4(void) {
    if (!D_80367C00) {
        if (D_8036D180 < 2) {
            D_8036D178 = 0;
        } else {
            D_8036D178 = 2;
        }
    }
}

void func_802794E4(void) {
    D_8036D178 = 0;
}

u8 func_802794F0(void) {
    s32 unused;

    return !(D_8036D178 == 0);
}

/* Pushes a sample onto the ring, dropping the oldest when full */
void func_80279514(s32 x, s32 y, s32 z, s32 a, s32 b, s32 c) {
    D_8036CB60[D_8036CC68].a = (f32) a / 65536.0;
    D_8036CB60[D_8036CC68].b = (f32) b / 65536.0;
    D_8036CB60[D_8036CC68].c = (f32) c / 65536.0;
    D_8036CB60[D_8036CC68].x = x / 32.0f;
    D_8036CB60[D_8036CC68].y = y / 32.0f;
    D_8036CB60[D_8036CC68].z = z / 32.0f;
    D_8036CC68++;
    if (D_8036CC68 == 11) {
        D_8036CC68 = 0;
    }
    if (D_8036CC68 == D_8036CC6C) {
        D_8036CC6C++;
        if (D_8036CC6C == 11) {
            D_8036CC6C = 0;
        }
    }
}

/* Fetches the ring indices n and n + 1 steps back from the head; 0 if not there */
s32 func_802796D8(s32 n, s32 *a, s32 *b) {
    s32 idx = D_8036CC68;

    while (n--) {
        if (idx) {
            idx--;
        } else {
            idx = 10;
        }
        if (idx == D_8036CC6C) {
            return 0;
        }
    }
    *a = idx;
    if (idx) {
        idx--;
    } else {
        idx = 10;
    }
    *b = idx;
    return 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80279778.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_80279EE8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027A7DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027AA04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027AC00.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B200.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B5D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/34430/func_8027B87C.s")
