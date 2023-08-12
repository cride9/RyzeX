#include "EnemyAnimations.h"
#include "../aimbot.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../xorstr.h"
#include "../../Visuals/ESP.h"
#include "../../Visuals/chams.h"

void Animations::SetupLayerHardCode() {

	GoingLeftLeft.emplace(21.484373, 5.8720565e-05);
	GoingLeftLeft.emplace(36.468643, 0.0019380145);
	GoingLeftLeft.emplace(51.452965, 0.0019380145);
	GoingLeftLeft.emplace(66.43728, 0.003496006);
	GoingLeftLeft.emplace(81.42159, 0.003496006);
	GoingLeftLeft.emplace(96.290405, 0.0049562533);
	GoingLeftLeft.emplace(109.951126, 0.0049562533);
	GoingLeftLeft.emplace(122.50191, 0.006014956);
	GoingLeftLeft.emplace(134.03294, 0.006014956);
	GoingLeftLeft.emplace(144.62706, 0.0071707903);
	GoingLeftLeft.emplace(154.36041, 0.0071707903);
	GoingLeftLeft.emplace(163.30295, 0.008486761);
	GoingLeftLeft.emplace(171.51888, 0.008486761);
	GoingLeftLeft.emplace(179.06726, 0.009724718);
	GoingLeftLeft.emplace(186.00238, 0.009724718);
	GoingLeftLeft.emplace(192.37398, 0.010827008);
	GoingLeftLeft.emplace(198.2279, 0.010827008);
	GoingLeftLeft.emplace(203.60619, 0.011766492);
	GoingLeftLeft.emplace(208.54749, 0.011766492);
	GoingLeftLeft.emplace(213.08733, 0.012518161);
	GoingLeftLeft.emplace(217.25829, 0.012518161);
	GoingLeftLeft.emplace(221.09036, 0.013064177);
	GoingLeftLeft.emplace(224.61107, 0.013064177);
	GoingLeftLeft.emplace(227.84573, 0.0133962985);
	GoingLeftLeft.emplace(230.81757, 0.0133962985);
	GoingLeftLeft.emplace(233.54794, 0.013517402);
	GoingLeftLeft.emplace(236.05649, 0.013517402);
	GoingLeftLeft.emplace(238.36119, 0.013441293);
	GoingLeftLeft.emplace(240.47865, 0.013441293);
	GoingLeftLeft.emplace(242.42407, 0.0131908795);
	GoingLeftLeft.emplace(244.21141, 0.0131908795);
	GoingLeftLeft.emplace(245.85353, 0.012795235);
	GoingLeftLeft.emplace(247.36224, 0.012795235);
	GoingLeftLeft.emplace(248.74837, 0.012286282);
	GoingLeftLeft.emplace(249.99944, 0.012286282);

	GoingLeftRight.emplace(21.484375, 6.4052074e-05);
	GoingLeftRight.emplace(36.468643, 0.001974628);
	GoingLeftRight.emplace(51.452972, 0.001974628);
	GoingLeftRight.emplace(66.43727, 0.00377401);
	GoingLeftRight.emplace(81.421585, 0.00377401);
	GoingLeftRight.emplace(96.29039, 0.0055466597);
	GoingLeftRight.emplace(109.95112, 0.0055466597);
	GoingLeftRight.emplace(122.50189, 0.006841827);
	GoingLeftRight.emplace(134.03293, 0.006841827);
	GoingLeftRight.emplace(144.62706, 0.008090441);
	GoingLeftRight.emplace(154.36041, 0.008090441);
	GoingLeftRight.emplace(163.30293, 0.009461771);
	GoingLeftRight.emplace(171.5189, 0.009461771);
	GoingLeftRight.emplace(179.06729, 0.010697144);
	GoingLeftRight.emplace(186.0024, 0.010697144);
	GoingLeftRight.emplace(192.37401, 0.01175186);
	GoingLeftRight.emplace(198.22795, 0.01175186);
	GoingLeftRight.emplace(203.60622, 0.012606867);
	GoingLeftRight.emplace(208.54755, 0.012606867);
	GoingLeftRight.emplace(213.08736, 0.013248912);
	GoingLeftRight.emplace(217.25833, 0.013248912);
	GoingLeftRight.emplace(221.0904, 0.013672521);
	GoingLeftRight.emplace(224.6111, 0.013672521);
	GoingLeftRight.emplace(227.84575, 0.01388125);
	GoingLeftRight.emplace(230.8176, 0.01388125);
	GoingLeftRight.emplace(233.54796, 0.013887708);
	GoingLeftRight.emplace(236.05652, 0.013887708);
	GoingLeftRight.emplace(238.36122, 0.013712412);
	GoingLeftRight.emplace(240.4787, 0.013712412);
	GoingLeftRight.emplace(242.4241, 0.01338164);
	GoingLeftRight.emplace(244.21146, 0.01338164);
	GoingLeftRight.emplace(245.85358, 0.012924901);
	GoingLeftRight.emplace(247.3623, 0.012924901);
	GoingLeftRight.emplace(248.74843, 0.012372382);

	GoingRightLeft.emplace(21.484375, 7.100059e-05);
	GoingRightLeft.emplace(36.468643, 0.002347457);
	GoingRightLeft.emplace(51.452972, 0.002347457);
	GoingRightLeft.emplace(66.43727, 0.0042464053);
	GoingRightLeft.emplace(81.421585, 0.0042464053);
	GoingRightLeft.emplace(96.29038, 0.006042329);
	GoingRightLeft.emplace(109.95111, 0.006042329);
	GoingRightLeft.emplace(122.50189, 0.007357179);
	GoingRightLeft.emplace(134.03293, 0.007357179);
	GoingRightLeft.emplace(144.62706, 0.008701701);
	GoingRightLeft.emplace(154.36041, 0.008701701);
	GoingRightLeft.emplace(163.30295, 0.010105081);
	GoingRightLeft.emplace(171.51888, 0.010105081);
	GoingRightLeft.emplace(179.06728, 0.011340644);
	GoingRightLeft.emplace(186.00238, 0.011340644);
	GoingRightLeft.emplace(192.374, 0.01236508);
	GoingRightLeft.emplace(198.2279, 0.01236508);
	GoingRightLeft.emplace(203.6062, 0.01316494);
	GoingRightLeft.emplace(208.5475, 0.01316494);
	GoingRightLeft.emplace(213.08733, 0.013734189);
	GoingRightLeft.emplace(217.25829, 0.013734189);
	GoingRightLeft.emplace(221.09036, 0.014075319);
	GoingRightLeft.emplace(224.61108, 0.014075319);
	GoingRightLeft.emplace(227.84573, 0.014199744);
	GoingRightLeft.emplace(230.81758, 0.014199744);
	GoingRightLeft.emplace(233.54797, 0.014126722);
	GoingRightLeft.emplace(236.0565, 0.014126722);
	GoingRightLeft.emplace(238.36122, 0.013881674);
	GoingRightLeft.emplace(240.47867, 0.013881674);
	GoingRightLeft.emplace(242.4241, 0.013493791);
	GoingRightLeft.emplace(244.21144, 0.013493791);
	GoingRightLeft.emplace(245.85358, 0.012993587);
	GoingRightLeft.emplace(247.36227, 0.012993587);
	GoingRightLeft.emplace(248.74841, 0.0124107115);
	GoingRightLeft.emplace(249.99942, 0.0124107115);

	GoingRightRight.emplace(21.484375, 6.6431574e-05);
	GoingRightRight.emplace(36.468643, 0.0023530596);
	GoingRightRight.emplace(51.45297, 0.0023530596);
	GoingRightRight.emplace(66.43726, 0.0042291405);
	GoingRightRight.emplace(81.42158, 0.0042291405);
	GoingRightRight.emplace(96.290375, 0.0056357468);
	GoingRightRight.emplace(109.9511, 0.0056357468);
	GoingRightRight.emplace(122.50189, 0.006834331);
	GoingRightRight.emplace(134.03293, 0.006834331);
	GoingRightRight.emplace(144.62704, 0.008142785);
	GoingRightRight.emplace(154.36041, 0.008142785);
	GoingRightRight.emplace(163.30295, 0.009510421);
	GoingRightRight.emplace(171.51888, 0.009510421);
	GoingRightRight.emplace(179.06726, 0.0107418755);
	GoingRightRight.emplace(186.00237, 0.0107418755);
	GoingRightRight.emplace(192.37398, 0.011787191);
	GoingRightRight.emplace(198.22789, 0.011787191);
	GoingRightRight.emplace(203.60619, 0.012627681);
	GoingRightRight.emplace(208.5475, 0.012627681);
	GoingRightRight.emplace(213.08731, 0.013251223);
	GoingRightRight.emplace(217.25829, 0.013251223);
	GoingRightRight.emplace(221.09035, 0.013654052);
	GoingRightRight.emplace(224.61107, 0.013654052);
	GoingRightRight.emplace(227.84573, 0.013841746);
	GoingRightRight.emplace(230.81757, 0.013841746);
	GoingRightRight.emplace(233.54796, 0.013828932);
	GoingRightRight.emplace(236.0565, 0.013828932);
	GoingRightRight.emplace(238.3612, 0.013637904);
	GoingRightRight.emplace(240.47867, 0.013637904);
	GoingRightRight.emplace(242.42409, 0.013296114);
	GoingRightRight.emplace(244.21143, 0.013296114);
	GoingRightRight.emplace(245.85356, 0.012833641);
	GoingRightRight.emplace(247.36226, 0.012833641);
	GoingRightRight.emplace(248.7484, 0.012280634);
	GoingRightRight.emplace(249.99939, 0.012280634);
}

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
		visual::vecDamageIndicator.push_back(std::make_pair(refCurrentData.vecTargetShootPosition, iHitDmg));
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
	if (!cfg::rage::bResolver || !pLocal || !pEntity || !pEntity->IsAlive() || !pRecord || !pPrevious)
		return;

#ifdef NDEBUG
	if (pEntity->GetPlayerInfo().bFakePlayer)
		return;
#endif

	const int iEntityID = pEntity->EntIndex();
	return SetYaw(pRecord, LEFT + arrMissedShots[iEntityID] % 3);
}