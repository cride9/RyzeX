#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkRunCommand(void* ecx, void* edx, CBaseEntity* pEnt, CUserCmd* pCmd, IMoveHelper* pMovehelper) {

	static auto original = detour::runCommand.GetOriginal<decltype(&h::hkRunCommand)>();

	if (!g::pCmd)
		return detour::runCommand.CallOriginal<void>(ROP::EngineGadget_t::uReturnGadget, ecx, edx, pEnt, pCmd, pMovehelper);

	i::MoveHelper = pMovehelper;

	detour::runCommand.CallOriginal<void>(ROP::EngineGadget_t::uReturnGadget, ecx, edx, pEnt, pCmd, pMovehelper);

	misc::RevolverRunCommand(pEnt);
}