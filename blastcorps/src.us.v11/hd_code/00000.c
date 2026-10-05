#include "common.h"
#include <ultra64.h>

/* hd.c (from its assert strings): boot, the main game thread and the
 * frame loop. Owns .rodata at 0x80307B90.
 *
 * Variables defined here (the tell is a shared `lui` on paired stores).
 * Only part of hd.c's .bss is modelled so far: the section is placed at
 * 0x80310D80 by hd_code_bss.us.v11.ld, and the later variables get their
 * real addresses from undefined_syms (absolute symbols win). */
u64 D_80310D80[0x400]; /* main thread stack, filled with a guard pattern */
u64 D_80364A90;
u64 D_80364A98; /* game mode flags */

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
extern u8 D_8035805C; /* current frame buffer index */

extern u8 D_00787F40[]; /* static code segment bounds (ROM) */
extern u8 D_00788000[];
extern u8 D_80364A70;
extern s32 D_80358068;
extern s32 D_80358064;
extern u32 D_80358060;
extern s16 D_80367BD6;
extern u8 D_803FF600[];
extern u8 *D_8035806C;
extern s32 D_80358078;
extern s32 D_80358070; /* heap pointer */
extern s32 D_802E8BDC; /* current level */
extern s32 D_8036E694;
typedef struct {
    u8 unk0;
    u8 pad1[0x43];
} LevelInfo;
extern LevelInfo D_802E8F94[];
extern u8 D_803B9888;
extern u8 D_80358088[];
extern s32 D_803643C8;
extern u8 D_803643D9;
extern u8 D_803643DA;
extern u8 D_803643D8;
extern u8 D_803643D6;
extern u8 D_803643D7;
extern u8 D_802E8BD8;
extern u8 D_802E8BD4;
extern u8 D_802E8BD0;
extern u8 D_8036EB99;
extern s32 D_803669B4;
extern u8 D_8039CAA2;

u8 func_80261A44(u64);
void func_802D6710(void);
void func_8026A8BC(void);
void func_8026A974(void);
void func_8028AE88(void);
void func_8028B720(void);
void func_8028B4C4(void *, void *, u32 *, s32, s32, s32);
void func_802558C8(Gfx *, s32 *);
void func_802559F8(Gfx *, s32 *);
void func_80257490(s32 *, s32);
void func_802A0700(void);
void func_80278E3C(void);
void func_8028B3E0(void);
void func_80297530(s32);
void func_80272C50(void);
void func_801F7850(void);
void func_8026B118(s32);
void func_8028A42C(void);
void func_802592F0(void);

#define TOPLEVEL_DL_SIZE 0xB5E

/* Rare's assert; line numbers are the original hd.c's */
#define HD_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "hd.c", line)

extern s32 D_80000300; /* osTvType */
extern u32 D_80358050[]; /* frame buffer physical addresses */
typedef struct {
    u8 pad0[0x10];
    u32 unk10;
    u8 pad14[0xEC];
} Player;
extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern u8 D_803649ED;
extern u8 D_80364456;
extern s32 D_803649E8;
extern u8 D_803649EC;
extern u8 D_803649EE;
extern s32 D_803156F0;
extern s32 D_80367738;
s32 func_802AB878(u8);
void func_802AB478(u8);
void func_8028F6B4(u8);
void func_80291ED8(u8);
void func_802794E4(void);
s32 func_8024AFA8(u8);
void func_802AE860(void);
void func_8026AD30(s32);
s32 func_80260634(s32);
void func_80260650(s32, s32, s32 *);

extern OSMesgQueue D_803150A0;
extern OSMesg D_803150B8[];
extern OSMesgQueue D_80315180;
extern OSMesg D_80315198[];
extern u64 D_80312D80[];
extern u8 D_80315440[];
extern OSMesgQueue D_803153D8;
extern OSMesg D_803153F8[];
extern u8 D_803156D8[];
extern u16 D_80000400[][320 * 240];
extern u8 D_8021ED00[];
extern u32 D_80358058;
extern u8 D_802FDBD0;
extern u8 D_802FDBD4;
void func_80270D20(void *, void *, s32, s32, s32);
void func_80270E50(void *, void *, OSMesgQueue *, s32, s32);
u8 func_8028A370(void);
void func_80261588(void);
void func_80284DB0(void);
void func_8028FC10(void);
extern s32 D_803643E0;
extern s32 D_803643E8;
extern f32 D_80364418;
extern f32 D_80364414;
extern u8 D_8036441C;
extern u8 D_8036441D;
s32 func_802AC4C4(s32, s32, s32, s32, s32, s32, s32, s32);
#define PHYS(x) ((u32)(x) & 0x1FFFFFFF)

extern u8 D_80370C1E;
extern u8 D_80370C21;
extern u8 D_80370C24;
extern u8 D_80370C27;
u8 func_80255628(void);
void func_802A45D4(s32);

extern u16 D_8035807C;
extern s32 D_80358074;
extern Gfx D_01000010[]; /* segment 1 */
void func_802A467C(s32, Gfx *, Vtx *, s32);

/* (end of declarations) */

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

/* Applies a pending vehicle switch (D_803649ED) */
void func_8024AE2C(void) {
    s32 pad;

    if (D_803649ED != 0 && D_803649ED != 0xFF) {
        if (func_802AB878(D_803649ED) == 0) {
            func_802AB478(D_803649ED);
            func_8028F6B4(D_803649ED);
            func_80291ED8(D_803649ED);
            func_8028B720();
            func_802794E4();
            if (func_8024AFA8(D_803649ED)) {
                D_80364456 = D_803649ED;
                func_802AE860();
                if (!D_802E8BD0 && D_80358060 >= 11 &&
                    (D_80364456 == 8 || D_80364456 == 15 || D_80364456 == 13 || D_80364456 == 14)) {
                    func_8026AD30(0x4A);
                }
            }
            D_80364AF0[D_80364AE8].unk10 |= 1 << D_80364456;
            D_803649E8 = 1;
            D_803649EC = 1;
            D_803649EE = 1;
        } else if (!func_80260634(D_803156F0)) {
            func_80260650(D_80367738, 0x2B, &D_803156F0);
        }
    }
}

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

/* Builds and runs a small task drawing an 8-vertex box (12 triangles) */
void func_8024B8F4(Mtx *proj, Mtx *view) {
    Gfx dl[50];
    Gfx *gdl = dl;
    Vtx v[8];
    s32 i;

    for (i = 0; i < 8; i++) {
        v[i].v.flag = 0;
        v[i].v.tc[0] = 0;
        v[i].v.tc[1] = 0;
        v[i].v.cn[0] = 0;
        v[i].v.cn[1] = 0;
        v[i].v.cn[2] = 0;
        v[i].v.cn[3] = 0;
    }
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000010);
    gSPMatrix(gdl++, PHYS(proj), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, D_8035807C);
    gSPMatrix(gdl++, PHYS(view), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPVertex(gdl++, PHYS(v), 8, 0);
    gSP1Triangle(gdl++, 0, 1, 4, 0);
    gSP1Triangle(gdl++, 1, 4, 5, 0);
    gSP1Triangle(gdl++, 0, 3, 4, 0);
    gSP1Triangle(gdl++, 3, 4, 7, 0);
    gSP1Triangle(gdl++, 2, 3, 7, 0);
    gSP1Triangle(gdl++, 2, 6, 7, 0);
    gSP1Triangle(gdl++, 1, 2, 5, 0);
    gSP1Triangle(gdl++, 2, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 6, 7, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    gDPTileSync(gdl++);
    gSPEndDisplayList(gdl++);
    osWritebackDCache(dl, (s32) (gdl - dl) * sizeof(Gfx));
    osWritebackDCache(proj, 0x40);
    osWritebackDCache(view, 0x40);
    func_802A467C(D_80358074, dl, v, (s32) (gdl - dl) * sizeof(Gfx));
}

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

/* Polar to cartesian: point at angle (degrees) on a circle of radius r*sqrt(2) */
void func_80255034(s32 r, f32 angle, s32 *outX, s32 *outY) {
    f32 len;
    f32 r2;
    s32 x;
    s32 y;
    f32 a;

    r2 = r * r;
    len = sqrtf(r2 + r2);
    a = angle;
    a = a / 360.0;
    a = a * 6.28318;
    x = sinf(a) * len;
    y = sqrtf(len * len - x * x);
    if (angle >= 90.0 && angle < 270.0) {
        y = 0.0 - y;
    }
    if (y >= -100 && y <= 100) {
        y = 0;
    }
    *outX = x;
    *outY = y;
}

/* Map screen: Z/R rotate the map 45 degrees at 3 degrees a frame */
void func_80255190(void) {
    u8 ok;

    if (D_802E8BD0 == 0) {
        ok = func_80255628();
        if (ok && ((D_80370C21 && !D_80370C27) || (D_80370C1E && !D_80370C24)) && (D_80364A90 & 0x104)) {
            func_80260650(D_80367738, 0xD0, NULL);
        }
        if (D_80370C21 && !D_80370C27 && !D_8036441C && !D_8036441D && !ok && D_80364A90 != 0x2000) {
            D_80364418 = D_80364414 + 45.0;
            if (D_80364418 >= 360.0) {
                D_80364418 = D_80364418 - 360.0;
            }
            D_8036441C = 1;
            if (D_80364A90 != 0x40) {
                func_80260650(D_80367738, 0xDD, NULL);
            }
        }
        if (D_8036441C) {
            if (D_80364414 < D_80364418) {
                D_80364414 = D_80364414 + 3.0;
                if (D_80364414 >= D_80364418) {
                    D_8036441C = 0;
                    D_80364414 = D_80364418;
                }
            } else {
                D_80364414 = D_80364414 + 3.0;
                if (D_80364414 >= 360.0) {
                    D_80364414 = D_80364414 - 360.0;
                    if (D_80364414 >= D_80364418) {
                        D_8036441C = 0;
                        D_80364414 = D_80364418;
                    }
                }
            }
        }
        if (D_80370C1E && !D_80370C24 && !D_8036441C && !D_8036441D && !ok && D_80364A90 != 0x2000) {
            D_80364418 = D_80364414 - 45.0;
            if (D_80364418 < 0.0) {
                D_80364418 = D_80364418 + 360.0;
            }
            D_8036441D = 1;
            if (D_80364A90 != 0x40) {
                func_80260650(D_80367738, 0xDC, NULL);
            }
        }
        if (D_8036441D) {
            if (D_80364414 > D_80364418) {
                D_80364414 = D_80364414 - 3.0;
                if (D_80364414 <= D_80364418) {
                    D_8036441D = 0;
                    D_80364414 = D_80364418;
                }
            } else {
                D_80364414 = D_80364414 - 3.0;
                if (D_80364414 < 0.0) {
                    D_80364414 = D_80364414 + 360.0;
                    if (D_80364414 <= D_80364418) {
                        D_8036441D = 0;
                        D_80364414 = D_80364418;
                    }
                }
            }
        }
        if (D_8036441D || D_8036441C) {
            func_802A45D4(2);
        }
    }
}

u8 func_80255628(void) {
    u8 ok = 0;

    if (D_80364A90 == 0x100000000000 || D_80364A90 == 2) {
        ok = 1;
    } else {
        switch (D_802E8BDC) {
            case 4:
                if (func_802AC4C4(D_803643E0 >> 5, D_803643E8 >> 5, 0xE56, 0x8EC, 0xBB8, 0x6A4, 0x1068, 0x1F4) ||
                    func_802AC4C4(D_803643E0 >> 5, D_803643E8 >> 5, 0xE56, 0x8EC, 0x1068, 0x1F4, 0x1324, 0x4B0)) {
                    ok = 1;
                }
                break;
            case 16:
                if (D_803643E0 > 0x46500 && D_803643E8 > 0x3E800) {
                    ok = 1;
                }
                break;
            case 13:
                if (D_803643E0 > 0x42680 && D_803643E8 > 0x46500) {
                    ok = 1;
                }
                break;
            case 0x3B:
                ok = 1;
                break;
        }
    }
    if (ok) {
        if (D_80364418 < 134.0 || D_80364418 > 136.0) {
            D_8036441D = 0;
            D_8036441C = 0;
            D_80364418 = 135.0f;
            if (D_80364418 < D_80364414) {
                if (D_80364414 - D_80364418 > 180.0) {
                    D_8036441C = 1;
                } else {
                    D_8036441D = 1;
                }
            } else {
                D_8036441C = 1;
            }
        }
    }
    return ok;
}

/* Clears the current frame buffer with a fill rectangle */
void func_802558C8(Gfx *arg0, s32 *len) {
    Gfx *gdl = arg0;

    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    gDPSetFillColor(gdl++, 0x00010001);
    gDPPipeSync(gdl++);
    gDPFillRectangle(gdl++, 0, 0, 319, 239);
    *len += gdl - arg0;
}

/* Closes the top-level display list and returns its length in *length */
void func_802559F8(Gfx *arg0, s32 *length) {
    Gfx *gdl = arg0;

    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    *length = gdl - D_803156F8[D_8035805C].dl;
    HD_ASSERT(*length<TOPLEVEL_DL_SIZE, 3665);
}

void func_80255AD0(void) {
    s32 i;
    f32 one;
    s32 pad[4];
    u8 status;

    one = 1.0f;
    D_80364A90 = 32;
    osCreateMesgQueue(&D_803150A0, D_803150B8, 50);
    osCreateMesgQueue(&D_80315180, D_80315198, 0x90);
    func_80270D20(D_80315440, D_80312D80 + 0x400, 13, (D_80000300 != 1) ? 0x10 : 2, 1);
    osCreateMesgQueue(&D_803153D8, D_803153F8, 16);
    func_80270E50(D_80315440, D_803156D8, &D_803153D8, 1, 1);
    status = func_8028A370();
    func_80261588();
    func_8029A7E4("audio inited\n");
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
    D_80358050[0] = PHYS(D_80000400[0]);
    D_80358050[1] = PHYS(D_80000400[1]);
    D_80358058 = PHYS(D_8021ED00);
    func_80284DB0();
    func_802D6710();
    func_8028FC10();
    if (!(status & 1)) {
        D_80364A98 = 0x0800000000000000;
    } else if (D_802FDBD0) {
        D_80364A98 = 0x0000080000000000;
    } else if (D_802FDBD4) {
        D_80364A98 = 0x0040000000000000;
    } else {
        D_80364A98 = 0x10;
    }
    for (i = 0x1FF; i >= 0; i--) {
        D_80310D80[i] = 0x1122334455667788LL;
    }
}

/* Checks the main thread's stack guard pattern from the top down */
void func_80255D34(void) {
    u8 bad;
    s32 i;

    bad = 0;
    for (i = 0x1FF; i >= 0 && !bad; i--) {
        if (D_80310D80[i] != 0x1122334455667788LL) {
            bad = 1;
            func_8029A7E4("stack end =%d\n", i);
        }
    }
}

/* One-time game init: memory, display lists, heap, subsystems */
void func_80255DC8(void) {
    u32 end;
    s32 pad;
    u32 size;
    s32 old;

    size = D_00788000 - D_00787F40;
    osViBlack(1);
    D_80364A70 = func_80261A44(D_80364A98);
    func_802D6710();
    osInvalDCache((void *) 0x80000000, 0x400000);
    D_803649F4 = 0;
    D_80358068 = 0;
    D_80358064 = 0;
    D_80358060 = 0;
    D_8035805C = 0;
    D_80367BD6 = 0;
    func_8026A8BC();
    func_8026A974();
    func_8028AE88();
    func_8028B720();
    D_8035806C = D_803FF600;
    func_8028B4C4(D_00787F40, D_803FF600, &size, 10, 0, 2);
    end = (u32) (D_00788000 - D_00787F40) + (u32) D_803FF600;
    func_8029A7E4("Static end = 0x%x, space=0x%x (%d) bytes\n", end, 0x80400000 - end, 0x80400000 - end);
    D_80358078 = 0;
    func_802558C8(D_803156F8[D_8035805C].dl, &D_80358078);
    func_802559F8(D_803156F8[D_8035805C].dl, &D_80358078);
    D_80358070 = 0x8004B400;
    func_80257490(&D_80358070, 0x10);
    D_8036E694 = D_80358070;
    D_80358070 += 0xA000;
    if (D_802E8F94[D_802E8BDC].unk0 == 2 && !(D_80364A98 & 0x100000000002)) {
        func_8029A7E4("Allocating ghost buffer memory\n");
        D_80358070 += 0x20000;
    }
    D_803B9888 = 0;
    func_802A0700();
    D_803643C8 = ((u32) D_80358088 + 0x40) & ~0x3F;
    func_80278E3C();
    D_803643D9 = 0;
    D_803643DA = 0;
    D_803643D8 = 0;
    D_803643D6 = 0;
    D_803643D7 = 0;
    D_802E8BD8 = 0;
    D_802E8BD4 = 0;
    D_802E8BD0 = 0;
    D_8036EB99 = 0;
    D_803669B4 = 0;
    if (D_80364A98 & 0xC9FD8FE7FBFFC0B0) {
        func_8028B3E0();
    }
    func_80297530(D_802E8BDC);
    func_80272C50();
    if (D_80364A98 == 0x40000000000) {
        func_801F7850();
    }
    old = D_80358070;
    func_8026B118(0);
    func_8029A7E4("Yoshi windows allocated %d bytes, %x\n", D_80358070 - old, D_80358070);
    D_803649D0 = D_80364460;
    func_8028A42C();
    func_802592F0();
    D_8039CAA2 = 0;
}

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
