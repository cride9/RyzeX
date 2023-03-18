#pragma once
#include "../../../SDK/Menu/config.h"
#include "../../Misc/misc.h"

// that's pasted from uc so I won't comment it ty

enum EBRUTE {

	RIGHT,
	LEFT,
	MIDDLE,
	LOW
};

namespace resolver {

	//inline std::deque<Vector> deqBulletImpacts;
	//inline LagComp::playerrecord_t* targetedRecord;
	//inline bool bPlayerHurt[65];
	//inline bool bBulletImpact[65];
	//inline int iMissedShots[65];

	//inline void CheckShootsMissed(CBaseEntity* pEnt) {

	//	if (!deqBulletImpacts.empty() && targetedRecord != nullptr) {

	//		CTraceFilter filter(g::pLocal, TRACE_EVERYTHING);
	//		Trace_t trace;
	//		Ray_t ray(g::pLocal->GetEyePosition(), deqBulletImpacts.front());

	//		auto oldData = lagcomp.deqLagRecords[pEnt->EntIndex()].front();
	//		i::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	//		CBaseEntity* hitEntity = trace.pHitEntity;

	//		if (hitEntity && hitEntity->EntIndex() == pEnt->EntIndex() && !bPlayerHurt[pEnt->EntIndex()]) {

	//			util::LogConsole("missed shot due to animation desyncronization\n");

	//			// resolver miss
	//			iMissedShots[pEnt->EntIndex()]++;

	//			bBulletImpact[pEnt->EntIndex()] = false;
	//			bPlayerHurt[pEnt->EntIndex()] = false;
	//			deqBulletImpacts.clear();
	//			targetedRecord = nullptr;
	//		}
	//		else if (bPlayerHurt[pEnt->EntIndex()]) {

	//			// hit target
	//			bBulletImpact[pEnt->EntIndex()] = false;
	//			bPlayerHurt[pEnt->EntIndex()] = false;
	//			deqBulletImpacts.clear();
	//			targetedRecord = nullptr;
	//		}
	//		else if (!bPlayerHurt[pEnt->EntIndex()] && hitEntity == nullptr) {

	//			util::LogConsole("missed shot due to spread\n");

	//			// missed due to spread
	//			bBulletImpact[pEnt->EntIndex()] = false;
	//			bPlayerHurt[pEnt->EntIndex()] = false;
	//			deqBulletImpacts.clear();
	//			targetedRecord = nullptr;
	//		}
	//	}
	//}

	//inline void EventManager(IGameEvent* pEvent) {

	//	if (!strcmp(pEvent->GetName(), "weapon_fire")) {

	//		int iEntIndex = pEvent->GetInt("userid");

	//		if (i::EngineClient->GetPlayerForUserID(iEntIndex) != i::EngineClient->GetLocalPlayer())
	//			return;

	//		deqBulletImpacts.clear();
	//	}
	//	
	//	else if (!strcmp(pEvent->GetName(), "player_hurt")) {

	//		int iEntIndex = pEvent->GetInt("userid");
	//		int iAttackerIndex = pEvent->GetInt("attacker");

	//		if (i::EngineClient->GetPlayerForUserID(iEntIndex) == i::EngineClient->GetLocalPlayer())
	//			return;

	//		if (i::EngineClient->GetPlayerForUserID(iAttackerIndex) != i::EngineClient->GetLocalPlayer())
	//			return;

	//		bPlayerHurt[iEntIndex] = true;
	//	}

	//	else if (!strcmp(pEvent->GetName(), "bullet_impact")) {

	//		if (targetedRecord == nullptr)
	//			return;

	//		int iEntIndex = pEvent->GetInt("userid");

	//		if (i::EngineClient->GetPlayerForUserID(iEntIndex) != i::EngineClient->GetLocalPlayer())
	//			return;

	//		deqBulletImpacts.emplace_front(Vector(pEvent->GetFloat("x"), pEvent->GetFloat("y"), pEvent->GetFloat("z")));
	//	}

	//	else if (!strcmp(pEvent->GetName(), "round_start")) {

	//		for (int i = 0; i < 65; i++) {
	//			iMissedShots[i] = 0;
	//		}
	//	}

	//	else if (!strcmp(pEvent->GetName(), "player_death")) {

	//		int iAttackerIndex = pEvent->GetInt("attacker");

	//		if (i::EngineClient->GetPlayerForUserID(iAttackerIndex) != i::EngineClient->GetLocalPlayer())
	//			return;

	//		deqBulletImpacts.clear();
	//	}
	//}


	//inline void Resolver(CBaseEntity* pEnt, LagComp::playerrecord_t* pRecord, LagComp::playerrecord_t* pPrevious, bool bResolve, CAnimState* pAnimstate) {

	//	if (!bResolve || !cfg::rage::resolver || !pPrevious)
	//		return;

	//	CheckShootsMissed(pEnt);

	//	static float oldYaw = 0.f;

	//	auto EntIndex = pEnt->EntIndex();

	//	float flLowerBody = M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget);

	//	bool bHasLowerBody = std::fabs(flLowerBody) >= 35.f && pRecord->layer[6].flWeight == 0.000f;

	//	bool bResolveable = pRecord->layer[6].flWeight > 0.000f;

	//	bool bSameVelocity = pRecord->layer[6].flWeight == pPrevious->layer[6].flWeight;
	//	float flPlaybackDifference = pRecord->layer[6].flPlaybackRate - pPrevious->layer[6].flPlaybackRate;

	//	static float yawResolved = 0.f;
	//	if (iMissedShots[EntIndex] == 0) {

	//		if (bResolveable) {

	//			if (bSameVelocity) {
	//				if (flPlaybackDifference > 0.f) {
	//					yawResolved = pAnimstate->GetMaxDesync();
	//					pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y + pAnimstate->GetMaxDesync() /** flLastDelta*/;
	//				}
	//				if (flPlaybackDifference < 0.f) {
	//					yawResolved = -pAnimstate->GetMaxDesync();
	//					pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - pAnimstate->GetMaxDesync() /** flLastDelta*/;
	//				}
	//			}
	//			else if (flLowerBody != 0.f) {
	//				if (flLowerBody < 0.f) {
	//					yawResolved = pAnimstate->GetMaxDesync();
	//					pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y + pAnimstate->GetMaxDesync();
	//				}
	//				if (flLowerBody > 0.f) {
	//					yawResolved = -pAnimstate->GetMaxDesync();
	//					pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - pAnimstate->GetMaxDesync();
	//				}
	//			}
	//		}
	//		else if (bHasLowerBody) {
	//			yawResolved = (flLowerBody * pAnimstate->flYawModifier() / 2);
	//			pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - (flLowerBody * pAnimstate->flYawModifier() / 2);
	//		}
	//	}
	//	else {

	//		switch (iMissedShots[EntIndex] % 3) {

	//		case 0:
	//			pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y + yawResolved;
	//			break;

	//		case 1:
	//			pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y;
	//			break;

	//		case 2:
	//			pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - yawResolved;
	//			break;
	//		}

	//		//switch (lastShotAngle[EntIndex]) {

	//		//case RIGHT: 
	//		//	pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y + pAnimstate->GetMaxDesync();
	//		//	lastShotAngle[EntIndex] = LEFT;
	//		//	break;

	//		//case LEFT: 
	//		//	pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - (flLowerBody * pAnimstate->flYawModifier() / 2);
	//		//	lastShotAngle[EntIndex] = LOW;
	//		//	break;

	//		//case LOW: 
	//		//	pAnimstate->flGoalFeetYaw = pEnt->GetEyeAngles().y - pAnimstate->GetMaxDesync();
	//		//	lastShotAngle[EntIndex] = RIGHT;
	//		//	break;
	//		//}
	//	}
	//}
}