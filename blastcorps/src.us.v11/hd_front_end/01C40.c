#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* player.c (per its assert strings): save slots, player ranks and the
 * player select / Controller Pak screens */

/* One entry per level in D_802E8F94 (0x44 bytes), as in hd_code 1D990.c */
typedef struct {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 pad1[0x2D];
    /* 0x2E */ u16 times[5]; /* rank times; [4] is the time limit */
    /* 0x38 */ u8 pad38[0xC];
} LevelInfo;

/* One per save slot (D_80364AF0, 0x100 bytes), as in hd_code 26570.c */
typedef struct {
    /* 0x00 */ char name[8];
    /* 0x08 */ u8 level; /* level the player was last on */
    /* 0x09 */ u8 pad9;
    /* 0x0A */ u16 stars; /* rank points: 3 per gold, 2 per silver, 1 per bronze */
    /* 0x0C */ u8 title; /* index into D_802081C0 */
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14; /* money */
    /* 0x18 */ u8 rank[0x3C]; /* per level: 1-5 when done */
    /* 0x54 */ u8 pad54[0x91 - 0x54];
    /* 0x91 */ u8 unk91;
    /* 0x92 */ u8 unk92[0x3C]; /* per level: index into D_802E8C44 */
    /* 0xCE */ u8 padCE[0x100 - 0xCE];
} Player;

/* Menu entries (D_8020C070, 0x1C bytes), as in hd_code 00000.c */
typedef struct {
    /* 0x00 */ u16 unk0; /* flags */
    /* 0x02 */ s16 unk2; /* x */
    /* 0x04 */ s16 unk4; /* y */
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ char *unkC; /* title */
    /* 0x10 */ void *unk10; /* glyph list */
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 pad15[3];
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 pad1A[2];
} MenuEntry;

/* Menu pages (D_802F8BDC, 0x1C bytes), as in hd_code 00000.c */
typedef struct {
    u8 pad0[4];
    s16 unk4;
    u8 pad6[2];
    s32 unk8; /* flags */
    u8 padC[2];
    u16 unkE;
    u16 unk10;
    u8 pad12[6];
    s16 unk18;
    u8 pad1A[2];
} MenuPage;

/* Per-frame dynamic buffer (D_803156F8, two of 0x21498 bytes); only the
 * player select matrices are named here */
typedef struct PlayerSelDyn {
    /* 0x000 */ u8 pad0[0x80];
    /* 0x080 */ Mtx persp;
    /* 0x0C0 */ u8 padC0[0xC0];
    /* 0x180 */ Mtx lookAt;
    /* 0x1C0 */ u8 pad1C0[0x400];
    /* 0x5C0 */ Mtx trans;
    /* 0x600 */ u8 pad600[0x1E00 - 0x600];
    /* 0x1E00 */ Vtx vtx[16]; /* name entry boxes */
    /* 0x1F00 */ u8 pad1F00[0x21498 - 0x1F00];
} PlayerSelDyn;

extern Player D_80364AF0[];
extern u8 D_80364AE8;
extern LevelInfo D_802E8F94[];
extern s32 D_802E8BDC;
extern u64 D_80364A90;
extern u64 D_80364A98;
extern u32 D_80364AA8;
extern u8 D_80364A87;
extern u8 D_803643D5;
extern MenuEntry D_8020C070[];
extern MenuPage D_802F8BDC[];
extern OSMesgQueue D_80219EF8;
extern OSMesgQueue D_80219F50;
extern PlayerSelDyn D_803156F8[];
extern u8 D_80365060[]; /* per slot: 0 no save, 1 saved game, 2 new game */
extern char D_80215520[][25];
extern u8 D_8039C538;
extern u8 D_802154B0;
extern u16 D_803046F8[];
extern u16 D_80304710[];
extern u16 D_80304730[];
extern s16 D_802154D4;
extern f32 D_802154E0;
extern s32 D_802154EC;
extern s32 D_80215508[];
extern u8 D_802155A0[]; /* ticker text */
#define SCROLL_TEXT ((char *) D_802155A0)
extern u8 D_80364AEA;
extern u8 D_802E8BF8;
extern u16 D_80364EF0[][16]; /* per player: saved level times */
extern u8 D_802E8C44[];
extern char D_80215480[][16];
extern u16 *D_802158A0;
extern u8 D_8039C53C[]; /* per slot: 1 + level to save, 0 = nothing pending */
extern u8 D_8039C540;
extern s16 yoshiState;
extern void *D_80367738;

extern Vtx D_80208380[];
extern Gfx D_80208400[];
extern Lights2 D_80208448;
extern u16 D_8021591C;
extern s16 D_8021593C;
extern s32 D_80215458;
extern u8 D_80215470[];
extern u8 D_80215915;
extern u8 D_80215916;
extern s16 D_802154B2;
extern s16 D_802154B4;
extern s16 D_802154B6;
extern s16 D_802154B8;
extern s16 D_802154BA;
extern u8 D_802154BC;
extern s16 D_802154BE;
extern s16 D_802154C0;
extern s32 D_802154C4;
extern s32 D_802154C8;
extern s32 D_802154CC;
extern u8 D_802154D0;
extern s16 D_80215918;
extern s16 D_8021591A;
extern s32 D_80215920;
extern u8 D_80215924;
extern char *D_80215928;
extern u16 D_80370C28; /* buttons held */
extern u16 D_80370C2A; /* buttons held last frame */
extern s8 D_80370C2C;  /* stick x */
extern s16 D_8036BB20;
extern u8 D_802F47B0[];
extern u32 D_803156C4;
extern f32 D_80215440;
extern f32 D_80215444;
extern f32 D_80215448;
extern f32 D_8021544C;
extern f32 D_80215450;
extern f32 D_80215454;
extern f32 D_8021545C;
extern f32 D_80215460;
extern f32 D_80215464;
extern f32 D_80215468;
extern s16 D_8021546C;
extern s32 D_80215940;
extern f32 D_80215944;
extern f32 D_80215948;
extern f32 D_8021594C;
extern f32 D_80215950;
extern s16 D_8021592C;

extern s16 D_802154D2;
extern s32 D_802154DC;
extern u8 D_80215900[];
extern u8 D_80215902[];
extern s32 D_80215908[];
extern s16 D_80215910[];
extern u8 D_80215914;
extern u16 D_80215930[];
extern u16 D_802158A8[];
extern u16 D_802E8C94[];
extern u16 D_802E8C98[];
extern u8 D_8021592E;
extern s32 D_802154D8;
extern f32 D_802154E4;
extern s32 D_802154E8;
extern s32 D_802154F0[];
extern u16 D_802082D8[];
extern u16 D_802082E4[];
extern u16 D_802082E8[];

void func_801E93DC(u8 arg0);
void func_801ED480(u8 *src, u8 *dst);
u16 func_801E9528(void);
void func_801EA268(Player *p);
Gfx *func_801EC49C(Gfx *arg0, s32 x, s32 y, u8 slot);

/* Highest rank shown: 4 once unk91 reaches 12, else 3 */
#define MAX_RANK() ((D_80364AF0[D_80364AE8].unk91 >= 12) ? 4 : 3)

/* .data (0x802081C0). The u16 pointers are hd_code data (glyph strings and
 * portraits) that have no symbols yet. */
typedef struct {
    char *title;
    u16 *glyphs;
} RankTitle;

RankTitle D_802081C0[31] = {
    { "ROOKIE WRECKER", (u16 *) 0x803041DC },
    { "TRAINED CRUSHER", (u16 *) 0x803041EC },
    { "EXPERIENCED RAVAGER", (u16 *) 0x803041FC },
    { "DECORATED DAMAGER", (u16 *) 0x8030420C },
    { "PROFESSIONAL RAZER", (u16 *) 0x8030421C },
    { "EXPERT DESTROYER", (u16 *) 0x8030422C },
    { "GIFTED RUINER", (u16 *) 0x8030423C },
    { "ACCOMPLISHED CONQUEROR", (u16 *) 0x8030424C },
    { "MASTER DESPOILER", (u16 *) 0x8030425C },
    { "DEMOLITION FANATIC", (u16 *) 0x8030426C },
    { "GRAND ERADICATOR", (u16 *) 0x8030427C },
    { "HEAVY DUTY WASTER", (u16 *) 0x80304288 },
    { "TOTAL PULVERISER", (u16 *) 0x8030429C },
    { "CHAMPION RANSACKER", (u16 *) 0x803042AC },
    { "MECHANICAL MAESTRO", (u16 *) 0x803042B8 },
    { "CHIEF OBLITERATOR", (u16 *) 0x803042CC },
    { "COMMANDING DESOLATOR", (u16 *) 0x803042E0 },
    { "SUPREME DEVASTATOR", (u16 *) 0x803042F0 },
    { "ULTIMATE ANNIHILATOR", (u16 *) 0x80304304 },
    { "LEVELING LEGEND", (u16 *) 0x8030430C },
    { "DESTRUCTIVE PSYCHOPATH", (u16 *) 0x80304318 },
    { "MINDLESS DESECRATOR", (u16 *) 0x80304328 },
    { "HYSTERICAL CLAUSTROPHOBE", (u16 *) 0x80304334 },
    { "UNCONTROLLABLE MADMAN", (u16 *) 0x80304344 },
    { "WORLD CLASS MEGALOMANIAC", (u16 *) 0x80304358 },
    { "CAPTAIN OF CARNAGE", (u16 *) 0x80304364 },
    { "SINGLE MINDED CHAOSMONGER", (u16 *) 0x80304370 },
    { "GRAND HIGH SLAUGHTERMASTER", (u16 *) 0x8030437C },
    { "LUNATIC LORD OF HAVOC", (u16 *) 0x80304388 },
    { "ARMAGEDDON ADEPT", (u16 *) 0x80304394 },
    { "YOU CAN STOP NOW.", (u16 *) 0x8030439C },
};

u8 D_802082B8[0x20] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 1, 1, 0,
};

/* sprite ids for func_80272C5C */
u16 D_802082D8[6] = { 0x777, 0x777, 0x776, 0x773, 0, 0 };
u16 D_802082E4[2] = { 0x774, 0x775 };
u16 D_802082E8[2] = { 0x576, 0x575 };
u16 D_802082EC[6] = { 0x91B, 0x91B, 0x91C, 0x91D, 0x572, 0 };
u16 D_802082F8[2] = { 0x91F, 0 };

/* name entry character sets and their lengths */
u8 D_802082FC[2][12] = { "1234/.\x7F", "0123456789/\x7F" };
u8 D_80208314[2][26] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "BCDFGHJKLMNPQRSTVWXYZ" };
s32 D_80208348[2] = { 7, 12 };
u32 D_80208350[2] = { 26, 21 };

/* Set up the player select screen for a player: sprites and both frames' matrices */
void func_801E8C40(u8 arg0) {
    PlayerSelDyn *dyn;
    s32 i;

    D_80364AE8 = arg0;
    D_80215915 = func_80272C5C(D_802082F8, NULL, 1, 1, 1, 1.0f);
    D_80215916 = func_80272C5C(D_802082EC, NULL, 5, 1, 0, 1.0f);
    for (i = 0; i < 2; i++) {
        dyn = &D_803156F8[i];
        guPerspective(&dyn->persp, &D_8021591C, 45.0f, 4.0f / 3.0f, 10.0f, 10000.0f, 1.0f);
        guLookAt(&dyn->lookAt, 0.0f, 277.0f, 480.0f, 0.0f, 189.0f, 200.0f, 0.0f, 0.0f, 1.0f);
        guTranslate(&dyn->trans, 0.0f, 88.0f, 0.0f);
    }
    D_8021593C = 0;
    func_801E8DCC(D_80364AE8);
}

void func_801E8DCC(u8 arg0) {
    s32 i;

    func_801E8EB8(arg0, 0);
    for (i = 0; i < 0x1B; i++) {
        D_802158A8[i] = D_802E8C94[D_8021592E];
    }
    D_802158A8[i] = D_802E8C98[D_8021592E];
    D_802154D8 = 0;
    D_802154E8 = 9999;
    for (i = 1; i < 5; i++) {
        D_802154F0[i] = 9999;
    }
    D_802154E4 = 3.0f;
}

#define PLAYER_ASSERT(EX, line) \
    if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "player.c", line)
#define sslen D_802154D2
#define TOTAL_SCROLL_LENGTH 256

/* Build the player select ticker text for a slot (4 = guest/new): the
 * biography in mode 0x1000000000000, else name, title, rank counts and money.
 * The local arrays' templates are .data 0x80208358/0x80208368 and sPrefix is
 * 0x80208378 (IDO puts function statics after the templates). */
void func_801E8EB8(u8 slot, u8 arg1) {
    s32 i;
    Player *p = &D_80364AF0[slot];
    u8 guest = 0;
    static char *sPrefix[2] = { "", "GUEST: " };
    char *bios[4] = {
        ".................... LEADER OF THE ARMY BASE WALKOUT YEARS AGO. AMBER'S SHARP MIND AND BRIGHT, SELFLESS "
        "OUTLOOK MAKE HER THE NEAREST THING BLAST CORPS HAS TO A LEADER ....................",
        ".................... A GENIUS IN HEAVY VEHICLE DESIGN. WHILE SOMETIMES OVERLY POSSESSIVE OF HIS CREATIONS, "
        "CLARK HAS TALENTS VITAL TO BLAST CORPS' SURVIVAL AND SUCCESS ....................",
        ".................... HEAD MECHANIC OF THE BLAST CORPS TEAM. WITH YEARS OF EXPERIENCE AND A GRUFF PRIDE IN "
        "HIS WORK, SPIKE ENSURES THAT THE DOZERS ARE BUILT TO PERFECTION ..................",
        ".................... A FEARLESS ARMY DAREDEVIL UNTIL HIS DISABLING ACCIDENT. WESLEY'S REJECTION BY HIS "
        "SUPERIORS TRIGGERED THE REBELLION THAT LED TO THE RISE OF BLAST CORPS ...............",
    };
    u16 *pics[4] = { (u16 *) 0x803043B8, (u16 *) 0x80304474, (u16 *) 0x80304544, (u16 *) 0x80304614 };

    if (!(D_80364A90 & 0x10E18000) && slot != D_80364AEA) {
        guest = 1;
    }
    D_802154EC = -1;
    for (i = 1; i < 5; i++) {
        D_80215508[i] = -1;
    }
    if (slot < 4) {
        func_801E93DC(slot);
    }
    if (D_80364A98 == 0x1000000000000) {
        D_802158A0 = NULL;
        sprintf(SCROLL_TEXT, "%s", bios[slot]);
    } else {
        D_802158A0 = NULL;
        if (slot < 4) {
            if (D_80365060[slot] == 1) {
                sprintf(SCROLL_TEXT, " ..... %s%s (%s) ... ", sPrefix[guest], p->name, D_802081C0[p->title].title);
                for (i = (D_80364AF0[slot].unk91 >= 12) ? 4 : 3; i > 0; i--) {
                    D_80215508[i] = func_8025B300(SCROLL_TEXT);
                    if (i != 1) {
                        sprintf(SCROLL_TEXT, "%s  %d .. ", SCROLL_TEXT, D_80215930[i]);
                    }
                }
                sprintf(SCROLL_TEXT, "%s  %d ... ", SCROLL_TEXT, D_80215930[1]);
                if (D_802E8BF8 == 0) {
                    sprintf(SCROLL_TEXT, "%s$%d ... ", SCROLL_TEXT, p->unk14);
                }
                D_802154EC = func_8025B300(SCROLL_TEXT);
                sprintf(SCROLL_TEXT, "%s  %d", SCROLL_TEXT, p->title);
                if ((D_80364A98 & 0x0200040000000000) || (D_80364A90 & 0x0100000000000000)) {
                    sprintf(SCROLL_TEXT, "%s ..... %s", SCROLL_TEXT, "USE Z/R TO CHANGE PLAYER, THEN A TO SELECT!");
                }
            } else {
                sprintf(SCROLL_TEXT, " ... NEW GAME");
            }
        } else {
            sprintf(SCROLL_TEXT, " ");
        }
    }
    if (D_802158A0 != NULL) {
        D_802154D2 = func_8025B370(D_802158A0);
        D_8021592E = 1;
        D_80215458 = 0x13;
        if (slot == 4) {
            D_802154D4 = 0x16;
        } else {
            D_802154D4 = 0xE;
        }
    } else {
        D_802154D2 = func_8025B300(SCROLL_TEXT);
        D_8021592E = 0;
        D_80215458 = 0xC;
        if (slot == 4) {
            D_802154D4 = 0x24;
        } else {
            D_802154D4 = 0x16;
        }
    }
    PLAYER_ASSERT(sslen<TOTAL_SCROLL_LENGTH, 400);
    D_802154DC = -1;
    if (arg1 || slot == 4) {
        D_802154E0 = 12.0f;
    } else {
        D_802154E0 = 8.0f;
    }
}

/* player select backdrop: two lit quads (.data after func_801E8EB8's) */
Vtx D_80208380[8] = {
    { { { -160, 174, 180 }, 0, { 0, 0 }, { 0x00, 0x81, 0x00, 0x28 } } },
    { { { 160, 174, 180 }, 0, { 0, 0 }, { 0x00, 0x81, 0x00, 0x28 } } },
    { { { 160, 204, 180 }, 0, { 0, 0 }, { 0x00, 0x7F, 0x1E, 0x28 } } },
    { { { -160, 204, 180 }, 0, { 0, 0 }, { 0x00, 0x7F, 0x1E, 0x28 } } },
    { { { -160, 198, 200 }, 0, { 0, 0 }, { 0x5A, 0x5A, 0x00, 0xB4 } } },
    { { { -160, 180, 200 }, 0, { 0, 0 }, { 0x5A, 0xA6, 0x00, 0xB4 } } },
    { { { 160, 180, 200 }, 0, { 0, 0 }, { 0x5A, 0xA6, 0x00, 0xB4 } } },
    { { { 160, 198, 200 }, 0, { 0, 0 }, { 0x5A, 0x5A, 0x00, 0xB4 } } },
};

Gfx D_80208400[] = {
    gsSPVertex(D_80208380, 8, 0),
    gsDPPipeSync(),
    gsSP1Triangle(0, 5, 1, 0),
    gsSP1Triangle(5, 1, 6, 0),
    gsSP1Triangle(4, 5, 6, 0),
    gsSP1Triangle(4, 6, 7, 0),
    gsSP1Triangle(4, 3, 7, 0),
    gsSP1Triangle(3, 7, 2, 0),
    gsSPEndDisplayList(),
};

Lights2 D_80208448 = gdSPDefLights2(0x28, 0x0A, 0x0A, 0xF0, 0xC8, 0x14, 69, -69, 69, 0xF0, 0x6E, 0x14, -69, 69, 69);
Lights2 D_80208470 = gdSPDefLights2(0x28, 0x02, 0x21, 0x5A, 0x02, 0xDC, 69, -69, 69, 0x5A, 0x02, 0xDC, -69, 69, 69);

u8 D_80208498[4] = " "; /* one-character string for the name entry wheel */

/* Count the player's ranks per grade (D_80215930[1..4]) and the finished
 * levels of the 0x81 types, other than levels 0x26, 0x2F and 0x31 ([3]) */
void func_801E93DC(u8 arg0) {
    s32 i;
    Player *p;

    p = &D_80364AF0[arg0];
    for (i = 0; i < 6; i++) {
        D_80215930[i] = 0;
    }
    for (i = 0; i < 0x3C; i++) {
        if (p->rank[i] > 0 && p->rank[i] < 5) {
            D_80215930[p->rank[i]]++;
        }
        if (((D_80364AF0[arg0].rank[i] > 0 && D_80364AF0[arg0].rank[i] < 6) ? 1 : 0) && (D_802E8F94[i].type & 0x81) &&
            i != 0x31 && i != 0x2F && i != 0x26) {
            D_80215930[3]++;
        }
    }
}

/* Advance the name ticker by one character and return the next glyph */
u16 func_801E9528(void) {
    s32 i;

    D_802154DC = (D_802154DC + 1) % D_802154D2;
    if (D_802154DC == D_802154EC) {
        D_802154E8 = 0;
    }
    for (i = 1; i <= MAX_RANK(); i++) {
        if (D_80215508[i] == D_802154DC) {
            D_802154F0[i] = 0;
        }
    }
    if (D_802154D4 != 0) {
        D_802154D4--;
        if (D_802154D4 == 0) {
            D_802154E0 = 3.0f;
        }
    }
    D_802154E4 = (D_802154E0 - D_802154E4) * 0.2 + D_802154E4;
    if (D_802158A0 != NULL) {
        return D_802158A0[D_802154DC];
    }
    return D_802155A0[D_802154DC];
}

s32 func_801E96F8(void) {
    return D_802154D2 == D_802154DC + 8;
}

/* Player select scene: lit backdrop fading with L/R (D_8021593C), the
 * scrolling vehicle icons and the scrolling name ticker */
Gfx *func_801E9718(Gfx *arg0, PlayerSelDyn *dyn, s32 arg2) {
    Gfx *gdl = arg0;

    gSPMatrix(gdl++, &dyn->persp, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->lookAt, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(gdl++, &dyn->trans, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gImmp1(gdl++, G_RDPHALF_1, D_8021591C); /* old gSPPerspNormalize */
    gDPPipeSync(gdl++);
    gSPTexture(gdl++, 0x7C0, 0x7C0, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_LIGHTING | G_SHADING_SMOOTH | G_SHADE);
    {
        s32 pad[4]; /* four unused stack slots sit here */

        gSPSetLights2(gdl++, D_80208448);
    }
    gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
    {
        s32 x;
        s32 i;

        if (D_80364A90 & 0x0005040008010080) {
            D_8021593C += 0x10;
            if (D_8021593C >= 0x100) {
                D_8021593C = 0xFF;
            }
        } else if (D_80364A90 & 0x0008000202020000) {
            D_8021593C -= 0x10;
            if (D_8021593C <= 0) {
                D_8021593C = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            D_80208380[i].v.cn[3] = (D_8021593C * 40) / 255;
        }
        for (i = 4; i < 7; i++) {
            D_80208380[i].v.cn[3] = (D_8021593C * 180) / 255;
        }
        osWritebackDCache(D_80208380, 0x80);
        if (D_8021593C != 0) {
            gSPDisplayList(gdl++, D_80208400);
        }
        gDPPipeSync(gdl++);
        gSPClearGeometryMode(gdl++, G_LIGHTING);
        D_802154D8 += D_802154E4;
        if (D_802154D8 >= D_80215458) {
            D_802154D8 -= D_80215458;
            for (i = 1; D_802158A8[i] != D_802E8C98[D_8021592E]; i++) {
                D_802158A8[i - 1] = D_802158A8[i];
            }
            i--;
            D_802158A8[i] = func_801E9528();
            D_802158A8[i + 1] = D_802E8C98[D_8021592E];
        }
        gdl = func_80274868(gdl);
        D_802154E8 += D_802154E4;
        x = 0x136 - D_802154E8;
        if (x >= -0x1F && x < 0x140) {
            gdl = func_80272ED8(gdl, D_80215915, x, 0xC2, D_8021593C, 1, 1.0f);
        }
        for (i = 1; i < 5; i++) {
            D_802154F0[i] += D_802154E4;
            x = 0x136 - D_802154F0[i];
            if (x >= -0x1F && x < 0x140) {
                gdl = func_80272ED8(gdl, D_80215916 + i, x, 0xC2, D_8021593C, 1, 1.0f);
            }
        }
        gdl = func_80274AA4(gdl);
        func_80259CCC(dyn, (D_8021592E == 1) ? NULL : func_8025B558(D_802158A8), (D_8021592E == 1) ? D_802158A8 : NULL,
                      0, 0, (-D_802154D8 % D_80215458) - 3, 0xC9, 0x14, 0x14, 1, 0, 0, 0,
                      (D_8021593C / 2 - 0x1B < 0) ? 0 : D_8021593C / 2 - 0x1B);
        func_80259DC8(dyn, (D_8021592E == 1) ? NULL : func_8025B558(D_802158A8), (D_8021592E == 1) ? D_802158A8 : NULL,
                      0, 0, -D_802154D8 % D_80215458, 0xC7, 0x14, 0x14, 1, 0xFF, 0xFF, 0xFF, D_8021593C, 0xFF, 0xFF,
                      0xFF, D_8021593C);
        gDPPipeSync(gdl++);
    }
    return gdl;
}

/* Reset a save slot to a fresh "NEW GAME" player and tell the save thread */
void func_801EA108(u8 slot, u8 send, u8 newGame) {
    Player *p;
    u32 i;

    p = &D_80364AF0[slot];
    for (i = 0; i < 0x100; i++) {
        ((u8 *) p)[i] = 0;
    }
    func_801EA268(p);
    sprintf(p->name, "%s", "NEW GAME");
    D_80365060[slot] = 2;
    osSendMesg(&D_80219EF8, (OSMesg) ((slot << 16) | 7), OS_MESG_BLOCK);
    if (send) {
        osSendMesg(&D_80219EF8, (OSMesg) ((newGame ? 0x14 : 0x15) | (slot << 16) | 0x1000000), OS_MESG_BLOCK);
        osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
    } else {
        osSendMesg(&D_80219EF8, (OSMesg) ((newGame ? 0x14 : 0x15) | (slot << 16)), OS_MESG_BLOCK);
    }
}

void func_801EA268(Player *p) {
    p->unk10 = 0x1063E;
}

/* Load the four save slots through the save thread and build their menu labels */
void func_801EA278(void) {
    s32 i;
    s32 msg;

    for (i = 0; i < 4; i++) {
        osSendMesg(&D_80219EF8, (OSMesg) ((i << 16) | 6 | 0x1000000), OS_MESG_BLOCK);
        osRecvMesg(&D_80219F50, (OSMesg *) &msg, OS_MESG_BLOCK);
        if (msg == 0) {
            if (func_8025B3F0(D_80364AF0[i].name, "NEW GAME")) {
                D_80365060[i] = 1;
            } else {
                D_80365060[i] = 2;
            }
        } else if (msg != 0x6E382) {
            if (i < D_8039C538) {
                osSendMesg(&D_80219EF8, (OSMesg) ((i << 16) | 3 | 0x1000000), OS_MESG_BLOCK);
                osRecvMesg(&D_80219F50, (OSMesg *) &msg, OS_MESG_BLOCK);
            }
            if (msg != 0 || i >= D_8039C538) {
                D_80365060[i] = 0;
                sprintf(D_80215520[i], "%d : %s", i + 1, "PAK FULL");
            } else {
                func_801EA108(i, 1, 0);
            }
        } else {
            func_801EA108(i, 1, 0);
        }
        if (D_80365060[i] == 1 || D_80365060[i] == 2) {
            sprintf(D_80215520[i], "%d : %s", i + 1, D_80364AF0[i].name);
        }
    }
}

/* Build the "load game" slot menu (entries 2-5, then ERASE GAME / IGNORE PAK) */
void func_801EA4B8(void) {
    s32 i;

    func_801EA278();
    D_802154B0 = 0;
    for (i = 0; i < 4; i++) {
        D_8020C070[i + 2].unkC = D_80215520[i];
        D_8020C070[i + 2].unk0 |= 0x81;
        D_8020C070[i + 2].unk0 &= ~0x20;
        D_8020C070[i + 2].unk18 = 7;
        D_8020C070[i + 2].unk19 = 4;
        D_8020C070[i + 2].unk2 = 0x50;
        /* an empty then-block: needed for i to be reloaded in the else */
        if (D_80365060[i] != 0) {
        } else {
            D_8020C070[i + 2].unk0 &= ~0x81;
            D_8020C070[i + 2].unk18 = 8;
            D_802154B0++;
        }
    }
    if (D_802154B0 != 4) {
        D_8020C070[6].unkC = "ERASE GAME";
        D_8020C070[6].unk10 = D_803046F8;
    } else {
        D_8020C070[6].unkC = "IGNORE PAK";
        D_8020C070[6].unk10 = D_80304710;
    }
    D_8020C070[6].unk19 = 2;
    D_802F8BDC[10].unk8 &= ~0x400;
}

/* Build the "erase game" slot menu */
void func_801EA6E8(void) {
    s32 i;

    func_801EA278();
    for (i = 0; i < 4; i++) {
        D_8020C070[i + 2].unk2 = 0x40;
        switch (D_80365060[i]) {
            case 1:
                sprintf(D_80215520[i], "ERASE %d : %s", i + 1, D_80364AF0[i].name);
                D_8020C070[i + 2].unk0 |= 0x81;
                D_8020C070[i + 2].unk0 &= ~0x20;
                D_8020C070[i + 2].unk18 = 7;
                D_8020C070[i + 2].unk19 = 4;
                break;
            case 0:
            case 2:
                D_8020C070[i + 2].unk18 = 8;
                D_8020C070[i + 2].unk0 |= 0x20;
                D_8020C070[i + 2].unk0 &= ~0x81;
                D_8020C070[i + 2].unk18 = 8;
                break;
        }
    }
    D_8020C070[6].unkC = "GO BACK";
    D_8020C070[6].unk10 = D_80304730;
    D_8020C070[6].unk19 = 2;
    D_802F8BDC[10].unk8 |= 0x400;
    D_802F8BDC[10].unk18 = 6;
}

/* Open the name entry screen (menu entries 7 and 8) on buf */
void func_801EA93C(char *title, u16 *glyphs, u8 arg2, u8 width, char *buf) {
    D_802154B2 = 0x7FFF;
    D_802154B4 = 0x7FFF;
    D_80215918 = 0;
    D_802154B8 = 0;
    D_802154B6 = 0;
    D_802154BA = -1;
    D_802154BC = 0;
    D_802154BE = 0;
    D_802154C0 = 1;
    D_8020C070[7].unk6 = width;
    D_8020C070[7].unk8 = width;
    D_802154CC = width;
    D_802154C8 = (width * 3) / 5;
    D_80215928 = buf;
    *buf = 0;
    D_8020C070[7].unkC = D_80215928;
    D_802154C4 = 0xA0 - D_802154C8 / 2;
    D_8020C070[7].unk2 = D_802154C4;
    D_8020C070[8].unkC = title;
    D_8020C070[8].unk10 = glyphs;
    D_80215924 = arg2;
    D_802154D0 = 1;
    D_8021591A = 0;
    D_80215920 = 1;
    D_8021592C = 0;
}

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define FABS(x) ((x) > 0.0f ? (x) : -(x))

/* Name entry screen: the wheel of 33 characters turned with the stick,
 * the picked character flying into the name, B to delete, START to accept */
Gfx *func_801EAA7C(Gfx *arg0, PlayerSelDyn *dyn, s32 *count) {
    Gfx *gdl = arg0;
    MenuEntry *e = &D_8020C070[7];
    u8 ch;
    s32 i;
    s32 pass;
    s16 diff;
    s32 x;
    s32 y;
    s32 selX;
    s32 selY;
    s32 selW;
    s32 selH;
    s32 nv = 0;
    s32 len = func_8025B300(D_80215928);
    u8 *col = &D_802F47B0[0x80];
    u8 *col2;
    s16 ang;
    s16 ang2;
    u8 alpha;

    if ((D_80370C28 & 0x300) && !(D_80370C2A & 0x300)) {
        func_80260650(D_80367738, 0xC, 0);
    }
    D_8021546C = D_8021591A;
    D_8021591A = D_80370C2C;
    if (ABS(D_8021591A) < 10) {
        D_8021591A = 0;
    } else {
        D_8021591A *= ABS(D_8021591A);
    }
    if (D_8021591A != 0) {
        if (D_8021546C == 0) {
            D_80215918 = D_802154B6;
        }
        D_802154B4 -= D_8021591A >> 3;
    } else {
        if (D_8021546C != 0 && D_802154B6 == D_80215918) {
            D_802154B6 += (D_8021546C >= 0) ? 1 : -1;
            if (D_802154B6 < 0) {
                D_802154B6 += 0x21;
            }
            if (D_802154B6 >= 0x21) {
                D_802154B6 -= 0x21;
            }
            D_802154B2 = 0x7FFF - D_802154B6 * 0x7C2;
        }
        diff = D_802154B4 - D_802154B2;
        D_802154B4 -= diff >> 4;
    }
    D_802154B6 = (0x83E0 - D_802154B4) / 0x7C2;
    if (D_802154B6 == 0x21) {
        D_802154B6 = 0;
    }
    if (D_8021591A != 0) {
        D_802154B2 = 0x7FFF - D_802154B6 * 0x7C2;
    }
    if (D_802154D0 != 0) {
        D_802154B8 = D_802154B6;
        D_802154D0 = 0;
    }
    if (D_802154B6 != D_802154B8) {
        func_80260650(D_80367738, 0x90, 0);
    }
    D_802154B8 = D_802154B6;
    for (pass = 0; pass < 2; pass++) {
        for (i = 0; i < 0x21; i++) {
            ang = D_802154B4 + i * 0x7C2 - 0x6000;
            ang2 = ang - 0x2000;
            ang2 = ang2 + 0x7FFF;
            ang2 = ang2 + 0x2000;
            ang2 = ang2 + 0x7FFF;
            x = sins(ang2) * 120.0 / 32768.0 - 7.0;
            y = coss(ang2) * 96.0 / 32768.0 - 10.0;
            if (i < D_80208350[0]) {
                D_80208498[0] = D_80208314[0][i];
            } else {
                D_80208498[0] = D_802082FC[0][i - D_80208350[0]];
            }
            if (D_80208498[0] != D_802154BA) {
                if (pass != 0) {
                    if (D_802154B6 == i) {
                        ch = D_80208498[0], selX = x, selY = y, selW = 0x18, selH = 0x14;
                    } else {
                        func_80259DC8(dyn, D_80208498, 0, 1, 0, x, y, 0x18, 0x14, 1, 0xC8, 0xC8, 0xC8, D_8036BB20, 0xFF,
                                      0xFF, 0xFF, D_8036BB20);
                    }
                } else if (D_802154B6 == i) {
                    func_80259CCC(dyn, D_80208498, 0, 1, 0, x - 0xB, y, 0x30, 0x28, 1, 0, 0, 0, D_8036BB20 / 2);
                } else {
                    func_80259CCC(dyn, D_80208498, 0, 1, 0, x - 4, y + 3, 0x18, 0x14, 1, 0, 0, 0, D_8036BB20 / 2);
                }
            }
        }
    }
    if (ch != D_802154BA && D_802154BA == -1) {
        if (D_803156C4 % 20 < 15) {
            alpha = D_8036BB20;
        } else {
            alpha = D_8036BB20 / 3;
        }
        D_80208498[0] = ch;
        func_80259BD4(&gdl, dyn);
        func_80259DC8(dyn, D_80208498, 0, 1, 0, selX - 5, selY - 5, selW * 2, selH * 2, 1, col[0], col[1], col[2],
                      alpha, col[4], col[5], col[6], alpha);
    }
    if (yoshiState == 2 && (D_80370C28 & 0x1000) && !(D_80370C2A & 0x1000) && func_802753C0() == 0) {
        if (D_802154BC != 0) {
            D_80364A98 = 0x1000000;
            D_80365060[D_80364AE8] = 1;
            func_8026AF6C(0x4000);
        } else {
            func_80260650(D_80367738, 0x2B, 0);
        }
    }
    switch (D_802154BE) {
        case 0:
            if (yoshiState == 2 && (D_80370C28 & 0xC000) && !(D_80370C2A & 0xC000) && func_802753C0() == 0) {
                if (D_80370C28 & 0x4000) {
                    ch = 0x7F;
                }
                if (ch == 0x7F) {
                    if (D_802154BC == 0) {
                        if (D_80370C28 & 0x4000) {
                            if (D_802E8BF8 != 0) {
                                func_80275270(0x0400000000000000, 0.5f);
                            } else {
                                D_80364A98 = 0x200000;
                            }
                            func_80260650(D_80367738, 0xDE, 0);
                            func_8026AF6C(0x4000);
                        } else {
                            func_80260650(D_80367738, 0x2B, 0);
                        }
                    } else {
                        D_802154BC--;
                        D_80215928[D_802154BC] = 0;
                        func_80260650(D_80367738, 0xE4, 0);
                        D_802154C0 = 0;
                        D_80215464 = e->unk6;
                        i = D_80215468 = D_802154CC;
                        i = i * 3 / 5;
                        D_80215450 = e->unk2;
                        D_80215454 = -((len - 1) * i / 2) - i / 2 + 0xA0;
                        D_8021545C = D_802154C4;
                        D_80215460 = (len - 1) * i / 2 - i / 2 + 0xA0;
                    }
                } else if (D_802154BC < D_80215924) {
                    func_80260650(D_80367738, 0x91, 0);
                    D_802154BE = 1;
                    D_802154C0 = 0;
                    D_802154BA = ch;
                    D_80215464 = e->unk6;
                    i = D_80215468 = D_802154CC;
                    i = i * 3 / 5;
                    D_80215450 = e->unk2;
                    D_80215454 = -((len + 1) * i / 2) - i / 2 + 0xA0;
                    D_8021545C = D_802154C4;
                    D_80215460 = (len + 1) * i / 2 - i / 2 + 0xA0;
                    D_80215944 = selX - 5;
                    D_80215948 = selY - 5;
                    D_8021594C = D_80215460 - 160.0f - i;
                    D_80215950 = e->unk4 - 0x78;
                    D_80215440 = 48.0f;
                    D_80215444 = 40.0f;
                    D_80215448 = D_80215468;
                    D_8021544C = D_80215468;
                } else {
                    func_80260650(D_80367738, 0x2B, 0);
                }
            }
            break;
        case 1:
            D_80215944 += (D_8021594C - D_80215944) * 0.2;
            D_80215948 += (D_80215950 - D_80215948) * 0.2;
            D_80215440 += (D_80215448 - D_80215440) * 0.2;
            D_80215444 += (D_8021544C - D_80215444) * 0.2;
            D_80208498[0] = D_802154BA;
            func_80259CCC(dyn, D_80208498, 0, 0, 0, D_80215944 - 4.0f, D_80215948 + 4.0f, D_80215440, D_80215444, 1, 0,
                          0, 0, D_8036BB20 / 2);
            func_80259DC8(dyn, D_80208498, 0, 0, 0, D_80215944, D_80215948, D_80215440, D_80215444, 1, col[0], col[1],
                          col[2], D_8036BB20, col[4], col[5], col[6], D_8036BB20);
            if (FABS(D_80215944 - D_8021594C) < 0.15 && FABS(D_80215948 - D_80215950) < 0.15) {
                func_80260650(D_80367738, 1, 0);
                D_802154BE = 0;
                D_802154C0 = 0;
                D_80215928[D_802154BC] = D_80208498[0];
                D_802154BC++;
                D_80215928[D_802154BC] = 0;
                D_802154BA = -1;
            }
            break;
    }
    func_80259BD4(&gdl, dyn);
    D_80215940 += D_80215920;
    if (D_80215920 < 0) {
        D_80215940 += D_80215920 * 2;
    }
    if (D_80215940 < 0 || D_80215940 >= 0x10) {
        D_80215940 -= D_80215920 * 2;
        D_80215920 = -D_80215920;
    }
    if (yoshiState == 2) {
        D_8021592C = (s32) ((D_8021592C + 0x10 < 0xFF) ? D_8021592C + 0x10 : 0xFF);
    } else {
        D_8021592C = 0;
    }
    col2 = &D_802F47B0[0x98];
    nv = func_80276130(dyn, 3, nv, D_802154C4 + D_802154C8 + D_80215940, e->unk4 + e->unk8 / 2,
                       e->unk6 / 3 + D_80215940 / 2, e->unk8 / 2 + 3, D_802F47B0[0x98], D_802F47B0[0x99],
                       D_802F47B0[0x9A], D_8021592C, D_802F47B0[0x9C], D_802F47B0[0x9D], D_802F47B0[0x9E], D_8021592C,
                       D_802F47B0[0x98], D_802F47B0[0x99], D_802F47B0[0x9A], D_8021592C, D_802F47B0[0x9C],
                       D_802F47B0[0x9D], D_802F47B0[0x9E], D_8021592C);
    nv = func_80276080(dyn, 3, nv, D_802154C4 + D_802154C8 + D_80215940 + 4, e->unk4 + e->unk8 / 2 + 3,
                       e->unk6 / 3 + D_80215940 / 2, e->unk8 / 2 + 3, 0, 0, 0, D_8021592C / 2);
    gdl = func_80275DA4(gdl, 0);
    gSPVertex(gdl++, &dyn->vtx[0], 8, 0);
    gSP1Triangle(gdl++, 4, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 6, 7, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    nv = func_80276130(dyn, 2, nv, e->unk2 - D_80215940 - 4, e->unk4 + e->unk8 / 2, e->unk6 / 3 + D_80215940 / 2,
                       e->unk8 / 2 + 3, col2[0], col2[1], col2[2], D_8021592C, col2[4], col2[5], col2[6], D_8021592C,
                       col2[0], col2[1], col2[2], D_8021592C, col2[4], col2[5], col2[6], D_8021592C);
    nv = func_80276080(dyn, 2, nv, e->unk2 - D_80215940 - 8, e->unk4 + e->unk8 / 2 + 3, e->unk6 / 3 + D_80215940 / 2,
                       e->unk8 / 2 + 3, 0, 0, 0, D_8021592C / 2);
    gdl = func_80275DA4(gdl, 0);
    gSPVertex(gdl++, &dyn->vtx[nv - 8], 8, 0);
    gSP1Triangle(gdl++, 4, 5, 6, 0);
    gSP1Triangle(gdl++, 4, 6, 7, 0);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 0, 2, 3, 0);
    switch (D_802154C0) {
        case 0:
            D_80215450 += (D_80215454 - D_80215450) * 0.2;
            e->unk2 = D_80215450;
            D_8021545C += (D_80215460 - D_8021545C) * 0.2;
            D_802154C4 = D_8021545C;
            break;
        case 1:
            break;
    }
    *count += gdl - arg0;
    return gdl;
}

void func_801EC288(u8 arg0) {
    s32 i;

    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    for (i = 0; i < 2; i++) {
        D_80215900[i] = 4;
        D_80215908[i] = 0x32;
        D_80215910[i] = 0;
    }
}

void func_801EC30C(u8 arg0) {
    s32 j;
    s32 i;

    D_80215914 = func_80272C5C(D_802082E4, D_802082D8, 4, 2, 0, 1.0f);
    func_80272C5C(D_802082E8, 0, 1, 2, 1, 1.0f);
    D_80215902[0] = 3;
    D_80215902[1] = arg0;
    if (D_802E8F94[D_802E8BDC].type == 1) {
        j = 0;
    } else {
        j = 1;
    }
    for (i = 0; j < 2; j++, i++) {
        D_80215900[j] = 0;
        D_80215908[j] = i * 60 + 0x28;
        D_80215910[j] = 0;
    }
}

void func_801EC464(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        D_80215900[i] = 3;
    }
}

/* Draw a save slot's rank badge, stepping its state: 0/4 count down, 1 fade
 * in, 2 shown, 3 fade out (D_80215900/08/10[slot]) */
Gfx *func_801EC49C(Gfx *arg0, s32 x, s32 y, u8 slot) {
    Gfx *gdl = arg0;
    s32 pad;
    s32 mode;

    if (yoshiState == 8) {
        D_80215900[slot] = 3;
    }
    switch (D_80215900[slot]) {
        case 0:
            D_80215908[slot]--;
            if (D_80215908[slot] == 0) {
                D_80215900[slot] = 1;
                if (D_80215902[slot] == 5) {
                    func_80260650(D_80367738, 0xEA, 0);
                } else {
                    func_80260650(D_80367738, 0xE6, 0);
                }
            }
            break;
        case 4:
            D_80215908[slot]--;
            if (D_80215908[slot] == 0) {
                D_80215900[slot] = 1;
            }
            break;
        case 1:
            D_80215910[slot] += 0x10;
            if (D_80215910[slot] >= 0x100) {
                D_80215910[slot] = 0xFF;
                D_80215900[slot] = 2;
            }
            break;
        case 3:
            D_80215910[slot] -= 0x20;
            if (D_80215910[slot] < 0) {
                D_80215910[slot] = 0;
            }
            break;
        case 2:
            break;
    }
    if (D_80215900[slot] == 0) {
        return gdl;
    }
    if (D_80215902[slot] == 5) {
        mode = 2;
    } else {
        mode = 1;
    }
    gdl = func_80272ED8(gdl, D_80215902[slot] % 5 + D_80215914, x, y, D_80215910[slot], mode, 1.0f);
    return gdl;
}

/* Draw the save-slot panel: the two slot boxes, or the rank badge and the
 * level's target time for it */
Gfx *func_801EC770(Gfx *start, void *gfxp, s32 *count) {
    Gfx *gdl;
    u8 n;

    gdl = start;
    gdl = func_80274868(gdl);
    if (D_80364AA8 == 1) {
        gdl = func_801EC49C(gdl, 0x5C, 0x68, 0);
        if (D_80215902[1] != 0) {
            gdl = func_801EC49C(gdl, 0xA8, 0x68, 1);
        }
    } else {
        if (D_80215902[1] == 5) {
            n = 1;
        } else {
            n = (MAX_RANK() < D_80215902[1] + 1) ? MAX_RANK() : D_80215902[1] + 1;
        }
        gdl = func_801EC49C(gdl, 0x82, 0x68, 1);
        gdl = func_80272ED8(gdl, n + D_80215914, 0x2E, 0x6C, (D_80215910[1] * 3) / 4, 0, 0.75f);
    }
    gdl = func_80274AA4(gdl);
    if (D_80364AA8 != 1) {
        func_80264A34(D_80215470, D_802E8F94[D_802E8BDC].times[5 - n], 0);
        D_80215470[5] = 0;
        func_80259DC8(gfxp, D_80215470, 0, 0, 0, 0x29, 0x7D, 0x10, 0x10, 1, 0xFF, 0xB4, 0, D_80215910[1], 0xFF, 0x78, 0,
                      D_80215910[1]);
    }
    *count += gdl - start;
    return gdl;
}

/* Level-select mask bit for a level: done, of a 0x81 type, or other */
u64 func_801ECA50(u8 level) {
    u64 mask;

    if ((D_80364AF0[D_80364AE8].rank[level] > 0 && D_80364AF0[D_80364AE8].rank[level] < 6) ? 1 : 0) {
        mask = 0x80;
    } else if (D_802E8F94[level].type & 0x81) {
        mask = 0x800;
    } else {
        mask = 0x20000000;
    }
    return mask;
}

/* Leave a level: queue the result messages for the current level and player */
void func_801ECB18(void) {
    func_801FE018(8);
    D_80364A87 = 0;
    D_803643D5 = 0;
    if ((D_80364AF0[D_80364AE8].rank[D_802E8BDC] > 0 && D_80364AF0[D_80364AE8].rank[D_802E8BDC] < 6) ? 1 : 0) {
        if (D_802E8F94[D_802E8BDC].type == 1) {
            osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 0xC | (D_80364AE8 << 16)), OS_MESG_BLOCK);
        }
        osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 8 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
    } else {
        osSendMesg(&D_80219EF8, (OSMesg) ((D_802E8BDC << 8) | 0x16 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
        func_801F8354(D_80364AE8);
    }
}

/* Send the pending saves (D_8039C53C) to the save thread */
void func_801ECC8C(void) {
    s32 i;
    s32 shown;

    shown = 0;
    for (i = 0; i < 4; i++) {
        if (D_8039C53C[i] != 0) {
            if (shown == 0) {
                func_80261570(0.0f);
                shown = 1;
            }
            osSendMesg(&D_80219EF8, (OSMesg) ((i << 16) | 0x14), OS_MESG_BLOCK);
            func_8029A7E4("saving player %d on level %d\n", i, D_8039C53C[i] - 1);
            D_80364AF0[i].level = D_802E8BDC;
            osSendMesg(&D_80219EF8, (OSMesg) (((D_8039C53C[i] - 1) << 8) | 7 | (i << 16)), OS_MESG_BLOCK);
            osSendMesg(&D_80219EF8, (OSMesg) (((D_8039C53C[i] - 1) << 8) | 9 | (i << 16)), OS_MESG_BLOCK);
            osSendMesg(&D_80219EF8, (OSMesg) (((D_8039C53C[i] - 1) << 8) | 0xB | (i << 16)), OS_MESG_BLOCK);
            if (D_8039C540 != 0 && D_80364AE8 == i) {
                osSendMesg(&D_80219EF8, (OSMesg) (((D_8039C540 - 1) << 8) | 0xD | (D_80364AE8 << 16)),
                           OS_MESG_BLOCK);
                D_8039C540 = 0;
            }
            osSendMesg(&D_80219EF8, (OSMesg) ((i << 16) | 0x15 | 0x1000000), OS_MESG_BLOCK);
            osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
            D_8039C53C[i] = 0;
        }
    }
}

/* Retype the 0x81-type levels: 0x80 once unk91 >= 11 in mode 0x4000, else 1 */
void func_801ECE9C(void) {
    s32 i;

    for (i = 0; i < 0x3C; i++) {
        if (D_802E8F94[i].type & 0x81) {
            if (D_80364AF0[D_80364AE8].unk91 >= 11 && D_80364A98 == 0x4000) {
                D_802E8F94[i].type = 0x80;
            } else {
                D_802E8F94[i].type = 1;
            }
        }
    }
}

#define saveIt D_8039C53C
#define playerNumber D_80364AE8

/* After a level: re-rank the player's finished race levels from their saved
 * times, add up the rank points and award new titles (menu entries 185-194) */
void func_801ECF5C(void) {
    Player *p = &D_80364AF0[D_80364AE8];
    u8 buf[0x20];
    LevelInfo *info;
    u8 stars[5] = { 0, 0, 0, 0, 0 };
    u8 gained;
    u16 oldStars = p->stars;
    s32 i;
    s32 x;
    u16 time;
    u8 rank;
    u8 level;

    func_801ED480((u8 *) D_80364EF0[D_80364AE8], buf);
    PLAYER_ASSERT(saveIt[playerNumber], 1384);
    for (level = 0; level < 0x3C; level++) {
        if (((D_80364AF0[D_80364AE8].rank[level] > 0 && D_80364AF0[D_80364AE8].rank[level] < 6) ? 1 : 0) &&
            D_802E8F94[level].type == 1 && level != 0x31 && level != 0x2F && level != 0x26) {
            info = &D_802E8F94[level];
            if (D_8039C53C[D_80364AE8] != level + 1) {
                osSendMesg(&D_80219EF8, (OSMesg) ((level << 8) | 8 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
                osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
            } else {
                func_801ED480(buf, (u8 *) D_80364EF0[D_80364AE8]);
            }
            time = D_80364EF0[D_80364AE8][D_802E8C44[0]];
            func_8029A7E4("level %d time is %d\n", level, time);
            rank = func_801EF2BC(time, level, D_80364AF0[D_80364AE8].unk91);
            p->rank[level] = rank;
            stars[rank - 1]++;
        }
    }
    func_801ED480(buf, (u8 *) D_80364EF0[D_80364AE8]);
    D_802E8BDC = 0;
    func_8029A7E4(" %d %d %d %d %d\n", stars[2] * 3, stars[1] * 2, stars[0], p->stars, oldStars);
    p->stars += stars[2] * 3 + stars[1] * 2 + stars[0];
    gained = p->stars / 12 - oldStars / 12;
    p->title += gained;
    func_8029A7E4("%d stars\n", gained);
    i = 0;
    x = (0x140 - gained * 32) / 2 + 0x28;
    for (; i < gained; i++, x += 0x20) {
        D_8020C070[i + 189].unk0 |= 0x100;
        D_8020C070[i + 189].unk2 = x;
        D_8020C070[i + 189].unk14 = 0x33;
    }
    for (; i < 6; i++) {
        D_8020C070[i + 189].unk0 &= ~0x100;
        D_8020C070[i + 189].unk14 = 0;
    }
    for (i = 0; i < 3; i++) {
        sprintf(D_80215480[i], "*******%-2d**", stars[2 - i]);
        D_8020C070[i + 185].unkC = D_80215480[i];
    }
}

void func_801ED480(u8 *src, u8 *dst) {
    u32 i;

    for (i = 0; i < 0x20; i++) {
        dst[i] = src[i];
    }
}

/* Like func_801ECF5C for all finished levels after a cheat-mode ("cmo") run:
 * re-rank them, a rank of 4 is worth one point */
void func_801ED4B8(void) {
    Player *p = &D_80364AF0[D_80364AE8];
    u8 buf[0x20];
    LevelInfo *info;
    u8 gained;
    u16 oldStars = p->stars;
    s32 i;
    u8 rank;
    u8 level;

    func_801ED480((u8 *) D_80364EF0[D_80364AE8], buf);
    PLAYER_ASSERT(saveIt[playerNumber], 1472);
    for (level = 0; level < 0x3C; level++) {
        if (((D_80364AF0[D_80364AE8].rank[level] > 0 && D_80364AF0[D_80364AE8].rank[level] < 6) ? 1 : 0) &&
            level != 0x31 && level != 0x2F && level != 0x26) {
            info = &D_802E8F94[level];
            if (D_8039C53C[D_80364AE8] != level + 1) {
                osSendMesg(&D_80219EF8, (OSMesg) ((level << 8) | 8 | (D_80364AE8 << 16) | 0x1000000), OS_MESG_BLOCK);
                osRecvMesg(&D_80219F50, NULL, OS_MESG_BLOCK);
            } else {
                func_801ED480(buf, (u8 *) D_80364EF0[D_80364AE8]);
            }
            rank = func_801EF2BC(D_80364EF0[D_80364AE8][D_802E8C44[D_80364AF0[D_80364AE8].unk92[level]]], level,
                                 D_80364AF0[D_80364AE8].unk91 + 1);
            p->rank[level] = rank;
            if (rank == 4) {
                p->stars++;
            }
        }
    }
    func_801ED480(buf, (u8 *) D_80364EF0[D_80364AE8]);
    D_802E8BDC = 0;
    gained = p->stars / 12 - oldStars / 12;
    p->title += gained;
    func_8029A7E4("cmo destroy %d stars\n", gained);
}
