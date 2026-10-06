/* Byte order on load (stage 3, "load").
 *
 * The RDRAM image is kept in host (little-endian) order: everything the
 * game's C reads natively must be swapped once, when it arrives from the ROM,
 * by the width the code reads it at.  Two entry points:
 *
 *  - the data images (hd_code .data/.rodata at 0x802E8BD0, the front end's at
 *    0x80208040): swapped by a table generated at build time
 *    (port/tools/swaptab.py: C declarations + traced read widths), right after
 *    they are inflated and before the native initialisers are copied in;
 *  - runtime assets: port_on_dma (every PI DMA, from the platform layer) and
 *    port_on_load (after the game decompresses a block: func_8028B4C4 and
 *    5CB60's packed-object loader), which look the ROM range up in the asset
 *    table (port_assets.c) and apply that asset type's schema.
 *
 * Textures, palettes, compressed streams and the byte streams the code reads
 * with BE16U/BE16S/BE32 or byte loads stay big-endian.
 */
#ifndef PORT_LOAD_H
#define PORT_LOAD_H

#include <stdint.h>

typedef struct {
    uint32_t addr;   /* N64 address of the first element */
    uint32_t count;  /* elements */
    uint32_t width;  /* 2, 4 or 8 */
} PortSwapRun;

/* generated (build/swaptab.c) */
extern const PortSwapRun port_swap_hd[];
extern const PortSwapRun port_swap_fe[];

/* swap N elements of WIDTH (2, 4, 8) bytes at host pointer P */
void port_bswap_n(void *p, uint32_t n, int width);
/* apply a run table (terminated by count 0); returns the bytes swapped */
uint32_t port_swap_runs(const PortSwapRun *runs);

/* After the hd_code data image is inflated (once, at boot). */
void port_load_image_hd(void);
/* After the front end is inflated to 0x801E7000 (boot and every reload):
 * swaps its data image, then copies the front end's native initialisers. */
void port_load_image_fe(void);

/* After the game decompressed ROM block ROM into DST (LEN bytes out). */
void port_on_load(uint32_t rom, uint32_t dst, uint32_t len);
/* (port/src/platform/port_dma.h) after every ROM -> RDRAM DMA. */
void port_on_dma(uint32_t dst, uint32_t rom, uint32_t len);

/* Statistics: per asset kind, how many loads and bytes were handled. */
void port_load_report(void);
/* 1: log every DMA/load and the kind it was classified as (stderr). */
extern int port_load_verbose;

#endif
