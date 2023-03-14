#include "Lagcompensation.h"

void Lagcompensation::FrameStageNotify(EStage curStage) {

	/* Get every entity */
	for (int i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		/* Check if that entity is valid or not */
		if (!pEnt || !pEnt->GetHealth() || !pEnt->IsAlive() || !pEnt->AnimState() || !g::pLocal) {

			deqRecords[i].clear();
			continue;
		}

		if (pEnt == g::pLocal) {

			continue;
		}

		if (pEnt->IsDormant()) {

			g::bAllowAnimations[i] = true;
			continue;
		}

		static bool bUpdate[65];
		/* Get important information before interpolation */
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {

			VarMapping_t* pVarMap = pEnt->GetVarMap();

			if (!pVarMap)
				continue;

			for (int i = 0; i < pVarMap->m_nInterpolatedEntries; i++) {

				VarMapEntry_t* pEntry = &pVarMap->m_Entries[i];
				pEntry->m_bNeedsToInterpolate = false;
			}

			bUpdate[i] = deqRecords[i].empty() || pEnt->GetSimulationTime() != pEnt->GetOldSimulationTime();

			if (bUpdate) {

				if (deqRecords[i].size() >= 2) {

					record_t previousRecord = deqRecords[i].at(1 );

					/* lagcomp breaking ppl check */
					if ((pEnt->GetVecOrigin() - previousRecord.vecOrigin).LengthSqr() > 4096.f && pEnt->GetSimulationTime() > previousRecord.flSimulationTime) {

						ExtrapolatePlayer( pEnt, &deqRecords[ i ].front( ), &previousRecord );

						previousRecord.bValid = false;
					}
				}
				/* let's create a new record and start updating some stuff in the right time */
				deqRecords[i].emplace_front(record_t());
				GetAnimationLayers(pEnt);
			}

			if (deqRecords[i].size() > 32)
				deqRecords[i].pop_back();
		}
		/* Update animations */
		else if (curStage == FRAME_NET_UPDATE_END) {

			if (bUpdate[i]) {

				/* Not animationfix!!! just animation update at the right time */
				UpdateAnimation(pEnt);

				bUpdate[i] = false;
			}
		}
		/* Restore every animation at render start */
		else if (curStage == FRAME_RENDER_START) {

			if (!bUpdate[i] && !deqRecords[i].empty() && deqRecords[i].front().bValid) {

				deqRecords[i].front().ApplyRecord(pEnt);
				pEnt->UpdateClientSideAnimations();
			}
		}
	}
}

void Lagcompensation::ExtrapolatePlayer( CBaseEntity* m_pEntity, record_t* m_pCurrentRecord, record_t* m_pPrevious ) const
{
	CSimulationData simulationData;

	simulationData.pEntity = m_pEntity;
	simulationData.vecOrigin = m_pCurrentRecord->vecOrigin;
	simulationData.vecVelocity = m_pCurrentRecord->vecVelocity;
	simulationData.bOnGround = m_pCurrentRecord->iFlags & FL_ONGROUND;
	simulationData.bDataFilled = true;

	int  iSimulationTickDelta = std::clamp( TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime ), 1, 15 );
	auto delta_ticks = ( std::clamp( TIME_TO_TICKS( i::EngineClient->GetNetChannelInfo( )->GetAvgLatency( FLOW_INCOMING ) + i::EngineClient->GetNetChannelInfo( )->GetAvgLatency( FLOW_OUTGOING ) ) + i::GlobalVars->iTickCount -
		TIME_TO_TICKS( m_pCurrentRecord->flSimulationTime + lagcomp.LerpTime( ) ), 0, 100 ) ) - iSimulationTickDelta;

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

void Lagcompensation::RebuildWalkToRunTransition( CBaseEntity* pEntity, record_t* pRecord )
{
	// static values to save last data.
	static float flPlaybackrate = 0.f;
	static bool bDone = false;
	static int iWalkToRunTransitionState;
	static float flWalkToRunTransition;

	// the speed x, y, z.
	Vector flVelocity = pRecord->vecVelocity;

	// that will not be correct since i calculate with wrong velocity but it will use the playbackrate.
	// its actually higher than this but i'm trying to predict it.
	if ( flVelocity.Length( ) > 260.0f * 0.52f )
	{
		// don't try more.
		if ( !bDone )
			flPlaybackrate = pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate;

		// i have done the calculation.
		bDone = true;
	}
	// do it again.
	else bDone = false;

	// rebuilt the data.
	if (flWalkToRunTransition > 0 && flWalkToRunTransition < 1 )
	{
		// currently transitioning between walk and run.
		if ( iWalkToRunTransitionState == 0 )
			flWalkToRunTransition += pEntity->AnimState( )->flLastUpdateIncrement * 2.0f;
		else flWalkToRunTransition -= pEntity->AnimState( )->flLastUpdateIncrement * 2.0f;

		flWalkToRunTransition = std::clamp( flWalkToRunTransition, 0.0f, 1.f );
	}

	// is moving higher than speed 135.
	if ( pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate > flPlaybackrate && iWalkToRunTransitionState == 1 )
	{
		flWalkToRunTransition = fmax( 0.01f, flWalkToRunTransition );

		//crossed the walk to run threshold.
		iWalkToRunTransitionState = 0;
	}
	// is moving lower than speed 135.
	else if ( pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate < flPlaybackrate && iWalkToRunTransitionState == 0 )
	{
		flWalkToRunTransition = fmax( 0.99f, flWalkToRunTransition );

		//crossed the run to walk threshold.
		iWalkToRunTransitionState = 1;
	}
}

void Lagcompensation::UpdateAnimation(CBaseEntity* pEnt) {

	if (deqRecords[pEnt->EntIndex()].empty())
		return;

	if (deqRecords[pEnt->EntIndex()].size() < 2 && !deqRecords[pEnt->EntIndex()].empty()) {

		deqRecords[pEnt->EntIndex()].front().StoreRecord(pEnt);
		g::bAllowAnimations[pEnt->EntIndex()] = true;
		return;
	}

	/* Get records */
	record_t* pCurrent = &deqRecords[pEnt->EntIndex()].front();
	record_t* pPrevious = &deqRecords[pEnt->EntIndex()].at(1);

	/* Backup globals to not mess with the game */
	const float flCurrentTime = i::GlobalVars->flCurrentTime;
	const float flFrameTime = i::GlobalVars->flFrameTime;

	/* Fixing networked players (fakelagging) */
	i::GlobalVars->flCurrentTime = pEnt->GetSimulationTime();
	i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;

	/* Absolute origin/velocity fix just in case if this shit game already messed it up */
	FixAbsoluteAngVec(pEnt, pPrevious, pCurrent);
	RebuildWalkToRunTransition( pEnt, pCurrent );

	/* Skip C_BaseEntity::CalcAbsoluteVelocity (we will also fix this) */
	pEnt->GetEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;
	// skip call to CalcAbsolutePosition.
	pEnt->GetEFlags( ) &= ~EFL_DIRTY_ABSTRANSFORM;

	// fix feet spin.
	pEnt->AnimState( )->flFeetCycle = pCurrent->pLayers[ 6 ].flCycle;
	// just get the feet weight.
	pEnt->AnimState( )->flMoveWeight = pCurrent->pLayers[ 6 ].flWeight / pEnt->AnimState( )->flInAirSmoothValue;

	/* Force full update this frame */
	if (pEnt->AnimState()->iLastUpdateFrame == i::GlobalVars->iFrameCount)
		pEnt->AnimState()->iLastUpdateFrame--;

	/* To save some FPS we only call this once */
	Vector vecEyeAngles = pEnt->GetEyeAngles();

	/* Update animation */
	g::bAllowAnimations[pEnt->EntIndex()] = true;

	/* x = pitch */ /* y = yaw */ /* z = roll */
	/* FORMULA: "z = y +- roll" (setting Z to the Y removes the infamous roll animation) */
	pEnt->AnimState()->Update(Vector(vecEyeAngles.x, vecEyeAngles.y, vecEyeAngles.y));
	pEnt->UpdateClientSideAnimations();

	g::bAllowAnimations[pEnt->EntIndex()] = false;

	/* Set the uninterpolated layers back after interpolation */
	pEnt->SetAnimationLayers(pCurrent->pLayers);

	/* Build matrix for the aimbot */
	pEnt->SetupBonesFix( pEnt, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_ATTACHMENT, i::GlobalVars->flCurrentTime, pCurrent->pMatrix );

	/* Set globals back to normal values */
	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flFrameTime = flFrameTime;

	/* Store this record */
	if (pEnt->GetTeam() != g::pLocal->GetTeam())
		pCurrent->StoreRecord(pEnt);
}

void Lagcompensation::GetAnimationLayers(CBaseEntity* pEnt) {

	/* duh ye I needed another function for that */
	pEnt->GetAnimationLayers(deqRecords[pEnt->EntIndex()].front().pLayers);

	/* TODO: animationlayer fixes */
	/* relavant ones: jump fall, shoot, moving */
}

void Lagcompensation::FixAbsoluteAngVec(CBaseEntity* pEnt, record_t* pPrevious, record_t* pLatest) {

	/* AbsOrigin -> set every frame */
	/* vecOrigin -> set every tick  */
	pEnt->SetAbsOrigin(pEnt->GetVecOrigin()); 
	/* this fixes the client updating frames while server updating ticks difference */

	if (pPrevious->flSimulationTime == pLatest->flSimulationTime)
		return;

	/* Calculate current and old delta */
	Vector vecOriginDelta = pEnt->GetVecOrigin() - pPrevious->vecOrigin;
	float flTimeDelta = pEnt->GetSimulationTime() - pPrevious->flSimulationTime;

	/* Sync client data how the server calculates */
	pEnt->GetVelocity() = vecOriginDelta / flTimeDelta;

	float flSpeed = 0.f;
	if (pEnt->GetFlags() & FL_ONGROUND && 
		pPrevious->iFlags & FL_ONGROUND &&
		pLatest->pLayers[11].flWeight > 0.f &&
		pLatest->pLayers[11].flWeight < 1.f &&
		pLatest->pLayers[11].flPlaybackRate == pPrevious->pLayers[11].flPlaybackRate) {

		float flModifier = 0.35f * (1.f - pLatest->pLayers[11].flWeight);
		if (flModifier > 0.f && flModifier < 1.f)
			flSpeed = pLatest->flMaxSpeed * (flModifier + 0.55f);
	}

	/* If the player is moving let's fix the values */
	if (flSpeed > 0.f) {

		pEnt->GetVelocity().x *= flSpeed;
		pEnt->GetVelocity().y *= flSpeed;
	}

	/* If player is in air lets also calculate and predict gravity */
	if (!(pEnt->GetFlags() & FL_ONGROUND))
		pEnt->GetVelocity().z -= i::ConVar->FindVar("sv_gravity")->GetFloat() * flTimeDelta * 0.5f;

	/* Set abs velocity to the velocity we calculated */
	pEnt->SetAbsVelocity(pEnt->GetVelocity());

	/* AbsVelocity -> set every frame */
	/* vecVelocity -> set every tick  */

	/* NOTE: it won't update since we return nothing in update client side animations hook */
	/*		 so it will really only update when we want it								   */	
}

float Lagcompensation::LerpTime() {

	static auto cl_interp = i::ConVar->FindVar("cl_interp");
	static auto cl_interp_ratio = i::ConVar->FindVar("cl_interp_ratio");
	static auto sv_client_min_interp_ratio = i::ConVar->FindVar("sv_client_min_interp_ratio");
	static auto sv_client_max_interp_ratio = i::ConVar->FindVar("sv_client_max_interp_ratio");
	static auto cl_updaterate = i::ConVar->FindVar("cl_updaterate");
	static auto sv_minupdaterate = i::ConVar->FindVar("sv_minupdaterate");
	static auto sv_maxupdaterate = i::ConVar->FindVar("sv_maxupdaterate");

	auto updaterate = std::clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
	auto lerp_ratio = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());

	return std::clamp(lerp_ratio / updaterate, cl_interp->GetFloat(), 1.0f);
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