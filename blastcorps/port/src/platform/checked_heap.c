/* Debugging heap corruption (make -C port headless-checked): linked into
 * bc_headless_checked.exe with -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,
 * --wrap=free, so every allocation made by the port and game code gets pages of
 * its own, like Windows' page heap:
 *   - the block ends right before an inaccessible page: a write (or read) past
 *     its end faults at the guilty instruction (the crash report names it);
 *     the up-to-7 bytes of alignment slack after it hold a canary checked by
 *     free/realloc;
 *   - free decommits the pages and the addresses are never reused: a use after
 *     free faults;
 *   - free/realloc of a pointer that was never allocated, or twice, stops with
 *     a crash report.
 * The pages come from one reserved arena (above the N64's RDRAM); pointers
 * from outside it (the C runtime's own allocations) go to the real functions.
 * Address space is not reused, so very long runs can exhaust the arena: then
 * allocations fall back to the real malloc (logged once). */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "plat_host.h"

void *__real_malloc(size_t n);
void *__real_calloc(size_t n, size_t m);
void *__real_realloc(void *p, size_t n);
void __real_free(void *p);

#define PAGE 0x1000u
#define ARENA_SIZE 0x40000000u   /* 1 GB of address space */
#define MAGIC 0xC4EC4ED0u
#define CANARY 0xCD

typedef struct {
    unsigned magic;
    unsigned size;      /* the caller's size */
    unsigned pages;     /* committed pages, header included (not the guard page) */
    unsigned serial;
} Hdr;

static unsigned char *g_lo, *g_hi, *g_next;
static unsigned g_serial, g_live, g_fallback;
static CRITICAL_SECTION g_lock;
static volatile LONG g_init;

static void init(void) {
    static const unsigned bases[] = {0x90000000u, 0xA0000000u, 0xB0000000u, 0x40000000u, 0x50000000u, 0};
    int i;
    if (InterlockedCompareExchange(&g_init, 1, 0) != 0) {
        while (g_init != 2) Sleep(0);
        return;
    }
    InitializeCriticalSection(&g_lock);
    for (i = 0; bases[i] && g_lo == NULL; i++)
        g_lo = VirtualAlloc((void *) (uintptr_t) bases[i], ARENA_SIZE, MEM_RESERVE, PAGE_NOACCESS);
    if (g_lo != NULL) {
        g_hi = g_lo + ARENA_SIZE;
        g_next = g_lo;
        fprintf(stderr, "checked heap: arena %p-%p (every block on its own pages, guard page after it)\n",
                (void *) g_lo, (void *) g_hi);
    } else {
        fprintf(stderr, "checked heap: can't reserve an arena; the real malloc is used\n");
    }
    g_init = 2;
}

static int ours(const void *p) {
    return g_lo != NULL && (const unsigned char *) p >= g_lo && (const unsigned char *) p < g_hi;
}

static void *alloc(size_t n, int zero) {
    unsigned need, pages;
    unsigned char *base, *user;
    Hdr *h;
    if (g_init != 2) init();
    if (n > 0x10000000u) return NULL;
    need = (unsigned) ((n + 7) & ~7u);
    pages = (need + sizeof(Hdr) + PAGE - 1) / PAGE;
    EnterCriticalSection(&g_lock);
    if (g_lo == NULL || g_next + (pages + 1) * PAGE > g_hi) {
        if (g_lo != NULL && !g_fallback++) fprintf(stderr, "checked heap: arena full; the real malloc from now on\n");
        LeaveCriticalSection(&g_lock);
        return zero ? __real_calloc(n ? n : 1, 1) : __real_malloc(n ? n : 1);
    }
    base = g_next;
    g_next += (pages + 1) * PAGE;   /* + the guard page (stays reserved, no access) */
    if (VirtualAlloc(base, pages * PAGE, MEM_COMMIT, PAGE_READWRITE) == NULL) {
        LeaveCriticalSection(&g_lock);
        host_fatal("checked heap: can't commit %u pages (%lu)", pages, GetLastError());
    }
    g_live++;
    h = (Hdr *) base;
    h->magic = MAGIC;
    h->size = (unsigned) n;
    h->pages = pages;
    h->serial = ++g_serial;
    LeaveCriticalSection(&g_lock);
    user = base + pages * PAGE - need;
    memset(user + n, CANARY, need - n);
    (void) zero;   /* fresh pages are zero */
    return user;
}

static Hdr *header(void *p, const char *what) {
    unsigned char *base = (unsigned char *) ((uintptr_t) p & ~(uintptr_t) (PAGE - 1));
    Hdr *h;
    MEMORY_BASIC_INFORMATION mbi;
    /* the header is at the start of the block's first page: walk back to it */
    for (;;) {
        if (!VirtualQuery(base, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) {
            static char why[160];
            host_snprintf(why, sizeof why, "checked heap: %s(%p): not a live block (freed twice, or never allocated)",
                          what, p);
            host_crash_now(why);
        }
        h = (Hdr *) base;
        if (h->magic == MAGIC) break;
        base -= PAGE;
    }
    {
        unsigned need = (h->size + 7) & ~7u;
        unsigned char *user = (unsigned char *) h + h->pages * PAGE - need, *q;
        static char why[200];
        if ((unsigned char *) p != user) {
            host_snprintf(why, sizeof why, "checked heap: %s(%p): not the start of a block (block %p, %u bytes)", what,
                          p, (void *) user, h->size);
            host_crash_now(why);
        }
        /* the gap between the header and the block stays zero: else an underrun */
        for (q = (unsigned char *) (h + 1); q < user; q++)
            if (*q != 0) {
                host_snprintf(why, sizeof why,
                              "checked heap: %s(%p): block of %u bytes (#%u) was written before its start (-%u)", what,
                              p, h->size, h->serial, (unsigned) (user - q));
                host_crash_now(why);
            }
        for (q = user + h->size; q < user + need; q++)
            if (*q != CANARY) {
                host_snprintf(why, sizeof why,
                              "checked heap: %s(%p): block of %u bytes (#%u) was written past its end (+%u)", what, p,
                              h->size, h->serial, (unsigned) (q - user));
                host_crash_now(why);
            }
    }
    return h;
}

static void release(Hdr *h) {
    unsigned pages = h->pages;
    h->magic = 0;
    VirtualFree(h, pages * PAGE, MEM_DECOMMIT);
    EnterCriticalSection(&g_lock);
    g_live--;
    LeaveCriticalSection(&g_lock);
}

void *__wrap_malloc(size_t n) {
    return alloc(n, 0);
}

void *__wrap_calloc(size_t n, size_t m) {
    if (m && n > 0x10000000u / m) return NULL;
    return alloc(n * m, 1);
}

/* the port's code frees only what it allocated (nothing it uses hands out
 * C-runtime-allocated memory): a pointer from outside the arena is a bug
 * (a stale or garbage pointer), unless the arena ran full */
static void foreign(void *p, const char *what) {
    static char why[160];
    if (g_lo == NULL || g_fallback) return;
    host_snprintf(why, sizeof why, "checked heap: %s(%p): not allocated by this heap (garbage or stale pointer)", what,
                  p);
    host_crash_now(why);
}

void __wrap_free(void *p) {
    if (p == NULL) return;
    if (!ours(p)) {
        foreign(p, "free");
        __real_free(p);
        return;
    }
    release(header(p, "free"));
}

void *__wrap_realloc(void *p, size_t n) {
    Hdr *h;
    void *q;
    if (p == NULL) return alloc(n, 0);
    if (!ours(p)) {
        foreign(p, "realloc");
        return __real_realloc(p, n);
    }
    h = header(p, "realloc");
    if (n == 0) {
        release(h);
        return NULL;
    }
    q = alloc(n, 0);
    if (q == NULL) return NULL;
    memcpy(q, p, h->size < n ? h->size : n);
    release(h);
    return q;
}
