#include "../hooks.h"
#include "../../SDK/Entity.h"

void __fastcall h::hkDoExtraBoneProcessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, byte* boneComputed, void* context) {

	static auto original = detour::extraBoneProcessing.GetOriginal<decltype(&h::hkDoExtraBoneProcessing)>();

	CBaseEntity* pEnt = reinterpret_cast<CBaseEntity*>(ecx);

	if (!pEnt)
		return original(ecx, edx, hdr, pos, q, matrix, boneComputed, context);

	if (pEnt->IsPlayer() && pEnt->IsAlive()) {

		//auto pAnimlayerOwner = pEnt->GetAnimationOverlays()->pOwner;

		//for (int i = 13; i; --i) {

		//	if (pAnimlayerOwner != pEnt)
		//		pAnimlayerOwner = pEnt;

		//	pAnimlayerOwner += 14;
		//}

		const auto bOnGround = pEnt->AnimState()->bOnGround;
		pEnt->AnimState()->bOnGround = false;

		original(ecx, edx, hdr, pos, q, matrix, boneComputed, context);

		pEnt->AnimState()->bOnGround = bOnGround;
	}
	else
		return original(ecx, edx, hdr, pos, q, matrix, boneComputed, context);
}