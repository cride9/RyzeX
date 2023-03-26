#include "antiaim.h"
#include "exploits.h"
#include "../../SDK/math.h"
#include "../../Features/Misc/enginepred.h"
#include "Animations/LocalAnimation.h"

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool&);
bool LBYUpdate();

static bool evenInvert = false;
static bool unevenInvert = false;

void antiaim::AntiAim(CUserCmd* pCmd, bool& bSendPacket) {

	static float oldValue = 0;

	// sanity checks
	if (!g::pLocal || !g::pLocal->GetHealth() || !g::pLocal->IsAlive() || !cfg::antiaim::enabled) {

		if (!cfg::antiaim::fakelag)
			bSendPacket = true;
		desyncValue = 0.f;
		g::bAntiaimEnabled = false;
		return;
	}

	// shooting checks
	if (ShouldDisableAntiaim(pCmd, bSendPacket) && pCmd->iButtons & IN_ATTACK) {

		desyncValue = 0.f;
		g::bAntiaimEnabled = false;
		bSendPacket = (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind)) ? bSendPacket : (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) ? g::bWaiting ? true : false : true;

		return;
	}

	// E, ladder, noclip check
	if (g::pCmd->iButtons & IN_USE || g::pLocal->GetMoveType() == MOVETYPE_LADDER || g::pLocal->GetMoveType() == MOVETYPE_NOCLIP) {

		desyncValue = 0.f;
		g::bAntiaimEnabled = false;

		return;
	}

	g::bAntiaimEnabled = true;

	// uneven, even fakelag jitter stuff
	evenInvert = !evenInvert;
	if (bSendPacket)
		unevenInvert = !unevenInvert;

	// pitch
	switch (cfg::antiaim::pitch) {

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

	if (cfg::antiaim::jittervalue)
		pCmd->angViewPoint.y += cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? -(cfg::antiaim::jittervalue) : (cfg::antiaim::jittervalue) : unevenInvert ? -(cfg::antiaim::jittervalue) : (cfg::antiaim::jittervalue);

	// yaw
	switch (cfg::antiaim::yaw) {

	case FORWARD:
		pCmd->angViewPoint.y += 0.f;
		break;

	case BACKWARD:
		pCmd->angViewPoint.y += 180.f;
		break;
	}

	if (cfg::antiaim::atTarget)
		FreeStanding(pCmd, pCmd->angViewPoint);

	// no lby break sry its 2022 nobody stands still and breaks lby
	if (pCmd->flForwardMove == 0.0f || pCmd->iButtons & IN_DUCK)
		pCmd->flForwardMove += evenInvert ? pCmd->iButtons & IN_DUCK ? -3.f : -1.1f : pCmd->iButtons & IN_DUCK ? 3.f : 1.1f;

	// pure cancer 4line antiaim no shit why do ppl hit my head 100%
	switch (cfg::antiaim::desynctype) {

	case STATIC:
			desyncValue = GetKeyState(cfg::antiaim::desyncinverter) ? (cfg::antiaim::desyncvalue) : -(cfg::antiaim::desyncvalue);
		break;

	case JITTER:
			desyncValue = cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? (cfg::antiaim::desyncvalue) : -(cfg::antiaim::desyncvalue) : unevenInvert ? (cfg::antiaim::desyncvalue) : -(cfg::antiaim::desyncvalue);
		break;

	default:
		desyncValue = 0.f;
		break;
	}

	if (!bSendPacket) {

		pCmd->angViewPoint.y += oldValue != desyncValue ? (desyncValue < 0.f ? -g::pLocal->AnimState()->GetMaxDesync() : g::pLocal->AnimState()->GetMaxDesync()) + desyncValue : desyncValue;
		oldValue = desyncValue;
	}
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

bool LBYUpdate()
{
	static float NextUpdate = 0;
	auto* AnimState = g::pLocal->AnimState();

	/*
		That LBY breaker is not even close to perfect so extended desync is not possible with it
	*/

	if (!AnimState || !(g::pLocal->GetFlags() & FL_ONGROUND)) {
		return false;
	}

	if (AnimState->flVelocityLenght2D > 0.1f)
		NextUpdate = TICKS_TO_TIME(g::pLocal->GetTickBase()) + 0.22f;

	if (NextUpdate < TICKS_TO_TIME(g::pLocal->GetTickBase()))
	{
		NextUpdate = TICKS_TO_TIME(g::pLocal->GetTickBase()) + 1.1f;
		return true;
	}
	return false;
}

void antiaim::FreeStanding(CUserCmd* cmd, Vector& angle) {

	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;

	static float m_bestthreat = 0.f;

	auto leyepos = g::pLocal->GetVecOrigin() + g::pLocal->GetViewOffset();
	auto headpos = g::pLocal->GetHitboxPosition(0);
	if (!headpos.has_value())
		return;
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
		return;

	float step = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

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
	}
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