/* Graphics data inside display-list areas: see port_load.h.
 *
 * Example: a front-end scene (ninlogo, ROM 0x6EA850) is offsets, vertices
 * and display lists in one block.  The schema swaps the block as u32 words
 * (the front end reads its offsets and commands as words), which turns each
 * Vtx {s16 x, y, z, flag, s, t; u8 r, g, b, a} into two halfword pairs and a
 * byte-reversed colour word: the emulator's RDRAM layout.  The display list
 * reaches them through segment 6 (G_VTX 0x06000080), which only the running
 * game knows, so the conversion happens when a task is about to be drawn,
 * following the commands with the real segment table:
 *   G_VTX                 vertices: halves 0-5 to host order, colour bytes back
 *   G_MOVEMEM viewport    four halfword pairs to host order
 *   G_MOVEMEM light/lookAt  bytes back to their N64 order
 *   G_SETTIMG + loads     texels/palettes back to big-endian bytes
 * Only bytes inside a registered area are touched, each word once per load
 * (a per-word map), so data the game builds itself is never changed. */
#include <stdlib.h>
#include <string.h>
#include "rdram.h"
#include "port_load.h"

#define MAX_AREAS 64

typedef struct {
    uint32_t lo, hi;   /* physical [lo, hi) */
    uint8_t *done;     /* per word: already in the native layout */
} Area;

static Area g_area[MAX_AREAS];
static int g_nareas;

static uint32_t phys(uint32_t a) {
    return a & 0x00FFFFFFu;
}

void port_gfx_forget(uint32_t addr, uint32_t len) {
    uint32_t lo = phys(addr), hi = lo + len;
    int i;
    for (i = 0; i < g_nareas; i++) {
        if (g_area[i].lo < hi && lo < g_area[i].hi) {
            free(g_area[i].done);
            g_area[i] = g_area[--g_nareas];
            i--;
        }
    }
}

void port_gfx_word_area(uint32_t addr, uint32_t len) {
    uint32_t lo = phys(addr) & ~3u, hi = (phys(addr) + len + 3) & ~3u;
    if (len == 0 || hi > RDRAM_SIZE) return;
    port_gfx_forget(lo, hi - lo);
    if (g_nareas == MAX_AREAS) {   /* the oldest goes */
        free(g_area[0].done);
        memmove(&g_area[0], &g_area[1], (MAX_AREAS - 1) * sizeof(Area));
        g_nareas--;
    }
    g_area[g_nareas].lo = lo;
    g_area[g_nareas].hi = hi;
    g_area[g_nareas].done = calloc((hi - lo) / 4, 1);
    g_nareas++;
}

enum { CONV_HALVES, CONV_BYTES, CONV_VTX };

static uint32_t g_converted;

static uint32_t *word_at(uint32_t pa) {
    return (uint32_t *) (uintptr_t) (RDRAM_BASE + pa);
}

/* convert the words of [lo, hi) that lie in an area and were not done yet */
static void convert(uint32_t lo, uint32_t hi, int how) {
    int i;
    lo &= ~3u;
    hi = (hi + 3) & ~3u;
    if (hi > RDRAM_SIZE || lo >= hi) return;
    for (i = 0; i < g_nareas; i++) {
        Area *ar = &g_area[i];
        uint32_t a = lo > ar->lo ? lo : ar->lo, b = hi < ar->hi ? hi : ar->hi;
        for (; a < b; a += 4) {
            uint32_t k = (a - ar->lo) / 4, *w, v;
            int as_bytes;
            if (ar->done[k]) continue;
            ar->done[k] = 1;
            w = word_at(a);
            v = *w;
            /* a Vtx: halves for words 0-2, bytes for the colour/normal word */
            as_bytes = how == CONV_BYTES || (how == CONV_VTX && ((a - lo) & 0xC) == 0xC);
            *w = as_bytes ? __builtin_bswap32(v) : (v >> 16) | (v << 16);
            g_converted += 4;
        }
    }
}

static int in_any_area(uint32_t lo, uint32_t hi) {
    int i;
    for (i = 0; i < g_nareas; i++)
        if (g_area[i].lo < hi && lo < g_area[i].hi) return 1;
    return 0;
}

uint32_t port_gfx_fix_task(uint32_t dl) {
    uint32_t seg[16], stack[10], pc = phys(dl);
    uint32_t timg = 0, timg_siz = 0, timg_width = 0;
    int sp = 0, steps = 0;
    if (g_nareas == 0) return 0;
    memset(seg, 0, sizeof seg);
    g_converted = 0;
#define SEGADDR(a) (phys(seg[((a) >> 24) & 15] + ((a) & 0x00FFFFFFu)) & (RDRAM_SIZE - 1))
    while (steps++ < 200000 && pc + 8 <= RDRAM_SIZE) {
        uint32_t w0 = word_at(pc)[0], w1 = word_at(pc)[1];
        pc += 8;
        switch (w0 >> 24) {
            case 0x03: {   /* G_MOVEMEM: index, size */
                uint32_t idx = (w0 >> 16) & 0xFF, size = w0 & 0xFFFF, a = SEGADDR(w1);
                if (idx == 0x80) convert(a, a + size, CONV_HALVES);                /* viewport */
                else if (idx >= 0x82 && idx <= 0x94) convert(a, a + size, CONV_BYTES); /* lookAt, lights */
                break;
            }
            case 0x04: {   /* G_VTX */
                uint32_t a = SEGADDR(w1), size = w0 & 0xFFFF;
                convert(a, a + size, CONV_VTX);
                break;
            }
            case 0x06:     /* G_DL: call or branch */
                if (((w0 >> 16) & 1) == 0) {
                    if (sp == 10) return g_converted;
                    stack[sp++] = pc;
                }
                pc = SEGADDR(w1);
                break;
            case 0xB8:     /* G_ENDDL */
                if (sp == 0) return g_converted;
                pc = stack[--sp];
                break;
            case 0xBC:     /* G_MOVEWORD G_MW_SEGMENT */
                if ((w0 & 0xFF) == 6) seg[((w0 >> 8) & 0xFFFF) / 4 & 15] = w1;
                break;
            case 0xFD:     /* G_SETTIMG */
                timg = SEGADDR(w1);
                timg_siz = (w0 >> 19) & 3;
                timg_width = (w0 & 0xFFF) + 1;
                break;
            case 0xF3: {   /* G_LOADBLOCK: lrs + 1 texels from uls */
                uint32_t uls = (w0 >> 12) & 0xFFF, lrs = (w1 >> 12) & 0xFFF;
                uint32_t a = timg + ((uls << timg_siz) >> 1), n = ((lrs - uls + 1) << timg_siz) >> 1;
                if (timg && lrs >= uls && in_any_area(a, a + n)) convert(a, a + n, CONV_BYTES);
                break;
            }
            case 0xF4: {   /* G_LOADTILE: a rectangle (10.2 coordinates) */
                uint32_t uls = ((w0 >> 12) & 0xFFF) >> 2, ult = (w0 & 0xFFF) >> 2;
                uint32_t lrs = ((w1 >> 12) & 0xFFF) >> 2, lrt = (w1 & 0xFFF) >> 2;
                uint32_t row = (timg_width << timg_siz) >> 1, t;
                if (!timg || lrs < uls || lrt < ult) break;
                for (t = ult; t <= lrt; t++) {
                    uint32_t a = timg + t * row + ((uls << timg_siz) >> 1);
                    uint32_t n = ((lrs - uls + 1) << timg_siz) >> 1;
                    if (in_any_area(a, a + n)) convert(a, a + n, CONV_BYTES);
                }
                break;
            }
            case 0xF0: {   /* G_LOADTLUT: count 16-bit entries */
                uint32_t n = (((w1 >> 14) & 0x3FF) + 1) * 2;
                if (timg && in_any_area(timg, timg + n)) convert(timg, timg + n, CONV_BYTES);
                break;
            }
            default:
                break;
        }
    }
    return g_converted;
#undef SEGADDR
}
