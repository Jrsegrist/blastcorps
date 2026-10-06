#include "common.h"
#include <ultra64.h>
#include "game/game.h"

/* font.c (from its assert strings): text/glyph-slot cache and string helpers */

extern u8 D_80365360[80];
extern u16 D_803653B0[80];
extern u8 D_8039CAF0[][0x200];
extern u8 D_80365458[];
extern u16 D_80365558[];
extern f32 D_802E8C84[];


#define ASSERT(EX, line) if (!(EX)) func_8029A7E4("\n\a --- ASSERTION FAULT - %s - %s, line %d\n\n", #EX, "font.c", line)

void func_8025B070(void) {
    s32 i;

    for (i = 0; i < 80; i++) {
        D_80365360[i] = 0;
        D_803653B0[i] = 0;
    }
}

u8 *func_8025B0B8(u16 arg0) {
    s32 i;
    u8 found;

    found = 0;
    for (i = 0; i < 80 && !found;) {
        if (D_803653B0[i] == arg0) {
            found = 1;
        } else {
            i++;
        }
    }
    if (!found) {
        for (i = 0; i < 80 && !found;) {
            if (D_803653B0[i] == 0) {
                func_802A1040(arg0, D_8039CAF0[i], 0);
                D_803653B0[i] = arg0;
                found = 1;
            } else {
                i++;
            }
        }
    }
    if (!found) {
        for (i = 0; i < 80 && !found;) {
            if (D_80365360[i] == 0) {
                func_802A1040(arg0, D_8039CAF0[i], 0);
                D_803653B0[i] = arg0;
                found = 1;
            } else {
                i++;
            }
        }
    }
    ASSERT(found, 81);
    if (found) {
        D_80365360[i] = 3;
    }
    return D_8039CAF0[i];
}

void func_8025B2B8(void) {
    s32 i;

    for (i = 0; i < 80; i++) {
        if (D_80365360[i] != 0) {
            D_80365360[i]--;
        }
    }
}

s32 func_8025B300(u8 *arg0) {
    s32 i;
    s32 n;

    i = 0;
    n = 0;
    if (arg0 != NULL) {
        for (; *(i + arg0) != 0; i++) {
            if (arg0[i] == '*') {
                n++;
            }
        }
    }
    return i - n;
}

s32 func_8025B370(u16 *arg0) {
    s32 i;
    s32 n;

    i = 0;
    n = 0;
    if (arg0 != NULL) {
        for (; arg0[i] != 0xFFE; i++) {
            if (arg0[i] == 0x1000) {
                n++;
            }
        }
    }
    return i - n;
}

s32 func_8025B3F0(u8 *a, u8 *b) {
    s32 i;
    s32 len;

    if ((len = func_8025B300(a)) != func_8025B300(b)) {
        return 1;
    }
    for (i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return 1;
        }
    }
    return 0;
}

s16 func_8025B498(s16 x, u16 scale, u8 *str, s32 arg3) {
    s32 unused;
    u16 result;
    register s32 len;

    unused = 0;
    len = func_8025B300(str);
    result = (s32)(x - ((D_802E8C84[0] * (len - 1) + 1.0f) * scale) / 2.0);
    return result;
}

u8 *func_8025B558(u16 *arg0) {
    s32 i;

    for (i = 0; arg0[i] != 0 && i < 0xFF;) {
        D_80365458[i] = arg0[i++];
    }
    D_80365458[i] = 0;
    return D_80365458;
}

u16 *func_8025B5D4(u16 *dst, u16 *src, u16 *str, s32 num) {
    u8 buf[12];
    s32 i;
    s32 j;
    s32 k;

    i = 0;
    j = 0;
    do {
        switch (src[i]) {
            case 0x1003:
                for (k = 0; str[k] != 0xFFE; k++, j++) {
                    dst[j] = str[k];
                }
                break;
            case 0x1004:
                func_802D6A60(buf, "%d", num);
                for (k = 0; buf[k] != 0; k++, j++) {
                    dst[j] = buf[k] - 0x20;
                }
                break;
            default:
                dst[j] = src[i];
                j++;
                break;
        }
        i++;
    } while (src[i] != 0xFFE);
    dst[j] = 0xFFE;
    return dst;
}

u16 *func_8025B7AC(u8 *str) {
    s32 i;

    for (i = 0; i < func_8025B300(str); i++) {
        switch (str[i]) {
            case ' ':
                D_80365558[i] = 0x1002;
                break;
            case '.':
                D_80365558[i] = 0x3C;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
                D_80365558[i] = str[i] - 0x20;
                break;
            default:
                D_80365558[i] = str[i] - 0x27;
                break;
            case '/':
                D_80365558[i] = 0x3D;
                break;
            case ':':
                D_80365558[i] = 0x3E;
                break;
        }
    }
    D_80365558[i] = 0xFFE;
    return D_80365558;
}

void func_8025B918(u16 *dst, u16 *src) {
    s32 i;
    s32 j;

    for (i = 0, j = func_8025B370(dst); i < func_8025B370(src); i++, j++) {
        dst[j] = src[i];
    }
    dst[j] = 0xFFE;
}
