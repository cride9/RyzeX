#include "../hooks.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"

float __fastcall h::hkGetViewModelFov(void* ecx, void* edx) {

	static auto original = detour::getViewmodelFov.GetOriginal<decltype(&h::hkGetViewModelFov)>();

	if (g::pLocal && g::pLocal->IsAlive()) {

		return cfg::misc::viewmodelFov;
	}

	return original(ecx, edx);
}