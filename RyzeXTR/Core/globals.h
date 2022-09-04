#pragma once
#include "SDK/CUserCmd.h"
#include "SDK/Entity.h"
namespace g {

	inline CUserCmd* pCmd;
	inline CBaseEntity* pLocal;
	inline Vector oldViewAngle;

	inline int defensiveTickbase;

	inline bool bAllowAnimations[65];
	inline bool bSendPacket;
	inline bool bShifting;
	inline bool bWaiting;
	inline bool bAntiaimEnabled;

	inline bool bSettingUpBones = false;

	inline int nRestoreTickbase;
	inline float flRestoreCurtime;
	inline bool bRestoreGlobals;

	inline Vector multiPoint[65][18][9];

	namespace predicted {

		inline int nFlags;
		inline int nMoveType;
	}

	namespace fonts {
		inline HFont HealthESP;
		inline HFont NameESP;
	}
}