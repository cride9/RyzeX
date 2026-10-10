#include "ESP.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/EnemyAnimations.h"
#include "../Rage/Animations/ExperimentalResolverMath.h"
#include "../Rage/aimbot.h"
#include "../Rage/exploits.h"
#include "../../SDK/Menu/gui.h"
#include "../../SDK/RayTracer rebuilt/CRayTrace.h"
#include "../../SDK/InputSystem.h"
#include "../../../Dependecies/BASS/dll.h"
#include "../Misc/Playerlist.h"
#include "drawlist.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <map>
#include <cmath>

namespace {
	std::mutex damageIndicatorMutex;
	std::vector<visual::DamageIndicator> damageIndicators;
	struct HudLine { std::string text; Color color; };
	std::mutex hudMutex;
	std::vector<HudLine> hotkeyLines;
	std::vector<HudLine> spectatorLines;

	std::array<std::string, 3> ResolverFlagLines(const Lagcompensation::LagRecord_t& record) {
		const auto mode = resolver::GetMode(cfg::rage::iResolverMode);
		if (mode == resolver::Mode::Off)
			return { "OFF", "", "" };
		if (record.iResolverMode != static_cast<int>(mode))
			return { mode == resolver::Mode::Experimental ? "EXP WAIT" : "WAIT", "", "" };
		if (mode != resolver::Mode::Experimental)
			return { misc::GetMatrixName(record.iResolveSide), "", "" };

		std::array<std::string, 3> lines;
		const auto reason = static_cast<experimental::Reason>(record.iExperimentalReason);
		const bool hasSide = record.iResolveSide == LEFT || record.iResolveSide == RIGHT || record.iResolveSide == CENTER;
		const bool accepted = hasSide && (reason == experimental::Reason::Override
			|| (std::isfinite(record.flResolverConfidence) && record.flResolverConfidence > 0.f
				&& (reason == experimental::Reason::MovingLayers || reason == experimental::Reason::LastMove)));
		lines[0] = accepted ? "EXP " + misc::GetMatrixName(record.iResolveSide) : "EXP BASE";
		if (accepted && std::isfinite(record.flResolveDelta))
			lines[0] += std::format(" {:+.1f}deg", record.flResolveDelta);

		switch (reason) {
		case experimental::Reason::Warmup: lines[1] = "WARMUP"; break;
		case experimental::Reason::Ambiguous: lines[1] = "UNCERTAIN"; break;
		case experimental::Reason::MovingLayers: lines[1] = "LAYER"; break;
		case experimental::Reason::LastMove: lines[1] = "LASTMOVE"; break;
		case experimental::Reason::LbyChanged: lines[1] = "LBY UPDATE"; break;
		case experimental::Reason::Transition: lines[1] = "TRANSITION"; break;
		case experimental::Reason::Override: lines[1] = "OVERRIDE"; break;
		default: lines[1] = "INVALID"; break;
		}
		if (accepted && reason != experimental::Reason::Override) {
			const int confidence = static_cast<int>(std::round(std::clamp(record.flResolverConfidence, 0.f, 1.f) * 100.f));
			lines[1] += std::format(" C{}", confidence);
		}
		if (record.bBreakingLagcompensation)
			lines[2] = "LC BREAK";
		else if (record.iChoked > 1)
			lines[2] = std::format("CHOKE {}", record.iChoked);
		return lines;
	}
}

void SafepointDebug(CBaseEntity* pEnt) {

	Vector vecMins{}, vecMaxs{};
	float flRadius{};

	if (!g::pLocal)
		return;

	for (size_t i = 0; i < 3; i++)
	{
		Vector currentPoint = visual::traceEnds[0];

		Vector AISpot;
		if (i::DebugOverlay->ScreenPosition(currentPoint, AISpot))
			return;

		Vector LocalEyePos;
		if (i::DebugOverlay->ScreenPosition(g::vecEyePosition, LocalEyePos))
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

void visual::DrawWeaponInfo( )
{
	if ( !g::pLocal || !g::pLocal->IsAlive( ) || !g::pLocal->GetWeapon( ))
		return;
	
	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	int x, y = 0;
	i::EngineClient->GetScreenSize( x, y );

	Vector vecViewAngle;
	i::EngineClient->GetViewAngles(vecViewAngle);

	static float helperNumber = 0;
	static float vecOriginalValueX = x / 2 + x / 9.6f;
	static float vecOriginalValueY = y / 2 - y / 19.2f;

	static Vector vecBeforeViewAngle = Vector(0, 0, 0);
	static float flLengthBefore = 0;
	static float flHeightBefore = 0;

	Vector vecTransformedOrigin = Vector( 0, 0, 0 );
	if (i::DebugOverlay->ScreenPosition( g::pLocal->GetBonePosition( 32 ).value( ), vecTransformedOrigin ))
		return;

	Vector vecDifference = Vector(vecViewAngle.x - vecBeforeViewAngle.x, vecViewAngle.y - vecBeforeViewAngle.y, vecViewAngle.z - vecBeforeViewAngle.z);
	if (vecDifference.y != 0) {
		flLengthBefore += M::NormalizeYaw(vecDifference.y) * 5;
	}

	if (vecDifference.x != 0) {
		flHeightBefore -= M::NormalizeYaw(vecDifference.x) * 5;
	}

	if (flLengthBefore > 0) {
		flLengthBefore -= std::fabsf(vecOriginalValueX + flLengthBefore - vecOriginalValueX) / 20;
	}

	if (flLengthBefore < 0) {
		flLengthBefore += std::fabsf(vecOriginalValueX + flLengthBefore - vecOriginalValueX) / 20;
	}

	if (flHeightBefore > 0) {
		flHeightBefore -= std::fabsf(vecOriginalValueY + flHeightBefore - vecOriginalValueY) / 20;
	}

	if (flHeightBefore < 0) {
		flHeightBefore += std::fabsf(vecOriginalValueY + flHeightBefore - vecOriginalValueY) / 20;
	}
	
	helperNumber += i::GlobalVars->flAbsFrameTime;
	float vecCurrentValueX = vecOriginalValueX + flLengthBefore;
	float vecCurrentValueY = vecOriginalValueY - (std::sin(helperNumber * 2) * 30) + flHeightBefore;

	drawlist::AddLine(Vector2D(vecTransformedOrigin.x, vecTransformedOrigin.y), Vector2D(vecCurrentValueX, vecCurrentValueY), Color(7, 0, 13, 255));
	drawlist::AddRect(Vector2D(vecCurrentValueX, vecCurrentValueY - (y / 19.2f) / 2), Vector2D(vecCurrentValueX + x / 9.6f, vecCurrentValueY + (y / 19.2f) / 2), DRAWFLAGS_FILLED, Color(7, 0, 13, 255));
	
	CCSWeaponInfo* pData = pWeapon->GetCSWpnData();

	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::string text = "";
	wchar_t* localizeName = i::Localize->Find(pData->szHudName);
	if (localizeName)
		text = converter.to_bytes(localizeName);

	drawlist::AddText(Vector2D(vecCurrentValueX + (x / 9.6f) / 10, vecCurrentValueY - (y / 19.2f) / 2), 0xA1, true, Color(255, 255, 255, 255), GetWeaponChar(text));
	drawlist::AddText(Vector2D(vecCurrentValueX + (x / 9.6f) / 10 + 20, vecCurrentValueY - (y / 19.2f) / 2), g::fonts::FlagESP, false, Color(255, 255, 255, 255), std::format("Speed: {} - {}", pData->flMaxSpeed[0], pData->flMaxSpeed[1]));
	drawlist::AddText(Vector2D(vecCurrentValueX + (x / 9.6f) / 10 + 20, vecCurrentValueY - (y / 19.2f) / 2 + 12), g::fonts::FlagESP, false, Color(255, 255, 255, 255), std::format("Damage: {}", pData->iDamage));
	drawlist::AddText(Vector2D(vecCurrentValueX + (x / 9.6f) / 10 + 20, vecCurrentValueY - (y / 19.2f) / 2 + 24), g::fonts::FlagESP, false, Color(255, 255, 255, 255), std::format("Ammo: {}", pWeapon->GetAmmo()));
	vecBeforeViewAngle = vecViewAngle;
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

	//DrawWeaponInfo( );
	WorldCrosshair();

	for (size_t i = 1; i <= i::GlobalVars->nMaxClients && i < playerList::arrPlayers.size(); i++) {

		if (playerList::arrPlayers[i].iIndex != i)
			continue;

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEnt || !g::pLocal) {
			iHealth[i] = -1;
			vecDormatPosition[i] = Vector(0, 0, 0);
			continue;
		}

		if (!pEnt->IsAlive()) {
			iHealth[i] = 100;
			vecDormatPosition[i] = Vector(0, 0, 0);
			continue;
		}

		if (!pEnt->GetHealth())
			continue;

		if (pEnt->GetHealth() > iHealth[i])
			iHealth[i] = pEnt->GetHealth();

		//SafepointDebug(pEnt);
		//HitboxVisualization();

		Vector vecAbsOrigin = Vector(0, 0, 0);
		if (pEnt->IsDormant())
			vecAbsOrigin = vecDormatPosition[i].IsZero() ? Vector(0, 0, 0) : vecDormatPosition[i];

		if (vecAbsOrigin.IsZero() && pEnt->IsDormant())
			continue;
		
		if (!pEnt->IsDormant()) {
			vecAbsOrigin = pEnt->GetAbsOrigin();
			vecDormatPosition[i] = vecAbsOrigin;
		}

		if (pEnt->IsEnemy(g::pLocal))
			OutOfFov(pEnt);

		Vector bot;
		if (i::DebugOverlay->ScreenPosition(vecAbsOrigin - Vector{ 0.f, 0.f, 9.f }, bot))
			continue;

		Vector top;
		if (i::DebugOverlay->ScreenPosition(vecAbsOrigin + Vector(0, 0, pEnt->vecMaxs().z + 5), top))
			continue;

		float h = bot.y - top.y;
		float w = h * 0.25f;

		int left = static_cast<int>(top.x - w);
		int right = static_cast<int>(top.x + w);
		bAmmoEnabled[i] = false;

		if (pEnt->IsEnemy(g::pLocal)) { // Enemy

			if (!bEnable[ENEMY])
				continue;

			if (bSkeleton[ENEMY]) SkeletonEsp(pEnt, flSkeletonColor[ENEMY]);
			if (bName[ENEMY]) NameEsp(left, top.y, right, bot.y, w, h, pEnt, pEnt->IsDormant() ? vecDormantColor : Color(flNameColor[ENEMY]));
			if (bBox[ENEMY]) BoxEsp(left, top.y, right, bot.y, pEnt->IsDormant() ? vecDormantColor : Color(flBoxColor[ENEMY]));
			if (bHealth[ENEMY]) HealthEsp(left, top.y, right, bot.y, w, h, pEnt->GetHealth(), pEnt->IsDormant() ? vecDormantColor : Color(flHealthColorStart[ENEMY]), pEnt->IsDormant() ? vecDormantColor : Color(flHealthColorEnd[ENEMY]), i);
			if (bArmor[ENEMY]) KevlarEsp(left, top.y, right, bot.y, pEnt, pEnt->IsDormant() ? vecDormantColor : Color(flArmorColor[ENEMY]));
			if (bAmmo[ENEMY]) AmmoEsp(left, top.y, right, bot.y, pEnt, pEnt->IsDormant() ? vecDormantColor : Color(flAmmoColor[ENEMY]));
			if (bWeapon[ENEMY]) WeaponEsp(left, top.y, right, bot.y, pEnt, pEnt->IsDormant() ? vecDormantColor : Color(flWeaponColor[ENEMY]));
			Flags(top.y, right, pEnt, i, bFlags[ENEMY], flFlagsColor[ENEMY], pEnt->IsDormant());
		}
		else {

			if (pEnt == g::pLocal && i::Input->bCameraInThirdPerson) { // Local

				if (!bEnable[LOCAL])
					continue;

				if (bSkeleton[LOCAL]) SkeletonEsp(pEnt, flSkeletonColor[LOCAL]);
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
			if (bSkeleton[TEAM]) SkeletonEsp(pEnt, flSkeletonColor[TEAM]);
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
	i::Surface->DrawFilledRectFade(left - 7, bot - (fDistance * flFactor), left - 5, bot, startColor[3], 0, false);

	i::Surface->DrawSetColor(endColor);
	i::Surface->DrawFilledRectFade(left - 7, bot - (fDistance * flFactor), left - 5, bot, 0, endColor[3], false);

	if (health < 95) {
		if (startColor != Color(0.5f, 0.5f, 0.5f, 0.5f))
			i::Surface->DrawT(left - 7, bot - (fDistance * flFactor) - 4, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, std::to_string(iHealth[iEntIndex]).c_str());
		else
			i::Surface->DrawT(left - 7, bot - (fDistance * flFactor) - 4, startColor, g::fonts::FlagESP, true, std::to_string(iHealth[iEntIndex]).c_str());
	}
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
	i::Surface->DrawOutlinedRect(left + 1, bot + 5, left + (flDifference * flFactor) - 1, bot + 7);

	i::Surface->DrawT(left + (flDifference * flFactor), bot + 6, pEnt->IsDormant() ? vecDormantColor : Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, std::to_string(pWeapon->GetAmmo()).c_str());
}

void visual::BreakLCESP(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt )
{
	
}

void visual::WeaponEsp(int& left, float& top, int& right, float& bot, CBaseEntity* pEnt, Color color) {

	if (!pEnt->GetWeapon())
		return;

	auto pWeaponInfo = pEnt->GetWeapon()->GetCSWpnData();

	if (!pWeaponInfo)
		return;

	std::string text = "";
	wchar_t* localizeName = i::Localize->Find(pWeaponInfo->szHudName);
	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	if (localizeName)
		text = converter.to_bytes(localizeName);

	if (bAmmoEnabled[pEnt->EntIndex()])
		bot += 10;

	i::Surface->DrawT(left, bot, Color(color), g::fonts::FlagESP, false, text.c_str());
}

void visual::Flags(float& top, int& right, CBaseEntity* pEnt, size_t& iIndex, bool* bFlags, float flFlagsColor[5][4], bool bDormant) {

	int spacing = -2;
	if (bFlags[NAME]) {

		static PlayerInfo_t info = { };
		if (i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info)) {

			i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[NAME], g::fonts::FlagESP, false, info.szName);
			spacing += 10;
		}
	}

	if (bFlags[HEALTH]) {

		int iHealth = pEnt->GetHealth();
		const float percentage = iHealth / 100.f;

		i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : Color((1.f - percentage) * 1.f, 1.f * percentage, 0.f), g::fonts::FlagESP, false, std::format(healthPrefix, iHealth).c_str());
		spacing += 10;
	}

	if (bFlags[ARMOR]) {

		if (pEnt->HasHeavyArmor()) {
			i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[ARMOR], g::fonts::FlagESP, false, "HK+");
			spacing += 10;
		}
		else if (pEnt->GetArmor() > 0) {
			i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[ARMOR], g::fonts::FlagESP, false, "HK");
			spacing += 10;
		}

	}
	CBaseCombatWeapon* pWeapon = pEnt->GetWeapon();
	if (pWeapon && bFlags[AMMO]) {

		i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[AMMO], g::fonts::FlagESP, false, std::format(ammoPrefix, pWeapon->GetAmmo(), pWeapon->GetAmmoReserve()).c_str());
		spacing += 10;
	}
	if (pWeapon && bFlags[WEAPON]) {

		std::string text = "";
		wchar_t* localizeName = i::Localize->Find(pWeapon->GetCSWpnData()->szHudName);
		static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		if (localizeName)
			text = converter.to_bytes(localizeName);

		i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[WEAPON], g::fonts::FlagESP, false, text.c_str());
		spacing += 10;
	}

	if (bFlags[MONEY]) {

		i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[MONEY], g::fonts::FlagESP, false, std::format(moneyPrefix, pEnt->GetMoney()).c_str());


		spacing += 10;
		//i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[MONEY], g::fonts::FlagESP, false, std::format("{} brueforce", anims.missedShots[pEnt->EntIndex()]).c_str());
		//spacing += 10;
	}

	if (bFlags[RESOLVER]) {

		auto* pLog = &lagcomp.GetLog(pEnt->EntIndex());
		if (pLog && !pLog->pRecord.empty() && pLog->iLastValid <= pLog->pRecord.size()) {
			for (const auto& line : ResolverFlagLines(pLog->pRecord.front())) {
				if (line.empty()) continue;
				i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[RESOLVER], g::fonts::FlagESP, false, line.c_str());
				spacing += 10;
			}
		}
	}

	if (bFlags[FAKEDUCK]) {

		if (pEnt->IsFakeducking()) {
			i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[RESOLVER], g::fonts::FlagESP, false, "FD");
			spacing += 10;
		}
	}

	if (bFlags[AIMBOT]) {

		auto* pLog = &lagcomp.GetLog(pEnt->EntIndex());
		if (pLog) {
			i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[AIMBOT], g::fonts::FlagESP, false, misc::GetHitgroupName(aimbot.GetAimbotData().iHitGroup).c_str());
			spacing += 10;
		}
	}
	/*
	RESOLVER,
	FAKEDUCK,
	BOMB,
	*/
	/*if (true) {

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEnt->EntIndex());

		if (pLog->pEntity && pLog->pRecord.size() >= 2) {
			if (lagcomp.IsBreakingLagcompensation(&pLog->pRecord.front())) {
			
				i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[MONEY], g::fonts::FlagESP, false, "LC");
				spacing += 10;
			}
		}
	}*/

#if _DEBUG
	if (Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEnt->EntIndex()); pLog && pLog->pEntity && !pLog->pRecord.empty()) {

		
		static auto something = [](int right, int top, int& spacing, std::string print) {

			i::Surface->DrawT(right + 2, (top + spacing) - 80, RYZEXCOLOR, g::fonts::DebugFont, false, print.c_str());
			spacing += 10;
		};

		//something(right, top, spacing, std::format("MOVE_YAW: {}", pEnt->GetPoseParameter()[MOVE_YAW]).c_str());

		//const unsigned int iLastValid = pLog->iLastValid;
		//if (iLastValid >= pLog->pRecord.size())
		//	return;

		//auto pRecord = &pLog->pRecord.front();
		////something(right, top, spacing, std::format("{}", *(float*)(*(DWORD*)((DWORD)(*(void**)((DWORD)(pEnt)+0x24))+0x8) + 0x24)).c_str());
		////something(right, top, spacing, std::format("{}", *(float*)(*(DWORD*)((DWORD)(*(void**)((DWORD)(pEnt)+0x24))+0x44) + 0x24)).c_str());

		////something(right, top, spacing, std::format("Guess: {}", pRecord->flGuessedYaw).c_str());
		//// from the server.
		//auto flFromServerPlaybackrate = pRecord->arrLayers[6].flPlaybackRate;

		//// resolver calculations.
		//const float fCenterPlaybackrate = pRecord->LayerData[CENTER].flPlaybackRate;
		//const float fRightPlaybackrate = pRecord->LayerData[RIGHT].flPlaybackRate;
		//const float fLeftPlaybackrate = pRecord->LayerData[LEFT].flPlaybackRate;

		//// differences.
		//const float fDifferenceCenterPlaybackrate = fabs(flFromServerPlaybackrate - fCenterPlaybackrate);
		//const float fDifferenceRightPlaybackrate = fabs(flFromServerPlaybackrate - fRightPlaybackrate);
		//const float fDifferenceLeftPlaybackrate = fabs(flFromServerPlaybackrate - fLeftPlaybackrate);

		//something(right, top, spacing, "[Layer 6]");
		//something(right, top, spacing, std::format("Server: {}", flFromServerPlaybackrate).c_str());
		//something(right, top, spacing, std::format("Velocity: {}", pRecord->vecVelocity.Length2D()).c_str());
		//something(right, top, spacing, std::format("Right: {}", fRightPlaybackrate).c_str());
		//something(right, top, spacing, std::format("Center: {}", fCenterPlaybackrate).c_str());
		//something(right, top, spacing, std::format("Server: {}", flFromServerPlaybackrate).c_str());

		//something(right, top, spacing, std::format("RightDiff: {}", fDifferenceRightPlaybackrate).c_str());
		//something(right, top, spacing, std::format("LeftDiff: {}", fDifferenceLeftPlaybackrate).c_str());
		//something(right, top, spacing, std::format("CenterDiff: {}", fDifferenceCenterPlaybackrate).c_str());
	}
#endif
}

void visual::Glow(CBaseEntity* pLocal)
{
	for (int i = 0; i < i::GlowObjectManager->vecGlowObjectDefinitions.Count(); i++)
	{
		IGlowObjectManager::GlowObject_t* hGlowObject = &i::GlowObjectManager->vecGlowObjectDefinitions[i];

		// is current object not used
		if (hGlowObject->IsEmpty())
			continue;

		// get current entity from object handle
		CBaseEntity* pEntity = hGlowObject->pEntity;

		if (pEntity == nullptr)
			continue;

		// set bloom state
		hGlowObject->bFullBloomRender = false;

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

			if (pEntity->IsEnemy(g::pLocal) && cfg::visual::bGlow[ENEMY])
				hGlowObject->Set(Color(cfg::visual::flGlowColor[ENEMY]));

			else if (!pEntity->IsEnemy(g::pLocal) && pEntity != g::pLocal && cfg::visual::bGlow[TEAM])
				hGlowObject->Set(Color(cfg::visual::flGlowColor[TEAM]));

			else if (pEntity == pLocal && cfg::visual::bGlow[LOCAL])
				hGlowObject->Set(Color(cfg::visual::flGlowColor[LOCAL]));

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
	if (!cfg::misc::bWorldCrosshair || !g::pLocal || !i::EngineClient->IsInGame()) {
		std::fill(std::begin(vecWorldCrosshair), std::end(vecWorldCrosshair), Vector{});
		return;
	}

	for (size_t i = 0; i < 5; i++) {

		if (vecWorldCrosshair[i] == Vector(0, 0, 0))
			continue;

		const float age = i::GlobalVars->flCurrentTime - flWorldCrosshairLength[i];
		if (age < 0.f || age >= 3.f) {
			vecWorldCrosshair[i] = Vector(0, 0, 0);
			continue;
		}

		Vector vecScreenPoint;
		if (i::DebugOverlay->ScreenPosition(vecWorldCrosshair[i], vecScreenPoint))
			continue;

		/* Set color */
		Color color(cfg::misc::flWorldCrosshairColor);
		color[3] = static_cast<std::uint8_t>(color[3] * (1.f - age / 3.f));
		i::Surface->DrawSetColor(color);

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

void visual::SkeletonEsp(CBaseEntity* pEntity, Color color) {

	if (pEntity->IsDormant())
		return;

	const Model_t* model = pEntity->GetModel();
	if (!model)
		return;

	studiohdr_t* pStudioHdr = i::ModelInfo->GetStudioModel(model);
	if (!pStudioHdr)
		return;

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pLog || pLog->pRecord.empty() && g::pLocal != pEntity)
		return;

	bool pLocal = g::pLocal == pEntity;
	matrix3x4_t* pBoneToWorld = pLocal ? localAnim->GetRealMatrix().data() : pLog->pCachedMatrix.data();

	for (int j = 0; j < pStudioHdr->nBones && j < MAXSTUDIOBONES; j++) {

		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (!pBone || !(pBone->iFlags & BONE_USED_BY_HITBOX) || pBone->iParent < 0 || pBone->iParent >= MAXSTUDIOBONES)
			continue;

		Vector vecChild = pBoneToWorld[j].GetOrigin();
		Vector vecParent = pBoneToWorld[pBone->iParent].GetOrigin();

		constexpr const static unsigned int const iChestBone = 6;
		Vector vecUpperDirection = pBoneToWorld[iChestBone + 1].GetOrigin() - pBoneToWorld[iChestBone].GetOrigin();
		Vector vecBreastBone = pBoneToWorld[iChestBone].GetOrigin() + vecUpperDirection / 2;

		Vector vecDeltaChild = vecChild - vecBreastBone;
		Vector vecDeltaParent = vecParent - vecBreastBone;

		if ((vecDeltaParent.Length() < 9 && vecDeltaChild.Length() < 9))
			vecParent = vecBreastBone;

		if (j == iChestBone - 1)
			vecChild = vecBreastBone;

		if (fabsf(vecDeltaChild.z) < 5 && (vecDeltaParent.Length() < 5 && vecDeltaChild.Length() < 5) || j == iChestBone)
			continue;

		Vector vecScreenParent, vecScreenChild;
		if (i::DebugOverlay->ScreenPosition(vecParent, vecScreenParent) ||
			i::DebugOverlay->ScreenPosition(vecChild, vecScreenChild))
			continue;

		i::Surface->DrawSetColor(Color(color));
		i::Surface->DrawLine(vecScreenParent[0], vecScreenParent[1], vecScreenChild[0], vecScreenChild[1]);
	}
}

// thx @94
namespace {
	float WorldDistance(const Vector& origin) {
		return g::pLocal ? (origin - g::pLocal->GetAbsOrigin()).Length() * .0254f : 0.f;
	}
	Color WorldColor(float* rgba, float distance) {
		Color color(rgba);
		const int limit = std::clamp(cfg::misc::iWorldEspMaxDistance, 0, 200);
		if (limit > 0)
			color[3] = static_cast<std::uint8_t>(color[3] * std::clamp((limit - distance) / (limit * .15f), 0.f, 1.f));
		return color;
	}
	void GrenadeTimer(const Vector& screen, Color color, float remaining, float duration) {
		if (!std::isfinite(remaining) || !std::isfinite(duration) || remaining <= 0.f || duration <= 0.f)
			return;
		remaining = (std::min)(remaining, duration);
		i::Surface->DrawT(screen.x, screen.y + 14, color, g::fonts::FlagESP, true, std::format("{:.1f}s", remaining).c_str());
		i::Surface->DrawSetColor(Color(0, 0, 0, color[3] / 2));
		i::Surface->DrawFilledRect(screen.x - 30, screen.y + 27, screen.x + 30, screen.y + 30);
		i::Surface->DrawSetColor(color);
		i::Surface->DrawFilledRect(screen.x - 30, screen.y + 27, screen.x - 30 + 60 * std::clamp(remaining / duration, 0.f, 1.f), screen.y + 30);
	}
	void DrawWeaponTypeIcon(const Vector& screen, Color color, int type) {
		// Keep an icon for weapons without a glyph in the game's icon font.
		const int x = static_cast<int>(screen.x), y = static_cast<int>(screen.y) - 11;
		i::Surface->DrawSetColor(color);
		i::Surface->DrawFilledRect(x - 8, y - 7, x + 6, y - 3);
		i::Surface->DrawFilledRect(x + 6, y - 6, x + (type == WEAPONTYPE_PISTOL ? 11 : 18), y - 4);
		i::Surface->DrawFilledRect(x - 6, y - 3, x - 2, y + 2);
		if (type != WEAPONTYPE_PISTOL) {
			i::Surface->DrawFilledRect(x - 18, y - 5, x - 8, y - 1);
			i::Surface->DrawFilledRect(x, y - 3, x + 4, y + 2);
		}
		if (type == 5) // sniper scope
			i::Surface->DrawFilledRect(x - 3, y - 11, x + 6, y - 8);
	}
}

void visual::WorldEsp() {

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL || !g::pLocal || !i::EngineClient->IsConnected() || !i::EngineClient->IsInGame())
		return;

	if (!cfg::misc::bDroppedWeaponESP && !cfg::misc::bProjectileESP)
		return;
	const int highestEntityIndex = i::EntityList->GetHighestEntityIndex();
	for (int i = 1; i <= highestEntityIndex; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || pEntity->IsPlayer() || pEntity->IsDormant())
			continue;
		const float distance = WorldDistance(pEntity->GetAbsOrigin());
		const int limit = std::clamp(cfg::misc::iWorldEspMaxDistance, 0, 200);
		if (limit > 0 && distance >= limit)
			continue;

		// change the entity to a weapon
		CBaseCombatWeapon* pWeapon = reinterpret_cast<CBaseCombatWeapon*>(pEntity);

		Vector vecScreenPosition;
		if (i::DebugOverlay->ScreenPosition(pWeapon->GetAbsOrigin(), vecScreenPosition))
			continue;

		if (!pWeapon->GetClientClass())
			continue;

		if (pWeapon->IsWeapon() && !i::EntityList->GetClientEntityFromHandle(pEntity->GetOwnerEntityHandle()))
			DroppedWeapons(pWeapon, vecScreenPosition);
		const EClassIndex iClientID = pWeapon->GetClientClass()->nClassID;

		if (!cfg::misc::bProjectileESP)
			continue;

		using enum EClassIndex;
		const auto projectileColor = WorldColor(cfg::misc::flProjectileESP, distance);
		if (iClientID == CDecoyProjectile)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Decoy");

		else if (iClientID == CMolotovProjectile)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Molotov");

		else if (iClientID == CSmokeGrenadeProjectile) {
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Smoke");
			const int begin = reinterpret_cast<::CSmokeGrenade*>(pEntity)->GetEffectTickBegin();
			if (cfg::misc::bGrenadeTimers && begin > 0)
				GrenadeTimer(vecScreenPosition, projectileColor, 18.f - (i::GlobalVars->flCurrentTime - TICKS_TO_TIME(begin)), 18.f);
		}

		else if (iClientID == CBaseCSGrenadeProjectile) {

			const Model_t* pModel = pWeapon->GetModel();

			if (pModel) {

				std::string szName = pModel->szName;

				if (szName.find("flashbang") != std::string::npos)
					i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Flashbang");
				else if (szName.find("fraggrenade") != std::string::npos)
					i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Grenade");
			}
		}

		else if (iClientID == CInferno) {
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Fire");
			const int begin = reinterpret_cast<::CInferno*>(pEntity)->GetEffectTickBegin();
			static auto* lifetime = i::ConVar->FindVar("inferno_flame_lifetime");
			const float seconds = lifetime && std::isfinite(lifetime->GetFloat()) ? std::clamp(lifetime->GetFloat(), .1f, 60.f) : 7.f;
			if (cfg::misc::bGrenadeTimers && begin > 0)
				GrenadeTimer(vecScreenPosition, projectileColor, seconds - (i::GlobalVars->flCurrentTime - TICKS_TO_TIME(begin)), seconds);
		}
		
		else if (iClientID == CPlantedC4)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, projectileColor, g::fonts::FlagESP, true, "Bomb");
		if (cfg::misc::bWorldEspDistance && (iClientID == CDecoyProjectile || iClientID == CMolotovProjectile ||
			iClientID == CSmokeGrenadeProjectile || iClientID == CBaseCSGrenadeProjectile || iClientID == CInferno || iClientID == CPlantedC4))
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y + (cfg::misc::bGrenadeTimers ? 34 : 14), projectileColor,
				g::fonts::FlagESP, true, std::format("{:.0f}m", distance).c_str());
	}
}

void visual::DroppedWeapons(CBaseCombatWeapon* pWeapon, Vector& vecScreenPosition) {

	if (!cfg::misc::bDroppedWeaponESP)
		return;
	const float distance = WorldDistance(pWeapon->GetAbsOrigin());
	const auto color = WorldColor(cfg::misc::flDroppedWeaponESP, distance);

	const EClassIndex iClientID = pWeapon->GetClientClass()->nClassID;

	using enum EClassIndex;
	if (iClientID == CC4) {
		i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, color, g::fonts::FlagESP, true, "Bomb");
		if (cfg::misc::bWorldEspDistance)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y + 14, color, g::fonts::FlagESP, true, std::format("{:.0f}m", distance).c_str());
		return;
	}

	if (pWeapon->GetItemDefinitionIndex() == WEAPON_TASER && pWeapon->GetAmmo() == 0)
		return;

	CCSWeaponInfo* pData = pWeapon->GetCSWpnData();

	if (!pData)
		return;

	if (pData->nWeaponType < WEAPONTYPE_PISTOL || pData->nWeaponType > WEAPONTYPE_MACHINEGUN)
		return;

	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	wchar_t* pName = pData->szHudName ? i::Localize->Find(pData->szHudName) : nullptr;
	std::string name = "";
	if (pName)
		name = converter.to_bytes(pName);

	if (name.empty() && pData->szWeaponName) {
		name = pData->szWeaponName;
		if (name.starts_with("weapon_"))
			name.erase(0, 7);
	}
	const std::string icon = GetWeaponChar(name);
	std::transform(name.begin(), name.end(), name.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});

	i::Surface->DrawT(vecScreenPosition.x + 1, vecScreenPosition.y + 2, color, g::fonts::FlagESP, true, name.c_str());
	if (cfg::misc::bDroppedWeaponIcon) {
		if (icon != "<")
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y - 18, color, 0xA1, true, icon.c_str());
		else
			DrawWeaponTypeIcon(vecScreenPosition, color, pData->nWeaponType);
	}
	std::string details;
	if (cfg::misc::bDroppedWeaponAmmo)
		details = std::format("{} / {}", (std::max)(0, pWeapon->GetAmmo()), (std::max)(0, pWeapon->GetAmmoReserve()));
	if (cfg::misc::bWorldEspDistance)
		details += std::format("{}{:0.0f}m", details.empty() ? "" : "  ", distance);
	if (!details.empty())
		i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y + 15, color, g::fonts::FlagESP, true, details.c_str());
}

void visual::CoolHackKeyBindList() {
	std::vector<HudLine> binds, spectators;
	if (i::EngineClient->IsInGame() && g::pLocal) {
		if (cfg::misc::bKeyBindList) {
			struct Bind { const char* name; int key; bool enabled; int option; };
			const Bind entries[] = {
				{"Aimbot", cfg::rage::iAimbotKey, cfg::rage::bEnable, 0},
				{"Doubletap", cfg::rage::iDoubletapKey, cfg::rage::bDoubletap, 1},
				{"Hide shots", cfg::rage::iHideShotKey, cfg::rage::bHideshot, 1},
				{"Break LC", cfg::antiaim::iLCKey, cfg::antiaim::bBreakLagcompensation, 1},
				{"Force body", cfg::rage::iForceBaimKey, cfg::rage::bForceBaim, 2},
				{"Damage override", cfg::rage::iOverrideBind, true, 3},
				{"Slow walk", cfg::antiaim::iFakeWalkKey, cfg::antiaim::bFakeWalk, 4},
				{"Fake duck", cfg::antiaim::iFakeDuckKey, cfg::antiaim::bFakeDuck, 5},
				{"Auto peek", cfg::antiaim::iAutoPeek, cfg::antiaim::bAutoPeek, 6},
				{"Thirdperson", cfg::misc::iThirdPersonKey, cfg::misc::bThirdPerson, 7},
				{"Blockbot", cfg::misc::iBlockbotKey, cfg::misc::bBlockbot, 8},
				{"Auto nade", cfg::rage::iAutoNadeBind, cfg::rage::bAutoNade, 10},
				{"Manual right", cfg::antiaim::iManualAAR, cfg::antiaim::bManualAA, 11},
				{"Manual left", cfg::antiaim::iManualAAL, cfg::antiaim::bManualAA, 11},
				{"Radio mute", cfg::misc::iRadioMuteHotKey, cfg::misc::bEnableRadio, 12}
			};
			for (const auto& entry : entries) {
				if (entry.enabled && cfg::misc::iKeyBindList[entry.option] && IPT::HandleInput(entry.key)) {
					const int mode = cfg::m_iKeyStates[entry.key];
					const char* modeName = mode == TOGGLE ? "toggle" : mode == ALWAYS_ON ? "always" : "hold";
					binds.push_back({std::format("{} [{}]", entry.name, modeName), Color(238, 238, 238, 255)});
				}
			}
			if (auto* channel = i::EngineClient->GetNetChannelInfo(); channel && cfg::misc::iKeyBindList[9]) {
				const int ping = static_cast<int>((channel->GetLatency(FLOW_OUTGOING) + channel->GetLatency(FLOW_INCOMING)) * 1000.f);
				binds.push_back({std::format("Ping: {} ms", (std::max)(0, ping)), Color(238, 238, 238, 255)});
			}
		}
		if (cfg::misc::bSpectatorList) {
			auto* watched = g::pLocal->IsAlive() ? g::pLocal :
				static_cast<CBaseEntity*>(i::EntityList->GetClientEntityFromHandle(g::pLocal->GetObserverTarget()));
			for (int index = 1; watched && index <= (std::min)(64, i::GlobalVars->nMaxClients); ++index) {
				auto* player = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(index));
				if (!player || player == g::pLocal || player->IsAlive() || player->IsDormant())
					continue;
				if (i::EntityList->GetClientEntityFromHandle(player->GetObserverTarget()) != watched)
					continue;
				PlayerInfo_t info{};
				if (i::EngineClient->GetPlayerInfo(index, &info))
					spectators.push_back({std::string(info.szName, strnlen_s(info.szName, sizeof(info.szName))), Color(238, 238, 238, 255)});
			}
		}
	}
	std::lock_guard<std::mutex> lock(hudMutex);
	hotkeyLines = std::move(binds);
	spectatorLines = std::move(spectators);
}

namespace {
	void DrawHudWindow(const char* name, bool enabled, const std::vector<HudLine>& rows, float* position) {
		if (!enabled || (rows.empty() && !menu::open))
			return;
		const auto display = ImGui::GetIO().DisplaySize;
		if (display.x <= 0.f || display.y <= 0.f)
			return;
		const float scale = std::isfinite(cfg::misc::flHudScale) ? std::clamp(cfg::misc::flHudScale, .75f, 2.f) : 1.f;
		for (int axis = 0; axis < 2; ++axis)
			position[axis] = std::isfinite(position[axis]) ? std::clamp(position[axis], 0.f, 1.f) : .02f;
		struct Position { float x = -1.f, y = -1.f; ImVec2 display{}; };
		static std::map<std::string, Position> savedPositions;
		auto& last = savedPositions[name];
		if (position[0] != last.x || position[1] != last.y || display.x != last.display.x || display.y != last.display.y)
			ImGui::SetNextWindowPos(ImVec2(position[0] * display.x, position[1] * display.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2((std::min)(220.f * scale, display.x), 0.f), ImGuiCond_Always);
		ImGui::SetNextWindowSizeConstraints(ImVec2(1.f, 1.f), display);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f * scale, 6.f * scale));
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
		if (!menu::open)
			flags |= ImGuiWindowFlags_NoInputs;
		if (ImGui::Begin(name, nullptr, flags)) {
			ImGui::SetWindowFontScale(scale);
			if (rows.empty())
				ImGui::TextUnformatted("None active");
			for (const auto& row : rows) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(row.color[0] / 255.f, row.color[1] / 255.f, row.color[2] / 255.f, row.color[3] / 255.f));
				ImGui::TextUnformatted(row.text.c_str());
				ImGui::PopStyleColor();
			}
		}
		const auto size = ImGui::GetWindowSize();
		auto pos = ImGui::GetWindowPos();
		pos.x = std::clamp(pos.x, 0.f, (std::max)(0.f, display.x - size.x));
		pos.y = std::clamp(pos.y, 0.f, (std::max)(0.f, display.y - size.y));
		ImGui::SetWindowPos(pos);
		position[0] = last.x = pos.x / display.x;
		position[1] = last.y = pos.y / display.y;
		last.display = display;
		ImGui::End();
		ImGui::PopStyleVar(2);
	}
}

void visual::DrawMiscHud() {
	std::vector<HudLine> binds, spectators;
	{
		std::lock_guard<std::mutex> lock(hudMutex);
		binds = hotkeyLines;
		spectators = spectatorLines;
	}
	DrawHudWindow("Hotkeys", cfg::misc::bKeyBindList, binds, cfg::misc::flKeybindPosition);
	DrawHudWindow("Spectators", cfg::misc::bSpectatorList, spectators, cfg::misc::flSpectatorPosition);
}

void visual::DrawRadioInformation() {
	if (!cfg::misc::bEnableRadio)
		return;
	int width, height;
	i::EngineClient->GetScreenSize(width, height);
	const auto status = misc::GetRadioStatus();
	const auto metadata = BASS::GetMetadata();
	i::Surface->DrawT(10, height / 2 - 40, RYZEXCOLOR, g::fonts::FlagESP, false, ("Radio: " + status).c_str());
	if (!metadata.empty())
		i::Surface->DrawT(10, height / 2 - 24, Color(238, 238, 238, 255), g::fonts::FlagESP, false, metadata.c_str());
}

void visual::OutOfFov(CBaseEntity* pEntity) {

	if (g::bUpdatingSkins || i::ClientState->iDeltaTick < 0)
		return;

	auto pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pLog || pLog->pRecord.empty() || !cfg::misc::bOOF || g::bUpdatingSkins || i::ClientState->iDeltaTick < 0)
		return;

	Vector vecWorldPosition;
	if (pEntity->IsDormant())
		vecWorldPosition = vecDormatPosition[pEntity->EntIndex()];
	else
		vecWorldPosition = pEntity->GetVecOrigin();

	Vector& vecEyePosition = g::vecEyePosition;

	Vector vecScreenPosition;
	if (!i::DebugOverlay->ScreenPosition(vecWorldPosition, vecScreenPosition))
		return;

	Vector vecViewAngles;
	std::pair<int, int> iScreenSize;

	i::EngineClient->GetViewAngles(vecViewAngles);
	i::EngineClient->GetScreenSize(iScreenSize.first, iScreenSize.second);

	const Vector2D vecScreenCenter = Vector2D(iScreenSize.first * .5f, iScreenSize.second * .5f);

	const float flRotation = vecViewAngles.y - M::CalcAngle(vecEyePosition, vecWorldPosition).y - 90;
	const float flAngleYawRad = M_DEG2RAD(flRotation);

	int iRadius = cfg::misc::iOOFDistance;
	int iSize = cfg::misc::iOOFSize;

	const auto flNewPointX = vecScreenCenter.x + ((((iScreenSize.first - (iSize * 3)) * .5f) * (iRadius * 0.01f)) * cos(flAngleYawRad)) + (int)(6.0f * (((float)iSize - 4.f) / 16.0f));
	const auto flNewPointY = vecScreenCenter.y + ((((iScreenSize.second - (iSize * 3)) * .5f) * (iRadius * 0.01f)) * sin(flAngleYawRad));

	std::array< Vector2D, 3 >arrPoints{ 
		Vector2D(flNewPointX - iSize, flNewPointY - iSize),
		Vector2D(flNewPointX + iSize, flNewPointY),
		Vector2D(flNewPointX - iSize, flNewPointY + iSize) 
	};

	const auto vecPointsCenter = (arrPoints.at(0) + arrPoints.at(1) + arrPoints.at(2)) / 3;
	for (auto& currentPoint : arrPoints) {

		currentPoint -= vecPointsCenter;

		const float flTempX = currentPoint.x;
		const float flTempY = currentPoint.y;

		const float theta = M_DEG2RAD(flRotation);
		const float c = cos(theta);
		const float s = sin(theta);

		currentPoint.x = flTempX * c - flTempY * s;
		currentPoint.y = flTempX * s + flTempY * c;

		currentPoint += vecPointsCenter;
	}

	std::array< Vertex_t, 3 >vertices{ Vertex_t(arrPoints.at(0)), Vertex_t(arrPoints.at(1)), Vertex_t(arrPoints.at(2)) };

	static int iTextureID = i::Surface->CreateNewTextureID(true);

	
	Color drawColor; 
	if (!pEntity->IsDormant())
		drawColor = Color(cfg::misc::flOOFColor);
	else
		drawColor = Color(vecDormantColor);

	i::Surface->DrawSetColor(drawColor * 2);
	i::Surface->DrawLine(arrPoints.at(0).x, arrPoints.at(0).y, arrPoints.at(1).x, arrPoints.at(1).y);
	i::Surface->DrawLine(arrPoints.at(1).x, arrPoints.at(1).y, arrPoints.at(2).x, arrPoints.at(2).y);
	i::Surface->DrawLine(arrPoints.at(2).x, arrPoints.at(2).y, arrPoints.at(0).x, arrPoints.at(0).y);

	i::Surface->DrawSetColor(drawColor);
	i::Surface->DrawSetTexture(iTextureID);
	i::Surface->DrawTexturedPolygon(3, vertices.data());
}

void visual::WelcomeUser(std::string szText) {

	if (!i::GlobalVars || bNotWelcome)
		return;

	static float iStartTick = i::GlobalVars->iTickCount;
	static bool bGoBack = false;
	static bool bDoneAnimating = false;
	if (i::GlobalVars->iTickCount - iStartTick > TIME_TO_TICKS(5)) {
		bNotWelcome = true;
	}
	if (i::GlobalVars->iTickCount - iStartTick > TIME_TO_TICKS(4)) {
		bGoBack = true;
	}

	constexpr int iEndPoint = 364;
	constexpr int iStepSize = iEndPoint / 32;
	static int iCurrentPoint = 0;

	int iEaseAnimation = 0.05f * abs(iEndPoint - iCurrentPoint);
	if (abs(iStartTick - i::GlobalVars->iTickCount) <= TIME_TO_TICKS(0.5f))
		iCurrentPoint += iStepSize * iEaseAnimation;
	else
		bDoneAnimating = true;

	if (bGoBack && iCurrentPoint > 0)
		iCurrentPoint = std::clamp(iCurrentPoint - iStepSize, 0, iEndPoint);

	// draw the border
	i::Surface->DrawSetColor(40, 40, 40, 255);
	i::Surface->DrawOutlinedRect(0, 50, iCurrentPoint, 125);

	// draw middle
	i::Surface->DrawSetColor(12, 12, 12, 255);
	i::Surface->DrawFilledRect(0, 51, iCurrentPoint - 1, 124);

	i::Surface->DrawSetColor(40, 40, 40, 255);
	i::Surface->DrawOutlinedRect(0, 51 + 8, iCurrentPoint - 1 - 8, 124 - 8);

	i::Surface->DrawSetColor(22, 22, 22, 255);
	i::Surface->DrawFilledRect(0, 51 + 9, iCurrentPoint - 1 - 9, 124 - 9);

	if (bDoneAnimating) {
		static int iAlpha = 0;

		if (abs(iStartTick - i::GlobalVars->iTickCount) >= TIME_TO_TICKS(3.5f))
			iAlpha = std::clamp(iAlpha - 255 / TIME_TO_TICKS(0.3f), 0, 255);
		else if (abs(iStartTick - i::GlobalVars->iTickCount) >= TIME_TO_TICKS(0.5f) && abs(iStartTick - i::GlobalVars->iTickCount) >= TIME_TO_TICKS(0.8f))
			iAlpha = std::clamp(iAlpha + 255 / TIME_TO_TICKS(0.3f), 0, 255);

		iAlpha = std::clamp(iAlpha, 0, 255);

		i::Surface->DrawT(iCurrentPoint / 2, 125 / 2 + 17, Color(255, 255, 255, iAlpha), g::fonts::DebugFont, true, szText.c_str());
	}
}

void visual::WorldLightning(Color color) {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (!i::EngineClient->IsInGame() || !pLocal || g::bUpdatingSkins || i::ClientState->iDeltaTick <= 0)
		return;

	if (!cfg::misc::bOverrideLampColors)
		return;

	static WorldBrushData_t* worldbrush = *reinterpret_cast<WorldBrushData_t***>(MEM::FindPattern(ENGINE_DLL, XorStr("8B 1D ? ? ? ? 56 57 83 EC")) + 2)[0];
	
	if (!worldbrush)
		return;
	
	if (worldbrush->numworldlights <= 0)
		return;

	Vector EyePosition = g::vecEyePosition;
	static int MaxDistance = 1000;

	if (!vecLights.empty()) {

		for (auto NewDLight : vecLights) {

			if (cfg::misc::iFlicker != 0) {
				float flRandom = M::RandomInt(0, cfg::misc::iFlicker) / 255.f;
				color[0] /= flRandom;
				color[1] /= flRandom;
				color[2] /= flRandom;
			}

			auto WorldLight = worldbrush->worldlights[NewDLight.second];
			Vector LightPosition = { WorldLight.origin.x, WorldLight.origin.y, WorldLight.origin.z };
			float LightDistance = LightPosition.DistTo(EyePosition);

			NewDLight.first->flags = 0;
			NewDLight.first->origin = WorldLight.origin;
			NewDLight.first->radius = 300 * ((MaxDistance - LightDistance) / MaxDistance);
			NewDLight.first->color[0] = color[0];
			NewDLight.first->color[1] = color[1];
			NewDLight.first->color[2] = color[2];
			NewDLight.first->color[3] = (color[3] / 255.f) * 10;
			NewDLight.first->die = i::GlobalVars->flCurrentTime + i::GlobalVars->flIntervalPerTick * 3; // in case of time fluctuation
			NewDLight.first->decay = 0;
		}
		return;
	}
	else
		vecLights.clear();

	for (int i = 0; i < worldbrush->numworldlights; i++)
	{
		auto WorldLight = worldbrush->worldlights[i];
		Vector LightPosition = { WorldLight.origin.x, WorldLight.origin.y, WorldLight.origin.z };
		float LightDistance = LightPosition.DistTo(EyePosition);

		// Save me from the fps eating monster
		//if (LightDistance <= MaxDistance)
		{
			Dlight_t* NewDLight = i::Effects->CL_AllocDlight(i);
			NewDLight->flags = 0;
			NewDLight->origin = WorldLight.origin;
			NewDLight->radius = 300 * ((MaxDistance - LightDistance) / MaxDistance);
			NewDLight->color[0] = color[0];
			NewDLight->color[1] = color[1];
			NewDLight->color[2] = color[2];
			NewDLight->color[3] = 6 * (color[3] / 255);
			NewDLight->die = i::GlobalVars->flCurrentTime + i::GlobalVars->flIntervalPerTick * 3; // in case of time fluctuation
			NewDLight->decay = 0;
			NewDLight->key = i;

			vecLights.push_back({ NewDLight , i});
		}
	}
}

// no please dont even ask that
const char* visual::GetWeaponChar(std::string input) {

	if (input == "USP-S")
		return "A";
	if (input == "Glock-18")
		return "C";
	if (input == "Dual Berettas")
		return "S";
	if (input == "P250" || input == "Zeus x27")
		return "U";
	if (input == "CZ75-Auto")
		return "D";
	if (input == "R8 Revolver")
		return "F";
	if (input == "MAC-10")
		return "L";
	if (input == "MP7")
		return "X";
	if (input == "UMP-45")
		return "Q";
	if (input == "P90")
		return "M";


	if (input == "SSG 08")
		return "N";
	if (input == "AWP")
		return "R";
	if (input == "G3SG1")
		return "I";
	if (input == "SCAR-20")
		return "O";
	if (input == "Desert Eagle")
		return "F";
	if (input == "Knife")
		return "h";

	if (input == "Molotov" || input == "Incendiary Grenade")
		return "G";

	if (input == "High Explosive Grenade")
		return "H";

	if (input == "Smoke Grenade")
		return "P";

	if (input == "Defuse Kit")
		return "f";

	if (input == "Medi-Shot")
		return "b";

	return "<";
}

void visual::CustomHud() {

	if (!g::pLocal)
		return;

	static CConVar* cl_draw_only_deathnotices = i::ConVar->FindVar("cl_draw_only_deathnotices");

	static bool bSetBack = false;
	if (!cfg::misc::bCustomHud) {
		if (bSetBack) {
			cl_draw_only_deathnotices->SetValue(0);
			bSetBack = false;
		}
		return;
	}
	bSetBack = true;
	if (cl_draw_only_deathnotices->GetInt() != 1)
		cl_draw_only_deathnotices->SetValue(1);

	//g::pLocal->HideHud() &= ~(HIDEHUD_CHAT | HIDEHUD_RADAR);

	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::vector<std::pair<std::string, bool>> vecWeaponNames{};
	for (auto nIndex : g::pLocal->GetWeaponsHandle()) {

		CBaseCombatWeapon* pWeapon = static_cast<CBaseCombatWeapon*>(i::EntityList->GetClientEntityFromHandle(nIndex));

		if (pWeapon == nullptr)
			continue;

		CCSWeaponInfo* pWeaponData = pWeapon->GetCSWpnData();

		if (pWeaponData == nullptr)
			continue;

		std::string text = "";
		wchar_t* localizeName = i::Localize->Find(pWeaponData->szHudName);
		if (localizeName)
			text = converter.to_bytes(localizeName);

		bool bCurrent = false;
		if (pWeapon == g::pLocal->GetWeapon())
			bCurrent = true;

		vecWeaponNames.push_back({text, bCurrent});
	}

	int iScreenX, iScreenY;
	i::EngineClient->GetScreenSize(iScreenX, iScreenY);

	int iPadding = 50;
	int iCenterOffset = (iScreenX / 2) - ((vecWeaponNames.size() / 2) * iPadding) + 10;

	for (auto szCurrent = vecWeaponNames.rbegin(); szCurrent != vecWeaponNames.rend(); ++szCurrent) {

		if ((*szCurrent).second) {

			i::Surface->DrawSetColor(Color(212, 212, 212, 100));
			i::Surface->DrawFilledRectFade(iCenterOffset - iPadding / 2, iScreenY - 70, iCenterOffset + iPadding / 2, iScreenY, 255, 0, false);
		}

		i::Surface->DrawT(iCenterOffset, iScreenY - 50, Color(255, 255, 255, 255), 0xA1, true, std::format("{}", GetWeaponChar((*szCurrent).first)).c_str());
		iCenterOffset += iPadding;
	}

	auto DrawSquare = [&](int x, int y, int size) {

		i::Surface->DrawFilledRect(x, y, x + size, y + size);
	};

	auto DrawEmptyHearth = [&](int x, int y) {

		i::Surface->DrawSetColor(Color(0, 0, 0, 255));

		static int iSize = 3;

		// outline
		DrawSquare(x, y, iSize);
		for (size_t i = 1; i <= 3; i++) {
			DrawSquare(x - iSize * i, y - iSize * i, iSize);
			DrawSquare(x + iSize * i, y - iSize * i, iSize);
		}
		for (size_t i = 0; i < 3; i++)
		{
			DrawSquare(x + iSize * 4, y - iSize * (4 + i), iSize);
			DrawSquare(x - iSize * 4, y - iSize * (4 + i), iSize);
		}
		DrawSquare(x + iSize * 3, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 7, iSize);

		DrawSquare(x + iSize * 2, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 8, iSize);

		DrawSquare(x + iSize * 1, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 8, iSize);

		DrawSquare(x, y - iSize * 7, iSize);
		DrawSquare(x, y - iSize * 7, iSize);

		// color fill
		i::Surface->DrawSetColor(Color(40, 40, 40, 255));
		DrawSquare(x, y - iSize, iSize);

		DrawSquare(x + iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 4, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 3, iSize);

		DrawSquare(x - iSize * 2, y - iSize * 3, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 4, iSize);

		DrawSquare(x + iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x + iSize * 1, y - iSize * 7, iSize);

		DrawSquare(x - iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 7, iSize);

		for (size_t i = 2; i < 7; i++)
			DrawSquare(x, y - iSize * i, iSize);

		for (size_t i = 3; i < 8; i++) {
			DrawSquare(x + iSize, y - iSize * i, iSize);
			DrawSquare(x - iSize, y - iSize * i, iSize);
		}
		for (size_t i = 4; i < 8; i++) {
			DrawSquare(x + iSize * 2, y - iSize * i, iSize);
			DrawSquare(x - iSize * 2, y - iSize * i, iSize);
		}
	};

	auto DrawFilledHearth = [&](int x, int y) {

		i::Surface->DrawSetColor(Color(0, 0, 0, 255));

		static int iSize = 3;

		// outline
		DrawSquare(x, y, iSize);
		for (size_t i = 1; i <= 3; i++) {
			DrawSquare(x - iSize * i, y - iSize * i, iSize);
			DrawSquare(x + iSize * i, y - iSize * i, iSize);
		}
		for (size_t i = 0; i < 3; i++)
		{
			DrawSquare(x + iSize * 4, y - iSize * (4 + i), iSize);
			DrawSquare(x - iSize * 4, y - iSize * (4 + i), iSize);
		}
		DrawSquare(x + iSize * 3, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 7, iSize);

		DrawSquare(x + iSize * 2, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 8, iSize);

		DrawSquare(x + iSize * 1, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 8, iSize);

		DrawSquare(x, y - iSize * 7, iSize);
		DrawSquare(x, y - iSize * 7, iSize);

		// color fill
		i::Surface->DrawSetColor(Color(187, 19, 19, 255));
		DrawSquare(x, y - iSize, iSize);

		DrawSquare(x + iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 4, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 3, iSize);

		DrawSquare(x - iSize * 2, y - iSize * 3, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 4, iSize);

	
		i::Surface->DrawSetColor(Color(255, 19, 19, 255));
		DrawSquare(x + iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x + iSize * 1, y - iSize * 7, iSize);

		DrawSquare(x - iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 7, iSize);

		for (size_t i = 2; i < 7; i++)
			DrawSquare(x, y - iSize * i, iSize);
		
		for (size_t i = 3; i < 8; i++) {
			DrawSquare(x + iSize, y - iSize * i, iSize);
			DrawSquare(x - iSize, y - iSize * i, iSize);
		}
		for (size_t i = 4; i < 8; i++) {
			DrawSquare(x + iSize * 2, y - iSize * i, iSize);
			DrawSquare(x - iSize * 2, y - iSize * i, iSize);
		}

		i::Surface->DrawSetColor(Color(255, 201, 201, 255));
		DrawSquare(x - iSize * 2, y - iSize * 6, iSize);
	};

	auto DrawHalfFilledHearth = [&](int x, int y) {

		i::Surface->DrawSetColor(Color(0, 0, 0, 255));

		static int iSize = 3;

		// outline
		DrawSquare(x, y, iSize);
		for (size_t i = 1; i <= 3; i++) {
			DrawSquare(x - iSize * i, y - iSize * i, iSize);
			DrawSquare(x + iSize * i, y - iSize * i, iSize);
		}
		for (size_t i = 0; i < 3; i++)
		{
			DrawSquare(x + iSize * 4, y - iSize * (4 + i), iSize);
			DrawSquare(x - iSize * 4, y - iSize * (4 + i), iSize);
		}
		DrawSquare(x + iSize * 3, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 7, iSize);

		DrawSquare(x + iSize * 2, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 8, iSize);

		DrawSquare(x + iSize * 1, y - iSize * 8, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 8, iSize);

		DrawSquare(x, y - iSize * 7, iSize);
		DrawSquare(x, y - iSize * 7, iSize);

		// color fill

		i::Surface->DrawSetColor(Color(40, 40, 40, 255));
		DrawSquare(x + iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 4, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 3, iSize);

		i::Surface->DrawSetColor(Color(187, 19, 19, 255));
		DrawSquare(x, y - iSize, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 3, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 2, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 4, iSize);

		i::Surface->DrawSetColor(Color(40, 40, 40, 255));
		DrawSquare(x + iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x + iSize * 1, y - iSize * 7, iSize);

		i::Surface->DrawSetColor(Color(255, 19, 19, 255));
		DrawSquare(x - iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x - iSize * 3, y - iSize * 6, iSize);
		DrawSquare(x - iSize * 2, y - iSize * 7, iSize);
		DrawSquare(x - iSize * 1, y - iSize * 7, iSize);

		for (size_t i = 2; i < 7; i++)
			DrawSquare(x, y - iSize * i, iSize);

		for (size_t i = 3; i < 8; i++) {
			i::Surface->DrawSetColor(Color(40, 40, 40, 255));
			DrawSquare(x + iSize, y - iSize * i, iSize);
			i::Surface->DrawSetColor(Color(255, 19, 19, 255));
			DrawSquare(x - iSize, y - iSize * i, iSize);
		}
		for (size_t i = 4; i < 8; i++) {
			i::Surface->DrawSetColor(Color(40, 40, 40, 255));
			DrawSquare(x + iSize * 2, y - iSize * i, iSize);
			i::Surface->DrawSetColor(Color(255, 19, 19, 255));
			DrawSquare(x - iSize * 2, y - iSize * i, iSize);
		}

		i::Surface->DrawSetColor(Color(255, 201, 201, 255));
		DrawSquare(x - iSize * 2, y - iSize * 6, iSize);
	};

	const int& iHealth = g::pLocal->GetHealth();
	float flHearthCount = iHealth / 10.f;
	int iDrawCount = 0;
	for (size_t i = 1; i < 10; i++) {

		int offset = 0;
		if (flHearthCount <= 3)
			offset = M::RandomInt(-5, 5);
		
		if (i - flHearthCount < 1 && i - flHearthCount > 0) 
			DrawHalfFilledHearth(iScreenX * 0.13f + (i * 26) + offset, iScreenY - 70 + offset);
		
		else if (i > flHearthCount) 
			DrawEmptyHearth(iScreenX * 0.13f + (i * 26) + offset, iScreenY - 70 + offset);
		
		else if (iHealth > 0)
			DrawFilledHearth(iScreenX * 0.13f + (i * 26) + offset, iScreenY - 70 + offset);
		
	}

	//i::Surface->DrawT(50 + (fDistance * flFactor), iScreenY - 70, Color(255, 255, 255), g::fonts::DebugFont, true, std::format("{}", g::pLocal->GetHealth()).c_str());

	auto DrawArrow = [&](int x, int y) {

		static int iSize = 3;

		i::Surface->DrawSetColor(Color(63, 63, 63, 255));
		DrawSquare(x, y, iSize);
		DrawSquare(x + iSize, y - iSize, iSize);
		DrawSquare(x - iSize, y - iSize, iSize);
		DrawSquare(x + iSize * 2, y - iSize * 2, iSize);

		i::Surface->DrawSetColor(Color(224, 224, 224, 255));
		DrawSquare(x, y - iSize, iSize);
		DrawSquare(x - iSize, y - iSize * 2, iSize);
		DrawSquare(x + iSize, y - iSize * 2, iSize);
		DrawSquare(x, y - iSize * 3, iSize);

		i::Surface->DrawSetColor(Color(198, 198, 198, 255));
		DrawSquare(x, y - iSize * 2, iSize);
		DrawSquare(x + iSize, y - iSize * 3, iSize);

		i::Surface->DrawSetColor(Color(40, 30, 11, 255));
		DrawSquare(x + iSize * 2, y - iSize * 3, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 4, iSize);
		DrawSquare(x + iSize * 4, y - iSize * 5, iSize);
		DrawSquare(x + iSize * 5, y - iSize * 6, iSize);
		DrawSquare(x + iSize * 6, y - iSize * 7, iSize);
		DrawSquare(x + iSize * 7, y - iSize * 8, iSize);
		DrawSquare(x + iSize * 8, y - iSize * 9, iSize);

		i::Surface->DrawSetColor(Color(137, 103, 39, 255));
		DrawSquare(x + iSize * 2, y - iSize * 4, iSize);
		DrawSquare(x + iSize * 3, y - iSize * 5, iSize);
		DrawSquare(x + iSize * 4, y - iSize * 6, iSize);
		DrawSquare(x + iSize * 5, y - iSize * 7, iSize);
		DrawSquare(x + iSize * 6, y - iSize * 8, iSize);
		DrawSquare(x + iSize * 7, y - iSize * 9, iSize);
		DrawSquare(x + iSize * 8, y - iSize * 10, iSize);

		i::Surface->DrawSetColor(Color(150, 150, 150, 255));
		DrawSquare(x + iSize * 9, y - iSize * 9, iSize);
		DrawSquare(x + iSize * 7, y - iSize * 9, iSize);
		DrawSquare(x + iSize * 9, y - iSize * 10, iSize);
		DrawSquare(x + iSize * 10, y - iSize * 9, iSize);

		i::Surface->DrawSetColor(Color(216, 216, 216, 255));
		DrawSquare(x + iSize * 9, y - iSize * 10, iSize);
		DrawSquare(x + iSize * 9, y - iSize * 11, iSize);
		DrawSquare(x + iSize * 8, y - iSize * 11, iSize);

		i::Surface->DrawSetColor(Color(255, 255, 255, 255));
		DrawSquare(x + iSize * 10, y - iSize * 12, iSize);

		i::Surface->DrawSetColor(Color(68, 68, 68, 255));
		DrawSquare(x + iSize * 9, y - iSize * 8, iSize);
		DrawSquare(x + iSize * 10, y - iSize * 9, iSize);
		DrawSquare(x + iSize * 10, y - iSize * 10, iSize);
		DrawSquare(x + iSize * 11, y - iSize * 11, iSize);
		DrawSquare(x + iSize * 12, y - iSize * 12, iSize);

	};

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	DrawArrow(iScreenX * 0.87f, iScreenY - 50);
	i::Surface->DrawT(iScreenX * 0.87f + 15, iScreenY - 50, Color(255, 255, 255, 255), g::fonts::DebugFont, true, std::format("{}/{}", pWeapon->GetAmmo(), pWeapon->GetAmmoReserve()).c_str());
}

void visual::AutoPeekCircle() {
	bool bShow = false;
	static Vector vecLastPositon = Vector(0, 0, 0);

	if (cfg::antiaim::bAutoPeek && IPT::HandleInput(cfg::antiaim::iAutoPeek) && misc::vecRecord != Vector(0, 0, 0)) {
		bShow = true;
		vecLastPositon = misc::vecRecord;
	}

	static float flAlpha = 0.f;
	flAlpha = std::lerp(flAlpha, bShow ? 1.f : 0.f, i::GlobalVars->flFrameTime * 10);
	flAlpha = std::clamp(flAlpha, 0.f, 10.f);

	if (flAlpha <= 0.01)
		return;

	int iOuterRadius = 25;
	const int iSegemnts = 90;
	const int iInnerRadius = 20;
	const float flStep = 0.02f;

	for (int i = 1; i <= iInnerRadius; ++i) {

		const float flDistanceFromCenter = static_cast<float>(iOuterRadius * flAlpha - i * flStep);
		const int a = static_cast<int>(255 * (1 - static_cast<float>(i) / iInnerRadius));

		std::vector<int> vecPointsX;
		std::vector<int> vecPointsY;
		for (int j = 0; j < iSegemnts; ++j) {
			const float flAngle = static_cast<float>(j) / iSegemnts * 2.0f * M_PI;
			const float flPointX = cos(flAngle) * flDistanceFromCenter;
			const float flPointY = sin(flAngle) * flDistanceFromCenter;

			Vector screen_pos;
			if (!i::DebugOverlay->ScreenPosition(Vector(vecLastPositon.x + flPointX, vecLastPositon.y + flPointY, vecLastPositon.z), screen_pos)) {
				vecPointsX.push_back(static_cast<int>(screen_pos.x));
				vecPointsY.push_back(static_cast<int>(screen_pos.y));
			}
		}

		Color colCircle = cfg::model::flAutopeekColor;
		colCircle = colCircle.Set<COLOR_A>(a);

		i::Surface->DrawSetColor(colCircle);
		i::Surface->DrawPolyLine(vecPointsX.data(), vecPointsY.data(), static_cast<int>(vecPointsX.size()));
	}
}

void visual::Hat() {

	if (!cfg::misc::bHat)
		return;

	if (g::pLocal && i::Input->bCameraInThirdPerson) {

		Vector vecHeadPosition = localAnim->GetRealMatrix().at(BONE_HEAD).at(3);

		float flStep = (2 * M_PI) / 18.f;
		float flRadius = 9;

		for (float rotation = 0; rotation < (M_PI * 2.0); rotation += flStep) {

			Vector pointA(flRadius * cos(rotation) + vecHeadPosition.x, flRadius * sin(rotation) + vecHeadPosition.y, vecHeadPosition.z + 2);
			Vector bottomRim(flRadius / 2 * cos(rotation) + vecHeadPosition.x, flRadius / 2.f * sin(rotation) + vecHeadPosition.y, vecHeadPosition.z + 8);

			Vector pointB(flRadius * cos(rotation + flStep) + vecHeadPosition.x, flRadius * sin(rotation + flStep) + vecHeadPosition.y, vecHeadPosition.z + 2);
			Vector bottomRimPredict(flRadius / 2 * cos(rotation + flStep) + vecHeadPosition.x, flRadius / 2.f * sin(rotation + flStep) + vecHeadPosition.y, vecHeadPosition.z + 8);

			Vector topOnScreen, botOnScreen, topOnScreenPredict, botOnScreenPredict;
			if (!i::DebugOverlay->ScreenPosition(pointA, topOnScreen))
			{
				if (!i::DebugOverlay->ScreenPosition(bottomRim, botOnScreen)) {

					i::Surface->DrawSetColor(Color(cfg::misc::flHat));

					i::Surface->DrawLine(topOnScreen.x, topOnScreen.y, botOnScreen.x, botOnScreen.y);

					if (!i::DebugOverlay->ScreenPosition(pointB, topOnScreenPredict))
						i::Surface->DrawLine(topOnScreen.x, topOnScreen.y, topOnScreenPredict.x, topOnScreenPredict.y);

					if (!i::DebugOverlay->ScreenPosition(bottomRimPredict, botOnScreenPredict))
						i::Surface->DrawLine(botOnScreen.x, botOnScreen.y, botOnScreenPredict.x, botOnScreenPredict.y);

				}
			}
		}
	}
}

void visual::CrossHair() {

	if (g::pLocal) {

		if (cfg::misc::bRemovals[5] && g::pLocal->IsScoped() && i::EngineClient->IsInGame()) {

			int iHeight = 0;
			int iWidth = 0;
			i::EngineClient->GetScreenSize(iWidth, iHeight);

			float flSize = iHeight * (cfg::misc::iScopeLength / 100.f);

			i::Surface->DrawSetColor(cfg::misc::flScopeColor);
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - flSize - 15,
				iHeight / 2 - 1,
				iWidth / 2 - 15,
				iHeight / 2,
				0, cfg::misc::flScopeColor[3] * 255.f, true);

			i::Surface->DrawSetColor(Color(cfg::misc::flScopeColorEnd));
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - flSize - 15,
				iHeight / 2 - 1,
				iWidth / 2 - 15,
				iHeight / 2,
				cfg::misc::flScopeColorEnd[3] * 255.f, 0, true);

			i::Surface->DrawSetColor(cfg::misc::flScopeColor);
			i::Surface->DrawFilledRectFade(
				iWidth / 2 + 15,
				iHeight / 2 - 1,
				iWidth / 2 + flSize + 15,
				iHeight / 2,
				cfg::misc::flScopeColor[3] * 255.f, 0, true);

			i::Surface->DrawSetColor(Color(cfg::misc::flScopeColorEnd));
			i::Surface->DrawFilledRectFade(
				iWidth / 2 + 15,
				iHeight / 2 - 1,
				iWidth / 2 + flSize + 15,
				iHeight / 2,
				0, cfg::misc::flScopeColorEnd[3] * 255.f, true);

			i::Surface->DrawSetColor(cfg::misc::flScopeColor);
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - 1,
				iHeight / 2 - flSize - 15,
				iWidth / 2,
				iHeight / 2 - 15,
				0, cfg::misc::flScopeColor[3] * 255.f, false);

			i::Surface->DrawSetColor(Color(cfg::misc::flScopeColorEnd));
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - 1,
				iHeight / 2 - flSize - 15,
				iWidth / 2,
				iHeight / 2 - 15,
				cfg::misc::flScopeColorEnd[3] * 255.f, 0, false);

			i::Surface->DrawSetColor(cfg::misc::flScopeColor);
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - 1,
				iHeight / 2 + 15,
				iWidth / 2,
				iHeight / 2 + flSize + 15,
				cfg::misc::flScopeColor[3] * 255.f, 0, false);

			i::Surface->DrawSetColor(Color(cfg::misc::flScopeColorEnd));
			i::Surface->DrawFilledRectFade(
				iWidth / 2 - 1,
				iHeight / 2 + 15,
				iWidth / 2,
				iHeight / 2 + flSize + 15,
				0, cfg::misc::flScopeColorEnd[3] * 255.f, false);
		}
	}
}

void visual::DrawList() {

	static Vector screenPosition{};
	for (Vector& drawPos : g::drawList)
	{
		if (i::DebugOverlay->ScreenPosition(drawPos, screenPosition))
			continue;

		i::Surface->DrawSetColor(Color(255, 255, 255, 255));
		i::Surface->DrawFilledRect(screenPosition.x - 2, screenPosition.y - 2, screenPosition.x + 2, screenPosition.y + 2);
	}

	while (g::drawList.size() > 32)
		g::drawList.pop_front();
}

void visual::AddDamageIndicator(const DamageIndicator& indicator) {
	if (!cfg::misc::bDamageIndicator || indicator.iDamage <= 0)
		return;
	std::lock_guard<std::mutex> lock(damageIndicatorMutex);
	if (cfg::misc::bMergeDamage && indicator.iVictimIndex > 0) {
		for (auto& existing : damageIndicators) {
			const int age = indicator.iTickcount - existing.iTickcount;
			if (existing.iVictimIndex == indicator.iVictimIndex && age >= 0 && age <= TIME_TO_TICKS(.05f)) {
				existing.iDamage = (std::min)(10000, existing.iDamage + std::clamp(indicator.iDamage, 1, 10000));
				existing.bHeadshot |= indicator.bHeadshot;
				return;
			}
		}
	}
	if (damageIndicators.size() >= 64)
		damageIndicators.erase(damageIndicators.begin());
	auto stored = indicator;
	stored.iDamage = std::clamp(stored.iDamage, 1, 10000);
	damageIndicators.push_back(stored);
}

void visual::ClearDamageIndicators() {
	std::lock_guard<std::mutex> lock(damageIndicatorMutex);
	damageIndicators.clear();
}

void visual::HandleDamageIndicator() {
	if (!cfg::misc::bDamageIndicator || !g::pLocal || !i::EngineClient->IsInGame()) {
		ClearDamageIndicators();
		return;
	}

	const int currentTick = i::GlobalVars->iTickCount;
	const float seconds = std::isfinite(cfg::misc::flDamageLifetime) ? std::clamp(cfg::misc::flDamageLifetime, .25f, 5.f) : 1.f;
	const int lifetime = (std::max)(1, TIME_TO_TICKS(seconds));
	const float rise = std::isfinite(cfg::misc::flDamageRise) ? std::clamp(cfg::misc::flDamageRise, 0.f, 80.f) : 18.f;
	const int fontSize = std::clamp(cfg::misc::iDamageFontSize, 10, 36);
	static int previousFontSize = 20;
	if (fontSize != previousFontSize) {
		i::Surface->SetFontGlyphSet(g::fonts::HitIndicator, "Verdana", fontSize, FW_NORMAL, 0, 0, FONTFLAG_ANTIALIAS);
		previousFontSize = fontSize;
	}
	std::vector<DamageIndicator> indicators;
	{
		std::lock_guard<std::mutex> lock(damageIndicatorMutex);
		std::erase_if(damageIndicators, [currentTick, lifetime](const DamageIndicator& indicator) {
			const int age = currentTick - indicator.iTickcount;
			return age < 0 || age >= lifetime;
		});
		indicators = damageIndicators;
	}

	for (const auto& it : indicators) {
		const int age = currentTick - it.iTickcount;
		const Vector position = it.vecPosition + Vector(0.f, 0.f, rise * TICKS_TO_TIME(age));
		Vector vecScreenPosition{};
		if (i::DebugOverlay->ScreenPosition(position, vecScreenPosition))
			continue;

		auto* selectedColor = it.bHeadshot ? cfg::misc::flDamageHeadshot : cfg::misc::flDamageIndicator;
		Color indicatorColor(selectedColor);
		const float alpha = std::clamp(selectedColor[3], 0.f, 1.f) *
			(1.f - static_cast<float>(age) / lifetime);
		indicatorColor[3] = static_cast<std::uint8_t>(alpha * 255.f);
		i::Surface->DrawT( vecScreenPosition.x, vecScreenPosition.y, indicatorColor, g::fonts::HitIndicator, true, std::to_string( it.iDamage ).c_str( ) );
	}
}
