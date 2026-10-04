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
    /* 0x02 */ u8 unk2[0xA];
    /* 0x0C */ u8 *text;
    /* 0x10 */ u16 *jtext;
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
    /* 0x00 */ u8 unk0[0x18];
    /* 0x18 */ u8 unk18[0x70]; /* per level */
    /* 0x88 */ u8 unk88[9];
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 unk92[0x6E];
} Player;

/* 0x22 bytes */
typedef struct {
    /* 0x00 */ u8 id;
    /* 0x02 */ s16 ids[16];
} YoshiTrigger;

/* Yoshi path node, 0x88 bytes */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 visited;
    /* 0x07 */ u8 cell;
    /* 0x08 */ u8 unk8[0x40];
    /* 0x48 */ u8 unk48[0x40];
} YoshiNode;

/* Per-level info, 0x44 bytes */
typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1[0x43];
} LevelInfo;

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define MIN255(x) ((x) >= 256 ? 255 : (x))
#define MAX0(x) ((x) < 0 ? 0 : (x))

#define VIDI_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "./vidiPrint.h", line)

#define YOSHI_OFF 1
#define NO_YOSHI_WINDOW -1
#define YOSHI_DEMAND_OFF 0x4000

typedef struct YoshiArg {
    /* 0x00 */ u8 unk0[8];
    /* 0x08 */ u32 flags;
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u8 unk12[0xA];
} YoshiArg;

void func_8029A7E4(const char *fmt, ...);
u16 func_8026F8A8(u16 arg0, u16 arg1, u16 start, u16 mask);
void func_8026FB50(struct YoshiArg *arg0);
void func_8026AF6C(u16 yd);
s32 func_80297EF8(s32 level);
void *func_8025B558(u16 *text);
u8 func_8026AD30(s16 arg0);
void func_8026A5CC(void *arg0, void *arg1, s32 arg2);
s32 func_8026A6F0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_802AC544(s32 arg0, s32 arg1, s32 arg2);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_8026BA7C(struct YoshiArg *arg0);
u8 func_8026FA38(char **name, s32 *arg1);
s32 func_8026F92C(u64 in);
Gfx *func_8026BCE0(Gfx *gfx, s32 arg1, s32 *count);
s8 func_80272C5C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, f32 arg5);

extern u16 yoshiDemandV;
extern s16 D_8036BB0C;
extern u8 D_8036BB0E;
extern s16 D_8036BB16;
extern u8 D_8036BAE8[];
extern LevelInfo D_802E8F94[];
extern u8 D_802F4878[];
extern u8 D_802E8BF8;
extern u16 D_8036BB48[];
extern u16 D_8036BB4A[];
/* Per-language special characters: [0] for English, [1] for Japanese */
extern u16 D_802E8C8C[];
extern u16 D_802E8C90[];
extern u16 D_802E8C94[];
extern u16 D_802E8C98[]; /* string terminator */
extern u16 D_802E8C9C[];
extern u32 D_803156C4;
extern u32 D_8036BB00;
extern u32 D_8036BAFC;
/* Debug switches set from the command line */
extern s32 D_802FA250; /* -v */
extern s32 D_802FA254; /* -d */
extern s32 D_802FA258; /* -s */
extern s32 D_802FA25C; /* -j */
extern s32 D_802FA260; /* -m */
extern s32 D_802FA264; /* -l */
extern s32 D_802FA268; /* -c, -C */
extern s32 D_802FA26C; /* -C */
extern u16 D_8036EB90;
extern u16 D_8036BBB0[];
extern u16 D_8036BBB2[];
extern s32 D_8036BED4;
extern YoshiNode *D_8036BED8;
extern f32 D_8036BEDC;
extern u8 D_8036BEE0;
extern void *D_80358070;
extern s32 D_803BE70C;
extern s32 D_803BE710;
extern s16 D_803BE714;
extern u8 D_802F99C0[];
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern u8 D_80364456;
extern s32 D_802FA200[];
extern u8 D_802E8BD0;
extern void *D_80367738;
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
extern s16 D_8036BB1E;
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

/* Resets the Yoshi state and sets up the level's path nodes */
void func_8026B118(u8 arg0) {
    YoshiArg *arg;
    YoshiTrigger *trig;
    u8 found;
    s32 i;
    s32 j;

    yoshiState = YOSHI_OFF;
    D_8036BB16 = 0;
    yoshiDemandV = 0;
    D_8036BB1A = -1;
    currentYoshiWindow = NO_YOSHI_WINDOW;
    D_8036BB0C = 0;
    arg = NULL;
    D_8036BB0E = 1;
    if (!arg0) {
        for (i = 0; i < 108; i++) {
            arg = &D_802F8BDC[i];
            if (arg->flags & 0x100) {
                arg->flags |= 0x80;
            }
        }
    }
    D_802F8BDC[6].unkC = 3;
    for (i = 0; i < 18; i++) {
        D_8036BAE8[i] = 0;
    }
    for (i = 0; i < 75; i++) {
        D_802F49F4[i].unk2E = -1;
    }
    switch (D_80364A98) {
        case 0x80:
        case 0x8000000:
            D_8020C070[25].unk14 = 0;
            /* fallthrough */
        case 0x40000000:
            arg = &D_802F8BDC[D_802F4868[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)]];
            if (D_802E8BF8) {
                D_8020C070[29].flags &= ~1;
                D_8020C070[29].unk14 = 11;
            }
            if (D_802E8F94[D_802E8BDC].unk0 == 1) {
                if (func_80297EF8(D_802E8BDC)) {
                    D_8020C070[18].unk14 = 24;
                } else {
                    D_8020C070[18].unk14 = 0;
                }
            } else if (D_802E8F94[D_802E8BDC].unk0 == 0x20) {
                D_8020C070[23].flags &= ~0x400;
                D_8020C070[26].flags &= ~0x400;
                D_8020C070[27].unk14 = 0;
                D_8020C070[28].unk14 = 0;
            } else {
                D_8020C070[23].unk14 = D_802F4878[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)];
                D_8020C070[26].unk14 = D_802F4878[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)];
                D_8020C070[27].unk14 = 0;
                D_8020C070[28].unk14 = 0;
            }
            break;
        case 0x40000000000:
            arg = &D_802F8BDC[22];
            break;
        case 0x4000000000000:
            arg = &D_802F8BDC[56];
            break;
        case 0x40:
            arg = &D_802F8BDC[D_802F4870[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)]];
            if (D_802E8F94[D_802E8BDC].unk0 == 0x20) {
                D_802F5804[27].flags &= ~0x400;
                D_802F5804[28].flags &= ~0x400;
                D_802F5804[29].unk14 = 0;
                D_802F5804[30].unk14 = 0;
            } else {
                D_802F5804[27].unk14 = D_802F4878[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)];
                D_802F5804[28].unk14 = D_802F4878[func_8026F92C(D_802E8F94[D_802E8BDC].unk0)];
                D_802F5804[29].unk14 = 0;
                D_802F5804[30].unk14 = 0;
            }
            break;
        case 0x2000:
            if (((D_80364AF0[D_80364AE8].unk18[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].unk18[D_802E8BDC] < 6) ? 1 : 0) &&
                D_802E8F94[D_802E8BDC].unk0 == 1) {
                arg = &D_802F8BDC[6];
            } else {
                arg = NULL;
            }
            break;
        case 0x100000000000:
            switch (D_802E8BDC) {
                case 55:
                    arg = &D_802F8BDC[65];
                    break;
                case 28:
                    arg = &D_802F8BDC[66];
                    break;
                case 53:
                    arg = &D_802F8BDC[67];
                    break;
                case 7:
                    arg = &D_802F8BDC[68];
                    break;
                case 19:
                    arg = &D_802F8BDC[69];
                    break;
                default:
                    arg = NULL;
                    break;
            }
            break;
        default:
            arg = NULL;
            break;
    }
    if (arg) {
        func_8026BA7C(arg);
    }
    if (D_80364A98 & 0x2004) {
        for (i = 0, found = 0; i < 8 && !found; i++) {
            trig = &D_802F48D0[i];
            if (trig->id == D_802E8BDC) {
                for (j = 0; j < 16 && trig->ids[j] != -1; j++) {
                    func_8026BA7C(&D_802F8BDC[trig->ids[j]]);
                }
            }
        }
    }
}

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

/* Returns the text of message node idx, revealing it one character at a
 * time while D_8036BB1E is 2 */
void *func_8026F004(YoshiArg *arg0, u16 idx, u8 japanese) {
    PathNode *node = &D_8036BB10[idx];
    u8 lang = japanese ? 1 : 0;
    u8 *text = node->text;
    u16 *jtext = node->jtext;
    u16 ch;
    s32 i;
    u16 next;

    D_8036BB48[0] = D_802E8C98[lang];
    switch (D_8036BB1E) {
        case 0:
            if (japanese) {
                return jtext;
            }
            return text;
        case 1:
            if (yoshiState == 2) {
                D_8036BB1E = 2;
                D_8036BB00 = D_803156C4;
            }
            break;
        case 2:
            if (idx < D_8036BB04) {
                if (japanese) {
                    return jtext;
                }
                return text;
            }
            if (idx <= D_8036BB04) {
                if (D_803156C4 - D_8036BB00 >= 5) {
                    D_8036BB00 = D_803156C4;
                    D_8036BB06++;
                    if (japanese) {
                        ch = jtext[D_8036BB06];
                    } else {
                        ch = text[D_8036BB06];
                    }
                    if (ch == D_802E8C90[lang]) {
                        func_80260650(D_80367738, 0x91, NULL);
                    } else if (ch != D_802E8C94[lang] && ch != D_802E8C98[lang] && ch != D_802E8C8C[lang]) {
                        func_80260650(D_80367738, 0x22, NULL);
                    }
                }
                if (japanese) {
                    VIDI_ASSERT(jtext, 70);
                } else {
                    VIDI_ASSERT(text, 70);
                }
                i = 0;
                if (japanese) {
                    while (jtext[i] != D_802E8C98[lang]) {
                        D_8036BB48[i] = jtext[i++];
                    }
                } else {
                    while (text[i] != D_802E8C98[lang]) {
                        D_8036BB48[i] = text[i];
                        i++;
                    }
                }
                D_8036BB48[i] = D_802E8C98[lang];
                if (D_8036BB48[D_8036BB06] == D_802E8C98[lang]) {
                    next = func_8026F8A8(arg0->unkE, arg0->unk10, D_8036BB04, 0x100);
                    if (next == D_8036BB04) {
                        D_8036BB1E = 0;
                        if ((arg0->flags & 0x400000) && yoshiState == 2) {
                            D_8036BAFC = D_803156C4;
                        }
                    } else {
                        func_80260650(D_80367738, 0x23, NULL);
                        D_8036BB04 = next;
                    }
                    D_8036BB06 = 0;
                } else {
                    D_8036BB48[D_8036BB06] = D_802E8C98[lang];
                    if (!(node->flags & 0x4000) && D_803156C4 % 10 >= 6) {
                        D_8036BB48[D_8036BB06] = D_802E8C9C[lang];
                        D_8036BB4A[D_8036BB06] = D_802E8C98[lang];
                    }
                }
                if (japanese) {
                    return D_8036BB48;
                }
                return func_8025B558(D_8036BB48);
            }
            break;
    }
    if ((node->flags & 0x100) || (node->flags & 0x200)) {
        if (japanese) {
            return D_8036BB48;
        }
        return func_8025B558(D_8036BB48);
    }
    if (japanese) {
        return jtext;
    }
    return text;
}

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

void func_8026FBB0(s16 *pos, s16 *end) {
    s32 pad;

    D_8036EB90 = 0;
    D_8036EA7C = 0;
    if (D_80364A90 != 0x40) {
        D_8036BED4 = *D_8036BBB0 = 0;
    }
    D_8036BED8 = D_80358070;
    D_8036BEE0 = 0;
    D_8036BEDC = 999999.0f;
    while (pos != end) {
        D_8036BED8[D_8036EB90].x = pos[0];
        D_8036BED8[D_8036EB90].y = pos[1];
        D_8036BED8[D_8036EB90].z = pos[2];
        D_8036BED8[D_8036EB90].visited = 0;
        D_8036BED8[D_8036EB90].cell = pos[0] / (D_803BE70C >> 5) + pos[2] / (D_803BE710 >> 5) * D_803BE714;
        func_8026A5CC(D_8036BED8[D_8036EB90].unk8, D_802F99C0, 0x40);
        func_8026A5CC(D_8036BED8[D_8036EB90].unk48, D_802F99C0, 0x40);
        D_8036EB90++, pos += 3;
    }
    D_80358070 = (YoshiNode *) D_80358070 + D_8036EB90;
}

u8 func_8026FE6C(s32 arg0) {
    return D_8036BED8[arg0].visited;
}

void func_8026FE8C(s32 arg0) {
    D_8036BED8[arg0].visited = 1;
    D_8036EA7C++;
}

void func_8026FEC4(void) {
    s32 i;
    s32 pad;
    s32 dist;
    u8 cell;
    u8 flag;

    flag = 0;
    cell = D_803643E8 / D_803BE710 * D_803BE714 + D_803643E0 / D_803BE70C;
    for (i = 0; i < D_8036EB90; i++) {
        if (D_8036BED8[i].cell == cell && !D_8036BED8[i].visited) {
            dist = func_8026A6F0(D_803643E0 >> 5, D_803643E4 >> 5, D_803643E8 >> 5,
                                 D_8036BED8[i].x, D_8036BED8[i].y, D_8036BED8[i].z);
            if (dist < D_802FA200[D_80364456]) {
                if (++D_8036EA7C >= 4 && !D_802E8BD0) {
                    func_8026AD30(0x46);
                }
                if (D_80364A90 != 0x40) {
                    D_8036BBB0[D_8036BED4] = D_8036BBB2[D_8036BED4] = i;
                    D_8036BED4++;
                }
                func_802AC544(D_8036BED8[i].x, D_8036BED8[i].y + 5, D_8036BED8[i].z);
                D_8036BED8[i].visited = 1;
                if (!flag) {
                    flag = 1;
                    if (D_80364AA8 == 0x40) {
                        func_80260650(D_80367738, 0x3B, NULL);
                    } else {
                        func_80260650(D_80367738, 0x27, NULL);
                    }
                }
            }
        }
    }
}

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

/* Splits a command line into words and sets a debug switch for each
 * leading "-x" argument */
void func_80270AE0(u8 *cmdline) {
    s32 argc;
    u8 *args[33];
    u8 **argv;
    s32 pad;
    u8 *p;

    argc = 1;
    argv = args;
    if (cmdline != NULL && *cmdline != '\0') {
        p = cmdline;
        while (*p != '\0') {
            while (*p != '\0' && *p == ' ') {
                *p = '\0';
                p++;
            }
            if (*p != '\0') {
                args[argc] = p;
                argc++;
            }
            while (*p != '\0' && *p != ' ') {
                p++;
            }
        }
        while (argc > 1 && argv[1][0] == '-') {
            switch (argv[1][1]) {
                case 'd':
                    D_802FA254 = 1;
                    break;
                case 'v':
                    D_802FA250 = 1;
                    break;
                case 's':
                    D_802FA258 = 1;
                    break;
                case 'j':
                    D_802FA25C = 1;
                    break;
                case 'm':
                    D_802FA260 = 1;
                    break;
                case 'l':
                    D_802FA264 = 1;
                    break;
                case 'c':
                    D_802FA268 = 1;
                    break;
                case 'C':
                    D_802FA26C = 1;
                    D_802FA268 = 1;
                    break;
            }
            argc--;
            argv++;
        }
    }
}
