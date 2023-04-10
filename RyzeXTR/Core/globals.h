#pragma once
#include "SDK/CUserCmd.h"
#include "SDK/Entity.h"
#include "Interface/Interfaces/entityListener.h"

namespace g {

	inline CUserCmd* pCmd;
	inline CBaseEntity* pLocal;
	inline CEntityListener entityListener;
	inline bool bNotInServer;

	inline bool bAllowAnimations[65];
	inline bool* bSendPacket;
	inline bool bWaiting;
	inline bool bAntiaimEnabled;

	inline bool bSettingUpBones[65];

	namespace localprediction {

		namespace before {

			inline int nFlags;
			inline int nMoveType;
		}
		namespace after {

			inline int nFlags;
			inline int nMoveType;
		}
	}

	namespace fonts {
		inline HFont HealthESP;
		inline HFont NameESP;
	}
}