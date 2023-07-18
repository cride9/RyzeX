#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkBuildTransformation(void* entityPointer, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& transform, int mask, uint8_t* computed) {

	static auto original = detour::buildTransform.GetOriginal<decltype(&h::hkBuildTransformation)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);
	if (!pEnt || !pEnt->IsAlive())
		return detour::buildTransform.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, hdr, pos, q, &transform, mask, computed);

	auto backup = hdr->vecBoneFlags;

	for (auto i = 0; i < hdr->vecBoneFlags.Count(); i++)
		hdr->vecBoneFlags.Element(i) &= ~BONE_ALWAYS_PROCEDURAL;

	detour::buildTransform.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, hdr, pos, q, &transform, mask, computed);

	hdr->vecBoneFlags = backup;
}