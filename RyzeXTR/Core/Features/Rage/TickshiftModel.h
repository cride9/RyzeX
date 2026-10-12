#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace tickshift {
inline constexpr int MaxNewCommands = 15;
inline constexpr int RechargeTicks = 16;
inline constexpr int HistorySize = 500;
enum class Owner { None, Defensive, HideShot, DoubleTap };
enum class Phase { Off, Prepare, Ready, Commit, Observe, Recover };

// Queued commands affect correction; only executed commands advance the result.
inline int StartTickbase(int oldTickbase, int serverTick, int queued, int correction,
    int simTicksThisFrame = 1, bool multiplayer = true) {
    if (queued <= 0) return oldTickbase;
    if (!multiplayer) return serverTick - queued + simTicksThisFrame;
    const int ideal = serverTick + correction;
    const int estimated = oldTickbase + queued;
    if (estimated > ideal + correction || estimated < ideal - correction)
        return ideal - queued + simTicksThisFrame;
    return oldTickbase;
}
inline int ShiftCapacity(int processLimit, int realCount, int stored) {
    const int budget = processLimit == 0 ? MaxNewCommands : processLimit;
    return (std::max)(0, (std::min)({MaxNewCommands - realCount, budget - realCount, stored}));
}
struct CommandTime {
    int command = 0;
    std::uint32_t epoch = 0;
    int tickbase = 0;
    bool committed = false;
};
class Ledger {
public:
    std::uint32_t epoch = 1;
    void Reset() {
        if (++epoch == 0) epoch = 1;
        records = {};
    }
    void Put(int command, int tickbase, bool committed = false) {
        if (command > 0) records[command % HistorySize] = {command, epoch, tickbase, committed};
    }
    const CommandTime* Find(int command) const {
        if (command <= 0) return nullptr;
        const auto& entry = records[command % HistorySize];
        return entry.command == command && entry.epoch == epoch ? &entry : nullptr;
    }
    void DiscardUncommitted() {
        for (auto& entry : records) if (!entry.committed) entry = {};
    }
    void DiscardAfter(int command) {
        for (auto& entry : records) if (entry.command > command) entry = {};
    }
private:
    std::array<CommandTime, HistorySize> records{};
};
class DefensiveScheduler {
public:
    int lastExtra = 0;
    int lastServerTick = 0;
    bool recovering = false;
    void Reset() { lastExtra = lastServerTick = 0; recovering = false; }
    int Next(int serverTick, int realCount) const {
        if (recovering) return 0;
        const int advance = lastExtra ? (std::max)(1, serverTick - lastServerTick) : 1;
        const int next = lastExtra + advance;
        return next <= MaxNewCommands - realCount ? next : 0;
    }
    void Commit(int serverTick, int extra) {
        lastServerTick = serverTick; lastExtra = extra; recovering = extra == 0;
    }
    void Recovered() { Reset(); }
};
struct PacketPlan {
    Owner owner = Owner::None;
    int firstReal = 0, lastReal = 0, realCount = 0, syntheticCount = 0;
    int serverTick = 0, uncertainty = 0, correction = 0;
    int startTickbase = 0, endTickbase = 0, sequence = 0;
    bool serialized = false, queued = false, sent = false;
};
struct SerializationCycle {
    const void* buffer = nullptr;
    int sequence = 0;
    bool replaced = false;
    void Begin(const void* nextBuffer, int nextSequence) {
        buffer = nextBuffer; sequence = nextSequence; replaced = false;
    }
    bool Owns(const void* nextBuffer, int nextSequence) const {
        return replaced && buffer == nextBuffer && sequence == nextSequence;
    }
    void Reset() { *this = {}; }
};
inline bool SendPacket(bool requested, bool force, bool choke, int choked) {
    if (choked >= MaxNewCommands - 1 || force) return true;
    if (choke) return false;
    return requested;
}
}
