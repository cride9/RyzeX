#include "EnemyAnimations.h"
#include "../aimbot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../xorstr.h"
#include "../../Visuals/ESP.h"
#include "../../Visuals/chams.h"
#include <format>
#include "../../../Lua/Lua.h"


void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!cfg::rage::bResolver || !pLocal || !pEntity || !pEntity->IsAlive() || !pRecord || !pPrevious)
		return;

#ifdef NDEBUG
	if (pEntity->GetPlayerInfo().bFakePlayer)
		return;
#endif
	const int iEntityID = pEntity->EntIndex();

	int iCurrentBrute = arrMissedShots[iEntityID] % 2;

	/* Up pitch is most likely overlapped with eachother */
	if (pRecord->vecEyeAngles.x < -30.f)
		return;

	float flVelocityDelta = fabsf(pRecord->flWalkToRunTransition - pPrevious->flWalkToRunTransition);
	/* Constant speed check lmao */
	if (flVelocityDelta < 0.1f && pRecord->flWalkToRunTransition > 0.f && pRecord->flWalkToRunTransition < 1.f) {

		/* Get playbackrate delta */
		float flPlaybackrateDelta = (pRecord->arrLayers[6].flPlaybackRate - pPrevious->arrLayers[6].flPlaybackRate) * 10000.f;

		/* At constant speed, if the playbackrate is increased suddenly by that many that means he's inverted */
		/* Based on previous logs & playbackrate caches LEFT side invert has a larger number */
		if (flPlaybackrateDelta > 3.f)
			SetYaw(pRecord, LEFT + iCurrentBrute);

		else if (flPlaybackrateDelta < 3.f)
			SetYaw(pRecord, RIGHT - iCurrentBrute);
	}

	/* Playbackrate increases with flWalkToRunTransition */
	else if (pRecord->flWalkToRunTransition > pPrevious->flWalkToRunTransition && pRecord->arrLayers[6].flPlaybackRate < pPrevious->arrLayers[6].flPlaybackRate)
		SetYaw(pRecord, RIGHT - iCurrentBrute);

	/* Don't resolve onshot */
	else if (pRecord->bDidShot)
		return;

	/* breaking to the left */
	else if (pRecord->flPoses[BODY_YAW] > 0.85f && pRecord->vecVelocity.Length2D() < 1.f)
		SetYaw(pRecord, RIGHT);

	/* breaking to the right */
	else if (pRecord->flPoses[BODY_YAW] < 0.15f && pRecord->vecVelocity.Length2D() < 1.f)
		SetYaw(pRecord, LEFT);

	/* Apply previous data if no new data */
	else if (pPrevious->iResolveSide != VISUAL)
		SetYaw(pRecord, pPrevious->iResolveSide == RIGHT ? RIGHT - iCurrentBrute : LEFT + iCurrentBrute);

	/* If we didn't get any data apply right side & no previous data */
	else if (pRecord->iResolveSide == VISUAL)
		SetYaw(pRecord, RIGHT - iCurrentBrute);
}

void Animations::ResolverLogic() {

	if (!aimbot.GetHitLogData().pAimbotTarget || !g::pLocal || bBulletImpact == Vector(0, 0, 0) || !aimbot.GetHitLogData().pTargetMatrix)
		return;

	if (aimbot.GetHitLogData().iTickcount + 64 < g::pCmd->iTickCount)
		return aimbot.GetHitLogData().ClearTarget();

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
	refCurrentData.iServerHitbox = iHitHitbox;
	refCurrentData.iBacktrackTicks = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));

	LuaImplementation::RunCallbacks(LuaImplementation::vecCallbackList[LuaImplementation::CALLBACK_ON_RAGEBOT_SHOT], refCurrentData );

	// Simulate a bullet shot
	FireBulletData_t data;
	autowall.GetDamage(g::pLocal, refCurrentData.vecLocalShootPosition, bBulletImpact, g::pLocal->GetWeapon(), refCurrentData.pRecord , &data);

	// Check if we killed, or hurt the player
	if (bResolverHandler[PLAYERHURT] || bResolverHandler[PLAYERDEATH]) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::vformat(
			XorStr("Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}" ), std::make_format_args(
			info.szName,
			misc::GetHitgroupName(iHitHitbox),
			iHitDmg,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		) ) );
		visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, iHitDmg));
		refCurrentData.ClearTarget();
		return;
	}

	// Check again just to be safe
	if (pTarget->GetHealth() < refCurrentData.iHealth || !pTarget->IsAlive()) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++; 

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::vformat(
			XorStr("Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}"), std::make_format_args(
			info.szName,
			misc::GetHitgroupName(iHitHitbox),
			iHitDmg,
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		)));
		visual::vecDamageIndicator.push_back(std::make_pair(bBulletImpact, iHitDmg));
		refCurrentData.ClearTarget();
		return;
	}

	// If we hit an entity but didn't deal any dmg its a resolver miss
	if (data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity == pTarget) {

		anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
		pLog->iLastResolve = refCurrentData.pRecord->iResolveSide;

		if (refCurrentData.bBacktrack) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::vformat(
				XorStr("Missed {}'s {} due to invalid record or resolver on backtrack. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args( 
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
			visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, 0));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::vformat(
			XorStr("Missed {}'s {} due to resolver. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args( 
			info.szName,
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		)));
		visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, 0));
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			misc::Print(std::vformat(
				XorStr("Missed {}'s {} due to correction. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
			visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, 0));
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
			misc::Print(std::vformat(
				XorStr("Missed {}'s {} due to occlusion. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
				info.szName,
				misc::GetHitgroupName(refCurrentData.iHitGroup),
				refCurrentData.flHitchance,
				(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
			visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, 0));
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		misc::Print(std::vformat(
			XorStr("Missed {}'s {} due to spread. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
			info.szName,
			misc::GetHitgroupName(refCurrentData.iHitGroup),
			refCurrentData.flHitchance,
			(refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount())),
			refCurrentData.flDamage,
			refCurrentData.pRecord->flResolveDelta
		)));
		visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, 0));
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
