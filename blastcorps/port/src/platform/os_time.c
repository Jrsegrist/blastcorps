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

OSTime osGetTime(void) {
    u32 call = g_calls++;
    u64 v;
    plat_stats.gettime_calls++;
    plat_now += plat_cfg.count_per_gettime;
    if (plat_gettime_hook && plat_gettime_hook(call, plat_now, &v)) return v;
    v = table_get(&g_gettime, call);
    return v != ~0ull ? v : plat_now;
}

int (*plat_getcount_hook)(u32 call, u64 now, u32 *c);
static u32 g_count_calls;

u32 osGetCount(void) {
    u32 call = g_count_calls++, c;
    plat_now += plat_cfg.count_per_gettime;
    if (plat_getcount_hook && plat_getcount_hook(call, plat_now, &c)) return c;
    return (u32) plat_now;
}
