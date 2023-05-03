#include "../hooks.h"

int __cdecl h::hkStartSound(void* stack) {
	
	static auto original = detour::startSound.GetOriginal<decltype(&h::hkStartSound)>();

	if (!stack)
		return 0;

	return original(stack);
	//if (stack.m_iUserData < 1 || stack.m_iUserData > 65)
	//	return 0;

	//if (stack.m_vecOrigin != Vector(0, 0, 0))
	//	util::LogConsole(std::format("{} : {} : {}\n", stack.m_vecOrigin.x, stack.m_vecOrigin.y, stack.m_vecOrigin.z).c_str());

	//return original(stack);
}