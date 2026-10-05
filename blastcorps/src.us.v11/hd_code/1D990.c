#include "common.h"
#include <ultra64.h>

/* Level objectives and the HUD's objective/timer text: per-level setup,
 * the countdown intro, and the per-mission-type progress checks */

/* One entry per level in D_802E8F94 (0x44 bytes) */
typedef struct {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 pad1;
    /* 0x02 */ u16 area[4];     /* race track bounds: x0, z0, x1, z1 */
    /* 0x0A */ u16 startBox[4]; /* race start/finish line box */
    /* 0x12 */ u8 quad[4];      /* race: track quadrants in lap order */
    /* 0x16 */ u8 pad16[2];
    /* 0x18 */ u32 target; /* laps, targets, damage or RDUs to get */
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 unk26[3];
    /* 0x2C */ u8 pad2C[0xA];
    /* 0x36 */ u16 time; /* time limit, in tenths of a second */
    /* 0x38 */ u8 unk38;
    /* 0x39 */ u8 pad39[3];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ u16 unk40;
    /* 0x42 */ u16 unk42;
} LevelInfo;

typedef struct {
    u8 id;
    u8 pad;
    s16 pos[3];
} Entry08;

typedef struct {
    u8 id;
    u8 pad;
    s16 pos[4];
} Entry0A;

/* Animated sprite definition (D_802F49F4, 0x30 bytes), as in 26570.c */
typedef struct {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u8 count;
    /* 0x05 */ u8 pad5;
    /* 0x06 */ u16 ids[10];
    /* 0x1A */ u8 nframes;
    /* 0x1B */ u8 frames[11];
    /* 0x26 */ u8 speed; /* frames per animation step */
    /* 0x27 */ u8 pad27;
    /* 0x28 */ f32 scale;
    /* 0x2C */ u8 unk2C;
    /* 0x2D */ u8 unk2D;
    /* 0x2E */ s8 unk2E;
    /* 0x2F */ u8 unk2F;
} Anim30;

typedef struct {
    u8 pad0[0x18];
    u8 rank[0x3C];
    u8 pad54[0x100 - 0x54];
} Player;

extern LevelInfo *D_80367C04;
extern LevelInfo D_802E8F94[];
extern u8 D_802E8F30[];
extern Entry0A D_802E8F68[];
extern f32 D_80364438;
extern u8 D_80367C10;
extern s32 D_803EF2EC;
extern s32 D_803EF2F0;
extern s32 D_803EF2F4;
extern u8 D_803EFEC8;
extern s32 D_803EFEB0;
extern s32 D_803EFEB4;
extern s32 D_803EFEB8;
extern s32 D_803EFEBC;
extern u8 D_803643D9;
extern u8 D_803643DA;
extern u16 D_8036EA7C;
extern u8 D_8036EA78;
extern u32 D_8036EA70;
extern u8 D_8036EB92;
extern u8 D_8036DCD4;
extern s32 D_803643E0;
extern s32 D_803643E4;
extern s32 D_803643E8;
extern char D_80367B60[][20]; /* HUD text lines (progress, lap times) */
extern u32 D_80364AA8;
extern u8 D_803F7806;
extern s32 D_802E8BDC;
extern u8 D_80367BFE;
extern u8 D_80367BFF;
extern s32 D_80358064;
extern s32 D_80367B50;
extern u8 D_80367B54;
extern u16 D_80367B58[];
extern u16 D_80367BF6;
extern Entry08 D_802E8F74[];
extern u8 D_80364410;
extern s32 D_80364404;
extern s32 D_80364408;
extern s32 D_8036440C;
extern s32 D_803F7C10;
extern s32 D_803F7C14;
extern u8 D_80364424;
extern s32 D_80364428;
extern u16 D_8036442C;
extern s32 D_80364430;
extern u32 D_803156C4;
extern s32 D_80367BC0;
extern u32 D_80367BC4;
extern s16 D_80367BD8;
extern char *D_80367C08;
extern s32 D_80367C0C;
extern char D_802E9F90[]; /* "BUILDINGS" */
extern s32 D_802E9F9C;
extern u8 D_802E9FA0[];   /* " S" */
extern u64 D_80364A90;
extern u64 D_80364A98;
extern u8 D_006A32B0[];
extern u8 D_006A8DA0[];
extern void *D_80358070;
extern void *D_80367BE0[];
extern u16 D_80367BC8;
extern u8 D_80367C00;
extern u8 D_80367C01;
extern Anim30 D_802F49F4[];
extern Anim30 *D_80367BCC;
extern Anim30 *D_80367BD0;
extern u8 D_80367BD4;
extern u8 D_80367BD5;
extern s16 D_80367BD6; /* HUD alpha cap */
extern s16 currentYoshiWindow;
extern char D_80367BB0[];
extern u8 D_80367BF8;  /* race: quadrants crossed this lap */
extern u8 D_80367BF9;  /* race: previous quadrant */
extern u8 D_80367BFA;  /* race: current quadrant */
extern u8 D_80367BFB;  /* race: best lap */
extern u16 D_80367BFC; /* race: best lap time */
extern s32 D_80367BBC;
extern u16 D_80370C28;
extern u8 D_802E8BD0;
extern char D_80367D10[];
extern char D_80367D28[];
extern ALCSPlayer *D_80367734;
extern char D_80367C18[];
extern char D_80367C40[];
extern s16 D_80367C68[];
extern s16 D_80367CB8[];
extern void *D_802F5804[];
extern void *D_80367738;
extern u8 D_8035805C;
extern u8 D_803156F4;
extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern u8 D_803643D6;
extern u8 D_803643D7;
extern s32 D_803156C0;
extern s32 D_80364A58;
extern u16 D_80367BF4;
extern u16 D_80367D08;
extern u8 D_803BE738;
extern s16 D_8036BB1A;
extern s16 yoshiState;

void func_802D6A60(char *, const char *, ...);
void func_8029A7E4(const char *, ...);
void func_802C1DD0(s32);
void func_8027EED8(s32, s32, s16 *);
u8 func_802C1B1C(void);
u8 func_8026FA38(char **name, s32 *arg1);
u8 func_80272C5C(u16 *ids, s32 arg1, u8 count, u8 frames, u8 flags, f32 scale);
void func_8028B4C4(void *, void *, s32 *, s32, s32, s32);
void func_8026AF6C(s32);
void func_80260650(void *, s32, s32);
s32 func_8026205C(s32);
s32 func_8028604C(s32);
s32 func_802753C0(void);
void func_80275270(u64, f32);
void func_80260A10(void);
void func_802609F0(void);
void func_80262840(void);
void func_80262FD0(void);
void func_8026303C(void);
void func_80263140(void);
void func_80263358(void);
void func_802633E0(void);
void func_80264A34(char *buf, u16 t, s32 arg2);
s32 func_8026394C(s16 x, s16 y, s16 x0, s16 y0, s16 x1, s16 y1);
void func_80259CCC(Gfx **, char *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
void func_8025E2CC(Gfx **arg0, Gfx **arg1, s32 arg2);
Gfx *func_80264264(Gfx **, Gfx *);
Gfx *func_80274868(Gfx *);
Gfx *func_80274AA4(Gfx *);
Gfx *func_80272ED8(Gfx *, s32, s32, s32, s32, s32, f32);

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void func_80262150(u8 arg0) {
    s32 i;

    D_80364438 = 45.0f;
    i = 0;
    D_80367C10 = 0;
    do {
        if (D_802E8F30[i] == arg0) {
            D_80364438 = 80.0f;
            D_80367C10 = 1;
        } else {
            i++;
        }
    } while (i < 5 && !D_80367C10);
}

void func_802621DC(u8 arg0) {
    LevelInfo *p;

    p = &D_802E8F94[arg0];
    D_803EF2EC = p->unk26[0] << 5;
    D_803EF2F0 = p->unk26[1] << 5;
    D_803EF2F4 = p->unk26[2] << 5;
}

void func_80262238(u8 arg0) {
    s32 i;
    u8 found;

    i = 0;
    found = 0;
    D_803EFEC8 = 0;
    do {
        if (D_802E8F68[i].id == arg0) {
            found = 1;
        } else {
            i++;
        }
    } while (!found && i < 1);
    if (found) {
        D_803EFEC8 = 1;
        D_803EFEB0 = D_802E8F68[i].pos[0] << 5;
        D_803EFEB4 = D_802E8F68[i].pos[1] << 5;
        D_803EFEB8 = D_802E8F68[i].pos[2] << 5;
        D_803EFEBC = D_802E8F68[i].pos[3] << 5;
    }
}

/* Level start: set up the objective, its HUD icon and (for most mission
 * types) load the HUD graphics from ROM */
void func_80262320(u8 arg0) {
    s32 i;
    u8 anim;
    u8 unused;
    u8 found;
    s32 size;

    i = 0;
    anim = 0;
    unused = 0;
    found = 0;
    do {
        if (D_802E8F74[i].id == arg0) {
            found = 1;
        } else {
            i++;
        }
    } while (!found && i < 4);
    if (found) {
        D_80364410 = 1;
        D_80364404 = D_802E8F74[i].pos[0] << 16;
        D_80364408 = D_802E8F74[i].pos[1] << 16;
        D_8036440C = D_802E8F74[i].pos[2] << 16;
    } else {
        D_80364410 = 0;
    }
    D_80367C04 = &D_802E8F94[arg0];
    D_803F7C10 = D_80367C04->unk1C << 5;
    D_803F7C14 = D_80367C04->unk1E << 5;
    D_80364424 = D_80367C04->unk38;
    D_80364428 = D_80367C04->unk3C << 5;
    D_8036442C = D_80367C04->unk40;
    D_80364430 = D_80367C04->unk42 << 5;
    D_80367BC0 = D_803156C4;
    D_80367BC4 = -1;
    D_80367BD8 = 0;
    switch (D_80364AA8) {
        case 4:
        case 0x80:
            anim = 6;
            break;
        case 0x20:
            anim = func_8026FA38(&D_80367C08, &D_80367C0C);
            D_80367BD8 = 2;
            break;
        case 0x10:
        case 0x40:
            anim = 9;
            D_80367BD8 = -2;
            break;
        case 1:
        case 2:
        case 8:
            break;
    }
    if (anim == 6) {
        D_80367BD8 = 6;
        D_80367C08 = D_802E9F90;
        unused = 0;
        D_80367C0C = D_802E9F9C;
    }
    if (D_80364AA8 != 1) {
        size = D_006A8DA0 - D_006A32B0;
        if (D_80364AA8 != 0x80 && D_80364A98 == 0x2000) {
            func_8028B4C4(D_006A32B0, D_80358070, &size, 10, 0, 1);
            for (i = 0; i < 5; i++) {
                D_80367BE0[i] = (i << 15) + (u8 *) D_80358070;
            }
            D_80358070 = (u8 *) D_80358070 + size;
        }
        if (D_80364A98 == 0x40) {
            D_80367BC8 = 0;
        } else {
            D_80367BC8 = 1;
        }
    } else {
        D_80367BC8 = 0;
    }
    D_80367C01 = 0;
    D_80367C00 = 0;
    if (D_80364A98 == 0x40) {
        D_80367BFF = 0;
    } else {
        D_80367BFF = 0;
        D_80367BFE = 0;
    }
    D_80367BCC = &D_802F49F4[anim];
    if (D_802E8F94[arg0].type == 0x20) {
        D_80367BD0 = NULL;
        D_80367BD4 = func_80272C5C(D_80367BCC->ids, 0, D_80367BCC->count, D_80367BCC->unk2C, D_80367BCC->unk2D,
                                   D_80367BCC->scale * 0.5);
    } else {
        D_80367BD0 = NULL;
        if (anim) {
            D_80367BD4 = func_80272C5C(D_80367BCC->ids, 0, D_80367BCC->count, D_80367BCC->unk2C, D_80367BCC->unk2D,
                                       1.0f);
        } else {
            D_80367BCC = NULL;
        }
    }
    if (D_80364A98 & 0x440) {
        func_80264A34(D_80367BB0, D_80367C04->time - D_80367BF6, 0);
    } else {
        D_80367B54 = 0;
    }
}

/* Mission intro: one step per second. Second 0 prints the objective and the
 * time limit, 1-4 play the countdown sounds */
void func_80262840(void) {
    s32 secs;
    u16 mins;
    u16 rem;

    secs = (u32) (D_803156C4 - D_80367BC0) / 60;
    if (secs != D_80367BC4) {
        switch (secs) {
            case 0:
                D_80367C68[0] = 0xFFF;
                D_80367CB8[0] = 0xFFF;
                switch (D_80364AA8) {
                    case 2:
                        func_802D6A60(D_80367C18, "FINISH %d LAPS IN", D_80367C04->target);
                        break;
                    case 4:
                    case 0x20:
                        if (D_802E8BDC == 0x34) {
                            func_802D6A60(D_80367C18, "DESTROY TARGETS IN");
                        } else {
                            func_802D6A60(D_80367C18, "DESTROY %s IN", D_80367C08);
                        }
                        break;
                    case 0x80:
                        if (D_802E8BDC == 0x32) {
                            func_802D6A60(D_80367C18, "CLEAR SHUTTLE PATH");
                        } else {
                            func_802D6A60(D_80367C18, "CLEAR CARRIER PATH");
                        }
                        break;
                    case 8:
                        func_802D6A60(D_80367C18, "CAUSE $%d DAMAGE", D_80367C04->target);
                        break;
                    case 0x10:
                    case 0x40:
                        func_802D6A60(D_80367C18, "FIND %d RDUS IN", D_80367C04->target);
                        break;
                }
                mins = D_80367C04->time / 600;
                rem = (D_80367C04->time / 10) % 60;
                func_802D6A60(D_80367C40, "%d MINUTE%c %d SECONDS", mins, D_802E9FA0[mins != 1], rem);
                D_802F5804[255] = D_80367C18;
                D_802F5804[262] = D_80367C40;
                D_802F5804[256] = D_80367C68;
                D_802F5804[263] = D_80367CB8;
                func_8026AF6C(0x8009);
                func_80260650(D_80367738, func_8026205C(0), 0);
                break;
            case 1:
                func_80260650(D_80367738, 0x96, 0);
                break;
            case 2:
                func_80260650(D_80367738, 0x96, 0);
                break;
            case 3:
                func_80260650(D_80367738, 0x96, 0);
                break;
            case 4:
                func_80260650(D_80367738, 0x99, 0);
                break;
        }
    }
    D_80367BC4 = secs;
    if (secs == 4 && D_8035805C == D_803156F4) {
        D_80364A98 = 4;
    }
}

/* Per-frame objective update: the time limit, the countdown beeps, and the
 * progress check for the current mission type */
void func_80262BF4(void) {
    if (!D_803643D7 && !D_803643D6) {
        D_80367BF6 = D_80367C04->time - MIN(D_80367C04->time, func_8028604C(D_803156C0 - D_80364A58));
    }
    func_80264A34(D_80367BB0, D_80367BF6, 1);
    switch (D_80364A90) {
        case 0x2000:
            func_80262840();
            break;
        case 0x04000000:
            if (D_803643D6 && func_802753C0() == 0 && yoshiState == 1) {
                if ((D_80364AF0[D_80364AE8].rank[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].rank[D_802E8BDC] < 6) ? 1
                                                                                                                 : 0) {
                    func_80275270(0x08000000, 0.25f);
                } else {
                    func_80275270(0x40, 0.25f);
                }
            }
            break;
        default:
            if (D_80367BF6 == 0) {
                D_803643D9 = 1;
            } else {
                D_80367BF4 = D_80367BF6 / 10;
                if (D_80367BF4 < 10 && D_80367BF4 != D_80367D08) {
                    func_80260650(D_80367738, 0xA3 - D_80367BF4, 0);
                }
                D_80367D08 = D_80367BF4;
            }
            if (D_803BE738 && D_80364AA8 == 0x40) {
                func_80260650(D_80367738, 0x3C, 0);
            }
            if (D_803BE738) {
                D_803643D9 = 1;
            }
            if (D_803643D6) {
                func_80260A10();
                func_802609F0();
                func_8026AF6C(0xA00E);
                D_8036BB1A = -1;
                D_80364A98 = 0x04000000;
            } else if (D_803643D7) {
                D_80364A98 = 0x04000000;
            } else {
                switch (D_80364AA8) {
                    case 2:
                        func_802633E0();
                        break;
                    case 4:
                        func_8026303C();
                        break;
                    case 0x20:
                    case 0x80:
                        func_80263140();
                        break;
                    case 8:
                        func_80263358();
                        break;
                    case 0x10:
                    case 0x40:
                        func_80262FD0();
                        break;
                }
            }
            break;
    }
}

/* Mission type 0x10/0x40 (find the RDUs): progress text */
void func_80262FD0(void) {
    if (D_8036EA7C >= D_80367C04->target) {
        D_803643DA = 1;
    }
    func_802D6A60(D_80367B60[0], "%d/%d", D_8036EA7C, D_80367C04->target);
}

/* Mission type 4 (destroy targets) */
void func_8026303C(void) {
    s16 h;

    func_802C1DD0(0);
    if (D_8036EA78 >= D_80367C04->target) {
        D_803643DA = 1;
    }
    if (D_8036DCD4) {
        func_8027EED8(D_803643E0 >> 5, D_803643E8 >> 5, &h);
        if (h > (D_803643E4 >> 5)) {
            D_803643D9 = 1;
        }
    } else if (D_80367C04->unk24 > (D_803643E4 >> 5)) {
        D_803643D9 = 1;
    }
    func_802D6A60(D_80367B60[0], "%d/%d", D_8036EA78, D_8036EB92);
}

/* Mission types 0x20/0x80 (destroy targets, clear the path) */
void func_80263140(void) {
    s16 h;

    func_802C1DD0(1);
    switch (D_80364AA8) {
        case 0x20:
            if (D_8036EA78 >= D_8036EB92) {
                D_803643DA = 1;
            }
            break;
        case 0x80:
            if (D_803F7806 || (D_802E8BDC == 0x32 && D_8036EA78 >= D_8036EB92)) {
                D_803643DA = 1;
            }
            break;
    }
    if (D_8036DCD4) {
        func_8027EED8(D_803643E0 >> 5, D_803643E8 >> 5, &h);
        if (h > (D_803643E4 >> 5)) {
            D_803643D9 = 1;
        }
    } else if (D_80367C04->unk24 > (D_803643E4 >> 5)) {
        D_803643D9 = 1;
    }
    switch (D_80364AA8) {
        case 0x20:
            func_802D6A60(D_80367B60[0], "%d/%d", D_8036EA78, D_8036EB92);
            break;
        case 0x80:
            if (D_802E8BDC == 0x32) {
                func_802D6A60(D_80367B60[0], "%d/%d", D_8036EA78, D_8036EB92);
            } else {
                func_802D6A60(D_80367B60[0], "%d/%d", func_802C1B1C(), D_8036EB92);
            }
            break;
    }
}

/* Mission type 8 (cause $N damage) */
void func_80263358(void) {
    s32 left;

    func_802C1DD0(0);
    if (D_8036EA70 >= D_80367C04->target) {
        D_803643DA = 1;
    }
    left = D_80367C04->target - D_8036EA70;
    if (left < 0) {
        left = 0;
    }
    func_802D6A60(D_80367B60[0], "$%d LEFT", left);
}

/* Mission type 2 (race): start line, lap times, quadrant tracking */
void func_802633E0(void) {
    s32 i;
    u8 left;

    if (D_80367B54 == 0) {
        if (func_8026394C(D_803643E0 >> 5, D_803643E8 >> 5, D_80367C04->startBox[0], D_80367C04->startBox[1],
                          D_80367C04->startBox[2], D_80367C04->startBox[3])) {
            D_80367B54 = 1;
            D_80367BF8 = 0;
            D_80367BF9 = D_80367C04->quad[0];
            D_80367BBC = D_803156C0;
            D_80367BFB = 1;
            D_80367BFC = 0xFFFF;
        }
    } else {
        D_80367BFA = ((D_803643E8 >> 5) - D_80367C04->area[1]) / ((D_80367C04->area[3] - D_80367C04->area[1]) >> 1);
        D_80367BFA *= 2;
        D_80367BFA += ((D_803643E0 >> 5) - D_80367C04->area[0]) / ((D_80367C04->area[2] - D_80367C04->area[0]) >> 1);
        D_80367B58[D_80367B54 - 1] = func_8028604C(D_803156C0 - D_80364A58);
        for (i = D_80367B54 - 2; i >= 0; i--) {
            D_80367B58[D_80367B54 - 1] -= D_80367B58[i];
        }
        if (D_80370C28 & 0x2000) {
            func_8029A7E4("box number=%d\n", D_80367BFA);
        }
        if (D_80367BF8 == 4) {
            if (func_8026394C(D_803643E0 >> 5, D_803643E8 >> 5, D_80367C04->startBox[0], D_80367C04->startBox[1],
                              D_80367C04->startBox[2], D_80367C04->startBox[3])) {
                if (D_80367B58[D_80367B54 - 1] < D_80367BFC) {
                    func_8029A7E4("new best lap %d %d\n", D_80367BFC, D_80367B58[D_80367B54 - 1]);
                    D_80367BFB = D_80367B54;
                    D_80367BFC = D_80367B58[D_80367B54 - 1];
                }
                if (D_80367C04->target - 1 < D_80367B54) {
                    D_803643DA = 1;
                } else {
                    left = D_80367C04->target - D_80367B54;
                    if (!D_802E8BD0) {
                        func_8026AF6C(0x8008);
                    }
                    if (left == 1) {
                        func_802D6A60(D_80367D10, "1 LAP LEFT!");
                    } else {
                        func_802D6A60(D_80367D10, "%d LAPS LEFT!", left);
                    }
                    D_802F5804[248] = D_80367D10;
                    D_802F5804[249] = D_80367D28;
                    alCSPSetTempo(D_80367734, alCSPGetTempo(D_80367734) * 0.95);
                }
                D_80367B54++;
                D_80367BF8 = 0;
            }
        } else if (D_80367C04->quad[D_80367BF8] == D_80367BF9 && D_80367C04->quad[(D_80367BF8 + 1) % 4] == D_80367BFA) {
            func_8029A7E4("box cross: %d to %d\n", D_80367BF9, D_80367BFA);
            D_80367BF8++;
        }
        D_80367BF9 = D_80367BFA;
    }
    for (i = 0; i < D_80367B54 && i < D_80367C04->target; i++) {
        func_80264A34(D_80367B60[i], D_80367B58[i], D_80367B54 == i + 1);
    }
}

/* Is (x, y) inside the box [x0, x1) x [y0, y1)? */
s32 func_8026394C(s16 x, s16 y, s16 x0, s16 y0, s16 x1, s16 y1) {
    if (x >= x0 && y >= y0 && x < x1 && y < y1) {
        return 1;
    }
    return 0;
}

/* Draw the objective HUD: progress text or lap times, the time limit
 * (flashing red under ten seconds), and the objective icon(s) */
Gfx *func_802639B4(Gfx *gdl, Gfx **arg1, u32 *arg2) {
    Gfx *g;
    s32 pad60;
    u8 grn;
    u8 red;
    u8 flash;
    s16 alpha;
    s32 pad54;
    s32 i;

    g = gdl;
    if ((D_80364A90 & 0x04000200) && currentYoshiWindow != 0x1E) {
        alpha = 0xFF;
    } else {
        alpha = D_80367BD6;
    }
    switch (D_80364AA8) {
        case 2:
            for (i = 0; i < D_80367B54 - ((D_80364A90 & 0x440) ? 1 : 0) && i < D_80367C04->target; i++) {
                if (i + 1 == D_80367BFB && i + 1 != D_80367B54) {
                    /* best lap */
                    func_80259CCC(arg1, D_80367B60[i], 0, 1, 0, 0x18, i * 18 + 0x12, 0x14, 0x14, 1, 0xFF, 0, 0,
                                  D_80367BD6);
                } else if (i + 1 == D_80367B54) {
                    /* current lap */
                    func_80259CCC(arg1, D_80367B60[i], 0, 1, 0, 0x18, i * 18 + 0x12, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF,
                                  D_80367BD6);
                } else {
                    func_80259CCC(arg1, D_80367B60[i], 0, 1, 0, 0x18, i * 18 + 0x12, 0x14, 0x14, 1, 0xA0, 0xA0, 0xA0,
                                  D_80367BD6);
                }
            }
            break;
        case 4:
        case 0x20:
        case 0x80:
            func_80259CCC(arg1, D_80367B60[0], 0, 1, 0, 0x38, 0x14, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF, alpha);
            break;
        case 8:
            func_80259CCC(arg1, D_80367B60[0], 0, 1, 0, 0x1C, 0x12, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF, alpha);
            break;
        case 0x10:
        case 0x40:
            func_80259CCC(arg1, D_80367B60[0], 0, 1, 0, 0x38, 0x12, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF, alpha);
            break;
    }
    red = 0xFF;
    grn = 0;
    if (D_80364A90 & 0x04000104) {
        flash = D_80367BF4 < 10;
        if (!flash) {
            grn = 0xFF;
        }
    } else {
        flash = 0;
        grn = 0xFF;
        red = 0;
    }
    if (D_80367BF4 == 0) {
        flash = 0;
    }
    if (!flash || D_803156C4 % 20 < 16) {
        if (D_80364AA8 == 2) {
            func_80259CCC(arg1, D_80367BB0, 0, 1, 0, 0x18, i * 18 + 0x14, 0x10, 0x10, 1, red, grn, 0, D_80367BD6);
        } else {
            func_80259CCC(arg1, D_80367BB0, 0, 1, 0, 0x1C, D_80367BD8 + 0x2A, 0x10, 0x10, 1, red, grn, 0, alpha);
        }
    }
    if (D_803643D7 && D_80364A90 == 0x04000000) {
        func_8025E2CC(&g, arg1, D_8035805C);
    }
    if (D_80367BC8) {
        g = func_80264264(arg1, g);
    }
    g = func_80274868(g);
    if (D_80367BCC) {
        g = func_80272ED8(g, D_80367BCC->frames[(D_803156C4 / D_80367BCC->speed) % D_80367BCC->nframes] + D_80367BD4 - 1,
                          0x18, D_80367BD8 + 0xC, alpha, 1, 1.0f);
    }
    if (D_80367BD0) {
        g = func_80272ED8(g, D_80367BD0->frames[(D_803156C4 / D_80367BD0->speed) % D_80367BD0->nframes] + D_80367BD5 - 1,
                          0x18, D_80367BD8 + 0xC, alpha, 1, 1.0f);
    }
    g = func_80274AA4(g);
    arg2 += g - gdl; /* dead store to the parameter, kept for the match */
    return g;
}

void func_8026420C(void) {
    if (D_80367BFE && D_80358064 == D_80367B50) {
        func_8029A7E4("Replay turbo ....\n");
        D_80367BFF = 1;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/1D990/func_80264264.s")

/* Format a time in tenths of a second as "MM:SS.t" */
void func_80264A34(char *buf, u16 t, s32 arg2) {
    buf[0] = t / 6000 + '0';
    t %= 6000;
    buf[1] = t / 600 + '0';
    buf[2] = ':';
    t %= 600;
    buf[3] = t / 100 + '0';
    t %= 100;
    buf[4] = t / 10 + '0';
    t %= 10;
    buf[5] = '.';
    buf[6] = t + '0';
    buf[7] = 0;
}

/* Race (mission type 2): time left = limit minus the finished laps' times */
void func_80264AEC(void) {
    s32 i;
    u16 sum;

    sum = 0;
    if (D_80364AA8 == 2 && D_80367B54 >= 2) {
        for (i = 0; i < D_80367B54 - 1; i++) {
            sum += D_80367B58[i];
        }
        D_80367BF6 = D_80367C04->time - sum;
    }
    if (D_80367BF6 >= 60000) {
        D_80367BF6 = 0;
    }
}

u8 func_80264BA4(u8 arg0) {
    u8 ret;

    switch (arg0) {
        case 40:
            ret = 0;
            break;
        case 43:
            ret = 5;
            break;
        case 44:
            ret = 4;
            break;
        case 45:
            ret = 2;
            break;
        case 46:
            ret = 1;
            break;
        default:
            ret = 3;
            break;
    }
    return ret;
}
