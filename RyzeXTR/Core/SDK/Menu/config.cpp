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

void CConfig::Setup() {

	// ragebot
	{
		using namespace cfg::rage;

		SetupValue(enable, false, "rage", "Rageenable");
		SetupValue(autostop, false, "rage", "Rageautostop");
		SetupValue(betweenshots, false, "rage", "Ragebetweenshots");
		SetupValue(overrideBind, 0, "rage", "overrideBind");
		SetupValue( m_bEnableBacktrack, false, "rage", "Ragebacktrack" );
		SetupValue(forceBaim, false, "rage", "forceBaim");
		SetupValue(forceBaimKey, 0, "rage", "forceBaimKey");
		SetupValue(aimbotTargetSelection, 0, "rage", "aimbotTargetSelection");
		SetupValue(autostopAggressiveness, 1, "rage", "autostopAggressiveness");

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

		SetupValue(enabled, false, "antiaim", "Antiaimenabled");

		SetupValue(pitch, 0, "antiaim", "pitch");
		SetupValue(yaw, 0, "antiaim", "yaw");
		SetupValue(yawBase, 0, "antiaim", "yawbase");
		SetupValue(slidewalk, 0, "antiaim", "slidewalk");

		SetupValue(modifier, 0, "antiaim", "modifier");
		SetupValue(desyncModifier, 0, "antiaim", "desyncModifier");
		SetupValue(desyncModifierValue, 0, "antiaim", "desyncModifierValue");

		SetupValue(jittervalue, 0, "antiaim", "jittervalue");
		SetupValue(invertangle, 0, "antiaim", "invertangle");

		SetupValue(desynctype, 0, "antiaim", "desynctype");
		SetupValue(desyncvalue, 0, "antiaim", "desyncvalue");
		SetupValue(desyncinverter, 0, "antiaim", "desyncinverter");

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
		SetupValue(enemyEsp, false, "visual", "enemyEsp");

		SetupValue(enemyName, false, "visual", "enemyName");
		SetupValue(enemyNameColor[0], 1.f, "visual", "enemyNameColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(enemyNameColor[1], 1.f, "visual", "enemyNameColorG");
		SetupValue(enemyNameColor[2], 1.f, "visual", "enemyNameColorB");
		SetupValue(enemyNameColor[3], 1.f, "visual", "enemyNameColorA");

		SetupValue(enemyBox, false, "visual", "enemyBox");
		SetupValue(enemyBoxColor[0], 1.f, "visual", "enemyBoxColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(enemyBoxColor[1], 1.f, "visual", "enemyBoxColorG");
		SetupValue(enemyBoxColor[2], 1.f, "visual", "enemyBoxColorB");
		SetupValue(enemyBoxColor[3], 1.f, "visual", "enemyBoxColorA");

		SetupValue(enemyHealth, false, "visual", "enemyHealth");
		SetupValue(enemyHealthColor[0], 0.044f, "visual", "enemyHealthColorR"); // (0.044f, 1.000f, 0.000f, 1.000f)
		SetupValue(enemyHealthColor[1], 1.000f, "visual", "enemyHealthColorG");
		SetupValue(enemyHealthColor[2], 0.000f, "visual", "enemyHealthColorB");
		SetupValue(enemyHealthColor[3], 1.f, "visual", "enemyHealthColorA");

		SetupValue(enemyHealthColorEnd[0], 0.044f, "visual", "enemyHealthColorEndR"); // (0.044f, 1.000f, 0.000f, 1.000f)
		SetupValue(enemyHealthColorEnd[1], 1.000f, "visual", "enemyHealthColorEndG");
		SetupValue(enemyHealthColorEnd[2], 0.000f, "visual", "enemyHealthColorEndB");
		SetupValue(enemyHealthColorEnd[3], 1.f, "visual", "enemyHealthColorEndA");

		SetupValue(enemyGlow, false, "visual", "enemyGlow");
		SetupValue(enemyGlowColor[0], 0.303f, "visual", "enemyGlowColorR"); // (0.303f, 0.302f, 0.140f, 1.000f)
		SetupValue(enemyGlowColor[1], 0.302f, "visual", "enemyGlowColorG");
		SetupValue(enemyGlowColor[2], 0.140f, "visual", "enemyGlowColorB");
		SetupValue(enemyGlowColor[3], 1.f, "visual", "enemyGlowColorA");

		SetupValue(enemyArmor, false, "visual", "enemyArmor");
		SetupValue(enemyArmorColor[0], 0.000f, "visual", "enemyArmorColorR"); // (0.000f, 0.924f, 1.000f, 1.000f)
		SetupValue(enemyArmorColor[1], 0.924f, "visual", "enemyArmorColorG");
		SetupValue(enemyArmorColor[2], 1.000f, "visual", "enemyArmorColorB");
		SetupValue(enemyArmorColor[3], 1.f, "visual", "enemyArmorColorA");

		SetupValue(enemyAmmo, false, "visual", "enemyAmmo");
		SetupValue(enemyAmmoColor[0], 0.345f, "visual", "enemyAmmoColorR"); // (0.345f, 0.620f, 1.000f, 1.000f)
		SetupValue(enemyAmmoColor[1], 0.620f, "visual", "enemyAmmoColorG");
		SetupValue(enemyAmmoColor[2], 1.f, "visual", "enemyAmmoColorB");
		SetupValue(enemyAmmoColor[3], 1.f, "visual", "enemyAmmoColorA");

		SetupValue(enemyWeapon, false, "visual", "enemyWeapon");
		SetupValue(enemyWeaponColor[0], 1.f, "visual", "enemyWeaponColorR"); // (1.000f, 1.000f, 1.000f, 1.000f)
		SetupValue(enemyWeaponColor[1], 1.f, "visual", "enemyWeaponColorG");
		SetupValue(enemyWeaponColor[2], 1.f, "visual", "enemyWeaponColorB");
		SetupValue(enemyWeaponColor[3], 1.f, "visual", "enemyWeaponColorA");

		SetupValue(enemyMoney, false, "visual", "enemyMoney");
		SetupValue(enemyMoneyColor[0], 0.790f, "visual", "enemyMoneyColorR"); // (0.790f, 1.000f, 0.000f, 1.000f)
		SetupValue(enemyMoneyColor[1], 1.000f, "visual", "enemyMoneyColorG");
		SetupValue(enemyMoneyColor[2], 0.000f, "visual", "enemyMoneyColorB");
		SetupValue(enemyMoneyColor[3], 1.f, "visual", "enemyMoneyColorA");

		// teammate
		SetupValue(teamEsp, false, "visual", "teamEsp");

		SetupValue(teamName, false, "visual", "teamName");
		SetupValue(teamNameColor[0], 1.f, "visual", "teamNameColorR"); 
		SetupValue(teamNameColor[1], 1.f, "visual", "teamNameColorG");
		SetupValue(teamNameColor[2], 1.f, "visual", "teamNameColorB");
		SetupValue(teamNameColor[3], 1.f, "visual", "teamNameColorA");

		SetupValue(teamBox, false, "visual", "teamBox");
		SetupValue(teamBoxColor[0], 1.f, "visual", "teamBoxColorR"); 
		SetupValue(teamBoxColor[1], 1.f, "visual", "teamBoxColorG");
		SetupValue(teamBoxColor[2], 1.f, "visual", "teamBoxColorB");
		SetupValue(teamBoxColor[3], 1.f, "visual", "teamBoxColorA");

		SetupValue(teamHealth, false, "visual", "teamHealth");
		SetupValue(teamHealthColor[0], 0.211f, "visual", "teamHealthColorR"); // (0.211f, 1.000f, 0.000f, 1.000f)
		SetupValue(teamHealthColor[1], 1.f, "visual", "teamHealthColorG");
		SetupValue(teamHealthColor[2], 0.000f, "visual", "teamHealthColorB");
		SetupValue(teamHealthColor[3], 1.f, "visual", "teamHealthColorA");

		SetupValue(teamHealthColorEnd[0], 0.211f, "visual", "teamHealthColorEndR"); // (0.211f, 1.000f, 0.000f, 1.000f)
		SetupValue(teamHealthColorEnd[1], 1.f, "visual", "teamHealthColorEndG");
		SetupValue(teamHealthColorEnd[2], 0.000f, "visual", "teamHealthColorEndB");
		SetupValue(teamHealthColorEnd[3], 1.f, "visual", "teamHealthColorEndA");

		SetupValue(teamGlow, false, "visual", "teamGlow");
		SetupValue(teamGlowColor[0], 0.125f, "visual", "teamGlowColorR"); // (0.125f, 0.272f, 0.000f, 1.000f)
		SetupValue(teamGlowColor[1], 0.272f, "visual", "teamGlowColorG");
		SetupValue(teamGlowColor[2], 0.000f, "visual", "teamGlowColorB");
		SetupValue(teamGlowColor[3], 1.f, "visual", "teamGlowColorA");

		SetupValue(teamArmor, false, "visual", "teamArmor");
		SetupValue(teamArmorColor[0], 0.000f, "visual", "teamArmorColorR"); // (0.000f, 1.000f, 0.954f, 1.000f)
		SetupValue(teamArmorColor[1], 1.f, "visual", "teamArmorColorG");
		SetupValue(teamArmorColor[2], 0.954f, "visual", "teamArmorColorB");
		SetupValue(teamArmorColor[3], 1.f, "visual", "teamArmorColorA");

		SetupValue(teamAmmo, false, "visual", "teamAmmo");
		SetupValue(teamAmmoColor[0], 0.000f, "visual", "teamAmmoColorR"); // (0.000f, 0.546f, 1.000f, 1.000f)
		SetupValue(teamAmmoColor[1], 0.546f, "visual", "teamAmmoColorG");
		SetupValue(teamAmmoColor[2], 1.f, "visual", "teamAmmoColorB");
		SetupValue(teamAmmoColor[3], 1.f, "visual", "teamAmmoColorA");

		SetupValue(teamMoney, false, "visual", "teamMoney");
		SetupValue(teamMoneyColor[0], 0.669f, "visual", "teamMoneyColorR"); // (0.669f, 1.000f, 0.000f, 1.000f)
		SetupValue(teamMoneyColor[1], 1.f, "visual", "teamMoneyColorG");
		SetupValue(teamMoneyColor[2], 0.000f, "visual", "teamMoneyColorB");
		SetupValue(teamMoneyColor[3], 1.f, "visual", "teamMoneyColorA");

		// local
		SetupValue(localEsp, false, "visual", "localEsp");

		SetupValue(localName, false, "visual", "localName");
		SetupValue(localNameColor[0], 1.f, "visual", "localNameColorR"); 
		SetupValue(localNameColor[1], 1.f, "visual", "localNameColorG");
		SetupValue(localNameColor[2], 1.f, "visual", "localNameColorB");
		SetupValue(localNameColor[3], 1.f, "visual", "localNameColorA");

		SetupValue(localBox, false, "visual", "localBox");
		SetupValue(localBoxColor[0], 1.f, "visual", "localBoxColorR"); 
		SetupValue(localBoxColor[1], 1.f, "visual", "localBoxColorG");
		SetupValue(localBoxColor[2], 1.f, "visual", "localBoxColorB");
		SetupValue(localBoxColor[3], 1.f, "visual", "localBoxColorA");

		SetupValue(localHealth, false, "visual", "localHealth");
		SetupValue(localHealthColor[0], 1.f, "visual", "localHealthColorR"); 
		SetupValue(localHealthColor[1], 1.f, "visual", "localHealthColorG");
		SetupValue(localHealthColor[2], 1.f, "visual", "localHealthColorB");
		SetupValue(localHealthColor[3], 1.f, "visual", "localHealthColorA");

		SetupValue(localHealthColorEnd[0], 1.f, "visual", "localHealthColorEndR");
		SetupValue(localHealthColorEnd[1], 1.f, "visual", "localHealthColorEndG");
		SetupValue(localHealthColorEnd[2], 1.f, "visual", "localHealthColorEndB");
		SetupValue(localHealthColorEnd[3], 1.f, "visual", "localHealthColorEndA");

		SetupValue(localGlow, false, "visual", "localGlow");
		SetupValue(localGlowColor[0], 0.137f, "visual", "localGlowColorR"); // (0.137f, 0.091f, 0.230f, 1.000f)
		SetupValue(localGlowColor[1], 0.091f, "visual", "localGlowColorG");
		SetupValue(localGlowColor[2], 0.230f, "visual", "localGlowColorB");
		SetupValue(localGlowColor[3], 1.f, "visual", "localGlowColorA");

		SetupValue(localArmor, false, "visual", "localArmor");
		SetupValue(localArmorColor[0], 1.f, "visual", "localArmorColorR"); // (1.000f, 0.832f, 0.000f, 1.000f)
		SetupValue(localArmorColor[1], 0.832f, "visual", "localArmorColorG");
		SetupValue(localArmorColor[2], 0.000f, "visual", "localArmorColorB");
		SetupValue(localArmorColor[3], 1.f, "visual", "localArmorColorA");

		SetupValue(localAmmo, false, "visual", "localAmmo");
		SetupValue(localAmmoColor[0], 1.f, "visual", "localAmmoColorR"); // (1.000f, 0.000f, 0.000f, 1.000f)
		SetupValue(localAmmoColor[1], 0.000f, "visual", "localAmmoColorG");
		SetupValue(localAmmoColor[2], 0.000f, "visual", "localAmmoColorB");
		SetupValue(localAmmoColor[3], 1.f, "visual", "localAmmoColorA");

		SetupValue(localMoney, false, "visual", "localMoney");
		SetupValue(localMoneyColor[0], 0.793f, "visual", "localMoneyColorR");
		SetupValue(localMoneyColor[1], 1.f, "visual", "localMoneyColorG");
		SetupValue(localMoneyColor[2], 0.000f, "visual", "localMoneyColorB");
		SetupValue(localMoneyColor[3], 1.f, "visual", "localMoneyColorA"); // (0.793f, 1.000f, 0.000f, 1.000f)
	}

	// model
	{
		using namespace cfg::model;

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

		SetupValue(aspectRatio, false, "misc", "aspectRatio");
		SetupValue(aspectRatioValue, 0, "misc", "aspectRatioValue");

		SetupValue(preserveKillfeed, false, "misc", "preserveKillfeed");
		SetupValue(bulletImpact, false, "misc", "bulletImpact");
		SetupValue(onlyCheatLogs, true, "misc", "onlyCheatLogs");

		SetupValue(bulletTracer, false, "misc", "bulletTracer");
		SetupValue(bulletTracerColor[0], 0.000f, "visual", "bulletTracerColorR"); // (0.000f, 0.246f, 0.646f, 1.000f)
		SetupValue(bulletTracerColor[1], 0.246f, "visual", "bulletTracerColorG");
		SetupValue(bulletTracerColor[2], 0.646f, "visual", "bulletTracerColorB");
		SetupValue(bulletTracerColor[3], 1.f, "visual", "bulletTracerColorA");

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
	}
}

void CConfig::SetupValue(int& value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue< int >(category, name, &value));
}

void CConfig::SetupValue(float& value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue< float >(category, name, &value));
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
}

void CConfig::Load(std::string ConfigName)
{
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