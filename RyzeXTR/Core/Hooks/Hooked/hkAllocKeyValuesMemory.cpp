#include "../hooks.h"

void* __fastcall h::hkAllocKeyValuesMemory(CKeyValuesSystem* thisptr, int edx, int iSize) {

	static auto original = detour::allocKeyValues.GetOriginal<decltype(&h::hkAllocKeyValuesMemory)>();

	static const std::uintptr_t uAllocKeyValuesEngine = util::GetAbsoluteAddress(util::FindSignature("engine.dll", "E8 ? ? ? ? 83 C4 08 84 C0 75 10 FF 75 0C") + 0x1) + 0x4A;
	static const std::uintptr_t uAllocKeyValuesClient = util::GetAbsoluteAddress(util::FindSignature("client.dll", "E8 ? ? ? ? 83 C4 08 84 C0 75 10") + 0x1) + 0x3E;

	if (const std::uintptr_t uReturnAddress = reinterpret_cast<std::uintptr_t>(_ReturnAddress()); uReturnAddress == uAllocKeyValuesEngine || uReturnAddress == uAllocKeyValuesClient)
		return nullptr;

	//x86RetSpoof::invokeFastcall<void*>(thisptr, edx, (uintptr_t)original, (uintptr_t)util::FindSignature("client.dll", "1B FF 23 F8 F6 87"));
	return original(thisptr, edx, iSize);
}