#include "Lagcompensation.h"
#include "EnemyAnimations.h"
#include "../exploits.h"
#include "../../Networking/networking.h"

Lagcompensation::LagRecord_t::LagRecord_t( CBaseEntity* pEntity )
{
	CBaseCombatWeapon* pWeapon = pEntity->GetWeapon( );

	this->pEntity = pEntity;
	iEntIndex = pEntity->EntIndex( );
	bDormant = pEntity->IsDormant( );
	vecVelocity = pEntity->GetVelocity( );
	vecAbsVelocity = pEntity->GetVecAbsVelocity( );
	vecOrigin = pEntity->GetVecOrigin( );
	vecAbsOrigin = pEntity->GetAbsOrigin( );
	vecMins = pEntity->GetCollideable( )->OBBMins( );
	vecMaxs = pEntity->GetCollideable( )->OBBMaxs( );

	pEntity->GetAnimationLayers( pLayers );

	pEntity->GetPoseParameters( flPoses );
	flInterpTime = 0.f;
	bValid = true;
	bDidShot = false;
	bFakewalking = false;
	bBreakingLagcompensation = false;
	flAnimationTime = pEntity->GetOldSimulationTime( ) + i::GlobalVars->flIntervalPerTick;
	flSimulationTime = pEntity->GetSimulationTime( );
	flOldSimulationTime = pEntity->GetOldSimulationTime( );
	flServerTick = i::ClientState->clockDriftMgr.nServerTick;
	flLastShotTime = pWeapon ? pWeapon->GetLastShotTime( ) : 0.f;
	flDuck = pEntity->GetDuckAmount( );
	flLowerBodyYawTarget = pEntity->GetLowerBodyYaw( );
	vecEyeAngles = pEntity->GetEyeAngles( );
	vecAbsAngles = pEntity->GetAbsAngles( );
	fFlags = pEntity->GetFlags( );
	iEFlags = pEntity->GetEFlags( );
	iEffects = pEntity->GetEffects( );
	iChoked = TIME_TO_TICKS( flSimulationTime - flOldSimulationTime );
	iChoked = std::clamp( iChoked, 0, 16 );
}

void Lagcompensation::LagRecord_t::Apply( CBaseEntity* pEntity, bool Backup )
{
	pEntity->GetFlags( ) = fFlags;
	pEntity->GetSimulationTime( ) = flSimulationTime;
	pEntity->GetLowerBodyYaw( ) = flLowerBodyYawTarget;
	pEntity->GetDuckAmount( ) = flDuck;
	pEntity->GetEyeAngles( ) = vecEyeAngles;
	pEntity->GetCollideable( )->OBBMaxs( ) = vecMaxs;
	pEntity->GetCollideable( )->OBBMins( ) = vecMins;
	pEntity->GetVelocity( ) = vecVelocity;
	pEntity->GetVecAbsVelocity( ) = Backup ? vecAbsVelocity : vecVelocity;
	pEntity->GetVecOrigin( ) = vecOrigin;
	pEntity->SetAbsOrigin( Backup ? vecAbsOrigin : vecOrigin );
	pEntity->SetBoneCache( pMatrix );
}

void Lagcompensation::LagRecord_t::Apply( CBaseEntity* pEntity )
{
	// set poses
	pEntity->SetPoseParameters( flPoses );
	pEntity->GetEyeAngles( ) = vecEyeAngles;
	pEntity->GetVelocity( ) = pEntity->GetVecAbsVelocity( ) = vecVelocity;
	pEntity->GetLowerBodyYaw( ) = flLowerBodyYawTarget;
	pEntity->GetDuckAmount( ) = flDuck;
	pEntity->GetFlags( ) = fFlags;
	pEntity->GetVecOrigin( ) = vecOrigin;
	pEntity->SetAbsOrigin( vecOrigin );
	pEntity->SetBoneCache( pMatrix );
}

void Lagcompensation::LagRecord_t::Restore( CBaseEntity* pEntity )
{
	pEntity->GetVelocity( ) = vecVelocity;
	pEntity->GetVecAbsVelocity( ) = vecAbsVelocity;
	pEntity->GetFlags( ) = fFlags;
	pEntity->GetEFlags( ) = iEFlags;
	pEntity->GetDuckAmount( ) = flDuck;
	// set animlayers
	pEntity->SetAnimationLayers( pLayers );
	pEntity->GetLowerBodyYaw( ) = flLowerBodyYawTarget;
	pEntity->GetVecOrigin( ) = vecOrigin;
	pEntity->SetAbsOrigin( vecAbsOrigin );
}

void Lagcompensation::FrameStageNotify() {
	
	bool bChanged = false;

	if (!g::pLocal)
		return;

	for ( int i = 1; i <= i::GlobalVars->nMaxClients; i++ )
	{
		CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>( i::EntityList->GetClientEntity( i ) );

		auto pCurrentLog = &pPlayerLogs[i];

		// check if nullptr.
		if ( !pEntity )
		{
			pCurrentLog->pRecord.clear( );
			continue;
		}

		// update entity ptr if required.
		// reset entity if changed.
		if ( pCurrentLog->pEntity != pEntity )
			pCurrentLog->pRecord.clear( );

		// update entity ptr.
		pCurrentLog->pEntity = pEntity;

		// check if nullptr, etc.
		if ( !pCurrentLog->pEntity || pCurrentLog->pEntity->EntIndex() == g::pLocal->EntIndex() || !pCurrentLog->pEntity->IsPlayer( ) )
		{
			pCurrentLog->pRecord.clear( );
			continue;
		}

		// don't store records if no we dont need backtrack but atleast save 2 records for previous record.
		if ( !cfg::rage::m_bEnableBacktrack && pCurrentLog->pRecord.size( ) > 2 )
			pCurrentLog->pRecord.pop_back( );

		// if this happens, delete all the animation.
		if ( !pCurrentLog->pEntity->IsAlive( ) )
		{
			pCurrentLog->pEntity->IsClientSideAnimation( ) = g::bAllowAnimations[ pCurrentLog->pEntity->EntIndex( ) ] = true;
			pCurrentLog->pRecord.clear( );
			continue;
		}

		if (pCurrentLog->pEntity->GetTeam() == g::pLocal->GetTeam()) {

			pCurrentLog->pEntity->IsClientSideAnimation() = g::bAllowAnimations[pCurrentLog->pEntity->EntIndex()] = true;
			pCurrentLog->pRecord.clear();
			continue;
		}

		// indicate that this entity has been out of pvs.
		// insert dummy record to separate records
		// to fix stuff like lag record and pPrediction.
		if ( pCurrentLog->pEntity->IsDormant( ) )
		{
			bool bInsert = true;

			// we have any records already?
			if ( !pCurrentLog->pRecord.empty( ) )
			{
				Lagcompensation::LagRecord_t& iFront = pCurrentLog->pRecord.front( );

				// we already have a dormancy separator.
				if ( iFront.bDormant )
					bInsert = false;
			}

			if ( bInsert )
			{
				// add new record.
				pCurrentLog->pRecord.push_front( Lagcompensation::LagRecord_t( pCurrentLog->pEntity ) );

				// get reference to newly added pRecord.
				Lagcompensation::LagRecord_t* pCurrentRecord = &pCurrentLog->pRecord.front( );

				// mark as dormant.
				pCurrentRecord->bDormant = true;
			}

			// fix it on dormant.
			//anims.FixJumpFallAnimation( pCurrentLog->pEntity );

			// reset data.
			continue;
		}

		// this is the first data update we are receving
		bool bUpdate = ( pCurrentLog->pRecord.empty( ) || anims.NewDataRecievedFromServer( pEntity ) );

		// we received data with a newer simulation context.
		if ( bUpdate )
		{
			// make a full backup of the entity
			Lagcompensation::LagRecord_t pBackupRecord = Lagcompensation::LagRecord_t( pCurrentLog->pEntity );
			pBackupRecord.Apply( pCurrentLog->pEntity );

			// add new record.
			pCurrentLog->pRecord.push_front( Lagcompensation::LagRecord_t( pCurrentLog->pEntity ) );

			// get reference to newly added record.
			Lagcompensation::LagRecord_t* pCurrentRecord = &pCurrentLog->pRecord.front( );

			// update animations on current record.
			anims.UpdateEnemyAnimations( pCurrentLog->pEntity, pCurrentRecord );

			// set animation layers.
			pCurrentLog->pEntity->SetAnimationLayers( pBackupRecord.pLayers );

			// create bone matrix for this pRecord.
			g::bSettingUpBones[i] = std::make_tuple(true, EMatrixFlags::Interpolated | EMatrixFlags::VisualAdjustment);
			pCurrentLog->pEntity->SetupBones(pCurrentRecord->pVisualMatrix, 128, 0, i::GlobalVars->flCurrentTime);

			g::bSettingUpBones[i] = std::make_tuple(true, EMatrixFlags::BoneUsedByHitbox);
			pCurrentLog->pEntity->SetupBones(pCurrentRecord->pMatrix, 128, 4, i::GlobalVars->flCurrentTime);

			g::bSettingUpBones[i] = std::make_tuple(false, 0);

			// restore correctly synced values.
			pBackupRecord.Restore( pCurrentLog->pEntity );

			// is data changed?
			bChanged = true;
		}

		// max tick amt.
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 16;

		// no need to store insane amount of data.
		while ( pCurrentLog->pRecord.size( ) > flMaxTickAmt )
			pCurrentLog->pRecord.pop_back( );
	}

	// filter the pRecord if the data changed.
	if ( bChanged )
		FilterRecords( );
}

Lagcompensation::AnimationInfo_t& Lagcompensation::GetLog( const int iEntIndex )
{
	return pPlayerLogs[ iEntIndex ];
}

void Lagcompensation::FilterRecords( )
{
	for ( int i = 1; i <= i::GlobalVars->nMaxClients; i++ )
	{
		if ( pPlayerLogs[ i ].pRecord.empty( ) )
			continue;

		if ( !pPlayerLogs[ i ].pEntity ) {
			pPlayerLogs[ i ].pRecord.clear( );
			continue;
		}

		// max tick amt.
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 16;

		// no need to store insane amount of data.
		while ( pPlayerLogs[ i ].pRecord.size( ) > flMaxTickAmt )
			pPlayerLogs[ i ].pRecord.pop_back( );

		for ( auto j = 0u; j < pPlayerLogs[ i ].pRecord.size( ); j++ )
		{
			auto pCurrentRecord = pPlayerLogs[ i ].pRecord[ j ];
			if ( pCurrentRecord.bDormant && !j )
				continue;

			if ( pCurrentRecord.bDormant )
			{
				pPlayerLogs[ i ].pRecord.erase( pPlayerLogs[ i ].pRecord.begin( ) + j );
				j--;
				continue;
			}

			if ( pCurrentRecord.bValid = lagcomp.IsValidRecord( pCurrentRecord.flSimulationTime ) )
				pPlayerLogs[ i ].iLastValid = j;
		}
	}
}

void Lagcompensation::SetInterpolationFlags(CBaseEntity* pEnemy)
{
	VarMapping_t* pVarMap = pEnemy->GetVarMap();

	if (!pVarMap)
		return;

	for (int i = 0; i < pVarMap->m_nInterpolatedEntries; i++) {

		VarMapEntry_t* pEntry = &pVarMap->m_Entries[i];
		pEntry->m_bNeedsToInterpolate = false;
	}
}

bool Lagcompensation::IsBreakingLagcompensation( Lagcompensation::LagRecord_t* pLagRecord )
{
	Lagcompensation::AnimationInfo_t* pInfo = &lagcomp.GetLog( pLagRecord->iEntIndex );

	// check if we have at least one entry.
	if ( !&pInfo || pInfo->pRecord.size( ) <= 0 )
		return false;

	Vector previousOrigin = pLagRecord->pEntity->GetAbsOrigin( );

	bool m_bFoundRecord = false;

	Lagcompensation::LagRecord_t* pPrevious = nullptr;

	// the previous record.
	if ( pInfo->pRecord.size( ) >= 2 )
		pPrevious = &pInfo->pRecord[ 1 ];

	// walk context looking for any invalidating event.
	for ( Lagcompensation::LagRecord_t& pRecord : pInfo->pRecord )
	{
		if ( !pRecord.pEntity->IsAlive( ) )
		{
	/*		L::PushConsoleColor( FOREGROUND_RED );
			L::Print( XorStr( "Lagcomp: client [{:d}] not alive, not lag compensating!" ), pRecord.pEntity->EntIndex( ) );
			L::PopConsoleColor( );*/
			// entity must be alive, lost track
			return false;
		}

		Vector delta = pRecord.vecOrigin - previousOrigin;
		if ( delta.LengthSqr( ) > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR )
		{
			ExtrapolatePlayer( pRecord.pEntity, &pRecord, pPrevious );

			Vector m_vecAbsOrigin = pRecord.pEntity->GetAbsOrigin( );
			//L::PushConsoleColor( FOREGROUND_RED );
			//L::Print( XorStr( "Lagcomp: client [{:d}] teleported, not lag compensating!" ), pRecord.pEntity->EntIndex( ) );
			//L::Print( XorStr( "Current Origin: [{:f}] [{:f}] [{:f}]" ), m_vecAbsOrigin.x, m_vecAbsOrigin.y, m_vecAbsOrigin.z );
			//L::PopConsoleColor( );

			// lost track, too much difference.
			return true;
		}

		// player is abusing tickbase and breaking lagcompensation
		if ( pRecord.flSimulationTime < pRecord.flOldSimulationTime )
		{
			//L::PushConsoleColor( FOREGROUND_RED );
			//L::Print( XorStr( "Lagcomp: client [{:d}] is shifting tickbase, not lag compensating!" ), pRecord.pEntity->EntIndex( ) );
			//L::PopConsoleColor( );
			return true;
		}
		//else if ( &pRecord && ( pInfo->pEntity->GetSimulationTime( ) == pRecord.flSimulationTime ) )
		//	return true;

		// did we find a context smaller than target time?
		if ( pRecord.flSimulationTime <= pLagRecord->flSimulationTime )
		{
			m_bFoundRecord = true;
			break; // hurra, stop-
		}

		previousOrigin = pRecord.vecOrigin;
	}

	if ( !m_bFoundRecord )
	{
		//L::PushConsoleColor( FOREGROUND_RED );
		//L::Print( XorStr( "No valid positions in history for BacktrackPlayer client [{:d}]" ), pLagRecord->pEntity->EntIndex( ) );
		//L::PopConsoleColor( );
		return false;
	}
}

void Lagcompensation::ExtrapolatePlayer( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pCurrentRecord, Lagcompensation::LagRecord_t* m_pPrevious ) const
{
	CSimulationData simulationData;

	simulationData.pEntity = m_pEntity;
	simulationData.vecOrigin = m_pCurrentRecord->vecOrigin;
	simulationData.vecVelocity = m_pCurrentRecord->vecVelocity;
	simulationData.bOnGround = m_pCurrentRecord->fFlags & FL_ONGROUND;
	simulationData.bDataFilled = true;

	int  iSimulationTickDelta = std::clamp( TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime ), 1, 15 );
	auto delta_ticks = ( std::clamp( TIME_TO_TICKS( i::EngineClient->GetNetChannelInfo( )->GetAvgLatency( FLOW_INCOMING ) + i::EngineClient->GetNetChannelInfo( )->GetAvgLatency( FLOW_OUTGOING ) ) + i::GlobalVars->iTickCount -
		TIME_TO_TICKS( m_pCurrentRecord->flSimulationTime + lagcomp.GetClientInterpAmount( ) ), 0, 100 ) ) - iSimulationTickDelta;

	if ( delta_ticks > 0 && simulationData.bDataFilled )
	{
		for ( ; delta_ticks >= 0; delta_ticks -= iSimulationTickDelta )
		{
			auto ticks_left = iSimulationTickDelta;
			do
			{
				Trace_t      trace;
				CTraceFilter filter( g::pLocal );

				auto predicted_origin = simulationData.vecOrigin;
				auto time_to_extrapolate = TIME_TO_TICKS( i::GlobalVars->iTickCount ) - m_pEntity->GetSimulationTime( );
				auto choke_delta_time = m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime;
				auto sv_gravity = i::ConVar->FindVar( "sv_gravity" )->GetFloat( );

				static auto predict_next_velocity = [ = ]( Vector v0, Vector v1 )
				{
					Vector v = v1;

					if ( v0 == v1 )
						v = v0;
					else
					{
						if ( v.Length2D( ) >= 0.1f )
						{
							auto a = ( v1 - v0 ) / choke_delta_time;
							v += a * time_to_extrapolate;
						}
						else
							v = Vector( );
					}

					return v;
				};

				predicted_origin = predict_next_velocity( m_pCurrentRecord->vecVelocity, m_pPrevious->vecVelocity );
				predicted_origin.z += simulationData.vecVelocity.z - sv_gravity * time_to_extrapolate;

				i::EngineTrace->TraceRay( Ray_t( simulationData.vecOrigin, predicted_origin, simulationData.pEntity->vecMins( ), simulationData.pEntity->vecMaxs( ) ), CONTENTS_SOLID, &filter, &trace );

				m_pCurrentRecord->flSimulationTime = m_pEntity->GetSimulationTime( ) + time_to_extrapolate;
				--ticks_left;
			} while ( ticks_left );
		}

		m_pCurrentRecord->vecOrigin = simulationData.vecOrigin;
		//m_pCurrentRecord->vecAbsOrigin = simulationData.vecOrigin;
	}
}

float Lagcompensation::GetClientInterpAmount( )
{
	if ( !i::ConVar->FindVar( "cl_interpolate" )->GetInt( ) )
		return 0.0f;

	static CConVar* m_pMinUpdateRate = i::ConVar->FindVar( "sv_minupdaterate" );
	static CConVar* m_pMaxUpdateRate = i::ConVar->FindVar( "sv_maxupdaterate" );

	float m_flLerpAmount = i::ConVar->FindVar( "cl_interp" )->GetFloat( );
	float m_nUpdateRate = i::ConVar->FindVar( "cl_updaterate" )->GetFloat( );

	if ( m_pMinUpdateRate && m_pMaxUpdateRate )
		m_nUpdateRate = std::clamp( m_nUpdateRate, m_pMinUpdateRate->GetFloat( ), m_pMaxUpdateRate->GetFloat( ) );

	float flLerpRatio = i::ConVar->FindVar( "cl_interp_ratio" )->GetFloat( );
	if ( flLerpRatio == 0 )
		flLerpRatio = 1.0f;

	static CConVar* pMin = i::ConVar->FindVar( "sv_client_min_interp_ratio" );
	static CConVar* pMax = i::ConVar->FindVar( "sv_client_max_interp_ratio" );

	if ( pMin && pMax && pMin->GetFloat( ) != -1 )
		flLerpRatio = std::clamp( flLerpRatio, pMin->GetFloat( ), pMax->GetFloat( ) );

	return fmax( m_flLerpAmount, flLerpRatio / m_nUpdateRate );
}

bool Lagcompensation::IsValidRecord( float mflSimulationTime, float flRange )
{
	if ( !i::EngineClient->GetNetChannelInfo( ) )
		return false;

	static CConVar* sv_maxunlag = i::ConVar->FindVar( "sv_maxunlag" );

	float m_flCorrect = i::EngineClient->GetNetChannelInfo( )->GetLatency( FLOW_INCOMING ) + i::EngineClient->GetNetChannelInfo( )->GetLatency( FLOW_OUTGOING ) + GetClientInterpAmount( );
	m_flCorrect = std::clamp( m_flCorrect, 0.f, sv_maxunlag->GetFloat( ) );

	// extra tick from tickbase shifting.
	//auto iExtraTick = !pTickbase.pShiftData.bInRechargeCycle && pTickbase.pShiftData.bRecharged && pTickbase.pShiftData.iShiftGettingUsed && !pTickbase.pShiftData.bDidShot ? pTickbase.pShiftData.iShiftGettingUsed : 0.f;
	auto iExtraTick = !exploits::bIsRecharging /*&& pTickbase.pShiftData.bRecharged */ && exploits::iShiftAmount && !exploits::bIsShiftingTicks ? exploits::iShiftAmount : 0.f;

	return std::fabsf( m_flCorrect - ( i::GlobalVars->flCurrentTime - mflSimulationTime ) ) <= flRange + TICKS_TO_TIME( iExtraTick / 2 );
}

int Lagcompensation::FixTickCount( const float& flSimulationTime )
{
	return TIME_TO_TICKS( flSimulationTime + GetClientInterpAmount( ) );
}

void Lagcompensation::UpdateIncomingSequences( INetChannel* pNetChannel )
{
	if ( pNetChannel == nullptr )
		return;

	// set to real sequence to update, otherwise needs time to get it work again
	if ( nLastIncomingSequence == 0 )
		nLastIncomingSequence = pNetChannel->iInSequenceNr;

	// check how much sequences we can spike
	if ( pNetChannel->iInSequenceNr > nLastIncomingSequence )
	{
		nLastIncomingSequence = pNetChannel->iInSequenceNr;
		vecSequences.emplace_front( SequenceObject_t( pNetChannel->iInReliableState, pNetChannel->iOutReliableState, pNetChannel->iInSequenceNr, i::GlobalVars->flRealTime ) );
	}

	// is cached too much sequences
	if ( vecSequences.size( ) > 2048U )
		vecSequences.pop_back( );
}

void Lagcompensation::ClearIncomingSequences( )
{
	if ( !vecSequences.empty( ) )
	{
		nLastIncomingSequence = 0;
		vecSequences.clear( );
	}
}

void Lagcompensation::AddLatencyToNetChannel( INetChannel* pNetChannel, float flLatency )
{
	for ( const auto& sequence : vecSequences )
	{
		if ( i::GlobalVars->flRealTime - sequence.flCurrentTime >= flLatency )
		{
			pNetChannel->iInReliableState = sequence.iInReliableState;
			pNetChannel->iInSequenceNr = sequence.iSequenceNr;
			break;
		}
	}
}

void Lagcompensation::RemoveInterpolation() {

	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity())
			continue;

		if (pEntity == g::pLocal) {
			if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey) && cfg::antiaim::defensive)
				SetInterpolationFlags(pEntity);
		}
		else
			SetInterpolationFlags(pEntity);
	}
}