#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026AD30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026AF6C.s")

extern u16 D_8036BB14;

u16 func_8026B10C(void) {
    return D_8036BB14;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026B118.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026B8F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BA7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BBD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026BCE0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026EF70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F004.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F644.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F82C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F8A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026F92C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FA38.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FB50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FBB0.s")

extern u8 *D_8036BED8;

u8 func_8026FE6C(s32 arg0) {
    return *(u8 *) ((u8 *) D_8036BED8 + (arg0 * 0x88) + 6);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FE8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_8026FEC4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802701A8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80270A54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80270AE0.s")

/* Blast Corps' scheduler: a Rare-modified copy of the SDK's sched.c
 * (GoldenEye's src/sched.c is a later version). The OSSched here lacks the
 * SDK's two leading OSScMsg fields, and clients carry two extra words. */
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
    /* 0x264 */ void *audioListHead;
    /* 0x268 */ void *gfxListHead;
    /* 0x26C */ void **audioListTail;
    /* 0x270 */ void **gfxListTail;
} BcSched;

extern s32 D_8036BF10;
extern s32 D_8036BF1C;
extern OSViMode D_80306E70[];
void func_802DA610(s32);
void func_80270F7C(void *arg);

/* osCreateScheduler */
void func_80270D20(BcSched *sc, void *stack, OSPri priority, u8 mode, u8 numFields) {
    sc->audioListTail = &sc->audioListHead;
    sc->gfxListTail = &sc->gfxListHead;
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

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80270F7C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802712B4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802712FC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271358.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802715DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271904.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271A84.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271C24.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271CE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271E88.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271F48.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_80271FD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/26570/func_802729F0.s")

void func_80272C40(s32 arg0) {
}
