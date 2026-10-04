#include "common.h"
#include <ultra64.h>

/* fade.c (named by its assert). Its .bss (0x8036C770) is defined here in
 * declaration order and placed by hd_code_bss.us.v11.ld; the shared %hi
 * on the u64 store needs postFadeLoop_done defined in this file. */
u16 D_8036C770;
f32 D_8036C774;            /* fade step per frame */
u64 postFadeLoop_done;     /* level flags to switch to when the fade ends */
u32 D_8036C780;            /* frame the fade started */
u8 D_8036C784;

#define FADE_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "fade.c", line)

extern u32 D_803156C4;
extern u64 D_80364A90;

void func_8029A7E4(const char *fmt, ...);
void func_80261570(f32 arg0);

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/30430/func_80274BF0.s")

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
