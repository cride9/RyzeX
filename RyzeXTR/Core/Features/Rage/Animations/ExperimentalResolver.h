#pragma once
#include "Lagcompensation.h"
#include "ExperimentalResolverMath.h"
#include <cstdint>

struct rageBotData_t;
class IGameEvent;

class ExperimentalResolver {
public:
    // Called by the existing enemy animfix. Only commits the selected foot yaw.
    void Resolve(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
                 const Lagcompensation::LagRecord_t* previous, Lagcompensation::AnimationInfo_t* log);
    // Additional safepoint hypotheses use the existing SetupPlayerMatrix path.
    void BuildSafePoints(CBaseEntity* entity, Lagcompensation::LagRecord_t* record);
    void RecordShot(const rageBotData_t& data, int command);
    void HandleGameEvent(IGameEvent* event);
    void ProcessShots();
    bool IsRejected(const Lagcompensation::LagRecord_t* record) const;
    static bool IsPredicting() { return predicting; }
    void Reset(int index);
    void ResetAll();

private:
    struct Candidate {
        CAnimState state{};
        CAnimationLayer layers[13]{};
        float poses[24]{};
        // ActivityModifiers is an owning CUtlVector<CUtlSymbol> in the engine.
        // Never retain its pointers in a copied animstate.
        std::vector<unsigned short> modifiers;
        bool valid{};
    };
    struct History {
        std::array<Candidate, 9> candidates;
        Candidate baseline;
        experimental::Evidence evidence;
        CBaseHandle entityHandle{}, weaponHandle{};
        const Model_t* model{};
        float spawnTime{}, simulationTime{};
        bool ready{};
    };
    std::array<History, 65> histories;
    struct Control {
        experimental::Recovery recovery;
        experimental::Standing standing;
        CBaseHandle entityHandle{};
        const Model_t* model{};
        float spawnTime{}, eyeYaw{};
        std::uint64_t epoch{};
        bool ready{}, moving{};
    };
    std::array<Control, 65> controls;
    std::uint64_t nextEpoch{};
    inline static bool predicting{};
    static void ResetShots(int index);

    static bool Capture(CBaseEntity* entity, Candidate& candidate);
    static bool Install(CBaseEntity* entity, const Candidate& candidate);
    static void ReleaseModifiers(CAnimState* state);
    static bool Replay(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
                       const Lagcompensation::LagRecord_t* previous, Candidate& candidate,
                       float fraction, bool constrained, bool warmup, bool manual = false, float manualYaw = 0.f);
    static bool BuildMatrix(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
                            float yaw, matrix3x4a_t* output);
};

inline ExperimentalResolver experimentalResolver;
