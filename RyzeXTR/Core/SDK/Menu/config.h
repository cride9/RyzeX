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

		inline bool bEnable = false;
		inline int iAimbotKey = 0;
		inline float iAimbotFov = 180.f;
		inline bool bSilentAim = false;

		inline bool bDoubletap = false;
		inline int  iDoubletapKey;
		inline bool bHideshot = false;
		inline int  iHideShotKey;
		inline bool bResolver = false;
		inline bool bOverride = false;
		inline int	iOverrideBind;

		inline bool bForceBaim = false;
		inline int iForceBaimKey = 0;

		// all weapon
		inline bool bHitboxes[3][7][6];

		inline int iHitchances[7]{ false };
		inline int iAccuracyBoost[7]{ false };
		inline int iMinDamages[7]{ false };
		inline int iHeadPoints[7]{ false };
		inline int iBodyPoints[7]{ false };
		inline int iOverride[7]{ false };

		inline bool bSafePoint[7]{ false };
		inline bool bAutostop[7]{ false };
		inline int	bAutostopAggressiveness[7]{ false };
		inline bool bConditions[7][2]{ false };
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

		inline int iFlickOffset[3];
		inline int flickAngleSwitch[3];

		inline int iModifier[3];
		inline int iJitterValue[3];
		inline int iSpinSpeed[3];
		inline bool bAntiJitter[3];

		inline bool bFakelag = false;
		inline int iFakelag = 0;
		inline int iFakelagMin = 0;
		inline int iFakeLagMax = 0;
		inline int iFakeLagType = 0;

		inline bool bBreakLagcompensation = false;

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

		inline int iAttachmentMaterial[3];

		inline bool bAttachment[3]{ false };
		inline bool bAttachmentXhair[3];
		inline float flAttachmentColor[3][4];

		inline bool bAttachmentOverlay[3]{ false };
		inline bool bAttachmentOverlayXhair[3]{ false };
		inline float bAttachmentOverlayColor[3][4];

		inline bool bAttachmentThinOverlay[3]{ false };
		inline bool bAttachmentThinOverlayXhair[3]{ false };
		inline float bAttachmentThinOverlayColor[3][4];

		inline bool bAttachmentAnimatedOverlay[3]{ false };
		inline bool bAttachmentAnimatedOverlayXhair[3]{ false };
		inline float bAttachmentAnimatedOverlayColor[3][4];

		inline int iBacktrackType = 0;
		inline float flBacktrackColor[4] = { 1.f, 1.f, 1.f, 1.f};
		inline bool bBacktrackXhair = false;
		inline bool bBacktrackChams = false;

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

		inline bool bAutopeek = false;
		inline float flAutopeekColor[4] = { 255.f, 255.f, 255.f, 255.f };

		inline bool bDesyncChams = false;
		inline bool bDesyncChamsXhair = false;
		inline bool bDesyncSkeleton = false;
		inline int bDesyncType = 0;
		inline float flDesyncChamsCol[4] = { 255.f, 255.f, 255.f, 255.f };

		inline bool bViewmodel = false;

		inline int iViewmodelType = 0;
		inline float flViewmodelColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bViewmodelXhair = false;

		inline bool bViewmodelOverlay = false;
		inline float flViewmodelOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bViewmodelOverlayXhair = false;

		inline bool bViewmodelThinOverlay = false;
		inline float flViewmodelThinOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bViewmodelThinOverlayXhair = false;

		inline bool bViewmodelAnimOverlay = false;
		inline float flViewmodelAnimOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bViewmodelAnimOverlayXhair = false;

		inline bool bWeapon = false;

		inline int iWeaponType = 0;
		inline float flWeaponColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bWeaponXhair = false;

		inline bool bWeaponOverlay = false;
		inline float flWeaponOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bWeaponOverlayXhair = false;

		inline bool bWeaponThinOverlay = false;
		inline float bWeaponThinOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool bWeaponThinOverlayXhair = false;

		inline bool bWeaponAnimOverlay = false;
		inline float flWeaponAnimOverlayColor[4] = { 255.f, 255.f, 255.f, 255.f };
		inline bool flWeaponAnimOverlayXhair = false;

		inline bool bPaperMode = false;
	}
	namespace misc {

		inline bool bDrawCapsule = false; // 
		inline float flDrawCapsuleColor[4];
		inline float flDrawCapsuleColorHit[4];

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
		inline float flNightmodeColor[4];
		inline float flPropColor[4];

		inline int iSkybox = 0; // 
		inline float flSkyboxColor[4];

		inline bool bOverrideLampColors = false; //
		inline float flLampColors[4];
		inline int iFlicker = 0;

		inline bool bEnableRadio = false; // 
		inline int iRadioStation = 0;
		inline float flRadioVolume = 0.f;
		inline int iRadioMuteHotKey = 0;

		inline bool bOOF = false; //
		inline float flOOFColor[4];
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
		inline float flWorldCrosshairColor[4];

		inline bool bDamageIndicator = false; //
		inline float flDamageIndicator[4];

		inline int iViewModelFov = 75; //
		inline int iDebugFov = 90; //

		inline bool bRemovals[7]; //
		inline bool bOnScopeViewmodel = false; //
		inline float flScopeColor[4]; //
		inline float flScopeColorEnd[4]; //
		inline int iScopeLength = 1; //

		inline bool bAutobuy = false; //
		inline int iPistols;
		inline int iSnipers;
		inline bool bEquipments[3];
		inline bool bGrenades[5];

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
		inline float flHat[4]; //
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

	bool& FindBool(std::string targetName) {

		auto it = std::find_if(bools.begin(), bools.end(),
			[&targetName](const ConfigValue<bool>* configValue) {
				return configValue->name == targetName;
			});

		if (it != bools.end())
			return *(*it)->value;
	}

	int& FindInt(std::string targetName) {

		auto it = std::find_if(ints.begin(), ints.end(),
			[&targetName](const ConfigValue<int>* configValue) {
				return configValue->name == targetName;
			});

		if (it != ints.end())
			return *(*it)->value;
	}

	float& FindFloat(std::string targetName) {

		auto it = std::find_if(floats.begin(), floats.end(),
			[&targetName](const ConfigValue<float>* configValue) {
				return configValue->name == targetName;
			});

		if (it != floats.end())
			return *(*it)->value;
	}

	std::string& FindString(std::string targetName) {

		auto it = std::find_if(strings.begin(), strings.end(),
			[&targetName](const ConfigValue<std::string>* configValue) {
				return configValue->name == targetName;
			});

		if (it != strings.end())
			return *(*it)->value;
	}

	std::vector< ConfigValue< int >* > ints;
	std::vector< ConfigValue< bool >* > bools;
	std::vector< ConfigValue< float >* > floats;
	std::vector< ConfigValue< std::string >* > strings;
private:
	
	std::string szCategory;
	void PushCategory(std::string szCateg) { szCategory = szCateg; };

	void SetupColor(float colColor[4], float colDefault[4], std::string szName);
	void SetupColor(float(*colColor)[4], int iLength, Color colDefault, std::string szName);
	void SetupColor(float colColor[4], Color colDefault, std::string szName);

	void SetupIntArray(int* arrArray, int iLength, int* iDefault, std::string szName);
	void SetupIntArray(int* arrArray, int iLength, int iDefault, std::string szName); 
	void SetupFloatArray(float* arrArray, int iLength, float* iDefault, std::string szName);
	void SetupFloatArray(float* arrArray, int iLength, float iDefault, std::string szName); 
	void SetupBoolArray(bool* arrArray, int iLength, bool* iDefault, std::string szName);
	void SetupBoolArray(bool* arrArray, int iLength, bool iDefault, std::string szName);

	void SetupInt(int& iValue, int iDefault, std::string szName);
	void SetupFloat(float& iValue, float iDefault, std::string szName);
	void SetupBool(bool& bValue, bool bDefault, std::string szName);
	void SetupString(std::string& szValue, std::string szDefault, std::string szName);
	void SetupStringArray(std::string* szValue, int iLength, std::string szDefault, std::string szName);

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