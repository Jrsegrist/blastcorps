/* Byte-swap primitives and the data-image swap (see port_load.h). */
#include <stdio.h>
#include <string.h>
#include "rdram.h"
#include "load/port_load.h"

/* Per RDRAM byte, the unit the host holds it in (see port_load.h
 * port_n64_byte): (offset in the unit << 4) | width, 0 = never recorded. */
static uint8_t unit_map[0x800000];

void port_unit_mark(void *p, uint32_t n, int width) {
    uint32_t a = (uint32_t) (uintptr_t) p, i;
    int k;
    if (a < 0x80000000u || a + n * (uint32_t) width > 0x80800000u) return;
    a -= 0x80000000u;
    for (i = 0; i < n; i++, a += (uint32_t) width)
        for (k = 0; k < width; k++) unit_map[a + k] = (uint8_t) (k << 4 | width);
}

void port_garbage(const void *p, uint32_t len) {
    if (port_load_verbose)
        fprintf(stderr, "load: garbage %08X len %X\n", (unsigned) (uintptr_t) p, (unsigned) len);
}

uint8_t port_n64_byte(const void *p) {
    uint32_t a = (uint32_t) (uintptr_t) p;
    const uint8_t *b = p;
    int m, w, k;
    if (a < 0x80000000u || a >= 0x80800000u) return *b;
    m = unit_map[a - 0x80000000u];
    w = m & 15;
    k = m >> 4;
    if (w < 2) return *b;
    return b[w - 1 - 2 * k];
}

void port_bswap_n(void *p, uint32_t n, int width) {
    uint8_t *b = p;
    uint32_t i;
    port_unit_mark(p, n, width);
    switch (width) {
        case 2:
            for (i = 0; i < n; i++, b += 2) {
                uint16_t v;
                memcpy(&v, b, 2);
                v = __builtin_bswap16(v);
                memcpy(b, &v, 2);
            }
            break;
        case 4:
            for (i = 0; i < n; i++, b += 4) {
                uint32_t v;
                memcpy(&v, b, 4);
                v = __builtin_bswap32(v);
                memcpy(b, &v, 4);
            }
            break;
        case 8:
            for (i = 0; i < n; i++, b += 8) {
                uint64_t v;
                memcpy(&v, b, 8);
                v = __builtin_bswap64(v);
                memcpy(b, &v, 8);
            }
            break;
    }
}

uint32_t port_swap_runs(const PortSwapRun *r) {
    uint32_t bytes = 0;
    for (; r->count; r++) {
        port_bswap_n((void *) (uintptr_t) r->addr, r->count, (int) r->width);
        bytes += r->count * r->width;
    }
    return bytes;
}

void port_load_image_hd(void) {
    port_swap_runs(port_swap_hd);
}

void port_load_image_fe(void) {
    const PortCopy *c;
    port_swap_runs(port_swap_fe);
    /* the front end's own C data: native initialisers (the N64 reload resets
     * them too: the heap overwrites the overlay while a level runs) */
    for (c = port_copytab; c->name; c++)
        if (c->addr >= FE_DATA_VRAM && c->addr < FE_BSS_END)
            memcpy((void *) (uintptr_t) c->addr, c->start, c->end - c->start);
}
