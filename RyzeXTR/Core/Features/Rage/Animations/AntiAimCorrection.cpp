#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"

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

	//if (refCurrentData.iTickcount + TIME_TO_TICKS(0.1f) < g::pCmd->iTickCount)
	//	return;

	// check if we did shot & had an impact
	if (!bResolverHandler[WEAPONFIRE] || !bResolverHandler[BULLETIMPACT]) {
		//misc::Print("No log due to event delay??!?!");
		//bResolverHandler = std::array<bool, HANDLERCOUNT>();
		//refCurrentData.ClearTarget();
		return;
	}

	// Check if we killed, or hurt the player
	if (bResolverHandler[PLAYERHURT] || bResolverHandler[PLAYERDEATH]) {

		if (pLog->iFreestandMatrix == pLog->iLastResolve)
			pLog->iPeekingReal++;
		else
			pLog->iPeekingFake++;

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			"Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}]",
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitbox),
			iHitDmg, refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
		return;
	}

	// Check again just to be safe
	if (pTarget->GetHealth() < refCurrentData.iHealth || !pTarget->IsAlive()) {

		if (pLog->iFreestandMatrix == pLog->iLastResolve)
			pLog->iPeekingReal++;
		else
			pLog->iPeekingFake++;

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			"Hit {} | [hc] {} | [bt] {} | [hg] {} [aimed: {}] | [dmg] {} [aimed: {}]",
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(iHitHitbox), misc::GetHitgroupName(refCurrentData.iHitbox),
			iHitDmg, refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
		return;
	}

	// apply shot matrix
	refCurrentData.pRecord->ApplyMatrix(pTarget, RESOLVE);

	// Simulate a bullet shot
	FireBulletData_t data;
	autowall.GetDamage(g::pLocal, refCurrentData.vecLocalShootPosition, bulletImpact, g::pLocal->GetWeapon(), &data);

	// If we hit an entity but didn't deal any dmg its a resolver miss
	if (data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity == pTarget) {

		anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;

		if (pLog->iFreestandMatrix == pLog->iLastResolve)
			pLog->iPeekingFake++;
		else
			pLog->iPeekingReal++;

		if (refCurrentData.bBacktrack) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
			misc::Print(std::format(
				"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: invalid record",
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitbox),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: resolver",
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitbox),
			refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: correction",
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitbox),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		// check for occlusion
		Trace_t traceData;
		Ray_t rayData(refCurrentData.vecLocalShootPosition, bulletImpact);
		CTraceFilter filterData(g::pLocal, TRACE_ENTITIES_ONLY);
		i::EngineTrace->TraceRay(rayData, MASK_SHOT, &filterData, &traceData);

		if (traceData.pHitEntity != nullptr && traceData.pHitEntity == refCurrentData.pAimbotTarget || data.flCurrentDamage == 0.f) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: occlusion",
				info.szName,
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
				misc::GetHitgroupName(refCurrentData.iHitbox),
				refCurrentData.flDamage
			));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::format(
			"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: spread",
			info.szName,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.pRecord->flSimulationTime)),
			misc::GetHitgroupName(refCurrentData.iHitbox),
			refCurrentData.flDamage
		));
		refCurrentData.ClearTarget();
	}
}

void Animations::ResolverHandler(IGameEvent* pEvent) {

	if (!ragebot.hitlogData.pAimbotTarget || !g::pLocal)
		return;

	if (!strcmp(pEvent->GetName(), "weapon_fire")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

		if (iUser == i::EngineClient->GetLocalPlayer()) {
			bResolverHandler[WEAPONFIRE] = true;
		}
	}
	if (!strcmp(pEvent->GetName(), "player_hurt")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto iAttacker = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
		auto targetIndex = ragebot.hitlogData.pAimbotTarget->EntIndex();

		if (iAttacker == i::EngineClient->GetLocalPlayer() && iUser == targetIndex) {
			bResolverHandler[PLAYERHURT] = true;
			iHitDmg = pEvent->GetInt("dmg_health");
			iHitHitbox = pEvent->GetInt("hitgroup");
		}
	}
	if (!strcmp(pEvent->GetName(), "bullet_impact")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

		if (iUser != i::EngineClient->GetLocalPlayer())
			return;

		bulletImpact = Vector(pEvent->GetFloat("x"), pEvent->GetFloat("y"), pEvent->GetFloat("z"));
		bResolverHandler[BULLETIMPACT] = true;
	}
	if (!strcmp(pEvent->GetName(), "player_death")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto iAttacker = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
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

void SetYaw(Lagcompensation::LagRecord_t* pRecord, float flYaw) {

	float flOldEyeYaw = pRecord->pEntity->AnimState()->flEyeYaw;

	// set eye yaw
	float flEyeRotation = pRecord->vecEyeAngles.y;
	pRecord->pEntity->AnimState()->flEyeYaw = M::NormalizeAngle(flEyeRotation + flYaw);

	// generate foot yaw
	float flFootYaw = anims.BuildFootYaw(pRecord->pEntity, pRecord);

	// restore eye yaw                                   
	pRecord->pEntity->AnimState()->flEyeYaw = flOldEyeYaw;

	// return result
	pRecord->pEntity->AnimState()->flGoalFeetYaw = flFootYaw;
}

//void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {
//
//	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
//	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pPrevious || !pRecord)
//		return;
//
//	if (!cfg::rage::resolver)
//		return;
//
//	int iEntIndex = pEntity->EntIndex();
//
//	auto& pData = lagcomp.GetLog(iEntIndex);
//	if (!&pData)
//		return;
//
//	static std::array<float, 65> flOldLowerbodyYaw{0};
//	static std::array<float, 65> flOldPlaybackrateYaw{0};
//
//	// use it if not gonna use the resolver.
//	float flOldGoalFeetYaw = pEntity->AnimState( )->flGoalFeetYaw;
//
//	// bot or either no resolver.
//	if (!cfg::rage::resolver || pEntity->GetPlayerInfo().bFakePlayer) {
//		// reset missed shots.
//		pData.iAntiAimType = Lagcompensation::LEGIT;
//		pData.iDesyncSide = NULL;
//
//		// reset the data.
//		pData.flTimeSinceBreakingLBY = 0.f;
//		pData.flTimeSinceBodySwayRight = 0.f;
//		pData.flTimeSinceBodySwayLeft = 0.f;
//		pData.flTimeSinceBodySwaying = 0.f;
//		pData.flTimeSinceNoDesync = 0.f;
//
//		// lets go back to old feet yaw faster.
//		pEntity->AnimState()->flGoalFeetYaw = flOldGoalFeetYaw;
//		return;
//	}
//
//	// the angle.
//	flGuessedYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pEntity->AnimState()->flGoalFeetYaw);
//
//	//if ( IPT::HandleInput( C::Get<int>( Vars.iRageResolverOverrideKey ) ) )
//	//{
//	//	// overriding.
//	//	pData.iAntiAimType = Animations::OVERRIDE;
//
//	//	Vector vecPosEnemy;
//	//	if ( !I::DebugOverlay->WorldToScreen( pRecord->vecOrigin, vecPosEnemy ) )
//	//		return;
//
//	//	POINT pMouse;
//	//	GetCursorPos( &pMouse );
//	//	float flDelta = pMouse.x - vecPosEnemy.x;
//
//	//	// set yaw.
//	//	flGuessedYaw = std::clamp( flDelta, -58.f, 58.f );
//
//	//	// desync side info.
//	//	pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
//
//	//	// fix legit anti-aim.
//	//	if ( pRecord->flDeltaAngle <= 90.f )
//	//		flGuessedYaw *= -1.f;
//
//	//	// apply the resolver.
//	//	pEntity->GetAnimationState( )->flGoalFeetYaw = M::NormalizeYaw( pRecord->angEyeAngles.y ) - flGuessedYaw;
//	//	return;
//	//}
//
//	// breaking the lowerbody.
//	if (fabsf(M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget)) > 35.f)
//	{
//		// its breaking lby so its opposite.
//		pData.iAntiAimType = Lagcompensation::OPPOSITE;
//
//		// its less than 179.998f you might say.
//		if (fabsf(M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget)) < 175.f)
//		{
//			// set to the opposite lowerbody.
//			flGuessedYaw = std::clamp(M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget), -60.f, 60.f) * -1.f;
//
//			// save the correct lby data.
//			if (flOldLowerbodyYaw[pEntity->EntIndex()] != flGuessedYaw)
//				flOldLowerbodyYaw[pEntity->EntIndex()] = flGuessedYaw;
//		}
//		// set the last saved data.
//		else if (fabsf(flOldLowerbodyYaw[pEntity->EntIndex()]) > 0.f)
//			// thats how to resolve onetap.
//			flGuessedYaw = flOldLowerbodyYaw[pEntity->EntIndex()];
//		// we don't have data so i assume its opposite.
//		else flGuessedYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - flOldGoalFeetYaw) * -1.f;
//
//		// max desync detection.
//		if (pRecord->pLayers[3].flCycle != 0.f || pRecord->pLayers[3].flWeight != 0.f)
//			++pData.flTimeSinceNoDesync;
//		else pData.flTimeSinceNoDesync = 0.f;
//
//		// increase while breaking.
//		++pData.flTimeSinceBreakingLBY;
//
//		// use the correct data.
//		if (M::NormalizeYaw(pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget) <= 0.f) {
//			pData.flTimeSinceBodySwayRight = 0.f;
//			++pData.flTimeSinceBodySwayLeft;
//		}
//		else {
//			pData.flTimeSinceBodySwayLeft = 0.f;
//			++pData.flTimeSinceBodySwayRight;
//		}
//
//		// this is probably sway anti-aim but we need more check.
//		if (pData.flTimeSinceBreakingLBY > 11.1f && pData.flTimeSinceBodySwayLeft < 11.1f && pData.flTimeSinceBodySwayRight < 11.1f)
//			// increase.
//			++pData.flTimeSinceBodySwaying;
//		// reset data.
//		else pData.flTimeSinceBodySwaying = 0.f;
//
//		// think about it.
//		if (pData.flTimeSinceBodySwaying > 11.1f)
//			pData.iAntiAimType = Lagcompensation::SWAY;
//
//		// its been fake desync for enough time.
//		if (pData.flTimeSinceNoDesync > 5.f) {
//			flGuessedYaw *= 0.f;
//
//			// its fake desync for sure.
//			pData.iAntiAimType = Lagcompensation::FAKE;
//		}
//
//		// fire detection.
//		if (pRecord->bDidShot && fabsf(pEntity->AnimState()->flEyePitch) < 89.f) {
//			pData.iAntiAimType = Lagcompensation::ONSHOT;
//
//			// change known side.
//			flGuessedYaw *= -1.f;
//
//			// use the default yaw.
//			pEntity->AnimState()->flGoalFeetYaw = flOldGoalFeetYaw;
//
//			// last side.
//			flGuessedYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pEntity->AnimState()->flGoalFeetYaw);
//
//			// desync side info.
//			if (!flGuessedYaw)
//				pData.iDesyncSide = 0;
//			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
//			return;
//		}
//	}
//	else {
//		// its normal desync for sure.
//		pData.iAntiAimType = Lagcompensation::DESYNC;
//
//		// reset the data.
//		pData.flTimeSinceBreakingLBY = 0.f;
//		pData.flTimeSinceBodySwayRight = 0.f;
//		pData.flTimeSinceBodySwayLeft = 0.f;
//		pData.flTimeSinceBodySwaying = 0.f;
//		pData.flTimeSinceNoDesync = 0.f;
//
//		// from the server.
//		auto flFromServerPlaybackrate = pRecord->pLayers[6].flPlaybackRate;
//
//		// resolver calculations.
//		const float fCenterPlaybackrate = pRecord->LayerData[CENTER].flPlaybackRate;
//		const float fRightPlaybackrate = pRecord->LayerData[RIGHT].flPlaybackRate;
//		const float fLeftPlaybackrate = pRecord->LayerData[LEFT].flPlaybackRate;
//
//		// differences.
//		const float fDifferenceCenterPlaybackrate = fabs(flFromServerPlaybackrate - fCenterPlaybackrate);
//		const float fDifferenceRightPlaybackrate = fabs(flFromServerPlaybackrate - fRightPlaybackrate);
//		const float fDifferenceLeftPlaybackrate = fabs(flFromServerPlaybackrate - fLeftPlaybackrate);
//
//		// while.
//		if (GetVelocityLengthXY(pEntity) > 0.f && !pRecord->pLayers[12].flWeight && (pPrevious && pRecord->pLayers[6].flWeight == pPrevious->pLayers[6].flWeight || pRecord->vecVelocity.Length2D() > 135.f && pRecord->pLayers[6].flWeight == 1.f))
//		{
//			pData.iAntiAimType = Lagcompensation::ANIMATION;
//
//			if (fDifferenceCenterPlaybackrate <= fDifferenceRightPlaybackrate && fDifferenceCenterPlaybackrate <= fDifferenceLeftPlaybackrate)
//				// center.
//				flGuessedYaw = 60.f;
//			else if (fDifferenceRightPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceRightPlaybackrate <= fDifferenceLeftPlaybackrate)
//				// right.
//				flGuessedYaw = 60.f;
//			else if (fDifferenceLeftPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceLeftPlaybackrate <= fDifferenceRightPlaybackrate)
//				// left.
//				flGuessedYaw = -60.f;
//			else // center.
//				flGuessedYaw = 0.f; //flGuessedYaw = -58.f;
//
//			// save the correct moving data.
//			if (flOldPlaybackrateYaw[pEntity->EntIndex()] != flGuessedYaw)
//				flOldPlaybackrateYaw[pEntity->EntIndex()] = flGuessedYaw;
//		}
//		// set the last saved data.
//		else if (fabs(flOldPlaybackrateYaw[pEntity->EntIndex()]) > 0.f)
//			flGuessedYaw = flOldPlaybackrateYaw[pEntity->EntIndex()];
//		// set the last saved data.
//		else  if (fabs(flOldLowerbodyYaw[pEntity->EntIndex()]) > 0.f)
//			// thats how to resolve onetap.
//			flGuessedYaw = flOldLowerbodyYaw[pEntity->EntIndex()];
//		// we don't have data.
//		else flGuessedYaw = 58.f;
//
//		// fire detection.
//		if (pRecord->bDidShot && fabsf(pEntity->AnimState()->flEyePitch) < 89.f) {
//			pData.iAntiAimType = Lagcompensation::ONSHOT;
//
//			// use the default yaw.
//			pEntity->AnimState()->flGoalFeetYaw = flOldGoalFeetYaw;
//
//			// last side.
//			flGuessedYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pEntity->AnimState()->flGoalFeetYaw);
//
//			// desync side info.
//			if (!flGuessedYaw)
//				pData.iDesyncSide = 0;
//			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
//			return;
//		}
//
//		// in use detection but not the greateast.
//		if (pLocal->IsAlive() && pRecord->flDeltaAngle < 90.f && fabsf(pEntity->AnimState()->flEyePitch) < 45.f) {
//			pData.iAntiAimType = Lagcompensation::LEGIT;
//
//			// use the default yaw.
//			pEntity->AnimState()->flGoalFeetYaw = flOldGoalFeetYaw;
//
//			// last side.
//			flGuessedYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y - pEntity->AnimState()->flGoalFeetYaw);
//
//			// desync side info.
//			if (!flGuessedYaw)
//				pData.iDesyncSide = 0;
//			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
//			return;
//		}
//	}
//
//	// bruteforce.
//	switch (arrMissedShots[pEntity->EntIndex()] % 3)
//	{
//	case 1: flGuessedYaw *= -1.f; break;
//	case 2: flGuessedYaw *= 0.f; break;
//	}
//
//	// desync side info.
//	if (!flGuessedYaw)
//		pData.iDesyncSide = 0;
//	else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
//
//	// apply the resolver.
//	pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y + flGuessedYaw);
//}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pPrevious || !pRecord)
		return;

	if (!cfg::rage::resolver)
		return;

	int iEntIndex = pEntity->EntIndex();

	//auto& pData = lagcomp.GetLog(iEntIndex);
	//if (!&pData)
	//	return;

	static auto CalculateHighSpeedCorrection = [&](float speed) -> float {

		const float highSpeedThreshold = 0;
		const float maxCorrection = 0;

		float correction = 0.0f;
		if (speed > highSpeedThreshold)
		{
			float correctionFactor = (speed - highSpeedThreshold) / highSpeedThreshold;
			correction = maxCorrection * correctionFactor;
		}

		return correction;
	};


	static auto ApplyHighSpeedCorrection = [&](float flEyeYaw, float flFootYaw, float flSpeed) -> float {

		float flAngleDelta = M::AngleDiff(flEyeYaw, flFootYaw);
		float flMaxCorrection = CalculateHighSpeedCorrection(flSpeed);
		float flCorrection = std::clamp(flAngleDelta, -flMaxCorrection, flMaxCorrection);
		return M::NormalizeAngle(flFootYaw + flCorrection);
	};

	CAnimState* pState = pEntity->AnimState();

	float flEyeFootDelta = M::AngleDiff(pState->flEyeYaw, pState->flGoalFeetYaw);
	float flAimMatrixWidthRange = pRecord->flDesyncDelta;

	float flTempYawMax = pState->flMaxBodyYaw * flAimMatrixWidthRange;
	float flTempYawMin = pState->flMinBodyYaw * flAimMatrixWidthRange;

	if (flEyeFootDelta > flTempYawMax) {
		pState->flGoalFeetYaw = pState->flEyeYaw - fabs(flTempYawMax);
	}
	else if (flEyeFootDelta < flTempYawMin) {
		pState->flGoalFeetYaw = pState->flEyeYaw + fabs(flTempYawMin);
	}

	pState->flGoalFeetYaw = M::NormalizeAngle(pState->flGoalFeetYaw);

	if (pState->bOnGround)
	{
		if (pState->flSpeedNormalized > 0.1f)
		{
			pState->flGoalFeetYaw = M::ApproachAngle(pState->flEyeYaw, pState->flGoalFeetYaw, pState->flLastUpdateIncrement * (30.0f + 20.0f * pState->flWalkToRunTransition));

			// Perform other resolver actions when the player is moving on the ground
			if (pState->flSpeedNormalized > 250.0f)
			{
				// High-speed movement correction
				//pState->flGoalFeetYaw = ApplyHighSpeedCorrection(pState->flEyeYaw, pState->flGoalFeetYaw, pState->flSpeedNormalized);
			}
		}
		else
		{
			pState->flGoalFeetYaw = M::ApproachAngle(pState->pEntity->GetLowerBodyYaw(), pState->flGoalFeetYaw, pState->flLastUpdateIncrement * CSGO_ANIM_LOWER_CATCHUP_IDLE);

			// Perform other resolver actions when the player is idle on the ground
		}
	}
	else
	{
		if (pState->flSpeedNormalized > 250.0f)
		{
			// High-speed in-air movement correction
			//pState->flGoalFeetYaw = ApplyHighSpeedCorrection(pState->flEyeYaw, pState->flGoalFeetYaw, pState->flSpeedNormalized);
		}
	}
}