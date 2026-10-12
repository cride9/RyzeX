#include "networking.h"
#include "../Rage/exploits.h"

inline bool IsVectorValid( Vector vecOriginal, Vector vecCurrent )
{
	Vector vecDelta = vecOriginal - vecCurrent;
	for ( int i = 0; i < 3; i++ )
	{
		if ( fabsf( vecDelta[ i ] ) > 0.03125f )
			return false;
	}

	return true;
}

inline bool IsFloatValid( float flOriginal, float flCurrent )
{
	if ( fabsf( flOriginal - flCurrent ) > 0.03125f )
		return false;

	return true;
}

void CNetworking::SaveNetvarData( int nCommand )
{
	if (!g::pLocal || nCommand <= 0)
		return; 
	
	iLastCommandNumber = nCommand;
	auto& saved = pCompressData[nCommand % 150];
	saved = {};
	saved.command = nCommand;
	saved.epoch = exploits::commandTimes.epoch;
	saved.localHandle = g::pLocal->GetRefEHandle();
	saved.spawnTime = g::pLocal->GetSpawnTime();
	pCompressData[ nCommand % 150 ].nTickbase = g::pLocal->GetTickBase( );
	pCompressData[ nCommand % 150 ].flDuckAmount = g::pLocal->GetDuckAmount( );
	pCompressData[ nCommand % 150 ].flDuckSpeed = g::pLocal->GetDuckSpeed( );
	pCompressData[ nCommand % 150 ].vecOrigin = g::pLocal->GetVecOrigin( );
	pCompressData[ nCommand % 150 ].vecVelocity = g::pLocal->GetVelocity( );
	pCompressData[ nCommand % 150 ].vecBaseVelocity = g::pLocal->GetVecBaseVelocity( );
	pCompressData[ nCommand % 150 ].flFallVelocity = g::pLocal->GetFallVelocity( );
	pCompressData[ nCommand % 150 ].vecViewOffset = g::pLocal->GetViewOffset( );
	pCompressData[ nCommand % 150 ].vecAimPunchAngle = g::pLocal->GetAimPunch( );
	pCompressData[ nCommand % 150 ].vecAimPunchAngleVel = g::pLocal->GetAimPunchVelocity( );
	pCompressData[ nCommand % 150 ].vecViewPunchAngle = g::pLocal->GetViewPunch( );

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon( );
	saved.weaponHandle = pWeapon ? pWeapon->GetRefEHandle() : 0;
	if ( !pWeapon )
	{
		pCompressData[ nCommand % 150 ].flRecoilIndex = 0.0f;
		pCompressData[ nCommand % 150 ].flAccuracyPenalty = 0.0f;

		return;
	}

	pCompressData[ nCommand % 150 ].flRecoilIndex = static_cast< CWeaponCSBase* >( pWeapon )->GetRecoilIndex( );
	pCompressData[ nCommand % 150 ].flAccuracyPenalty = static_cast< CWeaponCSBase* >( pWeapon )->GetAccuracyPenalty( );
}

void CNetworking::RestoreNetvarData( int nCommand )
{
	if (!g::pLocal || nCommand <= 0)
		return;

	volatile auto aNetVars = &pCompressData[ nCommand % 150 ];
	if (aNetVars->command != nCommand || aNetVars->epoch != exploits::commandTimes.epoch
		|| aNetVars->localHandle != g::pLocal->GetRefEHandle() || aNetVars->spawnTime != g::pLocal->GetSpawnTime()) return;
	if ( aNetVars->nTickbase != g::pLocal->GetTickBase( ) )
		return;

	if ( IsVectorValid( aNetVars->vecVelocity, g::pLocal->GetVelocity( ) ) )
		g::pLocal->GetVelocity( ) = aNetVars->vecVelocity;

	if ( IsVectorValid( aNetVars->vecBaseVelocity, g::pLocal->GetVecBaseVelocity( ) ) )
		g::pLocal->GetVecBaseVelocity( ) = aNetVars->vecBaseVelocity;

	if ( IsVectorValid( aNetVars->vecViewOffset, g::pLocal->GetViewOffset( ) ) )
		g::pLocal->GetViewOffset( ) = aNetVars->vecViewOffset;

	if ( IsVectorValid( aNetVars->vecAimPunchAngle, g::pLocal->GetAimPunch( ) ) )
		g::pLocal->GetAimPunch( ) = aNetVars->vecAimPunchAngle;

	if ( IsVectorValid( aNetVars->vecAimPunchAngleVel, g::pLocal->GetAimPunchVelocity( ) ) )
		g::pLocal->GetAimPunchVelocity( ) = aNetVars->vecAimPunchAngleVel;

	if ( IsVectorValid( aNetVars->vecViewPunchAngle, g::pLocal->GetViewPunch( ) ) )
		g::pLocal->GetViewPunch( ) = aNetVars->vecViewPunchAngle;

	if ( IsFloatValid( aNetVars->flFallVelocity, g::pLocal->GetFallVelocity( ) ) )
		g::pLocal->GetFallVelocity( ) = aNetVars->flFallVelocity;

	if ( IsFloatValid( aNetVars->flDuckAmount, g::pLocal->GetDuckAmount( ) ) )
		g::pLocal->GetDuckAmount( ) = aNetVars->flDuckAmount;

	if ( IsFloatValid( aNetVars->flDuckSpeed, g::pLocal->GetDuckSpeed( ) ) )
		g::pLocal->GetDuckSpeed( ) = aNetVars->flDuckSpeed;

	if (g::pLocal->GetWeapon() && aNetVars->weaponHandle == g::pLocal->GetWeapon()->GetRefEHandle())
	{
		if ( IsFloatValid( aNetVars->flAccuracyPenalty, static_cast< CWeaponCSBase* >( g::pLocal->GetWeapon( ) )->GetAccuracyPenalty( ) ) )
			static_cast< CWeaponCSBase* >( g::pLocal->GetWeapon( ) )->GetAccuracyPenalty( ) = aNetVars->flAccuracyPenalty;

		if ( IsFloatValid( aNetVars->flRecoilIndex, static_cast< CWeaponCSBase* >( g::pLocal->GetWeapon( ) )->GetRecoilIndex( ) ) )
			static_cast< CWeaponCSBase* >( g::pLocal->GetWeapon( ) )->GetRecoilIndex( ) = aNetVars->flRecoilIndex;
	}

	if ( g::pLocal->GetViewOffset( ).z > 64.0f )
		g::pLocal->GetViewOffset( ).z = 64.0f;
	else if ( g::pLocal->GetViewOffset( ).z <= 46.05f )
		g::pLocal->GetViewOffset( ).z = 46.0f;

	if ( g::pLocal->GetFlags( ) & FL_ONGROUND )
		g::pLocal->GetFallVelocity( ) = 0.0f;
}

void CNetworking::OnPacketEnd( CClientState* cl )
{
	if (!g::pLocal || !g::pLocal->IsAlive() || cl->iDeltaTick != static_cast<int>(cl->clockDriftMgr.nServerTick))
		return; 
	
	prediction.AdjustViewmodelData( g::pLocal );
	exploits::ObserveAcknowledgement(cl->iCommandAck, g::pLocal->GetTickBase(), TIME_TO_TICKS(g::pLocal->GetSimulationTime()));
	g::flVelocityModifier = g::pLocal->GetVelocityModifier();
	return this->RestoreNetvarData(cl->iCommandAck);
}

bool CPacketManager::ShouldProcessPacketStart( int iCommand )
{
	if (!g::pLocal || !g::pLocal->IsAlive()) {
		pCommandList.clear();
		return true;
	}

	// PacketStart also updates the incoming sequence. Repeated, cumulative and
	// loss-related acks must reach the engine even without an exact list match.
	while (!pCommandList.empty() && pCommandList.front() <= iCommand) pCommandList.pop_front();
	while (pCommandList.size() > 500) pCommandList.pop_front();
	return true;
}

int CNetworking::GetServerTick() {

	INetChannelInfo* m_NetChannel = i::EngineClient->GetNetChannelInfo();
	if (m_NetChannel) {

		return i::GlobalVars->iTickCount + TIME_TO_TICKS(m_NetChannel->GetLatency(FLOW_OUTGOING) + m_NetChannel->GetLatency(FLOW_INCOMING));
	}
	return i::GlobalVars->iTickCount;
}

int CNetworking::GetCorrectedTickbase() {

	if (!g::pLocal)
		return 0; 

	return g::pLocal->GetTickBase();
	//return pCompressData[iLastCommandNumber % 150].nTickbase - exploits::iShiftAmount;
}

CNetvarData* CNetworking::GetRecord() {

	return &pCompressData[iLastCommandNumber % 150];
}


void CNetworking::StartNetworking() {

	data.iTickRate = static_cast<int>(1.0f / i::GlobalVars->flIntervalPerTick);
	data.iMaxChoke = 14;
	data.bSkipDataGram = true;

	INetChannelInfo* pNetChannel = (i::EngineClient->GetNetChannelInfo());
	if (pNetChannel) {

		data.flLatency = pNetChannel->GetLatency(FLOW_OUTGOING) + pNetChannel->GetLatency(FLOW_INCOMING);
		data.iSequence = reinterpret_cast<INetChannel*>(pNetChannel)->iOutSequenceNr;
	}

	data.iServerTick = i::GlobalVars->iTickCount + TIME_TO_TICKS(data.flLatency);
	data.iCompensatedServerTick = data.iServerTick;
}

void CNetworking::FinishNetworking() {

	if (!i::ClientState || i::ClientState->iSignonState < SIGNONSTATE_FULL)
		return;

	if (!g::pLocal || !g::pLocal->IsAlive() || g::pLocal->HasImmunity())
		return;

	INetChannel* pNetChannel = reinterpret_cast<INetChannel*>(i::EngineClient->GetNetChannelInfo());
	if (!pNetChannel)
		return;

	if (pNetChannel->iChokedPackets % 4 || data.bSkipDataGram)
		return;

	/* run network fix */
	{
		/* store netchannel */
		int nSequenceNr = pNetChannel->iOutSequenceNr;
		int nChokedCommands = pNetChannel->iChokedPackets;

		/* fix net channel */
		pNetChannel->iChokedPackets = 0;
		pNetChannel->iOutSequenceNr = data.iSequence;

		/* send datagram */
		pNetChannel->SendDatagram(nullptr);

		/* restore netchannel */
		pNetChannel->iOutSequenceNr = nSequenceNr;
		pNetChannel->iChokedPackets = nChokedCommands;
	}
}

void CNetworking::ResetNetwork() {

	iLastCommandNumber = 0;
	LagcompensatedTicks = 0;
	data = networkData_t();
	pCompressData = std::array < CNetvarData, 150 >();
}
