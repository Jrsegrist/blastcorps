/* INTERIM byte-order shims, so the platform layer can be exercised past
 * boot before the "load" layer (port/src/load/: the real port_on_dma and the
 * load-time image swap) is merged.  Built only with `make headless
 * INTERIM=1`; delete once src/load/ exists.  Deliberately minimal:
 *  - the hd_code/front-end data image swapped by the typemap.py leaves
 *    (generated table interim_typemap.c), skipping objects that got native
 *    initialisers;
 *  - port_on_dma for the sequence file header (ROM 0x44F5C0) and the
 *    compressed-MIDI header of each sequence;
 *  - alBnkfNew wrapped (-Wl,--wrap) to swap a decompressed .ctl bank. */
#include <stdint.h>
#include <string.h>
#include "rdram.h"
#include "port_dma.h"
#include "plat_host.h"

typedef struct {
    uint32_t addr;
    uint8_t width;
} TmLeaf;
extern const TmLeaf interim_typemap[];
extern const unsigned interim_typemap_count;

static uint16_t sw16(uint16_t v) {
    return (uint16_t) ((v >> 8) | (v << 8));
}
static void swap16at(uint8_t *p) {
    uint16_t v;
    memcpy(&v, p, 2);
    v = sw16(v);
    memcpy(p, &v, 2);
}
static void swap32at(uint8_t *p) {
    uint32_t v;
    memcpy(&v, p, 4);
    v = __builtin_bswap32(v);
    memcpy(p, &v, 4);
}
static uint32_t rd32(const uint8_t *p) {
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}

static int native_initialised(uint32_t a) {
    const PortCopy *c;
    for (c = port_copytab; c->name; c++)
        if (a >= c->addr && a < c->addr + (uint32_t) (c->end - c->start)) return 1;
    return 0;
}

void interim_swap_image(void) {
    unsigned i, n = 0, skipped = 0;
    for (i = 0; i < interim_typemap_count; i++) {
        const TmLeaf *l = &interim_typemap[i];
        if (native_initialised(l->addr)) {
            skipped++;
            continue;
        }
        if (l->width == 2) swap_range(l->addr, 1, 2);
        else if (l->width == 4) swap_range(l->addr, 1, 4);
        else if (l->width == 8) swap_range(l->addr, 1, 8);
        n++;
    }
    host_log("interim: swapped %u typed leaves of the data images (%u inside native initialisers)\n", n, skipped);
}

/* ---- sequences --------------------------------------------------------------- */
#define ROM_SEQFILE 0x44F5C0u
static uint32_t g_seq_count;          /* from the swapped header */
static uint32_t g_seq_off[128], g_seq_len[128];

void port_on_dma(uint32_t dst, uint32_t rom, uint32_t len) {
    uint8_t *p = (uint8_t *) (uintptr_t) dst;
    uint32_t i, o;
    if (rom == 0x4CE0u) {
        /* texture table (5BF40.c): {u32 rom; u16 size; u16 type} */
        for (o = 0; o + 8 <= len; o += 8) {
            swap32at(p + o);
            swap16at(p + o + 4);
            swap16at(p + o + 6);
        }
        return;
    }
    if (rom == ROM_SEQFILE) {
        /* ALSeqFile: s16 revision, s16 seqCount, {u8 *offset; s32 len}[] */
        if (len >= 4) {
            swap16at(p);
            swap16at(p + 2);
            g_seq_count = *(uint16_t *) (p + 2);
            if (g_seq_count > 128) g_seq_count = 128;
        }
        for (o = 4; o + 4 <= len && o < 4 + 8 * g_seq_count; o += 4) swap32at(p + o);
        for (i = 0; i < g_seq_count && 4 + 8 * i + 8 <= len; i++) {
            g_seq_off[i] = rd32(p + 4 + 8 * i);
            g_seq_len[i] = rd32(p + 8 + 8 * i);
        }
        return;
    }
    for (i = 0; i < g_seq_count; i++) {
        if (rom == ROM_SEQFILE + g_seq_off[i] && len >= 0x44) {
            /* ALCMidiHdr: s32 division, s32 trackOffset[16] */
            for (o = 0; o < 0x44; o += 4) swap32at(p + o);
            return;
        }
    }
}

/* ---- banks ------------------------------------------------------------------- */
#define MAXSEEN 4096
static uint32_t g_seen[MAXSEEN];
static unsigned g_nseen;
static int seen(uint32_t off) {
    unsigned i;
    for (i = 0; i < g_nseen; i++)
        if (g_seen[i] == off) return 1;
    if (g_nseen < MAXSEEN) g_seen[g_nseen++] = off;
    return 0;
}

static void swap_wavetable(uint8_t *f, uint32_t off) {
    uint8_t *w;
    uint32_t loop, book;
    if (off == 0 || seen(off)) return;
    w = f + off;
    swap32at(w);     /* base */
    swap32at(w + 4); /* len */
    swap32at(w + 12);
    loop = rd32(w + 12);
    if (w[8] == 0) { /* AL_ADPCM_WAVE: loop, book */
        swap32at(w + 16);
        book = rd32(w + 16);
        if (loop && !seen(loop)) {
            int k;
            for (k = 0; k < 12; k += 4) swap32at(f + loop + k);
            for (k = 0; k < 16; k++) swap16at(f + loop + 12 + 2 * k);
        }
        if (book && !seen(book)) {
            uint32_t order, npred, k;
            swap32at(f + book);
            swap32at(f + book + 4);
            order = rd32(f + book);
            npred = rd32(f + book + 4);
            for (k = 0; k < order * npred * 8 && k < 4096; k++) swap16at(f + book + 8 + 2 * k);
        }
    } else if (loop && !seen(loop)) { /* raw: ALRawLoop */
        int k;
        for (k = 0; k < 12; k += 4) swap32at(f + loop + k);
    }
}

static void swap_sound(uint8_t *f, uint32_t off) {
    uint8_t *s;
    uint32_t env;
    if (off == 0 || seen(off)) return;
    s = f + off;
    swap32at(s);
    swap32at(s + 4);
    swap32at(s + 8);
    env = rd32(s);
    if (env && !seen(env)) {
        swap32at(f + env);
        swap32at(f + env + 4);
        swap32at(f + env + 8);
    }
    swap_wavetable(f, rd32(s + 8));
}

static void swap_inst(uint8_t *f, uint32_t off) {
    uint8_t *in;
    uint32_t n, i;
    if (off == 0 || seen(off)) return;
    in = f + off;
    swap16at(in + 12);
    swap16at(in + 14);
    n = *(uint16_t *) (in + 14);
    for (i = 0; i < n; i++) {
        swap32at(in + 16 + 4 * i);
        swap_sound(f, rd32(in + 16 + 4 * i));
    }
}

static void swap_bankfile(uint8_t *f) {
    uint32_t nb, b;
    g_nseen = 0;
    swap16at(f);
    swap16at(f + 2);
    nb = *(uint16_t *) (f + 2);
    for (b = 0; b < nb; b++) {
        uint32_t boff, ni, i;
        uint8_t *bk;
        swap32at(f + 4 + 4 * b);
        boff = rd32(f + 4 + 4 * b);
        if (boff == 0 || seen(boff)) continue;
        bk = f + boff;
        swap16at(bk);
        swap32at(bk + 4);
        swap32at(bk + 8);
        ni = *(uint16_t *) bk;
        swap_inst(f, rd32(bk + 8));
        for (i = 0; i < ni; i++) {
            swap32at(bk + 12 + 4 * i);
            swap_inst(f, rd32(bk + 12 + 4 * i));
        }
    }
}

/* ---- decompressed assets (func_8028B4C4 wrapped) ------------------------------ */
/* Front-end scene blobs (0DE70.c func_801F4E70): a header of offsets, then
 * display lists: swapped as 32-bit words (vertex s16 pairs and texels come
 * out wrong, which only the renderer would notice). */
void __real_func_8028B4C4(uint32_t devAddr, uint32_t dest, uint32_t *size, uint8_t a3, uint8_t a4, uint8_t a5);
void __wrap_func_8028B4C4(uint32_t devAddr, uint32_t dest, uint32_t *size, uint8_t a3, uint8_t a4, uint8_t a5) {
    uint32_t rom = devAddr;
    __real_func_8028B4C4(devAddr, dest, size, a3, a4, a5);
    if (rom >= 0x6E8980u && rom < 0x6EC4C0u) {
        swap_range(dest, *size / 4, 4);
        if (host_verbose) host_log("interim: word-swapped FE scene 0x%X (%u bytes)\n", rom, *size);
    }
}

void __real_alBnkfNew(void *file, uint8_t *table);
void __wrap_alBnkfNew(void *file, uint8_t *table) {
    swap_bankfile(file);
    __real_alBnkfNew(file, table);
}
