#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkStandardBlendingRules(void* entityPointer, void* unused1, void* pStudioHdr, void* pos, void* q, float currentTime, int boneMask) {

	static auto original = detour::blendingRules.GetOriginal<decltype(&h::hkStandardBlendingRules)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (!pEnt->IsAlive())
		return detour::blendingRules.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, unused1, pStudioHdr, pos, q, currentTime, boneMask);

	if (!(pEnt->GetEffects() & 8))
		pEnt->GetEffects() |= 8;

	detour::blendingRules.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, unused1, pStudioHdr, pos, q, currentTime, boneMask);

	pEnt->GetEffects() &= ~8;
}