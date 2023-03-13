#include "ragebot.h"
#include "../../globals.h"
#include "autowall.h"
#include "../../SDK/math.h"
#include "../Visuals/ESP.h"
#include "doubletap.h"
#include "Animations/resolver.h"

bool CheckShootingCondition(CUserCmd* pCmd);

void CRageBot::CreateMove(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket) {

	if (!pLocal || !cfg::rage::enable)
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (!pWeapon || !pWeapon->GetCSWpnData())
		return;

	for (auto pEntity : g::entityListener.vecEntities) {


	}
}

bool LowestHealth(std::pair<CBaseEntity*, int> first, std::pair<CBaseEntity*, int> second) {

	return first.second < first.second;
}

std::pair<int, int> CRageBot::ConfigMultipoint(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return std::make_pair(cfg::rage::autoHeadPoints, cfg::rage::autoBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return std::make_pair(cfg::rage::scoutHeadPoints, cfg::rage::scoutBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return std::make_pair(cfg::rage::awpHeadPoints, cfg::rage::awpBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return std::make_pair(cfg::rage::heavypistolHeadPoints, cfg::rage::heavypistolBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return std::make_pair(cfg::rage::pistolHeadPoints, cfg::rage::pistolBodyPoints);
	}
	else {
		return std::make_pair(cfg::rage::etcHeadPoints, cfg::rage::etcBodyPoints);
	}
}

int CRageBot::ConfigMinimumDamage(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::autoMindmg;
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::scoutMindmg;
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::awpMindmg;
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::heavypistolMindmg;
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::pistolMindmg;
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		return 100;
	}
	else {
		return cfg::rage::etcMindmg;
	}

}

int CRageBot::ConfigHitChance(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::autoHitchance;
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::scoutHitchance;
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::awpHitchance;
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::heavypistolHitchance;
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::pistolHitchance;
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		return 72;
	}
	else {
		return cfg::rage::etcHitchance;
	}
}

std::vector<int> CRageBot::ConfigHitboxes(CBaseCombatWeapon* pWeapon) {

	// "head", "upper chest", "lower chest", "stomach", "arms", "legs"

	std::vector<int> vecHitboxes;
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	static auto AddHitbox = [](int index, std::vector<int>& vecHitboxList) {

		if (index == 0) {
			vecHitboxList.push_back(HITBOX_HEAD);
		}
		if (index == 1) {
			vecHitboxList.push_back(HITBOX_UPPER_CHEST);
		}
		if (index == 2) {
			vecHitboxList.push_back(HITBOX_CHEST);
			vecHitboxList.push_back(HITBOX_THORAX);
		}
		if (index == 3) {
			vecHitboxList.push_back(HITBOX_STOMACH);
			vecHitboxList.push_back(HITBOX_PELVIS);
		}
		if (index == 4) {
			vecHitboxList.push_back(HITBOX_RIGHT_FOREARM);
			vecHitboxList.push_back(HITBOX_LEFT_FOREARM);

			vecHitboxList.push_back(HITBOX_LEFT_UPPER_ARM);
			vecHitboxList.push_back(HITBOX_RIGHT_UPPER_ARM);

			vecHitboxList.push_back(HITBOX_RIGHT_HAND);
			vecHitboxList.push_back(HITBOX_LEFT_HAND);
		}
		if (index == 5) {
			vecHitboxList.push_back(HITBOX_RIGHT_THIGH);
			vecHitboxList.push_back(HITBOX_LEFT_THIGH);

			vecHitboxList.push_back(HITBOX_RIGHT_CALF);
			vecHitboxList.push_back(HITBOX_LEFT_CALF);

			vecHitboxList.push_back(HITBOX_RIGHT_FOOT);
			vecHitboxList.push_back(HITBOX_LEFT_FOOT);
		}
	};

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		for (int i = 0; i < sizeof(cfg::rage::autoHitboxes); i++) {
			if (cfg::rage::autoHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		for (int i = 0; i < sizeof(cfg::rage::scoutHitboxes); i++) {
			if (cfg::rage::scoutHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		for (int i = 0; i < sizeof(cfg::rage::awpHitboxes); i++) {
			if (cfg::rage::awpHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		for (int i = 0; i < sizeof(cfg::rage::heavypistolHitboxes); i++) {
			if (cfg::rage::heavypistolHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		for (int i = 0; i < sizeof(cfg::rage::pistolHitboxes[i]); i++) {
			if (cfg::rage::pistolHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		AddHitbox(2, vecHitboxes);
		AddHitbox(3, vecHitboxes);
	}
	else {
		for (int i = 0; i < sizeof(cfg::rage::etcHitboxes); i++) {
			if (cfg::rage::etcHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}

	return vecHitboxes;
}

bool CheckShootingCondition(CUserCmd* pCmd) {

	auto local = g::pLocal;
	auto pWeapon = g::pLocal->GetWeapon();

	if (!local || !pWeapon)
		return false;

	auto time = TICKS_TO_TIME(local->GetTickBase());

	const auto info = pWeapon->GetCSWpnData();

	if (!info)
		return false;

	const auto is_zeus = pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_TASER;

	if (pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_C4)
		return false;

	if (pWeapon->GetAmmo() < 1)
		return false;

	if (pWeapon->GetNextPrimaryAttack() > time || pWeapon->GetNextSecondaryAttack() > time)
		return false;

	return true;
}