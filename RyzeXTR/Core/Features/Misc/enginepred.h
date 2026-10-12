#pragma once
#include "../Rage/Animations/LocalAnimation.h"
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"

struct CNetvarData
{
	int command = 0;
	std::uint32_t epoch = 0;
	CBaseHandle localHandle = 0, weaponHandle = 0, groundHandle = 0;
	float spawnTime = 0.f;
	int eflags = 0, buttons = 0, buttonLast = 0, buttonPressed = 0, buttonReleased = 0, nextThink = 0;
	float surfaceFriction = 0.f, velocityModifier = 0.f, nextAttack = 0.f;
	float nextPrimaryAttack = 0.f, nextSecondaryAttack = 0.f;
	Vector absOrigin{}, absVelocity{};
	float flRecoilIndex = 0.f;
	float flAccuracyPenalty = 0.f;
	float flDuckAmount = 0.f;
	float flDuckSpeed = 0.f;
	float flFallVelocity = 0.f;

	int nRenderMode = 0;
	int nTickbase = 0;
	int iFlags = 0;

	Vector vecOrigin = Vector( 0.f, 0.f, 0.f );
	Vector vecVelocity = Vector( 0.f, 0.f, 0.f );
	Vector vecBaseVelocity = Vector( 0.f, 0.f, 0.f );
	Vector vecViewOffset = Vector( 0.f, 0.f, 0.f );

	Vector vecAimPunchAngleVel = Vector( 0.f, 0.f, 0.f );
	Vector vecAimPunchAngle = Vector( 0.f, 0.f, 0.f );
	Vector vecViewPunchAngle = Vector( 0.f, 0.f, 0.f );
};

enum EThinkMethods : int
{
	THINK_FIRE_ALL_FUNCTIONS = 0,
	THINK_FIRE_BASE_ONLY,
	THINK_FIRE_ALL_BUT_BASE,
};

class Prediction {

public:
	Prediction() {
		uPredictionRandomSeed = *reinterpret_cast<int**>(MEM::FindPattern(CLIENT_DLL, XorStr("8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04")) + 2);
		pPredictionPlayer = *reinterpret_cast<CBaseEntity***>(MEM::FindPattern(CLIENT_DLL, XorStr("89 35 ? ? ? ? F3 0F 10 48 20")) + 0x2);
	}

	void Start(CUserCmd* pCmd, CBaseEntity* pLocal, int SequenceNumber );
	void End(CUserCmd* pCmd, CBaseEntity* pLocal);
	void RequestReprediction();

	CNetvarData GetNetvars( int iCommand ) { return this->pNetvarData[ iCommand % 150 ]; };

	void SaveNetvars( int iCommand , CBaseEntity* pLocal);
	void RestoreNetvars( int iCommand, CBaseEntity* pLocal);
	void SaveViewmodelData(CBaseEntity* pLocal);
	void AdjustViewmodelData(CBaseEntity* pLocal);

	float flSpread = 0.f;
	float flInaccuracy = 0.f;

	std::array < CNetvarData, 150 > pNetvarData = { };
	CUserCmd* pLastCmd = nullptr;

    void ResetData() {
        iObservedSimulationOffset = 0;
        pNetvarData = {};
        pLastCmd = nullptr;
        viewmodelHandle = 0;
    }
	void OnFrameStage(CBaseEntity* pLocal);

private:

	int iObservedSimulationOffset = 0;
	int* uPredictionRandomSeed = nullptr;

	CBaseEntity** pPredictionPlayer = nullptr;

	CMoveData moveData = { };
	bool active = false;
	int oldTickbase = 0, oldSeed = -1;
	CUserCmd* oldCommand = nullptr;
	CUserCmd oldLastCommand;
	CBaseEntity* oldPredictionPlayer = nullptr;
	CBaseHandle viewmodelHandle = 0;
	std::uint32_t viewmodelEpoch = 0;

	float flOldCurrentTime = 0.f;
	float flOldFrameTime = 0.f;
	int iOldTickCount = 0;

	float flCycle = 0.f;
	float flAnimTime = 0.f;
	int	iSequence = 0;
	int	iAnimationParity = 0;
};
inline Prediction prediction;
