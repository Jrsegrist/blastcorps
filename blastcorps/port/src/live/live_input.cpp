/* bc.exe's controller handling (see live_input.h). */
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdio>

#include "live_input.h"

extern "C" {
#include "plat_host.h"
unsigned plat_frames(void); /* save.c (input_test.cpp has its own) */
}

namespace live {

/* ---- controllers ---------------------------------------------------------------- */
static SDL_JoystickID instanceOf(SDL_GameController *p) {
    return SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(p));
}

void PadSet::scan() {
    for (size_t i = 0; i < pads.size();)
        if (pads[i].handle == nullptr || !SDL_GameControllerGetAttached(pads[i].handle)) {
            host_log("live: controller %u removed (%s, id %d; frame %u)\n", unsigned(i + 1), pads[i].name.c_str(),
                     int(pads[i].id), plat_frames());
            if (pads[i].handle != nullptr) SDL_GameControllerClose(pads[i].handle);
            pads.erase(pads.begin() + std::ptrdiff_t(i));
        } else {
            i++;
        }
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (!SDL_IsGameController(i)) continue;
        const SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(i);
        bool have = false;
        for (const PadState &p : pads) have |= p.id == id;
        if (have) continue;
        SDL_GameController *p = SDL_GameControllerOpen(i);
        if (p == nullptr) {
            host_log("live: controller (device %d) can't be opened: %s\n", i, SDL_GetError());
            continue;
        }
        PadState s;
        s.handle = p;
        s.id = instanceOf(p);
        s.name = SDL_GameControllerName(p) ? SDL_GameControllerName(p) : "?";
        pads.push_back(s);
        host_log("live: controller %u: %s (id %d; frame %u)\n", unsigned(pads.size()), s.name.c_str(), int(id),
                 plat_frames());
    }
    sample();
}

void PadSet::sample() {
    for (PadState &p : pads) {
        if (p.handle == nullptr) continue;
        for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; a++) p.axes[a] = SDL_GameControllerGetAxis(p.handle, SDL_GameControllerAxis(a));
        uint32_t b = 0;
        for (int k = 0; k < SDL_CONTROLLER_BUTTON_MAX && k < 32; k++)
            if (SDL_GameControllerGetButton(p.handle, SDL_GameControllerButton(k))) b |= 1u << k;
        p.buttons = b;
    }
}

void PadSet::closeAll() {
    for (PadState &p : pads)
        if (p.handle != nullptr) SDL_GameControllerClose(p.handle);
    pads.clear();
}

PadSet PadSet::copy() const {
    PadSet s = *this;
    for (PadState &p : s.pads) p.handle = nullptr;
    return s;
}

std::vector<const PadState *> PadSet::active(const Config &c) const {
    std::vector<const PadState *> out;
    if (c.padIndex < 0) {
        for (const PadState &p : pads) out.push_back(&p);
    } else if (size_t(c.padIndex) < pads.size()) {
        out.push_back(&pads[size_t(c.padIndex)]);
    }
    return out;
}

bool PadSet::isActive(const Config &c, SDL_JoystickID which) const {
    for (const PadState *p : active(c))
        if (p->id == which) return true;
    return false;
}

std::vector<std::string> PadSet::names() const {
    std::vector<std::string> out;
    for (const PadState &p : pads) out.push_back(p.name);
    return out;
}

/* ---- the controllers' thread ------------------------------------------------------- */
bool InputThread::start(std::string &error) {
    wake_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    quit_ = false;
    thread_ = std::thread([this] { run(); });
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return ready_ || failed_; });
    if (failed_) {
        error = error_;
        lock.unlock();
        thread_.join();
        return false;
    }
    return true;
}

void InputThread::stop() {
    if (!thread_.joinable()) return;
    quit_ = true;
    SetEvent(static_cast<HANDLE>(wake_));
    thread_.join();
    CloseHandle(static_cast<HANDLE>(wake_));
    wake_ = nullptr;
}

PadSet InputThread::latest() {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_;
}

unsigned InputThread::version() {
    std::lock_guard<std::mutex> lock(mutex_);
    return version_;
}

bool InputThread::waitNewer(unsigned v, int ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, std::chrono::milliseconds(ms), [&] { return version_ > v; });
}

InputThread::Stats InputThread::take() {
    std::lock_guard<std::mutex> lock(mutex_);
    Stats s = stats_;
    stats_.updates = 0, stats_.maxMs = 0, stats_.slow = 0;
    return s;
}

void InputThread::run() {
    LARGE_INTEGER freq, t0, t1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);
    SetThreadDescription(GetCurrentThread(), L"bc controllers");
    PadSet pads;
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        failed_ = true;
        error_ = SDL_GetError();
        cv_.notify_all();
        return;
    }
    pads.scan();
    QueryPerformanceCounter(&t1);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.initMs = double(t1.QuadPart - t0.QuadPart) * 1e3 / double(freq.QuadPart);
        latest_ = pads.copy();
        version_++;
        ready_ = true;
        cv_.notify_all();
    }
    unsigned n = 0;
    while (!quit_) {
        /* about every millisecond, or at once when Windows has messages for SDL's device windows (Raw Input's
         * reports and device changes) */
        HANDLE h = static_cast<HANDLE>(wake_);
        MsgWaitForMultipleObjects(1, &h, FALSE, 1, QS_ALLINPUT);
        if (quit_) break;
        QueryPerformanceCounter(&t0);
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        SDL_JoystickUpdate();   /* the drivers' updates and device detection */
        if (testHook) {
            SDL_LockJoysticks();
            testHook();
            SDL_UnlockJoysticks();
        }
        if (++n % 16 == 0) pads.scan();   /* (devices that came or went: ~16 ms) */
        else pads.sample();
        QueryPerformanceCounter(&t1);
        const double ms = double(t1.QuadPart - t0.QuadPart) * 1e3 / double(freq.QuadPart);
        std::lock_guard<std::mutex> lock(mutex_);
        latest_ = pads.copy();
        version_++;
        stats_.updates++;
        stats_.maxMs = std::max(stats_.maxMs, ms);
        stats_.slow += ms > kSlowMs;
        cv_.notify_all();
    }
    pads.closeAll();
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

/* ---- the N64 pad ------------------------------------------------------------------- */
/* an axis as an N64 stick value (-80..80 at full tilt), 0 inside the dead
 * zone; the stick sensitivity scales it (100 %: the full N64 range) */
static int axis(const Config &c, const PadState *pad, int a, bool stick) {
    int v = pad->axis(a);
    if (v > -c.deadzone && v < c.deadzone) return 0;
    if (!stick || c.sensitivity == 100) return v * 80 / 32767;
    return int(double(v) * 80.0 * c.sensitivity / (100.0 * 32767.0));
}

static bool padActive(const Config &c, const PadState *pad, const PadInput &in) {
    if (in.button >= 0) return pad->button(in.button);
    if (in.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || in.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return pad->axis(in.axis) > 12000;
    return axis(c, pad, in.axis, false) * in.dir > 40;
}

N64Pad readPad(const Config &c, const PadSet &pads, const Uint8 *k, bool altHeld) {
    /* Alt+Enter switches fullscreen: that Enter isn't a game key */
    const bool altEnter = k != nullptr && altHeld && k[SDL_SCANCODE_RETURN];
    auto keyDown = [&](int act) {
        if (k == nullptr) return false;
        for (SDL_Scancode sc : c.keys[act])
            if (k[sc] && !(altEnter && sc == SDL_SCANCODE_RETURN)) return true;
        return false;
    };
    unsigned b = 0;
    int x = 0, y = 0;
    for (int i = 0; i < kButtons; i++)
        if (keyDown(i)) b |= kButtonBits[i];
    if (keyDown(A_STICK_LEFT)) x -= 80;
    if (keyDown(A_STICK_RIGHT)) x += 80;
    if (keyDown(A_STICK_UP)) y += 80;
    if (keyDown(A_STICK_DOWN)) y -= 80;
    for (const PadState *pad : pads.active(c)) {
        for (int i = 0; i < kButtons; i++)
            for (const PadInput &in : c.pad[i])
                if (padActive(c, pad, in)) b |= kButtonBits[i];
        if (c.padStick >= 0) {
            const bool right = c.padStick == 1;
            int lx = axis(c, pad, right ? SDL_CONTROLLER_AXIS_RIGHTX : SDL_CONTROLLER_AXIS_LEFTX, true);
            int ly = -axis(c, pad, right ? SDL_CONTROLLER_AXIS_RIGHTY : SDL_CONTROLLER_AXIS_LEFTY, true);
            if (lx != 0 || ly != 0) x = lx, y = ly;
        }
    }
    N64Pad p;
    p.button = (unsigned short) b;
    p.x = std::max(-80, std::min(80, x));
    p.y = std::max(-80, std::min(80, y));
    return p;
}

bool comboHeld(const Config &c, const PadSet &pads, const std::vector<int> &buttons) {
    if (buttons.empty()) return false;
    for (const PadState *pad : pads.active(c)) {
        bool all = true;
        for (int b : buttons) all &= pad->button(b);
        if (all) return true;
    }
    return false;
}

bool menuComboHeld(const Config &c, const PadSet &pads) {
    return comboHeld(c, pads, c.menuPad);
}

/* ---- the gate after the menu ------------------------------------------------------- */
std::string buttonNames(unsigned short b) {
    std::string s;
    for (int i = 0; i < kButtons; i++)
        if (b & kButtonBits[i]) s += (s.empty() ? "" : "+") + std::string(kActionLabels[i]);
    return s.empty() ? "-" : s;
}

void InputGate::close(const N64Pad &now) {
    buttons = now.button;
    stick = now.x != 0 || now.y != 0;
    polls = 0;
    if (closed())
        host_log("live: input gate closed: held %s%s (frame %u)\n", buttonNames(buttons).c_str(),
                 stick ? ", stick" : "", plat_frames());
}

void InputGate::apply(N64Pad &p) {
    if (!closed()) return;
    buttons &= p.button;                /* a let-go button passes from now on */
    if (stick && p.x == 0 && p.y == 0) stick = false;
    if (++polls >= kMaxPolls && closed()) {
        host_log("live: input gate opened after %u reads with %s%s still held (frame %u)\n", polls,
                 buttonNames(buttons).c_str(), stick ? ", stick" : "", plat_frames());
        buttons = 0;
        stick = false;
        return;
    }
    p.button &= (unsigned short) ~buttons;
    if (stick) p.x = p.y = 0;
    if (!closed()) host_log("live: input gate opened (everything let go after %u reads, frame %u)\n", polls, plat_frames());
}

}  // namespace live
