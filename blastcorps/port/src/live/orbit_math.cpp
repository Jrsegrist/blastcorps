/* The free orbit camera's math (see orbit_math.h). */
#include <algorithm>
#include <cmath>

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
            for (double r = lim.riseStep; p0 + r <= std::max(lim.pitchMax, p0) + 1e-9 && r <= lim.riseMax + 1e-9;
                 r += lim.riseStep) {
                const double s = clearShare(g.target, direction(y0, p0 + r), dist, lim.clearance, height, lim.nearShare);
                if (s >= lim.riseBelow) {
                    wantRise = r;
                    break;
                }
            }
        }
    }
    if (wantRise > rise_) rise_ += (wantRise - rise_) * (1.0 - std::exp(-dt / std::max(0.001, lim.riseTime)));
    else rise_ += (wantRise - rise_) * (1.0 - std::exp(-dt / std::max(0.001, lim.letOutTime)));
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

}  // namespace orbit
}  // namespace live
