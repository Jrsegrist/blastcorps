#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* audio.c: Rare's copy of the SDK demos' audio manager (audiomgr.c). The
 * thread renders one audio frame per retrace message from the scheduler and
 * streams sample data through a small cache of 0x200-byte DMA buffers. */

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
} AudioInfo;

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
} AMDMABuffer;

typedef struct {
    /* 0x00 */ u8 initialized;
    /* 0x04 */ AMDMABuffer *firstUsed;
    /* 0x08 */ AMDMABuffer *firstFree;
} AMDMAState;

extern AMAudioMgr D_80368070;
extern AMDMAState D_8036A308;
/* .data (0x802F3AF0); the reverb template in func_802676A0 and
 * func_80267F88's firstTime follow in source order */
u32 D_802F3AF0 = 0; /* audFrameCt */
u32 D_802F3AF4 = 0; /* nextDMA */
s32 D_802F3AF8 = 0; /* curAcmdList */
extern s32 D_8036AFA0; /* deepest DMA list search */
extern OSMesgQueue D_8036AE68; /* audDMAMessageQ */
/* the old SDK's OSIoMesg, without piHandle */
typedef struct {
    OSIoMesgHdr hdr;
    void *dramAddr;
    u32 devAddr;
    u32 size;
} OldIoMesg;

extern OldIoMesg D_8036A8C8[]; /* audDMAIOMesgBuf */
extern u32 D_8036A8B8; /* minFrameSize */
extern u32 D_8036A8BC; /* frameSize */
extern u32 D_8036A8C0; /* maxFrameSize */
extern s32 D_8036A8C4; /* cmdLen */
extern u8 D_803682F8[];
extern u8 D_802E6820[];
extern u8 D_802E68F0[];
extern u8 D_8030EB90[];
extern OSMesgQueue D_80315440;
extern s32 D_803156A4;
/* .bss (0x80368050, placed by hd_code_bss.us.v11.ld); the first two words
 * are unreferenced placeholders */
s32 D_80368050;
s32 D_80368054;
u64 D_80368058; /* last audio frame start */
u64 D_80368060; /* audio frame start */
u64 D_80368068; /* audio frame end */
extern u8 D_80367728;
extern u8 D_80367729;
extern u8 D_8036772A;
extern s32 D_8036772C;
extern u8 D_80367730;

void func_802682A4(void);
ALDMAproc func_80268254(AMDMAState **state);
void func_80267A9C(void *arg);
extern s32 D_80000300; /* osTvType */
extern s32 osViClock;
extern OSMesg D_8036AE80[];
extern u64 D_80368308[]; /* audio thread stack */
extern AMDMABuffer D_8036A318[72];
s32 func_80267FE0(s32 addr, s32 len, void *state);
/* __amHandleFrameMsg */
void func_80267CDC(AudioInfo *info, AudioInfo *lastInfo);
void func_80267F88(AudioInfo *info);

#define AUDIO_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "audio.c", line)

#define MAX_RSP_CMDS 0xABE

/* amCreateAudioMgr */
void func_802676A0(ALSynConfig *c, OSPri pri) {
    s32 i;
    f32 fsize;

    c->dmaproc = (ALDMANew) func_80268254;
    if (D_80000300 != 1) {
        osViClock = 0x02E6025C;
    }
    c->outputRate = osAiSetFrequency(22050);
    fsize = (f32) c->outputRate * 2.0f / 60.0f;
    D_8036A8BC = (s32) fsize;
    if (D_8036A8BC < fsize) {
        D_8036A8BC++;
    }
    if (D_8036A8BC & 0xF) {
        D_8036A8BC = (D_8036A8BC & ~0xF) + 0x10;
    }
    D_8036A8B8 = D_8036A8BC - 16;
    D_8036A8C0 = D_8036A8BC + 0x35;
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

/* amStartAudioMgr */
void func_80267A74(void) {
    osStartThread(&D_80368070.thread);
}

/* __amMain */
void func_80267A9C(void *arg) {
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

#define osScGetCmdQ func_80270F74
#define sc D_80315440
#define cmdLen D_8036A8C4

void func_80267CDC(AudioInfo *info, AudioInfo *lastInfo) {
    s16 *audioPtr;
    Acmd *cmdp;
    s32 samplesLeft;
    BcScTask *t;

    samplesLeft = 0;
    func_802682A4();
    audioPtr = (s16 *) osVirtualToPhysical(info->data);
    if (lastInfo) {
        func_802D9B60(lastInfo->data, lastInfo->frameSamples << 2);
    }
    samplesLeft = func_802D9C10() >> 2;
    info->frameSamples = (D_8036A8BC - samplesLeft + 0x35) & ~0xF;
    if (info->frameSamples < D_8036A8B8) {
        info->frameSamples = D_8036A8B8;
    }
    cmdp = func_802D9D68(D_80368070.ACMDList[D_802F3AF8], &D_8036A8C4, audioPtr, info->frameSamples);
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

#undef sc
#undef osScGetCmdQ

/* __amHandleDoneMsg */
void func_80267F88(AudioInfo *info) {
    static s32 D_802F3C04 = 1; /* firstTime */
    u32 samplesLeft;

    samplesLeft = func_802D9C10() >> 2;
    if (samplesLeft == 0 && !D_802F3C04) {
        func_8029A7E4("audio: ai out of samples\n");
        D_802F3C04 = 0;
    }
}

/* __amDma */
s32 func_80267FE0(s32 addr, s32 len, void *state) {
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
    func_802DA2F0(&D_8036A8C8[D_802F3AF4++], OS_MESG_PRI_NORMAL, OS_READ, addr, foundBuffer, 0x200, &D_8036AE68);
    return osVirtualToPhysical(foundBuffer) + delta;
}

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

/* __clearAudioDMA */
void func_802682A4(void) {
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
