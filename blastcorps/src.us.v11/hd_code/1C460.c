#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* Music player: tune selection, push/pop tune stack, volume fades and audio init */

/* saved-tune stack entry (0x1F4 bytes) */
typedef struct {
    u32 chanState[64];
    ALCSeqMarker marker;
    s32 tempo;
    u8 tune;
} TuneState;

/* Blast Corps' SDK ALCSPlayer has one more word before state than 2.0I's */
typedef struct {
    u8 pad0[0x2C];
    s32 state;
    u8 pad30[0x30];
    u32 *chanState;
} BCSeqPlayer;

typedef struct {
    s32 maxVVoices;
    s32 maxPVoices;
    s32 maxUpdates;
    s32 maxFXbusses;
    void *dmaproc;
    ALHeap *heap;
    s32 outputRate;
    u8 fxType;
    s32 *params;
} SynConfig;


extern u8 D_00350950[];
extern u8 D_003539A0[];
extern u8 D_003A1920[];
extern u8 D_003A48C0[];
extern u8 D_0044F5C0[];

extern u8 D_802E8D84;
extern s16 D_802E8D00[];
extern f32 D_802E8D88;
extern s32 D_802E8BDC;
extern u8 D_802E8DC8[];
extern u8 D_802E8E04[];
extern u8 D_802E8E40[];
extern u16 D_802E8E7C[];
extern u8 D_802E8D8C[];
extern u8 D_802E8F94[][0x44];
extern s32 D_802E8EB4[][6];
extern s32 D_803156C4;
extern u64 D_80364A90;
extern TuneState D_80366C30[];
extern TuneState *D_80367400;
extern s32 D_80367408[];
extern void *D_80367510;
extern ALSeqFile *D_80367514;
extern ALCSeq D_80367518[];
extern u8 D_80367708;
extern f32 D_8036770C;
extern f32 D_80367710;
extern f32 D_80367714;
extern ALHeap D_80367718;
extern u8 D_80367728;
extern u8 D_80367729;
extern u8 D_8036772A;
extern s32 D_8036772C;
extern u8 D_80367730;
extern ALCSPlayer *D_80367734;
extern void *D_80367738;
extern void *D_8036773C;
extern s32 D_80367740;
extern u8 D_80370C80[];

#define SEQP ((BCSeqPlayer *) D_80367734)

void func_80260F60(f32 arg0);

void func_80260C20(u8 tune, f32 vol) {
    void *rom;

    D_802E8D84 ^= 1;
    alCSPStop(D_80367734);
    D_8036772A = 0;
    D_8036770C = vol;
    D_80367708 = tune;
    rom = D_80367514->seqArray[tune].offset;
    func_8028B4C4(rom, D_80367510, &D_80367408[tune], 0, 0, 0);
    alCSeqNew(&D_80367518[D_802E8D84], D_80367510);
    alCSPSetSeq(D_80367734, &D_80367518[D_802E8D84]);
    alCSPPlay(D_80367734);
    alCSPSetVol(D_80367734, D_802E8D00[D_80367708] * D_8036770C * D_802E8D88);
}

void func_80260D7C(f32 vol) {
    D_802E8D88 = vol;
    alCSPSetVol(D_80367734, D_802E8D00[D_80367708] * D_8036770C * vol);
}

f32 func_80260DF0(void) {
    return D_802E8D88;
}

void func_80260DFC(void) {
    func_80260EE0(D_802E8DC8[D_802E8BDC]);
}

void func_80260E2C(void) {
    D_80367740 = D_803156C4;
    D_8036772C = alCSPGetTempo(D_80367734);
    func_80261FB0(D_802E8E04[D_802E8BDC]);
}

void func_80260E80(void) {
    D_80367740 = D_803156C4;
    func_80261FB0(D_802E8E40[D_802E8BDC]);
}

void func_80260EC0(void) {
    func_80260DFC();
}

void func_80260EE0(u8 tune) {
    if (D_80367728) {
        func_8029A7E4("OH DEAR - pushing tune but we're still popping!\n");
    } else {
        func_8029A7E4("push tune %d\n", tune);
        D_80367400->tune = D_80367708;
        D_80367708 = tune;
        D_80367730 = 0;
        D_80367729 = 1;
    }
}

void func_80260F60(f32 vol) {
    func_8029A7E4("1 pop tune");
    if (D_80367400 == D_80366C30) {
        return;
    }
    D_80367400--;
    if (D_80367400 == D_80366C30) {
        D_80367730 = 1;
    }
    func_8029A7E4("2 pop tune %d\n", D_80367400->tune);
    D_80367708 = D_80367400->tune;
    alCSPStop(D_80367734);
    D_80367728 = 2;
    D_80367714 = vol;
}

void func_8026101C(void) {
    func_80260F60(0.0f);
}

void func_80261040(void) {
    func_80260F60(1.0f);
}

void func_80261068(void) {
    u32 i;
    ALCSeqMarker marker;

    switch (D_80367728) {
        case 2:
            if (func_802D4E10(D_80367734) == 0) {
                func_80260C20(D_80367708, D_80367714);
                D_80367728 = 1;
            }
            break;
        case 1:
            alCSeqGetLoc(&D_80367518[D_802E8D84], &marker);
            if (func_802D4E10(D_80367734) == 1 && marker.lastTicks != 0) {
                alCSeqSetLoc(&D_80367518[D_802E8D84], &D_80367400->marker);
                alCSPSetTempo(D_80367734, D_80367400->tempo);
                for (i = 0; i < 64; i++) {
                    SEQP->chanState[i] = D_80367400->chanState[i];
                }
                D_80367728 = 0;
                if (D_80367714 != 1.0) {
                    func_80261570(1.0f);
                }
            }
            break;
    }
}

void func_802611F0(void) {
    ALCSeqMarker marker;

    alCSeqGetLoc(&D_80367518[D_802E8D84], &marker);
    if (!D_80367729 && !D_80367728 && func_802D4E10(D_80367734) == 0 && marker.lastTicks != 0) {
        func_8029A7E4("auto popping\n");
        func_8026101C();
    }
}

void func_80261284(void) {
    u32 i;

    switch (D_80367729) {
        case 1:
            if (func_802D4E10(D_80367734) == 1) {
                alCSeqGetLoc(&D_80367518[D_802E8D84], &D_80367400->marker);
                D_80367400->tempo = alCSPGetTempo(D_80367734);
                for (i = 0; i < 64; i++) {
                    D_80367400->chanState[i] = SEQP->chanState[i];
                }
                alCSPStop(D_80367734);
                D_80367400++;
                D_80367729 = 2;
            }
            break;
        case 2:
            if (func_802D4E10(D_80367734) == 0) {
                func_80260C20(D_80367708, 1.0f);
                D_80367729 = 0;
            }
            break;
    }
}

#define ABSF(x) ((x) > 0.0f ? (x) : -(x))

void func_802613C8(void) {
    f32 cur;
    f32 target;
    s16 vol;

    cur = func_802D8310(D_80367734);
    target = D_802E8D00[D_80367708] * D_802E8D88;
    vol = cur + (target * D_8036770C - cur) * 0.075;
    if (ABSF(vol - target * D_8036770C) < 10.0f) {
        vol = target * D_8036770C;
        D_8036772A = 0;
    }
    alCSPSetVol(D_80367734, vol);
}

void func_80261528(void) {
    if (SEQP->state == 1) {
        alCSPSetTempo(D_80367734, D_8036772C);
        D_8036772C = 0;
    }
}

void func_80261570(f32 arg0) {
    D_8036770C = arg0;
    D_8036772A = 1;
}

void func_80261588(void) {
    SndConfig sndc;
    ALSeqpConfig seqc;
    SynConfig sync;
    ALBankFile *sfxBank;
    ALBankFile *musBank;
    s32 size2;
    s32 size1;
    u32 i;
    s32 pad[2];
    s32 seqSize;
    s32 hdrSize;

    alHeapInit(&D_80367718, D_80370C80, 0x2A280);
    size2 = size1 = D_003539A0 - D_00350950;
    func_8028B4C4(D_00350950, 0x8004B400, &size2, 0xD, 0, 2);
    musBank = alHeapDBAlloc(0, 0, &D_80367718, 1, size2);
    func_8028B4C4(D_00350950, musBank, &size1, 0xD, 0, 2);
    alBnkfNew(musBank, D_003539A0);
    D_8036773C = musBank->bankArray[0];
    size2 = size1 = D_003A48C0 - D_003A1920;
    func_8028B4C4(D_003A1920, 0x8004B400, &size2, 0xD, 0, 2);
    sfxBank = alHeapDBAlloc(0, 0, &D_80367718, 1, size2);
    func_8028B4C4(D_003A1920, sfxBank, &size1, 0xD, 0, 2);
    alBnkfNew(sfxBank, D_003A48C0);
    D_80367738 = sfxBank->bankArray[0];
    D_80367514 = alHeapDBAlloc(0, 0, &D_80367718, 1, 4);
    hdrSize = 4;
    func_8028B4C4(D_0044F5C0, D_80367514, &hdrSize, 0, 0, 0);
    seqSize = D_80367514->seqCount * 8 + 4;
    D_80367514 = alHeapDBAlloc(0, 0, &D_80367718, 1, 0x214);
    func_8028B4C4(D_0044F5C0, D_80367514, &seqSize, 0, 0, 0);
    alSeqFileNew(D_80367514, D_0044F5C0);
    D_80367510 = alHeapDBAlloc(0, 0, &D_80367718, 1, 0x21AE);
    for (i = 0; i < 0x42; i++) {
        D_80367408[i] = D_80367514->seqArray[i].len;
        if (D_80367408[i] & 1) {
            D_80367408[i]++;
        }
    }
    sync.maxVVoices = 0;
    sync.maxPVoices = 0x18;
    sync.maxUpdates = 0x80;
    sync.maxFXbusses = 1;
    sync.dmaproc = NULL;
    sync.fxType = 6;
    sync.outputRate = 0;
    sync.heap = &D_80367718;
    func_802676A0(&sync, 0xC);
    seqc.maxVoices = 0x18;
    seqc.maxEvents = 0x20;
    seqc.maxChannels = 0x10;
    seqc.heap = &D_80367718;
    seqc.initOsc = NULL;
    seqc.updateOsc = NULL;
    seqc.stopOsc = NULL;
    D_80367734 = alHeapDBAlloc(0, 0, &D_80367718, 1, 0x7C);
    alCSPNew(D_80367734, &seqc);
    alCSPSetBank(D_80367734, D_8036773C);
    sndc.maxEvents = 0x40;
    sndc.maxStates = 0x20;
    sndc.maxSounds = 8;
    sndc.slotCount = 8;
    sndc.heap = &D_80367718;
    func_8025EDF0(&sndc);
    func_8029A7E4("%d bytes audio heap left over\n", D_80367718.len - (D_80367718.cur - D_80367718.base));
    D_8036772A = 0;
    D_80367728 = 0;
    D_80367710 = D_8036770C = 1.0f;
    D_80367730 = 1;
    D_80367400 = D_80366C30;
    func_80267A74();
}

void func_802619D0(u32 id) {
    if (id >= 28) {
        func_8029A7E4("effect id %d out of range!\n", id);
    } else if (D_802E8E7C[id] != 0) {
        func_80260650(D_80367738, D_802E8E7C[id], NULL);
    }
}

u8 func_80261A44(u64 event) {
    u8 tune;
    u8 force;

    force = 0;
    tune = D_80367708;
    if (event != 4) {
        func_80260A10();
    }
    func_802609F0();
    func_80261E9C(event);
    if (D_80364A90 & 0xC9FD8FE7FBFFC0B0) {
        func_80260A30(0);
        func_80260A30(5);
    }
    D_80367710 = 1.0f;
    switch (event) {
        case 2:
            tune = 0x11;
            break;
        case 0x40:
            tune = 0xA;
            break;
        case 0x800:
        case 0x1000:
            tune = 3;
            break;
        case 0x10000:
            tune = 0x21;
            break;
        case 0x20000:
        case 0x40000:
        case 0x100000000:
        case 0x200000000:
        case 0x40000000000000:
            tune = 0x21;
            break;
        case 0x80000000:
            func_80261570(0.0f);
            break;
        case 0x8000000:
            tune = 0xE;
            break;
        case 0x4000:
        case 0x80:
            if (func_80264BA4(D_802E8BDC) == 3) {
                tune = 0xC;
            } else {
                tune = 0x13;
            }
            break;
        case 0x40000000:
            tune = 0xF;
            break;
        case 0x2000:
            if (D_802E8F94[D_802E8BDC][0] != 1) {
                func_80261570(0.0f);
                break;
            }
        case 4:
            tune = D_802E8D8C[D_802E8BDC];
            force = 1;
            break;
        case 0x20000000:
            tune = 0x1D;
            break;
        case 0x100000000000:
            tune = D_802E8D8C[D_802E8BDC];
            force = 1;
            if (D_802E8BDC == 0x26) {
                D_80367710 = 0.7f;
            }
            break;
        case 0x4000000000000:
            func_80261570(0.0f);
            break;
        case 0x10000000000000:
            tune = 0x28;
            break;
        case 0x800000000000:
            func_80261570(0.0f);
            break;
        case 0x4000000000000000:
            break;
    }
    if (tune != D_80367708 || force) {
        func_80261570(0.0f);
        return tune;
    }
    return 0;
}

void func_80261E9C(u64 event) {
    switch (event) {
        case 2:
            func_80260B40(0, 0x5DC0);
            func_80260B40(5, 0x5DC0);
            break;
        case 0x20000000:
            func_80260B40(0, 0x61A8);
            break;
        case 0x40:
            func_80260B40(0, 0x4E20);
            func_80260B40(5, 0x4E20);
            break;
        case 0x800:
        case 0x1000:
            func_80260B40(0, 0x6D60);
            func_80260B40(5, 0x6D60);
            break;
        default:
            func_80260B40(0, 0x7FFF);
            func_80260B40(5, 0x7FFF);
            break;
    }
}

void func_80261FB0(u8 tune) {
    D_80367728 = 0;
    D_80367729 = 0;
    D_80367400 = D_80366C30;
    D_80367730 = 1;
    func_80260C20(tune, D_80367710);
}

void func_80262008(u8 tune, f32 vol) {
    D_80367400 = D_80366C30;
    D_80367730 = 1;
    func_80260C20(tune, vol);
}

u8 func_80262050(void) {
    return D_80367708;
}

s32 func_8026205C(s32 arg0) {
    s32 count;
    s32 j;

    count = 0;
    for (j = 0; j < 5 && D_802E8EB4[arg0][j] != -1; j++, count++) {
    }
    return D_802E8EB4[arg0][osGetCount() % count];
}
