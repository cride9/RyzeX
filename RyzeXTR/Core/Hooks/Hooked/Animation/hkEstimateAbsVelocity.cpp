#include "../../hooks.h"
#include "../../../SDK/entity.h"
#include "../../../globals.h"

void __fastcall h::hkEstimateAbsVelocity(CBaseEntity* pPlayer, void* edx, Vector& vecVelocity)
{
	static auto EstimateAbsVelocity = detour::estimateAbsVelocity.GetOriginal<decltype(&hkEstimateAbsVelocity)>();

	return EstimateAbsVelocity(pPlayer, edx, vecVelocity);

	// no interpolation, we dont want to estimate abs velocity
	if (pPlayer->GetEffects() & EF_NOINTERP)
		return;

	// setting up bones, we dont want to estimate abs velocity
	if (g::bSettingUpBones[pPlayer->EntIndex()])
		return;

	// player is abusing tickbase, we dont want to estimate abs velocity
	if (pPlayer->GetSimulationTime() < pPlayer->GetOldSimulationTime())
		return;

	// all good to go, you can estimate the player abs velocity
	EstimateAbsVelocity(pPlayer, edx, vecVelocity);
}