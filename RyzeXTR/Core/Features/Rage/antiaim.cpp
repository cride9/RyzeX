#include "antiaim.h"
#include "exploits.h"
#include "../../SDK/math.h"
#include "../../Features/Misc/enginepred.h"
#include "Animations/LocalAnimation.h"
#include "ragebot.h"
#include "autowall.h"
#include "../../Interface/Classes/CCSGameRulesProxy.h"

#include "../../SDK/InputSystem.h"
#include "../Networking/networking.h"
#include "../Misc/Playerlist.h"

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool&);

static bool jitter = false;

void HandleJitter(AATYPE type) {

	int tickbase = g::pLocal->GetTickBase();
	static int last_tick = 0;
	static float flCurtime = i::GlobalVars->flCurrentTime;
	if (last_tick + (i::ClientState->nChokedCommands + 1) < tickbase || last_tick > tickbase)
	{
		last_tick = tickbase;

		if (cfg::antiaim::bAntiJitter[type]) {
			if (flCurtime + 0.7f < i::GlobalVars->flCurrentTime) {
				flCurtime = i::GlobalVars->flCurrentTime;
				return;
			}
		}
		jitter = !jitter;
	}
}

bool LBYUpdate(CBaseEntity* pLocal)
{
	static float NextUpdate = 0;
	auto* AnimState = pLocal->AnimState();

	/*
		That LBY breaker is not even close to perfect so extended desync is not possible with it
	*/

	if (!AnimState || !(pLocal->GetFlags() & FL_ONGROUND)) {
		return false;
	}

	if (AnimState->flVelocityLenght2D > 0.1f)
		NextUpdate = TICKS_TO_TIME(networking.GetCorrectedTickbase()) + 0.22f;

	if (NextUpdate < TICKS_TO_TIME(networking.GetCorrectedTickbase()))
	{
		NextUpdate = TICKS_TO_TIME(networking.GetCorrectedTickbase()) + 1.1f;
		return true;
	}
	return false;
}

void antiaim::DoAntiaim(CUserCmd* pCmd, bool& bSendPacket, AATYPE type) {

	using namespace cfg::antiaim;

	HandleJitter(type);
	bool bInverted = IPT::HandleInput(iInverterBind);
	short bInvertValue = bInverted ? -1 : 1;
	if (cfg::antiaim::bInvertOnShoot[type])
		bInvertValue = bInverted ? antiaim::shotInvert ? 1 : -1 : antiaim::shotInvert ? -1 : 1;
	short bJitterValue = jitter ? -1 : 1;
	switch (iPitch[type]) {

	case UP:
		pCmd->angViewPoint.x = -89.f;
		break;

	case ZERO:
		pCmd->angViewPoint.x = 0.f;
		break;

	case DOWN:
		pCmd->angViewPoint.x = 89.f;
		break;
	}

	bool bInitializedFreestand = false;
	if (freestand[type] == 1)
		bInitializedFreestand = FreeStandingDistance(pCmd, pCmd->angViewPoint);
	if (freestand[type] == 2)
		bInitializedFreestand = FreeStandingThreat(pCmd->angViewPoint);

	if (iYawBase[type] == 1 && !bInitializedFreestand)
		AtTarget(pCmd, pCmd->angViewPoint);

	if (!bInitializedFreestand) {
		// yaw
		switch (iYaw[type]) {

		case FORWARD:
			pCmd->angViewPoint.y += 0.f;
			break;

		case BACKWARD:
			pCmd->angViewPoint.y += 180.f;
			break;
		}
	}

	if (bodyLean[type])
		pCmd->angViewPoint.y += bodyLean[bInverted][type];

	// add real jitter
	if (modifier[type] == 1)
		pCmd->angViewPoint.y += jitter ? -(jittervalue[type]) : (jittervalue[type]);
	else if (modifier[type] == 2)
		pCmd->angViewPoint.y += M::GenerateRandom(-jittervalue[type], jittervalue[type]);

	float flDesyncValue = 0.f;
	static int iChangeOnTick = 0;
	needMicromovement = true;
	switch (iDesyncType[type]) {

	case STATIC:

		flDesyncValue = 120.f * bInvertValue;
		break;

	case JITTER:

		flDesyncValue = (120.f * bInvertValue) * bJitterValue;
		break;

	case EXTENDED:

		needMicromovement = false;
		if (LBYUpdate(g::pLocal)) {

			bSendPacket = false;
			pCmd->angViewPoint.y -= (120 * bInvertValue);
		}
		else if (!bSendPacket) {
			// 95 = unhittable
			pCmd->angViewPoint.y += (95 * bInvertValue);
		}
		break;

	case FLICK:

		flDesyncValue = 120.f * bInvertValue;
		if (flickJitter) {

			iChangeOnTick++;

			if (iChangeOnTick >= cfg::antiaim::flickAngleSwitch[type]) {

				pCmd->angViewPoint.y = M::NormalizeYaw(pCmd->angViewPoint.y - (cfg::antiaim::iFlickOffset[type] * bInvertValue));
				antiaim::shotInvert = !antiaim::shotInvert;
				iChangeOnTick = 0;
			}
		}

		break;

	case XJITTER:

		if (iEnabledJitters[type] == 0)
			break;

		try {
			static int iLatestWay = 0;
			static int iLastTick = 0;
			int iTickBase = networking.GetCorrectedTickbase();
			static float flCurtime = i::GlobalVars->flCurrentTime;
			if (iLastTick + (i::ClientState->nChokedCommands + 1) < iTickBase || iLastTick > iTickBase)
			{
				if (iLatestWay >= iEnabledJitters[type])
					iLatestWay = 0;
				else {
					if (cfg::antiaim::bAntiJitter[type]) {
						if (flCurtime + 0.7f < i::GlobalVars->flCurrentTime) {
							flCurtime = i::GlobalVars->flCurrentTime;
							pCmd->angViewPoint.y += vecJitterWays[type].at(iLatestWay);
							break;
						}
					}
					iLatestWay++;
				}
				iLastTick = iTickBase;
			}
			pCmd->angViewPoint.y = M::NormalizeYaw(pCmd->angViewPoint.y + vecJitterWays[type].at(min(iLatestWay, vecJitterWays[type].size())));
			//flDesyncValue = 120.f * bInvertValue;

		}
		catch (std::exception) {
			
		}
		
		break;
	}

	if ((pCmd->flForwardMove == 0.0f || pCmd->iButtons & IN_DUCK) && needMicromovement)
		pCmd->flForwardMove += pCmd->iCommandNumber % 2 ? pCmd->iButtons & IN_DUCK ? -3.f : -1.1f : pCmd->iButtons & IN_DUCK ? 3.f : 1.1f;

	// add fake jitter
	if (!bSendPacket && needMicromovement) 
		pCmd->angViewPoint.y += flDesyncValue;
	
}

void antiaim::AntiAim(CUserCmd* pCmd, bool& bSendPacket) {

	static float oldValue = 0;

	// sanity checks
	if (!g::pLocal || !g::pLocal->GetHealth() || !g::pLocal->IsAlive()) 
		return;

	if ((*GameRules)->m_bFreezePeriod()) 
		return;
	
	// shooting checks
	if (antiaim::ShouldDisableAntiaim(pCmd, bSendPacket)) {
		if (!(cfg::rage::hideshot && IPT::HandleInput(cfg::rage::hideshotkey)))
			bSendPacket = (cfg::antiaim::fakeduck && IPT::HandleInput(cfg::antiaim::fakeduckbind)) ? bSendPacket : (cfg::rage::doubletap && IPT::HandleInput(cfg::rage::doubletapkey)) ? false : true;
		return;
	}

	// E, ladder, noclip check
	if (pCmd->iButtons & IN_USE || g::pLocal->GetMoveType() == MOVETYPE_LADDER || g::pLocal->GetMoveType() == MOVETYPE_NOCLIP || g::pLocal->GetFlags() & FL_FROZEN) 
		return;

	// Update lower body yaw
	Update( pCmd );

	if (!(g::pLocal->GetFlags() & FL_ONGROUND) && cfg::antiaim::bEnabled[INAIR])
		DoAntiaim(pCmd, bSendPacket, INAIR);
	else if (g::pLocal->GetVelocity().Length2D() > 5.f && cfg::antiaim::bEnabled[MOVING])
		DoAntiaim(pCmd, bSendPacket, MOVING);
	else if (cfg::antiaim::bEnabled[STANDING])
		DoAntiaim(pCmd, bSendPacket, STANDING);
}

float GetCorrectedCurrentTime( CUserCmd* cmd )
{
	const INetChannelInfo* v1 = static_cast< INetChannelInfo* > ( i::EngineClient->GetNetChannelInfo( ) );

	const float v3 = v1->GetAvgLatency( INetChannelInfo::LOCALPLAYER );
	const float v4 = v1->GetAvgLatency( INetChannelInfo::GENERIC );
	int corrected_tickcount = cmd->iTickCount;
	return v3 + v4 + TICKS_TO_TIME( 1 ) + TICKS_TO_TIME( corrected_tickcount );
}

bool antiaim::NextLBYUpdate( CUserCmd* cmd )
{
	if ( !( g::pLocal->GetFlags( ) & FL_ONGROUND ) )
		return false;

	const float CurrentTime = GetCorrectedCurrentTime( cmd );

	return m_flNextLBYUpdate - CurrentTime <= i::GlobalVars->flIntervalPerTick;
}

void antiaim::ForceResync( CUserCmd* m_pCmd, int m_iLbyChange )
{
	if ( !g::pLocal )
		return;

	CAnimState* m_pAnimationState = g::pLocal->AnimState( );
	if ( !m_pAnimationState )
		return;

	bool m_bCurrentlyInducedMovement = m_pCmd->flForwardMove >= 1.5f || m_pCmd->flForwardMove <= -1.5f || m_pCmd->flSideMove >= 1.5f || m_pCmd->flSideMove <= -1.5f;

	if ( m_pAnimationState->flVelocityLenght2D > 0.1f || std::fabsf( m_pAnimationState->flJumpFallVelocity ) > 100.f )
	{
		NextLBYUpdateTime = m_iLbyChange + 0.22f;
	}
	else
	{
		NextLBYUpdateTime = m_iLbyChange + 1.1f;
	}
}

static bool m_bOutOfSync;
static int m_nLastTickState;
static int m_nJustUpdated;
static float m_flLastTickLBY;

void antiaim::Update( CUserCmd* m_pCmd )
{
	float m_flCurrentTime = GetCorrectedCurrentTime( m_pCmd );
	m_nLastTickState = m_nJustUpdated;
	m_nJustUpdated = LBYUpdateType::LBYUPDATE_None;

	if ( !g::pLocal )
		return;

	CAnimState* m_pAnimationState = g::pLocal->AnimState( );
	if ( !m_pAnimationState )
		return;

	bool m_bCurrentlyInducedMovement = m_pCmd->flForwardMove >= 1.5f || m_pCmd->flForwardMove <= -1.5f || m_pCmd->flSideMove >= 1.5f || m_pCmd->flSideMove <= -1.5f;

	m_bOutOfSync = false;

	if ( m_pAnimationState->flVelocityLenght2D > 0.1f || std::fabsf( m_pAnimationState->flJumpFallVelocity ) || m_bCurrentlyInducedMovement )
	{
		m_nJustUpdated = LBYUpdateType::LBYUPDATE_Moving;
		m_flNextLBYUpdate = m_flCurrentTime + 0.22f; // updated becouse movement

		// reset counter
		iCountUpdates = NULL;
	}
	else
	{
		if ( g::pLocal->GetLowerBodyYaw( ) != m_flLastTickLBY )
		{
			m_flLastLBYChange = m_flCurrentTime - i::GlobalVars->flIntervalPerTick; // @SetupVelocity hititn p
			m_bOutOfSync = m_nLastTickState != LBYUpdateType::LBYUPDATE_Standing;
		}
		if ( m_flCurrentTime > m_flNextLBYUpdate )
		{
			float m_flGoalFeetYaw = M::NormalizeYaw( m_pAnimationState->flGoalFeetYaw );
			float m_flEyeYaw = M::NormalizeYaw( m_pCmd->angViewPoint.y );
			float m_flGoalFeetYawToEyeYawDelta = std::fabsf( m_flGoalFeetYaw - m_flEyeYaw );

			if ( m_flGoalFeetYawToEyeYawDelta > 35.f )
			{
				m_nJustUpdated = LBYUpdateType::LBYUPDATE_Standing; // server will update it so np
				m_flNextLBYUpdate = m_flCurrentTime + 1.1f;

				// we updated so let's increment our countr
				iCountUpdates++;
			}
		}
	}
	m_flLastTickLBY = g::pLocal->GetLowerBodyYaw( );

	ForceResync( m_pCmd, m_flLastLBYChange );
}

bool antiaim::ShouldDisableAntiaim(CUserCmd* pCmd, bool& bSendPacket) 
{
	const auto time = TICKS_TO_TIME(g::pLocal->GetTickBase());

	//if (misc::CanFireWeapon(time) && pCmd->iButtons & IN_ATTACK)
	//	return true;

	if (g::pLocal->GetWeapon()) {

		auto pWeapon = g::pLocal->GetWeapon();

		const auto info = pWeapon->GetCSWpnData();

		if (!info)
			return false;

		if (info->nWeaponType == WEAPONTYPE_C4 || info->nWeaponType == WEAPONTYPE_HEALTHSHOT)
			return false;

		if (info->nWeaponType == WEAPONTYPE_GRENADE) {

			auto pNade = static_cast<CBaseCSGrenade*>(pWeapon);

			if (!pNade->IsPinPulled() || pCmd->iButtons & (IN_ATTACK | IN_SECOND_ATTACK)) {

				float flThrowTime = pNade->GetThrowTime();

				if (flThrowTime > 0.f)
					return true;
			}
		}

		if (g::pLocal->GetNextAttack() >= time || pWeapon->GetNextPrimaryAttack() >= time || pWeapon->GetNextSecondaryAttack() >= time)
			return false;

		if (pCmd->iButtons & IN_ATTACK && info->nWeaponType != WEAPONTYPE_GRENADE) {
			return true;
		}

		if (info->nWeaponType == WEAPONTYPE_KNIFE) {

			if ((pCmd->iButtons & IN_ATTACK && pWeapon->GetNextPrimaryAttack() <= time) ||
				(pCmd->iButtons & IN_SECOND_ATTACK && pWeapon->GetNextSecondaryAttack() <= time)) {
				return true;
			}
		}
	}

	return false;
}

bool antiaim::FreeStandingDistance(CUserCmd* pCmd, Vector& vecCMDViewAngle) {

	bool bNoActive = true;
	float flBestRotation = 0.f;
	float flHighestThickness = 0.f;
	Vector vecBestHead;

	static float flBestThreat = 0.f;

	Vector vecEyePosition = g::pLocal->GetVecOrigin() + g::pLocal->GetViewOffset();
	Vector vecHeadPosition = g::pLocal->GetHitboxPosition(HITBOX_HEAD, g_LocalAnimations->GetDesyncMatrix().data());

	auto vecOrigin = g::pLocal->GetAbsOrigin();

	auto flCheckThickness = [&](CBaseEntity* pEntity, Vector vecNewHead) -> float
	{
		Vector vecEndPos1, vecEndPos2;
		Vector vecEyePos = pEntity->GetVecOrigin() + pEntity->GetViewOffset();


		CTraceFilter filter(pEntity);

		Trace_t trace1, trace2;
		i::EngineTrace->TraceRay(Ray_t(vecNewHead, vecEyePos), MASK_SHOT_BRUSHONLY, &filter, &trace1);

		if (trace1.DidHit())
			vecEndPos1 = trace1.vecEnd;
		else
			return 0.f;

		i::EngineTrace->TraceRay(Ray_t(vecEyePos, vecNewHead), MASK_SHOT_BRUSHONLY, &filter, &trace2);

		if (trace2.DidHit())
			vecEndPos2 = trace2.vecEnd;

		float add = vecNewHead.DistTo(vecEyePos) - vecEyePosition.DistTo(vecEyePos) + 3.f;
		return vecEndPos1.DistTo(vecEndPos2) + add / 3;
	};

	int iIndex = ClosesToCrosshair();

	CBaseEntity* pEntity = nullptr;

	if (iIndex != -1)
		pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(iIndex));

	if (!pEntity)
		return false;

	float flStep = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

	float flRadius = fabs(Vector(vecHeadPosition - vecOrigin).Length2D());

	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += flStep) {
		Vector newhead(flRadius * cos(rotation) + vecEyePosition.x, flRadius * sin(rotation) + vecEyePosition.y, vecEyePosition.z);

		float totalthickness = 0.f;

		bNoActive = false;

		totalthickness += flCheckThickness(pEntity, newhead);

		if (totalthickness > flHighestThickness)
		{
			flHighestThickness = totalthickness;
			flBestRotation = rotation;
			vecBestHead = newhead;
		}
	}
	
	pCmd->angViewPoint.y = M_RAD2DEG(flBestRotation);
}

int antiaim::ClosesToCrosshair() {

	int iIndex = -1;
	float flLowestFOV = INT_MAX;

	auto pLocal = g::pLocal;

	if (!pLocal)
		return -1;

	Vector vecLocalPosition = pLocal->GetVecOrigin() + pLocal->GetViewOffset();

	Vector vecRealAngles;
	i::EngineClient->GetViewAngles(vecRealAngles);

	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || pEntity->GetTeam() == pLocal->GetTeam() || pEntity->IsDormant() || pEntity == pLocal)
			continue;

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(i);
		if (!pLog || pLog->pRecord.empty())
			continue;

		Vector output;
		M::VectorAngles(pEntity->GetHitboxPosition(HITBOX_HEAD, pLog->pCachedMatrix.data()) - vecLocalPosition, output);
		Vector vecDistanceBetween = (g::vecOriginalViewAngle - output.NormalizeAngle());

		float flFov = abs((vecDistanceBetween).NormalizeAngle().Length2D());

		if (flFov < flLowestFOV) {

			flLowestFOV = flFov;
			iIndex = i;
		}
	}

	return iIndex;
}

int antiaim::ClosestToLocal() {

	int index = -1;
	float lowest_distance = INT_MAX;

	auto local_player = g::pLocal;

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetVecOrigin() + local_player->GetViewOffset();

	Vector angles;
	i::EngineClient->GetViewAngles(angles);

	for (size_t i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		CBaseEntity* entity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!entity || !entity->IsAlive() || entity->GetTeam() == local_player->GetTeam() || entity->IsDormant() || entity == local_player)
			continue;

		Vector2D point1 = Vector2D(local_player->GetVecOrigin().x, local_player->GetVecOrigin().y);
		Vector2D point2 = Vector2D(entity->GetVecOrigin().x, entity->GetVecOrigin().y);

		int diffY = point1.y - point2.y;
		int diffX = point1.x - point2.x;

		float distance = sqrt((diffY * diffY) + (diffX * diffX));

		if (distance < lowest_distance)
		{
			lowest_distance = distance;
			index = i;
		}
	}

	return index;
}

void antiaim::AtTarget(CUserCmd* pCmd, Vector& vecAngle) {

	Vector vecBestEntity = Vector(0, 0, 0);
	float flBestFov = 480.f;
	for (size_t i = 1; i < i::GlobalVars->nMaxClients; i++)
	{
		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!g::pLocal || !pEnt || !pEnt->IsAlive() || pEnt->GetTeam() == g::pLocal->GetTeam() || pEnt->IsDormant() || pEnt == g::pLocal)
			continue;

		if (playerList::arrPlayers[i].iPriority == FRIEND)
			continue;

		Vector vecCalcAngle;
		auto vecHitboxPosition = pEnt->GetHitboxPosition(HITBOX_UPPER_CHEST, pEnt->GetCachedBoneData().Base());

		M::VectorAngles(vecHitboxPosition - g::pLocal->GetEyePosition(false), vecCalcAngle);
		Vector vecDistanceBetween = (g::vecOriginalViewAngle.NormalizeAngle() - vecCalcAngle.NormalizeAngle());

		if (abs(vecDistanceBetween.Length2D()) < flBestFov) {

			vecBestEntity = vecCalcAngle;
			flBestFov = abs(vecDistanceBetween.Length2D());
		}
	}
	if (vecBestEntity == Vector(0, 0, 0))
		return;

	pCmd->angViewPoint.y = vecBestEntity.y;
}

bool antiaim::FreeStandingThreat(Vector& angle)
{
	auto GRD_TO_BOG = [&](float GRD) -> float {
		return (M_PI / 180) * GRD;
	};

	static float FinalAngle;
	bool bSide1 = false;
	bool bSide2 = false;
	bool autowalld = false;
	for (size_t i = 1; i <= i::GlobalVars->nMaxClients; ++i) {

		CBaseEntity* pPlayerEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == g::pLocal
			|| pPlayerEntity->GetTeam() == g::pLocal->GetTeam())
			continue;

		float flAngToLocal = M::CalcAngle(g::pLocal->GetVecOrigin(), pPlayerEntity->GetVecOrigin()).y;
		Vector vecViewPoint = pPlayerEntity->GetVecOrigin() + Vector(0, 0, 90);

		Vector2D vecSide1 = { (45 * sin(GRD_TO_BOG(flAngToLocal))),(45 * cos(GRD_TO_BOG(flAngToLocal))) };
		Vector2D vecSide2 = { (45 * sin(GRD_TO_BOG(flAngToLocal + 180))) ,(45 * cos(GRD_TO_BOG(flAngToLocal + 180))) };

		Vector2D vecSide3 = { (50 * sin(GRD_TO_BOG(flAngToLocal))),(50 * cos(GRD_TO_BOG(flAngToLocal))) };
		Vector2D vecSide4 = { (50 * sin(GRD_TO_BOG(flAngToLocal + 180))) ,(50 * cos(GRD_TO_BOG(flAngToLocal + 180))) };

		Vector vecOrigin = g::pLocal->GetVecOrigin();

		Vector2D vecOriginLeftRight[] = { Vector2D(vecSide1.x, vecSide1.y), Vector2D(vecSide2.x, vecSide2.y) };

		Vector2D vecOriginLeftRightLocal[] = { Vector2D(vecSide3.x, vecSide3.y), Vector2D(vecSide4.x, vecSide4.y) };

		for (int iSide = 0; iSide < 2; iSide++) {

			Vector vecOriginAutowall = { vecOrigin.x + vecOriginLeftRight[iSide].x,  vecOrigin.y - vecOriginLeftRight[iSide].y , vecOrigin.z + 80 };
			Vector vecOriginAutowall2 = { vecViewPoint.x + vecOriginLeftRightLocal[iSide].x,  vecViewPoint.y - vecOriginLeftRightLocal[iSide].y , vecViewPoint.z };

			if (autowall.CanHitFloatingPoint(vecOriginAutowall, vecViewPoint)) {

				if (iSide == 1)	{

					bSide1 = true;
					FinalAngle = flAngToLocal + 90;
				}
				else if (iSide == 0) {

					bSide2 = true;
					FinalAngle = flAngToLocal - 90;
				}
				autowalld = true;
			}
			else {
				for (int iSideID = 0; iSideID < 2; iSideID++) {

					Vector vecOriginAutowall3 = { vecOrigin.x + vecOriginLeftRight[iSideID].x,  vecOrigin.y - vecOriginLeftRight[iSideID].y , vecOrigin.z + 80 };

					if (autowall.CanHitFloatingPoint(vecOriginAutowall3, vecOriginAutowall2)) {

						if (iSideID == 1) {

							bSide1 = true;
							FinalAngle = flAngToLocal + 90;
						}
						else if (iSideID == 0) {

							bSide2 = true;
							FinalAngle = flAngToLocal - 90;
						}
						autowalld = true;
					}
				}
			}
		}
	}

	if (!autowalld || (bSide1 && bSide2))
		return false;
	else
		angle.y = FinalAngle;

	return true;
}

void antiaim::InvertOnShoot(CUserCmd* pCmd) {

	/* Invert on shoot */
	if (pCmd->iButtons & IN_ATTACK && cfg::antiaim::bInvertOnShoot) {

		antiaim::shotInvert = !antiaim::shotInvert;
	}
}