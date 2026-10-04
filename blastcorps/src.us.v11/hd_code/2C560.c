#include "common.h"
#include <ultra64.h>

/* sched.c's .bss (0x8036BEF0-0x8036BFD4), defined here in declaration
 * order so IDO lays it out like the original (placed by
 * hd_code_bss.us.v11.ld). Defining them matters for codegen: stores to a
 * variable defined in the same file share one %hi. Unreferenced holes are
 * placeholders. */
u64 D_8036BEF0;   /* time of the last RSP yield */
u64 D_8036BEF8;
u64 D_8036BF00;
s32 D_8036BF08;
s32 D_8036BF0C;
s32 D_8036BF10;
s32 D_8036BF14;
s32 D_8036BF18;
s32 D_8036BF1C;
s32 D_8036BF20;
s32 D_8036BF24;
s32 D_8036BF28;
s32 D_8036BF2C;
u8 D_8036BF30[8];
OSTime D_8036BF38;
s32 D_8036BF40;
s32 D_8036BF44;
u64 D_8036BF48;
s32 D_8036BF50;
u8 D_8036BF54[0x3C];
s32 D_8036BF90;
s32 D_8036BF94;
u8 D_8036BF98[0x20];
s32 D_8036BFB8;
s32 D_8036BFBC;
f32 D_8036BFC0;
u8 D_8036BFC4;
u8 D_8036BFC5;
f32 D_8036BFC8;
f32 D_8036BFCC;
f32 D_8036BFD0;

/* Blast Corps' scheduler: a Rare-modified copy of the SDK's sched.c
 * (GoldenEye's src/sched.c is a later version). The OSSched here lacks the
 * SDK's two leading OSScMsg fields, and clients carry two extra words. */
void func_8029A7E4(const char *fmt, ...);

/* Rare's assert, with the original sched.c line numbers passed explicitly. */
#define SCHED_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "sched.c", line)

#define M_GFXTASK 1
#define M_AUDTASK 2
#define RSP_STATE_SUSPENDED 3

typedef struct BcScTask {
    /* 0x00 */ struct BcScTask *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *framebuffer;
    /* 0x10 */ OSTask list;
    /* 0x50 */ u32 unk50;
    /* 0x54 */ OSMesgQueue *msgQ;
    /* 0x58 */ OSMesg msg;
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
    /* 0x27C */ u8 pad27C[0x284 - 0x27C];
    /* 0x284 */ s32 frameCount;
    /* 0x288 */ OSTime rdpStartTime;
    /* 0x290 */ OSTime gfxStartTime;
} BcSched;

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

extern u64 D_80364A90;
extern s32 D_80358060;
s32 func_80271A84(BcSched *sc, BcScTask *t);

/* __scHandleRDP */
void func_80271904(BcSched *sc) {
    BcScTask *t;
    OSTime now;

    SCHED_ASSERT(sc->curRDPTask, 586);
    t = sc->curRDPTask;
    sc->curRDPTask = NULL;
    t->flags |= 8;
    if (sc->frameCount != D_8036BF14 || (D_80364A90 & 0xC9FD0FE79BFF80B0ULL)) {
        D_8036BF1C = 0;
        osViSwapBuffer(t->framebuffer);
        D_8036BF18 = D_8036BF14;
        D_8036BF14 = sc->frameCount + 1;
        func_802D4550(8);
    } else {
        D_8036BF1C = (s32) t;
    }
    now = osGetTime();
    D_8036BF20 = (now - sc->rdpStartTime) / 7825;
    if (D_80358060 == 3) {
        osViBlack(FALSE);
    }
    func_80271A84(sc, t);
}

s32 func_80271F48(OSMesgQueue *mq, OSMesg msg, s32 flag);

/* __scTaskComplete */
s32 func_80271A84(BcSched *sc, BcScTask *t) {
    s32 rv;
    s32 needs;
    s32 done;
    s32 type;

    needs = t->flags & 3;
    done = (t->flags >> 2) & 3;
    type = t->list.t.type;
    if (!(t->flags & 0x40)) {
        needs &= 1;
        done &= 1;
    }
    if (needs == done) {
        if (type == M_GFXTASK) {
            SCHED_ASSERT(sc->gfxListHead, 636);
            sc->gfxListHead = sc->gfxListHead->next;
            if (sc->gfxListHead == NULL) {
                sc->gfxListTail = (BcScTask *) &sc->gfxListHead;
            }
        }
        if (t->msgQ != NULL) {
            if (!D_8036BF1C || type != M_GFXTASK) {
                if (t->flags & 0x40) {
                    rv = func_80271F48(t->msgQ, t->msg, OS_MESG_NOBLOCK);
                } else {
                    rv = osSendMesg(t->msgQ, t->msg, OS_MESG_NOBLOCK);
                }
                SCHED_ASSERT(rv!=-1, 649);
            }
        }
        D_8036BFBC = 1;
    } else {
        D_8036BFBC = 0;
    }
    return D_8036BFBC;
}

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

extern u8 D_802FA270;
void func_802DAE1C(OSTask *task); /* osSpTaskLoad */

/* Start the next RSP task: audio when availRCP is 0, else graphics. */
void func_80271CE4(BcSched *sc, s32 availRCP) {
    BcScTask *t;
    OSTime now;

    SCHED_ASSERT(!sc->curRSPTask, 696);
    if (availRCP == 0) {
        t = sc->audioListHead;
        SCHED_ASSERT(t, 701);
        if (t == NULL) {
            return;
        }
        sc->audioListHead = sc->audioListHead->next;
        if (sc->audioListHead == NULL) {
            sc->audioListTail = (BcScTask *) &sc->audioListHead;
        }
        D_8036BF48 = osGetTime();
    } else {
        t = sc->gfxListHead;
        if (D_802FA270) {
            sc->gfxStartTime = osGetTime();
            now = osGetTime();
            D_8036BF2C = (now - D_8036BF38) / 7825;
            D_802FA270 = 0;
        }
    }
    t->state = 1;
    func_802DAE1C(&t->list);
    osSpTaskStartGo(&t->list);
    sc->curRSPTask = t;
    if (t->flags & 0x40) {
        sc->curRDPTask = t;
    }
}

/* __scYield */
void func_80271E88(BcSched *sc) {
    SCHED_ASSERT(sc->curRSPTask->list.t.type != M_AUDTASK, 735);
    if (sc->curRSPTask->list.t.type == M_GFXTASK) {
        SCHED_ASSERT(sc->curRSPTask->state != RSP_STATE_SUSPENDED, 739);
        sc->curRSPTask->state = RSP_STATE_SUSPENDED;
        D_8036BEF0 = osGetTime();
        osSpTaskYield();
    }
}


/* osSendMesg wrapper; the timing values are computed but never used
 * (leftover debug code). */
s32 func_80271F48(OSMesgQueue *mq, OSMesg msg, s32 flag) {
    OSTime remaining;
    OSTime now;
    s32 unused;

    remaining = D_8036BF38 + 391250 - osGetTime();
    now = osGetTime();
    unused = 0;
    return osSendMesg(mq, msg, flag);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_80271FD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/2C560/func_802729F0.s")

void func_80272C40(s32 arg0) {
}
