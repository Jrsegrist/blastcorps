/* SI: controllers from an input source, EEPROM as a file, no Controller Pak.
 *
 * Every SI transfer completes at once: the call that would start a PIF DMA
 * posts the SI event (OS_EVENT_SI) the hardware would raise, so code that
 * waits for it (osRecvMesg on the SI queue, or func_8028FCD4's poll of the
 * queue's validCount) continues straight away.
 *
 * EEPROM: 4 Kbit (64 blocks of 8 bytes), kept in memory and written back to
 * the file (--eeprom FILE) after each write, as raw bytes in EEPROM order.
 * A missing file starts as all 0xFF (a blank chip, as mupen64plus formats
 * it).  NOTE: the game writes its save structs through this as native
 * (little-endian) bytes; an N64-compatible file needs the save layout
 * swapped game-side (port_followups OPEN b).
 *
 * Controller Pak: not present (osPfsInitPak -> PFS_ERR_NOPACK).  A pak image
 * needs the front end's Pfs code (hd_front_end/ul_pfs*.c) compiled natively
 * over an emulated __osContRamRead/Write; not done yet. */
#include "plat.h"

#define EEPROM_BLOCKS 64
static u8 g_eeprom[EEPROM_BLOCKS * 8];
static int g_eeprom_dirty;

static u16 g_latch_button;
static s8 g_latch_x, g_latch_y;
static int g_cont_inited;

void plat_si_init(void) {
    unsigned size = 0;
    u8 *p;
    __builtin_memset(g_eeprom, 0xFF, sizeof g_eeprom);
    if (plat_cfg.eeprom_path != NULL && (p = host_read_file(plat_cfg.eeprom_path, &size)) != NULL) {
        __builtin_memcpy(g_eeprom, p, size < sizeof g_eeprom ? size : sizeof g_eeprom);
    }
}

void plat_si_flush(void) {
    if (g_eeprom_dirty && plat_cfg.eeprom_path != NULL) {
        if (host_write_file(plat_cfg.eeprom_path, g_eeprom, sizeof g_eeprom) != 0)
            host_log("si: can't write %s\n", plat_cfg.eeprom_path);
        g_eeprom_dirty = 0;
    }
}

static void si_done(void) {
    plat_post_event(OS_EVENT_SI);
}

/* ---- controllers ----------------------------------------------------------- */

/* osContSetCh: how many channels the SI commands cover (libultra's
 * __osMaxControllers); the game reads only controller 1 (the other
 * OSContPad entries are left alone, as on the N64) */
static int g_max_ch = 4;

static void fill_status(OSContStatus *data, u8 *pattern) {
    int i;
    u8 bits = 0;
    for (i = 0; i < g_max_ch; i++, data++) {
        if (i == 0 && plat_cfg.cont_present) {
            data->type = CONT_TYPE_NORMAL;
            data->status = CONT_CARD_PULL; /* no pak: what the PIF reports (mupen64plus) */
            data->errno = 0;
            bits |= 1 << i;
        } else {
            data->errno = CONT_NO_RESPONSE_ERROR;
        }
    }
    if (pattern) *pattern = bits;
}

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
    /* the SDK waits until 0.5 s after boot before talking to the PIF */
    static OSTimer timer;
    static OSMesgQueue tq;
    static OSMesg tmsg;
    OSTime t;
    const OSTime half_s = (OSTime) 500000 * PLAT_COUNT_HZ / 1000000;
    (void) mq;
    if (g_cont_inited) return 0;
    g_cont_inited = 1;
    t = osGetTime();
    if (t < half_s) {
        osCreateMesgQueue(&tq, &tmsg, 1);
        osSetTimer(&timer, half_s - t, 0, &tq, &tmsg);
        osRecvMesg(&tq, NULL, OS_MESG_BLOCK);
    }
    fill_status(data, bitpattern);
    return 0;
}

s32 osContStartQuery(OSMesgQueue *mq) {
    (void) mq;
    si_done();
    return 0;
}

void osContGetQuery(OSContStatus *data) {
    fill_status(data, NULL);
}

s32 osContStartReadData(OSMesgQueue *mq) {
    (void) mq;
    /* the PIF samples the pad now */
    plat_input_read(plat_stats.cont_reads, &g_latch_button, &g_latch_x, &g_latch_y);
    plat_stats.cont_reads++;
    si_done();
    return 0;
}

void osContGetReadData(OSContPad *data) {
    int i;
    for (i = 0; i < g_max_ch; i++, data++) {
        if (i == 0 && plat_cfg.cont_present) {
            data->button = g_latch_button;
            data->stick_x = g_latch_x;
            data->stick_y = g_latch_y;
            data->errno = 0;
        } else {
            data->errno = CONT_NO_RESPONSE_ERROR;
        }
    }
}

s32 osContSetCh(u8 ch) {
    g_max_ch = ch > 4 ? 4 : ch;
    return 0;
}

/* ---- EEPROM ------------------------------------------------------------------ */

s32 osEepromProbe(OSMesgQueue *mq) {
    (void) mq;
    return plat_cfg.eeprom_present ? EEPROM_TYPE_4K : 0;
}

s32 osEepromRead(OSMesgQueue *mq, u8 address, u8 *buffer) {
    (void) mq;
    if (!plat_cfg.eeprom_present) return CONT_NO_RESPONSE_ERROR;
    if (address >= EEPROM_BLOCKS) return -1;
    __builtin_memcpy(buffer, g_eeprom + address * 8, 8);
    plat_stats.eeprom_reads++;
    return 0;
}

s32 osEepromWrite(OSMesgQueue *mq, u8 address, u8 *buffer) {
    (void) mq;
    if (!plat_cfg.eeprom_present) return CONT_NO_RESPONSE_ERROR;
    if (address >= EEPROM_BLOCKS) return -1;
    __builtin_memcpy(g_eeprom + address * 8, buffer, 8);
    g_eeprom_dirty = 1;
    plat_stats.eeprom_writes++;
    plat_si_flush();
    return 0;
}

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    s32 ret = 0;
    while (nbytes > 0) {
        ret = osEepromRead(mq, address, buffer);
        if (ret != 0) return ret;
        nbytes -= 8;
        address++;
        buffer += 8;
    }
    return ret;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes) {
    /* the SDK waits 12 ms (one block's write time) after each block */
    static OSTimer timer;
    static OSMesgQueue tq;
    static OSMesg tmsg;
    s32 ret = 0;
    osCreateMesgQueue(&tq, &tmsg, 1);
    while (nbytes > 0) {
        ret = osEepromWrite(mq, address, buffer);
        if (ret != 0) return ret;
        nbytes -= 8;
        address++;
        buffer += 8;
        osSetTimer(&timer, (OSTime) 12000 * PLAT_COUNT_HZ / 1000000, 0, &tq, &tmsg);
        osRecvMesg(&tq, NULL, OS_MESG_BLOCK);
    }
    return ret;
}

/* the front end calls the EEPROM functions by address (0E7B0.c) */
s32 func_80204C10(OSMesgQueue *mq, u8 a, u8 *b) {
    return osEepromWrite(mq, a, b);
}
s32 func_802050F0(OSMesgQueue *mq, u8 a, u8 *b) {
    return osEepromRead(mq, a, b);
}
s32 func_802042D0(OSMesgQueue *mq, u8 a, u8 *b, int n) {
    return osEepromLongWrite(mq, a, b, n);
}
s32 func_80204410(OSMesgQueue *mq, u8 a, u8 *b, int n) {
    return osEepromLongRead(mq, a, b, n);
}

/* ---- Controller Pak: none ------------------------------------------------------ */

s32 osPfsIsPlug(OSMesgQueue *mq, u8 *pattern) {
    (void) mq;
    *pattern = 0;
    return 0;
}
s32 osPfsInitPak(OSMesgQueue *mq, OSPfs *pfs, int channel) {
    (void) mq;
    (void) pfs;
    (void) channel;
    return PFS_ERR_NOPACK;
}
s32 osPfsFreeBlocks(OSPfs *pfs, s32 *bytes) {
    (void) pfs;
    *bytes = 0;
    return PFS_ERR_NOPACK;
}
s32 osPfsChecker(OSPfs *pfs) {
    (void) pfs;
    return PFS_ERR_NOPACK;
}
s32 osPfsAllocateFile(OSPfs *pfs, u16 company, u32 game, u8 *name, u8 *ext, int size, s32 *file) {
    (void) pfs; (void) company; (void) game; (void) name; (void) ext; (void) size; (void) file;
    return PFS_ERR_NOPACK;
}
s32 osPfsDeleteFile(OSPfs *pfs, u16 company, u32 game, u8 *name, u8 *ext) {
    (void) pfs; (void) company; (void) game; (void) name; (void) ext;
    return PFS_ERR_NOPACK;
}
s32 osPfsFindFile(OSPfs *pfs, u16 company, u32 game, u8 *name, u8 *ext, s32 *file) {
    (void) pfs; (void) company; (void) game; (void) name; (void) ext; (void) file;
    return PFS_ERR_NOPACK;
}
s32 osPfsReadWriteFile(OSPfs *pfs, s32 file, u8 flag, int offset, int size, u8 *data) {
    (void) pfs; (void) file; (void) flag; (void) offset; (void) size; (void) data;
    return PFS_ERR_NOPACK;
}
s32 osPfsFileState(OSPfs *pfs, s32 file, OSPfsState *state) {
    (void) pfs; (void) file; (void) state;
    return PFS_ERR_NOPACK;
}
