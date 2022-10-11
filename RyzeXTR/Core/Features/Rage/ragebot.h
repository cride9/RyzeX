#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/lagcompensation.h"

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);

private:

	Vector HitScan(CBaseEntity*, float&, CBaseCombatWeapon*, Vector);

	int		ConfigMinimumDamage(CBaseCombatWeapon*);
	int		ConfigHitChance(CBaseCombatWeapon*);
	std::vector<int> ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);

	bool	CanShoot(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void	AutoStop(CUserCmd*, float);
	Vector	CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, int, Vector);
};
inline CRageBot ragebot;