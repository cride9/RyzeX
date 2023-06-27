#include "ESP.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/EnemyAnimations.h"
#include "../Rage/ragebot.h"
#include "../Rage/exploits.h"
#include "../../SDK/Menu/gui.h"
#include "../../SDK/RayTracer rebuilt/CRayTrace.h"
#include "../../SDK/InputSystem.h"
#include "../../../Dependecies/BASS/dll.h"
#include "../Misc/Playerlist.h"

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
		WorldCrosshair();
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

		if (pEnt->GetTeam() != g::pLocal->GetTeam())
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

		if (pEnt->GetTeam() != g::pLocal->GetTeam()) { // Enemy

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

		i::Surface->DrawT(right + 2, top + spacing, bDormant ? vecDormantColor : flFlagsColor[ARMOR], g::fonts::FlagESP, false, std::format(kevlarPrefix, pEnt->GetArmor()).c_str());
		spacing += 10;
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

		
		static auto something = [](int right, int top, int& spacing, const char* print) {

			i::Surface->DrawT(right + 2, (top + spacing) - 80, RYZEXCOLOR, g::fonts::FlagESP, false, print);
			spacing += 10;
		};

		const unsigned int iLastValid = pLog->iLastValid;
		if (iLastValid >= pLog->pRecord.size())
			return;

		auto pRecord = &pLog->pRecord.front();

		/*
			pLayer->flMovementSide = m_flMovementSide;
			pLayer->angMoveYaw = m_angAngle;
			pLayer->vecDirection = m_vecDirection;
			pLayer->flFeetWeight = m_flNewFeetWeight;
		*/

		const float fServerPlaybackrate = anims.GetLocalCycleIncrement(pEnt, pRecord, pRecord->pLayers[6].flPlaybackRate);
		const float fCenterPlaybackrate = anims.GetLocalCycleIncrement(pEnt, pRecord, pRecord->LayerData[CENTER].flPlaybackRate, &pRecord->LayerData[CENTER]);
		const float fRightPlaybackrate = anims.GetLocalCycleIncrement(pEnt, pRecord, pRecord->LayerData[RIGHT].flPlaybackRate, &pRecord->LayerData[RIGHT]);
		const float fLeftPlaybackrate = anims.GetLocalCycleIncrement(pEnt, pRecord, pRecord->LayerData[LEFT].flPlaybackRate, &pRecord->LayerData[LEFT]);

		float leftdiff = fabsf(fServerPlaybackrate - fLeftPlaybackrate);
		float rightdiff = fabsf(fServerPlaybackrate - fRightPlaybackrate);
		float centerdiff = fabsf(fServerPlaybackrate - fCenterPlaybackrate);
		float flReturn = (leftdiff < rightdiff && leftdiff < centerdiff) ? -58.0f : ((rightdiff < leftdiff && rightdiff < centerdiff) ? 58.0f : 0.0f);

		static float flLastValidResolveYaw = flReturn;
		if (flReturn != 0)
			flLastValidResolveYaw = flReturn;

		something(right, top, spacing, "[Layer 6]");
		something(right, top, spacing, std::format("Left: {}", fLeftPlaybackrate).c_str());
		something(right, top, spacing, std::format("Right: {}", fRightPlaybackrate).c_str());
		something(right, top, spacing, std::format("Center: {}", fCenterPlaybackrate).c_str());
		something(right, top, spacing, std::format("Server: {}", fServerPlaybackrate).c_str());
		something(right, top, spacing, std::format("Yaw: {}", flLastValidResolveYaw).c_str());
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

			if (pEntity->GetTeam() != pLocal->GetTeam() && cfg::visual::bGlow[ENEMY]) 
				hGlowObject->Set(Color(cfg::visual::flGlowColor[ENEMY]));

			else if (pEntity->GetTeam() == pLocal->GetTeam() && pEntity != g::pLocal && cfg::visual::bGlow[TEAM])
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

void visual::SkeletonEsp(CBaseEntity* pEntity, Color color) {

	if (pEntity->IsDormant())
		return;

	const Model_t* model = pEntity->GetModel();
	if (!model)
		return;

	studiohdr_t* pStudioHdr = i::ModelInfo->GetStudioModel(model);
	if (!pStudioHdr)
		return;

	auto* pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (pEntity != g::pLocal && (!pLog || pLog->pRecord.empty() || pLog->pCachedMatrix.data()->GetOrigin().IsZero()))
		return;

	auto skeleton_position = [=](const size_t idx)
	{
		auto child = (pEntity == g::pLocal) ? pEntity->GetCachedBoneData().Base()[idx].GetOrigin() : pLog->pCachedMatrix[idx].GetOrigin();
		return child;
	};
	auto skeleton_position_desync = [=](const size_t idx)
	{
		auto child = g_LocalAnimations->GetDesyncMatrix()[idx].GetOrigin();
		return child;
	};

	if (g::pLocal == pEntity && cfg::model::localDesync && cfg::model::localDesyncSkeleton) {
		for (int i = 0; i < pStudioHdr->nBones; i++){

			auto bone = pStudioHdr->GetBone(i);
			if (!bone)
				continue;

			if (bone->iParent == -1)
				continue;

			if (!(bone->iFlags & BONE_USED_BY_HITBOX))
				continue;

			auto child = skeleton_position_desync(i);
			auto parent = skeleton_position_desync(bone->iParent);
			auto chestbone = skeleton_position_desync(6);

			auto upper = skeleton_position_desync(6 + 1) - chestbone;
			auto breast = chestbone + upper - (upper / 3);

			auto deltachild = child - breast;
			auto deltaparent = parent - breast;

			if (deltaparent.Length() < 9.0f && deltachild.Length() < 9.0f)
				parent = breast;

			if (i == 5)
				child = breast;

			if (std::abs(deltachild.z) < 5.0f && deltaparent.Length() < 5.0f && deltachild.Length() < 5.0f || i == 6)
				continue;

			Vector sParent;
			Vector sChild;
			i::DebugOverlay->ScreenPosition(parent, sParent);
			i::DebugOverlay->ScreenPosition(child, sChild);

			i::Surface->DrawSetColor(cfg::model::localDesyncColor[0] * 255.f, cfg::model::localDesyncColor[1] * 255.f, cfg::model::localDesyncColor[2] * 255.f, color[3]);
			i::Surface->DrawLine(sParent[0], sParent[1], sChild[0], sChild[1]);
		}
	}

	for (int i = 0; i < pStudioHdr->nBones; i++)
	{
		auto bone = pStudioHdr->GetBone(i);
		if (!bone)
			continue;

		if (bone->iParent == -1)
			continue;

		if (!(bone->iFlags & BONE_USED_BY_HITBOX))
			continue;

		auto child = skeleton_position(i);
		auto parent = skeleton_position(bone->iParent);
		auto chestbone = skeleton_position(6);

		auto upper = skeleton_position(6 + 1) - chestbone;
		auto breast = chestbone + upper - (upper / 3);

		auto deltachild = child - breast;
		auto deltaparent = parent - breast;

		if (deltaparent.Length() < 9.0f && deltachild.Length() < 9.0f)
			parent = breast;

		if (i == 5)
			child = breast;

		if (std::abs(deltachild.z) < 5.0f && deltaparent.Length() < 5.0f && deltachild.Length() < 5.0f || i == 6)
			continue;

		Vector sParent;
		Vector sChild;
		i::DebugOverlay->ScreenPosition(parent, sParent);
		i::DebugOverlay->ScreenPosition(child, sChild);

		i::Surface->DrawSetColor(color[0], color[1], color[2], color[3]);
		i::Surface->DrawLine(sParent[0], sParent[1], sChild[0], sChild[1]);
	}
}

// thx @94
void visual::WorldEsp() {

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL || !g::pLocal || !i::EngineClient->IsConnected() || !i::EngineClient->IsInGame())
		return;

	for (size_t i = 0; i < i::EntityList->GetHighestEntityIndex(); i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || pEntity->IsPlayer())
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
			return;

		using enum EClassIndex;
		if (iClientID == CDecoyProjectile)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Decoy");

		else if (iClientID == CMolotovProjectile)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Molotov");

		else if (iClientID == CSmokeGrenadeProjectile)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Smoke");

		else if (iClientID == CBaseCSGrenadeProjectile) {

			const Model_t* pModel = pWeapon->GetModel();

			if (pModel) {

				std::string szName = pModel->szName;

				if (szName.find("flashbang") != std::string::npos)
					i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Flashbang");
				else if (szName.find("fraggrenade") != std::string::npos)
					i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Grenade");
			}
		}

		else if (iClientID == CInferno) 
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Fire");
		
		else if (iClientID == CPlantedC4)
			i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, cfg::misc::flProjectileESP, g::fonts::FlagESP, true, "Bomb");
	}
}

void visual::DroppedWeapons(CBaseCombatWeapon* pWeapon, Vector& vecScreenPosition) {

	if (!cfg::misc::bDroppedWeaponESP)
		return;

	const EClassIndex iClientID = pWeapon->GetClientClass()->nClassID;

	using enum EClassIndex;
	if (iClientID == CC4)
		i::Surface->DrawT(vecScreenPosition.x, vecScreenPosition.y, Color(1.f, 1.f, 1.f, 1.f), g::fonts::FlagESP, true, "Grenade");

	if (pWeapon->GetItemDefinitionIndex() == WEAPON_TASER && pWeapon->GetAmmo() == 0)
		return;

	CCSWeaponInfo* pData = pWeapon->GetCSWpnData();

	if (!pData)
		return;

	if (pData->nWeaponType < 1 || pData->nWeaponType > 9 || pData->nWeaponType == 8)
		return;

	if (pData->nWeaponType == 9) // grenade
		return;

	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	wchar_t* pName = i::Localize->Find(pData->szHudName);
	std::string name = "";
	if (pName)
		name = converter.to_bytes(pName);

	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	i::Surface->DrawT(vecScreenPosition.x + 1, vecScreenPosition.y - -2, cfg::misc::flDroppedWeaponESP, g::fonts::FlagESP, true, name.c_str());
}

void visual::CoolHackKeyBindList() {

	// static const char* options[] = { "Aimbot", "Doubletap", "Force baim", "DMG override", "Slow walk", "Fake duck", "Auto peek", "Thirdperson", "Blockbot", "Ping" };

	// g::fonts::SkeetFont

	if (!cfg::misc::bKeyBindListOld)
		return;

	int iWidth;
	int iHeight;
	i::EngineClient->GetScreenSize(iWidth, iHeight);

	//std::vector<std::pair<const char*, Color>> vecIndicators;
	//vecIndicators.push_back(std::make_pair("DT", Color(255, 255, 255)));

	//for (auto& current : vecIndicators) {

	//}

	int spacing = 10;
	if (IPT::HandleInput(cfg::rage::ragebotbind) && cfg::rage::enable && cfg::misc::bKeyBindListOld[0]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "AIMBOT");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::rage::doubletapkey) && cfg::rage::doubletap && cfg::misc::bKeyBindListOld[1]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, exploits::iTicksToStore ? Color(255, 255, 255) : Color(255, 0, 0), g::fonts::SkeetFont, false, "DT");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::rage::forceBaimKey) && cfg::rage::forceBaim && cfg::misc::bKeyBindListOld[2]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "BODY");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::rage::overrideBind) && cfg::misc::bKeyBindListOld[3]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "DMG");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::antiaim::fakewalkKey) && cfg::antiaim::fakewalkenable && cfg::misc::bKeyBindListOld[4]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "SLOW");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck && cfg::misc::bKeyBindListOld[5]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(min(143 + (i::ClientState->nChokedCommands * 8), 255), max(191 - i::ClientState->nChokedCommands * 8, 0), 61, 255), g::fonts::SkeetFont, false, "DUCK");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick && cfg::misc::bKeyBindListOld[6]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, misc::bRetreat ? Color(255, 0, 0) : Color(255, 255, 255), g::fonts::SkeetFont, false, "PEEK");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::misc::thirdpersonbind) && cfg::misc::thirdperson && cfg::misc::bKeyBindListOld[7]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "TP");
		spacing += 30;
	}
	if (IPT::HandleInput(cfg::misc::blockbotKey) && cfg::misc::blockbot && cfg::misc::bKeyBindListOld[8]) {

		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(255, 255, 255), g::fonts::SkeetFont, false, "BLOCK");
		spacing += 30;
	}
	if (auto pNetChannelInfo = i::EngineClient->GetNetChannelInfo(); pNetChannelInfo && cfg::misc::bKeyBindListOld[9]) {

		int outgoing = (pNetChannelInfo->GetLatency(FLOW_OUTGOING) + pNetChannelInfo->GetLatency(FLOW_INCOMING)) * 100;
		i::Surface->DrawT(10, iHeight / 2 + spacing, Color(min(143 + (outgoing), 255), max(191 - outgoing, 0), 61, 255), g::fonts::SkeetFont, false, "PING");
		spacing += 30;
	}
}

void visual::DrawRadioInformation() {

	int iWidth;
	int iHeight;
	i::EngineClient->GetScreenSize(iWidth, iHeight);
	auto position = (iHeight / 2) - 20;

	if (strlen(BASS::bass_metadata) > 0 && cfg::misc::bEnableRadio)
	{
		if (!IPT::HandleInput(cfg::misc::iRadioMuteHotKey))
			i::Surface->DrawT(10, position, RYZEXCOLOR, g::fonts::FlagESP, false, "Now playing:");
		else
			i::Surface->DrawT(10, position, RYZEXCOLOR, g::fonts::FlagESP, false, "Now playing: [MUTED]");
		i::Surface->DrawT(10, position + 16, Color(238, 238, 238, 255), g::fonts::FlagESP, false, BASS::bass_metadata);
	}
	else if (cfg::misc::iRadioStation == 12 && cfg::misc::bEnableRadio) {

		if (!IPT::HandleInput(cfg::misc::iRadioMuteHotKey))
			i::Surface->DrawT(10, position, RYZEXCOLOR, g::fonts::FlagESP, false, "Now playing:");
		else 
			i::Surface->DrawT(10, position, RYZEXCOLOR, g::fonts::FlagESP, false, "Now playing: [MUTED]");
		
		static std::string radio1lmao = "Radio 1 - Csak igazi mai slager megy";
		i::Surface->DrawT(10, position + 16, Color(238, 238, 238, 255), g::fonts::FlagESP, false, radio1lmao.c_str());
	}
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
		drawColor = Color(cfg::misc::flOOF);
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