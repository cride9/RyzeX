#include "EnemyAnimations.h"
#include "../aimbot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../xorstr.h"

void Animations::ResolverLogic() {

	if (!aimbot.GetHitLogData().pAimbotTarget || !g::pLocal || bBulletImpact == Vector(0, 0, 0) || !aimbot.GetHitLogData().pTargetMatrix)
		return;

	// make pointers and references for easier handling
	auto& refCurrentData = aimbot.GetHitLogData();
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
			("Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}"),
			info.szName,
			misc::GetHitgroupName(iHitHitbox),
			iHitDmg,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
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
			("Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}"),
			info.szName,
			misc::GetHitgroupName(iHitHitbox),
			iHitDmg,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
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
				("Missed {}'s {} due to invalid record or resolver on backtrack. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"),
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {}'s {} due to resolver. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"),
			info.szName,
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {}'s {} due to correction. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"),
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
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

		bool bOccluded = (bBulletImpact - refCurrentData.vecLocalShootPosition).LengthSqr() < (refCurrentData.vecTargetShootPosition - refCurrentData.vecLocalShootPosition).LengthSqr();
		if (bOccluded) { // traceData.pHitEntity != nullptr && traceData.pHitEntity == refCurrentData.pAimbotTarget || data.flCurrentDamage == 0.f

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {}'s {} due to occlusion. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"),
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {}'s {} due to spread. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"),
			info.szName,
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		));
		refCurrentData.ClearTarget();
	}
}

void Animations::ResolverHandler(IGameEvent* pEvent) {

	if (!aimbot.GetHitLogData().pAimbotTarget || !g::pLocal)
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
		auto targetIndex = aimbot.GetHitLogData().pAimbotTarget->EntIndex();

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
		auto targetIndex = aimbot.GetHitLogData().pAimbotTarget->EntIndex();

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

	if (flYaw == VISUAL)
		flYaw = LEFT;
	pRecord->iResolveSide = flYaw;
	switch (flYaw) {

	case LEFT:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, LEFT);
		pRecord->flResolveDelta = -pRecord->flDesyncDelta;
		break;

	case CENTER:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, CENTER);
		pRecord->flResolveDelta = 0.f;
		break;

	case RIGHT:
		pState->flGoalFeetYaw = GetYawRotation(pRecord, RIGHT);
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

	//if (pRecord->flGuessedYaw != 0.f && arrMissedShots[iEntityID] == 0) {
	//	pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y + pRecord->flGuessedYaw);
	//	return;
	//}
	//return SetYaw(pRecord, LEFT + (arrMissedShots[iEntityID] % 3));

	if (arrMissedShots[iEntityID] != 0) {
		if (iMissTracker[iEntityID] != arrMissedShots[iEntityID]) {

			switch (pPrevious->iResolveSide) {
			case RIGHT:
				iMissTracker[iEntityID] = arrMissedShots[iEntityID];
				return SetYaw(pRecord, LEFT);
			case LEFT:
				iMissTracker[iEntityID] = arrMissedShots[iEntityID];
				return SetYaw(pRecord, CENTER);
			case CENTER:
				iMissTracker[iEntityID] = arrMissedShots[iEntityID];
				return SetYaw(pRecord, RIGHT);
			};
		}
		return SetYaw(pRecord, pPrevious->iResolveSide);
	}

	static int iFirstHitSide[65]{0};

	CTraceFilter traceFilter = CTraceFilter(g::pLocal);
	Vector vecLeft = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pSideMatrixes[0]);
	Vector vecRight = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pSideMatrixes[1]);

	Trace_t traceLeft;
	i::EngineTrace->TraceRay(Ray_t(g::vecEyePosition, vecLeft), MASK_SHOT, &traceFilter, &traceLeft);
	
	Trace_t traceRight;
	i::EngineTrace->TraceRay(Ray_t(g::vecEyePosition, vecRight), MASK_SHOT, &traceFilter, &traceRight);

	bool bHitLeft = traceLeft.pHitEntity == pEntity;
	bool bHitRight = traceRight.pHitEntity == pEntity;

	if (!bHitLeft && !bHitRight)
		iFirstHitSide[iEntityID] == VISUAL;

	if (iFirstHitSide[iEntityID] != VISUAL)
		return SetYaw(pRecord, VISUAL);

	else if (bHitLeft && !bHitRight)
		iFirstHitSide[iEntityID] == RIGHT;

	else if (!bHitLeft && bHitRight)
		iFirstHitSide[iEntityID] == LEFT;

	iMissTracker[iEntityID] = arrMissedShots[iEntityID];
}