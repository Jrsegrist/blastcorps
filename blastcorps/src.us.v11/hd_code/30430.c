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
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "fade.c", line)

extern u32 D_803156C4;
extern u64 D_80364A90;
extern u64 D_80364A98;
extern s32 D_80358060;
extern s32 D_802E8BDC;
extern f32 D_802FA930;     /* fade-in step per frame */
extern u8 D_8035805C;
extern Vtx D_802FA8B0[][4];


Gfx *func_80274BF0(s32 arg0, Gfx *gfx) {
    Gfx *gdl = gfx;

    if (!D_80358060) {
        if ((D_80364A90 & 0x4055800100040000) || ((D_80364A90 & 0x1801) && D_802E8BDC == 50)) {
            D_8036C784 = 0xFF;
            if (D_80364A90 & 0x0051800100040000) {
                D_8036C770 = func_8026B10C();
                func_8026AF6C(0);
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
            D_80364A98 = postFadeLoop_done;
            postFadeLoop_done = 0;
        }
    } else if (D_8036C784) {
        D_8036C784 = (0.0f > 255.0f - (D_803156C4 - D_8036C780) * D_802FA930)
                         ? 0.0f
                         : 255.0f - (D_803156C4 - D_8036C780) * D_802FA930;
        if (!D_8036C784) {
            func_8026AF6C(D_8036C770);
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
        osWritebackDCache(D_802FA8B0[D_8035805C], sizeof(D_802FA8B0[0]));
    }
    return gdl;
}

/* Starts a fade; when it ends, the game switches to the level flags in next */
void func_80275270(u64 next, f32 speed) {
    FADE_ASSERT(!postFadeLoop_done, 100);
    if (!postFadeLoop_done) {
        postFadeLoop_done = next;
        D_8036C774 = 4.25 / speed;
        D_8036C780 = D_803156C4;
        if (!(next & 0x40000000080004C2) && !(D_80364A90 & 0x4000000000040000)) {
            func_80261570(0.0f);
        }
    }
}

void func_80275390(u64 next) {
    func_80275270(next, 0.25f);
}

s32 func_802753C0(void) {
    return postFadeLoop_done ? 1 : 0;
}

s32 func_802753F8(void) {
    return D_8036C770 ? 1 : 0;
}
