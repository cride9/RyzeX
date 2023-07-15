#include "../hooks.h"
#include "../../Features/Misc/misc.h"
#include "../../SDK/Menu/config.h"
#include "../../SDK/InputSystem.h"

void __fastcall h::hkOverrideView(void* ecx, void* edx, CViewSetup* pSetup) {

	static auto original = detour::overrideView.GetOriginal<decltype(&h::hkOverrideView)>();

	misc::ThirdPerson();

	if (!g::pLocal || !g::pLocal->IsAlive())
		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, pSetup);
		//return original(ecx, edx, pSetup);

	if (cfg::misc::removals[3])
		pSetup->flFOV = cfg::misc::fov;
	else if (!g::pLocal->IsScoped())
		pSetup->flFOV = cfg::misc::fov;

	if (cfg::misc::removals[2]) {

		Vector vecViewPunch = g::pLocal->GetViewPunch();
		Vector vecAimPunch = g::pLocal->GetAimPunch();

		pSetup->angView[0] -= (vecViewPunch[0] + (vecAimPunch[0] * 2 * 0.4499999f));
		pSetup->angView[1] -= (vecViewPunch[1] + (vecAimPunch[1] * 2 * 0.4499999f));
		pSetup->angView[2] -= (vecViewPunch[2] + (vecAimPunch[2] * 2 * 0.4499999f));
	}

	if (IPT::HandleInput(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {

		pSetup->vecOrigin.z = g::pLocal->GetAbsOrigin().z + 64.f;
	}
	

	invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, pSetup);
}