#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"

enum EPITCH : int {

	OFF,
	UP,
	ZERO,
	DOWN
};

enum EYAW : int {

	FORWARD,
	BACKWARD
};

enum EDESYNC : int {

	NONE,
	STATIC,
	EXTENDED,
	JITTER,
	FLICK
};

namespace antiaim {

	inline bool needMicromovement = false;

	inline float desyncValue = 0.f;
	void AntiAim(CUserCmd*, bool&);
	
	enum LBYUpdateType : int
	{
		LBYUPDATE_None,
		LBYUPDATE_Standing,
		LBYUPDATE_Moving
	};
	inline float NextLBYUpdateTime = -1.f;
	inline float m_flNextLBYUpdate = -1.f, m_flLastLBYChange = -1.f;
	// update count
	inline int iCountUpdates{};
	bool NextLBYUpdate( CUserCmd* cmd );
	void ForceResync( CUserCmd* m_pCmd, int m_iLbyChange );
	void Update( CUserCmd* m_pCmd );
	
	bool FreeStandingDistance(CUserCmd*, Vector&);
	bool FreeStandingThreat(Vector& angle);
	int ClosestToLocal();
	void AtTarget(CUserCmd*, Vector&);
}