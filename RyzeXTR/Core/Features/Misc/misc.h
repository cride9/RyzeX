#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Enums.h"

enum ECONFIG : int {

	SEMIRAGE,
	HVH,
	BAIM,
	HEADSHOT,
};

namespace misc {

	inline bool bFilter = false;
	inline bool bRetreat = false;
	inline Vector vecRecord = Vector(0.f, 0.f, 0.f);
	inline matrix3x4_t matrixRecord[128];

	void CreateMove(CUserCmd*, Vector&, bool&);

	void BunnyHop(CUserCmd* pCmd);
	void FakeLag(bool&);
	void ThirdPerson();
	void MovementFix(CUserCmd*, Vector&);
	void AutoStrafe(Vector&, CUserCmd*);
	void AspectRatio();
	void Slowwalk(CUserCmd*, float);
	void FastStop(CUserCmd*);
	void FakeDuck(CUserCmd*);
	void ViewModel();
	void BulletImpact();
	void SlideFix();
	void DefensiveDoubletap();
	void NightMode();
	void OnlyCheatLogs();
	void IdealTick(CUserCmd*);

	void Security();

	void PreserveKillfeed(IGameEvent*);
	void BuyBot(IGameEvent*);

	void CustomBombText(const char*);

	inline bool bResetNightMode = true;
}