#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* fade.c (named by its assert). Its .bss (0x8036C770) is defined here in
 * declaration order and placed by hd_code_bss.us.v11.ld; the shared %hi
 * on the u64 store needs postFadeLoop_done defined in this file. */
u16 D_8036C770;
f32 D_8036C774;            /* fade step per frame */
u64 postFadeLoop_done;     /* level flags to switch to when the fade ends */
u32 D_8036C780;            /* frame the fade started */
u8 D_8036C784;             /* fade alpha */

#define FADE_ASSERT(EX, line) \
    if (!(EX)) debug_printf("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "fade.c", line)

extern f32 D_802FA930;     /* fade-in step per frame */


/* Runs the screen fade each frame and draws it as a full-screen black rectangle; on a mode's first frame some
 * modes start black and hold back the yoshi window demand. arg0 is not used (0x80274BF0). */
Gfx *fade_draw(s32 arg0, Gfx *gfx) {
    Gfx *gdl = gfx;

    if (!((s32) gFramesInMode)) {
        if ((gGameMode & (MODE_NAME_ENTRY | MODE_PAK_FILES | MODE_ACADEMY_MESSAGE | MODE_ATTRACT_PICTURE | MODE_REGRADE_NOTICE | MODE_UNUSED_52 | MODE_ERASE_SAVE | MODE_QUIT_GAME)) || ((gGameMode & (MODE_INTRO_APPROACH | MODE_INTRO_FLY | MODE_INTRO_LANDING)) && levelno == 50)) {
            D_8036C784 = 0xFF;
            if (gGameMode & (MODE_NAME_ENTRY | MODE_PAK_FILES | MODE_ACADEMY_MESSAGE | MODE_ATTRACT_PICTURE | MODE_UNUSED_52 | MODE_ERASE_SAVE)) {
                D_8036C770 = yoshi_get_demand();
                yoshi_demand(0);
            }
            D_8036C780 = D_803156C4;
        } else {
            D_8036C784 = 0;
        }
    }
    if (postFadeLoop_done) {
        D_8036C784 = (255.0f < (D_803156C4 - D_8036C780) * D_8036C774) ? 255.0f
                                                                      : (D_803156C4 - D_8036C780) * D_8036C774;
        if (D_8036C784 == 0xFF) {
            gNextGameMode = postFadeLoop_done;
            postFadeLoop_done = 0;
        }
    } else if (D_8036C784) {
        D_8036C784 = (0.0f > 255.0f - (D_803156C4 - D_8036C780) * D_802FA930)
                         ? 0.0f
                         : 255.0f - (D_803156C4 - D_8036C780) * D_802FA930;
        if (!D_8036C784) {
            yoshi_demand(D_8036C770);
            D_8036C770 = 0;
        }
    }
    if (D_8036C784) {
        gDPPipeSync(gdl++);
        gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
        gSPTexture(gdl++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        gDPSetPrimColor(gdl++, 0, 0, 0, 0, 0, D_8036C784);
        gDPSetCycleType(gdl++, G_CYC_1CYCLE);
        gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPFillRectangle(gdl++, 0, 0, 319, 239);
        osWritebackDCache(D_802FA8B0[gBufferIndex], sizeof(D_802FA8B0[0]));
    }
    return gdl;
}

/* Starts a fade to black; when it ends the game switches to the mode bits in next. The fade step is 4.25 /
 * speed; the music fades out too, except for some target modes (0x80275270).
 * Starts a fade; when it ends, the game switches to the level flags in next */
void fade_to_mode_speed(u64 next, f32 speed) {
    FADE_ASSERT(!postFadeLoop_done, 100);
    if (!postFadeLoop_done) {
        postFadeLoop_done = next;
        D_8036C774 = 4.25 / speed;
        D_8036C780 = D_803156C4;
        if (!(next & 0x40000000080004C2) && !(gGameMode & (MODE_NAME_ENTRY | MODE_QUIT_GAME))) {
            music_fade_to(0.0f);
        }
    }
}

/* Starts a fade to next at the default speed (0.25) (0x80275390). */
void fade_to_mode(u64 next) {
    fade_to_mode_speed(next, 0.25f);
}

s32 areWeFading(void) {
    return postFadeLoop_done ? 1 : 0;
}

/* Returns 1 while fade_draw holds back a yoshi window demand (from a fade-in's start until it ends)
 * (0x802753F8). */
s32 fade_window_held(void) {
    return D_8036C770 ? 1 : 0;
}
