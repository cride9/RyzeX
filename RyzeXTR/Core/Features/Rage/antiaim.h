#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"

enum EPITCH : int {

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
	JITTER,
};

namespace antiaim {

	inline float flNextBodyUpdate = 0.f;
	inline bool bBreakLowerBody = false;

	inline float desyncValue = 0.f;
	void AntiAim(CUserCmd*, bool&);
	void FreeStanding(CUserCmd*, Vector&);
	int ClosestToLocal();
	void LBYBreaker();
}