#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkStandardBlendingRules(void* entityPointer, void* unused1, void* pStudioHdr, void* pos, void* q, float currentTime, int boneMask) {

	static auto original = detour::blendingRules.GetOriginal<decltype(&h::hkStandardBlendingRules)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (!pEnt->IsAlive())
		return original( entityPointer, unused1, pStudioHdr, pos, q, currentTime, boneMask);

	const int backupEffects = pEnt->GetEffects();
	pEnt->GetEffects() |= EF_NOINTERP;

	original( entityPointer, unused1, pStudioHdr, pos, q, currentTime, boneMask);

	pEnt->GetEffects() = backupEffects;
}
