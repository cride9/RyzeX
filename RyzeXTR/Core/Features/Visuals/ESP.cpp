#include "ESP.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../Misc/misc.h"

void visual::VisualRender() {

	for (int i = 0; i < 65; i++) {

		CBaseEntity* pEnt = (CBaseEntity*)i::EntityList->GetClientEntity(i);

		if (!pEnt || !g::pLocal)
			continue;

		if (!pEnt->IsAlive() || pEnt->IsDormant())
			continue;

		if (!pEnt->GetHealth())
			continue;

		matrix3x4_t boneMatrix[128];
		if (!pEnt->SetupBones(boneMatrix, 128, 0x7FF00, i::GlobalVars->flCurrentTime))
			continue;

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

		if (pEnt->GetTeam() != g::pLocal->GetTeam()) { // Enemy

			if (!cfg::visual::enemyEsp)
				continue;

			int enemySpacing = -2;

			if (cfg::visual::enemyName) NameEsp(enemySpacing, left, top.y, right, bot.y, w, h, pEnt, Color(cfg::visual::enemyNameColor));
			if (cfg::visual::enemyBox) BoxEsp(left, top.y, right, bot.y, Color(cfg::visual::enemyBoxColor));
			if (cfg::visual::enemyHealth) HealthEsp(enemySpacing, left, top.y, right, bot.y, w, h, pEnt->GetHealth());
			if (cfg::visual::enemyArmor) KevlarEsp(enemySpacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::enemyArmorColor));
			if (cfg::visual::enemyMoney) MoneyEsp(enemySpacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::enemyMoneyColor));
			if (cfg::visual::enemyAmmo) AmmoEsp(enemySpacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::enemyAmmoColor));
			if (cfg::visual::enemyWeapon) WeaponEsp(enemySpacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::enemyWeaponColor));
			if (cfg::visual::enemyBreakLC ) BreakLCESP(enemySpacing, left, top.y, right, bot.y, pEnt);
		}
		else {

			if (pEnt == g::pLocal) { // Local

				if (!cfg::visual::localEsp)
					continue;

				int localspacing = -2;

				if (cfg::visual::localName) NameEsp(localspacing, left, top.y, right, bot.y, w, h, pEnt, Color(cfg::visual::localNameColor[0], cfg::visual::localNameColor[1], cfg::visual::localNameColor[2], cfg::visual::localNameColor[3]));
				if (cfg::visual::localBox) BoxEsp(left, top.y, right, bot.y, Color(cfg::visual::localBoxColor[0], cfg::visual::localBoxColor[1], cfg::visual::localBoxColor[2], cfg::visual::localBoxColor[3]));
				if (cfg::visual::localHealth) HealthEsp(localspacing, left, top.y, right, bot.y, w, h, pEnt->GetHealth());
				if (cfg::visual::localArmor) KevlarEsp(localspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::localArmorColor));
				if (cfg::visual::localMoney) MoneyEsp(localspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::localMoneyColor));
				if (cfg::visual::localAmmo) AmmoEsp(localspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::localAmmoColor));
				AutoPeekCircle();

				continue;
			}
			if (!cfg::visual::teamEsp)
				continue;

			int teamspacing = -2;

			// Teammate
			if (cfg::visual::teamName) NameEsp(teamspacing, left, top.y, right, bot.y, w, h, pEnt, Color(cfg::visual::teamNameColor[0], cfg::visual::teamNameColor[1], cfg::visual::teamNameColor[2], cfg::visual::teamNameColor[3]));
			if (cfg::visual::teamBox) BoxEsp(left, top.y, right, bot.y, Color(cfg::visual::teamBoxColor[0], cfg::visual::teamBoxColor[1], cfg::visual::teamBoxColor[2], cfg::visual::teamBoxColor[3]));
			if (cfg::visual::teamHealth) HealthEsp(teamspacing, left, top.y, right, bot.y, w, h, pEnt->GetHealth());
			if (cfg::visual::teamArmor) KevlarEsp(teamspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::teamArmorColor));
			if (cfg::visual::teamMoney) MoneyEsp(teamspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::teamMoneyColor));
			if (cfg::visual::teamAmmo) AmmoEsp(teamspacing, left, top.y, right, bot.y, pEnt, Color(cfg::visual::teamAmmoColor));
		}
	}
}

//void visual::DrawMultipoints(CBaseEntity* pEnt) {
//
//	for (int nHitbox = 0; nHitbox < multiPoints[pEnt->EntIndex()][nHitbox].size(); nHitbox++) {
//
//		if (multiPoints[pEnt->EntIndex()][nHitbox].size() > nHitbox == HITBOX_HEAD ? 6 : 9)
//			multiPoints[pEnt->EntIndex()][nHitbox].pop_back();
//
//		for (Vector vecPosition : multiPoints[pEnt->EntIndex()][nHitbox]) {
//
//			Vector dotPosition;
//			i::DebugOverlay->ScreenPosition(vecPosition, dotPosition);
//
//			i::Surface->DrawSetColor(255.f, 255.f, 255.f, 255.f);
//			i::Surface->DrawFilledRect(dotPosition.x - 5, dotPosition.y - 5, dotPosition.x + 5, dotPosition.y + 5);
//		}
//	}
//}

void visual::BoxEsp(int left, int top, int right, int bot, Color color) {

	i::Surface->DrawSetColor(color[0], color[1], color[2], color[3]);
	i::Surface->DrawOutlinedRect(left, top, right, bot);

	i::Surface->DrawSetColor(0, 0, 0, color[3]);
	i::Surface->DrawOutlinedRect(left - 1, top - 1, right + 1, bot + 1);
}

void visual::HealthEsp(int& spacing, int left, int top, int right, int bot, int width, int height, int health) {

	const float percentage = health / 100.f;
	std::string text = "Health: ";
	text += std::to_string(health);

	i::Surface->DrawT(right + 2, top + spacing, Color((1.f - percentage) * 1.f, 1.f * percentage, 0.f), g::fonts::NameESP, false, text.c_str());

	spacing += 10;
}

void visual::NameEsp(int& spacing, int left, int top, int right, int bot, int width, int height, CBaseEntity* pEnt, Color color) {

	PlayerInfo_t info = { };

	if (!i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info))
		return;

	i::Surface->DrawT(left, top - 13, Color{ color[0], color[1], color[2], color[3] }, g::fonts::NameESP, false, info.szName);
}

void visual::KevlarEsp(int& spacing, int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	std::string text = "Kevlar: [";
	text += std::to_string(pEnt->GetArmor());
	text += "]";

	i::Surface->DrawT(right + 2, top + spacing, color, g::fonts::NameESP, false, text.c_str());

	spacing += 10;
}

void visual::AmmoEsp(int& spacing, int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	std::string text = "[ ";

	if (!pEnt->GetWeapon())
		return;

	text += std::to_string(pEnt->GetWeapon()->GetAmmo());
	text += "/";
	text += std::to_string(pEnt->GetWeapon()->GetAmmoReserve());
	text += " ]";

	//auto maxbullet = pEnt->GetWeapon()->GetCSWpnData()->iMaxClip1;

	i::Surface->DrawT(right + 2, top + spacing, color, g::fonts::NameESP, false, text.c_str());

	spacing += 10;
}

void visual::BreakLCESP( int& spacing, int left, int top, int right, int bot, CBaseEntity* pEnt ) 
{
	//if ( !lagcomp.IsBreakingLagcompensation( pEnt ) )
	//	return;

	i::Surface->DrawT( left, bot, Color{ 255, 255, 255, 255 }, g::fonts::NameESP, false, "Breaking Lagcomp" );
}

void visual::WeaponEsp(int& spacing, int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	if (!pEnt->GetWeapon())
		return;

	auto pWeaponInfo = pEnt->GetWeapon()->GetCSWpnData();

	if (!pWeaponInfo)
		return;

	std::string text = pWeaponInfo->szWeaponName;
	text.erase(0, 7);

	i::Surface->DrawT(left, bot, Color{ color[0], color[1], color[2], color[3] }, g::fonts::NameESP, false, text.c_str());

	spacing += 10;
}

void visual::MoneyEsp(int& spacing, int left, int top, int right, int bot, CBaseEntity* pEnt, Color color) {

	std::string text = "$";
	
	text += std::to_string(pEnt->GetMoney());

	i::Surface->DrawT(right + 2, top + spacing, color, g::fonts::NameESP, false, text.c_str());

	spacing += 10;
}

void visual::AutoPeekCircle() {

	//if (misc::vecRecord == Vector(0.f, 0.f, 0.f))
	//	return;

	//Vector vecDrawPosition;
	//i::DebugOverlay->ScreenPosition(misc::vecRecord - Vector{ 0.f, 0.f, 9.f }, vecDrawPosition);

	//i::Surface->DrawSetColor(255.f, 255.f, 255.f, 255.f);
	//i::Surface->DrawFilledRect(vecDrawPosition.x - 20, vecDrawPosition.y - 20, vecDrawPosition.x + 20, vecDrawPosition.y + 20);

}