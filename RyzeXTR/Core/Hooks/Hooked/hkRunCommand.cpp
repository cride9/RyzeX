#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/exploits.h"

using namespace exploits;
void __fastcall h::hkRunCommand(void* ecx, void* edx, CBaseEntity* pEnt, CUserCmd* pCmd, IMoveHelper* pMovehelper) {

	static auto original = detour::runCommand.GetOriginal<decltype(&h::hkRunCommand)>();

	if (!g::pCmd || pEnt != g::pLocal)
		return original( ecx, edx, pEnt, pCmd, pMovehelper);

	i::MoveHelper = pMovehelper;

	// Serialization-only rejected commands never enter this ring. Do not reject
	// genuine engine commands using a different, arbitrary future-tick threshold.
	pEnt->GetTickBase() = GetCommandTickbase(pCmd->iCommandNumber, pEnt->GetTickBase());

	original(ecx, edx, pEnt, pCmd, pMovehelper);
	misc::RevolverRunCommand(pEnt);
}
