#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

void __fastcall h::hkCalculateView(void* entityPointer, void* edx, Vector& unused1, Vector& unused2, float& unused3, float& unused4, float& unused5) {

	static auto original = detour::calculateView.GetOriginal<decltype(&h::hkCalculateView)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (pEnt != g::pLocal)
		return original(entityPointer, edx, unused1, unused2, unused3, unused4, unused5);

	if (!pEnt->IsAlive())
		return original(entityPointer, edx, unused1, unused2, unused3, unused4, unused5);

	const auto backup = pEnt->UseNewAnimationState();

	pEnt->UseNewAnimationState() = false;

	original(entityPointer, edx, unused1, unused2, unused3, unused4, unused5);

	pEnt->UseNewAnimationState() = backup;
}