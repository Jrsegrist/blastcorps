/* Boot, frame accounting, RDRAM dumps and the front-end overlay reload. */
#include "plat.h"
#include "../audio/port_audio.h"

PlatConfig plat_cfg;
PlatStats plat_stats;

/* game state shown in traces and crash reports (raw N64 addresses) */
#define G_MODE (*(u64 *) 0x80364A90)      /* game mode (master switch) */
#define G_NEXTMODE (*(u64 *) 0x80364A98)
#define G_LEVEL (*(s32 *) 0x802E8BDC)     /* levelno */
#define G_MODEFRAMES (*(u32 *) 0x80358060) /* frames in this mode */
#define G_VICOUNT (*(u32 *) 0x803156C4)   /* the game's retrace counter */

/* the boot entry in hd_code (00000.c); init.us.v11's job is done natively */
extern void func_802447C0(void);

/* ---- front-end overlay ------------------------------------------------------ */
/* The front end (0x801E7000-0x8021ED00) is inflated from the ROM whenever the
 * game returns to the menus (46C20.c func_8028B3E0); its .data and .bss come
 * back to their initial state each time, because the heap overwrote them.
 * Natively its code is in the exe; its data lives in RDRAM.  The initial
 * data image (ROM bytes after the load-time swap, plus the native
 * initialisers, plus zeroed .bss) is snapshotted at boot and put back after
 * every reload (the game's inflate has just written big-endian bytes over
 * it). */
#define FE_DATA_START 0x80208040u
#define FE_END 0x8021ED00u
static u8 *g_fe_snap;

void plat_fe_snapshot(void) {
    u32 n = FE_END - FE_DATA_START;
    g_fe_snap = host_realloc(NULL, n);
    __builtin_memcpy(g_fe_snap, (void *) FE_DATA_START, n);
}

void port_fe_loaded(void) {
    plat_stats.fe_reloads++;
    __builtin_memcpy((void *) FE_DATA_START, g_fe_snap, FE_END - FE_DATA_START);
    if (!plat_cfg.quiet)
        host_log("fe: front end reloaded (#%u) at frame %u, vi %u\n", (unsigned) plat_stats.fe_reloads,
                 (unsigned) plat_stats.frames, (unsigned) plat_vi_count);
}

/* ---- frames ---------------------------------------------------------------- */

static void dump_rdram(u32 frame) {
    char path[512];
    const char *d = plat_cfg.dump_dir ? plat_cfg.dump_dir : ".";
    u32 i = 0, j;
    char num[16];
    while (d[i] && i < 400) {
        path[i] = d[i];
        i++;
    }
    for (j = 0; "/frame_"[j]; j++) path[i++] = "/frame_"[j];
    for (j = 0; j < 7; j++) num[6 - j] = '0' + (frame % 10), frame /= 10;
    for (j = 0; j < 7; j++) path[i++] = num[j];
    for (j = 0; ".bin"[j]; j++) path[i++] = ".bin"[j];
    path[i] = 0;
    if (host_write_file(path, (void *) 0x80000000, 0x800000) != 0) host_log("dump: can't write %s\n", path);
    else if (!plat_cfg.quiet) host_log("dump: %s\n", path);
}

static void print_stats(void) {
    host_log("stats: frames %u, retraces %u, time %u.%03u s, gfx tasks %u (cull tests %u), audio tasks %u,\n"
             "       PI DMAs %u (%u KB), pad reads %u, EEPROM r/w %u/%u, front-end reloads %u,\n"
             "       osGetTime calls %u, thread switches %u\n",
             (unsigned) plat_stats.frames, (unsigned) plat_vi_count, (unsigned) (plat_now / PLAT_COUNT_HZ),
             (unsigned) (plat_now % PLAT_COUNT_HZ / (PLAT_COUNT_HZ / 1000)), (unsigned) plat_stats.gfx_tasks,
             (unsigned) plat_stats.cull_tasks, (unsigned) plat_stats.aud_tasks, (unsigned) plat_stats.pi_dmas,
             (unsigned) (plat_stats.pi_bytes >> 10), (unsigned) plat_stats.cont_reads,
             (unsigned) plat_stats.eeprom_reads, (unsigned) plat_stats.eeprom_writes,
             (unsigned) plat_stats.fe_reloads, (unsigned) plat_stats.gettime_calls,
             (unsigned) plat_stats.thread_switches);
    host_log("game: mode 0x%08X%08X, level %d, frames in mode %u, game VI counter %u\n",
             (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (int) G_LEVEL, (unsigned) G_MODEFRAMES,
             (unsigned) G_VICOUNT);
    plat_clock_report();
    plat_sync_report();
}

static u64 g_last_mode = ~0ull;

/* A frame: the game handed the scheduler its frame-ending gfx task. */
void plat_on_frame(void) {
    u32 f = ++plat_stats.frames;
    u32 i;
    host_trace("%u vi=%u t=%u mode=%08X%08X lvl=%d mf=%u gvi=%u\n", (unsigned) f, (unsigned) plat_vi_count,
               (unsigned) plat_now, (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (int) G_LEVEL,
               (unsigned) G_MODEFRAMES, (unsigned) G_VICOUNT);
    if (G_MODE != g_last_mode) {
        g_last_mode = G_MODE;
        if (!plat_cfg.quiet)
            host_log("frame %u (vi %u): game mode 0x%08X%08X, level %d\n", (unsigned) f,
                     (unsigned) plat_vi_count, (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (int) G_LEVEL);
    }
    for (i = 0; i < plat_cfg.n_dump_frames; i++)
        if (plat_cfg.dump_frames[i] == f) dump_rdram(f);
    if (plat_cfg.dump_every && f % plat_cfg.dump_every == 0) dump_rdram(f);
    if (plat_cfg.frames && f >= plat_cfg.frames) {
        host_log("stopping: %u frames reached\n", (unsigned) f);
        print_stats();
        plat_si_flush();
        host_exit(0);
    }
}

static void describe(void) {
    host_log("  thread %s, frame %u, vi %u\n", plat_thread_name(plat_running()), (unsigned) plat_stats.frames,
             (unsigned) plat_vi_count);
    print_stats();
    if (plat_cfg.dump_dir) dump_rdram(9999999); /* frame_9999999.bin: RDRAM at the crash */
}

/* ---- boot ------------------------------------------------------------------- */

void plat_start(const HostOpts *o) {
    plat_cfg = *o;
    plat_now = o->boot_count;
    plat_rom = o->rom;
    plat_rom_size = o->rom_size;
    host_install_crash_handler(describe);
    if (o->trace_path && host_trace_open(o->trace_path) != 0) host_fatal("can't write %s", o->trace_path);

    /* what init.us.v11 leaves behind besides the inflated images: the front
     * end's compressed ROM range at the top of RDRAM (func_80220730) */
    *(u32 *) 0x803FFFF8 = 0x7E3AD0;
    *(u32 *) 0x803FFFFC = 0x7F9BE0;

    if (o->live != NULL && o->live->boot != NULL) o->live->boot();
    port_audio_init(o);
    plat_fe_snapshot();
    plat_si_init();
    plat_input_init(o->input_path);
    plat_clock_init(o->gettime_path, o->frame_done_path);
    if (o->syms_path) plat_syms_load(o->syms_path);
    if (o->clock_path) plat_clock_keyed_load(o->clock_path);
    if (o->calls) {
        if (!o->syms_path) host_fatal("--calls needs --syms");
        plat_calls_init(o->calls);
    }
    if (o->sync_path) {
        if (!o->syms_path) host_fatal("--sync needs --syms");
        plat_sync_load(o->sync_path);
    }

    func_802447C0();   /* osInitialize, debug command line, idle thread */
    plat_run();
}
