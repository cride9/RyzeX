#pragma once
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"
#include <deque>
#include "../Rage/autowall.h"

namespace visual {

	void VisualRender();

	void BoxEsp(int, int, int, int, Color);
	void HealthEsp(int&, int, int, int, int, int, int, int, Color);
	void NameEsp(int&, int, int, int, int, int, int, CBaseEntity*, Color);
	void KevlarEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void AmmoEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void BreakLCESP( int&, int, int, int, int, CBaseEntity* );
	void WeaponEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void MoneyEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void Glow(CBaseEntity* pLocal);
	void BruteForceState(int&, int, int, int, int, CBaseEntity*);

	void AutoPeekCircle();

	void DrawMultipoints(CBaseEntity*);

	inline Vector selectedPoint[65][18];
}