/* bc.exe: the windowed build.  The game logic and the platform layer are
 * bc_headless's (port/src/platform/); this file adds what a player sees and
 * does (HostLive, plat_host.h):
 *  - a window (SDL2) and the RT64 renderer (port/rt64/: RT64 built with
 *    RT64_NATIVE_HOST_LAYOUT, RDRAM = this process's memory at 0x80000000);
 *  - every graphics task the game starts goes to RT64 (loadUCodeGBI +
 *    processDisplayLists); func_802A4B0C's visibility test stays with the
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
 * (Rev 1) by SHA-1, .z64/.v64/.n64) and remembered.  bc.exe is a Windows
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
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_syswm.h>

#include "hle/rt64_application.h"
#include "rhi/rt64_render_hooks.h"

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

/* ---- input ------------------------------------------------------------------- */
SDL_GameController *g_pad;
unsigned short g_button;
signed char g_stickX, g_stickY;

void openPad() {
    if (g_pad != nullptr) return;
    for (int i = 0; i < SDL_NumJoysticks(); i++)
        if (SDL_IsGameController(i) && (g_pad = SDL_GameControllerOpen(i)) != nullptr) {
            host_log("live: controller: %s\n", SDL_GameControllerName(g_pad));
            return;
        }
}

/* an axis as an N64 stick value (-80..80), 0 inside the dead zone */
int axis(int a) {
    int v = SDL_GameControllerGetAxis(g_pad, SDL_GameControllerAxis(a));
    if (v > -g_cfg.deadzone && v < g_cfg.deadzone) return 0;
    return v * 80 / 32767;
}

bool padActive(const live::PadInput &in) {
    if (in.button >= 0) return SDL_GameControllerGetButton(g_pad, SDL_GameControllerButton(in.button)) != 0;
    if (in.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || in.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return SDL_GameControllerGetAxis(g_pad, SDL_GameControllerAxis(in.axis)) > 12000;
    return axis(in.axis) * in.dir > 40;
}

/* N64 pad from the keyboard and the first game controller, by the bindings
 * in bc.ini (live_setup.cpp has the defaults):
 *   stick: arrows (keyboard), left stick       A: X / gamepad A
 *   B: C / gamepad B or X                      Z: Z or Space / either trigger
 *   START: Enter / Start                       L, R: A, S / shoulders
 *   C buttons: I J K L / right stick           D-pad: T F G H / D-pad */
void pollInput() {
    const Uint8 *k = SDL_GetKeyboardState(nullptr);
    /* Alt+Enter switches fullscreen: that Enter isn't a game key */
    const bool altEnter = (SDL_GetModState() & KMOD_ALT) != 0 && k[SDL_SCANCODE_RETURN];
    auto keyDown = [&](int act) {
        for (SDL_Scancode sc : g_cfg.keys[act])
            if (k[sc] && !(altEnter && sc == SDL_SCANCODE_RETURN)) return true;
        return false;
    };
    unsigned b = 0;
    int x = 0, y = 0;
    for (int i = 0; i < live::kButtons; i++)
        if (keyDown(i)) b |= live::kButtonBits[i];
    if (keyDown(live::A_STICK_LEFT)) x -= 80;
    if (keyDown(live::A_STICK_RIGHT)) x += 80;
    if (keyDown(live::A_STICK_UP)) y += 80;
    if (keyDown(live::A_STICK_DOWN)) y -= 80;
    if (g_pad != nullptr) {
        for (int i = 0; i < live::kButtons; i++)
            for (const live::PadInput &in : g_cfg.pad[i])
                if (padActive(in)) b |= live::kButtonBits[i];
        if (g_cfg.padStick >= 0) {
            const bool right = g_cfg.padStick == 1;
            int lx = axis(right ? SDL_CONTROLLER_AXIS_RIGHTX : SDL_CONTROLLER_AXIS_LEFTX);
            int ly = -axis(right ? SDL_CONTROLLER_AXIS_RIGHTY : SDL_CONTROLLER_AXIS_LEFTY);
            if (lx != 0 || ly != 0) x = lx, y = ly;
        }
    }
    g_button = (unsigned short) b;
    g_stickX = (signed char) std::max(-80, std::min(80, x));
    g_stickY = (signed char) std::max(-80, std::min(80, y));
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
    if (g_app != nullptr) g_app->end();
    if (g_window != nullptr) SDL_DestroyWindow(g_window);
    SDL_Quit();
    host_exit(code);
}

const char kTitle[] = "Blast Corps  (F1: controls)";
bool g_helpShown;

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

bool isQuitKey(SDL_Scancode sc) {
    for (SDL_Scancode q : g_cfg.keys[live::A_QUIT])
        if (q == sc) return true;
    return false;
}

void pumpEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && isQuitKey(e.key.keysym.scancode))) shutdown(0);
        if (e.type == SDL_CONTROLLERBUTTONDOWN && g_pad != nullptr)
            for (const live::PadInput &in : g_cfg.pad[live::A_QUIT])
                if (in.button == e.cbutton.button) shutdown(0);
        if (e.type == SDL_KEYDOWN && !e.key.repeat) {
            const SDL_Scancode sc = e.key.keysym.scancode;
            if (sc == SDL_SCANCODE_F11 || (sc == SDL_SCANCODE_RETURN && (e.key.keysym.mod & KMOD_ALT) != 0)) {
                setFullscreen(!g_opt.fullscreen);
            } else if (sc == SDL_SCANCODE_F1) {
                g_helpShown = !g_helpShown;
                SDL_SetWindowTitle(g_window, g_helpShown ? live::helpLine(g_cfg).c_str() : kTitle);
            } else if (e.key.keysym.scancode == SDL_SCANCODE_M) {
                g_opt.mute = !g_opt.mute;
                host_log("live: sound %s\n", g_opt.mute ? "off" : "on");
            } else if (e.key.keysym.scancode == SDL_SCANCODE_MINUS || e.key.keysym.scancode == SDL_SCANCODE_EQUALS) {
                g_opt.volume = std::max(0, std::min(100, g_opt.volume + (e.key.keysym.scancode == SDL_SCANCODE_MINUS ? -10 : 10)));
                host_log("live: volume %d%%\n", g_opt.volume);
            }
        }
        if (e.type == SDL_CONTROLLERDEVICEADDED) openPad();
        if (e.type == SDL_CONTROLLERDEVICEREMOVED && g_pad != nullptr &&
            !SDL_GameControllerGetAttached(g_pad)) {
            SDL_GameControllerClose(g_pad);
            g_pad = nullptr;
        }
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
    g_app->processDisplayLists(rdramBase(), dataPtr & 0x3FFFFFF, 0, true);
    for (auto &s : g_skipped) *s.first = s.second;
    g_skipped.clear();
}

unsigned long long g_when0;
unsigned g_nextShot;

void liveVi(const HostViRegs *r, unsigned viCount, unsigned long long when, unsigned frames) {
    (void) viCount;
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
    const double before = g_opt.paceLog != nullptr ? secondsSince(g_tStart) : 0.0;
    g_app->updateScreen();
    if (g_opt.paceLog != nullptr) {
        const double after = secondsSince(g_tStart);
        const double due = g_t0.QuadPart == 0 ? 0.0
            : double(g_t0.QuadPart - g_tStart.QuadPart) / double(g_freq.QuadPart) + double(when - g_when0) / 46875000.0;
        g_pace.push_back({before, due, after - before, frames});
    }
    pumpEvents();
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

/* ---- start-up ---------------------------------------------------------------- */
unsigned num(const char *s) {
    return unsigned(strtoul(s, nullptr, 0));
}

int extraArg(int argc, char **argv, int *i, HostOpts *o) {
    const char *a = argv[*i];
    auto next = [&]() -> const char * {
        if (*i + 1 >= argc) host_fatal("%s needs a value", a);
        return argv[++*i];
    };
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
    g_window = SDL_CreateWindow(kTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, int(320 * scale),
                                int(240 * scale), flags);
    if (g_window == nullptr) host_fatal("Can't create the game window: %s", SDL_GetError());
    SDL_SetWindowMinimumSize(g_window, 320, 240);
    if (g_opt.fullscreen) SDL_ShowCursor(SDL_DISABLE);
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    if (!SDL_GetWindowWMInfo(g_window, &wm)) host_fatal("SDL_GetWindowWMInfo: %s", SDL_GetError());
    host_gui_window = wm.info.win.window;
    openPad();

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
    if (g_opt.paceLog != nullptr) {
        g_presentTimes = new double[kMaxPresents];
        RT64::SetRenderHooks(nullptr, presentHook, nullptr);
    }
    RT64::Application::SetupResult res = g_app->setup(GetCurrentThreadId());
    if (res != RT64::Application::SetupResult::Success || !g_app->device)
        host_fatal("The renderer (RT64) could not start with %s (setup result %d).\n\n"
                   "Your graphics card or driver may not support it. Try the other graphics API: set\n"
                   "    api = %s\nin bc.ini (or start bc.exe --api %s), and make sure your graphics driver is up to date.",
                   g_opt.vulkan ? "Vulkan" : "Direct3D 12", int(res), g_opt.vulkan ? "d3d12" : "vulkan",
                   g_opt.vulkan ? "d3d12" : "vulkan");
    g_app->swapChain->setVsyncEnabled(g_opt.vsync);
    host_crash_rearm();   /* in case the renderer or a driver installed its own filter */
    int ww, wh, pw, ph;
    SDL_GetWindowSize(g_window, &ww, &wh);
    SDL_GetWindowSizeInPixels(g_window, &pw, &ph);
    host_log("live: RT64 up (%s), window %dx%d (%dx%d pixels)%s, vsync %s, display %u Hz\n",
             g_opt.vulkan ? "Vulkan" : "D3D12", ww, wh, pw, ph, g_opt.fullscreen ? " fullscreen" : "",
             g_opt.vsync ? "on" : "off", displayHz());
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
    host_exit_hook = writePaceLog;
    host_opts_hook = optsHook;
    host_rom_hook = romHook;
    return host_main(argc, argv.data(), extraArg, prestart);
}
