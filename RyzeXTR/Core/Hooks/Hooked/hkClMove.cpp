#include "../hooks.h"
#include "../../Features/Rage/exploits.h"

void __cdecl h::hkClMove(float flSamples, bool bFinalTick) {
	
	return exploits::CLMove(flSamples, bFinalTick);
}

void __cdecl h::hkClSendMove() {
	static auto original = detour::clSendMove.GetOriginal<decltype(&h::hkClSendMove)>();
	original();
}
