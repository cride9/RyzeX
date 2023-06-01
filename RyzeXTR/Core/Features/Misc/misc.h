#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Enums.h"
#include "../Rage/Animations/LocalAnimation.h"
#include <deque>

enum ECONFIG : int {

	SEMIRAGE,
	HVH,
	BAIM,
	HEADSHOT,
};

namespace misc {

	inline bool bNewRound = false;
	inline bool bDefensive = false;
	inline bool bFilter = false;
	inline bool bRetreat = false;
	inline Vector vecRecord = Vector(0.f, 0.f, 0.f);
	inline Vector vecEyePosition = Vector(0.f, 0.f, 0.f);
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
	void BulletImpact(IGameEvent*);
	void BulletImpactFrameStage(CBaseEntity* pLocal);
	void SlideFix();
	void SkyboxChanger();
	void OnlyCheatLogs();
	void IdealTick(CUserCmd*, CBaseEntity* pLocal);
	void ServerHitboxes();
	void RemovePostProcessing();
	void FixScopeSens();
	void AutoPistol(CUserCmd* pCmd, CBaseEntity* pLocal);
	void RemoveSmoke();
	void WalkBot(CUserCmd*);
	void WalkBotHandler(IGameEvent*);
	void MoveToPosition(Vector&);
	void ClanTag();
	void CapsuleHandler(IGameEvent*);
	void CapsuleOnHit(int, int, Color, float);
	void LeftHandKnife();

	void DrawBream(Vector, Vector, Color);
	void WorldCrosshairHandler(IGameEvent*);

	void Security();

	void EventHandler(IGameEvent*);
	void HandlePlayerHitEffects(IGameEvent*);
	void PreserveKillfeed(IGameEvent*);
	void BuyBot(IGameEvent*);
	void BulletTracer(IGameEvent*);
	void BlockBot(CUserCmd*);

	void CustomBombText(const char*);
	void CheatLog(IGameEvent*);
}