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
int g_tab;
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

void controlsTab(Config &c) {
    ImGui::TextWrapped("%s", helpLine(c).c_str());
    ImGui::Spacing();
    if (ImGui::BeginTable("bindings", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("N64");
        ImGui::TableSetupColumn("Keyboard");
        ImGui::TableSetupColumn("Controller");
        ImGui::TableHeadersRow();
        for (int a = 0; a < A_COUNT; a++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(kActionLabels[a]);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(keyNames(c.keys[a]).c_str());
            ImGui::TableNextColumn();
            if (a == A_MENU) ImGui::TextUnformatted(menuPadNames(c.menuPad).c_str());
            else if (a >= A_STICK_UP && a <= A_STICK_RIGHT) ImGui::TextUnformatted(c.padStick < 0 ? "-" : c.padStick ? "right stick" : "left stick");
            else ImGui::TextUnformatted(padNames(c.pad[a]).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::TextDisabled("Bindings are set in bc.ini ([keyboard], [controller]).");
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
            if (ImGui::BeginTabItem("Video")) {
                videoTab(c);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sound")) {
                audioTab(c);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Controls")) {
                controlsTab(c);
                ImGui::EndTabItem();
            }
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

bool menuEvent(SDL_Event &e) {
    if (!g_open || !g_inspector) return false;
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
    ImGui_ImplSDL2_NewFrame();
    if (g_inspector->graphicsAPI == RT64::UserConfiguration::GraphicsAPI::Vulkan) ImGui_ImplVulkan_NewFrame();
    else ImGui_ImplDX12_NewFrame();
    int ww = 0, wh = 0, pw = 0, ph = 0;
    SDL_GetWindowSize(g_env.window, &ww, &wh);
    SDL_GetWindowSizeInPixels(g_env.window, &pw, &ph);
    ImGuiIO &io = ImGui::GetIO();
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
    if (!g_open || !configSet(*g_env.cfg, spec)) return false;
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
