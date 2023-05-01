#include "ESP.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/EnemyAnimations.h"
#include "../Rage/ragebot.h"
#include "../../SDK/Menu/gui.h"
#include "../../SDK/RayTracer rebuilt/CRayTrace.h"

void SafepointDebug(CBaseEntity* pEnt) {

	Vector vecMins, vecMaxs;
	float flRadius;

	if (!g::pLocal)
		return;

	for (size_t i = 0; i < 3; i++)
	{
		Vector currentPoint = visual::traceEnds[0];

		Vector AISpot;
		if (i::DebugOverlay->ScreenPosition(currentPoint, AISpot))
			return;

		Vector LocalEyePos;
		if (i::DebugOverlay->ScreenPosition(g::pLocal->GetEyePosition(), LocalEyePos))
			return;

		switch (i)
		{
		case 0:
			i::Surface->DrawSetColor(255.f, 0.f, 0.f, 255.f);
			break;
		case 1:
			i::Surface->DrawSetColor(0.f, 255.f, 0.f, 255.f);
			break;
		case 2:
			i::Surface->DrawSetColor(0.f, 0.f, 255.f, 255.f);
			break;
		}
		i::Surface->DrawLine(LocalEyePos.x, LocalEyePos.y + i, AISpot.x, AISpot.y);
	}
	//i::Surface->DrawFilledRect(points.x - 10, points.y - 10, points.x + 10, points.y + 10);
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

	for (size_t i = 1; i < i::GlobalVars->nMaxClients; i++) {

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

		//SafepointDebug(pEnt);
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

		float h = bot.y - top.y;
		float w = h * 0.25f;

		int left = static_cast<int>(top.x - w);
		int right = static_cast<int>(top.x + w);
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
			Flags(top.y, right, pEnt, i, bFlags[ENEMY], flFlagsColor[ENEMY]);
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
				Flags(top.y, right, pEnt, i, bFlags[LOCAL], flFlagsColor[LOCAL]);

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
			Flags(top.y, right, pEnt, i, bFlags[TEAM], flFlagsColor[TEAM]);

		}
	}
}

void visual::BoxEsp(int& left, float& top, int& right, float& bot, Color color) {

	i::Surface->DrawSetColor(color[0], color[1], color[2], color[3]);
	i::Surface->DrawOutlinedRect(left, top, right, bot);

	i::Surface->DrawSetColor(0, 0, 0, color[3]);
	i::Surface->DrawOutlinedRect(left - 1, top - 1, right + 1, bot + 1);
}

void visual::HealthEsp(int& left, float& top, int& right, float& bot, float& width, float& height, int& health, Color startColor, Color endColor, size_t& iEntIndex) {

	if (iHealth[iEntIndex] <= 0)
		iHealth[iEntIndex] = health;

	if (iHealth[iEntIndex] > health)
		iHealth[iEntIndex] -= (iHealth[iEntIndex] - health < 3 ? 1 : 3);

	float fDistance = bot - top;
	const float flFactor = min(iHealth[iEntIndex], 100) * 0.01f;

	i::Surface->DrawSetColor(0.f, 0.f, 0.f, (startColor[3] + endColor[3]) * 0.5);
	i::Surface->DrawOutlinedRect(left - 8, top, left - 4, bot + 1);

	i::Surface->DrawSetColor(startColor);
	i::Surface->DrawFilledRectFade(left - 7, bot - (fDistance * flFactor) - 5, left - 5, bot, startColor[3], 0, false);

	i::Surface->DrawSetColor(endColor);
	i::Surface->DrawFilledRectFade(left - 7, bot - (fDistance * flFactor) - 5, left - 5, bot, 0, endColor[3], false);

	i::Surface->DrawT(left - 7, bot - (fDistance * flFactor) - 8, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, std::to_string(iHealth[iEntIndex]).c_str());
}

void visual::NameEsp(int& left, float& top, int& right, float& bot, float& width, float& height, CBaseEntity* pEnt, Color color) {

	PlayerInfo_t info = { };

	if (!i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info))
		return;

	i::Surface->DrawT(left, top - 13, Color{ color[0], color[1], color[2], color[3] }, g::fonts::FlagESP, false, info.szName);
}

void visual::KevlarEsp(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt, Color color) {


}

void visual::AmmoEsp(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt, Color color) {

	int iEntIndex = pEnt->EntIndex();

	if (!pEnt->GetWeapon() || !pEnt->GetWeapon()->GetCSWpnData() || pEnt->GetWeapon()->IsKnife() || pEnt->GetWeapon()->IsGrenade())
		return;

	bAmmoEnabled[iEntIndex] = true;

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

void visual::BreakLCESP(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt )
{
	//if ( !lagcomp.IsBreakingLagcompensation( pEnt ) )
	//	return;

	i::Surface->DrawT( left, bot, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, false, "Breaking Lagcomp" );
}

void visual::WeaponEsp(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt, Color color) {

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

void visual::Flags(float& top, int& right, CBaseEntity* pEnt, size_t& iIndex, bool* bFlags, float flFlagsColor[5][4]) {

	int spacing = -2;
	if (bFlags[NAME]) {

		static PlayerInfo_t info = { };
		if (i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info)) {

			i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[NAME], g::fonts::FlagESP, false, info.szName);
			spacing += 10;
		}
	}

	if (bFlags[HEALTH]) {

		int iHealth = pEnt->GetHealth();
		const float percentage = iHealth / 100.f;

		i::Surface->DrawT(right + 2, top + spacing, Color((1.f - percentage) * 1.f, 1.f * percentage, 0.f), g::fonts::FlagESP, false, std::format(healthPrefix, iHealth).c_str());
		spacing += 10;
	}

	if (bFlags[ARMOR]) {

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[ARMOR], g::fonts::FlagESP, false, std::format(kevlarPrefix, pEnt->GetArmor()).c_str());
		spacing += 10;
	}

	CBaseCombatWeapon* pWeapon = pEnt->GetWeapon();
	if (pWeapon && bFlags[AMMO]) {

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[AMMO], g::fonts::FlagESP, false, std::format(ammoPrefix, pWeapon->GetAmmo(), pWeapon->GetAmmoReserve()).c_str());
		spacing += 10;
	}
	if (pWeapon && bFlags[WEAPON]) {

		std::string text = pWeapon->GetCSWpnData()->szWeaponName;
		text.erase(0, 7);

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[WEAPON], g::fonts::FlagESP, false, text.c_str());
		spacing += 10;
	}

	if (bFlags[MONEY]) {

		i::Surface->DrawT(right + 2, top + spacing, flFlagsColor[MONEY], g::fonts::FlagESP, false, std::format(moneyPrefix, pEnt->GetMoney()).c_str());

		spacing += 10;
	}

#if _DEBUG
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

		static auto something = [](int right, int top, int& spacing, const char* print) {

			i::Surface->DrawT(right + 2, top + spacing, Color(184, 203, 131, 255), g::fonts::FlagESP, false, print);
			spacing += 10;
		};

		something(right, top, spacing, std::to_string(pLog->pRecord.front().LayerData[0].flPlaybackRate * 10000000).c_str());
		something(right, top, spacing, std::to_string(pLog->pRecord.front().LayerData[1].flPlaybackRate * 10000000).c_str());
		something(right, top, spacing, std::to_string(pLog->pRecord.front().LayerData[2].flPlaybackRate * 10000000).c_str());
		something(right, top, spacing, std::to_string(pLog->pRecord.front().pLayers[ANIMATION_LAYER_MOVEMENT_MOVE].flPlaybackRate * 10000000).c_str());
		//something(right, top, spacing, std::to_string(anims.GetLocalCycleIncrement(pLog->pEntity, pLog->pRecord.front().LayerData[0].flPlaybackRate)).c_str());
		//something(right, top, spacing, std::to_string(anims.GetLocalCycleIncrement(pLog->pEntity, pLog->pRecord.front().LayerData[1].flPlaybackRate)).c_str());
		//something(right, top, spacing, std::to_string(anims.GetLocalCycleIncrement(pLog->pEntity, pLog->pRecord.front().LayerData[2].flPlaybackRate)).c_str());
		//something(right, top, spacing, std::to_string(anims.GetLocalCycleIncrement(pLog->pEntity, pLog->pRecord.front().pLayers[6].flPlaybackRate)).c_str());

	}
#endif
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