#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_8039C4B4
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuItem *) D_8020C070)
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define D_802F8BDC ((s16 *) D_802F8BDC)
#define D_80310BD0 ((u8 *) &D_80310BD0)
#define D_80315440 ((u8 *) &D_80315440)
#define D_80364AF0 ((Player *) D_80364AF0)
#define D_8039C4B8 ((u8 *) D_8039C4B8)
#ifdef NON_MATCHING
#define D_8039C4B4 (*(s32 *) &D_8039C4B4)
#endif
/* end of views */

/*
 * pfsHandler.c: the Controller Pak / EEPROM save thread. Player records
 * (0x100 bytes each) and best times (0x20 bytes each) go to a "NBCE"/"01"
 * Controller Pak file, or to EEPROM, each block followed by a 4-byte CRC.
 */

u8 __osContDataCrc(u8 *data);
s32 osPfsInitPak(OSMesgQueue *, OSPfs *, int);

#define PFS_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "pfsHandler.c", line)
#define PFS_FILE_SIZE 0xE00

/*
 * The old-SDK EEPROM calls, by address: symbol_addrs.hd_front_end had the
 * Read/Write names swapped (being fixed in main), so these resolve to the
 * right functions before and after the rename. Drop the macros once merged.
 */
s32 func_80204C10(OSMesgQueue *, u8, u8 *);
s32 func_802050F0(OSMesgQueue *, u8, u8 *);
s32 func_802042D0(OSMesgQueue *, u8, u8 *, int);
s32 func_80204410(OSMesgQueue *, u8, u8 *, int);
#define osEepromWrite func_80204C10
#define osEepromRead func_802050F0
#define osEepromLongWrite func_802042D0
#define osEepromLongRead func_80204410

typedef struct {
    u32 file_size;
    u32 game_code;
    u16 company_code;
    u8 ext_name[4];
    u8 game_name[16];
} OldPfsState; /* the old SDK's OSPfsState: no pad after company_code */

typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];     /* 0x18 */
    u8 pad54[0x92 - 0x54];
    u8 timeSlot[0x3C]; /* 0x92: best-time slot per level */
    u8 padCE[0x100 - 0xCE];
} Player;

typedef struct {
    u8 type; /* 1: no status save */
    u8 pad[0x43];
} LevelInfo; /* 0x44 */

typedef struct {
    u8 pad0[0xC];
    char *text;  /* 0x0C */
    void *unk10; /* 0x10 */
    u8 pad14[8];
} MenuItem; /* 0x1C */

extern OSPfs D_8039B630;      /* hd_code .bss: the Controller Pak file system */
extern u8 D_8020C000[];       /* game name */
extern u8 D_8020C014[];       /* extension name */
extern u8 D_8020C01C[];       /* character set (0x45 entries) */
extern OldPfsState D_80218B20[]; /* the pak's files */
extern s32 D_80218D28;        /* number of files listed */
extern u8 D_802189C0[][17];   /* file game names */
extern u8 D_80218AD0[][5];    /* file extensions */
extern char D_80218740[][40]; /* file menu lines */
extern s32 D_80218EF0;        /* free bytes */
extern char D_80219F90[];
extern char D_80219FB0[];
extern u8 D_80301080[];
extern s32 D_8039B698[];      /* file_no per player */
extern u8 D_80218EF8[];       /* thread stack (0x1000) */
extern OSMesg D_80219F10[];
extern OSMesgQueue D_80219F30;
extern OSMesg D_80219F48[];
extern OSMesg D_80219F68[];
#ifndef NON_MATCHING
extern s32 D_8039C4B4;
#endif
extern u8 D_8039B6B0[];
extern u8 D_8020BEE0[];

int sprintf(char *, const char *, ...);
void bcopy(const void *, void *, int);

#define playerInfo (D_80364AF0[0])
#define playerNumberAtStart D_80364AEA

void func_801F58E8(void *);
void func_801F7410(u8 *str);
void func_801F74B0(u8 *str);
s32 func_801F75A4(u8 *data, s32 size);
s32 func_801F76E4(u8 *data, s32 size);
s32 func_801F6BD0(u8 player, u64 *sem);

void func_801F57B0(void) {
    s32 filesize;

    filesize = 0xDE0;
    osCreateThread(&D_80218D30, 2, func_801F58E8, NULL, D_80218EF8 + 0x1000, 0xB);
    osCreateMesgQueue(&D_80219EF8, D_80219F10, 8);
    osCreateMesgQueue(&D_80219F30, D_80219F48, 1);
    osCreateMesgQueue(&D_80219F50, D_80219F68, 8);
    func_801F74B0(D_8020C000);
    func_801F74B0(D_8020C014);
    func_8029A7E4("current pak file size is %d bytes\n", filesize);
    func_8029A7E4("current playerInfo size is %d bytes\n", 0x100);
    func_80270E50((struct BcSched *) D_80315440, (struct BcScClient *) D_80218EE0, &D_80219F30, 1, 3);
    PFS_ASSERT(filesize<PFS_FILE_SIZE, 104);
    osStartThread(&D_80218D30);
}

/*
 * TODO: func_801F58E8, the save thread's main loop (447 insns). The C below
 * matches every instruction except one: the target ends `case 1:` (the last
 * case body, at 0x801F5EB4) with a `b` to the very next instruction
 * (0x801F5EE8, the switch end) carrying `sb done` in its delay slot. IDO drops
 * that branch for every spelling tried: explicit break/goto/else, trailing
 * `default:`/`case 4:` (4 is the jtbl's default), do/while(0), while+break,
 * dead code after the break, an unused label. The callers' func_801F6160,
 * func_801F61C8 and func_801F6ED4 return values are used here (original
 * declared them s32 with no return statement), and func_801F58E8 takes no
 * argument (cast it for osCreateThread).
 *
 * The NON_MATCHING build uses this C (below the comment; it keeps the
 * osCreateThread prototype's unused argument), checked with
 * tools_port/checks/fe_0E7B0.txt; func_801F6160/61C8/6ED4 now return their
 * osPfs* call's result explicitly, which compiles to the same bytes.
 *
 * void func_801F58E8(void) {
 *     OSMesg msg;
 *     s32 ret;
 *     s32 lastRet;
 *     u8 pad33;
 *     u8 player;
 *     u8 arg1;
 *     u8 reply;
 *     u8 cmd;
 *     u8 next;
 *     u8 done;
 *     s32 yoshi;
 *     u32 tries;
 *
 *     while (1) {
 *         D_8039C4B0 = 0;
 *         tries = 0;
 *         osSetEventMesg(OS_EVENT_SI, &D_80370BF8, 0);
 *         osRecvMesg(&D_80219EF8, &msg, OS_MESG_BLOCK);
 *         osSetEventMesg(OS_EVENT_SI, &D_80370BF8, 0);
 *         while (D_8036BF10 != 0) {
 *         }
 *         cmd = (u32) msg & 0xFF;
 *         arg1 = ((u32) msg >> 8) & 0xFF;
 *         player = ((u32) msg >> 16) & 0xFF;
 *         reply = ((u32) msg >> 24) & 0xFF;
 *         D_8020C014[0] = player + 0x11;
 *         D_8039C4B0 = 1;
 *         func_8028A42C();
 *         func_801EE390();
 *         D_80218D24 = 0;
 *         do {
 *             done = 0;
 *             next = 0;
 *             yoshi = 0x5B;
 *             lastRet = 0;
 *             switch (cmd) {
 *                 case 1:
 *                 case 2:
 *                     ret = func_801F60C8();
 *                     break;
 *                 case 3:
 *                     ret = func_801F6160(player);
 *                     break;
 *                 case 4:
 *                     ret = func_801F61C8(player);
 *                     break;
 *                 case 5:
 *                     ret = func_801F6210(player);
 *                     break;
 *                 case 6:
 *                     ret = func_801F6264(player, 0);
 *                     break;
 *                 case 7:
 *                     ret = func_801F6264(player, 1);
 *                     break;
 *                 case 8:
 *                     ret = func_801F65C4(player, arg1, 0);
 *                     break;
 *                 case 9:
 *                     ret = func_801F65C4(player, arg1, 1);
 *                     break;
 *                 case 10:
 *                     ret = func_801F67E4(player, arg1, 0);
 *                     break;
 *                 case 11:
 *                     ret = func_801F67E4(player, arg1, 1);
 *                     break;
 *                 case 12:
 *                     ret = func_801F6CA4(player, arg1, 0);
 *                     break;
 *                 case 13:
 *                     ret = func_801F6CA4(player, arg1, 1);
 *                     break;
 *                 case 14:
 *                     ret = osPfsFreeBlocks(&D_8039B630, &D_80218EF0);
 *                     break;
 *                 case 15:
 *                     ret = func_801F5FE4();
 *                     break;
 *                 case 16:
 *                     done = 1;
 *                     ret = osEepromProbe(&D_80370BF8);
 *                     break;
 *                 case 17:
 *                     ret = func_801F6ED4(player);
 *                     break;
 *                 case 18:
 *                     ret = osPfsChecker(&D_8039B630);
 *                     break;
 *                 case 19:
 *                     ret = 10;
 *                     break;
 *                 case 20:
 *                     ret = func_801F6AF4(player, 0x2704197125121981);
 *                     break;
 *                 case 21:
 *                     ret = func_801F6AF4(player, 0x87569AB6CD076AEC);
 *                     break;
 *                 case 22:
 *                     ret = 0;
 *                     break;
 *                 default:
 *                     func_8029A7E4("Nonsense pak message\n");
 *                     break;
 *             }
 *             func_8029A7E4("pak command %d returned %d\n", cmd, ret);
 *             switch (ret) {
 *                 case 0x6E382:
 *                     if ((D_80364A90 & 0x10E18000) || (D_80364A98 & 0x20000000000000)) {
 *                         done = 1;
 *                         break;
 *                     }
 *                     // fallthrough
 *                 case 6:
 *                 case 10:
 *                 case 11:
 *                     if (tries >= 4) {
 *                         if (cmd != 0x13) {
 *                             if (ret == 0x6E382) {
 *                                 D_80219F88 = 0x5D;
 *                             } else {
 *                                 D_80219F88 = 0x5C;
 *                             }
 *                             func_801F6AF4(player, 0x2704197125121981);
 *                             next = 0x13;
 *                         }
 *                         yoshi = D_80219F88;
 *                     } else {
 *                         yoshi = 0;
 *                         tries++;
 *                     }
 *                     break;
 *                 case 0:
 *                 case 5:
 *                 case 9:
 *                     done = 1;
 *                     break;
 *                 case 8:
 *                     if (!(D_80364A90 & 0x10E18000) || func_801F5FE4() != 0) {
 *                         break;
 *                     }
 *                     // fallthrough
 *                 case 7:
 *                     D_8039C538 = (player < D_8039C538) ? player : D_8039C538;
 *                     done = 1;
 *                     break;
 *                 case 3:
 *                     if (tries >= 4) {
 *                         if (cmd != 0x13) {
 *                             func_801F6AF4(player, 0x2704197125121981);
 *                             next = 0x13;
 *                         }
 *                         yoshi = 0x5C;
 *                     } else {
 *                         func_8029A7E4("trying to fix pak ...\n");
 *                         if (cmd != 0x12) {
 *                             osSendMesg(&D_80219EF8, (OSMesg) (cmd | (arg1 << 8) | (player << 16) | (reply << 24)), OS_MESG_NOBLOCK);
 *                         }
 *                         next = 0x12;
 *                         reply = 0;
 *                         tries++;
 *                     }
 *                     break;
 *                 case 2:
 *                     if (lastRet == 8 && !(D_80364A90 & 0x10E18000)) {
 *                         PFS_ASSERT(1==0, 342);
 *                         done = 1;
 *                     }
 *                     break;
 *                 case 1:
 *                     if (D_80364A98 & 0x20000000000000) {
 *                         done = 1;
 *                     }
 *                     break;
 *             }
 *             lastRet = ret;
 *             if (D_8036BF10 == 0 && yoshi != 0 && !done && !next && D_80219F50.mtqueue == &D_80310BD0) {
 *                 func_801EE398(yoshi);
 *                 D_80218D24 = 1;
 *             }
 *             if (next) {
 *                 cmd = next;
 *                 next = 0;
 *             }
 *             osRecvMesg(&D_80219F30, NULL, OS_MESG_BLOCK);
 *         } while (!done);
 *         if (D_80218D24) {
 *             yoshiState = 1;
 *             currentYoshiWindow = -1;
 *         }
 *         if (reply) {
 *             osSendMesg(&D_80219F50, (OSMesg) ret, OS_MESG_BLOCK);
 *         }
 *     }
 * }
 */
#ifdef NON_MATCHING
extern s32 D_80218D24;     /* a Yoshi window was opened for a pak error */
extern s32 D_80219F88;     /* Yoshi window of the last pak error */
s32 func_801F5FE4(void);
s32 func_801F60C8(void);
s32 func_801F6160(u8 player);
s32 func_801F61C8(s32 arg0);
s32 func_801F6210(u8 item);
s32 func_801F6264(u8 player, u8 rw);
s32 func_801F65C4(u8 player, u8 slot, u8 rw);
s32 func_801F67E4(u8 pn, u8 level, u8 rw);
s32 func_801F6AF4(u8 player, u64 sem);
s32 func_801F6CA4(u8 player, u8 nlevels, u8 rw);
s32 func_801F6ED4(u8 arg0);

/* The save thread: one pak/EEPROM command per message (cmd | arg1 << 8 |
 * player << 16 | reply << 24), retried until it settles, the result sent
 * back on D_80219F50 when `reply` is set. */
void func_801F58E8(void *arg) {
    OSMesg msg;
#ifdef NON_MATCHING
    s32 ret = 0; /* a "Nonsense pak message" keeps the previous command's result; on the first pass the original reads an indeterminate register */
#else
    s32 ret;
#endif
    s32 lastRet;
    u8 pad33;
    u8 player;
    u8 arg1;
    u8 reply;
    u8 cmd;
    u8 next;
    u8 done;
    s32 yoshi;
    u32 tries;

    while (1) {
        D_8039C4B0 = 0;
        tries = 0;
        osSetEventMesg(OS_EVENT_SI, &D_80370BF8, 0);
        osRecvMesg(&D_80219EF8, &msg, OS_MESG_BLOCK);
        osSetEventMesg(OS_EVENT_SI, &D_80370BF8, 0);
        while (D_8036BF10 != 0) {
        }
        cmd = (u32) msg & 0xFF;
        arg1 = ((u32) msg >> 8) & 0xFF;
        player = ((u32) msg >> 16) & 0xFF;
        reply = ((u32) msg >> 24) & 0xFF;
        D_8020C014[0] = player + 0x11;
        D_8039C4B0 = 1;
        func_8028A42C();
        func_801EE390();
        D_80218D24 = 0;
        do {
            done = 0;
            next = 0;
            yoshi = 0x5B;
            lastRet = 0;
            switch (cmd) {
                case 1:
                case 2:
                    ret = func_801F60C8();
                    break;
                case 3:
                    ret = func_801F6160(player);
                    break;
                case 4:
                    ret = func_801F61C8(player);
                    break;
                case 5:
                    ret = func_801F6210(player);
                    break;
                case 6:
                    ret = func_801F6264(player, 0);
                    break;
                case 7:
                    ret = func_801F6264(player, 1);
                    break;
                case 8:
                    ret = func_801F65C4(player, arg1, 0);
                    break;
                case 9:
                    ret = func_801F65C4(player, arg1, 1);
                    break;
                case 10:
                    ret = func_801F67E4(player, arg1, 0);
                    break;
                case 11:
                    ret = func_801F67E4(player, arg1, 1);
                    break;
                case 12:
                    ret = func_801F6CA4(player, arg1, 0);
                    break;
                case 13:
                    ret = func_801F6CA4(player, arg1, 1);
                    break;
                case 14:
                    ret = osPfsFreeBlocks(&D_8039B630, &D_80218EF0);
                    break;
                case 15:
                    ret = func_801F5FE4();
                    break;
                case 16:
                    done = 1;
                    ret = osEepromProbe(&D_80370BF8);
                    break;
                case 17:
                    ret = func_801F6ED4(player);
                    break;
                case 18:
                    ret = osPfsChecker(&D_8039B630);
                    break;
                case 19:
                    ret = 10;
                    break;
                case 20:
                    ret = func_801F6AF4(player, 0x2704197125121981);
                    break;
                case 21:
                    ret = func_801F6AF4(player, 0x87569AB6CD076AEC);
                    break;
                case 22:
                    ret = 0;
                    break;
                default:
                    /* ret keeps its previous value (unset on the first pass) */
                    func_8029A7E4("Nonsense pak message\n");
                    break;
            }
            func_8029A7E4("pak command %d returned %d\n", cmd, ret);
            switch (ret) {
                case 0x6E382:
                    if ((D_80364A90 & 0x10E18000) || (D_80364A98 & 0x20000000000000)) {
                        done = 1;
                        break;
                    }
                    // fallthrough
                case 6:
                case 10:
                case 11:
                    if (tries >= 4) {
                        if (cmd != 0x13) {
                            if (ret == 0x6E382) {
                                D_80219F88 = 0x5D;
                            } else {
                                D_80219F88 = 0x5C;
                            }
                            func_801F6AF4(player, 0x2704197125121981);
                            next = 0x13;
                        }
                        yoshi = D_80219F88;
                    } else {
                        yoshi = 0;
                        tries++;
                    }
                    break;
                case 0:
                case 5:
                case 9:
                    done = 1;
                    break;
                case 8:
                    if (!(D_80364A90 & 0x10E18000) || func_801F5FE4() != 0) {
                        break;
                    }
                    // fallthrough
                case 7:
                    D_8039C538 = (player < D_8039C538) ? player : D_8039C538;
                    done = 1;
                    break;
                case 3:
                    if (tries >= 4) {
                        if (cmd != 0x13) {
                            func_801F6AF4(player, 0x2704197125121981);
                            next = 0x13;
                        }
                        yoshi = 0x5C;
                    } else {
                        func_8029A7E4("trying to fix pak ...\n");
                        if (cmd != 0x12) {
                            osSendMesg(&D_80219EF8, (OSMesg) (cmd | (arg1 << 8) | (player << 16) | (reply << 24)),
                                       OS_MESG_NOBLOCK);
                        }
                        next = 0x12;
                        reply = 0;
                        tries++;
                    }
                    break;
                case 2:
                    /* never true: lastRet is reset at the top of every pass */
                    if (lastRet == 8 && !(D_80364A90 & 0x10E18000)) {
                        PFS_ASSERT(1==0, 342);
                        done = 1;
                    }
                    break;
                case 1:
                    if (D_80364A98 & 0x20000000000000) {
                        done = 1;
                    }
                    break;
            }
            lastRet = ret;
            if (D_8036BF10 == 0 && yoshi != 0 && !done && !next && D_80219F50.mtqueue == (OSThread *) D_80310BD0) {
                func_801EE398(yoshi);
                D_80218D24 = 1;
            }
            if (next) {
                cmd = next;
                next = 0;
            }
            osRecvMesg(&D_80219F30, NULL, OS_MESG_BLOCK);
        } while (!done);
        if (D_80218D24) {
            yoshiState = 1;
            currentYoshiWindow = -1;
        }
        if (reply) {
            osSendMesg(&D_80219F50, (OSMesg) ret, OS_MESG_BLOCK);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F58E8.s")
#endif

s32 func_801F5FE4(void) {
    s32 ret;
    u8 status;
    OSMesg msg;

    msg = 0;
    if (D_80370BF8.validCount != 0) {
        PFS_ASSERT(1==0, 400);
        osRecvMesg(&D_80370BF8, &msg, OS_MESG_NOBLOCK);
    }
    if (func_8028FCD4(&D_80370BF8, &status)) {
        ret = 1;
    } else if (!(status & 1)) {
        ret = 1;
    } else {
        ret = 0;
    }
    if (msg != 0) {
        PFS_ASSERT(1==0, 414);
        osSendMesg(&D_80370BF8, msg, OS_MESG_NOBLOCK);
    }
    return ret;
}

s32 func_801F60C8(void) {
    u8 status;
    s32 ret;

    ret = 0;
    D_8039C538 = 4;
    if (func_8028FCD4(&D_80370BF8, &status)) {
        ret = 1;
    } else if (!(status & 1)) {
        ret = 1;
    }
    if (ret == 0) {
        ret = osPfsInitPak(&D_80370BF8, &D_8039B630, 0);
    }
    func_8028A370();
    return ret;
}

s32 func_801F6160(u8 player) {
    return osPfsAllocateFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014, PFS_FILE_SIZE,
                             &D_8039B698[player]);
}

s32 func_801F61C8(s32 arg0) {
    return osPfsDeleteFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014);
}

/* delete the file behind menu item `item` (file list starts at item 37) */
s32 func_801F6210(u8 item) {
    s32 ret;

    ret = osPfsDeleteFile(&D_8039B630, D_80218B20[item - 37].company_code, D_80218B20[item - 37].game_code,
                          D_80218B20[item - 37].game_name, D_80218B20[item - 37].ext_name);
    return ret;
}

/* read (rw 1) or write (rw 0) player record `player` */
s32 func_801F6264(u8 player, u8 rw) {
    s32 ret;
    s32 pad38;
    u32 i;
    s32 pad30;
    s32 tries;
    u8 *p;
    u64 sem;

    ret = 0;
    p = (u8 *) &D_80364AF0[player];
    PORT_SAVE_BEGIN(p, PORT_SAVE_PLAYER, 0x100);
    for (i = 0; i < 0x100 && rw == 1; i++) {
        func_8029A7E4("0x%x, ", p[i]);
    }
    func_8029A7E4("\n");
    PFS_ASSERT(sizeof(playerInfo)<=512, 0);
    if (rw == 1) {
        func_801F75A4(p, 0x100);
    }
    if (D_802E8BF8 != 0 || D_80364A90 == 0x40000000000000) {
        if (D_8039C4B4 == 0 || D_802FA264 != 0) {
            for (i = 0; i < 0x100; i++) {
                if (rw == 1) {
                    D_8039B6B0[i] = p[i];
                    D_8020BEE0[i] = p[i];
                } else {
                    D_8039B6B0[i] = D_8020BEE0[i];
                    p[i] = D_8039B6B0[i];
                }
            }
        } else if (rw == 1) {
            osEepromLongWrite(&D_80370BF8, 0, p, 0x100);
        } else {
            osEepromLongRead(&D_80370BF8, 0, p, 0x100);
        }
    } else {
        tries = 0;
        do {
            ret = osPfsFindFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014, &D_8039B698[player]);
            tries++;
        } while (ret != 0 && tries < 3);
        if (ret == 0) {
            tries = 0;
            do {
                ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], rw, 0, 0x100, p);
                tries++;
            } while (ret != 0 && tries < 3);
        }
    }
    if (ret == 0 && rw == 0) {
        ret = func_801F76E4(p, 0x100);
    }
    PORT_SAVE_END(p, PORT_SAVE_PLAYER, 0x100);
    if (ret == 0 && rw == 0) {
        func_801F6BD0(player, &sem);
        if (sem != 0x87569AB6CD076AEC) {
            ret = 0x6E382;
        }
    }
    return ret;
}

/* read/write best-time block `slot` of player `player` */
s32 func_801F65C4(u8 player, u8 slot, u8 rw) {
    s32 ret;
    s32 i;
    s32 pad;
    u32 start;
    u8 *p;

    ret = 0;
    p = (u8 *) D_80364EF0[player];
    PORT_SAVE_BEGIN(p, PORT_SAVE_TIMES, 0x20);
    start = slot * 0x20 + 0x100;
    if (rw == 1) {
        func_801F75A4(p, 0x20);
    }
    if (D_802E8BF8 != 0) {
        for (i = start; i < start + 0x20; i++) {
            if (rw == 1) {
                D_8039B6B0[i] = p[i - start];
            } else {
                p[i - start] = D_8039B6B0[i];
            }
        }
    } else {
        ret = osPfsFindFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014, &D_8039B698[player]);
        if (ret == 0) {
            ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], rw, start, 0x20, p);
        }
        for (i = 0; i < 14; i++) {
            func_8029A7E4("%d TIME %d = %d\n", rw, i, D_80364EF0[player][D_802E8C44[i]]);
        }
    }
    if (ret == 0 && rw == 0) {
        ret = func_801F76E4(p, 0x20);
    }
    PORT_SAVE_END(p, PORT_SAVE_TIMES, 0x20);
    return ret;
}

/* EEPROM best times of levels `level` and `level` + 1: write (rw 1) or read back and check (rw 0) */
s32 func_801F67E4(u8 pn, u8 level, u8 rw) {
    s32 ret;
    s32 i;
    u8 idx;
    u16 *p;

    ret = 0;
    idx = level * 2;
    p = &D_80364F70[idx & ~3];
    if (D_802E8BF8 != 0) {
        PFS_ASSERT(pn==playerNumberAtStart, 604);
        if (rw == 1) {
            D_80364F70[idx] = D_80364EF0[pn][D_802E8C44[D_80364AF0[pn].timeSlot[level]]];
            D_80364F70[idx + 1] = D_80364F70[idx] ^ 0x55AA;
            func_8029A7E4("%d %d EEWRITE %x %x\n", level, D_80364F70[idx], (u32) (idx * 2 + 0x100) >> 3, p);
            PORT_SAVE_BEGIN(p, PORT_SAVE_EETIMES, 8);
            osEepromWrite(&D_80370BF8, (u32) (idx * 2 + 0x100) >> 3, (u8 *) p);
            PORT_SAVE_END(p, PORT_SAVE_EETIMES, 8);
        } else {
            PORT_SAVE_BEGIN(p, PORT_SAVE_EETIMES, 8);
            osEepromRead(&D_80370BF8, (u32) (idx * 2 + 0x100) >> 3, (u8 *) p);
            PORT_SAVE_END(p, PORT_SAVE_EETIMES, 8);
            for (i = 0; i < 2; i++, level++) {
                if (((D_80364AF0[pn].rank[level] > 0 && D_80364AF0[pn].rank[level] < 6) ? 1 : 0)
                    && level != 0x31 && level != 0x2F && level != 0x26) {
                    D_80364EF0[pn][D_802E8C44[D_80364AF0[pn].timeSlot[level]]] = D_80364F70[level * 2];
                    func_8029A7E4("%d EETIMES: %d %d\n", level, D_80364F70[level * 2], D_80364F70[level * 2 + 1] ^ 0x55AA);
                    if (D_80364F70[level * 2] != (D_80364F70[level * 2 + 1] ^ 0x55AA)) {
                        ret = 0x6E382;
                    }
                }
            }
        }
    }
    return ret;
}

s32 func_801F6AF4(u8 player, u64 sem) {
    s32 ret;
    u8 *p;

    ret = 0;
    p = (u8 *) &D_80364AF0[player];
    PORT_SAVE_BE64(sem);
    if (D_802E8BF8 != 0 || D_80364A90 == 0x40000000000000) {
        osEepromWrite(&D_80370BF8, 0x3F, (u8 *) &sem);
    } else {
        func_8029A7E4("PUTTING SEMAPHORE %llu\n", sem);
        /* 0x20 bytes from &sem: the semaphore and 24 bytes of stack */
        ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], PFS_WRITE, 0xDE0, 0x20,
                                 PORT_SAVE_SEMBLOCK((u8 *) &sem));
    }
    return ret;
}

s32 func_801F6BD0(u8 player, u64 *sem) {
    s32 ret;
    u8 *p;
    u8 buf[0x20];

    ret = 0;
    p = (u8 *) &D_80364AF0[player];
    if (D_802E8BF8 != 0) {
        osEepromRead(&D_80370BF8, 0x3F, (u8 *) sem);
        PORT_SAVE_BE64(*sem);
    } else {
        ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], PFS_READ, 0xDE0, 0x20, buf);
        *sem = *(u64 *) buf;
        PORT_SAVE_BE64(*sem);
        func_8029A7E4("Getting SEMAPHORE %llu\n", *sem);
    }
    return ret;
}

/* read/write the 0x40-byte block D_8039C4B8 stored after the records of the first `nlevels` levels */
s32 func_801F6CA4(u8 player, u8 nlevels, u8 rw) {
    s32 ret;
    s32 tmp;
    s32 i;
    s32 count;
    s32 start;

    ret = 0;
    count = 0;
    for (i = 0; i < nlevels; i++) {
        if (D_802E8F94[i].type == 1 && i != 0x31 && i != 0x2F && i != 0x26) {
            count++;
        }
    }
    start = count * 0x40 + 0x880;
    PORT_SAVE_SWAP(D_8039C4B8, PORT_SAVE_STATUS, 0x40);
    if (rw == 1) {
        func_801F75A4(D_8039C4B8, 0x40);
    }
    if (D_802E8BF8 != 0) {
        for (i = start; i < start + 0x40; i++) {
            if (rw == 1) {
                D_8039B6B0[i] = D_8039C4B8[i - start];
            } else {
                D_8039C4B8[i - start] = D_8039B6B0[i];
            }
        }
    } else {
        ret = tmp = osPfsFindFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014, &D_8039B698[player]);
        if (tmp == 0) {
            ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], rw, start, 0x40, D_8039C4B8);
        }
    }
    if (ret == 0 && rw == 0) {
        ret = func_801F76E4(D_8039C4B8, 0x40);
    }
    PORT_SAVE_SWAP(D_8039C4B8, PORT_SAVE_STATUS, 0x40);
    return ret;
}

s32 func_801F6ED4(u8 arg0) {
    return osPfsFileState(&D_8039B630, arg0, (OSPfsState *) &D_80218B20[D_80218D28]);
}

/* list the pak's files into menu items 37 + n; returns whether there are any */
s32 func_801F6F18(void) {
    s32 i;
    s32 pad40;
    s32 pad3C;
    OSMesg msg;
    s32 pages;
    s32 flag;
    s32 empty;

    D_80218D28 = 0;
    for (i = 0; i < 16; i++) {
        do {
            osSendMesg(&D_80219EF8, (OSMesg) ((i << 16) | 0x11 | 0x1000000), OS_MESG_BLOCK);
            osRecvMesg(&D_80219F50, &msg, OS_MESG_BLOCK);
            if (msg != 0) {
                i++;
            }
        } while (msg != 0 && i < 16);
        if (i < 16) {
            bcopy(D_80218B20[D_80218D28].game_name, D_802189C0[D_80218D28], 17);
            bcopy(D_80218B20[D_80218D28].ext_name, D_80218AD0[D_80218D28], 5);
            D_802189C0[D_80218D28][16] = 0;
            D_80218AD0[D_80218D28][4] = 0;
            pages = D_80218B20[D_80218D28].file_size / 32 / 8;
            flag = D_80218AD0[D_80218D28][0];
            func_801F7410(D_802189C0[D_80218D28]);
            func_801F7410(D_80218AD0[D_80218D28]);
            sprintf(D_80218740[i], "%16s%c%-4s (%d)", D_802189C0[D_80218D28], flag ? '.' : ' ',
                    D_80218AD0[D_80218D28], pages);
            func_8029A7E4("%s\n", D_80218740[i]);
            if (pages < 99) {
                sprintf(D_80218740[i], "%s ", D_80218740[i]);
            }
            if (pages < 9) {
                sprintf(D_80218740[i], "%s ", D_80218740[i]);
            }
            D_8020C070[37 + D_80218D28].text = D_80218740[i];
            D_80218D28++;
        }
    }
    if (D_80218D28 == 0) {
        sprintf(D_80218740[0], "%s", "PAK EMPTY!");
        D_8020C070[37].text = D_80218740[0];
        empty = 1;
    } else {
        empty = 0;
    }
    D_802F8BDC[0x210 / 2] = (D_80218D28 + empty + 1) / 2 + 0x24;
    D_802F8BDC[0x208 / 2] = D_80218D28 + empty + 4;
    osSendMesg(&D_80219EF8, (OSMesg) 0x100000E, OS_MESG_BLOCK);
    osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
    sprintf(D_80219F90, "%d PAGES FREE", D_80218EF0 / 32 / 8);
    D_8020C070[35].text = D_80219F90;
    sprintf(D_80219FB0, "%d NEEDED PER PLAYER", 14);
    D_8020C070[36].text = D_80219FB0;
    D_8020C070[10].text = "DELETE THIS FILE?";
    D_8020C070[10].unk10 = D_80301080;
    return D_80218D28 != 0;
}

s32 func_801F73FC(void) {
    return D_80218D28 != 0;
}

/* character indices -> ASCII */
void func_801F7410(u8 *str) {
    s32 i;

    for (i = 0; i < func_8025B300(str); i++) {
        if (str[i] < 0x45) {
            str[i] = D_8020C01C[str[i]];
        } else {
            str[i] = '-';
        }
    }
}

/* ASCII -> character indices (0xF if not in the set) */
void func_801F74B0(u8 *str) {
    s32 i;
    s32 j;

    for (i = 0; i < func_8025B300(str); i++) {
        for (j = 0; j < 0x45 && D_8020C01C[j] != str[i]; j++) {
        }
        if (j != 0x45) {
            str[i] = j;
        } else {
            str[i] = 0xF;
        }
    }
}

/* store a 4-byte CRC (one byte per 0x20-byte lane) at the end of the block */
s32 func_801F75A4(u8 *data, s32 size) {
    s32 i;
    s32 j;
    s32 nblocks;
    u8 crc[4];
    s32 off;

    nblocks = (size + 0x7F) >> 7;
    for (i = 0; i < 4; i++) {
        crc[i] = 0;
        data[size + i - 4] = crc[i];
    }
    for (i = 0; i < nblocks; i++) {
        for (j = 0; j < 4; j++) {
            off = i * 0x80 + j * 0x20;
            if (off < size) {
                crc[j] += __osContDataCrc(data + off);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        data[size + i - 4] = crc[i];
    }
    return 0;
}

/* check the CRC written by func_801F75A4 */
s32 func_801F76E4(u8 *data, s32 size) {
    s32 i;
    s32 j;
    s32 nblocks;
    u8 saved[4];
    u8 crc[4];
    s32 off;

    nblocks = (size + 0x7F) >> 7;
    for (i = 0; i < 4; i++) {
        saved[i] = data[size + i - 4];
        crc[i] = 0;
        data[size + i - 4] = crc[i];
    }
    for (i = 0; i < nblocks; i++) {
        for (j = 0; j < 4; j++) {
            off = i * 0x80 + j * 0x20;
            if (off < size) {
                crc[j] += __osContDataCrc(data + off);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (crc[i] != saved[i]) {
            return 0x6E382;
        }
    }
    return 0;
}
