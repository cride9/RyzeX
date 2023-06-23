#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"

void Animations::ResolverLogic() {

	if (!ragebot.rageBotData.pAimbotTarget || !g::pLocal || bulletImpact == Vector(0, 0, 0) || !ragebot.rageBotData.pTargetMatrix)
		return;

	// make pointers and references for easier handling
	auto& refCurrentData = ragebot.rageBotData;
	CBaseEntity* pTarget = refCurrentData.pAimbotTarget;

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
	refCurrentData.pRecord->Apply(pTarget, false);

	// Simulate a bullet shot
	FireBulletData_t data;
	autowall.GetDamage(g::pLocal, refCurrentData.vecLocalShootPosition, bulletImpact, g::pLocal->GetWeapon(), &data);

	// If we hit an entity but didn't deal any dmg its a resolver miss
	if (data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity == pTarget) {

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

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::format(
				"Missed {} | [hc] {} | [bt] {} | [hg] {} | [dmg] {} | missed due to: unknown reason",
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

	auto& playerListData = playerList::arrPlayers[pEntity->EntIndex()];
	if (playerListData.bOverrideResolver) {

		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw + playerListData.flOverrideYaw);
		return;
	}
	if (!cfg::rage::resolver)
		return;

	Vector vecEyePosition = g::vecEyePosition;
	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();
	Vector vecLHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[LEFT]);
	Vector vecRHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[RIGHT]);
	Vector vecCHitboxPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pMatricies[CENTER]);

	CTraceFilter traceFilter(pLocal);
	Trace_t traceLData;
	Trace_t traceRData;

	float flResolveYaw = 58.f;
	static int iFoundSide = 0;

	switch (missedShots[pEntity->EntIndex()] % 3) {

	case 1: flResolveYaw *= 0;
		break;
	case 2: flResolveYaw *= -1;
		break;
	}

	float& vecEyeYaw = pEntity->AnimState()->flEyeYaw;
	if (pWeapon) {
		if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecLHitboxPosition, HITBOX_HEAD) == 3) {
			pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(vecEyeYaw - flResolveYaw);
			pRecord->bSafeResolve = true;
			return;
		}
		else if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecRHitboxPosition, HITBOX_HEAD) == 3) {
			pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(vecEyeYaw + flResolveYaw);
			pRecord->bSafeResolve = true;
			return;
		}
		else if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecCHitboxPosition, HITBOX_HEAD) == 3) {
			pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(vecEyeYaw);
			pRecord->bSafeResolve = true;
			return;
		}
	}

	pRecord->ApplyMatrix(pEntity, LEFT);
	float bHitLeft = autowall.GetDamage(pLocal, vecEyePosition, vecLHitboxPosition, pLocal->GetWeapon());
	pRecord->ApplyMatrix(pEntity, RIGHT);
	float bHitRight = autowall.GetDamage(pLocal, vecEyePosition, vecRHitboxPosition, pLocal->GetWeapon());

	if (bHitLeft > 1 && bHitRight > 1) {
		if (!iFoundSide)
			iFoundSide = RIGHT;
	}
	else if (bHitLeft <= 0.f && bHitRight <= 0)
		iFoundSide = 0;
	else if (bHitLeft > bHitRight && !iFoundSide)
		iFoundSide = RIGHT;
	else if (bHitLeft < bHitRight && !iFoundSide)
		iFoundSide = LEFT;

	if (iFoundSide != 0)
		flResolveYaw = iFoundSide == RIGHT ? 58 : -58;

	pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(vecEyeYaw + flResolveYaw);
}