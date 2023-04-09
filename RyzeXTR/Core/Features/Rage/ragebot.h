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
		Lagcompensation::LagRecord_t*	pBacktrackRecord;
		Vector							vecOldViewAngles;
		bool							bCanShoot;

	} rageBotData ;

	

private:

	std::vector<Vector>	CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, int, Vector);
	CBaseEntity*		SelectTarget(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	Vector				Hitscan(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*);
	Vector				InterpolateLocalEyePosition(Vector vecEyePosition, int iInterpolateTick = 1);

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