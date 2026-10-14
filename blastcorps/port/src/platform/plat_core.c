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
extern void hd_boot(void);

/* ---- front-end overlay ------------------------------------------------------ */
/* The front end (0x801E7000-0x8021ED00) is inflated from the ROM whenever the
 * game returns to the menus (46C20.c hd_load_front_end); its .data and .bss come
 * back to their initial state each time, because the heap overwrote them.
 * Natively its code is in the exe; its data lives in RDRAM.  The initial
 * data image (ROM bytes after the load-time swap, plus the native
 * initialisers, plus zeroed .bss) is snapshotted at boot and put back after
 * every reload (the game's inflate has just written big-endian bytes over
 * it). */
#define FE_DATA_START 0x80208040u
#define FE_END 0x8021ED00u
static u8 *g_fe_snap;
static void *g_fe_units;   /* their units for the stale-byte emulation (load/swap.c) */
void *port_unit_save(unsigned int addr, unsigned int n);
void port_unit_restore(unsigned int addr, const void *saved, unsigned int n);

void plat_fe_snapshot(void) {
    u32 n = FE_END - FE_DATA_START;
    g_fe_snap = host_realloc(NULL, n);
    PORT_MEMCPY(g_fe_snap, (void *) FE_DATA_START, n);
    g_fe_units = port_unit_save(FE_DATA_START, n);
}

void port_fe_loaded(void) {
    plat_stats.fe_reloads++;
    PORT_MEMCPY((void *) FE_DATA_START, g_fe_snap, FE_END - FE_DATA_START);
    port_unit_restore(FE_DATA_START, g_fe_units, FE_END - FE_DATA_START);
    if (!plat_cfg.quiet)
        host_log("fe: front end reloaded (#%u) at frame %u, vi %u\n", (unsigned) plat_stats.fe_reloads,
                 (unsigned) plat_stats.frames, (unsigned) plat_vi_count);
}

/* ---- frame interpolation tags (port.h PORT_ITAG) ----------------------------- */
/* bc.exe's tagged interpolation records them (live_interp.cpp); nothing else
 * does, and nothing goes back to the game. */
void (*host_itag_hook)(unsigned addr, int kind, int index);
void (*host_camera_hook)(int snap);
short *(*host_draw_cells_hook)(short *game);

void port_itag(const void *gdl, int kind, int index) {
    /* PORT_ICUT_IF: every frame, at the end of the game's camera (index: the snap flag) */
    if (kind == 30 && host_camera_hook != NULL) host_camera_hook(index);
    if (host_itag_hook != NULL && (kind != 30 || index != 0)) host_itag_hook((unsigned) (size_t) gdl, kind, index);
}

/* ---- widescreen tags (port.h PORT_WTAG) ------------------------------------- */
/* bc.exe's widescreen records them (live_wide.cpp); nothing goes back to the game. */
void (*host_wtag_hook)(unsigned addr, int kind, int x0, int x1);

void port_wtag(const void *gdl, int kind, int x0, int x1) {
    if (host_wtag_hook != NULL) host_wtag_hook((unsigned) (size_t) gdl, kind, x0, x1);
}

/* PORT_DRAW_CELLS: the visible-cell list the drawing paths use */
short *port_draw_cells(short *game) {
    return host_draw_cells_hook != NULL ? host_draw_cells_hook(game) : game;
}

/* bc.exe's orbit camera: the commands and vertex values 2D2D810.c
 * backdrop_draw would write for the backdrop with the camera heading and
 * pitch given (the game's gCamYaw / gCamPitch units), into host memory:
 * `out` (at most `max` Gfx), and `vtx[i]` = {ob[1], tc[0], tc[1]} of the 4
 * vertices of gBackdropVtx[0].  Nothing in RDRAM is written; the game's own
 * state (gBackdropScrollS ..., the vertex buffer index) isn't touched.  Returns the
 * number of commands, 0 when the backdrop would not be drawn. */
typedef struct {
    s16 id;
    u32 ptr;
    u8 sShift, tShift, cms, cmt;
} PortFadeEntry; /* 2D810.c FadeEntry */
int port_backdrop_gfx(void *outv, int max, unsigned dyn, int level, int heading, int pitch, short vtx[4][3]) {
    Gfx *gdl = (Gfx *) outv;
    Gfx *start = gdl;
    const u16 *fmts = (const u16 *) (size_t) 0x802FA8A0u;
    const PortFadeEntry *tab = (const PortFadeEntry *) (size_t) (0x802FA280u + (u32) level * 0x18u);
    const PortFadeEntry *entry;
    const f32 speed = *(const f32 *) (size_t) 0x8036BFC0u;
    f32 scroll, cc, d0, c8, s0, s1, t0, t1;
    s32 i;

    if (*(const u8 *) (size_t) 0x8036BFC4u == 0 || max < 32 || level < 0 || level > 0x3F) return 0;
    scroll = 120.0 - (f32) (s16) pitch * 0.22 + speed;
    cc = (0.0 > scroll) ? 0.0 : scroll;
    d0 = (0.0 > -scroll) ? 0.0 : -scroll;
    if ((s32) cc <= 0) return 0;
    c8 = ((s16) heading - 2048.0) * 0.5;
    gSPClearGeometryMode(gdl++, 0xFFFFFFFF);
    gSPSetGeometryMode(gdl++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(gdl++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gdl++);
    gDPSetCombine(gdl++, 0xFC24A3FF, 0x1FFC9238);
    gDPSetCycleType(gdl++, G_CYC_2CYCLE);
    gDPSetRenderMode(gdl++, G_RM_PASS, G_RM_OPA_SURF2);
    gDPSetTextureFilter(gdl++, G_TF_BILERP);
    for (i = 0; i < 2; i++) {
        entry = &tab[i];
        gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (void *) (size_t) entry->ptr);
        gDPTileSync(gdl++);
        gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, i * 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPLoadSync(gdl++);
        gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, 1023, 256);
    }
    gSPMatrix(gdl++, (Mtx *) (size_t) (u32) (dyn + 0x80000100u), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gdl++, (Mtx *) (size_t) (u32) (dyn + 0x800001C0u), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gDPTileSync(gdl++);
    gDPSetTextureLOD(gdl++, G_TL_TILE);
    for (i = 0; i < 2; i++) {
        entry = &tab[i];
        gDPSetTile(gdl++, fmts[i], G_IM_SIZ_16b, 8, i * 256, i, 0, entry->cmt & 3, 5, i, entry->cms & 3, 5, i);
        gDPSetTileSize(gdl++, i, 0, 0, 31 << G_TEXTURE_IMAGE_FRAC, 31 << G_TEXTURE_IMAGE_FRAC);
    }
    entry = &tab[1];
    s0 = (c8 + 16.0f) / (f32) (1 << entry->sShift);
    s1 = 319.0 / (f32) (1 << entry->sShift);
    t0 = (cc - speed - d0) / (f32) (1 << entry->tShift);
    t1 = (cc - 1.0) / (f32) (1 << entry->tShift);
    for (i = 0; i < 4; i++) {
        vtx[i][0] = (i >= 2) ? (s16) (cc * 4.0 - 1.0) : 0x7FFF;   /* 0x7FFF: keep the vertex's own */
        vtx[i][1] = (s16) (((i & 1) ? s0 + s1 : s0) * 32.0);
        vtx[i][2] = (s16) ((i < 2 ? t0 : t0 - t1) * 32.0);
    }
    gSPVertex(gdl++, (Vtx *) (size_t) 0x802FA820u, 4, 0);
    gDPPipeSync(gdl++);
    gSP1Triangle(gdl++, 0, 1, 2, 0);
    gSP1Triangle(gdl++, 1, 2, 3, 0);
    return (int) (gdl - start);
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
    host_log("cull: %u tests, %u not visible", (unsigned) plat_stats.cull_tasks, (unsigned) plat_stats.cull_hidden);
    if (plat_stats.cull_keyed)
        host_log("; %u answers from --clock, the model disagrees with %u", (unsigned) plat_stats.cull_keyed,
                 (unsigned) plat_stats.cull_disagree);
    host_log("\n");
    plat_clock_report();
    plat_sync_report();
}

static u64 g_last_mode = ~0ull;

/* A frame: the game handed the scheduler its frame-ending gfx task. */
void plat_on_frame(void) {
    u32 f = ++plat_stats.frames;
    u32 i;
    host_trace("%u vi=%u t=%u mode=%08X%08X lvl=%d mf=%u gvi=%u rd=%u\n", (unsigned) f, (unsigned) plat_vi_count,
               (unsigned) plat_now, (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (int) G_LEVEL,
               (unsigned) G_MODEFRAMES, (unsigned) G_VICOUNT, (unsigned) plat_stats.cont_reads);
    if (G_MODE != g_last_mode) {
        g_last_mode = G_MODE;
        if (!plat_cfg.quiet)
            host_log("frame %u (vi %u): game mode 0x%08X%08X, level %d\n", (unsigned) f,
                     (unsigned) plat_vi_count, (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (int) G_LEVEL);
    }
    for (i = 0; i < plat_cfg.n_dump_frames; i++)
        if (plat_cfg.dump_frames[i] == f) dump_rdram(f);
    if (plat_cfg.dump_every && f % plat_cfg.dump_every == 0) dump_rdram(f);
    host_watch_frame(f);   /* --watch (debugging, plat_host.c) */
    host_crash_test_frame(f);   /* --crash-test (crash.c) */
    for (i = 0; i < plat_cfg.n_pokes; i++) {
        const unsigned *p = plat_cfg.pokes[i];
        if (p[0] != f) continue;
        if (p[2] == 1) *(u8 *) p[1] = (u8) p[3];
        else if (p[2] == 2) *(u16 *) p[1] = (u16) p[3];
        else *(u32 *) p[1] = p[3];
        if (!plat_cfg.quiet) host_log("poke: frame %u: %08X = %X (%u bytes)\n", (unsigned) f, p[1], p[3], p[2]);
    }
    if (plat_cfg.frames && f >= plat_cfg.frames) {
        host_log("stopping: %u frames reached\n", (unsigned) f);
        print_stats();
        plat_si_flush();
        host_exit(0);
    }
}

/* the game state in a crash report (crash.c; no stdio, no heap) */
static void describe(char *buf, unsigned size) {
    host_snprintf(buf, size,
                  "game: thread %s, frame %u, retrace %u, mode 0x%08X%08X (next 0x%08X%08X), level %d, "
                  "frames in mode %u, game VI counter %u",
                  plat_thread_name(plat_running()), (unsigned) plat_stats.frames, (unsigned) plat_vi_count,
                  (unsigned) (G_MODE >> 32), (unsigned) G_MODE, (unsigned) (G_NEXTMODE >> 32), (unsigned) G_NEXTMODE,
                  (int) G_LEVEL, (unsigned) G_MODEFRAMES, (unsigned) G_VICOUNT);
}

/* after the report: with --dump-dir, frame_9999999.bin = RDRAM at the crash */
static void after_crash(void) {
    if (plat_cfg.dump_dir) dump_rdram(9999999);
}

/* ---- boot ------------------------------------------------------------------- */

int port_layout_check(void);   /* layout_check.c */

void plat_start(const HostOpts *o) {
    int bad = port_layout_check();
    if (bad) host_fatal("layout_check.c line %d: a GBI/ABI bit-field is not where the game expects it", bad);
    plat_cfg = *o;
    plat_now = o->boot_count;
    plat_rom = o->rom;
    plat_rom_size = o->rom_size;
    host_crash_set_describe(describe, after_crash);
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

    hd_boot();   /* osInitialize, debug command line, idle thread */
    plat_run();
}
