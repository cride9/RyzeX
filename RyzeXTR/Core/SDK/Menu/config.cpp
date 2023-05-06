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

	CreateMainDirectory();
	RefreshSounds();
	RefreshConfigs();

	// ragebot
	{
		using namespace cfg::rage;

		SetupValue(enable, false, "rage", "Rageenable");
		SetupValue(overrideBind, 0, "rage", "overrideBind");
		SetupValue( m_bEnableBacktrack, false, "rage", "Ragebacktrack" );
		SetupValue(forceBaim, false, "rage", "forceBaim");
		SetupValue(forceBaimKey, 0, "rage", "forceBaimKey");
		SetupValue(aimbotTargetSelection, 0, "rage", "aimbotTargetSelection");
		SetupValue(iAimbotFov, 180, "rage", "aimbotfov");

		SetupValue( autostop[ 0 ], false, "rage", "autostop1" );
		SetupValue( autostop[ 1 ], false, "rage", "autostop2" );
		SetupValue( autostop[ 2 ], false, "rage", "autostop3" );
		SetupValue( autostop[ 3 ], false, "rage", "autostop4" );
		SetupValue( autostop[ 4 ], false, "rage", "autostop5" );
		SetupValue( autostop[ 5 ], false, "rage", "autostop6" );

		SetupValue( betweenshots[ 0 ], false, "rage", "betweenshots1" );
		SetupValue( betweenshots[ 1 ], false, "rage", "betweenshots2" );
		SetupValue( betweenshots[ 2 ], false, "rage", "betweenshots3" );
		SetupValue( betweenshots[ 3 ], false, "rage", "betweenshots4" );
		SetupValue( betweenshots[ 4 ], false, "rage", "betweenshots5" );
		SetupValue( betweenshots[ 5 ], false, "rage", "betweenshots6" );

		SetupValue( autostopAggressiveness[ 0 ], false, "rage", "autostopAggressiveness1" );
		SetupValue( autostopAggressiveness[ 1 ], false, "rage", "autostopAggressiveness2" );
		SetupValue( autostopAggressiveness[ 2 ], false, "rage", "autostopAggressiveness3" );
		SetupValue( autostopAggressiveness[ 3 ], false, "rage", "autostopAggressiveness4" );
		SetupValue( autostopAggressiveness[ 4 ], false, "rage", "autostopAggressiveness5" );
		SetupValue( autostopAggressiveness[ 5 ], false, "rage", "autostopAggressiveness6" );

		SetupValue( autoscope[ 0 ], false, "rage", "autoscope1" );
		SetupValue( autoscope[ 1 ], false, "rage", "autoscope2" );
		SetupValue( autoscope[ 2 ], false, "rage", "autoscope3" );

		SetupValue(forceSafePoint[0], false, "rage", "forceSafePoint1");
		SetupValue(forceSafePoint[1], false, "rage", "forceSafePoint2");
		SetupValue(forceSafePoint[2], false, "rage", "forceSafePoint3");
		SetupValue(forceSafePoint[3], false, "rage", "forceSafePoint4");
		SetupValue(forceSafePoint[4], false, "rage", "forceSafePoint5");
		SetupValue(forceSafePoint[5], false, "rage", "forceSafePoint6");

		SetupValue(doubletap, false, "rage", "Ragedoubletap");
		SetupValue(doubletapkey, 0, "rage", "Ragedoubletapkey");

		SetupValue(resolver, false, "rage", "Rageresolver");

		SetupValue(etcMultiHitboxes, false, 6, "rage", "etcMultiHitboxes");
		SetupValue(etcHitboxes, false, 6, "rage", "etcHitboxes");
		SetupValue(etcSafeHitboxes, false, 6, "rage", "etcSafeHitboxes");
		SetupValue(etcHitchance, 0, "rage", "etcHitchance");
		SetupValue(etcMindmg, 0, "rage", "etcMindmg");
		SetupValue(etcHeadPoints, 0, "rage", "etcHeadPoints");
		SetupValue(etcBodyPoints, 0, "rage", "etcBodyPoints");
		SetupValue(etcOverride, 0, "rage", "etcOverride");

		SetupValue(autoMultiHitboxes, false, 6, "rage", "autoMultiHitboxes");
		SetupValue(autoHitboxes, false, 6, "rage", "autoHitboxes");
		SetupValue(autoSafeHitboxes, false, 6, "rage", "autoSafeHitboxes");
		SetupValue(autoHitchance, 0, "rage", "autoHitchance");
		SetupValue(autoMindmg, 0, "rage", "autoMindmg");
		SetupValue(autoHeadPoints, 0, "rage", "autoHeadPoints");
		SetupValue(autoBodyPoints, 0, "rage", "autoBodyPoints");
		SetupValue(autoOverride, 0, "rage", "autoOverride");

		SetupValue(scoutMultiHitboxes, false, 6, "rage", "scoutMultiHitboxes");
		SetupValue(scoutHitboxes, false, 6, "rage", "scoutHitboxes");
		SetupValue(scoutSafeHitboxes, false, 6, "rage", "scoutSafeHitboxes");
		SetupValue(scoutHitchance, 0, "rage", "scoutHitchance");
		SetupValue(scoutMindmg, 0, "rage", "scoutMindmg");
		SetupValue(scoutHeadPoints, 0, "rage", "scoutHeadPoints");
		SetupValue(scoutBodyPoints, 0, "rage", "scoutBodyPoints");
		SetupValue(scoutOverride, 0, "rage", "scoutOverride");

		SetupValue(awpMultiHitboxes, false, 6, "rage", "awpMultiHitboxes");
		SetupValue(awpHitboxes, false, 6, "rage", "awpHitboxes");
		SetupValue(awpSafeHitboxes, false, 6, "rage", "awpSafeHitboxes");
		SetupValue(awpHitchance, 0, "rage", "awpHitchance");
		SetupValue(awpMindmg, 0, "rage", "awpMindmg");
		SetupValue(awpHeadPoints, 0, "rage", "awpHeadPoints");
		SetupValue(awpBodyPoints, 0, "rage", "awpBodyPoints");
		SetupValue(awpOverride, 0, "rage", "awpOverride");

		SetupValue(pistolMultiHitboxes, false, 6, "rage", "pistolMultiHitboxes");
		SetupValue(pistolHitboxes, false, 6, "rage", "pistolHitboxes");
		SetupValue(pistolSafeHitboxes, false, 6, "rage", "pistolSafeHitboxes");
		SetupValue(pistolHitchance, 0, "rage", "pistolHitchance");
		SetupValue(pistolMindmg, 0, "rage", "pistolMindmg");
		SetupValue(pistolHeadPoints, 0, "rage", "pistolHeadPoints");
		SetupValue(pistolBodyPoints, 0, "rage", "pistolBodyPoints");
		SetupValue(pistolOverride, 0, "rage", "pistolOverride");

		SetupValue(heavypistolMultiHitboxes, false, 6, "rage", "heavypistolMultiHitboxes");
		SetupValue(heavypistolHitboxes, false, 6, "rage", "heavypistolHitboxes");
		SetupValue(heavypistolSafeHitboxes, false, 6, "rage", "heavypistolSafeHitboxes");
		SetupValue(heavypistolHitchance, 0, "rage", "heavypistolHitchance");
		SetupValue(heavypistolMindmg, 0, "rage", "heavypistolMindmg");
		SetupValue(heavypistolHeadPoints, 0, "rage", "heavypistolHeadPoints");
		SetupValue(heavypistolBodyPoints, 0, "rage", "heavypistolBodyPoints");
		SetupValue(heavypistolOverride, 0, "rage", "heavypistolOverride");

	}
	
	// antiaim
	{
		using namespace cfg::antiaim;

		SetupValue(bEnabled, false, "antiaim", "Antiaimenabled");

		SetupValue(iPitch, 0, "antiaim", "pitch");
		SetupValue(iYaw, 0, "antiaim", "yaw");
		SetupValue(iYawBase, 0, "antiaim", "yawbase");
		SetupValue(bSlideWalk, 0, "antiaim", "slidewalk");
		SetupValue(bInvertOnShoot, false, "antiaim", "bInvertOnShoot");
		SetupValue(bodyLean[0], 0.f, "antiaim", "bodyLean1");
		SetupValue(bodyLean[1], 0.f, "antiaim", "bodyLean2");

		SetupValue(modifier, 0, "antiaim", "modifier");
		SetupValue(desyncModifier, 0, "antiaim", "desyncModifier");
		SetupValue(desyncModifierValue, 0, "antiaim", "desyncModifierValue");

		SetupValue(jittervalue, 0, "antiaim", "jittervalue");
		SetupValue(invertangle, 0, "antiaim", "invertangle");

		SetupValue(iDesyncType, 0, "antiaim", "desynctype");
		SetupValue(flDesyncValue, 0, "antiaim", "desyncvalue");
		SetupValue(iInverterBind, 0, "antiaim", "desyncinverter");
		SetupValue(iFlickOffset, 0, "antiaim", "flickOffset");
		SetupValue(flickAngleSwitch, 0, "antiaim", "flickAngleSwitch");

		SetupValue( m_bSwayDesync, false, "antiaim", "swaylby" );
		
		SetupValue(enableFakelag, false, "antiaim", "fakelagswitch");
		SetupValue(fakelag, 0, "antiaim", "fakelag");
		SetupValue(fakelagmin, 0, "antiaim", "fakelagmin");
		SetupValue(fakelagmax, 0, "antiaim", "fakelagmax");
		SetupValue(fakelagType, 0, "antiaim", "fakelagType");

		SetupValue(defensive, false, "antiaim", "defensive");

		SetupValue(fakewalk, 0, "antiaim", "fakewalk");
		SetupValue(fakeduck, false, "antiaim", "fakeduck");
		SetupValue(fakeduckbind, 0, "antiaim", "fakeduckbind");
		SetupValue(idealTick, false, "antiaim", "idealTick");
		SetupValue(idealTickBind, 0, "antiaim", "idealTickBind");

		SetupValue(fakewalkenable, false, "antiaim", "fakewalk");
		SetupValue(fakewalk, 0, "antiaim", "fakewalkspeed");
		SetupValue(fakewalkKey, 0, "antiaim", "fakewalkbind");

		SetupValue(freestand, 0, "antiaim", "freestand");
	}

	// visual
	{
		using namespace cfg::visual;

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

		SetupValue(enemyType, 0, "model", "enemyType");

		SetupValue(enemyBTEnable, 0, "model", "enemyBTEnable");
		SetupValue(enemyBTType, 0, "model", "enemyBTType");
		SetupValue(enemyBTXhair, 0, "model", "enemyBTXhair");
		SetupValue(enemyBTColor[0], 1.f, "model", "enemyBTColorR"); // (1.000f, 0.000f, 0.000f, 0.570f)
		SetupValue(enemyBTColor[1], 0.000f, "model", "enemyBTColorG");
		SetupValue(enemyBTColor[2], 0.000f, "model", "enemyBTColorB");
		SetupValue(enemyBTColor[3], 0.570f, "model", "enemyBTColorA");

		// chams
		SetupValue(enemy, false, "model", "enemy");
		SetupValue(enemyXQZ, false, "model", "enemyXQZ");

		SetupValue(enemyColor[0], 0.834f, "model", "enemyColorR"); // (0.834f, 0.834f, 0.834f, 1.000f)
		SetupValue(enemyColor[1], 0.834f, "model", "enemyColorG");
		SetupValue(enemyColor[2], 0.834f, "model", "enemyColorB");
		SetupValue(enemyColor[3], 1.f, "model", "enemyColorA");

		SetupValue(enemyXQZColor[0], 0.771f, "model", "enemyXQZColorR"); // (0.771f, 0.771f, 0.771f, 0.327f)
		SetupValue(enemyXQZColor[1], 0.771f, "model", "enemyXQZColorG");
		SetupValue(enemyXQZColor[2], 0.771f, "model", "enemyXQZColorB");
		SetupValue(enemyXQZColor[3], 0.327f, "model", "enemyXQZColorA");

		SetupValue(enemyXhair, false, "model", "enemyXhair");
		SetupValue(enemyXQZXhair, false, "model", "enemyXQZXhair");

		// overlay
		SetupValue(enemyOverlay, false, "model", "enemyOverlay");
		SetupValue(enemyOverlayXQZ, false, "model", "enemyOverlayXQZ");

		SetupValue(enemyOverlayColor[0], 0.146f, "model", "enemyOverlayColorR"); // (0.146f, 0.104f, 0.252f, 1.000f)
		SetupValue(enemyOverlayColor[1], 0.104f, "model", "enemyOverlayColorG");
		SetupValue(enemyOverlayColor[2], 0.252f, "model", "enemyOverlayColorB");
		SetupValue(enemyOverlayColor[3], 1.f, "model", "enemyOverlayColorA");

		SetupValue(enemyOverlayXQZColor[0], 0.132f, "model", "enemyOverlayXQZColorR"); // (0.132f, 0.000f, 0.168f, 1.000f)
		SetupValue(enemyOverlayXQZColor[1], 0.000f, "model", "enemyOverlayXQZColorG");
		SetupValue(enemyOverlayXQZColor[2], 0.168f, "model", "enemyOverlayXQZColorB");
		SetupValue(enemyOverlayXQZColor[3], 1.f, "model", "enemyOverlayXQZColorA");

		SetupValue(enemyOverlayXhair, false, "model", "enemyOverlayXhair");
		SetupValue(enemyOverlayXQZXhair, false, "model", "enemyOverlayXQZXhair");

		// glow
		SetupValue(enemyThinOverlay, false, "model", "enemyThinOverlay");
		SetupValue(enemyThinOverlayXQZ, false, "model", "enemyThinOverlayXQZ");

		SetupValue(enemyThinOverlayColor[0], 0.019f, "model", "enemyThinOverlayColorR"); // (0.019f, 0.000f, 0.227f, 1.000f)
		SetupValue(enemyThinOverlayColor[1], 0.000f, "model", "enemyThinOverlayColorG");
		SetupValue(enemyThinOverlayColor[2], 0.227f, "model", "enemyThinOverlayColorB");
		SetupValue(enemyThinOverlayColor[3], 1.f, "model", "enemyThinOverlayColorA");

		SetupValue(enemyThinOverlayXQZColor[0], 1.f, "model", "enemyThinOverlayXQZColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(enemyThinOverlayXQZColor[1], 1.f, "model", "enemyThinOverlayXQZColorG");
		SetupValue(enemyThinOverlayXQZColor[2], 1.f, "model", "enemyThinOverlayXQZColorB");
		SetupValue(enemyThinOverlayXQZColor[3], 1.f, "model", "enemyThinOverlayXQZColorA");

		SetupValue(enemyThinOverlayXhair, false, "model", "enemyThinOverlayXhair");
		SetupValue(enemyThinOverlayXQZXhair, false, "model", "enemyThinOverlayXQZXhair");

		// animated
		SetupValue(enemyAnimOverlay, false, "model", "enemyAnimOverlay");
		SetupValue(enemyAnimOverlayXQZ, false, "model", "enemyAnimOverlayXQZ");

		SetupValue(enemyAnimOverlayColor[0], 0.429f, "model", "enemyAnimOverlayColorR"); // (0.429f, 0.928f, 1.000f, 1.000f)
		SetupValue(enemyAnimOverlayColor[1], 0.928f, "model", "enemyAnimOverlayColorG");
		SetupValue(enemyAnimOverlayColor[2], 1.000f, "model", "enemyAnimOverlayColorB");
		SetupValue(enemyAnimOverlayColor[3], 1.f, "model", "enemyAnimOverlayColorA");

		SetupValue(enemyAnimOverlayXQZColor[0], 0.494f, "model", "enemyAnimOverlayXQZColorR"); // (0.494f, 0.000f, 0.000f, 1.000f)
		SetupValue(enemyAnimOverlayXQZColor[1], 0.000f, "model", "enemyAnimOverlayXQZColorG");
		SetupValue(enemyAnimOverlayXQZColor[2], 0.000f, "model", "enemyAnimOverlayXQZColorB");
		SetupValue(enemyAnimOverlayXQZColor[3], 1.f, "model", "enemyAnimOverlayXQZColorA");

		SetupValue(enemyAnimOverlayXhair, false, "model", "enemyAnimOverlayXhair");
		SetupValue(enemyAnimOverlayXQZXhair, false, "model", "enemyAnimOverlayXQZXhair");

		// teammate
		SetupValue(teamType, 0, "model", "teamType");

		// chams
		SetupValue(team, false, "model", "team");
		SetupValue(teamXQZ, false, "model", "teamXQZ");

		SetupValue(teamColor[0], 0.591f, "model", "teamColorR"); // (0.591f, 0.591f, 0.591f, 1.000f)
		SetupValue(teamColor[1], 0.591f, "model", "teamColorG");
		SetupValue(teamColor[2], 0.591f, "model", "teamColorB");
		SetupValue(teamColor[3], 1.f, "model", "teamColorA");

		SetupValue(teamXQZColor[0], 0.723f, "model", "teamXQZColorR"); // (0.723f, 0.723f, 0.723f, 0.404f)
		SetupValue(teamXQZColor[1], 0.723f, "model", "teamXQZColorG");
		SetupValue(teamXQZColor[2], 0.723f, "model", "teamXQZColorB");
		SetupValue(teamXQZColor[3], 0.404f, "model", "teamXQZColorA");

		SetupValue(teamXhair, false, "model", "teamXhair");
		SetupValue(teamXQZXhair, false, "model", "teamXQZXhair");

		// overlay
		SetupValue(teamOverlay, false, "model", "teamOverlay");
		SetupValue(teamOverlayXQZ, false, "model", "teamOverlayXQZ");

		SetupValue(teamOverlayColor[0], 0.070f, "model", "teamOverlayColorR"); // (0.070f, 0.154f, 0.073f, 1.000f)
		SetupValue(teamOverlayColor[1], 0.154f, "model", "teamOverlayColorG");
		SetupValue(teamOverlayColor[2], 0.073f, "model", "teamOverlayColorB");
		SetupValue(teamOverlayColor[3], 1.f, "model", "teamOverlayColorA");

		SetupValue(teamOverlayXQZColor[0], 0.034f, "model", "teamOverlayXQZColorR"); // (0.034f, 0.047f, 0.098f, 1.000f)
		SetupValue(teamOverlayXQZColor[1], 0.047f, "model", "teamOverlayXQZColorG");
		SetupValue(teamOverlayXQZColor[2], 0.098f, "model", "teamOverlayXQZColorB");
		SetupValue(teamOverlayXQZColor[3], 1.f, "model", "teamOverlayXQZColorA");

		SetupValue(teamOverlayXhair, false, "model", "teamOverlayXhair");
		SetupValue(teamOverlayXQZXhair, false, "model", "teamOverlayXQZXhair");

		// glow
		SetupValue(teamThinOverlay, false, "model", "teamThinOverlay");
		SetupValue(teamThinOverlayXQZ, false, "model", "teamThinOverlayXQZ");

		SetupValue(teamThinOverlayColor[0], 0.216f, "model", "teamThinOverlayColorR"); // (0.216f, 0.209f, 0.051f, 1.000f)
		SetupValue(teamThinOverlayColor[1], 0.209f, "model", "teamThinOverlayColorG");
		SetupValue(teamThinOverlayColor[2], 0.051f, "model", "teamThinOverlayColorB");
		SetupValue(teamThinOverlayColor[3], 1.f, "model", "teamThinOverlayColorA");
		 
		SetupValue(teamThinOverlayXQZColor[0], 0.045f, "model", "teamThinOverlayXQZColorR"); // (0.045f, 0.033f, 0.133f, 1.000f)
		SetupValue(teamThinOverlayXQZColor[1], 0.033f, "model", "teamThinOverlayXQZColorG");
		SetupValue(teamThinOverlayXQZColor[2], 0.133f, "model", "teamThinOverlayXQZColorB");
		SetupValue(teamThinOverlayXQZColor[3], 1.f, "model", "teamThinOverlayXQZColorA");

		SetupValue(teamThinOverlayXhair, false, "model", "teamThinOverlayXhair");
		SetupValue(teamThinOverlayXQZXhair, false, "model", "teamThinOverlayXQZXhair");

		// animated
		SetupValue(teamAnimOverlay, false, "model", "teamAnimOverlay");
		SetupValue(teamAnimOverlayXQZ, false, "model", "teamAnimOverlayXQZ");

		SetupValue(teamAnimOverlayColor[0], 0.132f, "model", "teamAnimOverlayColorR"); // (0.132f, 0.074f, 0.813f, 1.000f)
		SetupValue(teamAnimOverlayColor[1], 0.074f, "model", "teamAnimOverlayColorG");
		SetupValue(teamAnimOverlayColor[2], 0.813f, "model", "teamAnimOverlayColorB");
		SetupValue(teamAnimOverlayColor[3], 1.f, "model", "teamAnimOverlayColorA");

		SetupValue(teamAnimOverlayXQZColor[0], 0.267f, "model", "teamAnimOverlayXQZColorR"); // (0.267f, 0.674f, 0.486f, 1.000f)
		SetupValue(teamAnimOverlayXQZColor[1], 0.674f, "model", "teamAnimOverlayXQZColorG");
		SetupValue(teamAnimOverlayXQZColor[2], 0.486f, "model", "teamAnimOverlayXQZColorB");
		SetupValue(teamAnimOverlayXQZColor[3], 1.f, "model", "teamAnimOverlayXQZColorA");

		SetupValue(teamAnimOverlayXhair, false, "model", "teamAnimOverlayXhair");
		SetupValue(teamAnimOverlayXQZXhair, false, "model", "teamAnimOverlayXQZXhair");

		// local
		SetupValue(localType, 0, "model", "localType");

		// chams
		SetupValue(local, false, "model", "local");
		SetupValue(localXQZ, false, "model", "localXQZ");

		SetupValue(localColor[0], 0.490f, "model", "localColorR");  // (0.490f, 0.490f, 0.490f, 1.000f)
		SetupValue(localColor[1], 0.490f, "model", "localColorG");
		SetupValue(localColor[2], 0.490f, "model", "localColorB");
		SetupValue(localColor[3], 1.f, "model", "localColorA");

		SetupValue(localXQZColor[0], 1.f, "model", "localXQZColorR"); // (1.000f, 0.753f, 0.000f, 0.333f)
		SetupValue(localXQZColor[1], 0.753f, "model", "localXQZColorG");
		SetupValue(localXQZColor[2], 0.f, "model", "localXQZColorB");
		SetupValue(localXQZColor[3], 0.333f, "model", "localXQZColorA");

		SetupValue(localXhair, false, "model", "localXhair");
		SetupValue(localXQZXhair, false, "model", "localXQZXhair");

		// overlay
		SetupValue(localOverlay, false, "model", "localOverlay");
		SetupValue(localOverlayXQZ, false, "model", "localOverlayXQZ");

		SetupValue(localOverlayColor[0], 0.012f, "model", "localOverlayColorR"); // (0.012f, 0.000f, 0.092f, 1.000f)
		SetupValue(localOverlayColor[1], 0.000f, "model", "localOverlayColorG");
		SetupValue(localOverlayColor[2], 0.092f, "model", "localOverlayColorB");
		SetupValue(localOverlayColor[3], 1.f, "model", "localOverlayColorA");

		SetupValue(localOverlayXQZColor[0], 1.f, "model", "localOverlayXQZColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(localOverlayXQZColor[1], 1.f, "model", "localOverlayXQZColorG");
		SetupValue(localOverlayXQZColor[2], 1.f, "model", "localOverlayXQZColorB");
		SetupValue(localOverlayXQZColor[3], 1.f, "model", "localOverlayXQZColorA");

		SetupValue(localOverlayXhair, false, "model", "localOverlayXhair");
		SetupValue(localOverlayXQZXhair, false, "model", "localOverlayXQZXhair");

		// glow
		SetupValue(localThinOverlay, false, "model", "localThinOverlay");
		SetupValue(localThinOverlayXQZ, false, "model", "localThinOverlayXQZ");

		SetupValue(localThinOverlayColor[0], 0.000f, "model", "localThinOverlayColorR"); // (0.000f, 0.017f, 0.016f, 1.000f)
		SetupValue(localThinOverlayColor[1], 0.017f, "model", "localThinOverlayColorG");
		SetupValue(localThinOverlayColor[2], 0.016f, "model", "localThinOverlayColorB");
		SetupValue(localThinOverlayColor[3], 1.f, "model", "localThinOverlayColorA");

		SetupValue(localThinOverlayXQZColor[0], 1.f, "model", "localThinOverlayXQZColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(localThinOverlayXQZColor[1], 1.f, "model", "localThinOverlayXQZColorG");
		SetupValue(localThinOverlayXQZColor[2], 1.f, "model", "localThinOverlayXQZColorB");
		SetupValue(localThinOverlayXQZColor[3], 1.f, "model", "localThinOverlayXQZColorA");

		SetupValue(localThinOverlayXhair, false, "model", "localThinOverlayXhair");
		SetupValue(localThinOverlayXQZXhair, false, "model", "localThinOverlayXQZXhair");

		// animated
		SetupValue(localAnimOverlay, false, "model", "localAnimOverlay");
		SetupValue(localAnimOverlayXQZ, false, "model", "localAnimOverlayXQZ");

		SetupValue(localAnimOverlayColor[0], 0.251f, "model", "localAnimOverlayColorR"); // (0.251f, 0.340f, 0.515f, 1.000f)
		SetupValue(localAnimOverlayColor[1], 0.340f, "model", "localAnimOverlayColorG");
		SetupValue(localAnimOverlayColor[2], 0.515f, "model", "localAnimOverlayColorB");
		SetupValue(localAnimOverlayColor[3], 1.f, "model", "localAnimOverlayColorA");

		SetupValue(localAnimOverlayXQZColor[0], 1.f, "model", "localAnimOverlayXQZColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(localAnimOverlayXQZColor[1], 1.f, "model", "localAnimOverlayXQZColorG");
		SetupValue(localAnimOverlayXQZColor[2], 1.f, "model", "localAnimOverlayXQZColorB");
		SetupValue(localAnimOverlayXQZColor[3], 1.f, "model", "localAnimOverlayXQZColorA");

		SetupValue(localAnimOverlayXhair, false, "model", "localAnimOverlayXhair");
		SetupValue(localAnimOverlayXQZXhair, false, "model", "localAnimOverlayXQZXhair");

		// ideal tick
		SetupValue(localIdealTick, false, "model", "localIdealTick");

		SetupValue(localIdealTickColor[0], 0.508f, "model", "localIdealTickColorR");  // (0.508f, 0.982f, 1.000f, 1.000f)
		SetupValue(localIdealTickColor[1], 0.982f, "model", "localIdealTickColorG");
		SetupValue(localIdealTickColor[2], 1.f, "model", "localIdealTickColorB");
		SetupValue(localIdealTickColor[3], 1.f, "model", "localIdealTickColorA");

		SetupValue(localDesync, false, "model", "localDesync");
		SetupValue(localDesyncType, 0, "model", "localDesyncType");
		SetupValue(localDesyncXhair, false, "model", "localDesyncXhair");

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

		SetupValue(keyBindList, true, "misc", "keybindList");
		SetupValue(bunnyhop, false, "misc", "bunnyhop");
		SetupValue(autoStrafe, false, "misc", "autoStrafe");
		SetupValue(faststop, false, "misc", "faststop");
		SetupValue(infiniteDuck, false, "misc", "infiniteDuck");
		SetupValue(blockbot, false, "misc", "blockbot");
		SetupValue(blockbotKey, 0, "misc", "blockbotKey");
		SetupValue(clantag, false, "misc", "clantag");

		SetupValue(nightmode, false, "misc", "nightmode");
		SetupValue(nightmodeColor[0], 1.f, "visual", "nightmodeColorR");
		SetupValue(nightmodeColor[1], 1.f, "visual", "nightmodeColorG");
		SetupValue(nightmodeColor[2], 1.f, "visual", "nightmodeColorB");
		SetupValue(nightmodeColor[3], 1.f, "visual", "nightmodeColorA");

		SetupValue(iSkybox, 0, "misc", "skyboxchanger");

		SetupValue(aspectRatio, false, "misc", "aspectRatio");
		SetupValue(aspectRatioValue, 0, "misc", "aspectRatioValue");

		SetupValue(preserveKillfeed, false, "misc", "preserveKillfeed");
		SetupValue(bulletImpact, false, "misc", "bulletImpact");
		SetupValue(impactColor[0], 4, 1.f, "misc", "impactColorS");
		SetupValue(impactColor[1], 4, 1.f, "misc", "impactColorC");

		SetupValue(onlyCheatLogs, true, "misc", "onlyCheatLogs");

		SetupValue(bulletTracer, false, "misc", "bulletTracer");
		SetupValue(bulletTracerColor[0], 0.000f, "visual", "bulletTracerColorR"); // (0.000f, 0.246f, 0.646f, 1.000f)
		SetupValue(bulletTracerColor[1], 0.246f, "visual", "bulletTracerColorG");
		SetupValue(bulletTracerColor[2], 0.646f, "visual", "bulletTracerColorB");
		SetupValue(bulletTracerColor[3], 1.f, "visual", "bulletTracerColorA");

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

		SetupValue(thirdperson, false, "misc", "thirdperson");
		SetupValue(thirdpersonbind, 0, "misc", "thirdpersonbind");

		SetupValue(viewmodelFov, 75, "misc", "viewmodelFov");
		SetupValue(fov, 90, "misc", "fov");

		SetupValue(removals, false, 5, "misc", "removals");
		SetupValue(drawViewmodelOnScope, false, "misc", "drawViewmodelOnScope");
		SetupValue(scopeColor[0], 1.f, "visual", "scopeColorR");
		SetupValue(scopeColor[1], 1.f, "visual", "scopeColorG");
		SetupValue(scopeColor[2], 1.f, "visual", "scopeColorB");
		SetupValue(scopeColor[3], 1.f, "visual", "scopeColorA");
		SetupValue(scopeLength, 1.f, "visual", "scopeLength");

		SetupValue(pistols, 0, "misc", "autobuypistol");
		SetupValue(snipers, 0, "misc", "autobuyrifles");
		SetupValue(equipments, false, 3, "misc", "autobuyequipments");
		SetupValue(grenades, false, 5, "misc", "autobuygrenades");
		SetupValue(autobuyEnabled, false, "misc", "autobuyEnabled");

		SetupValue(fakePing, false, "misc", "fakeping");
		SetupValue(fakePingFactor, 0.f, "misc", "fakepingFactor");

		SetupValue( m_iHitSound, 0, "misc", "hitsoundtype" );
		SetupValue( m_flHitSoundVolume, 100.f, "misc", "hitsoundvolume" );
		SetupValue( m_szWavPath, "", "misc", "hitsoundpath" );
		SetupValue(cfg::m_iKeyStates, 256, 0, "misc", "keystates");
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

void CConfig::Save(std::string ConfigName)
{
	bSaving = true;

	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\ryzextr\\";
		file = std::string(path) + "\\ryzextr\\" + ConfigName + ".xtr";
	}

	CreateDirectory(folder.c_str(), NULL);

	for (auto value : ints)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : floats)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	for ( auto value : strings )
		WritePrivateProfileString( value->category.c_str( ), value->name.c_str( ), reinterpret_cast< std::string* >( value->value )->c_str( ), file.c_str( ) );

	bSaving = false;
}

void CConfig::Load(std::string ConfigName)
{
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
			vecSoundFileNames.push_back( it.path( ).filename( ).string( ) );
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
			vecConfigs.push_back(it.path().filename().string());
			for (int i = 0; i < 4; i++)
				vecConfigs.back().pop_back();;
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