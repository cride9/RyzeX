#pragma once
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"
#include "Animations/Lagcompensation.h"
#define HITBOX_ARRAY std::array<bool, HITBOX_MAX>

struct Hitscan_t {

	Hitscan_t() {

	}
	Hitscan_t(Lagcompensation::LagRecord_t* recordPointer, Vector hitboxVector, float damageFloat, int hitboxInt, int hitgroupInt, bool safeBool, bool lethalBool, bool backtrackBool, bool headBool) {
		pRecord = recordPointer;
		vecPoint = hitboxVector;
		flDamage = damageFloat;
		iHitbox = hitboxInt;
		iHitgroup = hitgroupInt;
		bSafe = safeBool;
		bLethal = lethalBool;
		bBacktrack = backtrackBool;
		bHead = headBool;
	}

	Lagcompensation::LagRecord_t* pRecord = nullptr;
	Vector vecPoint = Vector();
	float flDamage = 0;
	int iHitbox = 0;
	int iHitgroup = 0;
	bool bSafe = false;
	bool bLethal = false;
	bool bBacktrack = false;
	bool bHead = false;
};

class CRageBot {

public:

	void CreateMove(CUserCmd*, CBaseEntity*, bool&);
	bool bSendPacketThisTick;
	int iTickCount = 0;
	bool bSetTickCount = false;
	std::vector<std::pair<Vector, Lagcompensation::LagRecord_t*>> vecSafePoints{};

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

		void SetTarget(Lagcompensation::LagRecord_t* _pRecord, Vector vecEyePosition, bool _bBacktrack) {

			pRecord = _pRecord;
			pAimbotTarget = _pRecord->pEntity;
			pTargetMatrix = _pRecord->pMatricies[1];
			flTargetSimulation = _pRecord->flSimulationTime;
			iHealth = _pRecord->pEntity->GetHealth();
			vecLocalShootPosition = vecEyePosition;
			bBacktrack = _bBacktrack;
		}

		void ClearTarget() {

			pRecord = nullptr;
			pAimbotTarget = nullptr;
			pTargetMatrix = nullptr;
			flTargetSimulation = 0.f;
			iHealth = -1;
			vecLocalShootPosition = Vector(0, 0, 0);
			bBacktrack = false;
			flHitchance = 0.f;
		}

	};
	rageBotData_t rageBotData;
	rageBotData_t hitlogData;

	std::vector<Vector> CreatePoints(CBaseEntity*, Lagcompensation::LagRecord_t*, CBaseCombatWeapon*, int, bool = false);
	std::vector<Vector> CreatePoints(Vector, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, int, EMatrixType = RESOLVE);
	std::array<Vector, 6> HitboxPoints(Lagcompensation::LagRecord_t*, CBaseCombatWeapon*, Vector&, int);
	bool bCollidePoint(const Vector&, const Vector&, mstudiobbox_t*, matrix3x4_t*);
	int	SafePoint(Vector&, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, Vector, int iHitbox, bool bOnlyIntersectFind = false);

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
	void				CapsuleRebuild(mstudiobbox_t* studioBox, matrix3x4_t* pMatrix, std::vector<Vector>* vecPointsOut = nullptr);

	int					ConfigMinimumDamage(CBaseCombatWeapon*);
	int					ConfigOverrideDamage(CBaseCombatWeapon*);
	int					ConfigHitChance(CBaseCombatWeapon*);
	bool				ConfigAutoScope( CBaseCombatWeapon*);
	bool				ConfigForceSafe(CBaseCombatWeapon*);
	bool				ConfigAutoStop( CBaseCombatWeapon*);
	bool				ConfigAutoStopInAir(CBaseCombatWeapon*);
	bool				ConfigAutoStopBetweenShots(CBaseCombatWeapon*);
	int					ConfigAutoStopAggressiveness(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigHitboxes(CBaseCombatWeapon*);
	std::pair<int, int> ConfigMultipoint(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigMultiHitboxes(CBaseCombatWeapon*);
	HITBOX_ARRAY		ConfigSafeHitboxes(CBaseCombatWeapon*);

	void				AddHitbox(int index, HITBOX_ARRAY& vecHitboxList);
	void				CapsuleRebuild(Lagcompensation::LagRecord_t* pRecord, int iHitbox);
};
inline CRageBot ragebot;