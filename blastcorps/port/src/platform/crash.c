/* Crash reporting for bc.exe and bc_headless.exe (see plat_host.h).
 *
 * host_crash_init (first thing in main) installs an unhandled-exception
 * filter, a SIGABRT handler and the error modes, and starts a reporter
 * thread that waits.  A crash (a fault nothing handled, on any thread or
 * fiber; std::terminate and abort() come here through host_crash_now) hands
 * its exception pointers to the reporter and blocks.  The reporter, on its
 * own stack (a stack overflow leaves the crashed one unusable) and without
 * the C runtime's locks or heap:
 *   - writes the report to stderr (bc.exe: its log, bc.log when it has no
 *     stderr) and to bc-crash-YYYYMMDD-HHMMSS.txt: the exception, the
 *     module + RVA of the fault, the registers, a backtrace with function
 *     names, and the game state (host_crash_set_describe);
 *   - writes bc-crash-YYYYMMDD-HHMMSS.dmp (MiniDumpWriteDump, dbghelp.dll
 *     from System32; with the N64 RDRAM and the exe's .data/.bss) next to it;
 *   - shows a message box (bc.exe without --no-msgbox) and ends the process
 *     with exit code 4 (TerminateProcess: no DLL or atexit teardown in a
 *     process that just crashed).
 *
 * Names: the exe's own COFF symbol table (the PE file keeps it: `nm` reads
 * it; the dist build strips only the DWARF), read from the file at crash
 * time, else the nearest export of a DLL.  The backtrace is a scan of the
 * crashed stack for return addresses: values pointing into code right after
 * a call instruction (the code is built without frame pointers), so it can
 * list a stale frame or two among the real ones.
 *
 * --no-msgbox (or BC_NO_MSGBOX in the environment): no window of any kind
 * pops up: host_message only logs, the crash report shows no box, Windows'
 * own fault/critical-error dialogs are off (SetErrorMode, WER no-UI), the C
 * runtime's assert/abort messages go to stderr, and MessageBox* calls by the
 * exe's statically linked code (RT64) and SDL2.dll only log their text. */
#include <windows.h>
#include <dbghelp.h>
#include <io.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plat_host.h"

int host_no_msgbox;
static const char *host_crash_tag = "Blast Corps port";
static const char *host_crash_version = "dev";
static const char *host_crash_exe_kind = "?";

#define CRASH_EXIT 4
#define MAX_FRAMES 40

static DWORD g_main_tid;
static HANDLE g_go, g_done;
static volatile LONG g_crashing;
static DWORD g_reporter_tid, g_dumper_tid;
static EXCEPTION_POINTERS *volatile g_ep;
static volatile DWORD g_crash_tid;
static void *volatile g_crash_fiber;
static const char *volatile g_reason;   /* host_crash_now: what happened */
static void (*g_describe)(char *buf, unsigned size);
static void (*g_after)(void);
static const char *g_box;               /* the message box text (bc.exe without --no-msgbox) */
static volatile LONG g_boxing;          /* the box is up */
static wchar_t g_dir[MAX_PATH];         /* where the .dmp/.txt go ("" = current folder) */
static wchar_t g_exe_path[MAX_PATH];
static HANDLE g_exe_file = INVALID_HANDLE_VALUE;   /* the exe, for its symbol table */
static LPTOP_LEVEL_EXCEPTION_FILTER g_prev_filter;

typedef BOOL(WINAPI *MiniDumpWriteDump_t)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION,
                                          PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);
typedef DWORD(WINAPI *GetMappedFileNameW_t)(HANDLE, LPVOID, LPWSTR, DWORD);
static MiniDumpWriteDump_t p_MiniDumpWriteDump;
static GetMappedFileNameW_t p_GetMappedFileNameW;

/* ---- output without the C runtime's locks or heap -------------------------- */
static char g_report[64 * 1024];
static unsigned g_report_len;
static HANDLE g_err_handle;

int host_snprintf(char *buf, unsigned size, const char *fmt, ...) {
    va_list ap;
    int n;
    if (size == 0) return 0;
    va_start(ap, fmt);
    n = _vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (n < 0 || (unsigned) n >= size) {
        buf[size - 1] = 0;
        n = (int) size - 1;
    }
    return n;
}

static void out(const char *fmt, ...) {
    char line[1024];
    va_list ap;
    int n;
    DWORD w;
    va_start(ap, fmt);
    n = _vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    if (n < 0 || n >= (int) sizeof line) n = (int) sizeof line - 1, line[n] = 0;
    if (g_err_handle != INVALID_HANDLE_VALUE && g_err_handle != NULL) WriteFile(g_err_handle, line, (DWORD) n, &w, NULL);
    if (g_report_len + n < sizeof g_report) {
        memcpy(g_report + g_report_len, line, n);
        g_report_len += n;
    }
}

/* ---- modules and names -------------------------------------------------------- */
static HMODULE module_of(DWORD addr) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void *) addr, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT || mbi.Type != MEM_IMAGE)
        return NULL;
    return (HMODULE) mbi.AllocationBase;
}

static int is_code(DWORD addr) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void *) addr, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) return 0;
    return (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}

static const char *module_name(HMODULE m, char *buf, unsigned size) {
    wchar_t w[MAX_PATH];
    const wchar_t *base;
    DWORD n = 0;
    if (p_GetMappedFileNameW != NULL) n = p_GetMappedFileNameW(GetCurrentProcess(), (void *) m, w, MAX_PATH);
    if (n == 0) return "?";
    w[n < MAX_PATH ? n : MAX_PATH - 1] = 0;
    base = wcsrchr(w, L'\\');
    base = base ? base + 1 : w;
    WideCharToMultiByte(CP_UTF8, 0, base, -1, buf, (int) size, NULL, NULL);
    buf[size - 1] = 0;
    return buf;
}

static IMAGE_NT_HEADERS *nt_headers(HMODULE m) {
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *) m;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
    return (IMAGE_NT_HEADERS *) ((BYTE *) m + dos->e_lfanew);
}

/* a DLL: the nearest named export at or below addr */
static int export_name(HMODULE m, DWORD addr, char *buf, unsigned size, DWORD *off) {
    IMAGE_NT_HEADERS *nt = nt_headers(m);
    IMAGE_DATA_DIRECTORY *dd;
    IMAGE_EXPORT_DIRECTORY *ex;
    DWORD *funcs, *names, rva = addr - (DWORD) m, best = 0, i;
    WORD *ords;
    const char *best_name = NULL;
    if (nt == NULL) return 0;
    dd = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (dd->VirtualAddress == 0 || dd->Size == 0) return 0;
    ex = (IMAGE_EXPORT_DIRECTORY *) ((BYTE *) m + dd->VirtualAddress);
    funcs = (DWORD *) ((BYTE *) m + ex->AddressOfFunctions);
    names = (DWORD *) ((BYTE *) m + ex->AddressOfNames);
    ords = (WORD *) ((BYTE *) m + ex->AddressOfNameOrdinals);
    for (i = 0; i < ex->NumberOfNames; i++) {
        DWORD f = funcs[ords[i]];
        if (f >= dd->VirtualAddress && f < dd->VirtualAddress + dd->Size) continue;   /* forwarder */
        if (f <= rva && f > best) {
            best = f;
            best_name = (const char *) m + names[i];
        }
    }
    if (best_name == NULL) return 0;
    host_snprintf(buf, size, "%s", best_name);
    *off = rva - best;
    return 1;
}

/* The exe: its COFF symbol table (PointerToSymbolTable), read from the file
 * once for all addresses.  Entries are 18 bytes: name (8 bytes, or 0 + an
 * offset into the string table that follows the symbols), value (section
 * offset), section number, type, storage class, aux count. */
typedef struct {
    DWORD addr;      /* what to name */
    DWORD sym;       /* best symbol's address (0: none) */
    char name[160];
    DWORD str_off;   /* long name: offset in the string table (0: short name in `name`) */
} SymQuery;

static BYTE g_symbuf[18 * 3640];

static void coff_names(SymQuery *q, int nq) {
    HMODULE exe = GetModuleHandleW(NULL);
    IMAGE_NT_HEADERS *nt = nt_headers(exe);
    IMAGE_SECTION_HEADER *sec;
    DWORD nsyms, symptr, done = 0, strtab, rd, skip = 0;
    HANDLE f;
    int i, nsec;
    if (nt == NULL || nt->FileHeader.PointerToSymbolTable == 0) return;
    nsyms = nt->FileHeader.NumberOfSymbols;
    symptr = nt->FileHeader.PointerToSymbolTable;
    sec = IMAGE_FIRST_SECTION(nt);
    nsec = nt->FileHeader.NumberOfSections;
    f = g_exe_file;   /* opened at start-up (opening a file can need the heap) */
    if (f == INVALID_HANDLE_VALUE) return;
    if (SetFilePointer(f, (LONG) symptr, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) return;
    while (done < nsyms) {
        DWORD want = nsyms - done, k;
        if (want > sizeof g_symbuf / 18) want = sizeof g_symbuf / 18;
        if (!ReadFile(f, g_symbuf, want * 18, &rd, NULL) || rd != want * 18) return;
        for (k = 0; k < want; k++) {
            BYTE *e = g_symbuf + k * 18;
            DWORD value, va;
            short secno;
            WORD type;
            BYTE cls, naux;
            if (skip) {   /* the previous symbol's aux records */
                skip--;
                continue;
            }
            memcpy(&value, e + 8, 4);
            memcpy(&secno, e + 12, 2);
            cls = e[16];
            naux = e[17];
            skip = naux;
            if (secno <= 0 || secno > nsec || (cls != IMAGE_SYM_CLASS_EXTERNAL && cls != IMAGE_SYM_CLASS_STATIC))
                continue;
            if (!(sec[secno - 1].Characteristics & IMAGE_SCN_CNT_CODE)) continue;
            /* functions (type 0x20), and globals from assembly; not section
             * symbols (.text, .text$name: static, type 0) */
            memcpy(&type, e + 14, 2);
            if (type != 0x20 && cls != IMAGE_SYM_CLASS_EXTERNAL) continue;
            if (e[0] == '.') continue;
            va = (DWORD) exe + sec[secno - 1].VirtualAddress + value;
            for (i = 0; i < nq; i++) {
                if (va <= q[i].addr && va >= q[i].sym) {
                    DWORD z;
                    memcpy(&z, e, 4);
                    q[i].sym = va;
                    if (z == 0) {
                        memcpy(&q[i].str_off, e + 4, 4);
                        q[i].name[0] = 0;
                    } else {
                        memcpy(q[i].name, e, 8);
                        q[i].name[8] = 0;
                        q[i].str_off = 0;
                    }
                }
            }
        }
        done += want;
    }
    strtab = symptr + nsyms * 18;
    for (i = 0; i < nq; i++) {
        if (q[i].sym == 0 || q[i].str_off == 0) continue;
        if (SetFilePointer(f, (LONG) (strtab + q[i].str_off), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) continue;
        if (ReadFile(f, q[i].name, sizeof q[i].name - 1, &rd, NULL)) q[i].name[rd < sizeof q[i].name ? rd : 0] = 0;
        q[i].name[sizeof q[i].name - 1] = 0;
    }
}

/* ---- the backtrace ------------------------------------------------------------ */
/* is `ret` a return address: right after a call instruction? */
static int after_call(DWORD ret) {
    const BYTE *p = (const BYTE *) ret;
    if (ret < 0x10000 || !is_code(ret - 7) || !is_code(ret - 1)) return 0;
    if (p[-5] == 0xE8) return 1;                                                  /* call rel32 */
    if (p[-2] == 0xFF && ((p[-1] & 0x38) == 0x10) && ((p[-1] & 0xC0) == 0xC0 ||   /* call reg */
                          ((p[-1] & 0xC0) == 0 && (p[-1] & 7) != 4 && (p[-1] & 7) != 5)))   /* call [reg] */
        return 1;
    if (p[-3] == 0xFF && (p[-2] & 0xF8) == 0x50 && (p[-2] & 7) != 4) return 1;   /* call [reg+d8] */
    if (p[-3] == 0xFF && p[-2] == 0x14) return 1;                                /* call [sib] */
    if (p[-4] == 0xFF && p[-3] == 0x54) return 1;                                /* call [sib+d8] */
    if (p[-6] == 0xFF && (p[-5] == 0x15 || ((p[-5] & 0xF8) == 0x90 && (p[-5] & 7) != 4))) return 1;   /* [d32] */
    if (p[-7] == 0xFF && p[-6] == 0x94) return 1;                                /* call [sib+d32] */
    return 0;
}

static int backtrace(const CONTEXT *c, DWORD *frames, int max) {
    int n = 0;
    DWORD sp = c->Esp & ~3u, end;
    MEMORY_BASIC_INFORMATION mbi;
    frames[n++] = c->Eip;
    if (!VirtualQuery((void *) sp, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) return n;
    end = (DWORD) mbi.BaseAddress + mbi.RegionSize;
    /* the committed part of this stack can span regions (guard page moves) */
    for (;;) {
        MEMORY_BASIC_INFORMATION next;
        if (!VirtualQuery((void *) end, &next, sizeof next) || next.State != MEM_COMMIT ||
            next.AllocationBase != mbi.AllocationBase)
            break;
        end = (DWORD) next.BaseAddress + next.RegionSize;
    }
    if (end - sp > 0x40000) end = sp + 0x40000;
    for (; sp + 4 <= end && n < max; sp += 4) {
        DWORD v = *(DWORD *) sp;
        if (n == 1 && v == c->Eip) continue;   /* host_crash_now: its own return address */
        if (after_call(v)) frames[n++] = v;
    }
    return n;
}

/* a direct call's target (following an import thunk `jmp [iat]`), else 0 */
static DWORD call_target(DWORD ret) {
    const BYTE *p = (const BYTE *) ret;
    DWORD t;
    LONG rel;
    if (p[-5] != 0xE8) return 0;
    memcpy(&rel, p - 4, 4);
    t = ret + (DWORD) rel;
    if (is_code(t) && ((const BYTE *) t)[0] == 0xFF && ((const BYTE *) t)[1] == 0x25) {
        DWORD slot;
        memcpy(&slot, (const BYTE *) t + 2, 4);
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((void *) slot, &mbi, sizeof mbi) && mbi.State == MEM_COMMIT &&
            !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            t = *(DWORD *) slot;
    }
    return t;
}

static void print_frames(const DWORD *frames, int n) {
    static SymQuery q[MAX_FRAMES];
    static DWORD start[MAX_FRAMES];
    HMODULE exe = GetModuleHandleW(NULL);
    char exe_name[128];
    int i, last_ok = 0;
    memset(q, 0, sizeof q);
    for (i = 0; i < n; i++) q[i].addr = i == 0 ? frames[i] : frames[i] - 1;   /* the call, not what follows */
    coff_names(q, n);
    module_name(exe, exe_name, sizeof exe_name);
    for (i = 0; i < n; i++) {
        HMODULE m = module_of(frames[i]);
        char mod[128], nm[160];
        const char *name = q[i].name;
        DWORD off = 0, t;
        char mark = ' ';
        start[i] = 0;
        if (m == exe && q[i].sym) {
            if (name[0] == '_') name++;   /* i686 C names */
            start[i] = q[i].sym;
        } else if (m != NULL && export_name(m, q[i].addr, nm, sizeof nm, &off)) {
            name = nm;
            start[i] = q[i].addr - off;
        } else {
            name = NULL;
        }
        /* a frame whose call (direct) went to the function of the frame below
         * it is certainly on the call chain; '?' = can't tell (an indirect
         * call, a tail call, or a stale value on the stack) */
        if (i > 0) {
            t = call_target(frames[i]);
            if (t != 0 && start[last_ok] != 0 && t == start[last_ok]) last_ok = i;
            else mark = '?';
        }
        off = name != NULL ? frames[i] - start[i] : 0;
        if (m == NULL)
            out("  #%-2d%c %08lX  (not in a module)\n", i, mark, frames[i]);
        else if (name != NULL)
            out("  #%-2d%c %08lX  %s+%06lX  %s+0x%lX\n", i, mark, frames[i], m == exe ? exe_name : module_name(m, mod, sizeof mod),
                frames[i] - (DWORD) m, name, off);
        else
            out("  #%-2d%c %08lX  %s+%06lX\n", i, mark, frames[i], module_name(m, mod, sizeof mod), frames[i] - (DWORD) m);
    }
}

/* ---- the minidump ------------------------------------------------------------- */
static struct {
    DWORD base, size;
} g_extra[8];
static int g_nextra, g_extra_i;

static BOOL CALLBACK dump_callback(PVOID param, const PMINIDUMP_CALLBACK_INPUT in, PMINIDUMP_CALLBACK_OUTPUT o) {
    (void) param;
    if (in == NULL || o == NULL) return FALSE;
    /* the reporter stays out of the dump, so MiniDumpWriteDump doesn't
     * suspend it: if the dump hangs (a corrupt heap), the reporter's time
     * limit still ends the process */
    if (in->CallbackType == IncludeThreadCallback) return in->IncludeThread.ThreadId != g_reporter_tid;
    if (in->CallbackType == MemoryCallback) {
        if (g_extra_i >= g_nextra) return FALSE;
        o->MemoryBase = g_extra[g_extra_i].base;
        o->MemorySize = g_extra[g_extra_i].size;
        g_extra_i++;
        return TRUE;
    }
    return TRUE;
}

static void add_extra(DWORD base, DWORD size) {
    MEMORY_BASIC_INFORMATION mbi;
    if (g_nextra >= 8 || size == 0) return;
    if (!VirtualQuery((void *) base, &mbi, sizeof mbi) || mbi.State != MEM_COMMIT) return;
    g_extra[g_nextra].base = base;
    g_extra[g_nextra].size = size;
    g_nextra++;
}

static int write_dump(HANDLE f, EXCEPTION_POINTERS *ep, DWORD tid) {
    MINIDUMP_EXCEPTION_INFORMATION mei;
    MINIDUMP_CALLBACK_INFORMATION cb;
    HMODULE exe = GetModuleHandleW(NULL);
    IMAGE_NT_HEADERS *nt = nt_headers(exe);
    if (p_MiniDumpWriteDump == NULL) return 0;
    /* the N64's RDRAM and the exe's writable sections (.data, .bss) */
    g_nextra = g_extra_i = 0;
    add_extra(0x80000000u, 0x800000u);
    if (nt != NULL) {
        IMAGE_SECTION_HEADER *s = IMAGE_FIRST_SECTION(nt);
        int i;
        for (i = 0; i < nt->FileHeader.NumberOfSections; i++)
            if ((s[i].Characteristics & IMAGE_SCN_MEM_WRITE) && !(s[i].Characteristics & IMAGE_SCN_MEM_EXECUTE))
                add_extra((DWORD) exe + s[i].VirtualAddress, s[i].Misc.VirtualSize);
    }
    mei.ThreadId = tid;
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;
    cb.CallbackRoutine = dump_callback;
    cb.CallbackParam = NULL;
    return p_MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), f,
                               (MINIDUMP_TYPE) (MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithThreadInfo |
                                                MiniDumpWithUnloadedModules),
                               &mei, NULL, &cb);
}

/* The dump is written on a third thread (also started at boot), with a time
 * limit: MiniDumpWriteDump allocates from the process heap, and a crash
 * inside the heap leaves its lock held by the crashed thread for good.  The
 * file is marked delete-on-close until the dump is complete, so a dump cut
 * short by the end of the process leaves no file behind. */
static HANDLE g_dump_go, g_dump_done, g_dump_file, g_reporter_thread;
static volatile LONG g_dump_ok, g_heap_bad;

static void set_delete(HANDLE f, BOOL del) {
    FILE_DISPOSITION_INFO d;
    d.DeleteFile = del;
    SetFileInformationByHandle(f, FileDispositionInfo, &d, sizeof d);
}

static void make_stem(void);
static wchar_t g_stem[MAX_PATH];
static wchar_t g_dump_path[MAX_PATH + 16];
static char g_dump_name[MAX_PATH * 3];

/* (creating files can allocate from the process heap too: path conversion) */
static DWORD WINAPI dumper(void *arg) {
    int i, k;
    wchar_t *path = g_dump_path;
    (void) arg;
    WaitForSingleObject(g_dump_go, INFINITE);
    /* bc-crash-YYYYMMDD-HHMMSS[-N].dmp: a new file in the crash folder, else %TEMP% */
    g_dump_file = INVALID_HANDLE_VALUE;
    for (k = 0; k < 2 && g_dump_file == INVALID_HANDLE_VALUE; k++) {
        if (k == 1) GetTempPathW(MAX_PATH, g_dir);
        make_stem();
        for (i = 0; i < 20; i++) {
            if (i == 0) _snwprintf(path, MAX_PATH + 16, L"%ls.dmp", g_stem);
            else _snwprintf(path, MAX_PATH + 16, L"%ls-%d.dmp", g_stem, i + 1);
            path[MAX_PATH + 15] = 0;
            g_dump_file = CreateFileW(path, GENERIC_WRITE | DELETE, FILE_SHARE_DELETE, NULL, CREATE_NEW,
                                      FILE_ATTRIBUTE_NORMAL, NULL);
            if (g_dump_file != INVALID_HANDLE_VALUE ||
                (GetLastError() != ERROR_FILE_EXISTS && GetLastError() != ERROR_ALREADY_EXISTS))
                break;
        }
    }
    WideCharToMultiByte(CP_UTF8, 0, path, -1, g_dump_name, sizeof g_dump_name, NULL, NULL);
    g_dump_name[sizeof g_dump_name - 1] = 0;
    /* the report next to it first (the dump may never finish) */
    {
        size_t n = wcslen(path);
        HANDLE txt = INVALID_HANDLE_VALUE;
        DWORD w;
        if (n > 4) {
            wcscpy(path + n - 4, L".txt");
            txt = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            wcscpy(path + n - 4, L".dmp");
            if (txt != INVALID_HANDLE_VALUE) WriteFile(txt, g_report, g_report_len, &w, NULL);
        }
        /* MiniDumpWriteDump suspends every other thread (the reporter too) and
         * allocates from the process heap: on a corrupt heap it can crash or
         * hang with them suspended.  HeapValidate first (it waits for the
         * heap's lock: one the crashed thread holds stops the dumper here,
         * before anything is suspended; the reporter's time limit goes on) */
        if (g_dump_file != INVALID_HANDLE_VALUE && !HeapValidate(GetProcessHeap(), 0, NULL)) {
            g_heap_bad = 1;
            set_delete(g_dump_file, TRUE);
            CloseHandle(g_dump_file);
            DeleteFileW(path);
        } else if (g_dump_file != INVALID_HANDLE_VALUE) {
            set_delete(g_dump_file, TRUE);
            if (write_dump(g_dump_file, g_ep, g_crash_tid)) {
                set_delete(g_dump_file, FALSE);
                g_dump_ok = 1;
            }
            CloseHandle(g_dump_file);
        }
        if (txt != INVALID_HANDLE_VALUE) {
            char line[MAX_PATH * 3 + 64];
            int len = host_snprintf(line, sizeof line, "minidump: %s\n",
                                    g_dump_ok   ? g_dump_name
                                    : g_heap_bad ? "not written: the process heap is corrupt (HeapValidate failed)"
                                                 : "not written");
            WriteFile(txt, line, (DWORD) len, &w, NULL);
            CloseHandle(txt);
        }
    }
    if (g_after != NULL) g_after();
    SetEvent(g_dump_done);
    return 0;
}

/* ---- the report ----------------------------------------------------------------- */
static const char *code_name(DWORD code) {
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: return "access violation";
        case EXCEPTION_STACK_OVERFLOW: return "stack overflow";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "integer division by zero";
        case EXCEPTION_INT_OVERFLOW: return "integer overflow";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "illegal instruction";
        case EXCEPTION_PRIV_INSTRUCTION: return "privileged instruction";
        case EXCEPTION_IN_PAGE_ERROR: return "in-page error";
        case EXCEPTION_BREAKPOINT: return "breakpoint";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "array bounds exceeded";
        case 0xC0000374: return "heap corruption";
        case 0xC0000409: return "stack buffer overrun / fail-fast";
        case 0xC000041D: return "exception in a user callback";
        case 0xE06D7363: return "unhandled C++ exception (MSVC-built code)";
        case HOST_CRASH_CODE: return "fatal internal error";
        default: return "exception";
    }
}

/* a file name in g_dir: bc-crash-YYYYMMDD-HHMMSS[-N].EXT; the .dmp is created
 * first (CREATE_NEW picks a free N), the .txt gets the same stem */
static void make_stem(void) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    _snwprintf(g_stem, MAX_PATH, L"%ls%lsbc-crash-%04u%02u%02u-%02u%02u%02u", g_dir,
               (g_dir[0] && g_dir[wcslen(g_dir) - 1] != L'\\' && g_dir[wcslen(g_dir) - 1] != L'/') ? L"\\" : L"",
               t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    g_stem[MAX_PATH - 1] = 0;
}

static void report(void) {
    EXCEPTION_POINTERS *ep = g_ep;
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *c = ep->ContextRecord;
    DWORD code = er->ExceptionCode, addr = (DWORD) er->ExceptionAddress;
    HMODULE m = module_of(addr);
    static DWORD frames[MAX_FRAMES];
    static char mod[128], dumpname[MAX_PATH * 3], msg[2048];
    int nframes, dump_ok = 0;
    g_err_handle = (HANDLE) _get_osfhandle(2);
    if (g_err_handle == INVALID_HANDLE_VALUE || g_err_handle == NULL) g_err_handle = GetStdHandle(STD_ERROR_HANDLE);
    host_trace_flush_raw();   /* the per-frame trace up to the crash */

    out("\nCRASH: %s (exception 0x%08lX) at 0x%08lX", code_name(code), code, addr);
    if (m != NULL) out(" = %s+0x%lX", module_name(m, mod, sizeof mod), addr - (DWORD) m);
    out("\n");
    if (g_reason != NULL) out("  %s\n", g_reason);
    if ((code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR) && er->NumberParameters >= 2)
        out("  %s of address 0x%08lX\n",
            er->ExceptionInformation[0] == 0 ? "read" : er->ExceptionInformation[0] == 8 ? "execution" : "write",
            (unsigned long) er->ExceptionInformation[1]);
    out("  eax=%08lX ebx=%08lX ecx=%08lX edx=%08lX esi=%08lX edi=%08lX\n  ebp=%08lX esp=%08lX eip=%08lX "
        "eflags=%08lX\n",
        c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp, c->Eip, c->EFlags);
    out("  host thread %lu (%s)", g_crash_tid, g_crash_tid == g_main_tid ? "main: the game's fibers" : "not the main thread");
    if (g_crash_tid == g_main_tid) out(", fiber %p", g_crash_fiber);
    out("; %s %s, exe %s\n", host_crash_tag, host_crash_version, host_crash_exe_kind);
    if (g_describe != NULL) {
        static char d[1024];
        d[0] = 0;
        g_describe(d, sizeof d);
        if (d[0]) out("  %s\n", d);
    }
    out("backtrace (return addresses found on the stack; ? = not confirmed by the call chain, may be stale):\n");
    nframes = backtrace(c, frames, MAX_FRAMES);
    print_frames(frames, nframes);

    /* the files (the .dmp, the .txt copy of the above) on the dumper thread,
     * with a time limit (heap and loader locks) */
    SetEvent(g_dump_go);
    if (WaitForSingleObject(g_dump_done, 20000) == WAIT_TIMEOUT) {
        out("minidump: not written: timed out after 20 s (the process heap is locked or corrupt)\n");
        if (g_dump_path[0]) DeleteFileW(g_dump_path);   /* (gone when the process ends) */
    } else if (g_heap_bad)
        out("minidump: not written: the process heap is corrupt (HeapValidate failed)\n");
    else if (g_dump_ok)
        out("minidump: %s\n", g_dump_name);
    else
        out("minidump: not written (%s; %s)\n", p_MiniDumpWriteDump == NULL ? "no dbghelp.dll"
                                                : g_dump_file == INVALID_HANDLE_VALUE ? "can't create the file"
                                                                                      : "MiniDumpWriteDump failed",
            g_dump_name);
    dump_ok = g_dump_ok;
    strcpy(dumpname, g_dump_name);
    if (host_gui && !host_no_msgbox) {
        host_snprintf(msg, sizeof msg,
                      "Blast Corps has crashed (%s, exception 0x%08lX at 0x%08lX%s%s).\n\n"
                      "The details were written to %s%s%s.\n\n"
                      "Please report it with %s and what you were doing.",
                      code_name(code), code, addr, m != NULL ? " in " : "", m != NULL ? mod : "",
                      host_log_path != NULL ? host_log_path : "the log",
                      dump_ok ? " and " : "", dump_ok ? dumpname : "", dump_ok ? "those files" : "that log");
        g_box = msg;
    }
}

static DWORD WINAPI reporter(void *arg) {
    (void) arg;
    WaitForSingleObject(g_go, INFINITE);
    report();
    if (g_box != NULL) {
        static wchar_t w[2048];
        /* (the game window's thread may be the crashed one: no synchronous
         * calls to it) */
        if (host_gui_window != NULL) ShowWindowAsync((HWND) host_gui_window, SW_HIDE);
        if (MultiByteToWideChar(CP_UTF8, 0, g_box, -1, w, 2048) <= 0)
            MultiByteToWideChar(CP_ACP, 0, g_box, -1, w, 2048);
        w[2047] = 0;
        g_boxing = 1;
        MessageBoxW(NULL, w, L"Blast Corps", MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
    }
    SetEvent(g_done);
    TerminateProcess(GetCurrentProcess(), CRASH_EXIT);
    return 0;
}

/* every crash path ends here (on the crashed thread) */
static LONG crash_dispatch(EXCEPTION_POINTERS *ep) {
    DWORD tid = GetCurrentThreadId();
    if (tid == g_dumper_tid && g_reporter_tid != 0) {
        /* MiniDumpWriteDump (or `after`) crashed: the reporter goes on without
         * it (resumed: the dump had suspended it) and ends the process */
        if (g_reporter_thread != NULL)
            while (ResumeThread(g_reporter_thread) > 1) {}
        SetEvent(g_dump_done);
        Sleep(INFINITE);
    }
    if (tid == g_reporter_tid) {
        /* the reporter itself crashed: stop now */
        static const char m[] = "\nCRASH: the crash reporter crashed too; exiting\n";
        DWORD w;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), m, sizeof m - 1, &w, NULL);
        TerminateProcess(GetCurrentProcess(), CRASH_EXIT);
    }
    if (InterlockedCompareExchange(&g_crashing, 1, 0) != 0) {
        /* another thread is reporting: wait for the end */
        Sleep(INFINITE);
    }
    g_ep = ep;
    g_crash_tid = tid;
    g_crash_fiber = tid == g_main_tid ? GetCurrentFiber() : NULL;
    if (g_go != NULL && g_done != NULL) {
        SetEvent(g_go);
        /* (a reporter stuck for 2 minutes ends the process; a box waits for the player) */
        if (WaitForSingleObject(g_done, 120000) == WAIT_TIMEOUT && g_boxing) WaitForSingleObject(g_done, INFINITE);
    }
    TerminateProcess(GetCurrentProcess(), CRASH_EXIT);
    return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI unhandled_filter(EXCEPTION_POINTERS *ep) {
    /* a debugger's breakpoint with no debugger attached is a crash too */
    return crash_dispatch(ep);
}

/* host_crash_now: a crash report from here (std::terminate, abort, ...) */
void host_crash_now(const char *why) {
    static CONTEXT c;
    static EXCEPTION_RECORD er;
    static EXCEPTION_POINTERS ep;
    memset(&c, 0, sizeof c);
    c.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&c);
    memset(&er, 0, sizeof er);
    er.ExceptionCode = HOST_CRASH_CODE;
    er.ExceptionAddress = (void *) c.Eip;
    ep.ExceptionRecord = &er;
    ep.ContextRecord = &c;
    g_reason = why;
    crash_dispatch(&ep);
    TerminateProcess(GetCurrentProcess(), CRASH_EXIT);
    for (;;) {}
}

static void sigabrt(int sig) {
    (void) sig;
    host_crash_now("abort() was called (a failed assertion or a C runtime error: see the lines above)");
}

/* ---- --no-msgbox: MessageBox* imported by a module only log their text ---------- */
static void log_box(const char *kind, const char *text) {
    fprintf(stderr, "message box suppressed (--no-msgbox; %s): %s\n", kind, text ? text : "");
    fflush(stderr);
}
static int box_answer(UINT type) {
    switch (type & 0xF) {
        case MB_OKCANCEL: case MB_RETRYCANCEL: case MB_YESNOCANCEL: case MB_CANCELTRYCONTINUE: return IDCANCEL;
        case MB_ABORTRETRYIGNORE: return IDABORT;
        case MB_YESNO: return IDNO;
        default: return IDOK;
    }
}
static char *narrow(const wchar_t *w) {
    static char buf[4096];
    if (w == NULL) return NULL;
    WideCharToMultiByte(CP_UTF8, 0, w, -1, buf, sizeof buf, NULL, NULL);
    buf[sizeof buf - 1] = 0;
    return buf;
}
static int WINAPI nobox_A(HWND h, LPCSTR t, LPCSTR c, UINT type) {
    (void) h, (void) c;
    log_box("MessageBoxA", t);
    return box_answer(type);
}
static int WINAPI nobox_W(HWND h, LPCWSTR t, LPCWSTR c, UINT type) {
    (void) h, (void) c;
    log_box("MessageBoxW", narrow(t));
    return box_answer(type);
}
static int WINAPI nobox_ExA(HWND h, LPCSTR t, LPCSTR c, UINT type, WORD l) {
    (void) h, (void) c, (void) l;
    log_box("MessageBoxExA", t);
    return box_answer(type);
}
static int WINAPI nobox_ExW(HWND h, LPCWSTR t, LPCWSTR c, UINT type, WORD l) {
    (void) h, (void) c, (void) l;
    log_box("MessageBoxExW", narrow(t));
    return box_answer(type);
}
static int WINAPI nobox_IndA(const MSGBOXPARAMSA *p) {
    log_box("MessageBoxIndirectA", p ? p->lpszText : NULL);
    return box_answer(p ? p->dwStyle : 0);
}
static int WINAPI nobox_IndW(const MSGBOXPARAMSW *p) {
    log_box("MessageBoxIndirectW", p ? narrow(p->lpszText) : NULL);
    return box_answer(p ? p->dwStyle : 0);
}

static int patch_iat(HMODULE m) {
    static const struct {
        const char *name;
        void *fn;
    } repl[] = {
        {"MessageBoxA", (void *) nobox_A},       {"MessageBoxW", (void *) nobox_W},
        {"MessageBoxExA", (void *) nobox_ExA},   {"MessageBoxExW", (void *) nobox_ExW},
        {"MessageBoxIndirectA", (void *) nobox_IndA}, {"MessageBoxIndirectW", (void *) nobox_IndW},
    };
    IMAGE_NT_HEADERS *nt = m ? nt_headers(m) : NULL;
    IMAGE_DATA_DIRECTORY *dd;
    IMAGE_IMPORT_DESCRIPTOR *d;
    int patched = 0;
    if (nt == NULL) return 0;
    dd = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dd->VirtualAddress == 0) return 0;
    for (d = (IMAGE_IMPORT_DESCRIPTOR *) ((BYTE *) m + dd->VirtualAddress); d->Name; d++) {
        IMAGE_THUNK_DATA *names, *iat;
        if (_stricmp((const char *) m + d->Name, "user32.dll") != 0 || d->OriginalFirstThunk == 0) continue;
        names = (IMAGE_THUNK_DATA *) ((BYTE *) m + d->OriginalFirstThunk);
        iat = (IMAGE_THUNK_DATA *) ((BYTE *) m + d->FirstThunk);
        for (; names->u1.AddressOfData; names++, iat++) {
            IMAGE_IMPORT_BY_NAME *ibn;
            unsigned k;
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            ibn = (IMAGE_IMPORT_BY_NAME *) ((BYTE *) m + names->u1.AddressOfData);
            for (k = 0; k < sizeof repl / sizeof repl[0]; k++) {
                DWORD old;
                if (strcmp((const char *) ibn->Name, repl[k].name) != 0) continue;
                if (VirtualProtect(&iat->u1.Function, sizeof(void *), PAGE_READWRITE, &old)) {
                    iat->u1.Function = (DWORD) repl[k].fn;
                    VirtualProtect(&iat->u1.Function, sizeof(void *), old, &old);
                    patched++;
                }
            }
        }
    }
    return patched;
}

/* ---- set-up --------------------------------------------------------------------- */

void host_crash_set_describe(void (*describe)(char *buf, unsigned size), void (*after)(void)) {
    g_describe = describe;
    g_after = after;
}

void host_crash_set_dir(const char *dir) {
    if (dir == NULL || !dir[0]) {
        g_dir[0] = 0;
        return;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, dir, -1, g_dir, MAX_PATH) <= 0) g_dir[0] = 0;
    g_dir[MAX_PATH - 1] = 0;
}

/* re-install the filter if a library replaced it (bc.exe: after RT64 starts) */
void host_crash_rearm(void) {
    LPTOP_LEVEL_EXCEPTION_FILTER prev = SetUnhandledExceptionFilter(unhandled_filter);
    if (prev != unhandled_filter) host_log("crash: the unhandled-exception filter had been replaced (%p); restored\n",
                                           (void *) prev);
}

void host_crash_init(int argc, char **argv, const char *exe_kind, const char *version) {
    HMODULE dbghelp;
    wchar_t sys[MAX_PATH];
    int i;
    g_main_tid = GetCurrentThreadId();
    host_crash_exe_kind = exe_kind;
    if (version != NULL) host_crash_version = version;
    GetModuleFileNameW(NULL, g_exe_path, MAX_PATH);
    g_exe_file = CreateFileW(g_exe_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                             OPEN_EXISTING, 0, NULL);
    if (getenv("BC_NO_MSGBOX") != NULL) host_no_msgbox = 1;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--no-msgbox")) host_no_msgbox = 1;
        else if (!strcmp(argv[i], "--crash-dir") && i + 1 < argc) host_crash_set_dir(argv[i + 1]);
    }
    /* the C runtime's assert/abort/R60xx messages: stderr, never a box */
    _set_error_mode(_OUT_TO_STDERR);
    signal(SIGABRT, sigabrt);
    if (host_no_msgbox) {
        typedef HRESULT(WINAPI * WerSetFlags_t)(DWORD);
        WerSetFlags_t wsf = (WerSetFlags_t) (void *) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "WerSetFlags");
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
        if (wsf != NULL) wsf(0x20 /* WER_FAULT_REPORTING_NO_UI */);
        patch_iat(GetModuleHandleW(NULL));
        patch_iat(GetModuleHandleW(L"SDL2.dll"));   /* bc.exe imports it: loaded */
    }
    /* dbghelp from System32 (not whatever sits next to the exe), loaded now:
     * nothing is loaded while reporting */
    if (GetSystemDirectoryW(sys, MAX_PATH) && wcslen(sys) + 14 < MAX_PATH) {
        wcscat(sys, L"\\dbghelp.dll");
        dbghelp = LoadLibraryW(sys);
        if (dbghelp != NULL)
            p_MiniDumpWriteDump = (MiniDumpWriteDump_t) (void *) GetProcAddress(dbghelp, "MiniDumpWriteDump");
    }
    p_GetMappedFileNameW =
        (GetMappedFileNameW_t) (void *) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "K32GetMappedFileNameW");
    g_go = CreateEventW(NULL, TRUE, FALSE, NULL);
    g_done = CreateEventW(NULL, TRUE, FALSE, NULL);
    g_dump_go = CreateEventW(NULL, TRUE, FALSE, NULL);
    g_dump_done = CreateEventW(NULL, TRUE, FALSE, NULL);
    g_reporter_thread = CreateThread(NULL, 0x40000, reporter, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, &g_reporter_tid);
    CloseHandle(CreateThread(NULL, 0x40000, dumper, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, &g_dumper_tid));
    g_prev_filter = SetUnhandledExceptionFilter(unhandled_filter);
    (void) g_prev_filter;
}

/* ---- --crash-test KIND[:FRAME] (tests of this file) ------------------------------- */
static char g_test_kind[16];
static unsigned g_test_frame;
void (*host_crash_test_cxx)(void);

int host_crash_test_set(const char *spec) {
    const char *colon = strchr(spec, ':');
    size_t n = colon ? (size_t) (colon - spec) : strlen(spec);
    if (n == 0 || n >= sizeof g_test_kind) return -1;
    memcpy(g_test_kind, spec, n);
    g_test_kind[n] = 0;
    g_test_frame = colon ? (unsigned) strtoul(colon + 1, NULL, 0) : 1;
    if (strcmp(g_test_kind, "av") && strcmp(g_test_kind, "thread") && strcmp(g_test_kind, "stack") &&
        strcmp(g_test_kind, "abort") && strcmp(g_test_kind, "fatal") && strcmp(g_test_kind, "box") &&
        strcmp(g_test_kind, "cxx") && strcmp(g_test_kind, "div") && strcmp(g_test_kind, "heaplock") &&
        strcmp(g_test_kind, "heapbad"))
        return -1;
    return 0;
}

static volatile int g_sink;
static volatile int *volatile g_bad = (volatile int *) 0x10;
static volatile int g_depth_limit = 0x7FFFFFFF;
static __attribute__((noinline)) int recurse(int n) {
    volatile char pad[4096];
    pad[0] = (char) n;
    if (n >= g_depth_limit) return 0;
    return recurse(n + 1) + pad[0];
}
static DWORD WINAPI test_thread(void *arg) {
    (void) arg;
    *g_bad = 1;
    return 0;
}

void host_crash_test_frame(unsigned frame) {
    if (g_test_kind[0] == 0 || frame != g_test_frame) return;
    fprintf(stderr, "crash test: %s at frame %u\n", g_test_kind, frame);
    fflush(stderr);
    if (!strcmp(g_test_kind, "av")) g_sink = *g_bad;
    else if (!strcmp(g_test_kind, "heaplock")) {   /* a crash inside the heap: its lock stays held */
        HeapLock(GetProcessHeap());
        g_sink = *g_bad;
    } else if (!strcmp(g_test_kind, "heapbad")) {   /* a corrupt heap block header, then a fault */
        unsigned char *p = HeapAlloc(GetProcessHeap(), 0, 64);
        memset(p - 8, 0x41, 8);
        g_sink = *g_bad;
    }
    else if (!strcmp(g_test_kind, "div")) {
        volatile int z = 0;
        g_sink = (g_sink + 7) / z;
    } else if (!strcmp(g_test_kind, "stack")) g_sink = recurse(0);
    else if (!strcmp(g_test_kind, "abort")) abort();
    else if (!strcmp(g_test_kind, "fatal")) host_fatal("crash test: a fatal error");
    else if (!strcmp(g_test_kind, "box")) host_message("crash test: a message box", 0);
    else if (!strcmp(g_test_kind, "thread")) {
        WaitForSingleObject(CreateThread(NULL, 0, test_thread, NULL, 0, NULL), INFINITE);
    } else if (!strcmp(g_test_kind, "cxx")) {
        if (host_crash_test_cxx != NULL) host_crash_test_cxx();
        host_fatal("crash test: cxx needs bc.exe");
    }
}
