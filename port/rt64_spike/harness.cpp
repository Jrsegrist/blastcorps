// RT64 feasibility harness (stage 0 spike, Blast Corps Windows port).
//
// Drives RT64 the way a native port would: an 8 MB RDRAM image, the game's own
// F3D ucode text/data (read at runtime from files extracted from the user's ROM,
// never committed), a hand-built old-F3D display list, and VI registers for the
// game's 320x240 RGBA16 mode. Renders a few frames into a Win32 window, then reads
// the colour image back from RDRAM to check the CPU-visible framebuffer.
//
// RDRAM layout expected by RT64 (same as emulators / N64Recomp): every 32-bit word
// is stored as a native (little-endian) u32 holding the big-endian word's value, so
// an N64 byte at address A lives at RDRAM[A ^ 3] and a halfword at RDRAM[A ^ 2].
//
// Usage: rt64_harness.exe <ucode dir> [d3d12|vulkan] [frames]

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

#include "hle/rt64_application.h"

static const uint32_t RDRAM_SIZE = 8 * 1024 * 1024;
static uint8_t *RDRAM;
static uint8_t DMEM[0x1000], IMEM[0x1000], HEADER[0x40];
static uint32_t MI_INTR_REG, DPC_START_REG, DPC_END_REG, DPC_CURRENT_REG, DPC_STATUS_REG, DPC_CLOCK_REG,
    DPC_BUFBUSY_REG, DPC_PIPEBUSY_REG, DPC_TMEM_REG;
static uint32_t VI_STATUS_REG, VI_ORIGIN_REG, VI_WIDTH_REG, VI_INTR_REG, VI_V_CURRENT_LINE_REG, VI_TIMING_REG,
    VI_V_SYNC_REG, VI_H_SYNC_REG, VI_LEAP_REG, VI_H_START_REG, VI_V_START_REG, VI_V_BURST_REG, VI_X_SCALE_REG,
    VI_Y_SCALE_REG;
static int spInterrupts, dpInterrupts;
static void checkInterrupts() {
    if (MI_INTR_REG & 0x01) spInterrupts++;
    if (MI_INTR_REG & 0x20) dpInterrupts++;
    MI_INTR_REG = 0;
}

// Word-level RDRAM helpers (address = N64 physical address).
static void w32(uint32_t a, uint32_t v) { memcpy(&RDRAM[a], &v, 4); }
static uint16_t r16(uint32_t a) { uint16_t v; memcpy(&v, &RDRAM[a ^ 2], 2); return v; }

static bool loadUcode(const char *path, uint32_t addr, uint32_t maxLen) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); return false; }
    std::vector<uint8_t> b(maxLen);
    size_t n = fread(b.data(), 1, maxLen, f);
    fclose(f);
    for (size_t i = 0; i + 3 < n; i += 4) // big-endian file -> native words
        w32(addr + (uint32_t)i, (uint32_t(b[i]) << 24) | (b[i + 1] << 16) | (b[i + 2] << 8) | b[i + 3]);
    return true;
}

// libultra fixed-point Mtx: 8 words of integer halves, then 8 words of fraction halves.
static void writeMtx(uint32_t a, const float m[4][4]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 2; c++) {
            int32_t e0 = (int32_t)lrintf(m[r][c * 2] * 65536.0f), e1 = (int32_t)lrintf(m[r][c * 2 + 1] * 65536.0f);
            w32(a + (r * 2 + c) * 4, (uint32_t(e0) & 0xFFFF0000u) | (uint32_t(e1) >> 16));
            w32(a + 32 + (r * 2 + c) * 4, (uint32_t(e0) << 16) | (uint32_t(e1) & 0xFFFFu));
        }
}

static void writeVtx(uint32_t a, int16_t x, int16_t y, int16_t z, uint8_t r, uint8_t g, uint8_t b) {
    w32(a + 0, (uint32_t(uint16_t(x)) << 16) | uint16_t(y));
    w32(a + 4, (uint32_t(uint16_t(z)) << 16) | 0);
    w32(a + 8, 0);
    w32(a + 12, (uint32_t(r) << 24) | (g << 16) | (b << 8) | 0xFF);
}

static uint32_t dlCursor;
static void gfx(uint32_t w0, uint32_t w1) { w32(dlCursor, w0); w32(dlCursor + 4, w1); dlCursor += 8; }

enum : uint32_t {
    FB0 = 0x000400, ZBUF = 0x0F0000, UCODE_TEXT = 0x100000, UCODE_DATA = 0x102000,
    DL = 0x200000, VP = 0x201000, MTX_P = 0x201040, MTX_M = 0x201080, VTX = 0x201100,
};

enum : uint32_t {
    DL_SHADOW = 0x202000, VP64 = 0x203000, MTX_P64 = 0x203040, VTX64 = 0x203100, CIMG = 0x300000,
    VTX_QUAD = 0x203200,
};

// Mirrors the shadow pass in 13A70.c func_80258544: a separate task that renders a silhouette
// with prim colour into a 64x64 CI8 colour image, which the main pass then loads as an IA8 texture.
static void buildShadowDisplayList(int frame) {
    w32(VP64 + 0, (128u << 16) | 128u); w32(VP64 + 4, (511u << 16) | 0);
    w32(VP64 + 8, (128u << 16) | 128u); w32(VP64 + 12, (511u << 16) | 0);
    const float proj[4][4] = {{1 / 32.f, 0, 0, 0}, {0, 1 / 32.f, 0, 0}, {0, 0, 1 / 128.f, 0}, {0, 0, 0, 1}};
    writeMtx(MTX_P64, proj);
    writeVtx(VTX64 + 0, -24, -20, 0, 255, 255, 255);
    writeVtx(VTX64 + 16, 24, -20, 0, 255, 255, 255);
    writeVtx(VTX64 + 32, int16_t(-24 + (frame % 48)), 26, 0, 255, 255, 255);

    dlCursor = DL_SHADOW;
    gfx(0x03000000 | (0x80 << 16) | 16, VP64);                   // gSPViewport
    gfx(0xB6000000, 0xFFFFFFFF);                                 // gSPClearGeometryMode
    gfx(0xBB000000, 0xFFFFFFFF);                                 // gSPTexture(0xFFFF, 0xFFFF, 0, 0, G_OFF)
    gfx(0xE7000000, 0);
    gfx(0xED000000, (63 * 4 << 12) | (63 * 4));                  // gDPSetScissor 0,0,63,63 (as the game)
    gfx(0xFA000000, 0xFFFFFFFF);                                 // gDPSetPrimColor 255,255,255,255
    gfx(0xBA000000 | (20 << 8) | 2, 3u << 20);                   // G_CYC_FILL
    gfx(0xFF000000 | (2 << 21) | (1 << 19) | (64 - 1), CIMG);    // gDPSetColorImage(G_IM_FMT_CI, G_IM_SIZ_8b, 64)
    gfx(0xF7000000, 0x00010001);                                 // gDPSetFillColor(0x00010001)
    gfx(0xF6000000 | (63 << 14) | (63 << 2), 0);                 // gDPFillRectangle 0,0,63,63
    gfx(0xE7000000, 0);
    gfx(0xBA000000 | (20 << 8) | 2, 0);                          // G_CYC_1CYCLE
    gfx(0xB9000000 | (3 << 8) | 29, 0x0F0A0000);                 // render mode GBL_c1/c2(CLR_IN,0,CLR_IN,1)
    gfx(0xFCFFFFFF, 0xFFFDF6FB);                                 // G_CC_PRIMITIVE x2
    gfx(0x01000000 | (3 << 16) | 64, MTX_P64);                   // projection
    gfx(0x01000000 | (2 << 16) | 64, MTX_M);                     // modelview (identity part rotates; fine)
    gfx(0x04000000 | (((3 - 1) << 4 | 0) << 16) | (16 * 3), VTX64);
    gfx(0xBF000000, (0 << 16) | (10 << 8) | 20);
    gfx(0xB8000000, 0);                                          // no full sync, like the game's shadow task
}

static void buildDisplayList(int frame) {
    // Viewport: vscale = vtrans = (320*2, 240*2, 511, 0) in 1/4 pixel units.
    w32(VP + 0, (640u << 16) | 480u); w32(VP + 4, (511u << 16) | 0);
    w32(VP + 8, (640u << 16) | 480u); w32(VP + 12, (511u << 16) | 0);
    const float proj[4][4] = {{1 / 160.f, 0, 0, 0}, {0, 1 / 120.f, 0, 0}, {0, 0, 1 / 128.f, 0}, {0, 0, 0, 1}};
    const float ang = frame * 0.05f, cs = cosf(ang), sn = sinf(ang);
    const float model[4][4] = {{cs, sn, 0, 0}, {-sn, cs, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    writeMtx(MTX_P, proj);
    writeMtx(MTX_M, model);
    writeVtx(VTX + 0, -100, -80, 0, 255, 0, 0);
    writeVtx(VTX + 16, 100, -80, 0, 0, 255, 0);
    writeVtx(VTX + 32, 0, 90, 0, 0, 0, 255);

    dlCursor = DL;
    gfx(0xE7000000, 0);                                          // gDPPipeSync
    gfx(0xFF000000 | (0 << 21) | (2 << 19) | (320 - 1), FB0);    // gDPSetColorImage RGBA16 w=320
    gfx(0xED000000, (320 * 4 << 12) | (240 * 4));                // gDPSetScissor 0,0,320,240
    gfx(0xBA000000 | (20 << 8) | 2, 3u << 20);                   // gDPSetCycleType(G_CYCLE_FILL) (old F3D SETOTHERMODE_H)
    const uint16_t blue = (2 << 11) | (4 << 6) | (12 << 1) | 1;
    gfx(0xF7000000, (uint32_t(blue) << 16) | blue);              // gDPSetFillColor
    gfx(0xF6000000 | (319 << 14) | (239 << 2), 0);               // gDPFillRectangle 0,0,319,239
    gfx(0xE7000000, 0);
    gfx(0xBA000000 | (20 << 8) | 2, 0);                          // G_CYCLE_1CYCLE
    gfx(0xB9000000 | (3 << 8) | 29, 0x0F0A4000);                 // gDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2)
    gfx(0xFCFFFFFF, 0xFFFE793C);                                 // gDPSetCombineMode(G_CC_SHADE, G_CC_SHADE)
    gfx(0xB6000000, 0xFFFFFFFF);                                 // gSPClearGeometryMode(all)
    gfx(0xB7000000, 0x00000204);                                 // gSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH)
    gfx(0x03000000 | (0x80 << 16) | 16, VP);                     // gSPViewport
    gfx(0x01000000 | (3 << 16) | 64, MTX_P);                     // gSPMatrix PROJECTION|LOAD|NOPUSH
    gfx(0x01000000 | (2 << 16) | 64, MTX_M);                     // gSPMatrix MODELVIEW|LOAD|NOPUSH
    gfx(0x04000000 | (((3 - 1) << 4 | 0) << 16) | (16 * 3), VTX); // gSPVertex(v, 3, 0)
    gfx(0xBF000000, (0 << 16) | (10 << 8) | 20);                 // gSP1Triangle(0, 1, 2, 0)

    // Textured quad sampling the CI8 shadow image as IA8 (as 00000.c:2926 / 13A70.c:269 do).
    const float ident[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    writeMtx(MTX_M + 0x40, ident);
    auto qv = [](uint32_t a, int16_t x, int16_t y, int16_t s, int16_t t) {
        w32(a + 0, (uint32_t(uint16_t(x)) << 16) | uint16_t(y)); w32(a + 4, 0);
        w32(a + 8, (uint32_t(uint16_t(s)) << 16) | uint16_t(t)); w32(a + 12, 0xFFFFFFFF);
    };
    qv(VTX_QUAD + 0, 40, 10, 0, 0x800); qv(VTX_QUAD + 16, 150, 10, 0x800, 0x800);
    qv(VTX_QUAD + 32, 150, 110, 0x800, 0); qv(VTX_QUAD + 48, 40, 110, 0, 0);
    gfx(0xE7000000, 0);
    gfx(0x01000000 | (2 << 16) | 64, MTX_M + 0x40);              // modelview identity
    gfx(0xB6000000, 0xFFFFFFFF);
    gfx(0xBB000000 | 1, 0xFFFFFFFF);                             // gSPTexture(0xFFFF, 0xFFFF, 0, 0, G_ON)
    gfx(0xFCFFFFFF, 0xFFFCF279);                                 // G_CC_DECALRGBA
    // gDPLoadTextureBlock(CIMG, G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0, clamp, clamp, 0, 0, 0, 0)
    gfx(0xFD000000 | (3 << 21) | (2 << 19), CIMG);
    gfx(0xF5000000 | (3 << 21) | (2 << 19), (7u << 24) | (2 << 18) | (2 << 8));
    gfx(0xE6000000, 0);
    gfx(0xF3000000, (7u << 24) | (2047 << 12) | 256);
    gfx(0xE7000000, 0);
    gfx(0xF5000000 | (3 << 21) | (1 << 19) | (8 << 9), (2 << 18) | (2 << 8));
    gfx(0xF2000000, ((63 << 2) << 12) | (63 << 2));
    gfx(0x04000000 | (((4 - 1) << 4 | 0) << 16) | (16 * 4), VTX_QUAD);
    gfx(0xBF000000, (0 << 16) | (10 << 8) | 20);
    gfx(0xBF000000, (0 << 16) | (20 << 8) | 30);
    gfx(0xE9000000, 0);                                          // gDPFullSync
    gfx(0xB8000000, 0);                                          // gSPEndDisplayList
}

static LRESULT CALLBACK wndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CLOSE) { DestroyWindow(h); return 0; }
    return DefWindowProcW(h, m, w, l);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <ucode dir> [d3d12|vulkan] [frames]\n", argv[0]); return 2; }
    const char *api = argc > 2 ? argv[2] : "d3d12";
    const int frames = argc > 3 ? atoi(argv[3]) : 90;
    printf("pointer size %u, harness built for %s\n", (unsigned)sizeof(void *), sizeof(void *) == 4 ? "i686" : "x86_64");

    RDRAM = (uint8_t *)calloc(RDRAM_SIZE, 1);
    char path[1024];
    snprintf(path, sizeof path, "%s/hd_text.bin", argv[1]);
    if (!loadUcode(path, UCODE_TEXT, 0x1800)) return 1;
    snprintf(path, sizeof path, "%s/hd_data.bin", argv[1]);
    if (!loadUcode(path, UCODE_DATA, 0x800)) return 1;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = wndProc; wc.hInstance = GetModuleHandleW(nullptr); wc.lpszClassName = L"rt64harness";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowW(L"rt64harness", L"RT64 harness (Blast Corps F3D)", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wc.hInstance, nullptr);

    // NTSC LAN1-style VI for 320x240 16-bit.
    VI_STATUS_REG = 0x0000311E; VI_ORIGIN_REG = FB0 + 640; VI_WIDTH_REG = 320; VI_INTR_REG = 2;
    VI_TIMING_REG = 0x03E52239; VI_V_SYNC_REG = 0x20D; VI_H_SYNC_REG = 0x00000C15; VI_LEAP_REG = 0x0C150C15;
    VI_H_START_REG = 0x006C02EC; VI_V_START_REG = 0x002501FF; VI_V_BURST_REG = 0x000E0204;
    VI_X_SCALE_REG = 0x200; VI_Y_SCALE_REG = 0x400;

    RT64::Application::Core core = {};
    core.window = hwnd;
    core.HEADER = HEADER; core.RDRAM = RDRAM; core.DMEM = DMEM; core.IMEM = IMEM;
    core.MI_INTR_REG = &MI_INTR_REG;
    core.DPC_START_REG = &DPC_START_REG; core.DPC_END_REG = &DPC_END_REG; core.DPC_CURRENT_REG = &DPC_CURRENT_REG;
    core.DPC_STATUS_REG = &DPC_STATUS_REG; core.DPC_CLOCK_REG = &DPC_CLOCK_REG; core.DPC_BUFBUSY_REG = &DPC_BUFBUSY_REG;
    core.DPC_PIPEBUSY_REG = &DPC_PIPEBUSY_REG; core.DPC_TMEM_REG = &DPC_TMEM_REG;
    core.VI_STATUS_REG = &VI_STATUS_REG; core.VI_ORIGIN_REG = &VI_ORIGIN_REG; core.VI_WIDTH_REG = &VI_WIDTH_REG;
    core.VI_INTR_REG = &VI_INTR_REG; core.VI_V_CURRENT_LINE_REG = &VI_V_CURRENT_LINE_REG; core.VI_TIMING_REG = &VI_TIMING_REG;
    core.VI_V_SYNC_REG = &VI_V_SYNC_REG; core.VI_H_SYNC_REG = &VI_H_SYNC_REG; core.VI_LEAP_REG = &VI_LEAP_REG;
    core.VI_H_START_REG = &VI_H_START_REG; core.VI_V_START_REG = &VI_V_START_REG; core.VI_V_BURST_REG = &VI_V_BURST_REG;
    core.VI_X_SCALE_REG = &VI_X_SCALE_REG; core.VI_Y_SCALE_REG = &VI_Y_SCALE_REG;
    core.checkInterrupts = checkInterrupts;

    RT64::ApplicationConfiguration appConfig;
    appConfig.useConfigurationFile = false;
    appConfig.detectDataPath = false;
    RT64::Application app(core, appConfig);
    app.userConfig.graphicsAPI = (strcmp(api, "vulkan") == 0) ? RT64::UserConfiguration::GraphicsAPI::Vulkan
                                                               : RT64::UserConfiguration::GraphicsAPI::D3D12;
    RT64::Application::SetupResult res = app.setup(GetCurrentThreadId());
    printf("setup result %d (0 = success), API %s\n", (int)res, api);
    if (res != RT64::Application::SetupResult::Success || !app.device) return 1;

    for (int f = 0; f < frames; f++) {
        buildShadowDisplayList(f);
        app.interpreter->loadUCodeGBI(UCODE_TEXT, UCODE_DATA, true);
        app.processDisplayLists(RDRAM, DL_SHADOW, 0, true);
        buildDisplayList(f);
        app.interpreter->loadUCodeGBI(UCODE_TEXT, UCODE_DATA, true);
        if (f == 0) printf("GBI detected: %s\n", app.interpreter->hleGBI ? "yes (F3D handler)" : "NO (unsupported ucode)");
        if (!app.interpreter->hleGBI) return 1;
        app.processDisplayLists(RDRAM, DL, 0, true);
        app.updateScreen();
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
        Sleep(16);
    }

    // CPU view of the colour image after RT64's native-resolution sync.
    auto px = [](int x, int y) { return r16(FB0 + (y * 320 + x) * 2); };
    printf("RDRAM framebuffer: corner %04X centre %04X (fill colour %04X); SP intr %d DP intr %d\n",
        px(2, 2), px(160, 130), (2 << 11) | (4 << 6) | (12 << 1) | 1, spInterrupts, dpInterrupts);
    int nonFill = 0;
    for (int y = 0; y < 240; y++) for (int x = 0; x < 320; x++) nonFill += px(x, y) != ((2 << 11) | (4 << 6) | (12 << 1) | 1);
    printf("pixels differing from the fill colour: %d of %d\n", nonFill, 320 * 240);

    // CI8 shadow image as seen by the CPU, and the main framebuffer, dumped for inspection.
    int ci0 = 0, ci1 = 0, ciOther = 0;
    for (int i = 0; i < 64 * 64; i++) { uint8_t v = RDRAM[(CIMG + i) ^ 3]; if (v == 0) ci0++; else if (v == 1) ci1++; else ciOther++; }
    printf("CI8 image bytes: 0x00 %d, 0x01 %d, other %d (sample row 32: ", ci0, ci1, ciOther);
    for (int x = 0; x < 64; x += 8) printf("%02X ", RDRAM[(CIMG + 32 * 64 + x) ^ 3]);
    printf(")\n");
    int quadLit = 0;
    for (int y = 0; y < 240; y++) for (int x = 0; x < 320; x++) {
        uint16_t p = px(x, y); if (x >= 200 && x < 310 && y >= 10 && y < 120 && (p >> 11) > 20 && ((p >> 6) & 31) > 20 && ((p >> 1) & 31) > 20) quadLit++;
    }
    printf("bright pixels inside the shadow-textured quad: %d\n", quadLit);
    snprintf(path, sizeof path, "fb_%s_%u.ppm", api, (unsigned)(sizeof(void *) * 8));
    if (FILE *o = fopen(path, "wb")) {
        fprintf(o, "P6 320 240 255\n");
        for (int y = 0; y < 240; y++) for (int x = 0; x < 320; x++) {
            uint16_t p = px(x, y); uint8_t rgb[3] = {uint8_t((p >> 11) << 3), uint8_t(((p >> 6) & 31) << 3), uint8_t(((p >> 1) & 31) << 3)};
            fwrite(rgb, 1, 3, o);
        }
        fclose(o);
        printf("wrote %s\n", path);
    }

    // The front-end ucode (D_80207090/D_80210690) if the files are present.
    snprintf(path, sizeof path, "%s/fe_text.bin", argv[1]);
    if (loadUcode(path, UCODE_TEXT + 0x10000, 0xFB0)) {
        snprintf(path, sizeof path, "%s/fe_data.bin", argv[1]);
        loadUcode(path, UCODE_DATA + 0x10000, 0x800);
        app.interpreter->loadUCodeGBI(UCODE_TEXT + 0x10000, UCODE_DATA + 0x10000, true);
        // RT64 knows this ucode by hash ("L3D Blast Corps") but maps it to GBIUCode::Unknown, which gets
        // only the RDP command table: no G_VTX / G_LINE3D handlers.
        const RT64::GBI *g = app.interpreter->hleGBI;
        printf("front-end ucode: %s (GBIUCode %u; G_VTX handler %s)\n", g ? "identified" : "not identified",
            g ? (unsigned)g->ucode : 0u, (g && g->map[0x04]) ? "present" : "absent");
    }

    app.end();
    DestroyWindow(hwnd);
    return 0;
}
