#pragma once
#include "HitboxGeometry.h"
#include <cstdint>
#include <limits>

namespace adaptiveaa {
inline float Yaw(float angle) {
    if (!std::isfinite(angle)) return 0.f;
    return std::remainder(angle, 360.f);
}
inline float Bound(float minYaw, float maxYaw, float run, float transition, float duck, float crouch, int side) {
    if (!std::isfinite(minYaw) || !std::isfinite(maxYaw) || !std::isfinite(run) || !std::isfinite(transition)
        || !std::isfinite(duck) || !std::isfinite(crouch)) return 0.f;
    const float width = std::lerp(1.f, std::lerp(.8f, .5f, std::clamp(transition, 0.f, 1.f)), std::clamp(run, 0.f, 1.f));
    const float scaled = std::lerp(width, .5f, std::clamp(duck, 0.f, 1.f) * std::clamp(crouch, 0.f, 1.f));
    return std::clamp((side < 0 ? -minYaw : maxYaw) * scaled, 0.f, 60.f);
}
inline hitbox::Vec Rotate(hitbox::Vec p, hitbox::Vec pivot, double degrees) {
    const double a = degrees * .017453292519943295;
    const auto d = p - pivot;
    return pivot + hitbox::Vec{d.x * std::cos(a) - d.y * std::sin(a), d.x * std::sin(a) + d.y * std::cos(a), d.z};
}
inline hitbox::Shape RotateShape(hitbox::Shape shape, hitbox::Vec pivot, double degrees) {
    shape.a = Rotate(shape.a, pivot, degrees); shape.b = Rotate(shape.b, pivot, degrees);
    for (auto& axis : shape.axes) axis = Rotate(axis, {}, degrees);
    return shape;
}
struct Core {
    hitbox::Vec point{};
    bool valid{}, common{};
};
// A rigid yaw-family *proxy*, not a claim that local fake matrices describe
// every server resolver hypothesis. Its useful output is a cover probe target.
inline Core EstimateCore(const hitbox::Shape& head, hitbox::Vec pivot, float negative, float positive) {
    Core result;
    if (!head.valid || !head.Center().Valid() || !pivot.Valid() || !std::isfinite(negative) || !std::isfinite(positive)
        || negative < 0.f || positive < 0.f) return result;
    std::array<hitbox::Shape, hitbox::MaxShapes> family;
    for (std::size_t i = 0; i < family.size(); ++i) {
        const double yaw = -negative + (negative + positive) * double(i) / double(family.size()-1);
        family[i] = RotateShape(head, pivot, yaw);
    }
    result.valid = true; result.point = head.Center();
    result.common = hitbox::FindIntersection(family.data(), family.size(), head.Center(), result.point);
    return result;
}
struct Candidate {
    float offset{}, exposure = 1.f;
};
enum class Profile { Adaptive, ExtendedHold, ExtendedPulse, ExtendedCover, ExtendedHybrid };
inline bool UsesAdaptiveChoice(Profile profile, bool stationary) {
    return profile == Profile::Adaptive || (profile == Profile::ExtendedHybrid && !stationary);
}
inline bool UsesCover(Profile profile) {
    return profile == Profile::Adaptive || profile == Profile::ExtendedCover || profile == Profile::ExtendedHybrid;
}
struct LowerBodyWindow {
    bool stationary{}, due{}, preBreak{};
    int cycle{};
};
// An estimate from the current command clock, not the server-only timer.
// Merely querying an expired deadline must not consume a break request.
class LowerBodyClock {
public:
    void Reset() { *this = {}; }
    LowerBodyWindow Prepare(int command, float time, float interval, float speed, bool ground) {
        if (command <= 0 || !std::isfinite(time) || !std::isfinite(interval) || interval <= 0.f || interval > .1f
            || !std::isfinite(speed) || speed < 0.f) { Reset(); return {}; }
        if (command == prepared) return window;
        if (command < prepared || (initialized && (time < lastTime - interval * 2.f || time - lastTime > 2.f))) Reset();
        prepared = command;
        // Preserve the legacy immediate idle attempt on a fresh lifecycle.
        // Actual ground movement below establishes the .22 second deadline.
        if (!initialized) { next = time - interval; initialized = true; }
        // Airborne time alone does not reset the server's idle deadline.
        // Ground movement refreshes it on every animation update.
        if (ground && speed > .1f) next = time + .22f;
        lastTime = time;
        window = {ground && speed <= .1f, false, false, cycles};
        window.due = window.stationary && time > next;
        window.preBreak = window.stationary && !window.due && time + interval > next;
        return window;
    }
    bool CommitBreak(int command, float time, bool hidden) {
        if (!hidden || command != prepared || command == committed || !window.due || !std::isfinite(time)) return false;
        next = time + 1.1f; committed = command; ++cycles;
        return true;
    }
    float Deadline() const { return next; }
private:
    int prepared{}, committed{}, cycles{};
    float next{}, lastTime{};
    bool initialized{};
    LowerBodyWindow window{};
};
struct Input {
    int command{}, movement{};
    float baseYaw{}, pitch = 89.f, negative = 58.f, positive = 58.f;
    int inverter = 1;
    bool protectedCommand{}, confirmedDefensive{}, autoYaw{}, defensivePitch{};
    Profile profile = Profile::Adaptive;
    bool stationary{}, lowerBodyDue{}, lowerBodyPre{}, legacyExtended{};
    int lowerBodyCycle{};
};
struct Output {
    float yaw{}, pitch{}, realYaw{}, fakeYaw{};
    int side = 1;
    bool apply{};
    bool hiddenBreak{}, hideFlick{};
};
struct Statistics {
    int traces{}, threat{}, side = 1, phase{}, packets{}, reactions{};
    float coreExposure = 1.f, chosenExposure = 1.f, offset{};
    bool validHead{}, commonCore{}, defensiveConfirmed{};
    int mode{}, lowerBodyCycle{};
    bool lowerBodyDue{}, hiddenBreak{};
};
class Controller {
public:
    void Reset(std::uint32_t seed = 1) { *this = {}; random = seed ? seed : 1; }
    void SetCover(const std::array<Candidate, 5>& next) {
        candidates = next;
        for (auto& c : candidates) {
            c.offset = std::clamp(Yaw(c.offset), -90.f, 90.f);
            c.exposure = std::isfinite(c.exposure) ? std::clamp(c.exposure, 0.f, 1.f) : 1.f;
        }
        // A material cover change can shorten the old dwell, but noisy traces
        // cannot flip the body side on every prediction command.
        int best = Best();
        if (candidates[best].exposure + .18f < candidates[selected].exposure) dwell = 0;
    }
    bool React(int command) {
        if (command <= lastReaction || static_cast<long long>(command)-lastReaction < 12) return false;
        lastReaction = command; reaction = true; dwell = 0;
        return true;
    }
    Output Evaluate(const Input& in, bool sendPacket) {
        if (in.protectedCommand || in.command <= 0 || in.command < prepared) return {};
        if (prepared != in.command) {
            prepared = in.command;
            if (!dwell || in.movement != movement || in.profile != profile || in.stationary != stationary
                || in.lowerBodyCycle != cycle || reaction) Choose(in);
            movement = in.movement;
            profile = in.profile; stationary = in.stationary; cycle = in.lowerBodyCycle;
            pitchPhase = Next() % 3;
        }
        Output out; out.apply = true;
        out.side = side * (in.inverter < 0 ? -1 : 1);
        out.pitch = in.confirmedDefensive && in.defensivePitch
            ? std::array<float,3>{89.f, -45.f, 12.f}[pitchPhase] : std::clamp(in.pitch, -89.f, 89.f);
        const float bound = out.side < 0 ? in.negative : in.positive;
        const float desync = std::isfinite(bound) ? 2.f * std::clamp(bound, 0.f, 60.f) : 0.f;
        float delta = desync * out.side;
        if (in.profile != Profile::Adaptive && (in.stationary || in.legacyExtended)) {
            // The server also requires a >35 degree eye/feet separation.
            // A narrower animation yaw limit cannot establish that here.
            const bool breaking = in.stationary && in.lowerBodyDue && bound > 35.f;
            out.hiddenBreak = !sendPacket && breaking;
            const bool pre = in.profile == Profile::ExtendedPulse && in.lowerBodyPre;
            out.hideFlick = !sendPacket && (breaking || pre);
            // Keep the known legacy 120/95 angles as the stable baseline.
            // Pulse changes only the hidden command and LBY-cycle side.
            const float hold = in.profile == Profile::ExtendedPulse
                ? std::array<float,3>{95.f,78.f,108.f}[static_cast<unsigned>(in.lowerBodyCycle)%3] : 95.f;
            delta = (breaking ? -120.f : pre ? -100.f : hold) * out.side;
        }
        // The configured yaw is real on a sent command. Only an explicit
        // auto-yaw opt-in may rotate that base; desync affects choked commands.
        out.realYaw = Yaw(in.baseYaw + (in.autoYaw ? offset : 0.f));
        out.fakeYaw = Yaw(out.realYaw + delta);
        out.yaw = sendPacket ? out.realYaw : out.fakeYaw;
        return out;
    }
    void Commit(int command, bool sent) {
        if (!sent || command != prepared || command <= committed) return;
        committed = command; ++packets;
        if (dwell) --dwell;
        reaction = false;
    }
    float Exposure() const { return candidates[selected].exposure; }
    float Offset() const { return offset; }
    int Side() const { return side; }
    int Phase() const { return selected; }
    int Packets() const { return packets; }
private:
    std::array<Candidate,5> candidates{{{-62,1}, {-29,1}, {0,1}, {34,1}, {67,1}}};
    std::uint32_t random = 1;
    int prepared{}, committed{}, packets{}, movement = -1, selected = 2, dwell{}, side = 1;
    int lastReaction = -100000, pitchPhase{};
    float offset{};
    bool reaction{};
    Profile profile = Profile::Adaptive;
    bool stationary{};
    int cycle{};
    std::uint32_t Next() { random ^= random << 13; random ^= random >> 17; random ^= random << 5; return random; }
    int Best() const {
        int best = 0;
        for (int i=1;i<5;++i) if (candidates[i].exposure < candidates[best].exposure) best=i;
        return best;
    }
    void Choose(const Input& in) {
        if (!UsesAdaptiveChoice(in.profile,in.stationary)) {
            if (!UsesCover(in.profile)) {
                selected=2; offset=0.f;
                if (in.profile == Profile::ExtendedPulse) {
                    if (in.lowerBodyCycle != cycle || in.profile != profile) side=(in.lowerBodyCycle/2)&1 ? -1 : 1;
                    if (reaction) side=-side;
                } else side=1;
            } else {
                const int best=Best();
                // Preserve a good sector until a material improvement appears.
                if (candidates[selected].exposure >= .35f || candidates[best].exposure + .10f < candidates[selected].exposure)
                    selected=best;
                if (reaction) {
                    int alternate=-1;
                    for (int i=0;i<5;++i) if (i!=selected && candidates[i].exposure <= candidates[best].exposure+.10f
                        && (alternate<0 || candidates[i].exposure<candidates[alternate].exposure)) alternate=i;
                    if (alternate>=0) selected=alternate;
                }
                offset=candidates[selected].offset;
                if (offset < -15.f) side=-1;
                else if (offset > 15.f) side=1;
                else if (reaction) side=-side;
            }
            dwell=4; reaction=false;
            return;
        }
        const int previous = selected, best = Best();
        std::array<int,5> eligible{}; unsigned count=0;
        for (int i=0;i<5;++i)
            if (candidates[i].exposure <= candidates[best].exposure + .10f && (i != previous || candidates[best].exposure < .35f))
                eligible[count++]=i;
        selected = count ? eligible[Next()%count] : best;
        // Add bounded variation only when it cannot move a protected head out
        // of a narrow cover sector. Cover wins over arbitrary jitter size.
        const float variance = candidates[selected].exposure < .35f ? 0.f : float(int(Next()%11)-5);
        offset = candidates[selected].offset + variance;
        if (reaction) side = -side;
        else if (offset < -15) side=-1;
        else if (offset > 15) side=1;
        else if ((Next()&3u)==0) side=-side;
        dwell = candidates[selected].exposure < .35f ? 2 + Next()%3 : 1 + Next()%3;
        reaction = false;
    }
};
}
