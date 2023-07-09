#include "Lagcompensation.h"
#include "EnemyAnimations.h"
#include "../../Networking/networking.h"
#include "../../../SDK/InputSystem.h"
#include "../exploits.h"
#include "../../Misc/Playerlist.h"

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
	bImmune = pEntity->HasImmunity();
	
	flAnimationTime = pEntity->GetOldSimulationTime() + i::GlobalVars->flIntervalPerTick;
	flSimulationTime = pEntity->GetSimulationTime();
	flOldSimulationTime = pEntity->GetOldSimulationTime();
	flServerTick = i::ClientState->clockDriftMgr.nServerTick;
	flLastShotTime = pWeapon ? pWeapon->GetLastShotTime() : 0.f;
	flDuck = pEntity->GetDuckAmount();
	flLowerBodyYawTarget = pEntity->GetLowerBodyYaw();
	flEyeYaw = pEntity->AnimState()->flEyeYaw;
	flInterpTime = 0.f;
	flMaxSpeed = pWeapon ? pEntity->IsScoped() ? pWeapon->GetCSWpnData()->flMaxSpeed[0] : pWeapon->GetCSWpnData()->flMaxSpeed[1] : 260.f;
	flThirdPersonRecoil = pEntity->GetThirdpersonRecoil();

	vecEyeAngles = pEntity->GetEyeAngles();
	vecAbsAngles = pEntity->GetAbsAngles();

	iFlags = pEntity->GetFlags();
	iEFlags = pEntity->GetEFlags();
	iEffects = pEntity->GetEffects();
	iChoked = TIME_TO_TICKS(flSimulationTime - flOldSimulationTime);
	iChoked = std::clamp(iChoked, 1, 16);
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
	if (!Backup)
		ApplyMatrix(pEntity, RESOLVE);
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

void Lagcompensation::FrameStageNotify() noexcept {

	if (!g::pLocal || g::bUpdatingSkins)
		return;

	for (int i = 1; i <= i::GlobalVars->nMaxClients; i++) {

		auto pLog = &pPlayerLogs[i];

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));
		if (!pEntity || !pEntity->IsPlayer() || !pEntity->IsAlive() || /*pEntity->GetTeam() == g::pLocal->GetTeam() ||*/ pEntity == g::pLocal) {
			anims.arrMissedShots[i] = 0;
			pLog->iLastResolve = 0;
			pLog->iLastValid = 0;
			pLog->iFirstValid = 32;
			pLog->bLeftDormancy = true;
			pLog->flExploitTime = 0.f;
			if (pEntity != g::pLocal)
				g::bAllowAnimations[i] = true;
			if (pEntity == g::pLocal && !pEntity->IsAlive())
				g::bAllowAnimations[i] = true;
			
			continue;
		}

		if (pEntity != pLog->pEntity) {
			pLog->flExploitTime = 0.f;
			pLog->iLastValid = 0;
			pLog->iFirstValid = 32;
			pLog->pRecord.clear();
		}

		pLog->pEntity = pEntity;

		if (pEntity->GetSimulationTime() == pEntity->GetOldSimulationTime())
			continue;

		if (pEntity->IsDormant()) {
			pLog->flExploitTime = 0.f;
			pLog->iLastValid = 0;
			pLog->iFirstValid = 32;
			pLog->bLeftDormancy = true;
			continue;
		}

		LagRecord_t pPrevious;
		pPrevious.bRestoreData = false;
		if (!pLog->pRecord.empty()) {
			pPrevious = pLog->pRecord.front();
			pPrevious.bRestoreData = true;
		}

		Lagcompensation::LagRecord_t pRecord(pPlayerLogs[i].pEntity);
		if (pPrevious.bRestoreData) {
			if (pPrevious.pLayers[11].flCycle == pRecord.pLayers[11].flCycle){
				pEntity->GetSimulationTime() = pPrevious.flSimulationTime;
				continue;
			}
		}

		if (pPrevious.flSimulationTime > pRecord.flSimulationTime) {

			pLog->flExploitTime = pEntity->GetSimulationTime();
			//pLog->pRecord.clear();
		}

		if (pLog->bLeftDormancy) 
			pLog->pRecord.clear();
		
		if (pRecord.flSimulationTime <= pLog->flExploitTime) {
			//pRecord.bValid = false;
			pRecord.bBreakingLagcompensation = true;
		}

		if (pPrevious.bRestoreData) {
			if ((pRecord.vecOrigin - pPrevious.vecOrigin).Length2DSqr() > 4096.0f) {
				pRecord.bBreakingLagcompensation = true;
				//pLog->pRecord.clear();
			}
		}

		if (pRecord.flSimulationTime < pPrevious.flSimulationTime) {

			//pRecord.bValid = false;
			pRecord.bBreakingLagcompensation = true;

			if (pPrevious.bRestoreData)
				pRecord.iChoked = TIME_TO_TICKS(pRecord.flOldSimulationTime - pPrevious.flOldSimulationTime);
			else
				pRecord.iChoked = pPrevious.iChoked;
		}
		else {
			if (pPrevious.bRestoreData)
				pRecord.iChoked = TIME_TO_TICKS(pRecord.flSimulationTime - pPrevious.flSimulationTime);
			else
				pRecord.iChoked = TIME_TO_TICKS(pRecord.flSimulationTime - pRecord.flOldSimulationTime);
		}

		if (pLog->bLeftDormancy)
			pRecord.bFirstAfterDormant = true;

		pLog->pRecord.push_front(pRecord);
		while (pLog->pRecord.size() > 32)
			pLog->pRecord.pop_back();

		anims.RebuildEnemyAnimations(pEntity, &pLog->pRecord.front(), pLog);

		pLog->bLeftDormancy = false;

		//FilterRecords();
		pPlayerLogs[i].iFirstValid = 32;
		for (auto j = 0u; j < pPlayerLogs[i].pRecord.size(); j++) {

			auto& pCurrentRecord = pLog->pRecord.at(j);

			if (pCurrentRecord.bValid = lagcomp.IsValidRecord(pCurrentRecord.flSimulationTime))
				pPlayerLogs[i].iLastValid = j;

			if (pCurrentRecord.bValid && pPlayerLogs[i].iFirstValid > j)
				pPlayerLogs[i].iFirstValid = j;

			if (pRecord.bBreakingLagcompensation)
				pCurrentRecord.bBreakingLagcompensation = true;
		}
	}
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
		float flMaxTickAmt = /*C::Get<bool>( Vars.bRageSafeBacktracking ) ? std::ceil( I::ConVar->FindVar( Str( "sv_maxunlag" ) )->GetFloat( ) * ( 1.f / I::Globals->m_flIntervalPerTick ) ) + 3 :*/ 32;

		// no need to store insane amount of data.
		while (pPlayerLogs[i].pRecord.size() > flMaxTickAmt)
			pPlayerLogs[i].pRecord.pop_back();

		for (auto j = 0u; j < pPlayerLogs[i].pRecord.size(); j++)
		{
			auto& pCurrentRecord = pPlayerLogs[i].pRecord[j];
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

		if (playerList::arrPlayers[i].iIndex != i)
			continue;

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity() || pEntity == g::pLocal)
			continue;

		void* m_VarMap = *(void**)((DWORD)(pEntity)+0x24);
		if (m_VarMap)
		{
			*(float*)(*(DWORD*)((DWORD)(m_VarMap)+0x8) + 0x24) = i::GlobalVars->flIntervalPerTick;
			*(float*)(*(DWORD*)((DWORD)(m_VarMap)+0x44) + 0x24) = i::GlobalVars->flIntervalPerTick;
		}
	}
}

//bool Lagcompensation::IsBreakingLagcompensation(Lagcompensation::LagRecord_t* pLagRecord)
//{
//	Lagcompensation::AnimationInfo_t& pInfo = lagcomp.GetLog(pLagRecord->iEntIndex);
//
//	// check if we have at least one entry.
//	if (!&pInfo || pInfo.pRecord.size() <= 0)
//		return false;
//
//	Vector previousOrigin = pLagRecord->pEntity->GetAbsOrigin();
//
//	bool m_bFoundRecord = false;
//
//	Lagcompensation::LagRecord_t* pPrevious = nullptr;
//
//	// the previous record.
//	if (pInfo.pRecord.size() >= 2)
//		pPrevious = &pInfo.pRecord[1];
//
//	// walk context looking for any invalidating event.
//	for (Lagcompensation::LagRecord_t& pRecord : pInfo.pRecord)
//	{
//		if (!pRecord.pEntity->IsAlive())
//		{
//			return false;
//		}
//
//		Vector delta = pRecord.vecOrigin - previousOrigin;
//		if (delta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
//		{
//			//ExtrapolatePlayer(pRecord.pEntity, &pRecord, pPrevious);
//
//			// lost track, too much difference.
//			return true;
//		}
//
//		// player is abusing tickbase and breaking lagcompensation
//		if (pRecord.flSimulationTime < pRecord.flOldSimulationTime)
//		{
//			return true;
//		}
//		else if (&pRecord && (pInfo.pEntity->GetSimulationTime() == pRecord.flSimulationTime))
//			return true;
//
//		previousOrigin = pRecord.vecOrigin;
//	}
//	return false;
//}

void Lagcompensation::ExtrapolatePlayer(CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* pCurrent, Lagcompensation::LagRecord_t* pPrevious) const
{
	if (!pPrevious)
		return;

	static CConVar* sv_gravity = i::ConVar->FindVar("sv_gravity");
	static CConVar* sv_jump_impulse = i::ConVar->FindVar("sv_jump_impulse");

	Vector velocity = pCurrent->vecVelocity;
	int flags = pCurrent->iFlags;

	if (!(flags & FL_ONGROUND))
		velocity.z -= (i::GlobalVars->flFrameTime * sv_gravity->GetFloat());
	else if (pPrevious->iFlags & FL_ONGROUND && !(pCurrent->iFlags & FL_ONGROUND))
		velocity.z = sv_jump_impulse->GetFloat();

	const Vector mins = m_pEntity->vecMins();
	const Vector max = m_pEntity->vecMaxs();

	const Vector src = pCurrent->vecOrigin;
	Vector end = src + (velocity * i::GlobalVars->flFrameTime);

	Ray_t ray(src, end, mins, max);

	CGameTrace trace;
	CTraceFilter filter(m_pEntity);

	i::EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	if (trace.flFraction != 1.f)
	{
		for (int i = 0; i < 2; i++)
		{
			velocity -= trace.plane.vecNormal * velocity.DotProduct(trace.plane.vecNormal);

			const float dot = velocity.DotProduct(trace.plane.vecNormal);
			if (dot < 0.f)
			{
				velocity.x -= dot * trace.plane.vecNormal.x;
				velocity.y -= dot * trace.plane.vecNormal.y;
				velocity.z -= dot * trace.plane.vecNormal.z;
			}

			end = trace.vecEnd + (velocity * (i::GlobalVars->flIntervalPerTick * (1.f - trace.flFraction)));

			ray = Ray_t(trace.vecEnd, end, mins, max);
			i::EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.flFraction == 1.f)
				break;
		}
	}

	pCurrent->vecOrigin = trace.vecEnd;
	end = trace.vecEnd;
	end.z -= 2.f;

	ray = Ray_t(pCurrent->vecOrigin, end, mins, max);
	i::EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	flags &= ~(1 << 0);

	if (trace.DidHit() && trace.plane.vecNormal.z > 0.7f)
		flags |= (1 << 0);

	pCurrent->flSimulationTime += i::GlobalVars->flIntervalPerTick;
	m_pEntity->GetVecOrigin() = pCurrent->vecOrigin;
	m_pEntity->SetAbsOrigin(pCurrent->vecOrigin);

	pCurrent->vecVelocity = velocity;
	m_pEntity->GetVelocity() = velocity;

	//g::drawList.push_back(pCurrent->vecOrigin);

	/*CSimulationData simulationData;

	simulationData.pEntity = m_pEntity;
	simulationData.vecOrigin = m_pCurrentRecord->vecOrigin;
	simulationData.vecVelocity = m_pCurrentRecord->vecVelocity;
	simulationData.bOnGround = m_pCurrentRecord->iFlags & FL_ONGROUND;
	simulationData.bDataFilled = true;

	int iSimulationTickDelta = std::clamp(m_pCurrentRecord->iChoked, 1, 15);
	int iDeltaTicks = (std::clamp(TIME_TO_TICKS(i::EngineClient->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + i::EngineClient->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING)) + i::GlobalVars->iTickCount -
		TIME_TO_TICKS(m_pCurrentRecord->flSimulationTime + lagcomp.GetClientInterpAmount()), 0, 100)) - iSimulationTickDelta;

	static CConVar* sv_gravity = i::ConVar->FindVar("sv_gravity");

	if (iDeltaTicks > 0 && simulationData.bDataFilled)
	{
		for (; iDeltaTicks >= 0; iDeltaTicks -= iSimulationTickDelta)
		{
			int iTicksLeft = iSimulationTickDelta;
			do
			{
				Vector vecPredictedOrigin = simulationData.vecOrigin;
				float flTimeToExtrapolate = TICKS_TO_TIME(i::GlobalVars->iTickCount) - m_pEntity->GetSimulationTime();
				float flChokeTimeDelta = TICKS_TO_TIME(m_pCurrentRecord->iChoked);
				float flGravity = sv_gravity->GetFloat();

				static auto PredictNextVelocity = [=](Vector vecCurrent, Vector vecPrevious)
				{
					Vector vecOutput = vecPrevious;

					if (vecCurrent == vecPrevious)
						return vecCurrent;

					if (vecOutput.Length2D() < 0.1f)
						return Vector(0, 0, 0);

					auto a = (vecPrevious - vecCurrent) / flChokeTimeDelta;
					vecOutput += a * flTimeToExtrapolate;

					return vecOutput;
				};

				vecPredictedOrigin = PredictNextVelocity(m_pCurrentRecord->vecVelocity, m_pPrevious->vecVelocity);
				vecPredictedOrigin.z += simulationData.vecVelocity.z - flGravity * flTimeToExtrapolate;

				m_pCurrentRecord->flSimulationTime = m_pEntity->GetSimulationTime() + flTimeToExtrapolate;
				--iTicksLeft;

			} while (iTicksLeft);
		}

		m_pCurrentRecord->vecOrigin = simulationData.vecOrigin;
		m_pCurrentRecord->vecAbsOrigin = simulationData.vecOrigin;
	}*/
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

	auto NetChannelInfo = i::EngineClient->GetNetChannelInfo();

	/* Lagcomp breaking = invalid */
	if ((i::GlobalVars->flCurrentTime - mflSimulationTime) < 0.f)
		return false;

	static CConVar* sv_maxunlag = i::ConVar->FindVar("sv_maxunlag");

	int iTickBase = networking.GetCorrectedTickbase();
	const float flLerpTime = GetClientInterpAmount();
	float flLatency = NetChannelInfo->GetLatency(FLOW_INCOMING) + NetChannelInfo->GetLatency(FLOW_OUTGOING);

	if (cfg::rage::doubletap && IPT::HandleInput(cfg::rage::doubletapkey) && exploits::iTicksToStore > 0)
		iTickBase -= TICKS_TO_TIME(14);

	float flDeltaTime = fminf(flLatency + flLerpTime, sv_maxunlag->GetFloat()) - TICKS_TO_TIME(iTickBase - TIME_TO_TICKS(mflSimulationTime));
	if (fabs(flDeltaTime) >= flRange)
		return false;

	int nDeadTime = (int)((float)(TICKS_TO_TIME(i::GlobalVars->iTickCount + TIME_TO_TICKS(flLatency))) - flRange);
	if (TIME_TO_TICKS(mflSimulationTime + flLerpTime) < nDeadTime)
		return false;

	return true;
	return (i::GlobalVars->flCurrentTime - mflSimulationTime) < flRange;
}

int Lagcompensation::FixTickCount(const float flSimulationTime)
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

	if (!pMatricies[iType]->GetOrigin().IsValid())
		return;
	
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

	//if (vecMins.IsValid() && vecMaxs.IsValid())
	//	pEntity->SetCollisionBounds(vecMins, vecMaxs);
	//return pEntity->InvalidateBoneCache();
}

void Lagcompensation::StartLagcompensation(CBaseEntity* pLocal) {

	if (g::bUpdatingSkins)
		return;

	for (size_t i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsAlive() || !pEntity->GetModel() || pEntity->IsDormant())
			continue;

		arrBackupData[i].first = Lagcompensation::LagRecord_t(pEntity);
		pEntity->GetAnimationLayers(arrBackupData[i].first.pLayers);
		pEntity->GetPoseParameters(arrBackupData[i].first.flPoses);
		pEntity->GetBoneCache(arrBackupData[i].first.pMatricies[VISUAL]);
		arrBackupData[i].second = true;
	}
}

void Lagcompensation::FinishLagcompensation(CBaseEntity* pLocal) {

	if (g::bUpdatingSkins)
		return;

	for (size_t i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!arrBackupData[i].second)
			continue;

		pEntity->SetAnimationLayers(arrBackupData[i].first.pLayers);
		pEntity->SetPoseParameters(arrBackupData[i].first.flPoses);
		pEntity->SetBoneCache(arrBackupData[i].first.pMatricies[VISUAL]);
		arrBackupData[i].first.Apply(pEntity, true);
		arrBackupData[i].second = false;
	}
}