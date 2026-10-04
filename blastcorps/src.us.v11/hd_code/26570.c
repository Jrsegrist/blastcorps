#include "common.h"
#include <ultra64.h>

#define YOSHI_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "yoshi.c", line)

/* One entry of the path table, 0x14 bytes */
typedef struct {
    /* 0x00 */ u8 id;
    /* 0x01 */ char name[15];
    /* 0x10 */ s32 unk10;
} PathInfo;

/* Pathfinding node, 0x1C bytes */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u8 unk2[0x1A];
} PathNode;

typedef struct {
    /* 0x00 */ u8 unk0[8];
    /* 0x08 */ u32 flags;
    /* 0x0C */ u8 unkC[2];
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
} YoshiArg;

void func_8029A7E4(const char *fmt, ...);
u16 func_8026F8A8(u16 arg0, u16 arg1, u16 start, u16 mask);

extern u16 D_8036BB04;
extern u16 D_8036BB06;
extern PathNode *D_8036BB10;
extern u16 D_8036BB1E;
extern PathNode *D_8036BB24;
extern u16 D_8036EA7C;
extern PathNode D_8020C070[];
extern PathNode D_802F5804[];
extern PathInfo D_802F9934[];
extern u16 D_803C30A8[];
extern s32 D_803F7684;

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026AD30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026AF6C.s")

extern u16 D_8036BB14;

u16 func_8026B10C(void) {
    return D_8036BB14;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026B118.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026B8F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BA7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BBD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BCE0.s")

void func_8026EF70(YoshiArg *arg0) {
    if (arg0->flags & 0x80) {
        D_8036BB04 = func_8026F8A8(arg0->unkE, arg0->unk10, arg0->unkE - 1, 0x100);
        D_8036BB06 = 0;
        if (D_8036BB04 + 1 == arg0->unkE) {
            D_8036BB1E = 0;
        } else {
            D_8036BB1E = 1;
        }
    } else {
        D_8036BB1E = 0;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F004.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F644.s")

u16 func_8026F82C(u16 lo, u16 hi, u16 mask) {
    s32 i;

    for (i = hi - 1; i >= lo; i--) {
        if (D_8036BB10[i].flags & mask) {
            return i;
        }
    }
    return hi;
}

u16 func_8026F8A8(u16 arg0, u16 arg1, u16 start, u16 mask) {
    s32 i;

    for (i = start + 1; i < arg0 + arg1; i++) {
        if (D_8036BB10[i].flags & mask) {
            return i;
        }
    }
    return start;
}

/* Index of the lowest set bit */
s32 func_8026F92C(u64 in) {
    u64 i;

    YOSHI_ASSERT(in, 2270);
    if (!in) {
        return -1;
    }
    for (i = 0; !((1LL << i) & in); i++) {
    }
    return i;
}

u8 func_8026FA38(char **name, s32 *arg1) {
    s32 i;
    s32 result = 0;

    func_8029A7E4("path builing=%d\n", D_803F7684);
    for (i = 0; i < 7 && result == 0; i++) {
        if (D_802F9934[i].id == D_803F7684) {
            result = i + 26;
        }
    }
    if (result == 0) {
        result = 26;
        i = 1;
    }
    if (name) {
        *name = D_802F9934[i - 1].name;
    }
    if (arg1) {
        *arg1 = D_802F9934[i - 1].unk10;
    }
    return result;
}

void func_8026FB50(YoshiArg *arg0) {
    if (arg0->flags & 0x8000) {
        D_8036BB10 = D_8036BB24;
    } else if (arg0->flags & 0x800) {
        D_8036BB10 = D_8020C070;
    } else {
        D_8036BB10 = D_802F5804;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FBB0.s")

extern u8 *D_8036BED8;

u8 func_8026FE6C(s32 arg0) {
    return *(u8 *) ((u8 *) D_8036BED8 + (arg0 * 0x88) + 6);
}

void func_8026FE8C(s32 arg0) {
    *(u8 *) ((u8 *) D_8036BED8 + (arg0 * 0x88) + 6) = 1;
    D_8036EA7C++;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FEC4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802701A8.s")

s32 func_80270A54(u8 *arg0, s32 arg1) {
    s32 i;
    u8 v;
    s32 pad;

    i = 0;
    v = arg0[7];
    for (; D_803C30A8[i] != 0xFFFF; ) {
        arg1 = D_803C30A8[i++] == v;
        if (arg1) {
            return 1;
        }
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80270AE0.s")
