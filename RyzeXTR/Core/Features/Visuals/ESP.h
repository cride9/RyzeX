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

	void VisualRender();

	void BoxEsp(int, int, int, int, Color);
	void HealthEsp(int, int, int, int, int, int, int, Color, Color, int);
	void NameEsp(int, int, int, int, int, int, CBaseEntity*, Color);
	void KevlarEsp(int, int, int, int, CBaseEntity*, Color);
	void AmmoEsp(int, int, int, int, CBaseEntity*, Color);
	void BreakLCESP(int, int, int, int, CBaseEntity* );
	void WeaponEsp(int, int, int, int, CBaseEntity*, Color);
	void MoneyEsp(int, int, int, int, CBaseEntity*, Color);
	void Glow(CBaseEntity* pLocal);

	void Flags(int, int, CBaseEntity*, int, bool*, float[5][4]);

	void WorldCrosshair();
	inline Vector vecWorldCrosshair[5];
	inline float flWorldCrosshairLength[5];

	struct flagsInfo {

		void StoreData(CBaseEntity* pEnt) {

			PlayerInfo_t info = { };

			if (i::EngineClient->GetPlayerInfo(pEnt->EntIndex(), &info)) 
				szName = info.szName;
			
			if (CBaseCombatWeapon* pWeapon = pEnt->GetWeapon(); pWeapon	) {

				iAmmo = pWeapon->GetAmmo();
				iAmmoReserve = pWeapon->GetAmmoReserve();
				szWeaponName = pWeapon->GetCSWpnData()->szWeaponName;
			}
			iHealth = pEnt->GetHealth();
			iArmor = pEnt->GetArmor();
			iMoney = pEnt->GetMoney();

			g::bNewTick[pEnt->EntIndex()] = false;
		}

		const char* szName;
		int iHealth;
		int iArmor;
		int iAmmo;
		int iAmmoReserve;
		int iMoney;
		const char* szWeaponName;
	};

	inline std::array<flagsInfo, 65> flagsData = {};
}