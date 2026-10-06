#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* Front-end loader: DMAs a (possibly compressed) block from ROM in 16 KB
 * pieces and decompresses it. */

extern s32 *D_802FDB30; /* ROM start of the front end */
extern s32 *D_802FDB34; /* ROM end of the front end */


/* Load the front end overlay to 0x801E7000 once */
void func_8028B3E0(void) {
    u32 size = *D_802FDB34 - *D_802FDB30;

    osViBlack(1);
    if (D_80370C50 == 0) {
        osInvalDCache((void *) 0x801E7000, 0x37D00);
        func_802DB730((void *) 0x801E7000, 0x37D00);
        func_8028B4C4(*D_802FDB30, 0x801E7000, &size, 0xD, 10, 1);
        func_802DB7B0((void *) (size + 0x801E7000), 0x37D00 - size);
        D_80370C50 = 1;
        func_801F57B0();
        func_8029A7E4("got front end\n");
    }
}

/* DMA *size bytes from ROM devAddr to dest (to a staging buffer at 0x8021ED00
 * when compressed), then decompress with method arg5 (1: func_8025C230,
 * 2: func_802C4070); *size becomes the decompressed length. */
void func_8028B4C4(u32 devAddr, u32 dest, u32 *size, u8 arg3, u8 arg4, u8 arg5) {
    u32 i;
    u32 rem;
    u32 buf;
    u32 orig;
    u32 p;

    orig = dest;
    if (arg3 || arg4) {
        buf = 0x8021ED00;
    } else {
        buf = dest;
    }
    osInvalDCache((void *) buf, *size);
    p = buf;
    for (i = 0; i < *size / 0x4000; i++) {
        func_802DA2F0(&D_80370C58, 0, 0, devAddr, (void *) p, 0x4000, &D_803150A0);
        osRecvMesg(&D_803150A0, NULL, 1);
        devAddr += 0x4000;
        p += 0x4000;
    }
    rem = *size - (*size / 0x4000) * 0x4000;
    if (rem) {
        func_802DA2F0(&D_80370C58, 0, 0, devAddr, (void *) p, rem, &D_803150A0);
        osRecvMesg(&D_803150A0, NULL, 1);
    }
    switch (arg5) {
        case 1:
            if (arg3) {
                func_8025C230((u8 **) &buf, (u8 **) &dest, 0x8004B400);
            }
            if (arg4) {
                func_8025C230((u8 **) &buf, (u8 **) &dest, 0x8004B400);
            }
            break;
        case 2:
            if (arg3) {
                func_802C4070(&buf, &dest, 0x8004B400, arg3);
            }
            if (arg4) {
                func_802C4070(&buf, &dest, 0x8004B400, arg4);
            }
            break;
    }
    if (arg3 || arg4) {
        *size = dest - orig;
    }
}
