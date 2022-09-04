#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkBuildTransformation(void* entityPointer, void* edx, CStudioHdr* hdr, void* unused2, void* unused3, const void* unused4, int unused5, void* unused6) {

	static auto original = detour::buildTransform.GetOriginal<decltype(&h::hkBuildTransformation)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	auto backup = hdr->vecBoneFlags;

	for (auto i = 0; i < hdr->vecBoneFlags.Count(); i++)
		hdr->vecBoneFlags.Element(i) &= ~BONE_ALWAYS_PROCEDURAL;

	original(entityPointer, edx, hdr, unused2, unused3, unused4, unused5, unused6);

	hdr->vecBoneFlags = backup;
}