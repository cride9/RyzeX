#include "../../hooks.h"

//int	h::hkInterpolationList() {
//
//	static auto original = detour::interpolationList.GetOriginal<decltype(&h::hkInterpolationList)>();
//
//	static auto bExtrapolation = *(bool**)(MEM::FindPattern(CLIENT_DLL, "A2 ? ? ? ? 8B 45 E8") + 0x1);
//
//	if (bExtrapolation)
//		*bExtrapolation = false;
//
//	return original();
//}