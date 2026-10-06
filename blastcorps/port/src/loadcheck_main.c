/* loadcheck: the byte-order layer (port/src/load/) against the original code.
 *
 * Real game C (NON_MATCHING, native) loads real assets from the user's ROM
 * through the game's own loaders (func_8028B4C4, func_8025615C, 5BF40's
 * texture loads), with osPiStartDma emulated here the way the platform layer
 * does it (copy from the ROM, then port_on_dma).  The same cases run through
 * the ORIGINAL MIPS code in unicorn on big-endian memory
 * (port/tools/loadref.py); `make -C port loadcheck` compares the two outputs
 * line by line.
 *
 *   T5  port_802E8BF8_word: the D_802E8BF8 flag/word-table alias (00000.c)
 *   T6  texture entries: DMA + func_802A57DC decode, output bytes (BE) CRC
 *   T7  every level: func_8025615C load + func_802A2C54 object-stream unpack
 *   T8  every level: func_802A5510 height zones at random positions
 *   T9  (needs a trace) every read the N64 code made of each traced asset
 *
 * usage: loadcheck.exe ROM [TESTS [FACTS]]   (TESTS: digits, default 5678)
 */
#include <windows.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rdram.h"
#include "load/port_load.h"

typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint32_t u32;

#define V8(a) (*(volatile uint8_t *) (uintptr_t) (a))
#define V16(a) (*(volatile uint16_t *) (uintptr_t) (a))
#define V32(a) (*(volatile uint32_t *) (uintptr_t) (a))

/* game functions under test */
s32 port_802E8BF8_word(s32 i);
void func_802A0700(void);
void func_802A0B34(s32 id, u8 *param);
void func_8025615C(s32 level, s32 dest, s32 *size);
void func_802A2C54(u8 *obj);
void func_802A5510(u8 *level);

/* ---- the platform surface these need (as port/src/platform does it) ---- */
static uint8_t *g_rom;
static size_t g_romsize;

s32 osPiStartDma(void *mb, s32 pri, s32 dir, u32 devAddr, void *dram, u32 size, void *mq) {
    (void) mb; (void) pri; (void) dir; (void) mq;
    if (devAddr + size > g_romsize) {
        fprintf(stderr, "DMA past the ROM end: %08X+%X\n", devAddr, size);
        exit(4);
    }
    memcpy(dram, g_rom + devAddr, size);
    port_on_dma((u32) (uintptr_t) dram, devAddr, size);
    return 0;
}
s32 func_802DA2F0(void *mb, s32 pri, s32 dir, u32 devAddr, void *dram, u32 size, void *mq) {
    return osPiStartDma(mb, pri, dir, devAddr, dram, size, mq);
}
s32 osRecvMesg(void *mq, void *msg, s32 flag) { (void) mq; (void) msg; (void) flag; return 0; }
void osInvalDCache(void *p, s32 n) { (void) p; (void) n; }
void osWritebackDCache(void *p, s32 n) { (void) p; (void) n; }
/* the platform layer's game hooks (include/game/port.h) */
void port_fe_loaded(void) {}
void port_spin(void) {}
void func_8029A7E4(const char *fmt, ...) { (void) fmt; }
void port_stub_hit(const char *name) {
    fprintf(stderr, "STUB called: %s\n", name);
    exit(3);
}

/* an access violation inside a case is reported as CRASH for that case */
static jmp_buf crash_jmp;
static volatile int crash_armed;
static LONG CALLBACK on_fault(EXCEPTION_POINTERS *ep) {
    if (crash_armed && ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        crash_armed = 0;
        longjmp(crash_jmp, 1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static uint32_t crc32(const uint8_t *p, uint32_t n) {
    uint32_t c = 0xFFFFFFFFu, k;
    while (n--) {
        c ^= *p++;
        for (k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1)));
    }
    return ~c;
}

static uint32_t rng_state;
static uint32_t rnd(void) {
    rng_state = rng_state * 1103515245u + 12345u;
    return rng_state >> 8;
}

#define HEAP     0x80100000u   /* texture / level loads go here */
#define PARAM    0x801F0000u   /* synthetic u16 table for texture types 4 and 5 */
#define HEAPPTR  0x80358070u   /* D_80358070 */

static void t5(void) {
    int flag, i;
    for (flag = 0; flag < 2; flag++)
        for (i = 0; i < 19; i++) {
            V8(0x802E8BF8) = (uint8_t) flag;   /* the game's own u8 store (00000.c) */
            printf("T5 %d.%02d %08X\n", flag, i, (unsigned) port_802E8BF8_word(i));
        }
}

static void t6(void) {
    uint32_t id, i, tab;
    /* big-endian, as in the game: these tables are texture entries loaded as
     * stored (type 0) data, which the port leaves in N64 byte order */
    for (i = 0; i < 0x200; i++) {
        uint16_t v = (uint16_t) (i * 0x2B7 + 0x55);
        V8(PARAM + i * 2) = (uint8_t) (v >> 8);
        V8(PARAM + i * 2 + 1) = (uint8_t) v;
    }
    V32(HEAPPTR) = HEAP;
    V8(0x803B9888) = 0;
    func_802A0700();                 /* DMAs the 0x8000-byte table to the heap */
    tab = V32(0x803B8D44);
    for (id = 0; id < 0xFFF; id += 7) {
        uint32_t rom = V32(tab + id * 8), next = V32(tab + id * 8 + 8);
        uint16_t type = V16(tab + id * 8 + 6);
        uint32_t buf = 0x80180000u, n;
        if (next <= rom || next - rom > 0x8000 || type > 6) continue;
        V32(HEAPPTR) = buf;
        func_802A0B34((s32) id, (type == 4 || type == 5) ? (u8 *) (uintptr_t) PARAM : NULL);
        n = V32(HEAPPTR) - buf;
        printf("T6 %03X %d %05X %08X\n", id, type, n, crc32((const uint8_t *) (uintptr_t) buf, n));
    }
}

/* level loads: returns the decompressed size or 0 */
static s32 load_level(int lvl) {
    s32 size = 0;
    func_8025615C(lvl, (s32) HEAP, &size);
    return size;
}

static void t7(void) {
    int lvl;
    for (lvl = 0; lvl < 60; lvl++) {
        uint32_t e, end;
        s32 size = load_level(lvl);
        if (setjmp(crash_jmp)) {
            printf("T7 %02d.size CRASH\n", lvl);
            continue;
        }
        crash_armed = 1;
        func_802A2C54((u8 *) (uintptr_t) HEAP);
        crash_armed = 0;
        end = V32(0x803BDFD4);
        printf("T7 %02d.size %05X first %02X n %u\n", lvl, (unsigned) size, V8(0x80364A6E),
               (unsigned) ((end - 0x803BDFD8u) / 0x24));
        for (e = 0x803BDFD8u; e < end && e < 0x803BDFD8u + 0x24 * 400; e += 0x24) {
            uint32_t k, n = V8(e + 0x13);
            printf("T7 %02d.%04X %08X %08X %08X %08X %02X %02X %02X %02X %02X ", lvl, (unsigned) ((e - 0x803BDFD8u) / 0x24),
                   V32(e), V32(e + 4), V32(e + 8), V32(e + 0xC), V8(e + 0x10), V8(e + 0x11), V8(e + 0x12),
                   V8(e + 0x13), V8(e + 0x14));
            for (k = 0; k < n && k < 0xF; k++) printf("%02X", V8(e + 0x15 + k));
            printf("\n");
        }
    }
}

static void t8(void) {
    int lvl, i;
    rng_state = 8;
    for (lvl = 0; lvl < 60; lvl++) {
        load_level(lvl);
        for (i = 0; i < 40; i++) {
            uint32_t a = (rnd() % 0x400) << 5;
            a |= rnd() & 31;
            V32(0x803643E0) = a;
            a = (rnd() % 0x400) << 5;
            a |= rnd() & 31;
            V32(0x803643E8) = a;
            V16(0x80364450) = rnd() & 0xFFFF;
            V16(0x8036444E) = 0x1234;
            V8(0x80364411) = 0x55;
            if (setjmp(crash_jmp)) {
                printf("T8 %02d.%02d CRASH\n", lvl, i);
                continue;
            }
            crash_armed = 1;
            func_802A5510((u8 *) (uintptr_t) HEAP);
            crash_armed = 0;
            printf("T8 %02d.%02d %04X %02X\n", lvl, i, V16(0x8036444E), V8(0x80364411));
        }
    }
}

/* ---- T9: every read the original code made of each traced asset ----
 * (tools/widths.py facts over a tools/m64widths.py trace; local, not
 * committed).  Right after the asset arrives and port_on_dma/port_on_load
 * ran, a native read of WIDTH bytes at OFF must give the big-endian value the
 * N64 read there; bytes read only bytewise must be unchanged. */
void func_802C4108(u8 **src, u8 **dst, s32 work);
void func_802C4070(u32 *src, u32 *dst, u32 work, u8 type);

#define T9_DST   0x80100000u
#define T9_STAGE 0x8021ED00u

static int lzss_bits(uint32_t rom) {
    if (rom == 0x787F40u) return 10;   /* static segment: func_8028B4C4(.., 10, 0, 2) */
    return 13;                         /* sound banks, front-end blocks: (.., 0xD, 0, 2) */
}

static int t9_load(uint32_t rom, uint32_t size, const char *how, uint8_t **raw) {
    uint32_t src, dst;
    if (size == 0 || size > 0xF0000 || rom + 0x100 > g_romsize) return 0;
    if (rom >= 0xCCE0u && rom < 0x350950u) return 0;   /* texture entries: decoded in place (T6) */
    /* texture bundles and pictures: tokens swapped at decode time, pictures
     * tinted bytewise in the port (196F0.c); the offset table 0x6EC4C0 shares
     * its heap buffer with later decodes in the trace */
    if (rom >= 0x66C900u && rom < 0x6E8980u && rom != 0x6A9F10u) return 0;
    if (rom == 0x6EC4C0u) return 0;
    if (!strcmp(how, "dma")) {
        memcpy((void *) (uintptr_t) T9_DST, g_rom + rom, size);
    } else {
        uint32_t n = g_romsize - rom < 0x25000 ? (uint32_t) (g_romsize - rom) : 0x25000;
        memcpy((void *) (uintptr_t) T9_STAGE, g_rom + rom, n);
        src = T9_STAGE;
        dst = T9_DST;
        if (g_rom[rom] == 0x1F && g_rom[rom + 1] == 0x8B) {
            int m;
            for (m = 0; m < 4 && dst - T9_DST < size; m++)
                func_802C4108((u8 **) &src, (u8 **) &dst, 0x8004B400);
        } else {
            func_802C4070(&src, &dst, 0x8004B400u, (u8) lzss_bits(rom));
        }
        if (dst - T9_DST < size) return 0;
    }
    *raw = malloc(size);
    memcpy(*raw, (void *) (uintptr_t) T9_DST, size);
    if (!strcmp(how, "dma"))
        port_on_dma(T9_DST, rom, size);
    else
        port_on_load(rom, T9_DST, size);
    return 1;
}

static void t9(const char *path) {
    FILE *f = fopen(path, "r");
    char line[256], how[8] = "";
    uint8_t *raw = NULL;
    uint32_t rom = 0, size = 0, nf = 0, bad = 0, multi = 0, assets = 0, badassets = 0, tf = 0, tbad = 0;
    int have = 0;
    char first[96] = "";
    if (!f) {
        printf("T9 no facts file %s\n", path);
        return;
    }
    for (;;) {
        int eof = !fgets(line, sizeof line, f);
        if (eof || line[0] == '@') {
            if (have) {
                printf("T9 %06X %s size %X facts %u bad %u multi %u%s%s\n", rom, how, size, nf, bad, multi,
                       bad ? "  e.g. " : "", first);
                assets++;
                badassets += bad != 0;
                tf += nf;
                tbad += bad;
            }
            free(raw);
            raw = NULL;
            have = 0;
            if (eof) break;
            if (sscanf(line, "@ %x %x %7s", &rom, &size, how) != 3) continue;
            have = t9_load(rom, size, how, &raw);
            nf = bad = multi = 0;
            first[0] = 0;
            continue;
        }
        if (have && line[0] != '#') {
            unsigned off, w;
            uint64_t be = 0, nat = 0;
            uint32_t k;
            if (sscanf(line, "%x %u", &off, &w) != 2 || off + w > size) continue;
            if (strchr(line, ',')) {
                multi++;
                continue;
            }
            for (k = 0; k < w; k++) be = (be << 8) | raw[off + k];
            memcpy(&nat, (void *) (uintptr_t) (T9_DST + off), w);
            nf++;
            if (be != nat) {
                if (!bad) snprintf(first, sizeof first, "+%X w%u N64 %llX native %llX", off, w,
                                   (unsigned long long) be, (unsigned long long) nat);
                bad++;
            }
        }
    }
    fclose(f);
    printf("T9 total: %u assets, %u with mismatches; %u reads checked, %u wrong\n", assets, badassets, tf, tbad);
}

int main(int argc, char **argv) {
    const char *rom = argc > 1 ? argv[1] : "baserom.us.v11.z64";
    const char *tests = argc > 2 ? argv[2] : "5678";
    setvbuf(stdout, NULL, _IOFBF, 1 << 16);
    AddVectoredExceptionHandler(1, on_fault);
    if (rdram_map() || rdram_load(rom)) return 1;
    g_rom = rom_bytes(&g_romsize);
    port_load_verbose = getenv("PORT_LOAD_VERBOSE") != NULL;
    if (strchr(tests, '5')) t5();
    if (strchr(tests, '6')) t6();
    if (strchr(tests, '7')) t7();
    if (strchr(tests, '8')) t8();
    if (strchr(tests, '9')) t9(argc > 3 ? argv[3] : "build/lc/facts.txt");
    fflush(stdout);
    port_load_report();
    return 0;
}
