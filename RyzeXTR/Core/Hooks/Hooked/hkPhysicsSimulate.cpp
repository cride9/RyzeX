#include "../../Features/Rage/doubletap.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../hooks.h"

void __fastcall h::hkPhysicsSimulate(void* ecx, void* edx) {

	static auto original = detour::physicsSimulate.GetOriginal<decltype(&h::hkPhysicsSimulate)>();

	/*
		m_nSimulationTick = gpGlobals->tickcount;
	
		if ( simulation_ticks > 0 )
		{
			AdjustPlayerTimeBase( simulation_ticks );
		}
	*/

	//int nTickCount = i::GlobalVars->iTickCount;

	//if (doubletap::cmdCommandNumber == g::pCmd->iCommandNumber)
	//	i::GlobalVars->iTickCount -= 15;

	original(ecx, edx);

	//if (doubletap::cmdCommandNumber == g::pCmd->iCommandNumber)
	//	i::GlobalVars->iTickCount = nTickCount;
}