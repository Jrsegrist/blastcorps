/* bc.exe's level editor, its model (level_edit.cpp): a host-side copy of a
 * level file (big-endian, as the ROM holds it: src/load/level_mod.h), the
 * change list over it with undo and redo, which records were edited, the
 * mod file, and the gizmo's ray math.  Plain C++: no renderer, window or
 * game state, so bc_cameratest (edit_test.cpp) tests it on synthetic level
 * images.  live_viewer.cpp draws it and hands it the user's edits. */
#ifndef LEVEL_EDIT_H
#define LEVEL_EDIT_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "load/level_mod.h"

namespace live {
namespace edit {

struct FieldValue {
    int field;
    long value;
};

/* one change: a set (fields), an add (a copy of `copyOf` at `index`, then fields) or a delete */
LmChange makeSet(int kind, int index, const std::vector<FieldValue> &fields);
LmChange makeAdd(int kind, int index, int copyOf, const std::vector<FieldValue> &fields);
LmChange makeDelete(int kind, int index);
std::string describe(const LmChange &c);

class Editor {
public:
    Editor() = default;
    ~Editor();
    Editor(const Editor &) = delete;
    Editor &operator=(const Editor &) = delete;

    /* a level: the ROM's file and the changes loaded mods already make to it (kept under the editor's own,
     * not undone); false and error() if the file doesn't parse or the mods' changes don't apply */
    bool open(int level, const uint8_t *orig, size_t size, const std::vector<LmChange> &base = {},
              const std::string &baseName = "");
    void close();
    bool isOpen() const { return lv_ != nullptr; }
    int level() const { return level_; }
    const std::string &error() const { return error_; }

    /* the editor's changes: push applies one (and drops the redo list); false and error() if it doesn't apply
     * or breaks one of the game's rules (lm_check): then nothing changes */
    bool push(const LmChange &c);
    bool canUndo() const { return !changes_.empty(); }
    bool canRedo() const { return !redo_.empty(); }
    bool undo();
    bool redo();
    void revertAll();     /* every change of the editor's undone (the mods' stay) */
    const std::vector<LmChange> &changes() const { return changes_; }
    const std::vector<LmChange> &baseChanges() const { return base_; }
    const std::string &baseName() const { return baseName_; }

    /* the records now */
    int count(int kind) const;
    bool get(int kind, int index, int field, long &v) const;
    long get(int kind, int index, const char *field, long fallback = 0) const;
    /* 1: the editor changed or added it, 2: a mod did, 0: as the ROM has it */
    int edited(int kind, int index) const;

    /* the file now (lm_build), its SHA-1, the ROM's SHA-1 */
    const std::vector<uint8_t> &image() const { return image_; }
    std::string imageSha1() const;
    std::string originalSha1() const;
    bool changed() const { return !changes_.empty() || !base_.empty(); }
    /* applied: the image the game was last given (Apply); dirty = the image differs from it */
    void markApplied() { applied_ = imageSha1(); }
    bool dirty() const { return applied_ != imageSha1(); }

    /* the mod file: the mods' changes then the editor's, with both hashes */
    std::string modText(const std::string &name, const std::string &description) const;

private:
    bool rebuild();   /* lv_ from the original, base_ and changes_; image_ and the marks */
    int level_ = -1;
    std::vector<uint8_t> orig_;
    std::vector<LmChange> base_, changes_, redo_;
    std::string baseName_;
    LmLevel *lv_ = nullptr;
    std::vector<uint8_t> image_;
    std::vector<uint8_t> marks_[LM_KINDS];
    std::string error_, applied_;
};

/* ---- the gizmo's math ---- */
struct V3 {
    double x = 0, y = 0, z = 0;
};
/* the ray under window point (sx, sy) of a picture rectangle (rx, ry, rw, rh) drawn with view * proj (row
 * vectors): its origin (on the near side) and unit direction (away from the camera); false if the matrices
 * don't invert */
bool pickRay(const double view[4][4], const double proj[4][4], double rx, double ry, double rw, double rh, double sx,
             double sy, V3 &origin, V3 &dir);
/* where the ray meets the horizontal plane at height y (false: parallel, or behind) */
bool rayPlaneY(const V3 &o, const V3 &d, double y, double &x, double &z);
/* the point of the line through p along axis a (unit) nearest the ray */
bool rayAxis(const V3 &o, const V3 &d, const V3 &p, const V3 &a, double &t);
/* degrees <-> the game's 12-bit heading (0 = +z, 90 = +x) */
int headingFromDegrees(double deg);
double degreesFromHeading(long h);
/* a value snapped to a grid step (0: as it is) */
long snapTo(double v, double step);

}  // namespace edit
}  // namespace live

#endif
