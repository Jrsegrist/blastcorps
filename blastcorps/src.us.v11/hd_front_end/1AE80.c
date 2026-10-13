#include "common.h"
#include <ultra64.h>
#include "game/game.h"
/* Views: this file reads these shared variables (game/variables.h) as other types. */
#define D_802E8F94 ((LevelInfo *) D_802E8F94)
#define players ((Player *) players)
/* end of views */

typedef struct {
    u8 unk0;
    u8 pad1[0x43];
} LevelInfo; /* 0x44 bytes */

typedef struct {
    u8 pad0[0x18];
    u8 unk18[0x7A]; /* per level: completion state */
    u8 unk92[0x5E]; /* per level */
    u8 padF0[0x10];
} Player; /* 0x100 bytes */


/* Restore the current player's records from EEPROM: one pak-thread request
 * (0xA) per even level, then for each completed level other than 0x26, 0x2F
 * and 0x31 requests 0xD (when D_802E8F94[level].unk0 == 1) and 9, copying a
 * time from D_80364F70 into D_80364EF0. Stops early on a non-zero reply. */
s32 func_80201E80(void) {
    u8 i;
    OSMesg msg;

    msg = 0;
    debug_printf("restoring from EE memory\n");
    for (i = 0; i < 0x3C && msg == 0; i++) {
        if (!(i & 1)) {
            osSendMesg(&D_80219EF8, (OSMesg) ((i << 8) | 0xA | (playerNumber << 16) | 0x01000000), 1);
            osRecvMesg(&pakToGameMessageQ, &msg, 1);
        }
        if (((players[playerNumber].unk18[i] > 0 && players[playerNumber].unk18[i] < 6) ? 1 : 0) &&
            i != 0x31 && i != 0x2F && i != 0x26 && msg == 0) {
            if (D_802E8F94[i].unk0 == 1) {
                pakBuffer[0] = 0x1234567887654321;
                osSendMesg(&D_80219EF8, (OSMesg) ((i << 8) | 0xD | (playerNumber << 16) | 0x01000000), 1);
                osRecvMesg(&pakToGameMessageQ, NULL, 1);
            }
            func_801F8354(playerNumber);
            D_80364EF0[playerNumber][D_802E8C44[players[playerNumber].unk92[i]]] = D_80364F70[i * 2];
            osSendMesg(&D_80219EF8, (OSMesg) ((i << 8) | 9 | (playerNumber << 16) | 0x01000000), 1);
            osRecvMesg(&pakToGameMessageQ, NULL, 1);
        }
    }
    return (s32) msg;
}
