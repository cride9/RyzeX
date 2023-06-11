#pragma once
#include "SDK/CUserCmd.h"
#include "SDK/Entity.h"
#include "Interface/Interfaces/entityListener.h"

#define _DEBUGTEXT "Cride Is A Nigger :kekw:"

namespace g {

	inline CUserCmd* pCmd;
	inline CBaseEntity* pLocal;
	inline CEntityListener entityListener;
	inline Vector vecOriginalViewAngle;

	inline bool bAllowAnimations[65];
	inline bool bWaiting;
	inline bool bNewTick[65];
	inline bool bUpdatingSkins = false;

	inline bool bSettingUpBones[65];

	inline std::vector<Vector> drawList{};

	namespace fonts {
		inline HFont HealthESP;
		inline HFont NameESP;
		inline HFont FlagESP;
		inline HFont SkeetFont;
	}
}