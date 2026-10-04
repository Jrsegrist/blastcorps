#include "common.h"
#include <ultra64.h>

/* D_8039B070: array of D_8039B610 0x48-byte entries. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ u8 pad0C[0x0E - 0x0C];
    /* 0x0E */ u8 unk0E;
    /* 0x0F */ u8 pad0F[0x14 - 0x0F];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 pad1A[0x1E - 0x1A];
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u8 pad20[0x23 - 0x20];
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 pad25;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ u8 pad2A[0x40 - 0x2A];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Entry48D00;
extern Entry48D00 D_8039B070_entries[];

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028D4C0.s")

/* TODO: func_8028DA5C - fill the 8 corner Vtx of box type arg1 from the
 * 0x18-stride min/max table at D_802FDB98 (x0/x1, y0/y1, z0/z1 at 0..0xA;
 * the same table's +0x14 is the radius used by func_8028F994), with
 * texture coords 0 or 31.0 (0x3E0). Best draft (BoxDef48D00 /
 * D_802FDB98_boxes alias, per corner `ob[0..2]` then `tc[1]`, `tc[0]`)
 * is exact except corners 2 and 5, where both tcs are 0x3E0: target
 * loads the tc[0] constant first (t5) but stores tc[1] first; every
 * spelling tried gives one or the other, not both (both statement
 * orders, chained assignment both ways, raw-offset / array / mixed
 * spellings for just those corners, tc-before-ob orders).
 *
 * void func_8028DA5C(Vtx *v, u8 arg1) {
 *     v[0].v.ob[0] = D_802FDB98_boxes[arg1].x1;
 *     v[0].v.ob[1] = D_802FDB98_boxes[arg1].y0;
 *     v[0].v.ob[2] = D_802FDB98_boxes[arg1].z0;
 *     v[0].v.tc[1] = 0;
 *     v[0].v.tc[0] = 0;
 *     ... corners (x, y, z, tc0, tc1): 1 = (x1, y1, z0, 0, 0x3E0),
 *     2 = (x1, y1, z1, 0x3E0, 0x3E0), 3 = (x1, y0, z1, 0x3E0, 0),
 *     4 = (x0, y0, z0, 0x3E0, 0), 5 = (x0, y1, z0, 0x3E0, 0x3E0),
 *     6 = (x0, y1, z1, 0, 0x3E0), 7 = (x0, y0, z1, 0, 0)
 * }
 * (BoxDef48D00: s16 x0, x1, y0, y1, z0, z1; pad to 0x14; s16 radius;
 * size 0x18. D_802FDB98_boxes is already in undefined_syms.) */
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DA5C.s")

/* Activate entry arg0. The mix of struct-field and per-field-symbol
 * accesses below is deliberate: it is the spelling whose schedule matches. */
extern u8 D_8039B070;
extern u8 D_8039B088;
extern u8 D_8039B089;
extern u8 D_8039B0B0;
extern u8 D_8039B0B4;
extern u8 D_802E8BE4;
extern s32 D_802E8BE8;
extern s32 D_80367738;

void func_802CDA10(s32, s32, s32);
void func_802608C8(s32);
void func_80260650(s32, s32, s32);
Entry48D00 *func_8028DE94(void);

void func_8028DD64(u8 arg0) {
    Entry48D00 *match;

    func_802CDA10(D_8039B070_entries[arg0].unk0, D_8039B070_entries[arg0].unk4, D_8039B070_entries[arg0].unk8);
    *(&D_8039B089 + arg0 * 0x48) = 5;
    D_8039B070_entries[arg0].unk18 = 0;
    D_802E8BE4 = 10;
    D_802E8BE8 = 0x190;
    if (D_8039B070_entries[arg0].unk40 != 0) {
        func_802608C8(D_8039B070_entries[arg0].unk40);
        match = func_8028DE94();
        if (match != NULL) {
            func_80260650(D_80367738, 0x73, (s32) &match->unk40);
        }
    }
    if (*(s32 *) (&D_8039B0B4 + arg0 * 0x48) != 0) {
        func_802608C8(*(s32 *) (&D_8039B0B4 + arg0 * 0x48));
    }
    func_80260650(D_80367738, 0x10, 0);
}

extern u8 D_8039B070;
extern s32 D_8039B610;

/* First entry with both unk18 and unk14 set, or NULL. */
Entry48D00 *func_8028DE94(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk14 != 0) {
            return &D_8039B070_entries[i];
        }
    }
    return NULL;
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028DF14.s")

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/48D00/func_8028E9E4.s")

void func_802AACD4(u8, s32, s32, void *, void *);
extern u8 D_8039B094;

void func_8028F6B4(u8 arg0) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk23 == arg0) {
            D_8039B070_entries[i].unk1E = 0;
            *(&D_8039B094 + i * 0x48) = 1;
            func_802AACD4(arg0, D_8039B070_entries[i].unk0, D_8039B070_entries[i].unk8,
                          &D_8039B070_entries[i].unk26, &D_8039B070_entries[i].unk28);
        }
    }
}

extern s32 D_8039B610;
extern u8 D_8039B070;

s32 func_802AAE1C(u8, s16, s16, void *, void *);
s32 func_802CE6F8(s32, s32, s32);
void func_802CE4F0(s32, s32, s32);
s32 func_802CDB70(s16, s16);
void func_8028DD64(u8);

void func_8028F794(u8 arg0) {
    s32 i;
    s16 local;

    for (i = 0; i < D_8039B610; i++) {
        if (*(&D_8039B070 + i * 0x48 + 0x18) != 0 && *(&D_8039B070 + i * 0x48 + 0x24) != 0) {
            func_802AAE1C(
                arg0,
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x26),
                *(s16 *) (&D_8039B070 + i * 0x48 + 0x28),
                (void *) (&D_8039B070 + i * 0x48),
                (void *) (&D_8039B070 + i * 0x48 + 8));

            *(s32 *) (&D_8039B070 + i * 0x48 + 4) = func_802CE6F8(
                *(s32 *) (&D_8039B070 + i * 0x48),
                *(s32 *) (&D_8039B070 + i * 0x48 + 8),
                *(s32 *) (&D_8039B070 + i * 0x48 + 4));

            if (*(s16 *) (&D_8039B070 + i * 0x48 + 0x1A) != 0) {
                local = 0;
            } else {
                local = *(s16 *) (&D_8039B070 + i * 0x48 + 0x1C);
            }

            func_802CE4F0(*(s32 *) (&D_8039B070 + i * 0x48),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 4),
                          *(s32 *) (&D_8039B070 + i * 0x48 + 8));

            if (func_802CDB70(*(s16 *) (0x802FDBAC + *(&D_8039B070 + i * 0x48 + 0xE) * 0x18), local) != 0) {
                func_8028DD64((u8) i);
            }
        }
    }
}

extern u8 D_8039B094;

void func_8028F93C(void) {
    s32 i;

    for (i = 0; i < D_8039B610; i++) {
        *(&D_8039B094 + i * 0x48) = 0;
    }
}

extern u8 D_803A7424;
s32 func_8026A6F0(s32, s32, s32, s32, s32, s32);

/* Set D_803A7424 if the point (arg0, arg1, arg2) is within any active
 * entry's category radius (all in 1/32 units). */
void func_8028F994(s32 arg0, s32 arg1, s32 arg2) {
    s32 i;
    s32 dist;

    /* One statement (likely a macro in the original): separate
     * statements schedule the three reloads in the opposite order. */
    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5;
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
                                 D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
            if (dist <= (*(s16 *) (0x802FDBAC + D_8039B070_entries[i].unk0E * 0x18) >> 5)) {
                D_803A7424 = 1;
            }
        }
    }
}

extern u8 D_802FDB98[];
void func_8028FAC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i;
    s32 dist;

    arg0 >>= 5, arg1 >>= 5, arg2 >>= 5, arg3 >>= 5;
    for (i = 0; i < D_8039B610; i++) {
        if (D_8039B070_entries[i].unk18 != 0 && D_8039B070_entries[i].unk24 == 0) {
            dist = func_8026A6F0(arg0, arg1, arg2, D_8039B070_entries[i].unk0 >> 5,
                                 D_8039B070_entries[i].unk4 >> 5, D_8039B070_entries[i].unk8 >> 5);
            if (dist <= (*(s16 *) ((u8 *) D_802FDB98 + 0x14 + D_8039B070_entries[i].unk0E * 0x18) >> 5) + arg3) {
                D_803A7424 = 1;
            }
        }
    }
}

extern OSMesgQueue D_80370BF8;
extern u8 D_802FDBD0;
extern u8 D_802FDBD4;
void func_802DB4D0(OSMesgQueue *);
void func_802DB594(OSContPad *);
u8 func_8028FCD4(void *arg0, u8 *arg1);

/* Boot-time controller check: read the pads (osContStartReadData,
 * osRecvMesg, osContGetReadData), note whether START is held, latch it
 * into D_802FDBD0 if controller 1 is present, and set D_802FDBD4 when
 * START is held but was not latched. */
void func_8028FC10(void) {
    u8 mask;
    u8 start;
    OSContPad pads[4];

    start = 0;
    func_802DB4D0(&D_80370BF8);
    osRecvMesg(&D_80370BF8, 0, 1);
    func_802DB594(pads);
    if (pads[0].button & 0x1000) {
        start = 1;
    }
    if (func_8028FCD4(&D_80370BF8, &mask) == 0 && (mask & 1)) {
        D_802FDBD0 = start;
    }
    D_802FDBD4 = start && !D_802FDBD0;
}

typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Status48D00;


/* Wait for arg0->unk8, then build a bitmask in *arg1 of the 4 status
 * entries with bit 0 of unk2 set and unk3 clear. */
u8 func_8028FCD4(void *arg0, u8 *arg1) {
    Status48D00 status[4];
    s32 i;

    *arg1 = 0;
    osContStartQuery(arg0);
    while (*(s32 *) ((u8 *) arg0 + 8) == 0) {
    }
    osRecvMesg(arg0, 0, 0);
    osContGetQuery(status);
    for (i = 0; i < 4; i++) {
        if ((status[i].unk2 & 1) && status[i].unk3 == 0) {
            *arg1 |= 1 << i;
        }
    }
    return status[0].unk3;
}
