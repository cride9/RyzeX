#include "config.h"
#include "../Text.h"
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
#include <map>
#include <cmath>
#include <limits>
#include "gui.h"
#include "../../Features/Rage/aimbot.h"

void CConfig::Setup() {

	for (auto* value : ints) delete value;
	for (auto* value : bools) delete value;
	for (auto* value : floats) delete value;
	for (auto* value : strings) delete value;
	ints.clear();
	bools.clear();
	floats.clear();
	strings.clear();

	CreateMainDirectory();
	RefreshSounds();
	RefreshConfigs();

	// rage
	{
		using namespace cfg::rage;
		PushCategory(XorStr("RageBot"));

		SetupBool(bEnable, false, XorStr("bEnable"));
		SetupInt(iAimbotKey, 0, XorStr("iAimbotKey"));
		SetupInt(iAimbotFov, 180, XorStr("iAimbotFov"));
		SetupBool(bLimitScan, false, XorStr("bLimitScan"));

		SetupBool(bSilentAim, false, XorStr("bSilentAim"));

		SetupBool(bDoubletap, false, XorStr("bDoubletap"));
		SetupInt(iDoubletapKey, 0, XorStr("iDoubletapKey"));

		SetupBool(bHideshot, false, XorStr("bHideshot"));
		SetupInt(iHideShotKey, 0, XorStr("iHideShotKey"));

	SetupInt(iResolverMode, 0, XorStr("iResolverMode"));
		SetupBool(bOverride, false, XorStr("bOverride"));
		SetupInt(iOverrideBind, 0, XorStr("iOverrideBind"));

		SetupBool(bForceBaim, false, XorStr("bForceBaim"));
		SetupInt(iForceBaimKey, 0, XorStr("iForceBaimKey"));

		SetupBool(bAutoNade, false, XorStr("bAutoNade"));
		SetupInt(iAutoNadeBind, 0, XorStr("iAutoNadeBind"));
		SetupInt(iAutoNadeMinDmg, 0, XorStr("iAutoNadeMinDmg"));

		SetupBoolArray(bHitboxes[NORMAL][AUTO], 6, false, XorStr("bHitboxesNormalAuto"));
		SetupBoolArray(bHitboxes[MULTIPOINT][AUTO], 6, false, XorStr("bHitboxesMultipointAuto"));
		SetupBoolArray(bHitboxes[SAFE][AUTO], 6, false, XorStr("bHitboxesSafeAuto"));
		SetupBoolArray(bConditions[AUTO], 2, false, XorStr("bConditionsAuto"));

		SetupBoolArray(bHitboxes[NORMAL][SCOUT], 6, false, XorStr("bHitboxesNormalScout"));
		SetupBoolArray(bHitboxes[MULTIPOINT][SCOUT], 6, false, XorStr("bHitboxesMultipointScout"));
		SetupBoolArray(bHitboxes[SAFE][SCOUT], 6, false, XorStr("bHitboxesSafeScout"));
		SetupBoolArray(bConditions[SCOUT], 2, false, XorStr("bConditionsScout"));

		SetupBoolArray(bHitboxes[NORMAL][AWP], 6, false, XorStr("bHitboxesNormalAwp"));
		SetupBoolArray(bHitboxes[MULTIPOINT][AWP], 6, false, XorStr("bHitboxesMultipointAwp"));
		SetupBoolArray(bHitboxes[SAFE][AWP], 6, false, XorStr("bHitboxesSafeAwp"));
		SetupBoolArray(bConditions[AWP], 2, false, XorStr("bConditionsAwp"));

		SetupBoolArray(bHitboxes[NORMAL][PISTOL], 6, false, XorStr("bHitboxesNormalPistol"));
		SetupBoolArray(bHitboxes[MULTIPOINT][PISTOL], 6, false, XorStr("bHitboxesMultipointPistol"));
		SetupBoolArray(bHitboxes[SAFE][PISTOL], 6, false, XorStr("bHitboxesSafePistol"));
		SetupBoolArray(bConditions[PISTOL], 2, false, XorStr("bConditionsPistol"));

		SetupBoolArray(bHitboxes[NORMAL][HEAVY_PISTOL], 6, false, XorStr("bHitboxesNormalHeavyPistol"));
		SetupBoolArray(bHitboxes[MULTIPOINT][HEAVY_PISTOL], 6, false, XorStr("bHitboxesMultipointHeavyPistol"));
		SetupBoolArray(bHitboxes[SAFE][HEAVY_PISTOL], 6, false, XorStr("bHitboxesSafeHeavyPistol"));
		SetupBoolArray(bConditions[HEAVY_PISTOL], 2, false, XorStr("bConditionsHeavyPistol"));

		SetupBoolArray(bHitboxes[NORMAL][OTHER], 6, false, XorStr("bHitboxesNormalOther"));
		SetupBoolArray(bHitboxes[MULTIPOINT][OTHER], 6, false, XorStr("bHitboxesMultipointOther"));
		SetupBoolArray(bHitboxes[SAFE][OTHER], 6, false, XorStr("bHitboxesSafeOther"));
		SetupBoolArray(bConditions[OTHER], 2, false, XorStr("bConditionsOther"));

		SetupBoolArray(bHitboxes[NORMAL][ZEUS], 6, false, XorStr("bHitboxesNormalZeus"));
		SetupBoolArray(bHitboxes[MULTIPOINT][ZEUS], 6, false, XorStr("bHitboxesMultipointZeus"));
		SetupBoolArray(bHitboxes[SAFE][ZEUS], 6, false, XorStr("bHitboxesSafeZeus"));
		SetupBoolArray(bConditions[ZEUS], 2, false, XorStr("bConditionsZeus"));

		SetupIntArray(iHitchances, 7, 0, XorStr("iHitchances"));
		SetupIntArray(iAccuracyBoost, 7, 0, XorStr("iAccuracyBoost"));
		SetupIntArray(iMinDamages, 7, 0, XorStr("iMinDamages"));
		SetupIntArray(iHeadPoints, 7, 0, XorStr("iHeadPoints"));
		SetupIntArray(iBodyPoints, 7, 0, XorStr("iBodyPoints"));
		SetupIntArray(iOverride, 7, 0, XorStr("iOverride"));

		SetupBoolArray(bSafePoint, 7, false, XorStr("bSafePoint"));
		SetupBoolArray(bAutostop, 7, false, XorStr("bAutostop"));
		SetupIntArray(bAutostopAggressiveness, 7, false, XorStr("bAutostopAggressiveness"));
		SetupBoolArray(bAutoScope, 7, false, XorStr("bAutoScope"));
		SetupBoolArray(bSafePoint, 7, false, XorStr("bSafePoint"));
	}

	// antiaim
	{

		using namespace cfg::antiaim;
		PushCategory(XorStr("AntiAim"));

		SetupIntArray(iEnabledJitters.data(), 3, 0, XorStr("iEnabledJitters"));
		for (size_t i = 0; i < 3; i++) for (size_t j = 0; j < 64; j++)
		SetupInt(vecJitterWays[i][j], 0, std::format("iJitterWays{}{}", i, j));

		SetupBoolArray(bEnabled, 3, false, XorStr("bEnabled"));
		SetupIntArray(iPitch, 3, 0, XorStr("iPitch"));
		SetupIntArray(iYawBase, 3, 0, XorStr("iYawBase"));
		SetupIntArray(iYaw, 3, 0, XorStr("iYaw"));

		SetupBool(bManualAA, false, XorStr("bManualAA"));

		

		SetupBool(bManualAAR, false, XorStr("bManualAAR"));
		SetupBool(bManualAAL, false, XorStr("bManualAAL"));

		SetupInt(iManualAAL, 0, XorStr("iManualAAL"));
		SetupInt(iManualAAR, 0, XorStr("iManualAAR"));

		SetupBoolArray(bInvertOnShoot, 3, false, XorStr("bInvertOnShoot"));

		SetupBool(bSlideWalk, false, XorStr("bSlideWalk"));
		SetupBool(bInverter, false, XorStr("bInverter"));

		SetupIntArray(iDesyncType, 3, 0, XorStr("iDesyncType"));
		SetupInt(iInverterBind, 0, XorStr("iInverterBind"));

		SetupIntArray(iFlickOffset, 3, 0, XorStr("iFlickOffset"));
		SetupIntArray(flickAngleSwitch, 3, 0, XorStr("flickAngleSwitch"));

		SetupIntArray(iModifier, 3, 0, XorStr("iModifier"));
		SetupIntArray(iJitterValue, 3, 0, XorStr("iJitterValue"));
		SetupIntArray(iSpinSpeed, 3, 0, XorStr("iSpinSpeed"));
		SetupBoolArray(bAntiJitter, 3, false, XorStr("bAntiJitter"));

		SetupBool(bFakelag, false, XorStr("bFakelag"));
		SetupInt(iFakelag, 0, XorStr("iFakelag"));
		SetupInt(iFakelagMin, 0, XorStr("iFakelagMin"));
		SetupInt(iFakeLagMax, 0, XorStr("iFakeLagMax"));
		SetupInt(iFakeLagType, 0, XorStr("iFakeLagType"));

		SetupBool(bBreakLagcompensation, false, XorStr("bBreakLagcompensation"));
		SetupInt( iLCKey, 0, XorStr( "iLCButton" ) );

		SetupBool(bFakeWalk, false, XorStr("bFakeWalk"));
		SetupInt(iFakeWalkKey, 0, XorStr("iFakeWalkKey"));
		SetupFloat(iFakeWalkSpeed, 0.f, XorStr("iFakeWalkSpeed"));

		SetupBool(bFakeDuck, false, XorStr("bFakeDuck"));
		SetupInt(iFakeDuckKey, 0, XorStr("iFakeDuckKey"));

		SetupBool(bAutoPeek, false, XorStr("bAutoPeek"));
		SetupInt(iAutoPeek, 0, XorStr("iAutoPeek"));

		SetupIntArray(iFreestand, 3, 0, XorStr("iFreestand"));
	}

	// visual
	{
		using namespace cfg::visual;
		PushCategory(XorStr("Visual"));

		SetupBool(bOverrideFog, false, XorStr("bOverrideFog"));
		SetupInt(iFogStart, 0, XorStr("iFogStart"));
		SetupInt(iFogEnd, 0, XorStr("iFogEnd"));
		SetupColor(flFogColor, Color(0.836478f, 0.848819f, 1.000000f, 0.421384f), XorStr("flFogColor"));
	
		SetupBoolArray(bEnable, 3, false, XorStr("bEnable"));

		SetupBoolArray(bName, 3, false, XorStr("bName"));
		SetupBoolArray(bBox, 3, false, XorStr("bBox"));
		SetupBoolArray(bHealth, 3, false, XorStr("bHealth"));
		SetupBoolArray(bGlow, 3, false, XorStr("bGlow"));
		SetupBoolArray(bArmor, 3, false, XorStr("bArmor"));
		SetupBoolArray(bAmmo, 3, false, XorStr("bAmmo"));
		SetupBoolArray(bWeapon, 3, false, XorStr("bWeapon"));
		SetupBoolArray(bSkeleton, 3, false, XorStr("bSkeleton"));
		SetupBoolArray(bBulletTracer, 3, false, XorStr("bBulletTracer"));

		SetupColor(flNameColor[ENEMY], Color(1.f, 1.f, 1.f, 1.f), XorStr("flNameColorEnemy"));
		SetupColor(flBoxColor[ENEMY], Color(0.825561f, 0.811321f, 1.f, 1.f), XorStr("flBoxColorEnemy"));
		SetupColor(flHealthColorStart[ENEMY], Color(0.044000f, 1.f, 0.000000f, 1.f), XorStr("flHealthColorStartEnemy"));
		SetupColor(flHealthColorEnd[ENEMY], Color(0.044000f, 1.f, 0.000000, 1.f), XorStr("flHealthColorEndEnemy"));
		SetupColor(flGlowColor[ENEMY], Color(0.203242f, 0.185831f, 0.339623f, 1.f), XorStr("flGlowColorEnemy"));
		SetupColor(flArmorColor[ENEMY], Color(0.000000f, 0.924000f, 1.f, 1.f), XorStr("flArmorColorEnemy"));
		SetupColor(flAmmoColor[ENEMY], Color(0.345000f, 0.394434f, 1.f, 1.f), XorStr("flAmmoColorEnemy"));
		SetupColor(flWeaponColor[ENEMY], Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponColorEnemy"));
		SetupColor(flSkeletonColor[ENEMY], Color(1.f, 1.f, 1.f, 0.383648f), XorStr("flSkeletonColorEnemy"));
		SetupColor(flBulletTracerColor[ENEMY], Color(0.262252f, 0.323914f, 0.534591f, 0.452830f), XorStr("flBulletTracerColorEnemy"));
		SetupColor(flFlagsColor[ENEMY], 9, Color(255, 255, 255, 255), XorStr("flFlagsColorEnemy"));
		SetupBoolArray(bFlags[ENEMY], 9, false, XorStr("bFlagsEnemy"));

		SetupColor(flNameColor[TEAM], Color(1.f, 1.f, 1.f, 1.f), XorStr("flNameColorTeam"));
		SetupColor(flBoxColor[TEAM], Color(0.825561f, 0.811321f, 1.f, 1.f), XorStr("flBoxColorTeam"));
		SetupColor(flHealthColorStart[TEAM], Color(0.211000f, 1.f, 0.000000f, 1.000000f), XorStr("flHealthColorStartTeam"));
		SetupColor(flHealthColorEnd[TEAM], Color(0.211000f, 1.f, 0.000000, 1.f), XorStr("flHealthColorEndTeam"));
		SetupColor(flGlowColor[TEAM], Color(0.125000f, 0.272000f, 0.000000f, 1.f), XorStr("flGlowColorTeam"));
		SetupColor(flArmorColor[TEAM], Color(0.000000f, 1.000000f, 0.954000f, 1.000000f), XorStr("flArmorColorTeam"));
		SetupColor(flAmmoColor[TEAM], Color(0.000000f, 0.546000f, 1.f, 1.f), XorStr("flAmmoColorTeam"));
		SetupColor(flWeaponColor[TEAM], Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponColorTeam"));
		SetupColor(flSkeletonColor[TEAM], Color(1.f, 1.f, 1.f, 0.383648f), XorStr("flSkeletonColorTeam"));
		SetupColor(flBulletTracerColor[TEAM], Color(0.262252f, 0.323914f, 0.534591f, 0.452830f), XorStr("flBulletTracerColorTeam"));
		SetupColor(flFlagsColor[TEAM], 9, Color(1.f, 1.f, 1.f, 1.f), XorStr("flFlagsColorTeam"));
		SetupBoolArray(bFlags[TEAM], 9, false, XorStr("bFlagsTeam"));

		SetupColor(flNameColor[LOCAL], Color(1.f, 1.f, 1.f, 1.f), XorStr("flNameColorLocal"));
		SetupColor(flBoxColor[LOCAL], Color(0.825561f, 0.811321f, 1.f, 1.f), XorStr("flBoxColorLocal"));
		SetupColor(flHealthColorStart[LOCAL], Color(0.211000f, 1.f, 0.000000f, 1.000000f), XorStr("flHealthColorStartLocal"));
		SetupColor(flHealthColorEnd[LOCAL], Color(0.211000f, 1.f, 0.000000, 1.f), XorStr("flHealthColorEndLocal"));
		SetupColor(flGlowColor[LOCAL], Color(0.137000f, 0.091000f, 0.230000f, 1.f), XorStr("flGlowColorLocal"));
		SetupColor(flArmorColor[LOCAL], Color(0.000000f, 0.832000f, 0.000000f, 1.000000f), XorStr("flArmorColorLocal"));
		SetupColor(flAmmoColor[LOCAL], Color(0.000000f, 0.546000f, 1.f, 1.f), XorStr("flAmmoColorLocal"));
		SetupColor(flWeaponColor[LOCAL], Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponColorLocal"));
		SetupColor(flSkeletonColor[LOCAL], Color(1.f, 1.f, 1.f, 0.383648f), XorStr("flSkeletonColorLocal"));
		SetupColor(flBulletTracerColor[LOCAL], Color(0.262252f, 0.323914f, 0.534591f, 0.452830f), XorStr("flBulletTracerColorLocal"));
		SetupColor(flFlagsColor[LOCAL], 9, Color(1.f, 1.f, 1.f, 1.f), XorStr("flFlagsColorLocal"));
		SetupBoolArray(bFlags[LOCAL], 9, false, XorStr("bFlagsLocal"));
	}

	// models
	{
		using namespace cfg::model;
		PushCategory(XorStr("Models"));

		SetupIntArray(iAttachmentMaterial, 3, 0, XorStr("iAttachmentMaterial"));
		SetupIntArray(iType, 3, 0, XorStr("iType"));

		SetupBoolArray(bAttachment, 3, false, XorStr("bAttachment"));
		SetupBoolArray(bAttachmentXhair, 3, false, XorStr("bAttachmentXhair"));
		SetupColor(flAttachmentColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("flAttachmentColor"));

		SetupBoolArray(bAttachmentOverlay, 3, false, XorStr("bAttachmentOverlay"));
		SetupBoolArray(bAttachmentOverlayXhair, 3, false, XorStr("bAttachmentOverlayXhair"));
		SetupColor(bAttachmentOverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("bAttachmentOverlayColor"));

		SetupBoolArray(bAttachmentThinOverlay, 3, false, XorStr("bAttachmentThinOverlay"));
		SetupBoolArray(bAttachmentThinOverlayXhair, 3, false, XorStr("bAttachmentThinOverlayXhair"));
		SetupColor(bAttachmentThinOverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("bAttachmentThinOverlayColor"));

		SetupBoolArray(bAttachmentAnimatedOverlay, 3, false, XorStr("bAttachmentAnimatedOverlay"));
		SetupBoolArray(bAttachmentAnimatedOverlayXhair, 3, false, XorStr("bAttachmentAnimatedOverlayXhair"));
		SetupColor(bAttachmentAnimatedOverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("bAttachmentAnimatedOverlayColor"));

		SetupBoolArray(bChams, 3, false, XorStr("bChams"));
		SetupBoolArray(bChamsXQZ, 3, false, XorStr("bChamsXQZ"));
		SetupColor(ChamsColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("ChamsColor"));
		SetupColor(ChamsColorXQZ, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("ChamsColorXQZ"));
		SetupBoolArray(bXhair, 3, false, XorStr("bXhair"));
		SetupBoolArray(bXhairXQZ, 3, false, XorStr("bXhairXQZ"));

		SetupBoolArray(bOverlay, 3, false, XorStr("bOverlay"));
		SetupBoolArray(bOverlayXQZ, 3, false, XorStr("bOverlayXQZ"));
		SetupBoolArray(bOverlayXhair, 3, false, XorStr("bOverlayXhair"));
		SetupBoolArray(bOverlayXhairXQZ, 3, false, XorStr("bOverlayXhairXQZ"));
		SetupColor(OverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("OverlayColor"));
		SetupColor(OverlayColorXQZ, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("OverlayColorXQZ"));

		SetupBoolArray(bThinOverlay, 3, false, XorStr("bThinOverlay"));
		SetupBoolArray(bThinOverlayXQZ, 3, false, XorStr("bThinOverlayXQZ"));
		SetupColor(ThinOverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("ThinOverlayColor"));
		SetupColor(ThinOverlayColorXQZ, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("ThinOverlayColorXQZ"));
		SetupBoolArray(bThinOverlayXhair, 3, false, XorStr("bThinOverlayXhair"));
		SetupBoolArray(bThinOverlayXhairXQZ, 3, false, XorStr("bThinOverlayXhairXQZ"));

		SetupBoolArray(bAnimOverlay, 3, false, XorStr("bAnimOverlay"));
		SetupBoolArray(bAnimOverlayXQZ, 3, false, XorStr("bAnimOverlayXQZ"));
		SetupColor(AnimOverlayColor, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("AnimOverlayColor"));
		SetupColor(AnimOverlayColorXQZ, 3, Color(1.f, 1.f, 1.f, 1.f), XorStr("AnimOverlayColorXQZ"));
		SetupBoolArray(bAnimOverlayXhair, 3, false, XorStr("bAnimOverlayXhair"));
		SetupBoolArray(bAnimOverlayXhairXQZ, 3, false, XorStr("bAnimOverlayXhairXQZ"));

		SetupInt(iBacktrackType, 0, XorStr("iBacktrackType"));
		SetupColor(flBacktrackColor, Color(1.f, 0.f, 0.f, 0.5f), XorStr("flBacktrackColor"));
		SetupBool(bBacktrackXhair, false, XorStr("bBacktrackXhair"));
		SetupBool(bBacktrackChams, false, XorStr("bBacktrackChams"));

		SetupBool(bBlend, false, XorStr("bBlend"));
		SetupFloat(flBlend, 0.f, XorStr("flBlend"));

		SetupBool(bAutopeek, false, XorStr("bAutopeek"));
		SetupColor(flAutopeekColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flAutopeekColor"));

		SetupBool(bDesyncChams, false, XorStr("bDesyncChams"));
		SetupBool(bDesyncChamsXhair, false, XorStr("bDesyncChamsXhair"));
		SetupBool(bDesyncSkeleton, false, XorStr("bDesyncSkeleton"));
		SetupInt(bDesyncType, 0, XorStr("bDesyncType"));
		SetupColor(flDesyncChamsCol, Color(1.f, 1.f, 1.f, 0.99f), XorStr("flDesyncChamsCol"));

		SetupBool(bViewmodel, false, XorStr("bViewmodel"));

		SetupInt(iViewmodelType, 0, XorStr("iViewmodelType"));
		SetupColor(flViewmodelColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flViewmodelColor"));
		SetupBool(bViewmodelXhair, false, XorStr("bViewmodelXhair"));

		SetupBool(bViewmodelOverlay, 0, XorStr("iViewmodelOverlay"));
		SetupColor(flViewmodelOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flViewmodelOverlayColor"));
		SetupBool(bViewmodelOverlayXhair, false, XorStr("bViewmodelOverlayXhair"));

		SetupBool(bViewmodelThinOverlay, 0, XorStr("bViewmodelThinOverlay"));
		SetupColor(flViewmodelThinOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flViewmodelThinOverlayColor"));
		SetupBool(bViewmodelThinOverlayXhair, false, XorStr("bViewmodelThinOverlayXhair"));

		SetupBool(bViewmodelAnimOverlay, 0, XorStr("bViewmodelAnimOverlay"));
		SetupColor(flViewmodelAnimOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flViewmodelAnimOverlayColor"));
		SetupBool(bViewmodelAnimOverlayXhair, false, XorStr("bViewmodelAnimOverlayXhair"));

		SetupBool(bSleeve, false, XorStr("bSleeve"));

		SetupInt(iSleeveType, 0, XorStr("iSleeveType"));
		SetupColor(flSleeveColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flSleeveColor"));
		SetupBool(bSleeveXhair, false, XorStr("bSleeveXhair"));

		SetupBool(bSleeveOverlay, 0, XorStr("iSleeveOverlay"));
		SetupColor(flSleeveOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flSleeveOverlayColor"));
		SetupBool(bSleeveOverlayXhair, false, XorStr("bSleeveOverlayXhair"));

		SetupBool(bSleeveThinOverlay, 0, XorStr("bSleeveThinOverlay"));
		SetupColor(flSleeveThinOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flSleeveThinOverlayColor"));
		SetupBool(bSleeveThinOverlayXhair, false, XorStr("bSleeveThinOverlayXhair"));

		SetupBool(bSleeveAnimOverlay, 0, XorStr("bSleeveAnimOverlay"));
		SetupColor(flSleeveAnimOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flSleeveAnimOverlayColor"));
		SetupBool(bSleeveAnimOverlayXhair, false, XorStr("bSleeveAnimOverlayXhair"));

		SetupBool(bWeapon, false, XorStr("bWeapon"));

		SetupInt(iWeaponType, 0, XorStr("iWeaponType"));
		SetupColor(flWeaponColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponColor"));
		SetupBool(bWeaponXhair, false, XorStr("bWeaponXhair"));

		SetupBool(bWeaponOverlay, 0, XorStr("bWeaponOverlay"));
		SetupColor(flWeaponOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponOverlayColor"));
		SetupBool(bWeaponOverlayXhair, false, XorStr("bWeaponOverlayXhair"));

		SetupBool(bWeaponThinOverlay, 0, XorStr("bWeaponThinOverlay"));
		SetupColor(bWeaponThinOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("bWeaponThinOverlayColor"));
		SetupBool(bWeaponThinOverlayXhair, false, XorStr("bWeaponThinOverlayXhair"));

		SetupBool(bWeaponAnimOverlay, 0, XorStr("bWeaponAnimOverlay"));
		SetupColor(flWeaponAnimOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flWeaponAnimOverlayColor"));
		SetupBool(flWeaponAnimOverlayXhair, false, XorStr("flWeaponAnimOverlayXhair"));

		SetupBool(bProps, false, XorStr("bProps"));

		SetupInt(iPropsType, 0, XorStr("iPropsType"));
		SetupColor(flPropsColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flPropsColor"));
		SetupBool(bPropsXhair, false, XorStr("bPropsXhair"));

		SetupBool(bPropsOverlay, 0, XorStr("iPropsOverlay"));
		SetupColor(flPropsOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flPropsOverlayColor"));
		SetupBool(bPropsOverlayXhair, false, XorStr("bPropsOverlayXhair"));

		SetupBool(bPropsThinOverlay, 0, XorStr("bPropsThinOverlay"));
		SetupColor(flPropsThinOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flPropsThinOverlayColor"));
		SetupBool(bPropsThinOverlayXhair, false, XorStr("bPropsThinOverlayXhair"));

		SetupBool(bPropsAnimOverlay, 0, XorStr("bPropsAnimOverlay"));
		SetupColor(flPropsAnimOverlayColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flPropsAnimOverlayColor"));
		SetupBool(bPropsAnimOverlayXhair, false, XorStr("bPropsAnimOverlayXhair"));

		SetupBool(bPaperMode, false, XorStr("bPaperMode"));
	}

	// misc
	{
		using namespace cfg::misc;
		PushCategory(XorStr("Miscellaneous"));

		SetupBool(bMotionBlur, false, XorStr("bMotionBlur"));
		SetupBool(bForwardEnabled, false, XorStr("bForwardEnabled"));
		SetupFloat(flFallingIntensity, 1.f, XorStr("flFallingIntensity"));
		SetupFloat(flRotationIntensity, 1.f, XorStr("flRotationIntensity"));
		SetupFloat(flStrength, 4.f, XorStr("flStrength"));

		SetupBool(bDrawCapsule, false, XorStr("bDrawCapsule"));
		SetupColor(flDrawCapsuleColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flDrawCapsuleColor"));
		SetupColor(flDrawCapsuleColorHit, Color(1.f, 1.f, 1.f, 1.f), XorStr("flDrawCapsuleColorHit"));

		SetupBool(bNadePrediction, false, XorStr("bNadePrediction"));
		

		SetupBool(bKeyBindList, false, XorStr("bKeyBindList"));
		SetupBoolArray(iKeyBindList, 13, false, XorStr("iKeyBindList"));
		SetupBool(bSpectatorList, false, "bSpectatorList");
		SetupFloat(flHudScale, 1.f, "flHudScale");
		float keyPosition[2] = { .02f, .5f }, spectatorPosition[2] = { .8f, .3f };
		SetupFloatArray(flKeybindPosition, 2, keyPosition, "flKeybindPosition");
		SetupFloatArray(flSpectatorPosition, 2, spectatorPosition, "flSpectatorPosition");

		SetupBool(bBunnyHop, false, XorStr("bBunnyHop"));
		SetupBool(bAutoStrafe, false, XorStr("bAutoStrafe"));
		SetupBool(bFastStop, false, XorStr("bFastStop"));
		SetupBool(bInfiniteDuck, false, XorStr("bInfiniteDuck"));
		SetupBool(bBlockbot, false, XorStr("bBlockbot"));
		SetupInt(iBlockbotKey, 0, XorStr("iBlockbotKey"));
		SetupBool(bClantag, false, XorStr("bClantag"));
		SetupString(szCustomClantagBuffer, "", XorStr("szCustomClantagBuffer"));
		SetupBool(bInvertKnife, false, XorStr("bInvertKnife"));

		SetupBool(bDrawServerHitbox, false, XorStr("bDrawServerHitbox"));
		SetupBool(bDrawServerHitboxOnAllEntities, false, XorStr("bDrawServerHitboxOnAllEntities"));

		SetupBool(bNightmode, false, XorStr("bNightmode"));
		SetupColor(flNightmodeColor, Color(255, 255, 255, 255), XorStr("flNightmodeColor"));
		SetupColor(flPropColor, Color(255, 255, 255, 255), XorStr("flPropColor"));

		SetupInt(iSkybox, 0, XorStr("iSkybox"));
		SetupColor(flSkyboxColor, Color(255, 255, 255, 255), XorStr("flSkyboxColor"));

		SetupBool(bOverrideLampColors, false, XorStr("bOverrideLampColors"));
		SetupColor(flLampColors, Color(1.f, 1.f, 1.f, 1.f), XorStr("flLampColors"));
		SetupInt(iFlicker, 0, XorStr("iFlicker"));

		SetupBool(bEnableRadio, false, XorStr("bEnableRadio"));
		SetupInt(iRadioStation, 0, XorStr("iRadioStation"));
		SetupFloat(flRadioVolume, 0.f, XorStr("flRadioVolume"));
		SetupInt(iRadioMuteHotKey, 0, XorStr("iRadioMuteHotKey"));
		SetupString(szCustomRadioUrl, "", "szCustomRadioUrl");
		SetupStringArray(szRadioFavoriteNames, 4, "", "szRadioFavoriteNames");
		SetupStringArray(szRadioFavoriteUrls, 4, "", "szRadioFavoriteUrls");

		SetupBool(bOOF, false, XorStr("bOOF"));
		SetupColor(flOOFColor, Color(0.355999f, 0.343815f, 0.666667f, 1.f), XorStr("flOOFColor"));
		SetupInt(iOOFDistance, 34, XorStr("iOOFDistance"));
		SetupInt(iOOFSize, 19, XorStr("iOOFSize"));

		SetupBool(bAspectRatio, false, XorStr("bAspectRatio"));
		SetupInt(iAspectRatio, 0, XorStr("iAspectRatio"));

		SetupBool(bPreserveKillfeed, false, XorStr("bPreserveKillfeed"));

		SetupBool(bThirdPerson, false, XorStr("bThirdPerson"));
		SetupInt(iThirdPersonKey, 0, XorStr("iThirdPersonKey"));
		SetupInt(iThirdPersonDistance, 80, XorStr("iThirdPersonDistance"));

		SetupBool(bBulletImpact, false, XorStr("bBulletImpact"));
		SetupColor(flImpactColor[0], Color(1.000000f, 0.000000f, 0.000000f, 0.45f), XorStr("flImpactColor0"));
		SetupColor(flImpactColor[1], Color(0.000000f, 0.113208f, 1.000000f, 0.45f), XorStr("flImpactColor1"));

		SetupBool(bDroppedWeaponESP, false, XorStr("bDroppedWeaponESP"));
		SetupColor(flDroppedWeaponESP, Color(1.f, 1.f, 1.f, 1.f), XorStr("flDroppedWeaponESP"));
		SetupBool(bDroppedWeaponIcon, false, "bDroppedWeaponIcon");
		SetupBool(bDroppedWeaponAmmo, false, "bDroppedWeaponAmmo");
		SetupBool(bWorldEspDistance, false, "bWorldEspDistance");
		SetupInt(iWorldEspMaxDistance, 0, "iWorldEspMaxDistance");

		SetupBool(bProjectileESP, false, XorStr("bProjectileESP"));
		SetupColor(flProjectileESP, Color(1.f, 1.f, 1.f, 1.f), XorStr("flProjectileESP"));
		SetupBool(bGrenadeTimers, false, "bGrenadeTimers");

		SetupBool(bOnlyCheatlog, false, XorStr("bOnlyCheatlog"));

		SetupBool(bWorldCrosshair, false, XorStr("bWorldCrosshair"));
		SetupColor(flWorldCrosshairColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flWorldCrosshairColor"));

		SetupBool(bDamageIndicator, false, XorStr("bDamageIndicator"));
		SetupColor(flDamageIndicator, Color(1.f, 1.f, 1.f, 1.f), XorStr("flDamageIndicator"));
		SetupColor(flDamageHeadshot, Color(255, 0, 0, 255), "flDamageHeadshot");
		SetupFloat(flDamageLifetime, 1.f, "flDamageLifetime");
		SetupFloat(flDamageRise, 18.f, "flDamageRise");
		SetupInt(iDamageFontSize, 20, "iDamageFontSize");
		SetupBool(bMergeDamage, false, "bMergeDamage");

		SetupInt(iViewModelFov, 60, XorStr("iViewModelFov"));
		SetupInt(iDebugFov, 90, XorStr("iDebugFov"));

		SetupBoolArray(bRemovals, 7, false, XorStr("bRemovals"));
		SetupBool(bOnScopeViewmodel, false, XorStr("bOnScopeViewmodel"));
		SetupColor(flScopeColor, Color(1.f, 1.f, 1.f, 1.f), XorStr("flScopeColor"));
		SetupColor(flScopeColorEnd, Color(1.f, 1.f, 1.f, 1.f), XorStr("flScopeColorEnd"));
		SetupInt(iScopeLength, 5, XorStr("iScopeLength"));

		SetupBool(bAutobuy, false, XorStr("bAutobuy"));
		SetupInt(iPistols, 0, XorStr("iPistols"));
		SetupInt(iSnipers, 0, XorStr("iSnipers"));
		SetupInt(iRifles, 0, "iRifles");
		SetupBool(bSeparateAutobuy, false, "bSeparateAutobuy");
		SetupIntArray(iTeamPistols, 2, 0, "iTeamPistols");
		SetupIntArray(iTeamRifles, 2, 0, "iTeamRifles");
		SetupIntArray(iTeamSnipers, 2, 0, "iTeamSnipers");
		for (int team = 0; team < 2; ++team) {
			SetupBoolArray(bTeamEquipments[team], 3, false, "bTeamEquipments" + std::to_string(team));
			SetupBoolArray(bTeamGrenades[team], 5, false, "bTeamGrenades" + std::to_string(team));
		}
		SetupInt(iAutobuyReserve, 0, "iAutobuyReserve");
		SetupBool(bAutobuyKeepWeapons, false, "bAutobuyKeepWeapons");
		SetupBoolArray(bEquipments, 3, false, XorStr("bEquipments"));
		SetupBoolArray(bGrenades, 5,  false, XorStr("bGrenades"));

		SetupBool(bFakePing, false, XorStr("bFakePing"));
		SetupFloat(flFakePingFactor, 200.f, XorStr("flFakePingFactor"));

		SetupBool(bRussianRoulette, false, XorStr("bRussianRoulette"));

		SetupInt(iHitSound, 0, XorStr("iHitSound"));
		SetupFloat(flHitSoundVolume, 100.f, XorStr("flHitSoundVolume"));
		SetupString(szWavPath, "", XorStr("szWavPath"));

		SetupBool(bKillsay, false, XorStr("bKillsay"));
		SetupString(szBombBuffer, "RyzeXTR", XorStr("szBombBuffer"));
		SetupString(szKillsayBuffer, "Ryze to the Top!", XorStr("szKillsayBuffer"));

		SetupBool(bSkinnyBoy, false, XorStr("bSkinnyBoy"));
		SetupInt(iSkinnyBoy, 100, XorStr("iSkinnyBoy"));

		SetupBool(bHat, false, XorStr("bHat"));
		SetupColor(flHat, Color(1.f, 1.f, 1.f, 1.f), XorStr("flHat"));
		SetupBool(bCustomHud, false, XorStr("bCustomHud"));

		SetupIntArray(cfg::m_iKeyStates, 256, 0, XorStr("KeyStates"));
		SetupBool( bSmoothFix, true, XorStr( "bSmoothFix" ) );
	}

	// skins
	{
		using namespace cfg::skin;
		PushCategory(XorStr("Skins"));

		SetupBool(bEnableSkinChanger, false, XorStr("bEnableSkinChanger"));
		SetupBool(bFilterByWeapon, false, XorStr("bFilterByWeapon"));
		SetupBoolArray(bModifySkinColors, 37, false, XorStr("bModifySkinColors"));

		SetupIntArray(iSkinId, 37, 0, XorStr("iSkinId"));
		SetupIntArray(iSeed, 37, 0, XorStr("iSeed"));
		SetupIntArray(iQuality, 37, 0, XorStr("iQuality"));
		SetupIntArray(iSkinStattrak, 37, 0, XorStr("iSkinStattrak"));
		SetupInt(iGloveModel, 0, XorStr("iGloveModel"));
		SetupInt(iKnifeModel, 0, XorStr("iKnifeModel"));

		SetupFloatArray(flSkinWear, 37, 0.f, XorStr("flSkinWear"));

		SetupColor(colSkins1, 37, Color(1.f, 1.f, 1.f, 1.f), XorStr("colSkins1"));
		SetupColor(colSkins2, 37, Color(1.f, 1.f, 1.f, 1.f), XorStr("colSkins2"));
		SetupColor(colSkins3, 37, Color(1.f, 1.f, 1.f, 1.f), XorStr("colSkins3"));
		SetupColor(colSkins4, 37, Color(1.f, 1.f, 1.f, 1.f), XorStr("colSkins4"));

		SetupStringArray(szSkinNametag, 37, "", XorStr("szSkinNametag"));

	}

	{
		using namespace cfg::vizualizer;
		PushCategory(XorStr("Vizualizer"));

		SetupBool(bEnabled, false, XorStr("bEnabled"));
		SetupBool(bDynamic, true, XorStr("bDynamic"));
		SetupFloat(flStaticThreshold, 0.55f, XorStr("flStaticThreshold"));
		SetupFloat(flSensitivity, 0.00f, XorStr("flSensitivity"));
	}

	{
		auto& s = cfg::radioVisualizer;
		const radio_visualizer::Settings defaults;
		PushCategory("RadioVisualizer");
		SetupBool(s.enabled, defaults.enabled, "Enabled");
		SetupInt(s.preset, defaults.preset, "Preset");
		SetupInt(s.quality, defaults.quality, "Quality");
		SetupFloat(s.size, defaults.size, "Size");
		SetupFloat(s.position[0], defaults.position[0], "PositionX");
		SetupFloat(s.position[1], defaults.position[1], "PositionY");
		SetupFloat(s.opacity, defaults.opacity, "Opacity");
		SetupColor(s.colorA, Color(defaults.colorA[0], defaults.colorA[1], defaults.colorA[2], defaults.colorA[3]), "GradientStart");
		SetupColor(s.colorB, Color(defaults.colorB[0], defaults.colorB[1], defaults.colorB[2], defaults.colorB[3]), "GradientEnd");
		SetupColor(s.logoColor, Color(defaults.logoColor[0], defaults.logoColor[1], defaults.logoColor[2], defaults.logoColor[3]), "LogoColor");
		SetupFloat(s.sensitivity, defaults.sensitivity, "Sensitivity");
		SetupFloat(s.smoothing, defaults.smoothing, "Smoothing");
		SetupFloat(s.bassReaction, defaults.bassReaction, "BassReaction");
		SetupBool(s.logoPulse, defaults.logoPulse, "LogoPulse");
		SetupBool(s.glow, defaults.glow, "Glow");
		SetupFloat(s.glowStrength, defaults.glowStrength, "GlowStrength");
		SetupBool(s.rings, defaults.rings, "Rings");
		SetupFloat(s.ringStrength, defaults.ringStrength, "RingStrength");
		SetupBool(s.particles, defaults.particles, "Particles");
		SetupFloat(s.particleDensity, defaults.particleDensity, "ParticleDensity");
	}

	// scripts
	{
		PushCategory( XorStr( "Scripts" ) );
		// Lua clears its transfer vectors after loading; registered values need their own lifetime.
		storedScripts = cfg::vecScripts;
		storedScriptVariables = cfg::vecScriptVariables;
		SetupStringArray(storedScripts.data(), storedScripts.size(), XorStr(""), XorStr("szScripts"));

		for (auto& variable : storedScriptVariables)
		{
			switch (variable.nType)
			{
				case EVARIABLETYPES::VARIABLES_BOOL:
				{
					auto& pObject = std::any_cast<BoolObject_t&>(variable.pObject);
					SetupBool( pObject.bValue, false, variable.strName );
					break;
				}
				case EVARIABLETYPES::VARIABLES_BOOL_ARRAY:
				{
					auto& pObject = std::any_cast<BoolArrayObject_t&>(variable.pObject);
					for (size_t i = 0u; i < pObject.deqValues.size( ); i++)
						SetupBool( pObject.deqValues[ i ], false, variable.strName + std::to_string( i ) );
					break;
				}
				case EVARIABLETYPES::VARIABLES_INT:
				{
					auto& pObject = std::any_cast<IntObject_t&>(variable.pObject);
					SetupInt( pObject.iValue, 0, variable.strName );
					break;
				}
				case EVARIABLETYPES::VARIABLES_FLOAT:
				{
					auto& pObject = std::any_cast<FloatObject_t&>(variable.pObject);
					SetupFloat( pObject.flValue, 0.0f, variable.strName );
					break;
				}
				case EVARIABLETYPES::VARIABLES_COLOR:
				{
					auto& pObject = std::any_cast<ColorObject_t&>(variable.pObject);
					SetupColor( pObject.flValue, Color( 1.f, 1.f, 1.f, 1.f ), variable.strName );
					break;
				}
				default:
					break;
			}
		}
	}

	bInitialized = true;
}

void CConfig::SetupColor(float colColor[4], Color colDefault, std::string szName) {

	for (size_t i = 0; i < 4; i++) {

		colColor[i] = bInitialized ? colColor[i] : (colDefault[i] / 255.f);
		floats.push_back(new ConfigValue<float>(szCategory, std::format("{}{}", szName, i == 0 ? "R" : i == 1 ? "G" : i == 2 ? "B" : "A"), &colColor[i]));
	}
}

void CConfig::SetupColor(float (*colColor)[4], int iLength, Color colDefault, std::string szName) {

	for (size_t j = 0; j < iLength; j++)
	{
		for (size_t i = 0; i < 4; i++) {

			colColor[j][i] = bInitialized ? colColor[j][i] : (colDefault[i] / 255.f);
			floats.push_back(new ConfigValue<float>(szCategory, std::format("{}{}{}", szName, j, i == 0 ? "R" : i == 1 ? "G" : i == 2 ? "B" : "A"), &colColor[j][i]));
		}
	}
}

void CConfig::SetupIntArray(int* arrArray, int iLength, int* iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault[i];
		ints.push_back(new ConfigValue<int>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupIntArray(int* arrArray, int iLength, int iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault;
		ints.push_back(new ConfigValue<int>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupFloatArray(float* arrArray, int iLength, float* iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault[i];
		floats.push_back(new ConfigValue<float>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupFloatArray(float* arrArray, int iLength, float iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault;
		floats.push_back(new ConfigValue<float>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupBoolArray(bool* arrArray, int iLength, bool* iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault[i];
		bools.push_back(new ConfigValue<bool>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupBoolArray(bool* arrArray, int iLength, bool iDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		arrArray[i] = bInitialized ? arrArray[i] : iDefault;
		bools.push_back(new ConfigValue<bool>(szCategory, std::format("{}{}", szName, i), &arrArray[i]));
	}
}

void CConfig::SetupInt(int& iValue, int iDefault, std::string szName) {

	iValue = bInitialized ? iValue : iDefault;
	ints.push_back(new ConfigValue<int>(szCategory, szName, &iValue));
}

void CConfig::SetupFloat(float& iValue, float flDefault, std::string szName) {

	iValue = bInitialized ? iValue : flDefault;
	floats.push_back(new ConfigValue<float>(szCategory, szName, &iValue));
}

void CConfig::SetupBool(bool& bValue, bool bDefault, std::string szName) {

	bValue = bInitialized ? bValue : bDefault;
	bools.push_back(new ConfigValue<bool>(szCategory, szName, &bValue));
}

void CConfig::SetupString(std::string& szValue, std::string szDefault, std::string szName) {

	szValue = bInitialized ? szValue : szDefault;
	strings.push_back(new ConfigValue<std::string>(szCategory, szName, &szValue));
}

void CConfig::SetupStringArray(std::string* szValue, int iLength, std::string szDefault, std::string szName) {

	for (size_t i = 0; i < iLength; i++) {

		szValue[i] = bInitialized ? szValue[i] : szDefault;
		strings.push_back(new ConfigValue<std::string>(szCategory, std::format( "{}{}", szName, i ), &szValue[i]));
	}
}

void CConfig::CreateMainDirectory( )
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;
	if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) )
	{
		folder = std::string(path) + "\\ryzextr\\";
	}
	
	if (CreateDirectory(folder.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
		printf( "Succesfully created directory" );
	else
		throw std::runtime_error( "Failed to create config directory" );
}

void CConfig::Save(std::string ConfigName) {
	if (bSaving || ConfigName.empty() || ConfigName.find_first_of("\\/:*?\"<>|\r\n") != std::string::npos)
		return;
	bSaving = true;
	try {
		if (bInitialized)
			Setup();
		TCHAR path[MAX_PATH]{};
		if (FAILED(SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, path)))
			throw std::runtime_error("Config folder unavailable");
		const std::string folder = std::string(path) + "\\ryzextr\\";
		const std::string file = folder + ConfigName + ".xtr";
		CreateDirectoryA(folder.c_str(), nullptr);
		// Serialize on the caller's thread so rendering cannot race a detached writer.
		std::map<std::string, std::map<std::string, std::string>> sections;
		for (const auto* value : ints) sections[value->category][value->name] = std::to_string(*value->value);
		for (const auto* value : floats) sections[value->category][value->name] = std::to_string(*value->value);
		for (const auto* value : bools) sections[value->category][value->name] = *value->value ? "true" : "false";
		for (const auto* value : strings) sections[value->category][value->name] = *value->value;
		const std::string temporary = file + ".tmp";
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		for (const auto& [category, entries] : sections) {
			output << "[" << category << "]\r\n";
			for (const auto& [name, text] : entries) {
				std::string line = text;
				std::replace(line.begin(), line.end(), '\r', ' ');
				std::replace(line.begin(), line.end(), '\n', ' ');
				output << name << "=" << line << "\r\n";
			}
			output << "\r\n";
		}
		output.close();
		if (!output)
			throw std::runtime_error("Could not write config");
		WritePrivateProfileStringA(nullptr, nullptr, nullptr, file.c_str());
		if (!MoveFileExA(temporary.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			throw std::runtime_error("Could not replace config");
		WritePrivateProfileStringA(nullptr, nullptr, nullptr, file.c_str());
		RefreshConfigs();
	} catch (const std::exception& error) {
		util::Print(error.what());
	}
	bSaving = false;
}

void CConfig::Load(std::string ConfigName)
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
	const DWORD attributes = GetFileAttributesA(file.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		bSaving = false;
		return;
	}

	char value_l[32] = { '\0' };
	char value_string[256] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), value_l, 32, file.c_str());
		char* end = nullptr;
		const auto parsed = std::strtoll(value_l, &end, 10);
		if (end != value_l && *end == '\0' && parsed >= (std::numeric_limits<int>::min)() && parsed <= (std::numeric_limits<int>::max)())
			*value->value = static_cast<int>(parsed);
	}

	for (auto value : floats)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), value_l, 32, file.c_str());
		char* end = nullptr;
		const float parsed = std::strtof(value_l, &end);
		if (end != value_l && *end == '\0' && std::isfinite(parsed))
			*value->value = parsed;
	}

	for (auto value : bools)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for ( auto value : strings )
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), value->value->c_str(), value_string, 256, file.c_str());
		*value->value = value_string;
		std::size_t limit = 255;
		if (value->name == "szCustomClantagBuffer" || value->name == "szKillsayBuffer") limit = 31;
		if (value->name == "szBombBuffer") limit = 11;
		if (value->value->size() > limit)
            value->value->resize(text::BoundedUtf8Length(*value->value, limit));
	}

	char resolverMode[32]{};
	char legacyResolver[32]{};
	GetPrivateProfileStringA("RageBot", "iResolverMode", "", resolverMode, sizeof(resolverMode), file.c_str());
	GetPrivateProfileStringA("RageBot", "bResolver", "", legacyResolver, sizeof(legacyResolver), file.c_str());
	cfg::rage::iResolverMode = resolver::LoadMode(resolverMode, legacyResolver, cfg::rage::iResolverMode);
	radio_visualizer::Sanitize(cfg::radioVisualizer);

	cfg::vecScripts = storedScripts;
	cfg::vecScriptVariables = storedScriptVariables;
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
CConfig* pConfig = new CConfig();
