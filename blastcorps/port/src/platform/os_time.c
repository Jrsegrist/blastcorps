/* osGetTime / osGetCount from the virtual clock, with injection.
 *
 * The game reads osGetTime() every frame as a random source (00000.c
 * D_803649D8, camera shake, animation choice), so matching an emulator
 * trace needs the emulator's values.  --gettime FILE supplies them: one
 * value per line (decimal or 0x hex), consumed by successive osGetTime
 * calls; '#' starts a comment; a line "@N V" sets the value for call N
 * (0-based) and later calls continue from there.  Calls past the end of the
 * file fall back to the virtual clock.  Optionally every call charges
 * count_per_gettime cycles of CPU time (default 0). */
#include "plat.h"

static u64 *g_inj;
static u32 g_ninj;
static u32 g_calls;

static int parse_u64(const char **pp, u64 *out) {
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
    *pp = p;
    *out = v;
    return any;
}

void plat_clock_init(const char *path) {
    unsigned size;
    char *text, *p, *end;
    u32 cap = 0, idx = 0;
    if (path == NULL) return;
    text = host_read_file(path, &size);
    if (text == NULL) host_fatal("can't read --gettime file %s", path);
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
            if (parse_u64(&q, &n)) idx = (u32) n;
        }
        if (*q != '#' && parse_u64(&q, &v)) {
            if (idx >= cap) {
                u32 ncap = cap ? cap * 2 : 1024;
                while (ncap <= idx) ncap *= 2;
                g_inj = host_realloc(g_inj, ncap * sizeof(u64));
                while (cap < ncap) g_inj[cap++] = ~0ull;
            }
            g_inj[idx++] = v;
            if (idx > g_ninj) g_ninj = idx;
        }
        p = eol + 1;
    }
    if (!plat_cfg.quiet) host_log("clock: %u injected osGetTime values from %s\n", (unsigned) g_ninj, path);
}

OSTime osGetTime(void) {
    u32 call = g_calls++;
    plat_stats.gettime_calls++;
    plat_now += plat_cfg.count_per_gettime;
    if (call < g_ninj && g_inj[call] != ~0ull) return g_inj[call];
    return plat_now;
}

u32 osGetCount(void) {
    plat_now += plat_cfg.count_per_gettime;
    return (u32) plat_now;
}
