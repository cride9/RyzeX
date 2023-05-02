#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/Lagcompensation.h"
#define HITBOX_ARRAY std::array<bool, HITBOX_MAX>

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
		int								iCommand;

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
	bool CheckBaimRecord(CBaseEntity* pLocal, Lagcompensation::LagRecord_t* pLog, Vector& vecEyePosition, CBaseCombatWeapon* pWeapon);
	bool				ShouldSendPacket(bool&);

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigOverrideDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	bool				ConfigAutoScope( CBaseCombatWeapon*);
	bool				ConfigForceSafe(CBaseCombatWeapon*);
	bool				ConfigAutoStop( CBaseCombatWeapon*);
	bool				ConfigAutoStopInAir(CBaseCombatWeapon*);
	bool				ConfigAutoStopBetweenShots(CBaseCombatWeapon*);
	bool				ConfigAutoStopAggressiveness(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigMultiHitboxes(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigSafeHitboxes(CBaseCombatWeapon*);

	void				AddHitbox(int index, HITBOX_ARRAY& vecHitboxList);

};
inline CRageBot ragebot;