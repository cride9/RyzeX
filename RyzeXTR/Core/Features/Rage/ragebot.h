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

	std::array<Vector, 3> CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int);
	std::array<Vector, 6> HitboxPoints(Lagcompensation::LagRecord_t*, CBaseCombatWeapon*, Vector&, int);

private:

	Vector				Hitscan(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector, int);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*, Vector);
	Vector				InterpolateLocalEyePosition(Vector, int = 1);
	bool				SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector&, int iHitbox);
	int					CalculateTickCount(float);
	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal );
	Lagcompensation::LagRecord_t* CheckOnShotRecord(Lagcompensation::AnimationInfo_t* pLog);
	bool				ShouldSendPacket(bool&);

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigOverrideDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	bool				ConfigAutoScope( CBaseCombatWeapon* pWeapon );
	bool				ConfigForceSafe(CBaseCombatWeapon* pWeapon);
	bool				ConfigAutoStop( CBaseCombatWeapon* pWeapon );
	std::array<bool, HITBOX_MAX>	ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
	std::array<bool, HITBOX_MAX> ConfigMultiHitboxes(CBaseCombatWeapon*);
	std::array<bool, HITBOX_MAX> ConfigSafeHitboxes(CBaseCombatWeapon*);
};
inline CRageBot ragebot;