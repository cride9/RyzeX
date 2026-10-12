#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

// Engine-independent scoring. Rates are compared in the engine's native units;
// this deliberately does not assume that layer 6 stores cycles per second.
namespace experimental {
inline constexpr std::array<float, 9> Fractions{ -1.f, -.75f, -.5f, -.25f, 0.f, .25f, .5f, .75f, 1.f };
inline constexpr int Center = 4;
inline constexpr float WeightQuantum = 1.f / 255.f;
inline constexpr float CycleQuantum = 1.f / 32768.f;

inline float NormalizeYaw(float value) {
    return std::isfinite(value) ? std::remainder(value, 360.f) : 0.f;
}

inline float YawWidth(float walkSpeed, float crouchSpeed, float transition, float duck) {
    if (!std::isfinite(walkSpeed) || !std::isfinite(crouchSpeed)
        || !std::isfinite(transition) || !std::isfinite(duck))
        return 0.f;
    float width = 1.f + std::clamp(walkSpeed, 0.f, 1.f)
        * (-.2f - .3f * std::clamp(transition, 0.f, 1.f));
    width += std::clamp(duck, 0.f, 1.f) * std::clamp(crouchSpeed, 0.f, 1.f) * (.5f - width);
    return width;
}

inline float CycleDistance(float a, float b) {
    return std::fabs(std::remainder(a - b, 1.f));
}

struct Layer {
    int sequence{};
    float rate{}, cycle{}, weight{};
    bool Valid() const {
        return sequence > 0 && std::isfinite(rate) && rate >= 0.f
            && std::isfinite(cycle) && cycle >= 0.f && cycle <= 1.f
            && std::isfinite(weight) && weight >= 0.f && weight <= 1.f;
    }
};

inline float Score(const Layer& received, const Layer& predicted) {
    if (!received.Valid() || !predicted.Valid() || received.sequence != predicted.sequence || received.rate <= 0.f)
        return (std::numeric_limits<float>::infinity)();
    const float rateError = std::fabs(predicted.rate - received.rate) / (std::max)(received.rate * .04f, .00001f);
    const float weightError = (std::max)(0.f, std::fabs(predicted.weight - received.weight) - WeightQuantum) / .035f;
    const float cycleError = (std::max)(0.f, CycleDistance(predicted.cycle, received.cycle) - CycleQuantum) / .02f;
    return .65f * rateError + .25f * weightError + .10f * cycleError;
}

// The inverse alive-loop relation only applies to its interior, on an unchanged
// sequence/rate. Endpoint weights and a cycle restart carry no unique speed.
inline bool AliveLoopSpeed(const Layer& current, const Layer& previous, float maxSpeed, float& speed) {
    if (!current.Valid() || !previous.Valid() || current.sequence != previous.sequence
        || current.rate <= 0.f || previous.rate <= 0.f || !std::isfinite(maxSpeed) || maxSpeed <= 0.f
        || current.weight <= WeightQuantum || current.weight >= 1.f - WeightQuantum
        || current.cycle < previous.cycle
        || std::fabs(current.rate - previous.rate) > previous.rate * .01f)
        return false;
    speed = maxSpeed * (.9f - .35f * current.weight);
    return true;
}

enum class Reason : int { Warmup, Ambiguous, MovingLayers, LastMove, LbyChanged, Transition, Override, Invalid,
    Standing, LbyExtended, Bruteforce, HitHistory };

// Standing has no unique layer-6 inversion. LBY cadence is a heuristic, not a
// measurement of server foot yaw. A moving/airborne transition starts a new clock.
struct Standing {
    float lastTime = -1.f, lastLby{}, lastUpdate = -1.f, stoppedTime = -1.f;
    float movingYaw{}, movingTime = -1.f;
    int cadence{};
    bool ready{}, wasMoving{};

    void Update(float time, float speed, bool ground, float lby, float footYaw) {
        if (!std::isfinite(time) || !std::isfinite(speed) || !std::isfinite(lby) || !std::isfinite(footYaw)) {
            *this = {};
            return;
        }
        if (ready && (time <= lastTime || time - lastTime > .5f)) *this = {};
        const bool moving = ground && speed > 5.f;
        if (!ground || moving) {
            cadence = 0;
            lastUpdate = stoppedTime = -1.f;
            if (moving) { movingYaw = footYaw; movingTime = time; }
        } else {
            if (!ready || wasMoving || stoppedTime < 0.f) stoppedTime = time;
            if (ready && !wasMoving && std::fabs(NormalizeYaw(lby - lastLby)) > 1.f) {
                const float period = time - lastUpdate;
                cadence = lastUpdate >= 0.f && period >= .9f && period <= 1.35f ? cadence + 1 : 0;
                lastUpdate = time;
            }
        }
        wasMoving = moving;
        lastTime = time;
        lastLby = lby;
        ready = true;
    }

    float Guess(float eyeYaw, float lby, float negativeBound, float positiveBound, float time, bool& extended) const {
        const float difference = NormalizeYaw(lby - eyeYaw);
        const bool periodic = cadence >= 1 && lastUpdate >= 0.f && time - lastUpdate <= 1.35f;
        // A constant EXTENDED flick can keep the same networked LBY value on
        // every update. After a full standing interval, also test a persistent
        // LBY outside the bounded yaw family. This remains a low-trust hint.
        const bool persistentOutside = stoppedTime >= 0.f && time - stoppedTime >= 1.35f
            && std::fabs(difference) > (std::max)(negativeBound, positiveBound) + 15.f;
        extended = (periodic || persistentOutside) && std::fabs(difference) > 35.f;
        // With the repository's EXTENDED pattern, the LBY flick is opposite the
        // unsent +95 yaw. Test the opposite edge first, then let shot feedback
        // reject it. Never copy the -120/+95 command angles into goal feet yaw.
        if (extended) return difference < 0.f ? positiveBound : -negativeBound;
        if (movingTime >= 0.f && time - movingTime >= 0.f && time - movingTime <= 1.5f)
            return std::clamp(NormalizeYaw(movingYaw - eyeYaw), -negativeBound, positiveBound);
        return std::clamp(difference, -negativeBound, positiveBound);
    }
};

struct RecoveryChoice { float delta{}; bool changed{}, hitHistory{}; };
struct Recovery {
    std::array<float, 12> failed{}, failedTime{};
    int count{}, misses{};
    float hitDelta{}, hitTime = -1.f;

    bool Blocked(float delta) const {
        for (int i = 0; i < count; ++i)
            if (std::fabs(NormalizeYaw(delta - failed[i])) <= 6.f) return true;
        return false;
    }
    void Miss(float delta, float time) {
        if (!std::isfinite(delta) || !std::isfinite(time)) return;
        ++misses;
        hitTime = -1.f;
        if (!Blocked(delta)) {
            if (count == static_cast<int>(failed.size())) {
                for (int i = 1; i < count; ++i) { failed[i - 1] = failed[i]; failedTime[i - 1] = failedTime[i]; }
                --count;
            }
            failed[count] = NormalizeYaw(delta); failedTime[count++] = time;
        }
    }
    void Hit(float delta, float time) {
        if (!std::isfinite(delta) || !std::isfinite(time)) return;
        hitDelta = NormalizeYaw(delta); hitTime = time;
        count = misses = 0;
    }
    RecoveryChoice Choose(float primary, float negativeBound, float positiveBound, float lbyDelta, float time,
                          bool allowHitHistory) {
        for (int i = 0; i < count;) {
            if (time < failedTime[i] || time - failedTime[i] > 8.f) {
                failed[i] = failed[count - 1]; failedTime[i] = failedTime[--count];
            } else ++i;
        }
        if (allowHitHistory && hitTime >= 0.f && time >= hitTime && time - hitTime <= 4.f && !Blocked(hitDelta))
            return {std::clamp(hitDelta, -negativeBound, positiveBound), false, true};
        if (!Blocked(primary)) return {primary, false, false};
        const std::array<float, 13> options{ -primary, 0.f, primary < 0.f ? positiveBound : -negativeBound,
            -negativeBound, positiveBound, -.5f * negativeBound, .5f * positiveBound,
            -.25f * negativeBound, .25f * positiveBound, -.75f * negativeBound, .75f * positiveBound,
            std::clamp(lbyDelta, -negativeBound, positiveBound), primary };
        for (float delta : options) {
            delta = std::clamp(delta, -negativeBound, positiveBound);
            if (!Blocked(delta)) return {delta, true, false};
        }
        // Start a new cycle while keeping the preferred bad yaw blocked. Merely
        // clearing everything would restore that yaw on the very next packet,
        // before the alternative has even been fired at.
        count = 1; failed[0] = NormalizeYaw(primary); failedTime[0] = time;
        for (float delta : options) {
            delta = std::clamp(delta, -negativeBound, positiveBound);
            if (!Blocked(delta)) return {delta, true, false};
        }
        return {primary, false, false};
    }
};

struct Decision {
    int candidate = Center;
    float confidence{};
    Reason reason = Reason::Ambiguous;
};

struct Evidence {
    int consecutive{}, previousWinner = -1, lastMoving = Center;
    float lastMovingTime = -1.f, lastConfidence{};

    Decision Update(const std::array<float, 9>& scores, bool eligible, bool standing,
                    bool lbyChanged, float time, bool historyReady) {
        Decision result;
        result.reason = historyReady ? Reason::Ambiguous : Reason::Warmup;
        if (!std::isfinite(time)) {
            *this = {};
            result.reason = Reason::Invalid;
            return result;
        }
        if (lbyChanged) {
            lastMovingTime = -1.f;
            consecutive = 0;
            previousWinner = -1;
            result.reason = Reason::LbyChanged;
        }
        if (eligible && historyReady) {
            int winner = Center;
            float best = (std::numeric_limits<float>::infinity)();
            for (int i = 0; i < static_cast<int>(scores.size()); ++i) {
                if (std::isfinite(scores[i]) && scores[i] < best) {
                    best = scores[i];
                    winner = i;
                }
            }
            float second = (std::numeric_limits<float>::infinity)();
            for (int i = 0; i < static_cast<int>(scores.size()); ++i)
                if (i != winner && std::isfinite(scores[i])) second = (std::min)(second, scores[i]);
            const float margin = std::isfinite(second) ? (second - best) / (std::max)(second, .5f) : 0.f;
            if (best <= 1.25f && margin >= .12f) {
                consecutive = winner == previousWinner ? consecutive + 1 : 1;
                previousWinner = winner;
                if (consecutive >= 2) {
                    result.candidate = winner;
                    result.confidence = std::clamp(margin * (1.f - best / 2.5f), 0.f, 1.f);
                    result.reason = Reason::MovingLayers;
                    lastMoving = winner;
                    lastMovingTime = time;
                    lastConfidence = result.confidence;
                    return result;
                }
            } else {
                consecutive = 0;
                previousWinner = -1;
            }
        } else {
            consecutive = 0;
            previousWinner = -1;
            if (historyReady && !standing && !lbyChanged) result.reason = Reason::Transition;
        }
        // Standing has no layer-based side observation. Carry only recent,
        // accepted moving evidence, and invalidate it on an LBY change.
        const float age = time - lastMovingTime;
        if (standing && historyReady && !lbyChanged && lastMovingTime >= 0.f && age >= 0.f && age <= .75f) {
            result.candidate = lastMoving;
            result.confidence = lastConfidence * (1.f - age / .75f);
            result.reason = Reason::LastMove;
        }
        return result;
    }
};
}
