#include "common.h"
#include <ultra64.h>

/* Falling debris / bouncing objects: a pool of 20 that are spawned around a
 * point, home in on a target, wander inside a box and play sounds. Also the
 * audio manager (audio.c, from 0x802676A0), a Rare-modified copy of the SDK
 * demos' audiomgr.c. */

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 unk6; /* target x */
    /* 0x08 */ s16 unk8; /* target z */
    /* 0x0A */ s16 unkA; /* box centre x */
    /* 0x0C */ s16 unkC; /* box centre z */
    /* 0x0E */ s16 unkE; /* box half size */
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15; /* state: 0 free, 1/2/3/4 active, 5 homing */
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18; /* heading, 0..0xFFF */
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C; /* speed */
    /* 0x1D */ u8 pad1D[3];
    /* 0x20 */ f32 unk20; /* last distance to target */
} Debris; /* size 0x24 */

extern Debris D_80367D60[20];
extern u32 D_8036B968;
extern s32 D_80368030;
extern s16 D_80368034;
extern s16 D_80368036;
extern s32 D_80368038;
extern s32 D_8036803C;
extern s32 D_80368040;
extern s32 D_80368044;
extern s32 D_80368048;
extern u8 D_8036EA79;
extern u8 D_802E8BD0;
extern s32 D_80367738;
extern s32 D_803643E0;
extern s32 D_803643E8;
extern s16 D_8036443C;
extern s16 D_8036443E;
extern s32 D_803EF2EC;
extern s32 D_803EF2F4;
extern s32 D_803EF308;
extern s32 D_803EF30C;
extern u8 D_803EF32C;
extern u8 D_803EF32D;
extern s32 D_803EF6DC;
extern s32 D_803EF6E4;
extern u16 D_803C30A8[];

s32 func_80260650(s32, s32, s32);
void func_80260AB8(s32, s32, s32);
s32 func_8026A610(s32, s32, s32, s32);
s32 func_8026A8E0(s16, s16);
void func_8026AD30(s32);
s32 func_80265A0C(s32 arg0);
void func_80265B7C(s32 arg0);

/* Clear the debris pool and reset the timers */
void func_80264C20(s32 arg0) {
    s32 i;

    for (i = 0; i < 20; i++) {
        D_80367D60[i].unk15 = 0;
    }
    D_8036B968 = osGetCount();
    D_80368038 = 99999999;
    if (arg0) {
        D_8036EA79 = D_80368040;
    } else {
        D_8036EA79 = 0;
    }
}

/* TODO: needs this file's .rodata split in the yaml (the 100000000.0f
 * constant at 0x80309640). Matches in probe:
void func_80264CB4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, s32 arg5) {
    s32 pad;
    s32 n;
    s32 i;
    u8 found;
    s16 r;

    i = 0;
    n = 0;
    D_8036EA79 += arg5;
    if (arg5 != 0 && D_802E8BD0 == 0) {
        func_8026AD30(0x48);
    }
    if (arg3 > 200) {
        arg3 = 200;
    }
    while (n < arg5 && i < 20) {
        if (n == 2) {
            func_80260650(D_80367738, 0x24, 0);
        }
        found = 0;
        while (i < 20 && !found) {
            if (D_80367D60[i].unk15 == 0) {
                found = 1;
            } else {
                i++;
            }
        }
        if (found) {
            D_80367D60[i].x = arg0;
            D_80367D60[i].z = arg2;
            D_80367D60[i].y = arg1;
            r = arg3 / 5;
            D_80367D60[i].unkA = func_8026A8E0(-r, r) + (arg0 - arg3);
            D_80367D60[i].unkC = func_8026A8E0(-r, r) + (arg2 + arg3);
            D_80367D60[i].unk10 = arg3 * 3 / 2;
            D_80367D60[i].unkE = arg3;
            D_80367D60[i].unk6 = D_80367D60[i].unkA + func_8026A8E0(-arg3, arg3);
            D_80367D60[i].unk8 = D_80367D60[i].unkC + func_8026A8E0(-arg3, arg3);
            D_80367D60[i].unk13 = 0;
            D_80367D60[i].unk14 = 0;
            D_80367D60[i].unk16 = func_8026A8E0(-500, 500);
            D_80367D60[i].unk12 = arg4;
            D_80367D60[i].unk1C = 0;
            D_80367D60[i].unk15 = 5;
            D_80367D60[i].unk18 = 4000;
            D_80367D60[i].unk20 = 100000000.0f;
            D_80367D60[i].unk1B = 0;
        }
        i++, n++;
    }
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80264CB4.s")

void func_80265428(void);
void func_8026513C(void);
void func_80265E48(void);

void func_8026510C(void) {
    func_80265428();
    func_8026513C();
    func_80265E48();
}

/* Move homing debris (state 5) 5 units a frame toward its target; it lands
 * (state 1) once it arrives or starts moving away */
void func_8026513C(void) {
    s16 dx;
    s16 dz;
    s16 adx;
    s16 adz;
    f32 dist;
    s16 sx;
    s16 sz;
    s32 i;

    for (i = 0; i < 20; i++) {
        if (D_80367D60[i].unk15 == 5) {
            dx = D_80367D60[i].unk6 - D_80367D60[i].x;
            if (dx >= 0) {
                adx = dx;
            } else {
                adx = -dx;
            }
            dz = D_80367D60[i].unk8 - D_80367D60[i].z;
            if (dz >= 0) {
                adz = dz;
            } else {
                adz = -dz;
            }
            dist = func_8026A610(D_80367D60[i].unk6, D_80367D60[i].unk8, D_80367D60[i].x, D_80367D60[i].z);
            if (dist < 1.0 || D_80367D60[i].unk20 < dist) {
                D_80367D60[i].unk15 = 1;
            } else {
                D_80367D60[i].unk20 = dist;
                sx = adx / dist * 5.0f;
                sz = adz / dist * 5.0f;
                if (dx >= 0) {
                    D_80367D60[i].x = D_80367D60[i].x + sx;
                } else {
                    D_80367D60[i].x -= sx;
                }
                if (dz >= 0) {
                    D_80367D60[i].z += sz;
                } else {
                    D_80367D60[i].z -= sz;
                }
            }
        }
    }
}

/* Wander: step each active piece along its heading, clamped to its box,
 * preferring the direction away from the player */
void func_80265428(void) {
    s32 i;
    s32 pad;
    s32 x;
    s32 z;
    s32 x2;
    s32 z2;
    s32 d1;
    s32 d2;

    for (i = 0; i < 20; i++) {
        if ((D_80367D60[i].unk15 == 1 || D_80367D60[i].unk15 == 4 || D_80367D60[i].unk15 == 2) && func_80265A0C(i)) {
            func_80265B7C(i);
            x = D_80367D60[i].x + D_80368034;
            z = D_80367D60[i].z + D_80368036;
            x2 = D_80367D60[i].x - D_80368034;
            z2 = D_80367D60[i].z - D_80368036;
            if (x < D_80367D60[i].unkA - D_80367D60[i].unkE) {
                x = D_80367D60[i].unkA - D_80367D60[i].unkE;
            }
            if (z < D_80367D60[i].unkC - D_80367D60[i].unkE) {
                z = D_80367D60[i].unkC - D_80367D60[i].unkE;
            }
            if (x >= D_80367D60[i].unkA + D_80367D60[i].unkE) {
                x = D_80367D60[i].unkA + D_80367D60[i].unkE - 1;
            }
            if (z >= D_80367D60[i].unkC + D_80367D60[i].unkE) {
                z = D_80367D60[i].unkC + D_80367D60[i].unkE - 1;
            }
            if (x2 < D_80367D60[i].unkA - D_80367D60[i].unkE) {
                x2 = D_80367D60[i].unkA - D_80367D60[i].unkE;
            }
            if (z2 < D_80367D60[i].unkC - D_80367D60[i].unkE) {
                z2 = D_80367D60[i].unkC - D_80367D60[i].unkE;
            }
            if (x2 >= D_80367D60[i].unkA + D_80367D60[i].unkE) {
                x2 = D_80367D60[i].unkA + D_80367D60[i].unkE - 1;
            }
            if (z2 >= D_80367D60[i].unkC + D_80367D60[i].unkE) {
                z2 = D_80367D60[i].unkC + D_80367D60[i].unkE - 1;
            }
            d1 = func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, x, z);
            d2 = func_8026A610(D_803643E0 >> 5, D_803643E8 >> 5, x2, z2);
            if (D_80367D60[i].unk1B) {
                if (D_80367D60[i].unk1A) {
                    d2 = 0;
                } else {
                    d1 = 0;
                }
                D_80367D60[i].unk1B--;
            }
            if (d1 < d2) {
                D_80367D60[i].x = x2;
                D_80367D60[i].z = z2;
                D_80367D60[i].unk18 += 0x800;
                if (D_80367D60[i].unk18 >= 0x1000) {
                    D_80367D60[i].unk18 -= 0xFFF;
                }
                if (D_80367D60[i].unk1A) {
                    D_80367D60[i].unk1B = 5;
                }
                D_80367D60[i].unk1A = 0;
            } else {
                D_80367D60[i].x = x;
                D_80367D60[i].z = z;
                if (!D_80367D60[i].unk1A) {
                    D_80367D60[i].unk1B = 5;
                }
                D_80367D60[i].unk1A = 1;
            }
            if (i == 1 && D_80367D60[i].unk15 != 4) {
                func_80260650(D_80367738, 0x24, 0);
            }
            if (D_80367D60[i].unk15 == 2) {
                D_803EF32C = 6;
            }
            D_80367D60[i].unk15 = 4;
        } else if (D_80367D60[i].unk15 == 4) {
            D_80367D60[i].unk15 = 1;
        }
    }
}

/* Is the player (D_803643E0/E8, >> 5) inside piece arg0's area? */
s32 func_80265A0C(s32 arg0) {
    s16 b3;
    s16 b2;
    s16 b1;
    s16 b0;

    b1 = D_80367D60[arg0].unkA - D_80367D60[arg0].unk10;
    b0 = D_80367D60[arg0].unkC - D_80367D60[arg0].unk10;
    b3 = D_803643E0 >> 5, b2 = D_803643E8 >> 5;
    if (b3 < b1 || b2 < b0) {
        return 0;
    }
    b1 = D_80367D60[arg0].unkA + D_80367D60[arg0].unk10;
    b0 = D_80367D60[arg0].unkC + D_80367D60[arg0].unk10;
    if (b3 >= b1 || b2 >= b0) {
        return 0;
    }
    return 1;
}

/* Turn piece arg0 and work out this frame's step (D_80368034/36) from its
 * heading and a speed that ramps toward D_8036443C / 40 */
void func_80265B7C(s32 arg0) {
    s16 ang;
    s16 t;
    s16 s;
    s16 c;
    s16 vx;
    s16 vz;
    s32 lim;

    lim = D_8036443C / 40;
    if (lim < 0) {
        lim = -lim;
    }
    if (D_8036443C != 0 && lim == 0) {
        lim = 2;
    }
    ang = D_80367D60[arg0].unk16 + D_8036443E + 0x400;
    if (ang >= 0x1000) {
        ang -= 0xFFF;
    }
    D_80367D60[arg0].unk18 = ang;
    t = ang % 0x400;
    s = sins(t * 0xFFFF / 0xFFF);
    c = coss(t * 0xFFFF / 0xFFF);
    if (D_80367D60[arg0].unk1C < lim) {
        D_80367D60[arg0].unk1C++;
    }
    if (D_80367D60[arg0].unk1C > lim) {
        D_80367D60[arg0].unk1C--;
    }
    vx = D_80367D60[arg0].unk1C * s >> 15;
    vz = D_80367D60[arg0].unk1C * c >> 15;
    if (ang < 0x400) {
        D_80368034 = vx;
        D_80368036 = vz;
    }
    if (ang >= 0x400 && ang < 0x800) {
        D_80368034 = vz;
        D_80368036 = -vx;
    }
    if (ang >= 0x800 && ang < 0xC00) {
        D_80368034 = -vx;
        D_80368036 = -vz;
    }
    if (ang >= 0xC00) {
        D_80368034 = -vz;
        D_80368036 = vx;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80265E48.s")

/* Point the camera target at the player */
void func_802661EC(void) {
    D_803EF308 = D_803EF6DC;
    D_803EF30C = D_803EF6E4;
    D_80368044 = D_803643E0;
    D_80368048 = D_803643E8;
    D_803EF32C = 1;
    D_80368038 = 2000;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80266248.s")

typedef struct {
    u8 pad[0x12];
    u8 cell;
} Node12;

/* Is the node's cell in the 0xFFFF-terminated list D_803C30A8? (same as
 * 26570's func_80270A54 with the cell at 0x12) */
s32 func_80267614(Node12 *node, s32 arg1) {
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

/* ---------------------------------------------------------------------------
 * audio.c (0x802676A0-0x802683F0): Rare's copy of the SDK demos' audiomgr.c.
 * Its strings ("No samples left\n" ... "Dma not done\n", 0x80309700-0x80309825)
 * show it's a separate source file from the code above, starting at the
 * 16-aligned 0x802676A0 (ROM 0x22EE0).
 *
 * Every function below except func_80267A74/func_80268254 matches in a probe
 * but can't be committed until the yaml gives these files their own sections
 * (I wasn't permitted to edit the shared yaml/Makefile):
 *  - .rodata: 0x80309640-0x80309700 for the code above (late rodata only:
 *    func_80264CB4's float, func_80266248's jump tables and doubles), and
 *    0x80309700-0x80309830 for audio.c's strings; the two files need a c split
 *    at ROM 0x22EE0 so the strings don't land before the late rodata.
 *  - .data 0x802F3AF0-0x802F3C10 (audFrameCt, nextDMA, curAcmdList, the
 *    custom-reverb template that func_802676A0 copies, firstTime).
 *  - .bss: func_80267A9C's u64 timers D_80368058/60/68 are defined in this
 *    file (paired stores share one lui).
 * The probe sources are in the comments.
 * ------------------------------------------------------------------------ */

typedef struct BcScTask {
    /* 0x00 */ struct BcScTask *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *framebuffer;
    /* 0x10 */ OSTask list;
    /* 0x50 */ void *unk50;
    /* 0x54 */ OSMesgQueue *msgQ;
    /* 0x58 */ OSMesg msg;
    /* 0x5C */ s32 pad5C;
} BcScTask;

typedef struct {
    /* 0x00 */ s16 *data;
    /* 0x04 */ s16 frameSamples;
    /* 0x08 */ BcScTask task;
} AudioInfo; /* 0x68 */

typedef struct {
    /* 0x000 */ Acmd *ACMDList[2];
    /* 0x008 */ AudioInfo *audioInfo[3];
    /* 0x018 */ OSThread thread;
    /* 0x1C8 */ OSMesgQueue audioFrameMsgQ;
    /* 0x1E0 */ OSMesg audioFrameMsgBuf[8];
    /* 0x200 */ OSMesgQueue audioReplyMsgQ;
    /* 0x218 */ OSMesg audioReplyMsgBuf[8];
    /* 0x238 */ ALGlobals g;
} AMAudioMgr;

typedef struct {
    /* 0x00 */ ALLink node;
    /* 0x08 */ u32 startAddr;
    /* 0x0C */ u32 lastFrame;
    /* 0x10 */ u8 *ptr;
} AMDMABuffer; /* 0x14 */

typedef struct {
    /* 0x00 */ u8 initialized;
    /* 0x04 */ AMDMABuffer *firstUsed;
    /* 0x08 */ AMDMABuffer *firstFree;
} AMDMAState;

/* The old SDK's OSIoMesg, without piHandle */
typedef struct {
    OSIoMesgHdr hdr;
    void *dramAddr;
    u32 devAddr;
    u32 size;
} OldIoMesg; /* 0x14 */

extern AMAudioMgr D_80368070;  /* __am */
extern AMDMAState D_8036A308;  /* dmaState */
extern AMDMABuffer D_8036A318[72]; /* dmaBuffs */

s32 func_80267FE0(s32 addr, s32 len, void *state);

/* TODO: needs .data for the reverb template (see above). Matches in probe:
void func_802676A0(ALSynConfig *c, OSPri pri) {
    s32 i;
    f32 fsize;

    c->dmaproc = func_80268254;
    if (D_80000300 != 1) {            // osTvType
        osViClock = 0x02E6025C;
    }
    c->outputRate = osAiSetFrequency(22050);
    fsize = (f32) c->outputRate * 2.0f / 60.0f;
    D_8036A8BC = (s32) fsize;         // u32 frameSize
    if (D_8036A8BC < fsize) {
        D_8036A8BC++;
    }
    if (D_8036A8BC & 0xF) {
        D_8036A8BC = (D_8036A8BC & ~0xF) + 0x10;
    }
    D_8036A8B8 = D_8036A8BC - 16;     // minFrameSize
    D_8036A8C0 = D_8036A8BC + 0x35;   // maxFrameSize
    if (c->fxType == 6) {
        s32 pad = 0;
        s32 params[66] = {
            8, 6800,
            0, 160, 9830, -9830, 0, 0, 0, 0,
            160, 320, 9830, -9830, 11140, 0, 0, 9472,
            800, 2560, 16384, -16384, 4587, 0, 0, 12288,
            960, 1920, 8192, -8192, 0, 0, 0, 0,
            3200, 5600, 16384, -16384, 4587, 0, 0, 13568,
            3360, 4800, 8192, -8192, 0, 0, 0, 0,
            4800, 5440, 8192, -8192, 0, 0, 0, 0,
            0, 5920, 13000, -13000, 0, 380, 10, 17664,
        };
        c->params = params;
        alInit(&D_80368070.g, c);
    } else {
        alInit(&D_80368070.g, c);
    }
    D_8036A318[0].node.prev = NULL;
    D_8036A318[0].node.next = NULL;
    for (i = 0; i < 71; i++) {
        alLink(&D_8036A318[i + 1].node, &D_8036A318[i].node);
        D_8036A318[i].ptr = alHeapAlloc(c->heap, 1, 0x200);
    }
    D_8036A318[i].ptr = alHeapAlloc(c->heap, 1, 0x200);
    for (i = 0; i < 2; i++) {
        D_80368070.ACMDList[i] = alHeapAlloc(c->heap, 1, 0x55F0);
    }
    for (i = 0; i < 3; i++) {
        D_80368070.audioInfo[i] = alHeapAlloc(c->heap, 1, sizeof(AudioInfo));
        D_80368070.audioInfo[i]->data = alHeapAlloc(c->heap, 1, D_8036A8C0 * 4);
    }
    osCreateMesgQueue(&D_80368070.audioReplyMsgQ, D_80368070.audioReplyMsgBuf, 8);
    osCreateMesgQueue(&D_80368070.audioFrameMsgQ, D_80368070.audioFrameMsgBuf, 8);
    osCreateMesgQueue(&D_8036AE68, D_8036AE80, 0x48);
    osCreateThread(&D_80368070.thread, 4, func_80267A9C, NULL, &D_80368308[0x400], pri);
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_802676A0.s")

/* amStartAudioMgr */
void func_80267A74(void) {
    osStartThread(&D_80368070.thread);
}

/* TODO: needs the strings and the u64 timers defined in .bss (see above).
 * Matches in probe:
void func_80267A9C(void *arg) {       // __amMain
    s32 done;
    s32 msg;
    AudioInfo *lastInfo;
    s32 firstTime;

    done = 0;
    lastInfo = NULL;
    firstTime = 1;
    func_80270E50(&D_80315440, D_803682F8, &D_80368070.audioFrameMsgQ, 2, 2);
    osSendMesg(&D_80368070.audioFrameMsgQ, (OSMesg) 5, OS_MESG_NOBLOCK);
    while (!done) {
        osRecvMesg(&D_80368070.audioFrameMsgQ, (OSMesg *) &msg, OS_MESG_BLOCK);
        switch (msg) {
            case 4:
                break;
            case 5:
                if (D_803156A4) {
                    osSendMesg(&D_80315440, (OSMesg) 0x29E, OS_MESG_BLOCK);
                }
                D_80368060 = osGetTime();
                func_80267CDC(D_80368070.audioInfo[D_802F3AF0 % 3], lastInfo);
                D_80368068 = osGetTime();
                D_80368058 = D_80368060;
                if (!firstTime) {
                    osRecvMesg(&D_80368070.audioReplyMsgQ, (OSMesg *) &lastInfo, OS_MESG_BLOCK);
                    func_80267F88(lastInfo);
                }
                firstTime = 0;
                if (D_8036772A) {
                    func_802613C8();
                }
                if (D_80367728) {
                    func_80261068();
                }
                if (D_80367729) {
                    func_80261284();
                }
                if (!D_80367730) {
                    func_802611F0();
                }
                if (D_8036772C) {
                    func_80261528();
                }
                break;
            case 10:
                done = 1;
                break;
            case 6:
                func_8029A7E4("No samples left\n");
                break;
        }
    }
    alClose(&D_80368070.g);
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267A9C.s")

/* TODO: needs the strings. Matches in probe, with
 *   #define AUDIO_ASSERT(EX, line) if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "audio.c", line)
 *   #define MAX_RSP_CMDS 0xABE
 *   #define osScGetCmdQ func_80270F74
 *   #define sc D_80315440
 *   #define cmdLen D_8036A8C4
void func_80267CDC(AudioInfo *info, AudioInfo *lastInfo) {   // __amHandleFrameMsg
    s16 *audioPtr;
    Acmd *cmdp;
    s32 samplesLeft;
    BcScTask *t;

    samplesLeft = 0;
    func_802682A4();
    audioPtr = (s16 *) osVirtualToPhysical(info->data);
    if (lastInfo) {
        func_802D9B60(lastInfo->data, lastInfo->frameSamples << 2);  // osAiSetNextBuffer
    }
    samplesLeft = func_802D9C10() >> 2;                              // osAiGetLength
    info->frameSamples = (D_8036A8BC - samplesLeft + 0x35) & ~0xF;
    if (info->frameSamples < D_8036A8B8) {
        info->frameSamples = D_8036A8B8;
    }
    cmdp = func_802D9D68(D_80368070.ACMDList[D_802F3AF8], &D_8036A8C4, audioPtr, info->frameSamples); // alAudioFrame
    AUDIO_ASSERT(cmdLen <= MAX_RSP_CMDS, 336);
    t = &info->task;
    t->next = NULL;
    t->msgQ = &D_80368070.audioReplyMsgQ;
    t->msg = (OSMesg) info;
    t->flags = 1;
    t->unk50 = D_803682F8;
    t->list.t.data_ptr = (u64 *) D_80368070.ACMDList[D_802F3AF8];
    t->list.t.data_size = (cmdp - D_80368070.ACMDList[D_802F3AF8]) * sizeof(Acmd);
    t->list.t.type = M_AUDTASK;
    t->list.t.ucode_boot = (u64 *) D_802E6820;
    t->list.t.ucode_boot_size = D_802E68F0 - D_802E6820;
    t->list.t.flags = 0;
    t->list.t.ucode = (u64 *) D_802E68F0;
    t->list.t.ucode_data = (u64 *) D_8030EB90;
    t->list.t.ucode_data_size = 0x800;
    t->list.t.yield_data_ptr = NULL;
    t->list.t.yield_data_size = 0;
    osWritebackDCache(t, sizeof(BcScTask));
    osWritebackDCache(t->list.t.data_ptr, t->list.t.data_size);
    AUDIO_ASSERT(osSendMesg(osScGetCmdQ(&sc), (OSMesg) t, OS_MESG_NOBLOCK)!=-1, 361);
    D_802F3AF8 ^= 1;
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267CDC.s")

/* TODO: needs the string. Matches in probe:
void func_80267F88(AudioInfo *info) {   // __amHandleDoneMsg
    u32 samplesLeft;

    samplesLeft = func_802D9C10() >> 2;
    if (samplesLeft == 0 && !D_802F3C04) {   // firstTime (.data, = 1)
        func_8029A7E4("audio: ai out of samples\n");
        D_802F3C04 = 0;
    }
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267F88.s")

/* TODO: needs the string. Matches in probe:
s32 func_80267FE0(s32 addr, s32 len, void *state) {   // __amDma
    void *foundBuffer;
    s32 delta;
    s32 addrEnd;
    s32 buffEnd;
    AMDMABuffer *dmaPtr;
    AMDMABuffer *lastDmaPtr;
    AMDMABuffer *prev;
    s32 count;

    dmaPtr = D_8036A308.firstUsed;
    count = 0;
    lastDmaPtr = NULL;
    addrEnd = addr + len;
    delta = addr & 1;
    while (dmaPtr) {
        prev = dmaPtr;
        buffEnd = dmaPtr->startAddr + 0x200;
        if (dmaPtr->startAddr > (u32) addr) {
            break;
        } else if (addrEnd <= buffEnd) {
            dmaPtr->lastFrame = D_802F3AF0;
            foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return osVirtualToPhysical(foundBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (AMDMABuffer *) dmaPtr->node.next;
        count++;
    }
    if (count > D_8036AFA0) {
        D_8036AFA0 = count;
    }
    dmaPtr = D_8036A308.firstFree;
    if (!dmaPtr) {
        func_8029A7E4("OH DEAR - No audio DMA buffers left\n");
    }
    if (!dmaPtr) {
        return osVirtualToPhysical(D_8036A308.firstUsed);
    }
    D_8036A308.firstFree = (AMDMABuffer *) dmaPtr->node.next;
    alUnlink((ALLink *) dmaPtr);
    if (lastDmaPtr) {
        alLink((ALLink *) dmaPtr, (ALLink *) lastDmaPtr);
    } else if (D_8036A308.firstUsed) {
        lastDmaPtr = D_8036A308.firstUsed;
        D_8036A308.firstUsed = dmaPtr;
        dmaPtr->node.next = (ALLink *) lastDmaPtr;
        dmaPtr->node.prev = NULL;
        lastDmaPtr->node.prev = (ALLink *) dmaPtr;
    } else {
        D_8036A308.firstUsed = dmaPtr;
        dmaPtr->node.next = NULL;
        dmaPtr->node.prev = NULL;
    }
    foundBuffer = dmaPtr->ptr;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = D_802F3AF0;
    func_802DA2F0(&D_8036A8C8[D_802F3AF4++], OS_MESG_PRI_NORMAL, OS_READ, addr, foundBuffer, 0x200, &D_8036AE68); // osPiStartDma
    return osVirtualToPhysical(foundBuffer) + delta;
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_80267FE0.s")

/* __amDmaNew */
ALDMAproc func_80268254(AMDMAState **state) {
    s32 pad;

    if (!D_8036A308.initialized) {
        D_8036A308.firstUsed = NULL;
        D_8036A308.firstFree = &D_8036A318[0];
        D_8036A308.initialized = 1;
    }
    *state = &D_8036A308;
    return (ALDMAproc) func_80267FE0;
}

/* TODO: needs the string. Matches in probe:
void func_802682A4(void) {   // __clearAudioDMA
    u32 i;
    OSIoMesg *iomsg;
    AMDMABuffer *dmaPtr;
    AMDMABuffer *nextPtr;

    for (i = 0; i < D_802F3AF4; i++) {
        if (osRecvMesg(&D_8036AE68, (OSMesg *) &iomsg, OS_MESG_NOBLOCK) == -1) {
            func_8029A7E4("Dma not done\n");
        }
    }
    dmaPtr = D_8036A308.firstUsed;
    while (dmaPtr) {
        nextPtr = (AMDMABuffer *) dmaPtr->node.next;
        if (dmaPtr->lastFrame + 1 < D_802F3AF0) {
            if (D_8036A308.firstUsed == dmaPtr) {
                D_8036A308.firstUsed = (AMDMABuffer *) dmaPtr->node.next;
            }
            alUnlink((ALLink *) dmaPtr);
            if (D_8036A308.firstFree) {
                alLink((ALLink *) dmaPtr, (ALLink *) D_8036A308.firstFree);
            } else {
                D_8036A308.firstFree = dmaPtr;
                dmaPtr->node.next = NULL;
                dmaPtr->node.prev = NULL;
            }
        }
        dmaPtr = nextPtr;
    }
    D_802F3AF4 = 0;
    D_802F3AF0++;
}
*/
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/20460/func_802682A4.s")
