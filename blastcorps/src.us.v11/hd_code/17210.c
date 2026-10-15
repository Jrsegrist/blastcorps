#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* recording.c (from its assert strings): demo/attract-mode controller recording and playback */

typedef struct {
    u8 btnHi;
    u8 btnLo;
    u8 count;
    s8 stickX;
    s8 stickY;
} RecEntry;

extern s32 D_80366990;
extern s32 D_80366994;
extern s32 D_80366998;
extern s32 D_8036699C;
extern s32 D_803669A0;
extern u16 D_803669A4;
extern s8 D_803669A6;
extern s8 D_803669A7;
extern u8 D_803669A8;
extern RecEntry * N64P D_803669AC;
extern void * N64P D_803669B0;
extern RecEntry D_80365588[];
extern u16 D_802E8CB0[]; /* sprite ids (spr_load) */


#define ASSERT(EX, line) if (!(EX)) debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "recording.c", line)

void rec_start_playback(s32 arg0, s32 *arg1) {
    s32 i;
    u8 *rom;
    s32 size;
    s32 len;
    u16 unk26;
    u16 unk24;
    u8 *p;

    rom = romDemoRecordings;
    size = romFeBgPicture1 - rom;
    p = gHeapPtr;
    hd_rom_load((u32) rom, (u32) p, (u32 *) &size, 9, 0, 1);
    gHeapPtr = (u8 *) gHeapPtr + size;
    align_up((s32 *) &gHeapPtr, 0x10);
    for (i = 0; i < arg0; i++) {
        len = *(s16 *) (p + 0x140C);
        p = p + len + 0x140E;
    }
    D_80366A04 = *(u16 *) p;
    unk26 = *(u16 *) (p + 2);
    D_80366998 = *(u16 *) (p + 8);
    *arg1 = p[0xA];
    D_8036698C = p[0xB];
    D_803669AC = (RecEntry *) (p + 0xC);
    len = *(s16 *) (p + 0x140C);
    p = p + 0x140E;
    align_up((s32 *) ((void **) &p), 2);
    D_803669B0 = p;
    D_80366994 = 0;
    D_803669A0 = D_803669AC->count;
    D_803669B4 = 1;
}

/* Starts recording the player's pad: empties the RAM recording and skips the first rec_record call
 * (0x8025BB38). */
void rec_start_recording(void) {
    D_80366990 = 0;
    D_803669A8 = 1;
}

/* Starts playing back the RAM recording just made (the post-level replay) and loads the replay sprites
 * (0x8025BB50). */
void rec_start_replay(void) {
    D_80366998 = D_80366990;
    D_80366994 = 0;
    D_803669AC = D_80365588;
    D_80365580 = spr_load((u16 *) D_802E8CB0, 0, 1, 1, 1, 1.0f);
    D_803669A0 = D_803669AC[D_80366994].count;
}

void rec_record(u16 buttons, s8 x, s8 y) {
    if (D_803669A8 == 0) {
        if (buttons != D_803669A4 || x != D_803669A6 || y != D_803669A7 || D_8036699C == 0xFF || gNextGameMode != 0) {
            if (D_80366990 < 0x400) {
                D_80365588[D_80366990].btnHi = D_803669A4 >> 8;
                D_80365588[D_80366990].btnLo = D_803669A4 & 0xFF;
                D_80365588[D_80366990].stickX = D_803669A6;
                D_80365588[D_80366990].stickY = D_803669A7;
                D_80365588[D_80366990].count = D_8036699C;
                D_80366990++;
                D_8036699C = 0;
            }
        } else {
            D_8036699C++;
        }
    } else {
        D_8036699C = 0;
    }
    D_803669A4 = buttons;
    D_803669A6 = x;
    D_803669A7 = y;
    D_803669A8 = 0;
}

/* At level setup after rec_start_playback: loads the recording's saved vehicle state, enters that vehicle and
 * makes it the current one (0x8025BD98). */
void rec_restore_start_vehicle(void) {
    switch (D_8036698C) {
        case 3:
            skyfall_load_state(D_803669B0);
            skyfall_enter();
            break;
        case 0:
            driver_load_state(D_803669B0);
            break;
        case 1:
            sideswipe_load_state(D_803669B0);
            sideswipe_enter();
            break;
        case 2:
            thunderfist_load_state(D_803669B0);
            thunderfist_enter();
            break;
        case 16:
            cyclone_load_state(D_803669B0);
            cyclone_enter();
            break;
        case 4:
            ramdozer_load_state(D_803669B0);
            ramdozer_enter();
            break;
        case 5:
            backlash_load_state(D_803669B0);
            backlash_enter();
            break;
        case 9:
            jbomb_load_state(D_803669B0);
            jbomb_enter();
            break;
        case 10:
            ballista_load_state(D_803669B0);
            ballista_enter();
            break;
    }
    if (D_8036698C) {
        D_803649E8 = 1;
    }
    gCurVehicle = D_8036698C;
    D_803669B4 = 0;
}

void rec_next(void) {
    s32 pad[2];

    if (D_80366994 > D_80366998 || (gHaveCarrier && gCarrierHit && (gGameMode & (MODE_REPLAY | MODE_REPLAY_CARRIER_END)))) {
        if (yoshiState != 1 || ((gGameMode & (MODE_REPLAY | MODE_REPLAY_CARRIER_END)) && !D_80364A50)) {
            if (!gPaused) {
                gPauseRequest = 1;
            }
        } else if (!areWeFading()) {
            switch (gGameMode) {
                case MODE_ATTRACT:
                    fade_to_mode_speed(MODE_ATTRACT, 0.25f);
                    break;
                case MODE_GLOBE_SEQ:
                    debug_printf("sequence playback over\n");
                    fade_to_mode(MODE_SEQ_END);
                    break;
                case MODE_REPLAY:
                case MODE_REPLAY_CARRIER_END:
                    gPauseRequest = 1;
                    if (gGameMode == MODE_REPLAY_CARRIER_END) {
                        fade_to_mode_speed(MODE_REPLAY, 1.25f);
                    } else {
                        fade_to_mode_speed(MODE_REPLAY, 0.5f);
                    }
                    break;
                default:
                    ASSERT(1==0, 388);
                    break;
            }
        }
    } else {
        gRecButtons = (D_803669AC[D_80366994].btnHi << 8) + D_803669AC[D_80366994].btnLo;
        gRecStickX = D_803669AC[D_80366994].stickX;
        gRecStickY = D_803669AC[D_80366994].stickY;
        if (--D_803669A0 < 0) {
            D_80366994++;
            D_803669A0 = D_803669AC[D_80366994].count;
        }
        if ((gGameMode & (MODE_ATTRACT | MODE_GLOBE_SEQ)) && D_8036698C != 9) {
            if ((gRecButtons & 0xC000) == 0x8000) {
                gRecButtons &= ~0x8000;
                gRecButtons |= 0x4000;
            }
            if ((gRecButtons & 0xC000) == 0x4000) {
                gRecButtons &= ~0x4000;
                gRecButtons |= 0x8000;
            }
        }
    }
}
