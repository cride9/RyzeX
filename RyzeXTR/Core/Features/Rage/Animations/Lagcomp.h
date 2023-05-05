#pragma once
#include "../../Networking/networking.h"
#include "../../../SDK/Entity.h"
#include <deque>

namespace lagcompensation {

	enum ROTATE_MODE
	{
		ROTATE_SERVER,
		ROTATE_LEFT,
		ROTATE_CENTER,
		ROTATE_RIGHT,
		ROTATE_LOW_LEFT, /* NO BONES */
		ROTATE_LOW_RIGHT, /* NO BONES */
		ROTATE_MAX_INDEX
	};

	struct LagRecord_t {
		LagRecord_t() {
			Reset();
		}

		LagRecord_t(CBaseEntity* CBaseEntity) {
			Construct(CBaseEntity);
		}

		void Reset() {
			iEntIndex = -1;
			// AnimationLayers
			// Bones
			// Poses
			bDataFilled = false;
			bAnimated = false;
			bValid = false;
			iUpdateTick = -1;

			vecMins = Vector();
			vecMaxs = Vector();
			vecOrigin = Vector();
			vecAbsOrigin = Vector();
			vecEyeAngles = Vector();
			vecAbsAngles = Vector();
			vecVelocity = Vector();
			vecAbsVelocity = Vector();
			vecWeaponShootPos = Vector();

			iChokedTicks = 0;
			iFlags = 0;
			iEFlags = 0;

			flEyeYaw = 0.f;
			flSimtime = 0.f;
			flOldSimtime = 0.f;
			flLowerBodyYaw = 0.f;
			flDuckAmount = 0.f;
			flLastShotTime = 0.f;

			bDormant = false;
			bBreakingLagcomp = false;

			flResolveDelta = 0.f;

			flAbsMaxDesyncDelta = 0.f;
		}

		void Construct(CBaseEntity* CBaseEntity);

		bool IsValid();

		int iEntIndex = -1;
		CAnimationLayer AnimationLayers[ROTATE_MAX_INDEX][ANIMATION_LAYER_COUNT];
		matrix3x4_t Bones[ROTATE_MAX_INDEX][256];
		float Poses[PLAYER_POSE_PARAM_COUNT];
		bool bDataFilled = false;
		bool bAnimated = false;
		bool bValid = false;
		int iUpdateTick = -1;

		Vector vecMins = Vector();
		Vector vecMaxs = Vector();
		Vector vecOrigin = Vector();
		Vector vecAbsOrigin = Vector();
		Vector vecEyeAngles = Vector();
		Vector vecAbsAngles = Vector();
		Vector vecVelocity = Vector();
		Vector vecAbsVelocity = Vector();
		Vector vecWeaponShootPos = Vector();

		int iChokedTicks = 0;
		int iFlags = 0;
		int iEFlags = 0;

		float flEyeYaw = 0.f;
		float flSimtime = 0.f;
		float flOldSimtime = 0.f;
		float flLowerBodyYaw = 0.f;
		float flDuckAmount = 0.f;
		float flLastShotTime = 0.f;

		bool bShooting = false;

		bool bDormant = false;
		bool bBreakingLagcomp = false;

		float flResolveDelta = 0.f;

		float flAbsMaxDesyncDelta = 0.f;
	};

	struct AnimationInfo_t {
		AnimationInfo_t() {
			Reset();
		}

		AnimationInfo_t(CBaseEntity* CBaseEntity) {
			Construct(CBaseEntity);
		}

		void Reset() {
			iEntIndex = -1;
			EntHandle = -1;
			flLastSpawnTime = 0.f;
			// CBaseEntityInfo
			if (aLagRecords.size()) { aLagRecords.clear(); }
			iMissedShots = 0;
			iResolveIndex = 0;
			flPreviousAngleDifference = 0.f;
			flAngleBeforeResolve = 0.f;
			flResolveAngle = 0.f;
			flAngleAfterResolve = 0.f;
		}

		void Construct(CBaseEntity* CBaseEntity);
		void UpdateInfo(CBaseEntity* CBaseEntity);

		void UpdateAnimations(CBaseEntity* CBaseEntity, LagRecord_t* current, LagRecord_t* previous, int iRotationMode);
		void UpdateAnimationsData(CBaseEntity* CBaseEntity, LagRecord_t* current, LagRecord_t* previous);

		int iEntIndex = -1;
		unsigned long EntHandle;
		float flLastSpawnTime = 0.f;
		PlayerInfo_t CBaseEntityInfo;
		std::deque< LagRecord_t > aLagRecords;
		int iMissedShots = 0;

		float flSimulationTime = 0.f;

		int iResolveIndex = 0;
		float flPreviousAngleDifference = 0.f; // diff between gfy and eye yaw, for resolver
		float flAngleBeforeResolve = 0.f;
		float flLastResolveAngle = 0.f;
		float flResolveAngle = 0.f;
		float flAngleAfterResolve = 0.f;

		float flLastBloodProcessRealTime = 0.f;
		int iLastBloodProcessTickCount = 0;
		Vector vecDirBlood = Vector();
		Vector vecBloodOrigin = Vector();
	};

	inline AnimationInfo_t aAnimationInfo[65];

	inline AnimationInfo_t& GetAnimationInfo(int i) {
		return aAnimationInfo[i];
	}

	LagRecord_t* GetLagRecord(CBaseEntity* CBaseEntity, bool last);
	LagRecord_t* GetShotRecord(CBaseEntity* CBaseEntity, bool last);

	float GetLerpTime();

	void instance();
}