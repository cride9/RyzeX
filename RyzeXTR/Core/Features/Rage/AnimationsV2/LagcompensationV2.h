#pragma once
#include <deque>
#include "../../../SDK/Entity.h"
#define ZERO Vector(0, 0, 0)
#define array std::array

static enum EMatrixSides {

	MVISUAL,
	MRESOLVED,
	MLEFT,
	MRIGHT,
	MCENTER,
	MMATRIXCOUNT
};

static enum ELayerSides {

	LSERVER,
	LLEFT,
	LRIGHT,
	LCENTER,
	LLAYERCOUNT
};

struct entityRecord_t {

	entityRecord_t(CBaseEntity* pEntity);
	entityRecord_t(entityRecord_t* pRecord);

	CBaseEntity* pEntity;

	/* bools */
	bool bValid = false;
	bool bLagcompBreak = false;
	bool bDormant = false;
	bool bDidShot = false;

	/* vectors */
	Vector vecVelocity = ZERO;
	Vector vecAbsVelocity = ZERO;
	Vector vecOrigin = ZERO;
	Vector vecAbsOrigin = ZERO;
	Vector vecMins = ZERO;
	Vector vecMaxs = ZERO;
	Vector vecEyeAngles = ZERO;
	Vector vecAbsAngles = ZERO;

	/* floats */
	float flSimulationTime = 0.f;
	float flOldSimulationTime = 0.f;
	float flDuckAmount = 0.f;
	float flLowerBodyYawTarget = 0.f;
	float flSpawnTime = 0.f;
	
	/* ints */
	int iFlags = 0;
	int iEFlags = 0;
	int iEffects = 0;
	int iChokeAmount = 0;

	/* data */
	array<matrix3x4_t[MAXSTUDIOBONES], MMATRIXCOUNT> pMatricies{};
	array<CAnimationLayer[ANIMATION_LAYER_COUNT], LLAYERCOUNT> pLayers{};
	array<float, MAXSTUDIOPOSEPARAM> pPoses{};

	/* Whole animstate backup for accurate animation information */
	CAnimState mAnimstate;
};

class LagcompensationV2 {

public:

	/* AnimationFix, Ragebot data handling */
	void FrameStageNotify(EStage iStage);

	/* Backup data each tick */
	void BeginLagcompensation();

	/* Restore data each tick to not mess with client animations */
	void EndLagcompensation();

private:

	void ValidData();
	void RemoveInterpolation(CBaseEntity* pEntity);
};