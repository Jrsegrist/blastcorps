#include "common.h"
#include <ultra64.h>

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6B70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6C1C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6C8C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6DB0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6E3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6EB0.s")

typedef struct SndLink {
    struct SndLink *next;
    struct SndLink *prev;
} SndLink;

/* libaudio alLink: insert ln after to. */
void func_802D6EE0(SndLink *ln, SndLink *to) {
    ln->next = to->next;
    ln->prev = to;
    if (to->next) {
        to->next->prev = ln;
    }
    to->next = ln;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F04.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6F70.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D6FC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/923B0/func_802D70A8.s")
