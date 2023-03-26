#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkInterpolateServerEntites(void* ecx, void* edx) {

	static auto original = detour::interpolateEntites.GetOriginal<decltype(&h::hkInterpolateServerEntites)>();

	if (!g::pLocal || !i::EngineClient->IsInGame() || !g::pLocal->IsAlive())
		return original(ecx, edx);

	original(ecx, edx);

	return g_LocalAnimations->InterpolateMatricies();
}