#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define gYoshiGameItems ((YoshiItem *) gYoshiGameItems)
#define gYoshiWindows ((YoshiArg *) gYoshiWindows)
/* end of views */

/* controller.c: controller init, reading, the demo/replay pad source and
 * per-button edge flags. */

#define CONTROLLER_ASSERT(EX, line) \
    if (!(EX)) debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "controller.c", line)

/* A text-window item, 0x1C bytes (yoshi.c's YoshiItem) */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u8 unk2[4];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ char * N64P text;
    /* 0x10 */ u16 * N64P jtext;
    /* 0x14 */ u8 unk14[8];
} YoshiItem;

/* 0x1C bytes (see 26570.c) */
typedef struct {
    /* 0x00 */ u8 unk0[8];
    /* 0x08 */ u32 flags;
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u8 unk12[6];
    /* 0x18 */ s16 unk18;
    /* 0x1A */ u8 unk1A[2];
} YoshiArg;

/* Per-player state, 0x100 bytes */
typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u8 unk4[0xFC];
} Player;

extern OSContPad gContPad; /* this frame's pad */
extern OSMesg gSiMesgBuf;
extern OSContStatus gContStatus;
extern u8 gContReadPending; /* a read is in flight */
extern Player D_80364BE0[];

/* stick x/y: this frame and last frame */
/* one flag per button, set while held */
extern u8 gHeldUp; /* U_JPAD */
extern u8 gHeldDown; /* D_JPAD */
extern u8 gHeldStart; /* START */
extern u8 gHeldCDown; /* D_CBUTTONS */
extern u8 gHeldCUp; /* U_CBUTTONS */
/* last frame's C buttons */
extern u8 gPrevHeldCDown;
extern u8 gPrevHeldCUp;
extern s8 gPrevPadStickY;
/* the alternative pad source */
extern s32 D_80370BC0;
extern s32 D_80370C40;

void cont_process_stick(u8 arg0, u8 arg1, u16 *btn, s8 *x, s8 *y);
void cont_buttons_to_stick(u16 btn, s8 *x, s8 *y);
void cont_stick_to_buttons(u16 *btn, s8 x, s8 y);
void cont_dead_zone(s8 *x, s8 *y);

s32 D_802FDB10 = 0;
u8 D_802FDB14 = 0;

/* Set up the SI queue and the controllers; returns the controller bit pattern */
u8 cont_init(void) {
    s32 pad;
    u8 pattern;

    osCreateMesgQueue(&gSiMesgQueue, &gSiMesgBuf, 1);
    osSetEventMesg(OS_EVENT_SI, &gSiMesgQueue, 0);
    osContInit(&gSiMesgQueue, &pattern, &gContStatus);
    osContSetCh(1);
    gContReadPending = 0;
    gControlMethod = 0;
    return pattern;
}

/* Start a controller read */
void cont_start_read(void) {
    if (pakBusy == 0) {
        cont_wait_read();
        osContStartReadData(&gSiMesgQueue);
        gContReadPending = 1;
    }
}

/* Wait for an in-flight controller read */
void cont_wait_read(void) {
    if (gContReadPending) {
        osRecvMesg(&gSiMesgQueue, NULL, 1);
        gContReadPending = 0;
    }
}

/* Read this frame's pad and update the button/stick state */
void cont_update(void) {
    OSContPad *pad = &gContPad;
    s32 unused;
    s32 mode;

    if (((u32) D_80358064) != 0) {
        if (pakBusy == 0 && gContReadPending != 0) {
            osRecvMesg(&gSiMesgQueue, NULL, 1);
            osContGetReadData(pad);
            if (pad->errno) {
                debug_printf("pad read error - zeroing data\n");
                pad->stick_x = pad->stick_y = pad->button = 0;
            }
            gContReadPending = 0;
        }
        gContPrevButtons = gContButtons;
        gContButtons = pad->button;
        if (gSwallowB) {
            if (pad->button & B_BUTTON) {
                pad->button &= ~B_BUTTON;
            } else {
                gSwallowB = 0;
            }
        }
        if (gFramesInMode < 6) {
            if (gFramesInMode == 5) {
                gContPrevButtons |= A_BUTTON | B_BUTTON | START_BUTTON;
            } else {
                gContButtons &= ~(A_BUTTON | B_BUTTON | START_BUTTON);
            }
        }
        gContPrevStickX = gContStickX;
        gContPrevStickY = gContStickY;
        gContStickX = pad->stick_x;
        gContStickY = pad->stick_y;
        gPrevHeldCLeft = gHeldCLeft;
        gPrevHeldCDown = gHeldCDown;
        gPrevHeldCUp = gHeldCUp;
        gPrevHeldCRight = gHeldCRight;
        switch (gGameMode) {
            case 2:
            case 0x40:
            case 0x400:
            case 0x100000000000:
                if (levelno != 0x26 && levelno != 0x31) {
                    do {
                        rec_next();
                        if (gRecButtons & 0x40) {
                            D_80370C40 = 0;
                        } else if (gRecButtons & 0x80) {
                            D_80370C40 = 1;
                        }
                    } while (gRecButtons & 0xC0);
                }
                pad->button = gRecButtons;
                pad->stick_x = gRecStickX;
                pad->stick_y = gRecStickY;
                pad->button &= ~U_CBUTTONS;
                break;
            case 4:
            case 0x100:
                if (!gPaused || gGameMode == MODE_LEVEL_START) {
                    rec_record(pad->button & ~START_BUTTON, pad->stick_x, pad->stick_y);
                }
                break;
            case 1:
            case 8:
            case 0x200:
            case 0x800:
            case 0x1000:
            case 0x4000000:
                pad->button = 0;
                pad->stick_x = 0;
                pad->stick_y = 0;
                break;
        }
        D_80370C34 = 0;
        if (gGameMode & 0x2000100000002546) {
            D_803ED40A = 0;
            D_803F7C34 = 0;
            if (gGameMode & 0x440) {
                mode = D_80370C40;
            } else if (D_80364BE0[playerNumber].flags & (1 << gCurVehicle)) {
                mode = 1;
            } else {
                mode = 0;
            }
            if (gGameMode == 2 || gGameMode == MODE_GLOBE_SEQ) {
                mode = 0;
            }
            gControlMethod = mode;
            if (gPaused) {
                cont_process_stick(1, 0, &pad->button, &pad->stick_x, &pad->stick_y);
            } else {
                switch (mode) {
                    case 0:
                        switch (gCurVehicle) {
                            case 9:
                                if (gJbombAirborne) {
                                    D_80370C34 = 1;
                                }
                            case 0:
                            case 2:
                            case 16:
                                cont_stick_to_steer(&pad->stick_x, &pad->stick_y, gCurVehicle);
                                cont_dead_zone(&pad->stick_x, &pad->stick_y);
                                break;
                            default:
                                D_80370C75 = 0;
                                cont_process_stick(0, 0, &pad->button, &pad->stick_x, &pad->stick_y);
                                break;
                        }
                        break;
                    case 1:
                        if (gCurVehicle == 9) {
                            D_80370C75 = 0;
                            cont_process_stick(1, 1, &pad->button, &pad->stick_x, &pad->stick_y);
                        } else {
                            D_80370C75 = 0;
                            cont_process_stick(1, 0, &pad->button, &pad->stick_x, &pad->stick_y);
                        }
                        break;
                }
            }
        }
    } else {
        pad->button = 0;
        pad->stick_x = 0;
        pad->stick_y = 0;
    }
    if (pad->button & L_JPAD) {
        gHeldLeft = 1;
    } else {
        gHeldLeft = 0;
    }
    if (pad->button & R_JPAD) {
        gHeldRight = 1;
    } else {
        gHeldRight = 0;
    }
    if (pad->button & U_JPAD) {
        gHeldUp = 1;
    } else {
        gHeldUp = 0;
    }
    if (pad->button & D_JPAD) {
        gHeldDown = 1;
    } else {
        gHeldDown = 0;
    }
    if (pad->button & START_BUTTON) {
        gHeldStart = 1;
    } else {
        gHeldStart = 0;
    }
    if (pad->button & L_TRIG) {
        gHeldL = 1;
    } else {
        gHeldL = 0;
    }
    if (pad->button & R_TRIG) {
        gHeldR = 1;
    } else {
        gHeldR = 0;
    }
    if (pad->button & A_BUTTON) {
        gHeldA = 1;
    } else {
        gHeldA = 0;
    }
    if (pad->button & B_BUTTON) {
        gHeldB = 1;
    } else {
        gHeldB = 0;
    }
    if (pad->button & L_CBUTTONS) {
        gHeldCLeft = 1;
    } else {
        gHeldCLeft = 0;
    }
    if (pad->button & D_CBUTTONS) {
        gHeldCDown = 1;
    } else {
        gHeldCDown = 0;
    }
    if (pad->button & U_CBUTTONS) {
        gHeldCUp = 1;
    } else {
        gHeldCUp = 0;
    }
    if (pad->button & R_CBUTTONS) {
        gHeldCRight = 1;
    } else {
        gHeldCRight = 0;
    }
    if (pad->button & Z_TRIG) {
        gHeldZ = 1;
    } else {
        gHeldZ = 0;
    }
    gHeldBOrZ = gHeldB || gHeldZ;
    gPrevPadStickX = gPadStickX;
    gPrevPadStickY = gPadStickY;
    gPadStickX = pad->stick_x;
    gPadStickY = pad->stick_y;
}

/* Post-process a pad: digital stick (from the D-pad and A/B) unless arg1,
 * dead zone and clamp, then stick back to buttons */
void cont_process_stick(u8 arg0, u8 arg1, u16 *btn, s8 *x, s8 *y) {
    if (!arg0 && !gPaused) {
        *y = 0;
    }
    if (!arg1) {
        cont_buttons_to_stick(*btn, x, y);
    }
    cont_dead_zone(x, y);
    if (!arg1) {
        cont_stick_to_buttons(btn, *x, *y);
    }
}

/* Clear the controller state */
void cont_clear(void) {
    gPrevHeldCRight = 0;
    gPrevHeldCUp = 0;
    gPrevHeldCDown = 0;
    gPrevHeldCLeft = 0;
    gPadStickY = 0;
    gPadStickX = 0;
    gHeldBOrZ = 0;
    gHeldZ = 0;
    gHeldCRight = 0;
    gHeldCUp = 0;
    gHeldLeft = gHeldRight = gHeldUp = gHeldDown = gHeldStart = gHeldL = gHeldR = gHeldA =
        gHeldB = gHeldCLeft = gHeldCDown = 0;
    gContPrevButtons = 0;
    gContButtons = 0;
    D_80370BC0 = 0;
    gSwallowB = 0;
}

/* D-pad left/right and A/B as a full-scale stick */
void cont_buttons_to_stick(u16 btn, s8 *x, s8 *y) {
    if (btn & L_JPAD) {
        *x = -80;
    }
    if (btn & R_JPAD) {
        *x = 80;
    }
    if (btn & B_BUTTON) {
        *y = -80;
    }
    if (btn & A_BUTTON) {
        *y = 80;
    }
}

/* Snap the stick to -80/0/80 for the digital control methods */
void cont_snap_stick(s8 *x, s8 *y) {
    switch (gCurVehicle) {
        case 1:
        case 3:
        case 4:
        case 5:
        case 6:
            if (*x <= 50 && *x >= -50) {
                *x = 0;
            } else {
                if (*x > 50) {
                    *x = 80;
                }
                if (*x < -50) {
                    *x = -80;
                }
            }
            if (*y <= 50 && *y >= -50) {
                *y = 0;
            } else {
                if (*y > 50) {
                    *y = 80;
                }
                if (*y < -50) {
                    *y = -80;
                }
            }
            break;
    }
}

/* A pushed stick also presses the matching buttons */
void cont_stick_to_buttons(u16 *btn, s8 x, s8 y) {
    if (!(*btn & L_JPAD) && x < -50) {
        *btn |= L_JPAD;
    }
    if (!(*btn & R_JPAD) && x > 50) {
        *btn |= R_JPAD;
    }
    if (!(*btn & A_BUTTON) && y > 50) {
        *btn |= A_BUTTON;
    }
    if (!(*btn & B_BUTTON) && y < -50) {
        *btn |= B_BUTTON;
    }
}

/* Stick dead zone (|v| < 10) and clamp to +-80 */
void cont_dead_zone(s8 *x, s8 *y) {
    if (*x < 10 && *x > -10) {
        *x = 0;
    }
    if (*x > 80) {
        *x = 80;
    }
    if (*x < -80) {
        *x = -80;
    }
    if (*y < 10 && *y > -10) {
        *y = 0;
    }
    if (*y > 80) {
        *y = 80;
    }
    if (*y < -80) {
        *y = -80;
    }
}

/* Show the control method's option text */
void cont_show_method_option(void) {
    char *tbl[3] = { "SPEED ON 3D STICK?", "360 DEGREE MODE?", "AIRBORNE 360' MODE?" };
    char *str;
    s32 found;
    s32 i;
    static u32 masks[3] = { 0xEDBA, 0x10005, 0x200 }; /* control methods each option applies to */

    str = "CONTROL METHOD:";
    found = 0;
    i = 0;
    do {
        if (masks[i] & (1 << gCurVehicle)) {
            found = 1;
        } else {
            i++;
        }
    } while (i < 3 && !found);
    CONTROLLER_ASSERT(found, 515);
    gYoshiGameItems[421].text = str;
    gYoshiGameItems[421].jtext = NULL;
    gYoshiGameItems[422].text = tbl[i];
    gYoshiGameItems[422].unk6 = 0x13;
    if ((D_80364BE0[playerNumber].flags ^ 0x10205) & (1 << gCurVehicle)) {
        gYoshiWindows[88].unk18 = 0x1A8;
    } else {
        gYoshiWindows[88].unk18 = 0x1A7;
    }
    yoshi_demand(0x8058);
}
