#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#define LEGACY_D_8036EA60
#define LEGACY_D_8036EA70
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuItem *) D_8020C070)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F5804 ((MenuItem *) D_802F5804)
#define D_802F8BDC ((MenuItem *) D_802F8BDC)
#define D_80364AF0 ((PlayerRec *) D_80364AF0)
#define D_8036EA80 ((u32 *) &D_8036EA80)
#define D_8036EA90 ((u32 *) &D_8036EA90)
#ifdef NON_MATCHING
#define D_8036EA60 (*(u32 *) &D_8036EA60)
#define D_8036EA70 (*(u32 *) &D_8036EA70)
#endif
/* end of views */

/* stats_perm.c: per-level results (score, time, counts and the best/saved
 * copies), the end-of-level status screen and the status save to the pak. */

/* Per-player save record, 0x100 bytes, indexed by level */
typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];          /* 0x18 per level: 1..5 once completed */
    u8 flags[0x100 - 0x54]; /* 0x54 per level */
} PlayerRec;

typedef struct {
    u8 type; /* 1: no status save */
    u8 pad[0x43];
} LevelInfo; /* 0x44 */

/* A text/menu item */
typedef struct {
    u16 flags;
    u8 pad[6];
    u32 f8;
    s16 fC;
    u8 pad2[0xA];
    s16 f18;
    u8 pad3[2];
} MenuItem; /* 0x1C */

extern u8 D_80364B08[][0x100]; /* = D_80364AF0[p].rank */
extern u8 D_80364B44[][0x100]; /* = D_80364AF0[p].flags */

/* This level's results (D_8036EA70) and the best (D_8036EA60), 16 bytes each:
 * money, time, then three counts with their targets in D_8036EB90..93 */
#ifndef NON_MATCHING
extern u32 D_8036EA60;
#endif
extern u8 D_8036EA68;
extern u8 D_8036EA69;
extern u16 D_8036EA6C;
#ifndef NON_MATCHING
extern u32 D_8036EA70;
#endif
extern s32 D_8036EA74;
extern u8 D_8036EA7A;
extern u8 D_8036EA7B;
extern u8 D_8036EB94[]; /* bonus earned, per bonus */
extern u8 D_8036EB9C[];

#ifndef NON_MATCHING
extern s32 D_80367738;
#endif

#define CUR (D_80364AF0[D_80364AE8])
#define LEVEL_DONE(l) ((CUR.rank[l] > 0 && CUR.rank[l] < 6) ? 1 : 0)

#define frontEndPresent D_80370C50
#define pakBuffer D_8039C4B8
#define create_status func_802C4E58
#define LEVEL_SAVE_SIZE 64
#define STATS_ASSERT(EX, line) \
    if (!(EX)) { func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "stats_perm.c", line); }

/* Start-of-level: load this level's previous counts and reset the bonuses */
void func_80285190(void) {
    s32 i;

    D_8036EA7B = D_80364AF0[D_80364AE8].flags[D_802E8BDC + 0x3E];
    D_8036EA74 = D_80364EF0[D_80364AE8][D_802E8C44[(D_802E8F94[D_802E8BDC].type == 1) ? 1 : D_8036EA7B]];
    D_8036EA7A = D_80364B08[D_80364AE8][D_802E8BDC] % 8;
    for (i = 0; i < 4; i++) {
        D_8036EB94[i] = 0;
    }
    D_8036EB98 = LEVEL_DONE(D_802E8BDC);
}

/* Print the status screen lines, highlight improved results, return the
 * average completion percentage */
u32 func_802852EC(void) {
    u32 pctA;
    u32 pctB;
    u32 pctC;
    u32 i;
    s32 f;
    s32 f2;
    s32 f1;
    char buf[0x20];

    f2 = 0;
    f1 = 0;
    if (D_8036EB92) {
        pctA = (D_8036EA78 * 100) / D_8036EB92;
    } else {
        pctA = 100;
    }
    if (D_8036EB93) {
        pctB = (D_8036EA79 * 100) / D_8036EB93;
    } else {
        pctB = 100;
    }
    if (D_8036EB90) {
        pctC = (D_8036EA7C * 100) / D_8036EB90;
    } else {
        pctC = 100;
    }
    func_802D6A60(D_8036B9A8, "***%2d (%d%c)*", D_8036EA78, pctA, '%');
    func_802D6A60(D_8036B9A8 + 0x20, "***$%d*", D_8036EA70);
    func_802D6A60(D_8036B9A8 + 0x40, "***%2d (%d%c)*", D_8036EA79, pctB, '%');
    func_802D6A60(D_8036B9A8 + 0x60, "***%2d (%d%c)*", D_8036EA7C, pctC, '%');
    func_80264A34(buf, D_8036EA74, 0);
    func_802D6A60(D_8036B9A8 + 0x80, "***%s*", buf);
    for (i = 18; i < 23; i++) {
        D_802F5804[i].flags = 0x400;
    }
    for (i = 14; i < 18; i++) {
        D_8020C070[i].flags = 0x400;
    }
    if (D_80364A98 == 0x40) {
        f2 = 0x100;
    }
    if (D_80364A90 & 0x30C) {
        f1 = 0x100;
    }
    if (D_8036EA78 > D_8036EA68) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    D_8020C070[14].flags |= f | 0x80;
    D_802F5804[18].flags |= f | f2 | 0x80;
    if (D_8036EA70 > D_8036EA60) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    D_8020C070[15].flags |= f | 0x80;
    D_802F5804[19].flags |= f | f2 | 0x80;
    if (D_8036EA79 > D_8036EA69) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    D_8020C070[16].flags |= f | 0x80;
    D_802F5804[20].flags |= f | f2 | 0x80;
    if (D_8036EA7C > D_8036EA6C) {
        f = f1 | 4;
    } else {
        f = 0;
    }
    D_8020C070[17].flags |= f | 0x80;
    D_802F5804[21].flags |= f | f2 | 0x80;
    D_802F5804[22].flags |= f2 | 0x80;
    if (D_802E8BF8) {
        return (pctA + pctC) / 2;
    } else {
        return (pctA + pctB + pctC) / 3;
    }
}

/* End-of-level: write the status to the pak if this level saves one, then
 * copy the results to the best/saved slots. Returns 1 if a status was written. */
u8 func_80285814(void) {
    u8 saved = 0;
    u8 coin;

    STATS_ASSERT(frontEndPresent, 140);
    frontEndPresent = 1;
    func_80255DC8();
    if (D_80364A90 == 0x4000) {
        osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
    }
    if (LEVEL_DONE(D_802E8BDC)) {
        if (D_802E8F94[D_802E8BDC].type == 1) {
            if (pakBuffer[0] == 0x1234567887654321) {
                func_80256A34(NULL);
                func_8029A7E4("Creating status ...\n");
                coin = D_80364B08[D_80364AE8][D_802E8BDC];
                if (coin == 5) {
                    coin = 4;
                }
                STATS_ASSERT(create_status(pakBuffer,coin)<=LEVEL_SAVE_SIZE-4, 161);
                func_802C4BF0(pakBuffer);
                func_802C1DD0(0);
                func_80264C20((s32) pakBuffer);
                saved = 1;
            } else {
                func_80256A34((s32) pakBuffer);
            }
        } else {
            func_80256A34(NULL);
        }
    } else {
        func_80256A34(NULL);
    }
    func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
    func_80285A78((u8 *) &D_8036EA70, (u8 *) D_8036EA80);
    func_80285A78((u8 *) &D_8036EA70, (u8 *) D_8036EA90);
    return saved;
}

/* Copy a 16-byte results record */
void func_80285A78(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}

/* Set flag `bit` (1-based) for the current level. The shift is 32 or more:
 * MIPS sllv uses its low 5 bits (bit 1 -> 1 << 0); C leaves it undefined,
 * so the port masks it (PORT_SHAMT). */
void func_80285AB0(u8 bit) {
    D_80364A87 |= 2;
    CUR.flags[D_802E8BDC] |= 1 << PORT_SHAMT(bit + 0x1F);
}

/* Test flag `bit` (1-based) for the current level */
s32 func_80285B10(u8 bit) {
    s32 unused;

    return (D_80364B44[D_80364AE8][D_802E8BDC] & (1 << PORT_SHAMT(bit + 0x1F))) ? 1 : 0;
}

/* Level just completed by the active player: mark it and fade out */
void func_80285B68(s32 arg0) {
    s32 unused;

    if (D_80364A90 & 0x104) {
        if (LEVEL_DONE(D_802E8BDC)) {
            if (D_802E8F94[D_802E8BDC].type != 1 && D_80364AE8 == D_80364AEA) {
                func_802CF5B0();
                D_802E8BD8 = 1;
                func_80275270(0x4000, 1.25f);
                D_8039C53C[D_80364AE8] = D_802E8BDC + 1;
                D_8036EB99 = 1;
            }
        }
    }
}

void func_80285CA0(void) {
    func_8026AD30(0x47);
}

/* Award each bonus once its count reaches the target, with a jingle */
void func_80285CC0(void) {
    s32 sfx = -1;
    s32 snd = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        D_8036EB9C[i] = D_8036EB94[i];
        if (D_8036EB94[i] == 0) {
            switch (i) {
                case 0:
                    if (D_8036EB94[i] = (D_8036EA7C == D_8036EB90)) {
                        sfx = 0x39;
                    }
                    break;
                case 1:
                    if (D_8036EB94[i] = (D_8036EA79 == D_8036EB93)) {
                        sfx = 0x3A;
                    }
                    break;
                case 2:
                    func_802C1DD0(0);
                    if (D_8036EB94[i] = (D_8036EA78 == D_8036EB92)) {
                        sfx = 0x3B;
                    }
                    break;
                case 3:
                    if (D_8036EB94[i] = (D_8036EB94[0] && D_8036EB94[2] && D_8036EB94[1])) {
                        sfx = 0x3C;
                        snd = 0xC6;
                    }
                    break;
            }
        }
    }
    if (D_80358064) {
        if (sfx != -1 && !(func_8026B10C() & 0x8000)) {
            func_8026AF6C(sfx | 0x8000);
        }
        if (snd) {
            func_80260650(D_80367738, snd, 0);
        }
    }
}

/* Level finished at frame `start`: build the status screen with the elapsed
 * time and save the results */
void func_80285EF4(s32 start) {
    s32 t;

    t = func_8028604C(((s32) D_803156C0) - start);
    func_802C1DD0(0);
    D_8036EA74 += t;
    func_802852EC();
    D_8036EA74 -= t;
    func_80285A78((u8 *) &D_8036EA70, (u8 *) &D_8036EA60);
    D_802F5804[24].flags &= ~1;
    D_802F5804[24].flags |= 0x800;
    D_802F5804[23].flags &= ~1;
    D_802F5804[23].flags |= 0x800;
    D_802F5804[17].flags |= 1;
    D_802F5804[17].flags &= ~0x800;
    D_802F8BDC[6].f18 = 0x11;
    D_802F8BDC[6].f8 |= 0x80;
    func_8026AF6C(0x8006);
    D_802F8BDC[6].fC = 0;
    D_802E8BD8 = 1;
}

/* Time units (1/10 s) to frames */
s32 func_80286038(u16 arg0) {
    return arg0 * 6;
}

/* Frames to time units (1/10 s), capped at 59999 */
u16 func_8028604C(u32 frames) {
    s32 unused;

    return (frames / 6 >= 60000) ? 59999 : frames / 6;
}

/* Has the current player completed `level`? */
u8 func_80286090(s32 level) {
    s32 unused;

    return LEVEL_DONE(level);
}
