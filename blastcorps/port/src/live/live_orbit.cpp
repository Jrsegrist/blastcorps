/* bc.exe's optional free orbit camera ([camera] mode = orbit; see
 * live_orbit.h and orbit_math.h).
 *
 * How it stays render-only:
 *  - the game's camera: at the end of the game's camera function (00000.c
 *    cam_update, PORT_ICUT_IF -> host_camera_hook) the eye (from the view
 *    matrix the game just built, DynamicBuf.unk140) and the point it looks at
 *    (gCamAtX/Y/Z) are copied for the DynamicBuf being built; read only;
 *  - the pictures: RT64 patch 0006 calls ViewHook for every perspective
 *    projection of every picture it renders (in-between pictures of a higher
 *    frame rate too).  A projection whose view is the frame's game view gets
 *    the orbit camera's view instead: target and game eye interpolated with
 *    the picture's weight, the orbit's direction and distance, and the input
 *    since the last step.  The projection is the game's own (fov, aspect,
 *    widescreen).  The game never sees any of it;
 *  - what the game culled: the level's geometry and the buildings whose cells
 *    the game's visibility walk didn't list would be missing from an orbit
 *    view.  The drawing paths (PORT_DRAW_CELLS, 5FD50.c, 77E20.c) get the
 *    game's cells and those the orbit camera can see (its own view test,
 *    with margins for turning: drawCells), unlisted buildings drawn as they
 *    are, without their per-object updates (the game leaves those alone:
 *    they stay as they are).  The game's logic keeps reading its own list
 *    (77E20.c's per-object updates run only for listed objects, so the list
 *    must not change);
 *  - the backdrop (2D2D810.c: a 2D quad scrolled by the camera's heading and
 *    pitch, and the ground colour below its horizon): while RT64 reads the
 *    frame, its vertices and the fill rectangle are given the orbit view's
 *    heading and pitch (the vertices are written just for RT64's read and put
 *    back right after it: RDRAM is as the game left it);
 *  - collision: the eye is kept above the level's ground triangles and the
 *    buildings' collision triangles (read-only copies of the game's height
 *    tests: 62740.c ground_scan_grid's grid, the 0x60-byte triangles at
 *    gCollTris, binned by x/z each step: orbit::TriGrid), pulled in along
 *    the line to the target.
 * The controls: in play, the camera bindings (default: the right stick) drive
 * the camera and not the N64 buttons bound to the same inputs (the C buttons'
 * right stick); outside play the right stick is the C buttons as before. */
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include "gbi/rt64_gbi.h"
#include "hle/rt64_application.h"
#include "gbi/rt64_display_list.h"
#include "hle/rt64_rsp.h"
#include "hle/rt64_state.h"
#include "hle/rt64_workload_queue.h"
#include "render/rt64_projection_processor.h"

#include "live_interp.h"
#include "live_orbit.h"
#include "live_wide.h"
#include "orbit_math.h"

extern "C" {
#include "plat_host.h"
unsigned plat_frames(void); /* save.c */
/* plat_core.c: the backdrop's commands for a heading and pitch (host memory) */
int port_backdrop_gfx(void *out, int max, unsigned dyn, int level, int heading, int pitch, short vtx[4][3]);
}

namespace live {

namespace {

using orbit::Vec3;

/* game state (fixed N64 addresses, host order; the game's names, where it has
 * one, in the comments: tools_port/names.txt) */
template <typename T> T rd(uint32_t a) {
    return *reinterpret_cast<volatile T *>(uintptr_t(a));
}
#define BC_MODE rd<unsigned long long>(0x80364A90u)   /* gGameMode */
#define BC_LEVEL rd<int>(0x802E8BDCu)                 /* levelno */
/* gDynamicBufs[2] */
constexpr uint32_t kDyn = 0x803156F8u, kDynSize = 0x21498u, kDynDl = 0x48B0u, kDynDlEnd = 0xA3A0u, kDynView = 0x140u,
                   kDynProj = 0x80u, kDynCull = 0x180u;

RT64::Application *g_app;
Config *g_cfg;
SDL_Window *g_window;
Config g_padCfg;          /* the bindings without the camera's inputs (orbit active) */
bool g_log;
bool g_test;              /* --orbit-test */
bool g_repeatable;        /* --orbit-repeatable */
bool g_cullAll;           /* --orbit-cull all: draw every cell (testing; as before the view test) */
std::atomic<unsigned> g_checkPictures, g_checkMissPictures, g_checkMissCells;   /* --orbit-log */
unsigned g_gameCellsChecked, g_gameCellsOutside;                                 /* --orbit-log */
OrbitPerf g_perf;         /* the host work since the start (main thread; orbitPerfTake, orbitFrameTake) */
std::atomic<uint64_t> g_viewTicks, g_views;   /* RT64's render thread: time in viewHook (QPC ticks), calls */

/* ---- the game's camera, per frame ------------------------------------------- */
struct Cam {
    bool valid = false;
    unsigned frame = 0;
    double view[4][4] = {};   /* the frame's view matrix (DynamicBuf.unk140) */
    double proj[4][4] = {};   /* ... and projection (DynamicBuf.unk80, guPerspective) */
    double cullView[4][4] = {};   /* ... and culling view (DynamicBuf.cullView: the eye pushed back) */
    Vec3 eye, target;
    bool snap = false, onFoot = false, play = false;
    unsigned long long mode = 0;
    int level = 0;
};
Cam g_built[2];           /* by DynamicBuf (host_camera_hook) */
Cam g_last;               /* the last frame drawn */
unsigned g_cuts;

void mtxToDouble(uint32_t addr, double m[4][4]) {
    const uint32_t *w = reinterpret_cast<const uint32_t *>(uintptr_t(addr));
    for (int n = 0; n < 16; n++) {
        const uint32_t iw = w[n / 2], fw = w[8 + n / 2];
        const int16_t hi = int16_t((n & 1) ? (iw & 0xFFFF) : (iw >> 16));
        const uint16_t lo = uint16_t((n & 1) ? (fw & 0xFFFF) : (fw >> 16));
        m[n / 4][n % 4] = double((int32_t(hi) * 65536) | lo) / 65536.0;
    }
}

/* normal play: the camera follows the player (no front end, map, intro or
 * ending views) */
bool playMode(unsigned long long mode) {
    return mode == 4;
}

void cameraHook(int snap) {
    const unsigned dyn = rd<uint8_t>(0x8035805Cu) & 1;   /* gBufferIndex */
    Cam &c = g_built[dyn];
    c.valid = false;
    mtxToDouble(kDyn + dyn * kDynSize + kDynView, c.view);
    mtxToDouble(kDyn + dyn * kDynSize + kDynProj, c.proj);
    mtxToDouble(kDyn + dyn * kDynSize + kDynCull, c.cullView);
    Vec3 fwd;
    if (!orbit::viewEye(c.view, c.eye, fwd)) return;
    /* gCamAtX/Y/Z */
    c.target = {rd<float>(0x80365084u), rd<float>(0x80365088u), rd<float>(0x8036508Cu)};
    c.snap = snap != 0;
    c.mode = BC_MODE;
    c.level = BC_LEVEL;
    c.onFoot = rd<uint8_t>(0x80364456u) == 0;   /* gCurVehicle */
    c.play = playMode(c.mode);
    c.frame = plat_frames();
    c.valid = std::isfinite(c.target.x) && std::isfinite(c.target.y) && std::isfinite(c.target.z);
}

/* ---- the orbit, shared with RT64's render thread ------------------------------ */
orbit::Orbit g_orbit;
orbit::Limits g_lim;
bool g_active;            /* orbit mode, in play, the game's camera known */
double g_yawRate, g_pitchRate;   /* degrees a second now (for pictures between steps) */
LARGE_INTEGER g_freq;

/* the cells a frame's drawing list had, with their boxes (--orbit-log: each
 * picture's view is checked against them) */
struct CellBox {
    Vec3 lo, hi;
};
struct CellSet {
    std::vector<CellBox> boxes;
    std::vector<uint8_t> drawn;
    double tanX = 0, tanY = 0, farDist = 0;
    Vec3 eye, target;             /* the view the list was made with ... */
    double margin = 0, back = 0;  /* ... and its margins */
};

/* what each rendered frame (workload) needs */
struct Entry {
    uint64_t workload = 0;
    Cam cur, prev;
    bool active = false;
    std::shared_ptr<const CellSet> cells;   /* --orbit-log */
    orbit::Orbit orbit;                     /* --orbit-repeatable: the orbit when the frame was made */
    orbit::Limits lim;
};
std::mutex g_mutex;       /* g_ring, g_shared */
std::vector<Entry> g_ring;
struct Shared {
    orbit::Orbit orbit;
    orbit::Limits lim;
    double yawRate = 0, pitchRate = 0;
    LARGE_INTEGER stepTime = {};
} g_shared;
Cam g_prevDrawn;          /* the frame drawn before (main thread) */
std::atomic<unsigned> g_hooked;   /* pictures drawn with the orbit's view (render thread) */
unsigned g_bdFrames;      /* frames whose backdrop followed the orbit */

double secondsSince(const LARGE_INTEGER &t) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return double(now.QuadPart - t.QuadPart) / double(g_freq.QuadPart);
}

bool sameView(const interop::float4x4 &a, const double b[4][4]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            const double x = double(a[r][c]), y = b[r][c];
            if (std::fabs(x - y) > 1e-3 * std::max(1.0, std::fabs(y))) return false;
        }
    return true;
}

void toFloat(const double m[4][4], interop::float4x4 &out) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) out[r][c] = float(m[r][c]);
}

void viewHookTimed(const RT64::ViewHookParams &p, interop::float4x4 &view, interop::float4x4 &proj,
                   interop::float4x4 &prevView, interop::float4x4 &prevProj);
/* RT64's render thread: every perspective projection of every picture */
void viewHook(const RT64::ViewHookParams &p, interop::float4x4 &view, interop::float4x4 &proj,
              interop::float4x4 &prevView, interop::float4x4 &prevProj) {
    LARGE_INTEGER t0, t1;
    QueryPerformanceCounter(&t0);
    viewHookTimed(p, view, proj, prevView, prevProj);
    QueryPerformanceCounter(&t1);
    g_viewTicks += uint64_t(t1.QuadPart - t0.QuadPart);
    g_views++;
}
void viewHookTimed(const RT64::ViewHookParams &p, interop::float4x4 &view, interop::float4x4 &proj,
                   interop::float4x4 &prevView, interop::float4x4 &prevProj) {
    (void) proj;
    (void) prevProj;
    Entry e;
    Shared s;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        const Entry *found = nullptr;
        for (const Entry &x : g_ring)
            if (x.workload == p.workloadId) found = &x;
        if (found == nullptr)   /* a re-submitted workload (the settings menu's pause): the newest of that view */
            for (auto it = g_ring.rbegin(); it != g_ring.rend() && found == nullptr; ++it)
                if (sameView(*p.gameView, it->cur.view)) found = &*it;
        if (found == nullptr || !found->active) return;
        if (!sameView(*p.gameView, found->cur.view)) return;   /* another projection (shadow, sub-view) */
        e = *found;
        s = g_shared;
    }
    double extra = std::min(secondsSince(s.stepTime), 0.05);
    if (g_repeatable) {   /* --orbit-repeatable: the orbit as it was when the frame was made (the same pictures every run) */
        s.orbit = e.orbit;
        s.lim = e.lim;
        extra = 0;
    }
    Vec3 curEye, curTarget;
    auto camFor = [&](float w, interop::float4x4 &out, bool cur) {
        orbit::Orbit::Game g;
        g.target = orbit::lerp(e.prev.target, e.cur.target, w);
        g.eye = orbit::lerp(e.prev.eye, e.cur.eye, w);
        Vec3 eye;
        s.orbit.eyeFor(g, s.yawRate * extra, s.pitchRate * extra, s.lim, eye);
        double m[4][4];
        orbit::lookAt(eye, g.target, m);
        toFloat(m, out);
        if (cur) curEye = eye, curTarget = g.target;
    };
    camFor(p.curFrameWeight, view, true);
    camFor(p.prevFrameWeight, prevView, false);
    g_hooked++;
    /* --orbit-log: every cell this picture's view can see must be in the
     * frame's drawing list */
    if (e.cells) {
        const CellSet &cs = *e.cells;
        const orbit::ViewVolume exact(curEye, curTarget, cs.tanX, cs.tanY, 0.0, 0.0, cs.farDist);
        unsigned missed = 0;
        size_t first = 0;
        for (size_t i = 0; i < cs.boxes.size(); i++)
            if (!cs.drawn[i] && exact.boxVisibleExact(cs.boxes[i].lo, cs.boxes[i].hi)) {
                if (missed++ == 0) first = i;
            }
        g_checkPictures++;
        if (missed != 0) {
            g_checkMissCells += missed;
            if (g_checkMissPictures++ < 10) {
                const CellBox &b = cs.boxes[first];
                host_log("live: orbit drawing list: a picture of frame %u sees %u cells not drawn (cell %u box %.0f %.0f "
                         "%.0f .. %.0f %.0f %.0f; picture eye %.1f %.1f %.1f at %.1f %.1f %.1f; list eye %.1f %.1f %.1f at "
                         "%.1f %.1f %.1f, margin %.1f back %.1f)\n",
                         e.cur.frame, missed, unsigned(first), b.lo.x, b.lo.y, b.lo.z, b.hi.x, b.hi.y, b.hi.z, curEye.x,
                         curEye.y, curEye.z, curTarget.x, curTarget.y, curTarget.z, cs.eye.x, cs.eye.y, cs.eye.z,
                         cs.target.x, cs.target.y, cs.target.z, cs.margin, cs.back);
            }
        }
    }
}

/* ---- ground height (read-only copies of the game's tests) --------------------- */
using orbit::triHeight;

/* every building's collision triangles: the gObjects records (0x803F4030,
 * 0xFC bytes each, up to gObjectsEnd at 0x803F7654) hold the range [+4, +8) of 0x60-byte triangles (77E20.c
 * CollisionTri: s32 vertices at +0x28 in world / 4 units, kept where the
 * building is) */
template <typename F> void forEachObstacle(F f) {
    const uint32_t end = rd<uint32_t>(0x803F7654u);
    if (end < 0x803F4030u || end > 0x803F4030u + 0xFCu * 64) return;
    for (uint32_t o = 0x803F4030u; o + 0xFC <= end; o += 0xFC) {
        const uint32_t a0 = rd<uint32_t>(o + 4), a1 = rd<uint32_t>(o + 8);
        if (a0 < 0x80000000u || a1 < a0 || a1 > 0x80800000u || a1 - a0 > 0x60u * 4096) continue;
        for (uint32_t a = a0; a + 0x60 <= a1; a += 0x60) f(reinterpret_cast<const int32_t *>(uintptr_t(a + 0x28)));
    }
}

/* the obstacles' triangles in drawing units, binned by x/z (orbit::TriGrid):
 * read again before each step's queries (the game moves and breaks them
 * between frames).  Testing every triangle for each of a step's many height
 * queries (~800 x 1700-2200 triangles) took most of a frame's time. */
orbit::TriGrid g_obstacles;
void groundRefresh();
std::vector<int32_t> g_obstRaw, g_obstNew;   /* the vertices the grid was made from; this step's */
void obstaclesRead() {
    g_obstNew.clear();
    forEachObstacle([&](const int32_t *v) { g_obstNew.insert(g_obstNew.end(), v, v + 9); });
    if (g_obstNew != g_obstRaw || g_obstacles.size() * 9 != g_obstNew.size()) {   /* (mostly unchanged) */
        g_obstRaw.swap(g_obstNew);
        g_perf.obstacleBuilds++;
        g_obstacles.clear();
        for (size_t i = 0; i + 9 <= g_obstRaw.size(); i += 9) {
            const int32_t *v = &g_obstRaw[i];
            const Vec3 p[3] = {{v[0] / 8.0, v[1] / 8.0, v[2] / 8.0}, {v[3] / 8.0, v[4] / 8.0, v[5] / 8.0},
                               {v[6] / 8.0, v[7] / 8.0, v[8] / 8.0}};
            g_obstacles.add(p);
        }
        g_obstacles.build();
    }
    groundRefresh();
}

/* the level's ground: the collision grid (62740.c ground_scan_grid): cells of
 * gGroundCellW x gGroundCellD world units, gGroundCols columns, lists in
 * gGroundCells[] of 0x14-byte triangles (9 s16 in 1/32 world units).  A
 * height query tests the list of the cell it is in (up to ~500 triangles);
 * each cell's list is also kept binned (orbit::TriGrid, the same triangles),
 * checked against the game's bytes before each step and made again if they
 * changed. */
template <typename F> void groundCellTris(uint32_t a, uint32_t end, F f) {
    if (a >= 0x80000000u && end >= a && end - a < 0x40000 && end < 0x80800000u) {
        end -= 4;
        for (; a + 0x14 <= end; a += 0x14) {
            const int16_t *t = reinterpret_cast<const int16_t *>(uintptr_t(a));
            const Vec3 p[3] = {{double(t[0]), double(t[1]), double(t[2])},
                               {double(t[3]), double(t[4]), double(t[5])},
                               {double(t[6]), double(t[7]), double(t[8])}};
            f(p);
        }
    }
}
struct GroundCell {
    uint32_t a = 0, end = 0;
    std::vector<uint8_t> raw;     /* the list's bytes it was made from */
    orbit::TriGrid grid;
};
struct GroundCache {
    uint32_t cw = 0, cd = 0;
    int cols = 0, rows = 0;
    std::vector<GroundCell> cells;
} g_ground;

bool groundGrid(uint32_t &cw, uint32_t &cd, int &cols, int &rows) {
    cw = rd<uint32_t>(0x803BE718u), cd = rd<uint32_t>(0x803BE71Cu);
    cols = rd<uint16_t>(0x803BE720u), rows = rd<uint16_t>(0x803BE722u);
    return cw != 0 && cd != 0 && cols > 0 && rows > 0 && cols * rows < 0x10000;
}

void groundRefresh() {
    uint32_t cw, cd;
    int cols, rows;
    if (!groundGrid(cw, cd, cols, rows)) {
        g_ground = GroundCache();
        return;
    }
    if (cw != g_ground.cw || cd != g_ground.cd || cols != g_ground.cols || rows != g_ground.rows) {
        g_ground = GroundCache();
        g_ground.cw = cw, g_ground.cd = cd, g_ground.cols = cols, g_ground.rows = rows;
        g_ground.cells.resize(size_t(cols) * rows);
    }
    const uint32_t *cells = reinterpret_cast<const uint32_t *>(uintptr_t(0x803BDB10u));
    for (int i = 0; i < cols * rows; i++) {
        GroundCell &g = g_ground.cells[i];
        const uint32_t a = cells[i], end = cells[i + 1];
        const bool ok = a >= 0x80000000u && end >= a && end - a < 0x40000 && end < 0x80800000u;
        const size_t n = ok ? end - a : 0;
        if (a == g.a && end == g.end && n == g.raw.size() &&
            (n == 0 || memcmp(g.raw.data(), reinterpret_cast<const void *>(uintptr_t(a)), n) == 0))
            continue;
        g.a = a, g.end = end;
        g_perf.groundBuilds++;
        g.raw.assign(reinterpret_cast<const uint8_t *>(uintptr_t(a)), reinterpret_cast<const uint8_t *>(uintptr_t(a)) + n);
        g.grid.clear();
        groundCellTris(a, end, [&](const Vec3 *p) { g.grid.add(p); });
        g.grid.build();
    }
}

/* drawing units (world / 32); the highest surface under x, z that is at most
 * kReach above y (a bridge deck or roof higher above isn't the ground there) */
constexpr double kReach = 40.0;
double g_lastTerrain, g_lastObstacle;   /* (--orbit-log) the two sources of the last query */
double groundHeight(double x, double y, double z) {
    g_perf.queries++;
    double best = -1e31, h;
    g_lastTerrain = g_lastObstacle = -1e31;
    bool obstacle = false;
    auto take = [&](double v) {
        if (v <= y + kReach) best = std::max(best, v);
        double &l = obstacle ? g_lastObstacle : g_lastTerrain;
        if (v <= y + kReach) l = std::max(l, v);
    };
    uint32_t cw, cd;
    int cols, rows;
    if (groundGrid(cw, cd, cols, rows)) {
        const int cx = std::max(0, std::min(cols - 1, int(std::floor(x * 32.0 / cw))));
        const int cz = std::max(0, std::min(rows - 1, int(std::floor(z * 32.0 / cd))));
        const uint32_t *cells = reinterpret_cast<const uint32_t *>(uintptr_t(0x803BDB10u));
        const uint32_t a = cells[cz * cols + cx], end = cells[cz * cols + cx + 1];
        const size_t i = size_t(cz) * cols + cx;
        if (cols == g_ground.cols && rows == g_ground.rows && cw == g_ground.cw && cd == g_ground.cd &&
            g_ground.cells[i].a == a && g_ground.cells[i].end == end)
            g_ground.cells[i].grid.forEachAt(x, z, [&](const Vec3 *p) {
                if (triHeight(p, x, z, h)) take(h);
            });
        else
            groundCellTris(a, end, [&](const Vec3 *p) {
                if (triHeight(p, x, z, h)) take(h);
            });
    }
    /* buildings and other obstacles: the 0x60-byte collision triangles
     * (77E20.c CollisionTri: s32 vertices at +0x28 in world / 4 units), as
     * obstaclesRead last read them */
    obstacle = true;
    g_obstacles.forEachAt(x, z, [&](const Vec3 *p) {
        double hh;
        if (triHeight(p, x, z, hh)) take(hh);
    });
    return best;
}

/* --orbit-log: every query also against all the obstacle triangles one by one
 * (as before the grid); the answers must be the same */
unsigned g_heightChecks, g_heightMismatches;
double groundHeightChecked(double x, double y, double z) {
    const double h = groundHeight(x, y, z), lastT = g_lastTerrain, lastO = g_lastObstacle;
    double o = -1e31;
    forEachObstacle([&](const int32_t *v) {
        const Vec3 p[3] = {{v[0] / 8.0, v[1] / 8.0, v[2] / 8.0}, {v[3] / 8.0, v[4] / 8.0, v[5] / 8.0},
                           {v[6] / 8.0, v[7] / 8.0, v[8] / 8.0}};
        double hh;
        if (triHeight(p, x, z, hh) && hh <= y + kReach) o = std::max(o, hh);
    });
    double t = -1e31;
    uint32_t cw, cd;
    int cols, rows;
    if (groundGrid(cw, cd, cols, rows)) {
        const int cx = std::max(0, std::min(cols - 1, int(std::floor(x * 32.0 / cw))));
        const int cz = std::max(0, std::min(rows - 1, int(std::floor(z * 32.0 / cd))));
        const uint32_t *cells = reinterpret_cast<const uint32_t *>(uintptr_t(0x803BDB10u));
        groundCellTris(cells[cz * cols + cx], cells[cz * cols + cx + 1], [&](const Vec3 *p) {
            double hh;
            if (triHeight(p, x, z, hh) && hh <= y + kReach) t = std::max(t, hh);
        });
    }
    const double all = std::max(t, o);
    g_heightChecks++;
    if (all != h || o != lastO || t != lastT) {
        if (g_heightMismatches++ < 10)
            host_log("live: orbit collision grid: query %.3f %.3f %.3f gave %.6f, every triangle %.6f (frame %u)\n", x, y, z,
                     h, all, plat_frames());
    }
    return h;
}

/* ---- input ------------------------------------------------------------------ */
double padValue(const Config &c, SDL_GameController *pad, const PadInput &in) {
    if (in.button >= 0) return SDL_GameControllerGetButton(pad, SDL_GameControllerButton(in.button)) ? 1.0 : 0.0;
    const int v = SDL_GameControllerGetAxis(pad, SDL_GameControllerAxis(in.axis));
    if (in.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || in.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return v > 8000 ? std::min(1.0, (v - 8000) / 24000.0) : 0.0;
    const double d = double(v) * in.dir;
    if (d <= c.deadzone) return 0.0;
    return std::min(1.0, (d - c.deadzone) / (32767.0 - c.deadzone));
}

double actionValue(const Config &c, const PadSet &pads, const Uint8 *keys, int act) {
    double v = 0;
    if (keys != nullptr)
        for (SDL_Scancode sc : c.keys[act])
            if (keys[sc]) v = 1.0;
    for (SDL_GameController *pad : pads.active(c))
        for (const PadInput &in : c.pad[act]) v = std::max(v, padValue(c, pad, in));
    return v;
}

double g_mouseX, g_mouseY, g_wheel;   /* since the last step */
bool g_dragging;
bool g_resetHeld;

/* --orbit-test (g_test: above); sweep: the stress test (orbit::sweepAt, from the steps in play) */
double g_testYaw, g_testPitch = 1e9, g_testZoom = 0;
bool g_sweep;
unsigned g_sweepSteps;

void buildPadConfig() {
    g_padCfg = *g_cfg;
    for (int b = 0; b < kButtons; b++) {
        std::vector<PadInput> keep;
        for (const PadInput &in : g_padCfg.pad[b]) {
            bool cam = false;
            for (int a = kCameraActions; a < A_COUNT; a++)
                for (const PadInput &c : g_cfg->pad[a])
                    cam |= c.button == in.button && c.axis == in.axis && c.dir == in.dir;
            if (!cam) keep.push_back(in);
        }
        g_padCfg.pad[b] = keep;
        std::vector<SDL_Scancode> keys;
        for (SDL_Scancode k : g_padCfg.keys[b]) {
            bool cam = false;
            for (int a = kCameraActions; a < A_COUNT; a++)
                for (SDL_Scancode c : g_cfg->keys[a]) cam |= c == k;
            if (!cam) keys.push_back(k);
        }
        g_padCfg.keys[b] = keys;
    }
}

/* ---- host work (g_perf: above; the frame-time report, --perf-log) ----------------- */
struct PerfTimer {
    double &acc;
    LARGE_INTEGER t0;
    explicit PerfTimer(double &a) : acc(a) {
        QueryPerformanceCounter(&t0);
    }
    ~PerfTimer() {
        LARGE_INTEGER t1;
        QueryPerformanceCounter(&t1);
        acc += double(t1.QuadPart - t0.QuadPart) * 1e3 / double(g_freq.QuadPart);
    }
};

unsigned listLength(const short *l) {
    unsigned n = 0;
    while (l[n] != -1) n++;
    return n;
}

/* ---- how fast the orbit's view moves (the drawing list's margins) ---------------
 * Each step: the angle the view direction turned and the distance the eye
 * moved (drawing units); the drawing list takes the largest of the last
 * kMotionSteps, as speeds. */
constexpr int kMotionSteps = 8;
struct Motion {
    double turn[kMotionSteps] = {}, move[kMotionSteps] = {};
    int next = 0;
    bool have = false;
    Vec3 eye, dir;
    double turnPerSec() const { return *std::max_element(turn, turn + kMotionSteps) * 60.0; }
    double movePerStep() const { return *std::max_element(move, move + kMotionSteps); }
    void reset() { *this = Motion(); }
    void add(const Vec3 &e, const Vec3 &target) {
        const Vec3 d = orbit::normalize(target - e);
        if (have) {
            const double c = std::max(-1.0, std::min(1.0, orbit::dot(d, dir)));
            turn[next] = std::acos(c) * 180.0 / 3.14159265358979;
            move[next] = orbit::length(e - eye);
            next = (next + 1) % kMotionSteps;
        }
        eye = e, dir = d, have = true;
    }
} g_motion;

/* ---- the drawing list (PORT_DRAW_CELLS) ------------------------------------------
 * The game's drawing paths (5FD50.c's level lists, 77E20.c's buildings) draw
 * the cells of this list: the game's own (its camera's visibility walk,
 * gVisibleCells, unchanged: the game's logic reads only that one) and the
 * cells the orbit camera can see, ascending, -1 at the end.  The cells (the
 * level grid: lvl_build_visible_dl's cells, cull_test_cells' boxes) get the
 * game's own box (its grid of ground height ranges, as cull_box_corners) grown
 * by the collision triangles of the buildings standing in them (taller than
 * the ground, or reaching over the cell's edge).  The orbit's view is a
 * pyramid (orbit::ViewVolume) with the frame's projection, widescreen and
 * interpolation width (host_cull_wide), from the newest camera the game made
 * (the frame before's: the level's lists are built before cam_update), and
 * margins for that and for the pictures shown until the next list (the
 * view may turn and the eye move meanwhile): 20
 * degrees plus what the view turned in 0.25 s at its recent speed, the apex
 * 4 units plus 10 steps of the eye's recent motion back, and the far plane.
 * Turning faster than that can cover (a margin over 70 degrees: from 200
 * degrees a second), or the camera unknown: every cell, as before.  (The
 * cells cost little to draw: the margins are generous; a sudden mouse flick
 * from rest is the one case they can lag.)  Made once a frame (the
 * paths ask for it once per record), for the game's list it was given. */
struct DrawList {
    unsigned frame = ~0u;
    bool fresh = false;           /* made since the last frame task (orbitAfterTask) */
    uint32_t lvl = 0;
    std::vector<short> game;      /* the game's list it was made for */
    std::vector<short> list;      /* the drawing list */
    std::shared_ptr<CellSet> cells;
    unsigned all = 0, made = 0;   /* lists with every cell (fast turns, camera unknown), lists made */
} g_draw;

uint32_t levelData() {
    return rd<uint32_t>(0x80358074u);   /* gLevelData */
}

/* the n cells' boxes: the game's (ground heights) grown by its buildings' collision triangles */
void cellBoxes(uint32_t lvl, unsigned cols, unsigned rows, std::vector<CellBox> &boxes) {
    const int xs = rd<int16_t>(lvl + 4), zs = rd<int16_t>(lvl + 6);
    const uint32_t grid = lvl + rd<uint32_t>(lvl + 0x48);
    boxes.resize(size_t(cols) * rows);
    for (unsigned z = 0; z < rows; z++)
        for (unsigned x = 0; x < cols; x++) {
            const uint32_t g = grid + ((z * cols + x) << 2);
            CellBox &b = boxes[z * cols + x];
            /* cull_box_corners: unsigned 32-bit products, as s32 */
            b.lo = {double(int32_t(x * uint32_t(xs))), double(rd<int16_t>(g)), double(int32_t(z * uint32_t(zs)))};
            b.hi = {double(int32_t((x + 1) * uint32_t(xs))), double(rd<int16_t>(g + 2)),
                    double(int32_t((z + 1) * uint32_t(zs)))};
        }
    /* gObjects records: cell at +0xE8, collision triangles [+4, +8) (forEachObstacle) */
    const uint32_t end = rd<uint32_t>(0x803F7654u);
    if (end < 0x803F4030u || end > 0x803F4030u + 0xFCu * 64) return;
    for (uint32_t o = 0x803F4030u; o + 0xFC <= end; o += 0xFC) {
        const unsigned cell = rd<uint8_t>(o + 0xE8);
        if (cell >= boxes.size()) continue;
        const uint32_t a0 = rd<uint32_t>(o + 4), a1 = rd<uint32_t>(o + 8);
        if (a0 < 0x80000000u || a1 < a0 || a1 > 0x80800000u || a1 - a0 > 0x60u * 4096) continue;
        CellBox &b = boxes[cell];
        for (uint32_t a = a0; a + 0x60 <= a1; a += 0x60) {
            const int32_t *v = reinterpret_cast<const int32_t *>(uintptr_t(a + 0x28));
            for (int k = 0; k < 3; k++) {
                const Vec3 p = {v[k * 3] / 8.0, v[k * 3 + 1] / 8.0, v[k * 3 + 2] / 8.0};
                b.lo = {std::min(b.lo.x, p.x), std::min(b.lo.y, p.y), std::min(b.lo.z, p.z)};
                b.hi = {std::max(b.hi.x, p.x), std::max(b.hi.y, p.y), std::max(b.hi.z, p.z)};
            }
        }
    }
}

bool sameList(const short *a, const std::vector<short> &b) {
    for (size_t i = 0; i < b.size(); i++)
        if (a[i] != b[i]) return false;
    return true;   /* (b ends with its -1) */
}

short *drawCells(short *game) {
    if (!g_active) return game;
    PerfTimer timer(g_perf.cellsMs);
    const unsigned cols = rd<uint16_t>(0x803BE714u), rows = rd<uint16_t>(0x803BE716u);   /* gLevelCols, gLevelRows */
    const unsigned n = cols * rows;
    if (n == 0 || n > 0x7FFF) return game;
    const unsigned frame = plat_frames();
    const uint32_t lvl = levelData();
    if (frame == g_draw.frame && lvl == g_draw.lvl && !g_draw.list.empty() && sameList(game, g_draw.game))
        return g_draw.list.data();
    g_draw.frame = frame;
    g_draw.lvl = lvl;
    g_draw.game.assign(game, game + listLength(game) + 1);
    g_draw.made++;
    auto cs = std::make_shared<CellSet>();
    cs->drawn.assign(n, 0);
    for (size_t i = 0; i + 1 < g_draw.game.size(); i++)
        if (unsigned(uint16_t(g_draw.game[i])) < n) cs->drawn[uint16_t(g_draw.game[i])] = 1;
    /* the newest camera the game made: the level's lists are built before the
     * frame's own camera (cam_update), so it is the frame before's; the
     * margins cover the difference */
    const Cam &c = g_built[0].valid && (!g_built[1].valid || int(g_built[0].frame - g_built[1].frame) > 0) ? g_built[0]
                                                                                                             : g_built[1];
    double nearZ = 0, farZ = 0;
    const double s = -c.proj[2][3];
    bool all = g_cullAll || !c.valid || frame - c.frame > 4 || !orbit::perspectiveRange(c.proj, nearZ, farZ) ||
               !(std::fabs(c.proj[0][0]) > 1e-9 && std::fabs(c.proj[1][1]) > 1e-9);
    const double margin = 20.0 + g_motion.turnPerSec() * 0.25;
    if (margin > 70.0) all = true;
    if (all) {
        g_draw.all++;
        std::fill(cs->drawn.begin(), cs->drawn.end(), uint8_t(1));
    } else {
        cellBoxes(lvl, cols, rows, cs->boxes);
        Vec3 eye;
        g_orbit.eyeFor({c.eye, c.target}, 0, 0, g_lim, eye);
        cs->tanX = host_cull_wide * s / std::fabs(c.proj[0][0]);
        cs->tanY = s / std::fabs(c.proj[1][1]);
        cs->farDist = farZ * 1.02;
        cs->eye = eye;
        cs->target = c.target;
        cs->margin = margin;
        cs->back = 4.0 + 10.0 * g_motion.movePerStep();
        const orbit::ViewVolume vol(eye, c.target, cs->tanX, cs->tanY, margin, cs->back, cs->farDist);
        for (unsigned i = 0; i < n; i++)
            if (!cs->drawn[i] && vol.boxVisible(cs->boxes[i].lo, cs->boxes[i].hi)) cs->drawn[i] = 1;
        Vec3 cullEye, cullFwd;
        if (g_log && orbit::viewEye(c.cullView, cullEye, cullFwd)) {
            /* the same test from the game's culling view (its eye pushed back: cam_eye_scale) should see
             * (about) the cells the game listed (it walks the grid over a few frames: lags a little) */
            const orbit::ViewVolume gameVol(cullEye, c.target, cs->tanX, cs->tanY, 0.0, 0.0, cs->farDist);
            for (size_t i = 0; i + 1 < g_draw.game.size(); i++) {
                const unsigned k = uint16_t(g_draw.game[i]);
                if (k >= n) continue;
                g_gameCellsChecked++;
                if (!gameVol.boxVisible(cs->boxes[k].lo, cs->boxes[k].hi)) g_gameCellsOutside++;
            }
        }
    }
    g_draw.list.clear();
    for (unsigned i = 0; i < n; i++)
        if (cs->drawn[i]) g_draw.list.push_back(short(i));
    g_draw.list.push_back(-1);
    g_perf.lists++;
    g_perf.listsAll += all;
    g_perf.cells += unsigned(g_draw.list.size() - 1);
    g_perf.gameCells += unsigned(g_draw.game.size() - 1);
    g_draw.fresh = true;
    if (g_log && !all) g_draw.cells = cs;
    else g_draw.cells.reset();
    return g_draw.list.data();
}

/* ---- the backdrop -------------------------------------------------------------- */
struct Backdrop {
    bool on = false;          /* this frame's task: give the backdrop the orbit's heading and pitch */
    bool vtxDone = false, fillDone = false;
    int cimg = 0;             /* G_SETCIMG commands seen */
    int heading = 0, pitch = 0;
    int height = 0;           /* the horizon (lines from the top) of the orbit's view */
    RT64::DisplayList gfx[64];   /* the backdrop's commands for the orbit's view (port_backdrop_gfx) */
    int count = 0;
    short vtx[4][3] = {};
    RT64::DisplayList cycle = {}, fillColor = {};
    bool haveCycle = false, haveColor = false;
} g_bd;

/* the game's heading (0..0xFFF) and pitch values of a camera (00000.c
 * cam_update: gCamYaw, gCamPitch) */
void headingPitch(const Vec3 &eye, const Vec3 &at, int &heading, int &pitch) {
    const Vec3 d = eye - at;
    double yaw, pd;
    orbit::angles(d, yaw, pd);
    heading = int(yaw * 4096.0 / 360.0) & 0xFFF;
    const double dist = std::max(1.0, orbit::length(d));
    const double a = std::asin(std::min(1.0, std::fabs(d.y) / dist)) * 180.0 / 3.14159265358979;
    const double b = std::asin(std::min(1.0, std::fabs(d.y) / 20000.0)) * 180.0 / 3.14159265358979;
    const double p = (a - b) * 2048.0 / 90.0;
    pitch = int(d.y >= 0 ? p : -p);
}

/* the orbit's backdrop vertex values into gBackdropVtx[0 or 1] at `a`
 * (physical), saved first; put back by restoreVtx */
int16_t g_vtxSaved[4][8];
int16_t *patchVtx(uint32_t a, const short vals[4][3]) {
    int16_t *v = reinterpret_cast<int16_t *>(uintptr_t(0x80000000u | a));
    memcpy(g_vtxSaved, v, sizeof g_vtxSaved);
    for (int i = 0; i < 4; i++) {
        int16_t *q = v + i * 8;   /* Vtx: ob[3], flag, tc[2], cn[4] (host order) */
        if (vals[i][0] != 0x7FFF) q[1] = vals[i][0];
        q[4] = vals[i][1];
        q[5] = vals[i][2];
    }
    return v;
}
void restoreVtx(int16_t *v) {
    memcpy(v, g_vtxSaved, sizeof g_vtxSaved);   /* RDRAM as the game left it */
}

bool commandHook(RT64::State *state, RT64::DisplayList **dl, const GbiOriginal *originals) {
    if (!g_bd.on) return false;
    const uint32_t w0 = (*dl)->w0, w1 = (*dl)->w1;
    const uint8_t op = uint8_t(w0 >> 24);
    if (g_bd.fillDone) return false;
    /* (the frame's first fill rectangle clears the depth image: the backdrop
     * comes after the second G_SETCIMG, the frame buffer's) */
    if (op == 0xFF) g_bd.cimg++;
    if (g_bd.cimg < 2) return false;
    if (op == 0x04 && !g_bd.vtxDone) {   /* G_VTX: the game drew its backdrop quad (gBackdropVtx[buf]) */
        const uint32_t a = state->rsp->fromSegmented(w1) & 0x7FFFFF;
        if (a != 0x2FA820 && a != 0x2FA860) return false;
        g_bd.vtxDone = true;
        if (g_bd.count == 0) {   /* the orbit view shows no sky: an empty quad */
            int16_t *v = reinterpret_cast<int16_t *>(uintptr_t(0x80000000u | a));
            short flat[4][3];
            for (int i = 0; i < 4; i++) flat[i][0] = v[1], flat[i][1] = v[i * 8 + 4], flat[i][2] = v[i * 8 + 5];
            int16_t *p = patchVtx(a, flat);
            originals[op](state, dl);
            restoreVtx(p);
            return true;
        }
        int16_t *p = patchVtx(a, g_bd.vtx);
        originals[op](state, dl);
        restoreVtx(p);
        g_bdFrames++;
        return true;
    }
    /* the fill mode the ground colour is drawn with (to restore after a backdrop of our own) */
    if (op == 0xBA && !g_bd.vtxDone) g_bd.cycle = **dl, g_bd.haveCycle = true;
    if (op == 0xF7) g_bd.fillColor = **dl, g_bd.haveColor = true;
    if (op == 0xF6) {   /* G_FILLRECT: the ground colour below the horizon */
        g_bd.fillDone = true;
        if (!g_bd.vtxDone && g_bd.count > 0 && g_bd.haveCycle && g_bd.haveColor) {
            /* the game drew no backdrop (its camera saw no sky) but the orbit's
             * does: the commands the game would have written, from host memory */
            int16_t *p = patchVtx(0x2FA820, g_bd.vtx);
            for (int i = 0; i < g_bd.count; i++) {
                RT64::DisplayList *c = &g_bd.gfx[i];
                const uint32_t op = c->w0 >> 24, w1 = c->w1;
                const RT64::WorkloadQueue &q = *state->ext.workloadQueue;
                const uint32_t first = q.workloads[q.writeCursor].drawData.vertexCount();
                originals[op](state, &c);
                if (op == 0x04) wideAfterVertex(state, w1, first);   /* widescreen: as wide as the picture */
            }
            restoreVtx(p);
            RT64::DisplayList *c = &g_bd.cycle;
            originals[0xBA](state, &c);
            c = &g_bd.fillColor;
            originals[0xF7](state, &c);
            g_bdFrames++;
        }
        const int uly = std::max(0, std::min(239, g_bd.height <= 0 ? 0 : g_bd.height - 1));
        RT64::DisplayList cmd = **dl;
        cmd.w1 = (w1 & ~0xFFFu) | (uint32_t(uly) << 2);
        RT64::DisplayList *p = &cmd;
        originals[op](state, &p);
        return true;
    }
    return false;
}

/* ---- billboards ---------------------------------------------------------------
 * Smoke, explosions and particles (60F60.c fx_build_matrix, 43A60.c) are quads
 * the game turns to face its own camera (a rotation by the camera's pitch
 * and heading in their model matrix).  In the orbit view they are turned to
 * face the orbit camera instead: a model matrix whose rotation is (up to its
 * uniform scale) about the inverse of the game view's rotation gets
 * A' = A Rg Ro^T (row vectors: world = local A, camera = world R; for a
 * billboard A = s Rg^T, so A' = s Ro^T).  Only RT64's copy of the matrix
 * changes. */
struct Billboards {
    bool on = false;          /* this frame's task */
    double rg[3][3] = {}, x[3][3] = {};   /* the game view's rotation; Rg Ro^T */
    unsigned turned = 0;
} g_bb;

void matrixHook(RT64::State *state, uint8_t params) {
    if (!g_bb.on) return;
    RT64::RSP &rsp = *state->rsp;
    if (params & rsp.projMask) return;
    hlslpp::float4x4 &m = rsp.modelMatrixStack[rsp.modelMatrixStackSize - 1];
    double a[3][3], b[3][3];
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) a[r][c] = float(m[r][c]);
    double s = 0;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            b[r][c] = a[r][0] * g_bb.rg[0][c] + a[r][1] * g_bb.rg[1][c] + a[r][2] * g_bb.rg[2][c];
            if (r == c) s += b[r][c] / 3.0;
        }
    if (s <= 1e-6) return;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            if (std::fabs(b[r][c] - (r == c ? s : 0.0)) > 0.12 * s) return;   /* not facing the game camera */
    float f[4][4];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            f[r][c] = (r < 3 && c < 3) ? float(a[r][0] * g_bb.x[0][c] + a[r][1] * g_bb.x[1][c] + a[r][2] * g_bb.x[2][c])
                                       : float(m[r][c]);
    m = hlslpp::float4x4(f[0][0], f[0][1], f[0][2], f[0][3], f[1][0], f[1][1], f[1][2], f[1][3], f[2][0], f[2][1],
                         f[2][2], f[2][3], f[3][0], f[3][1], f[3][2], f[3][3]);
    rsp.modelViewProjChanged = true;
    g_bb.turned++;
}

}  // namespace

/* ---- the interface ---------------------------------------------------------- */
void orbitSetup(RT64::Application *app, Config *cfg, SDL_Window *window) {
    g_app = app;
    g_cfg = cfg;
    g_window = window;
    QueryPerformanceFrequency(&g_freq);
    orbitApply();
}

void orbitApply() {
    const bool on = g_cfg->cameraMode == 1;
    host_camera_hook = on ? cameraHook : nullptr;
    host_draw_cells_hook = on ? drawCells : nullptr;
    RT64::ViewHook = on ? viewHook : nullptr;
    interpSetCommandHook(on ? commandHook : nullptr, on ? matrixHook : nullptr);
    if (!on) {
        g_active = false;
        g_orbit.reset(true);
        g_built[0].valid = g_built[1].valid = false;
        std::lock_guard<std::mutex> lock(g_mutex);
        g_ring.clear();
    }
    g_lim.pitchMin = g_cfg->camPitchMin;
    g_lim.pitchMax = g_cfg->camPitchMax;
    g_lim.zoomMin = g_cfg->camZoomMin;
    g_lim.zoomMax = g_cfg->camZoomMax;
    g_lim.recentre = g_cfg->camRecentre;
    g_lim.recentreDelay = g_cfg->camRecentreDelay;
    buildPadConfig();
    host_log("live: camera %s\n", on ? "orbit (free camera, render only)" : "original");
}

bool orbitActive() {
    return g_active;
}

const Config &orbitPadConfig() {
    return g_active ? g_padCfg : *g_cfg;
}

bool orbitEvent(const SDL_Event &e) {
    if (!g_active || !g_cfg->camMouse) {
        if (g_dragging) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            g_dragging = false;
        }
        return false;
    }
    switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT) {
            g_dragging = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
            return true;
        }
        if (e.button.button == SDL_BUTTON_MIDDLE) {
            g_resetHeld = true;
            return true;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (g_dragging && (SDL_GetMouseState(nullptr, nullptr) & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) == 0) {
            g_dragging = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        return true;
    case SDL_MOUSEMOTION:
        if (g_dragging) {
            g_mouseX += e.motion.xrel;
            g_mouseY += e.motion.yrel;
            return true;
        }
        break;
    case SDL_MOUSEWHEEL:
        g_wheel += e.wheel.preciseY != 0.0f ? e.wheel.preciseY : float(e.wheel.y);
        return true;
    default:
        break;
    }
    return false;
}

void orbitVi(const PadSet &pads, const Uint8 *keys) {
    /* one step a retrace (the game's virtual time: 60 a second in real time
     * when paced; --no-pace test runs then turn the camera the same per frame) */
    const double dt = 1.0 / 60.0;
    if (g_cfg->cameraMode != 1) return;
    const Cam &c = g_last;
    const bool was = g_active;
    g_active = c.valid && c.play;
    if (!g_active) {
        if (was) host_log("live: orbit camera inactive (game mode %08llX, frame %u)\n", c.mode, plat_frames());
        g_orbit.reset(false);
        g_motion.reset();
        g_yawRate = g_pitchRate = 0;
        g_mouseX = g_mouseY = g_wheel = 0;
        return;
    }
    if (!was) {
        unsigned tris = 0;
        forEachObstacle([&](const int32_t *) { tris++; });
        /* the ground grid's lists (groundHeight) */
        unsigned ground = 0, most = 0;
        const int cols = rd<uint16_t>(0x803BE720u), rows = rd<uint16_t>(0x803BE722u);
        const uint32_t *cells = reinterpret_cast<const uint32_t *>(uintptr_t(0x803BDB10u));
        if (cols > 0 && rows > 0 && cols * rows < 0x10000)
            for (int i = 0; i < cols * rows; i++) {
                const uint32_t a = cells[i], e = cells[i + 1];
                if (a < 0x80000000u || e < a + 4 || e - a >= 0x40000) continue;
                const unsigned n = (e - 4 - a) / 0x14;
                ground += n;
                most = std::max(most, n);
            }
        const uint32_t lvl = levelData();
        const bool lvlOk = lvl >= 0x80000000u && lvl < 0x80800000u;
        host_log("live: orbit camera active (level %d, frame %u, vehicle %u; %u obstacle triangles, ground grid %dx%d "
                 "with %u triangles, at most %u in a cell; drawing cells %ux%u of %dx%d units)\n",
                 c.level, plat_frames(), unsigned(rd<uint8_t>(0x80364456u)), tris, cols, rows, ground, most,
                 unsigned(rd<uint16_t>(0x803BE714u)), unsigned(rd<uint16_t>(0x803BE716u)),
                 lvlOk ? int(rd<int16_t>(lvl + 4)) : 0, lvlOk ? int(rd<int16_t>(lvl + 6)) : 0);
    }
    static bool lastFoot;
    if (c.onFoot != lastFoot) {
        lastFoot = c.onFoot;
        if (g_log) host_log("live: orbit camera: the player is %s (frame %u)\n", c.onFoot ? "on foot" : "in a vehicle",
                            plat_frames());
    }
    /* input: rates from the sticks and keys, steps from the mouse */
    const Config &cf = *g_cfg;
    const double sx = cf.camSensX / 100.0 * (cf.camInvertX ? -1.0 : 1.0);
    const double sy = cf.camSensY / 100.0 * (cf.camInvertY ? -1.0 : 1.0);
    auto curve = [](double v) { return v * std::fabs(v) * 0.6 + v * 0.4; };
    const double turn = curve(actionValue(cf, pads, keys, A_CAM_RIGHT) - actionValue(cf, pads, keys, A_CAM_LEFT));
    const double look = curve(actionValue(cf, pads, keys, A_CAM_UP) - actionValue(cf, pads, keys, A_CAM_DOWN));
    orbit::Input in;
    /* stick right: the view turns right (the eye goes to smaller yaw); look up: the eye goes down */
    g_yawRate = -turn * 150.0 * sx;
    g_pitchRate = -look * 90.0 * sy;
    in.yaw = g_yawRate * dt - g_mouseX * 0.25 * sx;
    in.pitch = g_pitchRate * dt + g_mouseY * 0.2 * sy;
    in.zoom = (actionValue(cf, pads, keys, A_CAM_ZOOM_OUT) - actionValue(cf, pads, keys, A_CAM_ZOOM_IN)) * 1.2 * dt -
              g_wheel * 0.15;
    const bool reset = actionValue(cf, pads, keys, A_CAM_RESET) > 0.5 || g_resetHeld;
    static bool resetWas;
    in.reset = reset && !resetWas;
    resetWas = reset;
    g_resetHeld = false;
    g_mouseX = g_mouseY = g_wheel = 0;
    if (g_sweep) {   /* --orbit-test sweep: turning, tilting and zooming by itself */
        const orbit::Sweep sw = orbit::sweepAt(g_sweepSteps++ * dt, g_lim);
        g_testYaw = sw.yawRate;
        g_testPitch = sw.pitch;
        g_testZoom = sw.zoom;
    }
    if (g_test) {   /* --orbit-test: turning by itself */
        in = orbit::Input();
        g_yawRate = g_testYaw;
        g_pitchRate = 0;
        in.yaw = g_testYaw * dt;
        if (g_testPitch < 1e8) in.pitch = (g_testPitch - g_orbit.pitch()) * std::min(1.0, dt * 4.0) + 1e-9;
        if (g_testZoom > 0) in.zoom = std::log2(g_testZoom / g_orbit.zoom());
    }
    orbit::Limits lim = g_lim;
    lim.recentre = g_test ? false : (cf.camRecentre || (c.onFoot && cf.camRecentreOnFoot));
    orbit::Orbit::Game g{c.eye, c.target};
    {
        PerfTimer timer(g_perf.stepMs);
        {
            PerfTimer readTimer(g_perf.readMs);
            obstaclesRead();
        }
        if (g_log) g_orbit.step(g, in, dt, lim, groundHeightChecked);
        else g_orbit.step(g, in, dt, lim, groundHeight);
    }
    if (g_orbit.engaged() || std::fabs(g_orbit.zoom() - 1.0) > 1e-6) {
        g_perf.steps++;
        g_perf.closeSteps += g_orbit.distanceRatio() < 0.3;
    }
    {
        Vec3 eye;
        g_orbit.eyeFor(g, 0, 0, lim, eye);
        g_motion.add(eye, c.target);
    }
    std::lock_guard<std::mutex> lock(g_mutex);
    g_shared.orbit = g_orbit;
    g_shared.lim = lim;
    g_shared.yawRate = g_orbit.engaged() ? g_yawRate : 0.0;
    g_shared.pitchRate = g_orbit.engaged() ? g_pitchRate : 0.0;
    QueryPerformanceCounter(&g_shared.stepTime);
}

void orbitBeforeTask(uint32_t dl, bool frameDl) {
    g_bd = Backdrop();
    g_bb.on = false;
    if (g_cfg->cameraMode != 1 || !frameDl) return;
    PerfTimer timer(g_perf.taskMs);
    dl &= 0x3FFFFFF;
    const unsigned dyn = (dl - (kDyn & 0x3FFFFFF)) / kDynSize;
    if (dyn > 1 || !g_built[dyn].valid) return;
    Cam c = g_built[dyn];
    /* a cut (the game snapped its camera, a new mode or level): no blend from
     * the frame before, the orbit starts from the game's camera again */
    const bool cut = !g_last.valid || c.snap || c.mode != g_last.mode || c.level != g_last.level;
    if (cut && g_orbit.engaged() && c.play) {
        g_orbit.reset(false);
        g_cuts++;
        if (g_log) host_log("live: orbit camera reset by a cut (frame %u)\n", plat_frames());
    }
    g_prevDrawn = cut ? c : g_last;
    g_last = c;
    if (g_active && c.play && (g_orbit.engaged() || std::fabs(g_orbit.zoom() - 1.0) > 1e-6)) {
        orbit::Orbit::Game g{c.eye, c.target};
        Vec3 eye;
        g_orbit.eyeFor(g, 0, 0, g_lim, eye);
        headingPitch(eye, c.target, g_bd.heading, g_bd.pitch);
        g_bd.on = true;
        const double scroll = 120.0 - double(int16_t(g_bd.pitch)) * 0.22 + rd<float>(0x8036BFC0u);
        g_bd.height = int(std::max(0.0, scroll));
        /* billboards: Rg (the game view's rotation) and Rg Ro^T */
        double ro[4][4];
        orbit::lookAt(eye, c.target, ro);
        g_bb.on = true;
        for (int r = 0; r < 3; r++)
            for (int k = 0; k < 3; k++) {
                g_bb.rg[r][k] = c.view[r][k];
                g_bb.x[r][k] = c.view[r][0] * ro[k][0] + c.view[r][1] * ro[k][1] + c.view[r][2] * ro[k][2];
            }
        static_assert(sizeof(RT64::DisplayList) == 8, "a Gfx command");
        g_bd.count = port_backdrop_gfx(g_bd.gfx, 64, kDyn + dyn * kDynSize, c.level, g_bd.heading, g_bd.pitch,
                                       g_bd.vtx);
        if (g_log && plat_frames() % 30 == 0) {
            int gh, gp;
            headingPitch(c.eye, c.target, gh, gp);
            obstaclesRead();
            const double ge = groundHeight(eye.x, eye.y, eye.z), te = g_lastTerrain, ob = g_lastObstacle;
            const double gt = groundHeight(c.target.x, c.target.y, c.target.z);
            host_log("live: orbit frame %u%s: target %.0f %.0f %.0f, game eye %.0f %.0f %.0f (heading %X pitch %d), "
                     "orbit eye %.0f %.0f %.0f (heading %X pitch %d), yaw %.1f pitch %.1f zoom %.2f pull-in %.2f lift %.0f "
                     "rise %.1f, "
                     "ground under eye %.0f (terrain %.0f, obstacles %.0f), under the target %.0f\n",
                     plat_frames(), c.onFoot ? " (on foot)" : "", c.target.x, c.target.y, c.target.z, c.eye.x, c.eye.y, c.eye.z, gh, gp, eye.x,
                     eye.y, eye.z, g_bd.heading, g_bd.pitch, g_orbit.yaw(), g_orbit.pitch(), g_orbit.zoom(),
                     g_orbit.distanceRatio(), g_orbit.lift(), g_orbit.rise(), ge, te, ob, gt);
        }
    }
}

void orbitAfterTask(bool frameDl) {
    if (g_cfg->cameraMode != 1 || !frameDl || !g_last.valid) return;
    static uint64_t lastId;
    const uint64_t id = g_app->state->workloadId;
    if (id == lastId) return;
    lastId = id;
    Entry e;
    e.workload = id;
    e.cur = g_last;
    e.prev = g_prevDrawn;
    e.active = g_active && g_last.play;
    if (g_log && g_draw.fresh) e.cells = g_draw.cells;   /* the list this frame's lists were made with */
    g_draw.fresh = false;
    std::lock_guard<std::mutex> lock(g_mutex);
    e.orbit = g_shared.orbit;
    e.lim = g_shared.lim;
    g_ring.push_back(e);
    if (g_ring.size() > 8) g_ring.erase(g_ring.begin());
}

bool orbitSetTest(const std::string &spec) {
    if (spec == "sweep") {   /* the stress test: orbit::sweepAt */
        g_test = g_sweep = true;
        return true;
    }
    double y = 0, p = 1e9, z = 0;
    const int n = sscanf(spec.c_str(), "%lf:%lf:%lf", &y, &p, &z);
    if (n < 1) return false;
    g_test = true;
    g_testYaw = y;
    g_testPitch = n >= 2 ? p : 1e9;
    g_testZoom = n >= 3 ? z : 0;
    return true;
}

void orbitSetLog(bool on) {
    g_log = on;
}

OrbitPerf OrbitPerf::operator-(const OrbitPerf &o) const {
    OrbitPerf d;
    d.stepMs = stepMs - o.stepMs, d.cellsMs = cellsMs - o.cellsMs, d.readMs = readMs - o.readMs;
    d.taskMs = taskMs - o.taskMs, d.viewMs = viewMs - o.viewMs;
    d.lists = lists - o.lists, d.cells = cells - o.cells, d.gameCells = gameCells - o.gameCells;
    d.queries = queries - o.queries, d.listsAll = listsAll - o.listsAll;
    d.obstacleBuilds = obstacleBuilds - o.obstacleBuilds, d.groundBuilds = groundBuilds - o.groundBuilds;
    d.views = views - o.views;
    d.steps = steps - o.steps, d.closeSteps = closeSteps - o.closeSteps;
    return d;
}

namespace {
OrbitPerf perfTotals() {
    OrbitPerf p = g_perf;
    p.viewMs = double(g_viewTicks.load()) * 1e3 / double(g_freq.QuadPart ? g_freq.QuadPart : 1);
    p.views = unsigned(g_views.load());
    return p;
}
OrbitPerf g_perfTaken, g_frameTaken;
}  // namespace

OrbitPerf orbitPerfTake() {
    const OrbitPerf p = perfTotals(), d = p - g_perfTaken;
    g_perfTaken = p;
    return d;
}

OrbitPerf orbitFrameTake() {
    const OrbitPerf p = perfTotals(), d = p - g_frameTaken;
    g_frameTaken = p;
    return d;
}

void orbitSummary() {
    if (g_cfg == nullptr || g_cfg->cameraMode != 1) return;
    host_log("live: orbit camera: %u pictures drawn with its view, %u cuts, backdrop turned with it in %u frames, "
             "%u billboards turned to it\n",
             g_hooked.load(), g_cuts, g_bdFrames, g_bb.turned);
    host_log("live: orbit drawing lists: %u made (%s), %u with every cell\n", g_draw.made,
             g_cullAll ? "every cell: --orbit-cull all" : "the game's cells and the orbit view's", g_draw.all);
    if (g_log)
        host_log("live: orbit checks: %u pictures checked against their drawing list, %u saw cells not drawn (%u "
                 "cells); %u collision queries, %u differing from a test of every triangle; of the game's listed "
                 "cells %u of %u outside the same test from the game's culling view\n",
                 g_checkPictures.load(), g_checkMissPictures.load(), g_checkMissCells.load(), g_heightChecks,
                 g_heightMismatches, g_gameCellsOutside, g_gameCellsChecked);
}

void orbitSetRepeatable(bool on) {
    g_repeatable = on;
}

bool orbitSetCull(const std::string &mode) {
    if (mode == "all") g_cullAll = true;
    else if (mode == "view") g_cullAll = false;
    else return false;
    return true;
}

}  // namespace live
