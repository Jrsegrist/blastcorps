/* bc.exe's level and model viewer (see live_viewer.h).
 *
 * States:
 *  - off;
 *  - load: a level was asked for (host_viewer_request: the game takes it at the end of a frame in a level or the
 *    attract demo, game/port.h PORT_VIEWER_FRAME) and the game runs, unpaced, silent, with no controller
 *    input, until that level has been in play (or its start) for a few frames;
 *  - capture: the game runs two more frames with every cell of the level in the drawing paths
 *    (PORT_DRAW_CELLS: as many as the game's display-list buffers hold) and its PORT_ITAG tags recorded;
 *  - frozen: the game waits inside the retrace hook (viewerVi, called from live_rt64.cpp liveVi) and the viewer
 *    runs its own loop: the graphics tasks of the last captured frame go to RT64 again every tick, the free
 *    camera's view replacing the game's (RT64::ViewHook), the categories turned off left out of the drawing
 *    (the command hook, live_interp.h), the far plane moved out; the panels and overlays are ImGui over the
 *    picture.  RDRAM is only read.
 * Leaving a viewer opened over play lets the game go on from where it was held (two frames on: the capture);
 * once the viewer loaded a level, leaving restarts bc.exe (ViewerEnv::restart).
 *
 * No GPL code: written against RT64's (MIT) API and the game's own data. */
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "gbi/rt64_display_list.h"
#include "gbi/rt64_gbi.h"
#include "hle/rt64_application.h"
#include "hle/rt64_rsp.h"
#include "hle/rt64_state.h"
#include "hle/rt64_workload_queue.h"
#include "imgui/imgui.h"
#include "render/rt64_projection_processor.h"

#include "level_edit.h"
#include "live_interp.h"
#include "live_menu.h"
#include "live_orbit.h"
#include "live_viewer.h"
#include "live_wide.h"
#include "orbit_math.h"
#include "viewer_records.h"

#include "load/port_mods.h"

extern "C" {
#include "plat_host.h"
unsigned plat_frames(void); /* save.c */
}

namespace live {

namespace {

using viewer::FlyCam;
using viewer::Record;
using viewer::Vec;

/* ---- the game's state (fixed N64 addresses, host order; tools_port/names.txt) ---- */
template <typename T> T rd(uint32_t a) {
    return *reinterpret_cast<volatile T *>(uintptr_t(a));
}
constexpr uint32_t kGameMode = 0x80364A90u, kLevelNo = 0x802E8BDCu, kLevelData = 0x80358074u,
                   kHeapPtr = 0x80358070u, kFrontEnd = 0x80370C50u, kFramesInMode = 0x80358060u,
                   kLevelInfo = 0x802E8F94u, kGlobeLevels = 0x8020D810u, kLevelDl = 0x80358030u,
                   kLevelFileDl = 0x803BE6E0u, kGObjects = 0x803F4030u, kGObjectsEnd = 0x803F7654u,
                   kGVehicles = 0x80364460u, kGVehiclesEnd = 0x803649D0u, kGroundCells = 0x803BDB10u,
                   kGroundCellW = 0x803BE718u, kLevelCols = 0x803BE714u, kCarrierX = 0x803EF6DCu,
                   kDriverPos = 0x803ED808u, kDigger = 0x803643D4u;
constexpr uint32_t kDyn = 0x803156F8u, kDynSize = 0x21498u, kDynDl = 0x48B0u, kDynDlEnd = 0xA3A0u, kDynView = 0x140u,
                   kDynProj = 0x80u, kDynLevelCalls = 0xA4E0u;
constexpr uint64_t MODE_LEVEL = 4, MODE_LEVEL_START = 0x2000, MODE_MAP_VIEW = 0x100;

bool inRam(uint32_t a, uint32_t n = 1) {
    return a >= 0x80000000u && a + n <= 0x80800000u && a + n >= a;
}

void mtxToDouble(uint32_t addr, double m[4][4]) {
    const uint32_t *w = reinterpret_cast<const uint32_t *>(uintptr_t(addr));
    for (int n = 0; n < 16; n++) {
        const uint32_t iw = w[n / 2], fw = w[8 + n / 2];
        const int16_t hi = int16_t((n & 1) ? (iw & 0xFFFF) : (iw >> 16));
        const uint16_t lo = uint16_t((n & 1) ? (fw & 0xFFFF) : (fw >> 16));
        m[n / 4][n % 4] = double((int32_t(hi) * 65536) | lo) / 65536.0;
    }
}

ViewerEnv g_env;

/* ---- what is drawn: categories -------------------------------------------------- */
enum Cat { CAT_TERRAIN, CAT_BUILDINGS, CAT_VEHICLES, CAT_EFFECTS, CAT_OTHER, CAT_SKY, CAT_HUD, CAT_COUNT };
const char *const kCatNames[CAT_COUNT] = {"terrain", "buildings", "vehicles", "effects", "other", "sky", "hud"};
const char *const kCatLabels[CAT_COUNT] = {"Terrain (the level's geometry)", "Buildings and objects",
                                           "Vehicles (and their shadows)", "Effects (smoke, particles, people)",
                                           "Other world items (RDUs, TNT, crates, trails, water)", "Sky",
                                           "HUD and text"};
/* the tag kinds (include/game/port.h PORT_ITAG_*) */
enum {
    K_NONE = 0, K_HUD = 1, K_OBJ, K_SHADOW, K_SHADOWFE, K_BUILDING, K_BPART, K_FALL, K_RISE, K_BILLBOARD,
    K_PARTICLE, K_PEOPLE, K_GSHADOW, K_MARKER, K_RADAR, K_TERRAIN = 100
};
int catOf(uint32_t kind, bool sky) {
    switch (kind) {
    case K_TERRAIN: return CAT_TERRAIN;
    case K_HUD: case K_RADAR: return CAT_HUD;
    case K_OBJ: case K_SHADOW: case K_SHADOWFE: return CAT_VEHICLES;
    case K_BUILDING: case K_BPART: case K_FALL: case K_RISE: case K_GSHADOW: return CAT_BUILDINGS;
    case K_BILLBOARD: case K_PARTICLE: case K_PEOPLE: case K_MARKER: return CAT_EFFECTS;
    default: return sky ? CAT_SKY : CAT_OTHER;
    }
}

/* ---- options (the panels, the test script) -------------------------------------- */
struct Options {
    bool show[CAT_COUNT] = {true, true, true, true, true, true, false};
    bool farOut = true;           /* the far plane out at kFar (else the game's 10000) */
    bool plain = true;            /* a plain background instead of the sky (the game's sky is a flat picture
                                     for its own camera's heading and pitch: wrong for any other view) */
    bool allCells = true;         /* capture with every cell that fits (else the game's own visible cells) */
    bool markers = true, labels = true, collision = false, zones = false;
    bool kinds[viewer::REC_KINDS] = {true, true, true, true, true, true, true, true, true, false, false, false,
                                     false, false};
    double speed = 600;           /* drawing units a second */
    double labelDist = 1200;      /* labels within this distance */
    bool ui = true;               /* the panels */
} g_opt;
constexpr double kFar = 120000.0;

/* ---- the state ------------------------------------------------------------------ */
enum State { S_OFF, S_LOAD, S_CAPTURE, S_FROZEN };
State g_state = S_OFF;
bool g_wanted;                    /* the menu's button: open over play */
int g_menuLevel = 0;              /* the menu's level picker */
bool g_loadedLevel;               /* the viewer loaded a level: leaving restarts bc.exe */
int g_bootLevel = -2;             /* --viewer LEVEL (-2: not given) */
int g_wantLevel = -1;             /* the level the load waits for */
unsigned g_loadStart, g_captureFrames, g_freezes, g_replays, g_shots, g_loads;
unsigned g_capturedFrame;         /* plat_frames() of the frozen frame */
bool g_recapture;                 /* a recapture asked for (the frozen loop returns) */
bool g_leave, g_quitAfter;
bool g_resetCam = true;           /* the next freeze starts from the game's camera (a new level, a new entry) */

/* the graphics tasks of the frames */
struct Task {
    unsigned ucode, data, dl;
    bool frameDl;
};
std::vector<Task> g_cur, g_frozenTasks;
/* the tags of the display list being built and of the frozen one (address -> kind, index) */
struct Tag {
    uint32_t kind, index;
};
std::unordered_map<uint32_t, Tag> g_tagsBuilding, g_tags;
bool g_recordTags;
unsigned g_tagCount;

/* the frozen frame */
struct Frozen {
    bool level = false;            /* a level frame (its DynamicBuf known): the free camera works */
    unsigned dyn = 0;
    double view[4][4] = {}, proj[4][4] = {};
    unsigned long long mode = 0;
    int levelNo = -1;
    uint32_t origin = 0;           /* the VI origin of the frozen picture */
    uint32_t lvl = 0;              /* gLevelData */
    uint32_t terrain[12] = {};     /* the level lists (both buffers) and fixed lists: terrain */
    uint32_t levelCalls = 0;
    unsigned cellsDrawn = 0, cells = 0;
} g_fz;

/* ---- the records ---------------------------------------------------------------- */
std::vector<Record> g_recs;       /* the level file's, then gObjects', then gVehicles' */
std::vector<std::string> g_notes;
viewer::LevelHeader g_hdr;
int g_sel = -1;                   /* the selected record */
std::string g_levelNames[60];
bool g_namesRead;
struct Tri {
    float v[9];
    uint8_t src;                  /* 0 grid A, 1 grid B, 2 building */
};
std::vector<Tri> g_tris;

/* ---- the camera ----------------------------------------------------------------- */
FlyCam g_cam;
struct ModelView {
    bool on = false;
    uint32_t kind = 0, index = 0; /* the owner shown (K_OBJ: a gVehicles index, K_BUILDING: a gObjects one) */
    int rec = -1;                 /* the record it was opened from */
    double yaw = 30, pitch = 20, dist = 0;
    Vec centre;
    double radius = 0;
    bool measured = false;        /* centre and radius from its drawing (the vertices it drew) */
    double lo[3], hi[3];
    unsigned vtx = 0;
    std::string title;
} g_mv;

/* the view the render thread gives RT64 (viewHook) */
struct ViewShare {
    bool on = false;
    float game[4][4] = {};        /* the frozen frame's view: projections with it get ours */
    float view[4][4] = {};
    bool farOut = false;
    bool haveProj = false;
    double proj[4][4] = {};       /* the projection RT64 drew with (for the overlays) */
    unsigned hooked = 0;
} g_vs;
std::mutex g_vsMutex;

void toFloat(const double m[4][4], float out[4][4]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) out[r][c] = float(m[r][c]);
}

bool sameView(const interop::float4x4 &a, const float b[4][4]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            const double x = double(a[r][c]), y = double(b[r][c]);
            if (std::fabs(x - y) > 1e-3 * std::max(1.0, std::fabs(y))) return false;
        }
    return true;
}

void viewHook(const RT64::ViewHookParams &p, interop::float4x4 &view, interop::float4x4 &proj,
              interop::float4x4 &prevView, interop::float4x4 &prevProj) {
    std::lock_guard<std::mutex> lock(g_vsMutex);
    if (!g_vs.on || !sameView(*p.gameView, g_vs.game)) return;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) view[r][c] = prevView[r][c] = g_vs.view[r][c];
    double pm[4][4];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) pm[r][c] = double(proj[r][c]);
    if (g_vs.farOut&& viewer::setFar(pm, kFar))
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++) proj[r][c] = float(pm[r][c]);
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            prevProj[r][c] = proj[r][c];
            g_vs.proj[r][c] = double(proj[r][c]);
        }
    g_vs.haveProj = true;
    g_vs.hooked++;
}

/* the camera's view now (model view: orbiting the model) */
void currentView(double m[4][4]) {
    if (g_mv.on) {
        FlyCam c;
        c.yaw = g_mv.yaw, c.pitch = -g_mv.pitch;
        c.lookAt(g_mv.centre, g_mv.dist);
        c.view(m);
        return;
    }
    g_cam.view(m);
}

void publishView() {
    double m[4][4];
    currentView(m);
    std::lock_guard<std::mutex> lock(g_vsMutex);
    toFloat(m, g_vs.view);
    g_vs.farOut= g_opt.farOut;
}

/* ---- the ground (2-D records, the carrier's route) --------------------------------- */
double groundAt(double x, double z, double fallback) {
    const uint32_t cw = rd<uint32_t>(kGroundCellW), cd = rd<uint32_t>(kGroundCellW + 4);
    const int cols = rd<uint16_t>(kGroundCellW + 8), rows = rd<uint16_t>(kGroundCellW + 10);
    if (cw == 0 || cd == 0 || cols <= 0 || rows <= 0 || cols * rows >= 0x10000) return fallback;
    const int cx = std::max(0, std::min(cols - 1, int(std::floor(x * 32.0 / cw))));
    const int cz = std::max(0, std::min(rows - 1, int(std::floor(z * 32.0 / cd))));
    const uint32_t a = rd<uint32_t>(kGroundCells + 4u * uint32_t(cz * cols + cx)),
                   e = rd<uint32_t>(kGroundCells + 4u * uint32_t(cz * cols + cx + 1));
    if (!inRam(a) || e < a + 4 || e - a > 0x40000 || !inRam(e)) return fallback;
    double best = -1e31, h;
    for (uint32_t t = a; t + 0x14 <= e - 4; t += 0x14) {
        const int16_t *v = reinterpret_cast<const int16_t *>(uintptr_t(t));
        const orbit::Vec3 p[3] = {{double(v[0]), double(v[1]), double(v[2])}, {double(v[3]), double(v[4]), double(v[5])},
                                  {double(v[6]), double(v[7]), double(v[8])}};
        if (orbit::triHeight(p, x, z, h)) best = std::max(best, h);
    }
    return best > -1e30 ? best : fallback;
}

/* ---- reading the level ------------------------------------------------------------ */
uint32_t levelData() {
    return rd<uint32_t>(kLevelData);
}

/* the level file's extent: up to the heap pointer (what follows it is the level's own heap: the parser only
 * reads inside the header's sections) */
size_t levelSpan(uint32_t lvl) {
    const uint32_t heap = rd<uint32_t>(kHeapPtr);
    if (!inRam(lvl, 0xC8) || heap <= lvl || !inRam(heap - 1)) return 0;
    return size_t(heap - lvl);
}

void readNames() {
    if (g_namesRead || rd<uint8_t>(kFrontEnd) == 0) return;
    g_namesRead = true;
    unsigned ok = 0;
    for (int i = 0; i < 60; i++) {
        const uint32_t p = rd<uint32_t>(kGlobeLevels + uint32_t(i) * 0x30u + 4);
        std::string s;
        if (inRam(p, 32))
            for (int k = 0; k < 31; k++) {
                const char c = rd<char>(p + uint32_t(k));
                if (c == 0) break;
                if (c < 0x20 || c > 0x7E) {
                    s.clear();
                    break;
                }
                s += c;
            }
        if (!s.empty()) ok++;
        g_levelNames[i] = s;
    }
    host_log("live: viewer: %u level names read from the globe's table\n", ok);
}

std::string kindName(unsigned k) {
    switch (k) {
    case 1: return "mission";
    case 0x80: return "timed mission";
    case 2: return "race";
    case 4: return "targets";
    case 8: return "damage";
    case 0x10: return "RDUs";
    case 0x20: return "buildings";
    case 0x40: return "power-pill maze";
    case 0: return "no kind (sequence or special)";
    default: {
        char b[16];
        snprintf(b, sizeof b, "kind 0x%X", k);
        return b;
    }
    }
}

std::string levelLabel(int i) {
    char b[160];
    const unsigned k = rd<uint8_t>(kLevelInfo + uint32_t(i) * 0x44u);
    snprintf(b, sizeof b, "%2d  %s  (%s)", i, g_levelNames[i].empty() ? "level" : g_levelNames[i].c_str(),
             kindName(k).c_str());
    return b;
}

/* the vehicle a type-1 spawn stands for on a bonus level: the lowest vehicle bit of the level's LevelInfo, else
 * Sideswipe */
int bonusVehicle(int level) {
    const uint32_t bits = rd<uint32_t>(kLevelInfo + uint32_t(level) * 0x44u + 0x2C);
    for (int t = 1; t <= 18; t++)
        if (t != 6 && t != 7 && t != 11 && t != 12 && t < 17 && (bits >> t) & 1) return t;
    return 1;
}

/* ---- the level editor (level_edit.h): its state here, the rest further down -------------------------- */
edit::Editor g_ed;
bool g_editMode;                  /* the editor's panels, gizmo and keys are on */
std::vector<Record> g_rtRecs;     /* the runtime records (gObjects, gVehicles) as read at the freeze */

/* the level file's records: the editor's copy while it edits this level, else the file in RDRAM */
void readFileRecords() {
    const bool fromEditor = g_editMode && g_ed.isOpen() && g_ed.level() == g_fz.levelNo;
    std::vector<Record> recs;
    if (fromEditor) {
        const std::vector<uint8_t> &img = g_ed.image();
        recs = viewer::parseLevel(img.data(), img.size(), &g_notes, true);
    } else {
        const uint32_t lvl = levelData();
        const size_t span = levelSpan(lvl);
        if (span != 0) recs = viewer::parseLevel(reinterpret_cast<const uint8_t *>(uintptr_t(lvl)), span, &g_notes);
    }
    /* 2-D records on the ground */
    for (Record &r : recs)
        if (r.hasPos && !r.yKnown) r.y = groundAt(r.x, r.z, 0.0) + 2;
    g_recs = recs;
    g_recs.insert(g_recs.end(), g_rtRecs.begin(), g_rtRecs.end());
}

void readRecords() {
    g_recs.clear();
    g_rtRecs.clear();
    g_notes.clear();
    g_tris.clear();
    g_sel = -1;
    const uint32_t lvl = levelData();
    const size_t span = levelSpan(lvl);
    std::string why;
    if (span == 0 || !viewer::parseHeader(reinterpret_cast<const uint8_t *>(uintptr_t(lvl)), span, g_hdr, &why)) {
        g_notes.push_back("no level data: " + why);
        return;
    }
    /* the runtime records */
    const uint32_t oe = rd<uint32_t>(kGObjectsEnd);
    if (oe >= kGObjects && oe <= kGObjects + 0xFCu * 64)
        for (uint32_t o = kGObjects, i = 0; o + 0xFC <= oe; o += 0xFC, i++)
            g_rtRecs.push_back(viewer::gameObjectRecord(reinterpret_cast<const uint8_t *>(uintptr_t(o)), int(i), o));
    const uint32_t ve = rd<uint32_t>(kGVehiclesEnd);
    if (ve >= kGVehicles && ve <= kGVehicles + 0x74u * 32)
        for (uint32_t v = kGVehicles, i = 0; v + 0x74 <= ve; v += 0x74, i++)
            g_rtRecs.push_back(viewer::gameVehicleRecord(reinterpret_cast<const uint8_t *>(uintptr_t(v)), int(i), v));
    readFileRecords();
    /* collision: grids A and B (0x6C, 0x70: per cell a BE u32 end offset, then 0x16-byte triangles of nine BE
     * s16) and the buildings' triangles (gObjects [+4, +8): 0x60-byte records, s32 vertices / 8 at +0x28) */
    const uint8_t *L = reinterpret_cast<const uint8_t *>(uintptr_t(lvl));
    const unsigned cells = g_hdr.collCols * g_hdr.collRows;
    for (int g = 0; g < 2 && cells > 0 && cells < 0x10000; g++) {
        const uint32_t start = g_hdr.offAt(g == 0 ? 0x6C : 0x70);
        if (start >= span) continue;
        uint32_t p = start;
        for (unsigned c = 0; c < cells && p + 4 <= span; c++) {
            const uint32_t end = start + (uint32_t(L[p]) << 24 | uint32_t(L[p + 1]) << 16 | uint32_t(L[p + 2]) << 8 | L[p + 3]);
            if (end < p + 4 || end > span) break;
            for (uint32_t t = p + 4; t + 0x16 <= end; t += 0x16) {
                Tri tr;
                for (int k = 0; k < 9; k++) tr.v[k] = float(int16_t((L[t + 2 * k] << 8) | L[t + 2 * k + 1]));
                tr.src = uint8_t(g);
                g_tris.push_back(tr);
            }
            p = end;
        }
    }
    if (oe >= kGObjects && oe <= kGObjects + 0xFCu * 64)
        for (uint32_t o = kGObjects; o + 0xFC <= oe; o += 0xFC) {
            const uint32_t a0 = rd<uint32_t>(o + 4), a1 = rd<uint32_t>(o + 8);
            if (!inRam(a0) || a1 < a0 || !inRam(a1) || a1 - a0 > 0x60u * 4096) continue;
            for (uint32_t a = a0; a + 0x60 <= a1; a += 0x60) {
                Tri tr;
                const int32_t *v = reinterpret_cast<const int32_t *>(uintptr_t(a + 0x28));
                for (int k = 0; k < 9; k++) tr.v[k] = float(v[k] / 8.0);
                tr.src = 2;
                g_tris.push_back(tr);
            }
        }
    unsigned byKind[viewer::REC_KINDS] = {};
    for (const Record &r : g_recs) byKind[r.kind]++;
    char line[512];
    int n = 0;
    for (int k = 0; k < viewer::REC_KINDS && n < 480; k++)
        if (byKind[k]) n += snprintf(line + n, sizeof line - n, " %s %u,", viewer::recKindKey(k), byKind[k]);
    if (n > 0) line[n - 1] = 0;
    else line[0] = 0;
    host_log("live: viewer: level %d records:%s; %u collision triangles; %u notes\n", rd<int>(kLevelNo), line,
             unsigned(g_tris.size()), unsigned(g_notes.size()));
    for (const std::string &s : g_notes) host_log("live: viewer: note: %s\n", s.c_str());
}

int findRecord(int kind, int index) {
    for (size_t i = 0; i < g_recs.size(); i++)
        if (g_recs[i].kind == kind && g_recs[i].index == index) return int(i);
    return -1;
}

/* the runtime vehicle record a spawn became: the k-th spawn of its type is the k-th gVehicles record of that
 * type (type 1 on bonus levels: the player's pick) */
int vehicleOf(const Record &spawn) {
    int type = spawn.type, k = 0;
    const unsigned kind = rd<uint8_t>(kLevelInfo + uint32_t(rd<int>(kLevelNo)) * 0x44u);
    if (type == 1 && kind != 1 && kind != 0x80) type = rd<uint8_t>(kDigger);
    for (const Record &r : g_recs)
        if (r.kind == viewer::REC_VEHICLE && r.index < spawn.index && r.type == spawn.type) k++;
    for (const Record &r : g_recs)
        if (r.kind == viewer::REC_GVEHICLE && r.type == type && k-- == 0) return r.index;
    return -1;
}

/* the player's start: the driver's spawn (type 0) */
bool playerStart(Vec &p, double &heading) {
    for (const Record &r : g_recs)
        if (r.kind == viewer::REC_VEHICLE && r.type == 0) {
            p = {r.x, r.y, r.z};
            heading = r.heading;
            return true;
        }
    return false;
}

void jumpToStart() {
    Vec p;
    double h;
    if (!playerStart(p, h)) return;
    g_mv.on = false;
    g_cam.yaw = h;
    g_cam.pitch = -25;
    g_cam.lookAt(p, 160);
    host_log("live: viewer: camera at the player's start (%.0f %.0f %.0f)\n", p.x, p.y, p.z);
}

void cameraFromGame() {
    orbit::Vec3 eye, fwd;
    if (!orbit::viewEye(g_fz.view, eye, fwd)) return;
    g_cam.pos = {eye.x, eye.y, eye.z};
    double yaw, pitch;
    orbit::angles(fwd, yaw, pitch);
    g_cam.yaw = yaw, g_cam.pitch = pitch;
}

/* above the level, looking down at its middle */
void cameraTop() {
    g_mv.on = false;
    const double cx = (g_hdr.minX + g_hdr.maxX) / 2.0, cz = (g_hdr.minZ + g_hdr.maxZ) / 2.0;
    const double size = std::max(1000.0, double(std::max(g_hdr.maxX - g_hdr.minX, g_hdr.maxZ - g_hdr.minZ)));
    g_cam.yaw = 0;
    g_cam.pitch = -60;
    g_cam.lookAt({cx, groundAt(cx, cz, 0), cz}, size * 0.75);
}

/* ---- the capture's drawing list (PORT_DRAW_CELLS) -------------------------------------
 * Every cell, as long as the game's level display-list buffers hold the lists (lvl_alloc_dl_buffers' sizes;
 * an upper bound of what lvl_build_visible_dl copies for a set of cells: the fixed part, each listed
 * record's whole range, each group record with a listed cell): cells nearest the camera first. */
std::vector<short> g_cellList;
short *cellsHook(short *game) {
    return g_cellList.empty() ? game : g_cellList.data();
}

unsigned dlCapacity(int level, int profile, int list) {
    static const unsigned special[5][5] = {{16, 50, 50, 50, 200}, {29, 50, 50, 50, 200}, {17, 50, 50, 50, 50},
                                           {11, 3000, 50, 50, 200}, {10, 50, 50, 50, 2000}};
    for (const auto &s : special)
        if (int(s[0]) == level) return s[1 + list];
    static const unsigned p0[4] = {6000, 5000, 100, 2000}, p1[4] = {1000, 1000, 100, 2000};
    return profile == 0 ? p0[list] : profile == 1 ? p1[list] : 0;
}

void buildCellList() {
    g_cellList.clear();
    g_fz.cells = g_fz.cellsDrawn = 0;
    const uint32_t lvl = levelData();
    const size_t span = levelSpan(lvl);
    viewer::LevelHeader h;
    if (!g_opt.allCells || span == 0 ||
        !viewer::parseHeader(reinterpret_cast<const uint8_t *>(uintptr_t(lvl)), span, h))
        return;
    const unsigned n = h.cols * h.rows;
    const int level = rd<int>(kLevelNo);
    const uint8_t *L = reinterpret_cast<const uint8_t *>(uintptr_t(lvl));
    auto s32 = [&](uint32_t o) { return int32_t(uint32_t(L[o]) | uint32_t(L[o + 1]) << 8 | uint32_t(L[o + 2]) << 16 |
                                                uint32_t(L[o + 3]) << 24); };
    /* per list: the fixed part, then the cost of each cell's records and the groups (with their cells) */
    struct Group {
        unsigned cost;
        std::vector<int> cells;
    };
    unsigned used[4], cap[4], nrec[4] = {}, ngrp[4] = {};
    std::vector<std::vector<unsigned>> cellCost(4, std::vector<unsigned>(n, 0));
    std::vector<std::vector<Group>> groups(4);
    const unsigned fixed = (h.offAt(0x8C) >= h.offAt(0x88) ? (h.offAt(0x8C) - h.offAt(0x88)) / 8 : 0) + 1;
    for (int k = 0; k < 4; k++) {
        used[k] = fixed;
        cap[k] = dlCapacity(level, h.dlProfile, k);
        const uint32_t r0 = h.offAt(0xA0 + 4 * k), r1 = h.offAt(0xA4 + 4 * k);
        for (uint32_t r = r0; r + 0x14 <= r1 && r + 0x14 <= span; r += 0x14) {
            const int32_t a = s32(r), b = s32(r + 4), e = s32(r + 8), cell = s32(r + 0x10);
            const int32_t len = std::max(e - a, e - b);
            if (cell >= 0 && unsigned(cell) < n && len > 0) cellCost[k][cell] += unsigned(len / 8);
            nrec[k]++;
        }
        const uint32_t g0 = h.offAt(0xB0 + 4 * k), g1 = h.offAt(0xB4 + 4 * k);
        for (uint32_t g = g0; g + 0x14 <= g1 && g + 0x14 <= span;) {
            const int32_t cnt = s32(g + 0x10);
            if (cnt < 0 || cnt > 4096) break;
            Group gr;
            gr.cost = unsigned(std::max(0, s32(g + 8) - s32(g)) / 8);
            for (int i = 0; i < cnt && g + 0x14 + 4 * i + 4 <= span; i++) gr.cells.push_back(s32(g + 0x14 + 4 * i));
            groups[k].push_back(gr);
            ngrp[k]++;
            g += 0x14 + 4 * uint32_t(cnt);
        }
    }
    /* nearest first (from the camera, else the player) */
    Vec c = g_cam.pos;
    std::vector<int> order(n);
    for (unsigned i = 0; i < n; i++) order[i] = int(i);
    const double cw = h.cellW ? h.cellW : 1, cd = h.cellD ? h.cellD : 1;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        const double ax = (a % h.cols + 0.5) * cw - c.x, az = (a / h.cols + 0.5) * cd - c.z;
        const double bx = (b % h.cols + 0.5) * cw - c.x, bz = (b / h.cols + 0.5) * cd - c.z;
        return ax * ax + az * az < bx * bx + bz * bz;
    });
    std::vector<uint8_t> on(n, 0);
    std::vector<std::vector<uint8_t>> grpOn(4);
    for (int k = 0; k < 4; k++) grpOn[k].assign(groups[k].size(), 0);
    for (int cell : order) {
        unsigned add[4];
        bool fits = true;
        for (int k = 0; k < 4; k++) {
            add[k] = cellCost[k][cell];
            for (size_t g = 0; g < groups[k].size(); g++)
                if (!grpOn[k][g] && std::find(groups[k][g].cells.begin(), groups[k][g].cells.end(), cell) !=
                                        groups[k][g].cells.end())
                    add[k] += groups[k][g].cost;
            if (used[k] + add[k] + 8 > cap[k]) fits = false;
        }
        if (!fits) continue;
        on[cell] = 1;
        for (int k = 0; k < 4; k++) {
            used[k] += add[k];
            for (size_t g = 0; g < groups[k].size(); g++)
                if (!grpOn[k][g] && std::find(groups[k][g].cells.begin(), groups[k][g].cells.end(), cell) !=
                                        groups[k][g].cells.end())
                    grpOn[k][g] = 1;
        }
    }
    /* the game's own cells are always in (the game drew them with these buffers already) */
    const short *game = reinterpret_cast<const short *>(uintptr_t(0x803C30A8u));   /* gVisibleCells */
    for (unsigned i = 0; i < n + 1 && game[i] != -1; i++)
        if (unsigned(uint16_t(game[i])) < n) on[uint16_t(game[i])] = 1;
    for (unsigned i = 0; i < n; i++)
        if (on[i]) g_cellList.push_back(short(i));
    g_cellList.push_back(-1);
    g_fz.cells = n;
    g_fz.cellsDrawn = unsigned(g_cellList.size() - 1);
    host_log("live: viewer: capture draws %u of %u cells (level lists %u/%u %u/%u %u/%u %u/%u commands of the "
             "game's buffers; records %u %u %u %u, groups %u %u %u %u)\n",
             g_fz.cellsDrawn, n, used[0], cap[0], used[1], cap[1], used[2], cap[2], used[3], cap[3], nrec[0], nrec[1],
             nrec[2], nrec[3], ngrp[0], ngrp[1], ngrp[2], ngrp[3]);
}

/* ---- tags (PORT_ITAG, plat_core.c's viewer hook) ---------------------------------------- */
void itagHook(unsigned addr, int kind, int index) {
    if (!g_recordTags || kind >= 30) return;
    g_tagsBuilding[addr & 0xFFFFFF] = {uint32_t(kind), uint32_t(index)};
    g_tagCount++;
}

/* ---- the command hook (RT64 reading a replayed task) ----------------------------------- */
struct Owner {
    uint32_t kind = K_NONE, index = 0;
};
std::vector<Owner> g_owners;
Owner g_owner, g_pendingOwner;
bool g_hasPending;
int g_cimg;
bool g_inSky, g_skyDone;
bool g_mainTask;                  /* the frame's own task (the categories and the sky apply to it only: the
                                     offscreen ones, e.g. the vehicle shadows' textures, are drawn as they are) */
unsigned g_skipped, g_drawn;
bool g_measure;                   /* this replay measures the model view's model */

bool isTerrain(uint32_t phys) {
    if (!g_fz.level) return false;
    phys &= 0x7FFFFF;
    for (uint32_t t : g_fz.terrain)
        if (t != 0 && (t & 0x7FFFFF) == phys) return true;
    return g_fz.levelCalls != 0 && (g_fz.levelCalls & 0x7FFFFF) == phys;
}

bool ownerShown(const Owner &o) {
    if (!g_mv.on) return true;
    if (o.kind != g_mv.kind) return false;
    return g_mv.kind == K_OBJ ? (o.index & 0xFF) == g_mv.index : o.index == g_mv.index;
}

void measureVtx(RT64::State *state, uint32_t w0, uint32_t w1) {
    const unsigned n = ((w0 >> 20) & 15) + 1;
    const uint32_t a = state->rsp->fromSegmented(w1) & 0x7FFFFF;
    if (a + n * 16 > 0x800000) return;
    const RT64::RSP &rsp = *state->rsp;
    const hlslpp::float4x4 &m = rsp.modelMatrixStack[rsp.modelMatrixStackSize - 1];
    double M[4][4];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) M[r][c] = double(float(m[r][c]));
    for (unsigned i = 0; i < n; i++) {
        const int16_t *v = reinterpret_cast<const int16_t *>(state->RDRAM + a + i * 16);
        const double x = v[0], y = v[1], z = v[2];
        const double w[3] = {x * M[0][0] + y * M[1][0] + z * M[2][0] + M[3][0],
                             x * M[0][1] + y * M[1][1] + z * M[2][1] + M[3][1],
                             x * M[0][2] + y * M[1][2] + z * M[2][2] + M[3][2]};
        if (!std::isfinite(w[0]) || !std::isfinite(w[1]) || !std::isfinite(w[2])) continue;
        for (int k = 0; k < 3; k++) {
            if (g_mv.vtx == 0 || w[k] < g_mv.lo[k]) g_mv.lo[k] = w[k];
            if (g_mv.vtx == 0 || w[k] > g_mv.hi[k]) g_mv.hi[k] = w[k];
        }
        g_mv.vtx++;
    }
}

/* every command (G_MTX and G_POPMTX too: a tag may name one's address): the owner of what follows at its
 * display-list level (a called list inherits it; the level lists' calls make their contents terrain) */
void ownerHook(RT64::State *state, uint32_t addr) {
    const size_t level = state->returnAddressStack.size();
    while (g_owners.size() > level) {
        g_owner = g_owners.back();
        g_owners.pop_back();
    }
    while (g_owners.size() < level) {
        g_owners.push_back(g_owner);
        if (g_hasPending) g_owner = g_pendingOwner;
        g_hasPending = false;
    }
    g_hasPending = false;
    auto it = g_tags.find(addr);
    if (it != g_tags.end()) g_owner = {it->second.kind, it->second.index};
}

bool commandHook(RT64::State *state, RT64::DisplayList **dl, const GbiOriginal *orig) {
    const uint32_t w0 = (*dl)->w0, w1 = (*dl)->w1;
    const uint8_t op = uint8_t(w0 >> 24);
    if (!g_mainTask || !g_fz.level) return false;   /* (a front-end frame: drawn as the game drew it) */
    if (op == 0xFF) g_cimg++;
    const bool sky = g_cimg >= 2 && !g_skyDone;
    if (op == 0x06 && g_owner.kind == K_NONE) {   /* G_DL: the level's lists are terrain */
        const uint32_t target = state->rsp->fromSegmented(w1);
        if (isTerrain(target)) {
            if ((w0 >> 16) & 1) g_owner.kind = K_TERRAIN;   /* (a branch: the same level) */
            else g_pendingOwner = {K_TERRAIN, 0}, g_hasPending = true;
        }
    }
    const int cat = catOf(g_owner.kind, sky);
    /* the sky's ground fill: a plain background (full screen) in its place */
    if (sky && (g_opt.plain || g_mv.on)) {
        if (op == 0xF7) {   /* G_SETFILLCOLOR */
            RT64::DisplayList c = **dl;
            c.w1 = 0x294B294Bu;   /* RGBA5551 (40, 40, 40)-ish grey, twice */
            RT64::DisplayList *p = &c;
            orig[op](state, &p);
            return true;
        }
        if (op == 0xF6) {   /* G_FILLRECT: from the top */
            g_skyDone = true;
            RT64::DisplayList c = **dl;
            c.w1 = 0;
            RT64::DisplayList *p = &c;
            orig[op](state, &p);
            return true;
        }
    }
    if (sky && op == 0xF6) g_skyDone = true;
    const bool draw = op == 0xBF || op == 0xB5 || op == 0xB1 || op == 0xE4 || op == 0xE5 || op == 0xF6 || op == 0x09;
    if (op == 0x04 && g_measure && g_mv.on && ownerShown(g_owner) && cat != CAT_SKY && cat != CAT_HUD)
        measureVtx(state, w0, w1);
    if (!draw) return false;
    bool hide = !g_opt.show[cat];
    if (g_mv.on) hide = cat == CAT_SKY ? true : cat == CAT_HUD ? true : !ownerShown(g_owner);
    if (sky && (g_opt.plain || g_mv.on) && op != 0xF6) hide = true;
    if (cat == CAT_SKY && !g_opt.show[CAT_SKY] && op == 0xF6) hide = false;   /* (the ground fill clears the picture) */
    if (g_cimg < 2) hide = false;   /* (the depth clear) */
    if (!hide) {
        g_drawn++;
        return false;
    }
    g_skipped++;
    if ((op == 0xE4 || op == 0xE5) && uint8_t((*dl)[1].w0 >> 24) == 0xB4 && uint8_t((*dl)[2].w0 >> 24) == 0xB3)
        *dl += 2;   /* (a texture rectangle's two halves) */
    return true;
}

void resetReplay() {
    g_owners.clear();
    g_owner = Owner();
    g_hasPending = false;
    g_cimg = 0;
    g_inSky = g_skyDone = false;
}

/* the frozen frame's tasks to RT64 again */
void replay() {
    RT64::Application *app = g_env.app;
    publishView();
    for (const Task &t : g_frozenTasks) {
        app->interpreter->loadUCodeGBI(t.ucode & 0x3FFFFFF, t.data & 0x3FFFFFF, true);
        if (app->interpreter->hleGBI == nullptr) continue;
        resetReplay();
        g_mainTask = t.frameDl;
        interpTagsBeforeTask(t.dl, t.frameDl);
        wideBeforeTask();
        app->processDisplayLists(app->core.RDRAM, t.dl & 0x3FFFFFF, 0, true);
        interpTagsAfterTask(t.frameDl);
        wideAfterTask(t.frameDl);
    }
    /* a new picture: the VI as it was (RT64 presents when the VI differs: the line counter alternates) */
    *g_env.viOrigin = g_fz.origin;
    *g_env.viCurrentLine = (g_replays & 1) ? 2 : 0;
    app->updateScreen();
    g_replays++;
}

/* ---- the frozen frame ---------------------------------------------------------------- */
/* the capture's: the drawing paths' cells, the tags */
void captureHooksOn() {
    host_draw_cells_hook = cellsHook;
    host_viewer_itag_hook = itagHook;
    g_recordTags = true;
}

/* the replays': who draws what, the free camera's view */
void frozenHooksOn() {
    host_draw_cells_hook = nullptr;
    host_viewer_itag_hook = nullptr;
    g_recordTags = false;
    interpSetCommandHook(commandHook, nullptr);
    interpSetOwnerHook(ownerHook);
    RT64::ViewHook = viewHook;
}

void hooksOff() {
    {
        std::lock_guard<std::mutex> lock(g_vsMutex);
        g_vs.on = false;
    }
    g_cellList.clear();
    host_viewer_itag_hook = nullptr;
    g_recordTags = false;
    interpSetOwnerHook(nullptr);
    orbitApply();   /* the camera's hooks as the settings have them (the command hook, the view hook, the cells) */
}

/* the free camera's position for the cell list's order: the game's camera of the buffer being built */
void camAtGameEye() {
    const unsigned dyn = rd<uint8_t>(0x8035805Cu) & 1;   /* gBufferIndex */
    double v[4][4];
    mtxToDouble(kDyn + dyn * kDynSize + kDynView, v);
    orbit::Vec3 e, f;
    if (orbit::viewEye(v, e, f)) g_cam.pos = {e.x, e.y, e.z};
}

void beginCapture() {
    if (g_resetCam) camAtGameEye();
    g_state = S_CAPTURE;
    g_captureFrames = 0;
    g_cur.clear();
    g_tagsBuilding.clear();
    hooksOff();
    captureHooksOn();
    {
        std::lock_guard<std::mutex> lock(g_vsMutex);
        g_vs.on = false;   /* (the game's own view while it steps) */
    }
    buildCellList();
    host_log("live: viewer: capturing (frame %u, level %d, mode %08llX)\n", plat_frames(), rd<int>(kLevelNo),
             rd<unsigned long long>(kGameMode));
}

/* the frozen frame's DynamicBuf, view, terrain lists and picture */
void setupFrozen() {
    g_fz = Frozen();
    g_fz.mode = rd<unsigned long long>(kGameMode);
    g_fz.levelNo = rd<int>(kLevelNo);
    g_fz.lvl = levelData();
    const Task *frame = nullptr;
    for (const Task &t : g_frozenTasks)
        if (t.frameDl) frame = &t;
    uint32_t cimg = 0;
    if (frame != nullptr) {
        const uint32_t dl = frame->dl & 0x3FFFFFF;
        g_fz.dyn = (dl - (kDyn & 0x3FFFFFF)) / kDynSize;
        if (g_fz.dyn <= 1 && (g_fz.mode & (MODE_LEVEL | MODE_LEVEL_START | MODE_MAP_VIEW | 0x2)) != 0) {
            const uint32_t base = kDyn + g_fz.dyn * kDynSize;
            mtxToDouble(base + kDynView, g_fz.view);
            mtxToDouble(base + kDynProj, g_fz.proj);
            orbit::Vec3 e, f;
            g_fz.level = orbit::viewEye(g_fz.view, e, f);
            for (int i = 0; i < 8; i++) g_fz.terrain[i] = rd<uint32_t>(kLevelDl + 4u * uint32_t(i));
            for (int i = 0; i < 4; i++) g_fz.terrain[8 + i] = rd<uint32_t>(kLevelFileDl + 4u * uint32_t(i));
            g_fz.levelCalls = base + kDynLevelCalls;
        }
        /* the colour image: the frame's second G_SETCIMG (the first is the depth clear's) */
        const uint32_t *w = reinterpret_cast<const uint32_t *>(uintptr_t(0x80000000u | dl));
        int seen = 0;
        for (unsigned i = 0; i < 0xB5E; i++) {
            const uint32_t op = w[i * 2] >> 24;
            if (op == 0xB8) break;
            if (op == 0xFF && ++seen == 2) {
                cimg = w[i * 2 + 1] & 0x7FFFFF;
                break;
            }
        }
    }
    /* the VI origin of that picture: the frame buffer's start plus the VI's offset into its buffer */
    const uint32_t vi = *g_env.viOrigin & 0x7FFFFF, fb0 = 0x400, fbSize = 320 * 240 * 2;
    g_fz.origin = *g_env.viOrigin;
    if (cimg != 0 && vi >= fb0 && vi < fb0 + 2 * fbSize) g_fz.origin = cimg + (vi - fb0) % fbSize;
    {
        std::lock_guard<std::mutex> lock(g_vsMutex);
        toFloat(g_fz.view, g_vs.game);
        g_vs.on = g_fz.level;
        g_vs.haveProj = false;
    }
    host_log("live: viewer: frozen at frame %u (level %d, mode %08llX): %u tasks, %u tags, %s; picture 0x%06X (VI "
             "0x%06X)\n",
             plat_frames(), g_fz.levelNo, g_fz.mode, unsigned(g_frozenTasks.size()), unsigned(g_tags.size()),
             g_fz.level ? "the free camera works" : "not a level frame: the game's own camera only", g_fz.origin,
             vi);
}

/* ---- the panels --------------------------------------------------------------------- */
struct Marker {
    int rec;
    float x, y, r;
    double depth;
};
std::vector<Marker> g_markers;
std::string g_status;
bool g_cleanShot;                 /* this tick's picture is a clean screenshot: no panels, no overlays */
bool g_overlayShot;               /* ... with the overlays, no panels */

ImU32 kindColor(int k) {
    switch (k) {
    case viewer::REC_VEHICLE: return IM_COL32(80, 200, 255, 255);
    case viewer::REC_CARRIER: return IM_COL32(255, 80, 80, 255);
    case viewer::REC_OBJECT: return IM_COL32(255, 210, 80, 255);
    case viewer::REC_RDU: return IM_COL32(120, 255, 120, 255);
    case viewer::REC_TNT: return IM_COL32(255, 120, 40, 255);
    case viewer::REC_CRATE: return IM_COL32(200, 160, 255, 255);
    case viewer::REC_COMMPT: return IM_COL32(80, 255, 230, 255);
    case viewer::REC_BLOCK: case viewer::REC_BLOCKSLOT: return IM_COL32(230, 230, 230, 255);
    case viewer::REC_GOBJECT: return IM_COL32(255, 170, 120, 255);
    case viewer::REC_GVEHICLE: return IM_COL32(120, 160, 255, 255);
    default: return IM_COL32(200, 200, 200, 255);
    }
}

/* the picture rectangle and the projection the overlays use */
bool overlayProjection(double proj[4][4], double &rx, double &ry, double &rw, double &rh) {
    {
        std::lock_guard<std::mutex> lock(g_vsMutex);
        if (g_vs.haveProj) memcpy(proj, g_vs.proj, sizeof g_vs.proj);
        else memcpy(proj, g_fz.proj, sizeof g_fz.proj);
    }
    if (!(std::fabs(proj[0][0]) > 1e-9)) return false;
    const ImGuiIO &io = ImGui::GetIO();
    viewer::pictureRect(io.DisplaySize.x, io.DisplaySize.y, std::fabs(proj[1][1] / proj[0][0]), rx, ry, rw, rh);
    return true;
}

/* ---- the level editor: edits ----------------------------------------------------------
 * The editor works on a host-side copy of the level file (edit::Editor over port_mods.h's copy of the ROM's
 * file, plus the changes a loaded mod already makes to it); the markers and the inspector show that copy
 * while editing.  Apply gives the game the edited file (port_level_set_edit) and reloads the level through
 * the game's own set-up, so what is shown afterwards is the game running it. */
struct Drag {
    int mode = 0;                 /* 0 none, 1 on the ground plane, 2 along x, 3 along y, 4 along z, 5 heading */
    int kind = -1, index = -1;
    Vec start, cur;
    double startHeading = 0, heading = 0;
    double grabX = 0, grabZ = 0, t0 = 0, a0 = 0;
    bool moved = false;
} g_drag;
struct EditOpts {
    bool snapGround = true;       /* a record moved on the ground plane sits on the ground (its y = the floor's) */
    int grid = 0;                 /* x / z snapped to this step (map units; 0: none) */
} g_eopt;
std::string g_editMsg;            /* the last edit's result (the Changes panel) */
std::string g_savedSha;           /* the edited file last saved to a mod file */
std::string g_savedPath;
char g_modName[64] = "my_level_mod";
bool g_modEnable = true;
bool g_playAfterLoad;             /* "Play from here": leave the viewer into play once the level is up */
bool g_testRun;                   /* this run played an edited level: saving is off */
/* the overlays' projection of this tick (the gizmo's picking) */
struct OverlayView {
    bool ok = false;
    double view[4][4] = {}, proj[4][4] = {};
    double rx = 0, ry = 0, rw = 0, rh = 0;
} g_ov;
/* edits asked for by the panels: run after the UI frame is built (they replace the records the panels read) */
std::vector<std::function<void()>> g_after;

bool isEditable(int kind) {
    return kind >= viewer::REC_VEHICLE && kind <= viewer::REC_BLOCKSLOT;
}
bool kindHasY(int kind) {
    return kind != viewer::REC_CARRIER;
}
bool kindHasHeading(int kind) {
    return kind == viewer::REC_VEHICLE || kind == viewer::REC_CARRIER;
}
bool editing() {
    return g_editMode && g_ed.isOpen() && g_ed.level() == g_fz.levelNo && g_fz.level && !g_mv.on;
}

/* the editor on the frozen level (opened on the ROM's file and the loaded mods' changes) */
bool editorOpen() {
    if (!g_fz.level || g_fz.levelNo < 0 || g_fz.levelNo >= 60) {
        g_editMsg = "The editor works on a level frame.";
        return false;
    }
    if (g_ed.isOpen() && g_ed.level() == g_fz.levelNo) return true;
    /* another level's edits: kept for when it comes back (the game keeps its applied file too) */
    static std::vector<LmChange> stash[60];
    if (g_ed.isOpen()) stash[g_ed.level()] = g_ed.changes();
    const uint8_t *orig = nullptr;
    size_t n = 0;
    if (port_level_original(g_fz.levelNo, &orig, &n)) {
        g_editMsg = "The level file can't be read from the ROM.";
        return false;
    }
    std::vector<LmChange> base;
    const LmChange *ch = nullptr;
    int nch = 0;
    const char *modName = nullptr;
    std::string baseName;
    if (port_level_mod_changes(g_fz.levelNo, &ch, &nch, &modName)) {
        base.assign(ch, ch + nch);
        baseName = modName ? modName : "";
    }
    if (!g_ed.open(g_fz.levelNo, orig, n, base, baseName)) {
        g_editMsg = "The level can't be edited: " + g_ed.error();
        host_log("live: editor: level %d: %s\n", g_fz.levelNo, g_ed.error().c_str());
        return false;
    }
    g_savedSha = g_ed.imageSha1();
    g_savedPath.clear();
    for (const LmChange &c : stash[g_fz.levelNo]) g_ed.push(c);
    stash[g_fz.levelNo].clear();
    /* what the game runs now: this level's edited file, if it was given one */
    const uint8_t *cur = nullptr;
    size_t curSize = 0;
    if (port_level_edited(g_fz.levelNo, &cur, &curSize)) {
        char sha[41];
        lm_sha1_hex(cur, curSize, sha);
        if (sha == g_ed.imageSha1()) g_ed.markApplied();
    }
    host_log("live: editor: level %d opened (file 0x%X bytes, SHA-1 %s%s%s)\n", g_fz.levelNo, unsigned(n),
             g_ed.originalSha1().c_str(), base.empty() ? "" : "; the changes of mod ", baseName.c_str());
    return true;
}

void reselect(int kind, int index) {
    g_sel = findRecord(kind, index);
}

void setEditMode(bool on) {
    int kind = -1, index = -1;
    if (g_sel >= 0 && g_sel < int(g_recs.size())) kind = g_recs[g_sel].kind, index = g_recs[g_sel].index;
    if (on && !editorOpen()) return;
    g_editMode = on;
    g_drag = Drag();
    readFileRecords();
    if (kind >= 0) reselect(kind, index);
    host_log("live: editor: %s\n", on ? "on" : "off");
}

bool pushEdit(const LmChange &c, int selKind, int selIndex) {
    if (!g_ed.push(c)) {
        g_editMsg = "Not done: " + g_ed.error();
        host_log("live: editor: refused: %s: %s\n", edit::describe(c).c_str(), g_ed.error().c_str());
        return false;
    }
    g_editMsg = edit::describe(c);
    host_log("live: editor: %s (%u changes; file SHA-1 %s)\n", g_editMsg.c_str(), unsigned(g_ed.changes().size()),
             g_ed.imageSha1().c_str());
    readFileRecords();
    if (selKind >= 0) reselect(selKind, selIndex);
    else g_sel = -1;
    return true;
}

void editUndo() {
    if (!g_ed.canUndo()) return;
    const LmChange c = g_ed.changes().back();
    g_ed.undo();
    g_editMsg = "Undone: " + edit::describe(c);
    host_log("live: editor: undo %s\n", edit::describe(c).c_str());
    readFileRecords();
    reselect(c.kind, std::min(c.index, std::max(0, g_ed.count(c.kind) - 1)));
}

void editRedo() {
    if (!g_ed.canRedo()) return;
    if (!g_ed.redo()) {
        g_editMsg = "Not redone: " + g_ed.error();
        return;
    }
    const LmChange c = g_ed.changes().back();
    g_editMsg = "Redone: " + edit::describe(c);
    host_log("live: editor: redo %s\n", edit::describe(c).c_str());
    readFileRecords();
    reselect(c.kind, c.op == LM_OP_DELETE ? -1 : c.index);
}

/* the selected record, if the editor can change it */
bool selEditable(int &kind, int &index) {
    if (!editing() || g_sel < 0 || g_sel >= int(g_recs.size()) || !isEditable(g_recs[g_sel].kind)) return false;
    kind = g_recs[g_sel].kind, index = g_recs[g_sel].index;
    return index < g_ed.count(kind);
}

void duplicateSel() {
    int kind, index;
    if (!selEditable(kind, index)) return;
    /* the copy goes next to it, 48 map units along x (snapped to the grid) */
    const int fx = lm_field_index(kind, "x");
    const long x = g_ed.get(kind, index, "x");
    long nx = edit::snapTo(double(x + 48), g_eopt.grid);
    long lo, hi;
    lm_field_range(kind, fx, &lo, &hi);
    if (nx > hi) nx = edit::snapTo(double(x - 48), g_eopt.grid);
    pushEdit(edit::makeAdd(kind, index + 1, index, {{fx, nx}}), kind, index + 1);
}

void deleteSel() {
    int kind, index;
    if (!selEditable(kind, index)) return;
    pushEdit(edit::makeDelete(kind, index), -1, -1);
}

void rotateSel(double deg) {
    int kind, index;
    if (!selEditable(kind, index) || !kindHasHeading(kind)) return;
    const double d = edit::degreesFromHeading(g_ed.get(kind, index, "heading")) + deg;
    pushEdit(edit::makeSet(kind, index, {{lm_field_index(kind, "heading"), edit::headingFromDegrees(d)}}), kind,
             index);
}

void groundSel() {
    int kind, index;
    if (!selEditable(kind, index) || !kindHasY(kind)) return;
    const double x = double(g_ed.get(kind, index, "x")), z = double(g_ed.get(kind, index, "z"));
    const double y = groundAt(x, z, double(g_ed.get(kind, index, "y")));
    pushEdit(edit::makeSet(kind, index, {{lm_field_index(kind, "y"), std::lround(y)}}), kind, index);
}

/* a field's value clamped to its range */
long clampField(int kind, const char *name, double v) {
    long lo, hi;
    lm_field_range(kind, lm_field_index(kind, name), &lo, &hi);
    return std::max(lo, std::min(hi, long(std::lround(v))));
}

std::string modsDir() {
    std::string dir = g_env.cfg != nullptr ? absPath(*g_env.cfg, g_env.cfg->modsDir.empty() ? "mods" : g_env.cfg->modsDir)
                                           : "mods";
    if (!dir.empty() && dir.back() != '\\' && dir.back() != '/') dir += "\\";
    return dir;
}

/* the editor's file for the game's next load of the level (NULL: the ROM's, or the mod's as loaded) */
void giveEditsToGame() {
    const int level = g_ed.level();
    if (g_ed.changes().empty()) {
        /* nothing of the editor's: what the game read before (the loaded mod's file, or the ROM's) */
        const LmChange *ch;
        int n;
        const char *name;
        if (port_level_mod_changes(level, &ch, &n, &name))
            port_level_set_edit(level, g_ed.image().data(), g_ed.image().size(), (std::string("mod ") + name).c_str());
        else
            port_level_set_edit(level, nullptr, 0, nullptr);
    } else {
        port_level_set_edit(level, g_ed.image().data(), g_ed.image().size(), "the level editor");
    }
    g_ed.markApplied();
}

void requestLevel(int level);

void applyEdits() {
    if (!g_ed.isOpen()) return;
    giveEditsToGame();
    g_editMsg = "Applied: the level is loaded again with the edits.";
    host_log("live: editor: apply: level %d reloaded with the edited file (SHA-1 %s, %u changes)\n", g_ed.level(),
             g_ed.imageSha1().c_str(), unsigned(g_ed.changes().size()));
    requestLevel(g_ed.level());
}

void playEdits() {
    if (!g_ed.isOpen()) return;
    giveEditsToGame();
    g_testRun = true;
    host_saves_block("a test run of an edited level");
    overlayMarker("TEST RUN of an edited level - saving is off");
    g_playAfterLoad = true;
    host_log("live: editor: play from here: level %d, edited file SHA-1 %s; saving off for this run\n", g_ed.level(),
             g_ed.imageSha1().c_str());
    requestLevel(g_ed.level());
}

bool saveMod(const std::string &name, bool enable) {
    std::string clean;
    for (char c : name)
        if (isalnum((unsigned char) c) || c == '_' || c == '-' || c == '.' || c == ' ') clean += c;
    while (!clean.empty() && (clean.back() == ' ' || clean.back() == '.')) clean.pop_back();
    if (clean.empty()) {
        g_editMsg = "Give the mod a name (letters, digits, - and _).";
        return false;
    }
    const std::string dir = modsDir();
    CreateDirectoryW(wide(dir).c_str(), nullptr);
    const std::string path = dir + clean + ".bcmod";
    char desc[160];
    snprintf(desc, sizeof desc, "level %d, %u change%s, made with bc.exe's level editor", g_ed.level(),
             unsigned(g_ed.baseChanges().size() + g_ed.changes().size()),
             g_ed.baseChanges().size() + g_ed.changes().size() == 1 ? "" : "s");
    const std::string text = g_ed.modText(clean, desc);
    if (text.empty() || host_write_file(path.c_str(), text.data(), unsigned(text.size())) != 0) {
        g_editMsg = "Can't write " + path;
        host_log("live: editor: can't write %s\n", path.c_str());
        return false;
    }
    g_savedSha = g_ed.imageSha1();
    g_savedPath = path;
    g_editMsg = "Saved " + path;
    host_log("live: editor: saved %s (%u bytes; level %d, edited file SHA-1 %s)\n", path.c_str(),
             unsigned(text.size()), g_ed.level(), g_savedSha.c_str());
    if (enable && g_env.cfg != nullptr) {
        Config &c = *g_env.cfg;
        if (std::find(c.mods.begin(), c.mods.end(), clean) == c.mods.end()) c.mods.push_back(clean);
        if (configSave(c, {{"mods", "enabled"}}))
            g_editMsg += " (enabled in bc.ini: used from the next start)";
        else
            g_editMsg += " (not enabled: no bc.ini in this run)";
    }
    return true;
}

/* edits not saved in a mod file when the viewer is left: kept in mods\autosave_LNN.bcmod */
void autosave() {
    if (!g_ed.isOpen() || g_ed.changes().empty() || g_ed.imageSha1() == g_savedSha) return;
    char name[32];
    snprintf(name, sizeof name, "autosave_L%02d", g_ed.level());
    const std::string msg = g_editMsg;
    if (saveMod(name, false)) host_log("live: editor: unsaved edits kept in %s\n", g_savedPath.c_str());
    g_editMsg = msg;
}

/* ---- the gizmo -------------------------------------------------------------------------- */
struct Gizmo {
    bool ok = false;
    ImVec2 c;                     /* the centre on screen */
    ImVec2 tip[3];                /* the x, y, z arrows' tips */
    bool axis[3] = {};
    std::vector<ImVec2> ring;     /* the heading ring (empty: none) */
    double len = 0;               /* the arrows' world length */
    Vec p;
};

bool projectPoint(const Vec &p, ImVec2 &s, double *depth = nullptr) {
    double x, y;
    if (!g_ov.ok || !viewer::project(g_ov.view, g_ov.proj, p, g_ov.rx, g_ov.ry, g_ov.rw, g_ov.rh, x, y, depth))
        return false;
    s = ImVec2(float(x), float(y));
    return true;
}

/* the selected record's position (the drag's while dragging) */
bool selPosition(Vec &p, int &kind, int &index) {
    if (!selEditable(kind, index)) return false;
    const Record &r = g_recs[g_sel];
    if (!r.hasPos) return false;
    p = g_drag.mode != 0 && g_drag.kind == kind && g_drag.index == index ? g_drag.cur : Vec{r.x, r.y, r.z};
    return true;
}

Gizmo gizmo() {
    Gizmo g;
    int kind, index;
    if (!selPosition(g.p, kind, index)) return g;
    double depth;
    if (!projectPoint(g.p, g.c, &depth) || depth < 1.0) return g;
    g.len = 70.0 * depth / (std::max(1e-6, std::fabs(g_ov.proj[1][1])) * g_ov.rh * 0.5);
    const Vec dirs[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    for (int a = 0; a < 3; a++) {
        if (a == 1 && !kindHasY(kind)) continue;
        g.axis[a] = projectPoint({g.p.x + dirs[a].x * g.len, g.p.y + dirs[a].y * g.len, g.p.z + dirs[a].z * g.len},
                                 g.tip[a]);
    }
    if (kindHasHeading(kind))
        for (int i = 0; i <= 48; i++) {
            const double t = i * 2.0 * 3.14159265358979 / 48;
            ImVec2 s;
            if (!projectPoint({g.p.x + std::sin(t) * g.len * 1.35, g.p.y, g.p.z + std::cos(t) * g.len * 1.35}, s)) {
                g.ring.clear();
                break;
            }
            g.ring.push_back(s);
        }
    g.ok = true;
    return g;
}

float segDist(ImVec2 p, ImVec2 a, ImVec2 b) {
    const float dx = b.x - a.x, dy = b.y - a.y, l2 = dx * dx + dy * dy;
    float t = l2 > 0 ? ((p.x - a.x) * dx + (p.y - a.y) * dy) / l2 : 0;
    t = std::max(0.0f, std::min(1.0f, t));
    return std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
}

/* the gizmo part under a window point: 0 none, else Drag::mode */
int gizmoHit(float x, float y) {
    const Gizmo g = gizmo();
    if (!g.ok) return 0;
    const ImVec2 p(x, y);
    if (std::hypot(x - g.c.x, y - g.c.y) <= 11) return 1;
    int best = 0;
    float bd = 8;
    for (int a = 0; a < 3; a++)
        if (g.axis[a]) {
            const float d = segDist(p, g.c, g.tip[a]);
            if (d < bd) bd = d, best = 2 + a;
        }
    if (best) return best;
    for (size_t i = 1; i < g.ring.size(); i++)
        if (segDist(p, g.ring[i - 1], g.ring[i]) < 7) return 5;
    return 0;
}

bool screenRay(float sx, float sy, edit::V3 &o, edit::V3 &d) {
    return g_ov.ok && edit::pickRay(g_ov.view, g_ov.proj, g_ov.rx, g_ov.ry, g_ov.rw, g_ov.rh, sx, sy, o, d);
}

bool beginDrag(int mode, float sx, float sy) {
    int kind, index;
    Vec p;
    edit::V3 o, d;
    if (mode == 0 || !selPosition(p, kind, index) || !screenRay(sx, sy, o, d)) return false;
    g_drag = Drag();
    g_drag.mode = mode, g_drag.kind = kind, g_drag.index = index;
    g_drag.start = g_drag.cur = p;
    g_drag.startHeading = g_drag.heading =
        kindHasHeading(kind) ? edit::degreesFromHeading(g_ed.get(kind, index, "heading")) : 0;
    double gx, gz, t;
    if (mode == 1 || mode == 5) {
        if (!edit::rayPlaneY(o, d, p.y, gx, gz)) return g_drag = Drag(), false;
        g_drag.grabX = p.x - gx, g_drag.grabZ = p.z - gz;
        g_drag.a0 = std::atan2(gx - p.x, gz - p.z) * 180.0 / 3.14159265358979;
    } else {
        const edit::V3 a = {mode == 2 ? 1.0 : 0.0, mode == 3 ? 1.0 : 0.0, mode == 4 ? 1.0 : 0.0};
        if (!edit::rayAxis(o, d, {p.x, p.y, p.z}, a, t)) return g_drag = Drag(), false;
        g_drag.t0 = t;
    }
    return true;
}

void updateDrag(float sx, float sy) {
    edit::V3 o, d;
    if (g_drag.mode == 0 || !screenRay(sx, sy, o, d)) return;
    const int kind = g_drag.kind;
    Vec n = g_drag.start;
    double gx, gz, t;
    switch (g_drag.mode) {
    case 1:
        if (!edit::rayPlaneY(o, d, g_drag.start.y, gx, gz)) return;
        n.x = double(edit::snapTo(gx + g_drag.grabX, g_eopt.grid));
        n.z = double(edit::snapTo(gz + g_drag.grabZ, g_eopt.grid));
        if (kindHasY(kind) && g_eopt.snapGround) n.y = std::round(groundAt(n.x, n.z, g_drag.start.y));
        break;
    case 2: case 3: case 4: {
        const edit::V3 a = {g_drag.mode == 2 ? 1.0 : 0.0, g_drag.mode == 3 ? 1.0 : 0.0, g_drag.mode == 4 ? 1.0 : 0.0};
        if (!edit::rayAxis(o, d, {g_drag.start.x, g_drag.start.y, g_drag.start.z}, a, t)) return;
        const double dv = t - g_drag.t0;
        if (g_drag.mode == 2) n.x = double(edit::snapTo(g_drag.start.x + dv, g_eopt.grid));
        if (g_drag.mode == 3) n.y = std::round(g_drag.start.y + dv);
        if (g_drag.mode == 4) n.z = double(edit::snapTo(g_drag.start.z + dv, g_eopt.grid));
        break;
    }
    case 5: {
        if (!edit::rayPlaneY(o, d, g_drag.start.y, gx, gz)) return;
        const double a = std::atan2(gx - g_drag.start.x, gz - g_drag.start.z) * 180.0 / 3.14159265358979;
        double h = g_drag.startHeading + (a - g_drag.a0);
        if (g_eopt.grid > 0) h = std::round(h / 15.0) * 15.0;   /* (with a grid: 15-degree steps) */
        g_drag.heading = edit::degreesFromHeading(edit::headingFromDegrees(h));
        break;
    }
    default:
        return;
    }
    n.x = double(clampField(kind, "x", n.x)), n.z = double(clampField(kind, "z", n.z));
    if (kindHasY(kind)) n.y = double(clampField(kind, "y", n.y));
    g_drag.cur = n;
    g_drag.moved = true;
}

void endDrag() {
    if (g_drag.mode == 0) return;
    const Drag d = g_drag;
    g_drag = Drag();
    if (!d.moved) return;
    std::vector<edit::FieldValue> f;
    if (d.mode == 5) {
        const int h = edit::headingFromDegrees(d.heading);
        if (h != g_ed.get(d.kind, d.index, "heading")) f.push_back({lm_field_index(d.kind, "heading"), h});
    } else {
        const long x = std::lround(d.cur.x), y = std::lround(d.cur.y), z = std::lround(d.cur.z);
        if (x != g_ed.get(d.kind, d.index, "x")) f.push_back({lm_field_index(d.kind, "x"), x});
        if (kindHasY(d.kind) && y != g_ed.get(d.kind, d.index, "y")) f.push_back({lm_field_index(d.kind, "y"), y});
        if (z != g_ed.get(d.kind, d.index, "z")) f.push_back({lm_field_index(d.kind, "z"), z});
    }
    if (!f.empty()) pushEdit(edit::makeSet(d.kind, d.index, f), d.kind, d.index);
}

void drawGizmo(ImDrawList *dl) {
    const Gizmo g = gizmo();
    if (!g.ok) return;
    const ImU32 cols[3] = {IM_COL32(240, 70, 70, 255), IM_COL32(80, 230, 80, 255), IM_COL32(80, 140, 255, 255)};
    const int active = g_drag.mode;
    if (!g.ring.empty()) {
        const ImU32 rc = active == 5 ? IM_COL32(255, 255, 120, 255) : IM_COL32(255, 220, 80, 200);
        dl->AddPolyline(g.ring.data(), int(g.ring.size()), rc, 0, active == 5 ? 3.5f : 2.0f);
        /* the heading's mark on the ring */
        const double h = (active == 5 ? g_drag.heading
                                      : edit::degreesFromHeading(g_ed.get(g_recs[g_sel].kind, g_recs[g_sel].index,
                                                                          "heading"))) * 3.14159265358979 / 180.0;
        ImVec2 s;
        if (projectPoint({g.p.x + std::sin(h) * g.len * 1.35, g.p.y, g.p.z + std::cos(h) * g.len * 1.35}, s)) {
            dl->AddLine(g.c, s, rc, 2.0f);
            dl->AddCircleFilled(s, 6.0f, rc, 12);
        }
    }
    for (int a = 0; a < 3; a++) {
        if (!g.axis[a]) continue;
        const bool on = active == 2 + a;
        dl->AddLine(g.c, g.tip[a], cols[a], on ? 5.0f : 3.0f);
        const float dx = g.tip[a].x - g.c.x, dy = g.tip[a].y - g.c.y, l = std::max(1.0f, std::hypot(dx, dy));
        const ImVec2 u(dx / l, dy / l), nrm(-u.y, u.x);
        dl->AddTriangleFilled(ImVec2(g.tip[a].x + u.x * 12, g.tip[a].y + u.y * 12),
                              ImVec2(g.tip[a].x + nrm.x * 6, g.tip[a].y + nrm.y * 6),
                              ImVec2(g.tip[a].x - nrm.x * 6, g.tip[a].y - nrm.y * 6), cols[a]);
        static const char *const names[3] = {"x", "y", "z"};
        dl->AddText(ImVec2(g.tip[a].x + u.x * 16 - 3, g.tip[a].y + u.y * 16 - 7), cols[a], names[a]);
    }
    const ImU32 cc = active == 1 ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 200);
    dl->AddRectFilled(ImVec2(g.c.x - 6, g.c.y - 6), ImVec2(g.c.x + 6, g.c.y + 6), IM_COL32(0, 0, 0, 140));
    dl->AddRect(ImVec2(g.c.x - 7, g.c.y - 7), ImVec2(g.c.x + 7, g.c.y + 7), cc, 0, 0, 2.0f);
    if (active != 0) {
        char b[96];
        if (active == 5) snprintf(b, sizeof b, "heading %.1f", g_drag.heading);
        else snprintf(b, sizeof b, "%.0f, %.0f, %.0f", g_drag.cur.x, g_drag.cur.y, g_drag.cur.z);
        dl->AddText(ImVec2(g.c.x + 12, g.c.y + 10), IM_COL32(255, 255, 255, 255), b);
    }
}

void drawOverlays() {
    g_markers.clear();
    g_ov.ok = false;
    if (!g_fz.level) return;
    double view[4][4], proj[4][4], rx, ry, rw, rh;
    currentView(view);
    if (!overlayProjection(proj, rx, ry, rw, rh)) return;
    g_ov.ok = true;
    memcpy(g_ov.view, view, sizeof view);
    memcpy(g_ov.proj, proj, sizeof proj);
    g_ov.rx = rx, g_ov.ry = ry, g_ov.rw = rw, g_ov.rh = rh;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    const float font = uiFontPx();
    auto proj2 = [&](const Vec &p, float &sx, float &sy, double *depth) {
        double x, y;
        if (!viewer::project(view, proj, p, rx, ry, rw, rh, x, y, depth)) return false;
        sx = float(x), sy = float(y);
        return true;
    };
    const Vec eye = g_mv.on ? Vec() : g_cam.pos;
    /* collision outlines near the camera */
    if (g_opt.collision && !g_mv.on) {
        unsigned n = 0;
        for (const Tri &t : g_tris) {
            const double cx = (t.v[0] + t.v[3] + t.v[6]) / 3.0, cz = (t.v[2] + t.v[5] + t.v[8]) / 3.0;
            if ((cx - eye.x) * (cx - eye.x) + (cz - eye.z) * (cz - eye.z) > 900.0 * 900.0) continue;
            float sx[3], sy[3];
            bool ok = true;
            for (int k = 0; k < 3 && ok; k++) ok = proj2({t.v[3 * k], t.v[3 * k + 1], t.v[3 * k + 2]}, sx[k], sy[k], nullptr);
            if (!ok) continue;
            const ImU32 col = t.src == 0 ? IM_COL32(255, 90, 90, 150) : t.src == 1 ? IM_COL32(255, 160, 60, 150)
                                                                                   : IM_COL32(90, 200, 255, 150);
            dl->AddTriangle(ImVec2(sx[0], sy[0]), ImVec2(sx[1], sy[1]), ImVec2(sx[2], sy[2]), col, 1.0f);
            if (++n >= 6000) break;
        }
    }
    if (!g_opt.markers || g_mv.on) return;
    /* the carrier's route */
    for (const Record &r : g_recs)
        if (r.kind == viewer::REC_CARRIER && r.type != 0 && g_opt.kinds[viewer::REC_CARRIER]) {
            double dx, dz;
            viewer::headingDir(r.heading, dx, dz);
            const viewer::Field *len = nullptr;
            for (const viewer::Field &f : r.fields)
                if (f.name == "route length") len = &f;
            const double L = len ? atof(len->value.c_str()) : 0;
            float px = 0, py = 0;
            bool have = false;
            for (int i = 0; i <= 40; i++) {
                const double x = r.x + dx * L * i / 40.0, z = r.z + dz * L * i / 40.0;
                float sx, sy;
                const bool ok = proj2({x, groundAt(x, z, r.y) + 3, z}, sx, sy, nullptr);
                if (ok && have) dl->AddLine(ImVec2(px, py), ImVec2(sx, sy), IM_COL32(255, 60, 60, 220), 3.0f);
                have = ok, px = sx, py = sy;
            }
        }
    /* zones */
    if (g_opt.zones)
        for (const Record &r : g_recs) {
            if (!r.hasBox) continue;
            const Vec c[4] = {{r.x0, r.y, r.z0}, {r.x1, r.y, r.z0}, {r.x1, r.y, r.z1}, {r.x0, r.y, r.z1}};
            float sx[4], sy[4];
            bool ok = true;
            for (int k = 0; k < 4 && ok; k++) ok = proj2(c[k], sx[k], sy[k], nullptr);
            if (!ok) continue;
            for (int k = 0; k < 4; k++)
                dl->AddLine(ImVec2(sx[k], sy[k]), ImVec2(sx[(k + 1) & 3], sy[(k + 1) & 3]),
                            r.kind == viewer::REC_CAMZONE ? IM_COL32(255, 255, 0, 160) : IM_COL32(0, 255, 255, 160), 1.5f);
        }
    const bool ed = editing();
    for (size_t i = 0; i < g_recs.size(); i++) {
        const Record &r = g_recs[i];
        if (!r.hasPos || r.hasBox || !g_opt.kinds[r.kind]) continue;
        /* the editor: a record being dragged is drawn where it goes; edited records stand out */
        const bool dragged = ed && g_drag.mode != 0 && r.kind == g_drag.kind && r.index == g_drag.index;
        const Vec at = dragged ? g_drag.cur : Vec{r.x, r.y, r.z};
        const double heading = dragged && g_drag.mode == 5 ? g_drag.heading : r.heading;
        const int mark = ed && isEditable(r.kind) ? g_ed.edited(r.kind, r.index) : 0;
        float sx, sy;
        double depth;
        if (!proj2(at, sx, sy, &depth) || depth < 1.0) continue;
        if (sx < -50 || sy < -50 || sx > ImGui::GetIO().DisplaySize.x + 50 || sy > ImGui::GetIO().DisplaySize.y + 50)
            continue;
        const ImU32 col = mark == 1 ? IM_COL32(255, 70, 255, 255) : mark == 2 ? IM_COL32(255, 150, 40, 255)
                                                                             : kindColor(r.kind);
        const float rad = std::max(4.0f, std::min(30.0f, float(r.radius * proj[1][1] / depth * rh * 0.5)));
        const bool sel = int(i) == g_sel;
        dl->AddCircle(ImVec2(sx, sy), rad, col, 16, sel ? 3.0f : mark ? 2.5f : 1.5f);
        if (mark) dl->AddCircle(ImVec2(sx, sy), rad + 4, col, 16, 1.0f);
        if (r.hasHeading) {
            double dx, dz;
            viewer::headingDir(heading, dx, dz);
            float hx, hy;
            if (proj2({at.x + dx * r.radius * 1.5, at.y, at.z + dz * r.radius * 1.5}, hx, hy, nullptr))
                dl->AddLine(ImVec2(sx, sy), ImVec2(hx, hy), col, 2.0f);
        }
        /* (labels near the camera only: the RDUs, lined up by the dozen, nearer still) */
        const double labelDist = r.kind == viewer::REC_RDU ? g_opt.labelDist * 0.4 : g_opt.labelDist;
        if (g_opt.labels && (sel || mark || depth < labelDist)) {
            const std::string text = mark ? r.label + (mark == 1 ? " *" : " (mod)") : r.label;
            const ImVec2 tp(sx + rad + 3, sy - font * 0.4f);
            dl->AddText(ImVec2(tp.x + 1, tp.y + 1), IM_COL32(0, 0, 0, 200), text.c_str());
            dl->AddText(tp, col, text.c_str());
        }
        g_markers.push_back({int(i), sx, sy, rad, depth});
    }
    if (ed) drawGizmo(dl);
}

void selectAt(float x, float y) {
    /* the marker under the point (the nearest to the camera of those within a few pixels of it) */
    int best = -1;
    double bestScore = 1e30;
    for (const Marker &m : g_markers) {
        const double d = std::hypot(m.x - x, m.y - y);
        if (d > m.r + 10) continue;
        const double score = d + m.depth * 1e-4;
        if (score < bestScore) {
            bestScore = score;
            best = m.rec;
        }
    }
    g_sel = best;
    if (best >= 0) host_log("live: viewer: selected %s\n", g_recs[best].title.c_str());
}

void openModel(uint32_t kind, uint32_t index, int rec, const std::string &title) {
    g_mv = ModelView();
    g_mv.on = true;
    g_mv.kind = kind;
    g_mv.index = index;
    g_mv.rec = rec;
    g_mv.title = title;
    /* a first centre from the record; the replay measures the model's drawing */
    if (rec >= 0 && g_recs[rec].hasPos) {
        g_mv.centre = {g_recs[rec].x, g_recs[rec].y, g_recs[rec].z};
        g_mv.radius = std::max(20.0, g_recs[rec].radius);
    } else {
        g_mv.centre = g_cam.pos;
        g_mv.radius = 50;
    }
    g_mv.dist = g_mv.radius * 3.0;
    g_measure = true;
    host_log("live: viewer: model view of %s (tag kind %u, index %u)\n", title.c_str(), kind, index);
}

/* the model a record stands for */
bool modelFor(int rec) {
    if (rec < 0) return false;
    const Record &r = g_recs[rec];
    if (r.kind == viewer::REC_GVEHICLE) {
        openModel(K_OBJ, uint32_t(r.index), rec, r.title);
        return true;
    }
    if (r.kind == viewer::REC_GOBJECT) {
        openModel(K_BUILDING, uint32_t(r.index), rec, r.title);
        return true;
    }
    if (r.kind == viewer::REC_VEHICLE || r.kind == viewer::REC_CARRIER) {
        int v = -1;
        if (r.kind == viewer::REC_CARRIER) {
            for (const Record &x : g_recs)
                if (x.kind == viewer::REC_GVEHICLE && x.type == 0xFF) v = x.index;
        } else {
            v = vehicleOf(r);
        }
        if (v < 0) return false;
        openModel(K_OBJ, uint32_t(v), rec,
                  r.title + " (" + viewer::vehicleModelFile(r.kind == viewer::REC_CARRIER ? 0xFF : r.type) + ")");
        return true;
    }
    if (r.kind == viewer::REC_OBJECT) {
        const int g = findRecord(viewer::REC_GOBJECT, r.index);
        if (g < 0) return false;
        openModel(K_BUILDING, uint32_t(r.index), rec, r.title);
        return true;
    }
    return false;
}

std::string shotPath(const std::string &name) {
    std::string dir = g_env.shotDir;
    if (dir.empty()) {
        dir = g_env.cfg != nullptr ? g_env.cfg->dir + "screenshots" : "screenshots";
        CreateDirectoryW(wide(dir).c_str(), nullptr);
    }
    return dir + "\\" + name + ".png";
}

std::string autoShotName(bool overlays) {
    char b[96];
    SYSTEMTIME t;
    GetLocalTime(&t);
    snprintf(b, sizeof b, "viewer_L%02d_%04u%02u%02u_%02u%02u%02u%s", g_fz.levelNo, t.wYear, t.wMonth, t.wDay, t.wHour,
             t.wMinute, t.wSecond, overlays ? "_overlays" : "");
    return b;
}

void requestLevel(int level);
void leave();

constexpr float kPanelScale = 0.8f;   /* the viewer's panels: smaller than the settings menu's text */

void inspector() {
    const ImGuiIO &io = ImGui::GetIO();
    const float f = uiFontPx() * kPanelScale;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - f * 0.5f, f * 0.5f), ImGuiCond_FirstUseEver, ImVec2(1, 0));
    /* (above the editor's Changes panel, bottom right) */
    ImGui::SetNextWindowSize(ImVec2(f * 21, std::min(io.DisplaySize.y * 0.54f, f * 34)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.85f);
    if (!ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::End();
        return;
    }
    ImGui::SetWindowFontScale(kPanelScale);
    if (g_sel < 0 || g_sel >= int(g_recs.size())) {
        ImGui::TextWrapped("Click a marker (or pick a record in the Records list) to see it here.");
        ImGui::End();
        return;
    }
    const Record &r = g_recs[g_sel];
    ImGui::TextWrapped("%s", r.title.c_str());
    ImGui::TextDisabled("%s, at 0x%06X%s", viewer::recKindName(r.kind), r.offset,
                        r.kind >= viewer::REC_GOBJECT ? " (RDRAM)" : " in the level file");
    if (!r.model.empty()) ImGui::Text("Model: %s", r.model.c_str());
    if (r.hasPos) ImGui::Text("Position: %.0f, %.0f, %.0f", r.x, r.y, r.z);
    if (r.hasHeading) ImGui::Text("Heading: %.1f degrees", r.heading);
    if (ImGui::Button("Look at it") && r.hasPos) {
        g_mv.on = false;
        g_cam.lookAt({r.x, r.y, r.z}, std::max(120.0, r.radius * 5));
    }
    ImGui::SameLine();
    if (ImGui::Button("Model view")) modelFor(g_sel);
    int ek, ei;
    if (selEditable(ek, ei)) {
        /* the editor: the record's fields as numbers (Enter or leaving the field makes the change) */
        const int mark = g_ed.edited(ek, ei);
        ImGui::Separator();
        ImGui::TextColored(mark == 1 ? ImVec4(1, 0.3f, 1, 1) : mark == 2 ? ImVec4(1, 0.6f, 0.15f, 1) : ImVec4(0.6f, 1, 0.6f, 1),
                           "%s", mark == 1 ? "Edited (not in the ROM's file)" : mark == 2 ? "Changed by the loaded mod"
                                                                                       : "Editable: as the ROM has it");
        ImGui::PushItemWidth(f * 7);
        for (int fi = 0; fi < lm_field_count(ek); fi++) {
            long v = 0, lo, hi;
            g_ed.get(ek, ei, fi, v);
            lm_field_range(ek, fi, &lo, &hi);
            int iv = int(v);
            ImGui::PushID(fi);
            ImGui::InputInt(lm_field_name(ek, fi), &iv, 1, 16);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                iv = int(std::max(lo, std::min(hi, long(iv))));
                if (iv != v) g_after.push_back([=] { pushEdit(edit::makeSet(ek, ei, {{fi, iv}}), ek, ei); });
            }
            if (!strcmp(lm_field_name(ek, fi), "heading")) {
                ImGui::SameLine();
                ImGui::TextDisabled("%.1f deg", edit::degreesFromHeading(v));
            }
            ImGui::PopID();
        }
        ImGui::PopItemWidth();
        if (ImGui::Button("Duplicate (Ctrl+D)")) g_after.push_back(duplicateSel);
        ImGui::SameLine();
        ImGui::BeginDisabled(ek == viewer::REC_CARRIER);
        if (ImGui::Button("Delete (Del)")) g_after.push_back(deleteSel);
        ImGui::EndDisabled();
        if (kindHasY(ek)) {
            ImGui::SameLine();
            if (ImGui::Button("On the ground (G)")) g_after.push_back(groundSel);
        }
        if (kindHasHeading(ek)) {
            if (ImGui::Button("-15 deg")) g_after.push_back([] { rotateSel(-15); });
            ImGui::SameLine();
            if (ImGui::Button("+15 deg (R)")) g_after.push_back([] { rotateSel(15); });
        }
        ImGui::TextDisabled("%d of at most %d %s records", g_ed.count(ek), lm_kind_max(ek), lm_kind_key(ek));
        ImGui::Separator();
        /* (the rest below is the record as it is now, from the editor's copy) */
        if (!ImGui::CollapsingHeader("As the viewer reads it")) {
            ImGui::End();
            return;
        }
    }
    if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
        for (const viewer::Field &fl : r.fields) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(fl.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", fl.value.c_str());
        }
        ImGui::EndTable();
    }
    /* the runtime record the placement or spawn became */
    int other = -1;
    if (r.kind == viewer::REC_OBJECT) other = findRecord(viewer::REC_GOBJECT, r.index);
    if (r.kind == viewer::REC_VEHICLE) {
        const int v = vehicleOf(r);
        if (v >= 0) other = findRecord(viewer::REC_GVEHICLE, v);
    }
    if (other >= 0 && ImGui::CollapsingHeader("Its runtime record")) {
        const Record &o = g_recs[other];
        ImGui::TextWrapped("%s", o.title.c_str());
        for (const viewer::Field &fl : o.fields) ImGui::TextWrapped("%s: %s", fl.name.c_str(), fl.value.c_str());
    }
    if (ImGui::CollapsingHeader("Raw bytes", ImGuiTreeNodeFlags_DefaultOpen))
        ImGui::TextUnformatted(viewer::hexDump(r.raw).c_str());
    ImGui::End();
}

void mainPanel() {
    const float f = uiFontPx() * kPanelScale;
    ImGui::SetNextWindowPos(ImVec2(f * 0.5f, f * 0.5f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(f * 21, std::min(ImGui::GetIO().DisplaySize.y * 0.9f, f * 40)),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.85f);
    if (!ImGui::Begin("Level viewer", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::End();
        return;
    }
    ImGui::SetWindowFontScale(kPanelScale);
    ImGui::TextWrapped("%s", g_status.c_str());
    if (ImGui::Button(g_loadedLevel ? "Leave (restarts the game)" : "Leave the viewer")) g_leave = true;
    ImGui::SameLine();
    if (ImGui::Button("Player start")) jumpToStart();
    ImGui::SameLine();
    if (ImGui::Button("Overview")) cameraTop();
    if (ImGui::Button("Screenshot (F12)")) g_cleanShot = true;
    ImGui::SameLine();
    if (ImGui::Button("With overlays (Shift+F12)")) g_overlayShot = true;
    if (ImGui::CollapsingHeader("Level", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int pick = -1;
        if (pick < 0) pick = std::max(0, g_fz.levelNo);
        const std::string cur = levelLabel(pick);
        if (ImGui::BeginCombo("##level", cur.c_str())) {
            for (int i = 0; i < 60; i++)
                if (ImGui::Selectable(levelLabel(i).c_str(), i == pick)) pick = i;
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        const bool fe = rd<uint8_t>(kFrontEnd) != 0;
        ImGui::BeginDisabled(fe);
        if (ImGui::Button("Load")) requestLevel(pick);
        ImGui::EndDisabled();
        ImGui::TextDisabled(fe ? "Levels load from a level or the attract demo (not the menus)."
                               : "Loaded by the game's own level set-up.");
        if (!g_fz.level)
            ImGui::TextWrapped("This picture isn't a level's (the menus, the globe): it is shown as the game drew it. "
                               "The free camera, the overlays and the model view work in levels.");
    }
    if (!g_fz.level) {
        ImGui::End();
        return;
    }
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        float sp = float(g_opt.speed);
        if (ImGui::SliderFloat("Speed", &sp, 50.0f, 8000.0f, "%.0f units/s", ImGuiSliderFlags_Logarithmic))
            g_opt.speed = sp;
        bool farOut = g_opt.farOut;
        if (ImGui::Checkbox("Far draw distance", &farOut)) g_opt.farOut = farOut;
        ImGui::SameLine();
        ImGui::TextDisabled("(the game's: 10000)");
        ImGui::TextDisabled("WASD move, Q/E down/up, Shift fast, Ctrl slow, right mouse button: look, wheel: speed");
    }
    if (ImGui::CollapsingHeader("Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool on = g_editMode;
        if (ImGui::Checkbox("Edit the level's records (Ctrl+E)", &on)) g_after.push_back([on] { setEditMode(on); });
        if (g_editMode) {
            ImGui::Checkbox("Moved records sit on the ground", &g_eopt.snapGround);
            static const int grids[] = {0, 8, 16, 32, 64, 128};
            int gi = 0;
            for (int i = 0; i < 6; i++)
                if (grids[i] == g_eopt.grid) gi = i;
            ImGui::SetNextItemWidth(f * 6);
            if (ImGui::Combo("Grid (x, z)", &gi, "none\0 8\0 16\0 32\0 64\0 128\0")) g_eopt.grid = grids[gi];
            ImGui::TextDisabled("Click a marker, drag its square (on the ground), an arrow (along x, y or z) or the "
                                "ring (heading). Ctrl+Z undo, Ctrl+Y redo, Ctrl+D duplicate, Del delete, R turn.");
        } else {
            ImGui::TextDisabled("Move, turn, add and delete vehicles, buildings, RDUs, TNT, crates, communication "
                                "points, push blocks and the carrier's start; then play the level with them.");
        }
        if (!g_editMsg.empty()) ImGui::TextWrapped("%s", g_editMsg.c_str());
    }
    if (ImGui::CollapsingHeader("Show", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int c = 0; c < CAT_COUNT; c++) {
            bool b = g_opt.show[c];
            if (ImGui::Checkbox(kCatLabels[c], &b)) g_opt.show[c] = b;
        }
        bool plain = g_opt.plain;
        if (ImGui::Checkbox("Plain background", &plain)) g_opt.plain = plain;
        bool all = g_opt.allCells;
        if (ImGui::Checkbox("Draw every cell (else what the game's camera saw)", &all)) {
            g_opt.allCells = all;
        }
        if (ImGui::Button("Recapture")) g_recapture = true;
        ImGui::SameLine();
        ImGui::TextDisabled("(the game steps 2 frames)");
        if (g_fz.cells) ImGui::TextDisabled("%u of %u cells drawn", g_fz.cellsDrawn, g_fz.cells);
    }
    if (ImGui::CollapsingHeader("Overlays")) {
        bool b = g_opt.markers;
        if (ImGui::Checkbox("Markers", &b)) g_opt.markers = b;
        ImGui::SameLine();
        b = g_opt.labels;
        if (ImGui::Checkbox("Labels", &b)) g_opt.labels = b;
        float ld = float(g_opt.labelDist);
        if (ImGui::SliderFloat("Label distance", &ld, 100.0f, 30000.0f, "%.0f", ImGuiSliderFlags_Logarithmic))
            g_opt.labelDist = ld;
        b = g_opt.collision;
        if (ImGui::Checkbox("Collision outlines (near the camera)", &b)) g_opt.collision = b;
        b = g_opt.zones;
        if (ImGui::Checkbox("Height zones", &b)) g_opt.zones = b;
        for (int k = 0; k < viewer::REC_KINDS; k++) {
            if (k == viewer::REC_PANEL || k == viewer::REC_CAMZONE || k == viewer::REC_CHOPPERZONE) continue;
            b = g_opt.kinds[k];
            ImGui::PushStyleColor(ImGuiCol_Text, kindColor(k));
            if (ImGui::Checkbox(viewer::recKindName(k), &b)) g_opt.kinds[k] = b;
            ImGui::PopStyleColor();
        }
    }
    if (ImGui::CollapsingHeader("Records")) {
        ImGui::BeginChild("recs", ImVec2(0, f * 12), true);
        for (size_t i = 0; i < g_recs.size(); i++) {
            ImGui::PushID(int(i));
            if (ImGui::Selectable(g_recs[i].title.c_str(), int(i) == g_sel)) g_sel = int(i);
            ImGui::PopID();
        }
        ImGui::EndChild();
        for (const std::string &n : g_notes) ImGui::TextDisabled("note: %s", n.c_str());
    }
    if (ImGui::CollapsingHeader("Models")) {
        if (g_mv.on) {
            ImGui::Text("Showing %s", g_mv.title.c_str());
            if (ImGui::Button("Back to the level")) g_mv.on = false;
        }
        ImGui::BeginChild("models", ImVec2(0, f * 10), true);
        for (size_t i = 0; i < g_recs.size(); i++) {
            const Record &r = g_recs[i];
            if (r.kind != viewer::REC_GVEHICLE && r.kind != viewer::REC_GOBJECT) continue;
            ImGui::PushID(int(i));
            const std::string t = r.kind == viewer::REC_GVEHICLE ? r.title + " - " + r.model : r.title;
            if (ImGui::Selectable(t.c_str(), g_mv.on && g_mv.rec == int(i))) modelFor(int(i));
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::TextDisabled("Vehicles and objects of this level, drawn by the game's own display lists.");
    }
    ImGui::End();
}

/* the editor's change list, and what to do with it */
void changesPanel() {
    const ImGuiIO &io = ImGui::GetIO();
    const float f = uiFontPx() * kPanelScale;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - f * 0.5f, io.DisplaySize.y - f * 2.5f), ImGuiCond_FirstUseEver,
                            ImVec2(1, 1));
    ImGui::SetNextWindowSize(ImVec2(f * 24, std::min(f * 21, io.DisplaySize.y * 0.4f)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.85f);
    if (!ImGui::Begin("Changes", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::End();
        return;
    }
    ImGui::SetWindowFontScale(kPanelScale);
    const auto &base = g_ed.baseChanges();
    const auto &ch = g_ed.changes();
    ImGui::Text("Level %d: %u change%s%s", g_ed.level(), unsigned(ch.size()), ch.size() == 1 ? "" : "s",
                g_ed.dirty() ? "  (not in the game yet: Apply)" : "  (the game runs them)");
    ImGui::BeginChild("changelist", ImVec2(0, f * 8), true);
    for (size_t i = 0; i < base.size(); i++)
        ImGui::TextDisabled("mod %s: %s", g_ed.baseName().c_str(), edit::describe(base[i]).c_str());
    for (size_t i = 0; i < ch.size(); i++) {
        ImGui::PushID(int(i));
        const std::string t = std::to_string(i + 1) + ". " + edit::describe(ch[i]);
        if (ImGui::Selectable(t.c_str(), false) && ch[i].op != LM_OP_DELETE) {
            const int k = ch[i].kind, idx = ch[i].index;
            g_after.push_back([k, idx] { reselect(k, idx); });
        }
        ImGui::PopID();
    }
    if (base.empty() && ch.empty()) ImGui::TextDisabled("No changes yet.");
    ImGui::EndChild();
    ImGui::BeginDisabled(!g_ed.canUndo());
    if (ImGui::Button("Undo (Ctrl+Z)")) g_after.push_back(editUndo);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!g_ed.canRedo());
    if (ImGui::Button("Redo (Ctrl+Y)")) g_after.push_back(editRedo);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(ch.empty());
    if (ImGui::Button("Undo all")) g_after.push_back([] {
        g_ed.revertAll();
        g_editMsg = "Every change undone (Redo brings them back).";
        readFileRecords();
        g_sel = -1;
    });
    ImGui::EndDisabled();
    ImGui::Separator();
    if (ImGui::Button("Apply (reload the level)")) g_after.push_back(applyEdits);
    ImGui::SameLine();
    if (ImGui::Button("Play from here")) g_after.push_back(playEdits);
    ImGui::TextDisabled("Apply: the game sets the level up again from the edited file. Play: leaves the viewer into "
                        "the edited level (a test run: saving is off until bc.exe is closed).");
    ImGui::Separator();
    ImGui::SetNextItemWidth(f * 12);
    ImGui::InputText("##modname", g_modName, sizeof g_modName);
    ImGui::SameLine();
    if (ImGui::Button("Save mod")) {
        const std::string n = g_modName;
        const bool en = g_modEnable;
        g_after.push_back([n, en] { saveMod(n, en); });
    }
    ImGui::Checkbox("Use it from the next start (bc.ini [mods])", &g_modEnable);
    ImGui::TextDisabled("%s<name>.bcmod: a small file with the changes only (no game data): share it.",
                        modsDir().c_str());
    if (!g_savedPath.empty())
        ImGui::TextDisabled("%s %s", g_ed.imageSha1() == g_savedSha ? "Saved:" : "Changed since", g_savedPath.c_str());
    /* the level's heap: an edited level that holds more uses more of it */
    const uint32_t heap = rd<uint32_t>(kHeapPtr);
    if (heap > 0x80000000u && heap < 0x8021ED00u)
        ImGui::TextDisabled("Level heap ends at 0x%08X: %u KB below the load buffer (0x8021ED00)", heap,
                            unsigned((0x8021ED00u - heap) / 1024));
    ImGui::End();
}

void buildUi() {
    if (g_cleanShot) return;   /* (nothing over the picture) */
    drawOverlays();
    if (g_overlayShot || !g_opt.ui) return;
    mainPanel();
    inspector();
    if (editing()) changesPanel();
    /* the status line */
    const ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y - 4), ImGuiCond_Always, ImVec2(0.5f, 1));
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("##viewerstatus", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::SetWindowFontScale(kPanelScale);
        if (g_mv.on)
            ImGui::Text("Model view: %s   (drag: turn, wheel: zoom, Esc / B: back)", g_mv.title.c_str());
        else
            ImGui::Text("x %.0f  y %.0f  z %.0f   yaw %.0f  pitch %.0f   speed %.0f   (Tab: panels, F12: screenshot, "
                        "Esc: leave)",
                        g_cam.pos.x, g_cam.pos.y, g_cam.pos.z, g_cam.yaw, g_cam.pitch, g_opt.speed);
    }
    ImGui::End();
}

/* ---- input ------------------------------------------------------------------------- */
bool g_looking;                   /* right mouse button held: mouse look */
double g_mdx, g_mdy, g_wheel;
unsigned short g_padPrev;

/* window points (SDL's, in desktop points) -> the picture's pixels (ImGui's) */
void windowToPixels(int wx, int wy, float &x, float &y) {
    int ww, wh, pw, ph;
    SDL_GetWindowSize(g_env.window, &ww, &wh);
    SDL_GetWindowSizeInPixels(g_env.window, &pw, &ph);
    x = float(wx) * pw / std::max(1, ww);
    y = float(wy) * ph / std::max(1, wh);
}

void viewerEvent(SDL_Event &e) {
    if (e.type == SDL_QUIT) g_env.quit(0);
    const bool uiMouse = uiWantsMouse() && !g_looking;
    if (g_opt.ui) uiEvent(e);
    switch (e.type) {
    case SDL_KEYDOWN: {
        if (e.key.repeat || uiWantsKeyboard()) break;
        const SDL_Scancode sc = e.key.keysym.scancode;
        const bool ctrl = (e.key.keysym.mod & KMOD_CTRL) != 0, shift = (e.key.keysym.mod & KMOD_SHIFT) != 0;
        /* the editor's keys */
        if (ctrl && sc == SDL_SCANCODE_E) {
            setEditMode(!g_editMode);
            break;
        }
        if (editing()) {
            bool used = true;
            if (ctrl && sc == SDL_SCANCODE_Z && !shift) editUndo();
            else if (ctrl && (sc == SDL_SCANCODE_Y || (sc == SDL_SCANCODE_Z && shift))) editRedo();
            else if (ctrl && sc == SDL_SCANCODE_D) duplicateSel();
            else if (sc == SDL_SCANCODE_DELETE) deleteSel();
            else if (sc == SDL_SCANCODE_R && !ctrl) rotateSel(shift ? -15 : 15);
            else if (sc == SDL_SCANCODE_G && !ctrl) groundSel();
            else used = false;
            if (used) break;
        }
        if (sc == SDL_SCANCODE_ESCAPE) {
            if (g_drag.mode != 0) g_drag = Drag();   /* (a drag called off) */
            else if (g_mv.on) g_mv.on = false;
            else g_leave = true;
        } else if (sc == SDL_SCANCODE_TAB || sc == SDL_SCANCODE_F1) {
            g_opt.ui = !g_opt.ui;
        } else if (sc == SDL_SCANCODE_F12) {
            if (e.key.keysym.mod & KMOD_SHIFT) g_overlayShot = true;
            else g_cleanShot = true;
        } else if (sc == SDL_SCANCODE_F11 || (sc == SDL_SCANCODE_RETURN && (e.key.keysym.mod & KMOD_ALT))) {
            g_env.toggleFullscreen();
        } else if (sc == SDL_SCANCODE_HOME || sc == SDL_SCANCODE_P) {
            jumpToStart();
        } else if (sc == SDL_SCANCODE_O) {
            cameraTop();
        } else if (sc == SDL_SCANCODE_M) {
            if (g_mv.on) g_mv.on = false;
            else modelFor(g_sel);
        } else if (sc == SDL_SCANCODE_F && g_sel >= 0 && g_recs[g_sel].hasPos) {
            g_mv.on = false;
            g_cam.lookAt({g_recs[g_sel].x, g_recs[g_sel].y, g_recs[g_sel].z}, std::max(120.0, g_recs[g_sel].radius * 5));
        }
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_RIGHT || (g_mv.on && e.button.button == SDL_BUTTON_LEFT && !uiMouse)) {
            g_looking = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else if (e.button.button == SDL_BUTTON_LEFT && !uiMouse) {
            float x, y;
            windowToPixels(e.button.x, e.button.y, x, y);
            /* the editor: the gizmo of the selected record first, else a selection */
            const int hit = editing() ? gizmoHit(x, y) : 0;
            if (hit == 0 || !beginDrag(hit, x, y)) selectAt(x, y);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (e.button.button == SDL_BUTTON_LEFT && g_drag.mode != 0) endDrag();
        if (g_looking && (SDL_GetMouseState(nullptr, nullptr) & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) == 0) {
            g_looking = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        break;
    case SDL_MOUSEMOTION:
        if (g_looking) g_mdx += e.motion.xrel, g_mdy += e.motion.yrel;
        if (g_drag.mode != 0) {
            float x, y;
            windowToPixels(e.motion.x, e.motion.y, x, y);
            updateDrag(x, y);
        }
        break;
    case SDL_MOUSEWHEEL:
        if (!uiMouse) g_wheel += e.wheel.preciseY != 0.0f ? e.wheel.preciseY : float(e.wheel.y);
        break;
    default:
        break;
    }
}

double axis(const PadState &p, int a) {
    const double v = p.axis(a) / 32767.0;
    return std::fabs(v) < 0.2 ? 0.0 : (v - (v > 0 ? 0.2 : -0.2)) / 0.8;
}

void stepCamera(double dt) {
    const Uint8 *k = SDL_GetKeyboardState(nullptr);
    const bool typing = uiWantsKeyboard();
    double fwd = 0, rt = 0, up = 0, turnX = 0, turnY = 0;
    double mult = 1.0;
    if (!typing) {
        fwd += (k[SDL_SCANCODE_W] || k[SDL_SCANCODE_UP] ? 1 : 0) - (k[SDL_SCANCODE_S] || k[SDL_SCANCODE_DOWN] ? 1 : 0);
        rt += (k[SDL_SCANCODE_D] || k[SDL_SCANCODE_RIGHT] ? 1 : 0) - (k[SDL_SCANCODE_A] || k[SDL_SCANCODE_LEFT] ? 1 : 0);
        up += (k[SDL_SCANCODE_E] || k[SDL_SCANCODE_SPACE] ? 1 : 0) - (k[SDL_SCANCODE_Q] || k[SDL_SCANCODE_C] ? 1 : 0);
        if (k[SDL_SCANCODE_LSHIFT] || k[SDL_SCANCODE_RSHIFT]) mult = 4.0;
        if (k[SDL_SCANCODE_LCTRL] || k[SDL_SCANCODE_RCTRL]) mult = 0.25;
    }
    /* controllers: left stick moves, right stick looks, triggers down / up, shoulders: speed, A: select at the
     * centre, B: back, Y: player start, Back: leave, Start: panels */
    unsigned short buttons = 0;
    if (g_env.pads) {
        const PadSet pads = g_env.pads();
        for (const PadState &p : pads.pads) {
            fwd -= axis(p, SDL_CONTROLLER_AXIS_LEFTY);
            rt += axis(p, SDL_CONTROLLER_AXIS_LEFTX);
            turnX += axis(p, SDL_CONTROLLER_AXIS_RIGHTX);
            turnY += axis(p, SDL_CONTROLLER_AXIS_RIGHTY);
            up += p.axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0 - p.axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0;
            buttons |= uint16_t(p.buttons & 0xFFFF);
        }
    }
    const unsigned short pressed = uint16_t(buttons & ~g_padPrev);
    g_padPrev = buttons;
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) g_opt.speed = std::max(50.0, g_opt.speed / 1.5);
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) g_opt.speed = std::min(8000.0, g_opt.speed * 1.5);
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_Y)) jumpToStart();
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_START)) g_opt.ui = !g_opt.ui;
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_BACK)) g_leave = true;
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_B)) {
        if (g_mv.on) g_mv.on = false;
        else g_sel = -1;
    }
    if (pressed & (1u << SDL_CONTROLLER_BUTTON_A)) {
        const ImGuiIO &io = ImGui::GetIO();
        selectAt(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    }
    if (g_mv.on) {
        g_mv.yaw -= g_mdx * 0.3 + turnX * 120 * dt + rt * 90 * dt;
        g_mv.pitch = std::max(-85.0, std::min(85.0, g_mv.pitch + g_mdy * 0.3 + turnY * 90 * dt));
        g_mv.dist = std::max(g_mv.radius * 0.6, g_mv.dist * std::pow(0.85, g_wheel) * std::pow(0.4, fwd * dt));
    } else {
        if (g_wheel != 0) g_opt.speed = std::max(50.0, std::min(8000.0, g_opt.speed * std::pow(1.25, g_wheel)));
        g_cam.turn(-g_mdx * 0.15 - turnX * 140 * dt, -g_mdy * 0.15 - turnY * 100 * dt);
        g_cam.move(std::max(-1.0, std::min(1.0, fwd)), std::max(-1.0, std::min(1.0, rt)),
                   std::max(-1.0, std::min(1.0, up)), g_opt.speed * mult * dt);
    }
    g_mdx = g_mdy = g_wheel = 0;
}

/* ---- the test script (--viewer-test) ------------------------------------------------- */
std::vector<std::string> g_test;
size_t g_testPos;
int g_testWait;

bool takeFlag(const std::string &v) {
    return v != "0" && v != "off" && v != "false";
}

void doShot(const std::string &name, bool overlays, bool panels = false) {
    /* a picture with the panels (and, clean, the overlays) left out: built and presented, then read */
    g_cleanShot = !overlays && !panels;
    g_overlayShot = overlays && !panels;
    for (int i = 0; i < 2; i++) {
        uiFrame(buildUi, false);
        replay();
    }
    if (g_cleanShot) uiHide();
    replay();
    g_env.saveWindow(shotPath(name));
    g_cleanShot = g_overlayShot = false;
    g_shots++;
}

/* the editor's test ops (--viewer-test):
 *   eset:KIND:I:FIELD=V,FIELD=V   a change of fields          edup:KIND:I    duplicate (Ctrl+D)
 *   edel:KIND:I                   delete                      eundo / eredo
 *   edrag:KIND:I:X,Z              drag the gizmo's square to x, z (through the picking, as the mouse would)
 *   erot:KIND:I:DEG               drag the gizmo's ring to heading DEG
 *   eaxis:KIND:I:A:D              drag arrow A (x, y or z) by D map units
 *   egrid:N / esnap:0|1           the grid, the ground snap
 *   eapply / eplay                Apply / Play from here       esave:NAME[:enable]   save the mod */
void runEditOp(const std::string &name, const std::string &arg) {
    std::vector<std::string> p;
    for (size_t a = 0;;) {
        const size_t b = arg.find(':', a);
        p.push_back(arg.substr(a, b == std::string::npos ? std::string::npos : b - a));
        if (b == std::string::npos) break;
        a = b + 1;
    }
    int kind = -1, index = p.size() > 1 ? atoi(p[1].c_str()) : 0;
    for (int k = 0; k < viewer::REC_KINDS; k++)
        if (!p.empty() && p[0] == viewer::recKindKey(k)) kind = k;
    auto selectIt = [&]() {
        reselect(kind, index);
        if (g_sel < 0) host_log("live: viewer test: no record %s\n", arg.c_str());
        uiFrame(buildUi, false);   /* (the overlays' projection of this view, for the picking) */
        return g_sel >= 0;
    };
    if (name == "eset" && p.size() >= 3 && kind >= 0) {
        std::vector<edit::FieldValue> f;
        size_t a = 0;
        const std::string &s = p[2];
        while (a < s.size()) {
            size_t b = s.find(',', a);
            if (b == std::string::npos) b = s.size();
            const std::string kv = s.substr(a, b - a);
            const size_t eq = kv.find('=');
            const int fi = lm_field_index(kind, kv.substr(0, eq).c_str());
            if (fi >= 0 && eq != std::string::npos) f.push_back({fi, atol(kv.c_str() + eq + 1)});
            a = b + 1;
        }
        pushEdit(edit::makeSet(kind, index, f), kind, index);
    } else if (name == "edup" && kind >= 0) {
        if (selectIt()) duplicateSel();
    } else if (name == "edel" && kind >= 0) {
        if (selectIt()) deleteSel();
    } else if (name == "eundo") {
        editUndo();
    } else if (name == "eredo") {
        editRedo();
    } else if (name == "egrid") {
        g_eopt.grid = std::max(0, atoi(arg.c_str()));
    } else if (name == "esnap") {
        g_eopt.snapGround = takeFlag(arg);
    } else if ((name == "edrag" || name == "erot" || name == "eaxis") && kind >= 0 && p.size() >= 3) {
        if (!selectIt()) return;
        const Gizmo g = gizmo();
        if (!g.ok) {
            host_log("live: viewer test: %s: no gizmo (not editing, or off screen)\n", name.c_str());
            return;
        }
        ImVec2 s0 = g.c, s1;
        int mode = 1;
        bool ok = true;
        if (name == "edrag") {
            double x = 0, z = 0;
            sscanf(p[2].c_str(), "%lf,%lf", &x, &z);
            ok = projectPoint({x, g.p.y, z}, s1);
        } else if (name == "erot") {
            mode = 5;
            const double h0 = edit::degreesFromHeading(g_ed.get(kind, index, "heading")) * 3.14159265358979 / 180.0,
                         h1 = atof(p[2].c_str()) * 3.14159265358979 / 180.0, r = g.len * 1.35;
            ok = projectPoint({g.p.x + std::sin(h0) * r, g.p.y, g.p.z + std::cos(h0) * r}, s0) &&
                 projectPoint({g.p.x + std::sin(h1) * r, g.p.y, g.p.z + std::cos(h1) * r}, s1);
        } else {
            const int a = p[2] == "x" ? 0 : p[2] == "y" ? 1 : 2;
            const double d = p.size() > 3 ? atof(p[3].c_str()) : 0;
            mode = 2 + a;
            s0 = ImVec2((g.c.x + g.tip[a].x) / 2, (g.c.y + g.tip[a].y) / 2);
            ok = projectPoint({g.p.x + (a == 0 ? g.len / 2 + d : 0), g.p.y + (a == 1 ? g.len / 2 + d : 0),
                               g.p.z + (a == 2 ? g.len / 2 + d : 0)}, s1);
        }
        const int hit = gizmoHit(s0.x, s0.y);
        host_log("live: viewer test: %s: the gizmo at (%.0f, %.0f) is part %d (want %d)\n", name.c_str(), s0.x, s0.y,
                 hit, mode);
        if (!ok) {
            host_log("live: viewer test: %s: the target isn't in the picture (move the camera)\n", name.c_str());
            return;
        }
        if (!beginDrag(mode, s0.x, s0.y)) {
            host_log("live: viewer test: %s: the drag didn't start\n", name.c_str());
            return;
        }
        updateDrag((s0.x + s1.x) / 2, (s0.y + s1.y) / 2);
        updateDrag(s1.x, s1.y);
        endDrag();
    } else if (name == "eapply") {
        applyEdits();
    } else if (name == "eplay") {
        playEdits();
    } else if (name == "esave") {
        saveMod(p.empty() ? "test" : p[0], p.size() > 1 && takeFlag(p[1]));
    } else {
        host_log("live: viewer test: %s:%s not understood\n", name.c_str(), arg.c_str());
        return;
    }
    if (!g_editMsg.empty()) host_log("live: viewer test: %s -> %s\n", name.c_str(), g_editMsg.c_str());
}

void runTestOp() {
    if (g_testWait > 0) {
        g_testWait--;
        return;
    }
    if (g_testPos >= g_test.size()) return;
    const std::string op = g_test[g_testPos++];
    host_log("live: viewer test: %s\n", op.c_str());
    const size_t c = op.find(':');
    const std::string name = op.substr(0, c), arg = c == std::string::npos ? "" : op.substr(c + 1);
    const size_t eq = arg.find('=');
    const std::string key = arg.substr(0, eq), val = eq == std::string::npos ? "" : arg.substr(eq + 1);
    if (name == "wait") {
        g_testWait = std::max(0, atoi(arg.c_str()));
    } else if (name == "cam") {
        if (arg == "start") jumpToStart();
        else if (arg == "game") cameraFromGame(), g_mv.on = false;
        else if (arg == "top") cameraTop();
        else {
            double x, y, z, yaw, pitch;
            if (sscanf(arg.c_str(), "%lf,%lf,%lf,%lf,%lf", &x, &y, &z, &yaw, &pitch) == 5) {
                g_mv.on = false;
                g_cam.pos = {x, y, z}, g_cam.yaw = yaw, g_cam.pitch = pitch;
            }
        }
    } else if (name == "move") {   /* move:FWD,RIGHT,UP (drawing units) */
        double f = 0, r = 0, u = 0;
        sscanf(arg.c_str(), "%lf,%lf,%lf", &f, &r, &u);
        g_cam.move(f, r, 0, 1.0);
        g_cam.pos.y += u;
    } else if (name == "show") {
        for (int k = 0; k < CAT_COUNT; k++)
            if (key == kCatNames[k]) g_opt.show[k] = takeFlag(val);
        if (key == "far") g_opt.farOut = takeFlag(val);
        if (key == "plain") g_opt.plain = takeFlag(val);
        if (key == "markers") g_opt.markers = takeFlag(val);
        if (key == "labels") g_opt.labels = takeFlag(val);
        if (key == "collision") g_opt.collision = takeFlag(val);
        if (key == "zones") g_opt.zones = takeFlag(val);
        if (key == "labeldist") g_opt.labelDist = std::max(0.0, atof(val.c_str()));
        if (key == "cells") g_opt.allCells = takeFlag(val);
        for (int k = 0; k < viewer::REC_KINDS; k++)
            if (key == viewer::recKindKey(k)) g_opt.kinds[k] = takeFlag(val);
    } else if (name == "select" || name == "dump") {   /* select:KIND:I */
        const size_t c2 = arg.find(':');
        const std::string kk = arg.substr(0, c2);
        const int idx = c2 == std::string::npos ? 0 : atoi(arg.c_str() + c2 + 1);
        for (int k = 0; k < viewer::REC_KINDS; k++)
            if (kk == viewer::recKindKey(k)) g_sel = findRecord(k, idx);
        if (g_sel >= 0) {
            const Record &r = g_recs[g_sel];
            host_log("live: viewer: record %s (%s, 0x%06X):\n", r.title.c_str(), viewer::recKindName(r.kind), r.offset);
            for (const viewer::Field &f : r.fields) host_log("live: viewer:   %s: %s\n", f.name.c_str(), f.value.c_str());
            std::string hex = viewer::hexDump(r.raw);
            for (char &ch : hex)
                if (ch == '\n') ch = '|';
            host_log("live: viewer:   raw: %s\n", hex.c_str());
            if (name == "select" && r.hasPos) {
                g_mv.on = false;
                g_cam.lookAt({r.x, r.y, r.z}, std::max(150.0, r.radius * 6));
            }
        } else {
            host_log("live: viewer test: no record %s\n", arg.c_str());
        }
    } else if (name == "model") {   /* model:KIND:I (a record) or model:off */
        if (arg == "off") {
            g_mv.on = false;
        } else {
            const size_t c2 = arg.find(':');
            const std::string kk = arg.substr(0, c2);
            const int idx = c2 == std::string::npos ? 0 : atoi(arg.c_str() + c2 + 1);
            int rec = -1;
            for (int k = 0; k < viewer::REC_KINDS; k++)
                if (kk == viewer::recKindKey(k)) rec = findRecord(k, idx);
            if (!modelFor(rec)) host_log("live: viewer test: no model for %s\n", arg.c_str());
            g_testWait = 3;   /* (measured by the next replays) */
        }
    } else if (name == "orbit") {   /* orbit:YAW,PITCH[,DISTANCE FACTOR] */
        double y = g_mv.yaw, p = g_mv.pitch, d = 1.0;
        sscanf(arg.c_str(), "%lf,%lf,%lf", &y, &p, &d);
        g_mv.yaw = y, g_mv.pitch = p, g_mv.dist = std::max(1.0, g_mv.radius * 2.6 * d);
    } else if (name == "shot" || name == "oshot" || name == "pshot") {   /* clean / overlays / as on screen */
        doShot(arg, name != "shot", name == "pshot");
    } else if (name == "edit") {   /* the editor: edit:on / edit:off */
        setEditMode(takeFlag(arg));
    } else if (name.size() > 1 && name[0] == 'e' && name != "edit") {
        runEditOp(name, arg);
    } else if (name == "ui") {
        g_opt.ui = takeFlag(arg);
    } else if (name == "level") {
        requestLevel(atoi(arg.c_str()));
    } else if (name == "recapture") {
        g_recapture = true;
    } else if (name == "leave") {
        g_leave = true;
    } else if (name == "quit") {
        g_quitAfter = true;
        g_leave = true;
    } else {
        host_log("live: viewer test: %s not understood\n", op.c_str());
    }
}

/* ---- the frozen loop ------------------------------------------------------------------ */
void statusLine() {
    char b[256];
    snprintf(b, sizeof b, "%s  -  frozen at game frame %u%s", levelLabel(std::max(0, g_fz.levelNo)).c_str(),
             g_capturedFrame, g_fz.level ? "" : " (not a level: the game's own view)");
    g_status = b;
}

void runFrozen() {
    g_state = S_FROZEN;
    g_freezes++;
    setupFrozen();
    frozenHooksOn();
    readRecords();
    if (g_resetCam) cameraFromGame();
    g_resetCam = false;
    statusLine();
    g_leave = g_recapture = false;
    SDL_ShowCursor(SDL_ENABLE);
    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    while (g_state == S_FROZEN) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) viewerEvent(e);
        QueryPerformanceCounter(&now);
        const double dt = std::min(0.1, double(now.QuadPart - last.QuadPart) / double(freq.QuadPart));
        last = now;
        if (!g_test.empty()) runTestOp();
        stepCamera(dt);
        if (g_cleanShot || g_overlayShot) {
            const bool overlays = g_overlayShot;
            doShot(autoShotName(overlays), overlays);
            host_log("live: viewer: screenshot saved\n");
        }
        if (g_opt.ui || g_opt.markers || g_opt.collision) uiFrame(buildUi, true);
        else uiHide();
        /* the edits the panels asked for (after the frame: they replace the records the panels read) */
        while (!g_after.empty()) {
            std::vector<std::function<void()>> todo;
            todo.swap(g_after);
            for (auto &fn : todo) fn();
        }
        replay();
        if (g_mv.on && g_measure && g_mv.vtx > 0) {
            /* the model's own extent, from what it drew */
            g_mv.centre = {(g_mv.lo[0] + g_mv.hi[0]) / 2, (g_mv.lo[1] + g_mv.hi[1]) / 2, (g_mv.lo[2] + g_mv.hi[2]) / 2};
            g_mv.radius = std::max(5.0, 0.5 * std::sqrt((g_mv.hi[0] - g_mv.lo[0]) * (g_mv.hi[0] - g_mv.lo[0]) +
                                                        (g_mv.hi[1] - g_mv.lo[1]) * (g_mv.hi[1] - g_mv.lo[1]) +
                                                        (g_mv.hi[2] - g_mv.lo[2]) * (g_mv.hi[2] - g_mv.lo[2])));
            g_mv.dist = g_mv.radius * 2.6;
            g_mv.measured = true;
            g_measure = false;
            host_log("live: viewer: model %s: %u vertices, centre %.0f %.0f %.0f, radius %.0f\n", g_mv.title.c_str(),
                     g_mv.vtx, g_mv.centre.x, g_mv.centre.y, g_mv.centre.z, g_mv.radius);
        }
        if (g_leave) {
            leave();
            break;
        }
        if (g_recapture) {
            uiHide();
            beginCapture();
            break;
        }
        if (g_state != S_FROZEN) break;   /* (a level load: requestLevel) */
        Sleep(10);
    }
    uiHide();
    if (g_looking) {
        g_looking = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void requestLevel(int level) {
    if (level < 0 || level >= 60) return;
    g_wantLevel = level;
    g_loadedLevel = true;
    g_loads++;
    g_resetCam = true;
    g_state = S_LOAD;
    g_loadStart = plat_frames();
    g_mv.on = false;
    g_sel = -1;
    hooksOff();
    host_viewer_request(level, bonusVehicle(level));
    host_log("live: viewer: loading level %d (%s) by the game's level set-up\n", level, levelLabel(level).c_str());
}

void leave() {
    hooksOff();
    g_state = S_OFF;
    uiHide();
    autosave();
    if (g_quitAfter) {
        host_log("live: viewer: done (--viewer-test quit)\n");
        g_env.quit(0);
    }
    if (g_loadedLevel) {
        host_log("live: viewer: left after loading a level: restarting the game\n");
        g_env.restart();
        return;
    }
    host_log("live: viewer: left; the game goes on from frame %u\n", plat_frames());
}

}  // namespace

/* ---- the interface -------------------------------------------------------------------- */
void viewerSetup(const ViewerEnv &env) {
    g_env = env;
}

void viewerStartAtBoot(int level) {
    g_bootLevel = level < 0 ? 0 : level;
}

bool viewerSetTest(const std::string &spec) {
    g_test.clear();
    size_t p = 0;
    while (p <= spec.size()) {
        size_t q = spec.find(';', p);
        if (q == std::string::npos) q = spec.size();
        if (q > p) g_test.push_back(spec.substr(p, q - p));
        p = q + 1;
    }
    return !g_test.empty();
}

void viewerMenuTab() {
    readNames();
    ImGui::TextWrapped("The level viewer holds the game still and lets you fly around the level, look at its "
                       "records (vehicles, buildings, RDUs, TNT, the carrier's route ...) and at each model on its own.");
    const unsigned long long mode = rd<unsigned long long>(kGameMode);
    const bool level = (mode & (MODE_LEVEL | MODE_MAP_VIEW | MODE_LEVEL_START)) != 0;
    if (ImGui::Button("Open the viewer here")) {
        g_wanted = true;
        menuClose();
    }
    ImGui::TextDisabled(level ? "Leaving it goes back to the game where it was (two frames on)."
                              : "Not in a level now: the viewer shows the picture as it is (no free camera).");
    ImGui::Separator();
    const std::string cur = levelLabel(g_menuLevel);
    if (ImGui::BeginCombo("Level", cur.c_str())) {
        for (int i = 0; i < 60; i++)
            if (ImGui::Selectable(levelLabel(i).c_str(), i == g_menuLevel)) g_menuLevel = i;
        ImGui::EndCombo();
    }
    const bool canLoad = rd<uint8_t>(kFrontEnd) == 0;
    ImGui::BeginDisabled(!canLoad);
    if (ImGui::Button("Load it in the viewer")) {
        requestLevel(g_menuLevel);
        menuClose();
    }
    ImGui::EndDisabled();
    ImGui::TextWrapped(canLoad ? "Loads the level with the game's own set-up. Leaving the viewer afterwards restarts "
                                 "the game (the level wasn't reached by play)."
                               : "Levels can be loaded from a level or the attract demo (the menus' save thread must "
                                 "not be running). Or start bc.exe --viewer LEVEL.");
}

bool viewerWanted() {
    return g_wanted;
}

void viewerOpen() {
    if (g_state == S_OFF) g_wanted = true;
}

bool viewerActive() {
    return g_state != S_OFF;
}

void viewerGfxTask(unsigned ucode, unsigned ucodeData, unsigned dl, bool frameDl) {
    if (g_state != S_CAPTURE) return;
    g_cur.push_back({ucode, ucodeData, dl, frameDl});
    if (!frameDl) {
        if (g_cur.size() > 16) g_cur.erase(g_cur.begin());
        return;
    }
    /* the frame's display list: the tags recorded since the last one are its own */
    g_frozenTasks = g_cur;
    g_tags.swap(g_tagsBuilding);
    g_tagsBuilding.clear();
    g_cur.clear();
    g_captureFrames++;
    g_capturedFrame = plat_frames();
}

void viewerVi(unsigned frames) {
    (void) frames;
    readNames();
    if (g_bootLevel >= -1) {   /* --viewer: ask for the level now (the game takes it in the attract demo) */
        const int l = g_bootLevel;
        g_bootLevel = -2;
        requestLevel(l);
    }
    if (g_wanted && g_state == S_OFF) {
        g_wanted = false;
        g_loadedLevel = false;
        g_resetCam = true;
        beginCapture();
        return;
    }
    if (g_state == S_LOAD) {
        const unsigned long long mode = rd<unsigned long long>(kGameMode);
        const bool there = host_viewer_state() == 2 && rd<int>(kLevelNo) == g_wantLevel &&
                           (mode == MODE_LEVEL || mode == MODE_LEVEL_START) && rd<uint32_t>(kFramesInMode) >= 12;
        if (there) {
            host_viewer_done();
            host_log("live: viewer: level %d up after %u frames\n", g_wantLevel, plat_frames() - g_loadStart);
            if (g_playAfterLoad) {
                /* "Play from here": the game goes on into the edited level, as play (paced, with sound and input) */
                g_playAfterLoad = false;
                autosave();
                hooksOff();
                g_state = S_OFF;
                uiHide();
                host_log("live: viewer: test run of the edited level %d from frame %u (saving off)\n", g_wantLevel,
                         plat_frames());
                return;
            }
            beginCapture();
        } else if (plat_frames() - g_loadStart > 6000) {
            host_log("live: viewer: level %d didn't come up (mode %08llX, request state %d): giving up\n", g_wantLevel,
                     mode, host_viewer_state());
            host_viewer_done();
            leave();
        }
        return;
    }
    if (g_state == S_CAPTURE && g_captureFrames >= 2) runFrozen();
}

void viewerSummary() {
    if (g_freezes == 0 && g_loads == 0) return;
    host_log("live: viewer: %u freezes, %u levels loaded, %u pictures replayed (%u with the free camera's view), %u "
             "screenshots; %u tags recorded\n",
             g_freezes, g_loads, g_replays, g_vs.hooked, g_shots, g_tagCount);
}

}  // namespace live
