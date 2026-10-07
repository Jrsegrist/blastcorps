/* Host (Windows) services: fibers, logging, files, crash reporting. */
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "plat_host.h"
#include "audio/port_audio.h"

int host_verbose;
int host_gui;
void *host_gui_window;
const char *host_log_path;
static FILE *g_trace;

/* UTF-8 -> UTF-16 (0 if the text isn't valid UTF-8 or doesn't fit) */
static int to_wide(const char *s, wchar_t *w, int n) {
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, w, n);
}

void *host_fopen(const char *path, const char *mode) {
    static wchar_t wp[4096];
    wchar_t wm[16];
    if (to_wide(path, wp, 4096) > 0 && to_wide(mode, wm, 16) > 0) return _wfopen(wp, wm);
    return fopen(path, mode);
}

void host_message(const char *text, int error) {
    static wchar_t w[8192];
    fprintf(stderr, "message box (%s): %s\n", error ? "error" : "warning", text);
    fflush(stderr);
    if (getenv("BC_NO_MSGBOX") != NULL) return;   /* automated tests */
    if (host_gui_window != NULL) ShowWindow((HWND) host_gui_window, SW_HIDE);
    if (to_wide(text, w, 8192) <= 0) MultiByteToWideChar(CP_ACP, 0, text, -1, w, 8192);
    MessageBoxW(NULL, w, L"Blast Corps",
                MB_OK | MB_SETFOREGROUND | MB_TOPMOST | (error ? MB_ICONERROR : MB_ICONWARNING));
}

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
    if (host_gui) {
        static char msg[4096];
        va_start(ap, fmt);
        vsnprintf(msg, sizeof msg, fmt, ap);
        va_end(ap);
        host_message(msg, 1);
    }
    ExitProcess(2);
}

int host_trace_open(const char *path) {
    g_trace = host_fopen(path, "w");
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
    FILE *f = host_fopen(path, "rb");
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
    FILE *f = host_fopen(path, "wb");
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

void (*host_exit_hook)(void);

void host_exit(int code) {
    void (*hook)(void) = host_exit_hook;
    host_exit_hook = NULL;
    if (hook != NULL) hook();
    port_audio_close();
    if (g_trace) fclose(g_trace);
    fflush(stdout);
    fflush(stderr);
    ExitProcess((UINT) code);
}

static void (*g_describe)(void);

/* Debugging (--watch FRAME:ADDR[:N], plat_core.c): from frame FRAME, log the
 * first N (default 20) writes to ADDR's 4 KB page: eip and the stack's code
 * pointers (symbolise with bc_headless.syms).  The page is made read-only;
 * each write is let through by single-stepping it and re-protecting. */
static DWORD g_watch_page, g_watch_left;
static int g_watch_step;
extern unsigned plat_frames(void);

static unsigned g_wf, g_wa, g_wn;

/* --watch FRAME:ADDR[:N] (WSL doesn't pass this environment to the exe) */
int host_watch_set(const char *s) {
    if (sscanf(s, "%u:%x:%u", &g_wf, &g_wa, &g_wn) < 2) return -1;
    fprintf(stderr, "watch: page of %08X from frame %u\n", g_wa, g_wf);
    return 0;
}

void host_watch_frame(unsigned frame) {
    unsigned wa = g_wa, wn = g_wn;
    DWORD old;
    if (g_wf == 0 || frame != g_wf) return;
    g_watch_page = wa & ~0xFFFu;
    g_watch_left = wn ? wn : 20;
    VirtualProtect((void *) g_watch_page, 0x1000, PAGE_READONLY, &old);
}

static int watch_filter(EXCEPTION_POINTERS *ep) {
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *c = ep->ContextRecord;
    DWORD old;
    if (er->ExceptionCode == EXCEPTION_SINGLE_STEP && g_watch_step) {
        g_watch_step = 0;
        if (g_watch_left) VirtualProtect((void *) g_watch_page, 0x1000, PAGE_READONLY, &old);
        return 1;
    }
    if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2 &&
        er->ExceptionInformation[0] == 1 && (er->ExceptionInformation[1] & ~0xFFFu) == g_watch_page &&
        g_watch_page) {
        DWORD *sp = (DWORD *) c->Esp;
        int i, k = 0;
        fprintf(stderr, "WATCH: frame %u write %08lX eip=%08lX stack", plat_frames(),
                (unsigned long) er->ExceptionInformation[1], (unsigned long) c->Eip);
        for (i = 0; i < 64 && k < 8; i++)
            if (sp[i] >= 0x401000 && sp[i] < 0x01000000) fprintf(stderr, " %08lX", sp[i]), k++;
        fprintf(stderr, "\n");
        g_watch_left--;
        VirtualProtect((void *) g_watch_page, 0x1000, PAGE_READWRITE, &old);
        c->EFlags |= 0x100;   /* trap after the store, then re-protect */
        g_watch_step = 1;
        return 1;
    }
    return 0;
}

static LONG WINAPI crash_filter(EXCEPTION_POINTERS *ep) {
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *c = ep->ContextRecord;
    DWORD code = er->ExceptionCode;
    if (g_watch_page && watch_filter(ep)) return EXCEPTION_CONTINUE_EXECUTION;
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
    if (host_gui) {
        char msg[1024];
        snprintf(msg, sizeof msg,
                 "Blast Corps has crashed (exception 0x%08lX at 0x%08lX).\n\n"
                 "The details were written to %s.\n\n"
                 "Please report it with that log and what you were doing.",
                 code, (unsigned long) c->Eip, host_log_path ? host_log_path : "the log (stderr)");
        host_message(msg, 1);
    }
    ExitProcess(4);
    return EXCEPTION_EXECUTE_HANDLER;
}

void host_install_crash_handler(void (*describe)(void)) {
    g_describe = describe;
    AddVectoredExceptionHandler(1, crash_filter);
}
