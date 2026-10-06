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
int host_env(const char *name);   /* is the environment variable set (debug switches) */
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
typedef struct HostOpts {
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
    const char *syms_path;     /* `nm -n` of the exe (names for --clock, crash reports) */
    const char *clock_path;    /* keyed osGetTime/osGetCount values (os_time.c) */
    const char *sync_path;     /* the emulator's thread timing (os_thread.c) */
    const char *frame_sp_path; /* when each frame task leaves the RSP (os_time.c) */
    const char *calls;         /* function names whose calls are logged (os_thread.c) */
    int quiet;
    int game_print;            /* print the game's debug messages (func_8029A7E4) */
    const char *mpk_path;      /* Controller Pak image (.mpk), or NULL: no pak */
    const char *wav_path;      /* --wav: the AI stream (what the game plays) as a WAV file */
    const char *audio_capture; /* --audio-capture: audio tasks' inputs/outputs (port/tools/audio) */
    unsigned audio_capture_max;
    int audio_off;             /* --no-audio: audio tasks are not run (no output) */
    const struct HostLive *live; /* renderer, window, live input (bc.exe); NULL in bc_headless */
} HostOpts;

/* The VI registers as libultra's VI manager would program them for the
 * frame on screen (os_hw.c fills them at every retrace). */
typedef struct {
    unsigned status, origin, width, intr, burst, v_sync, h_sync, leap, h_start, v_start, v_burst,
        x_scale, y_scale;
} HostViRegs;

/* What the windowed build (bc.exe, port/src/live/) adds to the platform.
 * The game logic steps exactly as in bc_headless: these hooks observe the
 * game (they render its graphics tasks and show its frames) and pace the
 * virtual clock to real time, but never change what the game sees, except
 * through `input` (the player's controller). */
typedef struct HostLive {
    /* after the ROM images are in RDRAM, before the game starts */
    void (*boot)(void);
    /* a graphics task the RSP would run: physical addresses of the ucode
     * text and data and of the display list */
    void (*gfx_task)(unsigned ucode, unsigned ucode_data, unsigned data_ptr, unsigned data_size);
    /* a retrace (virtual time `when`, in counts): VI registers of the frame
     * on screen; paces to real time and presents */
    void (*vi)(const HostViRegs *regs, unsigned vi_count, unsigned long long when, unsigned frames);
    /* controller 1 now: returns 1 and fills the pad if the live input is used */
    int (*input)(unsigned short *button, signed char *x, signed char *y);
    /* a buffer the AI starts playing: `frames` host-order L/R sample pairs
     * at `rate` Hz (virtual time; the output paces it to real time) */
    void (*audio)(const short *lr, unsigned frames, unsigned rate);
} HostLive;

/* platform entry (plat_core.c): boots the game and never returns */
void plat_start(const HostOpts *o) __attribute__((noreturn));

/* option parsing and start-up shared by bc_headless.exe and bc.exe
 * (headless_main.c).  `extra` gets the options host_main doesn't know
 * (return 1 if consumed; *i may advance), `prestart` runs just before the
 * game starts (after the ROM is loaded). */
int host_main(int argc, char **argv, int (*extra)(int argc, char **argv, int *i, HostOpts *o),
              void (*prestart)(HostOpts *o));

#endif
