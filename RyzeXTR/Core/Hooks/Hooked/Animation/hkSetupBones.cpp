#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

bool __fastcall h::hkSetupBones(void* ecx, void* edx, matrix3x4_t* matrix, int maxbones, int bonemask, float curtime) {

	static auto original = detour::setupBones.GetOriginal<decltype(&h::hkSetupBones)>();

	static auto retAddress = util::FindSignature("client.dll", "FF 75 08 E8 ? ? ? ? 5F 5E 5D C2 10 00") + 8;

	if (_ReturnAddress() == (void*)retAddress || _AddressOfReturnAddress() == (void*)retAddress)
		return false;

	const auto pEnt = reinterpret_cast<CBaseEntity*>((uintptr_t)ecx - 4);

	bool bResult = true;

	if (g::bSettingUpBones[pEnt->EntIndex()])
		bResult = original(ecx, edx, matrix, maxbones, bonemask, curtime);
	else if (matrix) {
		if (pEnt == g::pLocal) {
			g_LocalAnimations->CopyCachedMatrix(matrix, maxbones);
		}
	}

	if ( pEnt != g::pLocal || !pEnt || !pEnt->IsAlive( ) )
		return original( ecx, edx, matrix, maxbones, bonemask, curtime );
}