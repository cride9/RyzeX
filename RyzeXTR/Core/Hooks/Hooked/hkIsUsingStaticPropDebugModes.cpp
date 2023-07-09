#include "../hooks.h"

bool __stdcall h::hkIsUsingStaticPropDebugModes() {

	static auto original = detour::isUsingStaticPropDebugModes.GetOriginal<decltype(&h::hkIsUsingStaticPropDebugModes)>();

	return cfg::misc::nightmode;
}