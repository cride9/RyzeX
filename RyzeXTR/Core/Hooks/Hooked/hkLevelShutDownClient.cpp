#include "../hooks.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/Animations/EnemyAnimations.h"
#include "../../Features/Rage/Animations/ExperimentalResolver.h"
#include "../../Features/Visuals/chams.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Misc/Playerlist.h"
#include "../../Features/Rage/aimbot.h"
#include "../../Features/Changers/SkinChanger.h"

void __fastcall h::hkLevelShutDownClient(void* ecx, void* edx) {

	static auto original = detour::levelShutDown.GetOriginal<decltype(&h::hkLevelShutDownClient)>();

	misc::RestoreOverrides();
	experimentalResolver.ResetAll();
	exploits::ResetExploits();
	prediction.ResetData();
	networking.ResetNetwork();
	packetManager.pCommandList.clear();
	original(ecx, edx);

	/* Clear lagcomp & global player data */
	// Shutdown can run while the local player is alive and after maxClients
	// has been cleared. Reset the complete slot storage, including player 64.
	for (size_t i = 0; i < std::size(g::bSettingUpBones); ++i) {

		auto pLog = &lagcomp.GetLog(i);
		pLog->ClearData();

		g::bAllowAnimations[i] = true;
		g::bSettingUpBones[i] = false;
		g::bTracingLagRecord[i] = false;
		lagcomp.arrBackupData[i].second = false;
		playerList::arrPlayers[i] = playerSettings_t();
	}
	/* Clear global pointers */
	g::pCmd = nullptr;
	g::pLocal = nullptr;

	/* Reset networking */
	networking.ResetNetwork();
	packetManager.pCommandList.clear();

	/* Reset prediction and ragebot data */
	prediction.pLastCmd = nullptr;
	aimbot.GetAimbotData().pAimbotTarget = nullptr;
	aimbot.GetAimbotData().pTargetMatrix = nullptr;
	anims.bResolverHandler = std::array<bool, Animations::EResolverHandler::HANDLERCOUNT>();
	misc::exploitInitialized = false;
	skinChanger.flUpdateTime = 0.f;

	/* Clear visuals */
	chams::materials.fill(nullptr);
	visual::vecDormatPosition = std::array<Vector, 65>();
	visual::vecLights.clear();
	visual::ClearDamageIndicators();
	
	/* Set last sendpacket pointer to true */
	if (g::bSendPacket)
		*g::bSendPacket = true;

	lagcomp.ClearIncomingSequences();
	/* New server = new client these needs to be rehooked */
	//UnHookClientState();
}
