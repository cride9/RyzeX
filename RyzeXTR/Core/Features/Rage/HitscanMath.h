#pragma once
#include "HitboxGeometry.h"
#include <limits>
#include <chrono>
#include <cstdint>

namespace hitscan {
struct IntersectionCache {
    bool computed{}, valid{};
    hitbox::Vec point{};
};
struct Statistics {
    int damageCalls{}, reusedRays{}, boundSkips{}, intersectionPoints{}, fallbackPoints{};
    int recordsConsidered{}, recordVisits{}, deferredPoints{}, budgetStops{};
    int lowDamageStops{}, deadlineStops{}, traceCalls{}, exitSteps{};
    double damageMicros{}, totalMicros{};
};
enum class Failure { None, Invalid, NoHit, NoExit, Blocked, LowDamage, Deadline };
using Clock = std::chrono::steady_clock;
struct DamageQuery {
    float minimumDamage{};
    bool timed{};
    Clock::time_point deadline{};
    bool Expired() const { return timed && Clock::now() >= deadline; }
};
template<class T, std::size_t Capacity = 8> struct Points {
    std::array<T, Capacity> values{};
    std::size_t count{};
    void push_back(const T& value) { if (count < Capacity) values[count++] = value; }
    bool empty() const { return count == 0; }
    std::size_t size() const { return count; }
    T* begin() { return values.data(); }
    T* end() { return begin() + count; }
    const T* begin() const { return values.data(); }
    const T* end() const { return begin() + count; }
    T& operator[](std::size_t i) { return values[i]; }
};
inline std::uint64_t HashBytes(const void* bytes, std::size_t size, std::uint64_t hash = 14695981039346656037ull) {
    const auto* data = static_cast<const unsigned char*>(bytes);
    for (std::size_t i = 0; i < size; ++i) hash = (hash ^ data[i]) * 1099511628211ull;
    return hash;
}
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
