#pragma once
#include "HitboxGeometry.h"

namespace backtrack {
inline constexpr double TeleportDistanceSquared=64.*64.;
inline bool PositionDiscontinuity(hitbox::Vec a,hitbox::Vec b) {
    if (!a.Valid() || !b.Valid()) return true;
    const float x=static_cast<float>(a.x)-static_cast<float>(b.x);
    const float y=static_cast<float>(a.y)-static_cast<float>(b.y);
    const float z=static_cast<float>(a.z)-static_cast<float>(b.z);
    return x*x+y*y+z*z>4096.f;
}
// RecordEntity does not append unchanged or regressed simulation timestamps.
// Such a packet is not evidence that all previously captured poses vanished.
inline bool CanAppendSnapshot(float simulation, float newest, bool hasNewest) {
    return std::isfinite(simulation) && simulation>=0.f && (!hasNewest || (std::isfinite(newest) && simulation>newest));
}
// Follow the same newest-to-oldest position chain as BacktrackEntity. A jump
// from this record to the next older record is a boundary AFTER this pose.
class HistoryWalk {
public:
    explicit HistoryWalk(hitbox::Vec current) : previous(current) {}
    bool Visit(hitbox::Vec origin, bool alive, bool boundaryAfter) {
        if (stopped || !alive || PositionDiscontinuity(origin,previous)) { stopped=true; return false; }
        previous=origin; stopped=boundaryAfter;
        return true;
    }
private:
    hitbox::Vec previous;
    bool stopped{};
};
}
