#include "../hooks.h"
#include "../../Interface/Classes/CConvar.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Rage/exploits.h"
#include "../../Interface/interfaces.h"
#include "../../Features/Networking/networking.h"

void OnMovePacket(float flFrameTime) {

	static auto original = detour::clMove.GetOriginal<decltype(&h::hkClMove)>();

	INetChannel* pNetChannel = reinterpret_cast<INetChannel*>(i::EngineClient->GetNetChannelInfo());
	if (!pNetChannel)
		return;

	int nSequence = i::ClientState->iLastOutgoingCommand + i::ClientState->nChokedCommands + 1;

	networking.StartNetworking();

	bool bSendPacket = true;
	//g::bSendPacket = &bSendPacket;
	
	typedef void(__thiscall* CreateMove_t)(void*, int, float, bool);
	((CreateMove_t)(util::GetVFunc(i::ClientDll, 22)))(i::ClientDll, nSequence, i::GlobalVars->flIntervalPerTick - flFrameTime, true);

	if (bSendPacket) {

		static auto sig = (void*)((DWORD)(MEM::FindPattern(ENGINE_DLL, XorStr("55 8B EC 8B 4D 04 81 EC FC 0F 00 00 53 56 57"))));
		((void(__cdecl*)())(sig))();

		i::ClientState->iLastOutgoingCommand = pNetChannel->SendDatagram(0);
		i::ClientState->nChokedCommands = 0;

		// push cmd
		packetManager.pCommandList.emplace_back(i::ClientState->iLastOutgoingCommand);
	}
	else {

		++i::ClientState->nChokedCommands;
		++pNetChannel->iOutSequenceNr;
		++pNetChannel->iChokedPackets;

		// fix network
		networking.FinishNetworking();
	}
}

//https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/engine/cl_main.cpp#L2736
void __cdecl h::hkClMove(float flSamples, bool bFinalTick) {

	//static auto original = detour::clMove.GetOriginal<decltype(&h::hkClMove)>();

	//original(flSamples, bFinalTick);

	//return OnMovePacket(flSamples);

	// run exploits
	return exploits::OnCLMove( flSamples, bFinalTick );

	// this hook calls g_ClientDLL->CreateMove (that is hooked lmao)
	// g_ClientDLL->CreateMove( cl.lastoutgoingcommand + cl.chokedcommands + 1, host_state.interval_per_tick - flSamples, !cl.IsPaused() );

	// changing flSamples while doubletapping can cause some issues like freezing when doubletapping while our ticks are getting back to sync
	// and gl getting back ticks to synced values while we freeze :D (takes like 20-30sec to unfreeze the game)
}

void RebuiltClMove(float flSamples, bool bFinalTick) {


}