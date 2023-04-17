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
		CBaseEntity*					pAimbotTarget;
		matrix3x4_t*					pTargetMatrix;
		float							flTargetSimulation;
		Vector							vecOldViewAngles;
		bool							bCanShoot;

	} rageBotData ;

	std::vector<Vector>	CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, Vector, bool = true);

private:

	std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*>	SelectTarget(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	Vector				Hitscan(CBaseEntity*, std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*>, CBaseCombatWeapon*, Vector&, int&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector, int);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*);
	Vector				InterpolateLocalEyePosition(Vector, int = 1);
	bool				SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector&, float&);
	int					CalculateTickCount(float);
	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal );

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigOverrideDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	bool				ConfigAutoScope( CBaseCombatWeapon* pWeapon );
	bool				ConfigForceSafe(CBaseCombatWeapon* pWeapon);
	bool				ConfigAutoStop( CBaseCombatWeapon* pWeapon );
	std::vector<int>	ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
	std::array<bool, HITBOX_MAX> ConfigMultiHitboxes(CBaseCombatWeapon*);
	std::array<bool, HITBOX_MAX> ConfigSafeHitboxes(CBaseCombatWeapon*);
};
inline CRageBot ragebot;