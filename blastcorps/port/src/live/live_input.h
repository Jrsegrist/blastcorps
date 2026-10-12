/* bc.exe's controller handling (live_input.cpp): the game controllers SDL
 * knows, the N64 pad made from them and the keyboard, and the gate that
 * keeps the inputs that closed the settings menu from reaching the game.
 * No window or renderer: tools/.. bc_inputtest.exe (src/live/input_test.cpp)
 * drives it with SDL's virtual controllers. */
#ifndef LIVE_INPUT_H
#define LIVE_INPUT_H

#include <string>
#include <vector>

#include <SDL.h>

#include "live_setup.h"

namespace live {

/* Every game controller SDL reports is opened, in the order they appeared.
 * A controller that disconnects and comes back (new SDL instance id) is
 * closed and opened again, so `pad = 1` follows it.  Rescanned on SDL's
 * device events and once a second (events can be missed while the window
 * is busy). */
struct PadSet {
    std::vector<SDL_GameController *> pads;

    void scan();                 /* close detached ones, open new ones; logs both */
    void closeAll();
    std::vector<SDL_GameController *> active(const Config &c) const;   /* the ones that play */
    bool isActive(const Config &c, SDL_JoystickID which) const;
    std::vector<std::string> names() const;
};

/* the N64 controller's state */
struct N64Pad {
    unsigned short button = 0;
    int x = 0, y = 0;            /* -80..80 */
    bool idle() const { return button == 0 && x == 0 && y == 0; }
};

/* the N64 pad from the bindings: keyboard (`keys`: SDL_GetKeyboardState, or
 * nullptr) and the playing controllers */
N64Pad readPad(const Config &c, const PadSet &pads, const Uint8 *keys, bool altHeld);

/* all of a playing controller's menu buttons held */
bool menuComboHeld(const Config &c, const PadSet &pads);

/* After the settings menu closes, the buttons and stick held at that moment
 * don't reach the game until each is let go (each button separately; the
 * stick until it is back in its dead zone), and at most `kMaxPolls` reads:
 * a stuck key, a drifting stick or a trigger that doesn't rest at 0 can't
 * keep the game from hearing the controller.  Inputs pressed afterwards
 * pass at once. */
struct InputGate {
    static constexpr unsigned kMaxPolls = 60;   /* one second of retraces */
    unsigned short buttons = 0;  /* still blocked */
    bool stick = false;
    unsigned polls = 0;
    bool closed() const { return buttons != 0 || stick; }
    void close(const N64Pad &now);
    void apply(N64Pad &p);       /* filters p; logs when the gate opens */
};

std::string buttonNames(unsigned short b);

}  // namespace live

#endif
