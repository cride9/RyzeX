#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../xorstr.h"

void Animations::ResolverLogic() {

	if (!ragebot.hitlogData.pAimbotTarget || !g::pLocal || bulletImpact == Vector(0, 0, 0) || !ragebot.hitlogData.pTargetMatrix)
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
	autowall.GetDamage(g::pLocal, refCurrentData.vecLocalShootPosition, bulletImpact, g::pLocal->GetWeapon(), &data);

	// Check if we killed, or hurt the player
	if (bResolverHandler[PLAYERHURT] || bResolverHandler[PLAYERDEATH]) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;

		pLog->iHitSide[refCurrentData.pRecord->iResolveSide]++;
		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}]"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
		return;
	}

	// Check again just to be safe
	if (pTarget->GetHealth() < refCurrentData.iHealth || !pTarget->IsAlive()) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++; 

		pLog->iHitSide[refCurrentData.pRecord->iResolveSide]++;
		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}]"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage
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
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: invalid record or resolver on backtrack"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: resolver"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: correction"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		// check for occlusion
		Trace_t traceData;
		Ray_t rayData(refCurrentData.vecLocalShootPosition, bulletImpact);
		CTraceFilter filterData(g::pLocal, TRACE_ENTITIES_ONLY);
		i::EngineTrace->TraceRay(rayData, MASK_SHOT | CONTENTS_GRATE, &filterData, &traceData);

		if (traceData.pHitEntity != nullptr && traceData.pHitEntity == refCurrentData.pAimbotTarget || data.flCurrentDamage == 0.f) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: occlusion"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: spread"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage
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
	if (szEventName.find(cachedEvents::bulletImpact) != std::string_view::npos) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));

		if (iUser != i::EngineClient->GetLocalPlayer())
			return;

		bulletImpact = Vector(pEvent->GetFloat(XorStr("x")), pEvent->GetFloat(XorStr("y")), pEvent->GetFloat(XorStr("z")));
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
}

void SetResolveMatrix(Lagcompensation::AnimationInfo_t* pLog, Lagcompensation::LagRecord_t* pRecord, EMatrixType iType) {

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	if (ragebot.SafePoint(g::vecEyePosition, pWeapon, pRecord, pRecord->pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[iType]), HITBOX_HEAD) >= 3)
		pRecord->bResolved = true;
	else
		pRecord->bResolved = false;

	memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[iType], sizeof(matrix3x4_t) * 128);
	pLog->iLastResolve = iType;
}

float GetVelocityLengthXY(CBaseEntity* pEntity)
{
	Lagcompensation::AnimationInfo_t* pRecord = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pRecord)
		return 0.f;

	float flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence);
	float flSequenceGroundSpeed = fmax(pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence) / (1.0f / flSequenceCycleRate), 0.001f);

	return (pEntity->GetAnimationOverlays()[6].flPlaybackRate / flSequenceCycleRate) / (1.0f - (pRecord->flWalkToRunTransition * 0.15f)) * (flSequenceGroundSpeed) / pEntity->AnimState()->flLastUpdateIncrement;
}

void SetYaw(Lagcompensation::LagRecord_t* pRecord, EMatrixType flYaw) {

	CAnimState* pState = pRecord->pEntity->AnimState();
	switch (flYaw) {

	case LEFT:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flDesyncDelta);
		pRecord->iResolveSide = LEFT;
		break;

	case CENTER:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y);
		pRecord->iResolveSide = CENTER;
		break;

	case RIGHT:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y + pRecord->flDesyncDelta);
		pRecord->iResolveSide = RIGHT;
		break;
	}
}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pPrevious || !pRecord)
		return;

	if (!cfg::rage::resolver)
		return;

	CAnimState* pState = pEntity->AnimState();
	if (!pState)
		return;

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pLog)
		return;

	if (arrMissedShots[pEntity->EntIndex()] == 0 || pPrevious->iResolveSide == 0 && !(pLog->iHitSide[LEFT] < 3 && pLog->iHitSide[RIGHT] < 3 && pLog->iHitSide[CENTER] < 3)) {

		if (pLog->iHitSide[LEFT] > pLog->iHitSide[RIGHT]) {

			SetYaw(pRecord, LEFT);
		}
		else if (pLog->iHitSide[LEFT] < pLog->iHitSide[RIGHT]) {

			SetYaw(pRecord, RIGHT);
		}
		else if (pLog->iHitSide[LEFT] == pLog->iHitSide[RIGHT]) {

			SetYaw(pRecord, RIGHT);
			pLog->bJitterAntiAim = true;
		}
		else {
			SetYaw(pRecord, RIGHT);
		}
		return;
	}

	static std::array<int, 65> arrMissedShotsBackup{0};
	if (arrMissedShots[pEntity->EntIndex()] != arrMissedShotsBackup[pEntity->EntIndex()] && arrMissedShots[pEntity->EntIndex()] > 0) {

		switch (pLog->iLastResolve) {

		case RIGHT:
			SetYaw(pRecord, LEFT);
			break;

		case LEFT:
			SetYaw(pRecord, CENTER);
			break;

		case CENTER:
			SetYaw(pRecord, RIGHT);
			break;
		}
		arrMissedShots[pEntity->EntIndex()] = arrMissedShotsBackup[pEntity->EntIndex()];
		return;
	}
	else {

		switch (pPrevious->iResolveSide) {

		case LEFT:
			SetYaw(pRecord, LEFT);
			break;

		case CENTER:
			SetYaw(pRecord, CENTER);
			break;

		case RIGHT:
			SetYaw(pRecord, RIGHT);
			break;
		}
	}

	//switch (arrMissedShots[pEntity->EntIndex()] % 3) {

	//case 0: pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y + pRecord->flDesyncDelta);
	//	pRecord->iResolveSide = RIGHT;
	//	break;

	//case 1: pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flDesyncDelta);
	//	pRecord->iResolveSide = LEFT;
	//	break;

	//case 2: pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y);
	//	pRecord->iResolveSide = CENTER;
	//	break;
	//}
}