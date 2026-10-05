#include "common.h"
#include <ultra64.h>

/*
 * stats.c (named by its assert): end-of-level results. Grades the time,
 * records best times and units, and sets up the results screen.
 */

/* front-end view of hd_code's Player record (0x100 bytes, D_80364AF0) */
typedef struct {
    u8 pad0[0xA];
    u16 unkA; /* units */
    u8 unkC;
    u8 padD[7];
    s32 unk14;
    u8 unk18[60]; /* per level: grade */
    u8 unk54[60]; /* per level: bits */
    u8 pad90;
    u8 unk91;
    u8 unk92[60]; /* per level */
    u8 padCE[0x100 - 0xCE];
} FePlayer;

typedef struct {
    u8 pad0[4];
    char *unk4; /* level name */
    u8 pad8[0x10];
    s8 unk18[0x18]; /* -1 terminated */
} FeLevelEntry;     /* 0x30 bytes, one per level */

typedef struct {
    u8 unk0; /* type */
    u8 pad1[0x2F];
    u16 unk30; /* time thresholds, best first */
    u16 unk32;
    u16 unk34;
    u16 unk36;
    u8 pad38[0xC];
} LevelInfo;

/* hd_code's Score (00000.c) */
typedef struct {
    s32 ip;
    u32 tc; /* time */
    u8 bd;
    u8 cr;
    u8 coin; /* grade */
    u8 bdn;
    u16 rt;
} Score;

typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char *unkC;  /* title */
    void *unk10; /* glyph list */
    u8 unk14;
    u8 pad15[5];
    u8 unk1A;
    u8 pad1B;
} MenuEntry;

typedef struct {
    u8 pad0[4];
    u8 unk4;
    u8 pad5;
    u16 unk6[19];
    u8 unk2C;
    u8 unk2D;
    u8 pad2E[2];
} IconInfo;

extern FePlayer D_80364AF0[];
extern u8 D_80364AE8;  /* current player */
extern u8 D_80364AEA;
extern s32 D_802E8BDC; /* current level */
extern FeLevelEntry D_8020D810[];
extern LevelInfo D_802E8F94[];
extern Score D_8036EA60;
extern Score D_8036EA70;
extern Score D_8036EA80;
extern Score D_8036EA90;
extern u8 D_803643D4;
extern u8 D_803643D5;
extern s32 D_803649F0;
extern u64 D_80364A98;
extern u8 D_802E8C44[];
extern u16 D_80364EF0[][16]; /* best times, per player */
extern char D_8036B980[];
extern char D_8036B9A8[];
extern MenuEntry D_8020C070[];
extern IconInfo D_802F49F4[];
extern u16 D_80303B3C[];
extern u16 D_80303B48[];
extern u16 D_80303B58[];
extern u16 D_80303B68[];
extern s32 D_80358070; /* heap pointer */
extern s16 D_802159D0;
extern s32 D_802159D4;
extern s32 D_802159D8;
extern s16 D_802159DC;
extern f32 D_802159E0;
extern f32 D_802159E4;

/* ROM bounds of two compressed blobs (the first ends where the second starts) */
extern u8 D_0048F5A0[];
extern u8 D_0048F5A0_end[];
extern u8 D_0048F970[];
extern u8 D_0048F970_end[];

void func_8029A7E4(const char *fmt, ...);
int sprintf(char *, const char *, ...);
u32 func_802852EC(void);
s32 func_80286038(u16);
void func_80295A20(u32);
void func_80264A34(char *buf, u16 t, s32 arg2);
u8 func_80272C5C(u16 *ids, s32 arg1, u8 count, u8 frames, u8 flags, f32 scale);
void func_801E8DCC(u8);
void func_801F4E70();
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);
u8 func_801EEDB4();
u8 func_801EF2BC();

#define levelno D_802E8BDC
#define DUMMY_LEVELS(l) ((l) == 49 || (l) == 47 || (l) == 38)
#define STATS_ASSERT(EX, line) \
    if (!(EX))                 \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "stats.c", line)

#define PRINT_SCORE(name, s)                                                                                       \
    func_8029A7E4(name " ip=%8d : tc=%5d : bd=%2d : cr=%2d : rt=%3d : coin=%1d : bdn=%1d\n", (s).ip, (s).tc, (s).bd, \
                  (s).cr, (s).rt, (s).coin, (s).bdn)

/* Finish a level: grade it, update units and best times. *arg0 = 1 if a rank was gained. */
u8 func_801EE800(u8 *arg0, u8 arg1, u8 arg2) {
    FePlayer *p;
    LevelInfo *l;
    u32 t;
    u8 ret;
    s32 pad[2];

    p = &D_80364AF0[D_80364AE8];
    l = &D_802E8F94[levelno];
    PRINT_SCORE("new", D_8036EA70);
    PRINT_SCORE("old", D_8036EA60);
    PRINT_SCORE("res", D_8036EA80);
    PRINT_SCORE("rs2", D_8036EA90);
    func_8029A7E4("units %d\n", p->unkA);
    if (D_802E8F94[levelno].unk0 == 1) {
        t = func_802852EC();
        if (arg2) {
            if (arg1) {
                if (t >= 100) {
                    D_8036EA70.coin = 3;
                } else if (t >= 90) {
                    D_8036EA70.coin = 2;
                } else if (t >= 70) {
                    D_8036EA70.coin = 1;
                } else {
                    D_8036EA70.coin = 5;
                }
                if (D_803643D5) {
                    func_8029A7E4("Units up 3\n");
                    p->unkA += 3;
                }
                D_8036EA70.bdn = 1;
            } else {
                D_8036EA70.coin = 0;
            }
        }
        ret = D_8036EA70.coin;
    } else {
        ret = func_801EEDB4(levelno, arg1, arg2);
    }
    sprintf(D_8036B980, "%s", D_8020D810[levelno].unk4);
    *arg0 = 0;
    if (arg1 && arg2) {
        STATS_ASSERT(!DUMMY_LEVELS(levelno), 94);
        if (D_802E8F94[levelno].unk0 == 1) {
            p->unk14 = D_803649F0;
        }
        if (p->unkA < 360) {
            func_8029A7E4("UNITS UP %d\n", D_8036EA70.coin % 5 - D_8036EA60.coin % 5);
            p->unkA += D_8036EA70.coin % 5 - D_8036EA60.coin % 5;
        }
        if (p->unkA == 354) {
            p->unkA += 6;
        }
        if (p->unkA / 12 > p->unkC) {
            *arg0 = 1;
            p->unkC++;
        }
        if (!(D_802E8F94[levelno].unk0 & 0x81)) {
            p->unk92[levelno] = D_8036EA70.bdn;
        }
        D_80364EF0[D_80364AE8][D_802E8C44[D_8036EA70.bdn]] = D_8036EA70.tc;
        if (D_803643D5 && D_802E8F94[levelno].unk0 == 1) {
            D_80364EF0[D_80364AE8][D_802E8C44[0]] = D_8036EA70.tc;
        }
        p->unk18[levelno] = ret;
        func_801E8DCC(D_80364AE8);
    }
    return ret;
}

/* results screen banner and its glyph list, by outcome */
char *D_802084D0[] = { "YOUR NEW BEST!", "BEST TO DATE", "YOUR BEST STAYS", "GUEST BEST IS" };
u16 *D_802084E0[] = { D_80303B3C, D_80303B48, D_80303B58, D_80303B68 };

/* Grade a timed level `arg0` and record the best time; returns the grade. */
u8 func_801EEDB4(arg0, arg1, arg2)
    u8 arg0;
    u8 arg1;
    u8 arg2;
{
    s32 x;
    s32 pad;
    s32 idx;
    FePlayer *p;
    LevelInfo *l;
    IconInfo *e;
    MenuEntry *m;
    char buf[32];
    u16 old;

    p = &D_80364AF0[D_80364AE8];
    l = &D_802E8F94[arg0];
    if (D_80364A98 == 0x08000000 && arg1 && l->unk0 == 2) {
        func_80295A20(func_80286038(D_8036EA70.tc));
    }
    if (arg2 && arg1) {
        if (D_802E8F94[arg0].unk0 == 0x80) {
            D_8036EA70.bdn = 0;
        } else if (D_8036EA70.tc <= D_8036EA60.tc) {
            D_8036EA70.bdn = D_803643D4;
        } else if (D_8036EA70.tc != 0xFFFF &&
                   ((old = D_80364EF0[D_80364AE8][D_802E8C44[D_803643D4]]) == 0 || D_8036EA70.tc < old)) {
            D_80364EF0[D_80364AE8][D_802E8C44[D_803643D4]] = D_8036EA70.tc;
        }
    }
    if (D_8036EA70.tc <= D_8036EA60.tc && arg1) {
        D_8036EA70.coin = func_801EF2BC(D_8036EA70.tc, arg0, D_80364AF0[D_80364AE8].unk91);
    } else {
        D_8036EA70.tc = D_8036EA60.tc;
    }
    if (D_8036EA70.tc < D_8036EA60.tc && !D_803643D5) {
        x = 0x484;
        if (arg2) {
            x = 0x584;
        }
    } else {
        x = 0x480;
    }
    func_80264A34(buf, D_8036EA70.tc, 0);
    sprintf(&D_8036B9A8[0x80], "****%s*", buf);
    if (arg1) {
        m = &D_8020C070[25];
        D_8020C070[25].unk0 = x;
        func_8029A7E4("getting icon %d\n", D_8036EA70.bdn);
        m->unk14 = D_8036EA70.bdn + 0x22;
        e = &D_802F49F4[m->unk14];
        m->unk1A = func_80272C5C(e->unk6, 0, e->unk4, e->unk2C, e->unk2D | 4, 1.0f);
        if (D_80364AE8 != D_80364AEA) {
            idx = 3;
        } else if (D_80364A98 == 0x80 || D_803643D5) {
            idx = 1;
        } else if (D_8036EA70.tc < D_8036EA60.tc) {
            idx = 0;
        } else {
            idx = 2;
        }
        D_8020C070[24].unkC = D_802084D0[idx];
        D_8020C070[24].unk10 = D_802084E0[idx];
    }
    return D_8036EA70.coin;
}

/* Count the current level's entries (at most 2) whose bit is set for the player. */
s8 func_801EF1E0(void) {
    FeLevelEntry *e;
    s32 i;
    s32 count;

    e = &D_8020D810[D_802E8BDC];
    if (e->unk18[0] == -1) {
        return -1;
    }
    for (i = 0, count = 0; e->unk18[i] != -1 && i < 2; i++) {
        if (D_80364AF0[D_80364AE8].unk54[D_802E8BDC] & (1 << i)) {
            count++;
        }
    }
    return count;
}

/* Grade a time against level `arg1`'s thresholds: 4 (best, needs arg2 >= 12) .. 1, else 5. */
u8 func_801EF2BC(arg0, arg1, arg2)
    u16 arg0;
    u8 arg1;
    u8 arg2;
{
    u8 ret;
    LevelInfo *l;

    l = &D_802E8F94[arg1];
    if (l->unk30 >= arg0 && arg2 >= 12) {
        ret = 4;
    } else if (l->unk32 >= arg0) {
        ret = 3;
    } else if (l->unk34 >= arg0) {
        ret = 2;
    } else if (l->unk36 >= arg0) {
        ret = 1;
    } else {
        ret = 5;
    }
    return ret;
}

/* Results screen init: load scene `arg0` and inflate the two blobs onto the heap. */
void func_801EF380(s32 arg0) {
    s32 size1;
    s32 size2;

    size1 = D_0048F5A0_end - D_0048F5A0;
    size2 = D_0048F970_end - D_0048F970;
    func_801F4E70(arg0);
    if (arg0 == 2) {
        D_802159D0 = 90;
    } else {
        D_802159D0 = 0;
    }
    func_8028B4C4(D_0048F5A0, (void *) D_80358070, &size1, 12, 0, 1);
    D_802159D4 = D_80358070;
    D_80358070 += size1;
    func_8028B4C4(D_0048F970, (void *) D_80358070, &size2, 12, 0, 1);
    D_802159D8 = D_80358070;
    D_80358070 += size2;
    D_802159DC = arg0;
    D_802159E0 = 0.0f;
    D_802159E4 = 3.0f;
}

/* 8 unreferenced zero bytes sit between the last string and func_801EF4AC's
 * double (0x8020EFA4..AB); probably compiled-out debug strings. */
const u32 D_8020EFA4[2] = { 0, 0 };

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/07800/func_801EF4AC.s")
