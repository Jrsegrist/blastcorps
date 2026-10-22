/* The free orbit camera's math (see orbit_math.h). */
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "orbit_math.h"

namespace live {
namespace orbit {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kDeg = kPi / 180.0;
}  // namespace

Vec3 operator+(const Vec3 &a, const Vec3 &b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(const Vec3 &a, const Vec3 &b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(const Vec3 &a, double k) { return {a.x * k, a.y * k, a.z * k}; }
double dot(const Vec3 &a, const Vec3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(const Vec3 &a, const Vec3 &b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
double length(const Vec3 &a) { return std::sqrt(dot(a, a)); }
Vec3 normalize(const Vec3 &a) {
    const double l = length(a);
    return l > 1e-12 ? a * (1.0 / l) : Vec3{0, 0, 1};
}
Vec3 lerp(const Vec3 &a, const Vec3 &b, double t) { return a + (b - a) * t; }

double wrap180(double d) {
    d = std::fmod(d, 360.0);
    if (d <= -180.0) d += 360.0;
    if (d > 180.0) d -= 360.0;
    return d;
}

double wrap360(double d) {
    d = std::fmod(d, 360.0);
    if (d < 0) d += 360.0;
    if (d >= 360.0) d -= 360.0;
    return d;
}

Vec3 direction(double yawDeg, double pitchDeg) {
    const double cp = std::cos(pitchDeg * kDeg);
    return {std::sin(yawDeg * kDeg) * cp, std::sin(pitchDeg * kDeg), std::cos(yawDeg * kDeg) * cp};
}

void angles(const Vec3 &d, double &yawDeg, double &pitchDeg) {
    const double h = std::sqrt(d.x * d.x + d.z * d.z);
    yawDeg = h > 1e-12 ? wrap360(std::atan2(d.x, d.z) / kDeg) : 0.0;
    pitchDeg = std::atan2(d.y, h) / kDeg;
}

double clearShare(const Vec3 &target, const Vec3 &dir, double dist, double clearance, const HeightFn &height,
                  double from) {
    if (!height || dist <= 0) return 1.0;
    /* march out from the target; the first blocked sample ends the free part,
     * refined by bisection between it and the last free one */
    const int n = std::max(8, std::min(64, int(dist / 4.0)));
    auto blocked = [&](double s) {
        const Vec3 p = target + dir * (dist * s);
        const double h = height(p.x, p.y, p.z);
        return h > -1e30 && p.y < h + clearance;
    };
    double lastFree = from;
    for (int i = 1; i <= n; i++) {
        const double s = from + (1.0 - from) * double(i) / n;
        if (blocked(s)) {
            double lo = lastFree, hi = s;
            for (int k = 0; k < 10; k++) {
                const double mid = 0.5 * (lo + hi);
                if (blocked(mid)) hi = mid;
                else lo = mid;
            }
            return lo;
        }
        lastFree = s;
    }
    return 1.0;
}

void Orbit::reset(bool zoomToo) {
    blend_ = 0.0;
    recentring_ = false;
    idle_ = 1e9;
    ratio_ = 1.0;
    lift_ = 0.0;
    rise_ = 0.0;
    if (zoomToo) zoom_ = 1.0;
}

void Orbit::step(const Game &g, const Input &in, double dt, const Limits &lim, const HeightFn &height) {
    dt = std::max(0.0, std::min(dt, 0.25));
    const Vec3 off = g.eye - g.target;
    const double gameDist = length(off);
    double gy = 0, gp = 0;
    angles(off, gy, gp);
    if (in.reset) reset(true);

    /* turning: from the game's direction if not engaged yet */
    const bool moved = in.yaw != 0.0 || in.pitch != 0.0;
    if (moved) {
        if (blend_ <= 0.0) {
            yaw_ = gy;
            pitch_ = gp;
        }
        blend_ = 1.0;
        recentring_ = false;
        yaw_ = wrap360(yaw_ + in.yaw);
        pitch_ += in.pitch;
        idle_ = 0.0;
    } else {
        idle_ += dt;
    }
    if (blend_ > 0.0) {
        /* the pitch limits, widened to include the game's own pitch (so easing
         * back to it is never stopped by them) */
        const double lo = std::min(lim.pitchMin, gp), hi = std::max(lim.pitchMax, gp);
        if (moved) pitch_ = std::max(lim.pitchMin, std::min(lim.pitchMax, pitch_));
        pitch_ = std::max(lo, std::min(hi, pitch_));
        if (lim.recentre && idle_ >= lim.recentreDelay) recentring_ = true;
        if (recentring_) {
            const double k = 1.0 - std::exp(-dt / std::max(0.01, lim.recentreTime));
            yaw_ = wrap360(yaw_ + wrap180(gy - yaw_) * k);
            pitch_ += (gp - pitch_) * k;
            /* close enough: hand over to the game's camera itself (blend to 0) */
            if (std::fabs(wrap180(gy - yaw_)) < 2.0 && std::fabs(gp - pitch_) < 2.0) {
                blend_ -= dt / 0.25;
                if (blend_ <= 0.0) {
                    blend_ = 0.0;
                    recentring_ = false;
                }
            }
        }
    }

    /* zoom (log scale) */
    if (in.zoom != 0.0) zoom_ *= std::pow(2.0, in.zoom);
    zoom_ = std::max(lim.zoomMin, std::min(lim.zoomMax, zoom_));

    /* collision: only where the camera differs from the game's (the game's own
     * camera is never moved: zoom 1, not engaged) */
    double want = 1.0, wantLift = 0.0, wantRise = 0.0;
    const bool collide = gameDist > 1e-6 && (blend_ > 0.0 || zoom_ > 1.0 + 1e-9);
    Vec3 dir0 = normalize(off);
    if (blend_ > 0.0) dir0 = normalize(lerp(dir0, direction(yaw_, pitch_), blend_));
    const double dist = gameDist * zoom_;
    if (collide) {
        want = clearShare(g.target, dir0, dist, lim.clearance, height, lim.nearShare);
        /* blocked close to the target: rise over the obstacle (a steeper view
         * that sees past it) rather than pull in to it */
        if (want < lim.riseBelow && rise_ < 1e-9 + lim.riseMax) {
            double y0, p0;
            angles(dir0, y0, p0);
            /* the first rise that sees past it; else the one that sees farthest, if that is
             * clearly farther (rather than pull in to the target: into the vehicle) */
            double best = want + lim.riseGain, bestRise = 0;
            for (double r = lim.riseStep; p0 + r <= std::max(lim.pitchMax, p0) + 1e-9 && r <= lim.riseMax + 1e-9;
                 r += lim.riseStep) {
                const double s = clearShare(g.target, direction(y0, p0 + r), dist, lim.clearance, height, lim.nearShare);
                if (s >= lim.riseBelow) {
                    bestRise = r;
                    break;
                }
                if (s > best) best = s, bestRise = r;
            }
            wantRise = bestRise;
        }
    }
    if (wantRise > rise_) {
        /* easing up would meanwhile pull the eye in close to the target (into the vehicle): at once */
        double y0, p0;
        angles(dir0, y0, p0);
        if (clearShare(g.target, direction(y0, std::min(89.0, p0 + rise_)), dist, lim.clearance, height,
                       lim.nearShare) < lim.riseSnapBelow)
            rise_ = wantRise;
        else
            rise_ += (wantRise - rise_) * (1.0 - std::exp(-dt / std::max(0.001, lim.riseTime)));
    } else {
        rise_ += (wantRise - rise_) * (1.0 - std::exp(-dt / std::max(0.001, lim.letOutTime)));
    }
    Vec3 dir = dir0;
    if (rise_ > 1e-6) {
        double y0, p0;
        angles(dir0, y0, p0);
        dir = direction(y0, std::min(89.0, p0 + rise_));
    }
    if (collide) {
        want = clearShare(g.target, dir, dist, lim.clearance, height, lim.nearShare);
        const double minShare = std::max(lim.minShare, std::min(1.0, lim.minDistance / dist));
        want = std::max(want, minShare);
    }
    if (want < ratio_) {
        /* in at once (the eye must never be inside), unless it's only a little:
         * then smoothly, so small bumps don't jerk the view */
        if (ratio_ - want > 0.05) ratio_ = want;
        else ratio_ += (want - ratio_) * (1.0 - std::exp(-dt / std::max(0.001, lim.pullInTime)));
        ratio_ = std::max(want - 0.05, ratio_);
    } else {
        ratio_ += (want - ratio_) * (1.0 - std::exp(-dt / std::max(0.001, lim.letOutTime)));
    }
    ratio_ = std::max(0.0, std::min(1.0, ratio_));
    /* still below the ground where it is (pulled in as far as allowed): lift it */
    if (collide && height) {
        const Vec3 eye = g.target + dir * (dist * ratio_);
        const double h = height(eye.x, eye.y, eye.z);
        if (h > -1e30) wantLift = std::max(0.0, h + lim.clearance - eye.y);
    }
    if (wantLift > lift_) lift_ = wantLift;
    else lift_ += (wantLift - lift_) * (1.0 - std::exp(-dt / std::max(0.001, lim.liftTime)));
}

void Orbit::eyeFor(const Game &g, double extraYaw, double extraPitch, const Limits &lim, Vec3 &eye) const {
    const Vec3 off = g.eye - g.target;
    const double gameDist = length(off);
    if (gameDist < 1e-6) {
        eye = g.eye;
        return;
    }
    Vec3 dir = off * (1.0 / gameDist);
    if (blend_ > 0.0) {
        const double p = std::max(std::min(lim.pitchMin, pitch_), std::min(std::max(lim.pitchMax, pitch_), pitch_ + extraPitch));
        dir = normalize(lerp(dir, direction(yaw_ + extraYaw, p), blend_));
    }
    if (rise_ > 1e-6) {
        double y0, p0;
        angles(dir, y0, p0);
        dir = direction(y0, std::min(89.0, p0 + rise_));
    }
    eye = g.target + dir * (gameDist * zoom_ * ratio_);
    eye.y += lift_;
}

void lookAt(const Vec3 &eye, const Vec3 &target, double m[4][4]) {
    /* camera space: x right, y up, z backward (the eye looks along -z) */
    Vec3 back = normalize(eye - target);
    Vec3 up = {0, 1, 0};
    if (std::fabs(dot(back, up)) > 0.9999) up = {0, 0, 1};   /* straight down/up: any right vector */
    const Vec3 right = normalize(cross(up, back));
    const Vec3 up2 = cross(back, right);
    const Vec3 ax[3] = {right, up2, back};
    for (int c = 0; c < 3; c++) {
        m[0][c] = ax[c].x;
        m[1][c] = ax[c].y;
        m[2][c] = ax[c].z;
        m[3][c] = -dot(eye, ax[c]);
    }
    m[0][3] = m[1][3] = m[2][3] = 0.0;
    m[3][3] = 1.0;
}

bool viewEye(const double m[4][4], Vec3 &eye, Vec3 &forward) {
    for (int r = 0; r < 3; r++) {
        const double len = m[r][0] * m[r][0] + m[r][1] * m[r][1] + m[r][2] * m[r][2];
        if (std::fabs(len - 1.0) > 0.02) return false;
    }
    /* eye = -R^T t (R's columns are the axes) */
    eye.x = -(m[3][0] * m[0][0] + m[3][1] * m[0][1] + m[3][2] * m[0][2]);
    eye.y = -(m[3][0] * m[1][0] + m[3][1] * m[1][1] + m[3][2] * m[1][2]);
    eye.z = -(m[3][0] * m[2][0] + m[3][1] * m[2][1] + m[3][2] * m[2][2]);
    forward = {-m[0][2], -m[1][2], -m[2][2]};
    return true;
}

bool triHeight(const Vec3 p[3], double x, double z, double &h) {
    const double minX = std::min({p[0].x, p[1].x, p[2].x}), maxX = std::max({p[0].x, p[1].x, p[2].x});
    const double minZ = std::min({p[0].z, p[1].z, p[2].z}), maxZ = std::max({p[0].z, p[1].z, p[2].z});
    if (x < minX || x > maxX || z < minZ || z > maxZ) return false;
    const Vec3 n = cross(p[1] - p[0], p[2] - p[0]);
    if (std::fabs(n.y) < 1e-9 * (std::fabs(n.x) + std::fabs(n.z) + 1.0)) return false;
    /* inside in x/z: the three edge signs agree */
    auto side = [&](const Vec3 &a, const Vec3 &b) { return (b.x - a.x) * (z - a.z) - (b.z - a.z) * (x - a.x); };
    const double s0 = side(p[0], p[1]), s1 = side(p[1], p[2]), s2 = side(p[2], p[0]);
    const bool neg = s0 < -1e-9 || s1 < -1e-9 || s2 < -1e-9, pos = s0 > 1e-9 || s1 > 1e-9 || s2 > 1e-9;
    if (neg && pos) return false;
    h = p[0].y - (n.x * (x - p[0].x) + n.z * (z - p[0].z)) / n.y;
    return true;
}

/* ---- TriGrid ---------------------------------------------------------------- */
namespace {
constexpr int kGridMax = 64;          /* bins a side at most */
constexpr int kBigBins = 256;         /* a triangle over more bins than this is tested by every query */
}  // namespace

void TriGrid::clear() {
    tris_.clear();
    start_.clear();
    index_.clear();
    big_.clear();
}

void TriGrid::add(const Vec3 p[3]) {
    Tri t;
    for (int i = 0; i < 3; i++) t.p[i] = p[i];
    t.minX = std::min({p[0].x, p[1].x, p[2].x});
    t.maxX = std::max({p[0].x, p[1].x, p[2].x});
    t.minZ = std::min({p[0].z, p[1].z, p[2].z});
    t.maxZ = std::max({p[0].z, p[1].z, p[2].z});
    tris_.push_back(t);
}

int TriGrid::col(double x) const {
    const double c = std::floor((x - x0_) / cw_);
    return c < 0 ? 0 : c >= nx_ ? nx_ - 1 : int(c);
}

int TriGrid::row(double z) const {
    const double r = std::floor((z - z0_) / cd_);
    return r < 0 ? 0 : r >= nz_ ? nz_ - 1 : int(r);
}

void TriGrid::build() {
    start_.clear();
    index_.clear();
    big_.clear();
    if (tris_.empty()) return;
    double x1 = tris_[0].maxX, z1 = tris_[0].maxZ;
    x0_ = tris_[0].minX;
    z0_ = tris_[0].minZ;
    for (const Tri &t : tris_) {
        x0_ = std::min(x0_, t.minX);
        z0_ = std::min(z0_, t.minZ);
        x1 = std::max(x1, t.maxX);
        z1 = std::max(z1, t.maxZ);
    }
    /* about as many bins as triangles, square-ish */
    const double w = std::max(1e-6, x1 - x0_), d = std::max(1e-6, z1 - z0_);
    const double side = std::sqrt(w * d / double(tris_.size()));
    nx_ = std::max(1, std::min(kGridMax, int(std::ceil(w / std::max(side, 1e-6)))));
    nz_ = std::max(1, std::min(kGridMax, int(std::ceil(d / std::max(side, 1e-6)))));
    cw_ = w / nx_;
    cd_ = d / nz_;
    /* counting pass, then the bins */
    std::vector<uint32_t> count(size_t(nx_) * nz_ + 1, 0);
    for (uint32_t i = 0; i < tris_.size(); i++) {
        const Tri &t = tris_[i];
        const int a = col(t.minX), b = col(t.maxX), c = row(t.minZ), e = row(t.maxZ);
        if ((b - a + 1) * (e - c + 1) > kBigBins) continue;
        for (int r = c; r <= e; r++)
            for (int q = a; q <= b; q++) count[size_t(r) * nx_ + q]++;
    }
    start_.assign(count.size(), 0);
    for (size_t b = 0; b + 1 < count.size(); b++) start_[b + 1] = start_[b] + count[b];
    index_.resize(start_.back());
    std::vector<uint32_t> fill(start_.begin(), start_.end() - 1);
    for (uint32_t i = 0; i < tris_.size(); i++) {
        const Tri &t = tris_[i];
        const int a = col(t.minX), b = col(t.maxX), c = row(t.minZ), e = row(t.maxZ);
        if ((b - a + 1) * (e - c + 1) > kBigBins) {
            big_.push_back(i);
            continue;
        }
        for (int r = c; r <= e; r++)
            for (int q = a; q <= b; q++) index_[fill[size_t(r) * nx_ + q]++] = i;
    }
}

/* ---- ViewVolume ---------------------------------------------------------------- */
ViewVolume::ViewVolume(const Vec3 &eye, const Vec3 &target, double tanX, double tanY, double marginDeg, double back,
                       double farDist) {
    const Vec3 fwd = normalize(target - eye);
    const Vec3 apex = eye - fwd * back;
    lookAt(apex, apex + fwd, m_);
    const double ax = std::atan(std::max(0.0, tanX)) / kDeg + marginDeg, ay = std::atan(std::max(0.0, tanY)) / kDeg + marginDeg;
    limX_ = ax < 89.0;
    limY_ = ay < 89.0;
    sx_ = limX_ ? std::tan(ax * kDeg) : 0.0;
    sy_ = limY_ ? std::tan(ay * kDeg) : 0.0;
    far_ = farDist > 0 ? farDist + back : 0.0;
}

bool ViewVolume::boxVisible(const Vec3 &lo, const Vec3 &hi) const {
    /* camera space: x right, y up, the view along -z (depth d = -z) */
    unsigned outR = 0, outL = 0, outT = 0, outB = 0, outBehind = 0, outFar = 0;
    for (int i = 0; i < 8; i++) {
        const double x = (i & 1) ? hi.x : lo.x, y = (i & 2) ? hi.y : lo.y, z = (i & 4) ? hi.z : lo.z;
        const double cx = x * m_[0][0] + y * m_[1][0] + z * m_[2][0] + m_[3][0];
        const double cy = x * m_[0][1] + y * m_[1][1] + z * m_[2][1] + m_[3][1];
        const double d = -(x * m_[0][2] + y * m_[1][2] + z * m_[2][2] + m_[3][2]);
        if (limX_) {
            outR += cx > sx_ * d;
            outL += -cx > sx_ * d;
        }
        if (limY_) {
            outT += cy > sy_ * d;
            outB += -cy > sy_ * d;
        }
        outBehind += d < 0.0;
        if (far_ > 0) outFar += d > far_;
    }
    return outR < 8 && outL < 8 && outT < 8 && outB < 8 && outBehind < 8 && outFar < 8;
}

bool ViewVolume::boxVisibleExact(const Vec3 &lo, const Vec3 &hi) const {
    /* the corners in camera space (x, y, depth) */
    double c[8][3];
    for (int i = 0; i < 8; i++) {
        const double x = (i & 1) ? hi.x : lo.x, y = (i & 2) ? hi.y : lo.y, z = (i & 4) ? hi.z : lo.z;
        c[i][0] = x * m_[0][0] + y * m_[1][0] + z * m_[2][0] + m_[3][0];
        c[i][1] = x * m_[0][1] + y * m_[1][1] + z * m_[2][1] + m_[3][1];
        c[i][2] = -(x * m_[0][2] + y * m_[1][2] + z * m_[2][2] + m_[3][2]);
    }
    /* the planes as "inside >= 0" functions of (x, y, d) */
    auto inside = [&](int plane, const double *p) {
        switch (plane) {
            case 0: return limX_ ? sx_ * p[2] - p[0] : 1.0;
            case 1: return limX_ ? sx_ * p[2] + p[0] : 1.0;
            case 2: return limY_ ? sy_ * p[2] - p[1] : 1.0;
            case 3: return limY_ ? sy_ * p[2] + p[1] : 1.0;
            case 4: return p[2] - 1e-6;   /* in front of the apex */
            default: return far_ > 0 ? far_ - p[2] : 1.0;
        }
    };
    static const int faces[6][4] = {{0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4}, {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 3, 7, 5}};
    for (const auto &f : faces) {
        double poly[2][16][3];
        int n = 4, cur = 0;
        for (int k = 0; k < 4; k++)
            for (int j = 0; j < 3; j++) poly[0][k][j] = c[f[k]][j];
        for (int plane = 0; plane < 6 && n > 0; plane++, cur ^= 1) {
            int m = 0;
            for (int k = 0; k < n && m < 15; k++) {
                const double *a = poly[cur][k], *b = poly[cur][(k + 1) % n];
                const double da = inside(plane, a), db = inside(plane, b);
                if (da >= 0) {
                    for (int j = 0; j < 3; j++) poly[cur ^ 1][m][j] = a[j];
                    m++;
                }
                if ((da >= 0) != (db >= 0)) {
                    const double t = da / (da - db);
                    for (int j = 0; j < 3; j++) poly[cur ^ 1][m][j] = a[j] + (b[j] - a[j]) * t;
                    m++;
                }
            }
            n = m;
        }
        if (n > 0) return true;
    }
    /* no face reaches into the view: it is inside the box, or the box is out of it */
    const double ax = -m_[3][0] * m_[0][0] - m_[3][1] * m_[0][1] - m_[3][2] * m_[0][2];   /* the apex (world) */
    const double ay = -m_[3][0] * m_[1][0] - m_[3][1] * m_[1][1] - m_[3][2] * m_[1][2];
    const double az = -m_[3][0] * m_[2][0] - m_[3][1] * m_[2][1] - m_[3][2] * m_[2][2];
    const Vec3 p = {ax - m_[0][2], ay - m_[1][2], az - m_[2][2]};   /* one unit ahead on the axis */
    return p.x >= lo.x && p.x <= hi.x && p.y >= lo.y && p.y <= hi.y && p.z >= lo.z && p.z <= hi.z;
}

bool perspectiveRange(const double p[4][4], double &nearZ, double &farZ) {
    /* guPerspective (times its scale): [2][2] = (f + n) / (n - f), [2][3] = -1,
     * [3][2] = 2 n f / (n - f), [3][3] = 0 */
    const double s = -p[2][3];
    if (!(s > 1e-9) || std::fabs(p[3][3]) > 1e-6 * s) return false;
    const double a = p[2][2] / s, b = p[3][2] / s;
    if (std::fabs(a - 1.0) < 1e-12 || std::fabs(a + 1.0) < 1e-12) return false;
    nearZ = b / (a - 1.0);
    farZ = b / (a + 1.0);
    return nearZ > 0 && farZ > nearZ;
}

Sweep sweepAt(double t, const Limits &lim) {
    const double pi2 = 2.0 * kPi;
    Sweep s;
    s.yawRate = 150.0 * std::sin(pi2 * t / 9.0) + 60.0 * std::sin(pi2 * t / 2.3);
    if (std::fmod(t, 13.0) >= 12.25) s.yawRate = 320.0;   /* a flick */
    const double lo = lim.pitchMin + 2.0, hi = lim.pitchMax - 5.0;
    s.pitch = hi > lo ? (lo + hi) * 0.5 + (hi - lo) * 0.5 * std::sin(pi2 * t / 7.0) : lim.pitchMin;
    const double zl = std::max(1e-3, lim.zoomMin), zh = std::max(zl, lim.zoomMax);
    s.zoom = zl * std::pow(zh / zl, 0.5 + 0.5 * std::sin(pi2 * t / 11.0));
    return s;
}

const char *spikeCauseName(int cause) {
    static const char *const kNames[SPIKE_CAUSES] = {
        "main thread: game", "main thread: display lists", "main thread: orbit camera", "main thread: RT64 submit",
        "GPU", "RT64 render thread", "new shaders", "new textures", "present / other"};
    return cause >= 0 && cause < SPIKE_CAUSES ? kNames[cause] : "?";
}

int spikeCause(const SpikeWindow &w) {
    if (w.retraceMs > 1.5 * w.periodMs) {
        const double parts[4] = {w.gameMs, w.listsMs, w.orbitMs, w.submitMs};
        int best = 0;
        for (int i = 1; i < 4; i++)
            if (parts[i] > parts[best]) best = i;
        if (best == 1 && w.queueFull)
            return w.targetMs > 0 && w.gpuMs > kSpikeGpuShare * w.targetMs ? SPIKE_GPU : SPIKE_RENDER;
        return SPIKE_MAIN_GAME + best;
    }
    if (w.targetMs > 0 && w.gpuMs > kSpikeGpuShare * w.targetMs) return SPIKE_GPU;
    if (w.frameMs > 0 && w.renderMs > w.frameMs) return SPIKE_RENDER;
    if (w.shaders > 0) return SPIKE_SHADERS;
    if (w.textures >= kSpikeTextures) return SPIKE_TEXTURES;
    return SPIKE_PRESENT;
}

}  // namespace orbit
}  // namespace live
