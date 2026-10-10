#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>

namespace radio_visualizer {
    using Clock = std::chrono::steady_clock;
    constexpr int FftSize = 2048;
    constexpr int BinCount = FftSize / 2;
    constexpr int BandCount = 64;
    constexpr float Pi = 3.14159265358979323846f;

    struct Settings {
        bool enabled = false;
        int preset = 0;
        int quality = 1;
        float size = 320.f;
        float position[2] = { .03f, .97f };
        float opacity = .9f;
        float colorA[4] = { .65f, .25f, 1.f, 1.f };
        float colorB[4] = { .1f, .85f, 1.f, 1.f };
        float logoColor[4] = { 1.f, 1.f, 1.f, 1.f };
        float sensitivity = 1.f;
        float smoothing = 1.f;
        float bassReaction = 1.f;
        bool logoPulse = true;
        bool glow = true;
        float glowStrength = .7f;
        bool rings = true;
        float ringStrength = .6f;
        bool particles = true;
        float particleDensity = .6f;
    };

    inline float Bounded(float value, float low, float high, float fallback) {
        return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
    }

    inline void Sanitize(Settings& s) {
        const Settings defaults;
        s.preset = std::clamp(s.preset, 0, 1);
        s.quality = std::clamp(s.quality, 0, 2);
        s.size = Bounded(s.size, 180.f, 640.f, defaults.size);
        for (int axis = 0; axis < 2; ++axis)
            s.position[axis] = Bounded(s.position[axis], 0.f, 1.f, defaults.position[axis]);
        s.opacity = Bounded(s.opacity, .05f, 1.f, defaults.opacity);
        s.sensitivity = Bounded(s.sensitivity, .25f, 3.f, defaults.sensitivity);
        s.smoothing = Bounded(s.smoothing, .25f, 3.f, defaults.smoothing);
        s.bassReaction = Bounded(s.bassReaction, 0.f, 2.f, defaults.bassReaction);
        s.glowStrength = Bounded(s.glowStrength, 0.f, 1.f, defaults.glowStrength);
        s.ringStrength = Bounded(s.ringStrength, 0.f, 1.f, defaults.ringStrength);
        s.particleDensity = Bounded(s.particleDensity, 0.f, 1.f, defaults.particleDensity);
        for (int c = 0; c < 4; ++c) {
            s.colorA[c] = Bounded(s.colorA[c], 0.f, 1.f, defaults.colorA[c]);
            s.colorB[c] = Bounded(s.colorB[c], 0.f, 1.f, defaults.colorB[c]);
            s.logoColor[c] = Bounded(s.logoColor[c], 0.f, 1.f, defaults.logoColor[c]);
        }
    }

    inline void ResetPreset(Settings& s) {
        Settings next;
        next.enabled = s.enabled;
        next.preset = s.preset;
        next.quality = s.quality;
        next.size = s.size;
        std::copy_n(s.position, 2, next.position);
        if (s.preset == 1) {
            const float orange[4] = { 1.f, .3f, .08f, 1.f };
            const float pink[4] = { 1.f, .08f, .45f, 1.f };
            std::copy_n(orange, 4, next.colorA);
            std::copy_n(pink, 4, next.colorB);
            next.bassReaction = 1.3f;
            next.smoothing = .7f;
        }
        s = next;
    }

    enum class AnalysisStatus { Waiting, Ready, Stalled, Unsupported };
    struct Frame {
        std::array<float, BinCount> fft{};
        std::uint32_t sampleRate = 0;
        std::uint64_t sequence = 0;
        std::uint64_t generation = 0;
        Clock::time_point timestamp{};
        AnalysisStatus status = AnalysisStatus::Waiting;
        bool valid = false;
    };

    inline bool IsFresh(const Frame& frame, Clock::time_point now) {
        return frame.valid && frame.sampleRate >= 4000 && frame.sampleRate <= 384000 &&
            now >= frame.timestamp && now - frame.timestamp <= std::chrono::milliseconds(250);
    }

    struct Energies {
        std::array<float, BandCount> bands{};
        float bass = 0.f;
        float peak = 0.f;
    };

    inline Energies Analyze(const Frame& frame) {
        Energies out;
        if (!frame.valid || frame.sampleRate < 4000 || frame.sampleRate > 384000)
            return out;
        const float binHz = static_cast<float>(frame.sampleRate) / FftSize;
        const float upperHz = (std::min)(16000.f, frame.sampleRate * .49f);
        const auto rms = [&](float low, float high) {
            const int first = std::clamp(static_cast<int>(std::floor(low / binHz)), 1, BinCount - 1);
            const int last = std::clamp(static_cast<int>(std::ceil(high / binHz)), first + 1, BinCount);
            float sum = 0.f;
            for (int bin = first; bin < last; ++bin) {
                const float value = Bounded(frame.fft[bin], 0.f, 4.f, 0.f);
                sum += value * value;
            }
            return std::sqrt(sum / (last - first));
        };
        for (int band = 0; band < BandCount; ++band) {
            const float low = 20.f * std::pow(upperHz / 20.f, band / static_cast<float>(BandCount));
            const float high = 20.f * std::pow(upperHz / 20.f, (band + 1) / static_cast<float>(BandCount));
            out.bands[band] = rms(low, high);
            out.peak = (std::max)(out.peak, out.bands[band]);
        }
        out.bass = rms(25.f, 180.f);
        return out;
    }

    inline float Follow(float current, float target, float dt, float attack, float release) {
        const float time = target > current ? attack : release;
        return current + (target - current) * (1.f - std::exp(-dt / time));
    }

    struct Envelope {
        std::array<float, BandCount> bands{};
        float bass = 0.f;
        float reference = .035f;
        float baseline = 0.f;
        float cooldown = 0.f;
        bool beat = false;
        std::uint64_t generation = 0;
        std::uint64_t sequence = 0;

        void Update(const Frame& frame, float dt, const Settings& settings, Clock::time_point now) {
            if (generation != frame.generation) {
                *this = {};
                generation = frame.generation;
            }
            dt = Bounded(dt, 0.f, .25f, 0.f);
            const bool fresh = IsFresh(frame, now);
            const auto energy = fresh ? Analyze(frame) : Energies{};
            reference = Follow(reference, (std::max)(.035f, energy.peak), dt, .025f, 1.f);
            for (int band = 0; band < BandCount; ++band) {
                // Do not normalize silence/noise into a full-strength spectrum.
                const float value = energy.bands[band];
                const float target = value > .0001f ? std::clamp(
                    (20.f * std::log10(value / reference) + 48.f) / 48.f * settings.sensitivity, 0.f, 1.f) : 0.f;
                bands[band] = Follow(bands[band], target, dt, .025f * settings.smoothing, .18f * settings.smoothing);
            }
            const float targetBass = std::clamp(energy.bass / (std::max)(.025f, reference * .55f) * settings.sensitivity, 0.f, 1.f);
            bass = Follow(bass, targetBass, dt, .025f, .18f * settings.smoothing);
            cooldown = (std::max)(0.f, cooldown - dt);
            beat = fresh && frame.sequence != sequence && cooldown <= 0.f &&
                targetBass > .32f && targetBass > baseline * 1.35f + .08f;
            baseline = Follow(baseline, targetBass, dt, .2f, .6f);
            if (beat) cooldown = .18f;
            sequence = frame.sequence;
        }
    };

    struct Quality { int points, particles, glowLayers; };
    inline Quality GetQuality(int quality) {
        constexpr Quality levels[] = { {128, 48, 2}, {256, 96, 3}, {384, 160, 4} };
        return levels[std::clamp(quality, 0, 2)];
    }

    // Symmetry is deliberate; a smooth Nation contour and sharp Gutter contour use the same real bands.
    inline float ContourRadius(const Envelope& audio, const Settings& settings, float angle) {
        const float coordinate = std::abs(std::sin(angle)) * (BandCount - 1);
        const int first = std::clamp(static_cast<int>(coordinate), 0, BandCount - 1);
        const int next = (std::min)(first + 1, BandCount - 1);
        float value = audio.bands[first];
        if (settings.preset == 0) {
            const auto smoothBand = [&](int index) {
                float sum = 0.f;
                for (int offset = -2; offset <= 2; ++offset)
                    sum += audio.bands[std::clamp(index + offset, 0, BandCount - 1)] * (3 - std::abs(offset));
                return sum / 9.f;
            };
            value = smoothBand(first);
            value += (smoothBand(next) - value) * (coordinate - first);
        }
        const float pulse = audio.bass * settings.bassReaction;
        if (settings.preset == 0) {
            const float ears = std::exp(-std::pow((std::abs(std::cos(angle)) - .67f) / .19f, 2.f)) *
                (std::max)(0.f, -std::sin(angle));
            return 1.f + pulse * .10f + value * .32f + ears * pulse * .38f;
        }
        return 1.f + pulse * .12f + std::pow(value, 1.7f) * .58f;
    }
}
