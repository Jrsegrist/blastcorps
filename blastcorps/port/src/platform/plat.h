/* Headless platform layer: internal interfaces.  Included by the os*
 * replacement files, which are compiled with the game's headers (<ultra64.h>,
 * include/2.0I) so that OSThread, OSMesgQueue, OSIoMesg ... have exactly the
 * layout the game C uses. */
#ifndef PLAT_H
#define PLAT_H

#include <ultra64.h>
#include "plat_host.h"
#include "port_dma.h"

/* ---- virtual clock ----------------------------------------------------- */
/* CPU count rate (osGetCount / osGetTime units): 46.875 MHz.  One NTSC field
 * (VI retrace) every 781250 counts = 60 Hz exactly. */
#define PLAT_COUNT_HZ 46875000u
#define PLAT_VI_PERIOD 781250u

extern u64 plat_now;     /* virtual time in count units */
extern u32 plat_vi_count; /* retraces so far */

/* Event kinds for the virtual-time event queue */
enum {
    PEV_VI = 1,    /* retrace (re-armed every PLAT_VI_PERIOD) */
    PEV_TIMER,     /* OSTimer expiry: p = OSTimer* */
    PEV_SP_DONE,   /* RSP task finished: post OS_EVENT_SP */
    PEV_DP_DONE    /* RDP full sync: post OS_EVENT_DP */
};
void plat_event_add(u64 when, int kind, void *p);
void plat_event_remove(int kind, void *p);
/* advance the clock to the next pending event and deliver it (and every
 * other event due at that time).  Returns 0 if there is none. */
int plat_advance(void);
/* post an interrupt-style event message (OS_EVENT_*) now */
void plat_post_event(int event);
/* the idle loop: runs game threads; returns never */
void plat_run(void) __attribute__((noreturn));
/* thread currently running (NULL = host idle loop) */
OSThread *plat_running(void);
const char *plat_thread_name(OSThread *t);

/* ---- per-subsystem hooks --------------------------------------------- */
void plat_vi_retrace(void);          /* os_vi.c */
void plat_timer_fire(OSTimer *t);    /* os_thread.c */

/* ---- configuration (HostOpts in plat_host.h, filled by headless_main.c) -- */
typedef HostOpts PlatConfig;
extern PlatConfig plat_cfg;
extern u8 *plat_rom;
extern u32 plat_rom_size;

/* ---- statistics --------------------------------------------------------- */
typedef struct {
    u32 frames;            /* frame (swap-buffer) gfx tasks */
    u32 gfx_tasks;         /* all gfx tasks dropped */
    u32 cull_tasks;        /* func_802A4B0C visibility tests answered "visible" */
    u32 aud_tasks;
    u32 pi_dmas;
    u64 pi_bytes;
    u32 cont_reads;
    u32 eeprom_reads, eeprom_writes;
    u32 fe_reloads;
    u32 gettime_calls;
    u32 thread_switches;
} PlatStats;
extern PlatStats plat_stats;

/* frame boundary hook (os_sp.c calls it when a frame's gfx task starts) */
void plat_on_frame(void);
/* input source (input.c) */
void plat_input_init(const char *path);
void plat_input_read(u32 index, u16 *button, s8 *x, s8 *y);
/* clock injection (os_time.c) */
void plat_clock_init(const char *gettime_path, const char *frame_done_path);
/* injected completion time of frame N's gfx task, or ~0 (use gfx_cycles) */
u64 plat_frame_done_time(u32 frame);
/* optional C hook for osGetTime (part 2's trace matching): returns 1 and
 * sets *t to override the value of call number `call` */
extern int (*plat_gettime_hook)(u32 call, u64 now, u64 *t);
/* the same for osGetCount (1C460.c and 20460.c use it as a random source) */
extern int (*plat_getcount_hook)(u32 call, u64 now, u32 *c);
/* front-end overlay: snapshot after boot load, restore on reload */
void plat_fe_snapshot(void);
/* SI devices */
void plat_si_init(void);
void plat_si_flush(void);
/* Controller Pak (pif.c) */
void plat_pak_init(void);
int plat_pak_present(void);

/* game symbols the platform reads (absolute, at their N64 addresses) */
extern u32 osViClock;

#endif
