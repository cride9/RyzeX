#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkRunCommand(void* ecx, void* edx, CBaseEntity* pEnt, CUserCmd* pCmd, IMoveHelper* pMovehelper) {

	static auto original = detour::runCommand.GetOriginal<decltype(&h::hkRunCommand)>();

	if (!g::pCmd)
		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, pEnt, pCmd, pMovehelper);
		//return original(ecx, edx, pEnt, pCmd, pMovehelper);

	i::MoveHelper = pMovehelper;

	invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::EngineGadget_t::uReturnGadget, pEnt, pCmd, pMovehelper);

	misc::RevolverRunCommand(pEnt);
}