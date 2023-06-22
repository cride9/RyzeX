#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall	h::hkCheckForSequenceChange(void* entityPointer, void* edx, void* unused1, int unused2, bool unused3, bool interpolation) {

	static auto original = detour::sequenceChange.GetOriginal<decltype(&h::hkCheckForSequenceChange)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (pEnt && !pEnt->IsAlive())
		return original(entityPointer, edx, unused1, unused2, unused3, interpolation);

	return original(entityPointer, edx, unused1, unused2, unused3, false);
}