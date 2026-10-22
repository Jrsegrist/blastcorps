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
 * the four analog-stick directions, quitting and the settings menu */
enum Action {
    A_A, A_B, A_Z, A_START, A_DUP, A_DDOWN, A_DLEFT, A_DRIGHT, A_L, A_R, A_CUP, A_CDOWN, A_CLEFT, A_CRIGHT,
    A_STICK_UP, A_STICK_DOWN, A_STICK_LEFT, A_STICK_RIGHT, A_QUIT, A_MENU, A_FPS,
    /* the orbit camera ([camera] mode = orbit; live_orbit.cpp) */
    A_CAM_LEFT, A_CAM_RIGHT, A_CAM_UP, A_CAM_DOWN, A_CAM_ZOOM_IN, A_CAM_ZOOM_OUT, A_CAM_RESET, A_COUNT
};
constexpr int kCameraActions = A_CAM_LEFT;      /* actions from this one on drive the orbit camera */
constexpr int kButtons = A_STICK_UP;           /* actions before this are N64 buttons */
extern const unsigned short kButtonBits[kButtons];
extern const char *const kActionNames[A_COUNT];  /* the config keys */
extern const char *const kActionLabels[A_COUNT]; /* for people (the settings menu) */

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
    bool pak = false;
    bool vulkan = false;
    unsigned scale = 3;
    bool fullscreen = false;
    bool vsync = true;
    bool vsyncPacing = false;       /* present_pacing: false = timer (RT64's own present timer: the
                                       lowest latency; the default), true = vsync (port patch 0005:
                                       pictures handed to the display on its refreshes, even when
                                       the PC is busy, about half a refresh more latency) */
    int pacingOther = 0;            /* with vsyncPacing false: 0 timer (the default; at a frame rate
                                       that divides the display's, with vsync on, the pictures are
                                       locked to its refreshes: port patch 0007), 2 vrr (an even
                                       timer a little under the display's rate, for G-Sync /
                                       FreeSync displays), 3 auto (picks one from the display:
                                       live_rt64.cpp applyPresentPacing) */
    int volume = 100;
    bool mute = false;
    /* optional enhancements (all off by default: the defaults are what the
     * port did before they existed) */
    int resolution = 0;             /* internal resolution: 0 = from the window (RT64's integer
                                       scale of the window), 1 = the N64's (320x240), N = N times it */
    int msaa = 0;                   /* multisample anti-aliasing: 0 (off), 2, 4 or 8 samples */
    int filtering = 0;              /* the picture's scaling: 0 anti-aliased pixel scaling, 1 bilinear,
                                       2 nearest */
    double aspect = 0;              /* 0 = the N64's 4:3; > 0 = widescreen at this width/height;
                                       -1 = as wide as the window */
    int hudPosition = 0;            /* widescreen HUD: 0 = centred in the 4:3 middle (as the N64), 1 = its
                                       left and right parts at the screen's edges (live_wide.cpp) */
    int framerate = 0;             /* frames shown per second: 0 = the game's own (20/30 a second),
                                       -1 = the display's refresh rate, N = N a second; above the
                                       game's own, RT64 adds interpolated frames (presentation only:
                                       the game itself runs exactly as before) */
    int interpolation = 1;          /* with framerate != 0: 1 = tagged (each object matched by its
                                       identity in the game, live_interp.cpp), 0 = auto (RT64 matches
                                       the draw calls itself) */
    int fpsCounter = 0;             /* frame-rate counter: 0 off, 1 on, 2 detailed (frame times, graph) */
    int fpsCorner = 0;              /* its corner: 0 top-left, 1 top-right, 2 bottom-left, 3 bottom-right */
    std::string texturePack;        /* RT64 texture pack folder or .rtz (relative to the exe's folder) */
    bool texturePackOn = false;
    bool texturePackReload = false; /* reload the pack when its rt64.json (or .rtz) changes */
    std::string textureDump = "texture_dump";   /* the texture dump's folder (live_texdump.cpp) */
    bool textureDumpOn = false;     /* write every texture the game loads there as a PNG */
    std::vector<SDL_Scancode> keys[A_COUNT];
    std::vector<PadInput> pad[A_COUNT];
    std::vector<int> menuPad;       /* controller buttons that, held together, open the menu */
    std::vector<int> fpsPad;        /* ... that switch the frame-rate counter (default none) */
    int padStick = 0;               /* 0 left stick, 1 right stick, -1 none */
    int deadzone = 7000;
    int sensitivity = 100;          /* analog stick, percent of the full N64 range */
    int padIndex = 0;               /* which game controller: 0 the first, N the N+1th, -1 all */
    /* [camera]: the optional free orbit camera (live_orbit.cpp; off by default) */
    int cameraMode = 0;             /* 0 original (the game's camera), 1 orbit */
    int camSensX = 100, camSensY = 100;   /* percent */
    bool camInvertX = false, camInvertY = false;
    bool camRecentre = true;        /* ease back behind the player after the input is idle */
    bool camRecentreOnFoot = true;  /* ... on foot even with camRecentre off */
    double camRecentreDelay = 1.5;  /* seconds */
    double camZoomMin = 0.5, camZoomMax = 2.5;      /* times the game's own distance */
    double camPitchMin = -10, camPitchMax = 80;     /* degrees above the horizontal */
    bool camMouse = true;           /* mouse look: drag with a mouse button held, wheel zooms */
    std::vector<std::string> warnings;  /* problems found in the file (shown once) */
};

/* one setting of the file: [section] key */
struct Setting {
    std::string section, key;
    bool operator==(const Setting &o) const { return section == o.section && key == o.key; }
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
/* "section.key=value" over the settings (bc.exe --set); false (and a
 * warning in c.warnings) if it isn't understood */
bool configSet(Config &c, const std::string &spec);
/* a setting's current value as the file writes it */
std::string configValue(const Config &c, const Setting &s);
/* write these settings' current values to the file: their lines are
 * rewritten (or added at the end of their section); everything else,
 * comments included, stays as it was.  False if there is no file
 * (--no-config) or it can't be written. */
bool configSave(Config &c, const std::vector<Setting> &which);

/* names as the file writes them */
std::string aspectName(double aspect);          /* "original", "16:9", "window", "1.85" ... */
std::string framerateName(int framerate);       /* "original", "display", "60" ... */
std::string fpsCounterName(int mode);           /* "off", "on", "detailed" */
std::string cornerName(int corner);             /* "top-left" ... */
std::string keyNames(const std::vector<SDL_Scancode> &keys);
std::string padNames(const std::vector<PadInput> &in);
std::string padInputName(const PadInput &in);   /* "a", "leftx-", "righttrigger" */
std::string menuPadNames(const std::vector<int> &buttons);   /* "back+start" */

/* "" if `path` is Blast Corps (USA) (Rev 1) in .z64, .v64 or .n64 form,
 * else a message for the user saying what it is instead */
std::string romCheck(const std::string &path);
/* the Windows open-file dialog; "" if cancelled */
std::string romDialog();

/* a short help line from the bindings (F1 puts it in the window title) */
std::string helpLine(const Config &c);

}  // namespace live

#endif
