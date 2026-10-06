#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_8036BB24
#define LEGACY_D_8036BED8
#define LEGACY_func_8026BBD0
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((PathNode *) D_8020C070)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F49F4 ((YoshiSnd *) D_802F49F4)
#define D_802F5804 ((PathNode *) D_802F5804)
#define D_802F8BDC ((YoshiArg *) D_802F8BDC)
#define D_80364AF0 ((Player *) D_80364AF0)
#ifdef NON_MATCHING
#define D_8036BB24 (*(PathNode * *) &D_8036BB24)
#define D_8036BED8 (*(YoshiNode * *) &D_8036BED8)
#endif
/* end of views */

#define YOSHI_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "yoshi.c", line)

/* One entry of the path table, 0x14 bytes */
typedef struct {
    /* 0x00 */ u8 id;
    /* 0x01 */ char name[15];
    /* 0x10 */ s32 unk10;
} PathInfo;

/* One item (text line or sprite) of a Yoshi window, 0x1C bytes */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ u16 w;
    /* 0x08 */ u16 h;
    /* 0x0A */ u8 unkA[2];
    /* 0x0C */ u8 *text;
    /* 0x10 */ u16 *jtext;
    /* 0x14 */ u8 unk14; /* sprite */
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u16 sound; /* played when selected */
    /* 0x18 */ u8 color;
    /* 0x19 */ u8 selColor;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
} PathNode;

/* Animated sprite, 0x30 bytes */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6[0x14];
    /* 0x1A */ u8 numFrames;
    /* 0x1B */ u8 frames[10];
    /* 0x25 */ u8 mode;
    /* 0x26 */ u8 speed;
    /* 0x27 */ u8 unk27;
    /* 0x28 */ f32 scale;
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
    /* 0x08 */ Vtx vtx[2][4]; /* double-buffered quad */
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

/* A Yoshi window, 0x1C bytes */
typedef struct YoshiArg {
    /* 0x00 */ u16 w;
    /* 0x02 */ u16 h;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ u32 flags;
    /* 0x0C */ u16 unkC; /* seconds before closing */
    /* 0x0E */ u16 unkE; /* first item */
    /* 0x10 */ u16 unk10; /* item count */
    /* 0x12 */ u16 openSound;
    /* 0x14 */ u16 closeSound;
    /* 0x16 */ u16 moveSound;
    /* 0x18 */ u16 sel; /* selected item */
    /* 0x1A */ u8 atEnd;
    /* 0x1B */ u8 unk1B;
} YoshiArg;

/* Frame buffers passed in as arg1 */
#define YMTX(off) ((Mtx *) (arg1 + (off)))

u16 func_8026F8A8(u16 arg0, u16 arg1, u16 start, u16 mask);
void func_8026FB50(struct YoshiArg *arg0);
s32 func_80270A54();
void func_8026BA7C(struct YoshiArg *arg0);
Gfx *func_8026BCE0(Gfx *gfx, s32 arg1, s32 *count);
void func_8026EF70(YoshiArg *arg0);
void *func_8026F004(YoshiArg *arg0, u16 idx, u8 japanese);
u8 func_8026F644(u16 *arg0, u16 *arg1, s16 arg2);
u16 func_8026F82C(u16 lo, u16 hi, u16 mask);

extern u16 yoshiDemandV;
extern s16 D_8036BB0C;
extern s8 D_8036BB0E;
extern u32 D_8036BB40;
extern f32 D_8036BB08;
extern f32 D_8036BB28;
extern f32 D_8036BB2C;
extern s32 D_8036BB30;
extern f32 D_8036BB38;
extern u16 D_8036BB3C;
extern u16 D_8036BB3E;
extern s32 D_8036BB44;
extern s32 D_802F9930;
extern u8 D_8036BA48[];
extern Gfx D_802F98B0[];
extern u8 D_8036BAE8[];
extern u8 D_802F4878[];
extern u16 D_8036BB48[];
extern u16 D_8036BB4A[];
/* Per-language special characters: [0] for English, [1] for Japanese */
extern u16 D_802E8C9C[];
extern u32 D_8036BB00;
extern u32 D_8036BAFC;
/* Debug switches set from the command line */
extern s32 D_802FA250; /* -v */
extern s32 D_802FA258; /* -s */
extern s32 D_802FA25C; /* -j */
extern s32 D_802FA260; /* -m */
extern u16 D_8036BBB2[];
extern s32 D_8036BED4;
#ifndef NON_MATCHING
extern YoshiNode *D_8036BED8;
#endif
extern f32 D_8036BEDC;
extern u8 D_8036BEE0;
extern Vtx D_802F99C0[]; /* quad template */
extern u8 D_802F9A00[]; /* 16x16 RGBA32 textures */
extern u8 D_802F9E00[];
extern s32 D_802FA200[];
extern YoshiTrigger D_802F48D0[];
extern u8 D_802F499A[];
extern u8 D_8036BAA2[];
extern u16 D_8036BB04;
extern u16 D_8036BB06;
extern PathNode *D_8036BB10;
#ifndef NON_MATCHING
extern PathNode *D_8036BB24;
#endif
extern u8 D_8036BA98[];
extern PathInfo D_802F9934[];

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
                node->unk1A = func_80272C5C((u16 *) snd->unk6, 0, snd->unk4, snd->unk2C, snd->unk2D | v, 1.0f);
                D_8036BA98[node->unk14] = 0;
            } else {
                node->unk1A = snd->unk2E;
            }
        }
    }
}

#ifdef NON_MATCHING
Gfx *func_8026BBD0(Gfx *gfx, s32 arg1, s32 *count) {
#else
void func_8026BBD0(Gfx *gfx, s32 arg1, s32 *count) {
#endif
    Gfx *gdl = gfx;

    YOSHI_ASSERT(!(yoshiState==YOSHI_OFF && currentYoshiWindow!=NO_YOSHI_WINDOW), 1567);
    gdl = func_8026BCE0(gdl, arg1, count);
    YOSHI_ASSERT(!(yoshiState==YOSHI_OFF && currentYoshiWindow!=NO_YOSHI_WINDOW), 1571);
    gDPPipeSync(gdl++);
    *count += gdl - gfx;
#ifdef NON_MATCHING
    /* The original returns nothing (a void function in the matching build)
     * but leaves func_8026BCE0's result in v0, and its callers (hd.c, the
     * front end) take that as the new display-list end: the next command
     * they write replaces the pipesync above. */
    return gdl - 1;
#endif
}

#define PRESSED(m) ((D_80370C28 & (m)) && !(D_80370C2A & (m)))
#define FRAMES(t) ((t) * 60 / 60)

/* Runs and draws the current Yoshi window: handles open/close demands, menu
 * input, the open/close animation, the scroll arrows, sprites and text */
Gfx *func_8026BCE0(Gfx *gfx, s32 arg1, s32 *count) {
    YoshiArg *arg;
    PathNode *node;
    u8 *text;
    u16 *wtext;
    Gfx *gdl;
    u16 sound;
    u16 i;
    u16 halfW;
    u16 halfH;
    u16 dt;
    u16 redraw;
    u8 *c;
    f32 one;
    u16 window;
    u16 force;
    u32 keep;
    u8 done;

    gdl = gfx;
    dt = (D_803156C4 - D_8036BB40) * 15;
    one = 1.0f;
    D_8036BB40 = D_803156C4;
    D_8036BB0C += D_8036BB0E * dt;
    redraw = 0;
    if (D_8036BB0C >= 256) {
        D_8036BB0C = 255;
        D_8036BB0E = -D_8036BB0E;
    }
    if (D_8036BB0C < 0) {
        D_8036BB0C = 0;
        D_8036BB0E = -D_8036BB0E;
    }
    c = D_802F47B0[16];
    c[1] = 255 - D_8036BB0C;
    c[5] = D_8036BB0C;
    c = D_802F47B0[17];
    c[1] = 170 - D_8036BB0C * 2 / 3;
    c[5] = D_8036BB0C * 2 / 3;
    c = D_802F47B0[18];
    c[0] = c[1] = 255 - D_8036BB0C;
    c[4] = c[5] = D_8036BB0C;
    c = D_802F47B0[19];
    c[2] = c[1] = 255 - D_8036BB0C;
    c[6] = c[5] = D_8036BB0C;
    c = D_802F47B0[20];
    c[2] = 255 - D_8036BB0C;
    c[6] = D_8036BB0C;
    if (D_80364A90 == 0x200 && D_803643DB && D_803643D6) {
        D_8036BB1A = -1;
        if (yoshiState == 4 || yoshiState == 2) {
            func_8029A7E4("putting off!\n");
            func_8026AF6C(YOSHI_DEMAND_OFF);
        }
    }
    if (currentYoshiWindow == NO_YOSHI_WINDOW && (yoshiDemandV & YOSHI_DEMAND_OFF)) {
        YOSHI_ASSERT(1==0, 1627);
        yoshiDemandV = 0;
        return gfx;
    }
    if (yoshiDemandV) {
        window = yoshiDemandV & 0xFF;
        force = yoshiDemandV & 0x2000;
        keep = 0;
        func_8029A7E4("yoshiDemand=%x\n", yoshiDemandV);
        if (yoshiDemandV & 0x8000) {
            D_8036BB1A = -1;
            if (currentYoshiWindow != NO_YOSHI_WINDOW) {
                keep = D_802F8BDC[currentYoshiWindow].flags & 0x8000000;
            }
            if (yoshiState == YOSHI_OFF || force || keep) {
                currentYoshiWindow = window;
                yoshiState = YOSHI_OFF;
            } else {
                D_8036BB1A = window;
            }
        }
        if (yoshiState != 8) {
            redraw = 1;
        }
        yoshiDemandV = 0;
    }
    if (currentYoshiWindow == NO_YOSHI_WINDOW) {
        currentYoshiWindow = D_8036BB1A;
        D_8036BB1A = -1;
        if (currentYoshiWindow == NO_YOSHI_WINDOW) {
            return gfx;
        }
        redraw = 1;
    }
    arg = &D_802F8BDC[currentYoshiWindow];
    func_8026FB50(arg);
    if ((arg->flags & 0x20) && yoshiState == 2) {
        if ((PRESSED(0x8000) || ((arg->flags & 0x80000000) && PRESSED(0x1000))) && arg->atEnd) {
            sound = D_8036BB10[arg->sel].sound;
            if (sound) {
                func_80260650(D_80367738, sound, NULL);
            }
            if (D_8036BB10[arg->sel].flags & 0x10) {
                D_802E8BD4 = 1;
            }
            D_8036BB16 = arg->sel;
            redraw = 1;
        }
        if (PRESSED(0x4000)) {
            if ((arg->flags & 0x20000000) && arg->atEnd) {
                func_80260650(D_80367738, 0xDE, NULL);
                D_8036BB16 = 0xFFFF;
                redraw = 1;
                if (arg->flags & 0x40000000) {
                    D_802E8BD4 = 1;
                }
            } else {
                func_80260650(D_80367738, 0xD0, NULL);
            }
        }
    }
    if (redraw) {
        D_8036BAFC = D_803156C4;
        switch (yoshiState) {
            case 8:
                D_8036BAFC = D_803156C4 - D_8036BB08 * D_8036BB38 * D_8036BB34;
            case 1:
                yoshiState = 4;
                D_8036BB34 = 1.0f;
                if (arg->flags & 0x18) {
                    D_8036BB08 = 40.0f;
                } else {
                    D_8036BB08 = 13.333333f;
                }
                func_8026EF70(arg);
                sound = arg->openSound;
                if (sound) {
                    func_80260650(D_80367738, sound, NULL);
                }
                if (!(arg->flags & 0x400)) {
                    for (i = arg->unkE; !(D_8036BB10[i].flags & 1) && i < arg->unkE + arg->unk10; i++) {
                    }
                    arg->sel = i;
                }
                for (i = arg->unkE; i < arg->unkE + arg->unk10; i++) {
                    node = &D_8036BB10[i];
                    if (node->flags & 0x20) {
                        if (arg->flags & 0x80000) {
                            node->x = func_8025B498(arg->w / 2, node->w, node->text, (s32) node->jtext);
                        } else {
                            node->x = func_8025B498(arg->w / 2, node->w, node->text, (s32) node->jtext);
                        }
                    }
                }
                if (arg->flags & 1) {
                    D_802E8BD8 = 1;
                }
                if (arg->flags & 2) {
                    func_80261570(0.0f);
                }
                if (arg->flags & 0x100000) {
                    D_8036BB28 = arg->h;
                } else {
                    D_8036BB28 = arg->h / 2 - D_8036BB10[arg->sel].y;
                    if (arg->flags & 0x40000) {
                        D_8036BB28 -= D_8036BB10[arg->sel].h / 2;
                    }
                }
                D_8036BB2C = D_8036BB28;
                break;
            case 4:
                D_8036BAFC = (D_8036BB38 - one) * D_8036BB08 + D_803156C4;
            case 2:
                yoshiState = 8;
                sound = arg->closeSound;
                if (sound) {
                    func_80260650(D_80367738, sound, NULL);
                }
                if (arg->flags & 0x200000) {
                    D_802E8BD4 = 1;
                }
                if (arg->flags & 4) {
                    func_80261570(1.0f);
                }
                break;
        }
    }
    switch (yoshiState) {
        case 2:
            if (arg->flags & 0x100000) {
                done = D_8036BB2C < (f32) (arg->h / 8 - D_8036BB10[arg->unkE + arg->unk10 - 1].y);
            } else {
                done = arg->unkC != 0 && (D_803156C4 - D_8036BAFC) / 60.0f > arg->unkC &&
                       (!(arg->flags & 0x400000) || !(D_8036BB1E != 0));
            }
            if (done) {
                sound = arg->closeSound;
                if (sound) {
                    func_80260650(D_80367738, sound, NULL);
                }
                if (arg->flags & 0x2000) {
                    func_80261570(0.0f);
                }
                yoshiState = 8;
                D_8036BAFC = D_803156C4;
            }
            break;
        case 4:
            D_8036BB38 = (D_803156C4 - D_8036BAFC) / D_8036BB08;
            if (D_8036BB38 > one) {
                D_8036BAFC = D_803156C4;
                yoshiState = 2;
                D_8036BB38 = one;
                if (arg->flags & 0x40) {
                    D_8036BB3C = 0x200;
                    D_8036BB3E = 0x100;
                } else {
                    D_8036BB3C = 0x800;
                    D_8036BB3E = 0x400;
                }
                if (arg->flags & 0x10000000) {
                    arg->atEnd = 1;
                } else {
                    arg->atEnd = 0;
                }
                if (func_8026F8A8(arg->unkE, arg->unk10, arg->sel, 1) == arg->sel) {
                    arg->atEnd = 1;
                }
            }
            break;
        case 8:
            D_8036BB38 = one - (D_803156C4 - D_8036BAFC) / D_8036BB08;
            if (D_8036BB38 < 0.001) {
                D_8036BB38 = 0.0f;
                yoshiState = 1;
                if (arg->flags & 0x2000000) {
                    D_802E8BD4 = 1;
                }
                if (arg->flags & 0x100) {
                    arg->flags &= ~0x80;
                }
                currentYoshiWindow = NO_YOSHI_WINDOW;
                return gfx;
            }
            break;
    }
    if (yoshiState != 1) {
        D_8036BB20 = (func_802574F0(D_8036BB38 * D_8036BB34 / one * 1.57 + 4.71) + 1.0) * 255.0;
    }
    if (yoshiState != 1 && D_8036BB38 * D_8036BB34 > 0.1) {
        halfW = arg->w / 2;
        halfH = arg->h / 2;
        guOrtho(YMTX(0x1240), -arg->x - halfW, -arg->x - halfW + 319, -arg->y - halfH + 239, -arg->y - halfH,
                -256.0f, 256.0f, 256.0f);
        gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x1240)), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
        if (arg->flags & 0x10) {
            guRotate(YMTX(0x12C0), 180.0 - D_8036BB38 * D_8036BB34 / one * 180.0, 2.0f, 0.0f, 1.0f);
            gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x12C0)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        } else {
            guTranslate(YMTX(0x12C0), 0.0f, 0.0f, 0.0f);
            gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x12C0)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        }
        gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x1300)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
        if (arg->flags & 8) {
            guScale(YMTX(0x1300), halfW * D_8036BB38 * D_8036BB34 / 1000.0f,
                    halfH * D_8036BB38 * D_8036BB34 / 1000.0f, 1.0f);
            gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x1300)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        } else {
            guScale(YMTX(0x1300), halfW / 1000.0f, halfH / 1000.0f, 1.0f);
            gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x1300)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        }
        gDPPipeSync(gdl++);
        gDPSetRenderMode(gdl++, 0x504340, 0);
        gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
        gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
        gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        if (!(arg->flags & 0x200)) {
            gSPDisplayList(gdl++, D_802F98B0);
        }
        gSPPopMatrix(gdl++, G_MTX_MODELVIEW);
        if (arg->flags & 8) {
            guScale(YMTX(0x1280), D_8036BB38, D_8036BB38, 1.0f);
            gSPMatrix(gdl++, OS_K0_TO_PHYSICAL(YMTX(0x1280)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
        }
        if ((arg->flags & 0x20) && yoshiState == 2) {
            s32 up;
            s32 down;

            up = 0;
            down = 0;
            if (D_8036BB3C == 0x800) {
                if (D_80370C12 >= 31 && D_80370C14 < 31) {
                    up = 1;
                } else {
                    up = 0;
                }
            } else if (D_80370C11 < -30 && D_80370C13 >= -30) {
                up = 1;
            } else {
                up = 0;
            }
            if (D_8036BB3E == 0x400) {
                if (D_80370C12 < -30 && D_80370C14 >= -30) {
                    down = 1;
                } else {
                    down = 0;
                }
            } else if (D_80370C11 >= 31 && D_80370C13 < 31) {
                down = 1;
            } else {
                down = 0;
            }
            if (PRESSED(D_8036BB3C) || up) {
                u16 prev;

                prev = func_8026F82C(arg->unkE, arg->sel, 1);
                sound = arg->moveSound;
                if (sound) {
                    if (prev != arg->sel) {
                        func_80260650(D_80367738, sound, NULL);
                    } else {
                        func_80260650(D_80367738, 0xD0, NULL);
                    }
                }
                D_8036BB28 += D_8036BB10[arg->sel].y - D_8036BB10[prev].y;
                arg->sel = prev;
            } else if (((((arg->atEnd ? 0 : 0x8000) | D_8036BB3E) & D_80370C28) &&
                        !(((arg->atEnd ? 0 : 0x8000) | D_8036BB3E) & D_80370C2A)) ||
                       down) {
                u16 next;

                next = func_8026F8A8(arg->unkE, arg->unk10, arg->sel, 1);
                if (func_8026F8A8(arg->unkE, arg->unk10, next, 1) == next) {
                    arg->atEnd = 1;
                }
                sound = arg->moveSound;
                if (sound) {
                    if (next != arg->sel) {
                        func_80260650(D_80367738, sound, NULL);
                    } else {
                        func_80260650(D_80367738, 0xD0, NULL);
                    }
                }
                D_8036BB28 += D_8036BB10[arg->sel].y - D_8036BB10[next].y;
                arg->sel = next;
                D_8036BAFC = D_803156C4;
            }
        }
        if (arg->flags & 0x4000) {
            if (arg->flags & 0x100000) {
                if (arg->unkC == 0 || !((D_803156C4 - D_8036BAFC) / 60.0f < arg->unkC)) {
                    if (arg->flags & 0x800000) {
                        D_8036BB2C -= 0.5;
                    } else {
                        D_8036BB2C -= 1.0;
                    }
                }
            } else {
                D_8036BB2C += (D_8036BB28 - D_8036BB2C) * 0.1;
            }
            if (arg->flags & 0x10000) {
                s32 cc;
                s16 off;

                cc = 0;
                if (arg->flags & 0x40000) {
                    off = 0x12;
                } else {
                    off = 0x1C;
                }
                if (!(D_80364A90 & 0xC9FD0FE79BFF80B0) || D_8035805C) {
                    D_8036BB44 += D_802F9930;
                }
                if (D_802F9930 < 0) {
                    D_8036BB44 += D_802F9930 * 2;
                }
                if (D_8036BB44 < 0 || D_8036BB44 >= 8) {
                    D_8036BB44 -= D_802F9930 * 2;
                    D_802F9930 = -D_802F9930;
                }
                if (func_8026F8A8(arg->unkE, arg->unk10, arg->sel, 1) != arg->sel) {
                    u8 *c2;

                    c2 = D_802F47B0[18];
                    cc = func_80276130((struct SpriteVtxBuf *) arg1, 0, cc, -halfW, halfH - D_8036BB44 - off, 16, D_8036BB44 / 2 + 10,
                                       c2[0], c2[1], c2[2], D_8036BB20, c2[4], c2[5], c2[6], D_8036BB20,
                                       c2[0], c2[1], c2[2], D_8036BB20, c2[4], c2[5], c2[6], D_8036BB20);
                    cc = func_80276080((struct SpriteVtxBuf *) arg1, 0, cc, -3 - halfW, halfH - D_8036BB44 - off + 3, 16,
                                       D_8036BB44 / 2 + 10, 0, 0, 0, D_8036BB20 / 2);
                    gdl = func_80275DA4(gdl, 1);
                    gSPVertex(gdl++, arg1 + 0x1E00, 8, 0);
                    gSP1Triangle(gdl++, 4, 5, 6, 0);
                    gSP1Triangle(gdl++, 4, 6, 7, 0);
                    gSP1Triangle(gdl++, 0, 1, 2, 0);
                    gSP1Triangle(gdl++, 0, 2, 3, 0);
                }
                if (func_8026F82C(arg->unkE, arg->sel, 1) != arg->sel) {
                    u8 *c3;

                    c3 = D_802F47B0[18];
                    /* The ROM reloads c3[0] for the second colour here (unlike the call above, where
                     * IDO reuses the first load). No plain spelling found that defeats ugen's load
                     * cache; the volatile cast reproduces it with identical code. */
                    cc = func_80276130((struct SpriteVtxBuf *) arg1, 1, cc, -halfW, D_8036BB44 - halfH + off, 16, D_8036BB44 / 2 + 10,
                                       c3[0], c3[1], c3[2], D_8036BB20, c3[4], c3[5], c3[6], D_8036BB20,
                                       ((volatile u8 *) c3)[0], c3[1], c3[2], D_8036BB20, c3[4], c3[5], c3[6], D_8036BB20);
                    cc = func_80276080((struct SpriteVtxBuf *) arg1, 1, cc, -3 - halfW, D_8036BB44 - halfH + off - 3, 16,
                                       D_8036BB44 / 2 + 10, 0, 0, 0, D_8036BB20 / 2);
                    gdl = func_80275DA4(gdl, 1);
                    gSPVertex(gdl++, &((Vtx *) (arg1 + 0x1D80))[cc], 8, 0);
                    gSP1Triangle(gdl++, 4, 5, 6, 0);
                    gSP1Triangle(gdl++, 4, 6, 7, 0);
                    gSP1Triangle(gdl++, 0, 1, 2, 0);
                    gSP1Triangle(gdl++, 0, 2, 3, 0);
                }
            }
        } else {
            D_8036BB2C = 0.0f;
        }
        D_8036BB30 = D_8036BB2C;
        if (arg->flags & 0x1000) {
            if (arg->flags & 0x20000) {
                gdl = func_80274868(gdl);
            } else {
                gdl = func_80274998(gdl);
            }
            for (i = arg->unkE; i < arg->unkE + arg->unk10; i++) {
                node = &D_8036BB10[i];
                if (!(node->flags & 0x800) && (node->flags & 0x400) && (!(node->flags & 0x300) || i <= D_8036BB04)) {
                    YoshiSnd *snd;
                    s16 x;
                    s16 y;
                    u8 visible;
                    u8 mode;
                    u8 prev;
                    u8 cur;
                    u8 frame;

                    snd = &D_802F49F4[node->unk14];
                    if (arg->flags & 0x20000) {
                        x = arg->x;
                    } else {
                        x = -halfW;
                    }
                    if (arg->flags & 0x20000) {
                        y = arg->y;
                    } else {
                        y = -halfH;
                    }
                    visible = 1;
                    mode = snd->mode;
                    if ((node->flags & 1) && i != arg->sel) {
                        visible = 0;
                    }
                    prev = D_8036BA48[node->unk14];
                    cur = D_8036BA48[node->unk14] = FRAMES(D_803156C4) / snd->speed % snd->numFrames;
                    if (cur != prev && (visible || D_8036BA98[node->unk14])) {
                        D_8036BA98[node->unk14] = (D_8036BA98[node->unk14] + 1) % snd->numFrames;
                    }
                    frame = snd->frames[D_8036BA98[node->unk14]];
                    if (frame) {
                        if (visible) {
                            if (i == arg->sel && (node->flags & 0x40)) {
                                mode |= 8;
                            }
                            gdl = func_80272ED8(gdl, node->unk1A + frame - 1, snd->x + node->x + x,
                                                ((node->flags & 0x1000) ? D_8036BB30 : 0) + (snd->y + node->y + y),
                                                func_8026F644((u16 *) arg, (u16 *) node, node->y + snd->y - halfH + D_8036BB30 + 8) *
                                                    D_8036BB38 * D_8036BB34,
                                                mode, snd->scale);
                        } else {
                            gdl = func_80272ED8(gdl, node->unk1A + frame - 1, snd->x + node->x + x,
                                                ((node->flags & 0x1000) ? D_8036BB30 : 0) + (snd->y + node->y + y),
                                                func_8026F644((u16 *) arg, (u16 *) node, node->y + snd->y - halfH + D_8036BB30 + 8) *
                                                    D_8036BB38 * D_8036BB34 * 0.7,
                                                mode & ~1, snd->scale);
                        }
                    }
                }
            }
            if (arg->flags & 0x20000) {
                gdl = func_80274AA4(gdl);
            } else {
                gdl = func_80274B08(gdl);
            }
        }
        if (currentYoshiWindow < 0x62 || currentYoshiWindow >= 0x6C || D_80364A90 == 2) {
            for (i = arg->unkE; i < arg->unkE + arg->unk10; i++) {
                node = &D_8036BB10[i];
                wtext = NULL;
                text = func_8026F004(arg, i, 0);
                if ((node->flags & 0x80) && !(node->flags & 0x800)) {
                    if (i == arg->sel) {
                        func_80259DC8((Gfx **) arg1, text, wtext, node->flags & 8, 0, node->x - halfW - 3,
                                      ((node->flags & 0x1000) ? D_8036BB30 : 0) + (node->y - halfH) + 3, node->w,
                                      node->h, 1, 0, 0, 0,
                                      D_8036BB20 * D_802F47B0[node->selColor][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30) / 65025 / 2,
                                      0, 0, 0,
                                      D_8036BB20 * D_802F47B0[node->selColor][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30 + node->h) / 65025 / 2);
                    } else if (!(node->flags & 4) || FRAMES(D_803156C4 % 23) < 16) {
                        func_80259DC8((Gfx **) arg1, text, wtext, node->flags & 8, 0, node->x - halfW - 3,
                                      ((node->flags & 0x1000) ? D_8036BB30 : 0) + (node->y - halfH) + 3, node->w,
                                      node->h, 1, 0, 0, 0,
                                      D_8036BB20 * D_802F47B0[node->color][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30) / 65025 / 2,
                                      0, 0, 0,
                                      D_8036BB20 * D_802F47B0[node->color][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30 + node->h) / 65025 / 2);
                    }
                }
            }
        }
        for (i = arg->unkE; i < arg->unkE + arg->unk10; i++) {
            node = &D_8036BB10[i];
            wtext = NULL;
            text = func_8026F004(arg, i, 0);
            if (!(node->flags & 0x800)) {
                if (i == arg->sel) {
                    if ((!(node->flags & 4) || FRAMES(D_803156C4 % 23) < 16) &&
                        (!(node->flags & 0x40) || FRAMES(D_803156C4 % 15) < 11)) {
                        func_80259DC8((Gfx **) arg1, text, wtext, node->flags & 8, 0, node->x - halfW,
                                      ((node->flags & 0x1000) ? D_8036BB30 : 0) + (node->y - halfH), node->w,
                                      node->h, 1, D_802F47B0[node->selColor][0], D_802F47B0[node->selColor][1],
                                      D_802F47B0[node->selColor][2],
                                      D_8036BB20 * D_802F47B0[node->selColor][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30) / 65025,
                                      D_802F47B0[node->selColor][4], D_802F47B0[node->selColor][5],
                                      D_802F47B0[node->selColor][6],
                                      D_8036BB20 * D_802F47B0[node->selColor][7] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30 + node->h) / 65025);
                    }
                } else if (!(node->flags & 4) || FRAMES(D_803156C4 % 23) < 16) {
                    func_80259DC8((Gfx **) arg1, text, wtext, node->flags & 8, 0, node->x - halfW,
                                  ((node->flags & 0x1000) ? D_8036BB30 : 0) + (node->y - halfH), node->w, node->h,
                                  1, D_802F47B0[node->color][0], D_802F47B0[node->color][1],
                                  D_802F47B0[node->color][2],
                                  D_8036BB20 * D_802F47B0[node->color][3] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30) / 65025,
                                  D_802F47B0[node->color][4], D_802F47B0[node->color][5], D_802F47B0[node->color][6],
                                  D_8036BB20 * D_802F47B0[node->color][7] * func_8026F644((u16 *) arg, (u16 *) node, node->y - halfH + D_8036BB30 + node->h) / 65025);
                }
            }
        }
        func_80259BD4(&gdl, arg1);
    }
    return gdl;
}

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
    D_8036BED8 = (YoshiNode *) D_80358070;
    D_8036BEE0 = 0;
    D_8036BEDC = 999999.0f;
    while (pos != end) {
        D_8036BED8[D_8036EB90].x = pos[0];
        D_8036BED8[D_8036EB90].y = pos[1];
        D_8036BED8[D_8036EB90].z = pos[2];
        D_8036BED8[D_8036EB90].visited = 0;
        D_8036BED8[D_8036EB90].cell = pos[0] / (D_803BE70C >> 5) + pos[2] / (D_803BE710 >> 5) * D_803BE714;
        func_8026A5CC((u64 *) (D_8036BED8[D_8036EB90].vtx[0]), (u64 *) D_802F99C0, sizeof(D_8036BED8->vtx[0]));
        func_8026A5CC((u64 *) (D_8036BED8[D_8036EB90].vtx[1]), (u64 *) D_802F99C0, sizeof(D_8036BED8->vtx[1]));
        D_8036EB90++, pos += 3;
    }
    D_80358070 = (u8 *) ((YoshiNode *) D_80358070 + D_8036EB90);
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

/* Draws the Yoshi path nodes, rotating their quads to face the camera */
void func_802701A8(Gfx **gfx, s32 arg1) {
    Gfx *gdl;
    s32 i;
    s32 j;
    f32 mtx[4][4];
    f32 ox[4];
    f32 oy[4];
    f32 oz[4];

    gdl = *gfx;
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    if (D_80364414 != D_8036BEDC) {
        D_8036BEE0 ^= 1;
        guRotateF(mtx, D_80364414 - 135.0, 0.0f, 1.0f, 0.0f);
        for (i = 0; i < 4; i++) {
            guMtxXFMF(mtx, D_802F99C0[i].v.ob[0], D_802F99C0[i].v.ob[1], D_802F99C0[i].v.ob[2], &ox[i], &oy[i],
                      &oz[i]);
        }
        for (i = 0; i < D_8036EB90; i++) {
            for (j = 0; j < 4; j++) {
                D_8036BED8[i].vtx[D_8036BEE0][j].v.ob[0] = (s16) ox[j] + D_8036BED8[i].x;
                D_8036BED8[i].vtx[D_8036BEE0][j].v.ob[1] = (s16) oy[j] + D_8036BED8[i].y;
                D_8036BED8[i].vtx[D_8036BEE0][j].v.ob[2] = (s16) oz[j] + D_8036BED8[i].z;
            }
        }
    }
    gDPLoadTextureBlock(gdl++, osVirtualToPhysical(D_802F9A00), G_IM_FMT_RGBA, G_IM_SIZ_32b, 16, 16, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (i = 0; i < D_8036EB90; i++) {
        if (!D_8036BED8[i].visited && func_80270A54(&D_8036BED8[i])) {
            gSPVertex(gdl++, D_8036BED8[i].vtx[D_8036BEE0], 4, 0);
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
        }
    }
    gDPPipeSync(gdl++);
    gDPLoadTextureBlock(gdl++, osVirtualToPhysical(D_802F9E00), G_IM_FMT_RGBA, G_IM_SIZ_32b, 16, 16, 0,
                        G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    for (i = 0; i < D_8036EB90; i++) {
        if (D_8036BED8[i].visited && func_80270A54(&D_8036BED8[i])) {
            gSPVertex(gdl++, D_8036BED8[i].vtx[D_8036BEE0], 4, 0);
            gSP1Triangle(gdl++, 0, 1, 2, 0);
            gSP1Triangle(gdl++, 0, 2, 3, 0);
        }
    }
    gDPPipeSync(gdl++);
    *gfx = gdl;
    D_8036BEDC = D_80364414;
}

s32 func_80270A54(YoshiNode *node, s32 arg1) {
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
