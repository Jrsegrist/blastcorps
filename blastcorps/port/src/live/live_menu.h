/* bc.exe's settings menu (live_menu.cpp): an overlay drawn with Dear ImGui
 * (bundled with RT64, MIT) on top of the game's picture, opened with the
 * menu key (F1) or the controller's menu buttons (Back+Start).  The game
 * stays paused while it is open.  It edits the Config (bc.ini's settings);
 * live_rt64.cpp applies each change at once (MenuEnv::changed) and the menu
 * writes the settings it changed to bc.ini when it closes. */
#ifndef LIVE_MENU_H
#define LIVE_MENU_H

#include <functional>
#include <string>
#include <vector>

#include "live_setup.h"

namespace RT64 {
struct Application;
}
namespace plume {
struct RenderCommandList;
}

namespace live {

struct MenuEnv {
    RT64::Application *app = nullptr;
    SDL_Window *window = nullptr;
    Config *cfg = nullptr;
    std::function<void(const Setting &)> changed;          /* cfg changed: apply this setting now */
    std::function<void()> quit;                            /* the "Quit the game" button */
    std::function<std::vector<std::string>()> padNames;    /* the game controllers connected now */
    std::function<std::string()> status;                   /* one line about the renderer (resolution ...) */
    std::function<std::string()> packStatus;               /* the texture pack's state ("" = never loaded) */
    std::function<unsigned()> msaaSupported;               /* MSAA sample counts possible (mask: 2, 4, 8) */
};

void menuSetup(const MenuEnv &env);   /* once RT64 runs */
void menuOpen();
void menuClose();                     /* saves the settings it changed */
bool menuIsOpen();
/* while open: every SDL event goes here first; true if the menu used it */
bool menuEvent(SDL_Event &e);
/* waiting for the key or button to bind (then every event is the menu's) */
bool menuCapturing();
/* one frame of the menu (main thread, before RT64's updateScreen) */
void menuFrame();
/* RT64's present thread, after the picture: draws the menu's last frame */
void menuDraw(plume::RenderCommandList *list);
/* RT64 is shutting down (its deinit hook): free the menu's GPU objects */
void menuShutdown();
/* testing (--menu-set): change "section.key=value" as the open menu would
 * (applied now, saved when the menu closes) */
bool menuTestSet(const std::string &spec);

/* The frame-rate counter: a small box in a corner of the window, drawn by
 * the same overlay as the menu (and with it while the menu is open).  Off,
 * nothing of it exists or runs. */
struct CounterStats {
    int mode = 0;               /* 0 off, 1 on, 2 detailed */
    int corner = 0;             /* 0 top-left, 1 top-right, 2 bottom-left, 3 bottom-right */
    double shown = 0;           /* pictures presented a second */
    double game = 0;            /* the game's own frames a second */
    double frameMs = 0, frameMsMax = 0;   /* present intervals (mean, max) */
    std::vector<float> graph;   /* the last present intervals, ms (detailed) */
    std::string note;           /* e.g. "interpolated, tagged" */
};
void counterSet(const CounterStats &s);
/* main thread, about 60 times a second while the menu is closed and the
 * counter on: its ImGui frame */
void counterFrame();

}  // namespace live

#endif
