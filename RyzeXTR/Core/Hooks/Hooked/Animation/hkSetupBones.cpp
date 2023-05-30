#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"
#include "../../../Features/Rage/Animations/EnemyAnimations.h"

bool __fastcall h::hkSetupBones(void* ecx, void* edx, matrix3x4_t* matrix, int maxbones, int bonemask, float curtime) {

	static auto original = detour::setupBones.GetOriginal<decltype(&h::hkSetupBones)>();

	static auto retAddress = util::FindSignature("client.dll", "FF 75 08 E8 ? ? ? ? 5F 5E 5D C2 10 00") + 8;

	if (_ReturnAddress() == (void*)retAddress || _AddressOfReturnAddress() == (void*)retAddress)
		return false;

	const auto pEnt = reinterpret_cast<CBaseEntity*>((uintptr_t)ecx - 4);

	bool bResult = true;
	if (!g::pLocal || !pEnt->IsAlive() || (pEnt->GetTeam() == g::pLocal->GetTeam() && pEnt != g::pLocal) || !pEnt->IsPlayer() || g::bUpdatingSkins)
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	if (std::get<0>(g::bSettingUpBones[pEnt->EntIndex()]))
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	if (matrix) {
		if (pEnt == g::pLocal) 
			bResult = g_LocalAnimations->CopyCachedMatrix(matrix, maxbones);
		else
			bResult = anims.CopyCachedMatrix(pEnt, matrix, maxbones);
	}

	return bResult;
}