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
		int								iTargetedHitbox;
		int								iTickCount;

		void SetTarget(Lagcompensation::LagRecord_t* pRecord, int iTargetHitbox) {

			pAimbotTarget = pRecord->pEntity;
			pTargetMatrix = pRecord->pMatrix;
			flTargetSimulation = pRecord->flSimulationTime;
			iTargetedHitbox = iTargetHitbox;
		}

	} rageBotData ;

	Vector	CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, Vector, bool = true);

private:

	Vector				Hitscan(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector, int);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*, Vector);
	Vector				InterpolateLocalEyePosition(Vector, int = 1);
	bool				SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector&, float&);
	int					CalculateTickCount(float);
	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal );
	bool				ShouldSendPacket(bool&);

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