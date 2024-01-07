#include "setupbones.h"
#include "../../SDK/math.h"
#include "../../globals.h"

bool BoneSetup::build( CBaseEntity* base_animating, matrix3x4a_t* bone_to_world, int max_bones, int bone_mask )
{
    g::bSettingUpBones[ base_animating->EntIndex( ) ] = true;
    CMDLCacheCriticalSection criticalSection( i::MDLCache );
    {
        auto studio_hdr = base_animating->GetModelPtr( );

        if ( studio_hdr ) {
            alignas( 16 ) BoneVector pos[ MAXSTUDIOBONES ];
            alignas( 16 ) BoneQuaternion q[ MAXSTUDIOBONES ];

            // Allocate our own
            CIKContext* ik = new CIKContext;

            // Backup game ptr and set game to ours
            CIKContext* backup = reinterpret_cast< CIKContext* >( base_animating->GetIKContext( ) );
            base_animating->GetIKContext( ) = reinterpret_cast< IKContext* >( ik );

            uint8_t bone_computed[ 32 ]{};
            RtlSecureZeroMemory( &bone_computed, sizeof bone_computed );

            ik->Init( studio_hdr, base_animating->GetAbsAngles( ), base_animating->GetAbsOrigin( ), base_animating->GetSimulationTime( ), i::GlobalVars->iTickCount, bone_mask );
            get_skeleton( base_animating, studio_hdr, pos, q, bone_mask, ik );

            ik->UpdateTargets( pos, q, bone_to_world, &bone_computed );
            base_animating->CalculateIKLocks( base_animating->GetSimulationTime( ) ); // Need to set game's ik ptr for this
            ik->SolveDependencies( pos, q, bone_to_world, &bone_computed );

            // reset and cleanup
            base_animating->GetIKContext( ) = reinterpret_cast< IKContext* >( backup );
            delete ik;

            // TODO: phage - Implement MoveParent handling
            // Useless in csgo, never seen a single instance of MoveParents
            build_matrices( base_animating, studio_hdr, pos, q, base_animating->GetCachedBoneData( ).Base( ), bone_mask );
        }

        if ( bone_to_world && max_bones >= base_animating->GetCachedBoneData( ).Count( ) ) {
            memcpy( bone_to_world, base_animating->GetCachedBoneData( ).Base( ), sizeof( matrix3x4a_t ) * base_animating->GetCachedBoneData( ).Count( ) );
        }
    }

    base_animating->ClampBonesInBBox( bone_to_world, bone_mask );
    g::bSettingUpBones[ base_animating->EntIndex( ) ] = false;

    return true;
}

bool BoneSetup::build( CBaseEntity* base_animating, matrix3x4a_t* bone_to_world, int max_bones, int bone_mask, int extrapolte )
{
    g::bSettingUpBones[ base_animating->EntIndex( ) ] = true;
    CMDLCacheCriticalSection criticalSection( i::MDLCache );
    {
        auto studio_hdr = base_animating->GetModelPtr( );

        if ( studio_hdr ) {
            alignas( 16 ) BoneVector pos[ MAXSTUDIOBONES ];
            alignas( 16 ) BoneQuaternion q[ MAXSTUDIOBONES ];

            // Allocate our own
            CIKContext* ik = new CIKContext;

            // Backup game ptr and set game to ours
            CIKContext* backup = reinterpret_cast< CIKContext* >( base_animating->GetIKContext( ) );
            base_animating->GetIKContext( ) = reinterpret_cast< IKContext* >( ik );

            uint8_t bone_computed[ 32 ]{};
            RtlSecureZeroMemory( &bone_computed, sizeof bone_computed );

            ik->Init( studio_hdr, base_animating->GetAbsAngles( ), base_animating->GetAbsOrigin( ), base_animating->GetSimulationTime( ), i::GlobalVars->iTickCount + extrapolte, bone_mask );
            get_skeleton( base_animating, studio_hdr, pos, q, bone_mask, ik );

            ik->UpdateTargets( pos, q, bone_to_world, &bone_computed );
            base_animating->CalculateIKLocks( base_animating->GetSimulationTime( ) ); // Need to set game's ik ptr for this
            ik->SolveDependencies( pos, q, bone_to_world, &bone_computed );

            // reset and cleanup
            base_animating->GetIKContext( ) = reinterpret_cast< IKContext* >( backup );
            delete ik;

            // TODO: phage - Implement MoveParent handling
            // Useless in csgo, never seen a single instance of MoveParents
            build_matrices( base_animating, studio_hdr, pos, q, base_animating->GetCachedBoneData( ).Base( ), bone_mask );
        }

        if ( bone_to_world && max_bones >= base_animating->GetCachedBoneData( ).Count( ) ) {
            memcpy( bone_to_world, base_animating->GetCachedBoneData( ).Base( ), sizeof( matrix3x4a_t ) * base_animating->GetCachedBoneData( ).Count( ) );
        }
    }
    base_animating->ClampBonesInBBox( bone_to_world, bone_mask );
    g::bSettingUpBones[ base_animating->EntIndex( ) ] = false;

    return true;
}

// CBaseEntityOverlay::GetSkeleton
void BoneSetup::get_skeleton( CBaseEntity* base_animating, CStudioHdr* studio_hdr, BoneVector* pos, BoneQuaternion* q, int bone_mask, CIKContext* ik )
{
    CBoneSetup boneSetup( studio_hdr, bone_mask, base_animating->GetPoseParameter( ).data( ) );
    boneSetup.InitPose( pos, q );
    boneSetup.AccumulatePose( pos, q, base_animating->GetSequence( ), base_animating->GetCycle( ), 1.f, base_animating->GetSimulationTime( ), ik );

    // sort the layers
    constexpr int MAX_LAYER_COUNT = 15;
    int layer[ MAX_LAYER_COUNT ]{};
    for ( int i = 0; i < 13; i++ ) {
        layer[ i ] = MAX_LAYER_COUNT;
    }

    for ( int i = 0; i < 13; i++ ) {
        auto pLayer = &base_animating->GetAnimationOverlays( )[ i ];
        if ( ( pLayer->flWeight > 0.f ) && pLayer->nSequence != -1 && pLayer->iOrder >= 0 && pLayer->iOrder < 13 ) {
            layer[ pLayer->iOrder ] = i;
        }
    }

    CBaseCombatWeapon* weapon = nullptr;
    CBaseEntity* weapon_world_model = nullptr;
    CBoneMergeCache* bone_merge_cache = nullptr;
    CBoneMergeCache* bone_merge_backup = nullptr;

    bool do_weapon_setup = false;

    if ( base_animating->GetClientClass( )->nClassID == EClassIndex::CCSPlayer ) {
        CBaseEntity* player = base_animating;
        weapon = player->GetWeapon( );
        if ( weapon ) {
            weapon_world_model = ( CBaseEntity* )( i::EntityList->GetClientEntityFromHandle( weapon->GetRefEHandle( ) ) );
            // Currently just return true in HoldsPlayerAnimations, something fucked on knives and I can't be bothered
            if ( weapon_world_model && weapon_world_model->GetModelPtr( ) && weapon_world_model->GetHoldPlayerAnimations( ) ) {
                bone_merge_backup = weapon_world_model->GetBoneMergeCache( );

                bone_merge_cache = new CBoneMergeCache;
                weapon_world_model->GetBoneMergeCache( ) = bone_merge_cache;
                weapon_world_model->GetBoneMergeCache( )->Init( weapon_world_model );

                if ( weapon_world_model->GetBoneMergeCache( ) )
                    do_weapon_setup = true;
            }
        }
    }

    if ( do_weapon_setup ) {
        CStudioHdr* weapon_studio_hdr = weapon_world_model->GetModelPtr( );

        weapon_world_model->GetBoneMergeCache( )->MergeMatchingPoseParams( );

        CIKContext* weaponIK = new CIKContext;
        weaponIK->Init( weapon_studio_hdr, base_animating->GetAbsAngles( ), base_animating->GetAbsOrigin( ), base_animating->GetSimulationTime( ), 0, BONE_USED_BY_BONE_MERGE );

        CBoneSetup weaponSetup( weapon_studio_hdr, BONE_USED_BY_BONE_MERGE, weapon_world_model->GetPoseParameter( ).data( ) );
        BoneVector weaponPos[ 256 ];
        BoneQuaternion weaponQ[ 256 ];

        weaponSetup.InitPose( weaponPos, weaponQ );

        for ( int i = 0; i < 13; i++ ) {
            CAnimationLayer* pLayer = &base_animating->GetAnimationOverlays( )[ i ];
            if ( pLayer->nSequence <= 1 || pLayer->flWeight <= 0.f )
                continue;

            // TODO: phage - This should be moved to the end of animation update,
            // will break shit if we happen to call setupbones more than once per
            // tick on an entity
            // Alternatively, we ensure that the above condition never happens
            base_animating->UpdateDispatchLayer( pLayer, weapon_studio_hdr, pLayer->nSequence );

            if ( pLayer->nDispatchedDst <= 0 || pLayer->nDispatchedDst >= studio_hdr->GetNumSeq( ) ) {
                boneSetup.AccumulatePose( pos, q, pLayer->nSequence, pLayer->flCycle, pLayer->flWeight, base_animating->GetSimulationTime( ), ik );
            }
            else {
                weapon_world_model->GetBoneMergeCache( )->CopyFromFollow( pos, q, BONE_USED_BY_BONE_MERGE, weaponPos, weaponQ );

                mstudioseqdesc_t& seqdesc = studio_hdr->pSeqdesc( pLayer->nSequence );
                ik->AddDependencies( seqdesc, pLayer->nSequence, pLayer->flCycle, base_animating->GetPoseParameter( ).data( ), pLayer->flWeight );

                weaponSetup.AccumulatePose( weaponPos, weaponQ, pLayer->nDispatchedDst, pLayer->flCycle, pLayer->flWeight, base_animating->GetSimulationTime( ), weaponIK );

                weapon_world_model->GetBoneMergeCache( )->CopyToFollow( weaponPos, weaponQ, BONE_USED_BY_BONE_MERGE, pos, q );

                weaponIK->CopyTo( ik, weapon_world_model->GetBoneMergeCache( )->arrRawIndexMapping );
            }
        }

        delete weaponIK;
    }
    else {
        for ( int i = 0; i < 13; i++ ) {
            if ( layer[ i ] >= 0 && layer[ i ] < 13 ) {
                CAnimationLayer pLayer = base_animating->GetAnimationOverlays( )[ layer[ i ] ];
                boneSetup.AccumulatePose( pos, q, pLayer.nSequence, pLayer.flCycle, pLayer.flWeight, base_animating->GetSimulationTime( ), ik );
            }
        }
    }

    CIKContext* auto_ik = new CIKContext;
    auto_ik->Init( studio_hdr, base_animating->GetAbsAngles( ), base_animating->GetAbsOrigin( ), base_animating->GetSimulationTime( ), 0, bone_mask );
    boneSetup.CalcAutoplaySequences( pos, q, base_animating->GetSimulationTime( ), auto_ik );
    delete auto_ik;

    if ( studio_hdr->pStudioHdr->nBoneControllers > 0 ) {
        boneSetup.CalcBoneAdj( pos, q, base_animating->GetEncodedControllerArray( ).data( ) );
    }

    if ( weapon_world_model && bone_merge_cache ) {
        weapon_world_model->GetBoneMergeCache( ) = bone_merge_backup;
        delete bone_merge_cache;
    }
}

// you can choose to rebuild this or sig the client's, they're identical 
void BoneSetup::update_dispatch_layer( CAnimationLayer* layer, CStudioHdr* studio_hdr )
{
    //if( !studio_hdr || !layer )
    //{
    //  if( layer )
    //  {
    //    layer->nDispatchedDst = -1;
    //  }
    //
    //  return;
    //}
    //
    //if( layer->pStudioHdr != studio_hdr || layer->nDispatchedSrc != layer->nSequence || layer->nDispatchedSrc >= studio_hdr->GetNumSeq() )
    //{
    //  layer->pStudioHdr = studio_hdr;
    //  layer->nDispatchedSrc = layer->nSequence;
    //
    //  const char* pszLayerName = GetSequenceName( layer->nSequence );
    //  layer->nDispatchedDst = studio_hdr->LookupSequence( pszLayerNAme );
    //}
}

void BoneSetup::build_matrices( CBaseEntity* base_animating, CStudioHdr* studio_hdr, BoneVector* pos, BoneQuaternion* q, matrix3x4a_t* bone_to_world, int bone_mask )
{
    int i = 0, j = 0;
    int chain[ MAXSTUDIOBONES ] = {};
    int chain_length = studio_hdr->pStudioHdr->nBones;

    for ( i = 0; i < studio_hdr->pStudioHdr->nBones; i++ ) {
        chain[ chain_length - i - 1 ] = i;
    }

    matrix3x4a_t bone_matrix = {};
    matrix3x4a_t rotation_matrix = {};
    M::AngleMatrix( base_animating->GetAbsAngles( ), base_animating->GetAbsOrigin( ), rotation_matrix );

    for ( j = chain_length - 1; j >= 0; j-- ) {
        i = chain[ j ];

        auto bone = studio_hdr->pStudioHdr->GetBone( i );
        if ( bone->iFlags & bone_mask ) {

            M::QuaternionMatrix( q[ i ], pos[ i ], bone_matrix );

            int bone_parent = bone->iParent;
            if ( bone_parent == -1 ) {
                concat_transforms( rotation_matrix, bone_matrix, bone_to_world[ i ] );
            }
            else {
                concat_transforms( bone_to_world[ bone_parent ], bone_matrix, bone_to_world[ i ] );
            }
        }
    }
}

void BoneSetup::concat_transforms( const matrix3x4a_t& m0, const matrix3x4a_t& m1, matrix3x4a_t& out )
{
    for ( int i = 0; i < 3; i++ ) {
        // Normally, you can't just multiply 2 3x4 matrices together, so translation is done separately
        out[ i ][ 3 ] = m1[ 0 ][ 3 ] * m0[ i ][ 0 ] + m1[ 1 ][ 3 ] * m0[ i ][ 1 ] + m1[ 2 ][ 3 ] * m0[ i ][ 2 ] + m0[ i ][ 3 ]; // translation

        for ( int j = 0; j < 3; j++ ) // rotation/scale
        {
            out[ i ][ j ] = m0[ i ][ 0 ] * m1[ 0 ][ j ] + m0[ i ][ 1 ] * m1[ 1 ][ j ] + m0[ i ][ 2 ] * m1[ 2 ][ j ];
        }
    }
}

// IK_Context section:
void* CIKContext::operator new( size_t size )
{
    CIKContext* ptr = ( CIKContext* )i::MemAlloc->Alloc( size );
    Construct( ptr );

    return ptr;
}

void CIKContext::operator delete( void* ptr )
{
    i::MemAlloc->Free( ptr );
}

void CIKContext::Construct( CIKContext* ik )
{
    using ConstructFn = CIKContext * ( __fastcall* )( CIKContext* );
    static ConstructFn Construct = reinterpret_cast< ConstructFn >( MEM::GetAbsoluteAddress( MEM::FindPattern( CLIENT_DLL, XorStr( "E8 ? ? ? ? A1 ? ? ? ? FF 75 18" ) ) + 0x1 ) );
    // client.dll - \x53\x8B\xD9\xF6\xC3\x03\x74\x0B\xFF\x15\x00\x00\x00\x00\x84\xC0\x74\x01\xCC\xC7\x83\x00\x00\x00\x00\x00\x00\x00\x00\x8B\xCB

    Construct( ik );
}

void CIKContext::Init( const CStudioHdr* hdr, const Vector& local_angles, const Vector& local_origin, float current_time, int frame_count, int bone_mask )
{
    using InitFn = void( __thiscall* )( CIKContext*, const CStudioHdr*, const Vector&, const Vector&, float, int, int );
    static InitFn Init = reinterpret_cast<InitFn>( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D" ) ) );
    // client.dll - \x55\x8B\xEC\x83\xEC\x08\x8B\x45\x08\x56\x57\x8B\xF9\x8D

    Init( this, hdr, local_angles, local_origin, current_time, frame_count, bone_mask );
}

void CIKContext::UpdateTargets( BoneVector* pos, BoneQuaternion* q, matrix3x4_t* bone_cache, void* computed )
{
    using UpdateTargetsFn = void( __thiscall* )( CIKContext*, BoneVector*, BoneQuaternion*, matrix3x4_t*, void* );
    static UpdateTargetsFn UpdateTargets = reinterpret_cast<UpdateTargetsFn>( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 33 D2 89" ) ) );
    // client.dll - \x55\x8B\xEC\x83\xE4\xF0\x81\xEC\x00\x00\x00\x00\x33\xD2

    UpdateTargets( this, pos, q, bone_cache, computed );
}

void CIKContext::SolveDependencies( BoneVector* pos, BoneQuaternion* q, matrix3x4_t* bone_cache, void* computed )
{
    using SolveDependenciesFn = void( __thiscall* )( CIKContext*, BoneVector*, BoneQuaternion*, matrix3x4_t*, void* );
    static SolveDependenciesFn SolveDependencies = reinterpret_cast<SolveDependenciesFn>( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81" ) ) );
    // client.dll - \x55\x8B\xEC\x83\xE4\xF0\x81\xEC\x00\x00\x00\x00\x8B\x81

    SolveDependencies( this, pos, q, bone_cache, computed );
}

struct IKTarget {
    int m_iFrameCount;

private:
    char pad_00004[ 0x51 ];
};

void CIKContext::ClearTargets( )
{
    int m_iTargetCount = *( int* )( ( uintptr_t )this + 0xFF0 );
    auto m_pIkTarget = ( IKTarget* )( ( uintptr_t )this + 0xD0 );
    for ( int i = 0; i < m_iTargetCount; i++ ) {
        m_pIkTarget->m_iFrameCount = -9999;
        m_pIkTarget++;
    }
}

void CIKContext::AddDependencies( mstudioseqdesc_t& seqdesc, int iSequence, float flCycle, const float poseParameters[ ], float flWeight )
{
    using AddDependenciesFn = void( __thiscall* )( CIKContext*, mstudioseqdesc_t&, int, float, const float[ ], float );
    static AddDependenciesFn AddDependencies = reinterpret_cast< AddDependenciesFn >( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 0F" ) ) );
    // server.dll - \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x56\x57\x8B\xF9\x0F\x28\xCB\xF3\x0F\x11\x4D\x00

    AddDependencies( this, seqdesc, iSequence, flCycle, poseParameters, flWeight );
}

void CIKContext::CopyTo( CIKContext* other, const unsigned short* iRemapping )
{
    using CopyToFn = void( __thiscall* )( CIKContext*, CIKContext*, const unsigned short* );
    static CopyToFn CopyTo = reinterpret_cast<CopyToFn>( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 EC 24 8B 45 08 57 8B F9 89 7D F4 85 C0" ) ) );
    // server.dll - \x55\x8B\xEC\x83\xEC\x24\x8B\x45\x08\x57\x8B\xF9\x89\x7D\xF4\x85\xC0

    CopyTo( this, other, iRemapping );
}

CBoneSetup::CBoneSetup( const CStudioHdr* studio_hdr, int bone_mask, float* pose_parameters )

    : m_pStudioHdr( studio_hdr )
    , m_boneMask( bone_mask )
    , m_flPoseParameter( pose_parameters )
    , m_pPoseDebugger( nullptr )
{
}

void CBoneSetup::InitPose( BoneVector pos[ ], BoneQuaternion q[ ] )
{
    auto hdr = m_pStudioHdr->pStudioHdr;

    for ( int i = 0; i < hdr->nBones; i++ ) {
        auto bone = hdr->GetBone( i );

        if ( bone->iFlags & m_boneMask ) {
            pos[ i ] = bone->vecPosition;
            q[ i ] = bone->qAlignment; // qWorld or qAlignment idk for sure
        }
    }
}

void CBoneSetup::AccumulatePose( BoneVector pos[ ], BoneQuaternion q[ ], int sequence, float cycle, float weight, float time, void* IKContext )
{
    using AccumulatePoseFn = void( __thiscall* )( CBoneSetup*, BoneVector*, BoneQuaternion*, int, float, float, float, void* );
    static AccumulatePoseFn AccumulatePose = reinterpret_cast<AccumulatePoseFn>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1" ) ) ); // client.dll - \x55\x8B\xEC\x83\xE4\xF0\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1
   
    AccumulatePose( this, pos, q, sequence, cycle, weight, time, IKContext );
}

void CBoneSetup::CalcAutoplaySequences( BoneVector pos[ ], BoneQuaternion q[ ], float real_time, void* IKContext )
{
    using CalcAutoplaySequencesFn = void( __thiscall* )( CBoneSetup*, BoneVector*, BoneQuaternion*, float, void* );
    static CalcAutoplaySequencesFn CalcAutoplaySequences = reinterpret_cast< CalcAutoplaySequencesFn >( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 EC 10 53 56 57 8B 7D 10" ) ) ); // client.dll - \x55\x8B\xEC\x83\xEC\x10\x53\x56\x57\x8B\x7D\x10

    // Thanks clang!
    __asm
    {
        mov ecx, this
        movss xmm0, real_time
        push IKContext
        push q
        push pos
        call CalcAutoplaySequences
    }
}

void CBoneSetup::CalcBoneAdj( BoneVector pos[ ], BoneQuaternion q[ ], const float controllers[ ] )
{
    using CalcBoneAdjFn = void( __thiscall* )( CBoneSetup*, const CStudioHdr*, BoneVector*, BoneQuaternion*, const float*, int );
    static CalcBoneAdjFn CalcBoneAdj = reinterpret_cast<CalcBoneAdjFn>( MEM::FindPattern( CLIENT_DLL, XorStr( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89" ) ) ); // client.dll - \x55\x8B\xEC\x83\xE4\xF8\x81\xEC\x00\x00\x00\x00\x8B\xC1\x89

    // Thanks clang!
    __asm
    {
        mov ecx, m_pStudioHdr
        mov edx, this
        push m_boneMask
        push controllers
        push q
        push pos
        call CalcBoneAdj
    }
}