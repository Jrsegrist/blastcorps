/* Controller 1 input source.
 *
 * Without --input the pad is plugged in with nothing pressed (the attract
 * demos play from the game's own recordings).  --input FILE replays a
 * recording: one line per controller read (osContStartReadData; the game
 * reads once per game frame), "BUTTONS STICK_X STICK_Y" (decimal or 0x hex,
 * sticks signed).  "@N BUTTONS X Y" places a line at read N; a value holds
 * until the next line, so sparse files work.  '#' starts a comment. */
#include "plat.h"

typedef struct {
    u32 index;
    u16 button;
    s8 x, y;
} InputLine;

static InputLine *g_lines;
static u32 g_n, g_cap;

static int parse_num(const char **pp, s32 *out) {
    const char *p = *pp;
    s32 v = 0, neg = 0, any = 0, base = 10;
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    if (*p == '-') {
        neg = 1;
        p++;
    }
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
    *out = neg ? -v : v;
    return any;
}

void plat_input_init(const char *path) {
    unsigned size;
    char *text, *p, *end;
    u32 idx = 0;
    if (path == NULL) return;
    text = host_read_file(path, &size);
    if (text == NULL) host_fatal("can't read --input file %s", path);
    p = text;
    end = text + size;
    while (p < end) {
        char *eol = p;
        const char *q;
        s32 b, x, y, n;
        while (eol < end && *eol != '\n') eol++;
        *eol = 0;
        q = p;
        while (*q == ' ' || *q == '\t') q++;
        if (*q == '@') {
            q++;
            if (parse_num(&q, &n)) idx = (u32) n;
        }
        if (*q != '#' && parse_num(&q, &b)) {
            x = y = 0;
            parse_num(&q, &x);
            parse_num(&q, &y);
            if (g_n == g_cap) {
                g_cap = g_cap ? g_cap * 2 : 1024;
                g_lines = host_realloc(g_lines, g_cap * sizeof(InputLine));
            }
            g_lines[g_n].index = idx;
            g_lines[g_n].button = (u16) b;
            g_lines[g_n].x = (s8) x;
            g_lines[g_n].y = (s8) y;
            g_n++;
            idx++;
        }
        p = eol + 1;
    }
    if (!plat_cfg.quiet) host_log("input: %u lines from %s\n", (unsigned) g_n, path);
}

void plat_input_read(u32 index, u16 *button, s8 *x, s8 *y) {
    /* last line at or before index (lines are in file order; indices rise) */
    u32 lo = 0, hi = g_n;
    *button = 0;
    *x = *y = 0;
    while (lo < hi) {
        u32 mid = (lo + hi) / 2;
        if (g_lines[mid].index <= index) lo = mid + 1;
        else hi = mid;
    }
    if (lo > 0) {
        *button = g_lines[lo - 1].button;
        *x = g_lines[lo - 1].x;
        *y = g_lines[lo - 1].y;
    }
}
