#include "Lagcompensation.h"
#include "EnemyAnimations.h"
#include "../exploits.h"
#include "../../Networking/networking.h"

void SetupPlayerBones(CBaseEntity* pEnt, Lagcompensation::LagRecord_t* m_Record, matrix3x4_t* aMatrix, matrix3x4_t* leftMatrix, matrix3x4_t* rightMatrix, int nMask, int nFlags)
{
	/* Reset layers */
	std::memcpy(pEnt->GetAnimationOverlays(), m_Record->pLayers, sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

	// save globals
	std::tuple < float, float, float, float, float, int, int > m_Globals = std::make_tuple
	(
		// backup globals
		i::GlobalVars->flCurrentTime,
		i::GlobalVars->flRealTime,
		i::GlobalVars->flFrameTime,
		i::GlobalVars->flAbsFrameTime,
		i::GlobalVars->flInterpolationAmount,

		// backup frame count and tick count
		i::GlobalVars->iFrameCount,
		i::GlobalVars->iTickCount
	);

	/* Store player's data */
	std::tuple < int, int, int, int, int, Vector > m_PlayerData = std::make_tuple
	(
		pEnt->GetLastSkipFrameCount(),
		pEnt->GetEffects(),
		pEnt->GetClientEffects(),
		pEnt->GetOcclusionFrameCount(),
		pEnt->m_nOcclusionMask(),
		pEnt->GetAbsOrigin()
	);

	/* Force game's globals */
	int nSimulationTick = TIME_TO_TICKS(m_Record->flSimulationTime);
	i::GlobalVars->flCurrentTime = m_Record->flSimulationTime;
	i::GlobalVars->flRealTime = m_Record->flSimulationTime;
	i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->iTickCount = nSimulationTick;
	i::GlobalVars->iFrameCount = INT_MAX; /* ShouldSkipAnimationFrame fix */
	i::GlobalVars->flInterpolationAmount = 0.0f;

	/* Force it https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/client/c_baseanimating.cpp#L3102 */
	pEnt->InvalidateBoneCache();

	/* Force the owner of animation layers */
	for (int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++)
	{
		CAnimationLayer* m_Layer = &pEnt->GetAnimationOverlays()[iLayer];
		if (!m_Layer)
			continue;

		m_Layer->pOwner = pEnt;
	}

	/* Disable ACT_CSGO_IDLE_TURN_BALANCEADJUST animation */
	if (nFlags & 8)
	{
		pEnt->GetAnimationOverlays()[ANIMATION_LAYER_LEAN].flWeight = 0.0f;
		if (pEnt->GetSequenceActivity(pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
		{
			pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flCycle = 0.0f;
			pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flWeight = 0.0f;
		}
	}

	/* Remove interpolation if required */
	if (!(nFlags & 2))
		pEnt->SetAbsOrigin(m_Record->vecOrigin);

	/* Compute bone mask */
	int nBoneMask = BONE_USED_BY_ANYTHING;
	if (nFlags & 4)
		nBoneMask = BONE_USED_BY_HITBOX;

	/* Fix player's data */
	pEnt->GetClientEffects() |= 2;
	pEnt->GetEffects() |= EF_NOINTERP;
	pEnt->GetOcclusionFrameCount() = -1;
	pEnt->m_nOcclusionMask() &= ~2;
	pEnt->GetLastSkipFrameCount() = 0;

	/* Setup bones */
	g::bSettingUpBones[pEnt->EntIndex()] = true;
	pEnt->SetupBones(aMatrix, 128, nBoneMask, 0.f);
	g::bSettingUpBones[pEnt->EntIndex()] = false;

	/* Restore player's data */
	pEnt->GetLastSkipFrameCount() = std::get < 0 >(m_PlayerData);
	pEnt->GetEffects() = std::get < 1 >(m_PlayerData);
	pEnt->GetClientEffects() = std::get < 2 >(m_PlayerData);
	pEnt->GetOcclusionFrameCount() = std::get < 3 >(m_PlayerData);
	pEnt->m_nOcclusionMask() = std::get < 4 >(m_PlayerData);
	pEnt->SetAbsOrigin(std::get < 5 >(m_PlayerData));

	/* Reset layers */
	std::memcpy(pEnt->GetAnimationOverlays(), m_Record->pLayers, sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

	// restore globals
	i::GlobalVars->flCurrentTime = std::get < 0 >(m_Globals);
	i::GlobalVars->flRealTime = std::get < 1 >(m_Globals);
	i::GlobalVars->flFrameTime = std::get < 2 >(m_Globals);
	i::GlobalVars->flAbsFrameTime = std::get < 3 >(m_Globals);
	i::GlobalVars->flInterpolationAmount = std::get < 4 >(m_Globals);

	// restore frame count and tick count
	i::GlobalVars->iFrameCount = std::get < 5 >(m_Globals);
	i::GlobalVars->iTickCount = std::get < 6 >(m_Globals);
}

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

		// check if nullptr.
		if ( !pEntity )
		{
			pPlayerLogs[ i ].pRecord.clear( );
			continue;
		}

		// update entity ptr if required.
		// reset entity if changed.
		if ( pPlayerLogs[ i ].pEntity != pEntity )
			pPlayerLogs[ i ].pRecord.clear( );

		// update entity ptr.
		pPlayerLogs[ i ].pEntity = pEntity;

		// check if nullptr, etc.
		if ( !pPlayerLogs[ i ].pEntity || pPlayerLogs[ i ].pEntity->EntIndex() == g::pLocal->EntIndex() || !pPlayerLogs[ i ].pEntity->IsPlayer( ) )
		{
			pPlayerLogs[ i ].pRecord.clear( );
			continue;
		}

		// don't store records if no we dont need backtrack but atleast save 2 records for previous record.
		//if ( !C::Get<bool>( Vars.bRageBacktrack ) && pPlayerLogs[ i ].pRecord.size( ) > 2 )
		//	pPlayerLogs[ i ].pRecord.pop_back( );

		// if this happens, delete all the animation.
		if ( !pPlayerLogs[ i ].pEntity->IsAlive( ) )
		{
			pPlayerLogs[ i ].pEntity->IsClientSideAnimation( ) = g::bAllowAnimations[ pPlayerLogs[ i ].pEntity->EntIndex( ) ] = true;
			pPlayerLogs[ i ].pRecord.clear( );
			continue;
		}

		if (pPlayerLogs[i].pEntity->GetTeam() == g::pLocal->GetTeam()) {

			pPlayerLogs[i].pEntity->IsClientSideAnimation() = g::bAllowAnimations[pPlayerLogs[i].pEntity->EntIndex()] = true;
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		// indicate that this entity has been out of pvs.
		// insert dummy record to separate records
		// to fix stuff like lag record and pPrediction.
		if ( pPlayerLogs[ i ].pEntity->IsDormant( ) )
		{
			bool bInsert = true;

			// we have any records already?
			if ( !pPlayerLogs[ i ].pRecord.empty( ) )
			{
				Lagcompensation::LagRecord_t& iFront = pPlayerLogs[ i ].pRecord.front( );

				// we already have a dormancy separator.
				if ( iFront.bDormant )
					bInsert = false;
			}

			if ( bInsert )
			{
				// add new record.
				pPlayerLogs[ i ].pRecord.push_front( Lagcompensation::LagRecord_t( pPlayerLogs[ i ].pEntity ) );

				// get reference to newly added pRecord.
				Lagcompensation::LagRecord_t* pCurrentRecord = &pPlayerLogs[ i ].pRecord.front( );

				// mark as dormant.
				pCurrentRecord->bDormant = true;
			}

			// fix it on dormant.
			//anims.FixJumpFallAnimation( pPlayerLogs[ i ].pEntity );

			// reset data.
			continue;
		}

		// this is the first data update we are receving
		bool bUpdate = ( pPlayerLogs[ i ].pRecord.empty( ) || anims.NewDataRecievedFromServer( pEntity ) );

		// we received data with a newer simulation context.
		if ( bUpdate )
		{
			// make a full backup of the entity
			Lagcompensation::LagRecord_t pBackupRecord = Lagcompensation::LagRecord_t( pPlayerLogs[ i ].pEntity );
			pBackupRecord.Apply( pPlayerLogs[ i ].pEntity );

			// add new record.
			pPlayerLogs[ i ].pRecord.push_front( Lagcompensation::LagRecord_t( pPlayerLogs[ i ].pEntity ) );

			// get reference to newly added record.
			Lagcompensation::LagRecord_t* pCurrentRecord = &pPlayerLogs[ i ].pRecord.front( );

			// update animations on current record.
			anims.UpdateEnemyAnimations( pPlayerLogs[ i ].pEntity, pCurrentRecord );

			// set animation layers.
			pPlayerLogs[ i ].pEntity->SetAnimationLayers( pBackupRecord.pLayers );

			// create bone matrix for this pRecord.
			SetupPlayerBones(pPlayerLogs[i].pEntity, pCurrentRecord, pCurrentRecord->pMatrix, pCurrentRecord->leftMatrix, pCurrentRecord->rightMatrix, BONE_USED_BY_ANYTHING, 4);

			// restore correctly synced values.
			pBackupRecord.Restore( pPlayerLogs[ i ].pEntity );

			// is data changed?
			bChanged = true;
		}

		// max tick amt.
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 32;

		// no need to store insane amount of data.
		while ( pPlayerLogs[ i ].pRecord.size( ) > flMaxTickAmt )
			pPlayerLogs[ i ].pRecord.pop_back( );
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
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 32;

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
	Lagcompensation::AnimationInfo_t pInfo = lagcomp.GetLog( pLagRecord->iEntIndex );

	// check if we have at least one entry.
	if ( !&pInfo || pInfo.pRecord.size( ) <= 0 )
		return false;

	Vector previousOrigin = pLagRecord->pEntity->GetAbsOrigin( );

	bool m_bFoundRecord = false;

	Lagcompensation::LagRecord_t* pPrevious = nullptr;

	// the previous record.
	if ( pInfo.pRecord.size( ) >= 2 )
		pPrevious = &pInfo.pRecord[ 1 ];

	// walk context looking for any invalidating event.
	for ( Lagcompensation::LagRecord_t& pRecord : pInfo.pRecord )
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
		else if ( &pRecord && ( pInfo.pEntity->GetSimulationTime( ) == pRecord.flSimulationTime ) )
			return true;

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

		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant())
			continue;

		SetInterpolationFlags(pEntity);
	}
}

void Lagcompensation::CreateMatrix(CBaseEntity* pEnt, matrix3x4_t* Matrix, int boneMask, float SimulatedYaw) {

	/*
		VoidZero best guy (lambda owner p100)
	*/

	pEnt->AnimState()->iLastUpdateFrame--;
	pEnt->AnimState()->flLastUpdateTime -= i::GlobalVars->flIntervalPerTick;

	pEnt->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEnt->GetEyeAngles().y) + SimulatedYaw;

	pEnt->UpdateClientSideAnimations();

	pEnt->SetupBones(Matrix, 128, boneMask, 0.f);
}