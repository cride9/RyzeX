#include "Lagcomp.h"

template < class T >
__forceinline T Interpolate(const T& flCurrent, const T& flTarget, const int iProgress, const int iMaximum)
{
	return flCurrent + ((flTarget - flCurrent) / iMaximum) * iProgress;
}

void lagcompensation::LagRecord_t::Construct(CBaseEntity* pEntity) {

	if (!pEntity)
		return Reset();

	CBaseCombatWeapon* pWeapon = pEntity->GetWeapon();
	iEntIndex = pEntity->EntIndex();

	for (CAnimationLayer(&refLayer)[13] : AnimationLayers) 
		pEntity->GetAnimationLayers(refLayer);
	
	for (matrix3x4_t(&refBones)[256] : Bones) 
		pEntity->GetBoneCache(refBones);
	
	pEntity->GetPoseParameters(Poses);

	bDataFilled = true;
	bAnimated = false;
	bValid = true;
	iUpdateTick = i::ClientState->clockDriftMgr.nServerTick;

	vecMins = pEntity->vecMins();
	vecMaxs = pEntity->vecMaxs();
	vecOrigin = pEntity->GetVecOrigin();
	vecAbsOrigin = pEntity->GetAbsOrigin();
	vecEyeAngles = pEntity->GetEyeAngles();
	vecAbsAngles = pEntity->GetAbsAngles();
	vecVelocity = pEntity->GetVelocity();
	vecAbsVelocity = pEntity->GetVecAbsVelocity();
	//vecWeaponShootPos = pEntity->WeaponShootPos();

	iChokedTicks = TIME_TO_TICKS(pEntity->GetSimulationTime() - pEntity->GetOldSimulationTime());
	iFlags = pEntity->GetFlags();
	iEFlags = pEntity->GetEFlags();

	flEyeYaw = pEntity->AnimState() ? pEntity->AnimState()->flEyeYaw : 0.f;
	flSimtime = pEntity->GetSimulationTime();
	flOldSimtime = pEntity->GetOldSimulationTime();
	flLowerBodyYaw = pEntity->GetLowerBodyYaw();
	flDuckAmount = pEntity->GetDuckAmount();
	flLastShotTime = pWeapon ? pWeapon->GetLastShotTime() : 0.f;
	bShooting = pWeapon ? (flLastShotTime > flOldSimtime && flLastShotTime <= flSimtime) : false;

	bDormant = pEntity->IsDormant();
	bBreakingLagcomp = false;

	flResolveDelta = 0.f;
}