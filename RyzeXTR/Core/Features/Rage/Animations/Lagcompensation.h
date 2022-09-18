#pragma once
#include <deque>
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

struct record_t {

	/* Validity check to not shoot lagcomp breaking ppl */
	bool bValid;

	/* Later usage: fix Jumpfall animationlayer */
	int iFlags;

	Vector vecOrigin;
	Vector vecAngles;
	Vector vecMins;
	Vector vecMaxs;
	Vector vecVelocity;
	Vector vecAbsAngles;

	/* For backtrack / updatetime / choke calculation */
	float flSimulationTime;
	float flOldSimulationTime;
	float flMaxSpeed;
	float flLowerBody;

	/* Getting accurate poses */
	float flPoseParameters[24];

	/* Will be replaced with uninterpolated layers (easier to make a resolver for it) */
	CAnimationLayer pLayers[13];

	/* Aimbot will shoot the generated matrix */
	matrix3x4_t pMatrix[128];

	void StoreRecord(CBaseEntity* pEnt) {

		bValid = true;

		iFlags = pEnt->GetFlags();

		vecOrigin = pEnt->GetVecOrigin();
		vecAngles = pEnt->GetEyeAngles();
		vecMins = pEnt->GetCollideable()->OBBMins();
		vecMaxs = pEnt->GetCollideable()->OBBMaxs();
		vecVelocity = pEnt->GetVelocity();
		vecAbsAngles = pEnt->GetAbsAngles();

		flSimulationTime = pEnt->GetSimulationTime();
		flOldSimulationTime = pEnt->GetOldSimulationTime();
		flMaxSpeed = pEnt->GetWeapon() ? pEnt->GetWeapon()->GetCSWpnData()->flMaxSpeed[0] : 260.f;
		flLowerBody = pEnt->GetLowerBodyYaw();

		pEnt->GetPoseParameters(flPoseParameters);
		pEnt->GetAnimationLayers(pLayers);
	}

	void ApplyRecord(CBaseEntity* pEnt) {

		pEnt->GetFlags() = iFlags;

		pEnt->GetVecOrigin() = vecOrigin;
		pEnt->GetEyeAngles() = vecAngles;
		pEnt->GetCollideable()->OBBMins() = vecMins;
		pEnt->GetCollideable()->OBBMaxs() = vecMaxs;
		pEnt->GetVelocity() = vecVelocity;
		pEnt->SetAbsAngles(vecAbsAngles);

		pEnt->GetSimulationTime() = flSimulationTime;
		pEnt->GetOldSimulationTime() = flOldSimulationTime;
		pEnt->GetLowerBodyYaw() = flLowerBody;

		pEnt->SetPoseParameters(flPoseParameters);
		pEnt->SetAnimationLayers(pLayers);
		pEnt->SetBoneCache(pMatrix);
	}
};

class Lagcompensation {

public:

	/* Everything will be ran inside this */
	void FrameStageNotify(EStage curStage);

	/* Every entity data will be placed into this deque */
	std::deque<record_t> deqRecords[65];

private:

	/* Will be called with the function upper */
	void UpdateAnimation(CBaseEntity* pEnt);

	/* Backup animationlayers before interpolation */
	void GetAnimationLayers(CBaseEntity*);

	/* Fix animation time (velocity) and origin */
	void FixAbsoluteAngVec(CBaseEntity*, record_t*, record_t*);
};
inline Lagcompensation lagcomp;