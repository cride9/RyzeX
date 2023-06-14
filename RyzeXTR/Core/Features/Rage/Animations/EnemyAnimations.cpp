#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../../../SDK/Menu/config.h"
#include "../autowall.h"
#include "../../Misc/Playerlist.h"

float flOldLowerbodyYaw[65];
float flOldPlaybackrateYaw[65];

bool Animations::NewDataRecievedFromServer(CBaseEntity* pPlayer)
{
	return pPlayer->GetSimulationTime() != pPlayer->GetOldSimulationTime();
}

void Animations::ResolverLogic() {

	if (!ragebot.rageBotData.pAimbotTarget || !g::pLocal || bulletImpact == Vector(0, 0, 0) || !ragebot.rageBotData.pTargetMatrix)
		return;

	Ray_t ray(g::pLocal->GetEyePosition(), bulletImpact);
	Trace_t trace;

	const Model_t* model = ragebot.rageBotData.pAimbotTarget->GetModel();
	if (!model)
		return;

	studiohdr_t* studioHdr = i::ModelInfo->GetStudioModel(model);
	if (!studioHdr)
		return;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(ragebot.rageBotData.iTargetHitbox);
	if (!studioBox)
		return;

	bool bCollided = ragebot.bCollidePoint(g::pLocal->GetEyePosition(), bulletImpact, studioBox, ragebot.rageBotData.pTargetMatrix);
	ragebot.rageBotData.pAimbotTarget->SetBoneCache(ragebot.rageBotData.pTargetMatrix);
	i::EngineTrace->ClipRayToEntity(ray, MASK_SHOT, ragebot.rageBotData.pAimbotTarget, &trace);

	if (bResolverHandler[PLAYERDEATH] || bResolverHandler[PLAYERHURT]) {
		for (bool& bCurrent : bResolverHandler)
			bCurrent = false;
		return;
	}

	for (bool& bCurrent : bResolverHandler)
		bCurrent = false;

	if (trace.pHitEntity == ragebot.rageBotData.pAimbotTarget || bCollided) {

		missedShots[ragebot.rageBotData.pAimbotTarget->EntIndex()]++;
		didHurt = false;
		ragebot.rageBotData.pAimbotTarget = nullptr;
		bulletImpact = Vector(0, 0, 0);
		ragebot.rageBotData.pTargetMatrix = nullptr;
		ragebot.rageBotData.iTargetHitbox = 0;
		util::LogConsole("Missed shot due to animation desyncronaztion\n");
	}
	else {

		didHurt = false;
		ragebot.rageBotData.pAimbotTarget = nullptr;
		ragebot.rageBotData.pTargetMatrix = nullptr;
		bulletImpact = Vector(0, 0, 0);
		ragebot.rageBotData.iTargetHitbox = 0;
		util::LogConsole("Missed shot due to spread\n");
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

			for (bool& bCurrent : bResolverHandler)
				bCurrent = false;
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

			for (bool& bCurrent : bResolverHandler)
				bCurrent = false;
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

void Animations::UpdateClientSideAnimations(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord)
{
	CAnimState* pState = pEntity->AnimState();

	pState->iLastUpdateFrame = 0;
	if (pState->flLastUpdateTime == i::GlobalVars->flCurrentTime)
		pState->flLastUpdateTime = i::GlobalVars->flCurrentTime + i::GlobalVars->flIntervalPerTick;

	pState->pEntity = pEntity;
	pState->pLastActiveWeapon = pEntity->GetWeapon();

	for (int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++)
	{
		CAnimationLayer* pLayer = &pEntity->GetAnimationOverlays()[iLayer];
		if (!pLayer)
			continue;

		pLayer->pOwner = pEntity;
	}

	bool bClientSideAnimation = pEntity->IsClientSideAnimation();
	pEntity->IsClientSideAnimation() = true;

	g::bAllowAnimations[pEntity->EntIndex()] = true;
	pEntity->UpdateClientSideAnimations();
	g::bAllowAnimations[pEntity->EntIndex()] = false;

	pEntity->IsClientSideAnimation() = bClientSideAnimation;
	pEntity->InvalidatePhysicsRecursive(ANIMATION_CHANGED | ANGLES_CHANGED | POSITION_CHANGED);

	//// don't let the server update animation state.
	//FixAnimatingInSameFrame(pEntity);

	//// update player animation.
	//pEntity->IsClientSideAnimation() = g::bAllowAnimations[pEntity->EntIndex()] = true;
	//pEntity->UpdateClientSideAnimations();
	//pEntity->IsClientSideAnimation() = g::bAllowAnimations[pEntity->EntIndex()] = false;

	//// invalidate physics.
	//nInvalidateFlags |= (ANIMATION_CHANGED);

	//// invalidate physics.
	//pEntity->InvalidatePhysicsRecursive(nInvalidateFlags);
}

void Animations::GenerateSafePointMatricies(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	auto GetYawRotation = [&](int nRotationSide) -> float
	{
		float flOldEyeYaw = pRecord->pEntity->AnimState()->flEyeYaw;

		// set eye yaw
		float flEyeRotation = pRecord->vecEyeAngles.y;
		switch (nRotationSide)
		{
		case -1: pRecord->pEntity->AnimState()->flEyeYaw = M::NormalizeAngle(flEyeRotation - 60.0f); break;
		case 0: pRecord->pEntity->AnimState()->flEyeYaw = M::NormalizeAngle(flEyeRotation); break;
		case 1: pRecord->pEntity->AnimState()->flEyeYaw = M::NormalizeAngle(flEyeRotation + 60.0f); break;
		}

		// generate foot yaw
		float flFootYaw = BuildFootYaw(pEntity, pRecord);

		// restore eye yaw                                   
		pRecord->pEntity->AnimState()->flEyeYaw = flOldEyeYaw;

		// return result
		return flFootYaw;
	};

	// point building func
	auto BuildSafePoint = [&](int nRotationSide)
	{
		// save animation data
		std::array < CAnimationLayer, 13 > m_Layers;
		std::array < float, 24 > m_PoseParameters;
		CAnimState m_AnimationState;

		// copy data
		std::memcpy(m_Layers.data(), pEntity->GetAnimationOverlays(), sizeof(CAnimationLayer) * 13);
		std::memcpy(m_PoseParameters.data(), pEntity->GetPoseParameter().data(), sizeof(float) * 24);
		std::memcpy(&m_AnimationState, pEntity->AnimState(), sizeof(CAnimState));

		matrix3x4_t* aMatrix = nullptr;
		switch (nRotationSide)
		{
		case -1: aMatrix = pRecord->pMatricies[LEFT]; break;
		case 0: aMatrix = pRecord->pMatricies[CENTER]; break;
		case 1: aMatrix = pRecord->pMatricies[RIGHT]; break;
		}

		pEntity->AnimState()->flGoalFeetYaw = GetYawRotation(nRotationSide);

		UpdateClientSideAnimations(pEntity, pRecord);

		SetupPlayerMatrix(pEntity, pRecord, aMatrix, BoneUsedByHitbox);

		std::memcpy(pEntity->GetAnimationOverlays(), m_Layers.data(), sizeof(CAnimationLayer) * 13);
		std::memcpy(pEntity->GetPoseParameter().data(), m_PoseParameters.data(), sizeof(float) * 24);
		std::memcpy(pEntity->AnimState(), &m_AnimationState, sizeof(CAnimState));
	};

	// check conditions
	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	// build safe points
	BuildSafePoint(-1);
	BuildSafePoint(0);
	BuildSafePoint(1);
	//UpdateOnFeetYaw(pEntity, pRecord);
}

float Animations::BuildFootYaw(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	CAnimState* m_AnimationState = pEntity->AnimState();
	if (!m_AnimationState)
		return 0.0f;

	float flAimMatrixWidthRange = M::Lerp(std::clamp(m_AnimationState->flRunningSpeed, 0.f, 1.f), 1.0f, M::Lerp(m_AnimationState->flWalkToRunTransition, 0.8f, 0.5f));
	if (m_AnimationState->flDuckAmount > 0)
		flAimMatrixWidthRange = M::Lerp(m_AnimationState->flDuckAmount * std::clamp(m_AnimationState->flDuckingSpeed, 0.0f, 1.0f), flAimMatrixWidthRange, 0.5f);

	float flTempYawMax = m_AnimationState->flMaxBodyYaw * flAimMatrixWidthRange;
	float flTempYawMin = m_AnimationState->flMinBodyYaw * flAimMatrixWidthRange;

	float flFootYaw = M::NormalizeAngle(pRecord->pEntity->AnimState()->flEyeYaw);

	float flEyeFootDelta = M::AngleDiff(pRecord->vecEyeAngles.y, flFootYaw);
	if (flEyeFootDelta > flTempYawMax)
		flFootYaw = pRecord->vecEyeAngles.y - fabs(flTempYawMax);
	else if (flEyeFootDelta < flTempYawMin)
		flFootYaw = pRecord->vecEyeAngles.y + fabs(flTempYawMin);

	if (m_AnimationState->bOnGround)
	{
		if (m_AnimationState->flVelocityLenght2D > 0.1f || m_AnimationState->flJumpFallVelocity > 100.0f)
			flFootYaw = M::ApproachAngle(pRecord->pEntity->AnimState()->flEyeYaw, flFootYaw, i::GlobalVars->flIntervalPerTick * (30.0f + (20.0f * m_AnimationState->flWalkToRunTransition)));
		else
			flFootYaw = M::ApproachAngle(pRecord->flLowerBodyYawTarget, flFootYaw, i::GlobalVars->flIntervalPerTick * 100.f);
	}

	return M::NormalizeAngle(flFootYaw);
}

bool Animations::CopyCachedMatrix(CBaseEntity* pEnt, matrix3x4_t* pMatrix, int nBoneCount) {

	if (!g::pLocal || g::bUpdatingSkins)
		return false;

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEnt->EntIndex());

	if (!pLog)
		return false;

	if (pLog->pEntity == nullptr)
		return false;

	if (!pLog->pEntity->IsAlive() || pLog->pEntity != pEnt)
		return false;

	if (pLog->pRecord.empty())
		return false;

	if (pLog->pEntity->IsDormant() || pLog->pRecord.front().bDormant)
		return false;

	std::memcpy(pMatrix, pLog->pCachedMatrix.data(), sizeof(matrix3x4_t) * nBoneCount);

	return true;
}

void Animations::InterpolateMatricies(CBaseEntity* pEntity) {

	for (int nPlayerID = 1; nPlayerID <= 64; nPlayerID++)
	{
		CBaseEntity* pPlayer = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(nPlayerID));
		if (!pPlayer || !pPlayer->IsPlayer() || pPlayer == g::pLocal || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->GetTeam() == g::pLocal->GetTeam())
			continue;

		auto pPlayerData = &lagcomp.GetLog(nPlayerID);
		if (!pPlayerData || pPlayerData->pEntity != pPlayer || !pPlayerData->pCachedMatrix.data())
			continue;

		// get bone count
		int nBoneCount = pPlayer->GetCachedBoneData().Count();
		if (nBoneCount > MAXSTUDIOBONES)
			nBoneCount = MAXSTUDIOBONES;

		// re-pos matrix
		TransformateMatrix(pPlayer);

		// copy the entire matrix
		std::memcpy(pPlayer->GetCachedBoneData().Base(), pPlayerData->pCachedMatrix.data(), sizeof(matrix3x4_t) * nBoneCount);

		// build attachments
		std::memcpy(pPlayer->GetBoneAccessor().matBones, pPlayerData->pCachedMatrix.data(), sizeof(matrix3x4_t) * nBoneCount);
		pPlayer->SetupBones_AttachmentHelper();
		std::memcpy(pPlayer->GetBoneAccessor().matBones, pPlayerData->pCachedMatrix.data(), sizeof(matrix3x4_t) * nBoneCount);
	}
}

void Animations::TransformateMatrix(CBaseEntity* pEnt) {

	auto pRecord = &lagcomp.GetLog(pEnt->EntIndex());
	if (!pRecord || pRecord->pEntity != pEnt)
		return;	

	static Vector vecLastOrigin[65];
	Vector vecOriginDelta = pEnt->GetAbsOrigin() - vecLastOrigin[pEnt->EntIndex()];

	for (auto& Matrix : pRecord->pCachedMatrix)
	{
		Matrix[0][3] += vecOriginDelta.x;
		Matrix[1][3] += vecOriginDelta.y;
		Matrix[2][3] += vecOriginDelta.z;
	}
	vecLastOrigin[pEnt->EntIndex()] = pEnt->GetAbsOrigin();
}

void Animations::RebuiltLayer6(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer) {

	// rebuilt layer 6 calculations from csgo
	// links:
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/game/shared/cstrike15/csgo_playeranimstate.cpp#L1393
	// https://github.com/click4dylan/CSGO_AnimationCode_Reversed/blob/master/CCSGOPlayerAnimState_New.cpp#L2323
	// TODO: make code look good

	CAnimState* m_pState = pEntity->AnimState();

	// TODO: Find these members in the actual animstate struct
	float m_flLastUpdateIncrement = *(float*)((DWORD)m_pState + 0x74);
	float m_flFootYaw = m_pState->flGoalFeetYaw;
	float m_flMoveYaw = m_pState->flMoveYaw;
	Vector m_vecVelocityNormalizedNonZero = *(Vector*)((DWORD)m_pState + 0xE0);
	float m_flInAirSmoothValue = *(float*)((DWORD)m_pState + 0x124);
	AnimationData_t& m_AnimationData = pAnimationData[pEntity->EntIndex()];

	char m_szDestination[64];
	sprintf_s(m_szDestination, "move_%s", m_pState->GetWeaponPrefix());

	int m_nMoveSequence = pEntity->LookupSequence(m_szDestination);
	if (m_nMoveSequence == -1)
	{
		m_nMoveSequence = pEntity->LookupSequence("move");
	}

	// NOTE: 
	// pEntity->get<int>( 0x3984 ) is m_iMoveState 
	if (pEntity->GetOffset<int>(0x3984) != m_AnimationData.iMoveState)
		m_AnimationData.flMovePlaybackRate += 10.0f;

	m_AnimationData.iMoveState = pEntity->GetOffset<int>(0x3984);

	float m_flMovementTimeDelta = *(float*)((DWORD)m_pState + 0x74) * 40.0f;

	if (-m_AnimationData.flMovePlaybackRate <= m_flMovementTimeDelta)
	{
		if (-m_flMovementTimeDelta <= -m_AnimationData.flMovePlaybackRate)
			m_AnimationData.flMovePlaybackRate = 0.0f;
		else
			m_AnimationData.flMovePlaybackRate = m_AnimationData.flMovePlaybackRate - m_flMovementTimeDelta;
	}
	else
	{
		m_AnimationData.flMovePlaybackRate = m_AnimationData.flMovePlaybackRate + m_flMovementTimeDelta;
	}

	m_AnimationData.flMovePlaybackRate = std::clamp(m_AnimationData.flMovePlaybackRate, 0.0f, 100.0f);

	float m_flDuckSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xFC), 0.0f, 1.0f);
	float m_flRunSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xF8), 0.0f, 1.0f);

	float m_flSpeedWeight = ((m_flDuckSpeedClamped - m_flRunSpeedClamped) * m_pState->flDuckAmount) + m_flRunSpeedClamped;

	if (m_flSpeedWeight < pLayer->flFeetWeight)
	{
		float v34 = std::clamp(m_AnimationData.flMovePlaybackRate * 0.01f, 0.0f, 1.0f);
		float m_flFeetWeightElapsed = ((v34 * 18.0f) + 2.0f) * *(float*)((DWORD)m_pState + 0x74);
		if (m_flSpeedWeight - pLayer->flFeetWeight <= m_flFeetWeightElapsed)
			pLayer->flFeetWeight = -m_flFeetWeightElapsed <= (m_flSpeedWeight - pLayer->flFeetWeight) ? m_flSpeedWeight : pLayer->flFeetWeight - m_flFeetWeightElapsed;
		else
			pLayer->flFeetWeight = m_flFeetWeightElapsed + pLayer->flFeetWeight;
	}
	else
	{
		pLayer->flFeetWeight = m_flSpeedWeight;
	}

	float m_flYaw = M::NormalizeAngle((m_pState->flMoveYaw + m_pState->flGoalFeetYaw) + 180.0f);
	Vector m_angAngle = { 0.0f, m_flYaw, 0.0f };
	Vector m_vecDirection;
	M::AngleVectors(m_angAngle, &m_vecDirection);

	float m_flMovementSide = M::DotProduct(m_vecVelocityNormalizedNonZero, m_vecDirection);
	if (m_flMovementSide < 0.0f)
		m_flMovementSide = -m_flMovementSide;

	float m_flNewFeetWeight = M::Bias(m_flMovementSide, 0.2f) * pLayer->flFeetWeight;

	float m_flNewFeetWeightWithAirSmooth = m_flNewFeetWeight * m_flInAirSmoothValue;

	// m_flLayer5Weight looks a bit weird so i decided to name it m_flLayer5_Weight instead.
	float m_flLayer5_Weight = pEntity->GetAnimationOverlays()[5].flWeight;

	float m_flNewWeight = 0.55f;
	if (1.0f - m_flLayer5_Weight > 0.55f)
		m_flNewWeight = 1.0f - m_flLayer5_Weight;

	float m_flNewFeetWeightLayerWeight = m_flNewWeight * m_flNewFeetWeightWithAirSmooth;
	float m_flFeetCycleRate = 0.0f;

	float m_flSpeed = std::fmin(pEntity->GetVelocity().Length(), 260.0f);
	if (m_flSpeed > 0.00f)
	{
		float m_flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), m_nMoveSequence);

		float m_flSequenceMoveDist = pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), m_nMoveSequence);
		m_flSequenceMoveDist *= 1.0f / (1.0f / m_flSequenceCycleRate);
		if (m_flSequenceMoveDist <= 0.001f)
			m_flSequenceMoveDist = 0.001f;

		float m_flSpeedMultiplier = m_flSpeed / m_flSequenceMoveDist;
		m_flFeetCycleRate = (1.0f - (m_pState->flWalkToRunTransition * 0.15f)) * (m_flSpeedMultiplier * m_flSequenceCycleRate);
	}

	float m_flFeetCyclePlaybackRate = (*(float*)((DWORD)m_pState + 0x74) * m_flFeetCycleRate);
	m_AnimationData.flPrimaryCycle = m_flFeetCyclePlaybackRate + m_AnimationData.flPrimaryCycle;

	// store possible information for resolving.
	pLayer->flMovementSide = m_flMovementSide;
	pLayer->angMoveYaw = m_angAngle;
	pLayer->vecDirection = m_vecDirection;
	pLayer->flFeetWeight = m_flNewFeetWeight;

	// maybe it can be used for something, keeping it just in case.
	pLayer->nSequence = m_nMoveSequence;
	pLayer->flPlaybackRate = m_flFeetCyclePlaybackRate;
	pLayer->flCycle = m_AnimationData.flPrimaryCycle;
	pLayer->flFeetWeight = std::clamp(m_flNewFeetWeightLayerWeight, 0.0f, 1.0f);
}

Vector DeterminePlayerVelocity(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, CAnimState* pState) {

	/* Prepare data once */
	if (!pPrevious)
	{
		const float flVelLength = pRecord->vecVelocity.Length();
		if (flVelLength > pRecord->flMaxSpeed)
			pRecord->vecVelocity *= pRecord->flMaxSpeed / flVelLength;

		return pRecord->vecVelocity;
	}

	/* Define const */
	static CConVar* sv_maxspeed = i::ConVar->FindVar("sv_maxspeed");
	const float flMaxSpeed = sv_maxspeed->GetFloat();

	/* Get animation layers */
	const CAnimationLayer* AliveLoop = &pRecord->pLayers[ANIMATION_LAYER_ALIVELOOP];
	const CAnimationLayer* PrevAliveLoop = &pPrevious->pLayers[ANIMATION_LAYER_ALIVELOOP];
	const CAnimationLayer* Movement = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_MOVE];
	const CAnimationLayer* PrevMovement = &pPrevious->pLayers[ANIMATION_LAYER_MOVEMENT_MOVE];
	const CAnimationLayer* Landing = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	const CAnimationLayer* PrevLanding = &pPrevious->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];

	/* Recalculate velocity using origin delta */
	pRecord->vecVelocity = (pRecord->vecOrigin - pPrevious->vecOrigin) * (1.0f / TICKS_TO_TIME(pRecord->iChoked));

	/* Check PlaybackRate */
	if (Movement->flPlaybackRate < 0.00001f)
		pRecord->vecVelocity.x = pRecord->vecVelocity.y = 0.0f;
	else
	{
		/* Compute velocity using flSpeedAsPortionOfRunTopSpeed */
		float flWeight = AliveLoop->flWeight;
		if (flWeight < 1.0f)
		{
			/* Check PlaybackRate */
			if (AliveLoop->flPlaybackRate == PrevAliveLoop->flPlaybackRate)
			{
				/* Check Sequence */
				if (AliveLoop->nSequence == PrevAliveLoop->nSequence)
				{
					/* Very important cycle check */
					if (AliveLoop->flCycle > PrevAliveLoop->flCycle)
					{
						/* Check weapon */
						if (pState->pActiveWeapon == pEntity->GetWeapon())
						{
							/* Get flSpeedAsPortionOfRunTopSpeed */
							float flSpeedAsPortionOfRunTopSpeed = ((1.0f - flWeight) / 2.8571432f) + 0.55f;

							/* Check flSpeedAsPortionOfRunTopSpeed bounds ( from 55% to 90% from the speed ) */
							if (flSpeedAsPortionOfRunTopSpeed > 0.55f && flSpeedAsPortionOfRunTopSpeed < 0.9f)
							{
								/* Compute velocity */
								pRecord->flAnimationVelocity = flSpeedAsPortionOfRunTopSpeed * pRecord->flMaxSpeed;
								pRecord->nVelocityMode = EFixedVelocity::AliveLoopLayer;
							}
							else if (flSpeedAsPortionOfRunTopSpeed > 0.9f)
							{
								/* Compute velocity */
								pRecord->flAnimationVelocity = pRecord->vecVelocity.Length2D();
							}
						}
					}
				}
			}
		}

		/* Compute velocity using Movement ( 6 ) weight  */
		if (pRecord->flAnimationVelocity <= 0.0f)
		{
			/* Check Weight bounds from 10% to 90% from the speed */
			float flWeight = Movement->flWeight;
			if (flWeight > 0.1f && flWeight < 0.9f)
			{
				/* Skip on land */
				if (Landing->flWeight <= 0.0f)
				{
					/* Check Accelerate */
					if (flWeight > PrevMovement->flWeight)
					{
						/* Skip on direction switch */
						if (pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].nSequence == pPrevious->pLayers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].nSequence)
						{
							/* Check move sequence */
							if (Movement->nSequence == PrevMovement->nSequence)
							{
								/* Check land sequence */
								if (Landing->nSequence == PrevLanding->nSequence)
								{
									/* Check stand sequence */
									if (pRecord->pLayers[ANIMATION_LAYER_ADJUST].nSequence == pPrevious->pLayers[ANIMATION_LAYER_ADJUST].nSequence)
									{
										/* Check Flags */
										if (pRecord->iFlags & FL_ONGROUND)
										{
											/* Compute MaxSpeed modifier */
											float flSpeedModifier = 1.0f;
											if (pRecord->iFlags & FL_DUCKING)
												flSpeedModifier = CS_PLAYER_SPEED_DUCK_MODIFIER;
											else if (pRecord->bFakewalking)
												flSpeedModifier = CS_PLAYER_SPEED_WALK_MODIFIER;

											/* Compute Velocity ( THIS CODE ONLY WORKS IN DUCK AND WALK ) */
											if (flSpeedModifier < 1.0f)
											{
												pRecord->flAnimationVelocity = (flWeight * (pRecord->flMaxSpeed * flSpeedModifier));
												pRecord->nVelocityMode = EFixedVelocity::MovementLayer;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	/* Compute velocity from Record->flAnimationVelocity floating point */
	if (pRecord->flAnimationVelocity > 0.0f)
	{
		const float flModifier = pRecord->flAnimationVelocity / pRecord->vecVelocity.Length2D();
		pRecord->vecVelocity.x *= flModifier;
		pRecord->vecVelocity.y *= flModifier;
	}

	/* Prepare data once */
	const float flVelLength = pRecord->vecVelocity.Length();

	/* Clamp velocity if its out bounds */
	if (flVelLength > pRecord->flMaxSpeed)
		pRecord->vecVelocity *= pRecord->flMaxSpeed / flVelLength;

	return pRecord->vecVelocity;
}

int DetermineAnimationCycle(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	const CAnimationLayer* pAliveLoop = &pRecord->pLayers[ANIMATION_LAYER_ALIVELOOP];
	const CAnimationLayer* pPrevAliveLoop = &pPrevious->pLayers[ANIMATION_LAYER_ALIVELOOP];

	/* Get ticks animated on the server ( by default it's simtime delta ) */
	int nTicksAnimated = pRecord->iChoked;
	if (pAliveLoop->flPlaybackRate == pPrevAliveLoop->flPlaybackRate)
		nTicksAnimated = (pAliveLoop->flCycle - pPrevAliveLoop->flCycle) / (pAliveLoop->flPlaybackRate * i::GlobalVars->flIntervalPerTick);
	else
		nTicksAnimated = ((((pAliveLoop->flCycle / pAliveLoop->flPlaybackRate) + ((1.0f - pPrevAliveLoop->flCycle) / pPrevAliveLoop->flPlaybackRate)) / i::GlobalVars->flIntervalPerTick));

	return min(max(nTicksAnimated, pRecord->iChoked), 17);
}

Lagcompensation::LagRecord_t* SetupData(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::AnimationInfo_t* pLog) {

	if (pRecord->bFirstAfterDormant)
		return nullptr;

	if (pLog->pRecord.size() < 2)
		return nullptr;

	auto* pPrevious = &pLog->pRecord.at(1);
	pPrevious->iChoked = DetermineAnimationCycle(pEntity, pRecord, pPrevious);
	if (TIME_TO_TICKS(pRecord->flSimulationTime - pPrevious->flSimulationTime) > 17)
		return nullptr;

	return pPrevious;
}

void CopyRecordData(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, CAnimState* pState) {

	Lagcompensation::LagRecord_t* pForceRecord = pRecord;
	if (pPrevious)
		pForceRecord = pPrevious;

	CAnimationLayer* pStrafeLayer = &pForceRecord->pLayers[7];
	pState->flStrafeChangeCycle = pStrafeLayer->flCycle;
	pState->flStrafeChangeWeight = pStrafeLayer->flWeight;
	pState->nStrafeSequence = pStrafeLayer->nSequence;
	pState->flFeetCycle = pForceRecord->pLayers[6].flCycle;
	pState->flMoveWeight = pForceRecord->pLayers[6].flWeight;
	pState->flAccelerationWeight = pForceRecord->pLayers[12].flWeight;
	pEntity->SetAnimationLayers(pForceRecord->pLayers);
}

void UpdateCollision(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	ICollideable* pCollideable = pEntity->GetCollideable();
	if (!pCollideable)
		return;

	pEntity->UpdateCollisionBounds();
	pRecord->vecMins = pCollideable->OBBMins();
	pRecord->vecMaxs = pCollideable->OBBMaxs();
}

void SimulatePlayerActivity(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	const CAnimationLayer* pJumpingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	const CAnimationLayer* pLandingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	const CAnimationLayer* pPrevJumpingLayer = &pPrevious->pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	const CAnimationLayer* pPrevLandingLayer = &pPrevious->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];

	const int nJumpingActivity = pEntity->GetSequenceActivity(pJumpingLayer->nSequence);
	const int nLandingActivity = pEntity->GetSequenceActivity(pLandingLayer->nSequence);

	if (nJumpingActivity == ACT_CSGO_JUMP) {
		if (pJumpingLayer->flWeight > 0.0f && pJumpingLayer->flPlaybackRate > 0.0f) {
			if (pJumpingLayer->flCycle < pPrevJumpingLayer->flCycle) {
				pRecord->flDurationInAir = pJumpingLayer->flCycle / pJumpingLayer->flPlaybackRate;
				if (pRecord->flDurationInAir > 0.0f) {
					pRecord->iActivityTick = TIME_TO_TICKS(pRecord->flSimulationTime - pRecord->flDurationInAir) + 1;
					pRecord->iActivityType = Jump;
				}
			}
		}
	}

	if (nLandingActivity == ACT_CSGO_LAND_LIGHT || nLandingActivity == ACT_CSGO_LAND_HEAVY) {
		if (pLandingLayer->flWeight > 0.0f && pPrevLandingLayer->flWeight <= 0.0f) {
			if (pLandingLayer->flCycle > pPrevLandingLayer->flCycle) {
				float flLandDuration = pLandingLayer->flCycle / pLandingLayer->flPlaybackRate;
				if (flLandDuration > 0.0f) {

					pRecord->iActivityTick = TIME_TO_TICKS(pRecord->flSimulationTime - flLandDuration) + 1;
					pRecord->iActivityType = Land;

					float flDurationInAir = (pJumpingLayer->flCycle - pLandingLayer->flCycle);
					if (flDurationInAir < 0.0f)
						flDurationInAir += 1.0f;

					pRecord->flDurationInAir = flDurationInAir / pJumpingLayer->flPlaybackRate;
				}
			}
		}
	}
}

float ComputeActivityPlayback(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	CAnimationLayer* pJumpingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	CAnimationLayer* pLandingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];

	float flActivityPlayback = 0.0f;
	if (pJumpingLayer && pLandingLayer) {

		switch (pRecord->iActivityType) {

		case 1:
			flActivityPlayback = pEntity->GetLayerSequenceCycleRate(pJumpingLayer, pJumpingLayer->nSequence);
			break;

		case 2:
			flActivityPlayback = pEntity->GetLayerSequenceCycleRate(pLandingLayer, pLandingLayer->nSequence);
			break;
		}
	}
	return flActivityPlayback;
}

void HandleDormancyLeaving(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, CAnimState* pState) {

	/* Get animation layers */
	const CAnimationLayer* JumpingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	const CAnimationLayer* LandingLayer = &pRecord->pLayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];

	/* Final flLastUpdateTime */
	float flLastUpdateTime = pRecord->flSimulationTime - i::GlobalVars->flIntervalPerTick;

	/* Fix animation state timing */
	if (pRecord->iFlags & FL_ONGROUND) /* On ground */
	{
		/* Use information from landing */
		int nActivity = pEntity->GetSequenceActivity(LandingLayer->nSequence);
		if (nActivity == ACT_CSGO_LAND_HEAVY || nActivity == ACT_CSGO_LAND_LIGHT)
		{
			/* Compute land duration */
			float flLandDuration = LandingLayer->flCycle / LandingLayer->flPlaybackRate;

			/* Check landing time */
			float flLandingTime = pRecord->flSimulationTime - flLandDuration;
			if (flLandingTime == flLastUpdateTime)
			{
				pState->bOnGround = true;
				pState->bHitGroundAnimation = true;
				pState->flHitGroundCycle = 0.0f;
			}
			else if (flLandingTime - i::GlobalVars->flIntervalPerTick == flLastUpdateTime)
			{
				pState->bOnGround = false;
				pState->bHitGroundAnimation = false;
				pState->flHitGroundCycle = 0.0f;
			}

			/* Determine duration in air */
			float flDurationInAir = (JumpingLayer->flCycle - LandingLayer->flCycle);
			if (flDurationInAir < 0.0f)
				flDurationInAir += 1.0f;

			/* Set time in air */
			pState->flDurationInAir = flDurationInAir / JumpingLayer->flPlaybackRate;

			/* Check bounds.*/
			/* There's two conditions to let this data be useful: */
			/* It's useful if player has landed after the latest client animation update */
			/* It's useful if player has landed before the previous tick */
			if (flLandingTime < flLastUpdateTime && flLandingTime > pState->flLastUpdateTime)
				flLastUpdateTime = flLandingTime;
		}
	}
	else /* In air */
	{
		/* Use information from jumping */
		int nActivity = pEntity->GetSequenceActivity(JumpingLayer->nSequence);
		if (nActivity == ACT_CSGO_JUMP)
		{
			/* Compute duration in air */
			float flDurationInAir = JumpingLayer->flCycle / JumpingLayer->flPlaybackRate;

			/* Check landing time */
			float flJumpingTime = pRecord->flSimulationTime - flDurationInAir;
			if (flJumpingTime <= flLastUpdateTime)
				pState->bOnGround = false;
			else if (flJumpingTime - i::GlobalVars->flIntervalPerTick)
				pState->bOnGround = true;

			/* Check bounds.*/
			/* There's two conditions to let this data be useful: */
			/* It's useful if player has jumped after the latest client animation update */
			/* It's useful if player has jumped before the previous tick */
			if (flJumpingTime < flLastUpdateTime && flJumpingTime > pState->flLastUpdateTime)
				flLastUpdateTime = flJumpingTime;

			/* Set time in air */
			pState->flDurationInAir = flDurationInAir - i::GlobalVars->flIntervalPerTick;

			/* Disable landing */
			pState->bHitGroundAnimation = false;
		}
	}

	/* Set flLastUpdateTime */
	pState->flLastUpdateTime = flLastUpdateTime;
}

void Animations::RebuildEnemyAnimations(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::AnimationInfo_t* pLog) {

	CAnimState* pState = pEntity->AnimState();
	Lagcompensation::LagRecord_t* pPrevious = SetupData(pEntity, pRecord, pLog);
	
	pRecord->vecVelocity = DeterminePlayerVelocity(pEntity, pRecord, pPrevious, pState);
	if (pRecord->bFirstAfterDormant)
		HandleDormancyLeaving(pEntity, pRecord, pState);

	CopyRecordData(pEntity, pRecord, pPrevious, pState);

	const float flCurrentTime = i::GlobalVars->flCurrentTime;
	const float flRealTime = i::GlobalVars->flRealTime;
	const float flFrameTime = i::GlobalVars->flFrameTime;
	const float flAbsFrameTime = i::GlobalVars->flAbsFrameTime;
	const float flInterpolationAmount = i::GlobalVars->flInterpolationAmount;
	const float iTickCount = i::GlobalVars->iTickCount;
	const float iFrameCount = i::GlobalVars->iFrameCount;

	const Vector vecBackupVelocity = pEntity->GetVelocity();
	const Vector vecbackupAbsVelocity = pEntity->GetVecAbsVelocity();
	const Vector vecBackupAbsOrigin = pEntity->GetAbsOrigin();
	const int iBackupFlags = pEntity->GetFlags();
	const int iBackupEFlags = pEntity->GetEFlags();
	const float flBackupDuckAmount = pEntity->GetDuckAmount();
	const float flBackupLowerBodyYaw = pEntity->GetLowerBodyYaw();
	const float flBackupThirdpersonRecoil = pEntity->GetThirdpersonRecoil();

	pEntity->GetEFlags() &= ~(EFL_DIRTY_ABSVELOCITY | EFL_DIRTY_ABSTRANSFORM);

	UpdateCollision(pEntity, pRecord);

	if (pRecord->iChoked <= 1 || !pPrevious) {

		int iSimulatedCount = TIME_TO_TICKS(pRecord->flSimulationTime);

		i::GlobalVars->flCurrentTime = pRecord->flSimulationTime;
		i::GlobalVars->flRealTime = pRecord->flSimulationTime;
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->iFrameCount = iSimulatedCount;
		i::GlobalVars->iTickCount = iSimulatedCount;
		i::GlobalVars->flInterpolationAmount = 0.f;

		pEntity->SetAbsOrigin(pRecord->vecOrigin);
		pEntity->GetVelocity() = pRecord->vecVelocity;
		pEntity->GetVecAbsVelocity() = pRecord->vecVelocity;

		UpdateClientSideAnimations(pEntity, pRecord);
	}
	else {

		//if (pRecord->bBreakingLagcompensation)
		//	lagcomp.ExtrapolatePlayer(pEntity, pRecord, pPrevious);

		SimulatePlayerActivity(pEntity, pRecord, pPrevious);

		pRecord->flActivityPlayback = ComputeActivityPlayback(pEntity, pRecord);

		for (int iSimulationTick = 1; iSimulationTick <= pRecord->iChoked; iSimulationTick++) {

			float flSimulationTime = pPrevious->flSimulationTime + TICKS_TO_TIME(iSimulationTick);
			int iCurrentSimulationTick = TIME_TO_TICKS(flSimulationTime);

			i::GlobalVars->flCurrentTime = flSimulationTime;
			i::GlobalVars->flRealTime = flSimulationTime;
			i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
			i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
			i::GlobalVars->iFrameCount = iCurrentSimulationTick;
			i::GlobalVars->iTickCount = iCurrentSimulationTick;
			i::GlobalVars->flInterpolationAmount = 0.f;

			pEntity->GetDuckAmount() = M::AnimationLerp(pPrevious->flDuck, pRecord->flDuck, iSimulationTick, pRecord->iChoked);
			pEntity->GetLowerBodyYaw() = pPrevious->flLowerBodyYawTarget;
			pEntity->GetEyeAngles() = pPrevious->vecEyeAngles;

			pEntity->GetVecOrigin() = M::AnimationLerp(pPrevious->vecOrigin, pRecord->vecOrigin, iSimulationTick, pRecord->iChoked);
			pEntity->SetAbsOrigin(pEntity->GetVecOrigin());

			if (flSimulationTime < pRecord->flSimulationTime) {

				if (pRecord->bDidShot) {

					if (iCurrentSimulationTick < TIME_TO_TICKS(pRecord->flLastShotTime))
						pEntity->GetThirdpersonRecoil() = pPrevious->flThirdPersonRecoil;
					else {
						pEntity->GetEyeAngles() = pRecord->vecEyeAngles;
						pEntity->GetLowerBodyYaw() = pRecord->flLowerBodyYawTarget;
						pEntity->GetThirdpersonRecoil() = pRecord->flThirdPersonRecoil;
					}
				}

				if (pRecord->iActivityType != NoActivity) {
					if (iCurrentSimulationTick == pRecord->iActivityTick) {

						int nLayer = ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL;
						if (pRecord->iActivityType == Land)
							nLayer = ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB;

						pEntity->GetAnimationOverlays()[nLayer].flCycle = 0.0f;
						pEntity->GetAnimationOverlays()[nLayer].flWeight = 0.0f;
						pEntity->GetAnimationOverlays()[nLayer].flPlaybackRate = pRecord->flActivityPlayback;

						if (pRecord->iActivityType == Jump)
							pEntity->GetFlags() &= ~FL_ONGROUND;
						else if (pRecord->iActivityType == Land)
							pEntity->GetFlags() |= FL_ONGROUND;
					}
					else if (iCurrentSimulationTick < pRecord->iActivityTick) {

						if (pRecord->iActivityType == Jump)
							pEntity->GetFlags() |= FL_ONGROUND;
						else if (pRecord->iActivityType == Land)
							pEntity->GetFlags() &= ~FL_ONGROUND;
					}
				}
			}
			else {
				pEntity->GetFlags() = pRecord->iFlags;
				pEntity->GetDuckAmount() = pRecord->flDuck;
				pEntity->GetLowerBodyYaw() = pRecord->flLowerBodyYawTarget;
				pEntity->GetEyeAngles() = pRecord->vecEyeAngles;
			}

			Vector& vecVelocity = pEntity->GetVelocity();
			vecVelocity.x = M::AnimationLerp(pPrevious->vecVelocity.x, pRecord->vecVelocity.x, iSimulationTick, pRecord->iChoked);
			vecVelocity.y = M::AnimationLerp(pPrevious->vecVelocity.y, pRecord->vecVelocity.y, iSimulationTick, pRecord->iChoked);
			pEntity->m_vecAbsVelocity() = vecVelocity;

			UpdateClientSideAnimations(pEntity, pRecord);
		}
	}

	pEntity->SetAnimationLayers(pRecord->pLayers);

	pEntity->SetAbsOrigin(vecBackupAbsOrigin);

	pRecord->vecAbsAngles = Vector(0.0f, pState->flGoalFeetYaw, 0.0f);

	pEntity->GetPoseParameters(pRecord->flPoses);

	if (pEntity->GetTeam() != g::pLocal->GetTeam())
		Resolver(pEntity, pRecord, pPrevious);

	SetupPlayerMatrix(pEntity, pRecord, pRecord->pMatricies[VISUAL], Interpolated | VisualAdjustment);
	std::memcpy(pLog->pCachedMatrix.data(), pRecord->pMatricies[VISUAL], sizeof(matrix3x4_t)* MAXSTUDIOBONES);

	if (cfg::rage::enable && pEntity->GetTeam() != g::pLocal->GetTeam()) {
		//pRecord->pEntity->SetupBonesFix(pRecord->pEntity, BoneUsedByHitbox, i::GlobalVars->flCurrentTime, pRecord->pMatricies[RESOLVE]);
		SetupPlayerMatrix(pEntity, pRecord, pRecord->pMatricies[RESOLVE], BoneUsedByHitbox);
		GenerateSafePointMatricies(pRecord->pEntity, pRecord);
	}

	pEntity->GetVelocity() = vecBackupVelocity;
	pEntity->GetVecAbsVelocity() = vecbackupAbsVelocity;
	pEntity->SetAbsOrigin(vecBackupAbsOrigin);
	pEntity->GetFlags() = iBackupFlags;
	pEntity->GetEFlags() = iBackupEFlags;
	pEntity->GetDuckAmount() = flBackupDuckAmount;
	pEntity->GetLowerBodyYaw() = flBackupLowerBodyYaw;
	pEntity->GetThirdpersonRecoil() = flBackupThirdpersonRecoil;

	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flRealTime = flRealTime;
	i::GlobalVars->flFrameTime = flFrameTime;
	i::GlobalVars->flAbsFrameTime = flAbsFrameTime;
	i::GlobalVars->flInterpolationAmount = flInterpolationAmount;
	i::GlobalVars->iTickCount = iTickCount;
	i::GlobalVars->iFrameCount = iFrameCount;
}

void Animations::SetupPlayerMatrix(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, matrix3x4_t* pMatrix, int nFlags) {

	pEntity->SetAnimationLayers(pRecord->pLayers);

	const float flCurrentTime = i::GlobalVars->flCurrentTime;
	const float flRealTime = i::GlobalVars->flRealTime;
	const float flFrameTime = i::GlobalVars->flFrameTime;
	const float flAbsFrameTime = i::GlobalVars->flAbsFrameTime;
	const float flInterpolationAmount = i::GlobalVars->flInterpolationAmount;
	const float iTickCount = i::GlobalVars->iTickCount;
	const float iFrameCount = i::GlobalVars->iFrameCount;

	const int iLastSkipFrameCount = pEntity->GetLastSkipFrameCount();
	const int iEffect = pEntity->GetEffects();
	const int iClientEffects = pEntity->GetClientEffects();
	const int iOcclusionFlags = pEntity->GetOcclusionFlags();
	const Vector vecAbsOrigin = pEntity->GetAbsOrigin();

	int iSimulationTicks = TIME_TO_TICKS(pRecord->flSimulationTime);
	i::GlobalVars->flCurrentTime = pRecord->flSimulationTime;
	i::GlobalVars->flRealTime = pRecord->flSimulationTime;
	i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->iTickCount = iSimulationTicks;
	i::GlobalVars->iFrameCount = INT_MAX; /* ShouldSkipAnimationFrame fix */
	i::GlobalVars->flInterpolationAmount = 0.0f;

	pEntity->InvalidateBoneCache();

	for (int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++) {

		CAnimationLayer* pLayer = &pEntity->GetAnimationOverlays()[iLayer];
		if (!pLayer)
			continue;

		pLayer->pOwner = pEntity;
	}

	/* Disable ACT_CSGO_IDLE_TURN_BALANCEADJUST animation */
	if (nFlags & EMatrixFlags::VisualAdjustment) {

		pEntity->GetAnimationOverlays()[ANIMATION_LAYER_LEAN].flWeight = 0.0f;

		if (pEntity->GetSequenceActivity(pEntity->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST) {
			pEntity->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flCycle = 0.0f;
			pEntity->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flWeight = 0.0f;
		}
	}

	/* Remove interpolation if required */
	if (!(nFlags & EMatrixFlags::Interpolated) && pRecord->vecOrigin.IsValid())
		pEntity->SetAbsOrigin(pRecord->vecOrigin);

	/* Compute bone mask */
	int nBoneMask = BONE_USED_BY_ANYTHING;
	if (nFlags & EMatrixFlags::BoneUsedByHitbox)
		nBoneMask = BONE_USED_BY_HITBOX;

	pEntity->GetClientEffects() |= 2;
	pEntity->GetEffects() |= EF_NOINTERP;
	pEntity->GetOcclusionFrameCount() = -1;
	pEntity->GetOcclusionFlags() &= ~2;
	pEntity->GetLastSkipFrameCount() = 0;

	g::bSettingUpBones[pEntity->EntIndex()] = true;
	pEntity->SetupBones(pMatrix, MAXSTUDIOBONES, nBoneMask, 0.0f);
	g::bSettingUpBones[pEntity->EntIndex()] = false;

	pEntity->GetLastSkipFrameCount() = iLastSkipFrameCount;
	pEntity->GetEffects() = iEffect;
	pEntity->GetClientEffects() = iClientEffects;
	pEntity->GetOcclusionFlags() = iOcclusionFlags;
	pEntity->SetAbsOrigin(vecAbsOrigin);

	pEntity->SetAnimationLayers(pRecord->pLayers);

	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flRealTime = flRealTime;
	i::GlobalVars->flFrameTime = flFrameTime;
	i::GlobalVars->flAbsFrameTime = flAbsFrameTime;
	i::GlobalVars->flInterpolationAmount = flInterpolationAmount;
	i::GlobalVars->iTickCount = iTickCount;
	i::GlobalVars->iFrameCount = iFrameCount;
}