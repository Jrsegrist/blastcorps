#include "common.h"
#include <ultra64.h>

/* Blast Corps' scheduler: a Rare-modified copy of the SDK's sched.c
 * (GoldenEye's src/sched.c is a later version). The OSSched here lacks the
 * SDK's two leading OSScMsg fields, and clients carry two extra words. */
void func_8029A7E4(const char *fmt, ...);

/* Rare's assert, with the original sched.c line numbers passed explicitly. */
#define SCHED_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "sched.c", line)

#define M_GFXTASK 1
#define M_AUDTASK 2

typedef struct BcScTask {
    /* 0x00 */ struct BcScTask *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *framebuffer;
    /* 0x10 */ OSTask list;
} BcScTask;

typedef struct BcScClient {
    /* 0x00 */ struct BcScClient *next;
    /* 0x04 */ OSMesgQueue *msgQ;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
} BcScClient;

typedef struct {
    /* 0x000 */ OSMesgQueue interruptQ;
    /* 0x018 */ OSMesg intBuf[16];
    /* 0x058 */ OSMesgQueue cmdQ;
    /* 0x070 */ OSMesg cmdMsgBuf[16];
    /* 0x0B0 */ OSThread thread;
    /* 0x260 */ BcScClient *clientList;
    /* 0x264 */ BcScTask *audioListHead;
    /* 0x268 */ BcScTask *gfxListHead;
    /* 0x26C */ BcScTask *audioListTail; /* starts as &audioListHead: tail->next is head */
    /* 0x270 */ BcScTask *gfxListTail;
    /* 0x274 */ BcScTask *curRSPTask;
    /* 0x278 */ BcScTask *curRDPTask;
} BcSched;

extern s32 D_8036BF10;
extern s32 D_8036BF1C;
extern OSViMode D_80306E70[];
void func_802DA610(s32);
void func_80270F7C(void *arg);

/* osCreateScheduler */
void func_80270D20(BcSched *sc, void *stack, OSPri priority, u8 mode, u8 numFields) {
    sc->audioListTail = (BcScTask *) &sc->audioListHead;
    sc->gfxListTail = (BcScTask *) &sc->gfxListHead;
    D_8036BF10 = 0;
    D_8036BF1C = 0;
    osCreateMesgQueue(&sc->interruptQ, sc->intBuf, 16);
    osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, 16);
    func_802DA610(0xFE);
    osViSetMode(&D_80306E70[mode]);
    osViBlack(TRUE);
    osSetEventMesg(OS_EVENT_SP, &sc->interruptQ, (OSMesg) 0x29B);
    osSetEventMesg(OS_EVENT_DP, &sc->interruptQ, (OSMesg) 0x29C);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->interruptQ, (OSMesg) 0x29D);
    osSetEventMesg(OS_EVENT_FAULT, &sc->interruptQ, (OSMesg) 0x2A0);
    osViSetEvent(&sc->interruptQ, (OSMesg) 0x29A, numFields);
    osCreateThread(&sc->thread, 5, func_80270F7C, sc, stack, priority);
    osStartThread(&sc->thread);
}

/* osScAddClient, with two extra client fields. */
void func_80270E50(BcSched *sc, BcScClient *c, OSMesgQueue *msgQ, s32 arg3, s32 arg4) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    c->msgQ = msgQ;
    c->next = sc->clientList;
    sc->clientList = c;
    c->unk8 = arg3;
    c->unkC = arg4;
    osSetIntMask(mask);
}

/* osScRemoveClient */
void func_80270ECC(BcSched *sc, BcScClient *c) {
    BcScClient *client = sc->clientList;
    BcScClient *prev = NULL;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    while (client != NULL) {
        if (client == c) {
            if (prev) {
                prev->next = c->next;
            } else {
                sc->clientList = c->next;
            }
            break;
        }
        prev = client;
        client = client->next;
    }
    osSetIntMask(mask);
}

void *func_80270F74(void *arg0) {
    return (u8 *) arg0 + 0x58;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80270F7C.s")

void func_80271C24(BcSched *sc, BcScTask *t);
void func_80271CE4(BcSched *sc, s32 availRCP);
void func_80271E88(BcSched *sc);

/* Queue a task, then run the scheduler if the RSP is idle. */
void func_802712B4(BcSched *sc, BcScTask *t) {
    func_80271C24(sc, t);
    if (sc->curRSPTask == NULL) {
        func_80271CE4(sc, 1);
    }
}

u64 D_8036BF00; /* defined here: same-file definitions get shared-%hi stores */

/* Retrace-side scheduling: yield the running RSP task, or reset and
 * schedule when idle. */
void func_802712FC(BcSched *sc) {
    if (sc->curRSPTask != NULL) {
        func_80271E88(sc);
    } else {
        D_8036BF00 = 0;
        func_80271CE4(sc, 0);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271358.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_802715DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271904.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271A84.s")

/* __scAppendList */
void func_80271C24(BcSched *sc, BcScTask *t) {
    long type = t->list.t.type;

    SCHED_ASSERT((type == M_AUDTASK) || (type == M_GFXTASK), 668);
    if (type == M_AUDTASK) {
        sc->audioListTail->next = t;
        sc->audioListTail = t;
    } else {
        sc->gfxListTail->next = t;
        sc->gfxListTail = t;
    }
    t->next = NULL;
    t->state = 2;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271CE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271E88.s")

extern OSTime D_8036BF38;

/* osSendMesg wrapper; the timing values are computed but never used
 * (leftover debug code). */
void func_80271F48(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    OSTime remaining;
    OSTime now;
    s32 unused;

    remaining = D_8036BF38 + 391250 - osGetTime();
    now = osGetTime();
    unused = 0;
    osSendMesg(mq, msg, flag);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271FD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_802729F0.s")

void func_80272C40(s32 arg0) {
}
