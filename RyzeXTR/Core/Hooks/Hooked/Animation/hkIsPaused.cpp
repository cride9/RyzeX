#include "../../hooks.h"

bool __stdcall h::hkIsPaused() {

	static auto original = detour::isPaused.GetOriginal<decltype(&h::hkIsPaused)>();

	// 84 C0 75 14 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0 84 C0 74 07
	static auto ExtrapolationReturn = reinterpret_cast<uintptr_t*>(util::FindSignature("client.dll", "0F B6 0D ? ? ? ? 84 C0 0F 44"));

	if (_ReturnAddress() == (void*)ExtrapolationReturn)
		return true;

	return original();
}