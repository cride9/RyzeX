#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/Lagcompensation.h"
#define HITBOX_ARRAY std::array<bool, HITBOX_MAX>

struct Hitscan_t {

	Hitscan_t() {

	}
	Hitscan_t(Lagcompensation::LagRecord_t* recordPointer, Vector hitboxVector, float damageFloat, int hitboxInt, int hitgroupInt, bool safeBool, bool lethalBool, bool backtrackBool) {
		pRecord = recordPointer;
		vecPoint = hitboxVector;
		flDamage = damageFloat;
		iHitbox = hitboxInt;
		iHitgroup = hitgroupInt;
		bSafe = safeBool;
		bLethal = lethalBool;
		bBacktrack = backtrackBool;
		bHead = ((iHitbox == HITBOX_HEAD || HITBOX_NECK) && iHitgroup == HITGROUP_HEAD);
	}

	Lagcompensation::LagRecord_t* pRecord;
	Vector vecPoint;
	float flDamage;
	int iHitbox;
	int iHitgroup;
	bool bSafe;
	bool bLethal;
	bool bBacktrack;
	bool bHead;
};

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);
	bool bSendPacketThisTick;
	int iTickCount = 0;
	bool bSetTickCount = false;

	struct rageBotData_t
	{
		Lagcompensation::LagRecord_t*	pRecord;
		CBaseEntity*					pAimbotTarget;
		matrix3x4_t*					pTargetMatrix;
		Vector							vecLocalShootPosition;

		int								iHealth;
		int								iTickcount;
		int								iHitbox;
		int								iHitGroup;

		bool							bBacktrack;
		bool							bCanShoot;

		float							flTargetSimulation;
		float							flDamage;
		float							flHitchance;
		float							flResolveAngle;

		void SetTarget(Lagcompensation::LagRecord_t* _pRecord, Vector vecEyePosition, bool _bBacktrack, float _flResolveAngle) {

			pRecord = _pRecord;
			pAimbotTarget = _pRecord->pEntity;
			pTargetMatrix = _pRecord->pMatricies[1];
			flTargetSimulation = _pRecord->flSimulationTime;
			iHealth = _pRecord->pEntity->GetHealth();
			vecLocalShootPosition = vecEyePosition;
			bBacktrack = _bBacktrack;
			flResolveAngle = _flResolveAngle;
		}

		void ClearTarget() {

			pRecord = nullptr;
			pAimbotTarget = nullptr;
			pTargetMatrix = nullptr;
			flTargetSimulation = 0.f;
			iHealth = -1;
			vecLocalShootPosition = Vector(0, 0, 0);
			bBacktrack = false;
			flResolveAngle = 0.f;
			flHitchance = 0.f;
		}

	};
	rageBotData_t rageBotData;
	rageBotData_t hitlogData;

	std::vector<Vector> CreatePoints(CBaseEntity*, CBaseEntity*, CBaseCombatWeapon*, Vector, float, int, bool = false);
	std::vector<Vector> CreatePoints(Vector, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, int);
	std::array<Vector, 6> HitboxPoints(Lagcompensation::LagRecord_t*, CBaseCombatWeapon*, Vector&, int);
	bool bCollidePoint(const Vector&, const Vector&, mstudiobbox_t*, matrix3x4_t*);
	int	SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector, int iHitbox);

private:

	std::vector<CBaseEntity*> vecTargets{nullptr};

	void				SelectTargets(CBaseEntity*);
	Vector				Hitscan(CBaseEntity*, CBaseCombatWeapon*, Vector&);
	bool				Hitchance(CBaseEntity*, CBaseCombatWeapon*, Vector, int, Vector);
	void				AutoStop(CBaseEntity*, CBaseCombatWeapon*, CBaseEntity*, CUserCmd*, Vector);
	Vector				InterpolateLocalEyePosition(Vector, int = 1);
	int					CalculateTickCount(float);
	// helpers
	bool				CheckShootingCondition( CUserCmd* pCmd, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	bool				CheckBaimRecord(CBaseEntity* pLocal, Lagcompensation::LagRecord_t* pLog, Vector& vecEyePosition, CBaseCombatWeapon* pWeapon);
	bool				ShouldSendPacket(bool&);
	void				CapsuleRebuild(mstudiobbox_t* studioBox, matrix3x4_t* pMatrix);

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