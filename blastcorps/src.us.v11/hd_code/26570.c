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
    /* 0x02 */ u8 unk2[0x12];
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15[5];
    /* 0x1A */ s8 unk1A;
    /* 0x1B */ u8 unk1B;
} PathNode;

/* 0x30 bytes */
typedef struct {
    /* 0x00 */ u8 unk0[4];
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6[0x26];
    /* 0x2C */ u8 unk2C;
    /* 0x2D */ u8 unk2D;
    /* 0x2E */ s8 unk2E;
    /* 0x2F */ u8 unk2F;
} YoshiSnd;

/* Per-player state, 0x100 bytes */
typedef struct {
    /* 0x00 */ u8 unk0[0x88];
    /* 0x88 */ u8 unk88[9];
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 unk92[0x6E];
} Player;

/* 0x22 bytes */
typedef struct {
    /* 0x00 */ u8 id;
    /* 0x02 */ s16 ids[16];
} YoshiTrigger;

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define MIN255(x) ((x) >= 256 ? 255 : (x))
#define MAX0(x) ((x) < 0 ? 0 : (x))

#define YOSHI_OFF 1
#define NO_YOSHI_WINDOW -1
#define YOSHI_DEMAND_OFF 0x4000

typedef struct YoshiArg {
    /* 0x00 */ u8 unk0[8];
    /* 0x08 */ u32 flags;
    /* 0x0C */ u8 unkC[2];
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u8 unk12[0xA];
} YoshiArg;

void func_8029A7E4(const char *fmt, ...);
u16 func_8026F8A8(u16 arg0, u16 arg1, u16 start, u16 mask);
void func_8026FB50(struct YoshiArg *arg0);
void func_8026AF6C(u16 yd);
void func_8026BA7C(struct YoshiArg *arg0);
u8 func_8026FA38(char **name, s32 *arg1);
s32 func_8026F92C(u64 in);
Gfx *func_8026BCE0(Gfx *gfx, s32 arg1, s32 *count);
s8 func_80272C5C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, f32 arg5);

extern u16 yoshiDemandV;
extern u64 D_80364A90;
extern u8 D_80364AE8;
extern Player D_80364AF0[];
extern YoshiTrigger D_802F48D0[];
extern s32 D_802E8BDC;
extern u8 D_802F499A[];
extern u8 D_8036BAA2[];
extern s16 D_8036BB1A;
extern u32 D_80364AA8;
extern u64 D_80364A98;
extern u8 D_802F4868[];
extern u8 D_802F4870[];
extern struct YoshiArg D_802F8BDC[];
extern u16 D_8036BB04;
extern u16 D_8036BB06;
extern PathNode *D_8036BB10;
extern u16 D_8036BB1E;
extern PathNode *D_8036BB24;
extern s16 currentYoshiWindow;
extern s16 yoshiState;
extern u8 D_8036BA98[];
extern YoshiSnd D_802F49F4[];
extern u16 D_8036EA7C;
extern PathNode D_8020C070[];
extern PathNode D_802F5804[];
extern PathInfo D_802F9934[];
extern u16 D_803C30A8[];
extern s32 D_803F7684;

u8 func_8026AD30(s16 arg0) {
    YoshiTrigger *trig;
    u8 found = 0;
    s32 i;
    s32 j;
    s32 c1;
    s32 c2;

    if (!(D_80364A90 & 0x2104)) {
        return 0;
    }
    if (D_80364AF0[D_80364AE8].unk91 >= 11) {
        return 0;
    }
    for (i = 0; i < 8 && !found; i++) {
        trig = &D_802F48D0[i];
        if (trig->id == D_802E8BDC) {
            for (j = 0; j < 16 && !found && trig->ids[j] != -1; j++) {
                if (trig->ids[j] == arg0) {
                    c1 = D_80364AF0[D_80364AE8].unk88[arg0] < D_802F499A[arg0];
                    c2 = D_802E8BDC == 0;
                    if (!D_8036BAA2[arg0] && (c2 || c1)) {
                        if (c1 && !c2) {
                            D_80364AF0[D_80364AE8].unk88[arg0]++;
                        }
                        D_8036BAA2[arg0] = 1;
                        func_8026AF6C(arg0 | 0x8000 | 0x2000);
                        found = 1;
                    }
                }
            }
        }
    }
    return found;
}

void func_8026AF6C(u16 yd) {
    u16 oldWindow = yoshiDemandV & 0xFF;
    u16 newWindow = yd & 0xFF;

    if (yoshiDemandV) {
        YOSHI_ASSERT(!yoshiDemandV, 1312);
        func_8029A7E4("NEW: %x OLD:%x\n", yd, yoshiDemandV);
    }
    if (yd & 0x4000) {
        YOSHI_ASSERT(yd==YOSHI_DEMAND_OFF, 1317);
    }
    if (newWindow == 0x1E || newWindow == 0x23 || newWindow == 5 || newWindow == 0xE) {
        D_8036BB1A = -1;
    }
    if (oldWindow == 0x1E || oldWindow == 0x23 || oldWindow == 5 || oldWindow == 0xE) {
        YOSHI_ASSERT(1==0, 1324);
        func_8029A7E4("OH MY GOD!\n");
        return;
    }
    if (yoshiDemandV) {
        YOSHI_ASSERT(!yoshiDemandV, 1331);
        func_8029A7E4("GOING FOR NEW: %x OLD:%x\n", yd, yoshiDemandV);
    }
    yoshiDemandV = yd;
}

u16 func_8026B10C(void) {
    return yoshiDemandV;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026B118.s")

void func_8026B8F8(void) {
    if (D_80364AA8 & 0x20) {
        D_8020C070[23].flags |= 0x400;
        D_8020C070[26].flags |= 0x400;
        D_802F5804[27].flags |= 0x400;
        D_802F5804[28].flags |= 0x400;
        D_8020C070[23].unk14 = D_8020C070[26].unk14 = D_802F5804[27].unk14 = D_802F5804[28].unk14 =
            func_8026FA38(NULL, NULL);
        if (D_80364A98 == 0x40) {
            func_8026BA7C(&D_802F8BDC[D_802F4870[func_8026F92C(D_80364AA8)]]);
        } else {
            func_8026BA7C(&D_802F8BDC[D_802F4868[func_8026F92C(D_80364AA8)]]);
        }
    }
}

void func_8026BA7C(YoshiArg *arg0) {
    YoshiSnd *snd;
    s32 i;
    u8 v;
    PathNode *node;

    v = 4;
    func_8026FB50(arg0);
    if (arg0->flags & 0x20000) {
        v = 0;
    }
    for (i = arg0->unkE; i < arg0->unkE + arg0->unk10; i++) {
        node = &D_8036BB10[i];
        if (node->flags & 0x400) {
            snd = &D_802F49F4[node->unk14];
            if (snd->unk2E == -1) {
                node->unk1A = func_80272C5C(snd->unk6, 0, snd->unk4, snd->unk2C, snd->unk2D | v, 1.0f);
                D_8036BA98[node->unk14] = 0;
            } else {
                node->unk1A = snd->unk2E;
            }
        }
    }
}

void func_8026BBD0(Gfx *gfx, s32 arg1, s32 *count) {
    Gfx *gdl = gfx;

    YOSHI_ASSERT(!(yoshiState==YOSHI_OFF && currentYoshiWindow!=NO_YOSHI_WINDOW), 1567);
    gdl = func_8026BCE0(gdl, arg1, count);
    YOSHI_ASSERT(!(yoshiState==YOSHI_OFF && currentYoshiWindow!=NO_YOSHI_WINDOW), 1571);
    gDPPipeSync(gdl++);
    *count += gdl - gfx;
}

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

u8 func_8026F644(u16 *arg0, u16 *arg1, s16 arg2) {
    if (*arg1 & 0x1000) {
        return MAX0(MIN255(512 - (ABS(arg2) * 512) / (arg0[1] / 3)));
    }
    return 255;
}

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
