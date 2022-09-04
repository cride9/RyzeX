#pragma once
#include <deque>
#include "../../../SDK/Entity.h"
#include "../../../SDK/Math.h"
#include "../../../utilities.h"
#include "../../../globals.h"


class LagComp {
public:

	float LerpTime();

	struct playerrecord_t {

		CBaseEntity* pEnt;

		bool bValid;
		bool bDormant;
		bool bDidShot;
		bool bAllowAnimationUpdate;
		bool bAnimatePlayer;

		int EntIndex;
		int nFlags;
		int nEFlags;
		int nChoked;
		int nEffect;

		float flSimulationTime;
		float flDuckAmount;
		float flInterpolation;
		float flLowerBodyYawTarget;
		float flLastShotTime;
		float flSpawnTime;
		float flPoseParamater[24];
		float flMaxSpeed;

		Vector vecOrigin;
		Vector vecVelocity;
		Vector vecEyeAngles;
		Vector vecAbsOrigin;
		Vector vecAbsAngles;
		Vector vecMins;
		Vector vecMaxs;

		CAnimationLayer layer[13];
		CAnimState* pAnimstate;
		matrix3x4_t matrix[128];

		explicit playerrecord_t(CBaseEntity* pEnt) {
			StoreData(pEnt);
		}

		bool IsValid(float flSimulationTime, bool bValid, float flRange = 0.2f);
		void Restore(CBaseEntity* pEnt);
		void Apply(CBaseEntity* pEnt);
		void StoreData(CBaseEntity* pEnt) {
			
			const auto pWeapon = pEnt->GetWeapon();

			this->pEnt = pEnt;
			EntIndex = pEnt->EntIndex();

			nFlags = pEnt->GetFlags();
			nEFlags = pEnt->GetEFlags();
			nEffect = pEnt->GetEffects();
			nChoked = TIME_TO_TICKS(pEnt->GetSimulationTime() - pEnt->GetOldSimulationTime());

			bDormant = pEnt->IsDormant();
			vecVelocity = pEnt->GetVelocity();
			vecOrigin = pEnt->GetVecOrigin();
			vecAbsOrigin = pEnt->GetAbsOrigin();
			vecEyeAngles = pEnt->GetEyeAngles();
			vecAbsAngles = pEnt->GetAbsAngles();
			vecMins = pEnt->GetCollideable()->OBBMins();
			vecMaxs = pEnt->GetCollideable()->OBBMaxs();

			flSimulationTime = pEnt->GetSimulationTime();
			flInterpolation = 0.f;
			flLastShotTime = pWeapon ? pWeapon->GetLastShotTime() : 0.f;
			flDuckAmount = pEnt->GetDuckAmount();
			flLowerBodyYawTarget = pEnt->GetLowerBodyYaw();
			flMaxSpeed = pWeapon ? pWeapon->GetCSWpnData()->flMaxSpeed[0] : 260.f;

			pAnimstate = pEnt->AnimState() ? pEnt->AnimState() : nullptr;

			pEnt->GetPoseParameters(flPoseParamater);
			pEnt->GetAnimationLayers(layer);

			nChoked = std::clamp(nChoked, 0, 17);
			bValid = nChoked >= 0 && nChoked <= 17;
		}
	};

	void PostPlayerUpdate();

	void UpdateAnimations(CBaseEntity* pEnt, playerrecord_t* pRecord, playerrecord_t* pPreviousRecord);
	void UpdatePlayer(CBaseEntity* pEnt);
	void VelocityFix(CBaseEntity* pEnt, playerrecord_t* pRecord, playerrecord_t* pPreviousRecord);

	std::deque<playerrecord_t> deqLagRecords[65];
};
inline LagComp lagcomp;