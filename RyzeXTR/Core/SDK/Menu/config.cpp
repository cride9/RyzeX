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

		SetupValue(doubletap, false, "rage", "Ragedoubletap");
		SetupValue(doubletapkey, 0, "rage", "Ragedoubletapkey");

		SetupValue(resolver, false, "rage", "Rageresolver");

		SetupValue(etcHitboxes, false, 6, "rage", "etcHitboxes");
		SetupValue(etcHitchance, 0, "rage", "etcHitchance");
		SetupValue(etcMindmg, 0, "rage", "etcMindmg");
		SetupValue(etcHeadPoints, 0, "rage", "etcHeadPoints");
		SetupValue(etcBodyPoints, 0, "rage", "etcBodyPoints");

		SetupValue(autoHitboxes, false, 6, "rage", "autoHitboxes");
		SetupValue(autoHitchance, 0, "rage", "autoHitchance");
		SetupValue(autoMindmg, 0, "rage", "autoMindmg");
		SetupValue(autoHeadPoints, 0, "rage", "autoHeadPoints");
		SetupValue(autoBodyPoints, 0, "rage", "autoBodyPoints");

		SetupValue(scoutHitboxes, false, 6, "rage", "scoutHitboxes");
		SetupValue(scoutHitchance, 0, "rage", "scoutHitchance");
		SetupValue(scoutMindmg, 0, "rage", "scoutMindmg");
		SetupValue(scoutHeadPoints, 0, "rage", "scoutHeadPoints");
		SetupValue(scoutBodyPoints, 0, "rage", "scoutBodyPoints");

		SetupValue(awpHitboxes, false, 6, "rage", "awpHitboxes");
		SetupValue(awpHitchance, 0, "rage", "awpHitchance");
		SetupValue(awpMindmg, 0, "rage", "awpMindmg");
		SetupValue(awpHeadPoints, 0, "rage", "awpHeadPoints");
		SetupValue(awpBodyPoints, 0, "rage", "awpBodyPoints");

		SetupValue(pistolHitboxes, false, 6, "rage", "pistolHitboxes");
		SetupValue(pistolHitchance, 0, "rage", "pistolHitchance");
		SetupValue(pistolMindmg, 0, "rage", "pistolMindmg");
		SetupValue(pistolHeadPoints, 0, "rage", "pistolHeadPoints");
		SetupValue(pistolBodyPoints, 0, "rage", "pistolBodyPoints");

		SetupValue(heavypistolHitboxes, false, 6, "rage", "heavypistolHitboxes");
		SetupValue(heavypistolHitchance, 0, "rage", "heavypistolHitchance");
		SetupValue(heavypistolMindmg, 0, "rage", "heavypistolMindmg");
		SetupValue(heavypistolHeadPoints, 0, "rage", "heavypistolHeadPoints");
		SetupValue(heavypistolBodyPoints, 0, "rage", "heavypistolBodyPoints");

	}
	
	// antiaim
	{
		using namespace cfg::antiaim;

		SetupValue(enabled, false, "antiaim", "Antiaimenabled");

		SetupValue(pitch, 0, "antiaim", "pitch");
		SetupValue(yaw, 0, "antiaim", "yaw");
		SetupValue(atTarget, false, "antiaim", "atTarget");

		SetupValue(jittervalue, 0, "antiaim", "jittervalue");
		SetupValue(invertangle, 0, "antiaim", "invertangle");

		SetupValue(desynctype, 0, "antiaim", "desynctype");
		SetupValue(desyncvalue, 0, "antiaim", "desyncvalue");
		SetupValue(desyncinverter, 0, "antiaim", "desyncinverter");

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
	}

	// visual
	{
		using namespace cfg::visual;

		// enemy
		SetupValue(enemyEsp, false, "visual", "enemyEsp");

		SetupValue(enemyName, false, "visual", "enemyName");
		SetupValue(enemyNameColor[0], 1.f, "visual", "enemyNameColorR");
		SetupValue(enemyNameColor[1], 1.f, "visual", "enemyNameColorG");
		SetupValue(enemyNameColor[2], 1.f, "visual", "enemyNameColorB");
		SetupValue(enemyNameColor[3], 1.f, "visual", "enemyNameColorA");

		SetupValue(enemyBox, false, "visual", "enemyBox");
		SetupValue(enemyBoxColor[0], 1.f, "visual", "enemyBoxColorR");
		SetupValue(enemyBoxColor[1], 1.f, "visual", "enemyBoxColorG");
		SetupValue(enemyBoxColor[2], 1.f, "visual", "enemyBoxColorB");
		SetupValue(enemyBoxColor[3], 1.f, "visual", "enemyBoxColorA");

		SetupValue(enemyHealth, false, "visual", "enemyHealth");
		SetupValue(enemyHealthColor[0], 1.f, "visual", "enemyHealthColorR");
		SetupValue(enemyHealthColor[1], 1.f, "visual", "enemyHealthColorG");
		SetupValue(enemyHealthColor[2], 1.f, "visual", "enemyHealthColorB");
		SetupValue(enemyHealthColor[3], 1.f, "visual", "enemyHealthColorA");

		SetupValue(enemyGlow, false, "visual", "enemyGlow");
		SetupValue(enemyGlowColor[0], 1.f, "visual", "enemyGlowColorR");
		SetupValue(enemyGlowColor[1], 1.f, "visual", "enemyGlowColorG");
		SetupValue(enemyGlowColor[2], 1.f, "visual", "enemyGlowColorB");
		SetupValue(enemyGlowColor[3], 1.f, "visual", "enemyGlowColorA");

		SetupValue(enemyArmor, false, "visual", "enemyArmor");
		SetupValue(enemyArmorColor[0], 1.f, "visual", "enemyArmorColorR");
		SetupValue(enemyArmorColor[1], 1.f, "visual", "enemyArmorColorG");
		SetupValue(enemyArmorColor[2], 1.f, "visual", "enemyArmorColorB");
		SetupValue(enemyArmorColor[3], 1.f, "visual", "enemyArmorColorA");

		SetupValue(enemyAmmo, false, "visual", "enemyAmmo");
		SetupValue(enemyAmmoColor[0], 1.f, "visual", "enemyAmmoColorR");
		SetupValue(enemyAmmoColor[1], 1.f, "visual", "enemyAmmoColorG");
		SetupValue(enemyAmmoColor[2], 1.f, "visual", "enemyAmmoColorB");
		SetupValue(enemyAmmoColor[3], 1.f, "visual", "enemyAmmoColorA");

		SetupValue(enemyWeapon, false, "visual", "enemyWeapon");
		SetupValue(enemyWeaponColor[0], 1.f, "visual", "enemyWeaponColorR");
		SetupValue(enemyWeaponColor[1], 1.f, "visual", "enemyWeaponColorG");
		SetupValue(enemyWeaponColor[2], 1.f, "visual", "enemyWeaponColorB");
		SetupValue(enemyWeaponColor[3], 1.f, "visual", "enemyWeaponColorA");

		SetupValue(enemyMoney, false, "visual", "enemyMoney");
		SetupValue(enemyMoneyColor[0], 1.f, "visual", "enemyMoneyColorR");
		SetupValue(enemyMoneyColor[1], 1.f, "visual", "enemyMoneyColorG");
		SetupValue(enemyMoneyColor[2], 1.f, "visual", "enemyMoneyColorB");
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
		SetupValue(teamHealthColor[0], 1.f, "visual", "teamHealthColorR");
		SetupValue(teamHealthColor[1], 1.f, "visual", "teamHealthColorG");
		SetupValue(teamHealthColor[2], 1.f, "visual", "teamHealthColorB");
		SetupValue(teamHealthColor[3], 1.f, "visual", "teamHealthColorA");

		SetupValue(teamGlow, false, "visual", "teamGlow");
		SetupValue(teamGlowColor[0], 1.f, "visual", "teamGlowColorR");
		SetupValue(teamGlowColor[1], 1.f, "visual", "teamGlowColorG");
		SetupValue(teamGlowColor[2], 1.f, "visual", "teamGlowColorB");
		SetupValue(teamGlowColor[3], 1.f, "visual", "teamGlowColorA");

		SetupValue(teamArmor, false, "visual", "teamArmor");
		SetupValue(teamArmorColor[0], 1.f, "visual", "teamArmorColorR");
		SetupValue(teamArmorColor[1], 1.f, "visual", "teamArmorColorG");
		SetupValue(teamArmorColor[2], 1.f, "visual", "teamArmorColorB");
		SetupValue(teamArmorColor[3], 1.f, "visual", "teamArmorColorA");

		SetupValue(teamAmmo, false, "visual", "teamAmmo");
		SetupValue(teamAmmoColor[0], 1.f, "visual", "teamAmmoColorR");
		SetupValue(teamAmmoColor[1], 1.f, "visual", "teamAmmoColorG");
		SetupValue(teamAmmoColor[2], 1.f, "visual", "teamAmmoColorB");
		SetupValue(teamAmmoColor[3], 1.f, "visual", "teamAmmoColorA");

		SetupValue(teamMoney, false, "visual", "teamMoney");
		SetupValue(teamMoneyColor[0], 1.f, "visual", "teamMoneyColorR");
		SetupValue(teamMoneyColor[1], 1.f, "visual", "teamMoneyColorG");
		SetupValue(teamMoneyColor[2], 1.f, "visual", "teamMoneyColorB");
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

		SetupValue(localGlow, false, "visual", "localGlow");
		SetupValue(localGlowColor[0], 1.f, "visual", "localGlowColorR");
		SetupValue(localGlowColor[1], 1.f, "visual", "localGlowColorG");
		SetupValue(localGlowColor[2], 1.f, "visual", "localGlowColorB");
		SetupValue(localGlowColor[3], 1.f, "visual", "localGlowColorA");

		SetupValue(localArmor, false, "visual", "localArmor");
		SetupValue(localArmorColor[0], 1.f, "visual", "localArmorColorR");
		SetupValue(localArmorColor[1], 1.f, "visual", "localArmorColorG");
		SetupValue(localArmorColor[2], 1.f, "visual", "localArmorColorB");
		SetupValue(localArmorColor[3], 1.f, "visual", "localArmorColorA");

		SetupValue(localAmmo, false, "visual", "localAmmo");
		SetupValue(localAmmoColor[0], 1.f, "visual", "localAmmoColorR");
		SetupValue(localAmmoColor[1], 1.f, "visual", "localAmmoColorG");
		SetupValue(localAmmoColor[2], 1.f, "visual", "localAmmoColorB");
		SetupValue(localAmmoColor[3], 1.f, "visual", "localAmmoColorA");

		SetupValue(localMoney, false, "visual", "localMoney");
		SetupValue(localMoneyColor[0], 1.f, "visual", "localMoneyColorR");
		SetupValue(localMoneyColor[1], 1.f, "visual", "localMoneyColorG");
		SetupValue(localMoneyColor[2], 1.f, "visual", "localMoneyColorB");
		SetupValue(localMoneyColor[3], 1.f, "visual", "localMoneyColorA");
	}

	// model
	{
		using namespace cfg::model;

		SetupValue(enemyType, 0, "model", "enemyType");

		// chams
		SetupValue(enemy, false, "model", "enemy");
		SetupValue(enemyXQZ, false, "model", "enemyXQZ");

		SetupValue(enemyColor[0], 1.f, "model", "enemyColorR");
		SetupValue(enemyColor[1], 1.f, "model", "enemyColorG");
		SetupValue(enemyColor[2], 1.f, "model", "enemyColorB");
		SetupValue(enemyColor[3], 1.f, "model", "enemyColorA");

		SetupValue(enemyXQZColor[0], 1.f, "model", "enemyXQZColorR");
		SetupValue(enemyXQZColor[1], 1.f, "model", "enemyXQZColorG");
		SetupValue(enemyXQZColor[2], 1.f, "model", "enemyXQZColorB");
		SetupValue(enemyXQZColor[3], 1.f, "model", "enemyXQZColorA");

		SetupValue(enemyXhair, false, "model", "enemyXhair");
		SetupValue(enemyXQZXhair, false, "model", "enemyXQZXhair");

		// overlay
		SetupValue(enemyOverlay, false, "model", "enemyOverlay");
		SetupValue(enemyOverlayXQZ, false, "model", "enemyOverlayXQZ");

		SetupValue(enemyOverlayColor[0], 1.f, "model", "enemyOverlayColorR");
		SetupValue(enemyOverlayColor[1], 1.f, "model", "enemyOverlayColorG");
		SetupValue(enemyOverlayColor[2], 1.f, "model", "enemyOverlayColorB");
		SetupValue(enemyOverlayColor[3], 1.f, "model", "enemyOverlayColorA");

		SetupValue(enemyOverlayXQZColor[0], 1.f, "model", "enemyOverlayXQZColorR");
		SetupValue(enemyOverlayXQZColor[1], 1.f, "model", "enemyOverlayXQZColorG");
		SetupValue(enemyOverlayXQZColor[2], 1.f, "model", "enemyOverlayXQZColorB");
		SetupValue(enemyOverlayXQZColor[3], 1.f, "model", "enemyOverlayXQZColorA");

		SetupValue(enemyOverlayXhair, false, "model", "enemyOverlayXhair");
		SetupValue(enemyOverlayXQZXhair, false, "model", "enemyOverlayXQZXhair");

		// glow
		SetupValue(enemyThinOverlay, false, "model", "enemyThinOverlay");
		SetupValue(enemyThinOverlayXQZ, false, "model", "enemyThinOverlayXQZ");

		SetupValue(enemyThinOverlayColor[0], 1.f, "model", "enemyThinOverlayColorR");
		SetupValue(enemyThinOverlayColor[1], 1.f, "model", "enemyThinOverlayColorG");
		SetupValue(enemyThinOverlayColor[2], 1.f, "model", "enemyThinOverlayColorB");
		SetupValue(enemyThinOverlayColor[3], 1.f, "model", "enemyThinOverlayColorA");

		SetupValue(enemyThinOverlayXQZColor[0], 1.f, "model", "enemyThinOverlayXQZColorR");
		SetupValue(enemyThinOverlayXQZColor[1], 1.f, "model", "enemyThinOverlayXQZColorG");
		SetupValue(enemyThinOverlayXQZColor[2], 1.f, "model", "enemyThinOverlayXQZColorB");
		SetupValue(enemyThinOverlayXQZColor[3], 1.f, "model", "enemyThinOverlayXQZColorA");

		SetupValue(enemyThinOverlayXhair, false, "model", "enemyThinOverlayXhair");
		SetupValue(enemyThinOverlayXQZXhair, false, "model", "enemyThinOverlayXQZXhair");

		// animated
		SetupValue(enemyAnimOverlay, false, "model", "enemyAnimOverlay");
		SetupValue(enemyAnimOverlayXQZ, false, "model", "enemyAnimOverlayXQZ");

		SetupValue(enemyAnimOverlayColor[0], 1.f, "model", "enemyAnimOverlayColorR");
		SetupValue(enemyAnimOverlayColor[1], 1.f, "model", "enemyAnimOverlayColorG");
		SetupValue(enemyAnimOverlayColor[2], 1.f, "model", "enemyAnimOverlayColorB");
		SetupValue(enemyAnimOverlayColor[3], 1.f, "model", "enemyAnimOverlayColorA");

		SetupValue(enemyAnimOverlayXQZColor[0], 1.f, "model", "enemyAnimOverlayXQZColorR");
		SetupValue(enemyAnimOverlayXQZColor[1], 1.f, "model", "enemyAnimOverlayXQZColorG");
		SetupValue(enemyAnimOverlayXQZColor[2], 1.f, "model", "enemyAnimOverlayXQZColorB");
		SetupValue(enemyAnimOverlayXQZColor[3], 1.f, "model", "enemyAnimOverlayXQZColorA");

		SetupValue(enemyAnimOverlayXhair, false, "model", "enemyAnimOverlayXhair");
		SetupValue(enemyAnimOverlayXQZXhair, false, "model", "enemyAnimOverlayXQZXhair");

		// teammate
		SetupValue(teamType, 0, "model", "teamType");

		// chams
		SetupValue(team, false, "model", "team");
		SetupValue(teamXQZ, false, "model", "teamXQZ");

		SetupValue(teamColor[0], 1.f, "model", "teamColorR");
		SetupValue(teamColor[1], 1.f, "model", "teamColorG");
		SetupValue(teamColor[2], 1.f, "model", "teamColorB");
		SetupValue(teamColor[3], 1.f, "model", "teamColorA");

		SetupValue(teamXQZColor[0], 1.f, "model", "teamXQZColorR");
		SetupValue(teamXQZColor[1], 1.f, "model", "teamXQZColorG");
		SetupValue(teamXQZColor[2], 1.f, "model", "teamXQZColorB");
		SetupValue(teamXQZColor[3], 1.f, "model", "teamXQZColorA");

		SetupValue(teamXhair, false, "model", "teamXhair");
		SetupValue(teamXQZXhair, false, "model", "teamXQZXhair");

		// overlay
		SetupValue(teamOverlay, false, "model", "teamOverlay");
		SetupValue(teamOverlayXQZ, false, "model", "teamOverlayXQZ");

		SetupValue(teamOverlayColor[0], 1.f, "model", "teamOverlayColorR");
		SetupValue(teamOverlayColor[1], 1.f, "model", "teamOverlayColorG");
		SetupValue(teamOverlayColor[2], 1.f, "model", "teamOverlayColorB");
		SetupValue(teamOverlayColor[3], 1.f, "model", "teamOverlayColorA");

		SetupValue(teamOverlayXQZColor[0], 1.f, "model", "teamOverlayXQZColorR");
		SetupValue(teamOverlayXQZColor[1], 1.f, "model", "teamOverlayXQZColorG");
		SetupValue(teamOverlayXQZColor[2], 1.f, "model", "teamOverlayXQZColorB");
		SetupValue(teamOverlayXQZColor[3], 1.f, "model", "teamOverlayXQZColorA");

		SetupValue(teamOverlayXhair, false, "model", "teamOverlayXhair");
		SetupValue(teamOverlayXQZXhair, false, "model", "teamOverlayXQZXhair");

		// glow
		SetupValue(teamThinOverlay, false, "model", "teamThinOverlay");
		SetupValue(teamThinOverlayXQZ, false, "model", "teamThinOverlayXQZ");

		SetupValue(teamThinOverlayColor[0], 1.f, "model", "teamThinOverlayColorR");
		SetupValue(teamThinOverlayColor[1], 1.f, "model", "teamThinOverlayColorG");
		SetupValue(teamThinOverlayColor[2], 1.f, "model", "teamThinOverlayColorB");
		SetupValue(teamThinOverlayColor[3], 1.f, "model", "teamThinOverlayColorA");

		SetupValue(teamThinOverlayXQZColor[0], 1.f, "model", "teamThinOverlayXQZColorR");
		SetupValue(teamThinOverlayXQZColor[1], 1.f, "model", "teamThinOverlayXQZColorG");
		SetupValue(teamThinOverlayXQZColor[2], 1.f, "model", "teamThinOverlayXQZColorB");
		SetupValue(teamThinOverlayXQZColor[3], 1.f, "model", "teamThinOverlayXQZColorA");

		SetupValue(teamThinOverlayXhair, false, "model", "teamThinOverlayXhair");
		SetupValue(teamThinOverlayXQZXhair, false, "model", "teamThinOverlayXQZXhair");

		// animated
		SetupValue(teamAnimOverlay, false, "model", "teamAnimOverlay");
		SetupValue(teamAnimOverlayXQZ, false, "model", "teamAnimOverlayXQZ");

		SetupValue(teamAnimOverlayColor[0], 1.f, "model", "teamAnimOverlayColorR");
		SetupValue(teamAnimOverlayColor[1], 1.f, "model", "teamAnimOverlayColorG");
		SetupValue(teamAnimOverlayColor[2], 1.f, "model", "teamAnimOverlayColorB");
		SetupValue(teamAnimOverlayColor[3], 1.f, "model", "teamAnimOverlayColorA");

		SetupValue(teamAnimOverlayXQZColor[0], 1.f, "model", "teamAnimOverlayXQZColorR");
		SetupValue(teamAnimOverlayXQZColor[1], 1.f, "model", "teamAnimOverlayXQZColorG");
		SetupValue(teamAnimOverlayXQZColor[2], 1.f, "model", "teamAnimOverlayXQZColorB");
		SetupValue(teamAnimOverlayXQZColor[3], 1.f, "model", "teamAnimOverlayXQZColorA");

		SetupValue(teamAnimOverlayXhair, false, "model", "teamAnimOverlayXhair");
		SetupValue(teamAnimOverlayXQZXhair, false, "model", "teamAnimOverlayXQZXhair");

		// local
		SetupValue(teamType, 0, "model", "teamType");

		// chams
		SetupValue(team, false, "model", "team");
		SetupValue(teamXQZ, false, "model", "teamXQZ");

		SetupValue(teamColor[0], 1.f, "model", "teamColorR");
		SetupValue(teamColor[1], 1.f, "model", "teamColorG");
		SetupValue(teamColor[2], 1.f, "model", "teamColorB");
		SetupValue(teamColor[3], 1.f, "model", "teamColorA");

		SetupValue(teamXQZColor[0], 1.f, "model", "teamXQZColorR");
		SetupValue(teamXQZColor[1], 1.f, "model", "teamXQZColorG");
		SetupValue(teamXQZColor[2], 1.f, "model", "teamXQZColorB");
		SetupValue(teamXQZColor[3], 1.f, "model", "teamXQZColorA");

		SetupValue(teamXhair, false, "model", "teamXhair");
		SetupValue(teamXQZXhair, false, "model", "teamXQZXhair");

		// overlay
		SetupValue(teamOverlay, false, "model", "teamOverlay");
		SetupValue(teamOverlayXQZ, false, "model", "teamOverlayXQZ");

		SetupValue(teamOverlayColor[0], 1.f, "model", "teamOverlayColorR");
		SetupValue(teamOverlayColor[1], 1.f, "model", "teamOverlayColorG");
		SetupValue(teamOverlayColor[2], 1.f, "model", "teamOverlayColorB");
		SetupValue(teamOverlayColor[3], 1.f, "model", "teamOverlayColorA");

		SetupValue(teamOverlayXQZColor[0], 1.f, "model", "teamOverlayXQZColorR");
		SetupValue(teamOverlayXQZColor[1], 1.f, "model", "teamOverlayXQZColorG");
		SetupValue(teamOverlayXQZColor[2], 1.f, "model", "teamOverlayXQZColorB");
		SetupValue(teamOverlayXQZColor[3], 1.f, "model", "teamOverlayXQZColorA");

		SetupValue(teamOverlayXhair, false, "model", "teamOverlayXhair");
		SetupValue(teamOverlayXQZXhair, false, "model", "teamOverlayXQZXhair");

		// glow
		SetupValue(localThinOverlay, false, "model", "localThinOverlay");
		SetupValue(localThinOverlayXQZ, false, "model", "localThinOverlayXQZ");

		SetupValue(localThinOverlayColor[0], 1.f, "model", "localThinOverlayColorR");
		SetupValue(localThinOverlayColor[1], 1.f, "model", "localThinOverlayColorG");
		SetupValue(localThinOverlayColor[2], 1.f, "model", "localThinOverlayColorB");
		SetupValue(localThinOverlayColor[3], 1.f, "model", "localThinOverlayColorA");

		SetupValue(localThinOverlayXQZColor[0], 1.f, "model", "localThinOverlayXQZColorR");
		SetupValue(localThinOverlayXQZColor[1], 1.f, "model", "localThinOverlayXQZColorG");
		SetupValue(localThinOverlayXQZColor[2], 1.f, "model", "localThinOverlayXQZColorB");
		SetupValue(localThinOverlayXQZColor[3], 1.f, "model", "localThinOverlayXQZColorA");

		SetupValue(localThinOverlayXhair, false, "model", "localThinOverlayXhair");
		SetupValue(localThinOverlayXQZXhair, false, "model", "localThinOverlayXQZXhair");

		// animated
		SetupValue(localAnimOverlay, false, "model", "localAnimOverlay");
		SetupValue(localAnimOverlayXQZ, false, "model", "localAnimOverlayXQZ");

		SetupValue(localAnimOverlayColor[0], 1.f, "model", "localAnimOverlayColorR");
		SetupValue(localAnimOverlayColor[1], 1.f, "model", "localAnimOverlayColorG");
		SetupValue(localAnimOverlayColor[2], 1.f, "model", "localAnimOverlayColorB");
		SetupValue(localAnimOverlayColor[3], 1.f, "model", "localAnimOverlayColorA");

		SetupValue(localAnimOverlayXQZColor[0], 1.f, "model", "localAnimOverlayXQZColorR");
		SetupValue(localAnimOverlayXQZColor[1], 1.f, "model", "localAnimOverlayXQZColorG");
		SetupValue(localAnimOverlayXQZColor[2], 1.f, "model", "localAnimOverlayXQZColorB");
		SetupValue(localAnimOverlayXQZColor[3], 1.f, "model", "localAnimOverlayXQZColorA");

		SetupValue(localAnimOverlayXhair, false, "model", "localAnimOverlayXhair");
		SetupValue(localAnimOverlayXQZXhair, false, "model", "localAnimOverlayXQZXhair");

		// ideal tick
		SetupValue(localIdealTick, false, "model", "localIdealTick");

		SetupValue(localIdealTickColor[0], 1.f, "model", "localIdealTickColorR");
		SetupValue(localIdealTickColor[1], 1.f, "model", "localIdealTickColorG");
		SetupValue(localIdealTickColor[2], 1.f, "model", "localIdealTickColorB");
		SetupValue(localIdealTickColor[3], 1.f, "model", "localIdealTickColorA");

		SetupValue(localIdealTickColor2[0], 1.f, "model", "localIdealTickColor2R");
		SetupValue(localIdealTickColor2[1], 1.f, "model", "localIdealTickColor2G");
		SetupValue(localIdealTickColor2[2], 1.f, "model", "localIdealTickColor2B");
		SetupValue(localIdealTickColor2[3], 1.f, "model", "localIdealTickColor2A");
	}

	// misc 
	{
		using namespace cfg::misc;

		SetupValue(bunnyhop, false, "misc", "bunnyhop");
		SetupValue(autoStrafe, false, "misc", "autoStrafe");
		SetupValue(faststop, false, "misc", "faststop");

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

		SetupValue(thirdperson, false, "misc", "thirdperson");
		SetupValue(thirdpersonbind, 0, "misc", "thirdpersonbind");

		SetupValue(viewmodelFov, 75, "misc", "viewmodelFov");
		SetupValue(fov, 90, "misc", "fov");

		SetupValue(removals, false, 4, "misc", "removals");

		SetupValue(pistols, 0, "misc", "autobuypistol");
		SetupValue(snipers, 0, "misc", "autobuyrifles");
		SetupValue(equipments, false, 3, "misc", "autobuyequipments");
		SetupValue(grenades, false, 5, "misc", "autobuygrenades");
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
}

CConfig* Config2 = new CConfig();