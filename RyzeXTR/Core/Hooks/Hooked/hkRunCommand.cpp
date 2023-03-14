#include "../hooks.h"
#include "../../Features/Rage/doubletap.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Networking/networking.h"

void __fastcall h::hkRunCommand(void* ecx, void* edx, CBaseEntity* pEnt, CUserCmd* pCmd, IMoveHelper* pMovehelper) {

	static auto original = detour::runCommand.GetOriginal<decltype(&h::hkRunCommand)>();

	if (!g::pCmd)
		return original(ecx, edx, pEnt, pCmd, pMovehelper);

	if (!i::MoveHelper && pMovehelper)
		i::MoveHelper = pMovehelper;

	if (pCmd->iTickCount >= (g::pCmd->iTickCount + int(1 / i::GlobalVars->flIntervalPerTick) + 8)) {

		pCmd->bHasBeenPredicted = true;
		return;
	}

	g::nRestoreTickbase = prediction.GetTickBase(pCmd, pEnt);
	g::flRestoreCurtime = i::GlobalVars->flCurrentTime;

	float flVelocityModifier = pEnt->GetOffset<float>(0xA38C);
	if (localanim.update && pCmd->iCommandNumber == i::ClientState->iLastCommandAck + 1)
		pEnt->GetOffset<float>(0xA38C) = localanim.localdata.flVelocityModifier;

	/*if (doubletap::cmdCommandNumber == pCmd->iCommandNumber) {

		pEnt->GetTickBase() = doubletap::cmdTickbase - 16;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(prediction.GetTickBase(pCmd, pEnt));
	}
	if (doubletap::rechargeCommandNumber == pCmd->iCommandNumber) {

		pEnt->GetTickBase() = doubletap::rechargeTickbase + 14;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(prediction.GetTickBase(pCmd, pEnt));
	}
	if (doubletap::defensiveCommandNumber == pCmd->iCommandNumber) {

		pEnt->GetTickBase() = doubletap::defensiveTickbase - 3;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(prediction.GetTickBase(pCmd, pEnt));
	}
	if (doubletap::defensiveCommandNumberReset == pCmd->iCommandNumber) {

		pEnt->GetTickBase() = doubletap::defensiveTickbaseReset + 3;
		i::GlobalVars->flCurrentTime = TICKS_TO_TIME(prediction.GetTickBase(pCmd, pEnt));
	}*/

	original(ecx, edx, pEnt, pCmd, pMovehelper);

	//if (doubletap::cmdCommandNumber == pCmd->iCommandNumber || doubletap::rechargeCommandNumber == pCmd->iCommandNumber || doubletap::defensiveCommandNumber == pCmd->iCommandNumber || doubletap::defensiveCommandNumberReset == pCmd->iCommandNumber)
	//	g::bRestoreGlobals = true;

	prediction.SaveViewmodelData( g::pLocal );
	networking.SaveNetvarData( pEnt->GetTickBase( ) );

	if (!localanim.update)
		pEnt->GetOffset<float>(0xA38C) = flVelocityModifier;
}