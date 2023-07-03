#pragma once
#include <cstdint>
#include "DataTyes/Vector.h"
#include "DataTyes/Matrix.h"
#include "NetVar/Netvar.h"
#include "datamap.h"
#include "../Interface/Interfaces/IEngineTrace.h"
#include "../Interface/Classes/CModelInfo.h"
#include "Animstate.h"
#include <optional>
#include "../XorStr.h"

enum InvalidatePhysicsBits_t
{
	POSITION_CHANGED = 0x1,			// Means position has changed
	ANGLES_CHANGED = 0x2,			// Means angles has changed
	VELOCITY_CHANGED = 0x4,			// Means velocity has changed
	ANIMATION_CHANGED = 0x8,		// Means cycle has changed, or any other event which would cause render-to-texture shadows to need to be rerendeded
	BOUNDS_CHANGED = 0x10,			// Means render bounds have changed, so shadow decal projection is required, etc.
	SEQUENCE_CHANGED = 0x20,		// Means sequence has changed, only interesting when surrounding bounds depends on sequence																				
};

enum ETeamID : int
{
	TEAM_UNASSIGNED = 0,
	TEAM_SPECTATOR,
	TEAM_TT,
	TEAM_CT
};

enum EItemDefinitionIndex : short
{
	WEAPON_NONE = 0,
	WEAPON_DEAGLE = 1,
	WEAPON_ELITE = 2,
	WEAPON_FIVESEVEN = 3,
	WEAPON_GLOCK = 4,
	WEAPON_AK47 = 7,
	WEAPON_AUG = 8,
	WEAPON_AWP = 9,
	WEAPON_FAMAS = 10,
	WEAPON_G3SG1 = 11,
	WEAPON_GALILAR = 13,
	WEAPON_M249 = 14,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10 = 17,
	WEAPON_P90 = 19,
	WEAPON_ZONE_REPULSOR = 20,
	WEAPON_MP5SD = 23,
	WEAPON_UMP45 = 24,
	WEAPON_XM1014 = 25,
	WEAPON_BIZON = 26,
	WEAPON_MAG7 = 27,
	WEAPON_NEGEV = 28,
	WEAPON_SAWEDOFF = 29,
	WEAPON_TEC9 = 30,
	WEAPON_TASER = 31,
	WEAPON_HKP2000 = 32,
	WEAPON_MP7 = 33,
	WEAPON_MP9 = 34,
	WEAPON_NOVA = 35,
	WEAPON_P250 = 36,
	WEAPON_SHIELD = 37,
	WEAPON_SCAR20 = 38,
	WEAPON_SG556 = 39,
	WEAPON_SSG08 = 40,
	WEAPON_KNIFE_GG = 41,
	WEAPON_KNIFE = 42,
	WEAPON_FLASHBANG = 43,
	WEAPON_HEGRENADE = 44,
	WEAPON_SMOKEGRENADE = 45,
	WEAPON_MOLOTOV = 46,
	WEAPON_DECOY = 47,
	WEAPON_INCGRENADE = 48,
	WEAPON_C4 = 49,
	WEAPON_HEALTHSHOT = 57,
	WEAPON_KNIFE_T = 59,
	WEAPON_M4A1_SILENCER = 60,
	WEAPON_USP_SILENCER = 61,
	WEAPON_CZ75A = 63,
	WEAPON_REVOLVER = 64,
	WEAPON_TAGRENADE = 68,
	WEAPON_FISTS = 69,
	WEAPON_BREACHCHARGE = 70,
	WEAPON_TABLET = 72,
	WEAPON_MELEE = 74,
	WEAPON_AXE = 75,
	WEAPON_HAMMER = 76,
	WEAPON_SPANNER = 78,
	WEAPON_KNIFE_GHOST = 80,
	WEAPON_FIREBOMB = 81,
	WEAPON_DIVERSION = 82,
	WEAPON_FRAG_GRENADE = 83,
	WEAPON_SNOWBALL = 84,
	WEAPON_BUMPMINE = 85,
	WEAPON_KNIFE_BAYONET = 500,
	WEAPON_KNIFE_CSS = 503,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT = 506,
	WEAPON_KNIFE_KARAMBIT = 507,
	WEAPON_KNIFE_M9_BAYONET = 508,
	WEAPON_KNIFE_TACTICAL = 509,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY = 515,
	WEAPON_KNIFE_PUSH = 516,
	WEAPON_KNIFE_CORD = 517,
	WEAPON_KNIFE_CANIS = 518,
	WEAPON_KNIFE_URSUS = 519,
	WEAPON_KNIFE_GYPSY_JACKKNIFE = 520,
	WEAPON_KNIFE_OUTDOOR = 521,
	WEAPON_KNIFE_STILETTO = 522,
	WEAPON_KNIFE_WIDOWMAKER = 523,
	WEAPON_KNIFE_SKELETON = 525,
	GLOVE_STUDDED_BROKENFANG = 4725,
	GLOVE_STUDDED_BLOODHOUND = 5027,
	GLOVE_T = 5028,
	GLOVE_CT = 5029,
	GLOVE_SPORTY = 5030,
	GLOVE_SLICK = 5031,
	GLOVE_LEATHER_HANDWRAPS = 5032,
	GLOVE_MOTORCYCLE = 5033,
	GLOVE_SPECIALIST = 5034,
	GLOVE_STUDDED_HYDRA = 5035,
	SPECIAL_AGENT_BLUEBERRIES_BUCKSHOT = 4619,
	SPECIAL_AGENT_TWO_TIMES_MCCOY_TACP = 4680,
	SPECIAL_AGENT_COMMANDOR_MAE_JAMISON = 4711,
	SPECIAL_AGENT_1ST_LIEUTENANT_FARLOW,
	SPECIAL_AGENT_JOHN_KASK,
	SPECIAL_AGENT_BIO_HAZ_SPECIALIST,
	SPECIAL_AGENT_SERGEANT_BOMBSON,
	SPECIAL_AGENT_CHEM_HAZ_SPECIALIST,
	SPECIAL_AGENT_REZAN_THE_REDSHIRT = 4718,
	SPECIAL_AGENT_SIR_BLOODY_MIAMI_DARRYL = 4726,
	SPECIAL_AGENT_SAFECRACKER_VOLTZMANN,
	SPECIAL_AGENT_LITTLE_KEV,
	SPECIAL_AGENT_GETAWAY_SALLY = 4730,
	SPECIAL_AGENT_NUMBER_K = 4732,
	SPECIAL_AGENT_SIR_BLOODY_SILENT_DARRYL = 4733,
	SPECIAL_AGENT_SIR_BLOODY_SKULLHEAD_DARRYL,
	SPECIAL_AGENT_SIR_BLOODY_DARRYL_ROYALE,
	SPECIAL_AGENT_SIR_BLOODY_LOUDMOUTH_DARRYL,
	SPECIAL_AGENT_T = 5036,
	SPECIAL_AGENT_CT = 5037,
	SPECIAL_AGENT_GROUND_REBEL = 5105,
	SPECIAL_AGENT_OSIRIS,
	SPECIAL_AGENT_SHAHMAT,
	SPECIAL_AGENT_MUHLIK,
	SPECIAL_AGENT_SOLDIER = 5205,
	SPECIAL_AGENT_ENFORCER,
	SPECIAL_AGENT_SLINGSHOT,
	SPECIAL_AGENT_STREET_SOLDIER,
	SPECIAL_AGENT_OPERATOR = 5305,
	SPECIAL_AGENT_MARKUS_DELROW,
	SPECIAL_AGENT_MICHAEL_SYFERS,
	SPECIAL_AGENT_AVA,
	SPECIAL_AGENT_3RD_COMMANDO_COMPANY = 5400,
	SPECIAL_AGENT_SEAL_TEAM_6_SOLDIER,
	SPECIAL_AGENT_BUCKSHOT,
	SPECIAL_AGENT_TWO_TIMES_MCCOY_USAF,
	SPECIAL_AGENT_RICKSAW,
	SPECIAL_AGENT_DRAGOMIR = 5500,
	SPECIAL_AGENT_MAXIMUS,
	SPECIAL_AGENT_REZAN_THE_READY,
	SPECIAL_AGENT_BLACKWOLF = 5503,
	SPECIAL_AGENT_THE_DOCTOR,
	SPECIAL_AGENT_DRAGOMIR_FOOTSOLDIERS,
	SPECIAL_AGENT_B_SQUADRON_OFFICER = 5601
};

struct ClientHitVerify_t {
	Vector pos;
	float time;
	float expires;
};

class IInterpolatedVar
{
public:

	virtual void Setup(void* pValue, int type) = 0;
	virtual void SetInterpolationAmount(float seconds) = 0;

	// Returns true if the new value is different from the prior most recent value.
	virtual void NoteLastNetworkedValue() = 0;
	virtual bool NoteChanged(float flCurrentTime, float flChangeTime, bool bUpdateLastNetworkedValue) = 0;
	virtual void Reset(float flCurrentTime) = 0;

	// Returns 1 if the value will always be the same if currentTime is always increasing.
	virtual int Interpolate(float currentTime) = 0;

	virtual int	 GetType() const = 0;
	virtual void RestoreToLastNetworked() = 0;
	virtual void Copy(IInterpolatedVar* pSrc) = 0;

	virtual const char* GetDebugName() = 0;
	virtual void SetDebugName(const char* pName) = 0;
};


class VarMapEntry_t {
public:
	unsigned short type;
	unsigned short m_bNeedsToInterpolate; // Set to false when this var doesn't
										  // need Interpolate() called on it anymore.
	void* data;
	IInterpolatedVar* watcher;
};

struct VarMapping_t {
	VarMapping_t() {
		m_nInterpolatedEntries = 0;
	}

	VarMapEntry_t* m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};

class CBaseCombatWeapon;
using CBaseHandle = std::uintptr_t;

class IHandleEntity
{
public:
	virtual							~IHandleEntity() { }
	virtual void					SetRefEHandle(const CBaseHandle& hRef) = 0;
	virtual const CBaseHandle&		GetRefEHandle() const = 0;
};

class IClientUnknown;
class ICollideable
{
public:
	virtual IHandleEntity*			GetEntityHandle() = 0;
	virtual Vector&					OBBMins() const = 0;
	virtual Vector&					OBBMaxs() const = 0;
	virtual void					WorldSpaceTriggerBounds(Vector* pVecWorldMins, Vector* pVecWorldMaxs) const = 0;
	virtual bool					TestCollision(const Ray_t& ray, unsigned int fContentsMask, Trace_t& tr) = 0;
	virtual bool					TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, Trace_t& tr) = 0;
	virtual int						GetCollisionModelIndex() = 0;
	virtual const Model_t*			GetCollisionModel() = 0;
	virtual Vector&					GetCollisionOrigin() const = 0;
	virtual Vector&					GetCollisionAngles() const = 0;
	virtual const matrix3x4_t&		CollisionToWorldTransform() const = 0;
	virtual int						GetSolid() const = 0;
	virtual int						GetSolidFlags() const = 0;
	virtual IClientUnknown*			GetIClientUnknown() = 0;
	virtual int						GetCollisionGroup() const = 0;

	/*
	 * @note: should be equivalent to C_BaseAnimating::ComputeHitboxSurroundingBox
	 * DOESNT NEEDED TRANSORMATION!
	 * and bugged when trying to get non-player entity box
	 */
	virtual void					WorldSpaceSurroundingBounds(Vector* pVecMins, Vector* pVecMaxs) = 0;
	virtual unsigned int			GetRequiredTriggerFlags() const = 0;
	virtual const matrix3x4_t*		GetRootParentToWorldTransform() const = 0;
	virtual void*					GetVPhysicsObject() const = 0;
};

class IClientAlphaProperty
{
public:
	virtual IClientUnknown* GetIClientUnknown() = 0;
	virtual void SetAlphaModulation(std::uint8_t uAlpha) = 0;
	virtual void SetRenderFX(int nRenderFx, int nRenderMode, float flStartTime = FLT_MAX, float flDuration = 0.0f) = 0;
	virtual void SetFade(float flGlobalFadeScale, float flDistFadeStart, float flDistFadeEnd) = 0;
	virtual void SetDesyncOffset(int nOffset) = 0;
	virtual void EnableAlphaModulationOverride(bool bEnable) = 0;
	virtual void EnableShadowAlphaModulationOverride(bool bEnable) = 0;
	virtual void SetDistanceFadeMode(int nFadeMode) = 0;
};

class IClientNetworkable;
class IClientRenderable;
class IClientEntity;
class CBaseEntity;
class IClientThinkable;
class IClientUnknown : public IHandleEntity
{
public:
	virtual ICollideable* GetCollideable() = 0;
	virtual IClientNetworkable* GetClientNetworkable() = 0;
	virtual IClientRenderable* GetClientRenderable() = 0;
	virtual IClientEntity* GetIClientEntity() = 0;
	virtual CBaseEntity* GetBaseEntity() = 0;
	virtual IClientThinkable* GetClientThinkable() = 0;
	virtual IClientAlphaProperty* GetClientAlphaProperty() = 0;
};

using ClientShadowHandle_t = std::uint16_t;
using ClientRenderHandle_t = std::uint16_t;
using ModelInstanceHandle_t = std::uint16_t;

struct RenderableInstance_t
{
	std::uint8_t uAlpha;
};

class IClientRenderable
{
public:
	virtual IClientUnknown*			GetIClientUnknown() = 0;
	virtual Vector&					GetRenderOrigin() = 0;
	virtual Vector&					GetRenderAngles() = 0;
	virtual bool					ShouldDraw() = 0;
	virtual int						GetRenderFlags() = 0;
	virtual bool					IsTransparent() = 0;
	virtual ClientShadowHandle_t	GetShadowHandle() const = 0;
	virtual ClientRenderHandle_t&	RenderHandle() = 0;
	virtual const Model_t*			GetModel() const = 0;
	virtual int						DrawModel(int nFlags, const RenderableInstance_t& uInstance) = 0;
	virtual int						GetBody() = 0;
	virtual void					GetColorModulation(float* pColor) = 0;
	virtual bool					LODTest() = 0;
	virtual bool					SetupBones(matrix3x4_t* matBoneToWorldOut, int nMaxBones, int fBoneMask, float flCurrentTime) = 0;
	virtual void					SetupWeights(const matrix3x4_t* matBoneToWorld, int nFlexWeightCount, float* flFlexWeights, float* flFlexDelayedWeights) = 0;
	virtual void					DoAnimationEvents() = 0;
	virtual void*					GetPVSNotifyInterface() = 0;
	virtual void					GetRenderBounds(Vector& vecMins, Vector& vecMaxs) = 0;
	virtual void					GetRenderBoundsWorldspace(Vector& vecMins, Vector& vecMaxs) = 0;
	virtual void					GetShadowRenderBounds(Vector& vecMins, Vector& vecMaxs, int iShadowType) = 0;
	virtual bool					ShouldReceiveProjectedTextures(int nFlags) = 0;
	virtual bool					GetShadowCastDistance(float* pDistance, int iShadowType) const = 0;
	virtual bool					GetShadowCastDirection(Vector* vecDirection, int iShadowType) const = 0;
	virtual bool					IsShadowDirty() = 0;
	virtual void					MarkShadowDirty(bool bDirty) = 0;
	virtual IClientRenderable*		GetShadowParent() = 0;
	virtual IClientRenderable*		FirstShadowChild() = 0;
	virtual IClientRenderable*		NextShadowPeer() = 0;
	virtual int						ShadowCastType() = 0;
	virtual void					unused2() {}
	virtual void					CreateModelInstance() = 0;
	virtual ModelInstanceHandle_t	GetModelInstance() = 0;
	virtual const matrix3x4_t&		RenderableToWorldTransform() = 0;
	virtual int						LookupAttachment(const char* pAttachmentName) = 0;
	virtual bool					GetAttachment(int nIndex, Vector& vecOrigin, Vector& angView) = 0;
	virtual bool					GetAttachment(int nIndex, matrix3x4_t& matAttachment) = 0;
	virtual bool					ComputeLightingOrigin(int nAttachmentIndex, Vector vecModelLightingCenter, const matrix3x4_t& matrix, Vector& vecTransformedLightingCenter) = 0;
	virtual float*					GetRenderClipPlane() = 0;
	virtual int						GetSkin() = 0;
	virtual void					OnThreadedDrawSetup() = 0;
	virtual bool					UsesFlexDelayedWeights() = 0;
	virtual void					RecordToolMessage() = 0;
	virtual bool					ShouldDrawForSplitScreenUser(int nSlot) = 0;
	virtual std::uint8_t			OverrideAlphaModulation(std::uint8_t uAlpha) = 0;
	virtual std::uint8_t			OverrideShadowAlphaModulation(std::uint8_t uAlpha) = 0;
	virtual void*					GetClientModelRenderable() = 0;
};

enum EDataUpdateType : int
{
	DATA_UPDATE_CREATED = 0,
	DATA_UPDATE_DATATABLE_CHANGED,
};

class IClientNetworkable
{
public:
	virtual IClientUnknown*			GetIClientUnknown() = 0;
	virtual void					ReleaseNetworkable() = 0;
	virtual CBaseClient*			GetClientClass() = 0;
	virtual void					NotifyShouldTransmit(int iState) = 0;
	virtual void					OnPreDataChanged(EDataUpdateType updateType) = 0;
	virtual void					OnDataChanged(EDataUpdateType updateType) = 0;
	virtual void					PreDataUpdate(EDataUpdateType updateType) = 0;
	virtual void					PostDataUpdate(EDataUpdateType updateType) = 0;
	virtual void					OnDataUnchangedInPVS() = 0;
	virtual bool					IsDormant() const = 0;
	virtual int						EntIndex() const = 0;
	virtual void					ReceiveMessage(EClassIndex classIndex, void* msg) = 0;
	virtual void*					GetDataTableBasePtr() = 0;
	virtual void					SetDestroyedOnRecreateEntities() = 0;
};

class CClientThinkHandle;
using ClientThinkHandle_t = CClientThinkHandle*;
class IClientThinkable
{
public:
	virtual IClientUnknown* GetIClientUnknown() = 0;
	virtual void					ClientThink() = 0;
	virtual ClientThinkHandle_t		GetThinkHandle() = 0;
	virtual void					SetThinkHandle(ClientThinkHandle_t hThink) = 0;
	virtual void					Release() = 0;
};

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual const Vector&			GetAbsOrigin() const = 0;
	virtual const Vector&			GetAbsAngles() const = 0;
	virtual void*					GetMouth() = 0;
	virtual bool					GetSoundSpatialization(struct SpatializationInfo_t& info) = 0;
	virtual bool					IsBlurred() = 0;

	void SetAbsOrigin(const Vector& vecOrigin) {

		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector&);
		static auto oSetAbsOrigin = reinterpret_cast<SetAbsOriginFn>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8")));
		oSetAbsOrigin(this, vecOrigin);
	}

	void SetAbsAngles(const Vector& angView) {

		using SetAbsAngleFn = void(__thiscall*)(void*, const Vector&);
		static auto oSetAbsAngles = reinterpret_cast<SetAbsAngleFn>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8")));
		oSetAbsAngles(this, angView);
	}

	void SetAbsVelocity(const Vector& vecVelocity) {

		using Fn = void(__thiscall*)(void*, const Vector&);
		static auto fn = reinterpret_cast<Fn>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3")));
		fn(this, vecVelocity);
	}

	DataMap_t* GetDataDescMap()
	{
		return util::CallVFunc<DataMap_t*>(this, 15);
	}

	DataMap_t* GetPredictionDescMap()
	{
		return util::CallVFunc<DataMap_t*>(this, 17);
	}
};

// let's make macros hehe
#define ADD_OFFSET(name, type, offset) type& name() { \
	return *reinterpret_cast<type*>(uint32_t(this) + offset); \
}

#define ADD_POFFSET(name, type, offset) type* name() { \
	return *reinterpret_cast<type**>(uint32_t(this) + offset); \
}

#define ADD_NETVAR(name, type, netvar) type& name() { \
	static auto offset = n::netvars[fnv::HashConst(netvar)].uOffset; \
	return *reinterpret_cast<type*>(uint32_t(this) + offset); \
}

#define ADD_PNETVAR(name, type, netvar) type* name() { \
	static auto offset = n::netvars[fnv::HashConst(netvar)].uOffset; \
	return reinterpret_cast<type*>(uint32_t(this) + offset); \
}

#define ADD_NETVAROFFSET(name, type, netvar, address) type& name() { \
	static auto offset = n::netvars[fnv::HashConst(netvar)].uOffset; \
	return *reinterpret_cast<type*>(uint32_t(this) + offset + address); \
}

#define ADD_PNETVAROFFSET(name, type, netvar, address) type* name() { \
	static auto offset = n::netvars[fnv::HashConst(netvar)].uOffset; \
	return reinterpret_cast<type*>(uint32_t(this) + offset + address); \
}

#define ADD_DATAMAP(name, type, map, datafield) type& name() {		\
	static constexpr uint32_t hash = fnv::HashConst(datafield);		\
	static uintptr_t offset = n::FindInDataMap(map, hash);			\
	return *reinterpret_cast<type*>(uint32_t(this) + offset);		\
}

/* add function to get datamap variable */
#define ADD_DATAFIELD( szFunctionName, Type, pMap, szDataField )										\
[[nodiscard]] std::add_lvalue_reference_t<Type> szFunctionName()										\
{																										\
	static constexpr uint32_t uHash = fnv::HashConst(szDataField);										\
	static std::uintptr_t uOffset = n::FindInDataMap(pMap, uHash);										\
	return *(std::add_pointer_t<Type>)(reinterpret_cast<std::uintptr_t>(this) + uOffset);				\
}

#define ADD_PDATAMAP(name, type, map, datafield) type* name() {		\
	static constexpr uint32_t hash = fnv::HashConst(datafield);		\
	static uintptr_t offset = n::FindInDataMap(map, hash);			\
	return reinterpret_cast<type*>(uint32_t(this) + offset);		\
}

class IKContext
{
public:

	void Init( CStudioHdr* hdr, Vector& angles, Vector& origin, float curtime, int framecount, int boneMask ) {
		static const auto ik_init_address = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F"));
		reinterpret_cast< void( __thiscall* )( IKContext*, CStudioHdr*, Vector&, Vector&, float, int, int ) >( ik_init_address )( this, hdr, angles, origin, curtime, framecount, boneMask );
	}

	void UpdateTargets( Vector* pos, Quaternion* q, matrix3x4_t* bone_array, byte* computed ) {
		static const auto update_targets_address = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F0 81 EC ? ? ? ? 33 D2 89"));
		reinterpret_cast< void( __thiscall* )( IKContext*, Vector*, Quaternion*, matrix3x4_t*, byte* ) >( update_targets_address )( this, pos, q, bone_array, computed );
	}

	void SolveDependencies( Vector* pos, Quaternion* q, matrix3x4_t* bone_array, byte* computed ) {
		
		static const auto solve_dependencies_address = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81"));

		using Fn = void(__thiscall*)(IKContext*, Vector*, Quaternion*, matrix3x4_t*, byte*);
		static auto fn = reinterpret_cast<Fn>(solve_dependencies_address);

		if (!fn)
			return;

		fn(this, pos, q, bone_array, computed);

		// reinterpret_cast< void( __thiscall* )( IKContext*, Vector*, Quaternion*, matrix3x4_t*, byte* ) >( solve_dependencies_address )( this, pos, q, bone_array, computed );
	}

	void ClearTargets( )
	{
		auto v56 = 0;
		if ( *( int* )( ( DWORD )this + 4080 ) > 0 )
		{
			auto v57 = ( int* )( ( DWORD )this + 208 );
			do
			{
				*v57 = -9999;
				v57 += 85;
				++v56;
			} while ( v56 < *( int* )( ( DWORD )this + 4080 ) );
		}
	}
};

class CWeaponCSBase;
class CBaseEntity : public IClientEntity{

public:


	//ADD_POFFSET(AnimState, CAnimState, 0x9960); You are fr a handicapped fuck, just fucking netvar it
	//ADD_OFFSET(GetEFlags, int, 0xE8);

	// normal netvars
	ADD_NETVAR(GetViewPunch, Vector, "CBasePlayer->m_viewPunchAngle");
	ADD_NETVAR(GetAimPunch, Vector, "CBasePlayer->m_aimPunchAngle");
	ADD_NETVAR(GetAimPunchVelocity, Vector, "CBasePlayer->m_aimPunchAngleVel" );
	ADD_NETVAR(GetViewOffset, Vector, "CBasePlayer->m_vecViewOffset[0]");
	ADD_NETVAR(GetFriction, float, "CBasePlayer->m_flFriction");
	ADD_NETVAR(GetTickBase, int, "CBasePlayer->m_nTickBase");
	ADD_NETVAROFFSET(GeFinalPredictedTick, int, "CBasePlayer->m_nTickBase", 0x4);
	ADD_PNETVAR(GetNextThink, int, "CBasePlayer->m_nNextThinkTick");
	ADD_NETVAR(GetVelocity, Vector, "CBasePlayer->m_vecVelocity[0]");
	//ADD_NETVAR(GetGroundEntity, CBaseHandle, "CBasePlayer->m_hGroundEntity");
	ADD_NETVAR(GetHealth, int, "CBasePlayer->m_iHealth");
	ADD_NETVAR(GetLifeState, int, "CBasePlayer->m_lifeState");
	ADD_NETVAR(GetFlags, int, "CBasePlayer->m_fFlags");
	ADD_NETVAR(GetMaxSpeed, float, "CBasePlayer->m_flMaxspeed");
	ADD_NETVAR(GetObserverTarget, CBaseHandle, "CBasePlayer->m_hObserverTarget");
	ADD_NETVAR(GetViewModel, CBaseHandle, "CBasePlayer->m_hViewModel[0]");
	ADD_NETVAR(GetDuckAmount, float, "CBasePlayer->m_flDuckAmount");
	ADD_NETVAR(GetDuckSpeed, float, "CBasePlayer->m_flDuckSpeed");
	ADD_NETVAR(GetFallVelocity, float, "CBasePlayer->m_flFallVelocity");
	ADD_NETVAR( GetVelocityModifier, float, "CBasePlayer->m_flVelocityModifier" );

	// pointer netvars
	//ADD_PNETVAR(GetFallVelocity, float, "CBasePlayer->m_flFallVelocity"); Tf is going on here? Leaving it commented out incase you don't like the change
	ADD_NETVAR(GetObserverMode, int, "CBasePlayer->m_iObserverMode");
	ADD_PNETVAR(GetLastPlace, const char, "CBasePlayer->m_szLastPlaceName");

	// offset variables
	ADD_NETVAROFFSET(GetButtonDisabled, int, "CBasePlayer->m_hViewEntity", -0xC);
	ADD_NETVAROFFSET(GetButtonForced, int, "CBasePlayer->m_hViewEntity", -0x8);
	ADD_NETVAROFFSET(AnimState, CAnimState*, "CCSPlayer->m_bIsScoped", -0x14); // @ida: 8B 8E ? ? ? ? F3 0F 10 48 ? E8 ? ? ? ? C7 + 0x2
	ADD_NETVAR(GetThirdpersonRecoil, float, "CCSPlayer->m_flThirdpersonRecoil");
	ADD_NETVAR(m_bStrafing, bool, "CCSPlayer->m_bStrafing");
	ADD_NETVAR(GetMoveState, int, "CCSPlayer->m_iMoveState");
	ADD_NETVAR(GetDuckOverride, bool, "CCSPlayer->m_bDuckOverride");

	// pointer offset variables
	ADD_PNETVAROFFSET(GetViewAngles, Vector, "CBasePlayer->deadflag", 0x4);
	ADD_PNETVAROFFSET(GetCurrentCommand, CUserCmd*, "CBasePlayer->m_hViewEntity", -0x4);

	ADD_DATAMAP(GetEFlags, int, this->GetPredictionDescMap(), "m_iEFlags");
	ADD_PDATAMAP(GetButtons, int, this->GetPredictionDescMap(), "m_nButtons");
	ADD_DATAMAP(GetButtonLast, int, this->GetPredictionDescMap(), "m_afButtonLast");
	ADD_DATAMAP(GetButtonPressed, int, this->GetPredictionDescMap(), "m_afButtonPressed");
	ADD_DATAMAP(GetButtonReleased, int, this->GetPredictionDescMap(), "m_afButtonReleased");
	ADD_PDATAMAP(GetImpulse, int, this->GetPredictionDescMap(), "m_nImpulse");
	ADD_DATAMAP(GetVecBaseVelocity, Vector, this->GetPredictionDescMap( ), "m_vecBaseVelocity" );
	ADD_DATAFIELD(GetVecAbsVelocity, Vector&, this->GetPredictionDescMap( ), "m_vecAbsVelocity" );
	ADD_DATAMAP(GetSurfaceFriction, float, this->GetPredictionDescMap(), "m_surfaceFriction");
	ADD_DATAMAP(m_vecAbsVelocity, Vector, this->GetPredictionDescMap(), "m_vecAbsVelocity");

	template <typename T>
	T& GetOffset(uintptr_t offset) {
		return *reinterpret_cast<T*>(uint32_t(this) + offset);
	}

	template <typename T>
	T* GetOffsetPointer(uintptr_t offset) {
		return reinterpret_cast<T*>(uint32_t(this) + offset);
	}

	CBaseEntity* GetGroundEntity() {

		static auto offset = n::netvars[fnv::HashConst("CBasePlayer->m_hGroundEntity")].uOffset;
		return reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntityFromHandle(*reinterpret_cast<CBaseHandle*>(uint32_t(this) + offset)));
	}

	std::array<int, 64>& GetMyWeapons() {

		return *(std::array<int, 64>*)((uint32_t(this) + 0x2E08));
	}

	CUtlVector <matrix3x4_t>& GetCachedBoneData() {

		return *(CUtlVector <matrix3x4_t>*)(uintptr_t(this) + 0x2914);
	}

	PlayerInfo_t& GetPlayerInfo() {

		PlayerInfo_t pInfo;
		i::EngineClient->GetPlayerInfo(this->EntIndex(), &pInfo);
		return pInfo;
	}

	void InvalidatePhysicsRecursive(int32_t flags) {

		static const auto invalidate_physics_recursive = reinterpret_cast<void(__thiscall*)(CBaseEntity*, int32_t)>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56")));
		invalidate_physics_recursive(this, flags);
	}

	CUserCmd& GetLastCommand() {
		static const std::uintptr_t uLastCommandOffset = *reinterpret_cast<std::uintptr_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("8D 8E ? ? ? ? 89 5C 24 3C")) + 0x2);
		return *reinterpret_cast<CUserCmd*>(reinterpret_cast<std::uintptr_t>(this) + uLastCommandOffset);
	}

	int& GetTakeDamage() {
		static const std::uintptr_t uTakeDamageOffset = *reinterpret_cast<std::uintptr_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("80 BE ? ? ? ? ? 75 46 8B 86")) + 0x2);
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + uTakeDamageOffset);
	}

	VarMapping_t* GetVarMap() {
		return reinterpret_cast<VarMapping_t*>((std::uintptr_t)(this) + 0x24); //0x4C );
	}

	bool IsAlive() {
		return (this->GetHealth() > 0);
	}

	bool IsPlayer() {
		return util::CallVFunc<bool>(this, 158);
	}

	bool& JiggleBones() {

		return *reinterpret_cast<bool*>(uint32_t(this) + 0x2930);
	}

	bool& UseNewAnimationState() {

		return *reinterpret_cast<bool*>(uint32_t(this) + 0x3AB4);
	}

	void GetAnimationLayers(CAnimationLayer* pAnimlayer) {

		memcpy(pAnimlayer, this->GetAnimationOverlays(), 13 * sizeof(CAnimationLayer));
	}

	void SetAnimationLayers(CAnimationLayer* pAnimlayer) {

		memcpy(this->GetAnimationOverlays(), pAnimlayer, 13 * sizeof(CAnimationLayer));
	}

	struct LayerData_t
	{
		int nSequence;
		float flPlaybackRate;
		float flCycle;
		float flWeight;
		float flWeightWithAirSmooth;
		float flFeetWeight;

		float flMovementSide;
		Vector angMoveYaw;
		Vector vecDirection;
	};

	void GetPoseParameters(float* flPoseParameter) {

		memcpy(flPoseParameter, &this->GetPoseParameter(), 24 * sizeof(float));
	}

	void SetPoseParameters(float* flPoseParameter) {

		memcpy(&this->GetPoseParameter(), flPoseParameter, 24 * sizeof(float));
	}

	void GetBoneCache(matrix3x4_t* matrix) {

		if (!this->GetCachedBoneData().Base() || !this->GetCachedBoneData().Count() || !IsBoneCacheValid())
			return;

		memcpy(matrix, this->GetCachedBoneData().Base(), this->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
	}

	void SetBoneCache(matrix3x4_t* matrix) {

		if (!this->GetCachedBoneData().Base() || !this->GetCachedBoneData().Count() || !IsBoneCacheValid())
			return;

		memcpy(this->GetCachedBoneData().Base(), matrix, this->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
	}

	bool IsBoneCacheValid() {

		return (this->GetRecentModelBoneCounter() != GetModelBoneCounter());
	}

	void SetupBones_AttachmentHelper()
	{// 55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4
		
		static auto sig = MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 48 53 8B 5D"));
		return ((void(__thiscall*)(void*, void*))(sig))(this, this->GetStudioHdr());
	}

	static unsigned long& GetModelBoneCounter()
	{
		static std::uint8_t* uModelBoneCounterOffset = reinterpret_cast<std::uint8_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("3B 05 ? ? ? ? 0F 84 ? ? ? ? 8B 47")) + 0x2);
		return *reinterpret_cast<unsigned long*>(uModelBoneCounterOffset);
	}

	void UpdateIKLocks( float curtime ) {
		using o_fn = void( __thiscall* )( void*, float );
		util::CallVFunc<o_fn>( this, 191, curtime );
	}

	void CalculateIKLocks( float curtime ) {
		using o_fn = void( __thiscall* )( void*, float );
		util::CallVFunc<o_fn>( this, 192, curtime );
	}

	IKContext* GetIKContext( ) {
		return *( IKContext** )( ( uintptr_t )this + 9836 + 0x4 );
	}

	IKContext*& GetIKContext2( ) {
		return *( IKContext** )( ( uintptr_t )this + 0x2670 );
	}

	CStudioHdr* GetModelPtr( )
	{
		if (CStudioHdr* pStudioHdr = this->GetStudioHdr(); pStudioHdr == nullptr)
		{
			using LockStudioHdr_t = void(__thiscall*)(decltype(this));
			if (this->GetModel() != nullptr)
				reinterpret_cast<LockStudioHdr_t>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 51 53 8B D9 56 57 8D B3")));
			else
				return nullptr;
		}
		else if (pStudioHdr != nullptr)
			return pStudioHdr;

		return nullptr;

		/*
		using LockStudioHdr_t = void( __thiscall* )( decltype( this ) );

		if ( !GetStudioHdr( ) )
			reinterpret_cast< LockStudioHdr_t >( util::FindSignature( "client.dll", "55 8B EC 51 53 8B D9 56 57 8D B3" ) );

		return GetStudioHdr( );
		*/
	}

	// normal netvars
	ADD_NETVAR(GetShotsFired, int, "CCSPlayer->m_iShotsFired");
	ADD_NETVAR(GetMoney, int, "CCSPlayer->m_iAccount");
	ADD_NETVAR(GetTotalHits, int, "CCSPlayer->m_totalHitsOnServer");
	ADD_NETVAR(GetArmor, int, "CCSPlayer->m_ArmorValue");
	ADD_NETVAR(GetEyeAngles, Vector, "CCSPlayer->m_angEyeAngles");
	ADD_NETVAR(IsDefusing, bool, "CCSPlayer->m_bIsDefusing");
	ADD_NETVAR(IsScoped, bool, "CCSPlayer->m_bIsScoped");
	ADD_NETVAR(IsResumingScope, bool, "CCSPlayer->m_bResumeZoom");
	ADD_NETVAR(IsGrabbingHostage, bool, "CCSPlayer->m_bIsGrabbingHostage");
	ADD_NETVAR(IsRescuing, bool, "CCSPlayer->m_bIsRescuing");
	ADD_NETVAR(HasHelmet, bool, "CCSPlayer->m_bHasHelmet");
	ADD_NETVAR(HasHeavyArmor, bool, "CCSPlayer->m_bHasHeavyArmor");
	ADD_NETVAR(HasDefuser, bool, "CCSPlayer->m_bHasDefuser");
	ADD_NETVAR(HasImmunity, bool, "CCSPlayer->m_bGunGameImmunity");
	ADD_NETVAR(IsInBuyZone, bool, "CCSPlayer->m_bInBuyZone");
	ADD_NETVAR(GetLowerBodyYaw, float, "CCSPlayer->m_flLowerBodyYawTarget");
	ADD_NETVAR(GetSurvivalTeam, int, "CCSPlayer->m_nSurvivalTeam");
	ADD_NETVAR(GetFlashDuration, float, "CCSPlayer->m_flFlashDuration");

	// pointer netvars
	ADD_NETVAR(GetFlashMaxAlpha, float, "CCSPlayer->m_flFlashMaxAlpha");

	// offset variables
	ADD_NETVAROFFSET(GetFlashAlpha, float, "CCSPlayer->m_flFlashMaxAlpha", -0x8);
	ADD_NETVAROFFSET(GetGlowIndex, int, "CCSPlayer->m_flFlashDuration", 0x18);
	ADD_NETVAROFFSET(IsUsedNewAnimState, int, "CCSPlayer->m_flLastExoJumpTime", 0x8);
	ADD_NETVAROFFSET(GetSpawnTime, float, "CCSPlayer->m_iAddonBits", -0x4); // @ida: client.dll @ [89 86 ? ? ? ? E8 ? ? ? ? 80 BE ? ? ? ? ? + 0x2]


	inline bool IsArmored(const int iHitGroup) {
		// @ida isarmored: server.dll @ 55 8B EC 32 D2

		bool bIsArmored = false;

		if (this->GetArmor() > 0)
		{
			switch (iHitGroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case HITGROUP_NECK:
				bIsArmored = true;
				break;
			case HITGROUP_HEAD:
				if (this->HasHelmet())
					bIsArmored = true;
				[[fallthrough]];
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				if (this->HasHeavyArmor())
					bIsArmored = true;
				break;
			default:
				break;
			}
		}

		return bIsArmored;
	}

	ADD_NETVAR(GetAnimationTime, float, "CBaseEntity->m_flAnimTime");
	ADD_NETVAR(GetSimulationTime, float, "CBaseEntity->m_flSimulationTime");
	ADD_NETVAR(GetVecOrigin, Vector, "CBaseEntity->m_vecOrigin");
	ADD_NETVAR(GetRotation, Vector, "CBaseEntity->m_angRotation");
	ADD_NETVAR(GetEffects, int, "CBaseEntity->m_fEffects");
	ADD_NETVAR(GetTeam, int, "CBaseEntity->m_iTeamNum");
	ADD_NETVAR(GetOwnerEntityHandle, CBaseHandle, "CBaseEntity->m_hOwnerEntity");
	ADD_NETVAR(GetCollisionGroup, int, "CBaseEntity->m_CollisionGroup");

	ADD_PNETVAR(IsSpotted, bool, "CBaseEntity->m_bSpotted");
	ADD_PNETVAR(GetCollision, ICollideable, "CBaseEntity->m_Collision");

	ADD_NETVAROFFSET(GetOldSimulationTime, float, "CBaseEntity->m_flSimulationTime", 0x4);
	ADD_NETVAROFFSET(GetMoveType, int, "CBaseEntity->m_nRenderMode", 0x1);

	ADD_NETVAR( vecMaxs, Vector, "CBaseEntity->m_vecMaxs" );
	ADD_NETVAR( vecMins, Vector, "CBaseEntity->m_vecMins" );

	ADD_NETVAR(GetNextAttack, float, "CBaseCombatCharacter->m_flNextAttack");
	ADD_NETVAR(GetActiveWeaponHandle, CBaseHandle, "CBaseCombatCharacter->m_hActiveWeapon");

	__forceinline std::array<int, 64>& GetWeaponsHandle()
	{
		static auto offset = n::netvars[fnv::HashConst("CBaseCombatCharacter->m_hMyWeapons")].uOffset;
		return *reinterpret_cast<std::array<int, 64>*>(uint32_t(this) + offset);
	}
	//ADD_PNETVAR(GetWeaponsHandle, CBaseHandle, "CBaseCombatCharacter->m_hMyWeapons");

	ADD_OFFSET(GetStudioHdr, CStudioHdr*, 0x2950);
	ADD_OFFSET(GetRecentModelBoneCounter, unsigned long, 0x2690);
	ADD_OFFSET(GetLastSetupBonesTime, float, 0x2924);
	//ADD_OFFSET(GeFinalPredictedTick, int, 0x3434);
	ADD_OFFSET(GetOcclusionFrameCount, int, 0xA30);
	ADD_OFFSET(GetOcclusionFlags, int, 0xA28);
	ADD_OFFSET(GetClientEffects, int, 0x68);
	ADD_OFFSET(GetLastSkipFrameCount, int, 0xA68);
	ADD_OFFSET(MaintainSequenceTransition, bool, 0x9F0);
	ADD_OFFSET(InverseKinematics, LPVOID, 0x2670);

	ADD_PNETVAR(GetWearablesHandle, CBaseHandle, "CBaseCombatCharacter->m_hMyWearables");

	ADD_NETVAR(GetSequence, int, "CBaseAnimating->m_nSequence");
	ADD_NETVAROFFSET(GetBoneAccessor, CBoneAccessor, "CBaseAnimating->m_nForceBone", 0x18);
	ADD_NETVAR(GetHitboxSet, int, "CBaseAnimating->m_nHitboxSet");
	ADD_NETVAR(IsClientSideAnimation, bool, "CBaseAnimating->m_bClientSideAnimation");
	ADD_NETVAR(GetCycle, float, "CBaseAnimating->m_flCycle");

	//ADD_PNETVAROFFSET(GetStudioHdr, CStudioHdr, "CBaseAnimating->m_hLightingOrigin", 0x8);
	ADD_NETVAROFFSET(GetCustomBlendingRuleMask, int, "CBaseAnimating->m_nBody", 0x4);
	ADD_NETVAROFFSET(GetAnimationLODFlags, unsigned int, "CBaseAnimating->m_nBody", 0x8);
	ADD_NETVAROFFSET(GetOldAnimationLODFlags, unsigned int, "CBaseAnimating->m_nBody", 0xC);
	ADD_NETVAROFFSET(GetComputedAnimationLODFrame, unsigned int, "CBaseAnimating->m_nBody", 0x10);

	ADD_NETVAROFFSET( GetPrevBoneMask, int, "CBaseAnimating->m_nForceBone", 0x10 );
	ADD_NETVAROFFSET( GetAccumulatedBoneMask, int, "CBaseAnimating->m_nForceBone", 0x14 );
	//ADD_NETVAROFFSET(GetRecentModelBoneCounter, unsigned long, "CBaseAnimating->m_nForceBone", 0x4);
	//ADD_NETVAROFFSET(GetLastSetupBonesTime, unsigned long, "CBaseAnimating->m_nForceBone", -0x20);

	Vector& m_angVisualAngles()
	{
		return *(Vector*)((DWORD)(this) + 0x31E8);
	}

	std::array<float, 24>& GetPoseParameter() {

		static int _m_flPoseParameter = n::netvars[fnv::HashConst("CBaseAnimating->m_flPoseParameter")].uOffset;
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}

	// pattern::find( m_client_dll, ( "55 8b ec 56 8b f1 83 be ? ? ? ? ? 75 ? 8b 46 ? 8d 4e ? ff 50 ? 85 c0 74 ? 8b ce e8 ? ? ? ? 8b b6 ? ? ? ? 85 f6 74 ? 83 3e ? 74 ? 8b ce e8 ? ? ? ? 84 c0 74 ? ff 75" ) ).as<uintptr_t>( );
	int LookupSequence(const char* name) {

		static uintptr_t sig = (uintptr_t)MEM::FindPattern(CLIENT_DLL, XorStr("55 8b ec 56 8b f1 83 be ? ? ? ? ? 75 ? 8b 46 ? 8d 4e ? ff 50 ? 85 c0 74 ? 8b ce e8 ? ? ? ? 8b b6 ? ? ? ? 85 f6 74 ? 83 3e ? 74 ? 8b ce e8 ? ? ? ? 84 c0 74 ? ff 75"));
		return ((int(__thiscall*)(CBaseEntity*, const char*))sig)(this, name);
	}

	inline void SetPoseAngles(float flYaw, float flPitch) {

		auto& arrPose = this->GetPoseParameter();
		arrPose.at(11U) = (flPitch + 90.f) / 180.f;
		arrPose.at(2U) = (flYaw + 180.f) / 360.f;
	}
	
	void BuildTransformations( CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& transform, int mask, uint8_t* computed ) {

		using BuildTransformations_t = void( __thiscall* )( decltype( this ), CStudioHdr*, Vector*, Quaternion*, matrix3x4_t const&, int, uint8_t* );
		return util::GetVFunc< BuildTransformations_t >( this, 190 )( this, hdr, pos, q, transform, mask, computed );
	}

	void StandardBlendingRules( CStudioHdr* hdr, Vector* pos, Quaternion* q, float time, int mask ) {

		using StandardBlendingRules_t = void( __thiscall* )( decltype( this ), CStudioHdr*, Vector*, Quaternion*, float, int );
		return util::GetVFunc< StandardBlendingRules_t >( this, 206 )( this, hdr, pos, q, time, mask );
	}

	void ClampBonesInBBox(matrix3x4_t* bones, int boneMask) {

		using ClampBonesInBBox_t = void(__thiscall*)(decltype(this), matrix3x4_t*, int);
		static auto oClampBonesInBBox = reinterpret_cast<ClampBonesInBBox_t>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38")));
		return oClampBonesInBBox(this, bones, boneMask);
		//return util::GetVFunc< ClampBonesInBBox_t >(this, 206)(this, pMatrix, iMask);
	}

	int& GetSimulationTick( ) {
		return *reinterpret_cast< int* >( reinterpret_cast< std::uintptr_t >( this ) + 0x2AC );
	}

	CAnimationLayer* GetAnimationOverlays() {
		
		static const std::uintptr_t uAnimationOverlaysOffset = *reinterpret_cast<std::uintptr_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("8B 80 ? ? ? ? 8D 34 C8")) + 2);
		return *reinterpret_cast<CAnimationLayer**>(uintptr_t(this) + uAnimationOverlaysOffset);
	}

	int IsMaxHealth() {
		// @ida: client.dll @ [FF 90 ? ? ? ? 85 C0 0F 8F ? ? ? ? 80 + 0x2] / sizeof(std::uintptr_t)
		return util::CallVFunc<int>(this, 123);
	}

	void Think() {
		// @ida: client.dll @ [FF 90 ? ? ? ? FF 35 ? ? ? ? 8B 4C + 0x2] / sizeof(std::uintptr_t)
		util::CallVFunc<void>(this, 139);
	}

	const char* GetClassname() {
		// @ida: client.dll @ [8B 01 FF 90 ? ? ? ? 90 + 0x4] / sizeof(std::uintptr_t)
		return util::CallVFunc<const char*>(this, 143);
	}

	void SetModelIndex(int index) {
		return util::GetVFunc< void(__thiscall*)(void*, int) >(this, 75)(this, index);
	}

	unsigned int PhysicsSolidMaskForEntity() {
		// @xref: "func_breakable", "func_breakable_surf"
		return util::CallVFunc<unsigned int>(this, 152);
	}

	[[nodiscard]] Vector GetEyePosition(bool bShouldCorrect = true) {
		Vector vecPosition = { };

		// get eye position this shit crashes for no reason kys
		util::CallVFunc<void>(this, 169, std::ref(vecPosition));

		// correct this like it do weapon_shootpos
		// @ida weapon_shootpos: 55 8B EC 56 8B 75 08 57 8B F9 56 8B 07 FF 90
		if (IsUsedNewAnimState() && bShouldCorrect)
		{
			if (CAnimState* pAnimState = this->AnimState(); pAnimState != nullptr)
				ModifyEyePosition(pAnimState, &vecPosition);
		}

		// return corrected position
		return vecPosition;
	}

	void SetSequence(int iSequence) {
		// @ida: FF 90 ? ? ? ? 8B 07 8B CF FF 90 ? ? ? ? 8B CF + 0x2
		util::CallVFunc<void>(this, 219, iSequence);
	}

	void StudioFrameAdvance() {
		// @ida: FF 90 ? ? ? ? 8B 07 8B CF FF 90 ? ? ? ? 8B CF + 0xC
		util::CallVFunc<void>(this, 220);
	}

	void UpdateClientSideAnimations() {
		// @xref: "UpdateClientSideAnimations"
		// @ida updateclientsideanimations: 55 8B EC 51 56 8B F1 80 BE ? ? ? ? 00 74 ? 8B 06 FF
		util::CallVFunc<void>(this, 224);
	}

	void PreThink() {
		util::CallVFunc<void>(this, 318);
	}

	void UpdateCollisionBounds() {
		util::CallVFunc<void>(this, 340);
	}

	void SetCollisionBounds(Vector vecMins, Vector vecMaxs) {

		static auto sig = (void*)((DWORD)(MEM::FindPattern(CLIENT_DLL, XorStr("53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 18 56 57 8B 7B"))));
		((void(__thiscall*)(void*, Vector*, Vector*))(sig))((void*)((DWORD)(this) + 0x320), &vecMins, &vecMaxs);
	}

	bool PhysicsRunThink(int nThinkMethod) {

		using PhysicsRunThinkFn = bool(__thiscall*)(void*, int);
		static auto oPhysicsRunThink = reinterpret_cast<PhysicsRunThinkFn>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 10 53 56 57 8B F9 8B 87")));
		if (!oPhysicsRunThink)
			return false;
		
		return oPhysicsRunThink(this, nThinkMethod);
	}

	static CBaseEntity*		GetLocalPlayer();
	int						GetSequenceActivity(int iSequence);
	void					SetUpMovement();
	void					SetUpVelocity();
	bool					IsGrenade(CBaseCombatWeapon* weapon);
	CBaseCombatWeapon*		GetWeapon();
	int						GetMaxHealth();
	std::optional<Vector>	GetBonePosition(int iBone);
	int						GetBoneByHash(const uint32_t uBoneHash) const;
	std::optional<Vector>	GetHitboxPosition(const int iHitbox);									// current matrix
	std::optional<Vector>	GetHitboxPosition(const int iHitbox, Vector& vecMins, Vector& vecMaxs, float& flRadius);
	Vector					GetHitboxPosition(int hitbox, matrix3x4_t matrix[128]);					// selected matrix
	Vector					GetHitboxPosition(int hitbox, matrix3x4_t matrix[128], float& flRadius, mstudiobbox_t* pStudioBox = nullptr);// selected matrix + radius
	Vector					GetHitboxPosition(int hitbox, matrix3x4_t matrix[128], Vector& vecMins, Vector& vecMaxs, float& flRadius);
	std::optional<Vector>	GetHitGroupPosition(const int iHitGroup);
	void					ModifyEyePosition(const CAnimState* pAnimState, Vector* vecPosition) const;
	void					PostThink();
	bool					IsEnemy(CBaseEntity* pEntity);
	bool					IsTargetingLocal(CBaseEntity* pLocal);
	bool					CanShoot(CWeaponCSBase* pBaseWeapon);
	bool					IsVisible(CBaseEntity* pEntity, const Vector& vecEnd, bool bSmokeCheck = false);
	bool					IsBreakable();
	bool					SetupBonesFix( CBaseEntity* target, int boneMask, float currentTime, matrix3x4_t* pBoneToWorldOut );
	void					InvalidateBoneCache();
	float					GetSequenceCycleRate(CStudioHdr*, int);
	float					GetSequenceMoveDist(CStudioHdr*, int);
	float					GetLayerSequenceCycleRate(CAnimationLayer*, int);
	bool					InitializeAsClientEntity(const char* pszModelName, bool bRenderWithViewModels);
	bool					IsFakeducking();

	/*    
	N_ADD_VARIABLE(int, GetSequence, "CBaseAnimating->m_nSequence");
    N_ADD_PVARIABLE_OFFSET(CBoneAccessor, GetBoneAccessor, "CBaseAnimating->m_nForceBone", 0x1C);
    N_ADD_VARIABLE(int, GetHitboxSet, "CBaseAnimating->m_nHitboxSet");
    N_ADD_VARIABLE(bool, IsClientSideAnimation, "CBaseAnimating->m_bClientSideAnimation");
    N_ADD_VARIABLE(float, GetCycle, "CBaseAnimating->m_flCycle");
    N_ADD_OFFSET(CStudioHdr*, GetStudioHdr, 0x2950);
    N_ADD_OFFSET(int, GetOcclusionFrameCount, 0xA30);
    N_ADD_OFFSET(int, GetOcclusionFlags, 0xA28);
    N_ADD_OFFSET(unsigned long, GetRecentModelBoneCounter, 0x2690);
    N_ADD_OFFSET(float, GetLastSetupBonesTime, 0x2924);
    N_ADD_OFFSET(int, GetClientEffects, 0x68);
    N_ADD_OFFSET(int, GetLastSkipFrameCount, 0xA68);
    N_ADD_OFFSET(bool&, JiggleBones, 0x2930);
    N_ADD_OFFSET(bool&, MaintainSequenceTransition, 0x9F0);
    N_ADD_OFFSET(LPVOID&, InverseKinematics, 0x2670);
    N_ADD_OFFSET(int&, GeFinalPredictedTick, 0x3434);
	*/
};

enum EWeaponType : int {
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL = 1,
	WEAPONTYPE_SUBMACHINEGUN = 2,
	WEAPONTYPE_RIFLE = 3,
	WEAPONTYPE_SHOTGUN = 4,
	WEAPONTYPE_SNIPER = 5,
	WEAPONTYPE_MACHINEGUN = 6,
	WEAPONTYPE_C4 = 7,
	WEAPONTYPE_PLACEHOLDER = 8,
	WEAPONTYPE_GRENADE = 9,
	WEAPONTYPE_HEALTHSHOT = 11,
	WEAPONTYPE_FISTS = 12,
	WEAPONTYPE_BREACHCHARGE = 13,
	WEAPONTYPE_BUMPMINE = 14,
	WEAPONTYPE_TABLET = 15,
	WEAPONTYPE_MELEE = 16
};

class CCSWeaponData
{
public:
	std::byte pad0[0x14];			// 0x0000
	int iMaxClip1;					// 0x0014
	int iMaxClip2;					// 0x0018
	int iDefaultClip1;				// 0x001C
	int iDefaultClip2;				// 0x0020
	int iPrimaryMaxReserveAmmo;		// 0x0024
	int iSecondaryMaxReserveAmmo;	// 0x0028
	const char* szWorldModel;		// 0x002C
	const char* szViewModel;		// 0x0030
	const char* szDroppedModel;		// 0x0034
	std::byte pad1[0x50];			// 0x0038
	const char* szHudName;			// 0x0088
	const char* szWeaponName;		// 0x008C
	std::byte pad2[0x2];			// 0x0090
	bool bIsMeleeWeapon;			// 0x0092
	std::byte pad3[0x9];			// 0x0093
	float flWeaponWeight;			// 0x009C
	std::byte pad4[0x4];			// 0x00A0
	int iSlot;						// 0x00A4
	int iPosition;					// 0x00A8
	std::byte pad5[0x1C];			// 0x00AC
	int nWeaponType;				// 0x00C8
	std::byte pad6[0x4];			// 0x00CC
	int iWeaponPrice;				// 0x00D0
	int iKillAward;					// 0x00D4
	const char* szAnimationPrefix;	// 0x00D8
	float flCycleTime;				// 0x00DC
	float flCycleTimeAlt;			// 0x00E0
	std::byte pad8[0x8];			// 0x00E4
	bool bFullAuto;					// 0x00EC
	std::byte pad9[0x3];			// 0x00ED
	int iDamage;					// 0x00F0
	float flHeadShotMultiplier;		// 0x00F4
	float flArmorRatio;				// 0x00F8
	int iBullets;					// 0x00FC
	float flPenetration;			// 0x0100
	std::byte pad10[0x8];			// 0x0104
	float flRange;					// 0x010C
	float flRangeModifier;			// 0x0110
	float flThrowVelocity;			// 0x0114
	std::byte pad11[0xC];			// 0x0118
	bool bHasSilencer;				// 0x0124
	std::byte pad12[0xF];			// 0x0125
	float flMaxSpeed[2];			// 0x0134
	std::byte pad13[0x4];			// 0x013C
	float flSpread[2];				// 0x0140
	float flInaccuracyCrouch[2];	// 0x0148
	float flInaccuracyStand[2];		// 0x0150
	std::byte pad14[0x8];			// 0x0158
	float flInaccuracyJump[2];		// 0x0160
	float flInaccuracyLand[2];		// 0x0168
	float flInaccuracyLadder[2];	// 0x0170
	float flInaccuracyFire[2];		// 0x0178
	float flInaccuracyMove[2];		// 0x0180
	float flInaccuracyReload;		// 0x0188
	int iRecoilSeed;				// 0x018C
	float flRecoilAngle[2];			// 0x0190
	float flRecoilAngleVariance[2];	// 0x0198
	float flRecoilMagnitude[2];		// 0x01A0
	float flRecoilMagnitudeVariance[2]; // 0x01A8
	int iSpreadSeed;				// 0x01B0

	bool IsGun() const
	{
		switch (this->nWeaponType)
		{
		case WEAPONTYPE_PISTOL:
		case WEAPONTYPE_SUBMACHINEGUN:
		case WEAPONTYPE_RIFLE:
		case WEAPONTYPE_SHOTGUN:
		case WEAPONTYPE_SNIPER:
		case WEAPONTYPE_MACHINEGUN:
			return true;
		}

		return false;
	}
};

class IRefCounted;
class CEconItemView
{
public:

	CUtlVector<IRefCounted*> GetCustomMaterials() {

		return *reinterpret_cast<CUtlVector<IRefCounted*>*>(uintptr_t(this) + 0x18);
	}

	CUtlVector<CRefCounted*>& GetVisualsDataProcessors() {

		// @xref: "Original material not found! Name: %s"
		static const std::uintptr_t uVisualsDataProcessorsOffset = *reinterpret_cast<std::uintptr_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("81 C7 ? ? ? ? 8B 4F 0C 8B 57 04 89 4C")) + 0x2);
		return *reinterpret_cast<CUtlVector<CRefCounted*>*>(reinterpret_cast<std::uintptr_t>(this) + uVisualsDataProcessorsOffset);
	}
};

class CBaseCombatWeapon : public IClientEntity
{
public:

	ADD_NETVAR(GetNextPrimaryAttack, float, "CBaseCombatWeapon->m_flNextPrimaryAttack");
	ADD_NETVAR(GetNextSecondaryAttack, float, "CBaseCombatWeapon->m_flNextSecondaryAttack");
	ADD_NETVAR(GetAmmo, int, "CBaseCombatWeapon->m_iClip1");
	ADD_NETVAR(GetAmmoReserve, int, "CBaseCombatWeapon->m_iPrimaryReserveAmmoCount");
	ADD_NETVAR(GetViewModelIndex, int, "CBaseCombatWeapon->m_iViewModelIndex");
	ADD_NETVAR(GetWorldModelIndex, int, "CBaseCombatWeapon->m_iWorldModelIndex");
	ADD_NETVAR(GetWorldModelHandle, CBaseHandle, "CBaseCombatWeapon->m_hWeaponWorldModel");
	ADD_NETVAR(GetLastShotTime, float, "CWeaponCSBase->m_fLastShotTime");
	ADD_NETVAR(GetFireReadyTime, float, "CWeaponCSBase->m_flPostponeFireReadyTime");

	//N_ADD_DATAFIELD(bool, IsReloading, this->GetPredictionDescMap(), "m_bInReload");
	ADD_NETVAR(GetModelIndex, int, "CBaseViewModel->m_nModelIndex");
	ADD_NETVAR(GetItemDefinitionIndex, short, "CBaseAttributableItem->m_iItemDefinitionIndex");
	ADD_NETVAR(GetItemIDHigh, int, "CBaseAttributableItem->m_iItemIDHigh");
	ADD_NETVAR(GetItemIDLow, int, "CBaseAttributableItem->m_iItemIDLow");
	ADD_NETVAR(GetAccountID, int, "CBaseAttributableItem->m_iAccountID");
	ADD_NETVAR(GetEntityQuality, int, "CBaseAttributableItem->m_iEntityQuality");
	ADD_PNETVAR(GetCustomName, char, "CBaseAttributableItem->m_szCustomName");
	ADD_NETVAR(GetOwnerXuidLow, int, "CBaseAttributableItem->m_OriginalOwnerXuidLow");
	ADD_NETVAR(GetOwnerXuidHigh, int, "CBaseAttributableItem->m_OriginalOwnerXuidHigh");
	ADD_NETVAR(GetFallbackPaintKit, int, "CBaseAttributableItem->m_nFallbackPaintKit");
	ADD_NETVAR(GetFallbackSeed, int, "CBaseAttributableItem->m_nFallbackSeed");
	ADD_NETVAR(GetFallbackWear, float, "CBaseAttributableItem->m_flFallbackWear");
	ADD_NETVAR(GetFallbackStatTrak, int, "CBaseAttributableItem->m_nFallbackStatTrak");
	ADD_PNETVAR(GetEconItemView, CEconItemView, "CBaseAttributableItem->m_Item");

	ADD_PNETVAR(GetFireDeltaX, int, "CInferno->m_fireXDelta");
	ADD_PNETVAR(GetFireDeltaY, int, "CInferno->m_fireYDelta");
	ADD_PNETVAR(GetFireDeltaZ, int, "CInferno->m_fireZDelta");

	ADD_PNETVAR(IsFireBurning, bool, "CInferno->m_bFireIsBurning");
	ADD_NETVAR(GetFireCount, int, "CInferno->m_fireCount");
	ADD_NETVAR(GetFireBeginTick, int, "CInferno->m_nFireEffectTickBegin");

	ADD_DATAMAP(GetActivity, int, this->GetPredictionDescMap(), "m_Activity");

	bool IsGrenade() {

		if (auto pData = this->GetCSWpnData(); pData)
			return pData->nWeaponType == WEAPONTYPE_GRENADE;
		else
			return false;
	}

	bool IsKnife() {

		if (!this)
			return false;

		int idx = this->GetItemDefinitionIndex();
		return idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
			|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET
			|| idx == WEAPON_KNIFE_PUSH || idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL
			|| idx == WEAPON_KNIFE_GG || idx == WEAPON_KNIFE_GHOST || idx == WEAPON_KNIFE_GYPSY_JACKKNIFE || idx == WEAPON_KNIFE_STILETTO
			|| idx == WEAPON_KNIFE_URSUS || idx == WEAPON_KNIFE_WIDOWMAKER || idx == WEAPON_KNIFE_CSS || idx == WEAPON_KNIFE_CANIS
			|| idx == WEAPON_KNIFE_CORD || idx == WEAPON_KNIFE_OUTDOOR || idx == WEAPON_KNIFE_SKELETON;
	}

	void SetModelIndex(int nModelIndex) {
		util::CallVFunc<void>(this, 75, nModelIndex);
	}

	bool IsWeapon() {
		return util::CallVFunc<bool>(this, 166);
	}

	[[nodiscard]] float GetSpread() {
		return util::CallVFunc<float>(this, 453);
	}

	[[nodiscard]] float GetInaccuracy() {
		return util::CallVFunc<float>(this, 483);
	}

	CCSWeaponInfo* GetCSWpnData() {
		return i::WeaponSystem->GetWpnData(this->GetItemDefinitionIndex());
		// i::WeaponSystem->GetWpnData(pWeapon->GetItemDefinitionIndex());
	}

	ADD_OFFSET( CustomMaterialInitialized, bool, 0x32DD );
	ADD_OFFSET( GetCustomMaterials, CUtlVector<IRefCounted*>, 0x14 );
	ADD_OFFSET( GetCustomMaterials2, CUtlVector<IRefCounted*>, 0x9dcu );

	CUtlVector<CRefCounted*>& GetVisualsDataProcessors( )
	{
		// @xref: "Original material not found! Name: %s"
		static const std::uintptr_t uVisualsDataProcessorsOffset = *reinterpret_cast<std::uintptr_t*>(MEM::FindPattern(CLIENT_DLL, XorStr("81 C7 ? ? ? ? 8B 4F 0C 8B 57 04 89 4C")) + 0x2);
		return *reinterpret_cast< CUtlVector<CRefCounted*>* >( reinterpret_cast< std::uintptr_t >( this ) + uVisualsDataProcessorsOffset );
	}
};

class CTEFireBullets
{
public:
	std::byte	pad0[0x10];				//0x0000
	int			nPlayer;				//0x0010
	int			nItemDefinitionIndex;	//0x0014
	Vector		vecOrigin;				//0x0018
	Vector		vecAngles;				//0x0024
	int			iWeapon;				//0x0030
	int			nWeaponID;				//0x0034
	int			iMode;					//0x0038
	int			nSeed;					//0x003C
	float		flInaccuracy;			//0x0040
	float		flRecoilIndex;			//0x0044
	float		flSpread;				//0x0048
	int			nSoundType;				//0x004C
}; // Size: 0x0050

class CBaseViewModel;
class CWeaponCSBase : public CBaseCombatWeapon {
public:

	ADD_NETVAR(GetZoomLevel, int, "CWeaponCSBaseGun->m_zoomLevel");
	ADD_NETVAR(GetBurstShotsRemaining, int, "CWeaponCSBaseGun->m_iBurstShotsRemaining");

	ADD_NETVAR(IsBurstMode, bool, "CWeaponCSBase->m_bBurstMode");
	ADD_NETVAR(GetAccuracyPenalty, float, "CWeaponCSBase->m_fAccuracyPenalty");
	ADD_NETVAR(GetRecoilIndex, float, "CWeaponCSBase->m_flRecoilIndex" );

	void OnFireEvent(CBaseViewModel* pViewModel, const Vector& vecOrigin, const Vector& vecAngles, int iEvent, const char* szOptions) {
		util::CallVFunc<bool>(this, 59, pViewModel, vecOrigin, vecAngles, iEvent, szOptions);
	}
};

class CBaseCSGrenade : public CWeaponCSBase {
public:

	ADD_NETVAR(IsPinPulled, bool, "CBaseCSGrenade->m_bPinPulled");
	ADD_NETVAR(GetThrowTime, float, "CBaseCSGrenade->m_fThrowTime");
	ADD_NETVAR(GetThrowStrength, float, "CBaseCSGrenade->m_flThrowStrength");

	ADD_NETVAR(GetEffectTickBegin, int, "CBaseCSGrenadeProjectile->m_nExplodeEffectTickBegin");

};

class CSmokeGrenade {
public:

	ADD_NETVAR(GetEffectTickBegin, int, "CSmokeGrenadeProjectile->m_nSmokeEffectTickBegin");

	inline float GetMaxTime() {
		return 18.f;
	}
};

class CInferno {
public:

	ADD_NETVAR(GetEffectTickBegin, int, "CInferno->m_nFireEffectTickBegin");

	inline float GetMaxTime() {
		// @todo: get with inferno_flame_lifetime convar
		return 7.f;
	}
};

class CPlantedC4 {
public:

	ADD_NETVAR(GetBlowTime, float, "CPlantedC4->m_flC4Blow");
	ADD_NETVAR(GetTimerLength, float, "CPlantedC4->m_flTimerLength");
	ADD_NETVAR(GetDefuseLength, float, "CPlantedC4->m_flDefuseLength");
	ADD_NETVAR(GetDefuseCountDown, float, "CPlantedC4->m_flDefuseCountDown");
	ADD_NETVAR(IsPlanted, bool, "CPlantedC4->m_bBombTicking");
	ADD_NETVAR(GetDefuserHandle, CBaseHandle, "CPlantedC4->m_hBombDefuser");
	ADD_NETVAR(IsDefused, bool, "CPlantedC4->m_bBombDefused");

	inline float GetTimer(const float flServerTime) {
		return std::clamp(this->GetBlowTime() - flServerTime, 0.0f, this->GetTimerLength());
	}

	inline float GetDefuseTimer(const float flServerTime) {
		return std::clamp(this->GetDefuseCountDown() - flServerTime, 0.0f, this->GetDefuseLength());
	}
};

class CBaseViewModel {
public:

	ADD_NETVAR(GetModelIndex, int, "CBaseViewModel->m_nModelIndex");
	ADD_NETVAR(GetOwnerHandle, CBaseHandle, "CBaseViewModel->m_hOwner");
	ADD_NETVAR(GetWeaponHandle, CBaseHandle, "CBaseViewModel->m_hWeapon");

	ADD_NETVAR( iSequence, int, "CBaseViewModel->m_nSequence" );
	ADD_NETVAR( iAnimationParity, int, "CBaseViewModel->m_nAnimationParity" );
	
	// fix this yourself bozo
	//ADD_NETVAR( flCycle, float&, "CCSPlayer->m_flCycle" );
	//ADD_NETVAR( flAnimTime, float&, "CCSPlayer->m_flAnimTime" );

	void SendViewModelMatchingSequence(int nSequence) {
		util::CallVFunc<void>(this, 247, nSequence);
	}

	void SetWeaponModel(const char* szFileName, CBaseCombatWeapon* pWeapon) {
		// @ida setweaponmodel: 57 8B F9 8B 97 ? ? ? ? 83 FA FF 74 6A
		util::CallVFunc<void>(this, 248, szFileName, pWeapon);
	}
};

class CEnvTonemapController {
public:

	ADD_NETVAR(IsUseCustomAutoExposureMin, bool, "CEnvTonemapController->m_bUseCustomAutoExposureMin");
	ADD_NETVAR(IsUseCustomAutoExposureMax, bool, "CEnvTonemapController->m_bUseCustomAutoExposureMax");
	ADD_NETVAR(IsUseCustomBloomScale, bool, "CEnvTonemapController->m_bUseCustomBloomScale");
	ADD_NETVAR(GetCustomAutoExposureMin, float, "CEnvTonemapController->m_flCustomAutoExposureMin");
	ADD_NETVAR(GetCustomAutoExposureMax, float, "CEnvTonemapController->m_flCustomAutoExposureMax");
	ADD_NETVAR(GetCustomBloomScale, float, "CEnvTonemapController->m_flCustomBloomScale");
	ADD_NETVAR(GetCustomBloomScaleMin, float, "CEnvTonemapController->m_flCustomBloomScaleMinimum");
	ADD_NETVAR(GetBloomExponent, float, "CEnvTonemapController->m_flBloomExponent");
	ADD_NETVAR(GetBloomSaturation, float, "CEnvTonemapController->m_flBloomSaturation");

};

class IBreakableWithPropData
{
public:
	// Damage modifiers
	virtual void		SetDmgModBullet(float flDmgMod) = 0;
	virtual void		SetDmgModClub(float flDmgMod) = 0;
	virtual void		SetDmgModExplosive(float flDmgMod) = 0;
	virtual float		GetDmgModBullet() = 0;
	virtual float		GetDmgModClub() = 0;
	virtual float		GetDmgModExplosive() = 0;
	virtual float		GetDmgModFire() = 0;

	// Explosive
	virtual void		SetExplosiveRadius(float flRadius) = 0;
	virtual void		SetExplosiveDamage(float flDamage) = 0;
	virtual float		GetExplosiveRadius() = 0;
	virtual float		GetExplosiveDamage() = 0;

	// Physics damage tables
	virtual void		SetPhysicsDamageTable(string_t iszTableName) = 0;
	virtual string_t	GetPhysicsDamageTable() = 0;

	// Breakable chunks
	virtual void		SetBreakableModel(string_t iszModel) = 0;
	virtual string_t 	GetBreakableModel() = 0;
	virtual void		SetBreakableSkin(int iSkin) = 0;
	virtual int			GetBreakableSkin() = 0;
	virtual void		SetBreakableCount(int iCount) = 0;
	virtual int			GetBreakableCount() = 0;
	virtual void		SetMaxBreakableSize(int iSize) = 0;
	virtual int			GetMaxBreakableSize() = 0;

	// LOS blocking
	virtual void		SetPropDataBlocksLOS(bool bBlocksLOS) = 0;
	virtual void		SetPropDataIsAIWalkable(bool bBlocksLOS) = 0;

	// Interactions
	virtual void		SetInteraction(EPropertyDataInteractions Interaction) = 0;
	virtual bool		HasInteraction(EPropertyDataInteractions Interaction) = 0;

	// Multiplayer physics mode
	virtual void		SetPhysicsMode(int iMode) = 0;
	virtual int			GetPhysicsMode() = 0;

	// Multiplayer breakable spawn behavior
	virtual void		SetMultiplayerBreakMode(EMultiplayerBreak mode) = 0;
	virtual EMultiplayerBreak GetMultiplayerBreakMode() const = 0;

	// Used for debugging
	virtual void		SetBasePropData(string_t iszBase) = 0;
	virtual string_t	GetBasePropData() = 0;
};

class CBreakableSurface : public CBaseEntity, public IBreakableWithPropData {
public:

	ADD_NETVAR(IsBroken, bool, "CBreakableSurface->m_bIsBroken");

};

class C4 : public CWeaponCSBase {

public:
	virtual void Spawn();
	virtual void WeaponReset();
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual void UpdateShieldState();
	virtual float GetMaxSpeed() const;
	virtual EItemDefinitionIndex GetCSWeaponID(void) const;
	virtual bool Deploy(void);
	virtual bool Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	virtual void OnDataChanged(void* type);
	virtual bool OnFireEvent(CBaseViewModel* pViewModel, const Vector& origin, const Vector& angles, int event, const char* options);
	virtual Vector GetGlowColor(void);
	virtual void Precache();
	virtual int UpdateTransmitState();
	virtual int ShouldTransmit(const void* pInfo);
	virtual void GetControlPanelInfo(int nPanelIndex, const char*& pPanelName);
	virtual unsigned int PhysicsSolidMaskForEntity(void) const;
	virtual bool ShouldRemoveOnRoundRestart();
	virtual void PhysicsTouchTriggers(const Vector* pPrevAbsOrigin = NULL);
	virtual void	OnPickedUp(void* pNewOwner);
	virtual void	Drop(const Vector& vecVelocity);
	virtual bool IsRemoveable(void);
};
