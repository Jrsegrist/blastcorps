/* osGetTime / osGetCount from the virtual clock, with injection.
 *
 * The game reads osGetTime() every frame as a random source (00000.c
 * D_803649D8, camera shake, animation choice), so matching an emulator
 * trace needs the emulator's values:
 *
 *  --gettime FILE     one value per line (decimal or 0x hex), consumed by
 *                     successive osGetTime calls; "@N V" sets the value of
 *                     call N (0-based) and later lines continue from there.
 *                     Calls the file doesn't cover fall back to the clock.
 *  plat_gettime_hook  the same from C, for anything smarter.
 *
 * Frame pacing: running game code takes no virtual time, so the time a frame
 * costs on the N64 (CPU + RCP) is charged when its gfx task completes:
 *  --gfx-cycles N     every frame task completes N counts after it starts
 *                     (default 781250 = one retrace);
 *  --frame-done FILE  per frame: "@N VI[.F]" (or one "VI[.F]" per line from
 *                     frame 1): frame N's task completes at retrace VI plus
 *                     fraction F of a period (default .5), e.g. taken from an
 *                     emulator trace; frames the file doesn't cover use
 *                     --gfx-cycles.
 * --gettime-cost N charges N counts per osGetTime/osGetCount call. */
#include "plat.h"

int (*plat_gettime_hook)(u32 call, u64 now, u64 *t);

typedef struct {
    u64 *v;
    u32 n, cap;
} Table;

static Table g_gettime, g_framedone;
static u32 g_calls;

static void table_set(Table *t, u32 idx, u64 v) {
    if (idx >= t->cap) {
        u32 ncap = t->cap ? t->cap * 2 : 1024;
        while (ncap <= idx) ncap *= 2;
        t->v = host_realloc(t->v, ncap * sizeof(u64));
        while (t->cap < ncap) t->v[t->cap++] = ~0ull;
    }
    t->v[idx] = v;
    if (idx + 1 > t->n) t->n = idx + 1;
}

static u64 table_get(const Table *t, u32 idx) {
    return idx < t->n ? t->v[idx] : ~0ull;
}

/* number: decimal or 0x hex; with `vis`, decimal VI[.F] -> count units */
static int parse_val(const char **pp, u64 *out, int vis) {
    const char *p = *pp;
    u64 v = 0;
    int base = 10, any = 0;
    while (*p == ' ' || *p == '\t') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        base = 16;
        p += 2;
    }
    for (;; p++) {
        int d;
        if (*p >= '0' && *p <= '9') d = *p - '0';
        else if (base == 16 && *p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
        else if (base == 16 && *p >= 'A' && *p <= 'F') d = *p - 'A' + 10;
        else break;
        v = v * base + d;
        any = 1;
    }
    if (vis) {
        u64 frac_num = PLAT_VI_PERIOD / 2, scale = 1;
        if (*p == '.') {
            u64 f = 0;
            p++;
            while (*p >= '0' && *p <= '9' && scale < 1000000) f = f * 10 + (*p++ - '0'), scale *= 10;
            frac_num = f * PLAT_VI_PERIOD / scale;
        }
        v = v * PLAT_VI_PERIOD + frac_num;
    }
    *pp = p;
    *out = v;
    return any;
}

static void load_table(Table *t, const char *path, const char *what, u32 first, int vis) {
    unsigned size;
    char *text, *p, *end;
    u32 idx = first, lines = 0;
    text = host_read_file(path, &size);
    if (text == NULL) host_fatal("can't read %s file %s", what, path);
    p = text;
    end = text + size;
    while (p < end) {
        char *eol = p;
        const char *q;
        u64 v, n;
        while (eol < end && *eol != '\n') eol++;
        *eol = 0;
        q = p;
        while (*q == ' ' || *q == '\t') q++;
        if (*q == '@') {
            q++;
            if (parse_val(&q, &n, 0)) idx = (u32) n;
        }
        if (*q != '#' && parse_val(&q, &v, vis)) {
            table_set(t, idx++, v);
            lines++;
        }
        p = eol + 1;
    }
    if (!plat_cfg.quiet) host_log("clock: %u %s values from %s\n", (unsigned) lines, what, path);
}

void plat_clock_init(const char *gettime_path, const char *frame_done_path) {
    if (gettime_path) load_table(&g_gettime, gettime_path, "--gettime", 0, 0);
    if (frame_done_path) load_table(&g_framedone, frame_done_path, "--frame-done", 1, 1);
}

u64 plat_frame_done_time(u32 frame) {
    return table_get(&g_framedone, frame);
}

/* ---- symbols (--syms: `nm -n` of the exe) ---------------------------------
 * Names native code addresses: the keyed clock below and the crash report. */
typedef struct {
    u32 addr;
    const char *name;
} Sym;
static Sym *g_syms;
static u32 g_nsyms;

void plat_syms_load(const char *path) {
    unsigned size;
    char *text = host_read_file(path, &size), *p, *end;
    u32 cap = 0;
    if (text == NULL) host_fatal("can't read --syms file %s", path);
    for (p = text, end = text + size; p < end;) {
        char *eol = p, *name, *dot;
        u32 a = 0;
        int n = 0;
        while (eol < end && *eol != '\n' && *eol != '\r') eol++;
        *eol = 0;
        while (n < 8 && ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f')))
            a = a * 16 + (*p <= '9' ? *p - '0' : *p - 'a' + 10), p++, n++;
        /* "XXXXXXXX T name": text symbols only */
        if (n == 8 && p[0] == ' ' && (p[1] == 'T' || p[1] == 't') && p[2] == ' ') {
            name = p + 3;
            if (*name == '_') name++;          /* i686 C symbols have a leading _ */
            dot = name;
            while (*dot && *dot != '.') dot++; /* func.constprop.0, .isra.0, .part.0 */
            *dot = 0;
            if (g_nsyms == cap) {
                cap = cap ? cap * 2 : 4096;
                g_syms = host_realloc(g_syms, cap * sizeof(Sym));
            }
            g_syms[g_nsyms].addr = a;
            g_syms[g_nsyms].name = name;
            g_nsyms++;
        }
        p = eol + 1;
        while (p < end && (*p == '\n' || *p == '\r')) p++;
    }
}

/* name of the function containing native address A (NULL if unknown) */
const char *plat_sym_name(u32 a, u32 *off) {
    u32 lo = 0, hi = g_nsyms;
    if (g_nsyms == 0 || a < g_syms[0].addr) return NULL;
    while (hi - lo > 1) {
        u32 mid = (lo + hi) / 2;
        if (g_syms[mid].addr <= a) lo = mid;
        else hi = mid;
    }
    if (off) *off = a - g_syms[lo].addr;
    return g_syms[lo].name;
}

/* ---- keyed clock values (--clock FILE) ------------------------------------
 * The emulator side of the comparison (port/tools/compare.py) logs every
 * osGetTime / osGetCount result with the function it returned to.  Lines
 * "T NAME VI VALUE" / "C NAME VI VALUE" (VALUE hex) queue VALUE for the
 * next call of osGetTime / osGetCount from function NAME in (native) retrace
 * VI, so the values reach the same callers whatever order the threads
 * interleave in.  Other calls get the emulator's clock at the current
 * virtual time ("M R1 C1 PERIOD" line; the virtual clock follows the
 * emulator's through --sync).  Needs --syms. */
typedef struct {
    const char *name;
    u64 *v;
    u32 *vi;    /* the (native) retrace the value was read in */
    u32 n, cap, next, misses, used;
} KeyQ;
#define MAX_KEYS 256
static KeyQ g_keys[3][MAX_KEYS]; /* osGetTime, osGetCount, osAiGetLength */
static u32 g_nkeys[3];
static u32 g_key_nocaller[3];
/* "M R1 C1 PERIOD": the emulator's count at native time t is
 * C1 + (t - R1 * PLAT_VI_PERIOD) * PERIOD / PLAT_VI_PERIOD */
static int g_emu_map;
static u64 g_emu_r1, g_emu_c1;
static double g_emu_period;

/* a duration of N emulator counts in native counts (N without --clock) */
u64 plat_emu_counts(u64 n) {
    if (!g_emu_map) return n;
    return (u64) ((double) n * PLAT_VI_PERIOD / g_emu_period + 0.5);
}

static u64 emu_time(void) {
    double d = (double) (s64) (plat_now - g_emu_r1 * PLAT_VI_PERIOD) * g_emu_period / PLAT_VI_PERIOD;
    return (u64) ((s64) g_emu_c1 + (s64) d);
}

static int str_eq(const char *a, const char *b) {
    while (*a && *a == *b) a++, b++;
    return *a == *b;
}

static KeyQ *key_find(int kind, const char *name, int add) {
    u32 i;
    for (i = 0; i < g_nkeys[kind]; i++)
        if (str_eq(g_keys[kind][i].name, name)) return &g_keys[kind][i];
    if (!add || g_nkeys[kind] == MAX_KEYS) return NULL;
    g_keys[kind][g_nkeys[kind]].name = name;
    return &g_keys[kind][g_nkeys[kind]++];
}

void plat_clock_keyed_load(const char *path) {
    unsigned size;
    char *text = host_read_file(path, &size), *p, *end;
    u32 lines = 0;
    if (text == NULL) host_fatal("can't read --clock file %s", path);
    if (g_nsyms == 0) host_fatal("--clock needs --syms");
    for (p = text, end = text + size; p < end;) {
        char *eol = p, *name;
        const char *q;
        int kind;
        u64 v;
        KeyQ *k;
        while (eol < end && *eol != '\n') eol++;
        *eol = 0;
        if (p[0] == 'M' && p[1] == ' ') {
            q = p + 2;
            for (g_emu_r1 = 0; *q >= '0' && *q <= '9'; q++) g_emu_r1 = g_emu_r1 * 10 + (u64) (*q - '0');
            for (q++, g_emu_c1 = 0; *q >= '0' && *q <= '9'; q++) g_emu_c1 = g_emu_c1 * 10 + (u64) (*q - '0');
            {
                double f = 0, scale = 1;
                for (q++; *q >= '0' && *q <= '9'; q++) f = f * 10 + (*q - '0');
                if (*q == '.')
                    for (q++; *q >= '0' && *q <= '9'; q++) f = f * 10 + (*q - '0'), scale *= 10;
                g_emu_period = f / scale;
            }
            g_emu_map = g_emu_period > 0;
        } else if ((p[0] == 'T' || p[0] == 'C' || p[0] == 'A') && p[1] == ' ') {
            u32 vi = 0;
            kind = p[0] == 'T' ? 0 : p[0] == 'C' ? 1 : 2;
            name = p + 2;
            q = name;
            while (*q && *q != ' ') q++;
            if (*q == ' ') {
                *(char *) q = 0;
                q++;
                while (*q >= '0' && *q <= '9') vi = vi * 10 + (u32) (*q++ - '0');
                if (*q == ' ') q++;
                /* the value is hex, with or without 0x */
                if (q[0] == '0' && (q[1] == 'x' || q[1] == 'X')) q += 2;
                for (v = 0;; q++) {
                    if (*q >= '0' && *q <= '9') v = v * 16 + (*q - '0');
                    else if (*q >= 'a' && *q <= 'f') v = v * 16 + (*q - 'a' + 10);
                    else if (*q >= 'A' && *q <= 'F') v = v * 16 + (*q - 'A' + 10);
                    else break;
                }
                k = key_find(kind, name, 1);
                if (k == NULL) host_fatal("--clock: too many callers");
                if (k->n == k->cap) {
                    k->cap = k->cap ? k->cap * 2 : 64;
                    k->v = host_realloc(k->v, k->cap * sizeof(u64));
                    k->vi = host_realloc(k->vi, k->cap * sizeof(u32));
                }
                k->vi[k->n] = vi;
                k->v[k->n++] = v;
                lines++;
            }
        }
        p = eol + 1;
    }
    if (!plat_cfg.quiet) host_log("clock: %u keyed values from %s\n", (unsigned) lines, path);
}

/* The caller's next value read in this retrace; else the emulator's clock
 * at this (synchronised) virtual time. */
int plat_clock_key_take(int kind, void *ra, u64 *v) {
    const char *name;
    KeyQ *k;
    if (g_nkeys[kind] == 0 && !g_emu_map) return 0;
    name = plat_sym_name((u32) ra, NULL);
    k = name ? key_find(kind, name, 0) : NULL;
    if (k == NULL) g_key_nocaller[kind]++;
    else {
        while (k->next < k->n && k->vi[k->next] < plat_vi_count) k->next++;
        if (k->next < k->n && k->vi[k->next] == plat_vi_count) {
            *v = k->v[k->next++];
            k->used++;
            return 1;
        }
        k->misses++;
        if (host_verbose)
            host_log("clock: %s from %s in retrace %u: next value is from retrace %u\n",
                     kind == 2 ? "osAiGetLength" : kind ? "osGetCount" : "osGetTime", name, (unsigned) plat_vi_count,
                     k->next < k->n ? (unsigned) k->vi[k->next] : 0u);
    }
    if (!g_emu_map || kind == 2) return 0;
    *v = emu_time();
    return 1;
}

void plat_clock_report(void) {
    int kind;
    u32 i;
    for (kind = 0; kind < 3; kind++) {
        if (g_nkeys[kind] == 0) continue;
        host_log("clock: %s keyed: %u calls from unkeyed callers\n", kind == 2 ? "osAiGetLength" : kind ? "osGetCount" : "osGetTime",
                 (unsigned) g_key_nocaller[kind]);
        for (i = 0; i < g_nkeys[kind]; i++) {
            KeyQ *k = &g_keys[kind][i];
            host_log("  %-24s used %u of %u, %u calls with no value in their retrace\n", k->name, (unsigned) k->used,
                     (unsigned) k->n, (unsigned) k->misses);
        }
    }
}

OSTime osGetTime(void) {
    u32 call = g_calls++;
    u64 v;
    plat_stats.gettime_calls++;
    plat_now += plat_cfg.count_per_gettime;
    if (plat_clock_key_take(0, __builtin_return_address(0), &v)) return v;
    if (plat_gettime_hook && plat_gettime_hook(call, plat_now, &v)) return v;
    v = table_get(&g_gettime, call);
    return v != ~0ull ? v : plat_now;
}

int (*plat_getcount_hook)(u32 call, u64 now, u32 *c);
static u32 g_count_calls;

u32 osGetCount(void) {
    u32 call = g_count_calls++, c;
    u64 v;
    plat_now += plat_cfg.count_per_gettime;
    if (plat_clock_key_take(1, __builtin_return_address(0), &v)) return (u32) v;
    if (plat_getcount_hook && plat_getcount_hook(call, plat_now, &c)) return c;
    return (u32) plat_now;
}
