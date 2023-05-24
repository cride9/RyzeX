#include "LagcompensationV2.h"

entityRecord_t::entityRecord_t(CBaseEntity* pEnt) {

	pEntity = pEnt;
	bValid = true;
	bLagcompBreak = pEnt->GetSimulationTime() < pEnt->GetOldSimulationTime();
	bDormant = pEnt->IsDormant();
	bDidShot = pEnt->GetWeapon() ? pEnt->GetWeapon()->GetLastShotTime() == i::GlobalVars->flCurrentTime : false;

	vecVelocity = pEnt->GetVelocity();
	vecAbsVelocity = pEnt->GetVecAbsVelocity();
	vecOrigin = pEnt->GetVecOrigin();
	vecAbsOrigin = pEnt->GetAbsOrigin();
	vecMins = pEnt->vecMins();
	vecMaxs = pEnt->vecMaxs();
	vecEyeAngles = pEnt->GetEyeAngles();
	vecAbsAngles = pEnt->GetAbsAngles();

	flSimulationTime = pEnt->GetSimulationTime();
	flOldSimulationTime = pEnt->GetOldSimulationTime();
	flDuckAmount = pEnt->GetDuckAmount();
	flLowerBodyYawTarget = pEnt->GetLowerBodyYaw();
	flSpawnTime = pEnt->GetSpawnTime();

	iFlags = pEnt->GetFlags();
	iEFlags = pEnt->GetEFlags();
	iEffects = pEnt->GetEffects();
	iChokeAmount = abs(pEnt->GetSimulationTime() - pEnt->GetOldSimulationTime());

	pEnt->GetAnimationLayers(pLayers[LSERVER]);
	pEnt->GetPoseParameters(pPoses.data());
	pEnt->GetBoneCache(pMatricies[MVISUAL]);
	std::memcpy(&mAnimstate, &pEnt->AnimState(), sizeof(CAnimState));
}

entityRecord_t::entityRecord_t(entityRecord_t* pRecord) {

	pEntity = pRecord->pEntity;

	/* bools */
	bValid = pRecord->bValid;
	bLagcompBreak = pRecord->bLagcompBreak;
	bDormant = pRecord->bDormant;
	bDidShot = pRecord->bDidShot;

	/* vectors */
	vecVelocity = pRecord->vecVelocity;
	vecAbsVelocity = pRecord->vecAbsVelocity;
	vecOrigin = pRecord->vecOrigin;
	vecAbsOrigin = pRecord->vecAbsOrigin;
	vecMins = pRecord->vecMins;
	vecMaxs = pRecord->vecMaxs;
	vecEyeAngles = pRecord->vecEyeAngles;
	vecAbsAngles = pRecord->vecAbsAngles;

	/* floats */
	flSimulationTime = pRecord->flSimulationTime;
	flOldSimulationTime = pRecord->flOldSimulationTime;
	flDuckAmount = pRecord->flDuckAmount;
	flLowerBodyYawTarget = pRecord->flLowerBodyYawTarget;
	flSpawnTime = pRecord->flSpawnTime;

	/* ints */
	iFlags = pRecord->iFlags;
	iEFlags = pRecord->iEFlags;
	iEffects = pRecord->iEffects;
	iChokeAmount = pRecord->iChokeAmount;

	/* data */
	std::memcpy(&pMatricies, &pRecord->pMatricies, sizeof(matrix3x4_t[MAXSTUDIOBONES][MMATRIXCOUNT]));
	std::memcpy(&pLayers, &pRecord->pLayers, sizeof(matrix3x4_t[ANIMATION_LAYER_COUNT][LLAYERCOUNT]));
	std::memcpy(&pPoses, &pRecord->pPoses, sizeof(float[MAXSTUDIOPOSEPARAM]));

	/* Whole animstate backup for accurate animation information */
	std::memcpy(&mAnimstate, &pRecord->mAnimstate, sizeof(CAnimState));
}