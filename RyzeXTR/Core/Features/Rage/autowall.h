#pragma once
#include "../../SDK/Entity.h"
#include "../../SDK/math.h"

struct FireBulletData_t
{
	Vector			vecPosition = { };
	Vector			vecDirection = { };
	Trace_t			enterTrace = { };
	float			flCurrentDamage = 0.0f;
	int				iPenetrateCount = 0;
};

class CAutoWall {

public:
	/* returns damage at point and simulated bullet data (if given) */
	static float GetDamage(CBaseEntity* pLocal, const Vector& vecPoint, int iHitbox, Vector, FireBulletData_t* pDataOut = nullptr);
	/* calculates damage factor */
	static void ScaleDamage(const int iHitGroup, CBaseEntity* pEntity, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float& flDamage);
	/* simulates fire bullet to penetrate up to 4 walls, return true when hitting player */
	static bool SimulateFireBullet(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, FireBulletData_t& data, int);
	/* Check if we can hit a player */
	static bool CanHitFloatingPoint(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, const Vector& vecPoint, const Vector& vecSource, float flDmg);

private:
	// Main
	static void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, const unsigned int fMask, ITraceFilter* pFilter, Trace_t* pTrace, const float flMinRange = 0.0f);
	static bool TraceToExit(Trace_t& enterTrace, Trace_t& exitTrace, const Vector& vecPosition, const Vector& vecDirection, const CBaseEntity* pClipPlayer);
	static bool HandleBulletPenetration(CBaseEntity* pLocal, const CCSWeaponInfo* pWeaponData, const surfacedata_t* pEnterSurfaceData, FireBulletData_t& data);
};
inline CAutoWall autowall;