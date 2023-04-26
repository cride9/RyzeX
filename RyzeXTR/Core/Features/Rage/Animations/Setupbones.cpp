#include "Setupbones.h"
#define BONE_USED_BY_SERVER BONE_USED_BY_ANYTHING | BONE_USED_BY_VERTEX_LOD0 | BONE_USED_BY_VERTEX_LOD1 | BONE_USED_BY_VERTEX_LOD2 | BONE_USED_BY_VERTEX_LOD3 | BONE_USED_BY_VERTEX_LOD4 | BONE_USED_BY_VERTEX_LOD5 | BONE_USED_BY_VERTEX_LOD6 | BONE_USED_BY_VERTEX_LOD7

bool features::bones::HandleBoneSetup(CBaseEntity* player, matrix3x4_t* bone_to_world, int bone_mask, float curtime)
{
	if (!player || !player->GetClientClass() || player->GetClientClass()->nClassID != EClassIndex::CCSPlayer || !player->IsAlive())
		return false;

	CStudioHdr* hdr = player->GetModelPtr();
	if (!hdr)
		return false;

	matrix3x4_t* backup_bones = player->GetBoneAccessor()->matBones;
	if (!backup_bones)
		return false;

	Vector abs_origin = player->GetAbsOrigin();
	Vector abs_angles = player->GetAbsAngles();

	Vector backup_abs_origin = abs_origin;
	Vector backup_abs_angles = abs_angles;

	matrix3x4a_t parent_transform;
	M::AngleMatrix(abs_angles, abs_origin, parent_transform);

	int backup_effects = player->GetEffects();
	player->GetEffects() |= EF_NOINTERP;

	player->SetAbsOrigin(abs_origin);
	player->SetAbsAngles(abs_angles);

	IKContext* IK_context = player->GetIKContext();
	if (IK_context)
	{
		IK_context->ClearTargets();
		IK_context->Init(hdr, abs_angles, player->GetVecOrigin(),
			i::GlobalVars->flCurrentTime, i::GlobalVars->iFrameCount, BONE_USED_BY_SERVER);

		//IK_context->Init( hdr , abs_angles , abs_origin ,
		//	curtime , sdk::interfaces::global_vars->tickcount , bone_mask );
	}

	Vector pos[ /*128*/ 256]{};
	Quaternion q[ /*128*/ 256]{};
	uint8_t boneComputed[256]{};

	player->GetBoneAccessor()->matBones = bone_to_world;

	uint32_t fBackupOcclusionFlags = player->GetOcclusionFlags();
	player->GetOcclusionFlags() |= 0xA; // skipp call to accumulatelayers in standardblendingrules
	player->StandardBlendingRules(hdr, pos, q, curtime, bone_mask);
	player->GetOcclusionFlags() = fBackupOcclusionFlags; // standardblendingrules was called now restore

	if (IK_context)
	{
		player->UpdateIKLocks(i::GlobalVars->flCurrentTime /*curtime*/);
		IK_context->UpdateTargets(pos, q, bone_to_world, &boneComputed[0] /*boneComputed*/);
		player->CalculateIKLocks(i::GlobalVars->flCurrentTime /*curtime*/);
		IK_context->SolveDependencies(pos, q, bone_to_world, &boneComputed[0] /*boneComputed*/);
	}

	player->BuildTransformations(hdr, pos, q, parent_transform, bone_mask, &boneComputed[0] /*boneComputed*/);

	player->GetEffects() = backup_effects;

	player->GetBoneAccessor()->matBones = backup_bones;

	player->SetAbsOrigin(backup_abs_origin);
	player->SetAbsAngles(backup_abs_angles);

	return true;
}

bool features::bones::SetupBonesRebuild(CBaseEntity* entity, matrix3x4_t* pBoneMatrix, int nBoneCount, int boneMask, float time, int flags)
{
	if (!entity)
		return false;

	if (entity->GetSequence() == -1)
		return false;

	if (boneMask == -1) {
		boneMask = entity->GetPrevBoneMask();
	}

	boneMask = boneMask | 0x80000; // BONE_ALWAYS_SETUP

	// If we're setting up LOD N, we have set up all lower LODs also
	// because lower LODs always use subsets of the bones of higher LODs.
	int nLOD = 0;
	int nMask = BONE_USED_BY_VERTEX_LOD0;
	for (; nLOD < MAX_NUM_LODS; ++nLOD, nMask <<= 1) {
		if (boneMask & nMask)
			break;
	}
	for (; nLOD < MAX_NUM_LODS; ++nLOD, nMask <<= 1) {
		boneMask |= nMask;
	}

	CBoneAccessor backup_bone_accessor = *entity->GetBoneAccessor();
	CBoneAccessor* bone_accessor = entity->GetBoneAccessor();
	if (!bone_accessor)
		return false;

	static auto _InvalidateBoneCache = reinterpret_cast<uintptr_t>(util::FindSignature("client.dll", "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81"));
	unsigned long model_bone_counter = **(unsigned long**)(_InvalidateBoneCache + 0x000A);

	if (entity->GetRecentModelBoneCounter() != model_bone_counter || (flags & BoneSetupFlags::ForceInvalidateBoneCache)) {
		if (FLT_MAX >= entity->GetLastSetupBonesTime() || time < entity->GetLastSetupBonesTime()) {
			bone_accessor->nReadableBones = 0;
			bone_accessor->nWritableBones = 0;
			entity->GetLastSetupBonesTime() = (time);
		}

		entity->GetPrevBoneMask() = entity->GetAccumulatedBoneMask();
		entity->GetAccumulatedBoneMask() = 0;

		auto hdr = entity->GetStudioHdr();
		if (hdr) { // profiler stuff
			hdr->m_nPerfAnimatedBones = 0;
			hdr->m_nPerfUsedBones = 0;
			hdr->m_nPerfAnimationLayers = 0;
		}
	}

	// we don't need bones to jiggle
	//entity->m_isJiggleBonesEnabled() = false;

	// Keep track of everything asked for over the entire frame
	// But not those things asked for during bone setup
	entity->GetAccumulatedBoneMask() |= boneMask;

	// fix enemy poses getting raped when going out of pvs
	entity->GetOcclusionFrameCount() = 0;
	entity->GetOcclusionFlags() = 0;

	// Make sure that we know that we've already calculated some bone stuff this time around.
	entity->GetRecentModelBoneCounter() = model_bone_counter;

	bool bReturnCustomMatrix = (flags & BoneSetupFlags::UseCustomOutput) && pBoneMatrix;
	CStudioHdr* hdr = entity->GetStudioHdr();
	if (!hdr) {
		return false;
	}

	// Setup our transform based on render angles and origin.
	Vector origin = (flags & BoneSetupFlags::UseInterpolatedOrigin) ? entity->GetAbsOrigin() : entity->GetVecOrigin();
	Vector angles = entity->GetAbsAngles();

	matrix3x4a_t parentTransform;
	M::AngleMatrix(angles, origin, parentTransform);

	boneMask |= entity->GetPrevBoneMask();

	if (bReturnCustomMatrix) {
		bone_accessor->matBones = pBoneMatrix;
	}

	// Allow access to the bones we're setting up so we don't get asserts in here.
	int oldReadableBones = bone_accessor->nReadableBones;
	int oldWritableBones = bone_accessor->nWritableBones;
	int newWritableBones = oldReadableBones | boneMask;
	bone_accessor->nWritableBones = newWritableBones;
	bone_accessor->nReadableBones = newWritableBones;

	if (!(hdr->pStudioHdr->iFlags & 0x00000010)) {
		entity->GetEffects() |= EF_NOINTERP;

		entity->GetEFlags() |= EFL_SETTING_UP_BONES;

		entity->GetIKContext2() = nullptr;
		entity->GetOffset<unsigned short>( 0x68 ) |= 2; // ENTCLIENTFLAGS_DONTUSEIK

		alignas(16) Vector pos[128];
		alignas(16) Quaternion q[128];
		uint8_t computed[0x100];

		entity->StandardBlendingRules(hdr, pos, q, time, boneMask);

		std::memset(computed, 0, 0x100);
		entity->BuildTransformations(hdr, pos, q, parentTransform, boneMask, computed);

		entity->GetEFlags() &= ~EFL_SETTING_UP_BONES;

		// entity->ControlMouth( hdr );

		if (!bReturnCustomMatrix /*&& !bSkipAnimFrame*/) {
			// https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/game/client/c_baseanimating.cpp#L3390
			// for future updates we need to sig it..
			memcpy((void*)(uintptr_t(entity) + 0xA68), &pos[0], sizeof( Vector ) * hdr->pStudioHdr->nBones);
			memcpy((void*)(uintptr_t(entity) + 0x166C), &q[0], sizeof( Quaternion ) * hdr->pStudioHdr->nBones );
		}
	}
	else {
		parentTransform = bone_accessor->matBones[0];
	}

	if ( /*boneMask & BONE_USED_BY_ATTACHMENT*/ flags & BoneSetupFlags::AttachmentHelper) {
		entity->SetupBones_AttachmentHelper( );
	/*	static auto SetupBones_AttachmentHelper = patterns::SetupBones_AttachmentHelper.get< void(__thiscall*)(void*, CStudioHdr*)>();
		SetupBones_AttachmentHelper(entity, hdr);*/
	}

	// don't override bone cache if we're just generating a standalone matrix
	if (bReturnCustomMatrix) {
		*bone_accessor = backup_bone_accessor;

		return true;
	}

	return true;
}