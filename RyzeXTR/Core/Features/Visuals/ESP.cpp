#include "ESP.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/EnemyAnimations.h"
#include "../Rage/ragebot.h"
#include "../../SDK/Menu/gui.h"

void AIPoints() {

	for (auto& point : misc::AIVizualization) {

		Vector AISpot;
		if (i::DebugOverlay->ScreenPosition(point, AISpot))
			continue;

		if (!g::pLocal)
			return;

		Vector LocalEyePos;
		if (i::DebugOverlay->ScreenPosition(g::pLocal->GetAbsOrigin(), LocalEyePos))
			continue;

		i::Surface->DrawSetColor(255.f, 255.f, 255.f, 255.f);
		i::Surface->DrawLine(LocalEyePos.x, LocalEyePos.y, AISpot.x, AISpot.y);
		//i::Surface->DrawFilledRect(points.x - 10, points.y - 10, points.x + 10, points.y + 10);
	}
}

void HitboxVisualization() {

	Vector screenPoint;
	if (i::DebugOverlay->ScreenPosition(visual::safePoint, screenPoint))
		return;

	i::Surface->DrawSetColor(255.f, 255.f, 255.f, 255.f);
	i::Surface->DrawOutlinedCircle(screenPoint.x, screenPoint.y, 10, 64);
}
using namespace cfg::visual;

void visual::VisualRender() {

	for (int i = 0; i < i::GlobalVars->nMaxClients; i++) {

		AIPoints();

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEnt || !g::pLocal) {
			iHealth[i] = -1;
			continue;
		}

		if (!pEnt->IsAlive() || pEnt->IsDormant()) {
			iHealth[i] = -1;
			continue;
		}

		if (!pEnt->GetHealth())
			continue;

		WorldCrosshair();
		//HitboxVisualization();

		Vector bot;
		if (i::DebugOverlay->ScreenPosition(pEnt->GetAbsOrigin() - Vector{ 0.f, 0.f, 9.f }, bot))
			continue;

		Vector vecMin, vecMax;
		pEnt->GetRenderBounds(vecMin, vecMax);

		Vector top;
		if (i::DebugOverlay->ScreenPosition(pEnt->GetAbsOrigin() + Vector(0, 0, vecMax.z + 5), top))
			continue;

		const float h = bot.y - top.y;
		const float w = h * 0.25f;

		const auto left = static_cast<int>(top.x - w);
		const auto right = static_cast<int>(top.x + w);
		bAmmoEnabled[i] = false;

		if (pEnt->GetTeam() != g::pLocal->GetTeam()) { // Enemy

			if (!bEnable[ENEMY])
				continue;

			if (bName[ENEMY]) NameEsp(left, top.y, right, bot.y, w, h, pEnt, Color(flNameColor[ENEMY]));
			if (bBox[ENEMY]) BoxEsp(left, top.y, right, bot.y, Color(flBoxColor[ENEMY]));
			if (bHealth[ENEMY]) HealthEsp(left, top.y, right, bot.y, w, h, pEnt->GetHealth(), Color(flHealthColorStart[ENEMY]), Color(flHealthColorEnd[ENEMY]), i);
			if (bArmor[ENEMY]) KevlarEsp(left, top.y, right, bot.y, pEnt, Color(flArmorColor[ENEMY]));
			if (bAmmo[ENEMY]) AmmoEsp(left, top.y, right, bot.y, pEnt, Color(flAmmoColor[ENEMY]));
			if (bWeapon[ENEMY]) WeaponEsp(left, top.y, right, bot.y, pEnt, Color(flWeaponColor[ENEMY]));
			Flags(top.y, right, pEnt, bFlags[ENEMY], flFlagsColor[ENEMY]);
		}
		else {

			if (pEnt == g::pLocal) { // Local

				if (!bEnable[LOCAL])
					continue;

				if (bName[LOCAL]) NameEsp(left, top.y, right, bot.y, w, h, pEnt, Color(flNameColor[LOCAL]));
				if (bBox[LOCAL]) BoxEsp(left, top.y, right, bot.y, Color(flBoxColor[LOCAL]));
				if (bHealth[LOCAL]) HealthEsp(left, top.y, right, bot.y, w, h, pEnt->GetHealth(), Color(flHealthColorStart[LOCAL]), Color(flHealthColorEnd[LOCAL]), i);
				if (bArmor[LOCAL]) KevlarEsp(left, top.y, right, bot.y, pEnt, Color(flArmorColor[LOCAL]));
				if (bAmmo[LOCAL]) AmmoEsp(left, top.y, right, bot.y, pEnt, Color(flAmmoColor[LOCAL]));
				if (bWeapon[LOCAL]) WeaponEsp(left, top.y, right, bot.y, pEnt, Color(flWeaponColor[LOCAL]));
				Flags(top.y, right, pEnt, bFlags[LOCAL], flFlagsColor[LOCAL]);

				continue;
			}
			if (!bEnable[TEAM])
				continue;

			// Teammate
			if (bName[TEAM]) NameEsp(left, top.y, right, bot.y, w, h, pEnt, Color(flNameColor[TEAM]));
			if (bBox[TEAM]) BoxEsp(left, top.y, right, bot.y, Color(flBoxColor[TEAM]));
			if (bHealth[TEAM]) HealthEsp(left, top.y, right, bot.y, w, h, pEnt->GetHealth(), Color(flHealthColorStart[TEAM]), Color(flHealthColorEnd[TEAM]), i);
			if (bArmor[TEAM]) KevlarEsp(left, top.y, right, bot.y, pEnt, Color(flArmorColor[TEAM]));
			if (bAmmo[TEAM]) AmmoEsp(left, top.y, right, bot.y, pEnt, Color(flAmmoColor[TEAM]));
			if (bWeapon[TEAM]) WeaponEsp(left, top.y, right, bot.y, pEnt, Color(flWeaponColor[TEAM]));
			Flags(top.y, right, pEnt, bFlags[TEAM], flFlagsColor[TEAM]);

		}
	}
}

void visual::BoxEsp(int left, int top, int right, int bot, Color color) {

	i::Surface->DrawSetColor(color[0], color[1], color[2], color[3]);
	i::Surface->DrawOutlinedRect(left, top, right, bot);

	i::Surface->DrawSetColor(0, 0, 0, color[3]);
	i::Surface->DrawOutlinedRect(left - 1, top - 1, right + 1, bot + 1);
}

void visual::HealthEsp(int left, int top, int right, int bot, int width, int height, int health, Color startColor, Color endColor, int iEntIndex) {

	if (iHealth[iEntIndex] <= 0)
		iHealth[iEntIndex] = health;

	if (iHealth[iEntIndex] > health)
		iHealth[iEntIndex] -= (iHealth[iEntIndex] - health < 3 ? 1 : 3);

	float fDistance = abs((float)(bot - top));
	const float flFactor = static_cast<float>(min(iHealth[iEntIndex], 100)) / static_cast<float>(100.f);
	float colorChange = 255.0f / fDistance;

	// Calculate the color change per step of the gradient
	float rStep = static_cast<float>(endColor[0] - startColor[0]) / fDistance;
	float gStep = static_cast<float>(endColor[1] - startColor[1]) / fDistance;
	float bStep = static_cast<float>(endColor[2] - startColor[2]) / fDistance;
	float aStep = static_cast<float>(endColor[3] - startColor[3]) / fDistance;

	i::Surface->DrawSetColor(0.f, 0.f, 0.f, (startColor[3] + endColor[3]) / 2);
	i::Surface->DrawOutlinedRect(left - 8, top, left - 4, bot + 1);
	for (size_t i = 0; i < fDistance; i++)
	{
		// Interpolate the color for the current step of the gradient
		int red = startColor[0] + (rStep * i);
		int green = startColor[1] + (gStep * i);
		int blue = startColor[2] + (bStep * i);
		int alpha = startColor[3] + (aStep * i);

		i::Surface->DrawSetColor(red, green, blue, alpha);
		i::Surface->DrawFilledRect(left - 7, bot - (i * flFactor), left - 5, bot - (i * flFactor) + 1);
		if (i + 1 == fDistance)
			i::Surface->DrawT(left - 7, bot - (i * flFactor) - 3, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, std::to_string(iHealth[iEntIndex]).c_str());
	}
}

void visual::NameEsp(int left, int top, int right, int bot, int width, int height, CBaseEntity* pEnt, Color color) {

	PlayerInfo_t info = { };

	if (!i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info))
		return;

	i::Surface->DrawT(left, top - 13, Color{ color[0], color[1], color[2], color[3] }, g::fonts::FlagESP, false, info.szName);
}

void visual::KevlarEsp(int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {


}

void visual::AmmoEsp(int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	int iEntIndex = pEnt->EntIndex();

	if (!pEnt->GetWeapon() || !pEnt->GetWeapon()->GetCSWpnData() || pEnt->GetWeapon()->IsKnife() || pEnt->GetWeapon()->IsGrenade())
		return;

	bAmmoEnabled[iEntIndex] = true;

	static CBaseCombatWeapon* oldWeapon = pEnt->GetWeapon();

	CBaseCombatWeapon* pWeapon = pEnt->GetWeapon();
	CCSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

	const float flFactor = static_cast<float>(pWeapon->GetAmmo()) / static_cast<float>(pWeaponInfo->iMaxClip1);
	const float flDifference = abs(right - left);

	i::Surface->DrawSetColor(0.f, 0.f, 0.f, color[3]);
	i::Surface->DrawOutlinedRect(left, bot + 4, right, bot + 8);

	i::Surface->DrawSetColor(color[0], color[1], color[2], color[3]);
	i::Surface->DrawOutlinedRect(left, bot + 5, left + (flDifference * flFactor), bot + 7);

	i::Surface->DrawT(left + (flDifference * flFactor), bot + 6, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, std::to_string(pWeapon->GetAmmo()).c_str());
}

void visual::BreakLCESP(int left, int top, int right, int bot, CBaseEntity* pEnt ) 
{
	//if ( !lagcomp.IsBreakingLagcompensation( pEnt ) )
	//	return;

	i::Surface->DrawT( left, bot, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, false, "Breaking Lagcomp" );
}

void visual::WeaponEsp(int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	if (!pEnt->GetWeapon())
		return;

	auto pWeaponInfo = pEnt->GetWeapon()->GetCSWpnData();

	if (!pWeaponInfo)
		return;

	std::string text = pWeaponInfo->szWeaponName;
	text.erase(0, 7);

	if (bAmmoEnabled[pEnt->EntIndex()])
		bot += 10;

	i::Surface->DrawT(left, bot, Color(color), g::fonts::FlagESP, false, text.c_str());
}

void visual::Flags(int top, int right, CBaseEntity* pEnt, bool* bFlags, float flFlagsColor[5][4]) {

	int spacing = -2;
	if (bFlags[NAME]) {

		PlayerInfo_t info = { };

		if (i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info)) {


			i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[NAME], g::fonts::FlagESP, false, info.szName);
			spacing += 10;
		}
	}

	if (bFlags[HEALTH]) {

		const float percentage = pEnt->GetHealth() / 100.f;
		std::string text = "Health: [";
		text += std::to_string(pEnt->GetHealth());
		text += "]";

		i::Surface->DrawT(right + 2, top + spacing, Color((1.f - percentage) * 1.f, 1.f * percentage, 0.f), g::fonts::FlagESP, false, text.c_str());

		spacing += 10;
	}

	if (bFlags[ARMOR]) {

		std::string text = "Kevlar: [";
		text += std::to_string(pEnt->GetArmor());
		text += "]";

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[ARMOR], g::fonts::FlagESP, false, text.c_str());

		spacing += 10;
	}

	if (bFlags[AMMO]) {

		if (!pEnt->GetWeapon()) {
			std::string text = "[ ";

			text += std::to_string(pEnt->GetWeapon()->GetAmmo());
			text += "/";
			text += std::to_string(pEnt->GetWeapon()->GetAmmoReserve());
			text += " ]";

			i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[AMMO], g::fonts::FlagESP, false, text.c_str());

			spacing += 10;
		}
	}

	if (bFlags[MONEY]) {

		std::string text = "$";
		text += std::to_string(pEnt->GetMoney()) + "\n";

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[MONEY], g::fonts::FlagESP, false, text.c_str());

		spacing += 10;
	}

	if (bFlags[WEAPON]) {

		if (pEnt->GetWeapon()) {

			auto pWeaponInfo = pEnt->GetWeapon()->GetCSWpnData();

			if (pWeaponInfo) {

				std::string text = pWeaponInfo->szWeaponName;
				text.erase(0, 7);
				i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[WEAPON], g::fonts::FlagESP, false, text.c_str());
				spacing += 10;
			}
		}
	}

	if (Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEnt->EntIndex()); pLog && pLog->pEntity && !pLog->pRecord.empty()) {

		using enum Lagcompensation::EResolverMode;

		std::string text = "";
		switch (pLog->iAntiAimType) {
		case LEGIT:
			text = "LEGIT"; break;
		case DESYNC:
			text = "DESYNC"; break;
		case OPPOSITE:
			text = "OPPOSITE"; break;
		case SWAY:
			text = "SWAY"; break;
		case FAKE:
			text = "FAKE"; break;
		case OVERRIDE:
			text = "OVERRIDE"; break;
		case ONSHOT:
			text = "ONSHOT"; break;
		case JITTER:
			text = "JITTER"; break;
		case ANIMATION:
			text = "ANIMATION"; break;
		}

		i::Surface->DrawT(right + 2, top + spacing, Color(184, 203, 131, 255), g::fonts::FlagESP, false, text.c_str());

		spacing += 10;
	}
}

void visual::Glow(CBaseEntity* pLocal)
{
	for (int i = 0; i < i::GlowObjectManager->vecGlowObjectDefinitions.Count(); i++)
	{
		IGlowObjectManager::GlowObject_t& hGlowObject = i::GlowObjectManager->vecGlowObjectDefinitions[i];

		// is current object not used
		if (hGlowObject.IsEmpty())
			continue;

		// get current entity from object handle
		CBaseEntity* pEntity = hGlowObject.pEntity;

		if (pEntity == nullptr)
			continue;

		// set bloom state
		hGlowObject.bFullBloomRender = false;

		CBaseClient* pClientClass = pEntity->GetClientClass();

		if (pClientClass == nullptr)
			continue;

		// get class id
		const EClassIndex nIndex = pClientClass->nClassID;

		switch (nIndex)
		{
		case EClassIndex::CC4:
		case EClassIndex::CPlantedC4:
		case EClassIndex::CCSPlayer:
		{
			if (pEntity->IsDormant() || !pEntity->IsAlive())
				break;

			if (pEntity->GetTeam() != pLocal->GetTeam() && cfg::visual::bGlow[ENEMY]) 
				hGlowObject.Set(Color(cfg::visual::flGlowColor[ENEMY]));

			else if (pEntity->GetTeam() == pLocal->GetTeam() && pEntity != g::pLocal && cfg::visual::bGlow[TEAM])
				hGlowObject.Set(Color(cfg::visual::flGlowColor[TEAM]));

			else if (pEntity == pLocal && cfg::visual::bGlow[LOCAL])
				hGlowObject.Set(Color(cfg::visual::flGlowColor[LOCAL]));

			break;
		}
		case EClassIndex::CBaseCSGrenadeProjectile:
		case EClassIndex::CDecoyProjectile:
		case EClassIndex::CMolotovProjectile:
		case EClassIndex::CSensorGrenadeProjectile:
		case EClassIndex::CSmokeGrenadeProjectile:
		default:
			break;
		}
	}
}

void visual::WorldCrosshair() {

	for (size_t i = 0; i < 5; i++) {

		if (vecWorldCrosshair[i] == Vector(0, 0, 0))
			continue;

		Vector vecScreenPoint;
		if (i::DebugOverlay->ScreenPosition(vecWorldCrosshair[i], vecScreenPoint))
			return;

		if (i::GlobalVars->flCurrentTime - 3.f > flWorldCrosshairLength[i]) {
			vecWorldCrosshair[i] = Vector(0, 0, 0);
			continue;
		}

		if (cfg::misc::flWorldCrosshairColor[3] > 0.f)
			cfg::misc::flWorldCrosshairColor[3] -= 0.0001f;

		/* Set color */
		i::Surface->DrawSetColor(Color(cfg::misc::flWorldCrosshairColor));

		/* Top right */
		i::Surface->DrawLine(vecScreenPoint.x + 2, vecScreenPoint.y - 2, vecScreenPoint.x + 8, vecScreenPoint.y - 8);
		/* Bottom right*/
		i::Surface->DrawLine(vecScreenPoint.x + 2, vecScreenPoint.y + 2, vecScreenPoint.x + 8, vecScreenPoint.y + 8);

		/* Top left */
		i::Surface->DrawLine(vecScreenPoint.x - 2, vecScreenPoint.y - 2, vecScreenPoint.x - 8, vecScreenPoint.y - 8);
		/* Bottom right*/
		i::Surface->DrawLine(vecScreenPoint.x - 2, vecScreenPoint.y + 2, vecScreenPoint.x - 8, vecScreenPoint.y + 8);
	}
}