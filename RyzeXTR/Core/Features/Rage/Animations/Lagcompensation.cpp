#include "Lagcompensation.h"
#include "EnemyAnimations.h"

Lagcompensation::LagRecord_t::LagRecord_t(CBaseEntity* pEntity)
{
	CBaseCombatWeapon* pWeapon = pEntity->GetWeapon();

	this->pEntity = pEntity;
	iEntIndex = pEntity->EntIndex();
	bDormant = pEntity->IsDormant();
	vecVelocity = pEntity->GetVelocity();
	vecAbsVelocity = pEntity->GetVecAbsVelocity();
	vecOrigin = pEntity->GetVecOrigin();
	vecAbsOrigin = pEntity->GetAbsOrigin();
	vecMins = pEntity->GetCollideable()->OBBMins();
	vecMaxs = pEntity->GetCollideable()->OBBMaxs();
	pEntity->GetAnimationLayers(pLayers);
	pEntity->GetPoseParameters(flPoses);
	bValid = true;
	bDidShot = false;
	bFakewalking = false;
	bBreakingLagcompensation = false;
	
	flAnimationTime = pEntity->GetOldSimulationTime() + i::GlobalVars->flIntervalPerTick;
	flSimulationTime = pEntity->GetSimulationTime();
	flOldSimulationTime = pEntity->GetOldSimulationTime();
	flServerTick = i::ClientState->clockDriftMgr.nServerTick;
	flLastShotTime = pWeapon ? pWeapon->GetLastShotTime() : 0.f;
	flDuck = pEntity->GetDuckAmount();
	flLowerBodyYawTarget = pEntity->GetLowerBodyYaw();
	flEyeYaw = pEntity->AnimState()->flEyeYaw;
	flInterpTime = 0.f;

	vecEyeAngles = pEntity->GetEyeAngles();
	vecAbsAngles = pEntity->GetAbsAngles();

	iFlags = pEntity->GetFlags();
	iEFlags = pEntity->GetEFlags();
	iEffects = pEntity->GetEffects();
	iChoked = TIME_TO_TICKS(flSimulationTime - flOldSimulationTime);
	std::clamp(iChoked, 0, 16);
}

void Lagcompensation::LagRecord_t::Apply(CBaseEntity* pEntity, bool Backup)
{
	pEntity->GetFlags() = iFlags;
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
}

void Lagcompensation::LagRecord_t::Apply(CBaseEntity* pEntity)
{
	// set poses
	pEntity->SetPoseParameters(flPoses);
	pEntity->GetEyeAngles() = vecEyeAngles;
	pEntity->GetVelocity() = pEntity->GetVecAbsVelocity() = vecVelocity;
	pEntity->GetLowerBodyYaw() = flLowerBodyYawTarget;
	pEntity->GetDuckAmount() = flDuck;
	pEntity->GetFlags() = iFlags;
	pEntity->GetVecOrigin() = vecOrigin;
	pEntity->SetAbsOrigin(vecOrigin);
}

void Lagcompensation::LagRecord_t::Restore(CBaseEntity* pEntity)
{
	pEntity->GetVelocity() = vecVelocity;
	pEntity->GetVecAbsVelocity() = vecAbsVelocity;
	pEntity->GetFlags() = iFlags;
	pEntity->GetEFlags() = iEFlags;
	pEntity->GetDuckAmount() = flDuck;
	// set animlayers
	pEntity->SetAnimationLayers(pLayers);
	pEntity->GetLowerBodyYaw() = flLowerBodyYawTarget;
	pEntity->GetVecOrigin() = vecOrigin;
	pEntity->SetAbsOrigin(vecAbsOrigin);
}

void Lagcompensation::FrameStageNotify() {

	bool bChanged = false;

	if (!g::pLocal || i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return;

	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		// check if nullptr.
		if (!pEntity)
		{
			anims.missedShots[i] = 0;
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		// update entity ptr if required.
		// reset entity if changed.
		if (pPlayerLogs[i].pEntity != pEntity)
			pPlayerLogs[i].pRecord.clear();

		// update entity ptr.
		pPlayerLogs[i].pEntity = pEntity;

		// check if nullptr, etc.
		if (!pPlayerLogs[i].pEntity || pPlayerLogs[i].pEntity->EntIndex() == g::pLocal->EntIndex() || !pPlayerLogs[i].pEntity->IsPlayer())
		{
			anims.missedShots[i] = 0;
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		// don't store records if no we dont need backtrack but atleast save 2 records for previous record.
		if ( !cfg::rage::m_bEnableBacktrack && pPlayerLogs[ i ].pRecord.size( ) > 2 )
			pPlayerLogs[ i ].pRecord.pop_back( );

		// if this happens, delete all the animation.
		if (!pPlayerLogs[i].pEntity->IsAlive())
		{
			anims.missedShots[i] = 0;
			pPlayerLogs[i].pEntity->IsClientSideAnimation() = g::bAllowAnimations[pPlayerLogs[i].pEntity->EntIndex()] = true;
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		if (pPlayerLogs[i].pEntity->GetTeam() == g::pLocal->GetTeam()) {
			anims.missedShots[i] = 0;
			pPlayerLogs[i].pEntity->IsClientSideAnimation() = g::bAllowAnimations[pPlayerLogs[i].pEntity->EntIndex()] = true;
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		if (pPlayerLogs[i].iLastUpdateTick == i::GlobalVars->iTickCount) 
			continue;

		pPlayerLogs[i].iLastUpdateTick = i::GlobalVars->iTickCount;

		// indicate that this entity has been out of pvs.
		// insert dummy record to separate records
		// to fix stuff like lag record and pPrediction.
		if (pPlayerLogs[i].pEntity->IsDormant())
		{
			bool bInsert = true;

			// we have any records already?
			if (!pPlayerLogs[i].pRecord.empty())
			{
				Lagcompensation::LagRecord_t& iFront = pPlayerLogs[i].pRecord.front();

				// we already have a dormancy separator.
				if (iFront.bDormant)
					bInsert = false;
			}

			if (bInsert)
			{
				// add new record.
				pPlayerLogs[i].pRecord.push_front(Lagcompensation::LagRecord_t(pPlayerLogs[i].pEntity));

				// get reference to newly added pRecord.
				Lagcompensation::LagRecord_t* pCurrentRecord = &pPlayerLogs[i].pRecord.front();

				// mark as dormant.
				pCurrentRecord->bDormant = true;
			}

			// fix it on dormant.
			//anims.FixJumpFallAnimation( pPlayerLogs[ i ].pEntity );

			// reset data.
			continue;
		}

		// this is the first data update we are receving
		bool bUpdate = (pPlayerLogs[i].pRecord.empty() || anims.NewDataRecievedFromServer(pEntity));

		// we received data with a newer simulation context.
		if (bUpdate)
		{
			// make a full backup of the entity
			//Lagcompensation::LagRecord_t pBackupRecord = Lagcompensation::LagRecord_t(pPlayerLogs[i].pEntity);
			//pBackupRecord.Apply(pPlayerLogs[i].pEntity);

			// add new record.
			pPlayerLogs[i].pRecord.push_front(Lagcompensation::LagRecord_t(pPlayerLogs[i].pEntity));

			// get reference to newly added record.
			Lagcompensation::LagRecord_t* pCurrentRecord = &pPlayerLogs[i].pRecord.front();

			// update animations on current record.
			//anims.UpdateEnemyAnimations(pPlayerLogs[i].pEntity, pCurrentRecord);
			anims.RebuildEnemyAnimations(pPlayerLogs[i].pEntity, &pPlayerLogs[i]);

			// set animation layers.
			//pPlayerLogs[i].pEntity->SetAnimationLayers(pBackupRecord.pLayers);

			// restore correctly synced values.
			//pBackupRecord.Restore(pPlayerLogs[i].pEntity);

			// is data changed?
			bChanged = true;
		}

		// max tick amt.
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 32;

		// no need to store insane amount of data.
		while (pPlayerLogs[i].pRecord.size() > flMaxTickAmt)
			pPlayerLogs[i].pRecord.pop_back();
	}

	// filter the pRecord if the data changed.
	if (bChanged)
		FilterRecords();
}

Lagcompensation::AnimationInfo_t& Lagcompensation::GetLog(const int iEntIndex)
{
	return pPlayerLogs[iEntIndex];
}

void Lagcompensation::FilterRecords()
{
	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++)
	{
		if (pPlayerLogs[i].pRecord.empty())
			continue;

		if (!pPlayerLogs[i].pEntity) {
			pPlayerLogs[i].pRecord.clear();
			continue;
		}

		// max tick amt.
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( XorStr( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 32;

		// no need to store insane amount of data.
		while (pPlayerLogs[i].pRecord.size() > flMaxTickAmt)
			pPlayerLogs[i].pRecord.pop_back();

		for (auto j = 0u; j < pPlayerLogs[i].pRecord.size(); j++)
		{
			auto pCurrentRecord = pPlayerLogs[i].pRecord[j];
			if (pCurrentRecord.bDormant && !j)
				continue;

			if (pCurrentRecord.bDormant)
			{
				pPlayerLogs[i].pRecord.erase(pPlayerLogs[i].pRecord.begin() + j);
				j--;
				continue;
			}

			if (pCurrentRecord.bValid = lagcomp.IsValidRecord(pCurrentRecord.flSimulationTime))
				pPlayerLogs[i].iLastValid = j;

			if (pCurrentRecord.bValid && pPlayerLogs[i].iFirstValid > j)
				pPlayerLogs[i].iFirstValid = j;
		}
	}
}

void Lagcompensation::SetInterpolationFlags()
{
	for (size_t i = 1; i <= i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity() || pEntity == g::pLocal)
			continue;

		VarMapping_t* pVarMap = pEntity->GetVarMap();

		if (!pVarMap)
			return;

		for (int i = 0; i < pVarMap->m_nInterpolatedEntries; i++) {

			VarMapEntry_t& pEntry = pVarMap->m_Entries[i];
			pEntry.m_bNeedsToInterpolate = false;
		}
	}
}

bool Lagcompensation::IsBreakingLagcompensation(Lagcompensation::LagRecord_t* pLagRecord)
{
	Lagcompensation::AnimationInfo_t& pInfo = lagcomp.GetLog(pLagRecord->iEntIndex);

	// check if we have at least one entry.
	if (!&pInfo || pInfo.pRecord.size() <= 0)
		return false;

	Vector previousOrigin = pLagRecord->pEntity->GetAbsOrigin();

	bool m_bFoundRecord = false;

	Lagcompensation::LagRecord_t* pPrevious = nullptr;

	// the previous record.
	if (pInfo.pRecord.size() >= 2)
		pPrevious = &pInfo.pRecord[1];

	// walk context looking for any invalidating event.
	for (Lagcompensation::LagRecord_t& pRecord : pInfo.pRecord)
	{
		if (!pRecord.pEntity->IsAlive())
		{
			return false;
		}

		Vector delta = pRecord.vecOrigin - previousOrigin;
		if (delta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
		{
			//ExtrapolatePlayer(pRecord.pEntity, &pRecord, pPrevious);

			// lost track, too much difference.
			return true;
		}

		// player is abusing tickbase and breaking lagcompensation
		if (pRecord.flSimulationTime < pRecord.flOldSimulationTime)
		{
			return true;
		}
		else if (&pRecord && (pInfo.pEntity->GetSimulationTime() == pRecord.flSimulationTime))
			return true;

		previousOrigin = pRecord.vecOrigin;
	}
	return false;
}

void Lagcompensation::ExtrapolatePlayer(CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pCurrentRecord, Lagcompensation::LagRecord_t* m_pPrevious) const
{
	if (!m_pPrevious)
		return;

	CSimulationData simulationData;

	simulationData.pEntity = m_pEntity;
	simulationData.vecOrigin = m_pCurrentRecord->vecOrigin;
	simulationData.vecVelocity = m_pCurrentRecord->vecVelocity;
	simulationData.bOnGround = m_pCurrentRecord->iFlags & FL_ONGROUND;
	simulationData.bDataFilled = true;

	int  iSimulationTickDelta = std::clamp(TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime), 1, 15);
	auto delta_ticks = (std::clamp(TIME_TO_TICKS(i::EngineClient->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + i::EngineClient->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING)) + i::GlobalVars->iTickCount -
		TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime + lagcomp.GetClientInterpAmount()), 0, 100)) - iSimulationTickDelta;

	if (delta_ticks > 0 && simulationData.bDataFilled)
	{
		for (; delta_ticks >= 0; delta_ticks -= iSimulationTickDelta)
		{
			auto ticks_left = iSimulationTickDelta;
			do
			{
				Trace_t      trace;
				CTraceFilter filter(g::pLocal);

				auto predicted_origin = simulationData.vecOrigin;
				auto time_to_extrapolate = TIME_TO_TICKS(i::GlobalVars->iTickCount) - m_pEntity->GetSimulationTime();
				auto choke_delta_time = m_pCurrentRecord->flSimulationTime - m_pPrevious->flSimulationTime;
				auto sv_gravity = i::ConVar->FindVar("sv_gravity")->GetFloat();

				static auto predict_next_velocity = [=](Vector v0, Vector v1)
				{
					Vector v = v1;

					if (v0 == v1)
						v = v0;
					else
					{
						if (v.Length2D() >= 0.1f)
						{
							auto a = (v1 - v0) / choke_delta_time;
							v += a * time_to_extrapolate;
						}
						else
							v = Vector();
					}

					return v;
				};

				predicted_origin = predict_next_velocity(m_pCurrentRecord->vecVelocity, m_pPrevious->vecVelocity);
				predicted_origin.z += simulationData.vecVelocity.z - sv_gravity * time_to_extrapolate;

				i::EngineTrace->TraceRay(Ray_t(simulationData.vecOrigin, predicted_origin, simulationData.pEntity->vecMins(), simulationData.pEntity->vecMaxs()), CONTENTS_SOLID, &filter, &trace);

				m_pCurrentRecord->flSimulationTime = m_pEntity->GetSimulationTime() + time_to_extrapolate;
				--ticks_left;
			} while (ticks_left);
		}

		m_pCurrentRecord->vecOrigin = simulationData.vecOrigin;
		//m_pCurrentRecord->vecAbsOrigin = simulationData.vecOrigin;
	}
}

float Lagcompensation::GetClientInterpAmount()
{
	if (!i::ConVar->FindVar("cl_interpolate")->GetInt())
		return 0.0f;

	static CConVar* m_pMinUpdateRate = i::ConVar->FindVar("sv_minupdaterate");
	static CConVar* m_pMaxUpdateRate = i::ConVar->FindVar("sv_maxupdaterate");

	float m_flLerpAmount = i::ConVar->FindVar("cl_interp")->GetFloat();
	float m_nUpdateRate = i::ConVar->FindVar("cl_updaterate")->GetFloat();

	if (m_pMinUpdateRate && m_pMaxUpdateRate)
		m_nUpdateRate = std::clamp(m_nUpdateRate, m_pMinUpdateRate->GetFloat(), m_pMaxUpdateRate->GetFloat());

	float flLerpRatio = i::ConVar->FindVar("cl_interp_ratio")->GetFloat();
	if (flLerpRatio == 0)
		flLerpRatio = 1.0f;

	static CConVar* pMin = i::ConVar->FindVar("sv_client_min_interp_ratio");
	static CConVar* pMax = i::ConVar->FindVar("sv_client_max_interp_ratio");

	if (pMin && pMax && pMin->GetFloat() != -1)
		flLerpRatio = std::clamp(flLerpRatio, pMin->GetFloat(), pMax->GetFloat());

	return fmax(m_flLerpAmount, flLerpRatio / m_nUpdateRate);
}

bool Lagcompensation::IsValidRecord(float mflSimulationTime, float flRange)
{
	if (!i::EngineClient->GetNetChannelInfo())
		return false;

	/* Lagcomp breaking = invalid */
	if ((i::GlobalVars->flCurrentTime - mflSimulationTime) < 0.f)
		return false;

	//static CConVar* sv_maxunlag = i::ConVar->FindVar("sv_maxunlag");

	//float m_flCorrect = i::EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + i::EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) /*+ GetClientInterpAmount()*/;
	//m_flCorrect = std::clamp(m_flCorrect, 0.f, sv_maxunlag->GetFloat());

	return (i::GlobalVars->flCurrentTime - mflSimulationTime) <= flRange;
}

int Lagcompensation::FixTickCount(const float& flSimulationTime)
{
	return TIME_TO_TICKS(flSimulationTime + GetClientInterpAmount());
}

void Lagcompensation::UpdateIncomingSequences(INetChannel* pNetChannel)
{
	if (pNetChannel == nullptr)
		return;

	// set to real sequence to update, otherwise needs time to get it work again
	if (nLastIncomingSequence == 0)
		nLastIncomingSequence = pNetChannel->iInSequenceNr;

	// check how much sequences we can spike
	if (pNetChannel->iInSequenceNr > nLastIncomingSequence)
	{
		nLastIncomingSequence = pNetChannel->iInSequenceNr;
		vecSequences.emplace_front(SequenceObject_t(pNetChannel->iInReliableState, pNetChannel->iOutReliableState, pNetChannel->iInSequenceNr, i::GlobalVars->flRealTime));
	}

	// is cached too much sequences
	if (vecSequences.size() > 2048U)
		vecSequences.pop_back();
}

void Lagcompensation::ClearIncomingSequences()
{
	if (!vecSequences.empty())
	{
		nLastIncomingSequence = 0;
		vecSequences.clear();
	}
}

void Lagcompensation::AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency)
{
	for (const auto& sequence : vecSequences)
	{
		if (i::GlobalVars->flRealTime - sequence.flCurrentTime >= flLatency)
		{
			pNetChannel->iInReliableState = sequence.iInReliableState;
			pNetChannel->iInSequenceNr = sequence.iSequenceNr;
			break;
		}
	}
}

void Lagcompensation::LagRecord_t::ApplyMatrix(CBaseEntity* pEntity, EMatrixType iType) {

	switch (iType)
	{
	case VISUAL: pEntity->SetBoneCache(pMatricies[VISUAL]);
		break;
	case RESOLVE:pEntity->SetBoneCache(pMatricies[RESOLVE]);
		break;
	case LEFT:pEntity->SetBoneCache(pMatricies[LEFT]);
		break;
	case RIGHT:pEntity->SetBoneCache(pMatricies[RIGHT]);
		break;
	case CENTER:pEntity->SetBoneCache(pMatricies[CENTER]);
		break;
	default:
		break;
	}
}