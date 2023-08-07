#include "../../hooks.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"

enum
{
	INTERPOLATE_STOP = 0,
	INTERPOLATE_CONTINUE
};

// client.dll -> 55 8B EC 51 8B 45 14 56
int __fastcall h::hkBaseInterpolatePart(CBaseEntity* pEntity, void* edx, float& currentTime, Vector& oldOrigin, Vector& oldAngles, int& bNoMoreChanges) {

	//// MoveToLastReceivedPosition -> ida: client.dll -> 55 8B EC 51 53 56 8B F1 32 DB 8B 06
	//// Disable interpolation when those conditions are met
	//bNoMoreChanges = ((int(__thiscall*)(CBaseEntity*, bool))(MEM::FindPattern(CLIENT_DLL, "55 8B EC 51 53 56 8B F1 32 DB 8B 06")))(pEntity, true);
	//return INTERPOLATE_STOP;

	static auto original = detour::baseInterpolatePart.GetOriginal<decltype(&hkBaseInterpolatePart)>();

	VarMapping_t* pVarmap = pEntity->GetVarMap();

	for (int i = 0; i < pVarmap->m_nInterpolatedEntries; i++) {

		VarMapEntry_t* pEntry = &pVarmap->m_Entries[i];
		pEntry->m_bNeedsToInterpolate = true;
	}
	pVarmap->m_lastInterpolationTime = currentTime;

	return original(pEntity, edx, currentTime, oldOrigin, oldAngles, bNoMoreChanges);
}