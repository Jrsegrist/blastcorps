/* bc.exe's widescreen fixes for the game's full-width 2D (see live_wide.h).
 *
 * With video.aspect wider than 4:3, RT64 widens the picture: a projection
 * whose viewport spans the whole screen gets its x scaled by 1 / k (k = the
 * picture's width over 4:3; rt64_projection_processor.cpp, perspective and
 * orthographic alike), so 3D sees further to the sides and 2D drawn with an
 * orthographic projection keeps its shape, centred (pillarboxed) like the
 * HUD.  Texture rectangles narrower than the screen are centred too;
 * full-width fill rectangles are stretched.
 *
 * A few of the game's 2D draws are meant to cover the whole screen, and
 * stopped at the 4:3 edges:
 *  - the sky (2D810.c backdrop_draw): one textured quad across the screen
 *    under the orthographic projection (DynamicBuf.orthoHi), the ground
 *    colour below its horizon a full-width fill rectangle.  Above the
 *    horizon the sides showed whatever was there: the wider 3D view's far
 *    ground, or older pictures (nothing clears the colour image there; the
 *    sky always covered it on the N64);
 *  - the fade to black of a failed mission (17E10.c hud_mission_failed) and
 *    at the end of the attract demo (title_draw_attract): a full-screen
 *    quad (D_802FA8B0, 0..319 x 0..239) under DynamicBuf.ortho;
 *  - the dark panels behind the full-width message windows (26570.c
 *    yoshi_run_window, gYoshiWindowDl: a +-1000 square scaled to the
 *    window, its edges fading out);
 *  - the front end's background picture (hd_front_end/196F0.c fe_bg_draw):
 *    a 320x240 picture drawn as 64x16 texture rectangles, each centred.
 *
 * The fixes, all on RT64's side while it reads the display list (the GBI
 * table wrapped by live_interp.cpp), RDRAM and the game's display lists and
 * vertices untouched:
 *  - after RT64 read those vertices (G_VTX of their addresses), its own
 *    copies (the workload's vertex arrays, host memory) are widened about
 *    the screen's centre by k (a little more; the rest is clipped).  The
 *    sky's s texture coordinates go with them, so its texture continues to
 *    the sides as it would on a wider screen (it repeats: the game scrolls
 *    it with the camera's heading) with the same texel size; the message
 *    windows widen gradually from 80 % of the screen's width up (a small
 *    window keeps its shape, a full-width one reaches the edges);
 *  - the background picture's rectangles are drawn with RT64's extended
 *    rectangle aspect G_EX_ASPECT_STRETCH (the picture stretched to the
 *    width, as full-width rectangles are).
 * Two front-end pictures limited to the 4:3 middle get the same treatment
 * (the game names them with PORT_WTAG, port.h):
 *  - the ticker's lit band (01C40.c fe_player_draw_panel, gPanelDl: a 3D
 *    strip 320 units wide; the player select and the results screens) is
 *    widened like the sky;
 *  - the globe's starfield (11530.c fe_globe_draw_stars): the stars are
 *    spawned in a 4:3 cone; each star's quad is moved out by k from the view's
 *    axis (its size kept), so they fill the wider picture as densely.
 *
 * video.hud_position = edges (default centred): the HUD's left and right
 * parts move to the screen's edges.  The game tags its HUD drawing with the
 * side each part belongs to (PORT_WTAG: the dial, the radar, the icons, the
 * panel ...; text strings by where they lie, 14B30.c); while a side is set,
 * RT64's extended-GBI origins (as G_EX_SETVIEWPORTALIGN / G_EX_SETRECTALIGN
 * would set them) anchor the drawing to that edge: the viewport (vertices
 * under any projection, the radar's turning parts included) and texture /
 * fill rectangles keep their distance from the 4:3 picture's edge to the
 * screen's edge, and the scissor spans the whole screen.  Centred parts and
 * everything untagged (menus, message windows, markers that follow the 3D
 * view) stay centred.  With the extended aspect left at Original (Centered)
 * nothing is set.
 *
 * The HUD, text and other 2D are otherwise left as they are (pillarboxed).
 * With video.aspect = original nothing is wrapped or changed.
 *
 * No GPL code: written against RT64's (MIT) API. */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "gbi/rt64_gbi.h"
#include "hle/rt64_rdp.h"
#include "hle/rt64_rsp.h"
#include "hle/rt64_state.h"
#include "hle/rt64_workload_queue.h"

#include "live_interp.h"
#include "live_wide.h"

extern "C" {
#include "plat_host.h"
}

namespace live {

namespace {

double g_k = 1.0;          /* the picture's width over 4:3 */
/* statistics */
unsigned g_sky, g_fade, g_window, g_bgRects;

/* physical addresses (tools_port/names.txt) */
constexpr uint32_t kBackdropVtx0 = 0x2FA820u, kBackdropVtx1 = 0x2FA860u;   /* gBackdropVtx[2][4] */
constexpr uint32_t kFadeVtx0 = 0x2FA8B0u, kFadeVtx1 = 0x2FA8F0u;           /* D_802FA8B0[2][4] */
constexpr uint32_t kWindowVtx = 0x2F97B0u, kWindowVtxEnd = 0x2F98B0u;      /* gYoshiWindowDl's vertices */
constexpr uint32_t kBgPicturePtr = 0x8021AB80u;                            /* gBgPicture (front end) */
/* a little wider than the picture: no seam at its edges after rounding */
constexpr double kMargin = 0.05;

RT64::Workload &workload(RT64::State *state) {
    return state->ext.workloadQueue->workloads[state->ext.workloadQueue->writeCursor];
}

/* widen RT64's copies of the vertices [first, first + n) of the current
 * workload by f about the screen's centre (the x that the current
 * projection puts at clip x 0); `texS`: the s texture coordinate goes with
 * x (an affine function of x along the quad's first two vertices);
 * `quads`: each group of 4 vertices moves as its centre does (its size kept) */
bool widen(RT64::State *state, uint32_t first, uint32_t n, double f, bool texS, bool quads = false) {
    RT64::RSP &rsp = *state->rsp;
    auto &d = workload(state).drawData;
    if (n == 0 || first + n > d.vertexCount() || d.posFloats.size() < size_t(first + n) * 3 ||
        d.tcFloats.size() < size_t(first + n) * 2 || d.posTransformed.size() < first + n ||
        d.posScreen.size() < first + n)
        return false;
    const hlslpp::float4x4 &m = rsp.modelViewProjMatrix;
    const double m00 = float(m[0][0]), m10 = float(m[1][0]), m20 = float(m[2][0]), m30 = float(m[3][0]);
    if (std::fabs(m00) < 1e-12) return false;
    double dsdx = 0;
    if (texS && n >= 2) {
        const float *p = &d.posFloats[first * 3];
        const double dx = double(p[3]) - double(p[0]);
        if (std::fabs(dx) < 1e-6 || p[4] != p[1]) return false;
        dsdx = (double(d.tcFloats[(first + 1) * 2]) - double(d.tcFloats[first * 2])) / dx;
    }
    const interop::RSPViewport &vp = rsp.viewportStack[rsp.viewportStackSize - 1];
    double shift = 0;   /* quads: the current quad's move */
    for (uint32_t i = first; i < first + n; i++) {
        float *p = &d.posFloats[i * 3];
        if (quads && (i - first) % 4 == 0) {
            /* move the whole quad as its centre moves: its size is kept */
            const uint32_t m4 = std::min<uint32_t>(4, first + n - i);
            double c[3] = {0, 0, 0};
            for (uint32_t j = 0; j < m4; j++)
                for (int a = 0; a < 3; a++) c[a] += double(d.posFloats[(i + j) * 3 + a]) / m4;
            const double pc = -(c[1] * m10 + c[2] * m20 + m30) / m00;
            shift = (c[0] - pc) * (f - 1.0);
        }
        const double pivot = -(double(p[1]) * m10 + double(p[2]) * m20 + m30) / m00;
        const double x = quads ? double(p[0]) + shift : pivot + (double(p[0]) - pivot) * f;
        if (texS) d.tcFloats[i * 2] = float(double(d.tcFloats[i * 2]) + (x - double(p[0])) * dsdx);
        p[0] = float(x);
        /* what RT64 derived from the position (rt64_rsp.cpp setVertexCommon) */
        const hlslpp::float4 tf = hlslpp::mul(hlslpp::float4(p[0], p[1], p[2], 1.0f), m);
        d.posTransformed[i] = tf;
        d.posScreen[i] = (tf.xyz / hlslpp::float3(tf.w, -tf.w, tf.w)) * vp.scale + vp.translate;
    }
    return true;
}

/* ---- the game's widescreen tags (port.h PORT_WTAG) ------------------------- */
enum { W_NONE = 0, W_LEFT, W_RIGHT, W_SPAN, W_TEXT_SIDES, W_TICKER, W_STARS, W_TEXT };
enum Side { S_CENTRE = 0, S_LEFT, S_RIGHT };

struct WTag {
    int kind, x0, x1;
};
/* recorded while the game builds a display list, used while RT64 reads it,
 * dropped after each task (as live_interp.cpp's tags) */
std::unordered_map<uint32_t, WTag> g_wtags;   /* display-list address -> tag */
struct TextRange {
    uint32_t first, end;   /* physical addresses of the string's vertices */
    Side side;
};
std::vector<TextRange> g_text;
bool g_matched;            /* the task being read met a tag (then the tags are dropped after it) */

bool g_edges;              /* video.hud_position = edges */
/* while RT64 reads a task */
int g_mode = W_NONE;       /* the tag in effect (the last one met) */
Side g_dlSide = S_CENTRE;  /* the side it gives */
Side g_applied = S_CENTRE; /* the side RT64's origins are set for */
/* statistics */
unsigned g_tickerVerts, g_starQuads, g_sideCmds, g_textStrings;

/* which side of the 4:3 screen x0..x1 lies on: left or right when it keeps to
 * the outer part of one half, centred when it straddles the middle */
Side sideOf(int x0, int x1) {
    if (x0 > x1) std::swap(x0, x1);
    const double mid = (x0 + x1) / 2.0;
    if (x0 < 107 && mid < 145) return S_LEFT;
    if (x1 > 213 && mid > 175) return S_RIGHT;
    return S_CENTRE;
}

void wtagHook(unsigned addr, int kind, int x0, int x1) {
    if (kind == W_TEXT) {
        /* a queued string: x0 vertices from addr (the game's Vtx: s16 ob[3] first, 16 bytes); x1 = fit */
        if (x0 <= 0 || x0 > 4096) return;
        const uint8_t *v = reinterpret_cast<const uint8_t *>(uintptr_t(addr));
        int lo = 32767, hi = -32768;
        for (int i = 0; i < x0; i++) {
            const int x = *reinterpret_cast<const int16_t *>(v + i * 16);
            lo = std::min(lo, x);
            hi = std::max(hi, x);
        }
        const uint32_t a = addr & 0xFFFFFF;
        g_textStrings++;
        if (g_text.size() > 4096) g_text.clear();   /* (never read: no task ran) */
        g_text.push_back({a, a + uint32_t(x0) * 16, x1 != 0 ? S_CENTRE : sideOf(lo, hi)});
        return;
    }
    if (g_wtags.size() > 65536) g_wtags.clear();
    g_wtags[addr & 0xFFFFFF] = {kind, x0, x1};
}

/* RT64's extended origins for side s (only with the HUD at the edges) */
void applySide(RT64::State *state, Side s) {
    if (s == g_applied) return;
    g_applied = s;
    g_sideCmds++;
    RT64::RSP &rsp = *state->rsp;
    RT64::RDP &rdp = *state->rdp;
    /* a right origin adds the colour image's width (rt64_rdp.cpp movedFromOrigin): the
     * offsets take it back, so the game's own 4:3 coordinates are kept, measured from
     * the right edge */
    const int32_t w4 = int32_t(rdp.colorImage.width != 0 ? rdp.colorImage.width : 320) * 4;
    const uint16_t ori = s == S_LEFT ? G_EX_ORIGIN_LEFT : s == S_RIGHT ? G_EX_ORIGIN_RIGHT : G_EX_ORIGIN_NONE;
    const int32_t off = s == S_RIGHT ? -w4 : 0;
    /* vertices: the viewport (the current one, and those the game sets later) */
    rsp.setViewportAlign(ori, int16_t(off), 0);
    rsp.extended.viewportOriginStack[rsp.viewportStackSize - 1] = ori;
    rsp.viewportChanged = true;
    /* rectangles */
    RT64::ExtendedAlignment rect;
    RT64::ExtendedAlignment sc;
    if (s != S_CENTRE) {
        rect.leftOrigin = rect.rightOrigin = ori;
        rect.leftOffset = rect.rightOffset = off;
        /* the scissor spans the whole screen (the game's spans the 4:3 one) */
        sc.leftOrigin = G_EX_ORIGIN_LEFT;
        sc.rightOrigin = G_EX_ORIGIN_RIGHT;
        sc.rightOffset = -w4;
    }
    rdp.setRectAlign(rect);
    rdp.setScissorAlign(sc);
    const int top = rdp.scissorStackSize - 1;
    const RT64::FixedRect r = rdp.scissorRectStack[top];
    rdp.setScissor(rdp.scissorModeStack[top], r.ulx, r.uly, r.lrx, r.lry, sc);
}

/* before every command: a tag at its address? */
void address(RT64::State *state, uint32_t addr) {
    if (g_wtags.empty()) return;
    auto it = g_wtags.find(addr);
    if (it == g_wtags.end()) return;
    g_matched = true;
    const WTag &t = it->second;
    g_mode = t.kind;
    g_dlSide = t.kind == W_LEFT ? S_LEFT : t.kind == W_RIGHT ? S_RIGHT : t.kind == W_SPAN ? sideOf(t.x0, t.x1) : S_CENTRE;
    if (g_edges) applySide(state, g_dlSide);
}

void afterVertex(RT64::State *state, uint32_t w1, uint32_t first) {
    if (g_k <= 1.0) return;
    const uint32_t a = state->rsp->fromSegmented(w1) & 0x7FFFFF;
    const uint32_t n = workload(state).drawData.vertexCount() - first;
    const double f = g_k + kMargin;
    if (g_mode == W_TICKER) {
        if (widen(state, first, n, f, false)) g_tickerVerts += n;
    } else if (g_mode == W_STARS) {
        /* the stars: out to the wider picture's edges (no margin: they are
         * points, not something that must reach the edge) */
        if (n % 4 == 0 && widen(state, first, n, g_k, false, true)) g_starQuads += n / 4;
    } else if (a == kBackdropVtx0 || a == kBackdropVtx1) {
        if (n == 4 && widen(state, first, n, f, true)) g_sky++;
    } else if (a == kFadeVtx0 || a == kFadeVtx1) {
        if (n == 4 && widen(state, first, n, f, false)) g_fade++;
    } else if (a >= kWindowVtx && a < kWindowVtxEnd) {
        /* the window's square spans +-1000 in its model space: its width on
         * screen over the screen's (clip x -1..1, orthographic: w = 1) */
        const hlslpp::float4x4 &m = state->rsp->modelViewProjMatrix;
        const double w = std::fabs(double(float(m[3][3])));
        if (w < 1e-9) return;
        const double q = 2000.0 * std::fabs(double(float(m[0][0]))) / w / 2.0;
        const double t = std::min(1.0, std::max(0.0, (q - 0.8) / 0.2));
        if (t > 0 && widen(state, first, n, 1.0 + (f - 1.0) * t, false)) g_window++;
    }
}

bool g_bgImage;   /* the last G_SETTIMG was the front end's background picture */

bool command(RT64::State *state, RT64::DisplayList **dl, const GbiOriginal *originals) {
    if (g_k <= 1.0) return false;
    const uint32_t w0 = (*dl)->w0, w1 = (*dl)->w1;
    const uint8_t op = uint8_t(w0 >> 24);
    if (op == 0x04 && g_edges && g_mode == W_TEXT_SIDES) {
        /* G_VTX of a HUD text flush: the side of the string these glyph
         * vertices belong to (the flush sorts the glyphs of all strings) */
        const uint32_t a = state->rsp->fromSegmented(w1) & 0xFFFFFF;
        Side s = S_CENTRE;
        for (auto r = g_text.rbegin(); r != g_text.rend(); ++r)   /* the newest first */
            if (a >= r->first && a < r->end) {
                s = r->side;
                break;
            }
        applySide(state, s);
        return false;
    }
    if (op == 0xFD) {   /* G_SETTIMG: RGBA16, 320 wide, at gBgPicture? */
        const uint32_t pic = *reinterpret_cast<volatile const uint32_t *>(uintptr_t(kBgPicturePtr));
        g_bgImage = (w0 & 0xFFF) == 319 && ((w0 >> 19) & 0x1F) == ((G_IM_FMT_RGBA << 2) | G_IM_SIZ_16b) &&
                    (state->rsp->fromSegmented(w1) & 0x7FFFFF) == (pic & 0x7FFFFF) && (pic & 0x7FFFFF) != 0;
        return false;
    }
    if (op == 0xE4 && g_bgImage) {   /* G_TEXRECT of the background picture */
        RT64::RDP &rdp = *state->rdp;
        const uint8_t before = rdp.extended.global.rectAspect;
        rdp.setRectAspect(G_EX_ASPECT_STRETCH);
        originals[op](state, dl);
        rdp.setRectAspect(before);
        g_bgRects++;
        return true;
    }
    return false;
}

}  // namespace

void wideSetFactor(double k) {
    if (!(k > 1.0 + 1e-6)) k = 1.0;
    if (k == g_k) return;
    const bool was = g_k > 1.0;
    g_k = k;
    if ((k > 1.0) != was) {
        interpSetWideHooks(k > 1.0 ? command : nullptr, k > 1.0 ? afterVertex : nullptr, k > 1.0 ? address : nullptr);
        host_wtag_hook = k > 1.0 ? wtagHook : nullptr;
        g_wtags.clear();
        g_text.clear();
    }
    g_bgImage = false;
    if (k > 1.0)
        host_log("live: widescreen: full-screen 2D (sky, fades, windows, ticker band) widened %.3f times; HUD %s\n",
                 k + kMargin, g_edges ? "at the edges" : "centred");
}

void wideSetHudEdges(bool on) {
    if (on == g_edges) return;
    g_edges = on;
    if (g_k > 1.0) host_log("live: widescreen: HUD %s\n", on ? "at the edges" : "centred");
}

double wideFactor() {
    return g_k;
}

void wideAfterVertex(RT64::State *state, uint32_t w1, uint32_t first) {
    afterVertex(state, w1, first);
}

void wideBeforeTask() {
    /* RT64 starts each task with its extended state at the defaults (no origins) */
    g_mode = W_NONE;
    g_dlSide = S_CENTRE;
    g_applied = S_CENTRE;
}

void wideAfterTask(bool frameDl) {
    /* the tags of a frame are dropped once its display list was read (the
     * game's frame list, or a front-end list that met them); other tasks in
     * between (the shadow and motion-blur pictures) leave them */
    if (frameDl || g_matched) {
        g_wtags.clear();
        g_text.clear();
    }
    g_matched = false;
}

void wideSummary() {
    if (g_sky + g_fade + g_window + g_bgRects + g_tickerVerts + g_starQuads != 0)
        host_log("live: widescreen: widened %u sky quads, %u fades, %u window panels, %u ticker band vertices, "
                 "%u star quads; %u background rectangles stretched\n",
                 g_sky, g_fade, g_window, g_tickerVerts, g_starQuads, g_bgRects);
    if (g_sideCmds != 0)
        host_log("live: widescreen HUD at the edges: %u side changes, %u text strings placed\n", g_sideCmds,
                 g_textStrings);
}

}  // namespace live
