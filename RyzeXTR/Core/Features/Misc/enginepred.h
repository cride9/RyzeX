#pragma once
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"

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
private:

	int* uPredictionRandomSeed = nullptr;

	int* pPredictionPlayer = nullptr;

	CMoveData moveData = { };

	float flOldCurrentTime = 0.f;
	float flOldFrameTime = 0.f;
	int iOldTickCount = 0;
};
inline Prediction prediction;