#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Enums.h"
#include "../Rage/Animations/LocalAnimation.h"

enum ECONFIG : int {

	SEMIRAGE,
	HVH,
	BAIM,
	HEADSHOT,
};

namespace misc {

	inline bool bDefensive = false;
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
	void BulletImpact(IGameEvent* = nullptr, EStage = FRAME_UNDEFINED, bool bFrameStage = false );
	void SlideFix();
	void NightMode();
	void OnlyCheatLogs();
	void IdealTick(CUserCmd*);
	void ServerHitboxes();
	void RemovePostProcessing();
	void FixScopeSens();
	void AutoPistol(CUserCmd* pCmd, CBaseEntity* pLocal);
	void RemoveSmoke();

	void DrawBream(Vector, Vector, Color);

	void Security();

	void EventHandler(IGameEvent*);
	void HandlePlayerHitEffects(IGameEvent*);
	void PreserveKillfeed(IGameEvent*);
	void BuyBot(IGameEvent*);
	void BulletTracer(IGameEvent*);

	void CustomBombText(const char*);

	inline bool bResetNightMode = true;
}