#include "config.h"
#include <Windows.h>
#include <Psapi.h>
#include <lmcons.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <shlobj.h>
#include <time.h>
#include <random>
#include <sstream>
#include <fstream>
#include <shlwapi.h>
#include <iomanip>
#include <ctime>
#include <thread>
#include "gui.h"

void CConfig::Setup() {

	ints.clear();
	bools.clear();
	floats.clear();
	strings.clear();

	CreateMainDirectory();
	RefreshSounds();
	RefreshConfigs();

	// ragebot
	{
		using namespace cfg::rage;

		SetupValue(bEnable, false, "rage", "Rageenable");
		SetupValue(iOverrideBind, 0, "rage", "overrideBind");
		SetupValue(bForceBaim, false, "rage", "forceBaim");
		SetupValue(iForceBaimKey, 0, "rage", "forceBaimKey");
		SetupValue(iAimbotFov, 180, "rage", "aimbotfov");
		SetupValue(bSilentAim, false, "rage", "bSiletAim");
		SetupValue(iAimbotKey, 0, "rage", "ragebotbind");
		SetupValue(bOverride, false, "rage", "bOverride");

		SetupValue( bAutostop[ 0 ], false, "rage", "autostop1" );
		SetupValue( bAutostop[ 1 ], false, "rage", "autostop2" );
		SetupValue( bAutostop[ 2 ], false, "rage", "autostop3" );
		SetupValue( bAutostop[ 3 ], false, "rage", "autostop4" );
		SetupValue( bAutostop[ 4 ], false, "rage", "autostop5" );
		SetupValue( bAutostop[ 5 ], false, "rage", "autostop6" );

		for (size_t i = 0; i < 6; i++)
			SetupValue(bConditions[i], false, 2, "rage", std::format("condition{}", i));

		SetupValue( bAutostopAggressiveness[ 0 ], false, "rage", "autostopAggressiveness1" );
		SetupValue( bAutostopAggressiveness[ 1 ], false, "rage", "autostopAggressiveness2" );
		SetupValue( bAutostopAggressiveness[ 2 ], false, "rage", "autostopAggressiveness3" );
		SetupValue( bAutostopAggressiveness[ 3 ], false, "rage", "autostopAggressiveness4" );
		SetupValue( bAutostopAggressiveness[ 4 ], false, "rage", "autostopAggressiveness5" );
		SetupValue( bAutostopAggressiveness[ 5 ], false, "rage", "autostopAggressiveness6" );

		SetupValue( bAutoScope[ 0 ], false, "rage", "autoscope1" );
		SetupValue( bAutoScope[ 1 ], false, "rage", "autoscope2" );
		SetupValue( bAutoScope[ 2 ], false, "rage", "autoscope3" );

		SetupValue(bSafePoint[0], false, "rage", "forceSafePoint1");
		SetupValue(bSafePoint[1], false, "rage", "forceSafePoint2");
		SetupValue(bSafePoint[2], false, "rage", "forceSafePoint3");
		SetupValue(bSafePoint[3], false, "rage", "forceSafePoint4");
		SetupValue(bSafePoint[4], false, "rage", "forceSafePoint5");
		SetupValue(bSafePoint[5], false, "rage", "forceSafePoint6");

		SetupValue(bDoubletap, false, "rage", "Ragedoubletap");
		SetupValue(iDoubletapKey, 0, "rage", "Ragedoubletapkey");

		SetupValue(bHideshot, false, "rage", "bHideshot");
		SetupValue(iHideShotKey, 0, "rage", "iHideshot");

		SetupValue(bResolver, false, "rage", "Rageresolver");

		for (size_t i = 0; i < 6; i++) {
			SetupValue(bHitboxes[1][i], false, 6, "rage", std::format("multihitbox{}", i));
			SetupValue(bHitboxes[0][i], false, 6, "rage", std::format("hitbox{}", i));
			SetupValue(bHitboxes[2][i], false, 6, "rage", std::format("safehitbox{}", i));
			SetupValue(iHitchances[i], 0, "rage", std::format("hitchance{}", i));
			SetupValue(iMinDamages[i], 0, "rage", std::format("mindmg{}", i));
			SetupValue(iHeadPoints[i], 0, "rage", std::format("headpoints{}", i));
			SetupValue(iBodyPoints[i], 0, "rage", std::format("bodypoints{}", i));
			SetupValue(iOverride[i], 0, "rage", std::format("override{}", i));
		}
	}
	
	// antiaim
	{
		using namespace cfg::antiaim;

		SetupValue(bEnabled, false, 3, "antiaim", "Antiaimenabled");
		SetupValue(vecJitterWays[0].data(), 64, 0, "antiaim", "wayStand");
		SetupValue(vecJitterWays[1].data(), 64, 0, "antiaim", "wayMove");
		SetupValue(vecJitterWays[2].data(), 64, 0, "antiaim", "wayJump");
		SetupValue(iEnabledJitters.data(), 3, 0, "antiaim", "wayNumberSaves");

		SetupValue(iPitch, 3, 0, "antiaim", "pitch");
		SetupValue(iYaw, 3, 0, "antiaim", "yaw");
		SetupValue(iYawBase, 3, 0, "antiaim", "yawbase");
		SetupValue(bSlideWalk, 0, "antiaim", "slidewalk");
		SetupValue(bInvertOnShoot, false, 3, "antiaim", "bInvertOnShoot");
		SetupValue(bAntiJitter, false, 3, "antiaim", "bAntiJitter");

		SetupValue(iModifier,3, 0, "antiaim", "modifier");

		SetupValue(iJitterValue, 3, 0, "antiaim", "jittervalue");
		SetupValue(iSpinSpeed, 3, 0, "antiaim", "iSpinSpeed");
		SetupValue(iInvertAngle, 3, 0.f, "antiaim", "invertangle");

		SetupValue(iDesyncType, 3, 0, "antiaim", "desynctype");
		SetupValue(iInverterBind, 0, "antiaim", "desyncinverter");
		SetupValue(bInverter, 0, "antiaim", "bInverter");
		SetupValue(iFlickOffset, 3, 0, "antiaim", "flickOffset");
		SetupValue(flickAngleSwitch, 3, 0, "antiaim", "flickAngleSwitch");

		SetupValue(bSwayDesync, false, 3, "antiaim", "swaylby" );
		
		SetupValue(bFakelag, false, "antiaim", "fakelagswitch");
		SetupValue(iFakelag, 0, "antiaim", "fakelag");
		SetupValue(iFakelagMin, 0, "antiaim", "fakelagmin");
		SetupValue(iFakeLagMax, 0, "antiaim", "fakelagmax");
		SetupValue(iFakeLagType, 0, "antiaim", "fakelagType");

		SetupValue(bDefensive, false, "antiaim", "defensive");

		SetupValue(iFakeWalkSpeed, 0, "antiaim", "fakewalk");
		SetupValue(bFakeDuck, false, "antiaim", "fakeduck");
		SetupValue(iFakeDuckKey, 0, "antiaim", "fakeduckbind");
		SetupValue(bAutoPeek, false, "antiaim", "idealTick");
		SetupValue(iAutoPeek, 0, "antiaim", "idealTickBind");

		SetupValue(bFakeWalk, false, "antiaim", "fakewalk");
		SetupValue(iFakeWalkSpeed, 0, "antiaim", "fakewalkspeed");
		SetupValue(iFakeWalkKey, 0, "antiaim", "fakewalkbind");

		SetupValue(iFreestand, 3, 0, "antiaim", "freestand");
	}

	// visual
	{
		using namespace cfg::visual;
		SetupValue(bOverrideFog, false, "visual", "bFog");
		SetupValue(iFogStart, iFogStart, "visual", "iFogStart");
		SetupValue(iFogEnd, iFogEnd, "visual", "iFogEnd");
		SetupValue(flFogColor, 4, 1.f, "visual", "flFogColor");

		// enemy
		SetupValue(bEnable[ENEMY], false, "visual", "enemyEsp");

		SetupValue(bName[ENEMY], false, "visual", "enemyName");
		SetupValue(flNameColor[ENEMY][0], 1.f, "visual", "enemyNameColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(flNameColor[ENEMY][1], 1.f, "visual", "enemyNameColorG");
		SetupValue(flNameColor[ENEMY][2], 1.f, "visual", "enemyNameColorB");
		SetupValue(flNameColor[ENEMY][3], 1.f, "visual", "enemyNameColorA");

		SetupValue(bBox[ENEMY], false, "visual", "enemyBox");
		SetupValue(flBoxColor[ENEMY][0], 1.f, "visual", "enemyBoxColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(flBoxColor[ENEMY][1], 1.f, "visual", "enemyBoxColorG");
		SetupValue(flBoxColor[ENEMY][2], 1.f, "visual", "enemyBoxColorB");
		SetupValue(flBoxColor[ENEMY][3], 1.f, "visual", "enemyBoxColorA");

		SetupValue(bHealth[ENEMY], false, "visual", "enemyHealth");
		SetupValue(flHealthColorStart[ENEMY][0], 0.044f, "visual", "enemyHealthColorR"); // (0.044f, 1.000f, 0.000f, 1.000f)
		SetupValue(flHealthColorStart[ENEMY][1], 1.000f, "visual", "enemyHealthColorG");
		SetupValue(flHealthColorStart[ENEMY][2], 0.000f, "visual", "enemyHealthColorB");
		SetupValue(flHealthColorStart[ENEMY][3], 1.f, "visual", "enemyHealthColorA");

		SetupValue(flHealthColorEnd[ENEMY][0], 0.044f, "visual", "enemyHealthColorEndR"); // (0.044f, 1.000f, 0.000f, 1.000f)
		SetupValue(flHealthColorEnd[ENEMY][1], 1.000f, "visual", "enemyHealthColorEndG");
		SetupValue(flHealthColorEnd[ENEMY][2], 0.000f, "visual", "enemyHealthColorEndB");
		SetupValue(flHealthColorEnd[ENEMY][3], 1.f, "visual", "enemyHealthColorEndA");

		SetupValue(bGlow[ENEMY], false, "visual", "enemyGlow");
		SetupValue(flGlowColor[ENEMY][0], 0.303f, "visual", "enemyGlowColorR"); // (0.303f, 0.302f, 0.140f, 1.000f)
		SetupValue(flGlowColor[ENEMY][1], 0.302f, "visual", "enemyGlowColorG");
		SetupValue(flGlowColor[ENEMY][2], 0.140f, "visual", "enemyGlowColorB");
		SetupValue(flGlowColor[ENEMY][3], 1.f, "visual", "enemyGlowColorA");

		SetupValue(bArmor[ENEMY], false, "visual", "enemyArmor");
		SetupValue(flArmorColor[ENEMY][0], 0.000f, "visual", "enemyArmorColorR"); // (0.000f, 0.924f, 1.000f, 1.000f)
		SetupValue(flArmorColor[ENEMY][1], 0.924f, "visual", "enemyArmorColorG");
		SetupValue(flArmorColor[ENEMY][2], 1.000f, "visual", "enemyArmorColorB");
		SetupValue(flArmorColor[ENEMY][3], 1.f, "visual", "enemyArmorColorA");

		SetupValue(bAmmo[ENEMY], false, "visual", "enemyAmmo");
		SetupValue(flAmmoColor[ENEMY][0], 0.345f, "visual", "enemyAmmoColorR"); // (0.345f, 0.620f, 1.000f, 1.000f)
		SetupValue(flAmmoColor[ENEMY][1], 0.620f, "visual", "enemyAmmoColorG");
		SetupValue(flAmmoColor[ENEMY][2], 1.f, "visual", "enemyAmmoColorB");
		SetupValue(flAmmoColor[ENEMY][3], 1.f, "visual", "enemyAmmoColorA");

		SetupValue(bWeapon[ENEMY], false, "visual", "enemyWeapon");
		SetupValue(flWeaponColor[ENEMY][0], 1.f, "visual", "enemyWeaponColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(flWeaponColor[ENEMY][1], 1.f, "visual", "enemyWeaponColorG");
		SetupValue(flWeaponColor[ENEMY][2], 1.f, "visual", "enemyWeaponColorB");
		SetupValue(flWeaponColor[ENEMY][3], 1.f, "visual", "enemyWeaponColorA");

		SetupValue(bFlags[ENEMY][0], false, "visual", "ENEMYbFlags0");
		SetupValue(bFlags[ENEMY][1], false, "visual", "ENEMYbFlags1");
		SetupValue(bFlags[ENEMY][2], false, "visual", "ENEMYbFlags2");
		SetupValue(bFlags[ENEMY][3], false, "visual", "ENEMYbFlags3");
		SetupValue(bFlags[ENEMY][4], false, "visual", "ENEMYbFlags4");
		SetupValue(bFlags[ENEMY][5], false, "visual", "ENEMYbFlags5");
		SetupValue(bFlags[ENEMY][6], false, "visual", "ENEMYbFlags6");
		SetupValue(bFlags[ENEMY][7], false, "visual", "ENEMYbFlags7");
		SetupValue(bFlags[ENEMY][8], false, "visual", "ENEMYbFlags8");

		SetupValue(flFlagsColor[ENEMY][0][0], 1.f, "visual", "ENEMYflFlags0R");
		SetupValue(flFlagsColor[ENEMY][0][1], 1.f, "visual", "ENEMYflFlags0G");
		SetupValue(flFlagsColor[ENEMY][0][2], 1.f, "visual", "ENEMYflFlags0B");
		SetupValue(flFlagsColor[ENEMY][0][3], 1.f, "visual", "ENEMYflFlags0A");

		SetupValue(flFlagsColor[ENEMY][1][0], 1.f, "visual", "ENEMYflFlags1R");
		SetupValue(flFlagsColor[ENEMY][1][1], 1.f, "visual", "ENEMYflFlags1G");
		SetupValue(flFlagsColor[ENEMY][1][2], 1.f, "visual", "ENEMYflFlags1B");
		SetupValue(flFlagsColor[ENEMY][1][3], 1.f, "visual", "ENEMYflFlags1A");

		SetupValue(flFlagsColor[ENEMY][2][0], 1.f, "visual", "ENEMYflFlags2R");
		SetupValue(flFlagsColor[ENEMY][2][1], 1.f, "visual", "ENEMYflFlags2G");
		SetupValue(flFlagsColor[ENEMY][2][2], 1.f, "visual", "ENEMYflFlags2B");
		SetupValue(flFlagsColor[ENEMY][2][3], 1.f, "visual", "ENEMYflFlags2A");

		SetupValue(flFlagsColor[ENEMY][3][0], 1.f, "visual", "ENEMYflFlags3R");
		SetupValue(flFlagsColor[ENEMY][3][1], 1.f, "visual", "ENEMYflFlags3G");
		SetupValue(flFlagsColor[ENEMY][3][2], 1.f, "visual", "ENEMYflFlags3B");
		SetupValue(flFlagsColor[ENEMY][3][3], 1.f, "visual", "ENEMYflFlags3A");

		SetupValue(flFlagsColor[ENEMY][4][0], 1.f, "visual", "ENEMYflFlags4R");
		SetupValue(flFlagsColor[ENEMY][4][1], 1.f, "visual", "ENEMYflFlags4G");
		SetupValue(flFlagsColor[ENEMY][4][2], 1.f, "visual", "ENEMYflFlags4B");
		SetupValue(flFlagsColor[ENEMY][4][3], 1.f, "visual", "ENEMYflFlags4A");

		SetupValue(flFlagsColor[ENEMY][5][0], 1.f, "visual", "ENEMYflFlags5R");
		SetupValue(flFlagsColor[ENEMY][5][1], 1.f, "visual", "ENEMYflFlags5G");
		SetupValue(flFlagsColor[ENEMY][5][2], 1.f, "visual", "ENEMYflFlags5B");
		SetupValue(flFlagsColor[ENEMY][5][3], 1.f, "visual", "ENEMYflFlags5A");

		SetupValue(flFlagsColor[ENEMY][6][0], 1.f, "visual", "ENEMYflFlags6R");
		SetupValue(flFlagsColor[ENEMY][6][1], 1.f, "visual", "ENEMYflFlags6G");
		SetupValue(flFlagsColor[ENEMY][6][2], 1.f, "visual", "ENEMYflFlags6B");
		SetupValue(flFlagsColor[ENEMY][6][3], 1.f, "visual", "ENEMYflFlags6A");

		SetupValue(flFlagsColor[ENEMY][7][0], 1.f, "visual", "ENEMYflFlags7R");
		SetupValue(flFlagsColor[ENEMY][7][1], 1.f, "visual", "ENEMYflFlags7G");
		SetupValue(flFlagsColor[ENEMY][7][2], 1.f, "visual", "ENEMYflFlags7B");
		SetupValue(flFlagsColor[ENEMY][7][3], 1.f, "visual", "ENEMYflFlags7A");

		SetupValue(flFlagsColor[ENEMY][8][0], 1.f, "visual", "ENEMYflFlags8R");
		SetupValue(flFlagsColor[ENEMY][8][1], 1.f, "visual", "ENEMYflFlags8G");
		SetupValue(flFlagsColor[ENEMY][8][2], 1.f, "visual", "ENEMYflFlags8B");
		SetupValue(flFlagsColor[ENEMY][8][3], 1.f, "visual", "ENEMYflFlags8A");

		SetupValue(bSkeleton[ENEMY], false, "visual", "bSkeletonENEMY");
		SetupValue(flSkeletonColor[ENEMY][0], 1.f, "visual", "flSkeletonColorENEMYR");
		SetupValue(flSkeletonColor[ENEMY][1], 1.f, "visual", "flSkeletonColorENEMYG");
		SetupValue(flSkeletonColor[ENEMY][2], 1.f, "visual", "flSkeletonColorENEMYB");
		SetupValue(flSkeletonColor[ENEMY][3], 1.f, "visual", "flSkeletonColorENEMYA");

		SetupValue(bBulletTracer[ENEMY], false, "visual", "bBulletTracerENEMY");
		SetupValue(flBulletTracerColor[ENEMY][0], 1.f, "visual", "flBulletTracerColorENEMYR");
		SetupValue(flBulletTracerColor[ENEMY][1], 1.f, "visual", "flBulletTracerColorENEMYG");
		SetupValue(flBulletTracerColor[ENEMY][2], 1.f, "visual", "flBulletTracerColorENEMYB");
		SetupValue(flBulletTracerColor[ENEMY][3], 1.f, "visual", "flBulletTracerColorENEMYA");

		// teammate
		SetupValue(bEnable[TEAM], false, "visual", "teamEsp");

		SetupValue(bName[TEAM], false, "visual", "teamName");
		SetupValue(flNameColor[TEAM][0], 1.f, "visual", "teamNameColorR");
		SetupValue(flNameColor[TEAM][1], 1.f, "visual", "teamNameColorG");
		SetupValue(flNameColor[TEAM][2], 1.f, "visual", "teamNameColorB");
		SetupValue(flNameColor[TEAM][3], 1.f, "visual", "teamNameColorA");

		SetupValue(bBox[TEAM], false, "visual", "teamBox");
		SetupValue(flBoxColor[TEAM][0], 1.f, "visual", "teamBoxColorR");
		SetupValue(flBoxColor[TEAM][1], 1.f, "visual", "teamBoxColorG");
		SetupValue(flBoxColor[TEAM][2], 1.f, "visual", "teamBoxColorB");
		SetupValue(flBoxColor[TEAM][3], 1.f, "visual", "teamBoxColorA");

		SetupValue(bHealth[TEAM], false, "visual", "teamHealth");
		SetupValue(flHealthColorStart[TEAM][0], 0.211f, "visual", "teamHealthColorR"); // (0.211f, 1.000f, 0.000f, 1.000f)
		SetupValue(flHealthColorStart[TEAM][1], 1.f, "visual", "teamHealthColorG");
		SetupValue(flHealthColorStart[TEAM][2], 0.000f, "visual", "teamHealthColorB");
		SetupValue(flHealthColorStart[TEAM][3], 1.f, "visual", "teamHealthColorA");

		SetupValue(flHealthColorEnd[TEAM][0], 0.211f, "visual", "teamHealthColorEndR"); // (0.211f, 1.000f, 0.000f, 1.000f)
		SetupValue(flHealthColorEnd[TEAM][1], 1.f, "visual", "teamHealthColorEndG");
		SetupValue(flHealthColorEnd[TEAM][2], 0.000f, "visual", "teamHealthColorEndB");
		SetupValue(flHealthColorEnd[TEAM][3], 1.f, "visual", "teamHealthColorEndA");

		SetupValue(bGlow[TEAM], false, "visual", "teamGlow");
		SetupValue(flGlowColor[TEAM][0], 0.125f, "visual", "teamGlowColorR"); // (0.125f, 0.272f, 0.000f, 1.000f)
		SetupValue(flGlowColor[TEAM][1], 0.272f, "visual", "teamGlowColorG");
		SetupValue(flGlowColor[TEAM][2], 0.000f, "visual", "teamGlowColorB");
		SetupValue(flGlowColor[TEAM][3], 1.f, "visual", "teamGlowColorA");

		SetupValue(bArmor[TEAM], false, "visual", "teamArmor");
		SetupValue(flArmorColor[TEAM][0], 0.000f, "visual", "teamArmorColorR"); // (0.000f, 1.000f, 0.954f, 1.000f)
		SetupValue(flArmorColor[TEAM][1], 1.f, "visual", "teamArmorColorG");
		SetupValue(flArmorColor[TEAM][2], 0.954f, "visual", "teamArmorColorB");
		SetupValue(flArmorColor[TEAM][3], 1.f, "visual", "teamArmorColorA");

		SetupValue(bAmmo[TEAM], false, "visual", "teamAmmo");
		SetupValue(flAmmoColor[TEAM][0], 0.000f, "visual", "teamAmmoColorR"); // (0.000f, 0.546f, 1.000f, 1.000f)
		SetupValue(flAmmoColor[TEAM][1], 0.546f, "visual", "teamAmmoColorG");
		SetupValue(flAmmoColor[TEAM][2], 1.f, "visual", "teamAmmoColorB");
		SetupValue(flAmmoColor[TEAM][3], 1.f, "visual", "teamAmmoColorA");

		SetupValue(bWeapon[TEAM], false, "visual", "teamWeapon");
		SetupValue(flWeaponColor[TEAM][0], 1.f, "visual", "teamWeaponColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(flWeaponColor[TEAM][1], 1.f, "visual", "teamWeaponColorG");
		SetupValue(flWeaponColor[TEAM][2], 1.f, "visual", "teamWeaponColorB");
		SetupValue(flWeaponColor[TEAM][3], 1.f, "visual", "teamWeaponColorA");

		SetupValue(bFlags[TEAM][0], false, "visual", "TEAMbFlags0");
		SetupValue(bFlags[TEAM][1], false, "visual", "TEAMbFlags1");
		SetupValue(bFlags[TEAM][2], false, "visual", "TEAMbFlags2");
		SetupValue(bFlags[TEAM][3], false, "visual", "TEAMbFlags3");
		SetupValue(bFlags[TEAM][4], false, "visual", "TEAMbFlags4");
		SetupValue(bFlags[TEAM][5], false, "visual", "TEAMbFlags5");
		SetupValue(bFlags[TEAM][6], false, "visual", "TEAMbFlags6");
		SetupValue(bFlags[TEAM][7], false, "visual", "TEAMbFlags7");
		SetupValue(bFlags[TEAM][8], false, "visual", "TEAMbFlags8");

		SetupValue(flFlagsColor[TEAM][0][0], 1.f, "visual", "TEAMflFlags0R");
		SetupValue(flFlagsColor[TEAM][0][1], 1.f, "visual", "TEAMflFlags0G");
		SetupValue(flFlagsColor[TEAM][0][2], 1.f, "visual", "TEAMflFlags0B");
		SetupValue(flFlagsColor[TEAM][0][3], 1.f, "visual", "TEAMflFlags0A");

		SetupValue(flFlagsColor[TEAM][1][0], 1.f, "visual", "TEAMflFlags1R");
		SetupValue(flFlagsColor[TEAM][1][1], 1.f, "visual", "TEAMflFlags1G");
		SetupValue(flFlagsColor[TEAM][1][2], 1.f, "visual", "TEAMflFlags1B");
		SetupValue(flFlagsColor[TEAM][1][3], 1.f, "visual", "TEAMflFlags1A");

		SetupValue(flFlagsColor[TEAM][2][0], 1.f, "visual", "TEAMflFlags2R");
		SetupValue(flFlagsColor[TEAM][2][1], 1.f, "visual", "TEAMflFlags2G");
		SetupValue(flFlagsColor[TEAM][2][2], 1.f, "visual", "TEAMflFlags2B");
		SetupValue(flFlagsColor[TEAM][2][3], 1.f, "visual", "TEAMflFlags2A");

		SetupValue(flFlagsColor[TEAM][3][0], 1.f, "visual", "TEAMflFlags3R");
		SetupValue(flFlagsColor[TEAM][3][1], 1.f, "visual", "TEAMflFlags3G");
		SetupValue(flFlagsColor[TEAM][3][2], 1.f, "visual", "TEAMflFlags3B");
		SetupValue(flFlagsColor[TEAM][3][3], 1.f, "visual", "TEAMflFlags3A");

		SetupValue(flFlagsColor[TEAM][4][0], 1.f, "visual", "TEAMflFlags4R");
		SetupValue(flFlagsColor[TEAM][4][1], 1.f, "visual", "TEAMflFlags4G");
		SetupValue(flFlagsColor[TEAM][4][2], 1.f, "visual", "TEAMflFlags4B");
		SetupValue(flFlagsColor[TEAM][4][3], 1.f, "visual", "TEAMflFlags4A");

		SetupValue(flFlagsColor[TEAM][5][0], 1.f, "visual", "TEAMflFlags5R");
		SetupValue(flFlagsColor[TEAM][5][1], 1.f, "visual", "TEAMflFlags5G");
		SetupValue(flFlagsColor[TEAM][5][2], 1.f, "visual", "TEAMflFlags5B");
		SetupValue(flFlagsColor[TEAM][5][3], 1.f, "visual", "TEAMflFlags5A");

		SetupValue(flFlagsColor[TEAM][6][0], 1.f, "visual", "TEAMflFlags6R");
		SetupValue(flFlagsColor[TEAM][6][1], 1.f, "visual", "TEAMflFlags6G");
		SetupValue(flFlagsColor[TEAM][6][2], 1.f, "visual", "TEAMflFlags6B");
		SetupValue(flFlagsColor[TEAM][6][3], 1.f, "visual", "TEAMflFlags6A");

		SetupValue(flFlagsColor[TEAM][7][0], 1.f, "visual", "TEAMflFlags7R");
		SetupValue(flFlagsColor[TEAM][7][1], 1.f, "visual", "TEAMflFlags7G");
		SetupValue(flFlagsColor[TEAM][7][2], 1.f, "visual", "TEAMflFlags7B");
		SetupValue(flFlagsColor[TEAM][7][3], 1.f, "visual", "TEAMflFlags7A");

		SetupValue(flFlagsColor[TEAM][8][0], 1.f, "visual", "TEAMflFlags8R");
		SetupValue(flFlagsColor[TEAM][8][1], 1.f, "visual", "TEAMflFlags8G");
		SetupValue(flFlagsColor[TEAM][8][2], 1.f, "visual", "TEAMflFlags8B");
		SetupValue(flFlagsColor[TEAM][8][3], 1.f, "visual", "TEAMflFlags8A");

		SetupValue(bSkeleton[TEAM], false, "visual", "bSkeletonTEAM");
		SetupValue(flSkeletonColor[TEAM][0], 1.f, "visual", "flSkeletonColorTEAMR");
		SetupValue(flSkeletonColor[TEAM][1], 1.f, "visual", "flSkeletonColorTEAMG");
		SetupValue(flSkeletonColor[TEAM][2], 1.f, "visual", "flSkeletonColorTEAMB");
		SetupValue(flSkeletonColor[TEAM][3], 1.f, "visual", "flSkeletonColorTEAMA");

		SetupValue(bBulletTracer[TEAM], false, "visual", "bBulletTracerTEAM");
		SetupValue(flBulletTracerColor[TEAM][0], 1.f, "visual", "flBulletTracerColorTEAMR");
		SetupValue(flBulletTracerColor[TEAM][1], 1.f, "visual", "flBulletTracerColorTEAMG");
		SetupValue(flBulletTracerColor[TEAM][2], 1.f, "visual", "flBulletTracerColorTEAMB");
		SetupValue(flBulletTracerColor[TEAM][3], 1.f, "visual", "flBulletTracerColorTEAMA");

		// local
		SetupValue(bEnable[LOCAL], false, "visual", "localEsp");

		SetupValue(bName[LOCAL], false, "visual", "localName");
		SetupValue(flNameColor[LOCAL][0], 1.f, "visual", "localNameColorR");
		SetupValue(flNameColor[LOCAL][1], 1.f, "visual", "localNameColorG");
		SetupValue(flNameColor[LOCAL][2], 1.f, "visual", "localNameColorB");
		SetupValue(flNameColor[LOCAL][3], 1.f, "visual", "localNameColorA");

		SetupValue(bBox[LOCAL], false, "visual", "localBox");
		SetupValue(flBoxColor[LOCAL][0], 1.f, "visual", "localBoxColorR");
		SetupValue(flBoxColor[LOCAL][1], 1.f, "visual", "localBoxColorG");
		SetupValue(flBoxColor[LOCAL][2], 1.f, "visual", "localBoxColorB");
		SetupValue(flBoxColor[LOCAL][3], 1.f, "visual", "localBoxColorA");

		SetupValue(bHealth[LOCAL], false, "visual", "localHealth");
		SetupValue(flHealthColorStart[LOCAL][0], 1.f, "visual", "localHealthColorR");
		SetupValue(flHealthColorStart[LOCAL][1], 1.f, "visual", "localHealthColorG");
		SetupValue(flHealthColorStart[LOCAL][2], 1.f, "visual", "localHealthColorB");
		SetupValue(flHealthColorStart[LOCAL][3], 1.f, "visual", "localHealthColorA");

		SetupValue(flHealthColorEnd[LOCAL][0], 1.f, "visual", "localHealthColorEndR");
		SetupValue(flHealthColorEnd[LOCAL][1], 1.f, "visual", "localHealthColorEndG");
		SetupValue(flHealthColorEnd[LOCAL][2], 1.f, "visual", "localHealthColorEndB");
		SetupValue(flHealthColorEnd[LOCAL][3], 1.f, "visual", "localHealthColorEndA");

		SetupValue(bGlow[LOCAL], false, "visual", "localGlow");
		SetupValue(flGlowColor[LOCAL][0], 0.137f, "visual", "localGlowColorR"); // (0.137f, 0.091f, 0.230f, 1.000f)
		SetupValue(flGlowColor[LOCAL][1], 0.091f, "visual", "localGlowColorG");
		SetupValue(flGlowColor[LOCAL][2], 0.230f, "visual", "localGlowColorB");
		SetupValue(flGlowColor[LOCAL][3], 1.f, "visual", "localGlowColorA");

		SetupValue(bArmor[LOCAL], false, "visual", "localArmor");
		SetupValue(flArmorColor[LOCAL][0], 1.f, "visual", "localArmorColorR"); // (1.000f, 0.832f, 0.000f, 1.000f)
		SetupValue(flArmorColor[LOCAL][1], 0.832f, "visual", "localArmorColorG");
		SetupValue(flArmorColor[LOCAL][2], 0.000f, "visual", "localArmorColorB");
		SetupValue(flArmorColor[LOCAL][3], 1.f, "visual", "localArmorColorA");

		SetupValue(bAmmo[LOCAL], false, "visual", "localAmmo");
		SetupValue(flAmmoColor[LOCAL][0], 1.f, "visual", "localAmmoColorR"); // (1.000f, 0.000f, 0.000f, 1.000f)
		SetupValue(flAmmoColor[LOCAL][1], 0.000f, "visual", "localAmmoColorG");
		SetupValue(flAmmoColor[LOCAL][2], 0.000f, "visual", "localAmmoColorB");
		SetupValue(flAmmoColor[LOCAL][3], 1.f, "visual", "localAmmoColorA");

		SetupValue(bWeapon[LOCAL], false, "visual", "localWeapon");
		SetupValue(flWeaponColor[LOCAL][0], 1.f, "visual", "localWeaponColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(flWeaponColor[LOCAL][1], 1.f, "visual", "localWeaponColorG");
		SetupValue(flWeaponColor[LOCAL][2], 1.f, "visual", "localWeaponColorB");
		SetupValue(flWeaponColor[LOCAL][3], 1.f, "visual", "localWeaponColorA");

		SetupValue(bFlags[LOCAL][0], false, "visual", "LOCALbFlags0");
		SetupValue(bFlags[LOCAL][1], false, "visual", "LOCALbFlags1");
		SetupValue(bFlags[LOCAL][2], false, "visual", "LOCALbFlags2");
		SetupValue(bFlags[LOCAL][3], false, "visual", "LOCALbFlags3");
		SetupValue(bFlags[LOCAL][4], false, "visual", "LOCALbFlags4");
		SetupValue(bFlags[LOCAL][5], false, "visual", "LOCALbFlags5");
		SetupValue(bFlags[LOCAL][6], false, "visual", "LOCALbFlags6");
		SetupValue(bFlags[LOCAL][7], false, "visual", "LOCALbFlags7");
		SetupValue(bFlags[LOCAL][8], false, "visual", "LOCALbFlags8");

		SetupValue(flFlagsColor[LOCAL][0][0], 1.f, "visual", "LOCALflFlags0R");
		SetupValue(flFlagsColor[LOCAL][0][1], 1.f, "visual", "LOCALflFlags0G");
		SetupValue(flFlagsColor[LOCAL][0][2], 1.f, "visual", "LOCALflFlags0B");
		SetupValue(flFlagsColor[LOCAL][0][3], 1.f, "visual", "LOCALflFlags0A");

		SetupValue(flFlagsColor[LOCAL][1][0], 1.f, "visual", "LOCALflFlags1R");
		SetupValue(flFlagsColor[LOCAL][1][1], 1.f, "visual", "LOCALflFlags1G");
		SetupValue(flFlagsColor[LOCAL][1][2], 1.f, "visual", "LOCALflFlags1B");
		SetupValue(flFlagsColor[LOCAL][1][3], 1.f, "visual", "LOCALflFlags1A");

		SetupValue(flFlagsColor[LOCAL][2][0], 1.f, "visual", "LOCALflFlags2R");
		SetupValue(flFlagsColor[LOCAL][2][1], 1.f, "visual", "LOCALflFlags2G");
		SetupValue(flFlagsColor[LOCAL][2][2], 1.f, "visual", "LOCALflFlags2B");
		SetupValue(flFlagsColor[LOCAL][2][3], 1.f, "visual", "LOCALflFlags2A");

		SetupValue(flFlagsColor[LOCAL][3][0], 1.f, "visual", "LOCALflFlags3R");
		SetupValue(flFlagsColor[LOCAL][3][1], 1.f, "visual", "LOCALflFlags3G");
		SetupValue(flFlagsColor[LOCAL][3][2], 1.f, "visual", "LOCALflFlags3B");
		SetupValue(flFlagsColor[LOCAL][3][3], 1.f, "visual", "LOCALflFlags3A");

		SetupValue(flFlagsColor[LOCAL][4][0], 1.f, "visual", "LOCALflFlags4R");
		SetupValue(flFlagsColor[LOCAL][4][1], 1.f, "visual", "LOCALflFlags4G");
		SetupValue(flFlagsColor[LOCAL][4][2], 1.f, "visual", "LOCALflFlags4B");
		SetupValue(flFlagsColor[LOCAL][4][3], 1.f, "visual", "LOCALflFlags4A");

		SetupValue(flFlagsColor[LOCAL][5][0], 1.f, "visual", "LOCALflFlags5R");
		SetupValue(flFlagsColor[LOCAL][5][1], 1.f, "visual", "LOCALflFlags5G");
		SetupValue(flFlagsColor[LOCAL][5][2], 1.f, "visual", "LOCALflFlags5B");
		SetupValue(flFlagsColor[LOCAL][5][3], 1.f, "visual", "LOCALflFlags5A");

		SetupValue(flFlagsColor[LOCAL][6][0], 1.f, "visual", "LOCALflFlags6R");
		SetupValue(flFlagsColor[LOCAL][6][1], 1.f, "visual", "LOCALflFlags6G");
		SetupValue(flFlagsColor[LOCAL][6][2], 1.f, "visual", "LOCALflFlags6B");
		SetupValue(flFlagsColor[LOCAL][6][3], 1.f, "visual", "LOCALflFlags6A");

		SetupValue(flFlagsColor[LOCAL][7][0], 1.f, "visual", "LOCALflFlags7R");
		SetupValue(flFlagsColor[LOCAL][7][1], 1.f, "visual", "LOCALflFlags7G");
		SetupValue(flFlagsColor[LOCAL][7][2], 1.f, "visual", "LOCALflFlags7B");
		SetupValue(flFlagsColor[LOCAL][7][3], 1.f, "visual", "LOCALflFlags7A");

		SetupValue(flFlagsColor[LOCAL][8][0], 1.f, "visual", "LOCALflFlags8R");
		SetupValue(flFlagsColor[LOCAL][8][1], 1.f, "visual", "LOCALflFlags8G");
		SetupValue(flFlagsColor[LOCAL][8][2], 1.f, "visual", "LOCALflFlags8B");
		SetupValue(flFlagsColor[LOCAL][8][3], 1.f, "visual", "LOCALflFlags8A");

		SetupValue(bSkeleton[LOCAL], false, "visual", "bSkeletonLOCAL");
		SetupValue(flSkeletonColor[LOCAL][0], 1.f, "visual", "flSkeletonColorLOCALR");
		SetupValue(flSkeletonColor[LOCAL][1], 1.f, "visual", "flSkeletonColorLOCALG");
		SetupValue(flSkeletonColor[LOCAL][2], 1.f, "visual", "flSkeletonColorLOCALB");
		SetupValue(flSkeletonColor[LOCAL][3], 1.f, "visual", "flSkeletonColorLOCALA");
	
		SetupValue(bBulletTracer[LOCAL], false, "visual", "bBulletTracerLOCAL");
		SetupValue(flBulletTracerColor[LOCAL][0], 1.f, "visual", "flBulletTracerColorLOCALR");
		SetupValue(flBulletTracerColor[LOCAL][1], 1.f, "visual", "flBulletTracerColorLOCALG");
		SetupValue(flBulletTracerColor[LOCAL][2], 1.f, "visual", "flBulletTracerColorLOCALB");
		SetupValue(flBulletTracerColor[LOCAL][3], 1.f, "visual", "flBulletTracerColorLOCALA");
	}

	// model
	{
		using namespace cfg::model;

		SetupValue(attachmentChamsMaterial, 3, 0, "model", "attachmentChamsMaterial");
		SetupValue(attachmentChams, false, 3, "model", "attachmentChams");
		SetupValue(attachmentChamsXhair, false, 3, "model", "attachmentChamsXhair");
		SetupValue(attachmentOverlay, false, 3, "model", "attachmentOverlay");
		SetupValue(attachmentOverlayXhair, false, 3, "model", "attachmentOverlayXhair");
		SetupValue(attachmentThinOverlay, false, 3, "model", "attachmentThinOverlay");
		SetupValue(attachmentThinOverlayXhair, false, 3, "model", "attachmentThinOverlayXhair");
		SetupValue(attachmentAnimatedOverlay, false, 3, "model", "attachmentAnimatedOverlay");
		SetupValue(attachmentAnimatedOverlayXhair, false, 3, "model", "attachmentAnimatedOverlayXhair");
		
		SetupValue(attachmentChamsColor[ENEMY], 4, 1.f, "model", "attachmentChamsColorENEMY");
		SetupValue(attachmentChamsColor[TEAM], 4, 1.f, "model", "attachmentChamsColorTEAM");
		SetupValue(attachmentChamsColor[LOCAL], 4, 1.f, "model", "attachmentChamsColorLOCAL");

		SetupValue(attachmentOverlayColor[ENEMY], 4, 1.f, "model", "attachmentOverlayColorENEMY");
		SetupValue(attachmentOverlayColor[TEAM], 4, 1.f, "model", "attachmentOverlayColorTEAM");
		SetupValue(attachmentOverlayColor[LOCAL], 4, 1.f, "model", "attachmentOverlayColorLOCAL");

		SetupValue(attachmentThinOverlayColor[ENEMY], 4, 1.f, "model", "attachmentThinOverlayColorENEMY");
		SetupValue(attachmentThinOverlayColor[TEAM], 4, 1.f, "model", "attachmentThinOverlayColorTEAM");
		SetupValue(attachmentThinOverlayColor[LOCAL], 4, 1.f, "model", "attachmentThinOverlayColorLOCAL");

		SetupValue(attachmentAnimatedOverlayColor[ENEMY], 4, 1.f, "model", "attachmentAnimatedOverlayColorENEMY");
		SetupValue(attachmentAnimatedOverlayColor[TEAM], 4, 1.f, "model", "attachmentAnimatedOverlayColorTEAM");
		SetupValue(attachmentAnimatedOverlayColor[LOCAL], 4, 1.f, "model", "attachmentAnimatedOverlayColorLOCAL");

		SetupValue(enemyBTEnable, 0, "model", "enemyBTEnable");
		SetupValue(enemyBTType, 0, "model", "enemyBTType");
		SetupValue(enemyBTXhair, 0, "model", "enemyBTXhair");
		SetupValue(enemyBTColor[0], 1.f, "model", "enemyBTColorR"); // (1.000f, 0.000f, 0.000f, 0.570f)
		SetupValue(enemyBTColor[1], 0.000f, "model", "enemyBTColorG");
		SetupValue(enemyBTColor[2], 0.000f, "model", "enemyBTColorB");
		SetupValue(enemyBTColor[3], 0.570f, "model", "enemyBTColorA");

		SetupValue(iType, 3, 0, "model", "iType");

		SetupValue(bChams, 0, 3, "model", "bChams");
		SetupValue(bChamsXQZ, 0, 3, "model", "bChamsXQZ");
		SetupValue(bXhair, 0, 3, "model", "bXhair");
		SetupValue(bXhairXQZ, 0, 3, "model", "bXhairXQZ");

		SetupValue(bOverlay, 0, 3,  "model", "bOverlay");
		SetupValue(bOverlayXQZ, 0, 3,  "model", "bOverlayXQZ");
		SetupValue(bOverlayXhair, 0, 3,  "model", "bOverlayXhair");
		SetupValue(bOverlayXhairXQZ, 0, 3,  "model", "bOverlayXhairXQZ");

		SetupValue(bThinOverlay, 0, 3,  "model", "bThinOverlay");
		SetupValue(bThinOverlayXQZ, 0, 3,  "model", "bThinOverlayXQZ");
		SetupValue(bThinOverlayXhair, 0, 3,  "model", "bThinOverlayXhair");
		SetupValue(bThinOverlayXhairXQZ, 0, 3,  "model", "bThinOverlayXhairXQZ");

		SetupValue(bAnimOverlay, 0, 3,  "model", "bAnimOverlay");
		SetupValue(bAnimOverlayXQZ, 0, 3,  "model", "bAnimOverlayXQZ");
		SetupValue(bAnimOverlayXhair, 0, 3,  "model", "bAnimOverlayXhair");
		SetupValue(bAnimOverlayXhairXQZ, 0, 3,  "model", "bAnimOverlayXhairXQZ");

		for (size_t i = 0; i < 3; i++)
		{
			SetupValue(ChamsColor[i], 4, 1.f, "model", std::format("ChamsColor{}", i));
			SetupValue(ChamsColorXQZ[i], 4, 1.f, "model", std::format("ChamsColorXQZ{}", i));

			SetupValue(OverlayColor[i], 4, 1.f, "model", std::format("OverlayColor{}", i));
			SetupValue(OverlayColorXQZ[i], 4, 1.f, "model", std::format("OverlayColorXQZ{}", i));

			SetupValue(ThinOverlayColor[i], 4, 1.f, "model", std::format("ThinOverlayColor{}", i));
			SetupValue(ThinOverlayColorXQZ[i], 4, 1.f, "model", std::format("ThinOverlayColorXQZ{}", i));

			SetupValue(AnimOverlayColor[i], 4, 1.f, "model", std::format("AnimOverlayColor{}", i));
			SetupValue(AnimOverlayColorXQZ[i], 4, 1.f, "model", std::format("AnimOverlayColorXQZ{}", i));
		}

		// chams
		SetupValue(bBlend, false, "model", "bBlend");
		SetupValue(flBlend, false, "model", "flBlend");

		// ideal tick
		SetupValue(localIdealTick, false, "model", "localIdealTick");

		SetupValue(localIdealTickColor[0], 0.508f, "model", "localIdealTickColorR");  // (0.508f, 0.982f, 1.000f, 1.000f)
		SetupValue(localIdealTickColor[1], 0.982f, "model", "localIdealTickColorG");
		SetupValue(localIdealTickColor[2], 1.f, "model", "localIdealTickColorB");
		SetupValue(localIdealTickColor[3], 1.f, "model", "localIdealTickColorA");

		SetupValue(localDesync, false, "model", "localDesync");
		SetupValue(localDesyncType, 0, "model", "localDesyncType");
		SetupValue(localDesyncXhair, false, "model", "localDesyncXhair");
		SetupValue(localDesyncSkeleton, false, "model", "localDesyncSkeleton");

		SetupValue(localDesyncColor[0], 0.480f, "model", "localDesyncColorR"); // (0.480f, 0.634f, 0.813f, 0.868f)
		SetupValue(localDesyncColor[1], 0.634f, "model", "localDesyncColorG");
		SetupValue(localDesyncColor[2], 0.813f, "model", "localDesyncColorB");
		SetupValue(localDesyncColor[3], 0.868f, "model", "localDesyncColorA");

		SetupValue(viewmodel, false, "model", "viewmodel");
		SetupValue(viewmodelType, 0, "model", "viewmodelType");
		SetupValue(viewmodelColor[0], 0.000f, "model", "viewmodelColorR"); // (0.000f, 0.422f, 1.000f, 0.258f)
		SetupValue(viewmodelColor[1], 0.422f, "model", "viewmodelColorG");
		SetupValue(viewmodelColor[2], 1.f, "model", "viewmodelColorB");
		SetupValue(viewmodelColor[3], 0.258f, "model", "viewmodelColorA");
		SetupValue(viewmodelXhair, false, "model", "viewmodelXhair");

		SetupValue(viewmodelOverlay, false, "model", "viewmodelOverlay");
		SetupValue(viewmodelOverlayXhair, false, "model", "viewmodelOverlayXhair"); 
		SetupValue(viewmodelOverlayColor[0], 0.064f, "model", "viewmodelOverlayColorR"); // (0.064f, 0.021f, 0.021f, 1.000f)
		SetupValue(viewmodelOverlayColor[1], 0.021f, "model", "viewmodelOverlayColorG");
		SetupValue(viewmodelOverlayColor[2], 0.021f, "model", "viewmodelOverlayColorB");
		SetupValue(viewmodelOverlayColor[3], 1.f, "model", "viewmodelOverlayColorA");

		SetupValue(viewmodelThinOverlay, false, "model", "viewmodelThinOverlay");
		SetupValue(viewmodelThinOverlayXhair, false, "model", "viewmodelThinOverlayXhair");
		SetupValue(viewmodelThinOverlayColor[0], 0.771f, "model", "viewmodelThinOverlayColorR"); // (0.771f, 0.771f, 0.771f, 1.000f)
		SetupValue(viewmodelThinOverlayColor[1], 0.771f, "model", "viewmodelThinOverlayColorG");
		SetupValue(viewmodelThinOverlayColor[2], 0.771f, "model", "viewmodelThinOverlayColorB");
		SetupValue(viewmodelThinOverlayColor[3], 1.f, "model", "viewmodelThinOverlayColorA");

		SetupValue(viewmodelAnimOverlay, false, "model", "viewmodelAnimOverlay");
		SetupValue(viewmodelAnimOverlayXhair, false, "model", "viewmodelAnimOverlayXhair");
		SetupValue(viewmodelAnimOverlayColor[0], 0.501f, "model", "viewmodelAnimOverlayColorR"); // (0.501f, 0.237f, 0.237f, 1.000f)
		SetupValue(viewmodelAnimOverlayColor[1], 0.237f, "model", "viewmodelAnimOverlayColorG");
		SetupValue(viewmodelAnimOverlayColor[2], 0.237f, "model", "viewmodelAnimOverlayColorB");
		SetupValue(viewmodelAnimOverlayColor[3], 1.f, "model", "viewmodelAnimOverlayColorA");

		SetupValue(weapon, false, "model", "weapon");
		SetupValue(weaponType, 0, "model", "weaponType");
		SetupValue(weaponColor[0], 0.411f, "model", "weaponColorR"); // (0.411f, 0.411f, 0.411f, 1.000f)
		SetupValue(weaponColor[1], 0.411f, "model", "weaponColorG");
		SetupValue(weaponColor[2], 0.411f, "model", "weaponColorB");
		SetupValue(weaponColor[3], 1.f, "model", "weaponColorA");
		SetupValue(weaponXhair, false, "model", "weaponXhair");

		SetupValue(weaponOverlay, false, "model", "weaponOverlay");
		SetupValue(weaponOverlayXhair, false, "model", "weaponOverlayXhair");
		SetupValue(weaponOverlayColor[0], 0.054f, "model", "weaponOverlayColorR"); // (0.054f, 0.056f, 0.092f, 1.000f)
		SetupValue(weaponOverlayColor[1], 0.056f, "model", "weaponOverlayColorG");
		SetupValue(weaponOverlayColor[2], 0.092f, "model", "weaponOverlayColorB");
		SetupValue(weaponOverlayColor[3], 1.f, "model", "weaponOverlayColorA");

		SetupValue(weaponThinOverlay, false, "model", "weaponThinOverlay");
		SetupValue(weaponThinOverlayXhair, false, "model", "weaponOverlayXhair");
		SetupValue(weaponThinOverlayColor[0], 0.008f, "model", "weaponThinOverlayColorR"); // (0.008f, 0.000f, 0.000f, 1.000f)
		SetupValue(weaponThinOverlayColor[1], 0.000f, "model", "weaponThinOverlayColorG");
		SetupValue(weaponThinOverlayColor[2], 0.000f, "model", "weaponThinOverlayColorB");
		SetupValue(weaponThinOverlayColor[3], 1.f, "model", "weaponThinOverlayColorA");

		SetupValue(weaponAnimOverlay, false, "model", "weaponAnimOverlay");
		SetupValue(weaponAnimOverlayXhair, false, "model", "weaponAnimOverlayXhair");
		SetupValue(weaponAnimOverlayColor[0], 0.368f, "model", "weaponAnimOverlayColorR"); // (0.368f, 0.400f, 0.515f, 1.000f)
		SetupValue(weaponAnimOverlayColor[1], 0.400f, "model", "weaponAnimOverlayColorG");
		SetupValue(weaponAnimOverlayColor[2], 0.515f, "model", "weaponAnimOverlayColorB");
		SetupValue(weaponAnimOverlayColor[3], 1.f, "model", "weaponAnimOverlayColorA");

		SetupValue(paperMode, false, "model", "paperMode");
	}

	// misc 
	{
		using namespace cfg::misc;

		SetupValue(bKeyBindList, true, "misc", "keybindList");
		SetupValue(bKeyBindList, "false", "misc", "bKeyBindListOldEnable");
		SetupValue(iKeyBindList, false, 10, "misc", "bKeyBindListOld");
		SetupValue(bBunnyHop, false, "misc", "bunnyhop");
		SetupValue(bAutoStrafe, false, "misc", "autoStrafe");
		SetupValue(bFastStop, false, "misc", "faststop");
		SetupValue(bInfiniteDuck, false, "misc", "infiniteDuck");
		SetupValue(bBlockbot, false, "misc", "blockbot");
		SetupValue(iBlockbotKey, 0, "misc", "blockbotKey");
		SetupValue(bClantag, false, "misc", "clantag");
		SetupValue(bInvertKnife, false, "misc", "bInvertKnife");

		SetupValue(bNightmode, false, "misc", "nightmode");
		SetupValue(flNightmodeColor[0], 1.f, "visual", "nightmodeColorR");
		SetupValue(flNightmodeColor[1], 1.f, "visual", "nightmodeColorG");
		SetupValue(flNightmodeColor[2], 1.f, "visual", "nightmodeColorB");
		SetupValue(flNightmodeColor[3], 1.f, "visual", "nightmodeColorA");
		SetupValue(flPropColor, 4, 1.f, "visual", "propColors");

		SetupValue(bOOF, false, "misc", "bOOF");
		SetupValue(flOOF[0], 1.f, "visual", "flOOFR");
		SetupValue(flOOF[1], 1.f, "visual", "flOOFG");
		SetupValue(flOOF[2], 1.f, "visual", "flOOFB");
		SetupValue(flOOF[3], 1.f, "visual", "flOOFA");
		SetupValue(iOOFDistance, 100, "visual", "iOOFDistance");
		SetupValue(iOOFSize, 30, "visual", "iOOFSize");

		SetupValue(iSkybox, 0, "misc", "skyboxchanger");
		SetupValue(flSkyboxColor[0], 1.f, "visual", "flSkyboxColorR");
		SetupValue(flSkyboxColor[1], 1.f, "visual", "flSkyboxColorG");
		SetupValue(flSkyboxColor[2], 1.f, "visual", "flSkyboxColorB");
		SetupValue(flSkyboxColor[3], 1.f, "visual", "flSkyboxColorA");

		SetupValue(bOverrideLampColors, false, "visual", "bLampColor");
		SetupValue(flLampColors, 4, 0.f, "visual", "flLampColors");
		SetupValue(iFlicker, 0, "visual", "iFlicker");

		SetupValue(bAspectRatio, false, "misc", "aspectRatio");
		SetupValue(iAspectRatio, 0, "misc", "aspectRatioValue");

		SetupValue(bPreserveKillfeed, false, "misc", "preserveKillfeed");
		SetupValue(bBulletImpact, false, "misc", "bulletImpact");
		SetupValue(flImpactColor[0], 4, 1.f, "misc", "impactColorS");
		SetupValue(flImpactColor[1], 4, 1.f, "misc", "impactColorC");

		SetupValue(bDroppedWeaponESP, false, "misc", "bDroppedWeaponESP");
		SetupValue(flDroppedWeaponESP, 4, 1.f, "misc", "flDroppedWeaponESP");

		SetupValue(bProjectileESP, false, "misc", "bProjectileESP");
		SetupValue(flProjectileESP, 4, 1.f, "misc", "flProjectileESP");

		SetupValue(bOnlyCheatlog, false, "misc", "onlyCheatLogs");

		SetupValue(bDrawCapsule, false, "misc", "bDrawCapsule");
		SetupValue(flDrawCapsuleColor[0], 1.f, "visual", "flDrawCapsuleColorR"); // (0.000f, 0.246f, 0.646f, 1.000f)
		SetupValue(flDrawCapsuleColor[1], 1.f, "visual", "flDrawCapsuleColorG");
		SetupValue(flDrawCapsuleColor[2], 1.f, "visual", "flDrawCapsuleColorB");
		SetupValue(flDrawCapsuleColor[3], 1.f, "visual", "flDrawCapsuleColorA");

		SetupValue(flDrawCapsuleColorHit[0], 1.f, "visual", "flDrawCapsuleColorHitR"); // (0.000f, 0.246f, 0.646f, 1.000f)
		SetupValue(flDrawCapsuleColorHit[1], 1.f, "visual", "flDrawCapsuleColorHitG");
		SetupValue(flDrawCapsuleColorHit[2], 1.f, "visual", "flDrawCapsuleColorHitB");
		SetupValue(flDrawCapsuleColorHit[3], 1.f, "visual", "flDrawCapsuleColorHitA");

		SetupValue(bWorldCrosshair, false, "misc", "bWorldCrosshair");
		SetupValue(flWorldCrosshairColor[0], 1.f, "visual", "flWorldCrosshairColorR"); // (0.000f, 0.246f, 0.646f, 1.000f)
		SetupValue(flWorldCrosshairColor[1], 1.f, "visual", "flWorldCrosshairColorG");
		SetupValue(flWorldCrosshairColor[2], 1.f, "visual", "flWorldCrosshairColorB");
		SetupValue(flWorldCrosshairColor[3], 1.f, "visual", "flWorldCrosshairColorA");

		SetupValue(bThirdPerson, false, "misc", "thirdperson");
		SetupValue(iThirdPersonKey, 0, "misc", "thirdpersonbind");
		SetupValue(iThirdPersonDistance, 160, "misc", "thirdpersonDistance");

		SetupValue(iViewModelFov, 60, "misc", "viewmodelFov");
		SetupValue(iDebugFov, 90, "misc", "fov");

		SetupValue(szBombBuffer, "RyzeXTR", "misc", "bombText");
		SetupValue(bKillsay, false, "misc", "bKillsay");
		SetupValue(szKillsayBuffer, "", "misc", "killSayBuffer");

		SetupValue(bSkinnyBoy, false, "misc", "bKinnyBoy");
		SetupValue(iSkinnyBoy, 0, "misc", "iSkinnyBoy");

		SetupValue(bHat, false, "misc", "bHat");
		SetupValue(flHat, 4, 1.f, "misc", "flHat");
		SetupValue(bCustomHud, false, "misc", "bCustomHud");

		SetupValue(bRemovals, false, 7, "misc", "removals");
		SetupValue(bOnScopeViewmodel, false, "misc", "drawViewmodelOnScope");
		SetupValue(flScopeColor[0], 1.f, "visual", "scopeColorR");
		SetupValue(flScopeColor[1], 1.f, "visual", "scopeColorG");
		SetupValue(flScopeColor[2], 1.f, "visual", "scopeColorB");
		SetupValue(flScopeColor[3], 1.f, "visual", "scopeColorA");
		SetupValue(flScopeColorEnd[0], 1.f, "visual", "scopeColorEndR");
		SetupValue(flScopeColorEnd[1], 1.f, "visual", "scopeColorEndG");
		SetupValue(flScopeColorEnd[2], 1.f, "visual", "scopeColorEndB");
		SetupValue(flScopeColorEnd[3], 1.f, "visual", "scopeColorEndA");
		SetupValue(iScopeLength, 1.f, "visual", "scopeLength");

		SetupValue(iPistols, 0, "misc", "autobuypistol");
		SetupValue(iSnipers, 0, "misc", "autobuyrifles");
		SetupValue(bEquipments, false, 3, "misc", "autobuyequipments");
		SetupValue(bGrenades, false, 5, "misc", "autobuygrenades");
		SetupValue(bAutobuy, false, "misc", "autobuyEnabled");

		SetupValue(bFakePing, false, "misc", "fakeping");
		SetupValue(flFakePingFactor, 0.f, "misc", "fakepingFactor");

		SetupValue( iHitSound, 0, "misc", "hitsoundtype" );
		SetupValue( flHitSoundVolume, 100.f, "misc", "hitsoundvolume" );
		SetupValue( szWavPath, "", "misc", "hitsoundpath" );
		SetupValue(cfg::m_iKeyStates, 256, 0, "misc", "keystates");

	}

	// skin
	{
		using namespace cfg::skin;
		SetupValue(bEnableSkinChanger, false, "skins", "bEnableSkinChanger");
		SetupValue(bFilterByWeapon, false, "skins", "bFilterByWeapon");
		SetupValue(bModifySkinColors, false, 37, "skins", "bModifySkinColors");

		SetupValue(iSkinId, 37, 0, "skins", "iSkinId");
		SetupValue(iSeed, 37, 0, "skins", "iSeed");
		SetupValue(iQuality, 37, 0, "skins", "iQuality");
		SetupValue(iSkinStattrak, 37, 0, "skins", "iSkinStattrak");
		SetupValue(iGloveModel, 0, "skins", "iGloveModel");
		SetupValue(iKnifeModel, 0, "skins", "iKnifeModel");

		SetupValue(flSkinWear, 37, 0.f, "skins", "flSkinWear");
		for (size_t i = 0; i < 37; i++)
		{
			SetupValue(colSkins1[i], 4, 1.f, "skins", std::format("colSkins1{}", i));
			SetupValue(colSkins2[i], 4, 1.f, "skins", std::format("colSkins2{}", i));
			SetupValue(colSkins3[i], 4, 1.f, "skins", std::format("colSkins3{}", i));
			SetupValue(colSkins4[i], 4, 1.f, "skins", std::format("colSkins4{}", i));
			SetupValue(szSkinNametag[i], "", "skins", std::format("szSkinNametag{}", i));
		}

	}
}

void CConfig::SetupValue(int& value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue< int >(category, name, &value));
}

void CConfig::SetupValue(int* value, int length, int def, std::string category, std::string name)
{
	for (size_t i = 0; i < length; i++)
	{
		// do not save every keybind
		//if (length == 256) 
		//	if (value[i] == 0)
		//		continue;
		
		value[i] = def;
		ints.push_back(new ConfigValue< int >(category, std::format("{}-{}", name, i), &value[i]));
	}
}

void CConfig::SetupValue(float& value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue< float >(category, name, &value));
}

void CConfig::SetupValue(float* value, int length, float def, std::string category, std::string name)
{
	for (size_t i = 0; i < length; i++)
	{
		value[i] = def;
		floats.push_back(new ConfigValue< float >(category, std::format("{}-{}", name, i), &value[i]));
	}
}

void CConfig::SetupValue(float* value, int length, float* def, std::string category, std::string name)
{
	for (size_t i = 0; i < length; i++)
	{
		value[i] = def[i];
		floats.push_back(new ConfigValue< float >(category, std::format("{}-{}", name, i), &value[i]));
	}
}

void CConfig::SetupValue(bool& value, bool def, std::string category, std::string name)
{
	value = def;
	bools.push_back(new ConfigValue< bool >(category, name, &value));
}

void CConfig::SetupValue(bool* value, bool def, int size, std::string category, std::string name) // for multiboxes
{
	for (int c = 0; c < size; c++)
	{
		value[c] = def;

		name += std::to_string(c);

		bools.push_back(new ConfigValue< bool >(category, name, &value[c]));
	}
}

void CConfig::SetupValue( std::string& value, std::string def, std::string category, std::string name )
{
	value = def;
	strings.push_back( new ConfigValue< std::string >( category, name, &value ) );
}

void CConfig::CreateMainDirectory( )
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;
	if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) )
	{
		folder = std::string(path) + "\\ryzextr\\";
	}
	
	if ( SUCCEEDED( CreateDirectory(folder.c_str(), NULL ) ) )
		printf( "Succesfully created directory" );
	else
		throw std::runtime_error( "Failed to create config directory" );


}

std::string cfgName = "";
void SaveThread();
void CConfig::Save(std::string ConfigName)
{
	if (bSaving || ConfigName.empty())
		return;

	bSaving = true;

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
		file = std::string(path) + "\\ryzextr\\" + ConfigName + ".xtr";
	}

	CreateDirectory(folder.c_str(), NULL);

	// Define lambda functions for saving each type of value
	auto saveInts = [&]() {
		for (auto value : ints)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	};

	auto saveFloats = [&]() {
		for (auto value : floats)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	};

	auto saveBools = [&]() {
		for (auto value : bools)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
	};

	auto saveStrings = [&]() {
		for (auto value : strings)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), reinterpret_cast<std::string*>(value->value)->c_str(), file.c_str());
	};

	//auto saveStuff = [&]() {

	//	// Create separate threads for each type of value
	//	std::thread intsThread(saveInts);
	//	std::thread floatsThread(saveFloats);
	//	std::thread boolsThread(saveBools);
	//	std::thread stringsThread(saveStrings);

	//	// Wait for all threads to finish
	//	intsThread.join();
	//	floatsThread.join();
	//	boolsThread.join();
	//	stringsThread.join();

	//};
	//std::thread save(saveStuff);
	//save.join();

	cfgName = ConfigName;
	auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(SaveThread), nullptr, 0, nullptr);
	CloseHandle(handle);

	bSaving = false;
}

void SaveThread() {

	Config2->bSaving = true;

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
		file = std::string(path) + "\\ryzextr\\" + cfgName + ".xtr";
	}

	CreateDirectory(folder.c_str(), NULL);

	// Define lambda functions for saving each type of value
	auto saveInts = [&]() {
		for (auto value : Config2->ints)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	};

	auto saveFloats = [&]() {
		for (auto value : Config2->floats)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	};

	auto saveBools = [&]() {
		for (auto value : Config2->bools)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
	};

	auto saveStrings = [&]() {
		for (auto value : Config2->strings)
			WritePrivateProfileString(value->category.c_str(), value->name.c_str(), reinterpret_cast<std::string*>(value->value)->c_str(), file.c_str());
	};

	// Create separate threads for each type of value
	std::thread intsThread(saveInts);
	std::thread floatsThread(saveFloats);
	std::thread boolsThread(saveBools);
	std::thread stringsThread(saveStrings);

	// Wait for all threads to finish
	intsThread.join();
	floatsThread.join();
	boolsThread.join();
	stringsThread.join();

	Config2->RefreshConfigs();
	Config2->bSaving = false;
}

void CConfig::Load(std::string ConfigName)
{
	if (bSaving)
		return;

	bSaving = true;

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
		file = std::string(path) + "\\ryzextr\\" + ConfigName + ".xtr";
	}

	CreateDirectory(folder.c_str(), NULL);

	char value_l[32] = { '\0' };
	char value_string[256] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : floats)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atof(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for ( auto value : strings )
	{
		GetPrivateProfileString( value->category.c_str( ), value->name.c_str( ), "", value_string, 256, file.c_str( ) );
		*value->value = value_string;
	}

	bSaving = false;
}

void CConfig::RefreshSounds( )
{
	vecSoundFileNames.clear( );

	static TCHAR path[ MAX_PATH ];
	std::string folder, file;

	if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) )
	{
		folder = std::string( path ) + "\\ryzextr\\sounds\\";
		SoundPath = folder;
	}

	CreateDirectory( folder.c_str( ), NULL );

	for ( const auto& it : std::filesystem::directory_iterator( folder.c_str( ) ) )
	{
		if ( it.path( ).filename( ).extension( ) == ".wav" )
		{
			//printf( std::format("found sound file: {}" , it.path( ).filename( ).string( ) ) );
			vecSoundFileNames.push_back( it.path( ).filename( ).string( ).c_str() );
		}
	}
}

void CConfig::RefreshConfigs() {

	vecConfigs.clear();

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
		ConfigPath = folder;
	}

	CreateDirectory(folder.c_str(), NULL);

	for (const auto& it : std::filesystem::directory_iterator(folder.c_str()))
	{
		if (it.path().filename().extension() == ".xtr")
		{
			//printf( std::format("found sound file: {}" , it.path( ).filename( ).string( ) ) );
			vecConfigs.push_back(it.path().filename().string().c_str());
			
			for (int i = 0; i < 4; i++)
				vecConfigs.back().pop_back();
		}
	}
}

void CConfig::DeleteConfig(std::string ConfigName) {

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
	}

	CreateDirectory(folder.c_str(), NULL);

	for (const auto& it : std::filesystem::directory_iterator(folder.c_str()))
	{
		if (it.path().filename() == ConfigName + ".xtr")
		{
			remove((folder + ConfigName + ".xtr").c_str());
		}
	}
}
CConfig* Config2 = new CConfig();