#include "common.h"
#include <ultra64.h>

/* libultra libc string.c (this file is built at -O2, see Makefile). */
void *memcpy(void *s1, const void *s2, u32 n) {
    u8 *su1 = (u8 *) s1;
    const u8 *su2 = (const u8 *) s2;

    while (n > 0) {
        *su1++ = *su2++;
        n--;
    }
    return (void *) s1;
}

u32 strlen(const u8 *s) {
    const u8 *sc = s;

    while (*sc != '\0') {
        sc++;
    }
    return (u32) (sc - s);
}

#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/99580/strchr.s")
