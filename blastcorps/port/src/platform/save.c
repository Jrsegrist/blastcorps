/* Save data in N64 byte order (PORT_SAVE_* hooks in hd_front_end/0E7B0.c,
 * include/game/port.h).
 *
 * The save thread (pfsHandler.c) hands the EEPROM and the Controller Pak the
 * game's records as raw bytes and puts a CRC over those bytes into the last
 * four.  On the N64 the bytes are big-endian; natively the multi-byte fields
 * are little-endian.  So that save files interchange with emulators (and the
 * CRC is the one an N64 would compute), each save routine works on a big-
 * endian copy: port_save_begin() returns a buffer holding the record in N64
 * byte order, the routine computes/checks the CRC and does the transfer on
 * it, and port_save_end() copies the buffer back in host order (including
 * whatever the routine changed: the CRC bytes, the data read).  The record
 * itself never holds big-endian fields while other threads can run.
 *
 * Layouts (every other byte is a u8):
 *   PLAYER   D_80364AF0[n], 0x100: u16 0x0A (rank points), u32 0x10 (vehicle
 *            unlocks), u32 0x14 (money), u32 0xF0 (vehicle flags); CRC 0xFC.
 *   TIMES    D_80364EF0[n], 0x20: u16[14] best times; CRC 0x1C.
 *   EETIMES  D_80364F70 block, 8: u16[4] (time, time ^ 0x55AA pairs).
 *   STATUS   D_8039C4B8, 0x40: a bit stream (bytes), or the u64 marker
 *            0x1234567887654321 at 0 ("no status"); CRC 0x3C.  Swapped in
 *            place (a macro array; the marker is the only multi-byte value).
 * The semaphore (u64 at EEPROM block 0x3F / pak offset 0xDE0) goes through
 * port_save_be64. */
#include "plat.h"

enum { KIND_PLAYER, KIND_TIMES, KIND_EETIMES, KIND_STATUS };

static u16 bswap16(u16 v) {
    return (u16) ((v >> 8) | (v << 8));
}
static u32 bswap32(u32 v) {
    return (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
}

static void swap16(u8 *p, int off) {
    u16 v;
    PORT_MEMCPY(&v, p + off, 2);
    v = bswap16(v);
    PORT_MEMCPY(p + off, &v, 2);
}
static void swap32(u8 *p, int off) {
    u32 v;
    PORT_MEMCPY(&v, p + off, 4);
    v = bswap32(v);
    PORT_MEMCPY(p + off, &v, 4);
}

/* host <-> N64 order (an involution) */
static void convert(u8 *p, int kind, int size) {
    int i;
    switch (kind) {
        case KIND_PLAYER:
            swap16(p, 0x0A);
            swap32(p, 0x10);
            swap32(p, 0x14);
            swap32(p, 0xF0);
            break;
        case KIND_TIMES:
            for (i = 0; i < 0x1C; i += 2) swap16(p, i);
            break;
        case KIND_EETIMES:
            for (i = 0; i < size; i += 2) swap16(p, i);
            break;
        case KIND_STATUS: {
            static const u8 le[8] = { 0x21, 0x43, 0x65, 0x87, 0x78, 0x56, 0x34, 0x12 };
            static const u8 be[8] = { 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21 };
            if (PORT_MEMCMP(p, le, 8) == 0) PORT_MEMCPY(p, be, 8);
            else if (PORT_MEMCMP(p, be, 8) == 0) PORT_MEMCPY(p, le, 8);
            break;
        }
    }
}

static u8 g_buf[0x100];
static u8 *g_orig;
static int g_kind, g_size;

void *port_save_begin(void *p, int kind, int size) {
    if (kind == KIND_STATUS) {
        convert(p, kind, size);
        return p;
    }
    if (g_orig != NULL) host_fatal("port_save_begin: nested save (kind %d in %d)", kind, g_kind);
    if (size > (int) sizeof g_buf) host_fatal("port_save_begin: size %d", size);
    g_orig = p;
    g_kind = kind;
    g_size = size;
    PORT_MEMCPY(g_buf, p, size);
    convert(g_buf, kind, size);
    return g_buf;
}

void *port_save_end(void *buf, int kind, int size) {
    u8 *orig;
    if (kind == KIND_STATUS) {
        convert(buf, kind, size);
        return buf;
    }
    if (buf != g_buf || g_orig == NULL || kind != g_kind) host_fatal("port_save_end: unmatched (kind %d)", kind);
    convert(g_buf, kind, g_size);
    PORT_MEMCPY(g_orig, g_buf, g_size);
    if (host_verbose)
        host_log("save: kind %d at %p done (frame %u): %02x %02x %02x %02x ... %02x %02x %02x %02x\n", kind,
                 (void *) g_orig, (unsigned) plat_stats.frames, g_buf[0], g_buf[1], g_buf[2], g_buf[3],
                 g_buf[g_size - 4], g_buf[g_size - 3], g_buf[g_size - 2], g_buf[g_size - 1]);
    orig = g_orig;
    g_orig = NULL;
    return orig;
}

/* func_801F6AF4 writes 0x20 bytes to the pak from the address of its u64
 * parameter: the semaphore and then whatever follows it on the stack.  On
 * the N64 (mupen64plus, original ROM, every write seen) that is
 * 00000000 00000000 00000000 802DC2C8 00000000 00000000; the port writes
 * those bytes rather than the host's stack. */
void *port_save_semblock(const void *sem) {
    static u8 block[0x20];
    static const u8 tail[0x18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x2D, 0xC2, 0xC8, 0, 0, 0, 0, 0, 0, 0, 0 };
    PORT_MEMCPY(block, sem, 8);
    PORT_MEMCPY(block + 8, tail, sizeof tail);
    return block;
}

/* for host-side logging (headless_main.c's --print) */
unsigned plat_frames(void) {
    return plat_stats.frames;
}

u64 port_save_be64(u64 v) {
    return ((u64) bswap32((u32) v) << 32) | bswap32((u32) (v >> 32));
}
