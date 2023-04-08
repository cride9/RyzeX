#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/Lagcompensation.h"

struct aimbotData_t {


};

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);

	struct
	{
		CBaseEntity*					aimbotTarget;
		matrix3x4_t*					targetMatrix;
		float							flTargetSimulation;
		Lagcompensation::LagRecord_t*	backtrackRecord;
	} rageBotData ;

	Vector vecOldViewAngles = Vector(0, 0, 0);

private:

	std::vector<Vector>	CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, int, Vector);
	CBaseEntity*		SelectTarget(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	Vector				Hitscan(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void				AutoStop(CUserCmd*, float);

	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal );

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigOverrideDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	int					ConfigAutoScope( CBaseCombatWeapon* pWeapon );
	std::vector<int>	ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
	std::array<bool, HITBOX_MAX> ConfigMultiHitboxes(CBaseCombatWeapon*);
};
inline CRageBot ragebot;