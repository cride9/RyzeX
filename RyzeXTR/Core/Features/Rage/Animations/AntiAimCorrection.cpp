#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../xorstr.h"

void Animations::ResolverLogic() {

	if (!ragebot.hitlogData.pAimbotTarget || !g::pLocal || bBulletImpact == Vector(0, 0, 0) || !ragebot.hitlogData.pTargetMatrix)
		return;

	// make pointers and references for easier handling
	auto& refCurrentData = ragebot.hitlogData;
	CBaseEntity* pTarget = refCurrentData.pAimbotTarget;
	auto* pLog = &lagcomp.GetLog(refCurrentData.pAimbotTarget->EntIndex());
	if (!pLog)
		return;

	// get player name and info
	PlayerInfo_t info;
	i::EngineClient->GetPlayerInfo(pTarget->EntIndex(), &info);

	// check if we did shot & had an impact
	if (!bResolverHandler[WEAPONFIRE] || !bResolverHandler[BULLETIMPACT]) 
		return;
	
	// apply shot matrix
	refCurrentData.pRecord->ApplyMatrix(pTarget, RESOLVE);

	// Simulate a bullet shot
	FireBulletData_t data;
	autowall.GetDamage(g::pLocal, refCurrentData.vecLocalShootPosition, bBulletImpact, g::pLocal->GetWeapon(), &data);

	// Check if we killed, or hurt the player
	if (bResolverHandler[PLAYERHURT] || bResolverHandler[PLAYERDEATH]) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}] | [yaw] {}"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
		return;
	}

	// Check again just to be safe
	if (pTarget->GetHealth() < refCurrentData.iHealth || !pTarget->IsAlive()) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++; 

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}] | [yaw] {}"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
		return;
	}

	// If we hit an entity but didn't deal any dmg its a resolver miss
	if (data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity == pTarget) {

		anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
		pLog->iLastResolve = refCurrentData.pRecord->iResolveSide;

		if (refCurrentData.bBacktrack) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: invalid record or resolver on backtrack"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: resolver"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: correction"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			));
			refCurrentData.ClearTarget();
			return;
		}

		// check for occlusion
		Trace_t traceData;
		Ray_t rayData(refCurrentData.vecLocalShootPosition, bBulletImpact);
		CTraceFilter filterData(g::pLocal, TRACE_ENTITIES_ONLY);
		i::EngineTrace->TraceRay(rayData, MASK_SHOT | CONTENTS_GRATE, &filterData, &traceData);

		if (traceData.pHitEntity != nullptr && traceData.pHitEntity == refCurrentData.pAimbotTarget || data.flCurrentDamage == 0.f) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: occlusion"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: spread"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
	}
}

void Animations::ResolverHandler(IGameEvent* pEvent) {

	if (!ragebot.hitlogData.pAimbotTarget || !g::pLocal)
		return;

	std::string_view szEventName = pEvent->GetName();

	if (szEventName.find(cachedEvents::weaponFire) != std::string_view::npos) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));

		if (iUser == i::EngineClient->GetLocalPlayer()) {
			bResolverHandler[WEAPONFIRE] = true;
		}
	}
	if (szEventName.find(cachedEvents::playerHurt) != std::string_view::npos) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));
		auto iAttacker = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("attacker")));
		auto targetIndex = ragebot.hitlogData.pAimbotTarget->EntIndex();

		if (iAttacker == i::EngineClient->GetLocalPlayer() && iUser == targetIndex) {
			bResolverHandler[PLAYERHURT] = true;
			iHitDmg = pEvent->GetInt(XorStr("dmg_health"));
			iHitHitbox = pEvent->GetInt(XorStr("hitgroup"));
		}
	}
	if (szEventName.find(cachedEvents::bBulletImpact) != std::string_view::npos) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));

		if (iUser != i::EngineClient->GetLocalPlayer())
			return;

		bBulletImpact = Vector(pEvent->GetFloat(XorStr("x")), pEvent->GetFloat(XorStr("y")), pEvent->GetFloat(XorStr("z")));
		bResolverHandler[BULLETIMPACT] = true;
	}
	if (szEventName.find(cachedEvents::playerDeath) != std::string_view::npos) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));
		auto iAttacker = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("attacker")));
		auto targetIndex = ragebot.hitlogData.pAimbotTarget->EntIndex();

		if (iAttacker == i::EngineClient->GetLocalPlayer() && iUser == targetIndex) {
			bResolverHandler[PLAYERDEATH] = true;
		}
	}
	if (szEventName.find(cachedEvents::roundStart) != std::string_view::npos) {

		for (size_t i = 0; i < 65; i++)
			lagcomp.GetLog(i).ClearData();
	}
}

void SetResolveMatrix(Lagcompensation::LagRecord_t* pRecord, int iType) {

	//memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[iType], sizeof(matrix3x4_t) * 128);
	pRecord->iResolveSide = iType;
}

float Animations::GetVelocityLengthXY(CBaseEntity* pEntity)
{
	Lagcompensation::AnimationInfo_t* pRecord = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pRecord)
		return 0.f;

	float flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence);
	float flSequenceGroundSpeed = fmax(pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence) / (1.0f / flSequenceCycleRate), 0.001f);

	return (pEntity->GetAnimationOverlays()[6].flPlaybackRate / flSequenceCycleRate) / (1.0f - (pEntity->AnimState()->flWalkToRunTransition * 0.15f)) * (flSequenceGroundSpeed) / pEntity->AnimState()->flLastUpdateIncrement;
}

void Animations::SetYaw(Lagcompensation::LagRecord_t* pRecord, int flYaw) {

	CAnimState* pState = pRecord->pEntity->AnimState();
	pRecord->iResolveSide = flYaw;
	switch (flYaw) {

	case LEFT:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, LEFT)/*M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flDesyncDelta)*/;
		pRecord->flResolveDelta = -pRecord->flDesyncDelta;
		break;

	case CENTER:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, CENTER)/*M::NormalizeYaw(pRecord->vecEyeAngles.y)*/;
		pRecord->flResolveDelta = 0.f;
		break;

	case RIGHT:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, RIGHT)/*M::NormalizeYaw(pRecord->vecEyeAngles.y + pRecord->flDesyncDelta)*/;
		pRecord->flResolveDelta = pRecord->flDesyncDelta;
		break;
	}
}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pRecord || !pPrevious)
		return;

#ifdef NDEBUG
	if (pEntity->GetPlayerInfo().bFakePlayer)
		return;
#endif

	if (!cfg::rage::bResolver)
		return;

	static std::array<int, 65> iMissTracker{0};
	const int iEntityID = pEntity->EntIndex();
	return SetYaw(pRecord, LEFT + (arrMissedShots[iEntityID] % 3));

	if (pPrevious->iAntiFreestand != 0 && arrMissedShots[iEntityID] == 0)
		return SetYaw(pRecord, pPrevious->iAntiFreestand);

	if (pPrevious->iLayerResolve != 0 && arrMissedShots[iEntityID] == 0)
		return SetYaw(pRecord, pPrevious->iLayerResolve);

	int& it = pPrevious->iResolveSide;
	if (it == VISUAL)
		it = RIGHT;

	if (iMissTracker[iEntityID] != arrMissedShots[iEntityID])
		SetYaw(pRecord, it == RIGHT ? LEFT : it == LEFT ? CENTER : RIGHT);

	if (iMissTracker[iEntityID] == arrMissedShots[iEntityID])
		SetYaw(pRecord, it);

	iMissTracker[iEntityID] = arrMissedShots[iEntityID];
}

void Animations::PostResolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pRecord || g::bUpdatingSkins || i::ClientState->iDeltaTick <= 0)
		return;

	const int iEntityID = pEntity->EntIndex();
	if (arrMissedShots[iEntityID] != 0)
		return;

	if (auto* pLog = &lagcomp.GetLog(iEntityID); pLog && pLog->pRecord.size() > 3) {

		auto* pBefore = &pLog->pRecord.at(1);

		// from the server.
		auto flFromServerPlaybackrate = pRecord->pLayers[6].flPlaybackRate * 1000000.0f;

		// resolver calculations.
		const float fCenterPlaybackrate = pRecord->LayerData[CENTER].flPlaybackRate * 1000000.0f;
		const float fRightPlaybackrate = pRecord->LayerData[RIGHT].flPlaybackRate * 1000000.0f;
		const float fLeftPlaybackrate = pRecord->LayerData[LEFT].flPlaybackRate * 1000000.0f;

		// differences.
		pRecord->flLayerDifferences[CENTER] = fabsf(flFromServerPlaybackrate - fCenterPlaybackrate);
		pRecord->flLayerDifferences[RIGHT] = fabsf(flFromServerPlaybackrate - fRightPlaybackrate);
		pRecord->flLayerDifferences[LEFT] = fabsf(flFromServerPlaybackrate - fLeftPlaybackrate);

		if (GetVelocityLengthXY(pEntity) > 0.f && 
			!pRecord->pLayers[12].flWeight && 
			(pBefore && 
				(pRecord->pLayers[6].flWeight == pBefore->pLayers[6].flWeight ||
					pRecord->vecVelocity.Length2D() > 135.f)
				&& pRecord->pLayers[6].flWeight == 1.f))
		{
			if (pRecord->flLayerDifferences[CENTER] < pRecord->flLayerDifferences[RIGHT] &&
				pRecord->flLayerDifferences[CENTER] < pRecord->flLayerDifferences[LEFT] &&
				pBefore->flLayerDifferences[CENTER] < pBefore->flLayerDifferences[RIGHT] &&
				pBefore->flLayerDifferences[CENTER] < pBefore->flLayerDifferences[LEFT])
				pRecord->iLayerResolve = CENTER;

			else if (
				pRecord->flLayerDifferences[RIGHT] < pRecord->flLayerDifferences[CENTER] &&
				pRecord->flLayerDifferences[RIGHT] < pRecord->flLayerDifferences[LEFT] &&
				pBefore->flLayerDifferences[RIGHT] < pBefore->flLayerDifferences[CENTER] &&
				pBefore->flLayerDifferences[RIGHT] < pBefore->flLayerDifferences[LEFT])
				pRecord->iLayerResolve = RIGHT;

			else if (
				pRecord->flLayerDifferences[LEFT] < pRecord->flLayerDifferences[CENTER] &&
				pRecord->flLayerDifferences[LEFT] < pRecord->flLayerDifferences[RIGHT] &&
				pBefore->flLayerDifferences[LEFT] < pBefore->flLayerDifferences[CENTER] &&
				pBefore->flLayerDifferences[LEFT] < pBefore->flLayerDifferences[RIGHT])
				pRecord->iLayerResolve = LEFT;
		}

	}

	CTraceFilter filterRight(g::pLocal), filterLeft(g::pLocal);
	Trace_t dataRight, dataLeft;

	i::EngineTrace->TraceRay(Ray_t(g::vecEyePosition, pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[RIGHT])), MASK_SOLID & ~CONTENTS_MONSTER, &filterRight, &dataRight);
	i::EngineTrace->TraceRay(Ray_t(g::vecEyePosition, pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[LEFT])), MASK_SOLID & ~CONTENTS_MONSTER, &filterLeft, &dataLeft);

	bool bTraceRight = dataRight.pHitEntity == pEntity;
	bool bTraceLeft = dataLeft.pHitEntity == pEntity;

	static std::array<int, 65> iFoundHit{0};

	if (!bTraceRight && !bTraceLeft)
		iFoundHit[iEntityID] = 0;

	pRecord->iAntiFreestand = iFoundHit[iEntityID];
	if (iFoundHit[iEntityID] != 0) 
		return;

	pRecord->iAntiFreestand = iFoundHit[iEntityID] = bTraceLeft ? !bTraceRight ? RIGHT : 0 : bTraceRight ? LEFT : 0;
}