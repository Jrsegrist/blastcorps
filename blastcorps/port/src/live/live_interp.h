/* bc.exe's tagged frame interpolation (live_interp.cpp): who draws what, from
 * the game's PORT_ITAG tags, turned into RT64 transform-group IDs while RT64
 * reads each display list.  Host side only: the game's display lists and
 * RDRAM are never changed. */
#ifndef LIVE_INTERP_H
#define LIVE_INTERP_H

#include <cstdint>

namespace RT64 {
struct Application;
struct State;
struct DisplayList;
}

namespace live {

/* on: tags are recorded and applied (video.framerate above the game's own
 * and video.interpolation = tagged); off: RT64's own matching, the GBI
 * untouched (and the game's tags cost one call each) */
void interpTagsSetup(RT64::Application *app);
void interpTagsEnable(bool on);
bool interpTagsOn();
/* around RT64's processDisplayLists of a task (after loadUCodeGBI) */
void interpTagsBeforeTask(unsigned dl, bool mainFrame);
void interpTagsAfterTask(bool mainFrame);
/* PORT_ICUT_IF since the last main frame (cleared by reading) */
bool interpTagsTakeCut();
/* --interp-log: log jumps (a tagged object that teleported) and per-frame counts */
void interpTagsSetLog(bool on);
/* --tag-test MASK (experiments; see live_interp.cpp g_test) */
void interpTagsSetTest(unsigned mask);
/* one line for the log at exit (tag counts, time spent) */
void interpTagsSummary();

/* Another host feature's look at each GBI command while RT64 reads a task
 * (the orbit camera's backdrop, live_orbit.cpp): the GBI table is wrapped
 * while a hook is set, tags or not.  The hook runs before the command
 * (except G_MTX / G_POPMTX, which it doesn't see); it returns true if it
 * ran the command itself (with RT64's own functions, `originals[op]`, also
 * for commands of its own), false to let it run as usual.  It must leave
 * RDRAM as it found it. */
typedef void (*GbiOriginal)(RT64::State *state, RT64::DisplayList **dl);
typedef bool (*GbiCommandHook)(RT64::State *state, RT64::DisplayList **dl, const GbiOriginal *originals);
/* ... and after every G_MTX RT64 ran (`params`: the command's parameter
 * byte), e.g. to adjust the matrix RT64 now holds */
typedef void (*GbiMatrixHook)(RT64::State *state, uint8_t params);
void interpSetCommandHook(GbiCommandHook hook, GbiMatrixHook mtxHook);

}  // namespace live

#endif
