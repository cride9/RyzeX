#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall	h::hkCheckForSequenceChange(void* entityPointer, void* edx, void* unused1, int unused2, bool unused3, bool interpolation) {

	static auto original = detour::sequenceChange.GetOriginal<decltype(&h::hkCheckForSequenceChange)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (pEnt && !pEnt->IsAlive())
		return invokeFastcall<void>(adr(entityPointer), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, unused1, unused2, unused3, interpolation);

	return invokeFastcall<void>(adr(entityPointer), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, unused1, unused2, unused3, interpolation);
}