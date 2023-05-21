#include "Lagcompensation.h"
#include "EnemyAnimations.h"
#include "../exploits.h"
#include "../../Networking/networking.h"
#include "Setupbones.h"

#include "../../../SDK/InputSystem.h"

void Lagcompensation::SetupPlayerBones(CBaseEntity* pEnt, Lagcompensation::LagRecord_t* pRecord, matrix3x4_t* Matrix, int nFlags) {

	if (!pRecord || !Matrix || !pEnt || !pEnt->IsAlive() || !pEnt->GetAnimationOverlays() || !pEnt->AnimState())
		return;

	int nBoneMask2 = BONE_USED_BY_ANYTHING;
	if (nFlags & BoneUsedByHitbox)
		nBoneMask2 = BONE_USED_BY_HITBOX;

	g::bSettingUpBones[pEnt->EntIndex()] = std::make_tuple(true, nFlags);
	pEnt->SetupBones(Matrix, 128, nBoneMask2, TICKS_TO_TIME(lagcomp.FixTickCount(pRecord->flSimulationTime)));
	g::bSettingUpBones[pEnt->EntIndex()] = std::make_tuple(false, 0);

	return;

	/* Reset layers */
	pEnt->SetAnimationLayers(pRecord->pLayers);

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
	int nSimulationTick = TIME_TO_TICKS(pRecord->flSimulationTime);
	i::GlobalVars->flCurrentTime = pRecord->flSimulationTime;
	i::GlobalVars->flRealTime = pRecord->flSimulationTime;
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
	if (nFlags & VisualAdjustment)
	{
		pEnt->GetAnimationOverlays()[ANIMATION_LAYER_LEAN].flWeight = 0.0f;
		if (pEnt->GetSequenceActivity(pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
		{
			pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flCycle = 0.0f;
			pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flWeight = 0.0f;
		}
	}

	/* Remove interpolation if required */
	if (!(nFlags & Interpolated))
		pEnt->SetAbsOrigin(pRecord->vecOrigin);

	/* Compute bone mask */
	int nBoneMask = BONE_USED_BY_ANYTHING;
	if (nFlags & BoneUsedByHitbox)
		nBoneMask = BONE_USED_BY_HITBOX;

	/* Fix player's data */
	pEnt->GetClientEffects() |= 2;
	pEnt->GetEffects() |= EF_NOINTERP;
	pEnt->GetOcclusionFrameCount() = -1;
	pEnt->m_nOcclusionMask() &= ~2;
	pEnt->GetLastSkipFrameCount() = 0;

	/* Setup bones */
	//bResult = original(ecx, edx, matrix, maxbones, nBoneMask, TICKS_TO_TIME(lagcomp.FixTickCount(m_Record->flSimulationTime)));
	g::bSettingUpBones[pEnt->EntIndex()] = std::make_tuple(true, nFlags);
	pEnt->SetupBones(Matrix, 128, nBoneMask, TICKS_TO_TIME(lagcomp.FixTickCount(pRecord->flSimulationTime)));
	g::bSettingUpBones[pEnt->EntIndex()] = std::make_tuple(false, 0);

	/* Restore player's data */
	pEnt->GetLastSkipFrameCount() = std::get < 0 >(m_PlayerData);
	pEnt->GetEffects() = std::get < 1 >(m_PlayerData);
	pEnt->GetClientEffects() = std::get < 2 >(m_PlayerData);
	pEnt->GetOcclusionFrameCount() = std::get < 3 >(m_PlayerData);
	pEnt->m_nOcclusionMask() = std::get < 4 >(m_PlayerData);
	pEnt->SetAbsOrigin(std::get < 5 >(m_PlayerData));

	/* Reset layers */
	std::memcpy(pEnt->GetAnimationOverlays(), pRecord->pLayers, sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

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
	pEntity->SetBoneCache( pVisualMatrix );
}

void Lagcompensation::LagRecord_t::Apply(CBaseEntity* pEntity, bool Backup, int pMatrixID)
{
	pEntity->GetFlags() = fFlags;
	pEntity->GetSimulationTime() = flSimulationTime;
	pEntity->GetLowerBodyYaw() = flLowerBodyYawTarget;
	pEntity->GetDuckAmount() = flDuck;
	pEntity->GetEyeAngles() = vecEyeAngles;
	pEntity->GetCollideable()->OBBMaxs() = vecMaxs;
	pEntity->GetCollideable()->OBBMins() = vecMins;
	pEntity->GetVelocity() = vecVelocity;
	pEntity->GetVecAbsVelocity() = Backup ? vecAbsVelocity : vecVelocity;
	pEntity->GetVecOrigin() = vecOrigin;
	pEntity->SetAbsOrigin(Backup ? vecAbsOrigin : vecOrigin);
	switch (pMatrixID)
	{
	case 1:
		pEntity->SetBoneCache(pMatrix);
		break;
	case 2:
		pEntity->SetBoneCache(pLeftMatrix);
		break;
	case 3:
		pEntity->SetBoneCache(pRightMatrix);
		break;
	case 4:
		pEntity->SetBoneCache(pCenterMatrix);
		break;
	default:
		pEntity->SetBoneCache(pVisualMatrix);
		break;
	}
	//pEntity->SetBoneCache(pVisualMatrix);
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
	pEntity->SetBoneCache( pVisualMatrix );
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

	if (!g::pLocal || cfg::debugSwitch)
		return;

	for (size_t i = 1; i < i::GlobalVars->nMaxClients; i++ )
	{
		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));
		Lagcompensation::AnimationInfo_t* pCurrentLog = &pPlayerLogs[i];

		if ( !pEntity || pEntity == g::pLocal ) {
			pCurrentLog->pRecord.clear( );
			continue;
		}

		if (pCurrentLog->pEntity != pEntity) {

			pCurrentLog->pRecord.clear();
			pCurrentLog->pEntity = pEntity;
		}

		if ( !pCurrentLog->pEntity->IsAlive( ) || pCurrentLog->pEntity->GetTeam() == g::pLocal->GetTeam()) {

			pCurrentLog->pEntity->IsClientSideAnimation( ) = g::bAllowAnimations[ pCurrentLog->pEntity->EntIndex( ) ] = true;
			pCurrentLog->pRecord.clear( );
			continue;
		}

		if (pCurrentLog->pEntity->IsDormant()) {

			bool bInsert = true;

			if (!pCurrentLog->pRecord.empty()) {

				Lagcompensation::LagRecord_t& iFront = pCurrentLog->pRecord.front();

				if (iFront.bDormant)
					bInsert = false;
			}

			if (bInsert) {

				pCurrentLog->pRecord.push_front(Lagcompensation::LagRecord_t(pCurrentLog->pEntity));

				Lagcompensation::LagRecord_t* pCurrentRecord = &pCurrentLog->pRecord.front();

				pCurrentRecord->bDormant = true;
			}
			continue;
		}

		bool bUpdate = ( pCurrentLog->pRecord.empty( ) || anims.NewDataRecievedFromServer( pEntity ) );

		if ( bUpdate ) {

			pCurrentLog->pRecord.push_front(Lagcompensation::LagRecord_t(pCurrentLog->pEntity));
			anims.RebuildEnemyAnimations(pCurrentLog->pEntity, pCurrentLog);

			bChanged = true;
		}

		while ( pCurrentLog->pRecord.size( ) > 32)
			pCurrentLog->pRecord.pop_back( );

		// filter the pRecord if the data changed.
		if (bChanged) {

			for (size_t j = 0; j < pCurrentLog->pRecord.size(); j++)
			{
				if (pCurrentLog->pRecord.at(j).bValid = lagcomp.IsValidRecord(pCurrentLog->pRecord.at(j).flSimulationTime))
					pCurrentLog->iLastValid = j;

				if (pCurrentLog->pRecord.at(j).bValid && pCurrentLog->iFirstValid >= j)
					pCurrentLog->iFirstValid = j;
			}
		}
	}
}

Lagcompensation::AnimationInfo_t& Lagcompensation::GetLog( const int iEntIndex )
{
	return pPlayerLogs[ iEntIndex ];
}

void Lagcompensation::FilterRecords( )
{
	for (size_t i = 1; i <= i::GlobalVars->nMaxClients; i++ )
	{
		if ( pPlayerLogs[ i ].pRecord.empty( ) )
			continue;

		if ( !pPlayerLogs[ i ].pEntity ) {
			pPlayerLogs[ i ].pRecord.clear( );
			continue;
		}

		// no need to store insane amount of data.
		//while ( pPlayerLogs[ i ].pRecord.size( ) > 16 )
		//	pPlayerLogs[ i ].pRecord.pop_back( );

		for ( auto j = 0u; j < pPlayerLogs[ i ].pRecord.size( ); j++ )
		{
			auto& pCurrentRecord = pPlayerLogs[ i ].pRecord[ j ];
			if ( pCurrentRecord.bDormant && !j )
				continue;

			//if ( pCurrentRecord.bDormant )
			//{
			//	pPlayerLogs[ i ].pRecord.erase( pPlayerLogs[ i ].pRecord.begin( ) + j );
			//	j--;
			//	continue;
			//}


		}
	}
}

void Lagcompensation::SetInterpolationFlags(CBaseEntity* pEnemy)
{
	VarMapping_t* pVarMap = pEnemy->GetVarMap();

	if (!pVarMap)
		return;

	for (int i = 0; i < pVarMap->m_nInterpolatedEntries; i++) {

		VarMapEntry_t& pEntry = pVarMap->m_Entries[i];
		pEntry.m_bNeedsToInterpolate = false;
	}	
}

bool Lagcompensation::IsBreakingLagcompensation( Lagcompensation::LagRecord_t* pLagRecord )
{
	Lagcompensation::AnimationInfo_t* pInfo = &lagcomp.GetLog( pLagRecord->iEntIndex );

	// check if we have at least one entry.
	if ( !&pInfo || pInfo->pRecord.size( ) <= 2 )
		return false;

	Vector previousOrigin = pLagRecord->pEntity->GetAbsOrigin( );
	Lagcompensation::LagRecord_t* pPrevious = &pInfo->pRecord.at(1);

	// walk context looking for any invalidating event.
	//for ( Lagcompensation::LagRecord_t& pRecord : pInfo->pRecord )
	{
		if ( !pLagRecord->pEntity->IsAlive( ) )
		{
			return false;
		}

		Vector delta = pLagRecord->vecOrigin - pPrevious->vecOrigin;
		if ( delta.LengthSqr( ) > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR )
		{
			ExtrapolatePlayer(pLagRecord->pEntity, pLagRecord, pPrevious );

			// lost track, too much difference.
			return true;
		}

		// player is abusing tickbase and breaking lagcompensation
		if (pLagRecord->flSimulationTime < pLagRecord->flOldSimulationTime )
		{
			return true;
		}
		//else if ( &pRecord && ( pInfo->pEntity->GetSimulationTime( ) == pRecord.flSimulationTime ) )
		//	return true;

		previousOrigin = pLagRecord->vecOrigin;
	}

	return false;
}

void Lagcompensation::ExtrapolatePlayer( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pCurrentRecord, Lagcompensation::LagRecord_t* m_pPrevious ) const
{
	CSimulationData simulationData;

	simulationData.pEntity = m_pEntity;
	simulationData.vecOrigin = m_pCurrentRecord->vecOrigin;
	simulationData.vecVelocity = m_pCurrentRecord->vecVelocity;
	simulationData.bOnGround = m_pCurrentRecord->fFlags & FL_ONGROUND;
	simulationData.bDataFilled = true;

	INetChannelInfo* pNetChannel = i::EngineClient->GetNetChannelInfo();
	if (!pNetChannel)
		return;

	int  iSimulationTickDelta = std::clamp( abs(TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime )), 1, 15 );
	auto delta_ticks = ( std::clamp( TIME_TO_TICKS(pNetChannel->GetAvgLatency( FLOW_INCOMING ) + pNetChannel->GetAvgLatency( FLOW_OUTGOING ) ) + i::GlobalVars->iTickCount -
		TIME_TO_TICKS( m_pCurrentRecord->flSimulationTime + lagcomp.GetClientInterpAmount( ) ), 0, 100 ) ) - iSimulationTickDelta;

	if ( delta_ticks > 0 && simulationData.bDataFilled )
	{
		for ( ; delta_ticks >= 0; delta_ticks -= iSimulationTickDelta )
		{
			auto ticks_left = iSimulationTickDelta;
			do
			{
				//Trace_t      trace;
				//CTraceFilter filter( g::pLocal );
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

				//i::EngineTrace->TraceRay( Ray_t( simulationData.vecOrigin, predicted_origin, simulationData.pEntity->vecMins( ), simulationData.pEntity->vecMaxs( ) ), CONTENTS_SOLID, &filter, &trace );

				m_pCurrentRecord->flSimulationTime = m_pEntity->GetSimulationTime( ) + time_to_extrapolate;
				--ticks_left;
			} while ( ticks_left );
		}

		m_pCurrentRecord->vecOrigin = simulationData.vecOrigin;
		m_pCurrentRecord->vecAbsOrigin = simulationData.vecOrigin;
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

	return /*std::fabsf*/( /*m_flCorrect -*/ ( i::GlobalVars->flCurrentTime - mflSimulationTime ) ) < 0.19f /*+ TICKS_TO_TIME( iExtraTick / 2 )*/;
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
	while ( vecSequences.size( ) > 2048U )
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

	for (size_t i = 1; i <= i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity() || pEntity == g::pLocal)
			continue;

		SetInterpolationFlags(pEntity);
	}
}