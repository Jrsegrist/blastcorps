/* bc.exe's settings menu (see live_menu.h).
 *
 * Drawing: RT64's Inspector class sets Dear ImGui up for the device and
 * swap chain RT64 created (D3D12 or Vulkan backend, SDL2 input); the menu
 * builds an ImGui frame on the main thread (menuFrame) and RT64's present
 * thread draws it over the game's picture from the render hook
 * (menuDraw).  Nothing of this exists until the menu is first opened, and
 * nothing is drawn while it is closed. */
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <SDL.h>

#include "gui/rt64_inspector.h"
#include "hle/rt64_application.h"
#include "imgui/imgui.h"

#include "live_menu.h"

extern "C" {
#include "plat_host.h"
}

/* the backends' per-frame calls (in RT64's library; their headers need the
 * Vulkan loader's) */
IMGUI_IMPL_API void ImGui_ImplSDL2_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX12_NewFrame();
IMGUI_IMPL_API void ImGui_ImplVulkan_NewFrame();

namespace live {

namespace {

MenuEnv g_env;
std::unique_ptr<RT64::Inspector> g_inspector;
std::atomic<bool> g_visible{false};   /* the present thread draws the menu */
bool g_open;
std::vector<Setting> g_dirty;         /* changed since the menu opened (saved when it closes) */
std::string g_saveNote;               /* the last save's result */
int g_selectTab = -1;                 /* testing: open this tab on the next frame */
float g_fontPx = 16.0f;

void markChanged(const std::string &section, const std::string &key) {
    Setting s{section, key};
    if (std::find(g_dirty.begin(), g_dirty.end(), s) == g_dirty.end()) g_dirty.push_back(s);
    if (g_env.changed) g_env.changed(s);
}

/* the ImGui context, backends and font (the first time the menu opens) */
bool ensureInspector() {
    if (g_inspector) return true;
    RT64::Application *app = g_env.app;
    if (app == nullptr || !app->device || !app->swapChain) return false;
    g_inspector = std::make_unique<RT64::Inspector>(app->device.get(), app->swapChain.get(), app->chosenGraphicsAPI,
                                                    g_env.window);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;   /* no imgui.ini */
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    /* a font sized for the window (the backends build the atlas on the first frame) */
    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(g_env.window, &pw, &ph);
    g_fontPx = std::max(15.0f, std::min(40.0f, float(ph) / 720.0f * 22.0f));
    ImFontConfig fc;
    fc.OversampleH = 2;
    bool font = false;
    wchar_t windir[MAX_PATH];
    if (GetWindowsDirectoryW(windir, MAX_PATH) != 0) {
        char path[MAX_PATH * 3];
        WideCharToMultiByte(CP_UTF8, 0, windir, -1, path, sizeof path, nullptr, nullptr);
        std::string ttf = std::string(path) + "\\Fonts\\segoeui.ttf";
        if (GetFileAttributesA(ttf.c_str()) != INVALID_FILE_ATTRIBUTES)
            font = io.Fonts->AddFontFromFileTTF(ttf.c_str(), g_fontPx, &fc) != nullptr;
    }
    if (!font) {
        fc.SizePixels = g_fontPx;
        io.Fonts->AddFontDefault(&fc);
    }
    ImGuiStyle &st = ImGui::GetStyle();
    st.WindowRounding = 6.0f;
    st.FrameRounding = 4.0f;
    st.ScaleAllSizes(g_fontPx / 15.0f);
    host_log("live: settings menu ready (%s, font %.0f px)\n",
             app->chosenGraphicsAPI == RT64::UserConfiguration::GraphicsAPI::Vulkan ? "Vulkan" : "D3D12", g_fontPx);
    return true;
}

/* ---- widgets ---------------------------------------------------------------- */
bool combo(const char *label, int *value, const std::vector<std::string> &items) {
    bool changed = false;
    const char *preview = (*value >= 0 && *value < int(items.size())) ? items[*value].c_str() : "?";
    if (ImGui::BeginCombo(label, preview)) {
        for (int i = 0; i < int(items.size()); i++) {
            const bool sel = i == *value;
            if (ImGui::Selectable(items[i].c_str(), sel)) {
                changed = *value != i;
                *value = i;
            }
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

void help(const char *text) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text);
}

/* ---- tabs --------------------------------------------------------------------- */
void videoTab(Config &c) {
    bool b = c.fullscreen;
    if (ImGui::Checkbox("Fullscreen (Alt+Enter, F11)", &b)) {
        c.fullscreen = b;
        markChanged("video", "fullscreen");
    }
    int scale = int(c.scale);
    ImGui::BeginDisabled(c.fullscreen);
    if (ImGui::SliderInt("Window size", &scale, 1, 8, "320x240 x %d")) {
        c.scale = unsigned(scale);
        markChanged("video", "scale");
    }
    ImGui::EndDisabled();
    b = c.vsync;
    if (ImGui::Checkbox("Vertical sync (no tearing)", &b)) {
        c.vsync = b;
        markChanged("video", "vsync");
    }
    ImGui::SeparatorText("Picture (optional enhancements)");
    {
        /* internal resolution: 0 window, 1 the N64's, 2..8 */
        std::vector<std::string> items = {"From the window size (default)", "N64 (320x240)"};
        for (int i = 2; i <= 8; i++) items.push_back(std::to_string(i) + "x (" + std::to_string(320 * i) + "x" + std::to_string(240 * i) + ")");
        int r = std::min(c.resolution, 8);
        if (combo("Internal resolution", &r, items)) {
            c.resolution = r;
            markChanged("video", "resolution");
        }
        help("How sharp the 3D world is drawn. \"From the window size\" picks the whole multiple of 240 lines that "
             "covers the window, as before; higher values need a faster graphics card.");
    }
    {
        const unsigned ok = g_env.msaaSupported ? g_env.msaaSupported() : 1;
        static const int counts[] = {0, 2, 4, 8};
        std::vector<std::string> items;
        std::vector<int> map;
        for (int n : counts)
            if (n == 0 || (ok & unsigned(n)) != 0) {
                items.push_back(n == 0 ? "Off (default)" : std::to_string(n) + "x MSAA");
                map.push_back(n);
            }
        int sel = 0;
        for (int i = 0; i < int(map.size()); i++)
            if (map[i] == c.msaa) sel = i;
        ImGui::BeginDisabled(map.size() < 2);
        if (combo("Anti-aliasing", &sel, items)) {
            c.msaa = map[sel];
            markChanged("video", "msaa");
        }
        ImGui::EndDisabled();
        if (map.size() < 2) help("This graphics card (or its driver) has no programmable sample positions, which RT64 needs for MSAA.");
        else help("Smooths the edges of 3D shapes (multisample anti-aliasing).");
    }
    {
        static const double ratios[] = {0, 16.0 / 9.0, 16.0 / 10.0, 21.0 / 9.0, -1};
        std::vector<std::string> items = {"N64 4:3 (default)", "16:9", "16:10", "21:9", "As wide as the window"};
        int sel = -1;
        for (int i = 0; i < 5; i++)
            if (std::abs(c.aspect - ratios[i]) < 1e-6) sel = i;
        if (sel < 0) {
            items.push_back("Custom (" + aspectName(c.aspect) + ")");
            sel = 5;
        }
        if (combo("Aspect ratio", &sel, items) && sel < 5) {
            c.aspect = ratios[sel];
            markChanged("video", "aspect");
        }
        help("Widescreen: the 3D view gets wider (more of the level at the sides, nothing stretched); the HUD and "
             "other 2D pictures keep their 4:3 shape in the middle. The game then also counts the wider view as "
             "visible (which level cells it draws), so it can differ slightly from the N64's.");
    }
    {
        int f = c.filtering;
        if (combo("Picture scaling", &f, {"Anti-aliased pixel scaling (default)", "Bilinear (smooth)", "Nearest (sharp pixels)"})) {
            c.filtering = f;
            markChanged("video", "filtering");
        }
        help("How the finished picture is scaled to the window.");
    }
    ImGui::SeparatorText("Graphics card");
    int api = c.vulkan ? 1 : 0;
    if (combo("Graphics API", &api, {"Direct3D 12", "Vulkan"})) {
        c.vulkan = api == 1;
        markChanged("video", "api");
    }
    const bool running = g_env.app != nullptr &&
                         (g_env.app->chosenGraphicsAPI == RT64::UserConfiguration::GraphicsAPI::Vulkan) != c.vulkan;
    if (running) ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "The graphics API changes the next time the game starts.");
}

void audioTab(Config &c) {
    int v = c.volume;
    if (ImGui::SliderInt("Volume", &v, 0, 100, "%d %%")) {
        c.volume = v;
        markChanged("audio", "volume");
    }
    bool m = c.mute;
    if (ImGui::Checkbox("Sound off (M)", &m)) {
        c.mute = m;
        markChanged("audio", "mute");
    }
}

/* binding capture: the next key (keyboard column) or controller button /
 * axis (controller column) becomes the action's binding */
struct Capture {
    int action = -1;
    bool pad = false;
    int frames = 0;              /* menu frames since it started (events of the click itself are ignored) */
    std::vector<int> combo;      /* the menu's controller buttons, collected while held */
};
Capture g_cap;

bool padBindable(int a) {
    return a != A_STICK_UP && a != A_STICK_DOWN && a != A_STICK_LEFT && a != A_STICK_RIGHT;
}

int g_navCooldown;   /* menu frames without ImGui's gamepad navigation (the button just bound is still held) */

void finishCapture() {
    g_cap = Capture();
    g_navCooldown = 30;
}

/* an event while capturing (the first ones belong to the click that started it) */
void captureEvent(const SDL_Event &e) {
    Config &c = *g_env.cfg;
    if (g_cap.frames < 2) return;
    const int a = g_cap.action;
    if (e.type == SDL_KEYDOWN && !e.key.repeat) {
        const SDL_Scancode sc = e.key.keysym.scancode;
        if (sc == SDL_SCANCODE_ESCAPE) return finishCapture();
        if (sc == SDL_SCANCODE_BACKSPACE || sc == SDL_SCANCODE_DELETE) {   /* clear */
            if (g_cap.pad) {
                if (a == A_MENU) c.menuPad.clear();
                else c.pad[a].clear();
                markChanged("controller", kActionNames[a]);
            } else {
                c.keys[a].clear();
                markChanged("keyboard", kActionNames[a]);
            }
            return finishCapture();
        }
        if (!g_cap.pad) {
            c.keys[a] = {sc};
            markChanged("keyboard", kActionNames[a]);
            return finishCapture();
        }
        return;
    }
    if (!g_cap.pad) return;
    if (a == A_MENU) {   /* a combination: every button held until the first is let go */
        if (e.type == SDL_CONTROLLERBUTTONDOWN &&
            std::find(g_cap.combo.begin(), g_cap.combo.end(), int(e.cbutton.button)) == g_cap.combo.end())
            g_cap.combo.push_back(e.cbutton.button);
        if (e.type == SDL_CONTROLLERBUTTONUP && !g_cap.combo.empty()) {
            c.menuPad = g_cap.combo;
            markChanged("controller", "menu");
            finishCapture();
        }
        return;
    }
    PadInput in;
    if (e.type == SDL_CONTROLLERBUTTONDOWN) {
        in.button = e.cbutton.button;
    } else if (e.type == SDL_CONTROLLERAXISMOTION && std::abs(int(e.caxis.value)) > 20000) {
        const bool trigger = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
        if (trigger && e.caxis.value < 0) return;
        in.axis = e.caxis.axis;
        in.dir = trigger || e.caxis.value > 0 ? 1 : -1;
    } else {
        return;
    }
    c.pad[a] = {in};
    markChanged("controller", kActionNames[a]);
    finishCapture();
}

void resetControls(Config &c) {
    Config d;
    d.dir = c.dir;
    configDefaults(d);
    for (int a = 0; a < A_COUNT; a++) {
        c.keys[a] = d.keys[a];
        markChanged("keyboard", kActionNames[a]);
        if (padBindable(a) && a != A_MENU) {
            c.pad[a] = d.pad[a];
            markChanged("controller", kActionNames[a]);
        }
    }
    c.menuPad = d.menuPad;
    c.padStick = d.padStick;
    c.deadzone = d.deadzone;
    c.sensitivity = d.sensitivity;
    c.padIndex = d.padIndex;
    for (const char *k : {"menu", "stick", "deadzone", "sensitivity", "pad"}) markChanged("controller", k);
}

void controlsTab(Config &c) {
    if (g_cap.action >= 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), g_cap.pad
                               ? (g_cap.action == A_MENU ? "Hold the controller buttons for \"%s\" together, then let go"
                                                         : "Press a controller button, or push a stick or trigger, for \"%s\"")
                               : "Press a key for \"%s\"",
                           kActionLabels[g_cap.action]);
        ImGui::TextDisabled("Esc: cancel   Backspace: no binding");
    } else {
        ImGui::TextDisabled("Click a binding (or select it and press A / Enter), then press the new key or button.");
    }
    const float rowH = ImGui::GetFrameHeightWithSpacing();
    const float tableH = ImGui::GetContentRegionAvail().y - rowH * 5.6f - ImGui::GetTextLineHeightWithSpacing() * 3.2f;
    if (ImGui::BeginTable("bindings", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY,
                          ImVec2(0.0f, std::max(rowH * 4.0f, tableH)))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("N64");
        ImGui::TableSetupColumn("Keyboard");
        ImGui::TableSetupColumn("Controller");
        ImGui::TableHeadersRow();
        for (int a = 0; a < A_COUNT; a++) {
            ImGui::PushID(a);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(kActionLabels[a]);
            ImGui::TableNextColumn();
            std::string k = keyNames(c.keys[a]);
            if (g_cap.action == a && !g_cap.pad) k = "...";
            if (ImGui::Button((k.empty() ? "-" : k).append("##k").c_str(), ImVec2(-FLT_MIN, 0.0f))) g_cap = {a, false, 0, {}};
            ImGui::TableNextColumn();
            if (padBindable(a)) {
                std::string p = a == A_MENU ? menuPadNames(c.menuPad) : padNames(c.pad[a]);
                if (g_cap.action == a && g_cap.pad) p = "...";
                if (ImGui::Button((p.empty() ? "-" : p).append("##p").c_str(), ImVec2(-FLT_MIN, 0.0f))) g_cap = {a, true, 0, {}};
            } else {
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("%s", c.padStick < 0 ? "-" : c.padStick ? "right stick" : "left stick");
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    int stick = c.padStick + 1;
    if (combo("Controller's analog stick", &stick, {"None", "Left stick (default)", "Right stick"})) {
        c.padStick = stick - 1;
        markChanged("controller", "stick");
    }
    int dz = c.deadzone;
    if (ImGui::SliderInt("Dead zone", &dz, 0, 20000)) {
        c.deadzone = dz;
        markChanged("controller", "deadzone");
    }
    int sens = c.sensitivity;
    if (ImGui::SliderInt("Stick sensitivity", &sens, 25, 200, "%d %%")) {
        c.sensitivity = sens;
        markChanged("controller", "sensitivity");
    }
    help("100 %: full tilt is the N64 stick's full range, as before.");
    std::vector<std::string> pads = {"All of them"};
    std::vector<std::string> connected = g_env.padNames ? g_env.padNames() : std::vector<std::string>();
    const int n = std::max(int(connected.size()), c.padIndex + 1);
    for (int i = 0; i < n; i++)
        pads.push_back(std::to_string(i + 1) + ": " + (i < int(connected.size()) ? connected[i] : "(not connected)"));
    int sel = c.padIndex + 1;
    if (combo("Game controller", &sel, pads)) {
        c.padIndex = sel - 1;
        markChanged("controller", "pad");
    }
    help("Which game controller plays (the game has one player). The first one found is the default.");
    if (ImGui::Button("Default controls")) resetControls(c);
}

void build() {
    Config &c = *g_env.cfg;
    ImGuiIO &io = ImGui::GetIO();
    const ImVec2 size(std::min(io.DisplaySize.x * 0.9f, g_fontPx * 42.0f), std::min(io.DisplaySize.y * 0.9f, g_fontPx * 30.0f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);
    if (ImGui::Begin("Blast Corps settings", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoSavedSettings)) {
        if (ImGui::BeginTabBar("tabs")) {
            auto flags = [](int i) { return g_selectTab == i ? ImGuiTabItemFlags_SetSelected : 0; };
            if (ImGui::BeginTabItem("Video", nullptr, flags(0))) {
                videoTab(c);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sound", nullptr, flags(1))) {
                audioTab(c);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Controls", nullptr, flags(2))) {
                controlsTab(c);
                ImGui::EndTabItem();
            }
            g_selectTab = -1;
            ImGui::EndTabBar();
        }
        /* bottom rows */
        const float line = ImGui::GetTextLineHeightWithSpacing();
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - line * 2.0f -
                             ImGui::GetStyle().WindowPadding.y - ImGui::GetStyle().ItemSpacing.y);
        ImGui::Separator();
        if (g_env.status) ImGui::TextDisabled("%s", g_env.status().c_str());
        if (c.file.empty()) ImGui::TextDisabled("Changes last until the game closes (no settings file: --no-config).");
        else if (!g_saveNote.empty()) ImGui::TextDisabled("%s; changes are saved when the menu closes.", g_saveNote.c_str());
        else ImGui::TextDisabled("Changes apply now and are saved to bc.ini when the menu closes.");
        if (ImGui::Button("Back to the game")) menuClose();
        ImGui::SameLine();
        if (ImGui::Button("Quit the game") && g_env.quit) {
            menuClose();
            g_env.quit();
        }
    }
    ImGui::End();
}

}  // namespace

void menuSetup(const MenuEnv &env) {
    g_env = env;
}

bool menuIsOpen() {
    return g_open;
}

void menuOpen() {
    if (g_open || !ensureInspector()) return;
    g_open = true;
    g_dirty.clear();
    SDL_ShowCursor(SDL_ENABLE);
    host_log("live: settings menu opened\n");
}

void menuClose() {
    if (!g_open) return;
    g_open = false;
    g_visible = false;
    g_cap = Capture();
    if (g_env.cfg->fullscreen) SDL_ShowCursor(SDL_DISABLE);
    if (!g_dirty.empty() && !g_env.cfg->file.empty()) {
        const bool ok = configSave(*g_env.cfg, g_dirty);
        g_saveNote = ok ? "Saved to " + g_env.cfg->file : "Could not write " + g_env.cfg->file;
        host_log("live: settings menu closed: %u setting(s) %s %s\n", unsigned(g_dirty.size()),
                 ok ? "saved to" : "NOT saved to", g_env.cfg->file.c_str());
    } else {
        host_log("live: settings menu closed (%u setting(s) changed)\n", unsigned(g_dirty.size()));
    }
    g_dirty.clear();
}

bool menuCapturing() {
    return g_open && g_cap.action >= 0;
}

bool menuEvent(SDL_Event &e) {
    if (!g_open || !g_inspector) return false;
    if (g_cap.action >= 0) {
        captureEvent(e);
        return true;
    }
    /* (the mouse position comes from menuFrame, in pixels) */
    if (e.type != SDL_MOUSEMOTION) g_inspector->handleSdlEvent(&e);
    return true;
}

/* Inspector::newFrame/endFrame, but in the swap chain's pixels: bc.exe is
 * DPI aware with SDL's window sizes in desktop points, and the D3D12
 * backend draws in DisplaySize units (it ignores DisplayFramebufferScale),
 * so the menu works in pixels and the mouse is scaled to them */
void menuFrame() {
    if (!g_open || !g_inspector) return;
    std::lock_guard<std::mutex> lock(g_inspector->frameMutex);
    ImGuiIO &io = ImGui::GetIO();
    if (g_cap.action >= 0) g_cap.frames++;
    if (g_navCooldown > 0) g_navCooldown--;
    if (g_cap.action >= 0 || g_navCooldown > 0) io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    else io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplSDL2_NewFrame();
    if (g_inspector->graphicsAPI == RT64::UserConfiguration::GraphicsAPI::Vulkan) ImGui_ImplVulkan_NewFrame();
    else ImGui_ImplDX12_NewFrame();
    int ww = 0, wh = 0, pw = 0, ph = 0;
    SDL_GetWindowSize(g_env.window, &ww, &wh);
    SDL_GetWindowSizeInPixels(g_env.window, &pw, &ph);
    if (pw > 0 && ph > 0 && ww > 0 && wh > 0) {
        io.DisplaySize = ImVec2(float(pw), float(ph));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        int mx, my;
        if (SDL_GetMouseFocus() == g_env.window) {
            SDL_GetMouseState(&mx, &my);
            io.AddMousePosEvent(float(mx) * pw / ww, float(my) * ph / wh);
        }
    }
    ImGui::NewFrame();
    build();
    ImGui::Render();
    g_visible = g_open;
}

void menuDraw(plume::RenderCommandList *list) {
    if (g_visible && g_inspector) g_inspector->draw(list);
}

bool menuTestSet(const std::string &spec) {
    if (!g_open) return false;
    /* tab=N: show that tab; presskey=ACTION:KEY: bind a key the way a player
     * does (start the capture, then a key press arrives as an SDL event) */
    if (spec.rfind("tab=", 0) == 0) {
        g_selectTab = atoi(spec.c_str() + 4);
        return true;
    }
    if (spec.rfind("presskey=", 0) == 0) {
        const size_t colon = spec.find(':');
        if (colon == std::string::npos) return false;
        const std::string act = spec.substr(9, colon - 9);
        for (int a = 0; a < A_COUNT; a++)
            if (act == kActionNames[a]) {
                SDL_Event e = {};
                e.type = SDL_KEYDOWN;
                e.key.state = SDL_PRESSED;
                e.key.keysym.scancode = SDL_GetScancodeFromName(spec.substr(colon + 1).c_str());
                if (e.key.keysym.scancode == SDL_SCANCODE_UNKNOWN) return false;
                g_cap = {a, false, 2, {}};
                return SDL_PushEvent(&e) == 1;
            }
        return false;
    }
    if (!configSet(*g_env.cfg, spec)) return false;
    const size_t dot = spec.find('.'), eq = spec.find('=');
    std::string key = spec.substr(dot + 1, eq - dot - 1);
    for (char &ch : key) ch = char(tolower((unsigned char) ch));
    std::string section = spec.substr(0, dot);
    for (char &ch : section) ch = char(tolower((unsigned char) ch));
    markChanged(section, key);
    return true;
}

void menuShutdown() {
    g_visible = false;
    g_open = false;
    g_inspector.reset();
}

}  // namespace live
