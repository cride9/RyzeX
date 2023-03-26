#include "../hooks.h"
#include "../../SDK/Entity.h"

void __fastcall h::hkDoExtraBoneProcessing(void* entityPointer, void* edx, int a1, int a2, int a3, int a4, int a5, int a6) {

	static auto original = detour::extraBoneProcessing.GetOriginal<decltype(&h::hkDoExtraBoneProcessing)>();

	return;

	CBaseEntity* pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (!pEnt)
		return original(entityPointer, edx, a1, a2, a3, a4, a5, a6);

	if (pEnt->IsPlayer() && pEnt->IsAlive()) {

		auto pAnimlayerOwner = pEnt->GetAnimationOverlays()->pOwner;

		for (int i = 13; i; --i) {

			if (pAnimlayerOwner != pEnt)
				pAnimlayerOwner = pEnt;

			pAnimlayerOwner += 14;
		}

		const auto bOnGround = pEnt->AnimState()->bOnGround;
		pEnt->AnimState()->bOnGround = false;

		original(entityPointer, edx, a1, a2, a3, a4, a5, a6);

		pEnt->AnimState()->bOnGround = bOnGround;
	}
	else
		return original(entityPointer, edx, a1, a2, a3, a4, a5, a6);
}