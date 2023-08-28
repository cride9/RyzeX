#include "Setupbones.h"
#include "../../../SDK/DataTyes/BoneMargeCache.h"

bool SetupBonesRebuild::SetupBones(IClientRenderable* thisptr, matrix3x4a_t* arrBonesToWorld, int iMaxBones, int nBoneMask, float flCurrentTime) {

	CBaseEntity* pPlayer = thisptr->GetIClientUnknown()->GetBaseEntity();
	
	if (!pPlayer)
		return;

	const int nPlayerIndex = pPlayer->EntIndex();

	pPlayer->GetCustomBlendingRuleMask() = -1;
	pPlayer->GetAnimationLODFlags() = 0U;
	pPlayer->GetOldAnimationLODFlags() = 0U;
	pPlayer->GetComputedAnimationLODFrame() = i::GlobalVars->iFrameCount;

	CMDLCacheCriticalSection mdlCacheCriticalSection(i::MDLCache);

	CStudioHdr* pStudioHdr = pPlayer->GetModelPtr();

	if (pStudioHdr == nullptr)
		return false;

	if (nBoneMask == -1)
		nBoneMask = pPlayer->GetPrevBoneMask();

	else {

		int nLOD = 0;
		int nLODMask = BONE_USED_BY_VERTEX_LOD0;
		for (; nLOD < MAX_NUM_LODS; ++nLOD, nLODMask <<= 1)
		{
			if (nBoneMask & nLODMask)
				break;
		}
		for (; nLOD < MAX_NUM_LODS; ++nLOD, nLODMask <<= 1)
			nBoneMask |= nLODMask;
	}

	CBoneAccessor& boneAccessor = pPlayer->GetBoneAccessor();
	int& nEFlags = pPlayer->GetEFlags();

	int& nAccumulatedBoneMask = pPlayer->GetAccumulatedBoneMask();
	pPlayer->GetRecentModelBoneCounter() = pPlayer->GetModelBoneCounter();
	pPlayer->GetPrevBoneMask() = nAccumulatedBoneMask;
	nAccumulatedBoneMask = nBoneMask;
	pPlayer->GetLastSetupBonesTime() = i::GlobalVars->flCurrentTime;

	boneAccessor.nReadableBones = nBoneMask;
	boneAccessor.nWritableBones = nBoneMask;

	nEFlags |= EFL_SETTING_UP_BONES;

	const Vector& vecAbsOrigin = pPlayer->GetAbsOrigin();
	const Vector& angAbsView = pPlayer->GetAbsAngles();

	BoneVector_t arrBonesPosition[MAXSTUDIOBONES];
	BoneQuaternionAligned_t arrBonesRotation[MAXSTUDIOBONES];

	CBoneBitList arrBonesComputed = { };

	if (IKContext* pIKContext = pPlayer->GetIKContext(); pIKContext != nullptr)
	{
		pIKContext->Init(pStudioHdr, angAbsView, vecAbsOrigin, flCurrentTime, i::GlobalVars->iFrameCount, boneAccessor.nWritableBones);

		GetSkeleton(pPlayer, pStudioHdr, arrBonesPosition, arrBonesRotation, boneAccessor.nWritableBones);

		pIKContext->UpdateTargets(arrBonesPosition, arrBonesRotation, reinterpret_cast<matrix3x4a_t*>(boneAccessor.matBones), arrBonesComputed);
		pPlayer->CalculateIKLocks(flCurrentTime);
		pIKContext->SolveDependencies(arrBonesPosition, arrBonesRotation, reinterpret_cast<matrix3x4a_t*>(boneAccessor.matBones), arrBonesComputed);
	}
	else
		GetSkeleton(pPlayer, pStudioHdr, arrBonesPosition, arrBonesRotation, boneAccessor.nWritableBones);

	if (pPlayer->GetEffects() & EF_BONEMERGE)
	{
		if (const CBaseHandle& hMoveParent = pPlayer->GetMoveParentHandle(); hMoveParent != 0xFFFFFFFF)
		{
			//if (CBaseAnimating* pParent = I::ClientEntityList->Get<CBaseEntity>(hMoveParent)->GetBaseAnimating(); pParent != nullptr)
			//	Q_ASSERT(false); // currently isn't used in cs:go
		}
	}
	// @test: i had never got a break, so probably the whole code block can be removed
	else if (CBoneMergeCache* pBoneMergeCache = pPlayer->GetBoneMergeCache(); pBoneMergeCache != nullptr)
	{
		delete pBoneMergeCache;
		pBoneMergeCache = nullptr;

		pPlayer->GetBoneMergeCache() = pBoneMergeCache;
	}


}

void SetupBonesRebuild::GetSkeleton(CBaseEntity* pEntity, CStudioHdr* pStudioHdr, BoneVector_t* arrBonesPosition, BoneQuaternionAligned_t* arrBonesRotation, int nBoneMask) {

	IKContext* pIKContext = pEntity->GetIKContext();
	if (pIKContext == nullptr)
	{
		pIKContext = new IKContext;
		pIKContext->Init(pStudioHdr, pEntity->GetAbsAngles(), pEntity->GetAbsOrigin(), i::GlobalVars->flCurrentTime, 0, BONE_USED_BY_BONE_MERGE);

		pEntity->GetIKContext() = pIKContext;
	}

	CBoneSetup boneSetup(pStudioHdr, nBoneMask, pEntity->GetPoseParameter().data());
	boneSetup.InitPose(arrBonesPosition, arrBonesRotation);
	boneSetup.AccumulatePose(arrBonesPosition, arrBonesRotation, pEntity->GetSequence(), pEntity->GetCycle(), 1.0f, i::GlobalVars->flCurrentTime, pIKContext);

	bool bDoWeaponSetup = false;
	CBaseEntity* pWeaponWorldModel = nullptr;

	if (pEntity->IsPlayer())
	{
		if (CBaseEntity* pPlayer = pEntity; pPlayer != nullptr && pPlayer->IsUsedNewAnimState())
		{
			if (CBaseCombatWeapon* pWeapon = pPlayer->GetWeapon(); pWeapon != nullptr)
			{
				CBaseEntity* pWeaponWorldModel = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntityFromHandle(pWeapon->GetWorldModelHandle()));

				if (pWeaponWorldModel != nullptr && pWeaponWorldModel->IsHoldPlayerAnimations() && pWeaponWorldModel->GetBoneMergeCache() != nullptr)
					bDoWeaponSetup = true;
			}
		}
	}

	CAnimationLayer* vecAnimationLayers = pEntity->GetAnimationOverlays();
	if (bDoWeaponSetup)
	{
		CStudioHdr* pWeaponStudioHdr = pWeaponWorldModel->GetModelPtr();

		// copy matching player pose parameters to weapon pose parameters
		CBoneMergeCache* pBoneMergeCache = pWeaponWorldModel->GetBoneMergeCache();
		pBoneMergeCache->MergeMatchingPoseParams();

		// build a temporary setup for the weapon
		IKContext* pWeaponIK = new IKContext;
		pWeaponIK->Init(pWeaponStudioHdr, pEntity->GetAbsAngles(), pEntity->GetAbsOrigin(), i::GlobalVars->flCurrentTime, 0, BONE_USED_BY_BONE_MERGE);

		BoneVector_t arrWeaponPositions[MAXSTUDIOBONES];
		BoneQuaternionAligned_t arrWeaponRotations[MAXSTUDIOBONES];

		CBoneSetup weaponSetup(pWeaponStudioHdr, BONE_USED_BY_BONE_MERGE, pWeaponWorldModel->GetPoseParameter().data());
		weaponSetup.InitPose(arrWeaponPositions, arrWeaponRotations);

		for (int i = 0; i < 13; i++)
		{
			CAnimationLayer* pLayer = &vecAnimationLayers[i];

			if (pLayer->nSequence <= 1 || pLayer->flWeight <= 0.0f)
				continue;

			pEntity->UpdateDispatchLayer(pLayer, pWeaponStudioHdr, pLayer->nSequence); // @note: this assumes we call setupbones once per tick (as we should do and ensure)

			int iCount = pStudioHdr->pStudioHdr->nLocalSequences;
			if (pStudioHdr->pVirtualModel)
				iCount = pStudioHdr->pVirtualModel->vecSequence.Count();

			if (pLayer->nDispatchedDst <= 0 || pLayer->nDispatchedDst >= iCount)
				boneSetup.AccumulatePose(arrBonesPosition, arrBonesRotation, pLayer->nSequence, pLayer->flCycle, pLayer->flWeight, i::GlobalVars->flCurrentTime, pIKContext);
			else
			{
				// copy player bones to weapon setup bones
				pBoneMergeCache->CopyFromFollow(arrBonesPosition, arrBonesRotation, BONE_USED_BY_BONE_MERGE, arrWeaponPositions, arrWeaponRotations);

				// [side change] removed null check for 'pIkContext', because it will never happen
				// respect ik rules on archetypal sequence, even if we're not playing it
				mstudioseqdesc_t& sequenceDescription = GetSequenceDescription(pStudioHdr, pLayer->nSequence);
				pIKContext->AddDependencies(sequenceDescription, pLayer->nSequence, pLayer->flCycle, pEntity->GetPoseParameter().data(), pLayer->flWeight);

				weaponSetup.AccumulatePose(arrWeaponPositions, arrWeaponRotations, pLayer->nDispatchedDst, pLayer->flCycle, pLayer->flWeight, i::GlobalVars->flCurrentTime, pWeaponIK);
				pBoneMergeCache->CopyToFollow(arrWeaponPositions, arrWeaponRotations, BONE_USED_BY_BONE_MERGE, arrBonesPosition, arrBonesRotation);

				pWeaponIK->CopyTo(pIKContext, pBoneMergeCache->arrRawIndexMapping);
			}
		}

		delete pWeaponIK;
	}
	else
	{
		// [side change] moved whole ordering under check, changed initial/error value to minimize checks count
		int arrLayers[13];
		const int nLayersCount = 13;

		for (int i = 0; i < 13; i++)
			arrLayers[i] = -1;

		// sort the layers
		for (int i = 0; i < nLayersCount; i++)
		{
			if (const CAnimationLayer& layer = vecAnimationLayers[i]; layer.flWeight > 0.0f && layer.nSequence != -1 && layer.iOrder >= 0 && layer.iOrder < nLayersCount)
				arrLayers[layer.iOrder] = i;
		}

		for (int i = 0; i < nLayersCount; i++)
		{
			if (const int nIndex = arrLayers[i]; nIndex != -1)
			{
				const CAnimationLayer& layer = vecAnimationLayers[nIndex];
				boneSetup.AccumulatePose(arrBonesPosition, arrBonesRotation, layer.nSequence, layer.flCycle, layer.flWeight, i::GlobalVars->flCurrentTime, pIKContext);
			}
		}
	}

	IKContext* pAutoIK = new IKContext;
	pAutoIK->Init(pStudioHdr, pEntity->GetAbsAngles(), pEntity->GetAbsOrigin(), i::GlobalVars->flCurrentTime, 0, nBoneMask);
	boneSetup.CalcAutoplaySequences(arrBonesPosition, arrBonesRotation, i::GlobalVars->flCurrentTime, pAutoIK);
	delete pAutoIK;

	boneSetup.CalcBoneAdjustment(arrBonesPosition, arrBonesRotation, pEntity->GetEncodedControllerArray().data());
}

float SetupBonesRebuild::SetPoseParameter(CBaseEntity* pEntity, const CStudioHdr* pStudioHdr, const int iParameter, float flValue) {

	if (iParameter >= 0)
	{
		float flNewValue;
		flValue = Studio_SetPoseParameter(pStudioHdr, iParameter, flValue, flNewValue);
		pEntity->GetPoseParameter()[iParameter] = flNewValue;
	}

	return flValue;
}