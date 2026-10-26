#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

namespace backtrack {
struct Context {
    float interval{}, lerp{}, latency{}, maxUnlag{}, shotTime{}, historyTime{};
};
struct Evaluation {
    int commandTick = -1;
    float lerp{}, requestedTime{}, roundingError{}, correction{}, delta{};
    bool valid{}, historyExpired{}, fallback{};
};
// Match the server's float-time calculation after encoding tick_count.
// History is trimmed in whole seconds, so its cutoff must also be compared
// against simulation seconds, never an encoded tick count.
inline Evaluation Evaluate(float simulation, const Context& ctx, float window = .2f) {
    Evaluation result;
    if (!std::isfinite(simulation) || simulation < 0.f || !std::isfinite(ctx.interval) || ctx.interval <= 0.f
        || !std::isfinite(ctx.lerp) || ctx.lerp < 0.f || !std::isfinite(ctx.latency) || ctx.latency < 0.f
        || !std::isfinite(ctx.maxUnlag) || ctx.maxUnlag < 0.f || !std::isfinite(ctx.shotTime)
        || !std::isfinite(ctx.historyTime) || !std::isfinite(window) || window <= 0.f) return result;
    const float encoded = .5f + (simulation + ctx.lerp) / ctx.interval;
    const float cutoff = ctx.historyTime - ctx.maxUnlag;
    if (!std::isfinite(encoded) || encoded < 0.f || static_cast<double>(encoded) >= (std::numeric_limits<int>::max)()
        || !std::isfinite(cutoff) || static_cast<double>(cutoff) <= (std::numeric_limits<int>::min)()
        || static_cast<double>(cutoff) >= (std::numeric_limits<int>::max)()) return result;
    result.commandTick = static_cast<int>(encoded);
    result.lerp = ctx.lerp;
    result.requestedTime = result.commandTick * ctx.interval - ctx.lerp;
    result.roundingError = result.requestedTime - simulation;
    result.correction = std::clamp(ctx.latency + ctx.lerp, 0.f, ctx.maxUnlag);
    result.delta = result.correction - (ctx.shotTime - result.requestedTime);
    result.historyExpired = simulation < static_cast<float>(static_cast<int>(cutoff));
    result.fallback = std::fabs(result.delta) > (std::min)(.2f, window);
    result.valid = !result.historyExpired && !result.fallback;
    return result;
}
}
