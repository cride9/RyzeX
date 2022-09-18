#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../Features/Rage/Animations/LocalAnimation.h"

bool __fastcall h::hkSetupBones(void* ecx, void* edx, matrix3x4_t* matrix, int maxbones, int bonemask, float curtime) {

	static auto original = detour::setupBones.GetOriginal<decltype(&h::hkSetupBones)>();

	static auto retAddress = util::FindSignature("client.dll", "FF 75 08 E8 ? ? ? ? 5F 5E 5D C2 10 00") + 8;

	if (_ReturnAddress() == (void*)retAddress || _AddressOfReturnAddress() == (void*)retAddress)
		return false;

	if (!g::pLocal)
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	const auto pEnt = reinterpret_cast<CBaseEntity*>((uintptr_t)ecx - 4);

	if (!pEnt || !pEnt->IsPlayer())
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	if (pEnt->GetTeam() == g::pLocal->GetTeam() && pEnt != g::pLocal)
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	if (g::bSettingUpBones[pEnt->EntIndex()]) {

		const auto backupFirstMask = pEnt->GetOffset<int>(0x269C);
		const auto backupSecondMask = pEnt->GetOffset<int>(0x26B0);
		const auto backupFlags = pEnt->GetFlags();
		const auto backupEffects = pEnt->GetEffects();
		const auto backupUsedPredictionTime = pEnt->GetOffset<int>(0x2ee);

		pEnt->GetOffset<int>(0xA68) = 0;

		const auto backupFrameTime = i::GlobalVars->flFrameTime;

		i::GlobalVars->flFrameTime = FLT_MAX;

		pEnt->GetOffset<int>(0x269C) = 0;
		pEnt->GetOffset<int>(0x26B0) |= 512;
		pEnt->GetFlags() |= 8;

		/* disable matrix interpolation */
		pEnt->GetEffects() |= 8;

		/* use our setup time */
		pEnt->GetOffset<bool>(0x2ee) = false;

		/* thanks chambers */
		auto backup = pEnt->GetOffset<int>(0x68); // char -> int
		pEnt->GetOffset<int>(0x68) |= 2;

		/* use uninterpolated origin */
		pEnt->InvalidateBoneCache();
		const auto retValue = original(ecx, edx, matrix, maxbones, bonemask, curtime);

		pEnt->GetOffset<char>(0x68) = backup;
		pEnt->GetOffset<int>(0x269C) = backupFirstMask;
		pEnt->GetOffset<int>(0x26B0) = backupSecondMask;
		pEnt->GetFlags() = backupFlags;
		pEnt->GetEffects() = backupEffects;
		pEnt->GetOffset<int>(0x2ee) = backupUsedPredictionTime;

		i::GlobalVars->flFrameTime = backupFrameTime;

		return retValue;
	}
	else 
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

}