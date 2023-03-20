#pragma once
#include "SDK/CUserCmd.h"
#include "SDK/Entity.h"
#include "Interface/Interfaces/entityListener.h"

namespace g {

	inline CUserCmd* pCmd;
	inline CBaseEntity* pLocal;
	inline CEntityListener entityListener;

	inline int defensiveTickbase;

	inline bool bAllowAnimations[65];
	inline bool* bSendPacket;
	inline bool bShifting;
	inline bool bWaiting;
	inline bool bAntiaimEnabled;
	inline int onetapV2ShotHiding;

	inline bool bSettingUpBones[65];

	inline int nRestoreTickbase;
	inline float flRestoreCurtime;
	inline bool bRestoreGlobals;

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