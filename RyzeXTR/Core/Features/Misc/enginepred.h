#pragma once
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"

struct CNetvarData
{
	float flRecoilIndex = 0.f;
	float flAccuracyPenalty = 0.f;
	float flDuckAmount = 0.f;
	float flDuckSpeed = 0.f;
	float flFallVelocity = 0.f;

	int nRenderMode = 0;
	int nTickbase = 0;
	int fFlags = 0;

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
		uPredictionRandomSeed = *reinterpret_cast<int**>(util::FindSignature("client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		pPredictionPlayer = *reinterpret_cast<int**>(util::FindSignature("client.dll", "89 35 ? ? ? ? F3 0F 10 46") + 2);
	}

	void Start(CUserCmd* pCmd, CBaseEntity* pLocal);
	void End(CUserCmd* pCmd, CBaseEntity* pLocal) const;

	static int GetTickBase(CUserCmd* pCmd, CBaseEntity* pLocal);

	CNetvarData GetNetvars( int iCommand ) { return this->pNetvarData[ iCommand % 150 ]; };

	void SaveNetvars( int iCommand , CBaseEntity* pLocal);
	void RestoreNetvars( int iCommand, CBaseEntity* pLocal);
	void SaveViewmodelData(CBaseEntity* pLocal);
	void AdjustViewmodelData(CBaseEntity* pLocal);

	std::array < CNetvarData, 150 > pNetvarData = { };

private:

	int* uPredictionRandomSeed = nullptr;

	int* pPredictionPlayer = nullptr;

	CMoveData moveData = { };

	float flOldCurrentTime = 0.f;
	float flOldFrameTime = 0.f;
	int iOldTickCount = 0;

	float flCycle = 0.f;
	float flAnimTime = 0.f;
	int	iSequence = 0;
	int	iAnimationParity = 0;
};
inline Prediction prediction;