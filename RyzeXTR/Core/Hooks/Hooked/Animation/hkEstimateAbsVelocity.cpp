#include "../../hooks.h"
#include "../../../SDK/entity.h"
#include "../../../globals.h"

void __fastcall h::hkEstimateAbsVelocity(CBaseEntity* pPlayer, void* edx, Vector& vecVelocity)
{
	static auto EstimateAbsVelocity = detour::estimateAbsVelocity.GetOriginal<decltype(&hkEstimateAbsVelocity)>();

	// sanity check
	if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->EntIndex() - 1 > 63 || !pPlayer->IsAlive() || g::pLocal == nullptr)
		return EstimateAbsVelocity(pPlayer, edx, vecVelocity);

	// no interpolation, we dont want to estimate abs velocity
	if (pPlayer->GetEffects() & EF_NOINTERP)
		return;

	//// setting up bones, we dont want to estimate abs velocity
	//if (g::bSettingUpBones)
	//	return;

	// player is abusing tickbase, we dont want to estimate abs velocity
	if (pPlayer->GetSimulationTime() < pPlayer->GetOldSimulationTime())
		return;

	// all good to go, you can estimate the player abs velocity
	EstimateAbsVelocity(pPlayer, edx, vecVelocity);
}