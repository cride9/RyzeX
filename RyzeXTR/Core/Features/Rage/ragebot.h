#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"

struct aimbotData_t {


};

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);

private:

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	std::vector<int>	ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
};
inline CRageBot ragebot;