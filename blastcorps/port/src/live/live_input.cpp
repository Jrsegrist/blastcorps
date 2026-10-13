/* bc.exe's controller handling (see live_input.h). */
#include <algorithm>
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
        if (!SDL_GameControllerGetAttached(pads[i])) {
            host_log("live: controller %u removed (%s, id %d; frame %u)\n", unsigned(i + 1),
                     SDL_GameControllerName(pads[i]) ? SDL_GameControllerName(pads[i]) : "?", int(instanceOf(pads[i])),
                     plat_frames());
            SDL_GameControllerClose(pads[i]);
            pads.erase(pads.begin() + std::ptrdiff_t(i));
        } else {
            i++;
        }
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (!SDL_IsGameController(i)) continue;
        const SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(i);
        bool have = false;
        for (SDL_GameController *p : pads) have |= instanceOf(p) == id;
        if (have) continue;
        SDL_GameController *p = SDL_GameControllerOpen(i);
        if (p == nullptr) {
            host_log("live: controller (device %d) can't be opened: %s\n", i, SDL_GetError());
            continue;
        }
        pads.push_back(p);
        host_log("live: controller %u: %s (id %d; frame %u)\n", unsigned(pads.size()),
                 SDL_GameControllerName(p) ? SDL_GameControllerName(p) : "?", int(id), plat_frames());
    }
}

void PadSet::closeAll() {
    for (SDL_GameController *p : pads) SDL_GameControllerClose(p);
    pads.clear();
}

std::vector<SDL_GameController *> PadSet::active(const Config &c) const {
    if (c.padIndex < 0) return pads;
    if (size_t(c.padIndex) < pads.size()) return {pads[size_t(c.padIndex)]};
    return {};
}

bool PadSet::isActive(const Config &c, SDL_JoystickID which) const {
    for (SDL_GameController *p : active(c))
        if (instanceOf(p) == which) return true;
    return false;
}

std::vector<std::string> PadSet::names() const {
    std::vector<std::string> out;
    for (SDL_GameController *p : pads) out.push_back(SDL_GameControllerName(p) ? SDL_GameControllerName(p) : "?");
    return out;
}

/* ---- the N64 pad ------------------------------------------------------------------- */
/* an axis as an N64 stick value (-80..80 at full tilt), 0 inside the dead
 * zone; the stick sensitivity scales it (100 %: the full N64 range) */
static int axis(const Config &c, SDL_GameController *pad, int a, bool stick) {
    int v = SDL_GameControllerGetAxis(pad, SDL_GameControllerAxis(a));
    if (v > -c.deadzone && v < c.deadzone) return 0;
    if (!stick || c.sensitivity == 100) return v * 80 / 32767;
    return int(double(v) * 80.0 * c.sensitivity / (100.0 * 32767.0));
}

static bool padActive(const Config &c, SDL_GameController *pad, const PadInput &in) {
    if (in.button >= 0) return SDL_GameControllerGetButton(pad, SDL_GameControllerButton(in.button)) != 0;
    if (in.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || in.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return SDL_GameControllerGetAxis(pad, SDL_GameControllerAxis(in.axis)) > 12000;
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
    for (SDL_GameController *pad : pads.active(c)) {
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
    for (SDL_GameController *pad : pads.active(c)) {
        bool all = true;
        for (int b : buttons) all &= SDL_GameControllerGetButton(pad, SDL_GameControllerButton(b)) != 0;
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
