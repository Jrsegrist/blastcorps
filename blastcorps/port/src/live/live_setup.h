/* bc.exe's player-facing setup (live_setup.cpp): the config file next to
 * the exe (bc.ini), key and controller bindings, and finding and checking
 * the user's ROM (file dialog, format and version check). */
#ifndef LIVE_SETUP_H
#define LIVE_SETUP_H

#include <string>
#include <vector>

#include <SDL.h>

namespace live {

/* what a binding drives: the N64 buttons (in the order of kButtonBits),
 * the four analog-stick directions, and quitting */
enum Action {
    A_A, A_B, A_Z, A_START, A_DUP, A_DDOWN, A_DLEFT, A_DRIGHT, A_L, A_R, A_CUP, A_CDOWN, A_CLEFT, A_CRIGHT,
    A_STICK_UP, A_STICK_DOWN, A_STICK_LEFT, A_STICK_RIGHT, A_QUIT, A_COUNT
};
constexpr int kButtons = A_STICK_UP;           /* actions before this are N64 buttons */
extern const unsigned short kButtonBits[kButtons];
extern const char *const kActionNames[A_COUNT];  /* the config keys */

/* one game-controller input: a button, or an axis pushed one way */
struct PadInput {
    int button = -1;    /* SDL_GameControllerButton */
    int axis = -1;      /* SDL_GameControllerAxis */
    int dir = 0;        /* +1 / -1 (triggers: +1) */
};

struct Config {
    std::string file;               /* bc.ini path ("" = none: --no-config) */
    std::string dir;                /* the exe's folder, with a trailing backslash */
    std::string rom;                /* as written in the file */
    std::string saves;              /* folder ("" = don't keep saves) */
    bool pak = true;
    bool vulkan = false;
    unsigned scale = 3;
    bool fullscreen = false;
    bool vsync = true;
    int volume = 100;
    bool mute = false;
    std::vector<SDL_Scancode> keys[A_COUNT];
    std::vector<PadInput> pad[A_COUNT];
    int padStick = 0;               /* 0 left stick, 1 right stick, -1 none */
    int deadzone = 7000;
    std::vector<std::string> warnings;  /* problems found in the file (shown once) */
};

std::string utf8(const std::wstring &w);
std::wstring wide(const std::string &s);
std::string exeDir();
/* `p` relative to the exe's folder unless it is absolute */
std::string absPath(const Config &c, const std::string &p);

/* the built-in defaults (the text of a new bc.ini) */
void configDefaults(Config &c);
/* defaults, then `file` over them; a missing file is created with the
 * defaults (and their comments).  Returns false if it couldn't be read or
 * created (warnings say why). */
bool configLoad(Config &c, const std::string &file);
/* remember the ROM: rewrite (or add) the `rom =` line, keeping the rest */
bool configSetRom(Config &c, const std::string &rom);

/* "" if `path` is Blast Corps (USA) (Rev 1) in .z64, .v64 or .n64 form,
 * else a message for the user saying what it is instead */
std::string romCheck(const std::string &path);
/* the Windows open-file dialog; "" if cancelled */
std::string romDialog();

/* a short help line from the bindings (F1 puts it in the window title) */
std::string helpLine(const Config &c);

}  // namespace live

#endif
