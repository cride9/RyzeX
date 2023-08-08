#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <mutex>
#include "../DataTyes/Color.h"

namespace cfg {

	inline bool bDoUnload = false;

	inline int configID = -1;

	inline bool debugSwitch = false;
	inline int debugSlider1 = 1;
	inline int debugSlider2 = 1;
	inline int debugSlider3 = 1;
	inline int debugSlider4 = 1;
	inline bool debugSwitch2 = false;
	inline float debugColorPicker[4] = {1.f, 1.f, 1.f, 1.f};

	inline int m_iKeyStates[ 256 ] = { 0 };

	namespace rage {

		inline bool bEnable;
		inline int iAimbotKey = 0;
		inline float iAimbotFov = 180.f;
		inline bool bSilentAim = false;

		inline bool bDoubletap;
		inline int  iDoubletapKey;
		inline bool bHideshot;
		inline int  iHideShotKey;
		inline bool bResolver;
		inline bool bOverride;
		inline int	iOverrideBind;

		inline bool bForceBaim;
		inline int iForceBaimKey = 0;

		// all weapon
		inline bool bHitboxes[3][7][6];

		inline int iHitchances[7];
		inline int iMinDamages[7];
		inline int iHeadPoints[7];
		inline int iBodyPoints[7];
		inline int iOverride[7];

		inline bool bSafePoint[7];
		inline bool bAutostop[7];
		inline int	bAutostopAggressiveness[7];
		inline bool bConditions[6][2];
		inline bool bAutoScope[] = { false, false, false };
	}
	namespace antiaim {

		inline std::array<int, 3> iEnabledJitters;
		inline std::array<std::array<int, 64>, 3> vecJitterWays;

		inline bool bEnabled[3];
		inline int iPitch[3];
		inline int iYawBase[3];
		inline int iYaw[3];
		inline bool bSlideWalk;
		inline bool bInvertOnShoot[3];
		inline bool bInverter = false;

		inline int iDesyncType[3];
		inline int iInverterBind = 0;

		inline float flBodyLean[2][3];

		inline int iFlickOffset[3];
		inline int flickAngleSwitch[3];

		inline bool bSwayDesync[3];

		inline int iModifier[3];
		inline int iJitterValue[3];
		inline float iInvertAngle[3];
		inline bool bAntiJitter[3];

		inline bool bFakelag = false;
		inline int iFakelag = 0;
		inline int iFakelagMin = 0;
		inline int iFakeLagMax = 0;
		inline int iFakeLagType = 0;

		inline bool bDefensive = false;

		inline bool bFakeWalk = false;
		inline int iFakeWalkKey = 0;
		inline float iFakeWalkSpeed = 0;
		inline bool bFakeDuck = false;
		inline int iFakeDuckKey = 0;

		inline bool bAutoPeek = false;
		inline int iAutoPeek = 0;

		inline int iFreestand[3];
	}
	namespace visual {

		inline bool bOverrideFog = false;
		inline int iFogStart = 200;
		inline int iFogEnd = 1500;
		inline float flFogColor[3] = { 1.f, 1.f, 1.f };

		inline bool bEnable[3] = { false, false, false };

		inline bool bName[3] = { false, false ,false };
		inline float flNameColor[3][4];

		inline bool bBox[3] = { false, false, false };
		inline float flBoxColor[3][4];

		inline bool bHealth[3] = { false, false, false };
		inline float flHealthColorStart[3][4];
		inline float flHealthColorEnd[3][4];

		inline bool bGlow[3] = { false, false ,false };
		inline float flGlowColor[3][4];

		inline bool bArmor[3] = { false, false, false };
		inline float flArmorColor[3][4];

		inline bool bAmmo[3] = { false, false, false };
		inline float flAmmoColor[3][4];

		inline bool bWeapon[3] = { false, false, false };
		inline float flWeaponColor[3][4];

		inline bool bSkeleton[3] = { false, false, false };
		inline float flSkeletonColor[3][4];

		inline bool bBulletTracer[3] = { false, false, false };
		inline float flBulletTracerColor[3][4];

		inline bool bFlags[3][9];
		inline float flFlagsColor[3][9][4];
	}
	namespace model {

		inline int attachmentChamsMaterial[3];

		inline bool attachmentChams[3]{ false };
		inline bool attachmentChamsXhair[3];
		inline float attachmentChamsColor[3][4];

		inline bool attachmentOverlay[3]{ false };
		inline bool attachmentOverlayXhair[3]{ false };
		inline float attachmentOverlayColor[3][4];

		inline bool attachmentThinOverlay[3]{ false };
		inline bool attachmentThinOverlayXhair[3]{ false };
		inline float attachmentThinOverlayColor[3][4];

		inline bool attachmentAnimatedOverlay[3]{ false };
		inline bool attachmentAnimatedOverlayXhair[3]{ false };
		inline float attachmentAnimatedOverlayColor[3][4];

		inline int enemyBTType = 0;
		inline float enemyBTColor[4] = { 1.f, 1.f, 1.f, 1.f};
		inline bool enemyBTXhair = false;
		inline bool enemyBTEnable = false;

		inline bool bBlend;
		inline float flBlend;

		inline int iType[3];
		inline bool bChams[3];
		inline bool bChamsXQZ[3];
		inline float ChamsColor[3][4];
		inline float ChamsColorXQZ[3][4];
		inline bool bXhair[3];
		inline bool bXhairXQZ[3];

		inline bool bOverlay[3];
		inline bool bOverlayXQZ[3];
		inline float OverlayColor[3][4];
		inline float OverlayColorXQZ[3][4];
		inline bool bOverlayXhair[3];
		inline bool bOverlayXhairXQZ[3];

		inline bool bThinOverlay[3];
		inline bool bThinOverlayXQZ[3];
		inline float ThinOverlayColor[3][4];
		inline float ThinOverlayColorXQZ[3][4];
		inline bool bThinOverlayXhair[3];
		inline bool bThinOverlayXhairXQZ[3];

		inline bool bAnimOverlay[3];
		inline bool bAnimOverlayXQZ[3];
		inline float AnimOverlayColor[3][4];
		inline float AnimOverlayColorXQZ[3][4];
		inline bool bAnimOverlayXhair[3];
		inline bool bAnimOverlayXhairXQZ[3];

		inline bool localIdealTick = false;
		inline float localIdealTickColor[4] = { 255.f, 255.f, 255.f, 255.f };

		inline bool localDesync = false;
		inline bool localDesyncXhair = false;
		inline bool localDesyncSkeleton = false;
		inline int localDesyncType = 0;
		inline float localDesyncColor[4] = { 255.f, 255.f, 255.f, 255.f };

		inline bool viewmodel = false;

		inline int viewmodelType = 0;
		inline float viewmodelColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool viewmodelXhair = false;

		inline bool viewmodelOverlay = false;
		inline float viewmodelOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool viewmodelOverlayXhair = false;

		inline bool viewmodelThinOverlay = false;
		inline float viewmodelThinOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool viewmodelThinOverlayXhair = false;

		inline bool viewmodelAnimOverlay = false;
		inline float viewmodelAnimOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool viewmodelAnimOverlayXhair = false;

		inline bool weapon = false;

		inline int weaponType = 0;
		inline float weaponColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool weaponXhair = false;

		inline bool weaponOverlay = false;
		inline float weaponOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool weaponOverlayXhair = false;

		inline bool weaponThinOverlay = false;
		inline float weaponThinOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool weaponThinOverlayXhair = false;

		inline bool weaponAnimOverlay = false;
		inline float weaponAnimOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool weaponAnimOverlayXhair = false;

		inline bool paperMode = false;
	}
	namespace misc {

		inline bool bDrawCapsule = false; // 
		inline float flDrawCapsuleColor[4] = {1.f, 1.f, 1.f, 1.f};
		inline float flDrawCapsuleColorHit[4] = { 1.f, 1.f, 1.f, 1.f };

		//inline bool bKeyBindList = false;
		inline bool bKeyBindList = false;
		inline bool iKeyBindList[10];

		inline bool bBunnyHop = false;//
		inline bool bAutoStrafe = false;
		inline bool bFastStop = false;
		inline bool bInfiniteDuck = false;
		inline bool bBlockbot = false;
		inline int iBlockbotKey = 0;
		inline bool bClantag = false;
		inline bool bInvertKnife = false;//

		inline bool bDrawServerHitbox = false;
		inline bool bDrawServerHitboxOnAllEntities = false;

		inline bool bNightmode = false; //
		inline float flNightmodeColor[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float flPropColor[4] = { 1.f, 1.f, 1.f, 1.f };

		inline int iSkybox = 0; // 
		inline float flSkyboxColor[4] = { 1.f, 1.f, 1.f, 1.f };

		inline bool bOverrideLampColors = false; //
		inline float flLampColors[4] = { 1.f, 1.f, 1.f, 1.f };
		inline int iFlicker = 0;

		inline bool bEnableRadio = false; // 
		inline int iRadioStation = 0;
		inline float flRadioVolume = 0.f;
		inline int iRadioMuteHotKey = 0;

		inline bool bOOF = false; //
		inline float flOOF[4];
		inline int iOOFDistance;
		inline int iOOFSize;

		inline bool bAspectRatio = false; //
		inline int iAspectRatio = 0; 

		inline bool bPreserveKillfeed = false;

		inline bool bThirdPerson = false; // 
		inline int iThirdPersonKey = 0;
		inline int iThirdPersonDistance = 0;

		inline bool bBulletImpact = false; //
		inline float flImpactColor[2][4];

		inline bool bDroppedWeaponESP = false; //
		inline float flDroppedWeaponESP[4];

		inline bool bProjectileESP = false; //
		inline float flProjectileESP[4];

		inline bool bOnlyCheatlog = false; //

		inline bool bWorldCrosshair = false; //
		inline float flWorldCrosshairColor[4] = { 1.f ,1.f, 1.f, 1.f };

		inline int iViewModelFov = 75; //
		inline int iDebugFov = 90; //

		inline bool bRemovals[] = { false, false ,false ,false, false, false }; //
		inline bool bOnScopeViewmodel = false; //
		inline float flScopeColor[4] = { 1.f, 1.f, 1.f, 1.f }; //
		inline float flScopeColorEnd[4] = { 1.f, 1.f, 1.f, 1.f }; //
		inline int iScopeLength = 1; //

		inline bool bAutobuy = false; //
		inline int iPistols;
		inline int iSnipers;
		inline bool bEquipments[] = { false, false, false };
		inline bool bGrenades[] = { false, false, false, false, false };

		inline bool bFakePing = false; //
		inline float flFakePingFactor = 0.f;

		// funny times
		inline bool bRussianRoulette = false;

		// hitsounds
		inline int iHitSound = 0; //
		inline float flHitSoundVolume = 100.f; //
		inline std::string szWavPath = ""; //

		inline bool bKillsay = false; //
		inline std::string szBombBuffer = "RyzeXTR"; //
		inline std::string szKillsayBuffer = ""; //

		inline bool bSkinnyBoy = false;
		inline int iSkinnyBoy = 0;

		inline bool bHat = false; //
		inline float flHat[4] = { 1.f, 1.f, 1.f, 1.f }; //
		inline bool bCustomHud = false;
	
	}
	namespace skin {

		inline bool bEnableSkinChanger = false;
		inline bool bFilterByWeapon = true;
		inline bool bModifySkinColors[37];

		inline int  iSkinId[37];
		inline int  iSeed[37];
		inline int  iQuality[37];
		inline int  iGloveModel;
		inline int  iKnifeModel;
		inline int  iSkinStattrak[37];

		inline float flSkinWear[37];
		inline float colSkins1[37][4];
		inline float colSkins2[37][4];
		inline float colSkins3[37][4];
		inline float colSkins4[37][4];

		inline std::string szSkinNametag[37];
	}
}

template< typename T >
class ConfigItem
{
	std::string category, name;
	T* value;
public:
	ConfigItem(std::string category, std::string name, T* value)
	{
		this->category = category;
		this->name = name;
		this->value = value;
	}
};

template< typename T >
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

class CConfig
{
public:
	std::vector< ConfigValue< int >* > ints;
	std::vector< ConfigValue< bool >* > bools;
	std::vector< ConfigValue< float >* > floats;
	std::vector< ConfigValue< std::string >* > strings;
private:

	void SetupValue(int&, int, std::string, std::string);
	void SetupValue(bool&, bool, std::string, std::string);
	void SetupValue(bool* value, bool def, int size, std::string category, std::string name);
	void SetupValue( std::string& value, std::string def, std::string category, std::string name );
	void SetupValue(float&, float, std::string, std::string);
	void SetupValue(int* value, int length, int def, std::string category, std::string name);
	void SetupValue(float* value, int length, float def, std::string category, std::string name);
	void SetupValue(float* value, int length, float* def, std::string category, std::string name);
	void SetupValue(std::vector<int>& value, int max, int def, std::string category, std::string name);

public:
	CConfig()
	{
		Setup();
	}

	void Setup();

	void CreateMainDirectory( );
	void Save( std::string ConfigName );
	void Load( std::string ConfigName );
	void DeleteConfig( std::string ConfigName );

	std::vector<std::string> vecConfigs = {};
	std::string ConfigPath = { };
	void RefreshConfigs( );

	std::vector<std::string> vecSoundFileNames = { };
	std::string SoundPath = { };
	void RefreshSounds( );

	bool bSaving = false;
};

extern CConfig* Config2;