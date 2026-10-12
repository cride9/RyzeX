#pragma once
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include <array>
#include <cstring>

// A damage query may call SetupBones internally. Keep it on the applied lag
// record, and restore the live pose afterwards so another target sees the
// same world. Nothing in this scope changes the resolver/animation state.
class RecordTraceScope {
public:
    explicit RecordTraceScope(CBaseEntity* player) : entity(player) {
        if (!entity) return;
        index = entity->EntIndex();
        auto& cache = entity->GetCachedBoneData();
        count = cache.Count();
        if (index < 1 || index >= 65 || count <= 0 || count > MAXSTUDIOBONES || !cache.Base()
            || !entity->GetBoneAccessor().matBones) return;
        cacheBase = cache.Base();
        accessor = entity->GetBoneAccessor();
        std::memcpy(cacheBackup.data(), cacheBase, count * sizeof(matrix3x4a_t));
        if (accessor.matBones != cacheBase)
            std::memcpy(accessorBackup.data(), accessor.matBones, count * sizeof(matrix3x4a_t));
        boneTime = entity->GetLastSetupBonesTime();
        boneCounter = entity->GetRecentModelBoneCounter();
        occlusion = entity->GetOcclusionFlags();
        occlusionFrame = entity->GetOcclusionFrameCount();
        previous = g::bTracingLagRecord[index];
        g::bTracingLagRecord[index] = ready = true;
    }
    ~RecordTraceScope() {
        if (!ready) return;
        auto& cache = entity->GetCachedBoneData();
        entity->GetBoneAccessor() = accessor;
        if (cache.Base() == cacheBase && cache.Count() == count) {
            std::memcpy(cacheBase, cacheBackup.data(), count * sizeof(matrix3x4a_t));
            if (accessor.matBones != cacheBase)
                std::memcpy(accessor.matBones, accessorBackup.data(), count * sizeof(matrix3x4a_t));
            entity->GetLastSetupBonesTime() = boneTime;
            entity->GetRecentModelBoneCounter() = boneCounter;
        } else {
            if (accessor.matBones == cacheBase) entity->GetBoneAccessor().matBones = cache.Base();
            entity->InvalidateBoneCache();
        }
        entity->GetOcclusionFlags() = occlusion;
        entity->GetOcclusionFrameCount() = occlusionFrame;
        g::bTracingLagRecord[index] = previous;
    }
    bool Ready() const { return ready; }
    RecordTraceScope(const RecordTraceScope&) = delete;
    RecordTraceScope& operator=(const RecordTraceScope&) = delete;
private:
    CBaseEntity* entity{};
    int index{}, count{}, occlusion{}, occlusionFrame{};
    bool ready{}, previous{};
    float boneTime{};
    unsigned long boneCounter{};
    matrix3x4a_t* cacheBase{};
    CBoneAccessor accessor{};
    std::array<matrix3x4a_t, MAXSTUDIOBONES> cacheBackup, accessorBackup;
};
