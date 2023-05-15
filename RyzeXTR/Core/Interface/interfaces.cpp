#include "interfaces.h"
#include <stdexcept>

void i::SetupInterfaces() {

	ClientDll =		GetInterface<IBaseClientDLL>("client.dll", "VClient018");
	EntityList =	GetInterface<IClientEntityList>("client.dll", "VClientEntityList003");
	EngineClient =	GetInterface<IVEngineClient>("engine.dll", "VEngineClient014");
	Surface =		GetInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");
	Panel =			GetInterface<IPanel>("vgui2.dll", "VGUI_Panel009");
	StudioRender =	GetInterface<IStudioRender>("studiorender.dll", "VStudioRender026");
	Prediction =	GetInterface<IPrediction>("client.dll", "VClientPrediction001");
	GameMovement =	GetInterface<IGameMovement>("client.dll", "GameMovement001");
	EngineVGui =	GetInterface<IEngineVGui>("engine.dll", "VEngineVGui001");
	ModelRender =	GetInterface<IVModelRender>("engine.dll", "VEngineModel016");
	DebugOverlay =	GetInterface<IVDebugOverlay>("engine.dll", "VDebugOverlay004");
	MaterialSystem = GetInterface<IMaterialSystem>("materialsystem.dll", "VMaterialSystem080");
	ModelInfo =		GetInterface<IVModelInfo>("engine.dll", "VModelInfoClient004");
	ConVar =		GetInterface<IConVar>("vstdlib.dll", "VEngineCvar007");
	EngineTrace =	GetInterface<IEngineTrace>("engine.dll", "EngineTraceClient004");
	PhysicsProps =	GetInterface<IPhysicsSurfaceProps>("vphysics.dll", "VPhysicsSurfaceProps001");
	GameEvent =		GetInterface<IGameEventManager>("engine.dll", "GAMEEVENTSMANAGER002");
	EngineSoundClient = GetInterface<IEngineSoundClient>("engine.dll", "IEngineSoundClient003");
	InputSystem =	GetInterface<IInputSystem>( "inputsystem.dll", "InputSystemVersion001" );
	MDLCache =		GetInterface<IMDLCache>("datacache.dll", "MDLCache004");
	Localize =		GetInterface<ILocalize>("localize.dll", "Localize_001");

	ClientState = **reinterpret_cast<CClientState***>(util::FindSignature("engine.dll", "A1 ? ? ? ? 8B 88 ? ? ? ? 85 C9 75 07") + 0x1);
	if (ClientState == nullptr)
		throw std::runtime_error("Failed to get CClientState");

	KeyValuesSystem = reinterpret_cast<CKeyValuesSystem * (__cdecl*)()>(GetProcAddress(GetModuleHandle("vstdlib.dll"), "KeyValuesSystem"))();
	if (KeyValuesSystem == nullptr)
		throw std::runtime_error("Failed to get CKeyValuesSystem");

	DirectDevice = **reinterpret_cast< IDirect3DDevice9*** >( util::FindSignature( "shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C" ) + 0x1 ); // @xref: "HandleLateCreation"
	if ( DirectDevice == nullptr )
		throw std::runtime_error( "Failed to get DirectDevice" );

	Input = *reinterpret_cast<CInput**>(util::FindSignature("client.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 0x1);
	if (Input == nullptr)
		throw std::runtime_error("Failed to get CInput");

	ClientModeShared = **reinterpret_cast<CClientModeShared***>(util::GetVFunc<std::uintptr_t>(ClientDll, 10) + 0x5);
	if (ClientModeShared == nullptr)
		throw std::runtime_error("Failed to get CClientModeShared");

	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>(util::GetVFunc<std::uintptr_t>(ClientDll, 11) + 0xA);
	if (GlobalVars == nullptr)
		throw std::runtime_error("Failed to get CGlobalVarsBase");

	GlowObjectManager = *reinterpret_cast<IGlowObjectManager**>(util::FindSignature("client.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 0x3);
	if (GlowObjectManager == nullptr)
		throw std::runtime_error("Failed to get IGlowObjectManager");

	RenderBeam = *reinterpret_cast<IViewRenderBeams**>(util::FindSignature("client.dll", "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 1);
	if (RenderBeam == nullptr)
		throw std::runtime_error("Failed to get IViewRenderBeams");

	ClientMode = **reinterpret_cast<IClientMode***>((*reinterpret_cast<uintptr_t**>(ClientDll))[10] + 0x5);
	if (ClientMode == nullptr)
		throw std::runtime_error("Failed to get IClientMode");

	WeaponSystem = *(IWeaponSystem**)(util::FindSignature("client.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0") + 2);
	if (WeaponSystem == nullptr)
		throw std::runtime_error("Failed to get IWeaponSystem");

	//ItemSystem = reinterpret_cast<decltype(ItemSystem)>(util::GetAbsoluteAddress(util::FindSignature("client.dll", "E8 ? ? ? ? 0F B7 0F") + 1));
	//if (ItemSystem == nullptr)
	//	throw std::runtime_error("Failed to get ItemSystem");

	util::Print("Interfaces initialized!");
}