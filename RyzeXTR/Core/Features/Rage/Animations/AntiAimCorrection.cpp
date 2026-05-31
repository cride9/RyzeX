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

/* LMAO ye adding back again this one thanks exlo again */
float Animations::GetLocalCycleIncrement(CBaseEntity* pEntity, float flPlaybackrate)
{
	float flMoveCycleRate = flPlaybackrate ? flPlaybackrate : pEntity->GetAnimationOverlays()[6].flPlaybackRate;
	float flVelocityLengthXY = pEntity->AnimState()->flVelocityLenght2D <= 1.f ? 1.f : pEntity->AnimState()->flVelocityLenght2D;
	if (flVelocityLengthXY > 0.f)
	{
		float flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence);
		float flSequenceGroundSpeed = fmax(pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), pEntity->GetAnimationOverlays()[6].nSequence) / (1.0f / flSequenceCycleRate), 0.001f);

		float flSpeedMultiplier = flSequenceCycleRate * (flVelocityLengthXY / flSequenceGroundSpeed) * (1.0f - (pEntity->AnimState()->flWalkToRunTransition * 0.15f));
		flMoveCycleRate /= flSpeedMultiplier;
	}

	float flLocalCycleIncrement = (flMoveCycleRate * pEntity->AnimState()->flLastUpdateIncrement);
	return flLocalCycleIncrement * 1000000.0f;
}

void Animations::Resolver(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!cfg::rage::bResolver || !pLocal || !pEntity || !pEntity->IsAlive() || !pRecord || !pPrevious)
		return;

#ifdef NDEBUG
	if (pEntity->GetPlayerInfo().bFakePlayer)
		return;
#endif

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pRecord->iEntIndex);
	const int iEntityID = pEntity->EntIndex();

	/* ========== Fake pitch resolver ========== */
	static float flFakePitch[65];
	if (fabsf(pEntity->AnimState()->flEyePitch) == 180.f)
		flFakePitch[iEntityID] = pEntity->AnimState()->flEyePitch;
	else if (pRecord->bDidShot)
		flFakePitch[iEntityID] = NULL;

	if (fabsf(flFakePitch[iEntityID]) == 180.f)
		pEntity->GetEyeAngles() = Vector(89.f, pEntity->AnimState()->flEyeYaw, 0.f);

	/* ========== Freestand resolver (highest priority) ========== */
	if (auto side = FreestandResolver(pRecord); side != VISUAL) {
		SetYaw(pRecord, side);
		return;
	}

	/* ========== LBY delta based resolver ========== */
	// Track LBY update timings per player
	static float flLastLBYUpdateTime[65] = {};
	static float flLBYUpdateDelta[65] = {};
	static int iLBYUpdateCount[65] = {};

	const float flEyeYaw = pRecord->vecEyeAngles.y;
	const float flLBY = pRecord->flLowerBodyYawTarget;
	const float flLBYDelta = fabsf(M::NormalizeYaw(flEyeYaw - flLBY));

	// Detect LBY update: LBY changed from previous record
	if (pRecord->flLowerBodyYawTarget != pPrevious->flLowerBodyYawTarget) {
		flLBYUpdateDelta[iEntityID] = pRecord->flSimulationTime - flLastLBYUpdateTime[iEntityID];
		flLastLBYUpdateTime[iEntityID] = pRecord->flSimulationTime;
		iLBYUpdateCount[iEntityID]++;
	}

	// Extended desync detection: LBY updates at ~1.1s intervals while standing
	// Pattern: real = eyeAngles ± desyncDelta (sent on LBY break), fake = eyeAngles ∓ desyncDelta (choked)
	const bool bPotentialExtended = (iLBYUpdateCount[iEntityID] > 1)
		&& (flLBYUpdateDelta[iEntityID] > 1.0f && flLBYUpdateDelta[iEntityID] < 1.3f)
		&& (pRecord->iFlags & FL_ONGROUND);

	if (bPotentialExtended && flLBYDelta > pRecord->flDesyncDelta * 0.5f) {
		// On extended desync LBY break: the real angle is sent on the LBY update tick
		// The eye angles point toward the fake, LBY points toward the real
		// We need the side where flGoalFeetYaw would be closer to LBY
		const float flLBYToEyeDelta = M::NormalizeYaw(flEyeYaw - flLBY);
		if (flLBYToEyeDelta > 0.f)
			SetYaw(pRecord, LEFT);   // Eye is right of LBY → fake is right → real is left
		else
			SetYaw(pRecord, RIGHT);  // Eye is left of LBY → fake is left → real is right
		return;
	}

	/* ========== Static desync detection via LBY delta ========== */
	// When enemy uses static desync: eye angles are real, LBY = real ± body yaw offset
	// The desync delta between real and fake creates observable patterns in animation
	if (flLBYDelta > 35.f && pRecord->flDesyncDelta > 1.f) {
		// Eye angles are significantly offset from LBY → likely static desync
		// If eye is right of LBY → real is right, fake is left (desync left)
		// If eye is left of LBY → real is left, fake is right (desync right)
		// The real angle IS the eye angles (what we see)
		// But the enemy's client animation runs with fake = eyeAngles ± desyncDelta
		// The actual hitboxes will be at the real side, so we need to resolve to the real

		// Actually: eye angles are what the server uses. LBY is updated from real animstate.
		// When desyncing left (fake = eye - 120): LBY tends to trail to the left of eye
		// When desyncing right (fake = eye + 120): LBY tends to trail to the right of eye
		// But on LBY update, LBY = flGoalFeetYaw which is bounded around eye yaw
		// So LBY should be near eye yaw ± maxBodyYaw
		// The key: on LBY update tick, LBY snaps to flGoalFeetYaw which reflects server reality
		
		const float flLBYSign = M::NormalizeYaw(flLBY - flEyeYaw);
		if (flLBYSign > 0.f)
			SetYaw(pRecord, LEFT);   // LBY is right of eye → fake is right → real is left
		else
			SetYaw(pRecord, RIGHT);  // LBY is left of eye → fake is left → real is right
		return;
	}

	/* ========== Animation layer playback rate matching ========== */
	// First, use flGuessedYaw from FindDesyncSide if available
	if (fabsf(pRecord->flGuessedYaw) > 0.1f) {
		if (pRecord->flGuessedYaw > 5.f)
			SetYaw(pRecord, RIGHT);
		else if (pRecord->flGuessedYaw < -5.f)
			SetYaw(pRecord, LEFT);
		else
			SetYaw(pRecord, CENTER);
		return;
	}

	// Fallback: direct layer comparison (only valid during walk-to-run transition)
	if (pRecord->iFlags & FL_ONGROUND && pPrevious->iFlags & FL_ONGROUND
		&& pRecord->flWalkToRunTransition > 0.f && pRecord->flWalkToRunTransition < 1.f) {

		if (!(pRecord->arrLayers[ANIMATION_LAYER_LEAN].flWeight * 1000.f)
			&& ((pRecord->arrLayers[ANIMATION_LAYER_MOVEMENT_MOVE].flWeight * 1000.f) == (pPrevious->arrLayers[ANIMATION_LAYER_MOVEMENT_MOVE].flWeight * 1000.f))) {

			float flFromServerPlaybackrate = GetLocalCycleIncrement(pEntity, pRecord->arrLayers[ANIMATION_LAYER_MOVEMENT_MOVE].flPlaybackRate);

			const float fCenterPlaybackrate = GetLocalCycleIncrement(pEntity, pRecord->LayerData[CENTER].flPlaybackRate);
			const float fRightPlaybackrate = GetLocalCycleIncrement(pEntity, pRecord->LayerData[RIGHT].flPlaybackRate);
			const float fLeftPlaybackrate = GetLocalCycleIncrement(pEntity, pRecord->LayerData[LEFT].flPlaybackRate);

			const float fDifferenceCenterPlaybackrate = fabsf(flFromServerPlaybackrate - fCenterPlaybackrate);
			const float fDifferenceRightPlaybackrate = fabsf(flFromServerPlaybackrate - fRightPlaybackrate);
			const float fDifferenceLeftPlaybackrate = fabsf(flFromServerPlaybackrate - fLeftPlaybackrate);

			if (GetVelocityLengthXY(pEntity) > 0.f) {
				if (fDifferenceCenterPlaybackrate <= fDifferenceRightPlaybackrate && fDifferenceCenterPlaybackrate <= fDifferenceLeftPlaybackrate)
					SetYaw(pRecord, CENTER);
				else if (fDifferenceRightPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceRightPlaybackrate <= fDifferenceLeftPlaybackrate)
					SetYaw(pRecord, RIGHT);
				else if (fDifferenceLeftPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceLeftPlaybackrate <= fDifferenceRightPlaybackrate)
					SetYaw(pRecord, LEFT);
				return;
			}
		}
	}

	/* ========== Carry over previous resolve data ========== */
	if (pPrevious->iResolveSide != VISUAL) {
		SetYaw(pRecord, pPrevious->iResolveSide);
		return;
	}

	/* ========== Default fallback ========== */
	SetYaw(pRecord, RIGHT);

	/* ========== Miss brute-force logic ========== */
	// This overrides the resolved side when we've missed shots
	static std::array<int, 65> arrMissedShotsBackup{ 0 };

	if (arrMissedShots[iEntityID] != arrMissedShotsBackup[iEntityID]) {
		// Cycle through LEFT, RIGHT, CENTER based on miss count
		int iBruteMode = arrMissedShots[iEntityID] % 3;
		switch (iBruteMode) {
		case 0: SetYaw(pRecord, RIGHT);  break;
		case 1: SetYaw(pRecord, LEFT);   break;
		case 2: SetYaw(pRecord, CENTER); break;
		}
		arrMissedShotsBackup[iEntityID] = arrMissedShots[iEntityID];
	}
}

void Animations::ResolverLogic() {

	if (!aimbot.GetHitLogData().pAimbotTarget || !g::pLocal || bBulletImpact == Vector(0, 0, 0) || !aimbot.GetHitLogData().pTargetMatrix)
		return;

	if (aimbot.GetHitLogData().iTickcount + 64 < g::pCmd->iTickCount)
		return aimbot.GetHitLogData().ClearTarget();

	// make pointers and references for easier handling
	auto& refCurrentData = aimbot.GetHitLogData();
	CBaseEntity* pTarget = refCurrentData.pAimbotTarget;

	if (i::EntityList->GetClientEntity(refCurrentData.iEntityIndex) == nullptr) {
		refCurrentData.ClearTarget();
		return;
	}

	auto* pLog = &lagcomp.GetLog(refCurrentData.pAimbotTarget->EntIndex());
	if (!pLog) {
		refCurrentData.ClearTarget();
		return;
	}

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
	if ( bResolverHandler[ PLAYERHURT ] || bResolverHandler[ PLAYERDEATH ] ) {

		if ( data.enterTrace.iHitGroup != iHitHitbox )
			anims.arrMissedShots[ refCurrentData.pAimbotTarget->EntIndex( ) ]++;

		bResolverHandler = std::array<bool, HANDLERCOUNT>( );
		{
			std::string szHitgroupNameHitbox = misc::GetHitgroupName( iHitHitbox );
			int nBacktrack = ( refCurrentData.iTickcount - TIME_TO_TICKS( refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount( ) ) );
			std::string szHitgroupNameWanted = misc::GetHitgroupName( refCurrentData.iHitGroup );
			misc::Print( std::vformat(
				XorStr( "Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}" ), std::make_format_args(
					info.szName,
					szHitgroupNameHitbox,
					iHitDmg,
					refCurrentData.flHitchance,
					nBacktrack,
					szHitgroupNameWanted,
					refCurrentData.flDamage,
					refCurrentData.pRecord->flResolveDelta
				) ) );
		}
		pLog->iHitAmount++;
		visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, static_cast< int >( iHitDmg ), i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
		refCurrentData.ClearTarget();
		return;
	}

	// Check again just to be safe
	if (pTarget->GetHealth() < refCurrentData.iHealth || !pTarget->IsAlive()) {

		if (data.enterTrace.iHitGroup != iHitHitbox)
			anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++; 

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		{
			std::string szHitgroupNameHitbox = misc::GetHitgroupName(iHitHitbox);
			int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
			std::string szHitgroupNameWanted = misc::GetHitgroupName(refCurrentData.iHitGroup);
			misc::Print(std::vformat(
				XorStr("Hit {}'s {} for {} hp. | [hc] {} | [bt] {} | [wanted hg: {}] | [wanted dmg: {}] | [yaw] {}"), std::make_format_args(
				info.szName,
				szHitgroupNameHitbox,
				iHitDmg,
				refCurrentData.flHitchance,
				nBacktrack,
				szHitgroupNameWanted,
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
		}
		pLog->iHitAmount++;
		visual::vecDamageIndicator.push_back( { bBulletImpact, static_cast< int >( iHitDmg ), i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
		refCurrentData.ClearTarget();
		return;
	}

	// If we hit an entity but didn't deal any dmg its a resolver miss
	if (data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity == pTarget) {

		anims.arrMissedShots[refCurrentData.pAimbotTarget->EntIndex()]++;
		pLog->iLastResolve = refCurrentData.pRecord->iResolveSide;

		if (refCurrentData.bBacktrack) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			{
				std::string szHitgroupName = misc::GetHitgroupName(refCurrentData.iHitGroup);
				int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
				misc::Print(std::vformat(
					XorStr("Missed {}'s {} due to invalid record | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
					info.szName,
					szHitgroupName,
					refCurrentData.flHitchance,
					nBacktrack,
					refCurrentData.flDamage,
					refCurrentData.pRecord->flResolveDelta
				)));
			}
			visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, 0, i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		{
			std::string szHitgroupName = misc::GetHitgroupName(refCurrentData.iHitGroup);
			int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
			misc::Print(std::vformat(
				XorStr("Missed {}'s {} due to resolver. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
				info.szName,
				szHitgroupName,
				refCurrentData.flHitchance,
				nBacktrack,
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
		}
		visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, 0, i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
		refCurrentData.ClearTarget();
	}
	else {

		if (refCurrentData.flHitchance >= 99) {

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			{
				std::string szHitgroupName = misc::GetHitgroupName(refCurrentData.iHitGroup);
				int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
				misc::Print(std::vformat(
					XorStr("Missed {}'s {} due to correction. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
					info.szName,
					szHitgroupName,
					refCurrentData.flHitchance,
					nBacktrack,
					refCurrentData.flDamage,
					refCurrentData.pRecord->flResolveDelta
				)));
			}
			visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, 0, i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
			refCurrentData.ClearTarget();
			return;
		}

		// check for occlusionaw
		Trace_t traceData;
		Ray_t rayData(refCurrentData.vecLocalShootPosition, bBulletImpact);
		CTraceFilter filterData(g::pLocal, TRACE_ENTITIES_ONLY);
		i::EngineTrace->TraceRay(rayData, MASK_SHOT | CONTENTS_GRATE, &filterData, &traceData);

		bool bOccluded = (bBulletImpact - refCurrentData.vecLocalShootPosition).LengthSqr() < (refCurrentData.vecTargetShootPosition - refCurrentData.vecLocalShootPosition).LengthSqr();
		if (bOccluded) { // traceData.pHitEntity != nullptr && traceData.pHitEntity == refCurrentData.pAimbotTarget || data.flCurrentDamage == 0.f

			bResolverHandler = std::array<bool, HANDLERCOUNT>();
			{
				std::string szHitgroupName = misc::GetHitgroupName(refCurrentData.iHitGroup);
				int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
				misc::Print(std::vformat(
					XorStr("Missed {}'s {} due to occlusion. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
					info.szName,
					szHitgroupName,
					refCurrentData.flHitchance,
					nBacktrack,
					refCurrentData.flDamage,
					refCurrentData.pRecord->flResolveDelta
				)));
			}
			visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, 0, i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
			refCurrentData.ClearTarget();
			return;
		}

		bResolverHandler = std::array<bool, HANDLERCOUNT>();
		{
			std::string szHitgroupName = misc::GetHitgroupName(refCurrentData.iHitGroup);
			int nBacktrack = (refCurrentData.iTickcount - TIME_TO_TICKS(refCurrentData.flTargetSimulation + lagcomp.GetClientInterpAmount()));
			misc::Print(std::vformat(
				XorStr("Missed {}'s {} due to spread. | [hc] {} | [bt] {} | [dmg] {} | [yaw] {}"), std::make_format_args(
				info.szName,
				szHitgroupName,
				refCurrentData.flHitchance,
				nBacktrack,
				refCurrentData.flDamage,
				refCurrentData.pRecord->flResolveDelta
			)));
		}
		visual::vecDamageIndicator.push_back( { refCurrentData.vecTargetShootPosition, 0, i::GlobalVars->iTickCount, iHitHitbox == HITGROUP_HEAD } );
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

		for (size_t i = 0; i < 65; i++) {
			lagcomp.GetLog(i).ClearData();
			arrMissedShots[i] = 0;
		}
	}
}

void SetResolveMatrix(Lagcompensation::LagRecord_t* pRecord, int iType) {

	//memcpy(pRecord->pMatricies[RESOLVE], pRecord->pMatricies[iType], sizeof(matrix3x4_t) * MAXSTUDIOBONES);
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
		pRecord->flResolveDelta = M::NormalizeYaw(pRecord->vecEyeAngles.y - pState->flGoalFeetYaw);
		break;

	case CENTER:
		pState->flGoalFeetYaw = GetYawRotation( pRecord, CENTER );
		pRecord->flResolveDelta = M::NormalizeYaw(pRecord->vecEyeAngles.y - pState->flGoalFeetYaw);
		break;

	case RIGHT:
		pState->flGoalFeetYaw = GetYawRotation( pRecord, RIGHT );
		pRecord->flResolveDelta = M::NormalizeYaw(pRecord->vecEyeAngles.y - pState->flGoalFeetYaw);
		break;
	}
}

EMatrixType Animations::FreestandResolver( Lagcompensation::LagRecord_t* pRecord ) {

	if ( !pRecord )
		return VISUAL;

	auto GRD_TO_BOG = [ & ]( float GRD ) -> float {
		return ( M_PI / 180 ) * GRD;
	};

	static EMatrixType FinalAngle = VISUAL;
	bool bSide1 = false;
	bool bSide2 = false;
	bool autowalld = false;
	CBaseEntity* pPlayerEntity = pRecord->pEntity;

	float flAngToLocal = M::CalcAngle( pPlayerEntity->GetVecOrigin( ), g::pLocal->GetVecOrigin( ) ).y;
	Vector vecViewPoint = g::pLocal->GetVecOrigin( ) + Vector( 0, 0, 90 );

	Vector2D vecSide1 = { ( 45 * sin( GRD_TO_BOG( flAngToLocal ) ) ),( 45 * cos( GRD_TO_BOG( flAngToLocal ) ) ) };
	Vector2D vecSide2 = { ( 45 * sin( GRD_TO_BOG( flAngToLocal + 180 ) ) ) ,( 45 * cos( GRD_TO_BOG( flAngToLocal + 180 ) ) ) };

	Vector2D vecSide3 = { ( 50 * sin( GRD_TO_BOG( flAngToLocal ) ) ),( 50 * cos( GRD_TO_BOG( flAngToLocal ) ) ) };
	Vector2D vecSide4 = { ( 50 * sin( GRD_TO_BOG( flAngToLocal + 180 ) ) ) ,( 50 * cos( GRD_TO_BOG( flAngToLocal + 180 ) ) ) };

	Vector vecOrigin = pPlayerEntity->GetVecOrigin( );

	Vector2D vecOriginLeftRight[ ] = { Vector2D( vecSide1.x, vecSide1.y ), Vector2D( vecSide2.x, vecSide2.y ) };

	Vector2D vecOriginLeftRightLocal[ ] = { Vector2D( vecSide3.x, vecSide3.y ), Vector2D( vecSide4.x, vecSide4.y ) };

	for ( int iSide = 0; iSide < 2; iSide++ ) {

		Vector vecOriginAutowall = { vecOrigin.x + vecOriginLeftRight[ iSide ].x,  vecOrigin.y - vecOriginLeftRight[ iSide ].y , vecOrigin.z + 80 };
		Vector vecOriginAutowall2 = { vecViewPoint.x + vecOriginLeftRightLocal[ iSide ].x,  vecViewPoint.y - vecOriginLeftRightLocal[ iSide ].y , vecViewPoint.z };

		if ( autowall.CanHitFloatingPoint( vecOriginAutowall, vecViewPoint, pRecord ) ) {

			if ( iSide == 1 ) {

				bSide1 = true;
				FinalAngle = RIGHT;
			}
			else if ( iSide == 0 ) {

				bSide2 = true;
				FinalAngle = LEFT;
			}
			autowalld = true;
		}
		else {
			for ( int iSideID = 0; iSideID < 2; iSideID++ ) {

				Vector vecOriginAutowall3 = { vecOrigin.x + vecOriginLeftRight[ iSideID ].x,  vecOrigin.y - vecOriginLeftRight[ iSideID ].y , vecOrigin.z + 80 };

				if ( autowall.CanHitFloatingPoint( vecOriginAutowall3, vecOriginAutowall2, pRecord ) ) {

					if ( iSideID == 1 ) {

						bSide1 = true;
						FinalAngle = RIGHT;
					}
					else if ( iSideID == 0 ) {

						bSide2 = true;
						FinalAngle = LEFT;
					}
					autowalld = true;
				}
			}
		}
	}

	if ( !autowalld || ( bSide1 && bSide2 ) )
		return VISUAL;
	else
		return FinalAngle;

	return VISUAL;
}