#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall	h::hkCheckForSequenceChange(void* entityPointer, void* edx, void* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate) {

	static auto original = detour::sequenceChange.GetOriginal<decltype(&h::hkCheckForSequenceChange)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (pEnt && !pEnt->IsAlive())
		return detour::sequenceChange.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, hdr, nCurSequence, bForceNewSequence, bInterpolate);

	return detour::sequenceChange.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, hdr, nCurSequence, bForceNewSequence, bInterpolate);
}