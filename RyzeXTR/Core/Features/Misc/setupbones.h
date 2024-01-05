#pragma once
#include "../../SDK/Entity.h"

class CIKContext
{
    // Not sure of the correct size, also don't care
    uint8_t pad[ 0x1070 ];

public:
    void* operator new( size_t size );
    void  operator delete( void* ptr );

    static void Construct( CIKContext* ik );
    void Init( const CStudioHdr* hdr, const Vector& local_angles, const BoneVector& local_origin, float current_time, int frame_count, int bone_mask );
    void UpdateTargets( BoneVector* pos, BoneQuaternion* q, matrix3x4_t* bone_cache, void* computed );
    void SolveDependencies( BoneVector* pos, BoneQuaternion* q, matrix3x4_t* bone_cache, void* computed );
    void ClearTargets( );
    void AddDependencies( mstudioseqdesc_t& seqdesc, int iSequence, float flCycle, const float poseParameters[ ], float flWeight );
    void CopyTo( CIKContext* other, const unsigned short* iRemapping );
};

struct CBoneSetup {
    CBoneSetup( const CStudioHdr* studio_hdr, int bone_mask, float* pose_parameters );

    void InitPose( BoneVector pos[ ], BoneQuaternion q[ ] );
    void AccumulatePose( BoneVector pos[ ], BoneQuaternion q[ ], int sequence, float cycle, float weight, float time, void* IKContext );
    void CalcAutoplaySequences( BoneVector pos[ ], BoneQuaternion q[ ], float real_time, void* IKContext );
    void CalcBoneAdj( BoneVector pos[ ], BoneQuaternion q[ ], const float controllers[ ] );

    const CStudioHdr* m_pStudioHdr;
    int m_boneMask;
    float* m_flPoseParameter;
    void* m_pPoseDebugger;
};

struct IBoneSetup {
    CBoneSetup* bone_setup;
};

class BoneSetup {
public:
    bool build( CBaseEntity* base_animating, matrix3x4a_t* bone_to_world, int max_bones, int bone_mask );
    bool build( CBaseEntity* base_animating, matrix3x4a_t* bone_to_world, int max_bones, int bone_mask, int extrapolte );

private:
    void get_skeleton( CBaseEntity* base_animating, CStudioHdr* studio_hdr, BoneVector* pos, BoneQuaternion* q, int bone_mask, CIKContext* ik );
    void update_dispatch_layer( CAnimationLayer* layer, CStudioHdr* studio_hdr );
    void build_matrices( CBaseEntity* base_animating, CStudioHdr* studio_hdr, BoneVector* pos, BoneQuaternion* q, matrix3x4a_t* bone_to_world, int bone_mask );
    void concat_transforms( const matrix3x4a_t& m0, const matrix3x4a_t& m1, matrix3x4a_t& out );
};
inline BoneSetup rebuiltBones;