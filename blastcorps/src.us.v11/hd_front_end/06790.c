#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802081C0 ((RankName *) D_802081C0)
#define D_80364AF0 ((FePlayer *) D_80364AF0)
/* end of views */

/*
 * Promotion screen: "CONGRATULATIONS / ON YOUR PROMOTION!" plus the new
 * rank's name, over a spinning, growing 3D scene (scene 0 of 0DE70.c).
 */


/* front-end view of hd_code's Player record (0x100 bytes, D_80364AF0) */
typedef struct {
    u8 pad0[0xC];
    u8 unkC; /* rank */
    u8 padD[0x100 - 0xD];
} FePlayer;

typedef struct {
    u8 *unk0; /* rank name, words separated by spaces */
    u8 pad4[4];
} RankName;

extern s32 D_80215960; /* scale state */
extern s32 D_80215964; /* text fade state */
extern f32 D_80215968; /* scene angle */
extern f32 D_8021596C; /* scene scale */
extern s32 D_80215970;
extern s16 D_80215974; /* title alpha */
extern s16 D_80215976; /* rank name alpha */
extern s32 D_80215978; /* rank */
extern u8 D_80215980[]; /* rank name, first line */
extern u8 D_80215998[]; /* rank name, second line */
extern s16 D_802159B0;
extern u16 D_80303B78[];
extern u16 D_80303B88[];


/* title lines and their glyph lists */
u8 *D_802084B0 = (u8 *) "CONGRATULATIONS";
u8 *D_802084B4 = (u8 *) "ON YOUR PROMOTION!";
u16 *D_802084B8 = D_80303B78;
u16 *D_802084BC = D_80303B88;
s8 D_802084C0 = 1; /* colour cycle direction */

/* Promotion screen init: load scene 0 and reset the screen's state. */
void func_801ED790(void) {
    func_801F4E70(0);
    D_80215960 = 0;
    D_80215964 = 0;
    D_80215968 = D_8021596C = 0.0f;
    D_80215974 = D_80215976 = 0;
    D_80215978 = D_80364AF0[D_80364AE8].unkC;
}

/* Promotion screen frame: animate, draw the text and the scene; *arg3 += commands written. */
Gfx *func_801ED800(Gfx *arg0, FeDyn *dyn, u8 arg2, s32 *arg3) {
    Gfx *gdl = arg0;
    s32 i;
    s32 j;
    FePlayer *p;
    s32 size;

    if ((D_80370C28 & 0x8000) && !(D_80370C2A & 0x8000) && D_802FA268) {
        D_80215960 = 2;
        D_80215964 = 3;
    }
    switch (D_80215960) {
        case 0:
            D_8021596C = sins(D_80358060 * 0x4000 * 60 / 60 / 90) * 2.85 / 32767.0;
            if (D_8021596C >= 2.84) {
                D_8021596C = 2.84f;
                D_80215960 = 1;
                D_80215970 = 0x91;
            }
            break;
        case 1:
            if (D_80215970-- == 0) {
                D_80215960 = 2;
            }
            break;
        case 2:
            D_8021596C *= 0.9;
            break;
        case 3:
            break;
    }
    switch (D_80215964) {
        case 0:
            if (D_80358060 == 50) {
                D_80215964 = 1;
                D_80215974 = 0;
                D_80215976 = 0;
            }
            break;
        case 1:
            if (D_80215974 + 16 > 0xFF) {
                D_80215974 = 0xFF;
            } else {
                D_80215974 += 16;
            }
            if (D_80358060 == 200) {
                D_80215964 = 2;
            }
            break;
        case 2:
            if (D_80215974 - 32 < 0) {
                D_80215974 = 0;
            } else {
                D_80215974 -= 32;
            }
            if (D_80215976 + 16 > 0xFF) {
                D_80215976 = 0xFF;
            } else {
                D_80215976 += 16;
            }
            if (D_80358060 == 290) {
                D_80215964 = 3;
            }
            break;
        case 3:
            if (D_80215976 - 32 < 0) {
                D_80215976 = 0;
            } else {
                D_80215976 -= 32;
            }
            if (D_80215976 == 0) {
                D_80364A98 = 0x08000000;
            }
            break;
    }
    if (D_80215974 > 0 || D_80215976 > 0) {
        p = &D_80364AF0[D_80364AE8];
        func_80259CCC((Gfx **) dyn, D_802084B0, D_802084B8, 0, 0x9C, 0, 0x18, 0x1A, 0x1A, 1, 0, 0, 0, D_80215974 / 2);
        func_80259CCC((Gfx **) dyn, D_802084B4, D_802084BC, 0, 0x9D, 0, 0xCB, 0x16, 0x16, 1, 0, 0, 0, D_80215974 / 2);
        /* split the rank name into two lines */
        for (i = 0, j = 0; j < D_802082B8[D_80215978]; i++) {
            if ((D_80215980[i] = D_802081C0[D_80215978].unk0[i]) == ' ') {
                j++;
            }
        }
        D_80215980[i - 1] = 0;
        for (j = i; D_802081C0[D_80215978].unk0[i] != 0; i++) {
            D_80215998[i - j] = D_802081C0[D_80215978].unk0[i];
        }
        D_80215998[i - j] = 0;
        if (func_8025B300(D_80215980) >= 14 || func_8025B300(D_80215998) >= 14) {
            size = 0x1D;
        } else {
            size = 0x21;
        }
        func_80259CCC((Gfx **) dyn, D_80215980, NULL, 0, 0x9D, 0, 0x58, size, size, 1, 0, 0, 0, D_80215976 / 2);
        func_80259CCC((Gfx **) dyn, D_80215998, NULL, 0, 0x9D, 0, 0x76, size, size, 1, 0, 0, 0, D_80215976 / 2);
        D_802159B0 += D_802084C0 * 15;
        if (D_802159B0 > 0xFF) {
            D_802159B0 -= 30;
            D_802084C0 = -D_802084C0;
        }
        if (D_802159B0 < 0) {
            D_802159B0 += 30;
            D_802084C0 = -D_802084C0;
        }
        func_80259DC8((Gfx **) dyn, D_802084B0, D_802084B8, 0, 0xA0, 0, 0x14, 0x1A, 0x1A, 1, 0xFF, 0xFF - D_802159B0, 0,
                      D_80215974, 0xFF, D_802159B0, 0, D_80215974);
        func_80259DC8((Gfx **) dyn, D_802084B4, D_802084BC, 0, 0xA0, 0, 0xC8, 0x16, 0x16, 1, 0xFF, 0xFF - D_802159B0, 0,
                      D_80215974, 0xFF, D_802159B0, 0, D_80215974);
        func_80259DC8((Gfx **) dyn, D_80215980, NULL, 0, 0xA0, 0, 0x55, size, size, 1, 0xFF, 0xFF - D_802159B0, 0,
                      D_80215976, 0xFF, D_802159B0, 0, D_80215976);
        func_80259DC8((Gfx **) dyn, D_80215998, NULL, 0, 0xA0, 0, 0x73, size, size, 1, 0xFF, 0xFF - D_802159B0, 0,
                      D_80215976, 0xFF, D_802159B0, 0, D_80215976);
    }
    D_80215968 += 12.0 - D_8021596C * 2.0f;
    if (D_80215968 > 360.0) {
        D_80215968 -= 360.0;
    }
    if (D_80358060 < 2) {
        guPerspective(&dyn->unk1240, &D_8035807C, 45.0f, 1.3333334f, 40.0f, 4000.0f, 1.0f);
        guLookAtReflect(&dyn->unk140, &dyn->unk3C00, 5.0f, 7.0f, 400.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        guMtxIdent(&dyn->unk1280);
        guMtxIdent(&dyn->unk12C0);
    }
    guScale(&dyn->unk1300, D_8021596C / 8.0f, D_8021596C / 8.0f, D_8021596C / 8.0f);
    guRotate(&D_802182D0[arg2], D_80215968, 1.0f, 1.0f, 1.0f);
    if (D_8021596C > 0.2) {
        gdl = func_801F4FBC(dyn, gdl);
    }
    *arg3 += gdl - arg0;
    return gdl;
}
