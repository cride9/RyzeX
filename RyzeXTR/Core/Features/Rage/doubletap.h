#pragma once
#include "../../SDK/Menu/config.h"
#include "../Misc/enginepred.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"

namespace doubletap {

	inline int cmdTickbase;
	inline float cmdCurtime;
	inline int cmdCommandNumber;

	inline int rechargeTickbase;
	inline float rechargeCurtime;
	inline int rechargeCommandNumber;

	inline int defensiveTickbase;
	inline float defensiveCurtime;
	inline int defensiveCommandNumber;

	inline int shiftAmount;
	inline int rechargeAmount;

	inline bool bReset = false;
	inline bool bCharged = true;

	void Doubletap();
}