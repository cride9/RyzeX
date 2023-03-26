#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"

int __fastcall h::hkDoPostScreenEffect(CClientModeShared* thisptr, int edx, CViewSetup* pSetup) {

	static auto original = detour::doPostScreenEffects.GetOriginal<decltype(&h::hkDoPostScreenEffect)>();

	if (!i::EngineClient->IsInGame())
		return original(thisptr, edx, pSetup);

	if (CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer(); pLocal != nullptr)
		visual::Glow(pLocal);

	return original(thisptr, edx, pSetup);
}