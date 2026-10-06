/* Stage-0 spike: real game C, linked against absolute N64 data symbols, run
 * on the fixed RDRAM image.  Prints one line per test case; the same cases
 * run through the original MIPS code (port/tools/n64ref.py, unicorn on the
 * matching ELF) must print the same lines.
 *
 * usage: spike.exe ROM MODE
 *   MODE typed : data in host order (native initialisers for objects defined
 *                in C; byte-swapped by type for the tables the tests read from
 *                the ROM image and for the synthetic level asset)
 *   MODE raw   : the ROM image and asset left big-endian as loaded
 */
#include <windows.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rdram.h"

/* game functions (prototypes as in include/game/functions.h) */
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef float f32;
s32 func_802AD7D4(s32 sine);
s32 func_802A56C4(void);
void func_802A5510(u8 *level);
void func_802CB690(u8 *state);

#define V8(a) (*(volatile uint8_t *) (uintptr_t) (a))
#define V16(a) (*(volatile uint16_t *) (uintptr_t) (a))
#define V32(a) (*(volatile uint32_t *) (uintptr_t) (a))
#define V64(a) (*(volatile uint64_t *) (uintptr_t) (a))

static uint32_t rng_state;
static uint32_t rnd(void) {
    rng_state = rng_state * 1103515245u + 12345u;
    return rng_state >> 8;
}

/* an access violation inside a test case (raw mode: big-endian offsets
 * become wild pointers) is reported as CRASH for that case */
static jmp_buf crash_jmp;
static volatile int crash_armed;
static LONG CALLBACK on_fault(EXCEPTION_POINTERS *ep) {
    if (crash_armed && ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        crash_armed = 0;
        longjmp(crash_jmp, 1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#define GUARD(label) if (setjmp(crash_jmp)) { printf("%s CRASH\n", label); continue; } crash_armed = 1
#define UNGUARD() crash_armed = 0

void port_stub_hit(const char *name) {
    fprintf(stderr, "STUB called: %s\n", name);
    exit(3);
}

/* callees of func_802CB690, recorded instead of run */
static char t4_calls[256];
void *func_80260650(void *a0, s16 a1, void *a2) {
    size_t n = strlen(t4_calls);
    snprintf(t4_calls + n, sizeof t4_calls - n, "S(%08X,%04X,%08X)", (unsigned) (uintptr_t) a0,
             (unsigned) (uint16_t) a1, (unsigned) (uintptr_t) a2);
    return NULL;
}
void func_80278EB0(s32 n, f32 scale, s32 arg2) {
    size_t k = strlen(t4_calls);
    uint32_t bits;
    memcpy(&bits, &scale, 4);
    snprintf(t4_calls + k, sizeof t4_calls - k, "M(%d,%08X,%d)", n, bits, arg2);
}

/* write a big-endian value the way the ROM/asset would hold it */
static void be16(uint32_t a, uint32_t v) { V8(a) = v >> 8; V8(a + 1) = v; }
static void be32(uint32_t a, uint32_t v) { be16(a, v >> 16); be16(a + 2, v); }

#define LEVEL 0x80100000u
#define T2_ENTRIES 21

int main(int argc, char **argv) {
    const char *rom = argc > 1 ? argv[1] : "baserom.us.v11.z64";
    int typed = !(argc > 2 && !strcmp(argv[2], "raw"));
    uint32_t i;
    static uint8_t hdtext[0xC0000];
    long ntext;
    static char lbl[32];

    setvbuf(stdout, NULL, _IOFBF, 1 << 16);
    AddVectoredExceptionHandler(1, on_fault);
    if (rdram_map() || rdram_load(rom)) return 1;
    if (!typed) {
        /* raw: the asin table as the ROM holds it (big-endian, from the hd_code
         * text segment), instead of the C definition's native initialiser */
        ntext = rom_gunzip(ROM_HD_TEXT, hdtext, sizeof hdtext);
        memcpy((void *) 0x802AD880, hdtext + (0x802AD880 - 0x802447C0), 0x884);
        fprintf(stderr, "raw mode: hd text %ld bytes\n", ntext);
    } else {
        /* typed: s16 D_80305B90[][3] lives in a .data bin; swap by its type */
        swap_range(0x80305B90, T2_ENTRIES * 3, 2);
    }

    /* T1: arcsine lookup (69000.c) over the table D_802AD880 (69930.c) */
    for (i = 0; i < 0x10000; i++) printf("T1 %04X %08X\n", i, (unsigned) func_802AD7D4((s32) i));

    /* T2: per-level s16 triple from the .data image (60D50.c) */
    for (i = 0; i < T2_ENTRIES; i++) {
        s32 r;
        V8(0x803BE73A) = (uint8_t) i;
        V16(0x8036444C) = 0xAAAA;
        V16(0x80364450) = 0xBBBB;
        r = func_802A56C4();
        printf("T2 %02X %08X %04X %04X\n", i, (unsigned) r, V16(0x8036444C), V16(0x80364450));
    }

    /* T3: highest height zone, over a synthetic level asset in big-endian
     * form at LEVEL (as loaded from the ROM), swapped by schema if typed */
    rng_state = 3;
    for (i = 0; i < 300; i++) {
        uint32_t nz = rnd() % 9, z, end = 0x50 + nz * 10;
        be32(LEVEL + 0x40, 0x50);
        be32(LEVEL + 0x44, end);
        for (z = 0; z < nz; z++) {
            uint32_t a = LEVEL + 0x50 + z * 10;
            uint32_t mx = rnd() % 8, mz = rnd() % 8;
            uint32_t h = (rnd() % 5 == 0) ? 3000 : (rnd() % 4000) - 500;
            be16(a + 0, mx);
            be16(a + 2, mz);
            be16(a + 4, mx + rnd() % 8);
            be16(a + 6, mz + rnd() % 8);
            be16(a + 8, h);
        }
        if (typed) {
            swap_range(LEVEL + 0x40, 2, 4);
            swap_range(LEVEL + 0x50, nz * 5, 2);
        }
        {
            uint32_t a = (rnd() % 10) << 5;
            a |= rnd() & 31;
            V32(0x803643E0) = a;
            a = (rnd() % 10) << 5;
            a |= rnd() & 31;
            V32(0x803643E8) = a;
        }
        V16(0x80364450) = rnd() & 0xFFFF;
        V16(0x8036444E) = 0x1234;
        V8(0x80364411) = 0x55;
        snprintf(lbl, sizeof lbl, "T3 %03u", i);
        GUARD(lbl);
        func_802A5510((u8 *) LEVEL);
        UNGUARD();
        printf("T3 %03u %04X %02X\n", i, V16(0x8036444E), V8(0x80364411));
    }

    /* T4: vehicle helper (86ED0.c) writing the vehicle block and globals */
    rng_state = 4;
    for (i = 0; i < 200; i++) {
        static const uint32_t lo[] = {0x40, 0x40, 0x40, 0, 0x41, 0x1000};
        uint32_t st = 0x803F8AA0;
        uint32_t hi = (rnd() % 4 == 0) ? 1 : 0;
        V64(0x80364A98) = ((uint64_t) hi << 32) | lo[rnd() % 6];
        V16(0x80364A72) = rnd() & 0xFFFF;
        V32(0x80367738) = rnd();
        V16(st + 0x76) = rnd() & 0xFFFF;
        V8(0x80367BFF) = 1;
        V8(0x80367C00) = 0;
        t4_calls[0] = 0;
        func_802CB690((u8 *) st);
        printf("T4 %03u %s 76=%04X BFF=%02X C00=%02X\n", i, t4_calls, V16(st + 0x76), V8(0x80367BFF),
               V8(0x80367C00));
    }
    return 0;
}
