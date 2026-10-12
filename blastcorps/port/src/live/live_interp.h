/* bc.exe's tagged frame interpolation (live_interp.cpp): who draws what, from
 * the game's PORT_ITAG tags, turned into RT64 transform-group IDs while RT64
 * reads each display list.  Host side only: the game's display lists and
 * RDRAM are never changed. */
#ifndef LIVE_INTERP_H
#define LIVE_INTERP_H

namespace RT64 {
struct Application;
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
/* one line for the log at exit (tag counts, time spent) */
void interpTagsSummary();

}  // namespace live

#endif
