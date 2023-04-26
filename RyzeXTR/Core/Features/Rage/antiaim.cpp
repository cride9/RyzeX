#include "antiaim.h"
#include "exploits.h"
#include "../../SDK/math.h"
#include "../../Features/Misc/enginepred.h"
#include "Animations/LocalAnimation.h"
#include "ragebot.h"
#include "autowall.h"

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool&);

static bool evenInvert = false;
static bool unevenInvert = false;

void antiaim::AntiAim(CUserCmd* pCmd, bool& bSendPacket) {

	static float oldValue = 0;

	// sanity checks
	if (!g::pLocal || !g::pLocal->GetHealth() || !g::pLocal->IsAlive() || !cfg::antiaim::bEnabled) {

		desyncValue = 0.f;
		return;
	}
	int inverter = GetKeyState(cfg::antiaim::iInverterBind) ? 1 : -1;

	if (ragebot.rageBotData.iTickCount + 3 >= i::GlobalVars->iTickCount) {

		desyncValue = 0.f;
		bSendPacket = true;
	}

	// shooting checks
	if (ShouldDisableAntiaim(pCmd, bSendPacket)) {

		desyncValue = 0.f;
		bSendPacket = (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind)) ? bSendPacket : (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) ? g::bWaiting ? true : false : true;

		return;
	}

	// E, ladder, noclip check
	if (g::pCmd->iButtons & IN_USE || g::pLocal->GetMoveType() == MOVETYPE_LADDER || g::pLocal->GetMoveType() == MOVETYPE_NOCLIP) {

		desyncValue = 0.f;

		return;
	}

	// Update lower body yaw
	Update( pCmd );

	// uneven, even fakelag jitter stuff
	evenInvert = !evenInvert;
	if (bSendPacket)
		unevenInvert = !unevenInvert;

	// pitch
	switch (cfg::antiaim::iPitch) {

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
	if (cfg::antiaim::freestand == 1)
		bInitializedFreestand = FreeStandingDistance(pCmd, pCmd->angViewPoint);
	if (cfg::antiaim::freestand == 2)
		bInitializedFreestand = FreeStandingThreat(pCmd->angViewPoint);

	if (cfg::antiaim::iYawBase == 1 && !bInitializedFreestand)
		AtTarget(pCmd, pCmd->angViewPoint);

	if (!bInitializedFreestand) {
		// yaw
		switch (cfg::antiaim::iYaw) {

		case FORWARD:
			pCmd->angViewPoint.y += 0.f;
			break;

		case BACKWARD:
			pCmd->angViewPoint.y += 180.f;
			break;
		}
	}

	if (cfg::antiaim::modifier == 1)
		pCmd->angViewPoint.y += cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? -(cfg::antiaim::jittervalue) : (cfg::antiaim::jittervalue) : unevenInvert ? -(cfg::antiaim::jittervalue) : (cfg::antiaim::jittervalue);
	else if (cfg::antiaim::modifier == 2)
		pCmd->angViewPoint.y += M::GenerateRandom(-cfg::antiaim::jittervalue, cfg::antiaim::jittervalue);

	if ( exploits::bCharged || exploits::bIsRecharging )
		needMicromovement = true;
	else
		needMicromovement = false;

	// pure cancer 4line antiaim no shit why do ppl hit my head 100%
	switch (cfg::antiaim::iDesyncType) {

		case STATIC:
		{
			needMicromovement = true;
			desyncValue = cfg::antiaim::flDesyncValue;
			break;
		case EXTENDED:
		{
			needMicromovement = false;
			// time to break the lowerbody.
			if ( NextLBYUpdate( pCmd ) )
			{
				desyncValue = -120.f;

				if ( cfg::antiaim::m_bSwayDesync )
				{
					// lby amount.
					switch ( iCountUpdates % 2 )
					{
					case 0: desyncValue = 120.f; break;
					case 1: desyncValue = -120.f; break;
					default: desyncValue = 120.f; break;
					}
				}

				// set lby angle.
				pCmd->angViewPoint.y += desyncValue * inverter;

				// set bSendPacket to false.
				bSendPacket = false;
			}
			else if (!bSendPacket) {
				pCmd->angViewPoint.y -= desyncValue * inverter;
			}
		}
			break;
		case JITTER:
			needMicromovement = true;
				desyncValue = cfg::antiaim::fakelag % 2 != 0 ? evenInvert ? (cfg::antiaim::flDesyncValue) : -(cfg::antiaim::flDesyncValue) : unevenInvert ? (cfg::antiaim::flDesyncValue) : -(cfg::antiaim::flDesyncValue);
			break;

		case FLICK:

			needMicromovement = false;
			if (NextLBYUpdate(pCmd)) {

				static int bDoFlick = 0;
				if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey))
					max(cfg::antiaim::flickAngleSwitch, 2);

				if (bSendPacket)
					bDoFlick++;

				if (bDoFlick == cfg::antiaim::flickAngleSwitch) {
					desyncValue = M::NormalizeYaw(cfg::antiaim::flDesyncValue - cfg::antiaim::iFlickOffset);
					bDoFlick = 0;
				}
				else {
					desyncValue = M::NormalizeYaw(-cfg::antiaim::flDesyncValue + cfg::antiaim::iFlickOffset);
				}
				pCmd->angViewPoint.y += desyncValue * inverter;
				bSendPacket = false;
			}
			else if (!bSendPacket) {
				pCmd->angViewPoint.y -= desyncValue * inverter;
			}
			break;

		default:
			needMicromovement = false;
			desyncValue = 0.f;
			break;
		}
	}
	if (bSendPacket)
		pCmd->angViewPoint.y += cfg::antiaim::bodyLean[inverter == 1 ? 0 : 1];

	// no lby break sry its 2022 nobody stands still and breaks lby
	if ((pCmd->flForwardMove == 0.0f || pCmd->iButtons & IN_DUCK) && needMicromovement)
		pCmd->flForwardMove += g::pCmd->iCommandNumber % 2 ? pCmd->iButtons & IN_DUCK ? -3.f : -1.1f : pCmd->iButtons & IN_DUCK ? 3.f : 1.1f;

	if (cfg::antiaim::iDesyncType) {
		if (cfg::antiaim::desyncModifier == 1)
			desyncValue += cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? -(cfg::antiaim::desyncModifierValue) : (cfg::antiaim::desyncModifierValue) : unevenInvert ? -(cfg::antiaim::desyncModifierValue) : (cfg::antiaim::desyncModifierValue);
		else if (cfg::antiaim::desyncModifier == 2)
			desyncValue += M::GenerateRandom(-cfg::antiaim::desyncModifierValue, cfg::antiaim::desyncModifierValue);
	}

	desyncValue *= inverter;

	if ( !bSendPacket && cfg::antiaim::iDesyncType != EXTENDED && cfg::antiaim::iDesyncType != FLICK ) {

		pCmd->angViewPoint.y += M::NormalizeYaw(oldValue != desyncValue ? (desyncValue < 0.f ? -g::pLocal->AnimState()->GetMaxDesync() : g::pLocal->AnimState()->GetMaxDesync()) + desyncValue : desyncValue);
		oldValue = desyncValue;
	}
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

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool& bSendPacket) 
{
	const auto time = TICKS_TO_TIME(g::pLocal->GetTickBase());

	if (g::pLocal->GetWeapon()) {

		auto pWeapon = g::pLocal->GetWeapon();

		const auto info = pWeapon->GetCSWpnData();

		if (!info)
			return false;

		if (info->nWeaponType == WEAPONTYPE_GRENADE) {

			auto pNade = static_cast<CBaseCSGrenade*>(pWeapon);

			if (!pNade->IsPinPulled() || pCmd->iButtons & (IN_ATTACK | IN_SECOND_ATTACK)) {

				float flThrowTime = pNade->GetThrowTime();

				if (flThrowTime > 0.f)
					return true;
			}
		}

		if (g::pLocal->GetNextAttack() > time || pWeapon->GetNextPrimaryAttack() > time || pWeapon->GetNextSecondaryAttack() > time)
			return false;

		if (pCmd->iButtons & IN_ATTACK && info->nWeaponType != WEAPONTYPE_GRENADE) {
			bSendPacket = true;
			return true;
		}

		if (info->nWeaponType == WEAPONTYPE_KNIFE) {

			if ((pCmd->iButtons & IN_ATTACK && pWeapon->GetNextPrimaryAttack() <= time) ||
				(pCmd->iButtons & IN_SECOND_ATTACK && pWeapon->GetNextSecondaryAttack() <= time)) {
				bSendPacket = true;
				return true;
			}
		}
	}

	return false;
}

bool antiaim::FreeStandingDistance(CUserCmd* cmd, Vector& angle) {

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	static float m_bestthreat = 0.f;

	auto leyepos = g::pLocal->GetVecOrigin() + g::pLocal->GetViewOffset();
	auto headpos = g::pLocal->GetHitboxPosition(0);
	if (!headpos.has_value())
		return false;
	auto origin = g::pLocal->GetAbsOrigin();


	auto checkWallThickness = [&](CBaseEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetVecOrigin() + pPlayer->GetViewOffset();


		CTraceFilter filter(pPlayer);

		Trace_t trace1, trace2;
		i::EngineTrace->TraceRay(Ray_t(newhead, eyepos), MASK_SHOT_BRUSHONLY, &filter, &trace1);

		if (trace1.DidHit())
			endpos1 = trace1.vecEnd;
		else
			return 0.f;

		i::EngineTrace->TraceRay(Ray_t(eyepos, newhead), MASK_SHOT_BRUSHONLY, &filter, &trace2);

		if (trace2.DidHit())
			endpos2 = trace2.vecEnd;

		float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
		return endpos1.DistTo(endpos2) + add / 3;
	};

	int index = ClosestToLocal();

	CBaseEntity* entity = nullptr;

	if (index != -1)
		entity = (CBaseEntity*)i::EntityList->GetClientEntity(index);

	if (!entity)
		return false;

	float step = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

	if (!headpos.has_value())
		return false;

	float radius = fabs(Vector(headpos.value() - origin).Length2D());

	if (index == -1)
		no_active = true;
	else {

		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step) {
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

			float totalthickness = 0.f;

			no_active = false;

			totalthickness += checkWallThickness(entity, newhead);

			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}
	if (!no_active) {
		cmd->angViewPoint.y = M_RAD2DEG(bestrotation);
		return true;
	}
	else
		return false;
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

	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		auto entity = (CBaseEntity*)i::EntityList->GetClientEntity(i);

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
	float flBestFov = 180.f;
	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		CBaseEntity* pEnt = (CBaseEntity*)i::EntityList->GetClientEntity(i);

		if (!g::pLocal || !pEnt || !pEnt->IsAlive() || pEnt->GetTeam() == g::pLocal->GetTeam() || pEnt->IsDormant() || pEnt == g::pLocal)
			continue;

		Vector vecCalcAngle;
		auto vecHitboxPosition = pEnt->GetHitboxPosition(HITBOX_UPPER_CHEST, pEnt->GetCachedBoneData().Base());

		M::VectorAngles(vecHitboxPosition - g::pLocal->GetEyePosition(), vecCalcAngle);
		Vector vecDistanceBetween = (g::vecOriginalViewAngle - vecCalcAngle);

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
	for (int i = 0; i <= i::GlobalVars->nMaxClients; ++i) {

		CBaseEntity* pPlayerEntity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

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

		// Set up a generic keyboard event.
		INPUT inputs[2] = {};
		ZeroMemory(inputs, sizeof(inputs));

		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = cfg::antiaim::iInverterBind;

		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.wVk = cfg::antiaim::iInverterBind;
		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	}
}