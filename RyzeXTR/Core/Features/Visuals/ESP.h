#pragma once
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"
#include <deque>

namespace visual {

	void VisualRender();

	void BoxEsp(int, int, int, int, Color);
	void HealthEsp(int&, int, int, int, int, int, int, int);
	void NameEsp(int&, int, int, int, int, int, int, CBaseEntity*, Color);
	void KevlarEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void AmmoEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void BreakLCESP( int&, int, int, int, int, CBaseEntity* );
	void WeaponEsp(int&, int, int, int, int, CBaseEntity*, Color);
	void MoneyEsp(int&, int, int, int, int, CBaseEntity*, Color);

	void AutoPeekCircle();

	void DrawMultipoints(CBaseEntity*);

	inline std::deque<Vector> multiPoints[65][18];
}