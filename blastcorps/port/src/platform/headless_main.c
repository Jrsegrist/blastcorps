/* bc_headless.exe: the game logic with no graphics or audio output.
 *
 *   bc_headless.exe ROM [options]
 *     --frames N         stop after N frames (frame = a frame-ending gfx task)
 *     --vis N            stop after N retraces
 *     --dump F1,F2,...   dump RDRAM (8 MB, host byte order) at these frames
 *     --dump-every N     ... and every N frames
 *     --dump-dir DIR     where dumps go (default .)
 *     --trace FILE       one line per frame: retrace, time, game mode, level
 *     --input FILE       controller 1 recording (see input.c)
 *     --no-controller    controller 1 unplugged
 *     --gettime FILE     injected osGetTime values (see os_time.c)
 *     --frame-done FILE  injected frame completion retraces (see os_time.c)
 *     --clock FILE       osGetTime/osGetCount values keyed by caller (os_time.c)
 *     --syms FILE        `nm -n` of this exe (build/headless/bc_headless.syms):
 *                        caller names for --clock, symbols in crash reports
 *     --sync FILE        the emulator's time at each thread switch point
 *                        (os_thread.c; port/tools/compare.py writes it)
 *     --load-log         log every DMA/decompressed load and its asset kind
 *     --frame-sp FILE    per frame, when its task leaves the RSP ("@N VI.F",
 *                        like --frame-done; default: with the RDP)
 *     --eeprom FILE      EEPROM image (read at boot, written on save)
 *     --no-eeprom        no EEPROM chip
 *     --gfx-cycles N     RCP time of a frame's gfx task, in 46.875 MHz counts
 *                        (default 781250 = one retrace)
 *     --small-gfx-cycles N, --aud-cycles N   other gfx tasks / audio tasks (0)
 *     --gettime-cost N   CPU counts charged per osGetTime/osGetCount (0)
 *     --boot-count N     virtual time (counts since power-on) when hd_code
 *                        starts (0); retraces stay on multiples of 781250
 *     --cmdline STR      the debug command line at PI 0xFFB000 (default none)
 *     --mpk FILE         Controller Pak 1 image (mupen64plus .mpk format: four
 *                        32 KB paks, controller 1's first; created if missing)
 *     --print            print the game's debug messages (func_8029A7E4)
 *     -v / -q            verbose / quiet */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdram.h"
#include "plat_host.h"
#include "load/port_load.h"

static void usage(void) {
    fprintf(stderr, "usage: bc_headless.exe ROM [--frames N] [--vis N] [--dump F1,F2..] [--dump-every N]\n"
                    "       [--dump-dir DIR] [--trace FILE] [--input FILE] [--no-controller] [--gettime FILE]\n"
                    "       [--frame-done FILE] [--clock FILE] [--syms FILE] [--sync FILE] [--load-log]\n"
                    "       [--eeprom FILE] [--no-eeprom] [--gfx-cycles N] [--small-gfx-cycles N]\n"
                    "       [--aud-cycles N] [--gettime-cost N] [--boot-count N] [--cmdline STR]\n"
                    "       [--mpk FILE] [--print] [-v] [-q]\n");
    exit(1);
}

static unsigned num(const char *s) {
    return (unsigned) strtoul(s, NULL, 0);
}

static int g_game_print;
unsigned plat_frames(void); /* save.c */

/* the game's debug printf (hd_code 56010.c func_8029A7E4), with --print:
 * to stderr like the platform's own log, prefixed with the frame number */
void port_game_print(const char *fmt, va_list ap) {
    if (g_game_print) {
        fprintf(stderr, "game f=%u: ", plat_frames());
        vfprintf(stderr, fmt, ap);
        if (fmt[0] != 0 && fmt[strlen(fmt) - 1] != '\n') fputc('\n', stderr);
    }
}

/* gensyms' stubs.c: a function nothing defines was called */
void port_stub_hit(const char *name) {
    host_fatal("unimplemented function called: %s", name);
}

static int cmp_u(const void *a, const void *b) {
    unsigned x = *(const unsigned *) a, y = *(const unsigned *) b;
    return x < y ? -1 : x > y;
}

int host_main(int argc, char **argv, int (*extra)(int argc, char **argv, int *i, HostOpts *o),
              void (*prestart)(HostOpts *o)) {
    HostOpts o;
    int i;
    size_t rom_size;
    memset(&o, 0, sizeof o);
    o.gfx_cycles = 781250;
    o.cont_present = 1;
    o.eeprom_present = 1;
    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
#define ARG() (i + 1 < argc ? argv[++i] : (usage(), (char *) NULL))
        if (extra != NULL && extra(argc, argv, &i, &o)) continue;
        if (!strcmp(a, "--frames")) o.frames = num(ARG());
        else if (!strcmp(a, "--vis")) o.max_vis = num(ARG());
        else if (!strcmp(a, "--dump")) {
            char *s = ARG(), *tok;
            for (tok = strtok(s, ","); tok; tok = strtok(NULL, ",")) {
                o.dump_frames = realloc(o.dump_frames, (o.n_dump_frames + 1) * sizeof(unsigned));
                o.dump_frames[o.n_dump_frames++] = num(tok);
            }
            qsort(o.dump_frames, o.n_dump_frames, sizeof(unsigned), cmp_u);
        } else if (!strcmp(a, "--dump-every")) o.dump_every = num(ARG());
        else if (!strcmp(a, "--dump-dir")) o.dump_dir = ARG();
        else if (!strcmp(a, "--trace")) o.trace_path = ARG();
        else if (!strcmp(a, "--input")) o.input_path = ARG();
        else if (!strcmp(a, "--no-controller")) o.cont_present = 0;
        else if (!strcmp(a, "--gettime")) o.gettime_path = ARG();
        else if (!strcmp(a, "--frame-done")) o.frame_done_path = ARG();
        else if (!strcmp(a, "--clock")) o.clock_path = ARG();
        else if (!strcmp(a, "--syms")) o.syms_path = ARG();
        else if (!strcmp(a, "--sync")) o.sync_path = ARG();
        else if (!strcmp(a, "--frame-sp")) o.frame_sp_path = ARG();
        else if (!strcmp(a, "--calls")) o.calls = ARG();
        else if (!strcmp(a, "--load-log")) port_load_verbose = 1;
        else if (!strcmp(a, "--eeprom")) o.eeprom_path = ARG();
        else if (!strcmp(a, "--no-eeprom")) o.eeprom_present = 0;
        else if (!strcmp(a, "--gfx-cycles")) o.gfx_cycles = num(ARG());
        else if (!strcmp(a, "--small-gfx-cycles")) o.small_gfx_cycles = num(ARG());
        else if (!strcmp(a, "--aud-cycles")) o.aud_cycles = num(ARG());
        else if (!strcmp(a, "--gettime-cost")) o.count_per_gettime = num(ARG());
        else if (!strcmp(a, "--boot-count")) o.boot_count = num(ARG());
        else if (!strcmp(a, "--cmdline")) o.cmdline = ARG();
        else if (!strcmp(a, "--print")) o.game_print = g_game_print = 1;
        else if (!strcmp(a, "--mpk")) o.mpk_path = ARG();
        else if (!strcmp(a, "-v")) host_verbose = 1;
        else if (!strcmp(a, "-vv")) host_verbose = 2;
        else if (!strcmp(a, "-q")) o.quiet = 1;
        else if (a[0] == '-') usage();
        else if (o.rom_path == NULL) o.rom_path = a;
        else usage();
#undef ARG
    }
    if (o.rom_path == NULL) usage();
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (rdram_map() || rdram_load(o.rom_path)) return 1;
    o.rom = rom_bytes(&rom_size);
    o.rom_size = (unsigned) rom_size;
    if (prestart != NULL) prestart(&o);
    plat_start(&o);
}

#ifndef PORT_LIVE
int main(int argc, char **argv) {
    return host_main(argc, argv, NULL, NULL);
}
#endif
