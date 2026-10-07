#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802F5804 ((PathNode *) D_802F5804)
#define D_802F8BDC ((YoshiArg *) D_802F8BDC)
/* end of views */

/* controller.c: controller init, reading, the demo/replay pad source and
 * per-button edge flags. */

#define CONTROLLER_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "controller.c", line)

/* Pathfinding node, 0x1C bytes (see 26570.c) */
typedef struct {
    /* 0x00 */ u16 flags;
    /* 0x02 */ u8 unk2[4];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ char * N64P text;
    /* 0x10 */ u16 * N64P jtext;
    /* 0x14 */ u8 unk14[8];
} PathNode;

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

extern OSContPad D_80370BD8; /* this frame's pad */
extern OSMesg D_80370BF0;
extern OSContStatus D_80370BC8;
extern u8 D_80370C10; /* a read is in flight */
extern Player D_80364BE0[];

/* stick x/y: this frame and last frame */
/* one flag per button, set while held */
extern u8 D_80370C17; /* U_JPAD */
extern u8 D_80370C18; /* D_JPAD */
extern u8 D_80370C19; /* START */
extern u8 D_80370C1F; /* D_CBUTTONS */
extern u8 D_80370C20; /* U_CBUTTONS */
/* last frame's C buttons */
extern u8 D_80370C25;
extern u8 D_80370C26;
extern s8 D_80370C2F;
/* the alternative pad source */
extern s32 D_80370BC0;
extern s32 D_80370C40;

void func_8028ADF0(u8 arg0, u8 arg1, u16 *btn, s8 *x, s8 *y);
void func_8028AFA4(u16 btn, s8 *x, s8 *y);
void func_8028B0E8(u16 *btn, s8 x, s8 y);
void func_8028B190(s8 *x, s8 *y);

s32 D_802FDB10 = 0;
u8 D_802FDB14 = 0;

/* Set up the SI queue and the controllers; returns the controller bit pattern */
u8 func_8028A370(void) {
    s32 pad;
    u8 pattern;

    osCreateMesgQueue(&D_80370BF8, &D_80370BF0, 1);
    osSetEventMesg(OS_EVENT_SI, &D_80370BF8, 0);
    func_802DB0A0(&D_80370BF8, &pattern, &D_80370BC8);
    osContSetCh(1);
    D_80370C10 = 0;
    D_80370C35 = 0;
    return pattern;
}

/* Start a controller read */
void func_8028A3E4(void) {
    if (D_8039C4B0 == 0) {
        func_8028A42C();
        func_802DB4D0(&D_80370BF8);
        D_80370C10 = 1;
    }
}

/* Wait for an in-flight controller read */
void func_8028A42C(void) {
    if (D_80370C10) {
        osRecvMesg(&D_80370BF8, NULL, 1);
        D_80370C10 = 0;
    }
}

/* Read this frame's pad and update the button/stick state */
void func_8028A470(void) {
    OSContPad *pad = &D_80370BD8;
    s32 unused;
    s32 mode;

    if (((u32) D_80358064) != 0) {
        if (D_8039C4B0 == 0 && D_80370C10 != 0) {
            osRecvMesg(&D_80370BF8, NULL, 1);
            func_802DB594(pad);
            if (pad->errno) {
                func_8029A7E4("pad read error - zeroing data\n");
                pad->stick_x = pad->stick_y = pad->button = 0;
            }
            D_80370C10 = 0;
        }
        D_80370C2A = D_80370C28;
        D_80370C28 = pad->button;
        if (D_80370C38) {
            if (pad->button & B_BUTTON) {
                pad->button &= ~B_BUTTON;
            } else {
                D_80370C38 = 0;
            }
        }
        if (D_80358060 < 6) {
            if (D_80358060 == 5) {
                D_80370C2A |= A_BUTTON | B_BUTTON | START_BUTTON;
            } else {
                D_80370C28 &= ~(A_BUTTON | B_BUTTON | START_BUTTON);
            }
        }
        D_80370C13 = D_80370C11;
        D_80370C14 = D_80370C12;
        D_80370C11 = pad->stick_x;
        D_80370C12 = pad->stick_y;
        D_80370C24 = D_80370C1E;
        D_80370C25 = D_80370C1F;
        D_80370C26 = D_80370C20;
        D_80370C27 = D_80370C21;
        switch (D_80364A90) {
            case 2:
            case 0x40:
            case 0x400:
            case 0x100000000000:
                if (D_802E8BDC != 0x26 && D_802E8BDC != 0x31) {
                    do {
                        func_8025BEF8();
                        if (D_80370C30 & 0x40) {
                            D_80370C40 = 0;
                        } else if (D_80370C30 & 0x80) {
                            D_80370C40 = 1;
                        }
                    } while (D_80370C30 & 0xC0);
                }
                pad->button = D_80370C30;
                pad->stick_x = D_80370C32;
                pad->stick_y = D_80370C33;
                pad->button &= ~U_CBUTTONS;
                break;
            case 4:
            case 0x100:
                if (!D_802E8BD0 || D_80364A90 == 0x2000) {
                    func_8025BBE8(pad->button & ~START_BUTTON, pad->stick_x, pad->stick_y);
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
        if (D_80364A90 & 0x2000100000002546) {
            D_803ED40A = 0;
            D_803F7C34 = 0;
            if (D_80364A90 & 0x440) {
                mode = D_80370C40;
            } else if (D_80364BE0[D_80364AE8].flags & (1 << D_80364456)) {
                mode = 1;
            } else {
                mode = 0;
            }
            if (D_80364A90 == 2 || D_80364A90 == 0x100000000000) {
                mode = 0;
            }
            D_80370C35 = mode;
            if (D_802E8BD0) {
                func_8028ADF0(1, 0, &pad->button, &pad->stick_x, &pad->stick_y);
            } else {
                switch (mode) {
                    case 0:
                        switch (D_80364456) {
                            case 9:
                                if (D_803F7C3F) {
                                    D_80370C34 = 1;
                                }
                            case 0:
                            case 2:
                            case 16:
                                func_8028B734(&pad->stick_x, &pad->stick_y, D_80364456);
                                func_8028B190(&pad->stick_x, &pad->stick_y);
                                break;
                            default:
                                D_80370C75 = 0;
                                func_8028ADF0(0, 0, &pad->button, &pad->stick_x, &pad->stick_y);
                                break;
                        }
                        break;
                    case 1:
                        if (D_80364456 == 9) {
                            D_80370C75 = 0;
                            func_8028ADF0(1, 1, &pad->button, &pad->stick_x, &pad->stick_y);
                        } else {
                            D_80370C75 = 0;
                            func_8028ADF0(1, 0, &pad->button, &pad->stick_x, &pad->stick_y);
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
        D_80370C15 = 1;
    } else {
        D_80370C15 = 0;
    }
    if (pad->button & R_JPAD) {
        D_80370C16 = 1;
    } else {
        D_80370C16 = 0;
    }
    if (pad->button & U_JPAD) {
        D_80370C17 = 1;
    } else {
        D_80370C17 = 0;
    }
    if (pad->button & D_JPAD) {
        D_80370C18 = 1;
    } else {
        D_80370C18 = 0;
    }
    if (pad->button & START_BUTTON) {
        D_80370C19 = 1;
    } else {
        D_80370C19 = 0;
    }
    if (pad->button & L_TRIG) {
        D_80370C1A = 1;
    } else {
        D_80370C1A = 0;
    }
    if (pad->button & R_TRIG) {
        D_80370C1B = 1;
    } else {
        D_80370C1B = 0;
    }
    if (pad->button & A_BUTTON) {
        D_80370C1C = 1;
    } else {
        D_80370C1C = 0;
    }
    if (pad->button & B_BUTTON) {
        D_80370C1D = 1;
    } else {
        D_80370C1D = 0;
    }
    if (pad->button & L_CBUTTONS) {
        D_80370C1E = 1;
    } else {
        D_80370C1E = 0;
    }
    if (pad->button & D_CBUTTONS) {
        D_80370C1F = 1;
    } else {
        D_80370C1F = 0;
    }
    if (pad->button & U_CBUTTONS) {
        D_80370C20 = 1;
    } else {
        D_80370C20 = 0;
    }
    if (pad->button & R_CBUTTONS) {
        D_80370C21 = 1;
    } else {
        D_80370C21 = 0;
    }
    if (pad->button & Z_TRIG) {
        D_80370C22 = 1;
    } else {
        D_80370C22 = 0;
    }
    D_80370C23 = D_80370C1D || D_80370C22;
    D_80370C2E = D_80370C2C;
    D_80370C2F = D_80370C2D;
    D_80370C2C = pad->stick_x;
    D_80370C2D = pad->stick_y;
}

/* Post-process a pad: digital stick (from the D-pad and A/B) unless arg1,
 * dead zone and clamp, then stick back to buttons */
void func_8028ADF0(u8 arg0, u8 arg1, u16 *btn, s8 *x, s8 *y) {
    if (!arg0 && !D_802E8BD0) {
        *y = 0;
    }
    if (!arg1) {
        func_8028AFA4(*btn, x, y);
    }
    func_8028B190(x, y);
    if (!arg1) {
        func_8028B0E8(btn, *x, *y);
    }
}

/* Clear the controller state */
void func_8028AE88(void) {
    D_80370C27 = 0;
    D_80370C26 = 0;
    D_80370C25 = 0;
    D_80370C24 = 0;
    D_80370C2D = 0;
    D_80370C2C = 0;
    D_80370C23 = 0;
    D_80370C22 = 0;
    D_80370C21 = 0;
    D_80370C20 = 0;
    D_80370C15 = D_80370C16 = D_80370C17 = D_80370C18 = D_80370C19 = D_80370C1A = D_80370C1B = D_80370C1C =
        D_80370C1D = D_80370C1E = D_80370C1F = 0;
    D_80370C2A = 0;
    D_80370C28 = 0;
    D_80370BC0 = 0;
    D_80370C38 = 0;
}

/* D-pad left/right and A/B as a full-scale stick */
void func_8028AFA4(u16 btn, s8 *x, s8 *y) {
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
void func_8028B000(s8 *x, s8 *y) {
    switch (D_80364456) {
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
void func_8028B0E8(u16 *btn, s8 x, s8 y) {
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
void func_8028B190(s8 *x, s8 *y) {
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
void func_8028B240(void) {
    char *tbl[3] = { "SPEED ON 3D STICK?", "360 DEGREE MODE?", "AIRBORNE 360' MODE?" };
    char *str;
    s32 found;
    s32 i;
    static u32 masks[3] = { 0xEDBA, 0x10005, 0x200 }; /* control methods each option applies to */

    str = "CONTROL METHOD:";
    found = 0;
    i = 0;
    do {
        if (masks[i] & (1 << D_80364456)) {
            found = 1;
        } else {
            i++;
        }
    } while (i < 3 && !found);
    CONTROLLER_ASSERT(found, 515);
    D_802F5804[421].text = str;
    D_802F5804[421].jtext = NULL;
    D_802F5804[422].text = tbl[i];
    D_802F5804[422].unk6 = 0x13;
    if ((D_80364BE0[D_80364AE8].flags ^ 0x10205) & (1 << D_80364456)) {
        D_802F8BDC[88].unk18 = 0x1A8;
    } else {
        D_802F8BDC[88].unk18 = 0x1A7;
    }
    func_8026AF6C(0x8058);
}
