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

enum class Reason : int { Warmup, Ambiguous, MovingLayers, LastMove, LbyChanged, Transition, Override, Invalid };

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
