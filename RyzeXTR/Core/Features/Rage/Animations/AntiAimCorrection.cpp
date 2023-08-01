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
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}] | [yaw] {}"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage,
			refCurrentData.pRecord->flDesyncDelta
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
			("Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}] | [yaw] {}"),
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitGroup),
			iHitDmg, refCurrentData.flDamage,
			refCurrentData.pRecord->flDesyncDelta
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
				refCurrentData.pRecord->flDesyncDelta
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
			refCurrentData.pRecord->flDesyncDelta
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
				refCurrentData.pRecord->flDesyncDelta
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
				("Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | [yaw] {} | missed due to: occlusion"),
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flDesyncDelta
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
			refCurrentData.pRecord->flDesyncDelta
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

void SetResolveMatrix(Lagcompensation::LagRecord_t* pRecord, int iType) {

	//memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[iType], sizeof(matrix3x4_t) * 128);
	pRecord->iResolveSide = iType;
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

void Animations::SetYaw(Lagcompensation::LagRecord_t* pRecord, int flYaw) {

	CAnimState* pState = pRecord->pEntity->AnimState();
	pRecord->iResolveSide = flYaw;
	switch (flYaw) {

	case LEFT:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flDesyncDelta);
		break;

	case CENTER:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y);
		break;

	case RIGHT:
		pState->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y + pRecord->flDesyncDelta);
		break;
	}
}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pPrevious || !pRecord)
		return;

#ifdef NDEBUG
	if (pEntity->GetPlayerInfo().bFakePlayer)
		return;
#endif

	if (!cfg::rage::resolver)
		return;

	const int iEntityID = pEntity->EntIndex();
	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(iEntityID);
	if (!pLog)
		return;

	static std::array<int, 65> arrMissedShotsBackup{0};
	if (arrMissedShotsBackup[iEntityID] != arrMissedShots[iEntityID] && arrMissedShots[iEntityID] > 0) {

		switch (pLog->iLastResolve) {

		case LEFT: SetYaw(pRecord, RIGHT);
			break;

		case VISUAL: SetYaw(pRecord, LEFT);
			break;

		case RIGHT: SetYaw(pRecord, VISUAL);
			break;
		}

		arrMissedShotsBackup[iEntityID] = arrMissedShots[iEntityID];
	}
	else if (pPrevious->iResolveSide > 1)
		SetYaw(pRecord, pPrevious->iResolveSide);
	else 
		SetYaw(pRecord, LEFT);
	
}

void Animations::PostResolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pRecord)
		return;

	const int iEntityID = pEntity->EntIndex();
	if (arrMissedShots[iEntityID] != 0)
		return;

	int iHitLeft = autowall.GetDamage(pLocal, g::vecEyePosition, pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[LEFT]), pLocal->GetWeapon());
	int iHitRight = autowall.GetDamage(pLocal, g::vecEyePosition, pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[RIGHT]), pLocal->GetWeapon());

	cfg::debugSlider1 = iHitLeft;
	cfg::debugSlider2 = iHitRight;

	static std::array<int, 65> iFirstHitSide{0};

	if (iHitLeft < 0.f && iHitRight < 0.f) {
		iFirstHitSide[iEntityID] = 0;
		return;
	}

	if (iFirstHitSide[iEntityID] == LEFT || iFirstHitSide[iEntityID] == RIGHT) {
		SetResolveMatrix(pRecord, iFirstHitSide[iEntityID]);
		return;
	}

	if (iHitLeft < iHitRight)
		iFirstHitSide[iEntityID] = LEFT;

	if (iHitLeft > iHitRight)
		iFirstHitSide[iEntityID] = RIGHT;

	if (iFirstHitSide[iEntityID] == LEFT || iFirstHitSide[iEntityID] == RIGHT)
		SetResolveMatrix(pRecord, iFirstHitSide[iEntityID]);
}