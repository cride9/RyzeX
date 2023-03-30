#include "../hooks.h"
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

	float flVelocityModifier = pEnt->GetOffset<float>(0xA38C);
	if (localanim.update && pCmd->iCommandNumber == i::ClientState->iLastCommandAck + 1)
		pEnt->GetOffset<float>(0xA38C) = localanim.localdata.flVelocityModifier;

	original(ecx, edx, pEnt, pCmd, pMovehelper);

	prediction.SaveViewmodelData( g::pLocal );
	networking.SaveNetvarData( pEnt->GetTickBase( ) );

	if (!localanim.update)
		pEnt->GetOffset<float>(0xA38C) = flVelocityModifier;
}