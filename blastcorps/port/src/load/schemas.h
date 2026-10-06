/* Level / packed-object / model / static-segment / demo schemas (included
 * by port_assets.c).
 *
 * These assets are blocks whose header holds s32 offsets (from the block's
 * start) of their sections: section k = [base + hdr[slot], base + hdr[end]).
 * Sections overlap (several lists share one display-list area), so every
 * swap goes through a per-block byte map and each field is swapped once.
 * Sections not handled stay big-endian: byte streams the code reads bytewise
 * (the BE16U/BE16S/BE32 macros of 5CB60.c), byte tables, texels.
 *
 * Derived from the code that reads each section (5CB60.c, 5FD50.c, 60D50.c,
 * 62740.c, 77E20.c, 4B5E0.c, 479D0.c, 48D00.c ...) and checked against the
 * widths the NON_MATCHING code reads them at in the attract demos
 * (tools/m64widths.py, `make -C port loadcheck LC_TRACE=...`).
 */

typedef struct {
    uint32_t base, len;
    uint8_t *done;   /* per byte: already swapped (or deliberately kept) */
} Block;

static int blk_ok(Block *b, uint32_t off, uint32_t w) {
    uint32_t k;
    if (off + w > b->len) return 0;
    for (k = 0; k < w; k++)
        if (b->done[off + k]) return 0;
    return 1;
}

/* swap one W-byte field at OFF (once) */
static void blk_swap(Block *b, uint32_t off, int w) {
    if (!blk_ok(b, off, (uint32_t) w)) return;
    memset(b->done + off, 1, (size_t) w);
    port_bswap_n(P(b->base + off), 1, w);
}

/* mark bytes as handled without swapping them (byte data inside a range a
 * later, coarser rule would otherwise swap) */
static void blk_keep(Block *b, uint32_t off, uint32_t n) {
    if (off < b->len) memset(b->done + off, 1, off + n <= b->len ? n : b->len - off);
}

static void blk_n(Block *b, uint32_t off, uint32_t n, int w) {
    uint32_t i;
    for (i = 0; i < n; i++) blk_swap(b, off + i * (uint32_t) w, w);
}

static void blk_records(Block *b, uint32_t lo, uint32_t hi, uint32_t stride, const char *leaves) {
    Leaves l;
    uint32_t r;
    int i;
    parse_leaves(leaves, &l);
    for (r = lo; r + stride <= hi; r += stride)
        for (i = 0; i < l.n; i++) blk_swap(b, r + l.f[i].off, l.f[i].w);
}

static uint32_t blk_rd32(Block *b, uint32_t off) { return off + 4 <= b->len ? rd32(b->base + off) : 0; }
static uint32_t blk_be32(Block *b, uint32_t off) {
    uint8_t *p = P(b->base + off);
    return off + 4 <= b->len ? (uint32_t) p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3] : 0;
}

/* section [hdr[s], hdr[e]) if sane, after the header was swapped */
static int blk_sect(Block *b, uint32_t s, uint32_t e, uint32_t *lo, uint32_t *hi) {
    *lo = blk_rd32(b, s);
    *hi = blk_rd32(b, e);
    return *lo <= *hi && *hi <= b->len;
}

/* ------------------------------------------------------------------ levels */

/* 0x2C: texture records {u32 id; u8 count @4; u8 @5, @6; s16 @8, @A; then
 * count - 1 u32 texture ids from +0xC} (func_802A1C20, func_802A5020) */
static void lvl_texrecs(Block *b, uint32_t lo, uint32_t hi) {
    while (lo + 0xC <= hi) {
        uint32_t n = P(b->base + lo)[4];
        if (n == 0) break;
        blk_swap(b, lo, 4);
        blk_n(b, lo + 8, 2, 2);
        blk_n(b, lo + 0xC, n - 1, 4);
        lo += 0xC + (n - 1) * 4;
    }
}

/* 0x30: collision cells: a big-endian u32 (read bytewise, BE32: the offset of
 * the next cell from the section start), then 0x14-byte triangles (nine s16,
 * then two bytes) up to it (func_802A4464, func_802AA094) */
static void lvl_cells(Block *b, uint32_t lo, uint32_t hi) {
    uint32_t p = lo;
    while (p + 4 <= hi) {
        uint32_t next = lo + blk_be32(b, p);
        blk_keep(b, p, 4);
        if (next <= p || next > hi) break;
        blk_records(b, p + 4, next, 0x14, "2@0 2@2 2@4 2@6 2@8 2@A 2@C 2@E 2@10");
        p = next;
    }
}

/* 0x3C: lifts (func_8028FDA0): s16 n1, n1 x {s16 x, y, z, type}; s16 n2,
 * n2 x {s16 x, y, z; u8 type, n; s16 @8; n 0x16-byte triangle items read
 * bytewise (func_802A41B0, BE16S)} */
static void lvl_lifts(Block *b, uint32_t lo, uint32_t hi) {
    uint32_t p = lo, i, n;
    if (p + 2 > hi) return;
    blk_swap(b, p, 2);
    n = (uint16_t) rd16(b->base + p);
    p += 2;
    for (i = 0; i < n && p + 8 <= hi; i++, p += 8) blk_n(b, p, 4, 2);
    if (p + 2 > hi) return;
    blk_swap(b, p, 2);
    n = (uint16_t) rd16(b->base + p);
    p += 2;
    for (i = 0; i < n && p + 10 <= hi; i++) {
        uint32_t items = P(b->base + p)[7];
        blk_n(b, p, 3, 2);
        blk_swap(b, p + 8, 2);
        blk_keep(b, p + 10, items * 0x16);
        p += 10 + items * 0x16;
    }
}

/* 0x74: group chain (func_802A1A9C, func_802C2054, func_802C1F30): s32 n (0 =
 * empty), then groups: {s16 next (-1 = last); s16 [9]; u16 mtxOffset[10] @0x14;
 * s32 count @0x28} + count x {s16 index[9]; u8 @0x12, @0x13; {s32 unk0;
 * s32 base[3]}[3] @0x14} */
static void lvl_groups(Block *b, uint32_t lo, uint32_t hi) {
    uint32_t p = lo + 4, n, i;
    int16_t next;
    if (lo + 4 > hi) return;
    blk_swap(b, lo, 4);
    if (rd32(b->base + lo) == 0) return;
    do {
        if (p + 0x2C > hi) return;
        blk_n(b, p, 10, 2);
        blk_n(b, p + 0x14, 10, 2);
        blk_swap(b, p + 0x28, 4);
        next = (int16_t) rd16(b->base + p);
        n = rd32(b->base + p + 0x28);
        p += 0x2C;
        for (i = 0; i < n && p + 0x44 <= hi; i++, p += 0x44) {
            blk_n(b, p, 9, 2);
            blk_keep(b, p + 0x12, 2);
            blk_n(b, p + 0x14, 12, 4);
        }
    } while (next != -1);
}

static void swap_level(uint32_t base, uint32_t len) {
    Block b;
    uint32_t lo, hi, s;
    b.base = base;
    b.len = len;
    b.done = calloc(len + 4, 1);
    /* header: u16 x 12 (grid and bounds, func_802A2D68), s32 at 0x18 and
     * 0x1C, then the s32 section offsets 0x20..0xC4 */
    blk_n(&b, 0, 12, 2);
    blk_n(&b, 0x18, (0xC8 - 0x18) / 4, 4);
    /* segment 8 (D_80364458 = level + 0xC8): Vtx up to the first section */
    blk_records(&b, 0xC8, blk_rd32(&b, 0x20), 0x10, "2@0 2@2 2@4 2@6 2@8 2@A");
    if (blk_sect(&b, 0x20, 0x24, &lo, &hi)) blk_records(&b, lo, hi, 2, "2@0");   /* BoxSpawn s16 x4 */
    if (blk_sect(&b, 0x24, 0x28, &lo, &hi))                                       /* triangles */
        blk_records(&b, lo, hi, 0x14, "2@0 2@2 2@4 2@6 2@8 2@A 2@C 2@E 2@10");
    if (blk_sect(&b, 0x28, 0x2C, &lo, &hi)) blk_records(&b, lo, hi, 2, "2@0");
    if (blk_sect(&b, 0x2C, 0x30, &lo, &hi)) lvl_texrecs(&b, lo, hi);
    if (blk_sect(&b, 0x30, 0x34, &lo, &hi)) lvl_cells(&b, lo, hi);
    if (blk_sect(&b, 0x34, 0x38, &lo, &hi)) blk_records(&b, lo, hi, 2, "2@0");   /* path s16 */
    if (blk_sect(&b, 0x38, 0x3C, &lo, &hi)) blk_records(&b, lo, hi, 0xC, "2@0 2@2 2@4 2@8 2@A");
    if (blk_sect(&b, 0x3C, 0x40, &lo, &hi)) lvl_lifts(&b, lo, hi);
    for (s = 0x40; s < 0x50; s += 4)                                              /* height zones etc. */
        if (blk_sect(&b, s, s + 4, &lo, &hi)) blk_records(&b, lo, hi, 2, "2@0");
    /* 0x50, 0x54: byte streams (BE16S); 0x5C: 14-byte placements, BE16U bytes
     * but for the u16 at 0xA and 0xC; 0x60..0x74: byte grids */
    if (blk_sect(&b, 0x50, 0x58, &lo, &hi)) blk_keep(&b, lo, hi - lo);
    if (blk_sect(&b, 0x5C, 0x60, &lo, &hi)) blk_records(&b, lo, hi, 0xE, "2@A 2@C");
    if (blk_sect(&b, 0x60, 0x74, &lo, &hi)) blk_keep(&b, lo, hi - lo);
    if (blk_sect(&b, 0x74, 0x78, &lo, &hi)) lvl_groups(&b, lo, hi);
    /* display lists and the visibility records/groups (0xA0..0xC0): all u32
     * words (func_802A08E4, func_802A1C88, func_802A4E4C, func_802A51FC) */
    if (blk_sect(&b, 0xA0, 0xC0, &lo, &hi)) blk_records(&b, lo, hi, 4, "4@0");
    if (blk_sect(&b, 0x78, 0x84, &lo, &hi)) blk_records(&b, lo, hi, 4, "4@0");
    if (blk_sect(&b, 0x88, 0x9C, &lo, &hi)) blk_records(&b, lo, hi, 4, "4@0");
    free(b.done);
}

static int schema_apply(int kind, uint32_t dst, uint32_t rom, uint32_t len) {
    (void) rom;
    switch (kind) {
        case K_LEVEL:
            swap_level(dst, len);
            return 1;
        default:
            return 0;
    }
}
