#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdram.h"
#include "load/port_load.h"

static uint8_t *g_rom;
static size_t g_rom_size;

int rdram_map(void) {
    void *p = VirtualAlloc((void *) (uintptr_t) RDRAM_BASE, RDRAM_SIZE, MEM_RESERVE | MEM_COMMIT,
                           PAGE_READWRITE);
    if (p != (void *) (uintptr_t) RDRAM_BASE) {
        MEMORY_BASIC_INFORMATION mbi;
        fprintf(stderr, "rdram: VirtualAlloc(0x%08X) failed (got %p, error %lu)\n", RDRAM_BASE, p,
                GetLastError());
        if (VirtualQuery((void *) (uintptr_t) RDRAM_BASE, &mbi, sizeof mbi))
            fprintf(stderr, "rdram: region there: alloc base %p state 0x%lx type 0x%lx size 0x%lx\n",
                    mbi.AllocationBase, mbi.State, mbi.Type, (unsigned long) mbi.RegionSize);
        fprintf(stderr, "rdram: (is the exe linked with --large-address-aware?)\n");
        return -1;
    }
    return 0;
}

uint8_t *rom_bytes(size_t *size) {
    if (size) *size = g_rom_size;
    return g_rom;
}

static int load_rom(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "rdram: can't open ROM %s\n", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    g_rom_size = (size_t) ftell(f);
    fseek(f, 0, SEEK_SET);
    g_rom = malloc(g_rom_size);
    if (fread(g_rom, 1, g_rom_size, f) != g_rom_size) {
        fclose(f);
        return -1;
    }
    fclose(f);
    if (g_rom_size < 0x800000 || g_rom[0] != 0x80 || g_rom[1] != 0x37) {
        fprintf(stderr, "rdram: %s is not a big-endian (.z64) Blast Corps ROM\n", path);
        return -1;
    }
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
    if (n != (long) HD_DATA_SIZE) {
        fprintf(stderr, "rdram: hd_code data inflated to %ld bytes, expected 0x%X\n", n, HD_DATA_SIZE);
        return -1;
    }
    /* front end: text+ucode then data (the game reloads it per menu visit;
     * here once).  Its .text bytes are only needed for the RSP ucode. */
    n = rom_gunzip(ROM_FE_TEXT, (void *) (uintptr_t) FE_VRAM, FE_DATA_VRAM - FE_VRAM);
    if (n != (long) (FE_DATA_VRAM - FE_VRAM)) {
        fprintf(stderr, "rdram: front-end text inflated to %ld bytes, expected 0x%X\n", n,
                FE_DATA_VRAM - FE_VRAM);
        return -1;
    }
    n = rom_gunzip(ROM_FE_DATA, (void *) (uintptr_t) FE_DATA_VRAM, FE_BSS_END - FE_DATA_VRAM);
    if (n <= 0) {
        fprintf(stderr, "rdram: front-end data inflate failed\n");
        return -1;
    }
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
