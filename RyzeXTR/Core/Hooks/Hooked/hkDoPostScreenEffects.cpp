#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"

int __fastcall h::hkDoPostScreenEffect(void* thisptr, int edx, CViewSetup* pSetup) {

	static auto original = detour::doPostScreenEffects.GetOriginal<decltype(&h::hkDoPostScreenEffect)>();

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return detour::doPostScreenEffects.CallOriginal<int>(ROP::ClientGadget_t::uReturnGadget, thisptr, edx, pSetup);

	if (g::pLocal) {
		visual::Glow(g::pLocal);
		visual::WorldLightning(cfg::misc::flLampColors);
	}

	return detour::doPostScreenEffects.CallOriginal<int>(ROP::ClientGadget_t::uReturnGadget, thisptr, edx, pSetup);
}