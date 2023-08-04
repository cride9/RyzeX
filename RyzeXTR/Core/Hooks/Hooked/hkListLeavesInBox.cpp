#include "../hooks.h"
#include "../../SDK/Menu/config.h"
#include "../../SDK/Entity.h"

int __fastcall h::hkListLeavesInBox(void* thisptr, int edx, const Vector& vecMins, const Vector& vecMaxs, unsigned short* puList, int nListMax) {

	static auto original = detour::listLeaves.GetOriginal<decltype(&h::hkListLeavesInBox)>();

	static std::uintptr_t uInsertIntoTree = (uintptr_t)(MEM::FindPattern(CLIENT_DLL, XorStr("56 52 FF 50 18")) + 0x5);

	if (cfg::model::iType[2] && reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uInsertIntoTree) {

		if (const auto pInfo = *reinterpret_cast<RenderableInfo_t**>(reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) + 0x14); pInfo != nullptr) {
			
			if (const auto pRenderable = pInfo->pRenderable; pRenderable != nullptr) {
				
				if (const auto pEntity = pRenderable->GetIClientUnknown()->GetBaseEntity(); pEntity != nullptr && pEntity->IsPlayer()) {
					
					pInfo->uFlags &= ~RENDER_FLAGS_FORCE_OPAQUE_PASS;
					pInfo->uFlags2 |= RENDER_FLAGS_BOUNDS_ALWAYS_RECOMPUTE;

					constexpr Vector vecMapMin(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
					constexpr Vector vecMapMax(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
					//return invokeFastcall<int>(adr(thisptr), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, &vecMins, &vecMaxs, puList, nListMax);
					return original(thisptr, edx, vecMapMin, vecMapMax, puList, nListMax);
				}
			}
		}
	}
	return original(thisptr, edx, vecMins, vecMaxs, puList, nListMax);

	//return invokeFastcall<int>(adr(thisptr), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, &vecMins, &vecMaxs, puList, nListMax);
}