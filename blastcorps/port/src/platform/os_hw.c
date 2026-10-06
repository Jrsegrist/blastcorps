/* VI, PI, SP/DP, AI and the small libultra leftovers (caches, address
 * translation, osInitialize), headless. */
#include "plat.h"

/* ---- osInitialize --------------------------------------------------------- */

u8 *plat_rom;
u32 plat_rom_size;

#define LOWMEM(off) (*(u32 *) (0x80000300u + (off)))

void osInitialize(void) {
    /* what IPL3 + the SDK's osInitialize leave in low memory */
    LOWMEM(0x00) = 1;           /* osTvType: NTSC */
    LOWMEM(0x04) = 0;           /* osRomType: cartridge */
    LOWMEM(0x08) = 0xB0000000;  /* osRomBase */
    LOWMEM(0x0C) = 0;           /* osResetType: cold */
    LOWMEM(0x18) = 0x00800000;  /* osMemSize (the emulator's default 8 MB) */
    /* the old SDK takes osViClock from its .data (NTSC value); set it in
     * host order whatever the image swap did with it */
    osViClock = 0x02E6D354;
    host_set_fpu_mode();
}

/* ---- libm / libc bits ------------------------------------------------------ */

/* ultralib's sinf/cosf are weak aliases of __sinf/__cosf; strong definitions
 * here make sure the C runtime's versions never win (the game needs the
 * SDK's polynomial for N64-identical results). */
extern float __sinf(float);
extern float __cosf(float);
float sinf(float x) {
    return __sinf(x);
}
float cosf(float x) {
    return __cosf(x);
}

void bcopy(const void *src, void *dst, int n) {
    __builtin_memmove(dst, src, n);
}
void bzero(void *p, int n) {
    __builtin_memset(p, 0, n);
}

/* ---- caches, address translation ----------------------------------------- */

void osInvalDCache(void *p, s32 n) {
    (void) p;
    (void) n;
}
void osInvalICache(void *p, s32 n) {
    (void) p;
    (void) n;
}
void osWritebackDCache(void *p, s32 n) {
    (void) p;
    (void) n;
}
void osWritebackDCacheAll(void) {
}

u32 osVirtualToPhysical(void *addr) {
    u32 a = (u32) addr;
    /* KSEG0/KSEG1 -> physical.  Host pointers (stack, heap of the exe) have no
     * N64 physical address: pass them through so a bug shows up as a host
     * address in a DL rather than as silent aliasing. */
    if (a >= 0x80000000u && a < 0xC0000000u) return a & 0x1FFFFFFF;
    return a;
}

/* ---- VI ------------------------------------------------------------------ */

static void *g_vi_cur = (void *) 0x80000000, *g_vi_next = (void *) 0x80000000;
static OSMesgQueue *g_vi_mq;
static OSMesg g_vi_msg;
static u32 g_vi_retrace_count = 1, g_vi_retrace_left = 1;
static OSViMode *g_vi_mode;
static u8 g_vi_black;
static u32 g_vi_features;

void osCreateViManager(OSPri pri) {
    (void) pri;
}

void osViSetMode(OSViMode *m) {
    g_vi_mode = m;
}

void osViBlack(u8 active) {
    g_vi_black = active;
}

void osViSetSpecialFeatures(u32 f) {
    g_vi_features = f;
}

void osViSetEvent(OSMesgQueue *mq, OSMesg msg, u32 retraceCount) {
    g_vi_mq = mq;
    g_vi_msg = msg;
    g_vi_retrace_count = retraceCount ? retraceCount : 1;
    g_vi_retrace_left = g_vi_retrace_count;
}

void osViSwapBuffer(void *fb) {
    g_vi_next = fb;
}

void *osViGetCurrentFramebuffer(void) {
    return g_vi_cur;
}

void *osViGetNextFramebuffer(void) {
    return g_vi_next;
}

/* the VI manager's work at each retrace */
void plat_vi_retrace(void) {
    g_vi_cur = g_vi_next;
    if (g_vi_mq != NULL && --g_vi_retrace_left == 0) {
        g_vi_retrace_left = g_vi_retrace_count;
        osSendMesg(g_vi_mq, g_vi_msg, OS_MESG_NOBLOCK);
    }
}

/* ---- PI ------------------------------------------------------------------ */

void osCreatePiManager(OSPri pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt) {
    (void) pri;
    (void) cmdQ;
    (void) cmdBuf;
    (void) cmdMsgCnt;
}

static u32 rom_offset(u32 devAddr) {
    /* the game passes ROM offsets; accept PI bus addresses too */
    if (devAddr >= 0x10000000u && devAddr < 0x20000000u) devAddr -= 0x10000000u;
    return devAddr;
}

/* The DMA happens at once (the PI manager outranks every game thread, and
 * the transfer takes no virtual time); the completion message goes to mq
 * like the PI manager's. */
s32 osPiStartDma(OSIoMesg *mb, s32 pri, s32 direction, u32 devAddr, void *vAddr, u32 nbytes,
                 OSMesgQueue *mq) {
    u32 off = rom_offset(devAddr);
    mb->hdr.type = OS_MESG_TYPE_DMAREAD;
    mb->hdr.pri = (u8) pri;
    mb->hdr.retQueue = mq;
    mb->dramAddr = vAddr;
    mb->devAddr = devAddr;
    mb->size = nbytes;
    if (direction == OS_READ) {
        u8 *dst = vAddr;
        u32 n = nbytes, avail = off < plat_rom_size ? plat_rom_size - off : 0;
        if (n > avail) {
            if (host_verbose) host_log("pi: dma past ROM end (0x%X+0x%X)\n", (unsigned) off, (unsigned) n);
            __builtin_memset(dst + avail, 0, n - avail);
            n = avail;
        }
        if (n) __builtin_memcpy(dst, plat_rom + off, n);
        port_on_dma((u32) vAddr, off, nbytes);
        plat_stats.pi_dmas++;
        plat_stats.pi_bytes += nbytes;
    } else if (host_verbose) {
        host_log("pi: write DMA to 0x%X ignored\n", (unsigned) devAddr);
    }
    if (mq != NULL) osSendMesg(mq, (OSMesg) mb, OS_MESG_NOBLOCK);
    return 0;
}

/* One 32-bit PI read.  Inside the ROM: the big-endian word, as lw gives it.
 * The game's only use (func_802447C0) reads 16 words at 0xFFB000, past the
 * ROM, as a debug command-line *string*: there the words are returned so
 * that their bytes in host memory spell plat_cfg.cmdline (empty: zeros). */
s32 osPiRawReadIo(u32 devAddr, u32 *data) {
    u32 off = rom_offset(devAddr);
    if (off + 4 <= plat_rom_size) {
        const u8 *p = plat_rom + off;
        *data = ((u32) p[0] << 24) | ((u32) p[1] << 16) | ((u32) p[2] << 8) | p[3];
    } else {
        u8 b[4] = {0, 0, 0, 0};
        const char *s = plat_cfg.cmdline;
        u32 i, base = off - 0xFFB000u;
        if (s != NULL && off >= 0xFFB000u && off < 0xFFB040u) {
            u32 len = 0;
            while (s[len]) len++;
            for (i = 0; i < 4; i++)
                if (base + i < len) b[i] = (u8) s[base + i];
        }
        __builtin_memcpy(data, b, 4);
    }
    return 0;
}

u32 osPiGetStatus(void) {
    return 0;
}

/* ---- SP / DP --------------------------------------------------------------- */

static OSTask *g_sp_task;
static u8 g_sp_yield;

void osSpTaskLoad(OSTask *t) {
    g_sp_task = t;
}

#define UCODE_CULL 0x802E77B0u  /* D_802E77B0: the RDP-output-to-DRAM F3D (5FD50) */

static void complete(u32 delay, int dp) {
    if (delay == 0) {
        plat_post_event(OS_EVENT_SP);
        if (dp) plat_post_event(OS_EVENT_DP);
    } else {
        plat_event_add(plat_now + delay, PEV_SP_DONE, NULL);
        if (dp) plat_event_add(plat_now + delay, PEV_DP_DONE, NULL);
    }
}

/* Nothing runs on the RCP.  The game must still see every task finish:
 *  - audio (M_AUDTASK): the Acmd list is dropped (no output yet); SP done.
 *  - graphics: dropped and counted.  Tasks are submitted through the game's
 *    scheduler (2C560.c), which keeps the OSTask at +0x10 of its own task
 *    record (BcScTask, flags at +0x08).  Flag 0x40 marks the frame's last
 *    task, the one that ends in a full sync: it gets SP and DP done, the
 *    others (shadow passes, the cull test) SP done only, like the hardware.
 *  - func_802A4B0C's visibility test (ucode D_802E77B0, which writes RDP
 *    commands to output_buff for the CPU to read): answered "visible" by
 *    making the first output word something other than a lone sync
 *    (0xE8000000).  Exact answers need a CPU F3D clip/cull evaluator. */
void osSpTaskStartGo(OSTask *t) {
    g_sp_yield = 0;
    if (t->t.type == M_AUDTASK) {
        plat_stats.aud_tasks++;
        complete(plat_cfg.aud_cycles, 0);
        return;
    }
    plat_stats.gfx_tasks++;
    if ((u32) t->t.ucode == UCODE_CULL) {
        plat_stats.cull_tasks++;
        if (t->t.output_buff != NULL) *(u32 *) t->t.output_buff = 0;
        complete(plat_cfg.small_gfx_cycles, 0);
        return;
    }
    if (*(u32 *) ((u8 *) t - 0x10 + 0x08) & 0x40) {
        u64 when;
        plat_on_frame();
        when = plat_frame_done_time(plat_stats.frames);
        complete(when == ~0ull ? plat_cfg.gfx_cycles : (when > plat_now ? (u32) (when - plat_now) : 0), 1);
    } else {
        complete(plat_cfg.small_gfx_cycles, 0);
    }
}

void osSpTaskYield(void) {
    g_sp_yield = 1;
}

OSYieldResult osSpTaskYielded(OSTask *t) {
    (void) t;
    /* tasks never get the chance to yield: they finish first */
    return 0;
}

u32 __osSpGetStatus(void) {
    return 1; /* SP_STATUS_HALT */
}
void __osSpSetStatus(u32 v) {
    (void) v;
}
s32 __osSpSetPc(u32 pc) {
    (void) pc;
    return 0;
}

/* osDpSetStatus / osDpGetStatus (8FD90.c / 96300.c write the DPC register) */
void func_802D4550(u32 v) {
    (void) v;
}
u32 func_802DAAC0(void) {
    return 0;
}

/* ---- AI -------------------------------------------------------------------- */
/* A two-entry DMA FIFO playing at the programmed rate, timed by the virtual
 * clock.  The audio manager sizes each frame from osAiGetLength, and the
 * synthesizer's sequencer state (which the game polls) follows the sample
 * counts, so the model has to keep time even though nothing is heard. */

static u32 g_ai_dacrate;      /* VI clocks per sample */
static u64 g_ai_start;        /* when the playing buffer started */
static u32 g_ai_len[2];       /* playing, queued (bytes; 0 = none) */

static u64 ai_cycles(u32 bytes) {
    /* bytes/4 samples, each dacrate VI-clock ticks, in CPU counts */
    u64 vi_ticks = (u64) (bytes / 4) * g_ai_dacrate;
    return vi_ticks * PLAT_COUNT_HZ / (u32) osViClock;
}

static void ai_update(void) {
    while (g_ai_len[0] != 0 && g_ai_dacrate != 0) {
        u64 end = g_ai_start + ai_cycles(g_ai_len[0]);
        if (plat_now < end) break;
        g_ai_len[0] = g_ai_len[1];
        g_ai_len[1] = 0;
        g_ai_start = end;
    }
}

s32 osAiSetFrequency(u32 frequency) {
    unsigned int dacRate;
    unsigned char bitRate;
    float f;

    f = (s32) osViClock / (float) frequency + .5f;
    dacRate = f;
    if (dacRate < 132) return -1;
    bitRate = dacRate / 66;
    if (bitRate > 16) bitRate = 16;
    (void) bitRate;
    g_ai_dacrate = dacRate;
    return (s32) osViClock / (s32) dacRate;
}

s32 osAiSetNextBuffer(void *buf, u32 size) {
    (void) buf;
    ai_update();
    if (g_ai_len[0] == 0) {
        g_ai_len[0] = size;
        g_ai_start = plat_now;
    } else if (g_ai_len[1] == 0) {
        g_ai_len[1] = size;
    } else {
        return -1;
    }
    return 0;
}

u32 osAiGetLength(void) {
    u64 done;
    ai_update();
    if (g_ai_len[0] == 0 || g_ai_dacrate == 0) return 0;
    done = (plat_now - g_ai_start) * (u32) osViClock / PLAT_COUNT_HZ / g_ai_dacrate * 4;
    return done >= g_ai_len[0] ? 0 : g_ai_len[0] - (u32) done;
}

u32 osAiGetStatus(void) {
    return g_ai_len[1] != 0 ? 0x80000000u : 0;
}
