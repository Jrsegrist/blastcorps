#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* Analog-stick steering helpers: stick angle/magnitude to steering and
 * direction-button bits. */

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define SIGN(x) ((x) >= 0 ? 1 : -1)

f32 sqrtf(f32);

extern s8 gSteerReverse;
extern s16 gSteerTurnTarget;

u16 cont_steer_toward(s16 arg0, s16 arg1, s8 *arg2, u8 arg3);
f32 math_asin_ratio_deg(f32 arg0, f32 arg1);

/* Resets the analog steering state: the turn-around latch (gSteerTurnAround) and the heading snap (gSteerHeadingSnap)
 * (0x8028B720). */
void cont_steer_reset(void) {
    gSteerTurnAround = 0;
    gSteerHeadingSnap = 0;
}

/* Analog control method: the stick points where to go, relative to the camera; returns a left/right steer of
 * -80/0/+80 in *x and the stick's magnitude in *y, snapping the heading once within one frame's turn
 * (0x8028B734). */
void cont_stick_to_steer(s8 *arg0, s8 *arg1, u8 arg2) {
    s8 sp37;
    s8 sp36;
    s16 sp34;
    s16 sp32;
    s16 sp30;
    u16 sp2E;
    s16 sp2C;

    sp30 = -gSteerHeading * 16 + 0x7FF0;
    sp37 = *arg0;
    sp36 = *arg1;
    sp34 = math_angle_2d_deg(0, 0, sp37, -sp36) * 65536.0 / 360.0 + 32768.0 +
           (s16) ((360.0 - (gCamHeading + 180.0)) / 360.0 * 65536.0);
    sp32 = sqrtf(sp37 * sp37 + sp36 * sp36);
    *arg1 = sp32;
    gSteerReverse = 0;
    sp2E = 0;
    if (gSteerTurnAround == 1) {
        sp2C = gSteerTurnTarget - sp30;
        if (ABS(sp2C) <= ABS(gSteerRate * 16)) {
            gSteerTurnAround = 0;
        }
        sp2E = 0x200;
    } else {
        if (*arg1 >= 0xB) {
            sp2E = cont_steer_toward(sp30, sp34, &gSteerReverse, arg2);
        }
        if (gSteerReverse != 0 && gPlayerSpeed < 0x28) {
            gSteerTurnTarget = sp34;
            gSteerTurnAround = 1;
        } else if (gSteerHeadingSnap != 0) {
            gSteerSnapAngle = 0x7FF - sp34 / 16;
            *arg0 = 0;
            return;
        }
    }
    *arg0 = 0;
    if (sp2E == 0x200) {
        *arg0 = -0x50;
    }
    if (sp2E == 0x100) {
        *arg0 = 0x50;
    }
}

/* Returns R_JPAD or L_JPAD to turn the heading toward the target (sets the heading snap when close); if the
 * target is far behind and the speed is below 40, flips the reverse flag and returns 0. arg 4 is unused
 * (0x8028BA1C). */
u16 cont_steer_toward(s16 arg0, s16 arg1, s8 *arg2, u8 arg3) {
    s16 spE;
    u16 spC;
    s16 spA;
    u16 sp8;

    sp8 = 0x5A;
    spA = 0x78 - ABS(gLastSpeed) * (0x78 - sp8) / 200;
    spC = 0x8000 - ((spA < sp8 ? sp8 : spA) << 16) / 720;
    if (*arg2 != 0) {
        arg0 += 0x8000;
    }
    spE = arg1 - arg0;
    gContSteerError = spE;
    if (ABS(spE) <= ABS(gSteerRate * 16)) {
        gSteerHeadingSnap = 1;
    }
    if (spC < ABS(spE) && gPlayerSpeed < 0x28) {
        *arg2 ^= -1;
        return 0;
    }
    if (SIGN(gSteerRate) * spE > 0) {
        return 0x100;
    }
    return 0x200;
}

/* Angle in degrees of the vector from (x0, y0) to (x1, y1): -y is 0, +x 90, +y 180, -x 270 (0x8028BBF4).
 * Angle in degrees of the vector (arg2 - arg0, arg3 - arg1), per quadrant. */
f32 math_angle_2d_deg(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
#ifdef NON_MATCHING
    f32 sp1C = 0.0f; /* every quadrant test sets it; never read unset */
#else
    f32 sp1C;
#endif

    if (arg2 >= arg0 && arg3 >= arg1) {
        sp1C = math_asin_ratio_deg(arg3 - arg1, arg2 - arg0) + 90.0f;
    } else if (arg2 >= arg0 && arg3 < arg1) {
        sp1C = 90.0f - math_asin_ratio_deg(arg1 - arg3, arg2 - arg0);
    } else if (arg2 < arg0 && arg3 < arg1) {
        sp1C = math_asin_ratio_deg(arg1 - arg3, arg0 - arg2) + 270.0f;
    } else if (arg2 < arg0 && arg3 >= arg1) {
        sp1C = 270.0f - math_asin_ratio_deg(arg3 - arg1, arg0 - arg2);
    }
    return sp1C;
}

/* asin(a / sqrt(a*a + b*b)) in degrees through math_asin_c; 0 when both are 0 (0x8028BD88).
 * Angle in degrees from the ratio arg0 / |(arg0, arg1)|, through the
 * fixed-point inverse-trig lookup math_asin_c. */
f32 math_asin_ratio_deg(f32 arg0, f32 arg1) {
    s32 sp1C;
    f32 sp18;

    if (arg0 == 0.0 && arg1 == 0.0) {
        return 0.0f;
    }
    sp18 = arg0 / sqrtf(arg0 * arg0 + arg1 * arg1);
    sp1C = math_asin_c(sp18 * 65536.0);
    return ((f32) sp1C / 65536.0) * 360.0;
}

/* Unused: stick past +-50 presses D-pad left/right and A (y < -50) or B (y > 50); Z also presses L and R
 * (0x8028BE70).
 * Turn stick deflection past +-50 into direction-button bits. */
void cont_stick_to_buttons_inv_y(u16 *arg0, s8 arg1, s8 arg2) {
    if (!(*arg0 & 0x200) && (arg1 < -0x32)) {
        *arg0 |= 0x200;
    }
    if (!(*arg0 & 0x100) && (arg1 >= 0x33)) {
        *arg0 |= 0x100;
    }
    if (!(*arg0 & 0x8000) && (arg2 < -0x32)) {
        *arg0 |= 0x8000;
    }
    if (!(*arg0 & 0x4000) && (arg2 >= 0x33)) {
        *arg0 |= 0x4000;
    }
    if (*arg0 & 0x2000) {
        *arg0 |= 0x30;
    }
}

/* Unused analog-to-digital control: the stick's magnitude and its direction against the heading press D-pad
 * left/right (cont_steer_toward) and A, or B when reversing; Z also presses L and R (0x8028BF34). */
void cont_stick_dir_to_buttons(u16 *arg0, s8 arg1, s8 arg2, u8 arg3) {
    s16 sp2E;
    s16 sp2C;
    s16 sp2A;
    u16 sp28;

    sp2E = math_angle_2d_deg(0, 0, arg1, -arg2) * 65536.0 / 360.0 + 32768.0;
    sp2C = sqrtf(arg1 * arg1 + arg2 * arg2);
    sp28 = 0;
    sp2A = -gSteerHeading * 16 + 0x7FF0;
    sp2A -= 0x2000;
    if (sp2C >= 0x10) {
        sp28 = 1;
        if (arg3 == 0 || arg3 == 2 || arg3 == 0x10) {
            sp28 |= 2;
        }
    }
    if (sp2C >= 0x2E) {
        sp28 |= 2;
    }
    if (sp2C >= 0x4C && (gSteerReverse == 0 || arg3 != 3)) {
        sp28 |= 4;
    }
    if (sp28 == 0) {
        gSteerReverse = 0;
    }
    if (!(*arg0 & 0x300) && (sp28 & 1)) {
        *arg0 |= cont_steer_toward(sp2A, sp2E, &gSteerReverse, arg3);
    }
    if (!(*arg0 & 0xC000) && (sp28 & 2)) {
        if (gSteerReverse != 0) {
            *arg0 |= 0x8000;
        } else {
            *arg0 |= 0x4000;
        }
    }
    if (*arg0 & 0x2000) {
        *arg0 |= 0x30;
    }
}
