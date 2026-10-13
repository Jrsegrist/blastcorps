/* bc.exe's optional free orbit camera ([camera] mode = orbit; see
 * live_orbit.h and orbit_math.h).
 *
 * How it stays render-only:
 *  - the game's camera: at the end of the game's camera function (00000.c
 *    func_802507C8, PORT_ICUT_IF -> host_camera_hook) the eye (from the view
 *    matrix the game just built, DynamicBuf.unk140) and the point it looks at
 *    (D_80365084/88/8C) are copied for the DynamicBuf being built; read only;
 *  - the pictures: RT64 patch 0006 calls ViewHook for every perspective
 *    projection of every picture it renders (in-between pictures of a higher
 *    frame rate too).  A projection whose view is the frame's game view gets
 *    the orbit camera's view instead: target and game eye interpolated with
 *    the picture's weight, the orbit's direction and distance, and the input
 *    since the last step.  The projection is the game's own (fov, aspect,
 *    widescreen).  The game never sees any of it;
 *  - what the game culled: the level's geometry and the buildings whose cells
 *    the game's visibility walk didn't list would be missing from an orbit
 *    view.  The drawing paths (PORT_DRAW_CELLS, 5FD50.c, 77E20.c) get every
 *    cell of the level instead (as the game itself does while its own camera
 *    turns: func_802A45D4), unlisted buildings drawn as they are, without
 *    their per-object updates (the game leaves those alone: they stay as
 *    they are).  The game's logic keeps reading its own list (77E20.c's
 *    per-object updates run only for listed objects, so the list must not
 *    change);
 *  - the backdrop (2D2D810.c: a 2D quad scrolled by the camera's heading and
 *    pitch, and the ground colour below its horizon): while RT64 reads the
 *    frame, its vertices and the fill rectangle are given the orbit view's
 *    heading and pitch (the vertices are written just for RT64's read and put
 *    back right after it: RDRAM is as the game left it);
 *  - collision: the eye is kept above the level's ground triangles and the
 *    buildings' collision triangles (read-only copies of the game's height
 *    tests: 62740.c func_802AA094's grid, the 0x60-byte triangles at
 *    D_803B9890), pulled in along the line to the target.
 * The controls: in play, the camera bindings (default: the right stick) drive
 * the camera and not the N64 buttons bound to the same inputs (the C buttons'
 * right stick); outside play the right stick is the C buttons as before. */
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

#include "gbi/rt64_gbi.h"
#include "hle/rt64_application.h"
#include "gbi/rt64_display_list.h"
#include "hle/rt64_rsp.h"
#include "hle/rt64_state.h"
#include "render/rt64_projection_processor.h"

#include "live_interp.h"
#include "live_orbit.h"
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

/* game state (fixed N64 addresses, host order) */
template <typename T> T rd(uint32_t a) {
    return *reinterpret_cast<volatile T *>(uintptr_t(a));
}
#define BC_MODE rd<unsigned long long>(0x80364A90u)
#define BC_LEVEL rd<int>(0x802E8BDCu)
constexpr uint32_t kDyn = 0x803156F8u, kDynSize = 0x21498u, kDynDl = 0x48B0u, kDynDlEnd = 0xA3A0u, kDynView = 0x140u;

RT64::Application *g_app;
Config *g_cfg;
SDL_Window *g_window;
Config g_padCfg;          /* the bindings without the camera's inputs (orbit active) */
bool g_log;

/* ---- the game's camera, per frame ------------------------------------------- */
struct Cam {
    bool valid = false;
    unsigned frame = 0;
    double view[4][4] = {};   /* the frame's view matrix (DynamicBuf.unk140) */
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
    const unsigned dyn = rd<uint8_t>(0x8035805Cu) & 1;
    Cam &c = g_built[dyn];
    c.valid = false;
    mtxToDouble(kDyn + dyn * kDynSize + kDynView, c.view);
    Vec3 fwd;
    if (!orbit::viewEye(c.view, c.eye, fwd)) return;
    c.target = {rd<float>(0x80365084u), rd<float>(0x80365088u), rd<float>(0x8036508Cu)};
    c.snap = snap != 0;
    c.mode = BC_MODE;
    c.level = BC_LEVEL;
    c.onFoot = rd<uint8_t>(0x80364456u) == 0;
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

/* what each rendered frame (workload) needs */
struct Entry {
    uint64_t workload = 0;
    Cam cur, prev;
    bool active = false;
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

/* RT64's render thread: every perspective projection of every picture */
void viewHook(const RT64::ViewHookParams &p, interop::float4x4 &view, interop::float4x4 &proj,
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
    const double extra = std::min(secondsSince(s.stepTime), 0.05);
    auto camFor = [&](float w, interop::float4x4 &out) {
        orbit::Orbit::Game g;
        g.target = orbit::lerp(e.prev.target, e.cur.target, w);
        g.eye = orbit::lerp(e.prev.eye, e.cur.eye, w);
        Vec3 eye;
        s.orbit.eyeFor(g, s.yawRate * extra, s.pitchRate * extra, s.lim, eye);
        double m[4][4];
        orbit::lookAt(eye, g.target, m);
        toFloat(m, out);
    };
    camFor(p.curFrameWeight, view);
    camFor(p.prevFrameWeight, prevView);
    g_hooked++;
}

/* ---- ground height (read-only copies of the game's tests) --------------------- */
/* the height of triangle p at (x, z), or false outside it / vertical */
bool triHeight(const Vec3 p[3], double x, double z, double &h) {
    const double minX = std::min({p[0].x, p[1].x, p[2].x}), maxX = std::max({p[0].x, p[1].x, p[2].x});
    const double minZ = std::min({p[0].z, p[1].z, p[2].z}), maxZ = std::max({p[0].z, p[1].z, p[2].z});
    if (x < minX || x > maxX || z < minZ || z > maxZ) return false;
    const Vec3 n = orbit::cross(p[1] - p[0], p[2] - p[0]);
    if (std::fabs(n.y) < 1e-9 * (std::fabs(n.x) + std::fabs(n.z) + 1.0)) return false;
    /* inside in x/z: the three edge signs agree */
    auto side = [&](const Vec3 &a, const Vec3 &b) { return (b.x - a.x) * (z - a.z) - (b.z - a.z) * (x - a.x); };
    const double s0 = side(p[0], p[1]), s1 = side(p[1], p[2]), s2 = side(p[2], p[0]);
    const bool neg = s0 < -1e-9 || s1 < -1e-9 || s2 < -1e-9, pos = s0 > 1e-9 || s1 > 1e-9 || s2 > 1e-9;
    if (neg && pos) return false;
    h = p[0].y - (n.x * (x - p[0].x) + n.z * (z - p[0].z)) / n.y;
    return true;
}

/* every building's collision triangles: the D_803F4030 objects (0xFC bytes, up
 * to D_803F7654) hold the range [+4, +8) of 0x60-byte triangles (77E20.c
 * Unk803B9890: s32 vertices at +0x28 in world / 4 units, kept where the
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

/* drawing units (world / 32); the highest surface under x, z that is at most
 * kReach above y (a bridge deck or roof higher above isn't the ground there) */
constexpr double kReach = 40.0;
double g_lastTerrain, g_lastObstacle;   /* (--orbit-log) the two sources of the last query */
double groundHeight(double x, double y, double z) {
    double best = -1e31, h;
    g_lastTerrain = g_lastObstacle = -1e31;
    bool obstacle = false;
    auto take = [&](double v) {
        if (v <= y + kReach) best = std::max(best, v);
        double &l = obstacle ? g_lastObstacle : g_lastTerrain;
        if (v <= y + kReach) l = std::max(l, v);
    };
    /* the level's ground: the collision grid (62740.c func_802AA094): cells of
     * D_803BE718 x D_803BE71C world units, D_803BE720 columns, lists in
     * D_803BDB10[] of 0x14-byte triangles (9 s16 in 1/32 world units) */
    const uint32_t cw = rd<uint32_t>(0x803BE718u), cd = rd<uint32_t>(0x803BE71Cu);
    const int cols = rd<uint16_t>(0x803BE720u), rows = rd<uint16_t>(0x803BE722u);
    if (cw != 0 && cd != 0 && cols > 0 && rows > 0 && cols * rows < 0x10000) {
        const int cx = std::max(0, std::min(cols - 1, int(std::floor(x * 32.0 / cw))));
        const int cz = std::max(0, std::min(rows - 1, int(std::floor(z * 32.0 / cd))));
        const uint32_t *cells = reinterpret_cast<const uint32_t *>(uintptr_t(0x803BDB10u));
        uint32_t a = cells[cz * cols + cx], end = cells[cz * cols + cx + 1];
        if (a >= 0x80000000u && end >= a && end - a < 0x40000 && end < 0x80800000u) {
            end -= 4;
            for (; a + 0x14 <= end; a += 0x14) {
                const int16_t *t = reinterpret_cast<const int16_t *>(uintptr_t(a));
                const Vec3 p[3] = {{double(t[0]), double(t[1]), double(t[2])},
                                   {double(t[3]), double(t[4]), double(t[5])},
                                   {double(t[6]), double(t[7]), double(t[8])}};
                if (triHeight(p, x, z, h)) take(h);
            }
        }
    }
    /* buildings and other obstacles: the 0x60-byte collision triangles
     * (77E20.c Unk803B9890: s32 vertices at +0x28 in world / 4 units) */
    obstacle = true;
    forEachObstacle([&](const int32_t *v) {
        const Vec3 p[3] = {{v[0] / 8.0, v[1] / 8.0, v[2] / 8.0}, {v[3] / 8.0, v[4] / 8.0, v[5] / 8.0},
                           {v[6] / 8.0, v[7] / 8.0, v[8] / 8.0}};
        double hh;
        if (triHeight(p, x, z, hh)) take(hh);
    });
    return best;
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

/* --orbit-test */
bool g_test;
double g_testYaw, g_testPitch = 1e9, g_testZoom = 0;

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

/* ---- the drawing list: every cell (PORT_DRAW_CELLS) --------------------------- */
std::vector<short> g_allCells;
short *drawCells(short *game) {
    if (!g_active) return game;
    const unsigned n = unsigned(rd<uint16_t>(0x803BE714u)) * rd<uint16_t>(0x803BE716u);
    if (n == 0 || n > 0x7FFF) return game;
    if (g_allCells.size() != n + 1) {
        g_allCells.resize(n + 1);
        for (unsigned i = 0; i < n; i++) g_allCells[i] = short(i);
        g_allCells[n] = -1;
    }
    return g_allCells.data();
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
 * func_802507C8: D_80364452, D_80364454) */
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

/* the orbit's backdrop vertex values into D_802FA820[0 or 1] at `a`
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
    if (op == 0x04 && !g_bd.vtxDone) {   /* G_VTX: the game drew its backdrop quad (D_802FA820[buf]) */
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
                originals[c->w0 >> 24](state, &c);
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
    interpSetCommandHook(on ? commandHook : nullptr);
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
        g_yawRate = g_pitchRate = 0;
        g_mouseX = g_mouseY = g_wheel = 0;
        return;
    }
    if (!was) {
        unsigned tris = 0;
        forEachObstacle([&](const int32_t *) { tris++; });
        host_log("live: orbit camera active (level %d, frame %u, vehicle %u; %u obstacle triangles)\n", c.level,
                 plat_frames(), unsigned(rd<uint8_t>(0x80364456u)), tris);
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
    g_orbit.step(g, in, dt, lim, groundHeight);
    std::lock_guard<std::mutex> lock(g_mutex);
    g_shared.orbit = g_orbit;
    g_shared.lim = lim;
    g_shared.yawRate = g_orbit.engaged() ? g_yawRate : 0.0;
    g_shared.pitchRate = g_orbit.engaged() ? g_pitchRate : 0.0;
    QueryPerformanceCounter(&g_shared.stepTime);
}

void orbitBeforeTask(uint32_t dl, bool frameDl) {
    g_bd = Backdrop();
    if (g_cfg->cameraMode != 1 || !frameDl) return;
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
        static_assert(sizeof(RT64::DisplayList) == 8, "a Gfx command");
        g_bd.count = port_backdrop_gfx(g_bd.gfx, 64, kDyn + dyn * kDynSize, c.level, g_bd.heading, g_bd.pitch,
                                       g_bd.vtx);
        if (g_log && plat_frames() % 30 == 0) {
            int gh, gp;
            headingPitch(c.eye, c.target, gh, gp);
            const double ge = groundHeight(eye.x, eye.y, eye.z), te = g_lastTerrain, ob = g_lastObstacle;
            const double gt = groundHeight(c.target.x, c.target.y, c.target.z);
            host_log("live: orbit frame %u%s: target %.0f %.0f %.0f, game eye %.0f %.0f %.0f (heading %X pitch %d), "
                     "orbit eye %.0f %.0f %.0f (heading %X pitch %d), yaw %.1f pitch %.1f zoom %.2f pull-in %.2f lift %.0f, "
                     "ground under eye %.0f (terrain %.0f, obstacles %.0f), under the target %.0f\n",
                     plat_frames(), c.onFoot ? " (on foot)" : "", c.target.x, c.target.y, c.target.z, c.eye.x, c.eye.y, c.eye.z, gh, gp, eye.x,
                     eye.y, eye.z, g_bd.heading, g_bd.pitch, g_orbit.yaw(), g_orbit.pitch(), g_orbit.zoom(),
                     g_orbit.distanceRatio(), g_orbit.lift(), ge, te, ob, gt);
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
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ring.push_back(e);
    if (g_ring.size() > 8) g_ring.erase(g_ring.begin());
}

bool orbitSetTest(const std::string &spec) {
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

void orbitSummary() {
    if (g_cfg == nullptr || g_cfg->cameraMode != 1) return;
    host_log("live: orbit camera: %u pictures drawn with its view, %u cuts, backdrop turned with it in %u frames\n",
             g_hooked.load(), g_cuts, g_bdFrames);
}

}  // namespace live
