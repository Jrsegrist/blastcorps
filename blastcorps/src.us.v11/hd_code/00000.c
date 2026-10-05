#include "common.h"
#include <ultra64.h>

/* hd.c (from its assert strings): boot, the main game thread and the
 * frame loop. Its strings still live in the hd_code data bin, so they
 * are referenced through externs for now. */

void func_8029A7E4(char *, ...); /* debug printf */
void func_802D4020(void);
void func_80270AE0(u32 *);
void func_802D4550(s32);
void func_802D4560(s32, void *, void *, s32);
void func_80244870(void *);
void func_80244930(void *);

extern OSThread D_80310820;
extern u64 D_803109D0[]; /* idle thread stack */
extern OSThread D_80310BD0;
extern u64 D_80310D80[]; /* main thread stack, filled with a guard pattern */
extern u8 D_80314D80[];
extern u8 D_80314D98[];
extern s32 D_802FA254;
extern u32 D_803649F0;
extern u32 D_803649F4;

typedef struct {
    u8 pad0[0x5C];
    s32 unk5C;
    u8 pad60[0x14];
} Unk74;
extern Unk74 D_80364460[];
extern Unk74 *D_803649D0;

/* one per frame buffer, 0x21498 bytes */
typedef struct {
    u8 pad0[0x48B0];
    Gfx dl[0xB5E]; /* TOPLEVEL_DL_SIZE */
    u8 padA3A0[0x21498 - 0xA3A0];
} DynamicBuf;
extern DynamicBuf D_803156F8[];
extern u8 D_8035805C;

extern char D_80308274[]; /* "\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n" */
extern char D_803082A0[]; /* "*length<TOPLEVEL_DL_SIZE" */
extern char D_803082BC[]; /* "hd.c" */
extern char D_803082D4[]; /* "stack end =%d\n" */

/* Boot: reads 16 words from PI address 0xFFB000, then starts the idle thread */
void func_802447C0(void) {
    u32 i;
    s32 pad[2];
    u32 addr;
    u32 buf[16];
    s32 pad2[2];

    func_802D4020();
    for (addr = 0xFFB000, i = 0; i < 16; i++, addr += 4) {
        osPiRawReadIo(addr, &buf[i]);
    }
    func_80270AE0(buf);
    osCreateThread(&D_80310820, 1, func_80244870, NULL, D_803109D0 + 0x40, 10);
    osStartThread(&D_80310820);
}

/* Idle thread: starts the managers and the main thread, then spins */
void func_80244870(void *arg0) {
    s32 pad;

    func_802D4550(4);
    func_802D4560(0x96, D_80314D80, D_80314D98, 0xC2);
    osCreateThread(&D_80310BD0, 3, func_80244930, arg0, D_80310D80 + 0x400, 10);
    osStartThread(&D_80310BD0);
    if (D_802FA254 == 0) {
        osStartThread(&D_80310BD0);
    }
    osSetThreadPri(NULL, 0);
    while (1) {
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80244930.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802475D8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024A348.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024A92C.s")

/* Eases D_803649F4 towards D_803649F0 by a tenth of the gap plus 10 */
void func_8024ADD8(void) {
    u32 d;

    d = D_803649F0 - D_803649F4;
    D_803649F4 = d / 10 + D_803649F4;
    D_803649F4 += 10;
    if (D_803649F0 < D_803649F4) {
        D_803649F4 = D_803649F0;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024AE2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024AFA8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024B188.s")

/* Is any entry of D_80364460 (up to D_803649D0) tagged with id? */
s32 func_8024B418(u8 id) {
    s32 i;

    i = 0;
    while (&D_80364460[i] != D_803649D0) {
        if (D_80364460[i].unk5C == id) {
            return 1;
        }
        i++;
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024B4B8.s")

void func_802AB670(s32 arg0);
extern u8 D_803ED3F5;
extern u8 D_80364456;

void func_8024B5E8(void) {
    D_803ED3F5 = 1;
    func_802AB670(D_80364456);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024B618.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024B7AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024B8F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024BDA4.s")

void *func_8024C404(void *arg0, s32 arg1, s32 *arg2) {
    *arg2 = 0;
    return arg0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024C414.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024E4F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024F520.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8024FC2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802502EC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802507C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80254E54.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80255034.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80255190.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80255628.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_802558C8.s")

/* Closes the top-level display list and returns its length in *length */
void func_802559F8(Gfx *arg0, s32 *length) {
    Gfx *gdl = arg0;

    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    *length = gdl - D_803156F8[D_8035805C].dl;
    if (!(*length < 0xB5E)) {
        func_8029A7E4(D_80308274, D_803082A0, D_803082BC, 3665);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80255AD0.s")

/* Checks the main thread's stack guard pattern from the top down */
void func_80255D34(void) {
    u8 bad;
    s32 i;

    bad = 0;
    for (i = 0x1FF; i >= 0 && !bad; i--) {
        if (D_80310D80[i] != 0x1122334455667788LL) {
            bad = 1;
            func_8029A7E4(D_803082D4, i);
        }
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80255DC8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_8025615C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80256A34.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/00000/func_80257234.s")

/* Rounds *arg0 up to a multiple of arg1 */
void func_80257490(s32 *arg0, s32 arg1) {
    s32 r;

    r = *arg0 % arg1;
    if (r != 0) {
        r = arg1 - r;
    }
    *arg0 += r;
}


void func_802574F0(f32 arg0) {
    sinf(arg0);
}


void func_80257514(f32 arg0) {
    cosf(arg0);
}
