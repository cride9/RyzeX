#pragma once

class CBoneMergeCache
{
public:
	CBoneMergeCache()
	{
		Init(nullptr);
	}

	void* operator new(const std::size_t nSize)
	{
		return i::MemAlloc->Alloc(nSize);
	}

	void operator delete(void* pMemory)
	{
		i::MemAlloc->Free(pMemory);
	}

	void Init(CBaseAnimating* pInitialOwner)
	{
		// @ida CBoneMergeCache::Init(): server.dll -> ABS["E8 ? ? ? ? 8B 4C 24 10 83 B9" + 0x1]
		static auto fnInit = reinterpret_cast<void(__thiscall*)(CBoneMergeCache*, CBaseAnimating*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? FF 75 08 8B 8E")) + 0x1));
		fnInit(this, pInitialOwner);
	}

	// update the lookups that let it merge bones quickly
	void UpdateCache()
	{
		// @ida CBoneMergeCache::UpdateCache(): server.dll -> "55 8B EC 83 EC 10 53 8B D9 57"
		static auto fnUpdateCache = reinterpret_cast<void(__thiscall*)(CBoneMergeCache*)>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 14 53 56 57 8B F9 8B 37")));
		fnUpdateCache(this);
	}

	void MergeMatchingPoseParams()
	{
		UpdateCache();

		if (pOwnerHdr == nullptr || vecMergedBones.Count() == 0)
			return;

		for (int i = 0; i < MAXSTUDIOPOSEPARAM; i++)
		{
			if (arrOwnerToFollowPoseParamMapping[i] != -1)
			{
				pOwner->SetPoseParameter(pOwnerHdr, arrOwnerToFollowPoseParamMapping[i], pFollow->GetPoseParameter(pFollowHdr, i));
			}
		}
	}

	void CopyFromFollow(const BoneVector_t* arrFollowPositions, const BoneQuaternion_t* arrFollowRotations, int nBoneMask, BoneVector_t* arrMyPositions, BoneQuaternion_t* arrMyRotations)
	{
		// @ida CBoneMergeCache::CopyFromFollow(): server.dll -> ABS["E8 ? ? ? ? 8B 44 24 08 83 B8" + 0x1]
		static auto fnCopyFromFollow = reinterpret_cast<void(__thiscall*)(CBoneMergeCache*, const BoneVector_t*, const BoneQuaternion_t*, int, BoneVector_t*, BoneQuaternion_t*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? F3 0F 10 45 ? 8D 84 24")) + 0x1));
		fnCopyFromFollow(this, arrFollowPositions, arrFollowRotations, nBoneMask, arrMyPositions, arrMyRotations);
	}

	void CopyToFollow(const BoneVector_t* arrMyPositions, const BoneQuaternion_t* arrMyRotations, int nBoneMask, BoneVector_t* arrFollowPositions, BoneQuaternion_t* arrFollowRotations)
	{
		// @ida CBoneMergeCache::CopyToFollow(): server.dll -> ABS["E8 ? ? ? ? 8B 4C 24 10 8B 81 ? ? ? ? 8D 4C 24 60" + 0x1]
		static auto fnCopyToFollow = reinterpret_cast<void(__thiscall*)(CBoneMergeCache*, const BoneVector_t*, const BoneQuaternion_t*, int, BoneVector_t*, BoneQuaternion_t*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? 8B 87 ? ? ? ? 8D 8C 24 ? ? ? ? 8B 7C 24 18")) + 0x1));
		fnCopyToFollow(this, arrMyPositions, arrMyRotations, nBoneMask, arrFollowPositions, arrFollowRotations);
	}

	/// @returns: true if the specified bone is one that got merged, false otherwise
	//[[nodiscard]] int IsBoneMerged(int iBone) const;

	void ForceCacheClear()
	{
		bForceCacheClear = true;
		UpdateCache();
	}

public:
	class CMergedBone
	{
	public:
		unsigned short iMyBone; // index of merge cache's owner's bone
		unsigned short iParentBone; // index of follow's matching bone
	};

	CBaseEntity* pOwner; // 0x0000 // this is the entity that we're keeping the cache updated for

	// all the cache data is based off these. when they change, the cache data is regenerated. these are either all valid pointers or all null
	CBaseEntity* pFollow; // 0x0004
	CStudioHdr* pFollowHdr; // 0x0008
	const studiohdr_t* pFollowRenderHdr; // 0x000C
	CStudioHdr* pOwnerHdr; // 0x10
	const studiohdr_t* pOwnerRenderHdr; // 0x0014
	int nCopiedFrameCount; // 0x0018 // keeps track if this entity is part of a reverse bonemerge
	int nFollowBoneSetupMask; // 0x001C // this is the mask we need to use to set up bones on the followed entity to do the bone merge

	int arrOwnerToFollowPoseParamMapping[MAXSTUDIOPOSEPARAM]; // 0x0020 // this is an array of pose param indices on the follower to pose param indices on the owner
	CUtlVector<CMergedBone> vecMergedBones; // 0x0080
	std::byte pad0[0xC]; // CVarBitVec vecBoneMergeBits; // 0x0094
	unsigned short arrRawIndexMapping[MAXSTUDIOBONES]; // 0x00A0
	bool bForceCacheClear; // 0x02A0
};