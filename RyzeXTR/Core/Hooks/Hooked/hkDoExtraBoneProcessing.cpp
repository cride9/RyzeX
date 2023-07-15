#include "../hooks.h"
#include "../../SDK/Entity.h"

void __fastcall h::hkDoExtraBoneProcessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, byte* boneComputed, void* context) {

	static auto original = detour::extraBoneProcessing.GetOriginal<decltype(&h::hkDoExtraBoneProcessing)>();

	CBaseEntity* pEnt = reinterpret_cast<CBaseEntity*>(ecx);

	if (!pEnt)
		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, hdr, pos, q, matrix, boneComputed, context);

	if (pEnt->IsPlayer() && pEnt->IsAlive()) {

		const auto bOnGround = pEnt->AnimState()->bOnGround;
		pEnt->AnimState()->bOnGround = false;

		invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, hdr, pos, q, matrix, boneComputed, context);

		pEnt->AnimState()->bOnGround = bOnGround;
	}
	else
		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, hdr, pos, q, matrix, boneComputed, context);
}