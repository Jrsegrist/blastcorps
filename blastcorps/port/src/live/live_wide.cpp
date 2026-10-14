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
 * The HUD, text and other 2D are left as they are (pillarboxed).  With
 * video.aspect = original nothing is wrapped or changed.
 *
 * No GPL code: written against RT64's (MIT) API. */
#include <algorithm>
#include <cmath>
#include <cstdint>

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
 * x (an affine function of x along the quad's first two vertices) */
bool widen(RT64::State *state, uint32_t first, uint32_t n, double f, bool texS) {
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
    for (uint32_t i = first; i < first + n; i++) {
        float *p = &d.posFloats[i * 3];
        const double pivot = -(double(p[1]) * m10 + double(p[2]) * m20 + m30) / m00;
        const double x = pivot + (double(p[0]) - pivot) * f;
        if (texS) d.tcFloats[i * 2] = float(double(d.tcFloats[i * 2]) + (x - double(p[0])) * dsdx);
        p[0] = float(x);
        /* what RT64 derived from the position (rt64_rsp.cpp setVertexCommon) */
        const hlslpp::float4 tf = hlslpp::mul(hlslpp::float4(p[0], p[1], p[2], 1.0f), m);
        d.posTransformed[i] = tf;
        d.posScreen[i] = (tf.xyz / hlslpp::float3(tf.w, -tf.w, tf.w)) * vp.scale + vp.translate;
    }
    return true;
}

void afterVertex(RT64::State *state, uint32_t w1, uint32_t first) {
    if (g_k <= 1.0) return;
    const uint32_t a = state->rsp->fromSegmented(w1) & 0x7FFFFF;
    const uint32_t n = workload(state).drawData.vertexCount() - first;
    const double f = g_k + kMargin;
    if (a == kBackdropVtx0 || a == kBackdropVtx1) {
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
    if ((k > 1.0) != was) interpSetWideHooks(k > 1.0 ? command : nullptr, k > 1.0 ? afterVertex : nullptr);
    g_bgImage = false;
    if (k > 1.0) host_log("live: widescreen: full-screen 2D (sky, fades, windows) widened %.3f times\n", k + kMargin);
}

double wideFactor() {
    return g_k;
}

void wideAfterVertex(RT64::State *state, uint32_t w1, uint32_t first) {
    afterVertex(state, w1, first);
}

void wideSummary() {
    if (g_sky + g_fade + g_window + g_bgRects != 0)
        host_log("live: widescreen: widened %u sky quads, %u fades, %u window panels; %u background rectangles "
                 "stretched\n",
                 g_sky, g_fade, g_window, g_bgRects);
}

}  // namespace live
