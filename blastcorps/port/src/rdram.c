#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdram.h"
#include "load/port_load.h"

static uint8_t *g_rom;
static size_t g_rom_size;
static char g_err[1024];

const char *rdram_error(void) {
    return g_err;
}

/* record (and print) why loading failed; returns -1 */
static int fail(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_err, sizeof g_err, fmt, ap);
    va_end(ap);
    fprintf(stderr, "rdram: %s\n", g_err);
    return -1;
}

/* fopen for a UTF-8 path (bc.exe's paths), else the ANSI code page */
static FILE *open_path(const char *path, const char *mode) {
    wchar_t wp[4096], wm[8];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wp, 4096) > 0 &&
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, wm, 8) > 0)
        return _wfopen(wp, wm);
    return fopen(path, mode);
}

int rom_normalise(uint8_t *p, size_t n) {
    size_t i;
    if (n < 4) return -1;
    if (p[0] == 0x80 && p[1] == 0x37 && p[2] == 0x12 && p[3] == 0x40) return ROM_Z64;
    if (p[0] == 0x37 && p[1] == 0x80 && p[2] == 0x40 && p[3] == 0x12) {   /* .v64: bytes swapped in pairs */
        for (i = 0; i + 1 < n; i += 2) {
            uint8_t t = p[i];
            p[i] = p[i + 1];
            p[i + 1] = t;
        }
        return ROM_V64;
    }
    if (p[0] == 0x40 && p[1] == 0x12 && p[2] == 0x37 && p[3] == 0x80) {   /* .n64: little-endian words */
        for (i = 0; i + 3 < n; i += 4) {
            uint32_t v;
            memcpy(&v, p + i, 4);
            v = __builtin_bswap32(v);
            memcpy(p + i, &v, 4);
        }
        return ROM_N64;
    }
    return -1;
}

int rdram_map(void) {
    void *p = VirtualAlloc((void *) (uintptr_t) RDRAM_BASE, RDRAM_SIZE, MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
    if (p != (void *) (uintptr_t) RDRAM_BASE) {
        MEMORY_BASIC_INFORMATION mbi;
        unsigned long err = GetLastError();
        if (VirtualQuery((void *) (uintptr_t) RDRAM_BASE, &mbi, sizeof mbi))
            fprintf(stderr, "rdram: region there: alloc base %p state 0x%lx type 0x%lx size 0x%lx\n",
                    mbi.AllocationBase, mbi.State, mbi.Type, (unsigned long) mbi.RegionSize);
        return fail("VirtualAlloc(0x%08X) failed (got %p, error %lu); is the exe linked with "
                    "--large-address-aware?", RDRAM_BASE, p, err);
    }
    return 0;
}

uint8_t *rom_bytes(size_t *size) {
    if (size) *size = g_rom_size;
    return g_rom;
}

/* the ROM file into g_rom, in .z64 (big-endian) order */
static int load_rom(const char *path) {
    FILE *f = open_path(path, "rb");
    long n;
    if (!f) return fail("Can't open the ROM file %s", path);
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0x800000 || n > 0x4000000) {
        fclose(f);
        return fail("%s is not a Blast Corps ROM (%ld bytes; the game is 8 MB)", path, n);
    }
    g_rom_size = (size_t) n;
    g_rom = malloc(g_rom_size);
    if (g_rom == NULL || fread(g_rom, 1, g_rom_size, f) != g_rom_size) {
        fclose(f);
        return fail("Can't read the ROM file %s", path);
    }
    fclose(f);
    if (rom_normalise(g_rom, g_rom_size) < 0)
        return fail("%s is not an N64 ROM image (.z64, .v64 or .n64)", path);
    return 0;
}

long rom_gunzip(uint32_t off, void *dst, size_t dstmax) {
    const uint8_t *p = g_rom + off;
    size_t i = 10;
    if (p[0] != 0x1F || p[1] != 0x8B || p[2] != 8) return -1;
    if (p[3] & 4) i += 2 + (p[i] | (p[i + 1] << 8));
    if (p[3] & 8) while (p[i++]) {}
    if (p[3] & 16) while (p[i++]) {}
    if (p[3] & 2) i += 2;
    return inflate_raw(p + i, g_rom_size - off - i, dst, dstmax);
}

void swap_range(uint32_t addr, uint32_t n, int width) {
    uint8_t *p = (uint8_t *) (uintptr_t) addr;
    uint32_t i;
    for (i = 0; i < n; i++, p += width) {
        if (width == 2) {
            uint8_t t = p[0]; p[0] = p[1]; p[1] = t;
        } else if (width == 4) {
            uint32_t v;
            memcpy(&v, p, 4);
            v = __builtin_bswap32(v);
            memcpy(p, &v, 4);
        } else if (width == 8) {
            uint64_t v;
            memcpy(&v, p, 8);
            v = __builtin_bswap64(v);
            memcpy(p, &v, 8);
        }
    }
}

int rdram_load(const char *rom_path) {
    long n;
    const PortCopy *c;
    int copies = 0;
    if (load_rom(rom_path)) return -1;
    /* hd_code .data/.rodata at its link address (the code half is native) */
    n = rom_gunzip(ROM_HD_DATA, (void *) (uintptr_t) HD_DATA_VRAM, HD_BSS_START - HD_DATA_VRAM);
    if (n != (long) HD_DATA_SIZE)
        return fail("%s: the game data inflated to %ld bytes, expected 0x%X (not Blast Corps (USA) (Rev 1)?)",
                    rom_path, n, HD_DATA_SIZE);
    /* front end: text+ucode then data (the game reloads it per menu visit;
     * here once).  Its .text bytes are only needed for the RSP ucode. */
    n = rom_gunzip(ROM_FE_TEXT, (void *) (uintptr_t) FE_VRAM, FE_DATA_VRAM - FE_VRAM);
    if (n != (long) (FE_DATA_VRAM - FE_VRAM))
        return fail("%s: the front-end code inflated to %ld bytes, expected 0x%X", rom_path, n,
                    FE_DATA_VRAM - FE_VRAM);
    n = rom_gunzip(ROM_FE_DATA, (void *) (uintptr_t) FE_DATA_VRAM, FE_BSS_END - FE_DATA_VRAM);
    if (n <= 0) return fail("%s: the front-end data didn't inflate", rom_path);
    memset((void *) (uintptr_t) (FE_DATA_VRAM + n), 0, FE_BSS_END - FE_DATA_VRAM - n);
    memset((void *) (uintptr_t) HD_BSS_START, 0, HD_BSS_END - HD_BSS_START);
    /* the images' ROM bytes into host order (port/src/load/: generated swap
     * table), before the native initialisers go over the C-defined objects */
    port_load_image_hd();
    port_load_image_fe();
    /* objects defined in the linked C: their native (host-order) initialisers */
    for (c = port_copytab; c->name; c++) {
        if (c->n64_size && c->n64_size != (uint32_t) (c->end - c->start))
            fprintf(stderr, "rdram: WARNING %s: native size 0x%X, N64 size 0x%X\n", c->name,
                    (unsigned) (c->end - c->start), c->n64_size);
        memcpy((void *) (uintptr_t) c->addr, c->start, c->end - c->start);
        copies++;
    }
    printf("rdram: hd data 0x%X bytes at 0x%08X, front end data 0x%lX bytes at 0x%08X, %d native "
           "initialisers copied\n", HD_DATA_SIZE, HD_DATA_VRAM, n, FE_DATA_VRAM, copies);
    return 0;
}
