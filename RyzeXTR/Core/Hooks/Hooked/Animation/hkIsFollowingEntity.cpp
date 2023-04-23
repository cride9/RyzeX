#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

bool __fastcall h::hkIsFollowingEntity(void* ecx, void* edx)
{
	static auto IsFollowingEntity = detour::isFollowingEntity.GetOriginal<decltype(&hkIsFollowingEntity)>();

	CBaseEntity* pPlayer = (CBaseEntity*)(ecx);

	if (!pPlayer || !g::pLocal)
		return IsFollowingEntity(ecx, edx);

	static auto retToInterpolation = reinterpret_cast<void*>(util::FindSignature("client.dll", "84 C0 0F 85 ? ? ? ? 38 05 ? ? ? ? 0F 84 ? ? ? ? 53"));

	// note: we dont want to disable interp on local player here otherwise "it's time to get funky" - Cha Cha Slide : DJ Casper
	if (_ReturnAddress() == retToInterpolation && pPlayer != g::pLocal)
		return true;

	return IsFollowingEntity(ecx, edx);
}