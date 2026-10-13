/* bc.exe: the windowed build.  The game logic and the platform layer are
 * bc_headless's (port/src/platform/); this file adds what a player sees and
 * does (HostLive, plat_host.h):
 *  - a window (SDL2) and the RT64 renderer (port/rt64/: RT64 built with
 *    RT64_NATIVE_HOST_LAYOUT, RDRAM = this process's memory at 0x80000000);
 *  - every graphics task the game starts goes to RT64 (loadUCodeGBI +
 *    processDisplayLists); cull_rsp_test's visibility test stays with the
 *    platform (answered "visible");
 *  - every retrace: the VI registers of the frame on screen, RT64's
 *    updateScreen, and real-time pacing (60 retraces per second of virtual
 *    time; the game logic steps exactly as in bc_headless);
 *  - controller 1 from the keyboard or an SDL game controller;
 *  - screenshots (PNG, read back from RDRAM after RT64 wrote the frame).
 *
 *   bc.exe ROM [bc_headless options] [--api d3d12|vulkan] [--shot F1,F2,...]
 *          [--shot-dir DIR] [--shot-every N] [--no-pace] [--scale N]
 *          [--dl-dump FRAME[:TASKS]] [--no-gfx-fix] [--gfx-fix-log]
 *     --shot F,..       save the picture on screen once the game reached frame F
 *                       (bc_FFFFFFF.png in --shot-dir; --shot-every N: every N frames)
 *     --no-pace         run as fast as possible (default: 60 retraces a second)
 *     --scale N         window size 320x240 times N (default 3; bc.ini scale)
 *     --dl-dump F[:N]   print N tasks' display lists from frame F (default 12)
 *     --dl-dump-every N print the first task's display lists every N frames
 *     --dl-skip LO:HI   draw triangles whose commands lie in [LO, HI) (physical)
 *                       as no-ops (find which list draws something)
 *     --no-gfx-fix      don't convert graphics data in display-list areas
 *                       (port/src/load/gfx_fix.c; to see what it does)
 *     --gfx-fix-log     log graphics data the game changed in those areas
 *     --mute            start with the sound off (M toggles it)
 *     --volume N        sound volume in percent (default 100; - and = keys step it)
 *     --fullscreen / --windowed, --vsync / --no-vsync
 *     --saves DIR       keep the EEPROM and Controller Pak files in DIR
 *     --no-saves        keep no save files (the EEPROM starts blank, no pak)
 *     --no-pak          no Controller Pak (bc.ini controller_pak = 0)
 *     --config FILE     settings file (default bc.ini next to the exe)
 *     --no-config       no settings file: built-in defaults, no saves unless
 *                       --eeprom/--mpk/--saves (tests and comparisons use this,
 *                       so bc.exe runs like bc_headless with the same options)
 *     --no-rom-check    accept any Blast Corps-layout ROM (e.g. the NON_MATCHING
 *                       test ROM) instead of only the US v1.1 release
 *     --pace-log FILE   per retrace: pacing and present times (CSV), and a
 *                       summary in the log at exit
 *     --set S.K=V       a bc.ini setting for this run ([S] K = V; not saved), e.g.
 *                       --set audio.volume=50
 *     --resolution R    internal resolution: window (default), 1 (320x240), 2, 3, 4 ...
 *     --msaa N          multisample anti-aliasing: 0 (default), 2, 4, 8
 *     --filtering F     the picture's scaling: aa (default), linear, nearest
 *     --aspect A        widescreen: original (4:3, default), 16:9, 16:10, 21:9, W:H, window
 *     --framerate F     pictures a second: original (the game's 20/30, default), display, or N
 *                       (RT64 interpolates in-between pictures; the game runs as before)
 *     --interpolation M tagged (default: objects paired by the game's tags, live_interp.cpp)
 *                       or auto (RT64 pairs them itself); only with --framerate above original
 *     --fps-counter M   the frame-rate counter: off (default), on, detailed (F2 switches it)
 *     --interp-log      log every frame's camera motion (the interpolation cut test) and
 *                       tagged objects that jumped
 *     --tag-test MASK   (testing) tagged interpolation experiments: 1 no vertex blending, 2 HUD
 *                       matched like the rest, 4 no decomposition, 8 ORDER_AUTO, 16 every ID auto
 *     --display-hz N    (testing) tell RT64 the display runs at N Hz (its cap on --framerate)
 *     --wseq F:MS       (testing) from frame F, every new picture the window shows for MS ms
 *                       (seq_F_NN.png + seq_F.txt with the times, in --shot-dir)
 *     --texture-pack P  use this RT64 texture pack (folder with rt64.json, or .rtz)
 *     --texture-hashes F  log the RT64 hashes of the textures loaded at frame F
 *     --window WxH      the window's size (desktop points) instead of 320x240 x scale
 *     --wshot F,..      save the window's picture (RT64's output at the window's size,
 *                       win_FFFFFFF.png in --shot-dir) once the game reached frame F
 *     --menu-shot F     open the settings menu at frame F, save its picture
 *                       (menu_FFFFFFF.png) and close it again (testing)
 *     --menu-set S.K=V  in that menu, change a setting as a player would (applied
 *                       at once, written to bc.ini when the menu closes)
 *     --no-msgbox       (headless_main.c) no dialog of any kind: errors, crash
 *                       reports, RT64/SDL boxes only logged; no ROM file dialog
 *
 * Crashes (port/src/platform/crash.c): a report in the log and in
 * bc-crash-*.txt, a minidump bc-crash-*.dmp next to bc.log, a message box.
 *
 * Player setup (live_setup.cpp): bc.ini next to the exe holds the ROM path,
 * graphics API, window, sound, key and controller bindings and the saves
 * folder (created with commented defaults on the first start; command-line
 * options override it).  Without a ROM argument and a working configured
 * ROM, a file dialog asks for one; the ROM is checked (Blast Corps (USA)
 * (Rev 1) by SHA-1, .z64/.v64/.n64) and remembered.  The settings menu
 * (live_menu.cpp; F1, or Back+Start on a controller) changes the settings
 * while the game is paused and writes them to bc.ini.  bc.exe is a Windows
 * GUI program: errors show a message box; the log goes to stderr when it
 * has one, else to bc.log next to the exe.
 *
 * Sound: the buffers the game hands the AI (port/src/audio/) are queued to
 * an SDL audio device at the AI's rate.  The game makes them in virtual time,
 * which the pacing keeps on real time; a slight resampling (at most 0.5%)
 * holds the device queue near 60 ms against clock drift, and a queue that
 * grew past 0.3 s (after a stall) is dropped.  With --no-pace nothing plays.
 */
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <io.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <SDL.h>
#include <SDL_syswm.h>

#include "hle/rt64_application.h"
#include "common/rt64_tmem_hasher.h"
#include "render/rt64_render_target.h"
#include "render/rt64_texture_cache.h"
#include "rhi/rt64_render_hooks.h"

#include "live_input.h"
#include "live_interp.h"
#include "live_menu.h"
#include "live_setup.h"

extern "C" {
#include "plat_host.h"
#include "rdram.h"
#include "load/port_load.h"
unsigned plat_frames(void); /* save.c */
}

#ifndef BC_VERSION
#define BC_VERSION "dev"
#endif

namespace {

/* ---- options ------------------------------------------------------------- */
struct LiveOpts {
    bool vulkan = false;
    bool pace = true;
    unsigned scale = 3;              /* window size: 320x240 times this */
    bool fullscreen = false;
    bool vsync = true;
    std::vector<unsigned> shots;     /* frame numbers (game frames) to save */
    unsigned shotEvery = 0;
    const char *shotDir = ".";
    unsigned dlDumpFrame = ~0u, dlDumpTasks = 12, dlDumpEvery = 0;
    bool gfxFix = true;
    bool mute = false;
    int volume = 100;                /* percent */
    bool romCheck = true;
    const char *paceLog = nullptr;
    unsigned windowW = 0, windowH = 0;   /* --window WxH: the window's size in desktop points */
    std::vector<unsigned> wshots;        /* --wshot: frames whose window picture is saved */
    unsigned menuShot = ~0u;             /* --menu-shot F: open the menu at frame F, save its picture, close it */
    std::vector<std::string> menuSets;   /* --menu-set S.K=V: changed in that menu (as a player would) */
};
LiveOpts g_opt;
live::Config g_cfg;                  /* bc.ini (or the built-in defaults) */
std::string g_eepromPath, g_mpkPath; /* the saves the config names */

/* ---- RT64 state ------------------------------------------------------------ */
SDL_Window *g_window;
RT64::Application *g_app;
uint8_t g_dmem[0x1000], g_imem[0x1000], g_header[0x40];
uint32_t MI_INTR_REG, DPC_START_REG, DPC_END_REG, DPC_CURRENT_REG, DPC_STATUS_REG, DPC_CLOCK_REG, DPC_BUFBUSY_REG,
    DPC_PIPEBUSY_REG, DPC_TMEM_REG;
uint32_t VI_STATUS_REG, VI_ORIGIN_REG, VI_WIDTH_REG, VI_INTR_REG, VI_V_CURRENT_LINE_REG, VI_TIMING_REG, VI_V_SYNC_REG,
    VI_H_SYNC_REG, VI_LEAP_REG, VI_H_START_REG, VI_V_START_REG, VI_V_BURST_REG, VI_X_SCALE_REG, VI_Y_SCALE_REG;
unsigned g_spIntr, g_dpIntr, g_tasks, g_unknownUcode, g_fixedBytes, g_otherUcodeTasks;
double g_dlSeconds;                  /* time in processDisplayLists (frame-rate option on) */
unsigned g_dlTasks;

/* RT64 raises the RSP/RDP interrupts as it finishes a task (during
 * processDisplayLists).  The game hears about task completion from the
 * platform's virtual-time model instead (os_hw.c), so that it steps exactly
 * as in bc_headless; these only count. */
void checkInterrupts() {
    if (MI_INTR_REG & 0x01) g_spIntr++;
    if (MI_INTR_REG & 0x20) g_dpIntr++;
    MI_INTR_REG = 0;
}

uint8_t *rdramBase() {
    return reinterpret_cast<uint8_t *>(uintptr_t(RDRAM_BASE));
}

/* ---- microcode bytes ---------------------------------------------------------
 * RT64 identifies a ucode by hashing its text and data.  Natively the game
 * keeps neither where the RSP would find them: hd_code's text (holding the F3D
 * ucode) is not loaded, and the data images were swapped by type.  Put the
 * ROM's big-endian bytes back at their N64 addresses (nothing native reads
 * them).  The front end's ucode text is inflated with the front end, as
 * big-endian bytes, every time the game loads it. */
struct UcodePiece {
    uint32_t addr, size;
    int image;      /* 0: hd_code text, 1: hd_code data, 2: front-end data */
};
const UcodePiece kUcode[] = {
    {0x802E53F0u, 0x1430, 0},  /* D_802E53F0: hd F3D text (slots 1-3) */
    {0x8030E390u, 0x0800, 1},  /* D_8030E390: its data */
    {0x80210690u, 0x0800, 2},  /* D_80210690: front-end L3D data */
};
constexpr uint32_t HD_TEXT_VRAM = 0x802447C0u;

void stageUcode() {
    struct Image { uint32_t rom, vram, size; std::vector<uint8_t> bytes; };
    Image img[3] = {
        {ROM_HD_TEXT, HD_TEXT_VRAM, HD_DATA_VRAM - HD_TEXT_VRAM, {}},
        {ROM_HD_DATA, HD_DATA_VRAM, HD_DATA_SIZE, {}},
        {ROM_FE_DATA, FE_DATA_VRAM, FE_BSS_END - FE_DATA_VRAM, {}},
    };
    for (Image &i : img) {
        i.bytes.resize(i.size);
        if (rom_gunzip(i.rom, i.bytes.data(), i.size) <= 0) host_fatal("live: can't inflate ROM 0x%X", (unsigned) i.rom);
    }
    for (const UcodePiece &u : kUcode) {
        const Image &i = img[u.image];
        uint8_t *dst = reinterpret_cast<uint8_t *>(uintptr_t(u.addr));
        if (u.image == 0) {
            for (uint32_t k = 0; k < u.size; k++)
                if (dst[k] != 0) {
                    host_log("live: WARNING: 0x%08X+0x%X (ucode text) is in use natively; overwriting\n", (unsigned) u.addr,
                             (unsigned) k);
                    break;
                }
        }
        memcpy(dst, &i.bytes[u.addr - i.vram], u.size);
    }
}

/* ---- screenshots: RGBA16 frame from RDRAM -> PNG --------------------------- */
uint32_t crcTable[256];
uint32_t crc32(uint32_t crc, const uint8_t *p, size_t n) {
    if (crcTable[1] == 0)
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            crcTable[i] = c;
        }
    crc = ~crc;
    while (n--) crc = crcTable[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

void be32(std::vector<uint8_t> &v, uint32_t x) {
    v.push_back(uint8_t(x >> 24));
    v.push_back(uint8_t(x >> 16));
    v.push_back(uint8_t(x >> 8));
    v.push_back(uint8_t(x));
}

void pngChunk(std::vector<uint8_t> &out, const char *type, const std::vector<uint8_t> &data) {
    be32(out, uint32_t(data.size()));
    size_t start = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), data.begin(), data.end());
    be32(out, crc32(0, &out[start], out.size() - start));
}

/* RGB, stored (uncompressed) deflate blocks */
bool writePng(const char *path, unsigned w, unsigned h, const std::vector<uint8_t> &rgb) {
    std::vector<uint8_t> out = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    std::vector<uint8_t> ihdr;
    be32(ihdr, w);
    be32(ihdr, h);
    ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0});
    pngChunk(out, "IHDR", ihdr);
    std::vector<uint8_t> raw;
    for (unsigned y = 0; y < h; y++) {
        raw.push_back(0);
        raw.insert(raw.end(), rgb.begin() + y * w * 3, rgb.begin() + (y + 1) * w * 3);
    }
    std::vector<uint8_t> z = {0x78, 0x01};
    uint32_t a = 1, b = 0;
    for (uint8_t c : raw) a = (a + c) % 65521, b = (b + a) % 65521;
    for (size_t off = 0; off < raw.size() || off == 0;) {
        size_t n = std::min<size_t>(65535, raw.size() - off);
        bool last = off + n >= raw.size();
        z.push_back(last ? 1 : 0);
        z.push_back(uint8_t(n));
        z.push_back(uint8_t(n >> 8));
        z.push_back(uint8_t(~n));
        z.push_back(uint8_t(~n >> 8));
        z.insert(z.end(), raw.begin() + off, raw.begin() + off + n);
        off += n;
        if (last) break;
    }
    be32(z, (b << 16) | a);
    pngChunk(out, "IDAT", z);
    pngChunk(out, "IEND", {});
    return host_write_file(path, out.data(), unsigned(out.size())) == 0;
}

/* the picture the VI shows: origin, width, and the line count from vStart and yScale */
void saveFrame(const HostViRegs &r, unsigned frame) {
    unsigned w = r.width, vs = r.v_start >> 16, ve = r.v_start & 0x3FF;
    unsigned h = ve > vs ? (ve - vs) / 2 * (r.y_scale & 0xFFF) / 0x400 : 0;
    if (w == 0 || h == 0 || w > 1024 || h > 1024 || (r.status & 3) != 2) {
        host_log("live: frame %u: no 16-bit picture on screen (status %X width %u)\n", frame, r.status, w);
        return;
    }
    if (h > 240) h = 240;
    std::vector<uint8_t> rgb(w * h * 3);
    const uint8_t *fb = rdramBase() + (r.origin & 0x7FFFFF);
    for (unsigned i = 0; i < w * h; i++) {
        unsigned p = (fb[i * 2] << 8) | fb[i * 2 + 1];   /* big-endian RGBA5551 */
        rgb[i * 3 + 0] = uint8_t(((p >> 11) & 31) * 255 / 31);
        rgb[i * 3 + 1] = uint8_t(((p >> 6) & 31) * 255 / 31);
        rgb[i * 3 + 2] = uint8_t(((p >> 1) & 31) * 255 / 31);
    }
    char path[512];
    snprintf(path, sizeof path, "%s/bc_%07u.png", g_opt.shotDir, frame);
    if (writePng(path, w, h, rgb))
        host_log("live: saved %s (%ux%u, fb 0x%08X; %u tasks drawn, %u not F3D (L3D), %u bytes converted)\n", path,
                 w, h, r.origin, g_tasks, g_otherUcodeTasks, g_fixedBytes);
}

/* the window's picture as presented (RT64's output at the window's size:
 * internal resolution, widescreen, the settings menu): --wshot, --menu-shot.
 * Waits until RT64 presented what it was given, then asks the compositor for
 * the client area (PrintWindow with PW_RENDERFULLCONTENT, which also works
 * for a flip-model swap chain). */
void saveWindow(const char *name) {
    if (g_app == nullptr || host_gui_window == nullptr) return;
    g_app->presentQueue->waitForPresentId(g_app->state->presentId);
    Sleep(120);
    HWND hw = HWND(host_gui_window);
    RECT rc;
    GetClientRect(hw, &rc);
    const int w = rc.right - rc.left, h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return;
    HDC wdc = GetDC(hw), mdc = CreateCompatibleDC(wdc);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void *bits = nullptr;
    HBITMAP bmp = CreateDIBSection(wdc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mdc, bmp);
    const BOOL ok = PrintWindow(hw, mdc, PW_CLIENTONLY | 0x2 /* PW_RENDERFULLCONTENT */);
    std::vector<uint8_t> rgb(size_t(w) * h * 3);
    const uint8_t *p = static_cast<const uint8_t *>(bits);
    for (size_t i = 0; i < size_t(w) * h; i++) {
        rgb[i * 3 + 0] = p[i * 4 + 2];
        rgb[i * 3 + 1] = p[i * 4 + 1];
        rgb[i * 3 + 2] = p[i * 4 + 0];
    }
    SelectObject(mdc, old);
    DeleteObject(bmp);
    DeleteDC(mdc);
    ReleaseDC(hw, wdc);
    char path[512];
    snprintf(path, sizeof path, "%s/%s.png", g_opt.shotDir, name);
    if (ok && writePng(path, unsigned(w), unsigned(h), rgb)) host_log("live: saved %s (%dx%d window)\n", path, w, h);
    else host_log("live: window picture %s failed\n", path);
}

/* --wseq F:MS (testing the frame rate option): from game frame F, a thread
 * takes the window's picture every 2 ms for MS milliseconds and keeps each
 * new one (a picture differing from the one before), so every picture the
 * display got (interpolated ones too) is saved: seq_FFFFFFF_NN.png in
 * --shot-dir, with the time it was first seen in seq_FFFFFFF.txt */
struct WinSeq {
    unsigned frame = ~0u, ms = 0;
    std::thread thread;
    bool started = false;
} g_wseq;

bool grabWindow(std::vector<uint8_t> &rgb, int &w, int &h) {
    HWND hw = HWND(host_gui_window);
    RECT rc;
    GetClientRect(hw, &rc);
    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return false;
    /* from the screen (the composed desktop: fast enough for every picture
     * at 60 a second, unlike PrintWindow; the window must be uncovered) */
    POINT org = {0, 0};
    ClientToScreen(hw, &org);
    HDC wdc = GetDC(nullptr), mdc = CreateCompatibleDC(wdc);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void *bits = nullptr;
    HBITMAP bmp = CreateDIBSection(wdc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mdc, bmp);
    const BOOL ok = BitBlt(mdc, 0, 0, w, h, wdc, org.x, org.y, SRCCOPY);
    rgb.resize(size_t(w) * h * 3);
    const uint8_t *p = static_cast<const uint8_t *>(bits);
    for (size_t i = 0; i < size_t(w) * h; i++) {
        rgb[i * 3 + 0] = p[i * 4 + 2];
        rgb[i * 3 + 1] = p[i * 4 + 1];
        rgb[i * 3 + 2] = p[i * 4 + 0];
    }
    SelectObject(mdc, old);
    DeleteObject(bmp);
    DeleteDC(mdc);
    ReleaseDC(nullptr, wdc);
    return ok != 0;
}

void winSeqThread(unsigned frame, unsigned ms) {
    struct Pic { double t; std::vector<uint8_t> rgb; };
    std::vector<Pic> pics;
    int w = 0, h = 0;
    LARGE_INTEGER t0, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);
    auto msNow = [&]() {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return double(now.QuadPart - t0.QuadPart) * 1000.0 / double(freq.QuadPart);
    };
    std::vector<uint8_t> cur;
    while (msNow() < ms && pics.size() < 200) {
        int cw, ch;
        if (grabWindow(cur, cw, ch) && (pics.empty() || cur != pics.back().rgb)) {
            w = cw, h = ch;
            pics.push_back({msNow(), cur});
        }
        Sleep(1);
    }
    char path[512];
    snprintf(path, sizeof path, "%s/seq_%07u.txt", g_opt.shotDir, frame);
    std::string times;
    for (size_t i = 0; i < pics.size(); i++) {
        char name[512];
        snprintf(name, sizeof name, "%s/seq_%07u_%02u.png", g_opt.shotDir, frame, unsigned(i));
        if (pics[i].rgb.size() == size_t(w) * h * 3) writePng(name, unsigned(w), unsigned(h), pics[i].rgb);
        char line[64];
        snprintf(line, sizeof line, "%02u %.2f\n", unsigned(i), pics[i].t);
        times += line;
    }
    host_write_file(path, times.data(), unsigned(times.size()));
    host_log("live: --wseq: %u different pictures in %u ms from frame %u\n", unsigned(pics.size()), ms, frame);
}

/* ---- input ------------------------------------------------------------------- */
/* the controllers and the N64 pad: live_input.cpp.  bc.ini's controller.pad
 * picks which controller plays (default the first) or all of them. */
live::PadSet g_pads;
unsigned short g_button;
signed char g_stickX, g_stickY;
live::InputGate g_gate;              /* after the settings menu (live_input.h) */
bool g_focused = true;               /* the window has the keyboard focus */
unsigned g_lastScan;                 /* retrace of the last controller rescan */

std::vector<std::string> padNames() {
    return g_pads.names();
}

/* N64 pad from the keyboard and the game controller(s), by the bindings
 * in bc.ini (live_setup.cpp has the defaults):
 *   stick: arrows (keyboard), left stick       A: X / gamepad A
 *   B: C / gamepad B or X                      Z: Z or Space / either trigger
 *   START: Enter / Start                       L, R: A, S / shoulders
 *   C buttons: I J K L / right stick           D-pad: T F G H / D-pad */
void pollInput() {
    live::N64Pad p = live::readPad(g_cfg, g_pads, SDL_GetKeyboardState(nullptr), (SDL_GetModState() & KMOD_ALT) != 0);
    g_gate.apply(p);
    g_button = p.button;
    g_stickX = (signed char) p.x;
    g_stickY = (signed char) p.y;
}

bool menuCombo() {
    return live::menuComboHeld(g_cfg, g_pads);
}

/* ---- sound ------------------------------------------------------------------- */
SDL_AudioDeviceID g_audioDev;
unsigned g_audioRate, g_audioDrops, g_audioResets;
bool g_audioFailed;
double g_audioPos = -1.0;            /* resampler position in the next buffer (-1: the previous last frame) */
int16_t g_audioPrev[2];
std::vector<int16_t> g_audioOut;

bool openAudio(unsigned rate) {
    if (g_audioDev != 0 && rate == g_audioRate) return true;
    if (g_audioFailed) return false;
    if (g_audioDev != 0) SDL_CloseAudioDevice(g_audioDev);
    if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        host_log("live: no sound (SDL audio: %s)\n", SDL_GetError());
        g_audioFailed = true;
        return false;
    }
    SDL_AudioSpec want = {}, have = {};
    want.freq = int(rate);
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 512;
    g_audioDev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (g_audioDev == 0) {
        host_log("live: no sound (SDL_OpenAudioDevice: %s)\n", SDL_GetError());
        g_audioFailed = true;
        return false;
    }
    g_audioRate = rate;
    SDL_PauseAudioDevice(g_audioDev, 0);
    host_log("live: sound at %u Hz (%s)\n", rate, SDL_GetCurrentAudioDriver());
    return true;
}

void liveAudio(const short *lr, unsigned frames, unsigned rate) {
    if (!g_opt.pace || frames == 0 || !openAudio(rate)) return;
    const double target = rate * 0.060;   /* queue length held near 60 ms */
    double queued = double(SDL_GetQueuedAudioSize(g_audioDev)) / 4.0;
    if (queued > rate * 0.3) {            /* far behind real time (a stall): start over */
        SDL_ClearQueuedAudio(g_audioDev);
        g_audioResets++;
        queued = 0;
    }
    /* step through the input faster when the queue is long, slower when short */
    double step = 1.0 + std::max(-0.005, std::min(0.005, (queued - target) / target * 0.005));
    static unsigned calls;
    if (host_verbose && ++calls % 500 == 0)
        host_log("live: sound queue %.0f ms, step %.4f, resets %u (frame %u)\n", queued * 1000.0 / rate, step,
                 g_audioResets, plat_frames());
    const float gain = g_opt.mute ? 0.0f : float(g_opt.volume) / 100.0f;
    g_audioOut.clear();
    double p = g_audioPos;
    while (p < double(frames - 1)) {
        long i = long(std::floor(p));
        double f = p - double(i);
        for (int c = 0; c < 2; c++) {
            double a = i < 0 ? g_audioPrev[c] : lr[i * 2 + c];
            double b = lr[(i + 1) * 2 + c];
            double v = (a + (b - a) * f) * gain;
            g_audioOut.push_back(int16_t(std::max(-32768.0, std::min(32767.0, v))));
        }
        p += step;
    }
    g_audioPos = p - double(frames);
    g_audioPrev[0] = lr[(frames - 1) * 2];
    g_audioPrev[1] = lr[(frames - 1) * 2 + 1];
    if (!g_audioOut.empty() && SDL_QueueAudio(g_audioDev, g_audioOut.data(), Uint32(g_audioOut.size() * 2)) != 0)
        g_audioDrops++;
}

/* ---- pacing measurement (--pace-log) ---------------------------------------- */
struct PaceRec {
    double t;        /* real time when the retrace's picture went to RT64 (s) */
    double due;      /* its due time on the paced clock (s) */
    double update;   /* time spent in updateScreen (s) */
    unsigned frames; /* game frames so far */
};
std::vector<PaceRec> g_pace;
constexpr size_t kMaxPresents = 1 << 20;
double *g_presentTimes;              /* RT64's present thread: each picture presented */
std::atomic<size_t> g_presents;
unsigned g_paceResets;
LARGE_INTEGER g_freq, g_t0, g_tStart;

double secondsSince(const LARGE_INTEGER &t0) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return double(now.QuadPart - t0.QuadPart) / double(g_freq.QuadPart);
}

void presentHook(plume::RenderCommandList *, plume::RenderFramebuffer *) {
    size_t i = g_presents.fetch_add(1);
    if (i < kMaxPresents) g_presentTimes[i] = secondsSince(g_tStart);
}

/* ---- the frame-rate counter (video.fps_counter, F2) --------------------------
 * Presented pictures are counted on RT64's present thread (drawHook), the
 * game's frames from the platform's frame count; twice a second the numbers
 * go to the overlay (live_menu.cpp), which draws them in a corner.  Off,
 * nothing of this runs.  It only reads clocks: the game never sees it. */
struct Counter {
    std::atomic<bool> on{false};
    std::mutex mutex;                 /* the present times (present thread -> main thread) */
    std::vector<double> times;        /* present times (s) since the last update */
    std::vector<float> graph;         /* the last present intervals (ms) */
    double lastPresent = 0, lastUpdate = -1;
    unsigned lastFrames = 0;
    live::CounterStats stats;
} g_counter;

void counterPresent() {
    const double t = secondsSince(g_tStart);
    std::lock_guard<std::mutex> lock(g_counter.mutex);
    g_counter.times.push_back(t);
}

void counterApply() {
    const bool on = g_cfg.fpsCounter != 0;
    if (on != g_counter.on.load()) {
        g_counter.on = on;
        std::lock_guard<std::mutex> lock(g_counter.mutex);
        g_counter.times.clear();
        g_counter.graph.clear();
        g_counter.lastUpdate = -1;
    }
    g_counter.stats.mode = g_cfg.fpsCounter;
    g_counter.stats.corner = g_cfg.fpsCorner;
    live::counterSet(g_counter.stats);
}

/* each retrace (main thread) */
void counterTick(unsigned frames) {
    if (!g_counter.on) return;
    const double now = secondsSince(g_tStart);
    if (g_counter.lastUpdate < 0) {
        g_counter.lastUpdate = now;
        g_counter.lastFrames = frames;
    } else if (now - g_counter.lastUpdate >= 0.5) {
        std::vector<double> t;
        {
            std::lock_guard<std::mutex> lock(g_counter.mutex);
            t.swap(g_counter.times);
        }
        const double dt = now - g_counter.lastUpdate;
        live::CounterStats &s = g_counter.stats;
        s.shown = double(t.size()) / dt;
        s.game = double(frames - g_counter.lastFrames) / dt;
        double sum = 0, mx = 0;
        unsigned n = 0;
        double prev = g_counter.lastPresent;
        for (double x : t) {
            if (prev > 0) {
                const double ms = (x - prev) * 1e3;
                sum += ms;
                mx = std::max(mx, ms);
                n++;
                g_counter.graph.push_back(float(ms));
            }
            prev = x;
        }
        if (!t.empty()) g_counter.lastPresent = t.back();
        if (g_counter.graph.size() > 120) g_counter.graph.erase(g_counter.graph.begin(), g_counter.graph.end() - 120);
        s.frameMs = n != 0 ? sum / n : 0;
        s.frameMsMax = mx;
        s.graph = g_counter.graph;
        s.note = g_cfg.framerate == 0 ? "original frame rate"
                 : live::interpTagsOn() ? "interpolated, tagged" : "interpolated, auto";
        g_counter.lastUpdate = now;
        g_counter.lastFrames = frames;
        live::counterSet(s);
    }
    live::counterFrame();
}

/* the counter key / controller buttons: off -> on -> detailed -> off (for
 * this run; the menu saves it) */
void counterCycle() {
    g_cfg.fpsCounter = (g_cfg.fpsCounter + 1) % 3;
    host_log("live: frame-rate counter %s\n", live::fpsCounterName(g_cfg.fpsCounter).c_str());
    counterApply();
}

unsigned displayHz() {
    HMONITOR mon = MonitorFromWindow(HWND(host_gui_window), MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof mi;
    DEVMODEW dm = {};
    dm.dmSize = sizeof dm;
    if (!GetMonitorInfoW(mon, &mi) || !EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm)) return 0;
    return dm.dmDisplayFrequency;
}

struct Stats {
    double mean = 0, sd = 0, p01 = 0, p99 = 0, min = 0, max = 0;
    size_t n = 0;
};
Stats stats(std::vector<double> v) {
    Stats s;
    s.n = v.size();
    if (v.empty()) return s;
    std::sort(v.begin(), v.end());
    double sum = 0, sq = 0;
    for (double x : v) sum += x;
    s.mean = sum / double(v.size());
    for (double x : v) sq += (x - s.mean) * (x - s.mean);
    s.sd = std::sqrt(sq / double(v.size()));
    s.min = v.front();
    s.max = v.back();
    s.p01 = v[v.size() / 100];
    s.p99 = v[v.size() - 1 - v.size() / 100];
    return s;
}

void writePaceLog() {
    if (g_opt.paceLog == nullptr || g_pace.size() < 2) return;
    FILE *f = static_cast<FILE *>(host_fopen(g_opt.paceLog, "w"));
    if (f != nullptr) {
        fprintf(f, "kind,index,t_ms,due_ms,update_ms,frames\n");
        for (size_t i = 0; i < g_pace.size(); i++)
            fprintf(f, "vi,%u,%.3f,%.3f,%.3f,%u\n", unsigned(i), g_pace[i].t * 1e3, g_pace[i].due * 1e3,
                    g_pace[i].update * 1e3, g_pace[i].frames);
        size_t np = std::min(g_presents.load(), kMaxPresents);
        for (size_t i = 0; i < np; i++) fprintf(f, "present,%u,%.3f,,,\n", unsigned(i), g_presentTimes[i] * 1e3);
        fclose(f);
    }
    /* skip the first second (start-up, shader compiles) */
    std::vector<double> vi, upd, pr;
    double t0 = g_pace.front().t + 1.0;
    for (size_t i = 1; i < g_pace.size(); i++) {
        if (g_pace[i].t < t0) continue;
        vi.push_back((g_pace[i].t - g_pace[i - 1].t) * 1e3);
        upd.push_back(g_pace[i].update * 1e3);
    }
    size_t np = std::min(g_presents.load(), kMaxPresents);
    for (size_t i = 1; i < np; i++)
        if (g_presentTimes[i] >= t0) pr.push_back((g_presentTimes[i] - g_presentTimes[i - 1]) * 1e3);
    Stats a = stats(vi), u = stats(upd), p = stats(pr);
    size_t over = 0, prOver = 0;
    for (double d : vi) over += d > 1.5 * 1000.0 / 60.0;
    for (double d : pr) prOver += d > 1.5 * 1000.0 / 60.0;
    const double span = g_pace.back().t - g_pace.front().t;
    host_log("live: pacing: %u retraces in %.2f s real time = %.3f/s (virtual 60.000); display %u Hz, vsync %s, %s; "
             "%u pacing resets\n",
             unsigned(g_pace.size()), span, double(g_pace.size() - 1) / span, displayHz(), g_opt.vsync ? "on" : "off",
             g_opt.fullscreen ? "fullscreen" : "windowed", g_paceResets);
    host_log("live: pacing: retrace interval ms mean %.3f sd %.3f p1 %.3f p99 %.3f min %.3f max %.3f, %u > 25 ms\n",
             a.mean, a.sd, a.p01, a.p99, a.min, a.max, unsigned(over));
    host_log("live: pacing: updateScreen ms mean %.3f p99 %.3f max %.3f\n", u.mean, u.p99, u.max);
    host_log("live: pacing: %u presents (%u retraces); present interval ms mean %.3f sd %.3f p1 %.3f p99 %.3f "
             "max %.3f, %u > 25 ms\n",
             unsigned(np), unsigned(g_pace.size()), p.mean, p.sd, p.p01, p.p99, p.max, unsigned(prOver));
}

[[noreturn]] void shutdown(int code) {
    if (g_audioDev != 0) {
        host_log("live: sound queue resets %u, failed queue calls %u\n", g_audioResets, g_audioDrops);
        SDL_CloseAudioDevice(g_audioDev);
    }
    host_log("live: %u gfx tasks rendered (%u with an unknown ucode), RT64 interrupts SP %u DP %u, %u bytes of "
             "graphics data in display-list areas converted\n",
             g_tasks, g_unknownUcode, g_spIntr, g_dpIntr, g_fixedBytes);
    if (g_wseq.thread.joinable()) g_wseq.thread.join();
    if (g_app != nullptr) g_app->end();
    if (g_window != nullptr) SDL_DestroyWindow(g_window);
    SDL_Quit();
    host_exit(code);
}

const char kTitle[] = "Blast Corps  (F1: settings)";

void setFullscreen(bool on) {
    if (SDL_SetWindowFullscreen(g_window, on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
        host_log("live: fullscreen switch failed: %s\n", SDL_GetError());
        return;
    }
    g_opt.fullscreen = on;
    SDL_ShowCursor(on ? SDL_DISABLE : SDL_ENABLE);
    int pw, ph;
    SDL_GetWindowSizeInPixels(g_window, &pw, &ph);
    host_log("live: %s (%dx%d pixels)\n", on ? "fullscreen" : "windowed", pw, ph);
}

/* widescreen: the visibility test as wide as the picture RT64 draws
 * (host_cull_wide, os_hw.c); 1 with the N64's 4:3.  With a frame rate above
 * the game's own (interpolation), the in-between pictures show the camera
 * part of the way from the last frame's view to this one's, so the test is a
 * little wider still (kInterpCullMargin): no pop-in at the picture's sides
 * while the camera turns. */
constexpr double kInterpCullMargin = 1.1;
void updateCullWide() {
    double k = 1.0;
    if (g_cfg.aspect > 0) {
        k = g_cfg.aspect / (4.0 / 3.0);
    } else if (g_cfg.aspect < 0 && g_window != nullptr) {
        int pw = 0, ph = 0;
        SDL_GetWindowSizeInPixels(g_window, &pw, &ph);
        if (pw > 0 && ph > 0) k = double(pw) / double(ph) / (4.0 / 3.0);
    }
    if (k < 1.0) k = 1.0;
    if (g_cfg.framerate != 0) k *= kInterpCullMargin;
    if (k != host_cull_wide) {
        host_cull_wide = k;
        host_log("live: visibility test %.3f times the 4:3 width (widescreen %s, interpolation %s; frame %u)\n", k,
                 live::aspectName(g_cfg.aspect).c_str(), g_cfg.framerate != 0 ? "on" : "off", plat_frames());
    }
}

/* ---- frame rate: RT64's frame interpolation --------------------------------
 * video.framerate (bc.ini, the menu, --framerate): original (0, the default:
 * RT64's refreshRate Original, every picture as the game made it), display
 * (-1: RT64's Display, the display's refresh rate) or N (Manual, N a second;
 * RT64 caps it at the display's rate).  Above the game's own rate RT64's
 * workload queue renders extra pictures between the last two game frames
 * (rt64_workload_queue.cpp: curFrameWeight), matching the draw calls of the
 * two frames itself: the camera (projection and view), and every model
 * matrix (vehicles, debris, the train, the carrier ...) whose draw call it
 * finds again in the previous frame (G_EX_ID_AUTO, rt64_game_frame.cpp; an
 * object that jumped more than ten times as far as the frame before isn't
 * blended: RigidBody's AUTO rule).  Nothing is added to the game's display
 * lists, so the game (and N64 RAM) is exactly as with interpolation off;
 * only the pictures differ.
 *
 * What this file adds (all on the host, and only with the option on):
 *  - the game's own rate: RT64 would guess it from the VI history and turn
 *    interpolation off whenever the game's cadence wavers (a load, a slow
 *    frame); the cadence measured here (the median of the last three
 *    intervals between frame-buffer swaps) goes to State::setRefreshRate;
 *  - cuts: a frame that must not be blended with the one before (a new game
 *    mode or level: menus, the globe, mission intros, a respawn; the camera
 *    jumping or turning by more than any camera motion in play) gets rate 0,
 *    which makes RT64 show it as it is (no in-between pictures into it);
 *  - the wider visibility test (updateCullWide). */
struct Interp {
    uint32_t lastOrigin = 0;
    unsigned lastSwapVi = 0;
    unsigned intervals[3] = {2, 2, 2};   /* VIs between the last frame-buffer swaps */
    unsigned lastFrame = ~0u;
    unsigned long long lastMode = ~0ull;
    int lastLevel = -1000;
    bool haveCam = false;
    double eye[3] = {}, fwd[3] = {}, lastMove = 0;
    unsigned rate = 0;                   /* last rate given to RT64 (0: a cut) */
    unsigned cuts = 0, frames = 0;
    unsigned displayHz = 0;              /* --display-hz (testing): RT64's display rate */
    bool log = false;                    /* --interp-log: every frame's camera motion */
} g_interp;

/* the game state the cut test reads (fixed addresses, as plat_core.c's trace) */
#define BC_MODE (*(volatile unsigned long long *) uintptr_t(0x80364A90u))
#define BC_LEVEL (*(volatile int *) uintptr_t(0x802E8BDCu))
constexpr uint32_t kDynBase = 0x003156F8u;   /* gDynamicBufs[2] (DynamicBuf, types.h), physical */
constexpr uint32_t kDynSize = 0x21498u, kDynDl = 0x48B0u, kDynDlEnd = 0xA3A0u, kDynView = 0x140u;
constexpr double kCutMove = 400.0;           /* world units the camera may move in a frame ... */
constexpr double kCutMoveFactor = 4.0;       /* ... or this many times as far as the frame before */
constexpr double kCutTurnDeg = 40.0;         /* degrees the camera may turn in a frame */
constexpr double kSnapMove = 60.0;           /* ... when the game snapped its camera */
constexpr double kSnapTurnDeg = 15.0;

/* an N64 Mtx (s15.16; the integer halves in words 0-7, the fractions in
 * 8-15, two elements a word, as libultra's guMtxF2L writes them) */
void mtxToDouble(const uint8_t *p, double m[4][4]) {
    const uint32_t *w = reinterpret_cast<const uint32_t *>(p);
    for (int n = 0; n < 16; n++) {
        const uint32_t iw = w[n / 2], fw = w[8 + n / 2];
        const int16_t hi = int16_t((n & 1) ? (iw & 0xFFFF) : (iw >> 16));
        const uint16_t lo = uint16_t((n & 1) ? (fw & 0xFFFF) : (fw >> 16));
        m[n / 4][n % 4] = double((int32_t(hi) * 65536) | lo) / 65536.0;
    }
}

/* the camera of a view matrix (guLookAt: row vectors, translation in row 3):
 * false if the 3x3 part is not a rotation (the buffer holds something else) */
bool viewCamera(const double m[4][4], double eye[3], double fwd[3]) {
    for (int r = 0; r < 3; r++) {
        const double len = m[r][0] * m[r][0] + m[r][1] * m[r][1] + m[r][2] * m[r][2];
        if (std::fabs(len - 1.0) > 0.02) return false;
    }
    for (int j = 0; j < 3; j++) {
        eye[j] = -(m[3][0] * m[j][0] + m[3][1] * m[j][1] + m[3][2] * m[j][2]);
        fwd[j] = -m[j][2];
    }
    return true;
}

/* liveVi: the game's cadence from the frame-buffer swaps */
void interpVi(const HostViRegs &r, unsigned viCount) {
    if ((r.status & 3) == 0 || r.origin == g_interp.lastOrigin) return;
    if (g_interp.lastSwapVi != 0) {
        g_interp.intervals[0] = g_interp.intervals[1];
        g_interp.intervals[1] = g_interp.intervals[2];
        g_interp.intervals[2] = std::max(1u, std::min(6u, viCount - g_interp.lastSwapVi));
    }
    g_interp.lastOrigin = r.origin;
    g_interp.lastSwapVi = viCount;
}

/* liveGfxTask, before RT64 reads the task: once per game frame (its
 * top-level display list, inside a DynamicBuf), the rate RT64 interpolates
 * from, or 0 for a cut */
void interpTask(uint32_t dl) {
    dl &= 0x3FFFFFF;
    int dyn = -1;
    for (int i = 0; i < 2; i++)
        if (dl >= kDynBase + i * kDynSize + kDynDl && dl < kDynBase + i * kDynSize + kDynDlEnd) dyn = i;
    const unsigned frame = plat_frames();
    if (dyn < 0 || frame == g_interp.lastFrame) return;
    g_interp.lastFrame = frame;
    g_interp.frames++;
    const char *why = nullptr;
    const bool snap = live::interpTagsTakeCut();
    const unsigned long long mode = BC_MODE;
    const int level = BC_LEVEL;
    if (mode != g_interp.lastMode || level != g_interp.lastLevel) why = "game mode or level";
    g_interp.lastMode = mode;
    g_interp.lastLevel = level;
    double m[4][4], eye[3], fwd[3];
    mtxToDouble(rdramBase() + kDynBase + dyn * kDynSize + kDynView, m);
    if (viewCamera(m, eye, fwd)) {
        if (g_interp.haveCam) {
            const double dx = eye[0] - g_interp.eye[0], dy = eye[1] - g_interp.eye[1], dz = eye[2] - g_interp.eye[2];
            const double move = std::sqrt(dx * dx + dy * dy + dz * dz);
            const double dot = fwd[0] * g_interp.fwd[0] + fwd[1] * g_interp.fwd[1] + fwd[2] * g_interp.fwd[2];
            const double turn = std::acos(std::max(-1.0, std::min(1.0, dot))) * 57.29577951308232;
            if (g_interp.log)
                host_log("live: interp frame %u: camera moved %.1f turned %.1f deg (eye %.0f %.0f %.0f)\n", frame,
                         move, turn, eye[0], eye[1], eye[2]);
            if (why == nullptr && move > kCutMove && move > kCutMoveFactor * g_interp.lastMove) why = "camera jump";
            if (why == nullptr && turn > kCutTurnDeg) why = "camera turn";
            /* the game snapped its camera (PORT_ICUT_IF, tagged interpolation):
             * a cut if it really jumped (the snap is also set every frame in
             * the shuttle's and the map's views, which move smoothly) */
            if (why == nullptr && snap && ((move > kSnapMove && move > kCutMoveFactor * g_interp.lastMove) ||
                                           turn > kSnapTurnDeg))
                why = "the game's camera snap";
            g_interp.lastMove = move;
        }
        memcpy(g_interp.eye, eye, sizeof eye);
        memcpy(g_interp.fwd, fwd, sizeof fwd);
        g_interp.haveCam = true;
    } else {
        g_interp.haveCam = false;
    }
    unsigned iv[3] = {g_interp.intervals[0], g_interp.intervals[1], g_interp.intervals[2]};
    std::sort(iv, iv + 3);
    const unsigned rate = why != nullptr ? 0u : 60u / iv[1];
    if (why != nullptr) {
        g_interp.cuts++;
        host_log("live: frame %u shown without in-between pictures (%s)\n", frame, why);
    }
    if (rate != g_interp.rate || g_interp.frames == 1) g_app->state->setRefreshRate(uint16_t(rate));
    g_interp.rate = rate;
}

/* the option on or off (start-up, the menu): RT64's own guess of the rate
 * again when off */
void interpApply() {
    /* tags: only with in-between pictures and video.interpolation = tagged
     * (off or auto: the GBI untouched, no tag recorded) */
    live::interpTagsEnable(g_cfg.framerate != 0 && g_cfg.interpolation == 1);
    if (g_cfg.framerate == 0) g_app->state->setRefreshRate(UINT16_MAX);
    g_interp.frames = 0;
    g_interp.lastFrame = ~0u;
    g_interp.haveCam = false;
}

/* focus and visibility, logged: without the keyboard focus SDL delivers no
 * keys (and resets the ones held); controllers keep working
 * (SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, set at start-up) */
void windowEvent(const SDL_WindowEvent &w) {
    const char *what = nullptr;
    switch (w.event) {
        case SDL_WINDOWEVENT_FOCUS_LOST: g_focused = false; what = "lost the keyboard focus (keys stop; controllers keep working)"; break;
        case SDL_WINDOWEVENT_FOCUS_GAINED: g_focused = true; what = "has the keyboard focus"; break;
        case SDL_WINDOWEVENT_MINIMIZED: what = "minimized"; break;
        case SDL_WINDOWEVENT_RESTORED: what = "restored"; break;
        case SDL_WINDOWEVENT_HIDDEN: what = "hidden"; break;
        case SDL_WINDOWEVENT_SHOWN: what = "shown"; break;
        default: break;
    }
    if (what != nullptr) host_log("live: window %s (frame %u)\n", what, plat_frames());
}

bool isKey(int action, SDL_Scancode sc) {
    for (SDL_Scancode q : g_cfg.keys[action])
        if (q == sc) return true;
    return false;
}

/* the settings menu sees what bc.exe runs with now (keys and options may have
 * changed it since bc.ini was read) */
void openMenu() {
    g_cfg.fullscreen = g_opt.fullscreen;
    g_cfg.vsync = g_opt.vsync;
    g_cfg.volume = g_opt.volume;
    g_cfg.mute = g_opt.mute;
    live::menuOpen();
}

void pumpEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) shutdown(0);
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED && g_cfg.aspect < 0)
            updateCullWide();
        if (e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED) g_pads.scan();
        if (e.type == SDL_WINDOWEVENT) windowEvent(e.window);
        /* a controller used while another window has the focus (e.g. the Xbox
         * button's Game Bar, a notification that took it): fullscreen, take
         * the focus back so the keyboard works again too */
        if (!g_focused && g_opt.fullscreen && e.type == SDL_CONTROLLERBUTTONDOWN && g_pads.isActive(g_cfg, e.cbutton.which)) {
            host_log("live: controller button while unfocused: raising the window (frame %u)\n", plat_frames());
            SDL_RaiseWindow(g_window);
        }
        if (live::menuIsOpen()) {
            /* waiting for a key or button to bind: everything goes to the menu */
            if (live::menuCapturing()) {
                live::menuEvent(e);
                continue;
            }
            /* the menu's own keys: Esc or the menu key close it, as do the
             * controller's menu buttons; fullscreen still switches */
            if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                const SDL_Scancode sc = e.key.keysym.scancode;
                if (sc == SDL_SCANCODE_ESCAPE || isKey(live::A_MENU, sc)) {
                    live::menuClose();
                    continue;
                }
                if (sc == SDL_SCANCODE_F11 || (sc == SDL_SCANCODE_RETURN && (e.key.keysym.mod & KMOD_ALT) != 0)) {
                    setFullscreen(!g_opt.fullscreen);
                    g_cfg.fullscreen = g_opt.fullscreen;
                    continue;
                }
            }
            if (e.type == SDL_CONTROLLERBUTTONDOWN && menuCombo()) {
                live::menuClose();
                continue;
            }
            live::menuEvent(e);
            continue;
        }
        if (e.type == SDL_KEYDOWN && isKey(live::A_QUIT, e.key.keysym.scancode)) shutdown(0);
        if (e.type == SDL_CONTROLLERBUTTONDOWN && g_pads.isActive(g_cfg, e.cbutton.which)) {
            for (const live::PadInput &in : g_cfg.pad[live::A_QUIT])
                if (in.button == e.cbutton.button) shutdown(0);
            if (menuCombo()) openMenu();
            else if (live::comboHeld(g_cfg, g_pads, g_cfg.fpsPad)) counterCycle();
        }
        if (e.type == SDL_KEYDOWN && !e.key.repeat) {
            const SDL_Scancode sc = e.key.keysym.scancode;
            if (sc == SDL_SCANCODE_F11 || (sc == SDL_SCANCODE_RETURN && (e.key.keysym.mod & KMOD_ALT) != 0)) {
                setFullscreen(!g_opt.fullscreen);
            } else if (isKey(live::A_MENU, sc)) {
                openMenu();
            } else if (isKey(live::A_FPS, sc)) {
                counterCycle();
            } else if (e.key.keysym.scancode == SDL_SCANCODE_M) {
                g_opt.mute = !g_opt.mute;
                host_log("live: sound %s\n", g_opt.mute ? "off" : "on");
            } else if (e.key.keysym.scancode == SDL_SCANCODE_MINUS || e.key.keysym.scancode == SDL_SCANCODE_EQUALS) {
                g_opt.volume = std::max(0, std::min(100, g_opt.volume + (e.key.keysym.scancode == SDL_SCANCODE_MINUS ? -10 : 10)));
                host_log("live: volume %d%%\n", g_opt.volume);
            }
        }
    }
    /* once a second: controllers whose device events were missed */
    if (++g_lastScan >= 60) {
        g_lastScan = 0;
        g_pads.scan();
    }
    pollInput();
}

/* ---- HostLive ---------------------------------------------------------------- */
void liveBoot() {
    stageUcode();
}

/* --dl-dump: walk an F3D display list (segments, sub-lists) and print the
 * commands that matter for the data layout, with the first vertex of each
 * G_VTX in both layouts */
unsigned g_diagTasks, g_dlLastFrame = ~0u;
void diagWalk(unsigned dl) {
    uint32_t seg[16] = {0};
    uint32_t stack[16];
    int sp = 0, n = 0;
    auto res = [&](uint32_t a) {
        uint32_t full = (seg[(a >> 24) & 15] + (a & 0xFFFFFF)) & 0xFFFFFF;
        if (full >= 0x800000) host_log("  BAD address %08X (segment %u = %08X) -> %06X\n", a, (a >> 24) & 15, seg[(a >> 24) & 15], full);
        return full & 0x7FFFFF;
    };
    uint32_t a = dl & 0x7FFFFF;
    while (n++ < 20000) {
        const uint32_t *w = reinterpret_cast<const uint32_t *>(rdramBase() + a);
        uint32_t w0 = w[0], w1 = w[1], op = w0 >> 24;
        a += 8;
        switch (op) {
        case 0x01: host_log("  %06X MTX %08X -> %06X p%02X\n", a - 8, w1, res(w1), (w0 >> 16) & 0xFF); break;
        case 0x03: host_log("  %06X MOVEMEM %02X %08X -> %06X\n", a - 8, (w0 >> 16) & 0xFF, w1, res(w1)); break;
        case 0x04: {
            uint32_t r = res(w1);
            const int16_t *h = reinterpret_cast<const int16_t *>(rdramBase() + r);
            const uint8_t *b = rdramBase() + r;
            host_log("  %06X VTX n%u v%u %08X -> %06X  native(%d,%d,%d c%02X%02X%02X%02X) emu(%d,%d,%d)\n", a - 8,
                     ((w0 >> 20) & 15) + 1, (w0 >> 16) & 15, w1, r, h[0], h[1], h[2], b[12], b[13], b[14], b[15],
                     h[1], h[0], h[3]);
            break;
        }
        case 0x06:
            host_log("  %06X DL %08X -> %06X%s\n", a - 8, w1, res(w1), ((w0 >> 16) & 1) ? " (branch)" : "");
            if (!((w0 >> 16) & 1)) { if (sp < 16) stack[sp++] = a; }
            a = res(w1);
            break;
        case 0xB8:
            if (sp == 0) return;
            a = stack[--sp];
            break;
        case 0xBC:
            if ((w0 & 0xFF) == 6) { seg[((w0 >> 8) & 0xFFFF) / 4 & 15] = w1 & 0xFFFFFF; host_log("  %06X SEG %u = %08X\n", a - 8, ((w0 >> 8) & 0xFFFF) / 4, w1); }
            break;
        case 0xFD: host_log("  %06X SETTIMG %08X -> %06X fmt %u siz %u w %u\n", a - 8, w1, res(w1), (w0 >> 21) & 7, (w0 >> 19) & 3, (w0 & 0xFFF) + 1); break;
        case 0xFF: host_log("  %06X SETCIMG %08X\n", a - 8, w1); break;
        case 0xFA: host_log("  %06X PRIM %08X\n", a - 8, w1); break;
        case 0xF9: host_log("  %06X BLENDCOL %08X\n", a - 8, w1); break;
        case 0xF8: host_log("  %06X FOGCOL %08X\n", a - 8, w1); break;
        case 0xFB: host_log("  %06X ENVCOL %08X\n", a - 8, w1); break;
        case 0xFC: host_log("  %06X COMBINE %08X %08X\n", a - 8, w0, w1); break;
        case 0xB9: host_log("  %06X OTHERMODE_L %08X %08X\n", a - 8, w0, w1); break;
        case 0xBA: host_log("  %06X OTHERMODE_H %08X %08X\n", a - 8, w0, w1); break;
        case 0xB6: host_log("  %06X CLEARGEOM %08X\n", a - 8, w1); break;
        case 0xB7: host_log("  %06X SETGEOM %08X\n", a - 8, w1); break;
        case 0xF5: host_log("  %06X SETTILE %08X %08X\n", a - 8, w0, w1); break;
        case 0xF3: host_log("  %06X LOADBLOCK %08X %08X\n", a - 8, w0, w1); break;
        case 0xF4: host_log("  %06X LOADTILE %08X %08X\n", a - 8, w0, w1); break;
        case 0xF0: host_log("  %06X LOADTLUT %08X %08X\n", a - 8, w0, w1); break;
        case 0xBF: host_log("  %06X TRI %08X\n", a - 8, w1); break;
        case 0xE4: host_log("  %06X TEXRECT %08X %08X\n", a - 8, w0, w1); break;
        case 0xF6: host_log("  %06X FILLRECT %08X %08X\n", a - 8, w0, w1); break;
        case 0xBB: host_log("  %06X TEXTURE %08X %08X\n", a - 8, w0, w1); break;
        case 0xB5: host_log("  %06X LINE3D %08X\n", a - 8, w1); break;
        default: break;
        }
    }
}

/* --dl-skip LO:HI (debugging): triangles whose commands lie in [LO, HI) (physical) are
 * turned into no-ops while RT64 draws the task, then put back */
uint32_t g_skipLo, g_skipHi;
std::vector<std::pair<uint32_t *, uint32_t>> g_skipped;
void skipWalk(unsigned dl) {
    uint32_t seg[16] = {0};
    uint32_t stack[16];
    int sp = 0, n = 0;
    auto res = [&](uint32_t a) { return ((seg[(a >> 24) & 15] + (a & 0xFFFFFF)) & 0xFFFFFF) & 0x7FFFFF; };
    uint32_t a = dl & 0x7FFFFF;
    while (n++ < 40000) {
        uint32_t *w = reinterpret_cast<uint32_t *>(rdramBase() + a);
        uint32_t op = w[0] >> 24;
        a += 8;
        if (op == 0xBF || op == 0xB1) {
            if (a - 8 >= g_skipLo && a - 8 < g_skipHi) {
                g_skipped.push_back({w, w[0]});
                w[0] = 0;   /* G_SPNOOP */
            }
        } else if (op == 0x06) {
            if (!((w[0] >> 16) & 1) && sp < 16) stack[sp++] = a;
            a = res(w[1]);
        } else if (op == 0xB8) {
            if (sp == 0) return;
            a = stack[--sp];
        } else if (op == 0xBC && (w[0] & 0xFF) == 6) {
            seg[((w[0] >> 8) & 0xFFFF) / 4 & 15] = w[1] & 0xFFFFFF;
        }
    }
}

/* the game frame's top-level display list (inside a DynamicBuf), not an
 * offscreen task */
bool isFrameDl(uint32_t dl) {
    dl &= 0x3FFFFFF;
    for (int i = 0; i < 2; i++)
        if (dl >= kDynBase + i * kDynSize + kDynDl && dl < kDynBase + i * kDynSize + kDynDlEnd) return true;
    return false;
}

void liveGfxTask(unsigned ucode, unsigned ucodeData, unsigned dataPtr, unsigned dataSize) {
    (void) dataSize;
    if (g_diagTasks < g_opt.dlDumpTasks && plat_frames() >= g_opt.dlDumpFrame) {
        host_log("DIAG task %u ucode %08X dl %08X\n", g_diagTasks, ucode, dataPtr);
        diagWalk(dataPtr);
        g_diagTasks++;
    } else if (g_opt.dlDumpEvery != 0 && plat_frames() % g_opt.dlDumpEvery == 0 && plat_frames() != g_dlLastFrame) {
        g_dlLastFrame = plat_frames();   /* the frame's first task */
        host_log("DIAG frame %u task ucode %08X dl %08X\n", g_dlLastFrame, ucode, dataPtr);
        diagWalk(dataPtr);
    }
    g_app->interpreter->loadUCodeGBI(ucode & 0x3FFFFFF, ucodeData & 0x3FFFFFF, true);
    if (g_app->interpreter->hleGBI == nullptr) {
        if (g_unknownUcode++ == 0) host_log("live: RT64 does not know ucode 0x%08X/0x%08X\n", ucode, ucodeData);
        return;
    }
    g_tasks++;
    if (g_app->interpreter->hleGBI->ucode != RT64::GBIUCode::F3D) g_otherUcodeTasks++;
    if (g_opt.gfxFix) g_fixedBytes += port_gfx_fix_task(dataPtr);
    if (g_skipHi != 0 && g_app->interpreter->hleGBI->ucode == RT64::GBIUCode::F3D) skipWalk(dataPtr);
    if (g_cfg.framerate != 0) interpTask(dataPtr);
    const bool frameDl = isFrameDl(dataPtr);
    live::interpTagsBeforeTask(dataPtr, frameDl);
    LARGE_INTEGER t0, t1;
    if (g_cfg.framerate != 0) QueryPerformanceCounter(&t0);
    g_app->processDisplayLists(rdramBase(), dataPtr & 0x3FFFFFF, 0, true);
    if (g_cfg.framerate != 0) {
        QueryPerformanceCounter(&t1);
        g_dlSeconds += double(t1.QuadPart - t0.QuadPart) / double(g_freq.QuadPart);
        g_dlTasks++;
    }
    live::interpTagsAfterTask(frameDl);
    for (auto &s : g_skipped) *s.first = s.second;
    g_skipped.clear();
}

unsigned long long g_when0;
unsigned g_nextShot, g_nextWshot;
bool g_quitRequested;
unsigned g_hashFrame = ~0u;          /* --texture-hashes */
void logTextureHashes(unsigned frame);

/* The settings menu is open: the game waits (inside this retrace's hook)
 * while the menu runs, about 60 times a second, over the last picture.
 * RT64's own pause (its debugger's) re-renders the last frame's work and
 * presents it again at every updateScreen, so video settings show at once.
 * The pacing starts over afterwards.  `shotFrame` != 0: --menu-shot (save
 * the window's picture after a few frames and close the menu). */
void runMenu(unsigned shotFrame) {
    g_app->state->debuggerInspector.paused = true;
    for (unsigned n = 0; live::menuIsOpen(); n++) {
        if (shotFrame != 0 && n == 2)
            for (const std::string &s : g_opt.menuSets)
                if (!live::menuTestSet(s)) host_log("live: --menu-set %s: not understood\n", s.c_str());
        live::menuFrame();
        g_app->updateScreen();
        if (shotFrame != 0 && n == 30) {
            char name[32];
            snprintf(name, sizeof name, "menu_%07u", shotFrame);
            saveWindow(name);
            live::menuClose();
        }
        if (g_quitRequested) shutdown(0);
        Sleep(15);
        if (live::menuIsOpen()) pumpEvents();
    }
    g_app->state->debuggerInspector.paused = false;
    if (g_quitRequested) shutdown(0);
    g_t0.QuadPart = 0;   /* pacing: start over from the next retrace */
    /* what is held now (the keys or buttons that closed the menu) doesn't
     * reach the game until let go (each on its own, at most a second) */
    g_gate.close(live::readPad(g_cfg, g_pads, SDL_GetKeyboardState(nullptr), (SDL_GetModState() & KMOD_ALT) != 0));
}

void liveVi(const HostViRegs *r, unsigned viCount, unsigned long long when, unsigned frames) {
    if (g_cfg.framerate != 0) interpVi(*r, viCount);
    if (g_interp.displayHz != 0) g_app->sharedQueueResources->setSwapChainRate(g_interp.displayHz);
    if (r->status != VI_STATUS_REG)
        host_log("live: VI status %08X (type %u, gamma dither %u, gamma %u, divot %u, AA mode %u, dither filter %u)\n",
                 r->status, r->status & 3, (r->status >> 2) & 1, (r->status >> 3) & 1, (r->status >> 4) & 1,
                 (r->status >> 8) & 3, (r->status >> 16) & 1);
    VI_STATUS_REG = r->status;
    VI_ORIGIN_REG = r->origin;
    VI_WIDTH_REG = r->width;
    VI_INTR_REG = r->intr;
    VI_V_CURRENT_LINE_REG = 0;
    VI_TIMING_REG = r->burst;
    VI_V_SYNC_REG = r->v_sync;
    VI_H_SYNC_REG = r->h_sync;
    VI_LEAP_REG = r->leap;
    VI_H_START_REG = r->h_start;
    VI_V_START_REG = r->v_start;
    VI_V_BURST_REG = r->v_burst;
    VI_X_SCALE_REG = r->x_scale;
    VI_Y_SCALE_REG = r->y_scale;

    /* screenshots of the picture on screen once the game reached the frame */
    while (g_nextShot < g_opt.shots.size() && frames >= g_opt.shots[g_nextShot]) {
        saveFrame(*r, frames);
        g_nextShot++;
    }
    if (g_opt.shotEvery != 0 && frames != 0 && frames % g_opt.shotEvery == 0) {
        static unsigned last;
        if (last != frames) saveFrame(*r, frames), last = frames;
    }

    /* real time: retrace N of virtual time is shown at t0 + N/60 s.  The game
     * logic runs on the virtual clock either way; this only decides when each
     * retrace's picture goes to RT64 (and vsync then shows it at the
     * display's next refresh).  Waiting before handing it over (not after)
     * keeps the hand-over times regular: the game's and RT64's own work for
     * the next retrace happens after it, inside the wait's slack. */
    if (g_opt.pace) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        if (g_t0.QuadPart == 0) {
            g_t0 = now;
            g_when0 = when;
        }
        const double due = double(when - g_when0) / 46875000.0;
        double t = double(now.QuadPart - g_t0.QuadPart) / double(g_freq.QuadPart);
        if (t > due + 0.25) {   /* far behind (a level load, a stall): start over */
            g_t0 = now;
            g_when0 = when;
            g_paceResets++;
        } else {
            while (t < due) {
                const double left = due - t;
                if (left > 0.002) Sleep(DWORD((left - 0.0015) * 1000.0));
                QueryPerformanceCounter(&now);
                t = double(now.QuadPart - g_t0.QuadPart) / double(g_freq.QuadPart);
            }
        }
    }
    counterTick(frames);
    const double before = g_opt.paceLog != nullptr ? secondsSince(g_tStart) : 0.0;
    g_app->updateScreen();
    if (g_opt.paceLog != nullptr) {
        const double after = secondsSince(g_tStart);
        const double due = g_t0.QuadPart == 0 ? 0.0
            : double(g_t0.QuadPart - g_tStart.QuadPart) / double(g_freq.QuadPart) + double(when - g_when0) / 46875000.0;
        g_pace.push_back({before, due, after - before, frames});
    }
    if (frames >= g_hashFrame) {
        logTextureHashes(frames);
        g_hashFrame = ~0u;
    }
    if (!g_wseq.started && frames >= g_wseq.frame) {
        g_wseq.started = true;
        g_wseq.thread = std::thread(winSeqThread, frames, g_wseq.ms);
    }
    /* pictures of the window (testing): --wshot, --menu-shot */
    while (g_nextWshot < g_opt.wshots.size() && frames >= g_opt.wshots[g_nextWshot]) {
        char name[32];
        snprintf(name, sizeof name, "win_%07u", frames);
        saveWindow(name);
        g_nextWshot++;
    }
    if (g_opt.menuShot != ~0u && frames >= g_opt.menuShot) {
        g_opt.menuShot = ~0u;
        openMenu();
        runMenu(frames);
    }
    pumpEvents();
    if (live::menuIsOpen()) runMenu(0);
}

int liveInput(unsigned short *button, signed char *x, signed char *y) {
    static unsigned short lastButton;
    *button = g_button;
    *x = g_stickX;
    *y = g_stickY;
    if (host_verbose && g_button != lastButton)
        host_log("live: pad buttons %04X stick %d,%d (frame %u)\n", g_button, g_stickX, g_stickY, plat_frames());
    lastButton = g_button;
    return 1;
}

const HostLive kLive = {liveBoot, liveGfxTask, liveVi, liveInput, liveAudio};

/* ---- settings changed in the menu ------------------------------------------- */
/* RT64's present thread, after the picture: --pace-log's times, the menu */
void drawHook(plume::RenderCommandList *list, plume::RenderFramebuffer *fb) {
    if (g_presentTimes != nullptr) presentHook(list, fb);
    if (g_counter.on) counterPresent();
    live::menuDraw(list);
}

void deinitHook() {
    live::menuShutdown();
}

/* MSAA sample counts the device can render with (a mask of 1, 2, 4, 8);
 * 1 only when it lacks programmable sample locations, which RT64 needs */
unsigned msaaSupported() {
    if (g_app == nullptr || !g_app->device || !g_app->device->getCapabilities().sampleLocations) return 1;
    const bool hdr = g_app->shaderLibrary->usesHDR;
    return unsigned(g_app->device->getSampleCountsSupported(RT64::RenderTarget::colorBufferFormat(hdr)) &
                    g_app->device->getSampleCountsSupported(RT64::RenderTarget::depthBufferFormat()));
}

/* bc.ini's video enhancements as RT64's user configuration.  The defaults
 * (resolution = window, msaa = 0, filtering = aa) are RT64's own defaults,
 * what bc.exe always used. */
void configToRT64(RT64::UserConfiguration &u) {
    using U = RT64::UserConfiguration;
    if (g_cfg.resolution == 0) u.resolution = U::Resolution::WindowIntegerScale;
    else if (g_cfg.resolution == 1) u.resolution = U::Resolution::Original;
    else {
        u.resolution = U::Resolution::Manual;
        u.resolutionMultiplier = double(g_cfg.resolution);
    }
    u.antialiasing = g_cfg.msaa == 8 ? U::Antialiasing::MSAA8X : g_cfg.msaa == 4 ? U::Antialiasing::MSAA4X
                     : g_cfg.msaa == 2 ? U::Antialiasing::MSAA2X : U::Antialiasing::None;
    u.filtering = g_cfg.filtering == 1 ? U::Filtering::Linear : g_cfg.filtering == 2 ? U::Filtering::Nearest
                                                                                      : U::Filtering::AntiAliasedPixelScaling;
    /* widescreen: RT64 widens the viewport of every projection that covers the
     * whole width and narrows its x scale to match (no stretching); 2D
     * rectangles and partial-width views keep their 4:3 shape, centred */
    if (g_cfg.aspect == 0) {
        u.aspectRatio = U::AspectRatio::Original;
    } else if (g_cfg.aspect < 0) {
        u.aspectRatio = U::AspectRatio::Expand;
    } else {
        u.aspectRatio = U::AspectRatio::Manual;
        u.aspectTarget = g_cfg.aspect;
    }
    /* frame rate: in-between pictures (see interpTask) */
    if (g_cfg.framerate == 0) {
        u.refreshRate = U::RefreshRate::Original;
    } else if (g_cfg.framerate < 0) {
        u.refreshRate = U::RefreshRate::Display;
    } else {
        u.refreshRate = U::RefreshRate::Manual;
        u.refreshRateTarget = g_cfg.framerate;
    }
}

/* texture pack (RT64's replacements): loaded when enabled and set, cleared
 * when turned off.  Called before the game starts and from the menu (the
 * main thread, which is the one that submits textures, as RT64 requires).
 * Returns a line for the log / menu. */
bool g_packLoaded;
std::string g_packStatus;
void applyTexturePack() {
    if (g_cfg.texturePackOn && !g_cfg.texturePack.empty()) {
        const std::string path = live::absPath(g_cfg, g_cfg.texturePack);
        const bool ok = g_app->textureCache->loadReplacementDirectory(
            RT64::ReplacementDirectory(std::filesystem::path(live::wide(path))));
        g_packLoaded = true;
        g_packStatus = (ok ? "loaded " : "could not load ") + path;
        host_log("live: texture pack %s\n", g_packStatus.c_str());
    } else if (g_packLoaded) {
        g_app->textureCache->clearReplacementDirectories();
        g_packLoaded = false;
        g_packStatus = "off";
        host_log("live: texture pack off\n");
    }
}

/* --texture-hashes F (for making texture packs and testing): the RT64
 * hashes of the textures loaded at frame F */
void logTextureHashes(unsigned frame) {
    std::unique_lock<std::mutex> lock(g_app->textureCache->textureMapMutex);
    const RT64::TextureMap &m = g_app->textureCache->textureMap;
    unsigned n = 0;
    for (size_t i = 0; i < m.textures.size(); i++)
        if (m.textures[i] != nullptr) {
            host_log("texture %016llx %ux%u (hash version %u, frame %u)\n", (unsigned long long) m.hashes[i],
                     unsigned(m.textures[i]->width), unsigned(m.textures[i]->height),
                     unsigned(RT64::TMEMHasher::CurrentHashVersion), frame);
            n++;
        }
    host_log("live: %u textures loaded at frame %u\n", n, frame);
}

/* the menu changed g_cfg: apply it now where that is possible */
void settingChanged(const live::Setting &s) {
    if (s.section == "video" && (s.key == "texture_pack" || s.key == "texture_pack_enabled")) {
        applyTexturePack();
        return;
    }
    if (s.section == "video" && (s.key == "fps_counter" || s.key == "fps_counter_corner")) {
        counterApply();
    } else if (s.section == "video" && s.key == "interpolation") {
        interpApply();
    } else if (s.section == "video" && (s.key == "resolution" || s.key == "filtering" || s.key == "msaa" || s.key == "aspect" ||
                                 s.key == "framerate")) {
        if (s.key == "framerate") interpApply();
        const uint32_t before = g_app->userConfig.msaaSampleCount();
        if (s.key == "msaa" && g_cfg.msaa > 1 && (msaaSupported() & unsigned(g_cfg.msaa)) == 0) {
            host_log("live: %dx MSAA isn't supported by this device\n", g_cfg.msaa);
            g_cfg.msaa = int(before > 1 ? before : 0);
        }
        configToRT64(g_app->userConfig);
        g_app->userConfig.validate();
        /* new sample count: RT64 rebuilds its targets and shaders (as its inspector does) */
        if (g_app->userConfig.msaaSampleCount() != before) g_app->updateMultisampling();
        g_app->updateUserConfig(s.key == "resolution" || s.key == "aspect");
        updateCullWide();
    } else if (s.section == "video") {
        if (s.key == "fullscreen") {
            if (g_cfg.fullscreen != g_opt.fullscreen) setFullscreen(g_cfg.fullscreen);
            if (live::menuIsOpen()) SDL_ShowCursor(SDL_ENABLE);
        } else if (s.key == "scale") {
            g_opt.scale = g_cfg.scale;
            if (!g_opt.fullscreen) SDL_SetWindowSize(g_window, int(320 * g_opt.scale), int(240 * g_opt.scale));
        } else if (s.key == "vsync") {
            g_opt.vsync = g_cfg.vsync;
            g_app->swapChain->setVsyncEnabled(g_opt.vsync);
        }
        /* api: the next start */
    } else if (s.section == "audio") {
        g_opt.volume = g_cfg.volume;
        g_opt.mute = g_cfg.mute;
    }
    host_log("live: setting %s.%s = %s\n", s.section.c_str(), s.key.c_str(), live::configValue(g_cfg, s).c_str());
}

/* the menu's status line */
std::string rendererStatus() {
    char buf[256];
    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(g_window, &pw, &ph);
    float sx, sy;
    {
        std::scoped_lock<std::mutex> lock(g_app->sharedQueueResources->configurationMutex);
        sx = g_app->sharedQueueResources->resolutionScale.x;
        sy = g_app->sharedQueueResources->resolutionScale.y;
    }
    snprintf(buf, sizeof buf, "%s, window %dx%d%s, drawn at %.0fx%.0f, display %u Hz",
             g_opt.vulkan ? "Vulkan" : "Direct3D 12", pw, ph, g_opt.fullscreen ? " (fullscreen)" : "", 320.0f * sx,
             240.0f * sy, displayHz());
    return buf;
}

/* ---- start-up ---------------------------------------------------------------- */
unsigned num(const char *s) {
    return unsigned(strtoul(s, nullptr, 0));
}

/* options that set a bc.ini setting for the run (main applies them) */
const std::pair<const char *, const char *> kSetAliases[] = {
    {"--resolution", "video.resolution"},
    {"--msaa", "video.msaa"},
    {"--filtering", "video.filtering"},
    {"--aspect", "video.aspect"},
    {"--framerate", "video.framerate"},
    {"--interpolation", "video.interpolation"},
    {"--fps-counter", "video.fps_counter"},
    {"--texture-pack", "video.texture_pack"},   /* (and turns it on) */
};

int extraArg(int argc, char **argv, int *i, HostOpts *o) {
    const char *a = argv[*i];
    auto next = [&]() -> const char * {
        if (*i + 1 >= argc) host_fatal("%s needs a value", a);
        return argv[++*i];
    };
    for (const auto &alias : kSetAliases)
        if (!strcmp(a, alias.first)) {
            next();
            return 1;
        }
    if (!strcmp(a, "--api")) g_opt.vulkan = !strcmp(next(), "vulkan");
    else if (!strcmp(a, "--fullscreen")) g_opt.fullscreen = true;
    else if (!strcmp(a, "--windowed")) g_opt.fullscreen = false;
    else if (!strcmp(a, "--vsync")) g_opt.vsync = true;
    else if (!strcmp(a, "--no-vsync")) g_opt.vsync = false;
    else if (!strcmp(a, "--no-rom-check")) g_opt.romCheck = false;
    else if (!strcmp(a, "--pace-log")) g_opt.paceLog = next();
    else if (!strcmp(a, "--config") || !strcmp(a, "--no-config")) {
        if (a[2] == 'c') next();   /* read before host_main (main) */
    } else if (!strcmp(a, "--saves")) {
        g_cfg.saves = next();
        o->eeprom_path = (g_eepromPath = live::absPath(g_cfg, g_cfg.saves) + "\\blastcorps.eep").c_str();
        o->mpk_path = g_cfg.pak ? (g_mpkPath = live::absPath(g_cfg, g_cfg.saves) + "\\blastcorps.mpk").c_str() : nullptr;
    } else if (!strcmp(a, "--no-pak")) {
        o->mpk_path = nullptr;
    } else if (!strcmp(a, "--no-saves")) {
        o->eeprom_path = nullptr;
        o->mpk_path = nullptr;
    }
    else if (!strcmp(a, "--set")) next();   /* applied with the config file (main) */
    else if (!strcmp(a, "--window")) {
        const char *s = next();
        if (sscanf(s, "%ux%u", &g_opt.windowW, &g_opt.windowH) != 2 || g_opt.windowW < 160 || g_opt.windowH < 120)
            host_fatal("--window %s: WIDTHxHEIGHT, e.g. 1680x720", s);
    } else if (!strcmp(a, "--wshot")) {
        std::string s = next();
        size_t p = 0;
        while (p < s.size()) {
            size_t q = s.find(',', p);
            if (q == std::string::npos) q = s.size();
            g_opt.wshots.push_back(num(s.substr(p, q - p).c_str()));
            p = q + 1;
        }
        std::sort(g_opt.wshots.begin(), g_opt.wshots.end());
    } else if (!strcmp(a, "--menu-shot")) g_opt.menuShot = std::max(1u, num(next()));
    else if (!strcmp(a, "--menu-set")) g_opt.menuSets.push_back(next());
    else if (!strcmp(a, "--texture-hashes")) g_hashFrame = num(next());
    else if (!strcmp(a, "--display-hz")) g_interp.displayHz = std::max(1u, num(next()));
    else if (!strcmp(a, "--interp-log")) g_interp.log = true;
    else if (!strcmp(a, "--tag-test")) live::interpTagsSetTest(num(next()));
    else if (!strcmp(a, "--wseq")) {
        const char *s = next();
        if (sscanf(s, "%u:%u", &g_wseq.frame, &g_wseq.ms) != 2) host_fatal("--wseq %s: FRAME:MILLISECONDS", s);
    }
    else if (!strcmp(a, "--no-pace")) g_opt.pace = false;
    else if (!strcmp(a, "--scale")) g_opt.scale = std::max(1u, num(next()));
    else if (!strcmp(a, "--shot-dir")) g_opt.shotDir = next();
    else if (!strcmp(a, "--shot-every")) g_opt.shotEvery = num(next());
    else if (!strcmp(a, "--no-gfx-fix")) g_opt.gfxFix = false;
    else if (!strcmp(a, "--mute")) g_opt.mute = true;
    else if (!strcmp(a, "--volume")) g_opt.volume = std::max(0, std::min(100, int(num(next()))));
    else if (!strcmp(a, "--gfx-fix-log")) port_gfx_debug = 1;
    else if (!strcmp(a, "--dl-dump-every")) g_opt.dlDumpEvery = num(next());
    else if (!strcmp(a, "--dl-skip")) {
        const char *s = next(), *colon = strchr(s, ':');
        g_skipLo = num(s);
        g_skipHi = colon != nullptr ? num(colon + 1) : g_skipLo + 8;
    }
    else if (!strcmp(a, "--dl-dump")) {
        const char *s = next(), *colon = strchr(s, ':');
        g_opt.dlDumpFrame = num(s);
        if (colon != nullptr) g_opt.dlDumpTasks = num(colon + 1);
    }
    else if (!strcmp(a, "--shot")) {
        std::string s = next();
        size_t p = 0;
        while (p < s.size()) {
            size_t q = s.find(',', p);
            if (q == std::string::npos) q = s.size();
            g_opt.shots.push_back(num(s.substr(p, q - p).c_str()));
            p = q + 1;
        }
        std::sort(g_opt.shots.begin(), g_opt.shots.end());
    } else
        return 0;
    return 1;
}

/* the save files' folder must exist before the platform opens them */
void makeSaveDirs(const HostOpts *o) {
    for (const char *p : {o->eeprom_path, o->mpk_path}) {
        if (p == nullptr) continue;
        std::wstring w = live::wide(p);
        size_t slash = w.find_last_of(L"\\/");
        if (slash == std::wstring::npos || slash == 0) continue;
        std::wstring dir = w.substr(0, slash);
        wchar_t full[32768];
        if (GetFullPathNameW(dir.c_str(), 32768, full, nullptr) == 0) continue;
        int r = SHCreateDirectoryExW(nullptr, full, nullptr);
        if (r != ERROR_SUCCESS && r != ERROR_ALREADY_EXISTS && r != ERROR_FILE_EXISTS)
            g_cfg.warnings.push_back("can't create the saves folder " + live::utf8(full) +
                                     " (error " + std::to_string(r) + "): the game won't be saved");
    }
}

void prestart(HostOpts *o) {
    host_log("live: Blast Corps port %s, ROM %s\n", BC_VERSION, o->rom_path);
    host_log("live: saves: EEPROM %s, Controller Pak %s\n", o->eeprom_path ? o->eeprom_path : "(not kept)",
             o->mpk_path ? o->mpk_path : "(none)");
    makeSaveDirs(o);
    if (!g_cfg.warnings.empty()) {
        std::string msg = "Some settings were not understood and their defaults are used:\n\n";
        for (const std::string &w : g_cfg.warnings) {
            host_log("live: config: %s\n", w.c_str());
            msg += w + "\n";
        }
        host_message(msg.c_str(), 0);
    }

    timeBeginPeriod(1);   /* 1 ms Sleep granularity for the pacing */
    /* DPI aware: the swap chain gets the display's real pixels (a sharp
     * picture on scaled desktops, e.g. 1920x1080 at 125 %); window sizes stay
     * in desktop points, so the window looks as large as before */
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
    /* game controllers keep working when another window takes the focus (the
     * Game Bar of the Xbox button, a notification): SDL's default drops their
     * input until the window gets the focus back, which a controller-only
     * player in fullscreen can't do (the keyboard stops too: it can't be
     * helped; a controller button takes the focus back, pumpEvents) */
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
        host_fatal("Can't start SDL (video, controllers): %s", SDL_GetError());
    /* the window at 320x240 times the scale, as large as fits the desktop */
    SDL_Rect usable;
    unsigned scale = g_opt.scale;
    if (SDL_GetDisplayUsableBounds(0, &usable) == 0)
        while (scale > 1 && (320 * scale > unsigned(usable.w) || 240 * scale + 40 > unsigned(usable.h))) scale--;
    Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (g_opt.fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    const int winW = g_opt.windowW != 0 ? int(g_opt.windowW) : int(320 * scale);
    const int winH = g_opt.windowH != 0 ? int(g_opt.windowH) : int(240 * scale);
    g_window = SDL_CreateWindow(kTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, flags);
    if (g_window == nullptr) host_fatal("Can't create the game window: %s", SDL_GetError());
    SDL_SetWindowMinimumSize(g_window, 320, 240);
    if (g_opt.fullscreen) SDL_ShowCursor(SDL_DISABLE);
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    if (!SDL_GetWindowWMInfo(g_window, &wm)) host_fatal("SDL_GetWindowWMInfo: %s", SDL_GetError());
    host_gui_window = wm.info.win.window;
    g_pads.scan();

    RT64::Application::Core core = {};
    core.window = wm.info.win.window;
    core.HEADER = g_header;
    core.RDRAM = rdramBase();
    core.DMEM = g_dmem;
    core.IMEM = g_imem;
    core.MI_INTR_REG = &MI_INTR_REG;
    core.DPC_START_REG = &DPC_START_REG;
    core.DPC_END_REG = &DPC_END_REG;
    core.DPC_CURRENT_REG = &DPC_CURRENT_REG;
    core.DPC_STATUS_REG = &DPC_STATUS_REG;
    core.DPC_CLOCK_REG = &DPC_CLOCK_REG;
    core.DPC_BUFBUSY_REG = &DPC_BUFBUSY_REG;
    core.DPC_PIPEBUSY_REG = &DPC_PIPEBUSY_REG;
    core.DPC_TMEM_REG = &DPC_TMEM_REG;
    core.VI_STATUS_REG = &VI_STATUS_REG;
    core.VI_ORIGIN_REG = &VI_ORIGIN_REG;
    core.VI_WIDTH_REG = &VI_WIDTH_REG;
    core.VI_INTR_REG = &VI_INTR_REG;
    core.VI_V_CURRENT_LINE_REG = &VI_V_CURRENT_LINE_REG;
    core.VI_TIMING_REG = &VI_TIMING_REG;
    core.VI_V_SYNC_REG = &VI_V_SYNC_REG;
    core.VI_H_SYNC_REG = &VI_H_SYNC_REG;
    core.VI_LEAP_REG = &VI_LEAP_REG;
    core.VI_H_START_REG = &VI_H_START_REG;
    core.VI_V_START_REG = &VI_V_START_REG;
    core.VI_V_BURST_REG = &VI_V_BURST_REG;
    core.VI_X_SCALE_REG = &VI_X_SCALE_REG;
    core.VI_Y_SCALE_REG = &VI_Y_SCALE_REG;
    core.checkInterrupts = checkInterrupts;

    RT64::ApplicationConfiguration appConfig;
    appConfig.useConfigurationFile = false;
    appConfig.detectDataPath = false;
    g_app = new RT64::Application(core, appConfig);
    g_app->userConfig.graphicsAPI =
        g_opt.vulkan ? RT64::UserConfiguration::GraphicsAPI::Vulkan : RT64::UserConfiguration::GraphicsAPI::D3D12;
    configToRT64(g_app->userConfig);
    if (g_opt.paceLog != nullptr) g_presentTimes = new double[kMaxPresents];
    /* the present thread's hook: --pace-log's times, the settings menu */
    RT64::SetRenderHooks(nullptr, drawHook, deinitHook);
    RT64::Application::SetupResult res = g_app->setup(GetCurrentThreadId());
    if (res != RT64::Application::SetupResult::Success || !g_app->device)
        host_fatal("The renderer (RT64) could not start with %s (setup result %d).\n\n"
                   "Your graphics card or driver may not support it. Try the other graphics API: set\n"
                   "    api = %s\nin bc.ini (or start bc.exe --api %s), and make sure your graphics driver is up to date.",
                   g_opt.vulkan ? "Vulkan" : "Direct3D 12", int(res), g_opt.vulkan ? "d3d12" : "vulkan",
                   g_opt.vulkan ? "d3d12" : "vulkan");
    g_app->swapChain->setVsyncEnabled(g_opt.vsync);
    live::MenuEnv menu;
    menu.app = g_app;
    menu.window = g_window;
    menu.cfg = &g_cfg;
    menu.changed = settingChanged;
    menu.quit = [] { g_quitRequested = true; };
    menu.status = rendererStatus;
    menu.msaaSupported = msaaSupported;
    menu.padNames = padNames;
    menu.packStatus = [] { return g_packStatus; };
    live::menuSetup(menu);
    live::interpTagsSetup(g_app);
    live::interpTagsSetLog(g_interp.log);
    live::interpTagsEnable(g_cfg.framerate != 0 && g_cfg.interpolation == 1);
    if (g_cfg.fpsCounter != 0) counterApply();
    host_crash_rearm();   /* in case the renderer or a driver installed its own filter */
    int ww, wh, pw, ph;
    SDL_GetWindowSize(g_window, &ww, &wh);
    SDL_GetWindowSizeInPixels(g_window, &pw, &ph);
    host_log("live: RT64 up (%s), window %dx%d (%dx%d pixels)%s, vsync %s, display %u Hz\n",
             g_opt.vulkan ? "Vulkan" : "D3D12", ww, wh, pw, ph, g_opt.fullscreen ? " fullscreen" : "",
             g_opt.vsync ? "on" : "off", displayHz());
    if (g_cfg.resolution != 0 || g_cfg.msaa != 0 || g_cfg.filtering != 0 || g_cfg.aspect != 0 ||
        g_cfg.framerate != 0 || g_app->userConfig.msaaSampleCount() > 1)
        host_log("live: video: resolution %s, MSAA %ux%s, filtering %s, aspect %s, frame rate %s\n",
                 live::configValue(g_cfg, {"video", "resolution"}).c_str(), g_app->userConfig.msaaSampleCount(),
                 g_cfg.msaa > 1 && g_app->userConfig.msaaSampleCount() < 2 ? " (not supported here)" : "",
                 live::configValue(g_cfg, {"video", "filtering"}).c_str(),
                 live::configValue(g_cfg, {"video", "aspect"}).c_str(),
                 live::configValue(g_cfg, {"video", "framerate"}).c_str());
    if (g_interp.displayHz != 0) host_log("live: --display-hz %u: RT64 told the display runs at %u Hz\n",
                                          g_interp.displayHz, g_interp.displayHz);
    updateCullWide();
    applyTexturePack();
    o->live = &kLive;
}

/* ---- the process: GUI program, log, config, ROM ----------------------------- */

/* GUI subsystem: stderr exists when the parent handed us one (a console
 * redirect, WSL, a script); otherwise the log goes to bc.log next to the
 * exe (or in %TEMP% if that folder can't be written) */
void setupLog(const std::string &dir) {
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    if (h != nullptr && h != INVALID_HANDLE_VALUE && GetFileType(h) != FILE_TYPE_UNKNOWN) return;
    static std::string path;
    path = dir + "bc.log";
    FILE *f = _wfreopen(live::wide(path).c_str(), L"w", stderr);
    if (f == nullptr) {
        wchar_t tmp[MAX_PATH + 1];
        GetTempPathW(MAX_PATH + 1, tmp);
        path = live::utf8(tmp) + "bc.log";
        f = _wfreopen(live::wide(path).c_str(), L"w", stderr);
    }
    if (f != nullptr) {
        setvbuf(stderr, nullptr, _IONBF, 0);
        host_log_path = path.c_str();
        /* stdout (RT64's messages) into the same open file */
        _dup2(_fileno(stderr), _fileno(stdout));
        setvbuf(stdout, nullptr, _IONBF, 0);
    }
}

/* host_main's hooks */
void optsHook(HostOpts *o) {
    g_opt.vulkan = g_cfg.vulkan;
    g_opt.scale = g_cfg.scale;
    g_opt.fullscreen = g_cfg.fullscreen;
    g_opt.vsync = g_cfg.vsync;
    g_opt.volume = g_cfg.volume;
    g_opt.mute = g_cfg.mute;
    if (!g_cfg.file.empty() && !g_cfg.saves.empty()) {
        const std::string dir = live::absPath(g_cfg, g_cfg.saves);
        g_eepromPath = dir + "\\blastcorps.eep";
        o->eeprom_path = g_eepromPath.c_str();
        if (g_cfg.pak) {
            g_mpkPath = dir + "\\blastcorps.mpk";
            o->mpk_path = g_mpkPath.c_str();
        }
    }
}

void romHook(HostOpts *o) {
    static std::string chosen;
    if (o->rom_path != nullptr) {   /* given on the command line */
        if (g_opt.romCheck) {
            std::string why = live::romCheck(o->rom_path);
            if (!why.empty()) host_fatal("%s", why.c_str());
        }
        return;
    }
    if (!g_cfg.rom.empty()) {
        chosen = live::absPath(g_cfg, g_cfg.rom);
        std::string why = live::romCheck(chosen);
        if (why.empty()) {
            o->rom_path = chosen.c_str();
            return;
        }
        host_log("live: configured ROM: %s\n", why.c_str());
        if (host_no_msgbox) host_fatal("The ROM set in %s can't be used: %s", g_cfg.file.c_str(), why.c_str());
        host_message(("The ROM set in " + g_cfg.file + " can't be used:\n\n" + why + "\n\nPlease choose your ROM.")
                         .c_str(), 0);
    }
    /* --no-msgbox: no file dialog either */
    if (host_no_msgbox) host_fatal("No ROM was given (--no-msgbox: no file dialog).");
    for (;;) {
        chosen = live::romDialog();
        if (chosen.empty())
            host_fatal("Blast Corps needs your own copy of the game: a ROM image of Blast Corps (USA) (Rev 1), "
                       "also known as v1.1, as a .z64, .v64 or .n64 file.\n\n"
                       "No ROM was chosen, so the game will close. Start it again to choose one, or put its path "
                       "in bc.ini (rom = ...).");
        std::string why = live::romCheck(chosen);
        if (why.empty()) break;
        host_log("live: chosen ROM: %s\n", why.c_str());
        host_message((why + "\n\nPlease choose another file.").c_str(), 1);
    }
    o->rom_path = chosen.c_str();
    if (live::configSetRom(g_cfg, chosen)) host_log("live: ROM %s remembered in %s\n", chosen.c_str(), g_cfg.file.c_str());
}

/* an uncaught C++ exception (RT64, the standard library): a crash report */
void terminateHandler() {
    static char why[512];
    const char *what = "not a std::exception";
    try {
        std::exception_ptr e = std::current_exception();
        if (e) std::rethrow_exception(e);
        what = "std::terminate called without an exception";
    } catch (const std::exception &x) {
        snprintf(why, sizeof why, "uncaught C++ exception: %s", x.what());
        host_crash_now(why);
    } catch (...) {
    }
    snprintf(why, sizeof why, "uncaught C++ exception (%s)", what);
    host_crash_now(why);
}

/* SDL_assert_release failures: a crash report (SDL's own handler shows a box) */
SDL_AssertState SDLCALL sdlAssert(const SDL_AssertData *d, void *) {
    static char why[512];
    snprintf(why, sizeof why, "SDL assertion failed: %s (%s:%d, %s)", d->condition, d->filename, d->linenum,
             d->function);
    host_crash_now(why);
}

void crashTestCxx() {
    throw std::runtime_error("crash test (--crash-test cxx)");
}

}  // namespace

int main(int, char **) {
    host_gui = 1;
    /* the command line as UTF-8 (paths with any characters) */
    int argc = 0;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    static std::vector<std::string> args;
    std::vector<char *> argv;
    for (int i = 0; i < argc; i++) args.push_back(live::utf8(wargv[i]));
    for (std::string &s : args) argv.push_back(&s[0]);
    argv.push_back(nullptr);
    host_crash_init(argc, argv.data(), "bc", BC_VERSION);   /* reads --no-msgbox, --crash-dir */
    std::set_terminate(terminateHandler);
    SDL_SetAssertionHandler(sdlAssert, nullptr);
    host_crash_test_cxx = crashTestCxx;
    QueryPerformanceFrequency(&g_freq);
    QueryPerformanceCounter(&g_tStart);

    g_cfg.dir = live::exeDir();
    setupLog(g_cfg.dir);
    /* crash dumps next to the log (bc.log's folder; the exe's when the log is stderr) */
    if (std::find(args.begin(), args.end(), std::string("--crash-dir")) == args.end()) {
        std::string dir = g_cfg.dir;
        if (host_log_path != nullptr) {
            dir = host_log_path;
            dir.resize(dir.find_last_of("\\/") + 1);
        }
        host_crash_set_dir(dir.c_str());
    }
    std::string configFile = g_cfg.dir + "bc.ini";
    bool noConfig = false;
    for (int i = 1; i < argc; i++) {
        if (args[i] == "--no-config") noConfig = true;
        else if (args[i] == "--config" && i + 1 < argc) configFile = live::absPath(g_cfg, args[i + 1]);
    }
    if (noConfig) live::configDefaults(g_cfg);
    else live::configLoad(g_cfg, configFile);
    /* --set section.key=value: a setting for this run (over bc.ini; not saved);
     * --resolution X etc. are short for --set video.resolution=X */
    for (int i = 1; i + 1 < argc; i++) {
        std::string spec;
        if (args[i] == "--set") spec = args[i + 1];
        for (const auto &alias : kSetAliases)
            if (args[i] == alias.first) spec = std::string(alias.second) + "=" + args[i + 1];
        if (spec.empty()) continue;
        if (args[i] == "--texture-pack") g_cfg.texturePackOn = true;
        i++;
        if (!live::configSet(g_cfg, spec) && host_no_msgbox) host_fatal("%s", g_cfg.warnings.back().c_str());
    }
    host_exit_hook = [] {
        if (g_wseq.thread.joinable()) g_wseq.thread.join();
        writePaceLog();
        if (g_dlTasks != 0)
            host_log("live: frame rate option: %.3f ms per task in processDisplayLists (%u tasks, interpolation %s)\n",
                     g_dlSeconds * 1e3 / g_dlTasks, g_dlTasks, live::interpTagsOn() ? "tagged" : "auto");
        live::interpTagsSummary();
    };
    host_opts_hook = optsHook;
    host_rom_hook = romHook;
    return host_main(argc, argv.data(), extraArg, prestart);
}
