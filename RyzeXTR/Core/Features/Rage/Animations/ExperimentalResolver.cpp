#include "ExperimentalResolver.h"
#include "EnemyAnimations.h"
#include "../../../SDK/Menu/config.h"
#include "../StudioHitboxGeometry.h"
#include "ExperimentalShotFeedback.h"
#include "../aimbot.h"
#include "../../../Lua/Lua.h"
#include <memory>
#include <format>
#include <string_view>
#include <cstring>

namespace {
struct ShotPayload {
    // Lag records can be erased before the server returns the shot result.
    // Own one immutable copy per actual queued shot, not a deque element pointer.
    std::unique_ptr<Lagcompensation::LagRecord_t> record;
    rageBotData_t data;
    const Model_t* model{};
};
experimental::ShotQueue<ShotPayload> shotQueue;
// The Win32 CUtlVector layout already used by the SDK. Values are copied out;
// the original engine allocation is never shared by speculative updates.
struct ModifierVector {
    unsigned short* memory;
    int capacity, growSize, size;
    unsigned short* elements;
};
static_assert(sizeof(ModifierVector) == sizeof(CAnimState::ActivityModifiers));

ModifierVector ReadModifiers(const CAnimState& state) {
    ModifierVector result{};
    std::memcpy(&result, state.ActivityModifiers, sizeof(result));
    return result;
}

experimental::Layer Layer(const CAnimationLayer& layer) {
    return { static_cast<int>(layer.nSequence), layer.flPlaybackRate, layer.flCycle, layer.flWeight };
}

// Restore the live entity after each hypothesis, including the animfix's bone
// cache. Only Resolve's final foot-yaw assignment becomes live state.
class EntitySnapshot {
public:
    explicit EntitySnapshot(CBaseEntity* player) : entity(player), state(*player->AnimState()), globals(*i::GlobalVars),
        origin(player->GetVecOrigin()), absOrigin(player->GetAbsOrigin()), angles(player->GetAbsAngles()),
        eye(player->GetEyeAngles()), velocity(player->GetVelocity()), absVelocity(player->GetVecAbsVelocity()),
        flags(player->GetFlags()), eflags(player->GetEFlags()), effects(player->GetEffects()),
        clientEffects(player->GetClientEffects()), occlusion(player->GetOcclusionFlags()),
        occlusionFrame(player->GetOcclusionFrameCount()), skipFrame(player->GetLastSkipFrameCount()),
        sequence(player->GetSequence()), cycle(player->GetCycle()), duck(player->GetDuckAmount()),
        lby(player->GetLowerBodyYaw()), recoil(player->GetThirdpersonRecoil()),
        simTime(player->GetSimulationTime()), clientAnimation(player->IsClientSideAnimation()),
        jiggle(player->JiggleBones()), ik(player->GetIKContext()), accessor(player->GetBoneAccessor()),
        boneTime(player->GetLastSetupBonesTime()), boneCounter(player->GetRecentModelBoneCounter()),
        allow(g::bAllowAnimations[player->EntIndex()]), settingUp(g::bSettingUpBones[player->EntIndex()]),
        lod(player->GetAnimationLODFlags()), oldLod(player->GetOldAnimationLODFlags()),
        lodFrame(player->GetComputedAnimationLODFrame()), maintainTransition(player->MaintainSequenceTransition()),
        collisionTime(player->GetCollisionChangeTime()), collisionOrigin(player->GetCollisionChangeOrigin()) {
        entity->GetAnimationLayers(layers);
        entity->GetPoseParameters(poses);
        auto& cache = entity->GetCachedBoneData();
        cacheBase = cache.Base();
        if (cache.Base() && cache.Count() > 0 && cache.Count() <= MAXSTUDIOBONES)
            bones.assign(cache.Base(), cache.Base() + cache.Count());
        if (auto collideable = entity->GetCollideable()) {
            mins = collideable->OBBMins();
            maxs = collideable->OBBMaxs();
        }
    }
    ~EntitySnapshot() { Restore(); }
    void Restore() {
        *entity->AnimState() = state;
        entity->SetAnimationLayers(layers);
        entity->SetPoseParameters(poses);
        entity->GetVecOrigin() = origin;
        entity->SetAbsOrigin(absOrigin);
        entity->SetAbsAngles(angles);
        entity->GetEyeAngles() = eye;
        entity->GetVelocity() = velocity;
        entity->GetVecAbsVelocity() = absVelocity;
        entity->GetFlags() = flags;
        entity->GetEFlags() = eflags;
        entity->GetEffects() = effects;
        entity->GetClientEffects() = clientEffects;
        entity->GetOcclusionFlags() = occlusion;
        entity->GetOcclusionFrameCount() = occlusionFrame;
        entity->GetLastSkipFrameCount() = skipFrame;
        entity->GetSequence() = sequence;
        entity->GetCycle() = cycle;
        entity->GetDuckAmount() = duck;
        entity->GetLowerBodyYaw() = lby;
        entity->GetThirdpersonRecoil() = recoil;
        entity->GetSimulationTime() = simTime;
        entity->IsClientSideAnimation() = clientAnimation;
        entity->JiggleBones() = jiggle;
        entity->GetIKContext() = ik;
        entity->GetBoneAccessor() = accessor;
        auto& cache = entity->GetCachedBoneData();
        if (!bones.empty() && cache.Base() && cache.Count() == static_cast<int>(bones.size())) {
            // SetupBones may reallocate the cache. Do not restore a stale pointer.
            if (accessor.matBones == cacheBase) entity->GetBoneAccessor().matBones = cache.Base();
            std::memcpy(cache.Base(), bones.data(), bones.size() * sizeof(matrix3x4a_t));
            entity->GetLastSetupBonesTime() = boneTime;
            entity->GetRecentModelBoneCounter() = boneCounter;
        } else {
            if (accessor.matBones == cacheBase) entity->GetBoneAccessor().matBones = cache.Base();
            entity->InvalidateBoneCache();
        }
        // InvalidateBoneCache also writes occlusion fields on this client.
        entity->GetOcclusionFlags() = occlusion;
        entity->GetOcclusionFrameCount() = occlusionFrame;
        entity->GetAnimationLODFlags() = lod;
        entity->GetOldAnimationLODFlags() = oldLod;
        entity->GetComputedAnimationLODFrame() = lodFrame;
        entity->MaintainSequenceTransition() = maintainTransition;
        entity->GetCollisionChangeTime() = collisionTime;
        entity->GetCollisionChangeOrigin() = collisionOrigin;
        if (auto collideable = entity->GetCollideable()) {
            collideable->OBBMins() = mins;
            collideable->OBBMaxs() = maxs;
        }
        g::bAllowAnimations[entity->EntIndex()] = allow;
        g::bSettingUpBones[entity->EntIndex()] = settingUp;
        *i::GlobalVars = globals;
    }
private:
    CBaseEntity* entity;
    CAnimState state;
    CGlobalVarsBase globals;
    CAnimationLayer layers[13];
    float poses[24];
    std::vector<matrix3x4a_t> bones;
    Vector origin, absOrigin, angles, eye, velocity, absVelocity;
    int flags, eflags, effects, clientEffects, occlusion, occlusionFrame, skipFrame, sequence;
    float cycle, duck, lby, recoil, simTime;
    bool clientAnimation, jiggle;
    IKContext* ik;
    CBoneAccessor accessor;
    float boneTime;
    unsigned long boneCounter;
    bool allow, settingUp;
    unsigned int lod, oldLod, lodFrame;
    bool maintainTransition;
    float collisionTime, collisionOrigin;
    Vector mins{}, maxs{};
    matrix3x4a_t* cacheBase{};
};

void SetTime(float time) {
    i::GlobalVars->flCurrentTime = i::GlobalVars->flRealTime = time;
    i::GlobalVars->flFrameTime = i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
    i::GlobalVars->iFrameCount = i::GlobalVars->iTickCount = TIME_TO_TICKS(time);
    i::GlobalVars->flInterpolationAmount = 0.f;
}

float Width(const CAnimState& state) {
    return experimental::YawWidth(state.flRunningSpeed, state.flDuckingSpeed,
                                   state.flWalkToRunTransition, state.flDuckAmount);
}

float Offset(const CAnimState& state, float fraction) {
    const float bound = fraction < 0.f ? -state.flMinBodyYaw : state.flMaxBodyYaw;
    return fraction * std::clamp(std::isfinite(bound) ? bound : 0.f, 0.f, 58.f) * Width(state);
}

struct PredictionScope {
    bool& active;
    bool previous;
    explicit PredictionScope(bool& flag) : active(flag), previous(flag) { active = true; }
    ~PredictionScope() { active = previous; }
};
}

void ExperimentalResolver::Reset(int index) {
    if (index >= 0 && index < static_cast<int>(histories.size())) {
        histories[index] = {};
        controls[index] = {};
    }
}

void ExperimentalResolver::ResetAll() {
    for (int index = 0; index < static_cast<int>(histories.size()); ++index) Reset(index);
    ResetShots(-1);
}

void ExperimentalResolver::ResetShots(int index) { shotQueue.Reset(index); }

bool ExperimentalResolver::IsRejected(const Lagcompensation::LagRecord_t* record) const {
    if (!record || record->iResolverMode != static_cast<int>(resolver::Mode::Experimental)
        || record->iEntIndex < 1 || record->iEntIndex >= 65
        || record->iExperimentalReason == static_cast<int>(experimental::Reason::Override)) return false;
    const auto& control = controls[record->iEntIndex];
    return control.ready && control.epoch == record->uExperimentalEpoch
        && control.recovery.Blocked(record->flResolveDelta);
}

bool ExperimentalResolver::Capture(CBaseEntity* entity, Candidate& candidate) {
    candidate.state = *entity->AnimState();
    const auto modifiers = ReadModifiers(candidate.state);
    if (modifiers.size < 0 || modifiers.size > 64 || modifiers.capacity < modifiers.size
        || (modifiers.size && !modifiers.memory)) {
        candidate.valid = false;
        return false;
    }
    candidate.modifiers.clear();
    if (modifiers.size) candidate.modifiers.assign(modifiers.memory, modifiers.memory + modifiers.size);
    std::memset(candidate.state.ActivityModifiers, 0, sizeof(candidate.state.ActivityModifiers));
    entity->GetAnimationLayers(candidate.layers);
    entity->GetPoseParameters(candidate.poses);
    candidate.valid = std::isfinite(candidate.state.flGoalFeetYaw)
        && std::isfinite(candidate.state.flLastUpdateTime) && Layer(candidate.layers[6]).Valid();
    for (float pose : candidate.poses) candidate.valid &= std::isfinite(pose);
    return candidate.valid;
}

bool ExperimentalResolver::Install(CBaseEntity* entity, const Candidate& candidate) {
    ModifierVector modifiers{};
    if (!candidate.modifiers.empty()) {
        modifiers.size = modifiers.capacity = static_cast<int>(candidate.modifiers.size());
        modifiers.memory = static_cast<unsigned short*>(i::MemAlloc->Alloc(modifiers.size * sizeof(unsigned short)));
        if (!modifiers.memory) return false;
        modifiers.elements = modifiers.memory;
        std::memcpy(modifiers.memory, candidate.modifiers.data(), modifiers.size * sizeof(unsigned short));
    }
    *entity->AnimState() = candidate.state;
    std::memcpy(entity->AnimState()->ActivityModifiers, &modifiers, sizeof(modifiers));
    entity->SetAnimationLayers(const_cast<CAnimationLayer*>(candidate.layers));
    entity->SetPoseParameters(const_cast<float*>(candidate.poses));
    for (int layer = 0; layer < 13; ++layer) entity->GetAnimationOverlays()[layer].pOwner = entity;
    return true;
}

void ExperimentalResolver::ReleaseModifiers(CAnimState* state) {
    const auto modifiers = ReadModifiers(*state);
    if (modifiers.memory && modifiers.growSize >= 0) i::MemAlloc->Free(modifiers.memory);
    std::memset(state->ActivityModifiers, 0, sizeof(state->ActivityModifiers));
}

bool ExperimentalResolver::Replay(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
    const Lagcompensation::LagRecord_t* previous, Candidate& candidate, float fraction,
    bool constrained, bool warmup, bool manual, float manualYaw) {
    EntitySnapshot restore(entity);
    if (!Install(entity, candidate)) return false;
    PredictionScope prediction(predicting);
    auto state = entity->AnimState();
    const float interval = i::GlobalVars->flIntervalPerTick;
    // The existing animfix owns record->iChoked and velocity. The prediction
    // bank uses a bounded local time step without changing either input.
    const int ticks = previous ? std::clamp(TIME_TO_TICKS(record->flSimulationTime - previous->flSimulationTime), 1, 16) : 1;
    const float start = previous ? previous->flSimulationTime : record->flSimulationTime - interval;

    // Warm starts have no trustworthy client history. Seed the observable phase
    // and independent movement directions, then require later records to agree.
    if (warmup) {
        state->vecPositionCurrent = state->vecPositionLast = record->vecOrigin;
        state->flFeetCycle = record->arrLayers[6].flCycle;
        state->flMoveWeight = record->arrLayers[6].flWeight;
        state->flMoveWeightSmoothed = state->flMoveWeight;
        state->flDurationInAir = 0.f;
        state->bOnGround = (record->iFlags & FL_ONGROUND) != 0;
        state->flDuckAmount = record->flDuck;
        state->flTimeToAlignLowerBody = record->flSimulationTime + .22f;
    }
    // Landing/jump timing is only approximate; these records are excluded from
    // side evidence. Guard rates before deriving an event time.
    int eventTick = ticks;
    if (previous && ((previous->iFlags ^ record->iFlags) & FL_ONGROUND)) {
        const int layer = (record->iFlags & FL_ONGROUND) ? 5 : 4;
        const auto received = Layer(record->arrLayers[layer]);
        if (received.Valid() && received.rate > .00001f)
            eventTick = std::clamp(TIME_TO_TICKS(record->flSimulationTime - received.cycle / received.rate - start), 1, ticks);
    }

    for (int tick = 1; tick <= ticks; ++tick) {
        const float alpha = static_cast<float>(tick) / ticks;
        const bool final = tick == ticks;
        const float time = final ? record->flSimulationTime : start + tick * interval;
        SetTime(time);
        entity->GetSimulationTime() = time;
        entity->GetVecOrigin() = previous ? previous->vecOrigin + (record->vecOrigin - previous->vecOrigin) * alpha : record->vecOrigin;
        entity->SetAbsOrigin(entity->GetVecOrigin());
        entity->GetVelocity() = previous ? previous->vecVelocity + (record->vecVelocity - previous->vecVelocity) * alpha : record->vecVelocity;
        entity->GetVecAbsVelocity() = entity->GetVelocity();
        entity->GetDuckAmount() = previous ? previous->flDuck + (record->flDuck - previous->flDuck) * alpha : record->flDuck;
        const bool useCurrent = final || !previous || (record->bDidShot && time >= record->flLastShotTime);
        entity->GetEyeAngles() = useCurrent ? record->vecEyeAngles : previous->vecEyeAngles;
        entity->GetLowerBodyYaw() = useCurrent ? record->flLowerBodyYawTarget : previous->flLowerBodyYawTarget;
        entity->GetThirdpersonRecoil() = useCurrent ? record->flThirdPersonRecoil : previous->flThirdPersonRecoil;
        entity->GetFlags() = !previous || tick >= eventTick ? record->iFlags : previous->iFlags;
        entity->GetEFlags() &= ~(EFL_DIRTY_ABSVELOCITY | EFL_DIRTY_ABSTRANSFORM);
        entity->GetEffects() |= EF_NOINTERP;
        entity->GetAnimationLODFlags() = 0;
        entity->GetComputedAnimationLODFrame() = 0;
        state->pEntity = entity;
        state->flLastUpdateTime = time - interval;
        state->iLastUpdateFrame = i::GlobalVars->iFrameCount - 1;
        // Each fraction is a constrained history hypothesis. The engine still
        // applies SetupVelocity's approach/clamp; score and store its actual yaw.
        if (constrained && ((record->vecVelocity.Length2D() > 40.f && (record->iFlags & FL_ONGROUND)) || warmup || manual))
            state->flGoalFeetYaw = experimental::NormalizeYaw(entity->GetEyeAngles().y + (manual ? manualYaw : Offset(*state, fraction)));
        if (warmup && constrained) {
            const float direction = std::atan2(-record->vecVelocity.y, -record->vecVelocity.x) * (180.f / 3.14159265358979323846f);
            state->flMoveYaw = state->flMoveYawIdeal = experimental::NormalizeYaw(direction - state->flGoalFeetYaw);
        }
        const bool allow = g::bAllowAnimations[record->iEntIndex];
        const bool client = entity->IsClientSideAnimation();
        g::bAllowAnimations[record->iEntIndex] = entity->IsClientSideAnimation() = true;
        entity->UpdateClientSideAnimations();
        entity->IsClientSideAnimation() = client;
        g::bAllowAnimations[record->iEntIndex] = allow;
    }
    if (manual) {
        // Respect the existing per-player override exactly without a second
        // animation tick. Rotate movement/body poses with the new foot yaw.
        const float yaw = experimental::NormalizeYaw(record->vecEyeAngles.y + manualYaw);
        const float change = experimental::NormalizeYaw(yaw - state->flGoalFeetYaw);
        state->flGoalFeetYaw = yaw;
        state->flMoveYaw = experimental::NormalizeYaw(state->flMoveYaw - change);
        state->flMoveYawIdeal = experimental::NormalizeYaw(state->flMoveYawIdeal - change);
        auto setPose = [&](int mapping, float value) {
            const auto& pose = state->tPoseParamMappings[mapping];
            if (pose.bInitialized && pose.nIndex >= 0 && pose.nIndex < 24)
                entity->SetPoseParameter(entity->GetModelPtr(), pose.nIndex, value);
        };
        const float delta = experimental::NormalizeYaw(record->vecEyeAngles.y - yaw);
        const float bound = delta >= 0.f ? state->flMaxBodyYaw : -state->flMinBodyYaw;
        const float bodyYaw = bound > .001f ? std::clamp(delta * 60.f / bound, -60.f, 60.f) : 0.f;
        setPose(PLAYER_POSE_PARAM_BODY_YAW, bodyYaw);
        setPose(PLAYER_POSE_PARAM_MOVE_YAW, state->flMoveYaw);
    }
    const bool advanced = std::fabs(state->flLastUpdateTime - record->flSimulationTime) < interval * .1f;
    const bool captured = Capture(entity, candidate);
    ReleaseModifiers(state);
    candidate.valid = advanced && captured;
    return candidate.valid;
}

bool ExperimentalResolver::BuildMatrix(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
    float yaw, matrix3x4a_t* output) {
    EntitySnapshot restore(entity);
    Candidate fixedPose;
    auto& cache = entity->GetCachedBoneData();
    if (!std::isfinite(yaw) || !entity->GetModelPtr() || !cache.Base()
        || cache.Count() <= 0 || cache.Count() > MAXSTUDIOBONES
        || !Capture(entity, fixedPose) || !Install(entity, fixedPose)) return false;
    // Start every safepoint trial from the existing animfix's selected pose.
    // Match its regular side-matrix path: set yaw, refresh poses, build bones.
    entity->AnimState()->flGoalFeetYaw = yaw;
    anims.UpdateClientSideAnimations(entity, record);
    entity->AnimState()->flGoalFeetYaw = yaw;
    entity->SetAbsAngles(Vector(0.f, yaw, 0.f));
    entity->SetAnimationLayers(record->arrLayers);
    std::memset(output, 0, MAXSTUDIOBONES * sizeof(matrix3x4a_t));
    const bool built = anims.SetupPlayerMatrix(entity, record, output, BoneUsedByHitbox);
    ReleaseModifiers(entity->AnimState());
    return built;
}

void ExperimentalResolver::Resolve(CBaseEntity* entity, Lagcompensation::LagRecord_t* record,
    const Lagcompensation::LagRecord_t* previous, Lagcompensation::AnimationInfo_t* log) {
    if (!entity || !record || !log || !entity->AnimState() || !i::MemAlloc
        || record->iEntIndex < 1 || record->iEntIndex >= static_cast<int>(histories.size())) return;
    record->iResolverMode = static_cast<int>(resolver::Mode::Experimental);
    record->iExperimentalReason = static_cast<int>(experimental::Reason::Warmup);
    record->flResolverConfidence = 0.f;
    record->iExperimentalBruteStep = 0;
    record->uExperimentalEpoch = 0;
    auto& history = histories[record->iEntIndex];
    auto& control = controls[record->iEntIndex];
    const float interval = i::GlobalVars->flIntervalPerTick;
    if (!std::isfinite(interval) || interval <= 0.f || !std::isfinite(record->flSimulationTime)
        || record->flSimulationTime < 0.f || record->flSimulationTime / interval >= (std::numeric_limits<int>::max)() - 1024.f
        || !record->vecOrigin.IsValid() || !record->vecEyeAngles.IsValid()
        || !std::isfinite(record->flDuck) || !std::isfinite(record->flLowerBodyYawTarget)
        || !std::isfinite(record->flThirdPersonRecoil)) {
        Reset(record->iEntIndex);
        record->iExperimentalReason = static_cast<int>(experimental::Reason::Invalid);
        return;
    }
    const auto weapon = entity->GetWeapon();
    const CBaseHandle weaponHandle = weapon ? weapon->GetRefEHandle() : 0;
    const float elapsed = previous ? record->flSimulationTime - previous->flSimulationTime : 0.f;
    const int ticks = elapsed > 0.f && elapsed <= interval * 16.1f ? TIME_TO_TICKS(elapsed) : 0;
    const bool continuous = history.ready && previous && previous->iResolverMode == record->iResolverMode
        && !record->bFirstAfterDormant && !record->bBreakingLagcompensation
        && history.entityHandle == entity->GetRefEHandle() && history.weaponHandle == weaponHandle
        && history.model == entity->GetModel() && history.spawnTime == entity->GetSpawnTime()
        && std::fabs(history.simulationTime - previous->flSimulationTime) < interval * .1f
        && ticks >= 1 && ticks <= 16 && std::fabs(elapsed - ticks * interval) < interval * .1f
        && (record->vecOrigin - previous->vecOrigin).Length2DSqr() <= 4096.f;
    const bool moving = (record->iFlags & FL_ONGROUND) && record->vecVelocity.Length2D() > 40.f;
    const bool identityChanged = !control.ready || control.entityHandle != entity->GetRefEHandle()
        || control.model != entity->GetModel() || control.spawnTime != entity->GetSpawnTime();
    const bool poseChanged = control.ready && (moving != control.moving
        || std::fabs(experimental::NormalizeYaw(record->vecEyeAngles.y - control.eyeYaw)) > 60.f);
    if (identityChanged || record->bFirstAfterDormant || record->bBreakingLagcompensation)
        Reset(record->iEntIndex);
    else {
        if (poseChanged) {
            control.recovery = {};
            control.epoch = ++nextEpoch;
            if (std::fabs(experimental::NormalizeYaw(record->vecEyeAngles.y - control.eyeYaw)) > 60.f)
                control.standing = {};
        }
        if (!continuous) history = {}; // Losing prediction history must not erase verified misses.
    }
    if (!control.ready) {
        control.entityHandle = entity->GetRefEHandle(); control.model = entity->GetModel();
        control.spawnTime = entity->GetSpawnTime(); control.epoch = ++nextEpoch;
        control.ready = true;
    }
    control.moving = moving;
    control.eyeYaw = record->vecEyeAngles.y;
    record->uExperimentalEpoch = control.epoch;
    const auto prior = continuous ? previous : nullptr;
    const float speed = record->vecVelocity.Length2D();

    if (!continuous) {
        if (!Capture(entity, history.baseline)) {
            record->iExperimentalReason = static_cast<int>(experimental::Reason::Invalid);
            return;
        }
        // Seed from this packet's immutable layers, never another side's output.
        std::memcpy(history.baseline.layers, record->arrLayers, sizeof(record->arrLayers));
        for (auto& candidate : history.candidates) candidate = history.baseline;
    }
    const bool baselineValid = Replay(entity, record, prior, history.baseline, 0.f, false, !continuous);
    std::array<float, 9> scores;
    scores.fill((std::numeric_limits<float>::infinity)());
    bool candidatesValid = true;
    for (int i = 0; i < static_cast<int>(history.candidates.size()); ++i) {
        auto& candidate = history.candidates[i];
        if (!Replay(entity, record, prior, candidate, experimental::Fractions[i], true, !continuous)) {
            candidatesValid = false;
            continue;
        }
        scores[i] = experimental::Score(Layer(record->arrLayers[6]), Layer(candidate.layers[6]));
    }
    const bool stableAlive = prior && Layer(record->arrLayers[11]).Valid() && Layer(prior->arrLayers[11]).Valid()
        && record->arrLayers[11].nSequence == prior->arrLayers[11].nSequence
        && record->arrLayers[11].flCycle >= prior->arrLayers[11].flCycle
        && record->arrLayers[11].flPlaybackRate > 0.f
        && std::fabs(record->arrLayers[11].flPlaybackRate - prior->arrLayers[11].flPlaybackRate) < .01f * record->arrLayers[11].flPlaybackRate;
    const bool ground = prior && (record->iFlags & prior->iFlags & FL_ONGROUND);
    const bool eligible = baselineValid && candidatesValid && prior && ground && speed > 40.f && prior->vecVelocity.Length2D() > 40.f
        && !record->bDidShot && stableAlive && record->arrLayers[6].nSequence == prior->arrLayers[6].nSequence
        && record->arrLayers[6].flWeight > .05f && record->arrLayers[5].flWeight < .05f && prior->arrLayers[5].flWeight < .05f
        && std::isfinite(record->arrLayers[12].flWeight) && record->arrLayers[12].flWeight >= 0.f && record->arrLayers[12].flWeight < .25f
        && std::isfinite(prior->arrLayers[12].flWeight) && prior->arrLayers[12].flWeight >= 0.f && prior->arrLayers[12].flWeight < .25f
        && std::fabs(record->flDuck - prior->flDuck) < .08f
        && std::fabs(speed - prior->vecVelocity.Length2D()) < (std::max)(12.f, speed * .08f)
        && std::fabs(experimental::NormalizeYaw(record->vecEyeAngles.y - prior->vecEyeAngles.y)) < 15.f;
    const bool lbyChanged = prior && std::fabs(experimental::NormalizeYaw(record->flLowerBodyYawTarget - prior->flLowerBodyYawTarget)) > .1f;
    auto decision = history.evidence.Update(scores, eligible, ground && speed <= 40.f,
        lbyChanged, record->flSimulationTime, continuous);
    if (!baselineValid || !candidatesValid) {
        decision.confidence = 0.f;
        decision.reason = experimental::Reason::Invalid;
    }
    Candidate* selected = nullptr;
    if (decision.reason == experimental::Reason::MovingLayers || decision.reason == experimental::Reason::LastMove)
        selected = &history.candidates[decision.candidate];

    float yaw = entity->AnimState()->flGoalFeetYaw;
    if (selected && selected->valid) yaw = selected->state.flGoalFeetYaw;
    const float negativeBound = -Offset(*entity->AnimState(), -1.f);
    const float positiveBound = Offset(*entity->AnimState(), 1.f);
    control.standing.Update(record->flSimulationTime, speed, (record->iFlags & FL_ONGROUND) != 0,
        record->flLowerBodyYawTarget, yaw);
    const bool standing = (record->iFlags & FL_ONGROUND) && speed <= 40.f;
    if (standing && baselineValid && candidatesValid && continuous) {
        bool extended = false;
        const float guess = control.standing.Guess(record->vecEyeAngles.y, record->flLowerBodyYawTarget,
            negativeBound, positiveBound, record->flSimulationTime, extended);
        yaw = experimental::NormalizeYaw(record->vecEyeAngles.y + guess);
        if (extended || decision.reason != experimental::Reason::LastMove) {
            decision.reason = extended ? experimental::Reason::LbyExtended : experimental::Reason::Standing;
            decision.confidence = 0.f;
        }
        selected = nullptr;
    }
    if (baselineValid && candidatesValid && continuous && !record->bBreakingLagcompensation) {
        const float primary = experimental::NormalizeYaw(yaw - record->vecEyeAngles.y);
        auto choice = control.recovery.Choose(primary, negativeBound, positiveBound,
            experimental::NormalizeYaw(record->flLowerBodyYawTarget - record->vecEyeAngles.y),
            record->flSimulationTime, standing);
        if (choice.changed || choice.hitHistory) {
            yaw = experimental::NormalizeYaw(record->vecEyeAngles.y + choice.delta);
            decision.reason = choice.hitHistory ? experimental::Reason::HitHistory : experimental::Reason::Bruteforce;
            decision.confidence = 0.f;
            selected = nullptr;
        }
    }
    const auto& settings = playerList::arrPlayers[record->iEntIndex];
    if (settings.bOverrideResolver && std::isfinite(settings.flOverrideYaw)) {
        yaw = experimental::NormalizeYaw(record->vecEyeAngles.y + settings.flOverrideYaw);
        decision.confidence = 0.f;
        decision.reason = experimental::Reason::Override;
    }
    // Keep the animfix's velocity, origin, poses, layers and bone-building state.
    // This is the only live animation field committed by the resolver.
    entity->AnimState()->flGoalFeetYaw = yaw;
    record->flResolverConfidence = decision.confidence;
    record->iExperimentalReason = static_cast<int>(decision.reason);
    record->iExperimentalBruteStep = control.recovery.misses;
    record->flGuessedYaw = record->flResolveDelta = experimental::NormalizeYaw(yaw - record->vecEyeAngles.y);
    const bool chosen = decision.confidence > 0.f || decision.reason == experimental::Reason::Standing
        || decision.reason == experimental::Reason::LbyExtended || decision.reason == experimental::Reason::Bruteforce
        || decision.reason == experimental::Reason::HitHistory || decision.reason == experimental::Reason::Override;
    record->iResolveSide = VISUAL;
    if (chosen)
        record->iResolveSide = record->flResolveDelta < -.01f ? LEFT : record->flResolveDelta > .01f ? RIGHT : CENTER;
    log->iLastResolve = decision.reason == experimental::Reason::Override ? Lagcompensation::OVERRIDE
        : chosen ? Lagcompensation::ANIMATION : Lagcompensation::LEGIT;

    record->bHasExperimentalYaws = candidatesValid;
    for (int i = 0; i < static_cast<int>(history.candidates.size()); ++i) {
        const auto& side = history.candidates[i];
        record->experimentalYawHypotheses[i] = side.state.flGoalFeetYaw;
        EMatrixType type = MAX;
        if (i == 0) type = LEFT;
        else if (i == 8) type = RIGHT;
        else if (i == experimental::Center) type = CENTER;
        if (type != MAX) {
            record->LayerData[type] = Lagcompensation::LagRecord_t::LayerData_t(side.layers[6]);
            record->flLayerDifferences[type] = scores[i];
        }
    }
    record->LayerData[RESOLVE] = Lagcompensation::LagRecord_t::LayerData_t(selected ? selected->layers[6] : record->arrLayers[6]);
    // Assimilate only the received movement phase after scoring. Each bank keeps
    // its own yaw, movement smoothing and poses for the next packet.
    auto assimilate = [&](Candidate& candidate) {
        candidate.state.flFeetCycle = record->arrLayers[6].flCycle;
        candidate.layers[6].flCycle = record->arrLayers[6].flCycle;
        for (int layer : { 1, 2, 4, 5, 8, 9, 10, 11 }) candidate.layers[layer] = record->arrLayers[layer];
    };
    assimilate(history.baseline);
    for (auto& candidate : history.candidates) assimilate(candidate);
    history.entityHandle = entity->GetRefEHandle();
    history.weaponHandle = weaponHandle;
    history.model = entity->GetModel();
    history.spawnTime = entity->GetSpawnTime();
    history.simulationTime = record->flSimulationTime;
    history.ready = baselineValid && candidatesValid;
}

void ExperimentalResolver::BuildSafePoints(CBaseEntity* entity, Lagcompensation::LagRecord_t* record) {
    record->intersectionPoints = {};
    record->damageGeometry = {};
    record->iHeadHypothesisCount = 0;
    record->bHasHeadOverlap = false;
    record->bSafeMatrixValid = false;
    record->headHypotheses = {};
    if (!record->bAnimationMatrixValid) return;
    const auto head = entity->StudioHitbox(HITBOX_HEAD);
    matrix3x4a_t scratch[MAXSTUDIOBONES]{};
    bool built = true, matricesBuilt = true;
    for (int i = 0; i < static_cast<int>(experimental::Fractions.size()); ++i) {
        matrix3x4a_t* output = scratch;
        if (i == 0) output = record->pMatricies[LEFT];
        else if (i == 8) output = record->pMatricies[RIGHT];
        else if (i == experimental::Center) output = record->pMatricies[CENTER];
        // Standing predictions approach LBY and can collapse onto one pose.
        // Keep the whole bounded yaw family in the safepoint sweep instead.
        const bool usePrediction = record->bHasExperimentalYaws && record->vecVelocity.Length2D() > 40.f;
        const float yaw = usePrediction ? record->experimentalYawHypotheses[i]
            : experimental::NormalizeYaw(record->vecEyeAngles.y + Offset(*entity->AnimState(), experimental::Fractions[i]));
        const bool sideBuilt = BuildMatrix(entity, record, yaw, output);
        matricesBuilt &= sideBuilt;
        record->headHypotheses[i] = sideBuilt ? hitbox::FromStudio(head, output) : hitbox::Shape{};
        built &= sideBuilt && record->headHypotheses[i].valid;
    }
    record->bSafeMatrixValid = matricesBuilt;
    record->headHypotheses[9] = hitbox::FromStudio(head, record->pMatricies[RESOLVE]);
    // Safepoint failures must not discard a working animfix's selected matrix.
    if (built && record->headHypotheses[9].valid) {
        record->iHeadHypothesisCount = static_cast<int>(hitbox::MaxShapes);
        hitbox::Vec overlap;
        if (hitbox::FindIntersection(record->headHypotheses.data(), record->headHypotheses.size(), record->headHypotheses[9].Center(), overlap)) {
            const auto point = hitbox::ToVector(overlap);
            if (hitbox::ContainsAll(record->headHypotheses.data(), record->headHypotheses.size(), hitbox::FromVector(point), .03)) {
                record->vecRageSafePoint = point;
                record->bHasHeadOverlap = true;
            }
        }
    }
    std::memcpy(record->pMatricies[EXTRAPOLATED], record->pMatricies[RESOLVE], sizeof(record->pMatricies[RESOLVE]));
    std::memcpy(record->pSideMatrixes[0], record->pMatricies[LEFT], sizeof(record->pSideMatrixes[0]));
    std::memcpy(record->pSideMatrixes[1], record->pMatricies[RIGHT], sizeof(record->pSideMatrixes[1]));
}

void ExperimentalResolver::RecordShot(const rageBotData_t& data, int command) {
    if (resolver::GetMode(cfg::rage::iResolverMode) != resolver::Mode::Experimental
        || !data.pRecord || !data.pAimbotTarget || data.iEntityIndex < 1 || data.iEntityIndex >= 65
        || data.pRecord->iResolverMode != static_cast<int>(resolver::Mode::Experimental)) return;
    experimental::ShotQueue<ShotPayload>::Shot shot;
    shot.payload.record = std::make_unique<Lagcompensation::LagRecord_t>(*data.pRecord);
    shot.payload.data = data;
    shot.payload.data.pRecord = shot.payload.record.get();
    shot.payload.data.pTargetMatrix = shot.payload.record->pMatricies[RESOLVE];
    shot.payload.model = data.pAimbotTarget->GetModel();
    shot.target = data.iEntityIndex;
    shot.command = command;
    shot.created = i::GlobalVars->flRealTime;
    shot.start = hitbox::FromVector(data.vecLocalShootPosition);
    shot.intended = hitbox::FromVector(data.vecTargetShootPosition);
    shot.expected = hitbox::FromStudio(data.pAimbotTarget->StudioHitbox(data.iHitbox), data.pRecord->pMatricies[RESOLVE]);
    shot.entityHandle = data.pAimbotTarget->GetRefEHandle();
    shot.spawnTime = data.pAimbotTarget->GetSpawnTime();
    shot.epoch = data.pRecord->uExperimentalEpoch;
    shot.delta = data.pRecord->flResolveDelta;
    const auto reason = static_cast<experimental::Reason>(data.pRecord->iExperimentalReason);
    auto weapon = g::pLocal ? g::pLocal->GetWeapon() : nullptr;
    auto weaponData = weapon ? weapon->GetCSWpnData() : nullptr;
    shot.learn = shot.epoch != 0 && shot.expected.valid && data.pRecord->bAnimationMatrixValid && !data.bBacktrack
        && !data.bSafe && !data.pRecord->bBreakingLagcompensation && !data.pRecord->bFirstAfterDormant
        && !data.pRecord->bDidShot && weaponData && weaponData->iBullets == 1
        && reason != experimental::Reason::Override && reason != experimental::Reason::Warmup
        && reason != experimental::Reason::Invalid && reason != experimental::Reason::Transition;
    shotQueue.Add(std::move(shot));
    // The legacy single-shot logger retains raw lag-record pointers. Experimental
    // logging uses the owned queue, including its existing Lua shot callback.
    aimbot.GetHitLogData().ClearTarget();
}

void ExperimentalResolver::HandleGameEvent(IGameEvent* event) {
    if (!event) return;
    const std::string_view name(event->GetName());
    if (name == "round_start" || name == "game_newmap") { ResetAll(); return; }
    if (resolver::GetMode(cfg::rage::iResolverMode) != resolver::Mode::Experimental || !g::pLocal) return;
    const int user = i::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    const double time = i::GlobalVars->flRealTime;
    if (name == "player_spawn") {
        if (user == g::pLocal->EntIndex()) ResetAll(); else Reset(user);
    } else if (name == "weapon_fire" && user == g::pLocal->EntIndex()) {
        shotQueue.Fire(time);
    } else if (name == "bullet_impact" && user == g::pLocal->EntIndex()) {
        shotQueue.Impact({event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z")}, time);
    } else if (name == "player_hurt"
        && i::EngineClient->GetPlayerForUserID(event->GetInt("attacker")) == g::pLocal->EntIndex()) {
        shotQueue.Hurt(user, event->GetInt("dmg_health"), event->GetInt("hitgroup"), time);
    }
}

void ExperimentalResolver::ProcessShots() {
    if (resolver::GetMode(cfg::rage::iResolverMode) != resolver::Mode::Experimental || !g::pLocal) {
        ResetShots(-1);
        return;
    }
    // Hurt and impact may arrive in separate event batches. Wait at least .2 s
    // and account for network latency before learning a missing hurt event.
    float latency = 0.f;
    if (auto channel = i::EngineClient->GetNetChannelInfo())
        latency = channel->GetLatency(FLOW_INCOMING) + channel->GetLatency(FLOW_OUTGOING);
    const double settle = std::clamp(std::isfinite(latency) ? latency + .1f : .2f, .2f, .75f);
    while (auto finished = shotQueue.PopReady(i::GlobalVars->flRealTime, settle)) {
        auto& shot = *finished;
        auto entity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(shot.target));
        auto& control = controls[shot.target];
        if (!entity || entity != shot.payload.data.pAimbotTarget || entity->GetRefEHandle() != shot.entityHandle
            || entity->GetSpawnTime() != shot.spawnTime || entity->GetModel() != shot.payload.model) continue;
        auto& data = shot.payload.data;
        data.iServerHitbox = shot.hitgroup;
        data.iBacktrackTicks = data.iTickcount - TIME_TO_TICKS(data.flTargetSimulation + lagcomp.GetClientInterpAmount());
        if (shot.fired && (shot.hasImpact || shot.hurt))
            LuaImplementation::RunCallbacks(LuaImplementation::vecCallbackList[LuaImplementation::CALLBACK_ON_RAGEBOT_SHOT], data);
        // Keep a killing hit available for logging even if FrameStage has
        // already reset the dead player's control. Stale epochs never learn.
        const auto outcome = control.ready && control.epoch == shot.epoch ? shot.Outcome() : experimental::ShotOutcome::Ignore;
        const float time = histories[shot.target].simulationTime;
        if (outcome == experimental::ShotOutcome::GeometryMiss) control.recovery.Miss(shot.delta, time);
        // Body hits and overlaps do not uniquely confirm a head-yaw hypothesis.
        if (outcome == experimental::ShotOutcome::Hit && data.iHitbox == HITBOX_HEAD && shot.hitgroup == HITGROUP_HEAD)
            control.recovery.Hit(shot.delta, time);
        if (shot.hurt) {
            lagcomp.GetLog(shot.target).iHitAmount++;
            misc::Print(std::format("EXP hit {} for {} hp | yaw {:+.1f} | bt {}", misc::GetHitgroupName(shot.hitgroup),
                shot.damage, shot.delta, data.iBacktrackTicks));
        } else if (outcome == experimental::ShotOutcome::GeometryMiss) {
            misc::Print(std::format("EXP geometry miss | yaw {:+.1f} | brute {}", shot.delta, control.recovery.misses));
        } else if (shot.fired && shot.hasImpact) {
            misc::Print("EXP miss | no resolver feedback (spread/occlusion/safe/backtrack/ambiguous)");
        }
    }
}
