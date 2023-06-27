#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"

float Animations::GetLocalCycleIncrement(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, float flPlaybackrate, Lagcompensation::LagRecord_t::LayerData_t* pLayer)
{
	float flMoveCycleRate = flPlaybackrate;
	float flVelocityLengthXY = pRecord->vecVelocity.Length2D() <= 1.f ? 1.f : pRecord->vecVelocity.Length2D();
	if (flVelocityLengthXY > 0.f)
	{
		float flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), pLayer->nSequence);
		float flSequenceGroundSpeed = fmax(pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), pLayer->nSequence) / (1.0f / flSequenceCycleRate), 0.001f);

		float flSpeedMultiplier = flSequenceCycleRate * (flVelocityLengthXY / flSequenceGroundSpeed) * (1.0f - (pEntity->AnimState()->flWalkToRunTransition * 0.15f));
		flMoveCycleRate /= flSpeedMultiplier;
	}
	float flLocalCycleIncrement = flMoveCycleRate;
	if (pEntity->AnimState()->flLastUpdateIncrement != 0.f)
		flLocalCycleIncrement = (flMoveCycleRate * pEntity->AnimState()->flLastUpdateIncrement);
	return flLocalCycleIncrement * 1000000.0f;
}

float Animations::GetLocalCycleIncrement(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, float flPlaybackrate)
{
	float flMoveCycleRate = flPlaybackrate ? flPlaybackrate : pEntity->GetAnimationOverlays()[6].flPlaybackRate;
	float flVelocityLengthXY = pRecord->vecVelocity.Length2D() <= 1.f ? 1.f : pRecord->vecVelocity.Length2D();
	if (flVelocityLengthXY > 0.f)
	{
		float flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence);
		float flSequenceGroundSpeed = fmax(pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence) / (1.0f / flSequenceCycleRate), 0.001f);

		float flSpeedMultiplier = flSequenceCycleRate * (flVelocityLengthXY / flSequenceGroundSpeed) * (1.0f - (pEntity->AnimState()->flWalkToRunTransition * 0.15f));
		flMoveCycleRate /= flSpeedMultiplier;
	}

	float flLocalCycleIncrement = flMoveCycleRate;
	if (pEntity->AnimState()->flLastUpdateIncrement != 0.f)
		flLocalCycleIncrement = (flMoveCycleRate * pEntity->AnimState()->flLastUpdateIncrement);
	return flLocalCycleIncrement * 1000000.0f;
}

void Animations::ResolverLogic() {

	if (!ragebot.rageBotData.pAimbotTarget || !g::pLocal || bulletImpact == Vector(0, 0, 0) || !ragebot.rageBotData.pTargetMatrix)
		return;

	// make pointers and references for easier handling
	auto& refCurrentData = ragebot.rageBotData;
	CBaseEntity* pTarget = refCurrentData.pAimbotTarget;
	auto* pLog = &lagcomp.GetLog(refCurrentData.pAimbotTarget->EntIndex());
	if (!pLog)
		return;

	// get player name and info
	PlayerInfo_t info;
	i::EngineClient->GetPlayerInfo(pTarget->EntIndex(), &info);

	if (refCurrentData.iTickcount + TIME_TO_TICKS(0.1f) < g::pCmd->iTickCount)
		return;

	// check if we did shot & had an impact
	if (!bResolverHandler[WEAPONFIRE] || !bResolverHandler[BULLETIMPACT])
		return;

	// Check if we killed, or hurt the player
	if (bResolverHandler[PLAYERHURT] || bResolverHandler[PLAYERDEATH]) {

		pLog->bPeekingReal = pLog->iFreestandMatrix == pLog->iLastResolve;
		pLog->bInitialized = true;

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
	if (pTarget->GetHealth() < ragebot.rageBotData.iHealth || !pTarget->IsAlive()) {

		pLog->bPeekingReal = pLog->iFreestandMatrix == pLog->iLastResolve;
		pLog->bInitialized = true;

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

		pLog->bPeekingReal = !(pLog->iFreestandMatrix == pLog->iLastResolve);
		pLog->bInitialized = true;

		if (refCurrentData.bBacktrack) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			anims.missedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
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
		anims.missedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
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

			pLog->bPeekingReal = !(pLog->iFreestandMatrix == pLog->iLastResolve);
			pLog->bInitialized = true;

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

	if (!ragebot.rageBotData.pAimbotTarget || !g::pLocal)
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
		auto targetIndex = ragebot.rageBotData.pAimbotTarget->EntIndex();

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
		auto targetIndex = ragebot.rageBotData.pAimbotTarget->EntIndex();

		if (iAttacker == i::EngineClient->GetLocalPlayer() && iUser == targetIndex) {
			bResolverHandler[PLAYERDEATH] = true;
		}
	}
}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pEntity || !pEntity->IsAlive() || !pPrevious)
		return;

	if (!cfg::rage::resolver)
		return;

	auto pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pLog)
		return;

	Vector vecEyePosition = g::vecEyePosition;
	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();
	const int iEntIndex = pEntity->EntIndex();
	float& vecEyeYaw = pRecord->vecEyeAngles.y;
	Vector vecLHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[LEFT]);
	Vector vecRHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[RIGHT]);
	Vector vecCHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[CENTER]);

	if (pLog->bInitialized) {

		float flRightDamage = autowall.GetDamage(pLocal, vecEyePosition, vecRHitboxPosition, pWeapon);
		float flLeftDamage = autowall.GetDamage(pLocal, vecEyePosition, vecLHitboxPosition, pWeapon);

		if (abs(flRightDamage - flLeftDamage) > 30.f) {

			if (pLog->bPeekingReal) {

				if (flRightDamage > flLeftDamage)
					memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[RIGHT], sizeof(matrix3x4_t) * 128);
				else if (flLeftDamage > flRightDamage)
					memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[LEFT], sizeof(matrix3x4_t) * 128);
			}
			else {

				if (flRightDamage < flLeftDamage)
					memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[RIGHT], sizeof(matrix3x4_t) * 128);
				else if (flLeftDamage < flRightDamage)
					memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[LEFT], sizeof(matrix3x4_t) * 128);
			}
		}

		return;
	}

	if (missedShots[iEntIndex] == 0) {

		if (pWeapon) {
			if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecLHitboxPosition, HITBOX_HEAD) == 3) {
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[LEFT], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = LEFT;
			}
			else if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecRHitboxPosition, HITBOX_HEAD) == 3) {
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[RIGHT], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = RIGHT;
			}
			else if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecCHitboxPosition, HITBOX_HEAD) == 3) {
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[CENTER], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = CENTER;
			}
		}
	}
	static std::array<int, 65> arrBackupMisses{0};
	if (arrBackupMisses[iEntIndex] != missedShots[iEntIndex]) {

		switch (pLog->iLastResolve) {
			case LEFT:
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[RIGHT], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = RIGHT;
				break;
			case RIGHT:
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[LEFT], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = LEFT;
				break;
			case CENTER:
				memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[RIGHT], sizeof(matrix3x4_t) * 128);
				pLog->iLastResolve = RIGHT;
				break;
		}
		arrBackupMisses[iEntIndex] = missedShots[iEntIndex];
	}

	// check if they peeked with real or fake
	if (!pLog->bInitialized) {

		float flRightDamage = autowall.GetDamage(pLocal, vecEyePosition, vecRHitboxPosition, pWeapon);
		float flLeftDamage = autowall.GetDamage(pLocal, vecEyePosition, vecLHitboxPosition, pWeapon);

		if (abs(flRightDamage - flLeftDamage) > 30.f) {

			if (flRightDamage > flLeftDamage)
				pLog->iFreestandMatrix = RIGHT;
			else if (flLeftDamage > flRightDamage)
				pLog->iFreestandMatrix = LEFT;
		}
	}
}