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

		bool IsValid() {

			if (!bValid)
				return false;

			float flTime = i::GlobalVars->flCurrentTime;

			float sv_maxunlag = i::ConVar->FindVar("sv_maxunlag")->GetFloat();

			float flDelta = flTime - sv_maxunlag;

			if (flDelta > sv_maxunlag)
				return false;

			return true;
		}

		void Apply(CBaseEntity* pEnt) {

			pEnt->GetSimulationTime() = flSimulationTime;
			pEnt->GetLowerBodyYaw() = flLowerBodyYawTarget;
			pEnt->GetEyeAngles() = vecEyeAngles;
			pEnt->SetAbsAngles(vecAbsAngles);
			pEnt->GetVecOrigin() = vecOrigin;
			pEnt->SetAbsOrigin(vecOrigin);
			pEnt->GetCollideable()->OBBMins() = vecMins;
			pEnt->GetCollideable()->OBBMaxs() = vecMaxs;

			pEnt->SetPoseParameters(flPoseParamater);
			pEnt->SetAnimationLayers(layer);
			pEnt->SetBoneCache(matrix);
		}

		void StoreData(CBaseEntity* pEnt) {
			
			const auto pWeapon = pEnt->GetWeapon();

			this->pEnt = pEnt;
			EntIndex = pEnt->EntIndex();

			nFlags = pEnt->GetFlags();
			nEFlags = pEnt->GetEFlags();
			nEffect = pEnt->GetEffects();
			nChoked = TIME_TO_TICKS(i::GlobalVars->flCurrentTime - pEnt->GetSimulationTime());

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

	void UpdateLagRecords();
	void UpdatePlayer(CBaseEntity* pEnt);
	void VelocityFix(CBaseEntity* pEnt, playerrecord_t* pRecord, playerrecord_t* pPreviousRecord);
	void DisableInterpolation();

	std::deque<playerrecord_t> deqLagRecords[65];
	std::deque<playerrecord_t> deqValidLagRecords[65];
};
inline LagComp lagcomp;