#include "common.h"
#include <ultra64.h>

void func_802604FC(void *arg0);

/* Rare's sound player, an early relative of GoldenEye's snd.c (itself
 * derived from libultra's sndplayer.c). Layouts follow GoldenEye's. */
typedef struct {
    s32 attackTime;
    s32 decayTime;
    s32 releaseTime;
    u8 attackVolume;
    u8 decayVolume;
} SndEnvelope;

typedef struct {
    u8 velocityMin;
    u8 velocityMax;
    u8 keyMin;
    u8 keyMax;
} SndKeyMap;

typedef struct {
    SndEnvelope *envelope;
    SndKeyMap *keyMap;
    void *wavetable;
    u8 samplePan;
    u8 sampleVolume;
} SndSound;

typedef struct SndState {
    struct SndState *next;
    struct SndState *prev;
    SndSound *sound;
    u8 voice[0x1C];
    f32 pitch_28;
    f32 pitch_2c;
    struct SndState *state;
    s16 vol;
    u8 priority;
    s32 unk38;
    u8 pan;
    u8 fxMix;
    u8 flags;
    u8 playingState;
} SndState;

typedef struct {
    u16 type;
    SndState *state;
    union {
        s32 data;
        f32 pitch;
    } u;
    void *ptr;
} SndEvent;

typedef struct {
    s16 priority;
    s16 fxBus;
    u8 unityPitch;
} SndVoiceConfig;

/* TODO: func_8025EDF0 - links arg0->unk8 into D_802E8CEC->0x48, resets
 * D_802E8CEC->0x40 and sets ->0x4c=0x80e8; allocates arg0->unk0*0x40
 * bytes via func_802D6B10 into D_802E8CEC->0x44 and D_802E8CE8, then
 * arg0->unk4*28 bytes into func_802D6E3C(D_802E8CEC+0x14, ., arg0->unk4);
 * for i=1..arg0->unk0-1, calls func_802D6EE0 on 0x40-byte slices of the
 * first allocation; allocates arg0->unk10 u16s into D_80366C28 and fills
 * them with 0x7fff; sets D_802E8CEC->0x38=D_803065C0, ->0=0, ->8=the
 * function pointer func_8025F044, ->4=itself (a self-referential node),
 * calls func_802D6F70(->0x38, D_802E8CEC), posts a {code=0x20} event via
 * func_802D6C8C with D_802E8CEC->0x4c as the extra field, then sets
 * D_802E8CEC->0x50 = func_802D6DB0(D_802E8CEC+0x14, D_802E8CEC+0x28).
 * Logic and every field/offset are confirmed correct (down to 2
 * differing opcodes with the frame, instruction count and nearly every
 * register already exact). The one gap: in the first counted loop,
 * target computes the incremented index straight into the register it
 * reuses for the bound check (`addiu t2,t1,1; sw t2,...; ...; sltu
 * at,t2,t4`), while a plain `i = i + 1;` statement here always reloads i
 * fresh for the following while-test. Folding the increment into the
 * while-condition itself - `while ((i = i + 1) < bound)`, the exact
 * lever that fixed this same gap in func_80260210 and the retried
 * func_80260A30 - backfires here instead: since `i` is also read inside
 * the loop body's own call arguments, IDO promotes it to a callee-saved
 * $s-register across the whole loop (save-slots go from 4 to 12 bytes),
 * which is worse. Left as the plain two-statement form. */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/1A630/func_8025EDF0.s")

/* TODO: func_8025F044 - repeat {if entry->unk28==0x20, post a {code=0x20,
 * param=entry+0x14} event with entry->unk4c as the 3rd field, else call
 * func_8025F0F0(entry, entry+0x28); then entry->unk50 =
 * func_802D6DB0(entry+0x14, entry+0x28)} until unk50 becomes nonzero, then
 * entry->unk54 += unk50 and return unk50 (a polling/retry loop with a
 * fixed `entry`, never advancing - the retries are driven entirely by the
 * three calls' side effects). Logic, every field offset, and the loop
 * condition are all confirmed correct (diff score down to 460, frame size
 * exact, zero inserts/deletes) - the one gap: inside the `unk28==0x20`
 * branch, target reloads `entry` from its stack home a second time right
 * before using it for the call's a0/a2 despite having just loaded it for
 * the preceding comparison, while every phrasing tried keeps reusing the
 * already-loaded register instead, cascading into register-rename diffs
 * for the rest of the function. Tried moving the inner `eventCode` local
 * to function scope; no effect (score 462, same shape). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/1A630/func_8025F044.s")

extern s16 D_802E8CF0;
extern void *D_802E8CE4;
extern u16 *D_80366C28;
void func_8026005C(void *arg0);
void func_802600D8(void *arg0);
void func_80260148(void *, void *, u16);
u16 func_80260210(u16 *arg0, u16 *arg1);
void *func_80260650(void *arg0, s16 arg1, void *arg2);
void func_80260AB8(void *arg0, s16 arg1, s32 arg2);
void func_8029A7E4(char *, ...);
s32 func_802D6C8C(void *, void *, s32);
s32 func_802D70A8(void *, void *, SndVoiceConfig *);
void func_802D71F0(void *, void *, s32, s32);
void func_802D7290(void *, void *, void *);
void func_802D7320(void *, void *, u8);
void func_802D73B0(void *, void *, f32);
void func_802D7440(void *, void *, s32);

#define SNDP_DRVR(sndp) (*(void **) ((u8 *) (sndp) + 0x38))
#define SNDP_EVTQ(sndp) ((void *) ((u8 *) (sndp) + 0x14))
#define SNDP_MAXSOUNDS(sndp) (*(s32 *) ((u8 *) (sndp) + 0x48))
#define SLOT_VOLUME(keyMap) (((s16 *) D_80366C28)[(keyMap)->keyMin & 0x3F])

void func_8025F0F0(void *sndp, SndEvent *event) {
    SndVoiceConfig config;
    SndSound *sound;
    SndKeyMap *keyMap;
    void *voice;
    u8 pan;
    SndEvent spAC;
    SndEvent nextStateEvent;
    s32 delta;
    s32 fxMix;
    s32 volume;
    s32 panTmp;
    s32 limitReached;
    s32 isEventForSingleSound;
    s32 lastInSequence;
    s32 pad58;
    s32 isVoiceAllocated;
    SndState *soundState;
    SndState *nextState;

    voice = NULL;
    lastInSequence = TRUE;
    isVoiceAllocated = FALSE;
    nextState = NULL;

    do {
        if (nextState != NULL) {
            nextStateEvent.state = soundState;
            nextStateEvent.type = event->type;
            nextStateEvent.u.data = event->u.data;
            event = &nextStateEvent;
        }

        soundState = event->state;
        sound = soundState->sound;

        if (sound == NULL) {
            u16 numFree;
            u16 numAlloc;

            func_80260210(&numFree, &numAlloc);
            func_8029A7E4("Bad soundState: voices =%d, states free =%d, states busy =%d, type %d data %x\n",
                          D_802E8CF0, numFree, numAlloc, event->type, event->u.data);
            return;
        }

        keyMap = sound->keyMap;
        nextState = soundState->next;

        switch (event->type) {
            case 1:
                if (soundState->playingState != 5 && soundState->playingState != 4) {
                    return;
                }
                if (soundState->playingState == 1) {
                    func_8029A7E4("playing a playing sound\n");
                }

                config.fxBus = 0;
                config.priority = soundState->priority;
                config.unityPitch = 0;

                limitReached = D_802E8CF0 >= SNDP_MAXSOUNDS(sndp);

                if (!limitReached || (soundState->flags & 0x50)) {
                    isVoiceAllocated = func_802D70A8(SNDP_DRVR(sndp), soundState->voice, &config);
                }

                if (!isVoiceAllocated) {
                    if ((soundState->flags & 0x52) || soundState->unk38 > 0) {
                        soundState->playingState = 4;
                        soundState->unk38--;
                        func_802D6C8C(SNDP_EVTQ(sndp), event, 33333);
                    } else if (limitReached) {
                        SndState *iterState = (SndState *) D_802E8CE4;

                        do {
                            if (!(iterState->flags & 0x52) && (iterState->flags & 4) &&
                                iterState->playingState != 3) {
                                SndEvent interruptEvent;

                                limitReached = FALSE;
                                interruptEvent.type = 0x80;
                                interruptEvent.state = iterState;
                                iterState->playingState = 3;
                                func_802D6C8C(SNDP_EVTQ(sndp), &interruptEvent, 1000);
                                func_802D71F0(SNDP_DRVR(sndp), iterState->voice, 0, 1000);
                            }
                            iterState = iterState->prev;
                        } while (limitReached && iterState != NULL);

                        if (!limitReached) {
                            soundState->unk38 = 2;
                            func_802D6C8C(SNDP_EVTQ(sndp), event, 1001);
                        } else {
                            func_8026005C(soundState);
                        }
                    } else {
                        func_8026005C(soundState);
                    }
                    return;
                }

                soundState->flags |= 4;
                func_802D7290(SNDP_DRVR(sndp), soundState->voice, sound->wavetable);
                soundState->playingState = 1;
                D_802E8CF0++;

                delta = sound->envelope->attackTime / soundState->pitch_2c / soundState->pitch_28;
                volume = MAX(0, SLOT_VOLUME(keyMap) *
                                    (sound->envelope->attackVolume * soundState->vol * sound->sampleVolume / 16129) /
                                    32767 - 1);
                func_802D71F0(SNDP_DRVR(sndp), soundState->voice, 0, 0);
                func_802D71F0(SNDP_DRVR(sndp), soundState->voice, volume, delta);

                panTmp = soundState->pan + sound->samplePan - 0x40;
                pan = MIN(MAX(panTmp, 0), 0x7F);
                func_802D7320(SNDP_DRVR(sndp), soundState->voice, pan);

                func_802D73B0(SNDP_DRVR(sndp), soundState->voice, soundState->pitch_2c * soundState->pitch_28);

                fxMix = (soundState->fxMix + (keyMap->keyMax & 0xF)) * 8;
                fxMix = MIN(127, MAX(0, fxMix));
                func_802D7440(SNDP_DRVR(sndp), soundState->voice, fxMix);

                spAC.type = 0x40;
                spAC.state = soundState;
                delta = sound->envelope->attackTime / soundState->pitch_2c / soundState->pitch_28;
                func_802D6C8C(SNDP_EVTQ(sndp), &spAC, delta);
                break;

            case 2:
            case 0x400:
            case 0x1000:
                if (event->type != 0x1000 || (soundState->flags & 2)) {
                    switch (soundState->playingState) {
                        case 1:
                            func_80260148(SNDP_EVTQ(sndp), soundState, 0x40);
                            delta = sound->envelope->releaseTime / soundState->pitch_28 / soundState->pitch_2c;
                            func_802D71F0(SNDP_DRVR(sndp), soundState->voice, 0, delta);
                            if (delta != 0) {
                                spAC.type = 0x80;
                                spAC.state = soundState;
                                func_802D6C8C(SNDP_EVTQ(sndp), &spAC, delta);
                                soundState->playingState = 2;
                            } else {
                                func_8026005C(soundState);
                            }
                            break;
                        case 4:
                        case 5:
                            func_8026005C(soundState);
                            break;
                    }
                    if (event->type == 2) {
                        event->type = 0x1000;
                    }
                }
                break;

            case 4:
                soundState->pan = event->u.data;
                if (soundState->playingState == 1) {
                    panTmp = soundState->pan + sound->samplePan - 0x40;
                    pan = MIN(MAX(panTmp, 0), 0x7F);
                    func_802D7320(SNDP_DRVR(sndp), soundState->voice, pan);
                }
                break;

            case 0x10:
                soundState->pitch_2c = event->u.pitch;
                if (soundState->playingState == 1) {
                    func_802D73B0(SNDP_DRVR(sndp), soundState->voice, soundState->pitch_2c * soundState->pitch_28);
                    if (soundState->flags & 0x20) {
                        func_802600D8(soundState);
                    }
                }
                break;

            case 0x100:
                soundState->fxMix = event->u.data;
                if (soundState->playingState == 1) {
                    fxMix = (soundState->fxMix + (keyMap->keyMax & 0xF)) * 8;
                    fxMix = MIN(127, MAX(0, fxMix));
                    func_802D7440(SNDP_DRVR(sndp), soundState->voice, fxMix);
                }
                break;

            case 8:
                soundState->vol = event->u.data;
                if (soundState->playingState == 1) {
                    volume = MAX(0, SLOT_VOLUME(keyMap) *
                                        (sound->envelope->decayVolume * soundState->vol * sound->sampleVolume / 16129) /
                                        32767 - 1);
                    func_802D71F0(SNDP_DRVR(sndp), soundState->voice, volume, 1000);
                }
                break;

            case 0x800:
                if (soundState->playingState == 1) {
                    delta = sound->envelope->releaseTime / soundState->pitch_28 / soundState->pitch_2c;
                    volume = MAX(0, SLOT_VOLUME(keyMap) *
                                        (sound->envelope->decayVolume * soundState->vol * sound->sampleVolume / 16129) /
                                        32767 - 1);
                    func_802D71F0(SNDP_DRVR(sndp), soundState->voice, volume, delta);
                }
                break;

            case 0x40:
                if (!(soundState->flags & 2)) {
                    volume = MAX(0, SLOT_VOLUME(keyMap) *
                                        (sound->envelope->decayVolume * soundState->vol * sound->sampleVolume / 16129) /
                                        32767 - 1);
                    delta = sound->envelope->decayTime / soundState->pitch_28 / soundState->pitch_2c;
                    func_802D71F0(SNDP_DRVR(sndp), soundState->voice, volume, delta);

                    spAC.type = 2;
                    spAC.state = soundState;
                    func_802D6C8C(SNDP_EVTQ(sndp), &spAC, delta);

                    if (soundState->flags & 0x20) {
                        func_802600D8(soundState);
                    }
                }
                break;

            case 0x80:
                func_8026005C(soundState);
                break;

            case 0x200:
                if (soundState->flags & 0x10) {
                    void *newState;

                    newState = func_80260650(event->ptr, event->u.data, soundState->state);
                    func_80260AB8(newState, 8, soundState->vol);
                    func_80260AB8(newState, 4, soundState->pan);
                    func_80260AB8(newState, 0x100, soundState->fxMix);
                    func_80260AB8(newState, 0x10, *(s32 *) &soundState->pitch_2c);
                }
                break;

            default:
                func_8029A7E4("Nonsense sndp event\n");
                break;
        }

        isEventForSingleSound = event->type & (1 | 0x10 | 0x40 | 0x80 | 0x200);
        soundState = nextState;

        if (soundState != NULL && !isEventForSingleSound) {
            lastInSequence = soundState->flags & 1;
        }
    } while (!lastInSequence && soundState != NULL && !isEventForSingleSound);
}

extern void *D_802E8CEC;

void func_802D74E0(s32, void *);
void func_802D7560(s32, void *);
void func_80260148(void *, void *, u16);

void func_8026005C(void *arg0) {
    if (*((u8 *) arg0 + 0x3e) & 4) {
        func_802D74E0(*(s32 *) ((u8 *) D_802E8CEC + 0x38), (u8 *) arg0 + 0xc);
        func_802D7560(*(s32 *) ((u8 *) D_802E8CEC + 0x38), (u8 *) arg0 + 0xc);
    }
    func_802604FC(arg0);
    func_80260148((u8 *) D_802E8CEC + 0x14, arg0, 0xffff);
}

f32 func_802D7610(s32);

void func_802600D8(void *arg0) {
    SndEvent evt;
    f32 pitch;

    pitch = (f32) (func_802D7610(*(s8 *) ((u8 *) ((SndState *) arg0)->sound->keyMap + 5)) *
                   (f32) ((SndState *) arg0)->pitch_2c);
    evt.state = arg0;
    evt.type = 0x10;
    evt.u.data = *(s32 *) &pitch;
    func_802D6C8C((u8 *) D_802E8CEC + 0x14, &evt, 33333);
}

s32 func_802D6EB0(void *);
s32 func_802D6EE0(void *, void *);
s32 func_802D7660(s32);

void func_80260148(void *arg0, void *arg1, u16 arg2) {
    void *list;
    void *next;
    void *entry2;
    void *dup;
    void *addr;
    s32 savedState;

    savedState = func_802D7660(1);
    list = *(void **) ((u8 *) arg0 + 8);
    while (list != NULL) {
        next = *(void **) list;
        addr = (u8 *) list + 0xc;
        entry2 = list;
        dup = next;

        if (*(void **) ((u8 *) list + 0x10) == arg1) {
            if ((*(u16 *) ((u8 *) list + 0xc) & arg2) != 0) {
                if (next != NULL) {
                    *(s32 *) ((u8 *) next + 8) += *(s32 *) ((u8 *) list + 8);
                }
                func_802D6EB0(list);
                func_802D6EE0(list, arg0);
            }
        }
        list = next;
    }

    func_802D7660(savedState);
}

extern void *D_802E8CE0;
extern void *D_802E8CE4;
extern void *D_802E8CE8;
s32 func_802D7660(s32);

u16 func_80260210(u16 *arg0, u16 *arg1) {
    s32 savedState;
    u16 count1;
    u16 count2;
    u16 count3;
    void *list1;
    void *list2;
    void *list3;

    savedState = func_802D7660(1);

    list1 = D_802E8CE0;
    list2 = D_802E8CE8;
    list3 = D_802E8CE4;

    count1 = 0;
    if (list1 != NULL) {
        do {
            count1 = count1 + 1;
        } while ((list1 = *(void **) list1) != NULL);
    }

    count2 = 0;
    if (list2 != NULL) {
        do {
            count2 = count2 + 1;
        } while ((list2 = *(void **) list2) != NULL);
    }

    count3 = 0;
    if (list3 != NULL) {
        do {
            count3 = count3 + 1;
        } while ((list3 = *(void **) ((u8 *) list3 + 4)) != NULL);
    }

    *arg0 = count2;
    *arg1 = count1;

    func_802D7660(savedState);
    return count3;
}

/* TODO: func_80260300 - pop a node off the D_802E8CE8 free list (calling
 * func_802D6EB0 on it), link it onto the front of the D_802E8CE0 active
 * list (or make it the sole element of both D_802E8CE0 and D_802E8CE4 if
 * that list was empty), then initialize it from arg1 (stored at node+8)
 * and arg1->unk4 (byte fields at +3/+4/+5): flags at +0x36/+0x3e/+0x38,
 * a float at +0x2c=1.0 and +0x28=func_802D7610(arg1->unk4-derived
 * index*100 [+unk5 if a +0x3e flag bit is clear] - 0x1770), a "negative"
 * flag folded in from (arg1->unk0)->unk4==-1, and a handful of fixed
 * resets (+0x30, +0x3d, +0x3c, +0x34). Returns the node (NULL if the
 * free list was empty). Logic and every field/offset are confirmed
 * correct (down to a single differing opcode once the frame, instruction
 * count and register coloring are already exact). The one gap: in the
 * empty-active-list branch, target writes D_802E8CE0 and D_802E8CE4
 * (the same value, node) through a single shared `lui` - only possible
 * if both addresses come from the same relocation, i.e. the real source
 * reaches the second global via pointer arithmetic on the first rather
 * than naming it, but every arithmetic spelling tried
 * (`*((u8*)&D_802E8CE0+4)`, `*(&D_802E8CE0+1)`) made the front end
 * materialize the computed address as its own value first and then add
 * a second, redundant displacement, costing 2 instructions instead of
 * saving 1. Plain separate globals cost exactly the 1 extra `lui`
 * instead. Also ruled out by probe compiles: a {head, tail} struct
 * (fields still get one `lui at` each), a 2-element array (addiu-built
 * bases), the chained `D_802E8CE4 = D_802E8CE0 = node` (re-reads the
 * first store), and absolute-address macros (`lui` into t-regs, not
 * shared). */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/1A630/func_80260300.s")

extern s16 D_802E8CF0;

void func_802604FC(void *arg0) {
    if (D_802E8CE0 == arg0) {
        D_802E8CE0 = *(void **) arg0;
    }
    if (D_802E8CE4 == arg0) {
        D_802E8CE4 = *(void **) ((u8 *) arg0 + 4);
    }
    func_802D6EB0(arg0);

    if (D_802E8CE8 != NULL) {
        *(void **) arg0 = D_802E8CE8;
        *(s32 *) ((u8 *) arg0 + 4) = 0;
        *(s32 *) ((u8 *) D_802E8CE8 + 4) = (s32) arg0;
        D_802E8CE8 = arg0;
    } else {
        *(s32 *) ((u8 *) arg0 + 4) = 0;
        *(void **) arg0 = *(void **) ((u8 *) arg0 + 4);
        D_802E8CE8 = arg0;
    }

    if (*(u8 *) ((u8 *) arg0 + 0x3e) & 4) {
        D_802E8CF0--;
    }
    *(u8 *) ((u8 *) arg0 + 0x3f) = 0;
    if (*(void **) ((u8 *) arg0 + 0x30) != NULL) {
        if (**(void ***) ((u8 *) arg0 + 0x30) == arg0) {
            **(void ***) ((u8 *) arg0 + 0x30) = NULL;
        }
        *(void **) ((u8 *) arg0 + 0x30) = NULL;
    }
}

void func_80260618(void *arg0, u8 arg1) {
    if (arg0 != NULL) {
        *(s8 *)((u8 *) arg0 + 0x36) = (s8)(s16)(u8) arg1;
    }
}

u8 func_80260634(void *arg0) {
    if (arg0 != NULL) {
        return *(u8 *) ((u8 *) arg0 + 0x3f);
    }
    return 0;
}

extern s32 D_80358060;
extern s32 D_802E8BDC;
extern void *D_802E8CEC;
void *func_80260300(void *, void *);
s32 func_802D6C8C(void *, void *, s32);

void *func_80260650(void *arg0, s16 arg1, void *arg2) {
    void *node;
    void *result;
    void *entry4dead;
    void *entry;
    s16 flag1;
    s32 adjusted;
    s32 scaled;
    s32 totalSomething;
    s32 pad3;
    s32 pad4;
    s32 eventParam;
    s16 pad;
    s16 eventCode;
    s32 extra2;
    s32 extra1;
    s32 eventParam2;
    s16 pad2;
    s16 eventCode2;

    result = NULL;
    flag1 = 0;
    totalSomething = 0;
    if (arg1 == 0 || (D_80358060 == 0 && arg1 == 0xc)) {
        goto retNull;
    }
    if (arg1 == 0x14 || arg1 == 0x15) {
        if (D_802E8BDC == 0x26 || D_802E8BDC == 0x31) {
            goto retNull;
        }
    }
    goto mainLogic;

retNull:
    return NULL;

mainLogic:
    do {
        entry = *(void **) ((u8 *) (*(void **) ((u8 *) arg0 + 0xc))
            + (arg1 << 2) + 0xc);
        node = func_80260300(arg0, entry);
        if (node != NULL) {
            *(void **) ((u8 *) D_802E8CEC + 0x40) = node;
            eventCode = 1;
            eventParam = (s32) node;
            scaled = (s32) *(u8 *) ((u8 *) (*(void **) ((u8 *) entry + 4)) + 1)
                * 33333;

            if (*(u8 *) ((u8 *) node + 0x3e) & 0x10) {
                *(u8 *) ((u8 *) node + 0x3e) &= ~0x10;
                func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode,
                    totalSomething + 1);
                adjusted = scaled + 1;
                flag1 = arg1;
            } else {
                func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode,
                    scaled + 1);
            }
            result = node;
        } else {
            func_8029A7E4("Sound state allocate failed - sndId %d\n", arg1);
        }

        totalSomething = totalSomething + scaled;
        entry4dead = *(void **) ((u8 *) entry + 4);
        arg1 = ((*(u8 *) ((u8 *) entry4dead + 2) & 0xc0) << 2)
            + *(u8 *) entry4dead;
    } while (arg1 != 0 && node != NULL);

    if (result != NULL) {
        *(u8 *) ((u8 *) result + 0x3e) |= 1;
        *(s32 *) ((u8 *) result + 0x30) = (s32) arg2;
        if (flag1 != 0) {
            *(u8 *) ((u8 *) result + 0x3e) |= 0x10;
            eventCode2 = 0x200;
            eventParam2 = (s32) result;
            extra1 = flag1;
            extra2 = (s32) arg0;
            func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode2, adjusted);
        }
    }

    if (arg2 != NULL) {
        *(void **) arg2 = result;
    }
    return result;
}

extern void *D_802E8CEC;

s32 func_802D6C8C(void *, void *, s32);

/* eventTail/eventHead are dead stack space never read or written here - the
 * real source likely shares one ~0x10-byte event-record local across sites
 * like this one and only fills the leading eventCode/eventParam fields,
 * leaving the rest as reserved/unused padding (same shape as a plain array
 * would need, but writing it as one keeps the 0x24/0x20-offset slots from
 * ever materializing their own address, which doesn't match target). */
void func_802608C8(void *arg0) {
    s32 eventTail;
    s32 eventHead;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    eventCode = 0x400;
    eventParam = (s32) arg0;

    if (arg0 != NULL) {
        *((u8 *) arg0 + 0x3e) &= ~0x10;
        func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
    } else {
        func_8029A7E4("WARNING: Attempt to stop NULL sound aborted\n");
    }
}

extern void *D_802E8CE0;
s32 func_802D7660(s32);

void func_80260934(u8 arg0) {
    s32 savedState;
    s32 unused2;
    s32 unused1;
    s32 eventParam;
    s16 pad;
    s16 eventCode;
    void *entry;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    if (entry != NULL) {
        do {
            eventCode = 0x400;
            eventParam = (s32) entry;
            if ((*((u8 *) entry + 0x3e) & arg0) == arg0) {
                *((u8 *) entry + 0x3e) &= ~0x10;
                func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
            }
            entry = *(void **) entry;
        } while (entry != NULL);
    }
    func_802D7660(savedState);
}

void func_802609D0(void) {
    func_80260934(1);
}

void func_802609F0(void) {
    func_80260934(0x11);
}

void func_80260A10(void) {
    func_80260934(3);
}

void func_80260A30(u8 arg0) {
    s32 savedState;
    void *entry;
    s32 count;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    count = 0;
    if (entry != NULL) {
        do {
            if ((*(u8 *) ((u8 *) (*(void **) ((u8 *) (*(void **) ((u8 *) entry + 8)) + 4)) + 2) & 0x3f) == arg0) {
                func_802608C8(entry);
            }
            count = count + 1;
        } while ((entry = *(void **) entry) != NULL);
    }
    func_802D7660(savedState);
}


void func_80260AB8(void *arg0, s16 arg1, s32 arg2) {
    s32 unused;
    s32 eventExtra;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    eventCode = arg1;
    eventParam = (s32) arg0;
    eventExtra = arg2;

    if (arg0 != NULL) {
        func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
    } else {
        func_8029A7E4("WARNING: Attempt to modify NULL sound aborted\n");
    }
}

extern u16 *D_80366C28;

u16 func_80260B24(u8 arg0) {
    return D_80366C28[arg0];
}

extern void *D_802E8CE0;
extern void *D_802E8CEC;
s32 func_802D7660(s32);
s32 func_802D6C8C(void *, void *, s32);

void func_80260B40(u8 arg0, u16 arg1) {
    s32 savedState;
    void *entry;
    s32 count;
    s32 eventTail;
    s32 eventHead;
    s32 eventParam;
    s16 pad;
    s16 eventCode;

    savedState = func_802D7660(1);
    entry = D_802E8CE0;
    D_80366C28[arg0] = arg1;

    count = 0;
    while (entry != NULL) {
        if ((*(u8 *) ((u8 *) (*(void **) ((u8 *) (*(void **) ((u8 *) entry + 8)) + 4)) + 2) & 0x3f) == arg0) {
            eventCode = 0x800;
            eventParam = (s32) entry;
            func_802D6C8C((u8 *) D_802E8CEC + 0x14, &eventCode, 0);
        }
        count = count + 1;
        entry = *(void **) entry;
    }

    func_802D7660(savedState);
}
