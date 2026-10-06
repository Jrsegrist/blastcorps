/* Host (Windows) services for the platform layer.  This header uses no
 * libultra types, so it can be included both by files that see <windows.h>
 * (plat_host.c) and by files that see the game's <ultra64.h>. */
#ifndef PLAT_HOST_H
#define PLAT_HOST_H

typedef void *HostFiber;

/* Turn the calling (main) thread into a fiber; returns it. */
HostFiber host_fiber_init(void);
/* A new fiber running fn(arg) on its own host stack (the game's N64 stacks
 * are too small for gcc -O2 code and stay unused). */
HostFiber host_fiber_create(void (*fn)(void *), void *arg);
void host_fiber_switch(HostFiber f);
void host_fiber_delete(HostFiber f);

/* stderr logging */
void host_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void host_fatal(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
extern int host_verbose;

/* per-frame trace file (no-op until opened) */
int host_trace_open(const char *path);
void host_trace(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/* whole-file helpers: read returns a malloc'd buffer (NULL if missing) */
void *host_read_file(const char *path, unsigned *size);
int host_write_file(const char *path, const void *data, unsigned size);
void *host_realloc(void *p, unsigned size);

/* flush-to-zero for the current fiber (the game's osInitialize sets FPCSR FS) */
void host_set_fpu_mode(void);

/* exit the process (flushes stdio) */
void host_exit(int code) __attribute__((noreturn));

/* install the crash reporter; `describe` is called to add game state */
void host_install_crash_handler(void (*describe)(void));

/* Run options (headless_main.c parses them; the platform reads them as
 * plat_cfg).  Plain C types only: unsigned/int are 32 bits here. */
typedef struct {
    const char *rom_path;
    unsigned char *rom;        /* the ROM image (big-endian .z64) */
    unsigned rom_size;
    unsigned frames;           /* stop after this many frames (0 = never) */
    unsigned max_vis;          /* stop after this many retraces (0 = never) */
    unsigned gfx_cycles;       /* RSP+RDP time of a frame's gfx task (counts) */
    unsigned aud_cycles;       /* RSP time of an audio task */
    unsigned small_gfx_cycles; /* RSP time of other gfx tasks (shadows, cull test) */
    unsigned count_per_gettime;/* CPU counts charged per osGetTime/osGetCount */
    unsigned boot_count;       /* count (time since power-on) when hd_code starts */
    const char *dump_dir;
    unsigned *dump_frames;     /* frame numbers to dump RDRAM at (ascending) */
    unsigned n_dump_frames;
    unsigned dump_every;       /* also dump every N frames (0 = off) */
    const char *input_path;
    const char *gettime_path;
    const char *frame_done_path;
    const char *eeprom_path;
    int eeprom_present;
    int cont_present;          /* controller 1 plugged in */
    const char *cmdline;       /* debug command line the game reads at PI 0xFFB000 */
    const char *trace_path;    /* one summary line per frame */
    int quiet;
} HostOpts;

/* platform entry (plat_core.c): boots the game and never returns */
void plat_start(const HostOpts *o) __attribute__((noreturn));

#endif
