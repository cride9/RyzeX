#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/exploits.h"

void __fastcall h::hkRunCommand(void* ecx, void* edx, CBaseEntity* pEnt, CUserCmd* pCmd, IMoveHelper* pMovehelper) {

	static auto original = detour::runCommand.GetOriginal<decltype(&h::hkRunCommand)>();

	if (!g::pCmd)
		return original( ecx, edx, pEnt, pCmd, pMovehelper);

	i::MoveHelper = pMovehelper;

	exploits::iRestoreTickbase = pEnt->GetTickBase();
	exploits::flRestoreCurtime = i::GlobalVars->flCurrentTime;

	if (pCmd->iCommandNumber == exploits::iShiftCommand) {

		pEnt->GetTickBase() = exploits::iBackupTickbase - 15 + i::ClientState->nChokedCommands;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(exploits::iBackupTickbase - 15 + i::ClientState->nChokedCommands);
	}
	if (pCmd->iCommandNumber == exploits::iRechargeCommand) {

		pEnt->GetTickBase() = exploits::iBackupTickbase - i::ClientState->nChokedCommands;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(exploits::iBackupTickbase - i::ClientState->nChokedCommands);
	}

	original(ecx, edx, pEnt, pCmd, pMovehelper);

	exploits::bBackupTickbase = true;

	misc::RevolverRunCommand(pEnt);
}