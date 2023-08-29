#include "Setupbones.h"
#include "../../../globals.h"
#include "../../../SDK/math.h"

float Studio_SetPoseParameter( const CStudioHdr* pStudioHdr, const int iParameter, float flValue, float& flOutFactor )
{
	if (iParameter < 0 || iParameter >= pStudioHdr->pStudioHdr->nPoseParameters)
	{
		flOutFactor = 0.0f;
		return 0.0f;
	}

	const mstudioposeparamdesc_t& poseParameterDescription = const_cast< CStudioHdr* >( pStudioHdr )->GetPoseParameterDescription( iParameter );
	if (poseParameterDescription.flLoop > 0.0f)
	{
		const float flWrap = ( poseParameterDescription.flStart + poseParameterDescription.flEnd ) * 0.5f + poseParameterDescription.flLoop * 0.5f;
		const float flShift = poseParameterDescription.flLoop - flWrap;

		flValue = flValue - poseParameterDescription.flLoop * std::floorf( ( flValue + flShift ) / poseParameterDescription.flLoop );
	}

	flOutFactor = std::clamp( ( flValue - poseParameterDescription.flStart ) / ( poseParameterDescription.flEnd - poseParameterDescription.flStart ), 0.0f, 1.0f );
	return poseParameterDescription.flStart + ( poseParameterDescription.flEnd - poseParameterDescription.flStart ) * flOutFactor;
}

float CBaseEntity::SetPoseParameter( CStudioHdr* pStudioHdr, int iParameter, float flValue )
{
	if (!pStudioHdr)
		return flValue;

	if (iParameter >= 0)
	{
		float flNewValue;
		flValue = Studio_SetPoseParameter( pStudioHdr, iParameter, flValue, flNewValue );
		*( float* )( ( DWORD )this + n::netvars[ fnv::HashConst( XorStr( "CCSPlayer->m_flPoseParameter" ) ) ].uOffset + ( sizeof( float ) * iParameter ) ) = flNewValue;
	}

	return flValue;
}

void GetSkeleton( CBaseEntity* pPlayer, CStudioHdr* pStudioHdr, BoneVector arrBonesPosition[ ], BoneQuaternionAligned arrBonesRotation[ ], int nBoneMask )
{
	IKContext* pIKContext = pPlayer->GetIKContext( );
	if (pIKContext == nullptr)
	{
		pIKContext = new IKContext;
		pIKContext->Init( pStudioHdr, pPlayer->GetAbsAngles( ), pPlayer->GetAbsOrigin( ), i::GlobalVars->flCurrentTime, 0, BONE_USED_BY_BONE_MERGE );

		pPlayer->GetIKContext( ) = pIKContext;
	}

	CBoneSetup boneSetup( pStudioHdr, nBoneMask, pPlayer->GetPoseParameter( ).data( ) );
	boneSetup.InitPose( arrBonesPosition, arrBonesRotation );
	boneSetup.AccumulatePose( arrBonesPosition, arrBonesRotation, pPlayer->GetSequence( ), pPlayer->GetCycle( ), 1.0f, i::GlobalVars->flCurrentTime, pIKContext );

	bool bDoWeaponSetup = false;
	CBaseEntity* pWeaponWorldModel = nullptr;

	if (pPlayer->IsPlayer( ))
	{
		if (pPlayer->IsUsedNewAnimState( ))
		{
			if (CBaseCombatWeapon* pWeapon = pPlayer->GetWeapon( ); pWeapon != nullptr)
			{
				if (pWeaponWorldModel = reinterpret_cast< CBaseEntity*>( i::EntityList->GetClientEntityFromHandle( pWeapon->GetWorldModelHandle() ) ); pWeaponWorldModel != nullptr && pWeaponWorldModel->GetBoneMergeCache( ) != nullptr)
					bDoWeaponSetup = true;
			}
		}
	}

	if (bDoWeaponSetup)
	{
		CStudioHdr* pWeaponStudioHdr = reinterpret_cast< CBaseEntity* >( pWeaponWorldModel )->GetModelPtr( );

		CBoneMergeCache* pBoneMergeCache = pWeaponWorldModel->GetBoneMergeCache( );
		pBoneMergeCache->MergeMatchingPoseParams( );

		IKContext* pWeaponIK = new IKContext;
		pWeaponIK->Init( pWeaponStudioHdr, pPlayer->GetAbsAngles( ), pPlayer->GetAbsOrigin( ), i::GlobalVars->flCurrentTime, 0, BONE_USED_BY_BONE_MERGE );

		BoneVector arrWeaponPositions[ MAXSTUDIOBONES ];
		BoneQuaternionAligned arrWeaponRotations[ MAXSTUDIOBONES ];

		CBoneSetup weaponSetup( pWeaponStudioHdr, BONE_USED_BY_BONE_MERGE, reinterpret_cast< CBaseEntity* >( pWeaponWorldModel )->GetPoseParameter( ).data() );
		weaponSetup.InitPose( arrWeaponPositions, arrWeaponRotations );

		for (int i = 0; i < ANIMATION_LAYER_COUNT; i++)
		{
			auto pLayer = &pPlayer->GetAnimationOverlays( )[ i ];
			if (pLayer->nSequence <= 1 || pLayer->flWeight <= 0.0f)
				continue;

			pPlayer->UpdateDispatchLayer( pLayer, pWeaponStudioHdr, pLayer->nSequence );

			if (pLayer->nDispatchedDst <= 0 || pLayer->nDispatchedDst >= pStudioHdr->GetNumSeq( ) )
				boneSetup.AccumulatePose( arrBonesPosition, arrBonesRotation, pLayer->nSequence, pLayer->flCycle, pLayer->flWeight, i::GlobalVars->flCurrentTime, pIKContext );
			else
			{
				pBoneMergeCache->CopyFromFollow( arrBonesPosition, arrBonesRotation, BONE_USED_BY_BONE_MERGE, arrWeaponPositions, arrWeaponRotations );

				mstudioseqdesc_t& sequenceDescription = pStudioHdr->pSeqdesc( pLayer->nSequence );
				pIKContext->AddDependencies( sequenceDescription, pLayer->nSequence, pLayer->flCycle, pPlayer->GetPoseParameter( ).data( ), pLayer->flWeight );

				weaponSetup.AccumulatePose( arrWeaponPositions, arrWeaponRotations, pLayer->nDispatchedDst, pLayer->flCycle, pLayer->flWeight, i::GlobalVars->flCurrentTime, pWeaponIK );
				pBoneMergeCache->CopyToFollow( arrWeaponPositions, arrWeaponRotations, BONE_USED_BY_BONE_MERGE, arrBonesPosition, arrBonesRotation );

				pWeaponIK->CopyTo( pIKContext, pBoneMergeCache->arrRawIndexMapping );
			}
		}

		delete pWeaponIK;
	}
	else
	{
		int arrLayers[ ANIMATION_LAYER_COUNT ];
		const int nLayersCount = ANIMATION_LAYER_COUNT;

		for (int i = 0; i < ANIMATION_LAYER_COUNT; i++)
			arrLayers[ i ] = -1;

		// sort the layers
		for (int i = 0; i < nLayersCount; i++)
		{
			if (const CAnimationLayer& layer = pPlayer->GetAnimationOverlays( )[ i ]; layer.flWeight > 0.0f && layer.nSequence != -1 && layer.iOrder >= 0 && layer.iOrder < nLayersCount)
				arrLayers[ layer.iOrder ] = i;
		}

		for (int i = 0; i < nLayersCount; i++)
		{
			if (const int nIndex = arrLayers[ i ]; nIndex != -1)
			{
				const CAnimationLayer& layer = pPlayer->GetAnimationOverlays( )[ nIndex ];
				boneSetup.AccumulatePose( arrBonesPosition, arrBonesRotation, layer.nSequence, layer.flCycle, layer.flWeight, i::GlobalVars->flCurrentTime, pIKContext );
			}
		}
	}

	IKContext* pAutoIK = new IKContext;
	pAutoIK->Init( pStudioHdr, pPlayer->GetAbsAngles( ), pPlayer->GetAbsOrigin( ), i::GlobalVars->flCurrentTime, 0, nBoneMask );
	boneSetup.CalcAutoplaySequences( arrBonesPosition, arrBonesRotation, i::GlobalVars->flCurrentTime, pAutoIK );
	delete pAutoIK;

	boneSetup.CalcBoneAdjustment( arrBonesPosition, arrBonesRotation, pPlayer->GetEncodedControllerArray( ).data() );
}

void Studio_BuildMatrices( const CStudioHdr* pStudioHdr, const Vector& angView, const Vector& vecOrigin, const BoneVector* arrBonesPosition, const BoneQuaternion* arrBonesRotation, const float flScale, matrix3x4a_t arrBonesToWorld[ MAXSTUDIOBONES ], const int nBoneMask, const CBoneBitList& arrBonesComputed )
{
	int arrChain[ MAXSTUDIOBONES ] = { };
	int nChainLength = pStudioHdr->pStudioHdr->nBones;

	for (int i = 0; i < nChainLength; i++)
		arrChain[ nChainLength - i - 1 ] = i;

	matrix3x4a_t matRotation = M::ToMatrix( angView, vecOrigin );

	if (flScale < 1.0f - FLT_EPSILON || flScale > 1.0f + FLT_EPSILON)
	{
		Vector vecOffset = matRotation.GetOrigin( );
		vecOffset -= vecOrigin;
		vecOffset *= flScale;
		vecOffset += vecOrigin;
		matRotation.SetOrigin( vecOffset );

		*reinterpret_cast< Vector* >( matRotation[ 0 ] ) *= flScale;
		*reinterpret_cast< Vector* >( matRotation[ 1 ] ) *= flScale;
		*reinterpret_cast< Vector* >( matRotation[ 2 ] ) *= flScale;
	}

	assert( ( reinterpret_cast< std::uintptr_t >( arrBonesToWorld ) & 0xF ) == 0 );

	while (nChainLength-- > 0)
	{
		const int iChain = arrChain[ nChainLength ];

		if (arrBonesComputed[ iChain ])
			continue;

		if (pStudioHdr->vecBoneFlags[ iChain ] & nBoneMask)
		{
			matrix3x4a_t matBone = arrBonesRotation[ iChain ].ToMatrix( arrBonesPosition[ iChain ] );

			const int iParentBone = pStudioHdr->vecBoneParent[ iChain ];
			arrBonesToWorld[ iChain ] = ( iParentBone == -1 ? matRotation.ConcatTransforms( matBone ) : arrBonesToWorld[ iParentBone ].ConcatTransforms( matBone ) );
		}
	}
}

bool CSetupBones::SetupBones( CBaseEntity* pPlayer, matrix3x4a_t* arrBonesToWorld, int iMaxBones, int nBoneMask )
{
	if ( !pPlayer || !pPlayer->IsAlive() )
		return false;

	CMDLCacheCriticalSection mdlCacheCriticalSection(i::MDLCache);

	CStudioHdr* pStudioHdr = pPlayer->GetModelPtr( );
	if (pStudioHdr == nullptr)	
		return false;

	if (nBoneMask == -1)
		nBoneMask = pPlayer->GetPrevBoneMask( );
	else {
		// if we're setting up LOD N, we have set up all lower LODs also, because lower LODs always use subsets of the bones of higher LODs
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

	// get bone cache accessor and data it points to
	CBoneAccessor& boneAccessor = pPlayer->GetBoneAccessor( );
	int& nEFlags = pPlayer->GetEFlags( );

	// [side change] adapted for client's cache system, just adjust values that can be used by the game later
	int& nAccumulatedBoneMask = pPlayer->GetAccumulatedBoneMask( );
	pPlayer->GetRecentModelBoneCounter( ) = pPlayer->GetModelBoneCounter( );
	pPlayer->GetPrevBoneMask( ) = nAccumulatedBoneMask;
	nAccumulatedBoneMask = nBoneMask;
	pPlayer->GetLastSetupBonesTime( ) = i::GlobalVars->flCurrentTime;

	matrix3x4a_t* pBones = boneAccessor.matBones;

	if (arrBonesToWorld)
		boneAccessor.matBones = arrBonesToWorld;

	// allow access to the bones we're setting up
	boneAccessor.nReadableBones = nBoneMask;
	boneAccessor.nWritableBones = nBoneMask;

	nEFlags |= EFL_SETTING_UP_BONES;

	// [side change] server also taking into account 'm_flEstIkOffset' for NPC's, we can safely ignore this
	const Vector& vecAbsOrigin = pPlayer->GetAbsOrigin( );
	const Vector& angAbsView = pPlayer->GetAbsAngles( );

	BoneVector arrBonesPosition[ MAXSTUDIOBONES ];
	BoneQuaternionAligned arrBonesRotation[ MAXSTUDIOBONES ];

	CBoneBitList arrBonesComputed = { };

	// [side change] removed 'CBaseAnimating::CanSkipAnimation()' branch (PVS optimization for NPC's)
	if (IKContext* pIKContext = pPlayer->GetIKContext( ); pIKContext != nullptr)
	{
		pIKContext->Init( pStudioHdr, angAbsView, vecAbsOrigin, i::GlobalVars->flCurrentTime, i::GlobalVars->iFrameCount, boneAccessor.nWritableBones );

		GetSkeleton( pPlayer, pStudioHdr, arrBonesPosition, arrBonesRotation, boneAccessor.nWritableBones );

		pIKContext->UpdateTargets( arrBonesPosition, arrBonesRotation, boneAccessor.matBones, arrBonesComputed );
		pPlayer->CalculateIKLocks( i::GlobalVars->flCurrentTime );
		pIKContext->SolveDependencies( arrBonesPosition, arrBonesRotation, boneAccessor.matBones, arrBonesComputed );
	}
	else
		GetSkeleton( pPlayer, pStudioHdr, arrBonesPosition, arrBonesRotation, boneAccessor.nWritableBones );

	if (pPlayer->GetEffects( ) & EF_BONEMERGE)
	{
		if (CBaseEntity* pParent = reinterpret_cast<CBaseEntity*>( i::EntityList->GetClientEntityFromHandle( pPlayer->GetMoveParentHandle( ) ) ); pParent != nullptr)
			assert( false );
	}
	else if (CBoneMergeCache* pBoneMergeCache = pPlayer->GetBoneMergeCache( ); pBoneMergeCache != nullptr)
	{
		delete pBoneMergeCache;
		pBoneMergeCache = nullptr;

		pPlayer->GetBoneMergeCache( ) = pBoneMergeCache;
	}

	Studio_BuildMatrices( pStudioHdr, angAbsView, vecAbsOrigin, arrBonesPosition, arrBonesRotation, pPlayer->GetModelHierarchyScale( ), boneAccessor.matBones, boneAccessor.nWritableBones, arrBonesComputed );
	nEFlags &= ~EFL_SETTING_UP_BONES;

	if (nBoneMask & BONE_USED_BY_ATTACHMENT)
		pPlayer->SetupBones_AttachmentHelper( );

	pPlayer->ClampBonesInBBox( boneAccessor.matBones, boneAccessor.nWritableBones );

	if (arrBonesToWorld)
		std::memcpy( arrBonesToWorld, boneAccessor.matBones, sizeof( matrix3x4a_t ) * MAXSTUDIOBONES );

	boneAccessor.matBones = pBones;
}	