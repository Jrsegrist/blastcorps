/* wavcmp: compare two recordings of the same game run (16-bit stereo WAV),
 * e.g. the emulator's AI stream (ai_dump.so) and bc_headless --wav.
 *   wavcmp stat A.wav
 *   wavcmp cmp REF.wav TEST.wav [SECTIONS] [-w SECONDS] [-v]
 * The two streams start at different times and the game's audio frames
 * fall differently (the frame size follows the AI timing), so TEST is cut
 * into windows (default 1 s) and each window is aligned to REF on its own
 * (envelope cross-correlation, then sample-exact refinement around the
 * previous window's lag).  Per window: normalised correlation, SNR of REF
 * against the difference, share of bit-identical samples.  SECTIONS: lines
 * "START END NAME" (seconds of TEST) to summarise per section. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int16_t *s;     /* interleaved L, R */
    long n;         /* frames */
    unsigned rate;
} Wav;

static int load(const char *path, Wav *w) {
    FILE *f = fopen(path, "rb");
    uint8_t h[12], ch[8];
    if (!f) { perror(path); return -1; }
    if (fread(h, 12, 1, f) != 1 || memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4)) return -1;
    w->rate = 22050;
    while (fread(ch, 8, 1, f) == 1) {
        uint32_t len = ch[4] | ch[5] << 8 | ch[6] << 16 | (uint32_t) ch[7] << 24;
        if (!memcmp(ch, "fmt ", 4)) {
            uint8_t fmt[16];
            if (fread(fmt, 16, 1, f) != 1) return -1;
            w->rate = fmt[4] | fmt[5] << 8 | fmt[6] << 16 | (uint32_t) fmt[7] << 24;
            fseek(f, len - 16, SEEK_CUR);
        } else if (!memcmp(ch, "data", 4)) {
            long pos = ftell(f), end;
            fseek(f, 0, SEEK_END);
            end = ftell(f);
            fseek(f, pos, SEEK_SET);
            if (len == 0 || (long) len > end - pos) len = (uint32_t) (end - pos);
            w->n = len / 4;
            w->s = malloc(w->n * 4 + 4);
            if (fread(w->s, 4, w->n, f) != (size_t) w->n) return -1;
            fclose(f);
            return 0;
        } else {
            fseek(f, len, SEEK_CUR);
        }
    }
    return -1;
}

/* envelope: mean |L|+|R| per block of B frames */
static float *envelope(const Wav *w, int B, long *nb) {
    long i, k;
    float *e;
    *nb = w->n / B;
    e = malloc((*nb + 1) * sizeof *e);
    for (i = 0; i < *nb; i++) {
        double a = 0;
        for (k = 0; k < B; k++) a += abs(w->s[(i * B + k) * 2]) + abs(w->s[(i * B + k) * 2 + 1]);
        e[i] = (float) (a / B);
    }
    return e;
}

/* best lag (REF index = TEST index + lag) of envelope windows, in [lo, hi] */
static long env_lag(const float *er, long nr, const float *et, long nt, long t0, long len, long lo, long hi, double *best) {
    long lag, i, bl = lo;
    double bv = -2;
    for (lag = lo; lag <= hi; lag++) {
        double sxy = 0, sxx = 0, syy = 0, sx = 0, sy = 0, c;
        long m = 0;
        for (i = t0; i < t0 + len && i < nt; i++) {
            long j = i + lag;
            double x, y;
            if (j < 0 || j >= nr) continue;
            x = et[i], y = er[j];
            sx += x, sy += y, sxy += x * y, sxx += x * x, syy += y * y, m++;
        }
        if (m < len / 2) continue;
        sxy -= sx * sy / m, sxx -= sx * sx / m, syy -= sy * sy / m;
        c = (sxx > 0 && syy > 0) ? sxy / sqrt(sxx * syy) : -1;
        if (c > bv) bv = c, bl = lag;
    }
    *best = bv;
    return bl;
}

typedef struct {
    double r, snr, exact, rms_ref, rms_test;
    long lag;
} Win;

static void measure(const Wav *ref, const Wav *t, long t0, long len, long lag, Win *o) {
    double sxy = 0, sxx = 0, syy = 0, sd = 0;
    long i, ex = 0, m = 0;
    for (i = t0; i < t0 + len && i < t->n; i++) {
        long j = i + lag;
        int c;
        if (j < 0 || j >= ref->n) continue;
        for (c = 0; c < 2; c++) {
            double x = t->s[i * 2 + c], y = ref->s[j * 2 + c];
            sxy += x * y, sxx += x * x, syy += y * y, sd += (x - y) * (x - y);
            if (t->s[i * 2 + c] == ref->s[j * 2 + c]) ex++;
            m++;
        }
    }
    o->lag = lag;
    o->r = (sxx > 0 && syy > 0) ? sxy / sqrt(sxx * syy) : (sxx == 0 && syy == 0 ? 1 : 0);
    o->snr = sd > 0 ? 10 * log10((syy > 0 ? syy : 1) / sd) : 999;
    o->exact = m ? (double) ex / m : 0;
    o->rms_ref = m ? sqrt(syy / m) : 0;
    o->rms_test = m ? sqrt(sxx / m) : 0;
}

/* sample-exact lag around `center`: least squared difference over +-rad */
static long refine(const Wav *ref, const Wav *t, long t0, long len, long center, long rad) {
    long lag, best = center, i;
    double bv = -1;
    for (lag = center - rad; lag <= center + rad; lag++) {
        double sd = 0;
        for (i = t0; i < t0 + len && i < t->n; i += 1) {
            long j = i + lag;
            double d;
            if (j < 0 || j >= ref->n) { sd += 1e12; break; }
            d = (double) t->s[i * 2] - ref->s[j * 2];
            sd += d * d;
            d = (double) t->s[i * 2 + 1] - ref->s[j * 2 + 1];
            sd += d * d;
            if (bv >= 0 && sd > bv) break;
        }
        if (bv < 0 || sd < bv) bv = sd, best = lag;
    }
    return best;
}

static int cmp_d(const void *a, const void *b) {
    double x = *(const double *) a, y = *(const double *) b;
    return x < y ? -1 : x > y;
}

static void summary(const char *name, Win *w, long n, double secs) {
    double *r = malloc((n + 1) * sizeof *r), *s = malloc((n + 1) * sizeof *s), ex = 0, rr = 0, rt = 0;
    long i, m = 0, silent = 0, good = 0;
    for (i = 0; i < n; i++) {
        if (w[i].rms_ref < 30 && w[i].rms_test < 30) { silent++; continue; }
        r[m] = w[i].r, s[m] = w[i].snr, m++;
        ex += w[i].exact, rr += w[i].rms_ref, rt += w[i].rms_test;
        if (w[i].r > 0.99) good++;
    }
    if (m == 0) {
        printf("%-22s %4ld windows (%.0f s), all silent\n", name, n, secs);
        free(r), free(s);
        return;
    }
    qsort(r, m, sizeof *r, cmp_d);
    qsort(s, m, sizeof *s, cmp_d);
    printf("%-22s %4ld win (%3.0f s, %3ld silent): corr median %.4f p10 %.4f min %.4f; SNR median %5.1f dB p10 %5.1f; "
           "r>0.99 %3.0f%%; bit-exact samples %5.1f%%; rms ref %.0f test %.0f\n",
           name, n, secs, silent, r[m / 2], r[m / 10], r[0], s[m / 2], s[m / 10], 100.0 * good / m, 100 * ex / m,
           rr / m, rt / m);
    free(r), free(s);
}

int main(int argc, char **argv) {
    Wav a, b;
    if (argc >= 3 && !strcmp(argv[1], "stat")) {
        long i, k;
        if (load(argv[2], &a)) return 1;
        printf("%s: %u Hz, %ld frames (%.2f s)\n", argv[2], a.rate, a.n, (double) a.n / a.rate);
        for (i = 0; i < a.n; i += a.rate) {
            double s = 0;
            int pk = 0;
            for (k = i; k < i + (long) a.rate && k < a.n; k++) {
                s += (double) a.s[k * 2] * a.s[k * 2] + (double) a.s[k * 2 + 1] * a.s[k * 2 + 1];
                if (abs(a.s[k * 2]) > pk) pk = abs(a.s[k * 2]);
                if (abs(a.s[k * 2 + 1]) > pk) pk = abs(a.s[k * 2 + 1]);
            }
            printf("%s%.0f/%d", (i / a.rate) % 10 ? " " : (i ? "\n" : ""), sqrt(s / (2.0 * a.rate)), pk);
        }
        printf("\n");
        return 0;
    }
    if (argc >= 4 && !strcmp(argv[1], "cmp")) {
        const char *secf = NULL;
        double wsec = 1.0;
        int verbose = 0, i;
        long W, nw, k, nbr, nbt, lag;
        float *er, *et;
        Win *win;
        double best;
        const int B = 32;
        for (i = 4; i < argc; i++) {
            if (!strcmp(argv[i], "-w") && i + 1 < argc) wsec = atof(argv[++i]);
            else if (!strcmp(argv[i], "-v")) verbose = 1;
            else secf = argv[i];
        }
        if (load(argv[2], &a) || load(argv[3], &b)) return 1;
        printf("ref  %s: %u Hz, %.2f s\ntest %s: %u Hz, %.2f s\n", argv[2], a.rate, (double) a.n / a.rate, argv[3], b.rate,
               (double) b.n / b.rate);
        W = (long) (wsec * b.rate);
        er = envelope(&a, B, &nbr);
        et = envelope(&b, B, &nbt);
        /* global lag from the first 40 s of sound in TEST, searched over +-60 s */
        {
            long t0 = 0, span = (long) (40.0 * b.rate / B), lo = -(long) (60.0 * a.rate / B), hi = -lo;
            while (t0 < nbt && et[t0] < 20) t0++;
            lag = env_lag(er, nbr, et, nbt, t0, span, lo, hi, &best) * B;
            printf("global lag %ld frames (%.3f s), envelope correlation %.4f\n", lag, (double) lag / b.rate, best);
        }
        nw = b.n / W;
        win = calloc(nw + 1, sizeof *win);
        for (k = 0; k < nw; k++) {
            long t0 = k * W, el;
            double c;
            el = env_lag(er, nbr, et, nbt, t0 / B, W / B, lag / B - 64, lag / B + 64, &c);
            lag = refine(&a, &b, t0, W, el * B, B + 8);
            measure(&a, &b, t0, W, lag, &win[k]);
            if (verbose)
                printf("  %6.1f s lag %8ld r %.5f snr %6.1f exact %5.1f%% rms %5.0f/%5.0f\n", (double) t0 / b.rate, lag,
                       win[k].r, win[k].snr, 100 * win[k].exact, win[k].rms_ref, win[k].rms_test);
        }
        summary("all", win, nw, (double) nw * W / b.rate);
        if (secf) {
            FILE *f = fopen(secf, "r");
            char line[256], name[128];
            double s0, s1;
            while (f && fgets(line, sizeof line, f)) {
                long w0, w1;
                if (sscanf(line, "%lf %lf %127[^\n]", &s0, &s1, name) != 3) continue;
                w0 = (long) (s0 * b.rate / W), w1 = (long) (s1 * b.rate / W);
                if (w1 > nw) w1 = nw;
                if (w1 > w0) summary(name, win + w0, w1 - w0, (double) (w1 - w0) * W / b.rate);
            }
            if (f) fclose(f);
        }
        return 0;
    }
    fprintf(stderr, "usage: wavcmp stat A.wav | wavcmp cmp REF.wav TEST.wav [SECTIONS] [-w SECONDS] [-v]\n");
    return 2;
}
