#pragma once
#include <basetsd.h>
#include <string>
#include <intrin.h>
#include "../Interface/interfaces.h"
#include "../Interface/Classes/CMsg.h"
#include "../../Dependecies/Minhook/minhook.h"
#include "../../Dependecies/Minhook/detour.hpp"
#include "../../Dependecies/Minhook/trampoline.h"
#include "../Features/Rage/Animations/LocalAnimation.h"
#include "../SDK/Menu/config.h"
#include "../SDK/X86RetSpoof.h"

#define adr(s) uintptr_t(s)
using namespace x86RetSpoof;

namespace table {

	inline constexpr auto D3DEndScene = 42;
	inline constexpr auto D3DReset = 16;

	inline constexpr auto createMove = 22;			// run most features here that is command related
	inline constexpr auto frameStageNotify = 37;	// run animation features / that need to be handled at every frame
	inline constexpr auto paintTraverse = 41;		// run ESP and other drawing related functions
	inline constexpr auto overrideView = 18;		// Thirdperson, fov etc...
	inline constexpr auto lockCursor = 67;
	inline constexpr auto drawModel = 29;
	inline constexpr auto drawModelMdl = 21;
	inline constexpr auto listLeaves = 6;
	inline constexpr auto getViewmodelFov = 35;
	inline constexpr auto isPaused = 50;
	inline constexpr auto writeUserCmd = 24;

	inline constexpr auto fireEvent = 9;

	inline constexpr auto processPacket = 39;
	inline constexpr auto sendNetMsg = 40;
	inline constexpr auto setChoked = 45;
	inline constexpr auto sendDatagram = 46;

	inline constexpr auto packetStart = 5;
	inline constexpr auto packetEnd = 6;
	inline constexpr auto temptEntities = 36;

	inline constexpr auto runCommand = 19;			// for tickbase fix and getting movehelper interface
	inline constexpr auto allocKeyValues = 2;		// fixing keyvalues error while fakelaging

	inline constexpr auto doPostScreenEffects = 44;		// fixing keyvalues error while fakelaging

	inline constexpr auto emitSound = 5;
	inline constexpr auto installStringTableCallback = 36;
}

namespace detour {

	// ImGui
	inline CDetourHook D3DEndScene;
	inline CDetourHook D3DReset;

	// CSGO related hooks
	inline CDetourHook allocKeyValues;		// keyvalues -> 2
	inline CDetourHook createMove;			// client -> 22
	inline CDetourHook frameStageNotify;	// client -> 37
	inline CDetourHook paintTraverse;		// panel  -> 41
	inline CDetourHook clMove;				// engine -> 55 8B EC 81 EC ? ? ? ? 53 56 8A F9
	inline CDetourHook overrideView;		// clientmode -> 18
	inline CDetourHook lockCursor;			// panel -> 67
	inline CDetourHook drawModel;			// studiorender -> 29
	inline CDetourHook drawModelMdl;		// MDLrender -> 21
	inline CDetourHook listLeaves;			// bsptree -> 6
	inline CDetourHook runCommand;			// prediction -> 19
	inline CDetourHook getViewmodelFov;		// clientmode -> 35
	inline CDetourHook isPaused;			// engineclient -> 90
	inline CDetourHook physicsSimulate;		// CBasePlayer::PhysicsSimulate(void);
	inline CDetourHook writeUserCmd;		// client -> 24
	inline CDetourHook fireEvent;			// gamevent -> 9
	inline CDetourHook doPostScreenEffects; // clientmode -> 44
	inline CDetourHook emitSound;			// client -> 55 8B EC 83 EC 4C 53 8B D9 8B
	inline CDetourHook drawViewmodel;
	inline CDetourHook playerMove;
	inline CDetourHook clampBonesInBBox;
	inline CDetourHook getColorModulation;
	inline CDetourHook getAlphaModulation;
	inline CDetourHook setText;
	inline CDetourHook levelShutDown;
	inline CDetourHook viewmodelSequenceProxy;
	inline CDetourHook renderView;			// viewrender -> 6
	inline CDetourHook verifyReturnAddress;

	inline CDetourHook voiceData;			// engine -> 55 8B EC 83 E4 F8 A1 ? ? ? ? 81 EC 84 01 00

	// netchannel table
	inline CDetourHook processPacket;		// netchannel -> 39
	inline CDetourHook sendNetMsg;			// netchannel -> 40
	inline CDetourHook setChoked;			// netchannel -> 45
	inline CDetourHook sendDatagram;		// netchannel -> 46

	/*  inetchannelhandler table */
	inline CDetourHook packetStart;			// inetchannelhandler -> 5
	inline CDetourHook packetEnd;			// inetchannelhandler -> 6
	inline CDetourHook temptEntities;		// inetchannelhandler -> 36

	// animation hooks
	inline CDetourHook skipAnimation;
	inline CDetourHook procedrualFoot;
	inline CDetourHook buildTransform;
	inline CDetourHook sequenceChange;
	inline CDetourHook isHltv;				// engineclient -> 93
	inline CDetourHook blendingRules;
	inline CDetourHook calculateView;
	inline CDetourHook modifyEyePosition;
	inline CDetourHook animationState;
	inline CDetourHook clientAnimation;
	inline CDetourHook setupBones;
	inline CDetourHook extraBoneProcessing;
	inline CDetourHook interpolateEntites;
	inline CDetourHook isFollowingEntity;
	inline CDetourHook estimateAbsVelocity;
	inline CDetourHook processMovement;
	inline CDetourHook setupAliveLoop;
	inline CDetourHook baseInterpolatePart;
}

namespace h {

	inline std::array<void*, 65> vecEstimateAbsVelocityHooks{};

	void SetupHooks();
	void DestroyHooks();

	// i dont want to check and everything every time I hook something so I made a function for it duh...
	inline void	HookTable(CDetourHook& hook, void* thisptr, size_t index, void* function) {

		if (!hook.Create(util::GetVFunc(thisptr, index), function))
			util::Print(XorStr("Failed to initialize "), std::to_string(index).c_str());
	}
	inline void	HookSignature(CDetourHook& hook, const wchar_t* dll, const char* pattern, void* function, int add = 0x0) {

		if (!hook.Create((void*)(MEM::FindPattern(dll, pattern) + add), function))
			util::Print(XorStr("Failed to initialize "), pattern);
	}

	long __stdcall		EndScene(IDirect3DDevice9*);
	HRESULT __stdcall	Reset(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

	void* __fastcall	hkAllocKeyValuesMemory(CKeyValuesSystem*, int, int);
	void __fastcall		hkCreateMoveProxy(IBaseClientDLL*, int, int, float, bool);
	void __fastcall		hkFrameStageNotify(void*, void*, EStage);
	void __fastcall		hkPaintTraverse(void*, int, unsigned int, bool, bool);
	void __cdecl		hkClMove(float, bool);
	void __fastcall		hkOverrideView(void*, void*, CViewSetup*);
	void __fastcall		hkLockCursor(ISurface*, int);
	void __fastcall		hkDrawModel(IStudioRender*, int, DrawModelResults_t*, const DrawModelInfo_t&, matrix3x4_t*, float*, float*, const Vector&, int);
	void __fastcall		hkDrawModelMDL(void*, int, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	int  __fastcall		hkListLeavesInBox(void*, int, const Vector&, const Vector&, unsigned short*, int);
	void __fastcall		hkRunCommand(void*, void*, CBaseEntity*, CUserCmd*, IMoveHelper*);
	float __fastcall	hkGetViewModelFov(void*, void*);
	bool __fastcall		hkWriteUserCmdDeltaToBuffer(void*, void*, int, bf_write*, int, int, bool);
	bool __fastcall		hkFireEvent( void*, void*, IGameEvent* );
	int __fastcall		hkDoPostScreenEffect(void*, int, CViewSetup*);
	void __fastcall		hkEmitSound(void*, int, IRecipientFilter&, int, int, const char*, HSOUNDSCRIPTHASH, const char*, float, int, float, int, int, const Vector*, const Vector*, CUtlVector<Vector>*, bool, float, int, int);
	bool __fastcall		hkShouldDrawViewmodel(void*, void*);
	void __fastcall		hkPlayerMove(void*, void*);
	void __fastcall		hkClampBonesInBBox(CBaseEntity*, void*, matrix3x4_t*, int);
	void __fastcall		hkGetColorModulation(void*, void*, float*, float*, float*);
	float __fastcall	hkGetAlphaModulation(void*, void*);
	void __fastcall     hkInstallStringTableCallback(const char* tableName);
	void __fastcall		hkSetText(void* ecx, void* edx, const char* tokenName);
	void __fastcall		hkLevelShutDownClient(void* ecx, void* edx);
	void __fastcall     hkRenderView(void* ecx, void* edx, CViewSetup& view, CViewSetup& hudViewSetup, int nClearFlags, int whatToDraw);
	bool __fastcall     hkVerifyReturnAddress(void* edx, void* ecx, const char* moduleName);
	int __fastcall		hkBaseInterpolatePart(CBaseEntity* pEntity, void* edx, float& currentTime, Vector& oldOrigin, Vector& oldAngles, int& bNoMoreChanges);

	// netchannel table
	void __fastcall		hkProcessPacket( void*, void*, void*, bool );
	bool __fastcall		hkSVCMsg_VoiceData( void*, void*, C_SVCMsg_VoiceData* );
	bool __fastcall		hkSendNetMsg( INetChannel*, int, INetMessage*, bool, bool );
	void __fastcall		hkSetChoked( void*, void* );
	int	__fastcall		hkSendDatagram( INetChannel*, int, bf_write* );

	// inetchannelhandler
	void __fastcall		hkPacketStart( void*, void*, int, int );
	void __fastcall		hkPacketEnd( void*, void* );
	bool __fastcall		hkTemptEntities( void*, void*, void* );

	// animation hooks
	bool __fastcall		hkShouldSkipAnimationFrame(void*, void*);
	void __fastcall		hkDoProceduralFootPlant(void*, void*, void*, void*, void*, void*);
	void __fastcall		hkBuildTransformation(void*, void*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, int, uint8_t*);
	void __fastcall		hkCheckForSequenceChange(void*, void*, void*, int, bool, bool);
	bool __fastcall		hkIsHltv(void*, void*);
	void __fastcall		hkStandardBlendingRules(void*, void*, void*, void*, void*, float, int);
	void __fastcall		hkCalculateView(void*, void*, Vector&, Vector&, float&, float&, float&);
	void __fastcall		hkModifyEyePosition(void*, void*, Vector&);
	void __fastcall		hkUpdateClientSideAnimations(void*, void*);
	void __vectorcall	hkUpdateAnimationState(CAnimState*, void* edx, float z, float y, float x, void*);
	bool __fastcall		hkSetupBones(IClientRenderable*, void*, matrix3x4_t*, int, int, float);
	void __fastcall		hkDoExtraBoneProcessing(void*, void*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, byte*, void*);
	bool __stdcall		hkIsPaused();
	void __fastcall		hkPhysicsSimulate(CBaseEntity*, void*);
	void __fastcall		hkInterpolateServerEntites(void*, void*);
	bool __fastcall		hkIsFollowingEntity(void*, void*);
	void __fastcall		hkEstimateAbsVelocity(CBaseEntity*, void*, Vector&);
	void __fastcall		hkProcessMovement(void* ecx, void* edx, CBaseEntity* pEntity, CMoveData* pMove);
	void __fastcall		hkSetupAliveLoop(void* ecx, void* edx);
	long CALLBACK        hkWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	inline LPVOID oldpacketStart;
	inline LPVOID oldpacketEnd;
	inline LPVOID oldtemptEntities;

	void HookNetChannel(INetChannel*);
	void UnHookNetChannel();
	void HookClientState();
	void UnHookClientState();
	void HookEntites();

	inline WNDPROC pOriginalWNDProc;
	// Original CSGO window proc
}

class Event : public IGameEventListener
{
public:
	void FireGameEvent(IGameEvent* event);
	int  GetEventDebugID = 42;

	void Init() {

		i::GameEvent->AddListener(this, XorStr("player_hurt"), false);
		i::GameEvent->AddListener(this, XorStr("player_death"), false);
		i::GameEvent->AddListener(this, XorStr("bullet_impact"), false);
		i::GameEvent->AddListener(this, XorStr("weapon_fire"), false);
		i::GameEvent->AddListener(this, XorStr("round_start"), false);
		i::GameEvent->AddListener(this, XorStr("item_purchase"), false);
		i::GameEvent->AddListener(this, XorStr("bomb_beginplant"), false);
	}

	void Destroy() {

		i::GameEvent->RemoveListener(this);
	}
};
inline Event gameevent;