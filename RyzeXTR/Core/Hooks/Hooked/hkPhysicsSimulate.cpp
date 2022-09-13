#include "../../Features/Rage/doubletap.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../hooks.h"

int IncrementTickBase(uint32_t pCommand) {

	int iTickbase = g::pLocal->GetTickBase();

	return iTickbase;

	if (pCommand == doubletap::cmdCommandNumber) {
		return iTickbase - 15;
	}
	else if (pCommand == doubletap::defensiveCommandNumber) {
		return iTickbase - 13;
	}

	if (pCommand == doubletap::cmdCommandNumber + 1) {
		return iTickbase + 15;
	}
	else if (pCommand == doubletap::defensiveCommandNumber + 1) {
		return iTickbase + 13;
	}

	return iTickbase;
}

void __fastcall h::hkPhysicsSimulate(void* ecx, void* edx) {

	static auto original = detour::physicsSimulate.GetOriginal<decltype(&h::hkPhysicsSimulate)>();

	/*
		m_nSimulationTick = gpGlobals->tickcount;
	
		if ( simulation_ticks > 0 )
		{
			AdjustPlayerTimeBase( simulation_ticks );
		}
	*/

	CCommandContext* pCommandContext = g::pLocal->GetOffsetPointer<CCommandContext>(0x350C);

	if (!pCommandContext || !pCommandContext->bNeedsProcessing)
		return original(ecx, edx);

	// doubletap shooting fix
	if (doubletap::cmdCommandNumber == pCommandContext->nCommandNumber) {

		g::pLocal->GetTickBase() -= 13;
	}
	else if (doubletap::cmdCommandNumber == pCommandContext->nCommandNumber + 1) {

		g::pLocal->GetTickBase() += 13;
	}

	// defensive CHOKE fix (unchoke is not fixed)
	if (doubletap::defensiveCommandNumber == pCommandContext->nCommandNumber) {

		g::pLocal->GetTickBase() -= 13;
	}
	else if (doubletap::defensiveCommandNumber == pCommandContext->nCommandNumber + 1) {

		g::pLocal->GetTickBase() += 13;
	}

	// recharge fix
	if (doubletap::rechargeCommandNumber == pCommandContext->nCommandNumber) {

		g::pLocal->GetTickBase() += 6;
	}
	else if (doubletap::rechargeCommandNumber == pCommandContext->nCommandNumber + 1) {

		g::pLocal->GetTickBase() -= 6;
	}

	original(ecx, edx);
}