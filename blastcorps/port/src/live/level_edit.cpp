/* bc.exe's level editor, its model (see level_edit.h). */
#include "level_edit.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace live {
namespace edit {

namespace {

LmChange blank(int op, int kind, int index) {
    LmChange c;
    memset(&c, 0, sizeof c);
    c.op = op, c.kind = kind, c.index = index;
    return c;
}

void addFields(LmChange &c, const std::vector<FieldValue> &f) {
    for (const FieldValue &v : f)
        if (c.nfields < LM_MAX_FIELDS) c.field[c.nfields] = v.field, c.value[c.nfields++] = v.value;
}

/* the marks of one change (the kind's array of 0 / 1 / 2, as the records' order moves) */
void markChange(std::vector<uint8_t> &m, const LmChange &c, uint8_t how) {
    if (c.op == LM_OP_SET && c.index >= 0 && size_t(c.index) < m.size()) {
        if (m[c.index] != 1) m[c.index] = how;
    } else if (c.op == LM_OP_ADD && c.index >= 0 && size_t(c.index) <= m.size()) {
        m.insert(m.begin() + c.index, how);
    } else if (c.op == LM_OP_DELETE && c.index >= 0 && size_t(c.index) < m.size()) {
        m.erase(m.begin() + c.index);
    }
}

bool invert(const double a[4][4], double out[4][4]) {
    double m[4][8];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 8; c++) m[r][c] = c < 4 ? a[r][c] : (c - 4 == r ? 1.0 : 0.0);
    for (int c = 0; c < 4; c++) {
        int p = c;
        for (int r = c + 1; r < 4; r++)
            if (std::fabs(m[r][c]) > std::fabs(m[p][c])) p = r;
        if (std::fabs(m[p][c]) < 1e-12) return false;
        if (p != c)
            for (int k = 0; k < 8; k++) std::swap(m[p][k], m[c][k]);
        const double d = m[c][c];
        for (int k = 0; k < 8; k++) m[c][k] /= d;
        for (int r = 0; r < 4; r++) {
            if (r == c) continue;
            const double f = m[r][c];
            for (int k = 0; k < 8; k++) m[r][k] -= f * m[c][k];
        }
    }
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) out[r][c] = m[r][4 + c];
    return true;
}

void mul(const double a[4][4], const double b[4][4], double out[4][4]) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            double s = 0;
            for (int k = 0; k < 4; k++) s += a[r][k] * b[k][c];
            out[r][c] = s;
        }
}

}  // namespace

LmChange makeSet(int kind, int index, const std::vector<FieldValue> &fields) {
    LmChange c = blank(LM_OP_SET, kind, index);
    addFields(c, fields);
    return c;
}

LmChange makeAdd(int kind, int index, int copyOf, const std::vector<FieldValue> &fields) {
    LmChange c = blank(LM_OP_ADD, kind, index);
    c.copy_of = copyOf;
    addFields(c, fields);
    return c;
}

LmChange makeDelete(int kind, int index) {
    return blank(LM_OP_DELETE, kind, index);
}

std::string describe(const LmChange &c) {
    char b[512];
    lm_describe(&c, b, sizeof b);
    return b;
}

Editor::~Editor() {
    close();
}

void Editor::close() {
    lm_free(lv_);
    lv_ = nullptr;
    level_ = -1;
    orig_.clear();
    base_.clear(), changes_.clear(), redo_.clear();
    image_.clear();
    for (auto &m : marks_) m.clear();
    baseName_.clear();
    applied_.clear();
}

bool Editor::open(int level, const uint8_t *orig, size_t size, const std::vector<LmChange> &base,
                  const std::string &baseName) {
    close();
    orig_.assign(orig, orig + size);
    base_ = base;
    baseName_ = baseName;
    level_ = level;
    if (!rebuild()) {
        const std::string e = error_;
        close();
        error_ = e;
        return false;
    }
    applied_ = imageSha1();
    return true;
}

bool Editor::rebuild() {
    char err[512];
    LmLevel *lv = lm_open(orig_.data(), orig_.size(), err, sizeof err);
    if (lv == nullptr) {
        error_ = err;
        return false;
    }
    std::vector<uint8_t> marks[LM_KINDS];
    for (int k = 0; k < LM_KINDS; k++) marks[k].assign(size_t(lm_count(lv, k)), 0);
    for (int pass = 0; pass < 2; pass++) {
        const std::vector<LmChange> &list = pass == 0 ? base_ : changes_;
        for (size_t i = 0; i < list.size(); i++) {
            if (lm_apply(lv, &list[i], err, sizeof err)) {
                error_ = std::string(pass == 0 ? "the mod's change " : "change ") + std::to_string(i) + " (" +
                         describe(list[i]) + "): " + err;
                lm_free(lv);
                return false;
            }
            markChange(marks[list[i].kind], list[i], pass == 0 ? 2 : 1);
        }
    }
    if (lm_check(lv, err, sizeof err)) {
        error_ = err;
        lm_free(lv);
        return false;
    }
    size_t n = 0;
    uint8_t *img = lm_build(lv, &n, nullptr);
    if (img == nullptr) {
        error_ = "out of memory";
        lm_free(lv);
        return false;
    }
    lm_free(lv_);
    lv_ = lv;
    image_.assign(img, img + n);
    free(img);
    for (int k = 0; k < LM_KINDS; k++) marks_[k] = marks[k];
    return true;
}

bool Editor::push(const LmChange &c) {
    if (!isOpen()) return false;
    changes_.push_back(c);
    if (!rebuild()) {
        changes_.pop_back();
        const std::string e = error_;
        rebuild();
        error_ = e;
        return false;
    }
    redo_.clear();
    error_.clear();
    return true;
}

bool Editor::undo() {
    if (changes_.empty()) return false;
    redo_.push_back(changes_.back());
    changes_.pop_back();
    return rebuild();
}

bool Editor::redo() {
    if (redo_.empty()) return false;
    changes_.push_back(redo_.back());
    redo_.pop_back();
    if (!rebuild()) {
        redo_.push_back(changes_.back());
        changes_.pop_back();
        rebuild();
        return false;
    }
    return true;
}

void Editor::revertAll() {
    while (!changes_.empty()) {
        redo_.push_back(changes_.back());
        changes_.pop_back();
    }
    rebuild();
}

int Editor::count(int kind) const {
    return lv_ ? lm_count(lv_, kind) : 0;
}

bool Editor::get(int kind, int index, int field, long &v) const {
    return lv_ && lm_get(lv_, kind, index, field, &v) == 0;
}

long Editor::get(int kind, int index, const char *field, long fallback) const {
    long v;
    return get(kind, index, lm_field_index(kind, field), v) ? v : fallback;
}

int Editor::edited(int kind, int index) const {
    if (kind < 0 || kind >= LM_KINDS || index < 0 || size_t(index) >= marks_[kind].size()) return 0;
    return marks_[kind][index];
}

std::string Editor::imageSha1() const {
    char h[41];
    lm_sha1_hex(image_.data(), image_.size(), h);
    return h;
}

std::string Editor::originalSha1() const {
    char h[41];
    lm_sha1_hex(orig_.data(), orig_.size(), h);
    return h;
}

std::string Editor::modText(const std::string &name, const std::string &description) const {
    LmMod m;
    memset(&m, 0, sizeof m);
    snprintf(m.name, sizeof m.name, "%s", name.c_str());
    snprintf(m.description, sizeof m.description, "%s", description.c_str());
    LmModLevel ml;
    memset(&ml, 0, sizeof ml);
    ml.level = level_;
    snprintf(ml.original_sha1, sizeof ml.original_sha1, "%s", originalSha1().c_str());
    snprintf(ml.edited_sha1, sizeof ml.edited_sha1, "%s", imageSha1().c_str());
    std::vector<LmChange> all = base_;
    all.insert(all.end(), changes_.begin(), changes_.end());
    ml.nchanges = int(all.size());
    ml.changes = all.data();
    m.nlevels = 1;
    m.levels = &ml;
    char *t = lm_mod_write(&m);
    std::string s = t ? t : "";
    free(t);
    return s;
}

/* ---- the gizmo's math ---- */
bool pickRay(const double view[4][4], const double proj[4][4], double rx, double ry, double rw, double rh, double sx,
             double sy, V3 &origin, V3 &dir) {
    double vp[4][4], inv[4][4], vinv[4][4];
    mul(view, proj, vp);
    if (rw <= 0 || rh <= 0 || !invert(vp, inv) || !invert(view, vinv)) return false;
    const double nx = (sx - rx) / rw * 2.0 - 1.0, ny = 1.0 - (sy - ry) / rh * 2.0;
    const double in[4] = {nx, ny, 0.5, 1.0};
    double p[4];
    for (int j = 0; j < 4; j++) p[j] = in[0] * inv[0][j] + in[1] * inv[1][j] + in[2] * inv[2][j] + in[3] * inv[3][j];
    if (std::fabs(p[3]) < 1e-12) return false;
    const V3 eye = {vinv[3][0], vinv[3][1], vinv[3][2]};
    V3 d = {p[0] / p[3] - eye.x, p[1] / p[3] - eye.y, p[2] / p[3] - eye.z};
    const double n = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
    if (!(n > 1e-9)) return false;
    d.x /= n, d.y /= n, d.z /= n;
    /* away from the camera: it looks along view space's -z */
    const V3 fwd = {-vinv[2][0], -vinv[2][1], -vinv[2][2]};
    if (d.x * fwd.x + d.y * fwd.y + d.z * fwd.z < 0) d.x = -d.x, d.y = -d.y, d.z = -d.z;
    origin = eye;
    dir = d;
    return true;
}

bool rayPlaneY(const V3 &o, const V3 &d, double y, double &x, double &z) {
    if (std::fabs(d.y) < 1e-9) return false;
    const double t = (y - o.y) / d.y;
    if (t <= 0) return false;
    x = o.x + d.x * t;
    z = o.z + d.z * t;
    return true;
}

bool rayAxis(const V3 &o, const V3 &d, const V3 &p, const V3 &a, double &t) {
    const V3 w = {p.x - o.x, p.y - o.y, p.z - o.z};
    const double b = a.x * d.x + a.y * d.y + a.z * d.z;
    const double den = 1.0 - b * b;
    if (std::fabs(den) < 1e-6) return false;
    const double dw = d.x * w.x + d.y * w.y + d.z * w.z, aw = a.x * w.x + a.y * w.y + a.z * w.z;
    t = (b * dw - aw) / den;
    return std::isfinite(t);
}

int headingFromDegrees(double deg) {
    double d = std::fmod(deg, 360.0);
    if (d < 0) d += 360.0;
    return int(std::lround(d * 4096.0 / 360.0)) & 0xFFF;
}

double degreesFromHeading(long h) {
    return double(h & 0xFFF) * 360.0 / 4096.0;
}

long snapTo(double v, double step) {
    if (!(step > 0)) return std::lround(v);
    return std::lround(std::round(v / step) * step);
}

}  // namespace edit
}  // namespace live
