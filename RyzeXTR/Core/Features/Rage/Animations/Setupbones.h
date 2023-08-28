#pragma once
#include "../../../Interface/interfaces.h"
#include "../../../SDK/DataTyes/BitVec.h"
#include "../../../SDK/Entity.h"
#include "../../../SDK/math.h"
#include "../../../globals.h"
#include "../../../SDK/crt.h"

class SetupBonesRebuild {

public:

	bool SetupBones(IClientRenderable* thisptr, matrix3x4a_t* arrBonesToWorld, int iMaxBones, int nBoneMask, float flCurrentTime);
	void GetSkeleton(CBaseEntity* pEntity, CStudioHdr* pStudioHdr, BoneVector_t* arrBonesPosition, BoneQuaternionAligned_t* arrBonesRotation, int nBoneMask);
	float SetPoseParameter(CBaseEntity* pEntity, const CStudioHdr* pStudioHdr, const int iParameter, float flValue);

private:

};
inline SetupBonesRebuild setupbones;

class CBoneSetup
{
public:
	CBoneSetup(const CStudioHdr* pStudioHdr, const int nBoneMask, const float* arrPoseParameters, void* pPoseDebugger = nullptr) :
		pStudioHdr(pStudioHdr), nBoneMask(nBoneMask), pPoseParameters(arrPoseParameters), pPoseDebugger(pPoseDebugger) { }

	void* operator new(const std::size_t nSize) = delete;
	void operator delete(void* pMemory) = delete;

		void InitPose(BoneVector_t* arrBonesPosition, BoneQuaternionAligned_t* arrBonesRotation) const
	{
		for (int i = 0; i < pStudioHdr->pStudioHdr->nBones; i++)
		{
			if (const mstudiobone_t* pBone = pStudioHdr->pStudioHdr->GetBone(i); pBone->iFlags & nBoneMask)
			{
				arrBonesPosition[i] = pBone->vecPosition;
				arrBonesRotation[i] = pBone->qWorld;
			}
		}
	}

	void AccumulatePose(BoneVector_t* arrBonesPosition, BoneQuaternion_t* arrBonesRotation, int nSequence, float flCycle, float flWeight, float flTime, IKContext* pIKContext)
	{
		static auto fnAccumulatePose = reinterpret_cast<void(__thiscall*)(CBoneSetup*, BoneVector_t*, BoneQuaternion_t*, int, float, float, float, IKContext*)>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1")));
		fnAccumulatePose(this, arrBonesPosition, arrBonesRotation, nSequence, flCycle, flWeight, flTime, pIKContext);
	}

	void CalcAutoplaySequences(BoneVector_t* arrBonesPosition, BoneQuaternion_t* arrBonesRotation, float flRealTime, IKContext* pIKContext)
	{
		static auto fnCalcAutoplaySequences = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 10 53 56 57 8B 7D 10"));
		std::uintptr_t uCalcAutoplaySequences = reinterpret_cast<std::uintptr_t>(fnCalcAutoplaySequences); // @todo: clang compiles direct (E8) call instead of indirect (FF 15) without this

		__asm
		{
			mov ecx, this
			movss xmm3, flRealTime
			push pIKContext
			push arrBonesRotation
			push arrBonesPosition
			call uCalcAutoplaySequences
		}
	}

	// blend together two bone positions and rotations
	void CalcBoneAdjustment(BoneVector_t* arrBonesPosition, BoneQuaternion_t* arrBonesRotation, const float* arrEncodedControllers) const
	{
		static auto fnCalcBoneAdj = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89"));

		__asm
		{
			mov eax, this
			mov ecx, [eax + pStudioHdr]
			mov edx, arrBonesPosition
			push[eax + nBoneMask]
			push arrEncodedControllers
			push arrBonesRotation
			call fnCalcBoneAdj
			add esp, 0Ch
		}
	}

public:
	const CStudioHdr* pStudioHdr; // 0x00
	int nBoneMask; // 0x04
	const float* pPoseParameters; // 0x08
	void* pPoseDebugger; // 0x0C
};

inline void Studio_BuildMatrices(const CStudioHdr* pStudioHdr, const Vector& angView, const Vector& vecOrigin, const BoneVector_t* arrBonesPosition, const BoneQuaternion_t* arrBonesRotation, const float flScale, matrix3x4a_t arrBonesToWorld[MAXSTUDIOBONES], const int nBoneMask, const CBoneBitList& arrBonesComputed)
{
	// [side change] since 'iBone' parameter is removed, simplified logic here, reduced checks count
	int arrChain[MAXSTUDIOBONES] = { };
	int nChainLength = pStudioHdr->pStudioHdr->nBones;

	// build list of what bones to use
	for (int i = 0; i < nChainLength; i++)
		arrChain[nChainLength - i - 1] = i;

	// get model to world transformation
	matrix3x4_t matRotation = M::ToMatrix(angView, vecOrigin);

	// account for a change in scale
	if (flScale < 1.0f - FLT_EPSILON || flScale > 1.0f + FLT_EPSILON)
	{
		Vector vecOffset = matRotation.GetOrigin();
		vecOffset -= vecOrigin;
		vecOffset *= flScale;
		vecOffset += vecOrigin;
		matRotation.SetOrigin(vecOffset);

		// scale it uniformly
		*reinterpret_cast<Vector*>(matRotation[0]) *= flScale;
		*reinterpret_cast<Vector*>(matRotation[1]) *= flScale;
		*reinterpret_cast<Vector*>(matRotation[2]) *= flScale;
	}

	// [side change] simplified loop
	while (nChainLength-- > 0)
	{
		const int iChain = arrChain[nChainLength];

		// [side change] fixed old valve's fixme, if this bone has already been computed - skip transforms
		if (arrBonesComputed[iChain])
			continue;

		if (pStudioHdr->vecBoneFlags[iChain] & nBoneMask)
		{
			matrix3x4_t matBone = arrBonesRotation[iChain].ToMatrix(arrBonesPosition[iChain]);

			matrix3x4_t out;
			M::ConcatTransforms(matRotation, matBone, out);

			const int iParentBone = pStudioHdr->vecBoneParent[iChain];

			matrix3x4_t out2;
			M::ConcatTransforms(arrBonesToWorld[iParentBone], matBone, out2);

			arrBonesToWorld[iChain] = (iParentBone == -1 ? out : out2);
		}
	}
}


mstudioposeparamdesc_t& GetPoseParameterDescription(CStudioHdr* pStudioHdr, int iParameter)
{
	static auto fnpPoseParameter = reinterpret_cast<mstudioposeparamdesc_t&(__thiscall*)(CStudioHdr*, int)>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15")));
	return fnpPoseParameter(pStudioHdr, iParameter);
}

mstudioseqdesc_t& GetSequenceDescription(CStudioHdr* pStudioHdr, int iSequence)
{
	static auto fnpSeqdesc = reinterpret_cast<mstudioseqdesc_t & (__thiscall*)(CStudioHdr*, int)>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 56 8B 75 08 57 8B F9 85 F6 78 18")));
	return fnpSeqdesc(pStudioHdr, iSequence);
}

inline float Studio_GetPoseParameter(const CStudioHdr* pStudioHdr, const int iParameter, const float flFactor)
{
	// @ida Studio_GetPoseParameter(): client.dll | server.dll -> "55 8B EC 51 F3 0F 11 55"

	int iCount = pStudioHdr->pStudioHdr->nPoseParameters;
	if (pStudioHdr->pVirtualModel)
		iCount = pStudioHdr->pVirtualModel->vecPose.Count();

	if (iParameter < 0 || iParameter >= iCount)
		return 0.0f;

	const mstudioposeparamdesc_t& poseParameterDescription = GetPoseParameterDescription(const_cast<CStudioHdr*>(pStudioHdr), iParameter);
	return poseParameterDescription.flStart + (poseParameterDescription.flEnd - poseParameterDescription.flStart) * flFactor;
}

inline float Studio_SetPoseParameter(const CStudioHdr* pStudioHdr, const int iParameter, float flValue, float& flOutFactor)
{
	/*
	 * @ida Studio_SetPoseParameter():
	 * client.dll -> "55 8B EC 83 E4 F8 83 EC 08 F3 0F 11 54 24 ? 85"
	 * server.dll -> ABS["E8 ? ? ? ? D9 45 08" + 0x1]
	 */

	int iCount = pStudioHdr->pStudioHdr->nPoseParameters;
	if (pStudioHdr->pVirtualModel)
		iCount = pStudioHdr->pVirtualModel->vecPose.Count();

	if (iParameter < 0 || iParameter >= iCount)
	{
		flOutFactor = 0.0f;
		return 0.0f;
	}

	const mstudioposeparamdesc_t& poseParameterDescription = GetPoseParameterDescription(const_cast<CStudioHdr*>(pStudioHdr), iParameter);
	if (poseParameterDescription.flLoop > 0.0f)
	{
		const float flWrap = (poseParameterDescription.flStart + poseParameterDescription.flEnd) * 0.5f + poseParameterDescription.flLoop * 0.5f;
		const float flShift = poseParameterDescription.flLoop - flWrap;

		flValue = flValue - poseParameterDescription.flLoop * std::floorf((flValue + flShift) / poseParameterDescription.flLoop);
	}

	flOutFactor = CRT::Clamp((flValue - poseParameterDescription.flStart) / (poseParameterDescription.flEnd - poseParameterDescription.flStart), 0.0f, 1.0f);
	return poseParameterDescription.flStart + (poseParameterDescription.flEnd - poseParameterDescription.flStart) * flOutFactor;
}