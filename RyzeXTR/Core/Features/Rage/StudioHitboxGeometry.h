#pragma once
#include "HitboxGeometry.h"
#include "../../SDK/Entity.h"
#include "../../SDK/math.h"

namespace hitbox {
inline Vec FromVector(const Vector& value) { return { value.x, value.y, value.z }; }
inline Vector ToVector(Vec value) { return Vector(static_cast<float>(value.x), static_cast<float>(value.y), static_cast<float>(value.z)); }

inline Shape FromStudio(const mstudiobbox_t* box, const matrix3x4a_t* bones) {
    Shape shape;
    if (!box || !bones || box->iBone < 0 || box->iBone >= MAXSTUDIOBONES
        || !box->vecBBMin.IsValid() || !box->vecBBMax.IsValid() || !std::isfinite(box->flRadius)) return shape;
    matrix3x4_t rotation, transform;
    M::AngleMatrix(box->angOffsetOrientation, rotation);
    M::ConcatTransforms(bones[box->iBone], rotation, transform);
    const Vec translation{ transform[0][3], transform[1][3], transform[2][3] };
    std::array<double, 3> scales{};
    for (int i = 0; i < 3; ++i) {
        const Vec axis{ transform[0][i], transform[1][i], transform[2][i] };
        scales[i] = std::sqrt(axis.LengthSq());
        if (!axis.Valid() || scales[i] < 1e-6) return shape;
        shape.axes[i] = axis / scales[i];
    }
    if (!translation.Valid() || std::fabs(shape.axes[0].Dot(shape.axes[1])) > .001
        || std::fabs(shape.axes[0].Dot(shape.axes[2])) > .001 || std::fabs(shape.axes[1].Dot(shape.axes[2])) > .001) return shape;
    shape.capsule = box->flRadius >= 0.f;
    if (shape.capsule) {
        // Nonuniformly scaled capsules become swept ellipsoids. Reject rather
        // than overestimate their volume; the engine path remains a fallback.
        if (std::fabs(scales[0] - scales[1]) > .001 || std::fabs(scales[0] - scales[2]) > .001) return shape;
        shape.a = FromVector(M::VectorTransform(box->vecBBMin, transform));
        shape.b = FromVector(M::VectorTransform(box->vecBBMax, transform));
        shape.radius = box->flRadius * scales[0];
    } else {
        shape.a = FromVector(M::VectorTransform((box->vecBBMin + box->vecBBMax) * .5f, transform));
        for (int i = 0; i < 3; ++i) {
            shape.extents[i] = (box->vecBBMax[i] - box->vecBBMin[i]) * .5 * scales[i];
            if (shape.extents[i] < 0.) return shape;
        }
    }
    shape.valid = shape.a.Valid() && (!shape.capsule || (shape.b.Valid() && std::isfinite(shape.radius)));
    return shape;
}
}
