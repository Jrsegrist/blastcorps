#include "common.h"
#include <ultra64.h>

/*
 * pfsHandler.c: the Controller Pak / EEPROM save thread. Player records
 * (0x100 bytes each) and best times (0x20 bytes each) go to a "NBCE"/"01"
 * Controller Pak file, or to EEPROM, each block followed by a 4-byte CRC.
 */

void func_8029A7E4(const char *fmt, ...); /* debug printf */
u8 func_8028FCD4(void *arg0, u8 *arg1);
u8 func_8028A370(void);
s32 func_8025B300(u8 *);                  /* strlen */
void func_80270E50(void *, void *, OSMesgQueue *, s32, s32);
u8 __osContDataCrc(u8 *data);
s32 osPfsInitPak(OSMesgQueue *, OSPfs *, int);

#define PFS_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "pfsHandler.c", line)
#define PFS_FILE_SIZE 0xE00

extern OSPfs D_8039B630;      /* hd_code .bss: the Controller Pak file system */
extern u8 D_8020C000[];       /* game name */
extern u8 D_8020C014[];       /* extension name */
extern u8 D_8020C01C[];       /* character set (0x45 entries) */
extern OSPfsState D_80218B20[];
extern s32 D_80218D28;
extern OSMesgQueue D_80370BF8; /* hd_code: SI message queue */
extern u8 D_8039C538;
extern s32 D_8039B698[];      /* file_no per player */
extern OSThread D_80218D30;
extern u8 D_80218EF8[];       /* thread stack (0x1000) */
extern OSMesgQueue D_80219EF8;
extern OSMesg D_80219F10[];
extern OSMesgQueue D_80219F30;
extern OSMesg D_80219F48[];
extern OSMesgQueue D_80219F50;
extern OSMesg D_80219F68[];
extern u8 D_80218EE0[];
extern u8 D_80315440[];       /* hd_code scheduler */
extern u8 D_802E8BF8;
extern u64 D_80364A90;        /* game mode */
extern s32 D_8039C4B4;
extern s32 D_802FA264;
extern u8 D_80364AF0[][0x100]; /* player records */
extern u8 D_8039B6B0[];
extern u8 D_8020BEE0[];
extern u16 D_80364EF0[][16];  /* best times */
extern u8 D_802E8C44[];

#define playerInfo (D_80364AF0[0])

void func_801F58E8(void *);
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
    func_80270E50(D_80315440, D_80218EE0, &D_80219F30, 1, 3);
    PFS_ASSERT(filesize<PFS_FILE_SIZE, 104);
    osStartThread(&D_80218D30);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F58E8.s")

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

void func_801F6160(u8 player) {
    osPfsAllocateFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014, PFS_FILE_SIZE, &D_8039B698[player]);
}

void func_801F61C8(s32 arg0) {
    osPfsDeleteFile(&D_8039B630, 0x3031, 0x4E424345, D_8020C000, D_8020C014);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6210.s")

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
    p = D_80364AF0[player];
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
            osEepromLongRead(&D_80370BF8, 0, p, 0x100);
        } else {
            osEepromLongWrite(&D_80370BF8, 0, p, 0x100);
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
    return ret;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F67E4.s")

s32 func_801F6AF4(u8 player, u64 sem) {
    s32 ret;
    u8 *p;

    ret = 0;
    p = D_80364AF0[player];
    if (D_802E8BF8 != 0 || D_80364A90 == 0x40000000000000) {
        osEepromRead(&D_80370BF8, 0x3F, (u8 *) &sem);
    } else {
        func_8029A7E4("PUTTING SEMAPHORE %llu\n", sem);
        ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], PFS_WRITE, 0xDE0, 0x20, (u8 *) &sem);
    }
    return ret;
}

s32 func_801F6BD0(u8 player, u64 *sem) {
    s32 ret;
    u8 *p;
    u8 buf[0x20];

    ret = 0;
    p = D_80364AF0[player];
    if (D_802E8BF8 != 0) {
        osEepromWrite(&D_80370BF8, 0x3F, (u8 *) sem);
    } else {
        ret = osPfsReadWriteFile(&D_8039B630, D_8039B698[player], PFS_READ, 0xDE0, 0x20, buf);
        *sem = *(u64 *) buf;
        func_8029A7E4("Getting SEMAPHORE %llu\n", *sem);
    }
    return ret;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6CA4.s")

void func_801F6ED4(u8 arg0) {
    osPfsFileState(&D_8039B630, arg0, &D_80218B20[D_80218D28]);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_front_end/0E7B0/func_801F6F18.s")

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
