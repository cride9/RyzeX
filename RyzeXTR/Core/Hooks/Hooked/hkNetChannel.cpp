#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"

void __fastcall h::hkProcessPacket( void* ecx, void* edx, void* packet, bool header )
{
	static auto original = detour::processPacket.GetOriginal<decltype( &hkProcessPacket )>( );

	if ( !i::ClientState->pNetChannel )
		return original( ecx, edx, packet, header );

	original( ecx, edx, packet, header );

	// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
	for ( CEventInfo* it{ i::ClientState->pEvents }; it != nullptr; it = it->pNext )
	{
		if ( !it->iClassID )
			continue;

		// set all delays to instant.
		it->flFireDelay = 0.f;
	}

	// game events are actually fired in on_render_start which is WAY later after they are received
	// effective delay by lerp time, now we call them right after theyre received (all receive proxies are invoked without delay).
	i::EngineClient->FireEvents( );
}

bool __fastcall h::hkSendNetMsg( INetChannel* thisptr, int edx, INetMessage* pMessage, bool bForceReliable, bool bVoice )
{
	static auto original = detour::sendNetMsg.GetOriginal<decltype(&hkSendNetMsg)>();

	/*
	 * @note: disable files crc check (sv_pure)
	 * dont send message if it has FileCRCCheck type
	 */
	if ( pMessage->GetType( ) == INetChannelInfo::PAINTMAP)
		return false;

	/*
	 * @note: fix lag with chocking packets when voice chat is active
	 * check for voicedata group and enable voice stream
	 * @credits: Flaww
	 */
	if ( pMessage->GetGroup( ) == INetChannelInfo::VOICE )
		bVoice = true;

	return original( thisptr, edx, pMessage, bForceReliable, bVoice );
}


void __fastcall h::hkSetChoked( void* ecx, void* edx )
{
	static auto original = detour::setChoked.GetOriginal<decltype( &hkSetChoked )>( );

	INetChannel* pNetChannelInfo = reinterpret_cast< INetChannel* >( ecx );

	// sanity checks so i dont blow my brains out
	if ( !g::pLocal || !i::EngineClient->IsInGame( ) || pNetChannelInfo == nullptr )
		return original( ecx, edx );

	// fix game delays when choking
	const int iChockedPackets = pNetChannelInfo->iChokedPackets;

	pNetChannelInfo->iChokedPackets = 0;
	pNetChannelInfo->SendDatagram( NULL );

	--pNetChannelInfo->iOutSequenceNr;
	pNetChannelInfo->iChokedPackets = iChockedPackets;

	return original( ecx, edx );
}

int __fastcall h::hkSendDatagram( INetChannel* thisptr, int edx, bf_write* pDatagram )
{
	static auto original = detour::sendDatagram.GetOriginal<decltype( &hkSendDatagram )>( );

	INetChannelInfo* pNetChannelInfo = i::EngineClient->GetNetChannelInfo( );
	static CConVar* sv_maxunlag = i::ConVar->FindVar( "sv_maxunlag"  );

	if ( !i::EngineClient->IsInGame( ) || !cfg::misc::fakePing || pDatagram != nullptr || pNetChannelInfo == nullptr || sv_maxunlag == nullptr )
		return original( thisptr, edx, pDatagram );

	const int iOldInReliableState = thisptr->iInReliableState;
	const int iOldInSequenceNr = thisptr->iInSequenceNr;

	// calculate max available fake latency with our real ping to keep it w/o real lags or delays
	const float flMaxLatency = std::fmax( 0.f, std::clamp( cfg::misc::fakePingFactor / 1000.f, 0.f, sv_maxunlag->GetFloat( ) ) - pNetChannelInfo->GetLatency( FLOW_OUTGOING ) );
	//lagcomp.AddLatencyToNetChannel( thisptr, flMaxLatency );

	const int iReturn = original( thisptr, edx, pDatagram );

	thisptr->iInReliableState = iOldInReliableState;
	thisptr->iInSequenceNr = iOldInSequenceNr;

	return iReturn;
}
