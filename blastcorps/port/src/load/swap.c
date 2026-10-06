/* Byte-swap primitives and the data-image swap (see port_load.h). */
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
