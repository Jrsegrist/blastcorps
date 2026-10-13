/* bc_cameratest.exe: tests of the orbit camera's math (orbit_math.cpp):
 * limits, recentring, zoom, collision pull-in, the view matrix.  No window,
 * ROM or renderer; exit status 0 when all pass. */
#include <cmath>
#include <cstdio>

#include "orbit_math.h"

using namespace live::orbit;

static int g_fail, g_pass;

static void check(bool ok, const char *what) {
    if (ok) g_pass++;
    else {
        g_fail++;
        printf("FAIL: %s\n", what);
    }
}

static bool near(double a, double b, double eps) {
    return std::fabs(a - b) <= eps;
}

static bool nearV(const Vec3 &a, const Vec3 &b, double eps) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}

/* a game camera behind the target: yaw 30, pitch 20, distance 100 */
static Orbit::Game game(const Vec3 &target = {1000, 50, -400}) {
    Orbit::Game g;
    g.target = target;
    g.eye = target + direction(30, 20) * 100.0;
    return g;
}

static void run(Orbit &o, const Orbit::Game &g, const Input &in, double seconds, const Limits &lim,
                const HeightFn &h = nullptr) {
    const double dt = 1.0 / 60.0;
    for (double t = 0; t < seconds - 1e-9; t += dt) o.step(g, in, dt, lim, h);
}

int main() {
    Limits lim;
    /* angles and directions */
    {
        double y, p;
        angles(direction(123, -17), y, p);
        check(near(y, 123, 1e-9) && near(p, -17, 1e-9), "angles(direction) round trip");
        check(near(wrap180(190), -170, 1e-12) && near(wrap360(-10), 350, 1e-12) && near(wrap180(-180), 180, 1e-12),
              "wrap180 / wrap360");
    }
    /* the view matrix: a look-at gives back its eye and looks at the target */
    {
        const Vec3 eye = {10, 20, 30}, at = {-5, 2, 7};
        double m[4][4];
        lookAt(eye, at, m);
        Vec3 e, f;
        check(viewEye(m, e, f), "viewEye accepts a look-at matrix");
        check(nearV(e, eye, 1e-9), "viewEye gives the eye back");
        check(nearV(normalize(f), normalize(at - eye), 1e-9), "the view looks at the target");
        /* the target in camera space: straight ahead (-z), at its distance */
        const double cz = at.x * m[0][2] + at.y * m[1][2] + at.z * m[2][2] + m[3][2];
        const double cx = at.x * m[0][0] + at.y * m[1][0] + at.z * m[2][0] + m[3][0];
        check(near(cz, -length(at - eye), 1e-9) && near(cx, 0, 1e-9), "the target is straight ahead");
        /* straight down is still a valid view */
        lookAt({0, 100, 0}, {0, 0, 0}, m);
        check(viewEye(m, e, f) && nearV(e, {0, 100, 0}, 1e-9), "a view straight down");
    }
    /* untouched: exactly the game's camera */
    {
        Orbit o;
        const Orbit::Game g = game();
        run(o, g, Input(), 1.0, lim);
        Vec3 eye;
        o.eyeFor(g, 0, 0, lim, eye);
        check(!o.engaged() && nearV(eye, g.eye, 1e-9), "not engaged, zoom 1: the game's own eye");
    }
    /* turning: yaw moves, the distance stays the game's */
    {
        Orbit o;
        const Orbit::Game g = game();
        Input in;
        in.yaw = 1.5;   /* 90 degrees a second */
        run(o, g, in, 1.0, lim);
        check(o.engaged() && near(wrap180(o.yaw() - 120), 0, 1e-6), "turning 90 degrees in a second");
        Vec3 eye;
        o.eyeFor(g, 0, 0, lim, eye);
        check(near(length(eye - g.target), 100, 1e-6), "turning keeps the distance");
        double y, p;
        angles(eye - g.target, y, p);
        check(near(y, 120, 1e-6) && near(p, 20, 1e-6), "the eye is at the orbit's yaw and the game's pitch");
        /* the extra yaw of pictures between steps */
        Vec3 e2;
        o.eyeFor(g, 5, 0, lim, e2);
        angles(e2 - g.target, y, p);
        check(near(y, 125, 1e-6), "extra yaw for a picture between steps");
    }
    /* the pitch limits */
    {
        Orbit o;
        const Orbit::Game g = game();
        Input in;
        in.pitch = 2.0;
        run(o, g, in, 2.0, lim);
        check(near(o.pitch(), lim.pitchMax, 1e-9), "tilting up stops at the highest tilt");
        in.pitch = -2.0;
        run(o, g, in, 3.0, lim);
        check(near(o.pitch(), lim.pitchMin, 1e-9), "tilting down stops at the lowest tilt");
        Vec3 e;
        o.eyeFor(g, 0, -50, lim, e);
        double y, p;
        angles(e - g.target, y, p);
        check(p >= lim.pitchMin - 1e-9, "extra pitch between steps stays inside the limits");
    }
    /* zoom: log steps, clamped */
    {
        Orbit o;
        const Orbit::Game g = game();
        Input in;
        in.zoom = 1.0;
        o.step(g, in, 1.0 / 60, lim, nullptr);
        check(near(o.zoom(), 2.0, 1e-9), "zoom out one step (log2 1 = twice as far)");
        o.step(g, in, 1.0 / 60, lim, nullptr);
        check(near(o.zoom(), lim.zoomMax, 1e-9), "zoom stops at the farthest");
        in.zoom = -10;
        o.step(g, in, 1.0 / 60, lim, nullptr);
        check(near(o.zoom(), lim.zoomMin, 1e-9), "zoom stops at the closest");
        Vec3 eye;
        o.eyeFor(g, 0, 0, lim, eye);
        check(near(length(eye - g.target), 100 * lim.zoomMin, 1e-6), "zoom scales the game's distance");
        Input r;
        r.reset = true;
        o.step(g, r, 1.0 / 60, lim, nullptr);
        check(near(o.zoom(), 1.0, 1e-12) && !o.engaged(), "reset: zoom 1, the game's camera");
    }
    /* recentring: idle for the delay, then back to the game's view */
    {
        Orbit o;
        Orbit::Game g = game();
        Input in;
        in.yaw = 3.0;
        run(o, g, in, 0.5, lim);   /* 90 degrees */
        const double turned = o.yaw();
        run(o, g, Input(), lim.recentreDelay - 0.1, lim);
        check(o.engaged() && near(o.yaw(), turned, 1e-9), "recentre waits for the delay");
        run(o, g, Input(), 4.0, lim);
        check(!o.engaged(), "recentred after the delay (back to the game's camera)");
        Vec3 eye;
        o.eyeFor(g, 0, 0, lim, eye);
        check(nearV(eye, g.eye, 1e-6), "recentred: exactly the game's eye");
        /* recentring follows a game camera that turns meanwhile */
        run(o, g, in, 0.5, lim);
        Orbit::Game g2 = g;
        g2.eye = g.target + direction(200, 25) * 100.0;
        run(o, g2, Input(), lim.recentreDelay + 4.0, lim);
        o.eyeFor(g2, 0, 0, lim, eye);
        check(!o.engaged() && nearV(eye, g2.eye, 1e-6), "recentring to a game camera that moved");
    }
    /* recentring off: stays where it was left (world direction) */
    {
        Limits l2 = lim;
        l2.recentre = false;
        Orbit o;
        Orbit::Game g = game();
        Input in;
        in.yaw = 3.0;
        run(o, g, in, 0.5, l2);
        const double turned = o.yaw();
        g.eye = g.target + direction(300, 20) * 100.0;   /* the game's camera swings round */
        run(o, g, Input(), 10.0, l2);
        check(o.engaged() && near(o.yaw(), turned, 1e-9), "recentre off: the camera stays where it was left");
    }
    /* collision: flat ground at y = 0, the target 5 above it */
    {
        const HeightFn flat = [](double, double, double) { return 0.0; };
        Limits l = lim;
        l.pitchMin = -30;
        Orbit o;
        Orbit::Game g;
        g.target = {0, 5, 0};
        g.eye = g.target + direction(0, 20) * 100.0;
        /* clearShare on its own: a ray 30 degrees down from y = 5 reaches y = 1 after 8 units */
        const double share = clearShare(g.target, direction(0, -30), 100.0, 1.0, flat, 0.0);
        check(near(share * 100.0, 8.0, 0.05), "clearShare: the free part of a ray into the ground");
        check(near(clearShare(g.target, direction(0, 30), 100.0, 1.0, flat), 1.0, 1e-12), "clearShare: a clear ray");
        check(near(clearShare(g.target, direction(0, -30), 100.0, 1.0, nullptr), 1.0, 1e-12), "clearShare: no ground known");
        Input in;
        in.pitch = -3.0;   /* tilt down below the horizontal */
        run(o, g, in, 1.0, l, flat);
        Vec3 eye;
        o.eyeFor(g, 0, 0, l, eye);
        check(near(o.pitch(), -30, 1e-9), "tilted to the lowest tilt");
        check(eye.y >= l.clearance - 0.05, "the eye stays above the ground");
        check(o.rise() > 25.0, "tilted into the ground: the view rises over it");
        /* tilt back up: lets out slowly, not at once */
        in.pitch = 6.0;
        o.step(g, in, 1.0 / 60, l, flat);
        o.step(g, in, 1.0 / 60, l, flat);
        o.eyeFor(g, 0, 0, l, eye);
        const double d1 = length(eye - g.target);
        check(d1 < 50.0, "lets out slowly");
        run(o, g, Input(), 1.0, l, flat);
        run(o, g, Input(), 5.0, l, flat);   /* idle: recentres too, all the way out */
        o.eyeFor(g, 0, 0, l, eye);
        check(near(length(eye - g.target), 100.0, 0.5), "out to the full distance again");
        /* the game's own camera is never pulled in (zoom 1, not engaged) */
        Orbit o2;
        Orbit::Game low = g;
        low.eye = g.target + direction(0, -30) * 100.0;
        run(o2, low, Input(), 1.0, l, flat);
        o2.eyeFor(low, 0, 0, l, eye);
        check(nearV(eye, low.eye, 1e-9), "the game's own camera is left as it is");
        /* a wall behind the target 30 units away: pulled in to just before it */
        const HeightFn wall = [](double x, double, double) { return x > 30.0 ? 1e6 : 0.0; };
        Orbit o3;
        Orbit::Game gw;
        gw.target = {0, 5, 0};
        gw.eye = gw.target + direction(90, 10) * 100.0;   /* +x side */
        in = Input();
        in.yaw = 1e-6;
        run(o3, gw, in, 0.5, l, wall);
        o3.eyeFor(gw, 0, 0, l, eye);
        check(eye.x <= 30.0 + 1e-6 && eye.x > 25.0, "pulled in to just before a wall too high to see over");
        check(near(o3.lift(), 0.0, 1e-9) && near(o3.rise(), 0.0, 1e-9), "no lift or rise in front of the wall");
        /* a low wall 20 high at x 20..30: rises over it (clear 4 above it from x = 20:
         * tan p >= 19 / 20, p >= 43.5: to 45 degrees, 10 + 35) */
        const HeightFn lowWall = [](double x, double, double) { return x > 20.0 && x < 30.0 ? 20.0 : 0.0; };
        Orbit o4;
        run(o4, gw, in, 4.0, l, lowWall);
        o4.eyeFor(gw, 0, 0, l, eye);
        check(near(o4.rise(), 35.0, 0.5), "rises over a low wall");
        check(near(o4.distanceRatio(), 1.0, 0.01), "... at the full distance");
    }
    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
