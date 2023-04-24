#pragma once
#include "Lagcompensation.h"

enum BoneSetupFlags : int {
	None = 0,
	UseInterpolatedOrigin = (1 << 0),
	UseCustomOutput = (1 << 1),
	ForceInvalidateBoneCache = (1 << 2),
	AttachmentHelper = (1 << 3),
};

namespace features {
	namespace bones {
		/// <summary>
		/// rather use this on not drawn matrices (bone cache)
		/// </summary>
		/// <param name="player"></param>
		/// <param name="bone_to_world"> = BoneArray[256]</param>
		/// <param name="bone_mask"></param>
		/// <param name="curtime"></param>
		bool HandleBoneSetup(CBaseEntity* player, matrix3x4_t* bone_to_world, int bone_mask, float curtime);

		bool SetupBonesRebuild(CBaseEntity* entity, matrix3x4_t* pBoneMatrix, int nBoneCount, int boneMask, float time, int flags);

		/// <summary>
		/// @onetap we love you
		/// </summary>
		bool SetupBones(CBaseEntity* player, matrix3x4_t* pBoneToWorldOut, bool bSafeMatrix);
	}
}
