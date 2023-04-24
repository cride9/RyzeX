#include "../../hooks.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"
/*
void ___thiscall C_CSPlayer::BuildTransformations (DWORD this, int studioHdr, int a3, int a4, int a5, int boneMask, int boneComputed)
{
	C_BaseFlex::BuildTransformations((int *)this, studioHdr, a3, a4, a5, boneMask);
	v8 = *(_DWORD *)(this + 0x26A4);

	if (v8) {
		v9 = *(_DWORD *)(v8 +0x2950);

		if ( v9 &&*(_DWORD *)v9 && (**(_DWORD **)(v9 + 48) & *(_DWORD *)(this + 9904)) != 0 && (boneMask & 0xFFF00) != 0 ) {
			
			// this = entity pointer
			// *(_DWORD *)(this+0x26A8) = entity->m_dwBoneMatrix
			// boneMask = bonemask

			C_CSPlayer::ClampBonesInBBox((float *)this, *(_DWORD *)(this+0x26A8), boneMask);
		}
	}
}
*/

void __fastcall h::hkClampBonesInBBox(void* ecx, void* edx, matrix3x4_t* pMatrix, int iBoneMask) {

	static auto original = detour::clampBonesInBBox.GetOriginal<decltype(&hkClampBonesInBBox)>();

	return original(ecx, edx, pMatrix, iBoneMask);
}