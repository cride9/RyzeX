#include "../hooks.h"

int __fastcall h::hkGetObserverMode(CBaseEntity* ecx, void* edx) {

	static auto original = detour::getObserverMode.GetOriginal<decltype(&h::hkGetObserverMode)>();

	if (!ecx || !g::pLocal)
		return original(ecx, edx);

	if (ecx == g::pLocal)
		return OBS_MODE_CHASE;
}