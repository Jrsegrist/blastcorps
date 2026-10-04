#include "common.h"
#include <ultra64.h>

/* The digger code (this file's .bss starts at 0x8036C8D0) */
extern u8 D_80364456;      /* current vehicle */
extern u8 D_8036CB28;
extern u8 D_8036CB29;
extern s16 D_8036CB2A;
extern s16 D_8036CB2C;
extern u8 D_8036CB2E;
extern u8 D_8036CB30;
extern u8 D_8036CB31;
extern u8 D_8036CB33;

void func_80277EDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8029A7E4(const char *fmt, ...);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_802775C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_80277620.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_802778FC.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_80277C20.s")

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

extern u8 D_8036CB34;

void func_80278318(void) {
    D_8036CB34 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/32E00/func_80278324.s")
