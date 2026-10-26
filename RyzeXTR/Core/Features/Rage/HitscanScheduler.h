#pragma once
#include "HitscanMath.h"

namespace hitscan {
struct Policy {
    // Disable for the exhaustive diagnostic reference. No previous damage is
    // reused across scans in either mode.
    bool adaptive = true;
    int firingCalls = 48, preparingCalls = 16, frameCalls = 64;
    double firingMicros = 1250., preparingMicros = 450., frameMicros = 1800.;
};
struct Runtime {
    struct Hint {
        std::uint32_t handle{};
        const void* model{};
        float spawn{}, simulation{};
        hitbox::Vec point{};
        int hitbox{};
        bool valid{};
    };
    struct Probe { std::uint64_t key{}, retry{}, stamp{}; unsigned failures{}; bool valid{}; };
    Policy policy{};
    std::array<Hint, 65> hints{};
    std::array<unsigned, 65> recordCursor{};
    std::array<unsigned, 65> targetVisits{};
    std::array<Probe, 512> failures{};
    std::uint64_t sequence{}, context{};
    int frame = -1, frameUsedCalls{}, previousTarget{};
    double frameUsedMicros{};
    bool firing{};

    void Reset() {
        hints = {}; recordCursor = {}; targetVisits = {}; failures = {};
        sequence = context = 0; frame = -1; frameUsedCalls = previousTarget = 0; frameUsedMicros = 0.;
    }
    void Begin(int frameNumber, std::uint64_t signature, bool ready) {
        if (signature != context) { Reset(); context = signature; }
        ++sequence; firing = ready;
        if (frameNumber != frame) { frame = frameNumber; frameUsedCalls = 0; frameUsedMicros = 0.; }
    }
    int CallLimit() const { return (std::max)(1, firing ? policy.firingCalls : policy.preparingCalls); }
    double TimeLimit() const { return (std::max)(1., firing ? policy.firingMicros : policy.preparingMicros); }
    bool Exhausted(const Statistics& stats) const {
        return policy.adaptive && (stats.damageCalls >= CallLimit() || stats.damageMicros >= TimeLimit()
            || frameUsedCalls >= (std::max)(1, policy.frameCalls)
            || frameUsedMicros >= (std::max)(1., policy.frameMicros));
    }
    DamageQuery Query(float minimum, const Statistics& stats) const {
        DamageQuery query; query.minimumDamage = minimum;
        // Finish the first admitted probe even if it is slow, so a valid wall
        // shot cannot time out every command and become permanently unreachable.
        query.timed = policy.adaptive && stats.damageCalls > 0;
        const double remaining = (std::min)(TimeLimit() - stats.damageMicros,
            (std::max)(1., policy.frameMicros) - frameUsedMicros);
        query.deadline = Clock::now() + std::chrono::microseconds(static_cast<long long>((std::max)(1., remaining)));
        return query;
    }
    void Charge(double micros) { ++frameUsedCalls; frameUsedMicros += micros; }
    bool Deferred(std::uint64_t key) const {
        const auto& entry = failures[key % failures.size()];
        return policy.adaptive && entry.valid && entry.key == key && sequence < entry.retry;
    }
    void Observe(std::uint64_t key, bool acceptable, Failure reason) {
        if (!policy.adaptive || reason == Failure::Invalid || reason == Failure::Deadline) return;
        auto& entry = failures[key % failures.size()];
        if (acceptable) { if (entry.key == key) entry = {}; return; }
        const unsigned count = entry.valid && entry.key == key && sequence - entry.stamp < 16
            ? (std::min)(entry.failures + 1, 3u) : 1u;
        // First miss retries next scan. Repeated misses wait at most one
        // firing scan or three preparation scans; moving inputs change the key.
        const unsigned delay = count == 1 ? 1u : firing ? 2u : (std::min)(count + 1, 4u);
        entry = {key, sequence + delay, sequence, count, true};
    }
};
struct ScanTimer {
    Statistics& stats;
    Clock::time_point start = Clock::now();
    ~ScanTimer() { stats.totalMicros = std::chrono::duration<double, std::micro>(Clock::now() - start).count(); }
};
}
