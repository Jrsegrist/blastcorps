/* bc_cameratest.exe: tests of the orbit camera's math (orbit_math.cpp):
 * limits, recentring, zoom, collision pull-in, the view matrix.  No window,
 * ROM or renderer; exit status 0 when all pass. */
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

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
        check(o.distanceRatio() > 0.5, "... at once, not pulled in to the target meanwhile");
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
        run(o3, gw, in, 2.0, l, wall);
        o3.eyeFor(gw, 0, 0, l, eye);
        check(eye.x <= 30.0 + 1e-6 && eye.x > 25.0, "kept just before a wall too high to see over");
        check(near(o3.lift(), 0.0, 1e-9) && o3.rise() > 40.0 && length(eye - gw.target) > 50.0,
              "... looking down past it from higher up rather than pulled in to the target (no lift)");
        /* the rise that sees farthest only if clearly farther: a wall right behind the target
         * (no rise helps much) still pulls in */
        const HeightFn nearWall = [](double x, double, double) { return x > 3.0 ? 1e6 : 0.0; };
        Orbit o5;
        run(o5, gw, in, 0.5, l, nearWall);
        check(o5.rise() < 1e-6 && o5.distanceRatio() < 0.2, "a wall right behind the target: pulled in, no rise "
                                                             "(none sees clearly farther)");
        /* the wall gone: lets out slowly, not at once */
        o5.step(gw, in, 1.0 / 60, l, flat);
        o5.step(gw, in, 1.0 / 60, l, flat);
        check(o5.distanceRatio() < 0.5, "lets out slowly");
        /* a low wall 20 high at x 20..30: rises over it (clear 4 above it from x = 20:
         * tan p >= 19 / 20, p >= 43.5: to 45 degrees, 10 + 35) */
        const HeightFn lowWall = [](double x, double, double) { return x > 20.0 && x < 30.0 ? 20.0 : 0.0; };
        Orbit o4;
        run(o4, gw, in, 4.0, l, lowWall);
        o4.eyeFor(gw, 0, 0, l, eye);
        check(near(o4.rise(), 35.0, 0.5), "rises over a low wall");
        check(near(o4.distanceRatio(), 1.0, 0.01), "... at the full distance");
        /* turned to face a low wall close behind: rises at once, never pulled in to the target meanwhile */
        Orbit o6;
        Orbit::Game gf = gw;
        gf.eye = gf.target + direction(270, 10) * 100.0;   /* -x side: open */
        in = Input();
        in.yaw = 1e-6;
        run(o6, gf, in, 1.0, l, lowWall);
        in.yaw = 180.0;   /* one step round to the wall's side */
        o6.step(gf, in, 1.0 / 60, l, lowWall);
        check(o6.rise() > 30.0 && o6.distanceRatio() > 0.5, "turned to a low wall: rises at once, not pulled in");
    }
    /* the collision's triangle grid: the same heights as a test of every triangle */
    {
        unsigned seed = 12345;
        auto rnd = [&](double lo, double hi) {
            seed = seed * 1103515245u + 12345u;
            return lo + (hi - lo) * double((seed >> 8) & 0xFFFF) / 65535.0;
        };
        std::vector<std::array<Vec3, 3>> tris;
        for (int i = 0; i < 1500; i++) {   /* small walls and roofs, a few big slabs, some vertical */
            const double cx = rnd(-2000, 2000), cz = rnd(-2000, 2000), r = i % 97 == 0 ? 3000 : rnd(2, 60);
            std::array<Vec3, 3> t = {{{cx + rnd(-r, r), rnd(0, 300), cz + rnd(-r, r)},
                                      {cx + rnd(-r, r), rnd(0, 300), cz + rnd(-r, r)},
                                      {cx + rnd(-r, r), rnd(0, 300), cz + rnd(-r, r)}}};
            if (i % 50 == 0) t[2] = {t[0].x, t[0].y + 40, t[0].z};
            tris.push_back(t);
        }
        TriGrid grid;
        for (const auto &t : tris) grid.add(t.data());
        grid.build();
        check(grid.size() == tris.size(), "the grid keeps every triangle");
        int same = 0, total = 0, hits = 0;
        for (int q = 0; q < 20000; q++) {
            /* points anywhere, at triangle corners and outside the triangles' area */
            double x = rnd(-2600, 2600), z = rnd(-2600, 2600);
            if (q % 4 == 0) {
                const auto &t = tris[q % tris.size()];
                x = t[q % 3].x, z = t[q % 3].z;
            }
            double all = -1e31, h;
            for (const auto &t : tris)
                if (triHeight(t.data(), x, z, h)) all = std::max(all, h);
            double binned = -1e31;
            grid.forEachAt(x, z, [&](const Vec3 *p) {
                if (triHeight(p, x, z, h)) binned = std::max(binned, h);
            });
            same += all == binned;
            hits += all > -1e30;
            total++;
        }
        check(same == total, "grid height queries equal a test of every triangle");
        check(hits > 1000, "... with many queries over triangles");
        TriGrid empty;
        empty.build();
        int calls = 0;
        empty.forEachAt(0, 0, [&](const Vec3 *) { calls++; });
        check(calls == 0, "an empty grid has no triangles anywhere");
    }
    /* the drawing list's view volume */
    {
        /* guPerspective(fovy 60, aspect 4:3, near 10, far 5000, scale 1), row vectors */
        const double n = 10, f = 5000, cot = 1.0 / std::tan(30.0 * 3.14159265358979 / 180.0);
        double p[4][4] = {{cot / (4.0 / 3.0), 0, 0, 0}, {0, cot, 0, 0}, {0, 0, (n + f) / (n - f), -1},
                          {0, 0, 2 * n * f / (n - f), 0}};
        double nz, fz;
        check(perspectiveRange(p, nz, fz) && near(nz, 10, 1e-9) && near(fz, 5000, 1e-6), "perspective near/far");
        for (auto &r : p)
            for (double &v : r) v *= 2.0;   /* guPerspective's scale */
        check(perspectiveRange(p, nz, fz) && near(nz, 10, 1e-9) && near(fz, 5000, 1e-6), "... with a scale");
        double id[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
        check(!perspectiveRange(id, nz, fz), "not a perspective");
        const double tx = 1.0 / cot * (4.0 / 3.0), ty = 1.0 / cot;
        const Vec3 eye = {0, 100, 0}, at = {0, 100, -100};   /* looking along -z */
        const ViewVolume v(eye, at, tx, ty, 0.0, 0.0, 5000);
        auto box = [](const Vec3 &c, double r) { return std::make_pair(c - Vec3{r, r, r}, c + Vec3{r, r, r}); };
        auto vis = [&](const ViewVolume &vv, const Vec3 &c, double r) {
            const auto b = box(c, r);
            return vv.boxVisible(b.first, b.second);
        };
        check(vis(v, {0, 100, -500}, 5), "a box straight ahead is seen");
        check(!vis(v, {0, 100, 500}, 5), "a box behind is not");
        check(!vis(v, {2000, 100, -500}, 5), "a box far to the right is not");
        check(vis(v, {500 * tx - 1, 100, -500}, 5), "a box just inside the right edge is");
        check(!vis(v, {500 * tx + 10, 100, -500}, 5), "... just outside it is not");
        check(!vis(v, {0, 100 + 500 * ty + 10, -500}, 5), "... nor just above the top edge");
        check(!vis(v, {0, 100, -6000}, 5), "a box beyond the far plane is not");
        check(vis(v, {0, 100, 2}, 5), "a box around the eye is");
        const ViewVolume wide(eye, at, tx, ty, 15.0, 0.0, 5000);
        check(vis(wide, {500 * tx + 10, 100, -500}, 5), "the margin widens the edges");
        const ViewVolume back(eye, at, tx, ty, 0.0, 50.0, 5000);
        check(vis(back, {0, 100, 30}, 5), "the apex moved back sees a little behind the eye");
        const ViewVolume all(eye, at, tx, ty, 80.0, 0.0, 0);
        check(vis(all, {5000, 100, -1}, 5) && !vis(all, {0, 100, 500}, 5), "a side over 89 degrees stops limiting");
        /* the margins only ever add: every box the exact volume sees, a wider one sees */
        unsigned seed = 777;
        auto rnd = [&](double lo, double hi) {
            seed = seed * 1103515245u + 12345u;
            return lo + (hi - lo) * double((seed >> 8) & 0xFFFF) / 65535.0;
        };
        int lost = 0, seen = 0;
        for (int i = 0; i < 20000; i++) {
            const Vec3 e = {rnd(-500, 500), rnd(0, 300), rnd(-500, 500)};
            const Vec3 t = {rnd(-500, 500), rnd(0, 100), rnd(-500, 500)};
            if (length(t - e) < 1) continue;
            const ViewVolume ex(e, t, tx * 1.467, ty, 0.0, 0.0, 3000);
            const ViewVolume wd(e, t, tx * 1.467, ty, rnd(0, 40), rnd(0, 80), 3000);
            const Vec3 c = {rnd(-3000, 3000), rnd(-100, 400), rnd(-3000, 3000)};
            const double r = rnd(1, 200);
            /* (boxes in front of the eye: one reaching behind it may pass the
             * plane test without being seen at all) */
            const Vec3 fw = normalize(t - e);
            bool front = true;
            for (int k = 0; k < 8; k++) {
                const Vec3 q = {c.x + ((k & 1) ? r : -r), c.y + ((k & 2) ? r : -r), c.z + ((k & 4) ? r : -r)};
                front = front && dot(q - e, fw) > 0;
            }
            if (front && vis(ex, c, r)) {
                seen++;
                lost += !vis(wd, c, r);
            }
        }
        check(seen > 1000 && lost == 0, "margins never drop a box the exact view sees");
        /* the exact test: a big flat box beside and behind the eye passes the plane test but isn't seen */
        {
            /* the game's projection: fovy 45, 4:3 (proj [0][0] 1.81065, [1][1] 2.4142) */
            const Vec3 e = {2480.7, 252.8, 1524.2}, t = {2178.7, 0.7, 1328.2};
            const ViewVolume ex(e, t, 1.0 / 1.81065, 1.0 / 2.4142, 0.0, 0.0, 10200);
            check(ex.boxVisible({3000, 0, 0}, {4500, 1, 2500}) && !ex.boxVisibleExact({3000, 0, 0}, {4500, 1, 2500}),
                  "exact test: a big box beside the eye (level 3) is out, the plane test keeps it");
            check(ex.boxVisibleExact({2000, 0, 1000}, {2400, 1, 1600}), "exact test: the ground under the target is in");
            check(ex.boxVisibleExact({2400, 200, 1500}, {2500, 300, 1550}), "exact test: a box around the eye is in");
            check(ex.boxVisibleExact({-50000, -50000, -50000}, {50000, 50000, 50000}), "exact test: inside a huge box");
        }
        /* exact implies the plane test; and the margins keep every box the exact view sees */
        int bad = 0, lost2 = 0, seen2 = 0;
        for (int i = 0; i < 20000; i++) {
            const Vec3 e = {rnd(-500, 500), rnd(0, 300), rnd(-500, 500)};
            const Vec3 t = {rnd(-500, 500), rnd(0, 100), rnd(-500, 500)};
            if (length(t - e) < 1) continue;
            const ViewVolume ex(e, t, tx * 1.467, ty, 0.0, 0.0, 3000);
            const ViewVolume wd(e, t, tx * 1.467, ty, rnd(0, 40), rnd(0, 80), 3000);
            const Vec3 c = {rnd(-3000, 3000), rnd(-100, 400), rnd(-3000, 3000)};
            const Vec3 r = {rnd(1, 1500), rnd(1, 200), rnd(1, 1500)};
            if (ex.boxVisibleExact(c - r, c + r)) {
                seen2++;
                bad += !ex.boxVisible(c - r, c + r);
                lost2 += !wd.boxVisible(c - r, c + r);
            }
        }
        check(seen2 > 1000 && bad == 0, "the exact test only passes boxes the plane test passes");
        check(lost2 == 0, "margins keep every box the exact view sees (big boxes too)");
    }
    /* the stress test's motion (--orbit-test sweep) */
    {
        double yMin = 1e9, yMax = -1e9, pMin = 1e9, pMax = -1e9, zMin = 1e9, zMax = -1e9;
        bool inLimits = true, same = true;
        for (int i = 0; i < 60 * 60; i++) {
            const double t = i / 60.0;
            const Sweep s = sweepAt(t, lim), s2 = sweepAt(t, lim);
            same = same && s.yawRate == s2.yawRate && s.pitch == s2.pitch && s.zoom == s2.zoom;
            yMin = std::min(yMin, s.yawRate), yMax = std::max(yMax, s.yawRate);
            pMin = std::min(pMin, s.pitch), pMax = std::max(pMax, s.pitch);
            zMin = std::min(zMin, s.zoom), zMax = std::max(zMax, s.zoom);
            inLimits = inLimits && s.pitch >= lim.pitchMin && s.pitch <= lim.pitchMax && s.zoom >= lim.zoomMin - 1e-9 &&
                       s.zoom <= lim.zoomMax + 1e-9 && std::isfinite(s.yawRate);
        }
        check(same && inLimits, "sweep: the same at every call, pitch and zoom within the limits");
        check(yMin < -180 && yMax >= 320 && pMin < lim.pitchMin + 4 && pMax > lim.pitchMax - 7 &&
                  zMin < lim.zoomMin * 1.05 && zMax > lim.zoomMax * 0.95,
              "sweep: turns both ways with flicks, over the pitch and zoom ranges in a minute");
        check(sweepAt(12.5, lim).yawRate == 320 && sweepAt(25.5, lim).yawRate == 320 && sweepAt(12.0, lim).yawRate != 320,
              "sweep: a flick every 13 s");
    }
    /* the spike attribution's rules */
    {
        SpikeWindow w;
        w.targetMs = 8.33;
        w.frameMs = 33.3;
        check(spikeCause(w) == SPIKE_PRESENT, "spike: nothing found -> present / other");
        w.textures = 3;
        check(spikeCause(w) == SPIKE_PRESENT, "spike: a few new textures are usual");
        w.textures = 12;
        check(spikeCause(w) == SPIKE_TEXTURES, "spike: a burst of new textures");
        w.gpuMs = 6;
        check(spikeCause(w) == SPIKE_TEXTURES, "spike: a picture under 3/4 of the target is no GPU spike");
        w.gpuMs = 0;
        w.shaders = 1;
        check(spikeCause(w) == SPIKE_SHADERS, "spike: new shaders before new textures");
        w.renderMs = 40;
        check(spikeCause(w) == SPIKE_RENDER, "spike: the render thread over the game frame");
        w.gpuMs = 9;
        check(spikeCause(w) == SPIKE_GPU, "spike: a picture over the target on the GPU");
        w.retraceMs = 24;   /* under 1.5 retraces: not missed */
        w.orbitMs = 20;
        check(spikeCause(w) == SPIKE_GPU, "spike: a retrace under 1.5 periods is not missed");
        w.retraceMs = 30;
        w.gameMs = 6, w.listsMs = 3, w.submitMs = 1;
        check(spikeCause(w) == SPIKE_MAIN_ORBIT, "spike: a missed retrace, mostly the orbit camera");
        w.gameMs = 25;
        check(spikeCause(w) == SPIKE_MAIN_GAME, "spike: a missed retrace, mostly the game");
        w.listsMs = 26;
        check(spikeCause(w) == SPIKE_MAIN_LISTS, "spike: a missed retrace, mostly the display lists");
        w.queueFull = true;
        check(spikeCause(w) == SPIKE_GPU, "spike: ... with RT64's queue full and a slow picture: the GPU");
        w.gpuMs = 5;
        check(spikeCause(w) == SPIKE_RENDER, "spike: ... with RT64's queue full: the render thread");
        w.queueFull = false;
        w.gpuMs = 9;
        w.submitMs = 27;
        check(spikeCause(w) == SPIKE_MAIN_SUBMIT, "spike: a missed retrace, mostly RT64's submit");
        bool named = true;
        for (int c = 0; c < SPIKE_CAUSES; c++) named = named && spikeCauseName(c)[0] != '?';
        check(named && spikeCauseName(SPIKE_CAUSES)[0] == '?', "spike: every cause has a name");
    }
    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
