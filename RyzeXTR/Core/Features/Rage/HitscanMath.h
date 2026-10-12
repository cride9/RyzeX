#pragma once
#include "HitboxGeometry.h"
#include <limits>

namespace hitscan {
struct IntersectionCache {
    bool computed{}, valid{};
    hitbox::Vec point{};
};
struct Statistics {
    int damageCalls{}, reusedRays{}, boundSkips{}, intersectionPoints{}, fallbackPoints{};
};
struct DamageGeometry {
    bool computed{};
    const void* model{};
    int hitboxSet{}, count{};
    std::array<hitbox::Shape, 19> shapes{};
    std::array<int, 19> hitgroups{};
};

// Either the bullet flies directly to the player, or at least one surface
// removes damage. Wall thickness is not counted consistently by autowall's
// range falloff, so a penetrated path uses the surface-loss bound instead.
inline double RawDamageBound(double damage, double rangeModifier, double nearestDistance, double minimumSurfaceLoss) {
    if (!std::isfinite(damage) || damage < 0. || !std::isfinite(rangeModifier) || rangeModifier <= 0. || rangeModifier > 1.
        || !std::isfinite(nearestDistance) || nearestDistance < 0. || !std::isfinite(minimumSurfaceLoss)
        || minimumSurfaceLoss < 0. || minimumSurfaceLoss > 1.) return std::numeric_limits<double>::infinity();
    const double direct = damage * std::pow(rangeModifier, (std::max)(0., nearestDistance - .05) / 500.);
    const double penetrated = damage * (1. - minimumSurfaceLoss);
    return (std::max)(direct, penetrated) + damage * .0001 + .001;
}
}
