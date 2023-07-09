#include "../hooks.h"
#include "../../SDK/Menu/config.h"

bool __fastcall h::hkShouldDrawViewmodel(void* ecx, void* edx) {

	static auto original = detour::drawViewmodel.GetOriginal<decltype(&h::hkShouldDrawViewmodel)>();

	return original(ecx, edx);
	return cfg::misc::drawViewmodelOnScope ? true : original(ecx, edx);
}
