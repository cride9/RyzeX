#include "antiaim.h"
#include "doubletap.h"
#include "../../SDK/math.h"
#include "../../Features/Misc/enginepred.h"

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool&);

static bool evenInvert = false;
static bool unevenInvert = false;

void antiaim::AntiAim(CUserCmd* pCmd, bool& bSendPacket, Vector vecOldViewAngle) {

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
		pCmd->angViewPoint.y += cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? (cfg::antiaim::jittervalue) : -(cfg::antiaim::jittervalue) : unevenInvert ? (cfg::antiaim::jittervalue) : -(cfg::antiaim::jittervalue);

	// yaw
	switch (cfg::antiaim::yaw) {

	case FORWARD:
		pCmd->angViewPoint.y += 0.f;
		break;

	case BACKWARD:
		pCmd->angViewPoint.y += 180.f;
		break;
	}

	// at target
	if (cfg::antiaim::atTarget) {

		float flAtTarget = pCmd->angViewPoint.y;
		float flFov = 180.f;

		// get all possible enemies
		for (int i = 0; i < 65; i++) {

			CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

			if (!pEnt || !pEnt->IsAlive() || pEnt->GetTeam() == g::pLocal->GetTeam() || !pEnt->GetClientRenderable())
				continue;

			// get closest to fov

			// get local players viewangle
			Vector vecViewAngle = pCmd->angViewPoint;

			// get enemy position
			Vector vecEnemyPosition = pEnt->GetHitboxPosition(HITBOX_HEAD).value();
			Vector vecAimPoint = M::CalcAngle(g::pLocal->GetEyePosition(), vecEnemyPosition).Normalize().Clamp();
			Vector vecDelta = vecAimPoint - vecViewAngle;

			// calculate fov
			vecDelta.Normalize();
			float flTemporaryFov = min(sqrtf(powf(vecDelta.x, 2.0f) + powf(vecDelta.y, 2.0f)), 180.f);

			if (flTemporaryFov < flFov) {
				flFov = flTemporaryFov;
				flAtTarget = vecAimPoint.y + 180;
			}
		}

		pCmd->angViewPoint.y = flAtTarget;
	}

	// no lby break sry its 2022 nobody stands still and breaks lby
	if (pCmd->flForwardMove == 0.0f || pCmd->iButtons & IN_DUCK)
		pCmd->flForwardMove += evenInvert ? pCmd->iButtons & IN_DUCK ? -3.f : -1.1f : pCmd->iButtons & IN_DUCK ? 3.f : 1.1f;

	// pure cancer 4line antiaim no shit why do ppl hit my head 100%
	switch (cfg::antiaim::desynctype) {

	case STATIC:
			desyncValue = GetKeyState(cfg::antiaim::desyncinverter) ? (58 + cfg::antiaim::desyncvalue) : -(58 + cfg::antiaim::desyncvalue);
		break;

	case JITTER:
			desyncValue = cfg::antiaim::fakelag % 2 == 0 ? evenInvert ? (58 + cfg::antiaim::desyncvalue) : -(58 + cfg::antiaim::desyncvalue) : unevenInvert ? (58 + cfg::antiaim::desyncvalue) : -(58 + cfg::antiaim::desyncvalue);
		break;

	default:
		desyncValue = 0.f;
		break;
	}

	if (!bSendPacket)
		pCmd->angViewPoint.y += desyncValue;
}

bool ShouldDisableAntiaim(CUserCmd* pCmd, bool& bSendPacket) {

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