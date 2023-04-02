#include "networking.h"

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
	volatile auto aNetVars = &pCompressData[ nCommand % 150 ];
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

	if ( g::pLocal->GetWeapon( ) )
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

void CNetworking::OnPacketEnd( CClientState* ClientState )
{
	if ( !g::pLocal || !g::pLocal->IsAlive( ) )
		return;

	if ( *( int* )( ( DWORD )( ClientState )+0x16C ) != *( int* )( ( DWORD )( ClientState )+0x164 ) )
		return;

	prediction.AdjustViewmodelData( g::pLocal );
	return this->RestoreNetvarData( i::ClientState->iLastCommandAck );
}

bool CPacketManager::ShouldProcessPacketStart( int iCommand )
{
	for (auto pCmd = pCommandList.begin(); pCmd != pCommandList.end(); pCmd++)
	{
		if (*pCmd != iCommand)
			continue;

		pCommandList.erase( pCmd );
		return true;
	}

	return false;
}

int CNetworking::GetServerTick() {

	INetChannelInfo* m_NetChannel = i::EngineClient->GetNetChannelInfo();
	if (m_NetChannel) {

		return i::GlobalVars->iTickCount + TIME_TO_TICKS(m_NetChannel->GetLatency(FLOW_OUTGOING) + m_NetChannel->GetLatency(FLOW_INCOMING));
	}
	return i::GlobalVars->iTickCount;
}