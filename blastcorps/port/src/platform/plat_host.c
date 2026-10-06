/* Host (Windows) services: fibers, logging, files, crash reporting. */
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "plat_host.h"

int host_verbose;
static FILE *g_trace;

/* Fiber stacks: reserve 1 MB each (commit grows on demand). */
#define FIBER_STACK_COMMIT 0x10000
#define FIBER_STACK_RESERVE 0x100000

typedef struct {
    void (*fn)(void *);
    void *arg;
} FiberStart;

static void WINAPI fiber_trampoline(void *p) {
    FiberStart s = *(FiberStart *) p;
    free(p);
    host_set_fpu_mode();
    s.fn(s.arg);
    host_fatal("fiber function returned");
}

HostFiber host_fiber_init(void) {
    void *f = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
    if (f == NULL) host_fatal("ConvertThreadToFiberEx failed (%lu)", GetLastError());
    return f;
}

HostFiber host_fiber_create(void (*fn)(void *), void *arg) {
    FiberStart *s = malloc(sizeof *s);
    void *f;
    s->fn = fn;
    s->arg = arg;
    f = CreateFiberEx(FIBER_STACK_COMMIT, FIBER_STACK_RESERVE, FIBER_FLAG_FLOAT_SWITCH, fiber_trampoline, s);
    if (f == NULL) host_fatal("CreateFiberEx failed (%lu)", GetLastError());
    return f;
}

void host_fiber_switch(HostFiber f) {
    if (f != GetCurrentFiber()) SwitchToFiber(f);
}

void host_fiber_delete(HostFiber f) {
    if (f != NULL && f != GetCurrentFiber()) DeleteFiber(f);
}

void host_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

/* is environment variable NAME set (debug switches) */
int host_env(const char *name) {
    return getenv(name) != NULL;
}

void host_fatal(const char *fmt, ...) {
    va_list ap;
    fflush(stdout);
    if (g_trace) fflush(g_trace);
    fprintf(stderr, "FATAL: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    fflush(stderr);
    ExitProcess(2);
}

int host_trace_open(const char *path) {
    g_trace = fopen(path, "w");
    return g_trace ? 0 : -1;
}

void host_trace(const char *fmt, ...) {
    va_list ap;
    if (g_trace == NULL) return;
    va_start(ap, fmt);
    vfprintf(g_trace, fmt, ap);
    va_end(ap);
}

void *host_read_file(const char *path, unsigned *size) {
    FILE *f = fopen(path, "rb");
    long n;
    void *p;
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    p = malloc(n > 0 ? n : 1);
    if (n > 0 && fread(p, 1, n, f) != (size_t) n) {
        fclose(f);
        free(p);
        return NULL;
    }
    fclose(f);
    if (size) *size = (unsigned) n;
    return p;
}

int host_write_file(const char *path, const void *data, unsigned size) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) return -1;
    if (fwrite(data, 1, size, f) != size) {
        fclose(f);
        return -1;
    }
    return fclose(f);
}

void *host_realloc(void *p, unsigned size) {
    void *q = realloc(p, size);
    if (q == NULL) host_fatal("out of memory");
    return q;
}

void host_set_fpu_mode(void) {
    /* MXCSR: FTZ (bit 15) like the VR4300's FPCSR FS; all exceptions masked
     * (default).  Rounding: nearest (default), as FPCSR RM_RN. */
    unsigned int csr = __builtin_ia32_stmxcsr();
    csr |= 0x8000;
    __builtin_ia32_ldmxcsr(csr);
}

void host_exit(int code) {
    if (g_trace) fclose(g_trace);
    fflush(stdout);
    fflush(stderr);
    ExitProcess(code);
}

static void (*g_describe)(void);

static LONG WINAPI crash_filter(EXCEPTION_POINTERS *ep) {
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *c = ep->ContextRecord;
    DWORD code = er->ExceptionCode;
    /* only real faults; leave debugger/C++ exceptions alone */
    if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_INT_DIVIDE_BY_ZERO &&
        code != EXCEPTION_ILLEGAL_INSTRUCTION && code != EXCEPTION_STACK_OVERFLOW &&
        code != EXCEPTION_INT_OVERFLOW && code != EXCEPTION_PRIV_INSTRUCTION &&
        code != EXCEPTION_ARRAY_BOUNDS_EXCEEDED && code != EXCEPTION_IN_PAGE_ERROR)
        return EXCEPTION_CONTINUE_SEARCH;
    fflush(stdout);
    fprintf(stderr, "\nCRASH: exception 0x%08lX at eip=%08lX", code, (unsigned long) c->Eip);
    if (code == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2)
        fprintf(stderr, " (%s 0x%08lX)", er->ExceptionInformation[0] == 1 ? "write" : "read",
                (unsigned long) er->ExceptionInformation[1]);
    fprintf(stderr, "\n  eax=%08lX ebx=%08lX ecx=%08lX edx=%08lX esi=%08lX edi=%08lX ebp=%08lX esp=%08lX\n",
            c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp);
    {
        /* a few return-address candidates from the stack (no frame pointers) */
        DWORD *sp = (DWORD *) c->Esp;
        int i, n = 0;
        MEMORY_BASIC_INFORMATION mbi;
        fprintf(stderr, "  stack code ptrs:");
        for (i = 0; i < 512 && n < 16; i++) {
            DWORD v;
            if (!VirtualQuery(sp + i, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) break;
            v = sp[i];
            if (v >= 0x401000 && v < 0x01000000) {
                fprintf(stderr, " %08lX", v);
                n++;
            }
        }
        fprintf(stderr, "\n");
    }
    if (g_describe) g_describe();
    if (g_trace) fflush(g_trace);
    fflush(stderr);
    ExitProcess(4);
    return EXCEPTION_EXECUTE_HANDLER;
}

void host_install_crash_handler(void (*describe)(void)) {
    g_describe = describe;
    AddVectoredExceptionHandler(1, crash_filter);
}
