/* bc.exe's player-facing setup: config file, bindings, ROM check (see
 * live_setup.h). */
#include <windows.h>
#include <commdlg.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "live_setup.h"

extern "C" {
#include "plat_host.h"
#include "rdram.h"
}

namespace live {

const unsigned short kButtonBits[kButtons] = {
    0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
};
const char *const kActionNames[A_COUNT] = {
    "a", "b", "z", "start", "d_up", "d_down", "d_left", "d_right", "l", "r", "c_up", "c_down", "c_left", "c_right",
    "stick_up", "stick_down", "stick_left", "stick_right", "quit",
};

/* The defaults ARE this text: configDefaults parses it, and a missing
 * bc.ini is created from it. */
static const char kDefaultIni[] =
    "; Blast Corps (native Windows port) settings.\r\n"
    "; Created with the defaults on the first start; edit it while the game is closed.\r\n"
    "; Lines starting with ; are comments. Relative paths are relative to this folder.\r\n"
    "\r\n"
    "[game]\r\n"
    "; Your Blast Corps (USA) (Rev 1) ROM: .z64, .v64 or .n64. Empty: the game asks for it\r\n"
    "; with a file dialog on the next start and remembers the answer here.\r\n"
    "rom =\r\n"
    "; Folder for the saves: the EEPROM (blastcorps.eep, 512 bytes) and the Controller Pak\r\n"
    "; (blastcorps.mpk), in the same formats as mupen64plus. Empty: nothing is saved.\r\n"
    "saves = saves\r\n"
    "; A Controller Pak plugged into controller 1: 1 = yes, 0 = no.\r\n"
    "controller_pak = 1\r\n"
    "\r\n"
    "[video]\r\n"
    "; Graphics API: d3d12 or vulkan.\r\n"
    "api = d3d12\r\n"
    "; Window size: 320x240 times this. The window can be resized; the picture keeps 4:3.\r\n"
    "scale = 3\r\n"
    "; Start in fullscreen: 1 = yes, 0 = in a window. Alt+Enter or F11 switches.\r\n"
    "fullscreen = 0\r\n"
    "; Wait for the display's refresh (no tearing): 1 = yes, 0 = no.\r\n"
    "vsync = 1\r\n"
    "\r\n"
    "[audio]\r\n"
    "; Volume in percent (0-100); - and = change it in the game, M switches the sound off and on.\r\n"
    "volume = 100\r\n"
    "; Start with the sound off: 1 = yes.\r\n"
    "mute = 0\r\n"
    "\r\n"
    "[keyboard]\r\n"
    "; The N64 controller from the keyboard. Values are SDL key names, for example X, Space,\r\n"
    "; Return, Left Shift, Right Ctrl, Keypad 8, Up, F5. Several keys: separate them with\r\n"
    "; commas. Empty: not bound. Fixed keys: F1 help, Alt+Enter/F11 fullscreen, M sound,\r\n"
    "; - and = volume.\r\n"
    "a = X\r\n"
    "b = C\r\n"
    "z = Z, Space\r\n"
    "start = Return\r\n"
    "l = A\r\n"
    "r = S\r\n"
    "c_up = I\r\n"
    "c_down = K\r\n"
    "c_left = J\r\n"
    "c_right = L\r\n"
    "d_up = T\r\n"
    "d_down = G\r\n"
    "d_left = F\r\n"
    "d_right = H\r\n"
    "stick_up = Up\r\n"
    "stick_down = Down\r\n"
    "stick_left = Left\r\n"
    "stick_right = Right\r\n"
    "quit = Escape\r\n"
    "\r\n"
    "[controller]\r\n"
    "; The N64 controller from the first game controller (Xbox/XInput and others SDL knows).\r\n"
    "; Buttons: a b x y back guide start leftstick rightstick leftshoulder rightshoulder\r\n"
    "; dpup dpdown dpleft dpright. Axes pushed one way: leftx- leftx+ lefty- lefty+ rightx-\r\n"
    "; rightx+ righty- righty+ (y- is up), lefttrigger, righttrigger. Several: commas.\r\n"
    "a = a\r\n"
    "b = b, x\r\n"
    "z = lefttrigger, righttrigger\r\n"
    "start = start\r\n"
    "l = leftshoulder\r\n"
    "r = rightshoulder\r\n"
    "c_up = righty-\r\n"
    "c_down = righty+\r\n"
    "c_left = rightx-\r\n"
    "c_right = rightx+\r\n"
    "d_up = dpup\r\n"
    "d_down = dpdown\r\n"
    "d_left = dpleft\r\n"
    "d_right = dpright\r\n"
    "quit =\r\n"
    "; The N64 analog stick comes from: left, right or none.\r\n"
    "stick = left\r\n"
    "; Dead zone of that stick (0-32767).\r\n"
    "deadzone = 7000\r\n";

/* ---- strings and paths ----------------------------------------------------- */
std::string utf8(const std::wstring &w) {
    if (w.empty()) return std::string();
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), int(w.size()), nullptr, 0, nullptr, nullptr);
    std::string s(size_t(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), int(w.size()), &s[0], n, nullptr, nullptr);
    return s;
}

std::wstring wide(const std::string &s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), int(s.size()), nullptr, 0);
    std::wstring w(size_t(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), int(s.size()), &w[0], n);
    return w;
}

std::string exeDir() {
    std::wstring buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, &buf[0], DWORD(buf.size()));
    buf.resize(n);
    size_t slash = buf.find_last_of(L"\\/");
    return utf8(slash == std::wstring::npos ? std::wstring(L".\\") : buf.substr(0, slash + 1));
}

static bool isAbsolute(const std::string &p) {
    return (p.size() >= 2 && p[1] == ':') || (!p.empty() && (p[0] == '\\' || p[0] == '/'));
}

std::string absPath(const Config &c, const std::string &p) {
    return isAbsolute(p) ? p : c.dir + p;
}

static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

static std::string lower(std::string s) {
    for (char &ch : s) ch = char(tolower((unsigned char) ch));
    return s;
}

static std::vector<std::string> splitList(const std::string &v) {
    std::vector<std::string> out;
    size_t p = 0;
    while (p <= v.size()) {
        size_t q = v.find(',', p);
        if (q == std::string::npos) q = v.size();
        std::string item = trim(v.substr(p, q - p));
        if (!item.empty()) out.push_back(item);
        p = q + 1;
    }
    return out;
}

static bool readText(const std::string &path, std::string &out) {
    unsigned size = 0;
    char *p = static_cast<char *>(host_read_file(path.c_str(), &size));
    if (p == nullptr) return false;
    out.assign(p, size);
    free(p);
    if (out.compare(0, 3, "\xEF\xBB\xBF") == 0) out.erase(0, 3);   /* Notepad's BOM */
    return true;
}

/* ---- parsing ------------------------------------------------------------------ */
static bool parseBool(const std::string &v, bool &out) {
    std::string l = lower(v);
    if (l == "1" || l == "yes" || l == "true" || l == "on") return out = true, true;
    if (l == "0" || l == "no" || l == "false" || l == "off") return out = false, true;
    return false;
}

static bool parseInt(const std::string &v, int lo, int hi, int &out) {
    char *end;
    long n = strtol(v.c_str(), &end, 10);
    if (v.empty() || *end != 0 || n < lo || n > hi) return false;
    out = int(n);
    return true;
}

static bool parsePad(const std::string &item, PadInput &in) {
    std::string s = lower(item);
    int dir = 0;
    if (!s.empty() && (s.back() == '+' || s.back() == '-')) {
        dir = s.back() == '+' ? 1 : -1;
        s.pop_back();
    }
    SDL_GameControllerButton b = SDL_GameControllerGetButtonFromString(s.c_str());
    if (b != SDL_CONTROLLER_BUTTON_INVALID && dir == 0) {
        in.button = b;
        return true;
    }
    SDL_GameControllerAxis a = SDL_GameControllerGetAxisFromString(s.c_str());
    if (a == SDL_CONTROLLER_AXIS_INVALID) return false;
    bool trigger = a == SDL_CONTROLLER_AXIS_TRIGGERLEFT || a == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
    if (trigger ? dir == -1 : dir == 0) return false;
    in.axis = a;
    in.dir = trigger ? 1 : dir;
    return true;
}

static void parseText(Config &c, const std::string &text, const std::string &name) {
    std::string section;
    size_t pos = 0;
    int lineNo = 0;
    auto warn = [&](const std::string &msg) {
        c.warnings.push_back(name + " line " + std::to_string(lineNo) + ": " + msg);
    };
    while (pos < text.size()) {
        size_t eol = text.find('\n', pos);
        if (eol == std::string::npos) eol = text.size();
        std::string line = trim(text.substr(pos, eol - pos));
        pos = eol + 1;
        lineNo++;
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        if (line[0] == '[') {
            section = lower(trim(line.substr(1, line.find(']') == std::string::npos ? std::string::npos
                                                                                   : line.find(']') - 1)));
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            warn("\"" + line + "\" is not \"key = value\"");
            continue;
        }
        std::string key = lower(trim(line.substr(0, eq))), v = trim(line.substr(eq + 1));
        bool ok = true;
        int n;
        if (section == "game") {
            if (key == "rom") c.rom = v;
            else if (key == "saves") c.saves = v;
            else if (key == "controller_pak") ok = parseBool(v, c.pak);
            else ok = false;
        } else if (section == "video") {
            if (key == "api") {
                std::string l = lower(v);
                if (l == "d3d12" || l == "directx" || l == "dx12") c.vulkan = false;
                else if (l == "vulkan") c.vulkan = true;
                else ok = false;
            } else if (key == "scale") ok = parseInt(v, 1, 16, n) && (c.scale = unsigned(n), true);
            else if (key == "fullscreen") ok = parseBool(v, c.fullscreen);
            else if (key == "vsync") ok = parseBool(v, c.vsync);
            else ok = false;
        } else if (section == "audio") {
            if (key == "volume") ok = parseInt(v, 0, 100, c.volume);
            else if (key == "mute") ok = parseBool(v, c.mute);
            else ok = false;
        } else if (section == "keyboard" || section == "controller") {
            int act = -1;
            for (int i = 0; i < A_COUNT; i++)
                if (key == kActionNames[i]) act = i;
            if (section == "controller" && key == "stick") {
                std::string l = lower(v);
                if (l == "left") c.padStick = 0;
                else if (l == "right") c.padStick = 1;
                else if (l == "none" || l.empty()) c.padStick = -1;
                else ok = false;
            } else if (section == "controller" && key == "deadzone") {
                ok = parseInt(v, 0, 32767, c.deadzone);
            } else if (act < 0) {
                ok = false;
            } else if (section == "keyboard") {
                c.keys[act].clear();
                for (const std::string &item : splitList(v)) {
                    SDL_Scancode sc = SDL_GetScancodeFromName(item.c_str());
                    if (sc == SDL_SCANCODE_UNKNOWN) warn("unknown key name \"" + item + "\" for " + key);
                    else c.keys[act].push_back(sc);
                }
                continue;
            } else {
                if (act >= A_STICK_UP && act <= A_STICK_RIGHT) {
                    ok = false;
                } else {
                    c.pad[act].clear();
                    for (const std::string &item : splitList(v)) {
                        PadInput in;
                        if (parsePad(item, in)) c.pad[act].push_back(in);
                        else warn("unknown controller input \"" + item + "\" for " + key);
                    }
                    continue;
                }
            }
        } else {
            warn("unknown section [" + section + "]");
            continue;
        }
        if (!ok) warn("\"" + line + "\" is not understood (unknown key or bad value; using the default)");
    }
}

void configDefaults(Config &c) {
    std::string dir = c.dir;
    c = Config();
    c.dir = dir;
    parseText(c, kDefaultIni, "defaults");
    c.warnings.clear();
}

bool configLoad(Config &c, const std::string &file) {
    configDefaults(c);
    c.file = file;
    std::string text;
    if (readText(file, text)) {
        parseText(c, text, file);
        return true;
    }
    if (host_write_file(file.c_str(), kDefaultIni, unsigned(sizeof kDefaultIni - 1)) != 0) {
        c.warnings.push_back("can't create " + file + " (settings and the ROM's location won't be remembered)");
        return false;
    }
    host_log("live: created %s with the default settings\n", file.c_str());
    return true;
}

bool configSetRom(Config &c, const std::string &rom) {
    c.rom = rom;
    if (c.file.empty()) return false;
    std::string text, out, section;
    if (!readText(c.file, text)) text = kDefaultIni;
    bool done = false, inGame = false;
    size_t pos = 0;
    const std::string newLine = "rom = " + rom + "\r\n";
    while (pos < text.size()) {
        size_t eol = text.find('\n', pos);
        std::string raw = text.substr(pos, eol == std::string::npos ? std::string::npos : eol + 1 - pos);
        pos = eol == std::string::npos ? text.size() : eol + 1;
        std::string line = trim(raw);
        if (!line.empty() && line[0] == '[') {
            if (inGame && !done) out += newLine, done = true;   /* [game] without rom = */
            inGame = lower(line).rfind("[game]", 0) == 0;
        } else if (inGame && !done && line[0] != ';') {
            size_t eq = line.find('=');
            if (eq != std::string::npos && lower(trim(line.substr(0, eq))) == "rom") {
                out += newLine;
                done = true;
                continue;
            }
        }
        out += raw;
    }
    if (!done) {
        if (inGame) out += (out.empty() || out.back() == '\n' ? "" : "\r\n") + newLine;
        else out += std::string(out.empty() || out.back() == '\n' ? "" : "\r\n") + "\r\n[game]\r\n" + newLine;
    }
    if (host_write_file(c.file.c_str(), out.data(), unsigned(out.size())) != 0) {
        host_log("live: can't write %s\n", c.file.c_str());
        return false;
    }
    return true;
}

/* ---- the ROM ------------------------------------------------------------------- */
/* SHA-1 (FIPS 180-4) */
static void sha1(const uint8_t *data, size_t len, uint8_t out[20]) {
    uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    auto rol = [](uint32_t x, int n) { return (x << n) | (x >> (32 - n)); };
    uint64_t bits = uint64_t(len) * 8;
    size_t total = ((len + 8) / 64 + 1) * 64;
    for (size_t off = 0; off < total; off += 64) {
        uint8_t blk[64];
        for (size_t i = 0; i < 64; i++) {
            size_t k = off + i;
            if (k < len) blk[i] = data[k];
            else if (k == len) blk[i] = 0x80;
            else if (k >= total - 8) blk[i] = uint8_t(bits >> (8 * (total - 1 - k)));
            else blk[i] = 0;
        }
        uint32_t w[80];
        for (int i = 0; i < 16; i++)
            w[i] = uint32_t(blk[i * 4]) << 24 | uint32_t(blk[i * 4 + 1]) << 16 | uint32_t(blk[i * 4 + 2]) << 8 |
                   blk[i * 4 + 3];
        for (int i = 16; i < 80; i++) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) f = (b & c) | (~b & d), k = 0x5A827999u;
            else if (i < 40) f = b ^ c ^ d, k = 0x6ED9EBA1u;
            else if (i < 60) f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDCu;
            else f = b ^ c ^ d, k = 0xCA62C1D6u;
            uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d, d = c, c = rol(b, 30), b = a, a = t;
        }
        h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e;
    }
    for (int i = 0; i < 20; i++) out[i] = uint8_t(h[i / 4] >> (24 - 8 * (i % 4)));
}

struct KnownRom {
    const char *sha1, *name;
};
/* SHA-1 of the .z64 image (the decomp's README / *.sha1 files) */
static const char kSupportedSha1[] = "483f7161aea39de8b45c9fbc70a2c3883c4dea8c";
static const KnownRom kOtherRoms[] = {
    {"185a6ef7ba1adb243278062c81a7d4e119bda58c", "Blast Corps (USA), the first release (v1.0)"},
    {"b147fdbeb661c89107c440b00dc4810508f58636", "Blastdozer (Japan)"},
    {"460212600f8b9f0da95219c4c7330f2e626d9a7e", "Blast Corps (Europe)"},
};

std::string romCheck(const std::string &path) {
    const std::string want =
        "This port runs only Blast Corps (USA) (Rev 1), also known as v1.1 (SHA-1 " + std::string(kSupportedSha1) +
        " as .z64; .v64 and .n64 files of it work too).";
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExW(wide(path).c_str(), GetFileExInfoStandard, &fa))
        return "Can't find the ROM file\n" + path;
    if (fa.nFileSizeHigh != 0 || fa.nFileSizeLow > 0x4000000u)
        return path + "\nis too big to be an N64 ROM.\n\n" + want;
    unsigned size = 0;
    uint8_t *p = static_cast<uint8_t *>(host_read_file(path.c_str(), &size));
    if (p == nullptr) return "Can't read the ROM file\n" + path;
    std::string msg;
    if (size < 0x100 || rom_normalise(p, size) < 0) {
        msg = path + "\nis not an N64 ROM image.\n\n" + want;
    } else {
        uint8_t d[20];
        sha1(p, size, d);
        char hex[41];
        for (int i = 0; i < 20; i++) snprintf(hex + i * 2, 3, "%02x", d[i]);
        if (strcmp(hex, kSupportedSha1) != 0) {
            const char *known = nullptr;
            for (const KnownRom &k : kOtherRoms)
                if (strcmp(hex, k.sha1) == 0) known = k.name;
            char title[21];
            memcpy(title, p + 0x20, 20);
            title[20] = 0;
            for (char &ch : title)
                if (ch != 0 && (ch < 0x20 || ch > 0x7E)) ch = '?';
            std::string t = trim(title);
            if (known != nullptr)
                msg = path + "\nis " + known + ".\n\n" + want;
            else if (lower(t).find("blast") != std::string::npos)
                msg = path + "\nlooks like Blast Corps (\"" + t + "\", version byte " + std::to_string(p[0x3F]) +
                      ") but doesn't match a known good dump (SHA-1 " + hex + "): a modified or bad dump?\n\n" + want;
            else
                msg = path + "\nis not Blast Corps (the ROM's title is \"" + t + "\").\n\n" + want;
        }
    }
    free(p);
    return msg;
}

std::string romDialog() {
    std::wstring buf(32768, L'\0');
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof ofn;
    ofn.lpstrFilter = L"N64 ROMs (*.z64, *.v64, *.n64)\0*.z64;*.v64;*.n64;*.rom;*.bin\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = &buf[0];
    ofn.nMaxFile = DWORD(buf.size());
    ofn.lpstrTitle = L"Blast Corps: choose your Blast Corps (USA) (Rev 1) ROM";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&ofn)) {
        DWORD err = CommDlgExtendedError();
        if (err != 0) host_log("live: file dialog error 0x%lX\n", (unsigned long) err);
        return std::string();
    }
    buf.resize(wcslen(buf.c_str()));
    return utf8(buf);
}

/* ---- help ------------------------------------------------------------------------ */
std::string helpLine(const Config &c) {
    auto key = [&](int a) -> std::string {
        if (c.keys[a].empty()) return "-";
        std::string s;
        for (size_t i = 0; i < c.keys[a].size(); i++) {
            if (i) s += "/";
            s += SDL_GetScancodeName(c.keys[a][i]);
        }
        return s;
    };
    std::string s = "Stick " + key(A_STICK_UP) + key(A_STICK_LEFT) + key(A_STICK_DOWN) + key(A_STICK_RIGHT) +
                    "  A " + key(A_A) + "  B " + key(A_B) + "  Z " + key(A_Z) + "  Start " + key(A_START) +
                    "  L/R " + key(A_L) + "/" + key(A_R) + "  C " + key(A_CUP) + key(A_CLEFT) + key(A_CDOWN) +
                    key(A_CRIGHT) + "  D-pad " + key(A_DUP) + key(A_DLEFT) + key(A_DDOWN) + key(A_DRIGHT) +
                    "  |  M sound  -/= volume  Alt+Enter fullscreen  " + key(A_QUIT) + " quit  (F1 hides)";
    return s;
}

}  // namespace live
