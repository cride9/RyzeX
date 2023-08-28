#pragma once
#include "../../../SDK/Entity.h"

class CSetupBones
{
public:
	bool SetupBones( CBaseEntity* pPlayer, matrix3x4_t* arrBonesToWorld, int iMaxBones, int nBoneMask );
};

inline CSetupBones setupbones;