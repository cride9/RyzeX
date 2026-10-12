#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"
#include "../../../Features/Rage/Animations/EnemyAnimations.h"
#include "../../../Features/Misc/setupbones.h"

bool __fastcall h::hkSetupBones(IClientRenderable* ecx, void* edx, matrix3x4a_t* matrix, int maxbones, int bonemask, float curtime) {

	static auto original = detour::setupBones.GetOriginal<decltype(&h::hkSetupBones)>();

	if (!ecx || !ecx->GetIClientUnknown())
		return false;
	CBaseEntity* pEnt = ecx->GetIClientUnknown()->GetBaseEntity();
	if (!g::pLocal || !pEnt || !pEnt->IsAlive() || !pEnt->IsPlayer() || g::bUpdatingSkins || (!pEnt->IsEnemy(g::pLocal) && pEnt != g::pLocal))
		return original( ecx, edx, matrix, maxbones, bonemask, curtime);

	const int index = pEnt->EntIndex();
	if (index < 1 || index >= static_cast<int>(std::size(g::bSettingUpBones)))
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	// Only the full VISUAL build can bypass the cache hook for an enemy.
	// Hypothesis builds must never re-enter native SetupBones on the live entity.
	if (anims.IsBuildingVisualMatrix(index))
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);
	if ( g::bSettingUpBones[ index ] )
		return ( pEnt == g::pLocal ? original( ecx, edx, matrix, maxbones, bonemask, curtime ) : false );
	if (g::bTracingLagRecord[index]) {
		auto& cache = pEnt->GetCachedBoneData();
		const int count = cache.Count();
		if (!cache.Base() || count <= 0 || count > MAXSTUDIOBONES || (matrix && maxbones < count))
			return false;
		auto& accessor = pEnt->GetBoneAccessor();
		if (!accessor.matBones) return false;
		if (accessor.matBones != cache.Base())
			std::memcpy(accessor.matBones, cache.Base(), count * sizeof(matrix3x4a_t));
		accessor.nReadableBones = accessor.nWritableBones = BONE_USED_BY_HITBOX;
		if (matrix && matrix != cache.Base() && matrix != accessor.matBones)
			std::memcpy(matrix, cache.Base(), count * sizeof(matrix3x4a_t));
		return true;
	}

	if (pEnt == g::pLocal)
		return !matrix || localAnim->CopyCachedMatrix(matrix, maxbones);

	// SetupBones(nullptr, -1, ...) still has to publish bones/attachments.
	// maxbones is output capacity, not the number of matrices to read.
	if (anims.CopyCachedMatrix(pEnt, matrix, maxbones, bonemask))
		return true;

	// First packet, changed model or failed visual build: let the engine
	// initialize its cache instead of reporting success with empty matrices.
	pEnt->InvalidateBoneCache();
	return original(ecx, edx, matrix, maxbones, bonemask, curtime);
}
