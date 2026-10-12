#pragma once
#include "../HitboxGeometry.h"
#include <cstdint>
#include <deque>
#include <optional>
#include <utility>

namespace experimental {
enum class ShotOutcome { Ignore, Hit, GeometryMiss };

// Value-owned, bounded shot history. Event ordering assigns impacts to the most
// recent weapon_fire, not the current aim target (which may already have changed).
template<class Payload> class ShotQueue {
public:
    struct Shot {
        Payload payload;
        std::uint64_t serial{}, epoch{};
        unsigned int entityHandle{};
        int target{}, command{}, damage{}, hitgroup{};
        float spawnTime{}, delta{};
        double created{}, firedTime{}, lastEvent{};
        hitbox::Vec start{}, intended{}, impact{};
        hitbox::Shape expected;
        bool fired{}, hasImpact{}, hurt{}, hurtOther{}, ambiguous{}, learn{};

        ShotOutcome Outcome() const {
            if (!fired || !learn || ambiguous || hurtOther) return ShotOutcome::Ignore;
            if (hurt) return ShotOutcome::Hit;
            if (!hasImpact || !expected.valid || !start.Valid() || !impact.Valid()) return ShotOutcome::Ignore;
            // Do not extend the ray through the wall at which the bullet stopped.
            return expected.Intersects(start, impact) ? ShotOutcome::GeometryMiss : ShotOutcome::Ignore;
        }
    };

    bool Add(Shot shot) {
        if (!std::isfinite(shot.created) || !shot.start.Valid() || !shot.intended.Valid()) return false;
        for (const auto& pending : shots) if (pending.command == shot.command) return false;
        if (shots.size() == 16) shots.pop_front();
        shot.serial = ++sequence;
        shots.push_back(std::move(shot));
        return true;
    }
    void Fire(double time) {
        active = 0;
        for (auto& shot : shots) {
            if (!shot.fired && time >= shot.created && time - shot.created <= 2.) {
                shot.fired = true;
                shot.firedTime = shot.lastEvent = time;
                active = shot.serial;
                break;
            }
        }
    }
    void Impact(hitbox::Vec point, double time) {
        if (!point.Valid()) return;
        for (auto& shot : shots) if (shot.serial == active && shot.fired && time >= shot.firedTime && time - shot.firedTime <= 1.) {
            const auto aimed = shot.intended - shot.start, actual = point - shot.start;
            const double scale = std::sqrt(aimed.LengthSq() * actual.LengthSq());
            if (scale < 1e-6 || aimed.Dot(actual) / scale < .98) shot.ambiguous = true;
            if (!shot.hasImpact || actual.LengthSq() > (shot.impact - shot.start).LengthSq()) shot.impact = point;
            shot.hasImpact = true;
            shot.lastEvent = time;
            break;
        }
    }
    void Hurt(int target, int damage, int group, double time) {
        Shot* match = nullptr;
        for (auto& shot : shots) if (shot.serial == active && shot.fired) {
            if (shot.target == target) match = &shot;
            else shot.hurtOther = true;
        }
        if (!match) {
            // Out-of-order hurt events are safe to log, but not to learn from.
            for (auto it = shots.rbegin(); it != shots.rend(); ++it)
                if (it->fired && it->target == target && time >= it->firedTime && time - it->firedTime <= 1.) {
                    match = &*it; match->ambiguous = true; break;
                }
        }
        if (!match || time < match->firedTime || time - match->firedTime > 1.) return;
        match->hurt = true; match->damage += damage; match->hitgroup = group; match->lastEvent = time;
        // Two shots to the same target in one event batch cannot be uniquely
        // distinguished. Suppress false misses on the other shot as well.
        for (auto& shot : shots)
            if (&shot != match && shot.fired && shot.target == target && std::fabs(shot.firedTime - match->firedTime) < .03)
                shot.ambiguous = match->ambiguous = true;
    }
    std::optional<Shot> PopReady(double time, double settle) {
        if (!std::isfinite(time)) return {};
        for (auto it = shots.begin(); it != shots.end(); ++it) {
            const bool expired = time < it->created || time - it->created >= 2.;
            const bool ready = it->fired && (it->hurt || it->hasImpact) && time - it->lastEvent >= settle;
            if (expired || ready) {
                if (!ready) it->learn = false;
                Shot result = std::move(*it);
                shots.erase(it);
                return result;
            }
        }
        return {};
    }
    void Reset(int target = -1) {
        if (target < 0) { shots.clear(); active = 0; return; }
        for (auto it = shots.begin(); it != shots.end();)
            if (it->target == target) it = shots.erase(it); else ++it;
    }
    std::size_t Size() const { return shots.size(); }
private:
    std::deque<Shot> shots;
    std::uint64_t sequence{}, active{};
};
}
