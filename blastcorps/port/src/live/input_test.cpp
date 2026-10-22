/* bc_inputtest.exe: tests of bc.exe's controller handling (live_input.cpp)
 * with SDL's virtual game controllers: no window, no ROM, no renderer.
 *   - a controller is found, its buttons and stick make the N64 pad;
 *   - unplugged and plugged in again (a new SDL instance id): `pad = 1`
 *     follows it, also when the device events are missed (rescan only);
 *   - the input gate after the settings menu: each held input is blocked
 *     only until it is let go, new inputs pass at once, and nothing held
 *     (a stuck key, a drifting stick, a resting trigger) keeps it closed
 *     longer than InputGate::kMaxPolls reads;
 *   - the keyboard bindings, the menu's button combination;
 *   - bc.ini settings (live_setup.cpp): parsing, defaults, and saving a
 *     menu change and reading it back (files kept in memory).
 * Exit status 0 if every check passes. */
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include <SDL.h>

#include "live_input.h"
#include "live_setup.h"

/* files in memory (the bc.ini tests: configLoad / configSave) */
static std::map<std::string, std::string> &files() {
    static std::map<std::string, std::string> m;
    return m;
}

/* the platform services live_input.cpp / live_setup.cpp use */
extern "C" {
static unsigned g_frame;
unsigned plat_frames(void) { return g_frame; }
void host_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("  log: ", stdout);
    vprintf(fmt, ap);
    va_end(ap);
}
void *host_read_file(const char *path, unsigned *size) {
    auto it = files().find(path);
    if (it == files().end()) return nullptr;
    char *p = static_cast<char *>(malloc(it->second.size() + 1));
    memcpy(p, it->second.data(), it->second.size());
    *size = unsigned(it->second.size());
    return p;
}
int host_write_file(const char *path, const void *data, unsigned size) {
    files()[path].assign(static_cast<const char *>(data), size);
    return 0;
}
int rom_normalise(unsigned char *, size_t) { return -1; }
}

namespace {

int g_fail, g_pass;

void check(bool ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    (ok ? g_pass : g_fail)++;
}

/* a virtual game controller: SDL maps it like an Xbox pad */
int attach() {
    SDL_VirtualJoystickDesc d;
    SDL_zero(d);
    d.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
    d.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
    d.naxes = SDL_CONTROLLER_AXIS_MAX;
    d.nbuttons = SDL_CONTROLLER_BUTTON_MAX;
    d.name = "virtual test pad";
    return SDL_JoystickAttachVirtualEx(&d);
}

/* (a virtual joystick's trigger axis rests at -32768: the controller API maps it to 0) */
const Sint16 kTriggerRest = -32768;

SDL_Joystick *joyOf(live::PadSet &s, size_t i) {
    SDL_Joystick *j = i < s.pads.size() ? SDL_GameControllerGetJoystick(s.pads[i]) : nullptr;
    static SDL_Joystick *seen[16];
    bool known = false;
    for (SDL_Joystick *k : seen) known |= k == j;
    if (j != nullptr && !known) {   /* a new one: triggers at rest */
        SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_TRIGGERLEFT, kTriggerRest);
        SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, kTriggerRest);
        for (SDL_Joystick *&k : seen)
            if (k == nullptr) {
                k = j;
                break;
            }
    }
    return j;
}

void pump() {
    SDL_Event e;
    SDL_PumpEvents();
    while (SDL_PollEvent(&e)) {
    }
}

}  // namespace

int main(int, char **) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    /* only the virtual controllers: no real pad plugged into this PC */
    for (const char *h : {SDL_HINT_XINPUT_ENABLED, SDL_HINT_JOYSTICK_RAWINPUT, SDL_HINT_JOYSTICK_HIDAPI,
                          "SDL_JOYSTICK_WGI", "SDL_DIRECTINPUT_ENABLED"})
        SDL_SetHint(h, "0");
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        printf("FAIL SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    live::Config c;
    live::configDefaults(c);

    /* video.framerate (the frame rate option): parsing and the names written back */
    {
        live::Config f;
        live::configDefaults(f);
        check(f.framerate == 0 && live::configValue(f, {"video", "framerate"}) == "original", "framerate default: original");
        check(live::configSet(f, "video.framerate=144") && f.framerate == 144 &&
                  live::configValue(f, {"video", "framerate"}) == "144", "framerate 144");
        check(live::configSet(f, "video.framerate=display") && f.framerate == -1 &&
                  live::configValue(f, {"video", "framerate"}) == "display", "framerate display");
        check(live::configSet(f, "video.framerate=original") && f.framerate == 0, "framerate original");
        check(!live::configSet(f, "video.framerate=10") && f.framerate == 0, "framerate 10 rejected (below 30)");
        check(!live::configSet(f, "video.framerate=fast") && f.framerate == 0, "framerate 'fast' rejected");
    }
    /* video.hud_position: centered (the default) or edges */
    {
        live::Config f;
        live::configDefaults(f);
        check(f.hudPosition == 0 && live::configValue(f, {"video", "hud_position"}) == "centered",
              "hud position default: centered");
        check(live::configSet(f, "video.hud_position=edges") && f.hudPosition == 1 &&
                  live::configValue(f, {"video", "hud_position"}) == "edges", "hud position edges");
        check(live::configSet(f, "video.hud_position=centred") && f.hudPosition == 0, "hud position centred");
        check(!live::configSet(f, "video.hud_position=top") && f.hudPosition == 0, "hud position 'top' rejected");
    }
    /* video.interpolation, the frame-rate counter and its bindings */
    {
        live::Config f;
        live::configDefaults(f);
        check(f.interpolation == 1 && live::configValue(f, {"video", "interpolation"}) == "tagged",
              "interpolation default: tagged");
        check(live::configSet(f, "video.interpolation=auto") && f.interpolation == 0 &&
                  live::configValue(f, {"video", "interpolation"}) == "auto", "interpolation auto");
        check(!live::configSet(f, "video.interpolation=maybe") && f.interpolation == 0, "interpolation 'maybe' rejected");
        check(f.fpsCounter == 0 && live::configValue(f, {"video", "fps_counter"}) == "off", "fps counter default: off");
        check(live::configSet(f, "video.fps_counter=detailed") && f.fpsCounter == 2 &&
                  live::configValue(f, {"video", "fps_counter"}) == "detailed", "fps counter detailed");
        check(live::configSet(f, "video.fps_counter=on") && f.fpsCounter == 1, "fps counter on");
        check(f.fpsCorner == 0 && live::configSet(f, "video.fps_counter_corner=bottom-right") && f.fpsCorner == 3 &&
                  live::configValue(f, {"video", "fps_counter_corner"}) == "bottom-right", "fps counter corner");
        check(!live::configSet(f, "video.fps_counter_corner=middle") && f.fpsCorner == 3, "corner 'middle' rejected");
        check(f.keys[live::A_FPS].size() == 1 && f.keys[live::A_FPS][0] == SDL_SCANCODE_F2, "fps counter key F2");
        check(f.fpsPad.empty() && live::configSet(f, "controller.fps_counter=back+y") && f.fpsPad.size() == 2 &&
                  live::configValue(f, {"controller", "fps_counter"}) == "back+y", "fps counter buttons back+y");
        check(f.menuPad.size() == 2, "menu buttons untouched");
    }
    /* video.present_pacing: timer (the default) or vsync; bc.ini without the key
     * (one from before it existed) gives timer; the menu's change is saved and
     * read back */
    {
        live::Config f;
        live::configDefaults(f);
        check(!f.vsyncPacing && live::configValue(f, {"video", "present_pacing"}) == "timer",
              "present pacing default: timer");
        check(live::configSet(f, "video.present_pacing=vsync") && f.vsyncPacing &&
                  live::configValue(f, {"video", "present_pacing"}) == "vsync", "present pacing vsync");
        check(live::configSet(f, "video.present_pacing=Timer") && !f.vsyncPacing, "present pacing timer");
        check(!live::configSet(f, "video.present_pacing=fast") && !f.vsyncPacing, "present pacing 'fast' rejected");
        check(live::configSet(f, "video.present_pacing=VRR") && !f.vsyncPacing && f.pacingOther == 2 &&
                  live::configValue(f, {"video", "present_pacing"}) == "vrr", "present pacing vrr");
        check(live::configSet(f, "video.present_pacing=auto") && f.pacingOther == 3 &&
                  live::configValue(f, {"video", "present_pacing"}) == "auto", "present pacing auto");
        check(live::configSet(f, "video.present_pacing=vsync") && f.vsyncPacing && f.pacingOther == 0 &&
                  live::configSet(f, "video.present_pacing=timer") && !f.vsyncPacing && f.pacingOther == 0,
              "present pacing back to vsync, then timer");

        const std::string old = "old.ini";   /* no present_pacing line */
        files()[old] = "[game]\r\nrom = x.z64\r\n\r\n[video]\r\napi = d3d12\r\nvsync = 1\r\n"
                       "framerate = 60\r\n\r\n[audio]\r\nvolume = 80\r\n";
        live::Config g;
        check(live::configLoad(g, old) && g.warnings.empty() && !g.vsyncPacing && g.framerate == 60,
              "bc.ini without present_pacing: timer, no warnings");
        g.vsyncPacing = true;   /* the menu's choice */
        check(live::configSave(g, {{"video", "present_pacing"}}), "menu change saved");
        const std::string &t = files()[old];
        check(t == "[game]\r\nrom = x.z64\r\n\r\n[video]\r\napi = d3d12\r\nvsync = 1\r\n"
                   "framerate = 60\r\npresent_pacing = vsync\r\n\r\n[audio]\r\nvolume = 80\r\n",
              "saved at the end of [video] (before its blank line), the rest kept");
        live::Config h;
        check(live::configLoad(h, old) && h.warnings.empty() && h.vsyncPacing && h.framerate == 60,
              "read back: vsync");
        h.vsyncPacing = false;
        live::configSave(h, {{"video", "present_pacing"}});
        live::Config k;
        check(live::configLoad(k, old) && !k.vsyncPacing &&
                  files()[old].find("present_pacing = timer\r\n") != std::string::npos &&
                  files()[old].find("present_pacing = vsync") == std::string::npos,
              "changed back to timer: the same line rewritten");

        const std::string fresh = "new.ini";   /* no file: created with the defaults */
        live::Config n;
        check(live::configLoad(n, fresh) && !n.vsyncPacing &&
                  files()[fresh].find("present_pacing = timer\r\n") != std::string::npos,
              "new bc.ini: present_pacing = timer");
    }
    live::PadSet pads;
    const unsigned short A = live::kButtonBits[live::A_A], START = live::kButtonBits[live::A_START],
                         Z = live::kButtonBits[live::A_Z];

    /* a controller appears and plays */
    int dev = attach();
    check(dev >= 0, "virtual controller attached");
    pump();
    pads.scan();
    check(pads.pads.size() == 1, "scan opens it");
    SDL_Joystick *j = joyOf(pads, 0);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 1);
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_LEFTX, 32767);
    pump();
    live::N64Pad p = live::readPad(c, pads, nullptr, false);
    check(p.button == A && p.x == 80 && p.y == 0, "A + stick right reach the N64 pad");
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 32767);
    pump();
    p = live::readPad(c, pads, nullptr, false);
    check((p.button & Z) != 0, "right trigger = Z");
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, kTriggerRest);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 0);
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_LEFTX, 0);
    pump();
    check(live::readPad(c, pads, nullptr, false).idle(), "released: idle");

    /* menu combination (Back+Start) */
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_BACK, 1);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_START, 1);
    pump();
    check(live::menuComboHeld(c, pads), "Back+Start held: menu combination");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_BACK, 0);
    pump();
    check(!live::menuComboHeld(c, pads), "Start alone: not the combination");

    /* the gate: Start held when the menu closed */
    live::InputGate gate;
    gate.close(live::readPad(c, pads, nullptr, false));
    check(gate.closed() && gate.buttons == START, "gate closed on the held Start");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 1);
    pump();
    p = live::readPad(c, pads, nullptr, false);
    gate.apply(p);
    check(p.button == A, "while Start is held: A passes, Start is blocked");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_START, 0);
    pump();
    p = live::readPad(c, pads, nullptr, false);
    gate.apply(p);
    check(!gate.closed() && p.button == A, "Start let go: gate open");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_START, 1);
    pump();
    p = live::readPad(c, pads, nullptr, false);
    gate.apply(p);
    check((p.button & START) != 0, "Start pressed again: passes (the gate doesn't re-arm)");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_START, 0);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 0);

    /* a stuck input (drifting stick past the dead zone, a held button) */
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_LEFTY, 20000);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_B, 1);
    pump();
    gate.close(live::readPad(c, pads, nullptr, false));
    unsigned polls = 0;
    while (gate.closed() && polls < 1000) {
        p = live::readPad(c, pads, nullptr, false);
        gate.apply(p);
        polls++;
    }
    check(polls == live::InputGate::kMaxPolls, "stuck stick + button: gate opens after kMaxPolls reads");
    p = live::readPad(c, pads, nullptr, false);
    gate.apply(p);
    check(p.y != 0 && p.button != 0, "... and then they reach the game");
    SDL_JoystickSetVirtualAxis(j, SDL_CONTROLLER_AXIS_LEFTY, 0);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_B, 0);
    pump();

    /* unplugged and plugged in again: a new instance id; pad = 1 follows */
    const SDL_JoystickID oldId = SDL_JoystickInstanceID(j);
    SDL_JoystickDetachVirtual(dev);
    pump();
    pads.scan();
    check(pads.pads.empty(), "unplugged: closed");
    check(live::readPad(c, pads, nullptr, false).idle(), "unplugged: idle pad");
    dev = attach();
    pump();
    pads.scan();
    check(pads.pads.size() == 1 && SDL_JoystickInstanceID(joyOf(pads, 0)) != oldId, "plugged in again: reopened (new id)");
    j = joyOf(pads, 0);
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 1);
    pump();
    check(live::readPad(c, pads, nullptr, false).button == A, "plugged in again: A works");
    check(pads.isActive(c, SDL_JoystickInstanceID(j)), "its events count as the playing controller's");

    /* unplug + replug between two scans (the device events missed): one rescan fixes it */
    SDL_JoystickDetachVirtual(dev);
    dev = attach();
    pump();
    pads.scan();
    j = joyOf(pads, 0);
    check(pads.pads.size() == 1 && j != nullptr, "replug between scans: one controller again");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 1);
    pump();
    check(live::readPad(c, pads, nullptr, false).button == A, "replug between scans: A works");
    SDL_JoystickSetVirtualButton(j, SDL_CONTROLLER_BUTTON_A, 0);

    /* two controllers: pad = 1 is the first, pad = 2 the second, any = both */
    int dev2 = attach();
    pump();
    pads.scan();
    check(pads.pads.size() == 2, "a second controller is opened");
    SDL_JoystickSetVirtualButton(joyOf(pads, 1), SDL_CONTROLLER_BUTTON_A, 1);
    pump();
    check(live::readPad(c, pads, nullptr, false).button == 0, "pad = 1: the second one doesn't play");
    c.padIndex = 1;
    check(live::readPad(c, pads, nullptr, false).button == A, "pad = 2: it does");
    c.padIndex = -1;
    check(live::readPad(c, pads, nullptr, false).button == A, "pad = any: it does");
    c.padIndex = 0;
    SDL_JoystickDetachVirtual(dev2);

    /* the keyboard */
    Uint8 keys[SDL_NUM_SCANCODES] = {0};
    keys[SDL_SCANCODE_X] = 1;
    keys[SDL_SCANCODE_UP] = 1;
    p = live::readPad(c, pads, keys, false);
    check(p.button == A && p.y == 80, "keyboard X + Up");
    keys[SDL_SCANCODE_X] = 0;
    keys[SDL_SCANCODE_RETURN] = 1;
    check((live::readPad(c, pads, keys, true).button & START) == 0, "Alt+Enter isn't START");
    check((live::readPad(c, pads, keys, false).button & START) != 0, "Enter is START");

    pads.closeAll();
    SDL_JoystickDetachVirtual(dev);
    SDL_Quit();
    printf("bc_inputtest: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
