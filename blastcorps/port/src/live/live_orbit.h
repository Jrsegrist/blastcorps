/* bc.exe's optional free orbit camera (live_orbit.cpp; [camera] mode = orbit).
 *
 * Render-only: the game keeps computing its own camera for everything it
 * does; only the pictures RT64 renders use the orbit camera's view (RT64
 * patch 0006's ViewHook, per rendered picture).  Nothing the game reads
 * changes, so its per-frame trace is the same as with the option off. */
#ifndef LIVE_ORBIT_H
#define LIVE_ORBIT_H

#include <cstdint>
#include <string>

#include <SDL.h>

#include "live_input.h"
#include "live_setup.h"

namespace RT64 {
struct Application;
}

namespace live {

void orbitSetup(RT64::Application *app, Config *cfg, SDL_Window *window);
/* the settings changed (start-up, the menu): [camera], controller bindings */
void orbitApply();
/* the orbit camera is in charge now (orbit mode, in play) */
bool orbitActive();
/* the bindings the N64 pad is read with: the configuration's, or (orbit
 * active) without the inputs the camera uses */
const Config &orbitPadConfig();
/* SDL events (mouse look); true if the camera used it */
bool orbitEvent(const SDL_Event &e);
/* each retrace (main thread): the camera's input and one step */
void orbitVi(const PadSet &pads, const Uint8 *keys);
/* around RT64's processDisplayLists of a gfx task */
void orbitBeforeTask(uint32_t dl, bool frameDl);
void orbitAfterTask(bool frameDl);
/* testing: --orbit-test YAWSPEED[:PITCH[:ZOOM]] turns the camera by itself
 * (degrees a second, an absolute pitch in degrees, a zoom factor), with no
 * player input; --orbit-log logs every frame's cameras */
bool orbitSetTest(const std::string &spec);
void orbitSetLog(bool on);
void orbitSummary();

}  // namespace live

#endif
