#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/lagcompensation.h"

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);

private:

	Vector HitScan(CBaseEntity*, float&, CBaseCombatWeapon*);

	int		ConfigMinimumDamage(CBaseCombatWeapon*);
	int		ConfigHitChance(CBaseCombatWeapon*);
	std::vector<int> ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);

	bool	CanShoot(CBaseEntity*, CBaseCombatWeapon*, Vector, int);
	void	AutoStop(CUserCmd*, float);
	Vector	CreatePoints(Vector, float, int, int);
};
inline CRageBot ragebot;