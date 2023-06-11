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
	bool bSendPacketThisTick = false;
	int iTickCount = 0;
	bool bSetTickCount = false;

	struct
	{
		CBaseEntity*					pAimbotTarget;
		matrix3x4_t*					pTargetMatrix;
		float							flTargetSimulation;
		bool							bCanShoot;
		int								iCommand;
		int								iTargetHitbox;

		void SetTarget(Lagcompensation::LagRecord_t* pRecord, int iHitbox) {

			pAimbotTarget = pRecord->pEntity;
			pTargetMatrix = pRecord->pMatricies[1];
			flTargetSimulation = pRecord->flSimulationTime;
			iTargetHitbox = iHitbox;
		}

	} rageBotData ;

	std::vector<Vector> CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, bool = false);
	std::array<Vector, 6> HitboxPoints(Lagcompensation::LagRecord_t*, CBaseCombatWeapon*, Vector&, int);
	bool bCollidePoint(const Vector&, const Vector&, mstudiobbox_t*, matrix3x4_t*);
	int	SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector&, int iHitbox);

private:

	std::pair<CBaseEntity*, int> __fastcall SelectTargetIndex(CBaseCombatWeapon*, Vector&);
	Vector				Hitscan(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*, Vector);
	Vector				InterpolateLocalEyePosition(Vector, int = 1);
	int					CalculateTickCount(float);
	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal );
	Lagcompensation::LagRecord_t* CheckOnShotRecord(Lagcompensation::AnimationInfo_t* pLog, int&);
	bool				CheckBaimRecord(CBaseEntity* pLocal, Lagcompensation::LagRecord_t* pLog, Vector& vecEyePosition, CBaseCombatWeapon* pWeapon);
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