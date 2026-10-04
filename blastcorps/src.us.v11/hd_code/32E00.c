#include "common.h"
#include <ultra64.h>

/* The digger code (this file's .bss starts at 0x8036C8D0) */
extern u8 D_80364456;      /* current vehicle */
extern void *D_80358070;

/* 50-entry ring buffer of digger samples, indexed D_8036CB28..D_8036CB29 */
typedef struct {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 unk4;   /* digger state when sampled */
    /* 0x08 */ s32 time;
} DigEntry;

/* Timed digger events, 8 bytes */
typedef struct {
    /* 0x00 */ s16 time;
    /* 0x02 */ u8 a;
    /* 0x03 */ u8 b;
    /* 0x04 */ u8 c;
    /* 0x05 */ u8 d;
    /* 0x06 */ u8 done;
} DigTrigger;

extern DigTrigger *D_803BE6FC; /* first */
extern DigTrigger *D_803BE700; /* end */
extern u8 D_803643D6;
extern u8 D_803643DB;
extern s32 D_803EF6E4;
extern u8 D_8036CB2F;

extern DigEntry D_8036C8D0[50];
extern u8 D_8036CB32;
extern void *D_8036CB48[2];
extern u8 D_8036CB28;
extern u8 D_8036CB29;
extern s16 D_8036CB2A;
extern s16 D_8036CB2C;
extern u8 D_8036CB2E;
extern u8 D_8036CB30;
extern u8 D_8036CB31;
extern u8 D_8036CB33;
extern u8 D_8036CB34;

void func_80277EDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_8026205C(s32 arg0);
s32 func_80277D34(void);
s32 func_80277E08(void);
void func_80277C20(void);
void func_802778FC(void);
void func_80277AE0(void);
void func_80277B84(void);
void func_8029A7E4(const char *fmt, ...);

void func_802775C0(void) {
    D_8036CB34 = 0;
    D_8036CB48[0] = D_80358070;
    D_8036CB48[1] = D_80358070 = (u8 *) D_80358070 + 0xC80;
    D_80358070 = (u8 *) D_80358070 + 0xC80;
    D_8036CB28 = 0;
    D_8036CB29 = 0;
}

void func_80277620(s32 now) {
    u8 done = 0;
    DigTrigger *p = D_803BE6FC;
    s16 limit;

    while (!done && D_8036CB28 != D_8036CB29) {
        if (now - D_8036C8D0[D_8036CB28].time > 1000) {
            if (++D_8036CB28 == 50) {
                D_8036CB28 = 0;
            }
        } else {
            done = 1;
        }
    }
    if (D_8036CB2F) {
        if (D_8036CB29 + 1 != D_8036CB28 && !(D_8036CB29 == 49 && D_8036CB28 == 0)) {
            D_8036C8D0[D_8036CB29].unk0 = func_80277D34();
            D_8036C8D0[D_8036CB29].unk2 = func_80277E08();
            D_8036C8D0[D_8036CB29].unk4 = D_8036CB2E;
            D_8036C8D0[D_8036CB29].time = now;
            if (++D_8036CB29 == 50) {
                D_8036CB29 = 0;
            }
        }
    }
    if (D_8036CB2F) {
        func_80277C20();
        func_802778FC();
        func_80277AE0();
        func_80277B84();
    }
    if (now == 50 && !D_803643D6) {
        func_80277EDC(2, 1, 2, func_8026205C(1));
    }
    if (D_803643DB) {
        limit = D_803EF6E4 >> 5;
        while (p < D_803BE700) {
            if (!p->done && limit > p->time) {
                func_80277EDC(p->a, p->b, p->c, p->d);
                p->done = 1;
            }
            p++;
        }
    }
    D_8036CB2F = 0;
}

void func_802778FC(void) {
    switch (D_80364456) {
        case 5:
            if (D_8036CB31 >= 16 && D_8036CB30 >= 16) {
                func_80277EDC(3, 1, 1, 0x63);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 4:
            if (D_8036CB31 >= 41 && D_8036CB30 >= 41) {
                func_80277EDC(4, 1, 1, func_8026205C(4));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 3:
            if (D_8036CB31 >= 11 && D_8036CB30 >= 11) {
                func_80277EDC(0, 1, 3, 0xB2);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
        case 9:
            if (D_8036CB31 >= 26 && D_8036CB30 >= 26) {
                func_80277EDC(4, 1, 1, func_8026205C(4));
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("DOING REALLY WELL\n");
            }
            break;
    }
}

void func_80277AE0(void) {
    switch (D_80364456) {
        case 3:
        case 4:
        case 5:
        case 9:
            if (D_8036CB31 >= 6 && D_8036CB30 < 2) {
                func_80277EDC(1, 1, 3, 0xB3);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("USING WRONG DIGGER\n");
            }
            break;
    }
}

void func_80277B84(void) {
    switch (D_80364456) {
        case 3:
        case 4:
        case 5:
            if (D_8036CB33 >= 31 && D_8036CB30 < 6) {
                func_80277EDC(2, 1, 2, 0x58);
                D_8036CB28 = 0;
                D_8036CB29 = 0;
                func_8029A7E4("USING DIGGER INCORRECTLY\n");
            }
            break;
    }
}

void func_80277C20(void) {
    u8 i;

    D_8036CB30 = 0;
    i = D_8036CB28;
    D_8036CB31 = 0;
    D_8036CB32 = 0;
    D_8036CB33 = 0;
    while (i != D_8036CB29) {
        if (!D_8036C8D0[i].unk2) {
            D_8036CB30++;
        } else {
            D_8036CB32++;
        }
        if (!D_8036C8D0[i].unk0) {
            D_8036CB31++;
        } else {
            D_8036CB33++;
        }
        if (++i == 50) {
            i = 0;
        }
    }
}

s32 func_80277D34(void) {
    switch (D_8036CB2E) {
        case 5:
            if (D_8036CB2A > 400) {
                return 0;
            }
            return 1;
        case 4:
            if (D_8036CB2A > 400) {
                return 0;
            }
            return 1;
        case 9:
            if (D_8036CB2A >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (D_8036CB2A >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

s32 func_80277E08(void) {
    switch (D_8036CB2E) {
        case 5:
            if (D_8036CB2C > 80) {
                return 0;
            }
            return 1;
        case 4:
            if (D_8036CB2C > 80) {
                return 0;
            }
            return 1;
        case 9:
            if (D_8036CB2C >= 100) {
                return 0;
            }
            return 1;
        case 3:
            if (D_8036CB2C >= 100) {
                return 0;
            }
            return 1;
    }
    return 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_80277EDC.s")

void func_80278318(void) {
    D_8036CB34 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_80278324.s")
