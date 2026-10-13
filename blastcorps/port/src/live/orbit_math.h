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

#include <functional>

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
                                     (up to riseMax degrees more, in riseStep steps) that sees past it */
    double riseMax = 50.0;
    double riseStep = 5.0;
    double riseTime = 0.2;        /* ... reached in about this time (and back with letOutTime) */
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

}  // namespace orbit
}  // namespace live

#endif
