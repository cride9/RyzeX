#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

// Independent of the engine and its mutable bone cache. Double precision keeps
// near-tangent segment tests stable at large world coordinates.
namespace hitbox {
struct Vec {
    double x{}, y{}, z{};
    Vec operator+(Vec b) const { return { x + b.x, y + b.y, z + b.z }; }
    Vec operator-(Vec b) const { return { x - b.x, y - b.y, z - b.z }; }
    Vec operator*(double s) const { return { x * s, y * s, z * s }; }
    Vec operator/(double s) const { return *this * (1. / s); }
    double Dot(Vec b) const { return x * b.x + y * b.y + z * b.z; }
    double LengthSq() const { return Dot(*this); }
    bool Valid() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
};

inline Vec ClosestPoint(Vec point, Vec a, Vec b) {
    const Vec axis = b - a;
    const double length = axis.LengthSq();
    return length > 1e-20 ? a + axis * std::clamp((point - a).Dot(axis) / length, 0., 1.) : a;
}

inline double SegmentDistanceSq(Vec p, Vec q, Vec a, Vec b) {
    const Vec u = q - p, v = b - a, w = p - a;
    const double uu = u.LengthSq(), vv = v.LengthSq(), uv = u.Dot(v), uw = u.Dot(w), vw = v.Dot(w);
    double s = 0., t = 0.;
    if (uu <= 1e-20 && vv <= 1e-20) return w.LengthSq();
    if (uu <= 1e-20) t = std::clamp(vw / vv, 0., 1.);
    else if (vv <= 1e-20) s = std::clamp(-uw / uu, 0., 1.);
    else {
        const double denominator = uu * vv - uv * uv;
        if (denominator > 1e-14 * uu * vv) s = std::clamp((uv * vw - uw * vv) / denominator, 0., 1.);
        t = (uv * s + vw) / vv;
        if (t < 0.) { t = 0.; s = std::clamp(-uw / uu, 0., 1.); }
        else if (t > 1.) { t = 1.; s = std::clamp((uv - uw) / uu, 0., 1.); }
    }
    return (w + u * s - v * t).LengthSq();
}

struct Shape {
    bool capsule = true, valid = false;
    Vec a{}, b{}; // Capsule axis endpoints, or OBB center in a.
    double radius{};
    std::array<Vec, 3> axes{};
    std::array<double, 3> extents{};

    Vec Center() const { return capsule ? (a + b) * .5 : a; }
    bool Contains(Vec point, double inset = 0.) const {
        if (!valid || !point.Valid() || !std::isfinite(inset) || inset < 0.) return false;
        if (capsule) {
            const double r = radius - inset;
            return r >= 0. && (point - ClosestPoint(point, a, b)).LengthSq() <= r * r;
        }
        for (int i = 0; i < 3; ++i)
            if (extents[i] < inset || std::fabs((point - a).Dot(axes[i])) > extents[i] - inset) return false;
        return true;
    }
    Vec Project(Vec point, double inset) const {
        if (capsule) {
            const Vec nearest = ClosestPoint(point, a, b), offset = point - nearest;
            const double distance = std::sqrt(offset.LengthSq()), r = (std::max)(0., radius - inset);
            return distance <= r || distance <= 1e-20 ? point : nearest + offset * (r / distance);
        }
        Vec projected = a;
        for (int i = 0; i < 3; ++i) {
            const double extent = (std::max)(0., extents[i] - inset);
            projected = projected + axes[i] * std::clamp((point - a).Dot(axes[i]), -extent, extent);
        }
        return projected;
    }
    bool Intersects(Vec start, Vec end) const {
        if (!valid || !start.Valid() || !end.Valid()) return false;
        if (capsule) return SegmentDistanceSq(start, end, a, b) <= radius * radius;
        const Vec delta = end - start;
        double enter = 0., leave = 1.;
        for (int i = 0; i < 3; ++i) {
            const double origin = (start - a).Dot(axes[i]), direction = delta.Dot(axes[i]);
            if (std::fabs(direction) < 1e-12) {
                if (origin < -extents[i] || origin > extents[i]) return false;
                continue;
            }
            double first = (-extents[i] - origin) / direction, last = (extents[i] - origin) / direction;
            if (first > last) std::swap(first, last);
            enter = (std::max)(enter, first);
            leave = (std::min)(leave, last);
            if (enter > leave) return false;
        }
        return true;
    }
};

inline constexpr std::size_t MaxShapes = 10;
inline bool ContainsAll(const Shape* shapes, std::size_t count, Vec point, double inset) {
    if (!count || count > MaxShapes) return false;
    for (std::size_t i = 0; i < count; ++i) if (!shapes[i].Contains(point, inset)) return false;
    return true;
}

// Dykstra projections onto convex capsules/OBBs. A result is accepted only after
// checking every original shape with a positive clearance. The iteration cap
// may miss tiny intersections; it cannot certify a point outside a hitbox.
inline bool FindIntersection(const Shape* shapes, std::size_t count, Vec preferred, Vec& result) {
    if (!count || count > MaxShapes || !preferred.Valid()) return false;
    Vec average{};
    for (std::size_t i = 0; i < count; ++i) {
        if (!shapes[i].valid) return false;
        average = average + shapes[i].Center();
    }
    average = average / static_cast<double>(count);
    auto solve = [&](double inset, Vec seed, Vec& output) {
        std::array<Vec, MaxShapes> corrections{};
        Vec point = seed;
        for (int iteration = 0; iteration < 96; ++iteration) {
            for (std::size_t i = 0; i < count; ++i) {
                const Vec shifted = point + corrections[i];
                const Vec projected = shapes[i].Project(shifted, inset);
                corrections[i] = shifted - projected;
                point = projected;
            }
            // A small extra inset in projection avoids accepting numeric noise
            // at the requested boundary in the independent certification test.
            if (ContainsAll(shapes, count, point, inset * .99)) { output = point; return true; }
        }
        return false;
    };
    Vec point;
    if (!solve(.05, average, point) && !solve(.05, preferred, point)) return false;
    double low = .05, high = 4.;
    for (int i = 0; i < 5; ++i) {
        const double middle = (low + high) * .5;
        Vec improved;
        if (solve(middle, point, improved)) { low = middle; point = improved; }
        else high = middle;
    }
    if (!ContainsAll(shapes, count, point, .04)) return false;
    result = point;
    return true;
}
}
