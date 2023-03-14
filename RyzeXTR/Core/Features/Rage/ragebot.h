#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"

struct aimbotData_t {


};

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);

private:

	CBaseEntity*		SelectTarget(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	Vector				Hitscan(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void				AutoStop(CUserCmd*, float);

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	std::vector<int>	ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
};
inline CRageBot ragebot;