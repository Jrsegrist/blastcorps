/* Fixed-RAM model: the N64's 8 MB of RDRAM mapped at 0x80000000 (KSEG0)
 * in a 32-bit large-address-aware process, so every N64 address the game
 * uses (absolute symbols, literals, pointers inside data) is a valid host
 * pointer with no translation. */
#ifndef PORT_RDRAM_H
#define PORT_RDRAM_H

#include <stddef.h>
#include <stdint.h>

#define RDRAM_BASE 0x80000000u
#define RDRAM_SIZE 0x00800000u

/* hd_code: data+rodata image, then .bss up to the end of the 4 MB bank */
#define HD_DATA_VRAM 0x802E8BD0u
#define HD_DATA_SIZE 0x00026A90u
#define HD_BSS_START 0x8030F660u
#define HD_BSS_END   0x80400000u
/* front end overlay: text (and its RSP ucode) then data+rodata, then .bss */
#define FE_VRAM      0x801E7000u
#define FE_DATA_VRAM 0x80208040u
#define FE_DATA_END  0x80210E90u   /* end of .data+.rodata (start of .bss) */
#define FE_BSS_END   0x8021ED00u

/* compressed segments in the ROM (gzip members; blastcorps.us.v11.yaml) */
#define ROM_HD_TEXT  0x787FD0u
#define ROM_HD_DATA  0x7D73B4u
#define ROM_FE_TEXT  0x7E3AD0u
#define ROM_FE_DATA  0x7F63A1u

typedef struct {
    uint32_t addr;
    const char *start, *end;
    const char *name;
    uint32_t n64_size; /* st_size in the NON_MATCHING ELF (0 = unknown) */
} PortCopy;

extern const PortCopy port_copytab[];

/* Reserve+commit RDRAM at 0x80000000.  Returns 0 on success. */
int rdram_map(void);
/* Read the ROM file, inflate the data segments into place, zero .bss,
 * copy the native initialisers of pinned objects.  Returns 0 on success. */
int rdram_load(const char *rom_path);

uint8_t *rom_bytes(size_t *size);

/* gunzip one member at ROM offset OFF into DST (max DSTMAX); returns size or -1 */
long rom_gunzip(uint32_t off, void *dst, size_t dstmax);

/* byte-swap N elements of WIDTH bytes (2, 4 or 8) in place */
void swap_range(uint32_t addr, uint32_t n, int width);

/* in inflate.c: raw deflate */
long inflate_raw(const uint8_t *src, size_t srclen, uint8_t *dst, size_t dstmax);

#endif
