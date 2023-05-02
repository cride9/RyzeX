#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>

namespace cfg {

	inline int configID = -1;

	inline bool debugSwitch = false;
	inline int debugSlider = 1;
	inline bool debugSwitch2 = false;

	namespace rage {

		inline bool enable;
		inline bool autostop[ ] = { false, false, false, false ,false, false };
		inline int	autostopAggressiveness[ ] = { false, false, false, false ,false, false };
		inline bool m_bAutoStopInAir[ ] = { false, false, false, false ,false, false };
		inline bool betweenshots[ ] = { false, false, false, false ,false, false };
		inline int	aimbotTargetSelection;
		inline bool autoscope[] = { false, false, false };
		inline bool forceSafePoint[] = { false, false, false, false ,false, false };
		inline int iAimbotFov = 180.f;

		inline bool doubletap;
		inline int  doubletapkey;
		inline bool resolver;
		inline int	overrideBind;
		inline bool m_bEnableBacktrack;

		inline bool forceBaim;
		inline int forceBaimKey = 0;

		// all weapon
		inline bool etcMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool etcHitboxes[] = { false, false, false, false, false ,false };
		inline bool etcSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int etcHitchance;
		inline int etcMindmg;
		inline int etcHeadPoints = 0;
		inline int etcBodyPoints = 0;
		inline int etcOverride = 0;

		inline bool autoMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool autoHitboxes[] = { false, false, false, false, false ,false };
		inline bool autoSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int autoHitchance;
		inline int autoMindmg;
		inline int autoHeadPoints = 0;
		inline int autoBodyPoints = 0;
		inline int autoOverride = 0;

		inline bool scoutMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool scoutHitboxes[] = { false, false, false, false, false ,false };
		inline bool scoutSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int scoutHitchance;
		inline int scoutMindmg;
		inline int scoutHeadPoints = 0;
		inline int scoutBodyPoints = 0;
		inline int scoutOverride = 0;

		inline bool awpMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool awpHitboxes[] = { false, false, false, false, false ,false };
		inline bool awpSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int awpHitchance;
		inline int awpMindmg;
		inline int awpHeadPoints = 0;
		inline int awpBodyPoints = 0;
		inline int awpOverride = 0;

		inline bool pistolMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool pistolHitboxes[] = { false, false, false, false, false ,false };
		inline bool pistolSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int pistolHitchance;
		inline int pistolMindmg;
		inline int pistolHeadPoints = 0;
		inline int pistolBodyPoints = 0;
		inline int pistolOverride = 0;

		inline bool heavypistolMultiHitboxes[] = { false, false, false, false, false ,false };
		inline bool heavypistolHitboxes[] = { false, false, false, false, false ,false };
		inline bool heavypistolSafeHitboxes[] = { false, false, false, false, false ,false };
		inline int heavypistolHitchance;
		inline int heavypistolMindmg;
		inline int heavypistolHeadPoints = 0;
		inline int heavypistolBodyPoints = 0;
		inline int heavypistolOverride = 0;
	}
	namespace antiaim {

		inline bool bEnabled = false;
		inline int iPitch = 2;
		inline int iYawBase = 1;
		inline int iYaw = 1;
		inline bool bSlideWalk = false;
		inline bool bInvertOnShoot = false;

		inline int iDesyncType = 0;
		inline float flDesyncValue = 58.f;
		inline int iInverterBind = 0;

		inline float bodyLean[2];

		inline int iFlickOffset = 0;
		inline int flickAngleSwitch = 0;

		inline bool m_bSwayDesync = false;

		inline int modifier = 0;
		inline int desyncModifier = 0;
		inline int desyncModifierValue = 0;
		inline int jittervalue = 0;
		inline float invertangle = 0.f;

		inline bool enableFakelag = false;
		inline int fakelag = 0;
		inline int fakelagmin = 0;
		inline int fakelagmax = 0;
		inline int fakelagType = 0;

		inline bool defensive = false;

		inline bool fakewalkenable = false; // need config value
		inline int fakewalkKey = 0; // need config value
		inline float fakewalk = 0;
		inline bool fakeduck = false;
		inline int fakeduckbind = 0;

		inline bool idealTick = false;
		inline int idealTickBind = 0;

		inline int freestand = 0;
	}
	namespace visual {

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

		inline bool bFlags[3][6];
		inline float flFlagsColor[3][6][4];
	}

	namespace model {

		inline int enemyBTType = 0;
		inline float enemyBTColor[4] = { 1.f, 1.f, 1.f, 1.f};
		inline bool enemyBTXhair = false;
		inline bool enemyBTEnable = false;

		// enemy
		inline int enemyType = 0;
		inline bool enemy = false;
		inline bool enemyXQZ = false;
		inline float enemyColor[4] = { 0.f, 216.f / 255.f, 1.f ,1.f };
		inline float enemyXQZColor[4] = { 255.f / 255.f, 192.f / 255.f, 0.f, 85.f / 255.f };
		inline bool enemyXhair = false;
		inline bool enemyXQZXhair = false;

		inline bool enemyOverlay = false;
		inline bool enemyOverlayXQZ = false;
		inline float enemyOverlayColor[4] = { 94.f / 255.f, 18.f / 255.f, 0.f ,1.f };
		inline float enemyOverlayXQZColor[4] = { 109.f / 255.f, 0.f, 0.f ,1.f };
		inline bool enemyOverlayXhair = true;
		inline bool enemyOverlayXQZXhair = false;

		inline bool enemyThinOverlay = false;
		inline bool enemyThinOverlayXQZ = false;
		inline float enemyThinOverlayColor[4] = { 77.f / 255.f, 90.f / 255.f, 1.f ,1.f };
		inline float enemyThinOverlayXQZColor[4] = { 64.f / 255.f, 75.f / 255.f, 0.f ,1.f };
		inline bool enemyThinOverlayXhair = false;
		inline bool enemyThinOverlayXQZXhair = false;

		inline bool enemyAnimOverlay = false;
		inline bool enemyAnimOverlayXQZ = false;
		inline float enemyAnimOverlayColor[4] = { 109.f / 255.f, 1.f, 249.f / 255.f ,1.f };
		inline float enemyAnimOverlayXQZColor[4] = { 116.f / 255.f, 129.f / 255.f, 1.f ,1.f };
		inline bool enemyAnimOverlayXhair = true;
		inline bool enemyAnimOverlayXQZXhair = false;

		inline bool m_bDrawMatrix = false;

		// team
		inline int teamType = 0;
		inline bool team = false;
		inline bool teamXQZ = false;
		inline float teamColor[4] = { 0.f, 216.f / 255.f, 1.f ,1.f };
		inline float teamXQZColor[4] = { 255.f / 255.f, 192.f / 255.f, 0.f, 85.f / 255.f };
		inline bool teamXhair = false;
		inline bool teamXQZXhair = false;

		inline bool teamOverlay = false;
		inline bool teamOverlayXQZ = false;
		inline float teamOverlayColor[4] = { 94.f / 255.f, 18.f / 255.f, 0.f ,1.f };
		inline float teamOverlayXQZColor[4] = { 109.f / 255.f, 0.f, 0.f ,1.f };
		inline bool teamOverlayXhair = true;
		inline bool teamOverlayXQZXhair = false;

		inline bool teamThinOverlay = false;
		inline bool teamThinOverlayXQZ = false;
		inline float teamThinOverlayColor[4] = { 77.f / 255.f, 90.f / 255.f, 1.f ,1.f };
		inline float teamThinOverlayXQZColor[4] = { 64.f / 255.f, 75.f / 255.f, 0.f ,1.f };
		inline bool teamThinOverlayXhair = false;
		inline bool teamThinOverlayXQZXhair = false;

		inline bool teamAnimOverlay = false;
		inline bool teamAnimOverlayXQZ = false;
		inline float teamAnimOverlayColor[4] = { 109.f / 255.f, 1.f, 249.f / 255.f ,1.f };
		inline float teamAnimOverlayXQZColor[4] = { 116.f / 255.f, 129.f / 255.f, 1.f ,1.f };
		inline bool teamAnimOverlayXhair = true;
		inline bool teamAnimOverlayXQZXhair = false;

		// local
		inline int localType = 0;
		inline bool local = false;
		inline bool localXQZ = false;
		inline float localColor[4] = { 125.f / 255.f, 125.f / 255.f, 125.f / 255.f ,1.f };
		inline float localXQZColor[4] = { 255.f / 255.f, 192.f / 255.f, 0.f, 85.f / 255.f };
		inline bool localXhair = false;
		inline bool localXQZXhair = false;

		inline bool localOverlay = false;
		inline bool localOverlayXQZ = false;
		inline float localOverlayColor[4] = { 38.f / 255.f, 29.f / 255.f, 0.f ,1.f };
		inline float localOverlayXQZColor[4] = { 109.f / 255.f, 0.f, 0.f ,1.f };
		inline bool localOverlayXhair = false;
		inline bool localOverlayXQZXhair = false;

		inline bool localThinOverlay = false;
		inline bool localThinOverlayXQZ = false;
		inline float localThinOverlayColor[4] = { 32.f / 255.f, 0.f / 255.f, 49.f / 255.f ,1.f };
		inline float localThinOverlayXQZColor[4] = { 64.f / 255.f, 75.f / 255.f, 0.f ,1.f };
		inline bool localThinOverlayXhair = false;
		inline bool localThinOverlayXQZXhair = false;

		inline bool localAnimOverlay = false;
		inline bool localAnimOverlayXQZ = false;
		inline float localAnimOverlayColor[4] = { 255.f / 255.f, 0.f, 0.f ,1.f };
		inline float localAnimOverlayXQZColor[4] = { 0.f, 24.f / 255.f, 1.f ,1.f };
		inline bool localAnimOverlayXhair = true;
		inline bool localAnimOverlayXQZXhair = true;

		inline bool localIdealTick = false;
		inline float localIdealTickColor[4] = { 255.f, 255.f, 255.f, 255.f };

		inline bool localDesync = false;
		inline bool localDesyncXhair = false;
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

		inline bool bDrawCapsule = false;
		inline float flDrawCapsuleColor[4] = {1.f, 1.f, 1.f, 1.f};
		inline float flDrawCapsuleColorHit[4] = { 1.f, 1.f, 1.f, 1.f };

		inline bool keyBindList = false;
		inline bool bunnyhop = false;
		inline bool autoStrafe = false;
		inline bool faststop = false;
		inline bool infiniteDuck = false;
		inline bool blockbot = false;
		inline int blockbotKey = 0;
		inline bool clantag = false;

		inline bool m_bDrawServerHitbox = false;
		inline bool m_bDrawServerHitboxOnAllEntities = false;

		inline bool nightmode = false;
		inline float nightmodeColor[4] = { 1.f, 1.f, 1.f, 1.f };

		inline int iSkybox = 0;

		inline bool aspectRatio = false;
		inline int aspectRatioValue = 0;
		inline bool preserveKillfeed = false;
		inline bool thirdperson = false;
		inline bool bulletImpact = false;
		inline bool onlyCheatLogs = false;
		inline int thirdpersonbind = 0;

		inline bool bulletTracer = false;
		inline float bulletTracerColor[4] = { 1.f ,1.f, 1.f, 1.f };

		inline bool bWorldCrosshair = false;
		inline float flWorldCrosshairColor[4] = { 1.f ,1.f, 1.f, 1.f };

		inline int viewmodelFov = 75;
		inline int fov = 90;

		inline bool removals[] = { false, false ,false ,false, false };
		inline bool drawViewmodelOnScope = false;
		inline float scopeColor[4] = { 1.f, 1.f, 1.f, 1.f };
		inline float scopeLength = 1.f;

		inline bool autobuyEnabled = false;
		inline int pistols;
		inline int snipers;
		inline bool equipments[] = { false, false, false };
		inline bool grenades[] = { false, false, false, false, false };

		inline bool fakePing = false;
		inline float fakePingFactor = 0.f;

		// funny times
		inline bool m_bRussianRoulette = false;

		// hitsounds
		inline int m_iHitSound = 0;
		inline float m_flHitSoundVolume = 100.f;
		inline std::string m_szWavPath = "";
	
	}
	namespace skin {

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
protected:
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
};

extern CConfig* Config2;