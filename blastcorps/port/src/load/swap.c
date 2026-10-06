/* Byte-swap primitives and the data-image swap (see port_load.h). */
#include <stdlib.h>
#include <string.h>
#include "rdram.h"
#include "load/port_load.h"

void port_bswap_n(void *p, uint32_t n, int width) {
    uint8_t *b = p;
    uint32_t i;
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

/* The parts of image [lo, hi) left as ROM bytes: neither swapped by the
 * table nor replaced by a native initialiser.  gfx_fix.c converts the
 * display lists, vertices and matrices the RSP finds there (port_load.h). */
static void register_raw(uint32_t lo, uint32_t hi, const PortSwapRun *runs) {
    uint8_t *typed = calloc(hi - lo, 1);
    const PortCopy *c;
    uint32_t a, start = 0;
    int raw = 0;
    if (typed == NULL) return;
    for (; runs->count; runs++) {
        uint32_t s = runs->addr, e = runs->addr + runs->count * runs->width;
        for (a = s < lo ? lo : s; a < e && a < hi; a++) typed[a - lo] = 1;
    }
    for (c = port_copytab; c->name; c++) {
        uint32_t e = c->addr + (uint32_t) (c->end - c->start);
        for (a = c->addr < lo ? lo : c->addr; a < e && a < hi; a++) typed[a - lo] = 1;
    }
    for (a = lo; a <= hi; a++) {
        int t = a == hi || typed[a - lo];
        if (!t && !raw) start = a, raw = 1;
        else if (t && raw) {
            if (a - start >= 8) port_gfx_raw_area(start, a - start);
            raw = 0;
        }
    }
    free(typed);
}

void port_load_image_hd(void) {
    port_swap_runs(port_swap_hd);
    register_raw(HD_DATA_VRAM, HD_DATA_VRAM + HD_DATA_SIZE, port_swap_hd);
}

void port_load_image_fe(void) {
    const PortCopy *c;
    port_swap_runs(port_swap_fe);
    /* the front end's own C data: native initialisers (the N64 reload resets
     * them too: the heap overwrites the overlay while a level runs) */
    for (c = port_copytab; c->name; c++)
        if (c->addr >= FE_DATA_VRAM && c->addr < FE_BSS_END)
            memcpy((void *) (uintptr_t) c->addr, c->start, c->end - c->start);
    register_raw(FE_DATA_VRAM, FE_DATA_END, port_swap_fe);
}
