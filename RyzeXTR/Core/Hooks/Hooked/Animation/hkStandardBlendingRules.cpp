#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkStandardBlendingRules(void* entityPointer, void* unused1, void* unused2, void* unused3, void* unused4, float unused5, int unused6) {

	static auto original = detour::blendingRules.GetOriginal<decltype(&h::hkStandardBlendingRules)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (!pEnt->IsAlive())
		return original(entityPointer, unused1, unused2, unused3, unused4, unused5, unused6);

	if (!(pEnt->GetEffects() & 8))
		pEnt->GetEffects() |= 8;

	//invokeFastcall<void>(adr(entityPointer), adr(unused1), adr(original), ROP::ClientGadget_t::uReturnGadget, unused2, unused3, unused4, unused5, unused6);
	original(entityPointer, unused1, unused2, unused3, unused4, unused5, unused6);

	pEnt->GetEffects() &= ~8;
}