#include "../../hooks.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"

enum
{
	INTERPOLATE_STOP = 0,
	INTERPOLATE_CONTINUE
};

// client.dll -> 55 8B EC 51 8B 45 14 56
int __fastcall h::hkBaseInterpolatePart(CBaseEntity* pEntity, void* edx, float& currentTime, Vector& oldOrigin, Vector& oldAngles, int& bNoMoreChanges) {

	static auto original = detour::baseInterpolatePart.GetOriginal<decltype(&hkBaseInterpolatePart)>();

	if (!pEntity || !pEntity->IsPlayer() || pEntity->IsDormant() || !pEntity->IsAlive() || pEntity == g::pLocal)
		return original(pEntity, edx, currentTime, oldOrigin, oldAngles, bNoMoreChanges);

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEntity->EntIndex());
	if (!pLog || pLog->pRecord.size() < 2)
		return original(pEntity, edx, currentTime, oldOrigin, oldAngles, bNoMoreChanges);
	
	// MoveToLastReceivedPosition -> ida: client.dll -> 55 8B EC 51 53 56 8B F1 32 DB 8B 06
	// Disable interpolation when those conditions are met
	// using fn = int(__thiscall*)(CBaseEntity*, bool)

	return original(pEntity, edx, pLog->pRecord.front().flSimulationTime, oldOrigin, oldAngles, bNoMoreChanges);
}