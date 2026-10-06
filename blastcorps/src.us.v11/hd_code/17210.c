#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* recording.c (from its assert strings): demo/attract-mode controller recording and playback */

typedef struct {
    u8 btnHi;
    u8 btnLo;
    u8 count;
    s8 stickX;
    s8 stickY;
} RecEntry;

extern s32 D_80366990;
extern s32 D_80366994;
extern s32 D_80366998;
extern s32 D_8036699C;
extern s32 D_803669A0;
extern u16 D_803669A4;
extern s8 D_803669A6;
extern s8 D_803669A7;
extern u8 D_803669A8;
extern RecEntry *D_803669AC;
extern void *D_803669B0;
extern RecEntry D_80365588[];
extern u8 D_802E8CB0[];


#define ASSERT(EX, line) if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "recording.c", line)

void func_8025B9D0(s32 arg0, s32 *arg1) {
    s32 i;
    u8 *rom;
    s32 size;
    s32 len;
    u16 unk26;
    u16 unk24;
    u8 *p;

    rom = D_006A9F10;
    size = D_006AD3F0 - rom;
    p = D_80358070;
    func_8028B4C4(rom, p, &size, 9, 0, 1);
    D_80358070 = (u8 *) D_80358070 + size;
    func_80257490(&D_80358070, 0x10);
    for (i = 0; i < arg0; i++) {
        len = *(s16 *) (p + 0x140C);
        p = p + len + 0x140E;
    }
    D_80366A04 = *(u16 *) p;
    unk26 = *(u16 *) (p + 2);
    D_80366998 = *(u16 *) (p + 8);
    *arg1 = p[0xA];
    D_8036698C = p[0xB];
    D_803669AC = (RecEntry *) (p + 0xC);
    len = *(s16 *) (p + 0x140C);
    p = p + 0x140E;
    func_80257490((void **) &p, 2);
    D_803669B0 = p;
    D_80366994 = 0;
    D_803669A0 = D_803669AC->count;
    D_803669B4 = 1;
}

void func_8025BB38(void) {
    D_80366990 = 0;
    D_803669A8 = 1;
}

void func_8025BB50(void) {
    D_80366998 = D_80366990;
    D_80366994 = 0;
    D_803669AC = D_80365588;
    D_80365580 = func_80272C5C(D_802E8CB0, 0, 1, 1, 1, 1.0f);
    D_803669A0 = D_803669AC[D_80366994].count;
}

void func_8025BBE8(u16 buttons, s8 x, s8 y) {
    if (D_803669A8 == 0) {
        if (buttons != D_803669A4 || x != D_803669A6 || y != D_803669A7 || D_8036699C == 0xFF || D_80364A98 != 0) {
            if (D_80366990 < 0x400) {
                D_80365588[D_80366990].btnHi = D_803669A4 >> 8;
                D_80365588[D_80366990].btnLo = D_803669A4 & 0xFF;
                D_80365588[D_80366990].stickX = D_803669A6;
                D_80365588[D_80366990].stickY = D_803669A7;
                D_80365588[D_80366990].count = D_8036699C;
                D_80366990++;
                D_8036699C = 0;
            }
        } else {
            D_8036699C++;
        }
    } else {
        D_8036699C = 0;
    }
    D_803669A4 = buttons;
    D_803669A6 = x;
    D_803669A7 = y;
    D_803669A8 = 0;
}

void func_8025BD98(void) {
    switch (D_8036698C) {
        case 3:
            func_802B40D4(D_803669B0);
            func_802B2D7C();
            break;
        case 0:
            func_802AFC28(D_803669B0);
            break;
        case 1:
            func_802B0D70(D_803669B0);
            func_802AFFD4();
            break;
        case 2:
            func_802B2988(D_803669B0);
            func_802B1228();
            break;
        case 16:
            func_802D2524(D_803669B0);
            func_802D0C68();
            break;
        case 4:
            func_802B58C8(D_803669B0);
            func_802B448C();
            break;
        case 5:
            func_802B7308(D_803669B0);
            func_802B5CD8();
            break;
        case 9:
            func_802C80A0(D_803669B0);
            func_802C5714();
            break;
        case 10:
            func_802CB660(D_803669B0);
            func_802C9F54();
            break;
    }
    if (D_8036698C) {
        D_803649E8 = 1;
    }
    D_80364456 = D_8036698C;
    D_803669B4 = 0;
}

void func_8025BEF8(void) {
    s32 pad[2];

    if (D_80366994 > D_80366998 || (D_803643DB && D_803643D6 && (D_80364A90 & 0x440))) {
        if (yoshiState != 1 || ((D_80364A90 & 0x440) && !D_80364A50)) {
            if (!D_802E8BD0) {
                D_802E8BD8 = 1;
            }
        } else if (!func_802753C0()) {
            switch (D_80364A90) {
                case 2:
                    func_80275270(2, 0.25f);
                    break;
                case 0x100000000000:
                    func_8029A7E4("sequence playback over\n");
                    func_80275390(0x200000000000);
                    break;
                case 0x40:
                case 0x400:
                    D_802E8BD8 = 1;
                    if (D_80364A90 == 0x400) {
                        func_80275270(0x40, 1.25f);
                    } else {
                        func_80275270(0x40, 0.5f);
                    }
                    break;
                default:
                    ASSERT(1==0, 388);
                    break;
            }
        }
    } else {
        D_80370C30 = (D_803669AC[D_80366994].btnHi << 8) + D_803669AC[D_80366994].btnLo;
        D_80370C32 = D_803669AC[D_80366994].stickX;
        D_80370C33 = D_803669AC[D_80366994].stickY;
        if (--D_803669A0 < 0) {
            D_80366994++;
            D_803669A0 = D_803669AC[D_80366994].count;
        }
        if ((D_80364A90 & 0x100000000002) && D_8036698C != 9) {
            if ((D_80370C30 & 0xC000) == 0x8000) {
                D_80370C30 &= ~0x8000;
                D_80370C30 |= 0x4000;
            }
            if ((D_80370C30 & 0xC000) == 0x4000) {
                D_80370C30 &= ~0x4000;
                D_80370C30 |= 0x8000;
            }
        }
    }
}
