/* bc.exe's tagged frame interpolation (see live_interp.h).
 *
 * With a frame rate above the game's own, RT64 draws in-between pictures,
 * blending each transform of the last game frame with the one it matches in
 * the frame before.  Left to itself (G_EX_ID_AUTO) it guesses the pairs from
 * what is drawn (same render state and triangle count, nearest position),
 * which mixes up look-alike objects (debris, smoke puffs, identical vehicles),
 * blends objects that spawned into the slot of another one, and steps
 * whatever it can't pair.  Here the game names its objects instead:
 *
 *  - the game (PORT_HOST builds only) calls PORT_ITAG(gdl, kind, index) where
 *    it starts drawing an object (include/game/port.h); host_itag_hook
 *    records "the commands from this display-list address on belong to
 *    (kind, index)" in a host table.  Nothing is written to RDRAM, and the
 *    game never reads anything back, so it runs exactly as with the option
 *    off.  PORT_ISPAWN starts a new generation of a slot (a new object),
 *    PORT_ICUT_IF a camera snap;
 *  - while RT64 reads the task (Interpreter::processDisplayLists), every GBI
 *    command goes through dispatch() first (the GBI's function table, a
 *    public table of plain function pointers, is wrapped: RT64 itself is not
 *    changed).  A command at a tagged address makes that object the owner of
 *    what follows at that display-list level (and in the lists it calls); a
 *    model matrix the owner loads gets the ID hash(kind, index, generation,
 *    which matrix: its segment-relative address, else its order in the
 *    owner's drawing), set with RSP::matrixId (the call behind RT64's extended
 *    gEXMatrixGroup), so RT64 pairs it with the same ID of the frame before
 *    (G_EX_ORDER_LINEAR) and blends position, rotation and scale.  An ID that
 *    didn't exist in the frame before (a new object, a new generation) is
 *    shown as it is;
 *  - objects drawn as world-space vertices rebuilt every frame (people, the
 *    ground shadows of moving objects) get a transform of their own per
 *    object, with RT64's vertex interpolation (each vertex blended with the
 *    same vertex of the frame before);
 *  - the HUD and 2D (from PORT_ITAG_HUD on) get G_EX_ID_IGNORE: drawn exactly
 *    as the game drew them, never blended (text, the dial, the radar's
 *    blips), except the radar's turning sweep and heading arrow
 *    (PORT_ITAG_RADAR) and the attract mode's bobbing logo (untagged: AUTO);
 *  - untagged drawing (the terrain, the backdrop) keeps RT64's own matching
 *    (G_EX_ID_AUTO), as with video.interpolation = auto;
 *  - a tagged object that moved much further than it moves normally (a
 *    respawn, a teleport) starts a new generation: no blend across the jump.
 *
 * No GPL code: written against RT64's (MIT) API. */
#include <windows.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <unordered_map>
#include <vector>

#include "gbi/rt64_gbi.h"
#include "hle/rt64_application.h"
#include "hle/rt64_interpreter.h"
#include "hle/rt64_rsp.h"
#include "hle/rt64_state.h"
#include "hle/rt64_workload_queue.h"

#include "live_interp.h"

extern "C" {
#include "plat_host.h"
}

namespace live {

namespace {

/* the tag kinds (include/game/port.h) */
enum {
    K_NONE = 0, K_HUD = 1, K_OBJ, K_SHADOW, K_SHADOWFE, K_BUILDING, K_BPART, K_FALL, K_RISE, K_BILLBOARD,
    K_PARTICLE, K_PEOPLE, K_GSHADOW, K_MARKER, K_RADAR, K_CUT = 30, K_SPAWN = 31
};

/* world-space vertices rebuilt every frame: vertex interpolation */
bool vertexKind(uint32_t k) {
    return k == K_PEOPLE || k == K_GSHADOW;
}

RT64::Application *g_app;
bool g_on;
bool g_log;
GbiCommandHook g_hook;   /* interpSetCommandHook (the orbit camera) */
GbiMatrixHook g_mtxHook;
GbiCommandHook g_wideHook;   /* interpSetWideHooks (widescreen, live_wide.cpp) */
GbiVertexHook g_vtxHook;
GbiAddressHook g_addrHook;

/* the vertices RT64 has in the workload being built */
uint32_t vertexCount(RT64::State *state) {
    const RT64::WorkloadQueue &q = *state->ext.workloadQueue;
    return q.workloads[q.writeCursor].drawData.vertexCount();
}

bool anyHook() {
    return g_hook != nullptr || g_mtxHook != nullptr || g_wideHook != nullptr || g_vtxHook != nullptr ||
           g_addrHook != nullptr;
}

/* the tags of the display lists being built (physical address -> kind, index) */
struct Tag {
    uint32_t kind, index;
};
std::unordered_map<uint32_t, Tag> g_tags;
bool g_cut;
unsigned g_spawnsSeen;
/* object generations (PORT_ISPAWN, jumps) */
struct Gen {
    uint32_t gen = 0;
    unsigned jumpFrame = ~0u;
};
std::unordered_map<uint32_t, Gen> g_gen;

uint32_t slotKey(uint32_t kind, uint32_t index) {
    return (kind << 24) ^ index;
}

void itagHook(unsigned addr, int kind, int index) {
    if (kind == K_CUT) {
        g_cut = true;
    } else if (kind == K_SPAWN) {
        g_gen[slotKey(uint32_t(index) >> 16, uint32_t(index) & 0xFFFF)].gen++;
        g_spawnsSeen++;
    } else {
        g_tags[addr & 0xFFFFFF] = {uint32_t(kind), uint32_t(index)};
    }
}

/* ---- while RT64 reads a task ---------------------------------------------- */
struct Owner {
    uint32_t kind = K_NONE, index = 0, ordinal = 0;
};
std::vector<Owner> g_owners;   /* per display-list level (RT64's return-address stack) */
Owner g_owner;
constexpr int kDepth = 32;     /* RSP_MATRIX_STACK_SIZE */
uint32_t g_ids[kDepth];        /* the ID of each modelview matrix-stack level */
uint8_t g_vpos[kDepth];
int g_depth;
uint32_t g_applied;            /* the ID last given to RT64 */
uint8_t g_appliedVpos;
unsigned g_frame;              /* main frames seen */
bool g_mainTask;

/* jump detection: where each tagged transform was in the last frame */
struct Track {
    unsigned frame = 0;
    float pos[3] = {};
    float move = 0;
};
std::unordered_map<uint64_t, Track> g_tracks;
constexpr float kJumpMin = 400.0f;      /* world units in one frame ... */
constexpr float kJumpFactor = 8.0f;     /* ... and this many times its last move */

/* statistics */
unsigned g_tasks, g_tagged, g_jumps;
unsigned g_frameOwners, g_frameMtx, g_frameKinds[32];   /* this frame (--interp-log) */
float g_maxMove;                                          /* largest tagged move since the last log */
unsigned long long g_cmds;
double g_taskSeconds;
LARGE_INTEGER g_t0, g_freq;

/* original functions of each GBI whose table was wrapped */
struct Wrapped {
    RT64::GBI *gbi;
    RT64::GBIFunction orig[UCODE_MAP_SIZE];
};
std::vector<Wrapped> g_wrapped;
const RT64::GBIFunction *g_cur;   /* the current task's originals */

uint32_t mix(uint32_t h, uint32_t v) {
    h ^= v + 0x9E3779B9u + (h << 6) + (h >> 2);
    return h;
}

uint32_t ownerId(const Owner &o, uint32_t ordinal) {
    uint32_t gen = 0;
    auto it = g_gen.find(slotKey(o.kind, o.index));
    if (it != g_gen.end()) gen = it->second.gen;
    uint32_t h = mix(mix(mix(mix(0x42430000u, o.kind), o.index), gen), ordinal);
    if (h == G_EX_ID_IGNORE || h == G_EX_ID_AUTO) h = 0x12345678u;
    return h;
}

unsigned g_test;   /* --tag-test (experiments): 1 no vertex interpolation, 2 HUD auto, 4 no decompose,
                      8 ORDER_AUTO, 16 IDs auto (wrapping and tags only) */

void apply(RT64::State *state, uint32_t id, uint8_t vpos) {
    if (g_test & 1) vpos = G_EX_COMPONENT_SKIP;
    if ((g_test & 2) && id == G_EX_ID_IGNORE) id = G_EX_ID_AUTO;
    if (g_test & 16) id = G_EX_ID_AUTO;
    if (id == g_applied && vpos == g_appliedVpos) return;
    g_applied = id;
    g_appliedVpos = vpos;
    RT64::RSP &rsp = *state->rsp;
    if (id == G_EX_ID_AUTO) {
        /* RT64's defaults (TransformGroup) */
        rsp.matrixId(id, false, false, true, G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO,
                     G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                     G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO, G_EX_ORDER_AUTO, G_EX_ASPECT_AUTO, G_EX_EDIT_NONE,
                     false, false);
    } else if (id == G_EX_ID_IGNORE) {
        rsp.matrixId(id, false, false, true, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_ORDER_LINEAR, G_EX_ASPECT_AUTO, G_EX_EDIT_NONE,
                     false, false);
    } else {
        rsp.matrixId(id, false, false, !(g_test & 4), G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
                     G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, vpos,
                     G_EX_COMPONENT_SKIP, G_EX_COMPONENT_AUTO, G_EX_COMPONENT_AUTO,
                     (g_test & 8) ? G_EX_ORDER_AUTO : G_EX_ORDER_LINEAR, G_EX_ASPECT_AUTO, G_EX_EDIT_NONE, false,
                     false);
    }
}

/* the ID for drawing that has no matrix of its own under owner o */
uint32_t plainId(const Owner &o) {
    if (o.kind == K_HUD) return G_EX_ID_IGNORE;
    if (vertexKind(o.kind)) return ownerId(o, 0xFFFF);
    return G_EX_ID_AUTO;
}

/* a new owner at this level; geometry of its own (vertex kinds, HUD) starts
 * a transform of its own at its first vertex */
void setOwner(RT64::State *state, const Tag &t) {
    const Owner before = g_owner;
    g_owner.kind = t.kind;
    g_owner.index = t.index;
    g_owner.ordinal = 0;
    g_tagged++;
    g_frameOwners++;
    g_frameKinds[t.kind & 31]++;
    const bool own = vertexKind(t.kind) || t.kind == K_HUD;
    const bool had = vertexKind(before.kind) || before.kind == K_HUD;
    if (own || had) {
        g_ids[g_depth] = own ? plainId(g_owner) : G_EX_ID_AUTO;
        g_vpos[g_depth] = vertexKind(t.kind) ? G_EX_COMPONENT_INTERPOLATE : G_EX_COMPONENT_SKIP;
        state->rsp->modelViewProjChanged = true;   /* a new world transform at the next vertex */
        apply(state, g_ids[g_depth], g_vpos[g_depth]);
    }
}

/* after a tagged model matrix: did the object jump? (then a new generation) */
bool jumped(RT64::State *state, uint32_t ordinal) {
    const RT64::RSP &rsp = *state->rsp;
    const hlslpp::float4x4 &m = rsp.modelMatrixStack[rsp.modelMatrixStackSize - 1];
    const float p[3] = {float(m[3][0]), float(m[3][1]), float(m[3][2])};
    const uint64_t key = (uint64_t(slotKey(g_owner.kind, g_owner.index)) << 32) ^ ordinal;
    Track &t = g_tracks[key];
    bool jump = false;
    if (t.frame + 1 == g_frame) {
        const float dx = p[0] - t.pos[0], dy = p[1] - t.pos[1], dz = p[2] - t.pos[2];
        const float d = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (d > g_maxMove) g_maxMove = d;
        if (d > kJumpMin && d > kJumpFactor * t.move) {
            jump = true;
            g_jumps++;
            if (g_log)
                host_log("live: interp tag %u/%u (matrix %u) jumped %.0f units (last move %.0f): new generation\n",
                         g_owner.kind, g_owner.index, ordinal, d, t.move);
        }
        t.move = d;
    } else {
        t.move = 0;
    }
    if (t.frame != g_frame) {
        t.frame = g_frame;
        memcpy(t.pos, p, sizeof p);
    }
    return jump;
}

/* a command other than G_MTX / G_POPMTX: the hooks' turn (the orbit camera's,
 * then widescreen's), else RT64's own; after a G_VTX, the vertex hook */
void runHooks(RT64::State *state, RT64::DisplayList **dl, uint8_t op) {
    if (g_hook != nullptr && g_hook(state, dl, g_cur)) return;
    if (g_wideHook != nullptr && g_wideHook(state, dl, g_cur)) return;
    g_cur[op](state, dl);
}
void run(RT64::State *state, RT64::DisplayList **dl, uint8_t op) {
    if (op != 0x04 || g_vtxHook == nullptr) {
        runHooks(state, dl, op);
        return;
    }
    const uint32_t w1 = (*dl)->w1;
    const uint32_t first = vertexCount(state);
    runHooks(state, dl, op);
    g_vtxHook(state, w1, first);
}

void dispatch(RT64::State *state, RT64::DisplayList **dl) {
    const uint32_t w0 = (*dl)->w0;
    const uint8_t op = uint8_t(w0 >> 24);
    if (g_addrHook != nullptr)
        g_addrHook(state, uint32_t(reinterpret_cast<const uint8_t *>(*dl) - state->RDRAM) & 0xFFFFFF);
    if (!g_on) {   /* only the command hooks want the commands */
        if (op == 0x01) {
            g_cur[op](state, dl);
            if (g_mtxHook != nullptr) g_mtxHook(state, uint8_t(w0 >> 16));
        } else if (op == 0xBD) {
            g_cur[op](state, dl);
        } else {
            run(state, dl, op);
        }
        return;
    }
    g_cmds++;
    /* follow RT64's display-list levels: a called list inherits the owner */
    const size_t level = state->returnAddressStack.size();
    while (g_owners.size() > level) {
        g_owner = g_owners.back();
        g_owners.pop_back();
    }
    while (g_owners.size() < level) g_owners.push_back(g_owner);
    if (!g_tags.empty()) {
        const uint32_t addr = uint32_t(reinterpret_cast<const uint8_t *>(*dl) - state->RDRAM) & 0xFFFFFF;
        auto it = g_tags.find(addr);
        if (it != g_tags.end()) setOwner(state, it->second);
    }
    RT64::RSP &rsp = *state->rsp;
    if (op == 0x01) {   /* G_MTX */
        const uint8_t params = uint8_t(w0 >> 16);
        const uint32_t w1 = (*dl)->w1;
        const bool proj = (params & rsp.projMask) != 0;
        g_cur[op](state, dl);
        if (g_mtxHook != nullptr) g_mtxHook(state, params);
        if (!proj) {
            if ((params & rsp.pushMask) && g_depth < kDepth - 1) g_depth++;
            uint32_t id;
            if (g_owner.kind == K_NONE) {
                id = G_EX_ID_AUTO;
            } else if (g_owner.kind == K_HUD) {
                id = G_EX_ID_IGNORE;
            } else {
                /* which of the owner's matrices: a segment-relative address
                 * names it (the same part of a model every frame, even when
                 * other parts are left out); a physical one (double-buffered
                 * pools) by its order in the owner's drawing */
                const uint32_t ord = g_owner.ordinal++;
                const uint32_t k = ((w1 >> 24) & 0xF) != 0 ? (0x80000000u | (w1 & 0x0FFFFFFF)) : ord;
                if (jumped(state, k)) {
                    Gen &g = g_gen[slotKey(g_owner.kind, g_owner.index)];
                    if (g.jumpFrame != g_frame) {   /* once a frame, however many of its matrices jumped */
                        g.gen++;
                        g.jumpFrame = g_frame;
                    }
                }
                id = ownerId(g_owner, k);
                g_frameMtx++;
            }
            g_ids[g_depth] = id;
            g_vpos[g_depth] = G_EX_COMPONENT_SKIP;
            apply(state, id, G_EX_COMPONENT_SKIP);
        }
        return;
    }
    if (op == 0xBD) {   /* G_POPMTX */
        g_cur[op](state, dl);
        if ((*dl)->w1 == 0 && g_depth > 0) {
            g_depth--;
            apply(state, g_ids[g_depth], g_vpos[g_depth]);
        }
        return;
    }
    run(state, dl, op);
}

void resetTask() {
    g_owners.clear();
    g_owner = Owner();
    g_depth = 0;
    g_ids[0] = G_EX_ID_AUTO;
    g_vpos[0] = G_EX_COMPONENT_SKIP;
    g_applied = G_EX_ID_AUTO;   /* RSP::reset (each task) put RT64's ID stack back to AUTO */
    g_appliedVpos = G_EX_COMPONENT_SKIP;
}

void wrap(RT64::GBI *gbi) {
    for (const Wrapped &w : g_wrapped)
        if (w.gbi == gbi) {
            g_cur = w.orig;
            return;
        }
    g_wrapped.push_back({});
    Wrapped &w = g_wrapped.back();
    w.gbi = gbi;
    for (int i = 0; i < UCODE_MAP_SIZE; i++) {
        w.orig[i] = gbi->map[i];
        if (gbi->map[i] != nullptr) gbi->map[i] = dispatch;
    }
    g_cur = w.orig;
    host_log("live: tagged interpolation: GBI %u wrapped\n", unsigned(gbi->ucode));
}

void unwrapAll() {
    for (Wrapped &w : g_wrapped)
        for (int i = 0; i < UCODE_MAP_SIZE; i++) w.gbi->map[i] = w.orig[i];
    g_wrapped.clear();
    g_cur = nullptr;
}

}  // namespace

void interpTagsSetup(RT64::Application *app) {
    g_app = app;
    QueryPerformanceFrequency(&g_freq);
}

void interpTagsEnable(bool on) {
    if (on == g_on) return;
    g_on = on;
    g_tags.clear();
    g_cut = false;
    if (on) {
        host_itag_hook = itagHook;
    } else {
        host_itag_hook = nullptr;
        if (!anyHook()) unwrapAll();
    }
    host_log("live: tagged interpolation %s\n", on ? "on" : "off");
}

void interpSetCommandHook(GbiCommandHook hook, GbiMatrixHook mtxHook) {
    g_hook = hook;
    g_mtxHook = mtxHook;
    if (!anyHook() && !g_on) unwrapAll();
}

void interpSetWideHooks(GbiCommandHook hook, GbiVertexHook vtxHook, GbiAddressHook addrHook) {
    g_wideHook = hook;
    g_vtxHook = vtxHook;
    g_addrHook = addrHook;
    if (!anyHook() && !g_on) unwrapAll();
}

bool interpTagsOn() {
    return g_on;
}

void interpTagsSetLog(bool on) {
    g_log = on;
}

void interpTagsSetTest(unsigned mask) {
    g_test = mask;
}

void interpTagsBeforeTask(unsigned dl, bool mainFrame) {
    (void) dl;
    if ((!g_on && !anyHook()) || g_app->interpreter->hleGBI == nullptr) return;
    wrap(g_app->interpreter->hleGBI);
    resetTask();
    if (!g_on) return;
    g_mainTask = mainFrame;
    if (mainFrame) g_frame++;
    g_tasks++;
    QueryPerformanceCounter(&g_t0);
}

void interpTagsAfterTask(bool mainFrame) {
    if (!g_on) return;
    LARGE_INTEGER t1;
    QueryPerformanceCounter(&t1);
    g_taskSeconds += double(t1.QuadPart - g_t0.QuadPart) / double(g_freq.QuadPart);
    if (mainFrame) {
        if (g_log && g_frame % 50 == 0) {
            char kinds[256];
            int n = 0;
            for (int k = 0; k < 32 && n < 200; k++)
                if (g_frameKinds[k] != 0) n += snprintf(kinds + n, sizeof kinds - n, " %d:%u", k, g_frameKinds[k]);
            kinds[n] = 0;
            host_log("live: interp tags, frame %u: %u tags recorded, %u owners (kind:count%s), %u tagged matrices; "
                     "largest move %.1f\n",
                     g_frame, unsigned(g_tags.size()), g_frameOwners, kinds, g_frameMtx, g_maxMove);
            g_maxMove = 0;
        }
        g_frameOwners = g_frameMtx = 0;
        memset(g_frameKinds, 0, sizeof g_frameKinds);
        g_tags.clear();
        /* forget objects not seen for a while */
        if (g_tracks.size() > 4096)
            for (auto it = g_tracks.begin(); it != g_tracks.end();)
                it = g_frame - it->second.frame > 8 ? g_tracks.erase(it) : std::next(it);
    }
}

bool interpTagsTakeCut() {
    const bool c = g_cut;
    g_cut = false;
    return c;
}

void interpTagsSummary() {
    if (g_tasks == 0) return;
    host_log("live: tagged interpolation: %u tasks (%u frames), %llu commands, %u owner changes, %u spawns, "
             "%u jumps; %.3f ms per task reading the display lists (tags included)\n",
             g_tasks, g_frame, g_cmds, g_tagged, g_spawnsSeen, g_jumps, g_taskSeconds * 1e3 / g_tasks);
}

}  // namespace live
