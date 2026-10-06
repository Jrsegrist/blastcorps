/* Level / packed-object / model / static-segment / demo schemas (included
 * by port_assets.c).
 *
 * These assets are blocks whose header holds s32 offsets (from the block's
 * start) of its sections: section k = [base + hdr[slot], base + hdr[end slot]).
 * A schema lists the fixed header leaves, the offset slots, and per section
 * the record stride and the multi-byte leaves of one record.  Sections not
 * listed stay big-endian: byte streams the code reads bytewise (the BE16U/
 * BE16S/BE32 macros of 5CB60.c), byte tables, texels.
 *
 * Derived from the code that reads each section (5CB60.c, 5FD50.c, 60D50.c,
 * 62740.c, 77E20.c ...) and checked against the widths the NON_MATCHING code
 * reads them at in the attract demos (tools/m64widths.py; README).
 */

typedef struct {
    uint16_t slot;      /* header offset of the s32 holding the section start */
    uint16_t end;       /* header offset of the s32 holding its end */
    uint16_t stride;    /* record size */
    const char *leaves; /* "W@OFF ..." multi-byte fields of one record (hex offsets) */
} Section;

typedef struct {
    const char *fixed;      /* fixed header leaves "W@OFF ..." (applied once) */
    uint16_t slots_lo;      /* s32 offset slots [slots_lo, slots_hi) */
    uint16_t slots_hi;
    const Section *sections;
} BlockSchema;

static void swap_block(const BlockSchema *s, uint32_t base, uint32_t len) {
    const Section *sec;
    if (s->fixed) swap_leaves(base, s->fixed);
    sw32(base + s->slots_lo, (s->slots_hi - s->slots_lo) / 4);
    for (sec = s->sections; sec && sec->stride; sec++) {
        uint32_t lo = rd32(base + sec->slot), hi = rd32(base + sec->end);
        if (lo > hi || hi > len) continue;     /* not this block's layout: leave it */
        swap_records(base + lo, hi - lo, sec->stride, sec->leaves);
    }
}

static int schema_apply(int kind, uint32_t dst, uint32_t rom, uint32_t len) {
    (void) rom;
    (void) dst;
    (void) len;
    switch (kind) {
        default:
            return 0;
    }
}
