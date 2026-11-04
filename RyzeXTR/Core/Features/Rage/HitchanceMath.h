#pragma once
#include "HitboxGeometry.h"

// Hit probability over the weapon's deterministic spread distribution. The
// hitbox is prepared once in the aim frame; samples only evaluate scalar
// capsule-distance / box-interval equations. No engine traces or bone writes.
namespace hitchance {
using hitbox::Vec;
struct Offset { float x{}, y{}; };
struct Seed { float radiusA{}, radiusB{}, sinA{}, cosA{}, sinB{}, cosB{}; };
inline Offset Spread(const Seed& seed, float inaccuracy, float spread) {
    const float a = seed.radiusA * inaccuracy, b = seed.radiusB * spread;
    return {seed.cosA * a + seed.cosB * b, seed.sinA * a + seed.sinB * b};
}

struct Footprint {
    bool valid{}, capsule{};
    double range{}, radiusSquared{}, axisSquared{}, axisOrigin{};
    Vec origin{}, axis{};
    std::array<Vec, 3> basis{};
    std::array<double, 3> originDot{}, axisDot{}, boxOrigin{}, extents{};
    std::array<std::array<double, 3>, 3> gram{}, boxDirection{};

    Footprint(const hitbox::Shape& shape, Vec eye, Vec forward, Vec right, Vec up, double distance) {
        if (!shape.valid || !eye.Valid() || !forward.Valid() || !right.Valid() || !up.Valid()
            || !std::isfinite(distance) || distance <= 0.) return;
        basis = {forward, right, up}; range = distance; capsule = shape.capsule;
        // Keep the actual float AngleVectors basis, including its tiny metric
        // error, rather than assuming a perfectly orthonormal camera frame.
        for (int i = 0; i < 3; ++i) {
            if (std::fabs(basis[i].LengthSq() - 1.) > .001) return;
            for (int j = 0; j < 3; ++j) gram[i][j] = basis[i].Dot(basis[j]);
        }
        if (capsule) {
            if (!shape.a.Valid() || !shape.b.Valid() || !std::isfinite(shape.radius) || shape.radius < 0.) return;
            origin = eye - shape.a; axis = shape.b - shape.a;
            radiusSquared = shape.radius * shape.radius;
            axisSquared = axis.LengthSq(); axisOrigin = axis.Dot(origin);
            for (int i = 0; i < 3; ++i) { originDot[i] = basis[i].Dot(origin); axisDot[i] = basis[i].Dot(axis); }
        } else {
            for (int i = 0; i < 3; ++i) {
                if (!shape.axes[i].Valid() || !std::isfinite(shape.extents[i]) || shape.extents[i] < 0.) return;
                boxOrigin[i] = (eye - shape.a).Dot(shape.axes[i]); extents[i] = shape.extents[i];
                for (int j = 0; j < 3; ++j) boxDirection[i][j] = basis[j].Dot(shape.axes[i]);
            }
        }
        valid = true;
    }

    bool Contains(Offset offset) const {
        if (!valid || !std::isfinite(offset.x) || !std::isfinite(offset.y)) return false;
        const double x = offset.x, y = offset.y;
        if (!capsule) {
            double first = 0., last = range;
            for (int i = 0; i < 3; ++i) {
                const double slope = boxDirection[i][0] + x * boxDirection[i][1] + y * boxDirection[i][2];
                if (std::fabs(slope) < 1e-14) {
                    if (std::fabs(boxOrigin[i]) > extents[i]) return false;
                    continue;
                }
                double a = (-extents[i] - boxOrigin[i]) / slope, b = (extents[i] - boxOrigin[i]) / slope;
                if (a > b) std::swap(a, b);
                first = (std::max)(first, a); last = (std::min)(last, b);
                if (first > last) return false;
            }
            return true;
        }
        const double dd = gram[0][0] + x*x*gram[1][1] + y*y*gram[2][2]
            + 2.*(x*gram[0][1] + y*gram[0][2] + x*y*gram[1][2]);
        if (dd <= 1e-20) return false;
        const double da = originDot[0] + x*originDot[1] + y*originDot[2];
        const double dv = axisDot[0] + x*axisDot[1] + y*axisDot[2];
        double depth = 0., along = 0.;
        if (axisSquared <= 1e-20) depth = std::clamp(-da / dd, 0., range);
        else {
            const double determinant = dd*axisSquared - dv*dv;
            if (determinant > 1e-14*dd*axisSquared)
                depth = std::clamp((dv*axisOrigin - da*axisSquared) / determinant, 0., range);
            along = (dv*depth + axisOrigin) / axisSquared;
            if (along < 0.) { along = 0.; depth = std::clamp(-da / dd, 0., range); }
            else if (along > 1.) { along = 1.; depth = std::clamp((dv-da) / dd, 0., range); }
        }
        // Evaluate the residual in world coordinates to avoid cancellation
        // between squared distances for tiny hitboxes far from the shooter.
        const Vec residual = origin + (basis[0] + basis[1]*x + basis[2]*y)*depth - axis*along;
        return residual.LengthSq() <= radiusSquared;
    }
};

inline float Percent(const Footprint& footprint, const Offset* samples, int count) {
    if (!footprint.valid || !samples || count <= 0 || count > 256) return 0.f;
    int hits = 0;
    for (int i = 0; i < count; ++i) hits += footprint.Contains(samples[i]);
    return 100.f * static_cast<float>(hits) / static_cast<float>(count);
}
inline bool Meets(float percent, int required) {
    return std::isfinite(percent) && (required <= 1 || percent >= required);
}
}
