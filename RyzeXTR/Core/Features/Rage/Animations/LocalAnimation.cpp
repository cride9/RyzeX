#include "LocalAnimation.h"
#include "../../Misc/enginepred.h"
#include "../../../SDK/math.h"
#include "../../../SDK/Menu/config.h"
#include "../../Networking/networking.h"
#include "../../../SDK/InputSystem.h"
#include "../antiaim.h"

void localanimation::SetSequence(CAnimationLayer* pLayer, int iSequence) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->nSequence != iSequence)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->nSequence = iSequence;
}

void localanimation::SetCycle(CAnimationLayer* pLayer, int iCycle) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->flCycle != iCycle)
		pLayer->pOwner->InvalidatePhysicsRecursive(ANIMATION_CHANGED);

	pLayer->flCycle = iCycle;
}

void localanimation::SetOrder(CAnimationLayer* pLayer, int iOrder) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->iOrder != iOrder)
		if (pLayer->iOrder == 13 || iOrder == 13)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->iOrder = iOrder;
}

void localanimation::SetWeight(CAnimationLayer* pLayer, float flWeight) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->flWeight == 0.f || flWeight == 0.f)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->flWeight = flWeight;
}

void localanimation::SetLayerSequence(CAnimationLayer* layer, int sequence) {

	if (sequence >= 2) {

		if (layer) {
			SetSequence(layer, sequence);
			layer->flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(layer, sequence);
			SetCycle(layer, 0.f);
			SetWeight(layer, 0.f);
		}
	}
}

void C_LocalAnimations::AnimationBreaker(float* flPoseParameter) {

	if (!cfg::antiaim::bSlideWalk)
		return;
	/*
	callbacks.register("post_anim_update", function(e)
local J = entity_list.get_client_entity( engine.get_local_player( ))
if menu.misc.animation_breaker:get() == 0 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(2)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(0, 0.5) --forward legs
elseif menu.misc.animation_breaker:get() == 1 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(2)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(0, 1) --backward legs
elseif menu.misc.animation_breaker:get() == 2 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(1)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(7, 0) --forward run animation
elseif menu.misc.animation_breaker:get() == 3 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(1)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(8, 0) --no animations
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(9, 0) --no animations
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(10, 0) --no animations
elseif menu.misc.animation_breaker:get() == 4 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(1)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(7, 0.8) --crab
elseif J and client.is_alive and menu.misc.animation_breaker:get() == 5 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(1)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(7, 0) --forward run animation
    ffi.cast("CCSGOPlayerAnimationState_534535_t**", get_entity_address(J:index()) + 0x9960)[0].flAffectedFraction = 1 --anim in air
elseif J and client.is_alive and menu.misc.animation_breaker:get() == 6 then
    ui.get("Misc", "General", "Movement", "Leg movement"):set(1)
    e:get_prop("DT_BaseAnimating", "m_flPoseParameter"):set_float_index(7, 0.8) --crab
    ffi.cast("CCSGOPlayerAnimationState_534535_t**", get_entity_address(J:index()) + 0x9960)[0].flAffectedFraction = 1 --anim in air
end
	*/

	flPoseParameter[8] = 0;
	flPoseParameter[9] = 0;
	flPoseParameter[10] = 0;
}

/* New stuff */
void C_LocalAnimations::OnCreateMove(bool& bSendPacket, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
		return;

	g_LocalAnimations->StoreAnimationRecord(pLocal);

	if (!bSendPacket) 
		return;

	std::tuple < float, float, float, float, float, int, int > m_Globals = std::make_tuple
	(
		i::GlobalVars->flCurrentTime,
		i::GlobalVars->flRealTime,
		i::GlobalVars->flFrameTime,
		i::GlobalVars->flAbsFrameTime,
		i::GlobalVars->flInterpolationAmount,

		i::GlobalVars->iFrameCount,
		i::GlobalVars->iTickCount
	);

	this->m_LocalData.m_nSimulationTicks = i::ClientState->nChokedCommands + 1;
	std::tuple < Vector, Vector, float, float, Vector, Vector, Vector, Vector, int, int, int, float, float > m_Data = std::make_tuple
	(
		pLocal->m_angVisualAngles(),
		pLocal->GetEyeAngles(),
		pLocal->GetDuckAmount(),
		pLocal->GetDuckSpeed(),
		pLocal->GetAbsOrigin(),
		pLocal->GetVecOrigin(),
		pLocal->GetVecAbsVelocity(),
		pLocal->GetVelocity(),
		pLocal->GetEFlags(),
		pLocal->GetFlags(),
		pLocal->GetMoveType(),
		pLocal->GetThirdpersonRecoil(),
		pLocal->GetLowerBodyYaw()
	);

	/* set localplayer entity's flags */
	pLocal->GetEFlags() &= ~(EFL_DIRTY_ABSVELOCITY | EFL_DIRTY_ABSTRANSFORM);

	/* remove interpolation amount */
	i::GlobalVars->flInterpolationAmount = 0.0f;

	/* shot data */
	std::tuple < Vector, bool > m_ShotData = std::make_tuple < Vector, bool >(Vector(0, 0, 0), false);

	/* copy data */
	g_LocalAnimations->CopyPlayerAnimationData(false, pLocal);

	/* UpdatePlayerAnimations */
	for (int nSimulationTick = 1; nSimulationTick <= m_LocalData.m_nSimulationTicks; nSimulationTick++)
	{
		/* determine the tickbase and set globals to it */
		int GetTickBase = pLocal->GetTickBase() - m_LocalData.m_nSimulationTicks + nSimulationTick;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(GetTickBase);
		i::GlobalVars->flRealTime = TICKS_TO_TIME(GetTickBase);
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->iFrameCount = GetTickBase;
		i::GlobalVars->iTickCount = GetTickBase;

		AnimationRecord_t* m_Record = &m_LocalData.m_AnimRecords[(g::pCmd->iCommandNumber - m_LocalData.m_nSimulationTicks + nSimulationTick) % 150];
		if (m_Record)
		{
			/* set player data from the animation record */ 
			pLocal->GetThirdpersonRecoil() = m_Record->m_angAimPunch.x * i::ConVar->FindVar("weapon_recoil_scale")->GetFloat();
			pLocal->GetVelocity() = m_Record->m_vecVelocity;
			pLocal->GetVecAbsVelocity() = m_Record->m_vecVelocity;
			pLocal->GetDuckAmount() = m_Record->m_flDuckAmount;
			pLocal->GetDuckSpeed() = m_Record->m_flDuckSpeed;
			pLocal->m_angVisualAngles() = m_Record->m_angRealAngles;
			pLocal->GetEyeAngles() = m_Record->m_angRealAngles;
			pLocal->GetFlags() = m_Record->m_nFlags;
			pLocal->GetMoveType() = m_Record->m_nMoveType;

			/* fix localplayer strafe and sequences */
			g_LocalAnimations->SimulateStrafe(m_Record->m_nButtons, pLocal);
			g_LocalAnimations->DoAnimationEvent(m_Record->m_nButtons, pLocal);

			/* set shot angle */
			if (nSimulationTick == m_LocalData.m_nSimulationTicks)
			{
				if (std::get < 1 >(m_ShotData))
				{
					pLocal->m_angVisualAngles() = std::get < 0 >(m_ShotData);
					pLocal->GetEyeAngles() = std::get < 0 >(m_ShotData);
				}
			}
			else
			{
				if (m_Record->m_bIsShooting)
				{
					std::get < 0 >(m_ShotData) = m_Record->m_angRealAngles;
					std::get < 1 >(m_ShotData) = true;
				}
			}
		}

		/* Fix framecount and time */
		pLocal->AnimState()->iLastUpdateFrame = 0;
		pLocal->AnimState()->flLastUpdateTime = i::GlobalVars->flCurrentTime - i::GlobalVars->flIntervalPerTick;

		/* set player and weapon */
		pLocal->AnimState()->pEntity = pLocal;
		pLocal->AnimState()->pActiveWeapon = pLocal->GetWeapon();

		/* force client-side animation */
		bool IsClientSideAnimation = pLocal->IsClientSideAnimation();
		pLocal->IsClientSideAnimation() = true;

		/* update localplayer animations */
		g::bAllowAnimations[pLocal->EntIndex()] = true;
		pLocal->UpdateClientSideAnimations();
		g::bAllowAnimations[pLocal->EntIndex()] = false;

		/* restore client-side animation */
		pLocal->IsClientSideAnimation() = IsClientSideAnimation;
	}

	/* copy layers */
	std::memcpy(m_LocalData.m_Real.m_Layers.data(), pLocal->GetAnimationOverlays(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(m_LocalData.m_Real.m_PoseParameters.data(), pLocal->GetPoseParameter().data(), sizeof(float) * MAXSTUDIOPOSEPARAM);

	pLocal->SetAbsOrigin(m_LocalData.m_vecAbsOrigin);
	//if ( !g_Globals->m_Packet.m_bSkipMatrix )
	g_LocalAnimations->SetupPlayerBones(m_LocalData.m_Real.m_Matrix.data(), BONE_USED_BY_ANYTHING, pLocal);
	g_LocalAnimations->UpdateDesyncAnimations(pLocal);

	/* restore globals */
	i::GlobalVars->flCurrentTime = std::get < 0 >(m_Globals);
	i::GlobalVars->flRealTime = std::get < 1 >(m_Globals);
	i::GlobalVars->flFrameTime = std::get < 2 >(m_Globals);
	i::GlobalVars->flAbsFrameTime = std::get < 3 >(m_Globals);
	i::GlobalVars->flInterpolationAmount = std::get < 4 >(m_Globals);
	i::GlobalVars->iFrameCount = std::get < 5 >(m_Globals);
	i::GlobalVars->iTickCount = std::get < 6 >(m_Globals);

	/* restore changed localplayer's data */
	pLocal->m_angVisualAngles() = std::get < 0 >(m_Data);
	pLocal->GetEyeAngles() = std::get < 1 >(m_Data);
	pLocal->GetDuckAmount() = std::get < 2 >(m_Data);
	pLocal->GetDuckSpeed() = std::get < 3 >(m_Data);
	pLocal->GetVecOrigin() = std::get < 5 >(m_Data);
	pLocal->GetVecAbsVelocity() = std::get < 6 >(m_Data);
	pLocal->GetVelocity() = std::get < 7 >(m_Data);
	pLocal->GetEFlags() = std::get < 8 >(m_Data);
	pLocal->GetFlags() = std::get < 9 >(m_Data);
	pLocal->GetMoveType() = std::get < 10 >(m_Data);
	pLocal->GetThirdpersonRecoil() = std::get < 11 >(m_Data);
	pLocal->GetLowerBodyYaw() = std::get < 12 >(m_Data);
}
void C_LocalAnimations::CopyPlayerAnimationData(bool bFake, CBaseEntity* pLocal)
{
	std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_Layers = m_LocalData.m_Real.m_Layers;
	if (bFake)
		m_Layers = m_LocalData.m_Fake.m_Layers;

	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		&m_Layers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		sizeof(CAnimationLayer)
	);
	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		&m_Layers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		sizeof(CAnimationLayer)
	);
	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_ALIVELOOP],
		&m_Layers[ANIMATION_LAYER_ALIVELOOP],
		sizeof(CAnimationLayer)
	);
}
void C_LocalAnimations::UpdateDesyncAnimations(CBaseEntity* pLocal)
{
	CAnimState m_AnimationState;
	std::memcpy(&m_AnimationState, pLocal->AnimState(), sizeof(CAnimState));

	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		&m_LocalData.m_Fake.m_Layers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		sizeof(CAnimationLayer)
	);
	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		&m_LocalData.m_Fake.m_Layers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		sizeof(CAnimationLayer)
	);
	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_ALIVELOOP],
		&m_LocalData.m_Fake.m_Layers[ANIMATION_LAYER_ALIVELOOP],
		sizeof(CAnimationLayer)
	);

	std::memcpy(pLocal->AnimState(), &m_LocalData.m_Fake.m_AnimationState, sizeof(CAnimState));
	std::memcpy(pLocal->GetPoseParameter().data(), m_LocalData.m_Fake.m_PoseParameters.data(), sizeof(float) * MAXSTUDIOPOSEPARAM);

	std::tuple < Vector, bool > m_ShotData = std::make_tuple < Vector, bool >(Vector(0, 0, 0), false);

	/* UpdatePlayerAnimations */
	for (int nSimulationTick = 1; nSimulationTick <= m_LocalData.m_nSimulationTicks; nSimulationTick++)
	{
		/* determine the tickbase and set globals to it */
		int GetTickBase = pLocal->GetTickBase() - m_LocalData.m_nSimulationTicks + nSimulationTick;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(GetTickBase);
		i::GlobalVars->flRealTime = TICKS_TO_TIME(GetTickBase);
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->iFrameCount = GetTickBase;
		i::GlobalVars->iTickCount = GetTickBase;

		AnimationRecord_t* m_Record = &m_LocalData.m_AnimRecords[(g::pCmd->iCommandNumber - m_LocalData.m_nSimulationTicks + nSimulationTick) % 150];
		if (m_Record)
		{
			/* set player data from the animation record */
			pLocal->GetThirdpersonRecoil() = m_Record->m_angAimPunch.x * i::ConVar->FindVar("weapon_recoil_scale")->GetFloat();
			pLocal->GetVelocity() = m_Record->m_vecVelocity;
			pLocal->GetVecAbsVelocity() = m_Record->m_vecVelocity;
			pLocal->GetDuckAmount() = m_Record->m_flDuckAmount;
			pLocal->GetDuckSpeed() = m_Record->m_flDuckSpeed;
			pLocal->m_angVisualAngles() = m_Record->m_angFakeAngles;
			pLocal->GetEyeAngles() = m_Record->m_angFakeAngles;
			pLocal->GetFlags() = m_Record->m_nFlags;
			pLocal->GetMoveType() = m_Record->m_nMoveType;

			/* fix localplayer strafe and sequences */
			g_LocalAnimations->SimulateStrafe(m_Record->m_nButtons, pLocal);
			g_LocalAnimations->DoAnimationEvent(m_Record->m_nButtons, pLocal, true);

			/* set shot angle */
			if (nSimulationTick == m_LocalData.m_nSimulationTicks)
			{
				if (std::get < 1 >(m_ShotData))
				{
					pLocal->m_angVisualAngles() = std::get < 0 >(m_ShotData);
					pLocal->GetEyeAngles() = std::get < 0 >(m_ShotData);
				}
			}
			else
			{
				if (m_Record->m_bIsShooting)
				{
					std::get < 0 >(m_ShotData) = m_Record->m_angRealAngles;
					std::get < 1 >(m_ShotData) = true;
				}
			}
		}

		/* Fix framecount */
		pLocal->AnimState()->iLastUpdateFrame = 0;

		/* set player and weapon */
		pLocal->AnimState()->pEntity = pLocal;
		pLocal->AnimState()->pActiveWeapon = pLocal->GetWeapon();

		/* force client-side animation */
		bool IsClientSideAnimation = pLocal->IsClientSideAnimation();
		pLocal->IsClientSideAnimation() = true;

		/* update localplayer animations */
		g::bAllowAnimations[pLocal->EntIndex()] = true;
		pLocal->UpdateClientSideAnimations();
		g::bAllowAnimations[pLocal->EntIndex()] = false;

		/* restore client-side animation */
		pLocal->IsClientSideAnimation() = IsClientSideAnimation;
	}

	std::memcpy(&m_LocalData.m_Fake.m_AnimationState, pLocal->AnimState(), sizeof(CAnimState));
	std::memcpy(m_LocalData.m_Fake.m_Layers.data(), pLocal->GetAnimationOverlays(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(m_LocalData.m_Fake.m_PoseParameters.data(), pLocal->GetPoseParameter().data(), sizeof(float) * MAXSTUDIOPOSEPARAM);

	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		&m_LocalData.m_Fake.m_Layers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL],
		sizeof(CAnimationLayer)
	);
	std::memcpy
	(
		&pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		&m_LocalData.m_Fake.m_Layers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB],
		sizeof(CAnimationLayer)
	);

	pLocal->GetPoseParameter()[1] = m_LocalData.m_Fake.m_PoseParameters[1];
	std::memcpy(&pLocal->GetAnimationOverlays()[7], &m_LocalData.m_Fake.m_Layers[7], sizeof(CAnimationLayer));

	if (!(pLocal->GetFlags() & FL_ONGROUND))
		pLocal->GetPoseParameter()[JUMP_FALL] = 1.f;

	m_LocalData.m_flYawDelta = std::roundf(M::AngleDiff(M::NormalizeAngle(pLocal->AnimState()->flGoalFeetYaw), M::NormalizeAngle(m_AnimationState.flGoalFeetYaw)));

	g_LocalAnimations->SetupPlayerBones(m_LocalData.m_Fake.m_Matrix.data(), 0, pLocal);

	std::memcpy(pLocal->AnimState(), &m_AnimationState, sizeof(CAnimState));
	std::memcpy(pLocal->GetAnimationOverlays(), m_LocalData.m_Fake.m_Layers.data(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(pLocal->GetPoseParameter().data(), m_LocalData.m_Fake.m_PoseParameters.data(), sizeof(float) * MAXSTUDIOPOSEPARAM);
}
void C_LocalAnimations::SimulateStrafe(int nButtons, CBaseEntity* pLocal)
{
	Vector vecForward;
	Vector vecRight;
	Vector vecUp;

	M::AngleVectors(Vector(0, pLocal->AnimState()->flGoalFeetYaw, 0), &vecForward, &vecRight, &vecUp);
	vecRight.NormalizeInPlace();

	float flVelToRightDot = M::DotProduct(pLocal->AnimState()->vecVelocityNormalizedNonZero, vecRight);
	float flVelToForwardDot = M::DotProduct(pLocal->AnimState()->vecVelocityNormalizedNonZero, vecForward);

	bool bMoveRight = (nButtons & (IN_MOVERIGHT)) != 0;
	bool bMoveLeft = (nButtons & (IN_MOVELEFT)) != 0;
	bool bMoveForward = (nButtons & (IN_FORWARD)) != 0;
	bool bMoveBackward = (nButtons & (IN_BACK)) != 0;

	bool bStrafeRight = (pLocal->AnimState()->flRunningSpeed >= 0.73f && bMoveRight && !bMoveLeft && flVelToRightDot < -0.63f);
	bool bStrafeLeft = (pLocal->AnimState()->flRunningSpeed >= 0.73f && bMoveLeft && !bMoveRight && flVelToRightDot > 0.63f);
	bool bStrafeForward = (pLocal->AnimState()->flRunningSpeed >= 0.65f && bMoveForward && !bMoveBackward && flVelToForwardDot < -0.55f);
	bool bStrafeBackward = (pLocal->AnimState()->flRunningSpeed >= 0.65f && bMoveBackward && !bMoveForward && flVelToForwardDot > 0.55f);

	pLocal->m_bStrafing() = (bStrafeRight || bStrafeLeft || bStrafeForward || bStrafeBackward);
}
void C_LocalAnimations::DoAnimationEvent(int nButtons, CBaseEntity* pLocal, bool bIsFakeAnimations)
{
	CAnimationLayer* pLandOrClimbLayer = &pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	if (!pLandOrClimbLayer)
		return;

	CAnimationLayer* pJumpOrFallLayer = &pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	if (!pJumpOrFallLayer)
		return;

	int nCurrentFlags = m_LocalData.m_Real.m_nFlags;
	if (bIsFakeAnimations)
		nCurrentFlags = m_LocalData.m_Fake.m_nFlags;

	int nCurrentMoveType = m_LocalData.m_Real.m_nMoveType;
	if (bIsFakeAnimations)
		nCurrentMoveType = m_LocalData.m_Fake.m_nMoveType;

	if (nCurrentMoveType != MOVETYPE_LADDER && pLocal->GetMoveType() == MOVETYPE_LADDER)
		pLocal->AnimState()->SetLayerSequence(pLandOrClimbLayer, ACT_CSGO_CLIMB_LADDER);
	else if (nCurrentMoveType == MOVETYPE_LADDER && pLocal->GetMoveType() != MOVETYPE_LADDER)
		pLocal->AnimState()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
	else
	{
		if (pLocal->GetFlags() & FL_ONGROUND)
		{
			if (!(nCurrentFlags & FL_ONGROUND))
				pLocal->AnimState()->SetLayerSequence
				(
					pLandOrClimbLayer,
					pLocal->AnimState()->flDurationInAir > 1.0f ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT
				);
		}
		else if (nCurrentFlags & FL_ONGROUND)
		{
			if (pLocal->GetVelocity().z > 0.0f)
				pLocal->AnimState()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_JUMP);
			else
				pLocal->AnimState()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
		}
	}

	if (bIsFakeAnimations)
	{
		m_LocalData.m_Fake.m_nMoveType = pLocal->GetMoveType();
		m_LocalData.m_Fake.m_nFlags = pLocal->GetFlags();
	}
	else
	{
		m_LocalData.m_Real.m_nMoveType = pLocal->GetMoveType();
		m_LocalData.m_Real.m_nFlags = pLocal->GetFlags();
	}
}
void C_LocalAnimations::StoreAnimationRecord(CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
		return;

	AnimationRecord_t m_AnimRecord;

	// store record data
	m_AnimRecord.m_nFlags = m_LocalData.m_nFlags;
	m_AnimRecord.m_vecOrigin = pLocal->GetVecOrigin();
	m_AnimRecord.m_vecVelocity = pLocal->GetVelocity();
	m_AnimRecord.m_flDuckAmount = pLocal->GetDuckAmount();
	m_AnimRecord.m_flDuckSpeed = pLocal->GetDuckSpeed();
	m_AnimRecord.m_angRealAngles = g::pCmd->angViewPoint;
	m_AnimRecord.m_angFakeAngles = localanim.localdata.vecViewAngle;
	m_AnimRecord.m_angAimPunch = pLocal->GetAimPunch();
	m_AnimRecord.m_nButtons = g::pCmd->iButtons;
	m_AnimRecord.m_nMoveType = pLocal->GetMoveType();

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon)
	{
		if (pWeapon->IsGrenade())
		{
			CBaseCSGrenade* pGrenade = reinterpret_cast<CBaseCSGrenade*>(pWeapon);
			if (!pGrenade->IsPinPulled() && pGrenade->GetThrowTime() > 0.0f)
				m_AnimRecord.m_bIsShooting = true;
		}
		else if ((pWeapon->GetItemIDHigh() == WEAPON_REVOLVER /*&& g_Globals->m_Packet.m_bCanFireRev*/) || (pWeapon->GetItemDefinitionIndex() != WEAPON_REVOLVER /*&& g_Globals->m_Packet.m_bCanFire*/))
		{
			if (g::pCmd->iButtons & IN_ATTACK)
				m_AnimRecord.m_bIsShooting = true;

			if (pWeapon->IsKnife())
				if ((g::pCmd->iButtons & IN_ATTACK) || (g::pCmd->iButtons & IN_SECOND_ATTACK))
					m_AnimRecord.m_bIsShooting = true;
		}
	}

	if (m_AnimRecord.m_bIsShooting)
		m_AnimRecord.m_angFakeAngles = g::pCmd->angViewPoint;
	m_AnimRecord.m_angFakeAngles.z = 0.0f;

	/* proper roll aa display */
	m_LocalData.m_AnimRecords[g::pCmd->iCommandNumber % 150] = m_AnimRecord;
}
void C_LocalAnimations::BeforePrediction(CBaseEntity* pLocal)
{
	m_LocalData.m_nFlags = pLocal->GetFlags();
	m_LocalData.m_vecAbsOrigin = pLocal->GetAbsOrigin();

	if (m_LocalData.m_flSpawnTime != pLocal->GetSpawnTime())
	{
		std::memcpy(&m_LocalData.m_Fake.m_AnimationState, pLocal->AnimState(), sizeof(CAnimState));
		std::memcpy(m_LocalData.m_Fake.m_Layers.data(), pLocal->GetAnimationOverlays(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);
		std::memcpy(m_LocalData.m_Fake.m_PoseParameters.data(), pLocal->GetPoseParameter().data(), sizeof(float) * MAXSTUDIOPOSEPARAM);
	}
	m_LocalData.m_flSpawnTime = pLocal->GetSpawnTime();
}
void C_LocalAnimations::SetupShootPosition(CBaseEntity* pLocal)
{
	/* fix view offset */
	Vector GetViewOffset = pLocal->GetViewOffset();
	if (GetViewOffset.z <= 46.05f)
		GetViewOffset.z = 46.0f;
	else if (GetViewOffset.z > 64.0f)
		GetViewOffset.z = 64.0f;

	/* calculate default shoot position */
	m_LocalData.m_vecShootPosition = pLocal->GetVecOrigin() + GetViewOffset;

	/* backup data */
	std::tuple < Vector, Vector > m_Backup = std::make_tuple(pLocal->GetAbsOrigin(), pLocal->GetEyeAngles());

	/* force LocalPlayer data */
	pLocal->SetAbsOrigin(pLocal->GetVecOrigin());
	pLocal->GetEyeAngles() = g::pCmd->angViewPoint;

	/* normalize angles */
	M::NormalizeYaw(g::pCmd->angViewPoint.y);
	M::ClampAngles(g::pCmd->angViewPoint);

	/* modify eye position rebuild */
	{
		/* should we modify eye pos */
		bool bModifyEyePosition = false;

		/* modify eye pos on land */
		static int Flags = pLocal->GetFlags();
		if (Flags != pLocal->GetFlags())
		{
			if (!(Flags & FL_ONGROUND) && (pLocal->GetFlags() & FL_ONGROUND))
				bModifyEyePosition = true;

			Flags = pLocal->GetFlags();
		}

		/* modify eye pos on duck */
		if (pLocal->GetDuckAmount() != 0.0f)
			bModifyEyePosition = true;

		/* modify eye pos on FD */
		if (IPT::HandleInput(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck)
			bModifyEyePosition = true;

		/* modify LocalPlayer's EyePosition */
		if (bModifyEyePosition)
		{
			/* store old body pitch */
			const float m_flOldBodyPitch = pLocal->GetPoseParameter()[12];

			/* determine m_flThirdpersonRecoil */
			const float m_flThirdpersonRecoil = pLocal->GetAimPunch().x * i::ConVar->FindVar("weapon_recoil_scale")->GetFloat();

			/* set body pitch */
			pLocal->GetPoseParameter()[12] = std::clamp(M::AngleDiff(M::NormalizeAngle(m_flThirdpersonRecoil), 0.0f), 0.0f, 1.0f);

			/* build matrix */
			g_LocalAnimations->SetupPlayerBones(m_LocalData.m_Shoot.m_Matrix.data(), BONE_USED_BY_HITBOX, pLocal);

			/* reset body pitch */
			pLocal->GetPoseParameter()[12] = m_flOldBodyPitch;

			/* CAnimState::ModifyEyePosition rebuild */
			g_LocalAnimations->ModifyEyePosition(m_LocalData.m_vecShootPosition, m_LocalData.m_Shoot.m_Matrix.data());
		}
	}

	/* restore LocalPlayer data */
	pLocal->SetAbsOrigin(std::get < 0 >(m_Backup));
	pLocal->GetEyeAngles() = std::get < 1 >(m_Backup);
}
void C_LocalAnimations::SetupPlayerBones(matrix3x4_t* aMatrix, int nMask, CBaseEntity* pLocal)
{
	// save globals
	std::tuple < float, float, float, float, float, int, int > m_Globals = std::make_tuple
	(
		// backup globals
		i::GlobalVars->flCurrentTime,
		i::GlobalVars->flRealTime,
		i::GlobalVars->flFrameTime,
		i::GlobalVars->flAbsFrameTime,
		i::GlobalVars->flInterpolationAmount,

		// backup frame count and tick count
		i::GlobalVars->iFrameCount,
		i::GlobalVars->iTickCount
	);

	// save player data
	std::tuple < int, int, int, int, int, bool > m_PlayerData = std::make_tuple
	(
		pLocal->GetLastSkipFrameCount(),
		pLocal->GetEffects(),
		pLocal->GetClientEffects(),
		pLocal->GetOcclusionFrameCount(),
		pLocal->GetOcclusionFlags(),
		false
	);

	// backup animation layers
	std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_Layers;
	std::memcpy(m_Layers.data(), pLocal->GetAnimationOverlays(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

	/* set owners */
	for (int nLayer = 0; nLayer < ANIMATION_LAYER_COUNT; nLayer++)
	{
		CAnimationLayer* m_Layer = &pLocal->GetAnimationOverlays()[nLayer];
		if (!m_Layer)
			continue;

		m_Layer->pOwner = pLocal;
	}

	// get simulation time
	float flSimulationTime = TICKS_TO_TIME(networking.GetServerTick());

	// setup globals
	i::GlobalVars->flCurrentTime = flSimulationTime;
	i::GlobalVars->flRealTime = flSimulationTime;
	i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->flInterpolationAmount = 0.0f;
	i::GlobalVars->iTickCount = networking.GetServerTick();

	// fix skipanimframe ( part 1 )
	i::GlobalVars->iFrameCount = INT_MAX;

	// invalidate bone cache
	pLocal->InvalidateBoneCache();

	// disable ugly lean animation
	pLocal->GetAnimationOverlays()[ANIMATION_LAYER_LEAN].flWeight = 0.0f;

	// force client effects
	pLocal->GetClientEffects() |= 2; // disable ik

	// force effects to disable interp
	pLocal->GetEffects() |= EF_NOINTERP;

	// fix PVS occlusion
	pLocal->GetOcclusionFrameCount() = -1;
	pLocal->GetOcclusionFlags() &= ~2;

	// fix skipanimframe ( part 2 )
	pLocal->GetLastSkipFrameCount() = 0;

	// setup bones
	g::bSettingUpBones[pLocal->EntIndex()] = true;
	pLocal->SetupBones(aMatrix, MAXSTUDIOBONES, nMask, 0.0f);
	g::bSettingUpBones[pLocal->EntIndex()] = false;

	// restore animation layers
	std::memcpy(pLocal->GetAnimationOverlays(), m_Layers.data(), sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

	// restore player data
	pLocal->GetLastSkipFrameCount() = std::get < 0 >(m_PlayerData);
	pLocal->GetEffects() = std::get < 1 >(m_PlayerData);
	pLocal->GetClientEffects() = std::get < 2 >(m_PlayerData);
	pLocal->GetOcclusionFrameCount() = std::get < 3 >(m_PlayerData);
	pLocal->GetOcclusionFlags() = std::get < 4 >(m_PlayerData);

	// restore globals
	i::GlobalVars->flCurrentTime = std::get < 0 >(m_Globals);
	i::GlobalVars->flRealTime = std::get < 1 >(m_Globals);
	i::GlobalVars->flFrameTime = std::get < 2 >(m_Globals);
	i::GlobalVars->flAbsFrameTime = std::get < 3 >(m_Globals);
	i::GlobalVars->flInterpolationAmount = std::get < 4 >(m_Globals);

	// restore frame count and tick count
	i::GlobalVars->iFrameCount = std::get < 5 >(m_Globals);
	i::GlobalVars->iTickCount = std::get < 6 >(m_Globals);
}

void C_LocalAnimations::ModifyEyePosition(Vector& vecInputEyePos, matrix3x4_t* aMatrix)
{
	Vector vecHeadPos = Vector
	(
		aMatrix[8][0][3],
		aMatrix[8][1][3],
		aMatrix[8][2][3] + 1.7f
	);

	if (vecHeadPos.z > vecInputEyePos.z)
		return;

	float flLerp = M::RemapValClamped(abs(vecInputEyePos.z - vecHeadPos.z),
		4.f,
		10.f,
		0.0f, 1.0f);

	vecInputEyePos.z = (vecInputEyePos.z + (vecHeadPos.z - vecInputEyePos.z) * flLerp);
}
void C_LocalAnimations::InterpolateMatricies()
{
	auto pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pLocal->IsAlive())
		return;

	for (auto& current : m_LocalData.m_Real.m_Matrix)
		if (!current.GetOrigin().IsValid())
			continue;

	// correct matrix
	g_LocalAnimations->TransformateMatricies();

	// copy bones
	std::memcpy(pLocal->GetCachedBoneData().Base(), m_LocalData.m_Real.m_Matrix.data(), sizeof(matrix3x4_t) * pLocal->GetCachedBoneData().Count());
	std::memcpy(pLocal->GetBoneAccessor().matBones, m_LocalData.m_Real.m_Matrix.data(), sizeof(matrix3x4_t) * pLocal->GetCachedBoneData().Count());

	//pLocal->GetBoneAccessor().matBones = m_LocalData.m_Real.m_Matrix.data();

	return pLocal->SetupBones_AttachmentHelper();
}
void C_LocalAnimations::TransformateMatricies()
{
	if (i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal)
		return;

	Vector vecOriginDelta = pLocal->GetAbsOrigin() - m_LocalData.m_Real.m_vecMatriigin;
	for (auto& Matrix : m_LocalData.m_Real.m_Matrix)
	{
		Matrix[0][3] += vecOriginDelta.x;
		Matrix[1][3] += vecOriginDelta.y;
		Matrix[2][3] += vecOriginDelta.z;
	}

	for (auto& Matrix : m_LocalData.m_Fake.m_Matrix)
	{
		Matrix[0][3] += vecOriginDelta.x;
		Matrix[1][3] += vecOriginDelta.y;
		Matrix[2][3] += vecOriginDelta.z;
	}

	if (cfg::model::paperMode) {

		Vector curangle;
		i::EngineClient->GetViewAngles(curangle);
		int angle = 0;

		if (curangle.y <= 180 && curangle.y >= 145)
			angle = 0;
		else if (curangle.y >= -180 && curangle.y <= -145)
			angle = 0;
		else if (curangle.y >= 0 && curangle.y <= 45)
			angle = 0;
		else if (curangle.y <= 0 && curangle.y >= -45)
			angle = 0;
		else
			angle = 1;

		for (int i = 0; i < 128; i++) {

			m_LocalData.m_Fake.m_Matrix[i][angle][0] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][1] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][2] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][3] = (angle == 0 ? pLocal->GetAbsOrigin().x : pLocal->GetAbsOrigin().y);

			m_LocalData.m_Fake.m_Matrix[i][angle][0] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][1] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][2] = 0.1;
			m_LocalData.m_Fake.m_Matrix[i][angle][3] = (angle == 0 ? pLocal->GetAbsOrigin().x : pLocal->GetAbsOrigin().y);

			m_LocalData.m_Real.m_Matrix[i][angle][0] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][1] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][2] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][3] = (angle == 0 ? pLocal->GetAbsOrigin().x : pLocal->GetAbsOrigin().y);

			m_LocalData.m_Real.m_Matrix[i][angle][0] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][1] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][2] = 0.1;
			m_LocalData.m_Real.m_Matrix[i][angle][3] = (angle == 0 ? pLocal->GetAbsOrigin().x : pLocal->GetAbsOrigin().y);
		}

	}

	m_LocalData.m_Real.m_vecMatriigin = pLocal->GetAbsOrigin();
}
bool C_LocalAnimations::CopyCachedMatrix(matrix3x4_t* aInMatrix, int nBoneCount)
{
	std::memcpy(aInMatrix, m_LocalData.m_Real.m_Matrix.data(), sizeof(matrix3x4_t) * nBoneCount);
	return true;
}
void C_LocalAnimations::CleanSnapshots()
{
	*(float*)((DWORD)(g::pLocal) + 0x9B24) = 1.0f;
	*(float*)((DWORD)(g::pLocal) + 0xCF74) = 1.0f;
}
void C_LocalAnimations::ResetData()
{
	m_LocalData.m_nFlags = 0;
	m_LocalData.m_nSimulationTicks = 0;
	m_LocalData.m_flSpawnTime = 0.0f;
	m_LocalData.m_flYawDelta = 0.0f;
	m_LocalData.m_AnimRecords = { };
	m_LocalData.m_vecShootPosition = Vector(0, 0, 0);

	m_LocalData.m_Real.m_nMoveType = 0;
	m_LocalData.m_Real.m_nFlags = 0;
	m_LocalData.m_Real.m_Layers = { };
	m_LocalData.m_Real.m_PoseParameters = { };
	m_LocalData.m_Real.m_vecMatriigin = Vector(0, 0, 0);
	m_LocalData.m_Real.m_Matrix = { };

	m_LocalData.m_Fake.m_nMoveType = 0;
	m_LocalData.m_Fake.m_nFlags = 0;
	m_LocalData.m_Fake.m_Layers = { };
	m_LocalData.m_Fake.m_CleanLayers = { };
	m_LocalData.m_Fake.m_PoseParameters = { };
	m_LocalData.m_Fake.m_vecMatriigin = Vector(0, 0, 0);
	m_LocalData.m_Fake.m_Matrix = { };

	m_LocalData.m_Shoot.m_Matrix = { };
	m_LocalData.m_Shoot.m_Layers = { };
	m_LocalData.m_Shoot.m_PoseParameters = { };
}