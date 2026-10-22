/* The free orbit camera's math (bc.exe's optional camera, live_orbit.cpp):
 * plain C++, no renderer, window or game state, so bc_cameratest.exe
 * (orbit_test.cpp) can test it on its own.
 *
 * Units are the game's drawing units (its view matrix: game units / 32), y
 * up.  The game's own camera each frame is an eye E and the point it looks
 * at T (the player's vehicle, or the driver on foot).  The orbit camera
 * looks at the same T from a direction and distance of its own:
 *  - not engaged (nothing moved it, or it eased back): the game's direction,
 *    at the game's distance times the zoom: with zoom 1 exactly the game's
 *    camera;
 *  - engaged (the player turned it): an absolute (world) yaw and pitch, at the
 *    game's distance times the zoom.  With recentring on it eases back to the
 *    game's direction after the input has been idle for the delay; with it
 *    off it stays where it was left (the world direction) until reset.
 *  - collision: the eye is pulled in along the line to T where the ground
 *    (a height query the caller supplies, read-only) would cover it, quickly,
 *    and lets out again slowly.
 * The yaw is atan2(dx, dz) of eye - target in degrees, the pitch the angle
 * above the horizontal. */
#ifndef ORBIT_MATH_H
#define ORBIT_MATH_H

#include <cstdint>
#include <functional>
#include <vector>

namespace live {
namespace orbit {

struct Vec3 {
    double x = 0, y = 0, z = 0;
};
Vec3 operator+(const Vec3 &a, const Vec3 &b);
Vec3 operator-(const Vec3 &a, const Vec3 &b);
Vec3 operator*(const Vec3 &a, double k);
double dot(const Vec3 &a, const Vec3 &b);
Vec3 cross(const Vec3 &a, const Vec3 &b);
double length(const Vec3 &a);
Vec3 normalize(const Vec3 &a);
Vec3 lerp(const Vec3 &a, const Vec3 &b, double t);

double wrap180(double deg);   /* to (-180, 180] */
double wrap360(double deg);   /* to [0, 360) */

/* unit direction from the target to the eye for a yaw and pitch (degrees) */
Vec3 direction(double yawDeg, double pitchDeg);
/* yaw and pitch of a direction (any length > 0) */
void angles(const Vec3 &dir, double &yawDeg, double &pitchDeg);

/* the limits and behaviour (bc.ini [camera]; live_setup.cpp) */
struct Limits {
    double pitchMin = -10.0;      /* degrees above the horizontal (negative: from below) */
    double pitchMax = 80.0;
    double zoomMin = 0.5;         /* times the game's own distance */
    double zoomMax = 2.5;
    bool recentre = true;         /* ease back behind the player after the input is idle ... */
    double recentreDelay = 1.5;   /* ... this many seconds */
    double recentreTime = 0.35;   /* the easing's time constant (s) */
    double minDistance = 1.5;     /* drawing units: never closer to the target than this */
    double minShare = 0.15;       /* the collision pulls in to no less than this share of the distance ... */
    double nearShare = 0.15;      /* ... and doesn't test the part this close to the target (the
                                     target sits on the ground: the player's vehicle) */
    double clearance = 4.0;       /* drawing units above the ground */
    double liftTime = 0.3;        /* where pulled in as far as allowed and still too low: the eye is
                                     lifted above the ground (rises at once, settles in this time) */
    double riseBelow = 0.6;       /* blocked before this share of the distance: look for a steeper view
                                     (up to riseMax degrees more, in riseStep steps, at most pitchMax)
                                     that sees past it; if none does, the one that sees farthest when
                                     that is at least riseGain more of the distance (a camera pulled in
                                     to the target ends up inside the vehicle: ugly, and pictures of
                                     big close-up polygons cost the GPU up to 3x) */
    double riseMax = 75.0;
    double riseStep = 5.0;
    double riseGain = 0.15;
    double riseTime = 0.2;        /* ... reached in about this time (and back with letOutTime), or at once
                                     where the view as it is would be pulled in under riseSnapBelow */
    double riseSnapBelow = 0.3;
    double pullInTime = 0.04;     /* time constants of the collision distance (s): in fast ... */
    double letOutTime = 0.5;      /* ... out slowly */
};

/* one step's input, already scaled by sensitivity and inverted as set */
struct Input {
    double yaw = 0;               /* degrees to turn (positive: the eye moves to larger yaw) */
    double pitch = 0;             /* degrees to tilt (positive: the eye moves up) */
    double zoom = 0;              /* log2 change of the zoom (positive: farther away) */
    bool reset = false;           /* back to the game's camera, zoom 1 */
};

/* the ground's height (drawing units) under x, z for a point at height y:
 * the highest surface there that isn't far above y (a bridge high above is
 * not the ground), or a value <= -1e30 if none / unknown */
using HeightFn = std::function<double(double x, double y, double z)>;

class Orbit {
public:
    /* the game's camera this step */
    struct Game {
        Vec3 eye, target;
    };

    /* forget everything (a cut, a new level, orbit switched off): the game's camera, zoom kept unless `zoomToo` */
    void reset(bool zoomToo);
    /* one step of dt seconds */
    void step(const Game &g, const Input &in, double dt, const Limits &lim, const HeightFn &height);

    /* the camera for a picture: the target and the game's eye of that picture
     * (interpolated by the caller), `extraYaw`/`extraPitch` added (the input
     * since the last step, for pictures between steps) */
    void eyeFor(const Game &g, double extraYaw, double extraPitch, const Limits &lim, Vec3 &eye) const;

    /* the camera t (0..1) of the way from one step's state to the next's (a
     * picture shown between the two steps): yaw along the shorter arc, pitch,
     * the blend and the collision's pull-in, lift and rise linearly, the zoom
     * geometrically; a state not engaged (its own yaw and pitch unused) takes
     * the other's direction, so engaging and recentring ease through the blend
     * alone.  The rest (idle time, recentring) is b's. */
    static Orbit between(const Orbit &a, const Orbit &b, double t);

    bool engaged() const { return blend_ > 0.0; }
    double yaw() const { return yaw_; }
    double pitch() const { return pitch_; }
    double zoom() const { return zoom_; }
    double blend() const { return blend_; }
    double distanceRatio() const { return ratio_; }   /* the collision's pull-in (1: none) */
    double lift() const { return lift_; }             /* ... and lift (drawing units) */
    double rise() const { return rise_; }             /* ... and rise over obstacles (degrees) */
    double idle() const { return idle_; }

private:
    double yaw_ = 0, pitch_ = 0;  /* absolute, while engaged */
    double zoom_ = 1.0;
    double blend_ = 0.0;          /* 1: the orbit's direction, 0: the game's (eases to 0 after recentring) */
    bool recentring_ = false;
    double idle_ = 1e9;           /* seconds since the last input */
    double ratio_ = 1.0;          /* collision: share of the wanted distance used */
    double lift_ = 0.0;           /* collision: the eye raised by this much */
    double rise_ = 0.0;           /* collision: the view steeper by this many degrees */
};

/* the largest share (0..1] of the segment target -> target + dir * dist whose
 * points from share `from` on all stay `clearance` above the ground; 1 if
 * the height is unknown */
double clearShare(const Vec3 &target, const Vec3 &dir, double dist, double clearance, const HeightFn &height,
                  double from = 0.0);

/* a look-at view matrix (row vectors, translation in row 3: the N64's and
 * RT64's layout), up = +y */
void lookAt(const Vec3 &eye, const Vec3 &target, double m[4][4]);
/* eye and the point at `dist` ahead of a look-at view matrix */
bool viewEye(const double m[4][4], Vec3 &eye, Vec3 &forward);

/* the height of triangle p at (x, z): false outside it (in x/z) or for a
 * vertical one */
bool triHeight(const Vec3 p[3], double x, double z, double &h);

/* Triangles binned by their x/z bounds, for the collision's many height
 * queries (one step asks up to ~800): forEachAt gives every triangle whose
 * x/z bounds hold the point (and some others), so a query over them finds
 * exactly what a test of all of them would.  Rebuilt each step (the
 * buildings' triangles move and fall apart). */
class TriGrid {
public:
    void clear();
    void add(const Vec3 p[3]);
    void build();   /* after the adds, before the queries */
    size_t size() const { return tris_.size(); }
    template <typename F> void forEachAt(double x, double z, F f) const {
        if (tris_.empty() || !(x == x) || !(z == z)) return;
        for (uint32_t i : big_) f(tris_[i].p);
        const uint32_t b = uint32_t(row(z) * nx_ + col(x));
        for (uint32_t i = start_[b]; i < start_[b + 1]; i++) f(tris_[index_[i]].p);
    }

private:
    struct Tri {
        double minX, maxX, minZ, maxZ;
        Vec3 p[3];
    };
    int col(double x) const;
    int row(double z) const;
    std::vector<Tri> tris_;
    std::vector<uint32_t> start_, index_, big_;   /* bins (row-major), their triangles; ones in too many bins */
    double x0_ = 0, z0_ = 0, cw_ = 1, cd_ = 1;
    int nx_ = 1, nz_ = 1;
};

/* The drawing list's visibility test (render only): what a camera can see,
 * from the eye looking at the target, as a pyramid with the projection's
 * half angles (tan: tanX, tanY) widened by marginDeg (pictures between list
 * updates may look a little elsewhere), its apex moved back by `back` along
 * the view (the eye may move a little meanwhile), and the far plane at depth
 * farDist (> 0; plus `back`).  A side widened to 89 degrees or more stops
 * limiting.  (No `near`/`far` names here: windows.h defines them.) */
class ViewVolume {
public:
    ViewVolume(const Vec3 &eye, const Vec3 &target, double tanX, double tanY, double marginDeg, double back,
               double farDist);
    /* false if an axis-aligned box is surely outside (all its corners
     * beyond one of the planes) */
    bool boxVisible(const Vec3 &lo, const Vec3 &hi) const;
    /* exact: some part of the box is inside (its faces clipped against the
     * planes; or the view's axis starts inside it) - for checks; the plane
     * test above keeps some boxes that are outside (big ones beside the eye) */
    bool boxVisibleExact(const Vec3 &lo, const Vec3 &hi) const;

private:
    double m_[4][4];
    double sx_, sy_, far_;
    bool limX_, limY_;
};

/* the near and far distances of an N64 perspective projection (guPerspective,
 * row vectors); false if it isn't one */
bool perspectiveRange(const double p[4][4], double &nearZ, double &farZ);

/* The orbit stress test (--orbit-test sweep): the camera turned, tilted and
 * zoomed by itself, the same at every run, `t` seconds of game time after the
 * orbit became active: the yaw rate (degrees a second: both ways, up to about
 * 210, and a 320 flick of 0.75 s every 13 s), an absolute pitch over most of
 * the limits' range and a zoom over the whole range. */
struct Sweep {
    double yawRate = 0, pitch = 0, zoom = 1;
};
Sweep sweepAt(double t, const Limits &lim);

/* The frame-time report's spike attribution (live_rt64.cpp): a present
 * interval over 1.5 times the target, against what happened in the retraces
 * before it.  The causes, the first that applies (a missed retrace mostly
 * spent in the display lists with RT64's queue full is the render thread's
 * or, with a picture over the target, the GPU's): */
enum SpikeCause {
    SPIKE_MAIN_GAME,      /* the main thread missed a retrace: mostly the game (and other host work) */
    SPIKE_MAIN_LISTS,     /* ... mostly reading the display lists (processDisplayLists) */
    SPIKE_MAIN_ORBIT,     /* ... mostly the orbit camera's host work */
    SPIKE_MAIN_SUBMIT,    /* ... mostly RT64's updateScreen (waiting for the render thread) */
    SPIKE_MAIN_OTHER,     /* ... mostly the rest: the host's work after the retrace (events, the controllers'
                             rescan, screenshots) or time the thread didn't run (the OS) */
    SPIKE_GPU,            /* a picture took the GPU over kSpikeGpuShare of the target (the present's own
                             GPU work, not timed, comes on top) */
    SPIKE_RENDER,         /* RT64's render thread took longer than the game frame (CPU) */
    SPIKE_SHADERS,        /* new materials (shaders submitted: the ubershader draws them meanwhile) */
    SPIKE_TEXTURES,       /* a burst of new textures (kSpikeTextures or more: a few each frame are usual) */
    SPIKE_PRESENT,        /* none of these: the present / pacing side (or the OS) */
    SPIKE_CAUSES
};
constexpr double kSpikeGpuShare = 0.75;
constexpr unsigned kSpikeTextures = 8;
const char *spikeCauseName(int cause);
/* what the retraces around a spike had (their largest values / sums) */
struct SpikeWindow {
    double retraceMs = 0;            /* the longest interval between two retraces (the main thread's loop) */
    double periodMs = 1000.0 / 60;   /* a retrace's period */
    double gameMs = 0, listsMs = 0, orbitMs = 0, submitMs = 0;   /* that retrace's main-thread work */
    double otherMs = 0;              /* ... and the rest of it (SPIKE_MAIN_OTHER) */
    double gpuMs = 0;                /* the longest picture on the GPU */
    double renderMs = 0, frameMs = 0;   /* the longest RT64 workload (render thread); the game frame's period */
    double targetMs = 0;             /* the present target */
    unsigned shaders = 0, textures = 0;   /* new shaders submitted, textures added */
    bool queueFull = false;          /* that retrace's frame task found RT64's workload queue full: reading
                                        its display lists ended waiting for the render thread */
};
int spikeCause(const SpikeWindow &w);

}  // namespace orbit
}  // namespace live

#endif
