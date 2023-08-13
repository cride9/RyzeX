#pragma once
#include "Animations/Lagcompensation.h"
#include "../../SDK/Menu/config.h"
#include "../../SDK/InputSystem.h"
#include "../../SDK/Menu/gui.h"
#include "../Misc/enginepred.h"
#include "../../SDK/Entity.h"
#include "exploits.h"
#include "autowall.h"

enum EConditions : int {
	CONDITION_BETWEEN_SHOTS,
	CONDITION_INAIR,
	CONDITION_MAX
};

enum EHitboxType : int {
	NORMAL,
	MULTIPOINT,
	SAFE,
	HITBOXTYPE_MAX
};

struct Hitscan_t {

	Hitscan_t() {}
	Hitscan_t(Lagcompensation::LagRecord_t* _record, Vector& _point, FireBulletData_t& data, bool _baimlethal, bool _safe) {

		pRecord = _record;
		vecPoint = _point;

		bBaimLethal = _baimlethal;
		bSafe = _safe;
		bBacktrack = &lagcomp.GetLog(_record->iEntIndex).pRecord.front() != _record;

		flAnimationVelocity = _record->flAnimationVelocity;
		flDesyncDelta = _record->flDesyncDelta;
		flDamage = data.flCurrentDamage;

		iHitbox = data.enterTrace.iHitbox;
		iHitgroup = data.enterTrace.iHitGroup;
	}

	Lagcompensation::LagRecord_t* pRecord{};

	Vector vecPoint{};

	bool bBaimLethal{};
	bool bSafe{};
	bool bBacktrack{};

	float flAnimationVelocity{};
	float flDesyncDelta{};
	float flDamage{};

	int iHitbox{};
	int iHitgroup{};

	bool operator<(const Hitscan_t& other) const {
		// Sort by baimlethal (true first, false second)
		if (bBaimLethal && !other.bBaimLethal)
			return true;
		if (!bBaimLethal && other.bBaimLethal)
			return false;

		// Sort by bSafe (true first, false second)
		if (bSafe && !other.bSafe)
			return true;
		if (!bSafe && other.bSafe)
			return false;

		// Sort by flDesyncDelta (smaller values first)
		if (flDesyncDelta < other.flDesyncDelta)
			return true;
		if (flDesyncDelta > other.flDesyncDelta)
			return false;

		// Sort by flAnimationVelocity (larger values first)
		return flAnimationVelocity > other.flAnimationVelocity;
	}
};

struct rageBotData_t
{
	Lagcompensation::LagRecord_t* pRecord{};
	CBaseEntity* pAimbotTarget{};
	matrix3x4_t* pTargetMatrix{};
	Vector vecLocalShootPosition{};
	Vector vecTargetShootPosition{};

	int	iHealth{};
	int	iTickcount{};
	int	iHitbox{};
	int	iHitGroup{};

	bool bBacktrack{};
	bool bCanShoot{};

	float flTargetSimulation{};
	float flDamage{};
	float flHitchance{};

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

struct weaponConfig_t {

	CBaseCombatWeapon* pWeapon{};

	int iMinimumDamage{};
	int iHitchance{};
	int iAccuracyBoost{};
	int iAutostopValue{};
	int iHeadScale{};
	int iBodyScale{};

	bool bOverrideDamage{};
	bool bAutoScope{};
	bool bSafePoint{};

	bool bAutostop{};
	bool bConditions[CONDITION_MAX]{};
	std::vector<int> vecHitboxes[HITBOXTYPE_MAX]{};
};

class CAimBot {

public:
	void CreateMove(CUserCmd* pCmd, CBaseEntity* pLocal);
    void ResetAimbotData();
	void PostPrediction(CUserCmd* pCmd, bool& bSendPacket);

	rageBotData_t& GetHitLogData() { return hitlogData; }
	rageBotData_t& GetAimbotData() { return aimData; }
	weaponConfig_t& GetCurrentConfig() { return curConfig; }

private:

	bool bShouldSendPacket = false;
	int iTickCount = 0;

	rageBotData_t aimData;
	rageBotData_t hitlogData;

	weaponConfig_t curConfig;
	Vector vecEyePosition;

	bool HasEnoughAccuracy(CBaseEntity* pLocal, float flWeaponInAccuracy);
	void GetHitBoxes(int i, std::vector<int>& vecOut, int iWeapon);
	weaponConfig_t GetWeaponConfiguration(short iItemDefinitionIndex);
	Vector ScanHitboxes(std::vector<Lagcompensation::AnimationInfo_t*>& vecIn, CBaseEntity* pLocal);
	void AutoStop(CBaseEntity* pLocal, CUserCmd* pCmd);
	bool HitChance(CUserCmd* pCmd, CBaseEntity* pLocal, Vector vecWorldPosition, Vector vecPosition, Lagcompensation::LagRecord_t* pRecord);
	std::vector<Lagcompensation::AnimationInfo_t*> GetTargetableEntities(CBaseEntity* pLocal);
	std::vector<Vector> CreatePoints(Vector, CBaseCombatWeapon*, Lagcompensation::LagRecord_t*, int, EMatrixType = RESOLVE, bool = false);

};
inline CAimBot aimbot;