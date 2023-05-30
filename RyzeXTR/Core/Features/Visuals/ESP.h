#pragma once
#include "../../Interface/interfaces.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"
#include <deque>
#include "../Rage/autowall.h"

namespace visual {

	inline int iHealth[65];
	inline bool bAmmoEnabled[65];
	inline Vector safePoint;
	inline static float vecDormantColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	inline std::array<Vector, 65> vecDormatPosition{};

	void VisualRender();
	void CoolHackKeyBindList();

	void BoxEsp(int&, float&, int&, float&, Color);
	void HealthEsp(int&, float&, int&, float&, float&, float&, int&, Color, Color, size_t&);
	void NameEsp(int&, float&, int&, float&, float&, float&, CBaseEntity*, Color);
	void KevlarEsp(int&, float&, int&, float&, CBaseEntity*, Color);
	void AmmoEsp(int&, float&, int&, float&, CBaseEntity*, Color);
	void BreakLCESP(int&, float&, int&, float&, CBaseEntity* );
	void WeaponEsp(int&, float&, int&, float&, CBaseEntity*, Color);
	void MoneyEsp(int&, float&, int&, float&, CBaseEntity*, Color);
	void Glow(CBaseEntity* pLocal);
	void SkeletonEsp(CBaseEntity*, Color);
	
	void WorldEsp();
	void DroppedWeapons(CBaseCombatWeapon*, Vector&);

	void Flags(float&, int&, CBaseEntity*, size_t&, bool*, float[5][4], bool bDormant = false);

	void WorldCrosshair();
	inline Vector vecWorldCrosshair[5];
	inline float flWorldCrosshairLength[5];

	inline constexpr static std::format_string<int&> healthPrefix("Health: [{}]");
	inline constexpr static std::format_string<int&> kevlarPrefix("Kevlar [{}]");
	inline constexpr static std::format_string<int&, int&> ammoPrefix("[{}/{}]");
	inline constexpr static std::format_string<int&> moneyPrefix("${}");

	inline Vector traceEnds[3] = {Vector(0, 0, 0)};
}