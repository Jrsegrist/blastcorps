#include "common.h"
#include <ultra64.h>
#ifndef NON_MATCHING /* legacy declarations */
/* Matching build: IDO compiled the matched code here against older
 * declarations of these, which the file keeps; the NON_MATCHING build
 * uses game/game.h's. */
#define LEGACY_D_80367738
#endif /* legacy declarations */
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_8020C070 ((MenuEntry *) D_8020C070)
#define D_802F8BDC ((MenuPage *) D_802F8BDC)
#define D_803156F8 ((FeDyn *) D_803156F8)
/* end of views */

/* back_loop.c: the front end's per-frame loop (menus behind the yoshi windows) */


typedef struct {
    u8 pad0[4];
    s16 unk4;
    u8 pad6[2];
    s32 unk8;
    u8 padC[2];
    u16 unkE;
    u16 unk10;
    u8 pad12[6];
    u16 unk18;
    u8 pad1A[2];
} MenuPage;

typedef struct {
    u16 unk0; /* flags */
    u8 pad2[4];
    u16 unk6;
    u16 unk8;
    u8 padA[2];
    char *unkC;  /* title */
    void *unk10; /* glyph list */
    u8 pad14[8];
} MenuEntry;

#ifndef NON_MATCHING
extern s32 D_80367738;
#endif
#define pakToGameMessageQ D_80219F50
extern u8 D_8021AB70;
extern char D_8021AB72[];
extern s16 D_8021AB74;
extern s16 D_8021AB76;
extern s32 D_8021AB7C;

int sprintf(char *, const char *, ...);

/* Rare's assert; line numbers are the original back_loop.c's */
#define BACK_ASSERT(EX, line) \
    if (!(EX)) \
    func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "back_loop.c", line)

/* One front-end frame: handles pak replies and the menu (yoshi) selection for
 * the current mode, starts the previous display list and builds this frame's,
 * then picks the next mode once the current one is idle. */
void func_801FE990(void) {
    s32 i;
    Gfx *gdl;
    OSMesg msg;

    if (D_80364A90 & 0x80010000000) {
        if (D_8021AB7C == 0) {
            func_80260650(D_80367738, 0x73, &D_8021AB7C);
        }
        if (yoshiState == 2) {
            osRecvMesg(&pakToGameMessageQ, &msg, 1);
            BACK_ASSERT(MQ_IS_EMPTY(&pakToGameMessageQ), 62);
            D_8021AB70 = !msg;
            if (D_80364A90 == 0x10000000) {
                if (D_8021AB70) {
                    func_801EA278();
                }
                D_80364A98 = 0x8000;
            } else {
                D_80364A98 = 0x2000000000;
            }
            func_802608C8((void *) D_8021AB7C);
            func_8026AF6C(0x4000);
        }
    }
    if (D_8036BB16) {
        func_8029A7E4("yoshiSelection in back_loop is %d %d\n", D_8036BB16, func_8026F92C(D_80364A90));
        switch (D_80364A90) {
            case 0x10000:
                if (D_8036BB16 == 0xFFFF) {
                    D_80364A98 = 0x8000000000000000;
                    func_801E8EB8(4, 1);
                } else if (D_802F8BDC[10].unk18 - 2 < 4) {
                    D_80364A98 = 0x2000000;
                } else if (D_802154B0 == 4) {
                    D_8021AB70 = 0;
                    D_8039C541 = 1;
                    D_80364A98 = 0x8000;
                } else {
                    D_80364A98 = 0x800000;
                }
                break;
            case 0x100000000:
                if (D_8036BB16 != 0xFFFF && func_801F73FC()) {
                    D_8021AB74 = D_8036BB16;
                    D_80364A98 = 0x4000000000;
                }
                break;
            case 0x400000:
                if (D_8036BB16 == 0xFFFF) {
                    func_801E8EB8(4, 1);
                    D_80364A98 = 0x200000;
                } else {
                    D_80364AE8 = D_8036BB16 - 2;
                    if (D_80364AE8 < 4) {
                        D_80364A98 = 0x100000;
                    } else {
                        D_80364A98 = 0x200000;
                    }
                }
                break;
            case 0x80000:
                if (D_8036BB16 == 0xC) {
                    func_801EA108(D_80364AE8, 0, 0);
                }
                func_801E8EB8(4, 1);
                D_80364A98 = 0x800000;
                break;
            case 0x8000000000:
                if (D_8036BB16 == 0xC) {
                    osSendMesg(&D_80219EF8, (OSMesg) ((D_8021AB74 << 16) | 5), 1);
                }
                D_80364A98 = 0x10000000000;
                break;
            case 0x80:
            case 0x8000000:
                if (D_8036BB16 == 0xFFFF) {
                    D_80364A98 = 0x4000;
                } else {
                    i = D_802F8BDC[currentYoshiWindow].unk10 + D_802F8BDC[currentYoshiWindow].unkE - D_8036BB16 - 1;
                    switch (i) {
                        case 0:
                            D_80364A98 = 0x4000;
                            break;
                        case 1:
                            D_80364A98 = 0x40;
                            break;
                        case 2:
                            if (!(((s32) D_80364AA8) & 0x81)) {
                                if (((s32) D_80364AA8) == 2 && D_80364A90 == 0x8000000) {
                                    D_8039CA60 = 1;
                                }
                                D_80364A98 = 0x20000000;
                            } else {
                                D_80364A98 = 0x2000;
                            }
                            break;
                        case 3:
                            D_80364A98 = 0x40000000000;
                            break;
                    }
                }
                break;
            case 0x40000000000:
                if (D_8036BB16 == 0xFFFF) {
                    D_80364AE8 = D_80364AE9;
                }
                if (D_80364AE8 == D_80364AE9 || D_80364AE8 == D_80364AEA || D_8036BB16 == 0xFFFF) {
                    D_80364A98 = D_8021A830;
                } else {
                    D_80364A98 = 0x100000000000000;
                }
                break;
            case 0x4000000000000:
                func_801E8C40(4);
                D_80364A98 = 0x8000000000000;
                break;
            case 0x1000000000000:
                func_80275390(0x20000000000000);
                break;
            case 0x40000000000000:
                if (D_8036BB16 == 0xC) {
                    func_801EA108(D_80364AE8, 0, 1);
                }
                break;
            case 0x100000000000000:
                func_801E8EB8(4, 1);
                break;
            case 0x4000000000000000:
                if (D_8036BB16 == 0xC) {
                    func_80275270(0x400000000000000, 0.5f);
                } else {
                    func_80275270(0x4000, 0.5f);
                }
                break;
            default:
                func_8029A7E4("backdrop illegal yoshi selection\n");
                break;
        }
        D_8021AB76 = D_8036BB16;
        D_8036BB16 = 0;
    }
    D_80358080 = 0;
    D_80358084 = 0;
    func_802A5720();
    func_80284E54((u64 *) (D_803156F8[D_8035805C].dl), D_80358078, 1, 1, 0x4D2, 0);
    D_8035805C ^= 1;
    gdl = D_803156F8[D_8035805C].dl;
    gSPSegment(gdl++, 0, 0);
    gSPSegment(gdl++, 2, osVirtualToPhysical(&D_803156F8[D_8035805C]));
    gSPSegment(gdl++, 1, osVirtualToPhysical(D_8035806C));
    gSPDisplayList(gdl++, D_01000038);
    gSPDisplayList(gdl++, D_01000010);
    gSPClipRatio(gdl++, FRUSTRATIO_3);
    gDPSetCycleType(gdl++, G_CYC_FILL);
    if (D_80364A90 & 0x88000080) {
        gDPSetDepthImage(gdl++, D_80358058);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358058);
        gDPSetFillColor(gdl++, 0xFFFCFFFC);
        gDPFillRectangle(gdl++, 220, 110, 300, 160);
    } else if (D_80364A90 & 0x40000000) {
        gDPSetDepthImage(gdl++, D_80358058);
        gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358058);
        gDPSetFillColor(gdl++, 0xFFFCFFFC);
        gDPFillRectangle(gdl++, 0, 0, 319, 239);
    }
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, D_80358050[D_8035805C]);
    func_80259450();
    gdl = func_80200BE0(gdl, (s32) &D_803156F8[D_8035805C], &D_80358078);
    gDPPipeSync(gdl++);
    gDPSetCycleType(gdl++, G_CYC_1CYCLE);
    if (D_80364A90 & 0xC000000000000) {
        gdl = func_8026BBD0(gdl, (s32) &D_803156F8[D_8035805C], &D_80358078);
    }
    if (D_80364A90 == 0x4000000000000) {
        if (D_80358060 == 0) {
            func_80262008(0x27, 0);
            func_80260EE0(0xF);
        }
        if (D_8036BB1E == 0 && D_80364AE8 == 4) {
            D_80364AE8 = D_80364AEA;
            func_801E8DCC(D_80364AE8);
            D_802F8BDC[56].unk8 |= 0x20;
        }
    }
    if (D_80364A90 & 0x818D04001AFB8080) {
        gdl = func_801E9718(gdl, (struct PlayerSelDyn *) &D_803156F8[D_8035805C], 0xC2);
    }
    if (D_80364A90 & 0x898C0FE313F78002) {
        gdl = func_8025C878(gdl, (s32) &D_803156F8[D_8035805C], D_8035805C, &D_80358078);
    }
    if (D_80364A90 & 0x410000) {
        u8 old = D_80364AE8;

        D_80364AE8 = D_802F8BDC[10].unk18 - 2;
        if (D_80364AE8 != old) {
            if (old == 4) {
                func_801E8EB8(D_80364AE8, 0);
            } else {
                func_801E8EB8(D_80364AE8, 1);
            }
        }
    }
    func_8028A3E4();
    if (D_80364A90 & 0x40000000) {
        gdl = func_801ED800(gdl, &D_803156F8[D_8035805C], D_8035805C, &D_80358078);
    }
    if (D_80364A90 == 0x1000000000000) {
        gdl = func_80201364((s32) D_803156F8, gdl);
    }
    gdl = func_8024C404(gdl, (DynamicBuf *) &D_803156F8[D_8035805C], &D_80358078);
    if (D_80364A90 & 0x88000080) {
        gdl = func_801EC770(gdl, &D_803156F8[D_8035805C], &D_80358078);
        if (D_80364A71 != -1 && yoshiState == 2) {
            gdl = func_801F51C8(&D_803156F8[D_8035805C], gdl);
            gSPClearGeometryMode(gdl++, G_ZBUFFER);
        }
    }
    if (D_80364A90 == 0x80000000) {
        if (D_80364A64) {
            D_80364A64--;
        } else if (D_8036BB1E != 2 && yoshiState == 2) {
            func_8026AF6C(0x4000);
            func_80260650(D_80367738, 0x1C, NULL);
        }
    }
    func_8028A470();
    if (D_80364A90 == 0x800000000000) {
        func_802862DC();
    }
    if ((D_80364A90 & 0x88000080) && D_803156C4 % 20 >= 6 && D_80364A71 != -1 && yoshiState == 2) {
        s32 x;
        s32 y;

        x = 0x118, y = 0x8C;
        sprintf(D_8021AB72, "%d", D_80364A71);
        func_80259CCC((Gfx **) &D_803156F8[D_8035805C], (u8 *) D_8021AB72, 0, 1, 0, x, y, 0x1A, 0x16, 1, 0, 0, 0, D_8036BB20);
        func_80259DC8((Gfx **) &D_803156F8[D_8035805C], (u8 *) D_8021AB72, 0, 1, 0, x + 2, y + 2, 0x12, 0x12, 1, 0xFF, 0xFF, 0,
                      D_8036BB20, 0xFF, 0, 0, D_8036BB20);
    }
    if (D_80364A90 & 0x40000000000) {
        func_801F803C();
        gdl = func_801F8440((s32) &D_803156F8[D_8035805C], gdl);
    }
    func_80259C24(&gdl, (Mtx *) &D_803156F8[D_8035805C]);
    gdl = func_80274BF0((s32) &D_803156F8[D_8035805C], gdl);
    if (!(D_80364A90 & 0xC000000000000)) {
        gdl = func_8026BBD0(gdl, (s32) &D_803156F8[D_8035805C], &D_80358078);
    }
    if (yoshiState != 1 && currentYoshiWindow == 0xB) {
        gdl = func_801EAA7C(gdl, (struct PlayerSelDyn *) &D_803156F8[D_8035805C], &D_80358078);
    }
    gDPFullSync(gdl++);
    gSPEndDisplayList(gdl++);
    D_80358078 = gdl - D_803156F8[D_8035805C].dl;
    for (i = 0; i < D_80358080; i++) {
        osRecvMesg(&D_80315180, NULL, 1);
    }
    for (i = 0; i < D_80358080 - D_80358084; i++) {
        func_802A57AC();
    }
    if ((D_80364A90 & 0x81D9836583B28000) && yoshiState == 1 && !func_802753F8() && !func_802753C0() &&
        D_80364A98 == 0 && D_8036BB1A == -1) {
        switch (D_80364A90) {
            case 0x20000:
                D_80364A98 = 0x40000;
                break;
            case 0x200000:
                D_80364A98 = 0x10000;
                break;
            case 0x10000000000:
                D_80364A98 = 0x100000000;
                break;
            case 0x100000:
                D_80364A98 = 0x80000;
                D_8020C070[9].unkC = D_8020C070[D_80364AE8 + 2].unkC;
                D_8020C070[9].unk10 = NULL;
                D_8020C070[9].unk6 = D_8020C070[9].unk8 = 0x14;
                func_8026AF6C(0x800C);
                break;
            case 0x4000000000:
                D_80364A98 = 0x8000000000;
                D_8020C070[9].unkC = D_8020C070[D_8021AB74].unkC;
                D_8020C070[9].unk10 = D_8020C070[D_8021AB74].unk10;
                D_8020C070[9].unk6 = D_8020C070[9].unk8 = 0xF;
                func_8026AF6C(0x800C);
                break;
            case 0x800000:
                D_80364A98 = 0x400000;
                break;
            case 0x2000000:
                D_80364AE9 = D_80364AE8;
                D_80364AEA = D_80364AE8;
                if (D_80365060[D_80364AE8] == 1) {
                    func_80275390(0x4000);
                } else {
                    D_80364A98 = 0x20000;
                }
                break;
            case 0x8000000000000000:
                func_80275390(0x400000000000000);
                break;
            case 0x1000000:
                osSendMesg(&D_80219EF8, (OSMesg) ((D_80364AE8 << 16) | 7), 1);
                osSendMesg(&D_80219EF8, (OSMesg) ((D_80364AE8 << 16) | 0x15 | 0x01000000), 1);
                func_802995F0(4);
                func_80275390(0x100000000000);
                break;
            case 0x80000000:
                D_80364A98 = 0x40000000;
                break;
            case 0x8000:
                if (D_8039C541) {
                    func_80275390(0x20000000000000);
                } else if (D_8021AB70) {
                    D_80364A98 = 0x10000;
                } else {
                    D_80364A98 = 0x10000000;
                }
                break;
            case 0x2000000000:
                if (D_8021AB70) {
                    D_80364A98 = 0x100000000;
                } else {
                    D_80364A98 = 0x80000000000;
                }
                break;
            case 0x40000000000000:
            case 0x100000000:
                func_80275270(0x10, 0.5f);
                break;
            case 0x800000000000:
                func_80275270(0x2000000000000, 0.75f);
                func_80261570(0.0f);
                break;
            case 0x1000000000000:
                func_80275270(2, 0.5f);
                break;
            case 0x8000000000000:
                D_80364AE8 = D_80364AEA;
                func_80275270(0x4000, 0.6f);
                break;
            case 0x80000000000000:
                if (D_80364AE8 == D_80364AE9 || D_80364AE8 == D_80364AEA) {
                    D_80364A98 = D_8021A830;
                } else {
                    D_80364A98 = 0x100000000000000;
                }
                break;
            case 0x100000000000000:
                if (D_8021AB76 != 0xC) {
                    D_80364A98 = 0x200000000000000;
                } else {
                    func_80275390(D_8021A830);
                }
                break;
            default:
                func_8029A7E4("illegal yoshi wait game mode %d\n", func_8026F92C(D_80364A90));
                break;
        }
    }
}
