#include "Lagcompensation.h"
#include "resolver.h"

void LagComp::UpdateLagRecords() {

	if (!g::pLocal)
		return;

	for (int i = 0; i < 65; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEnt || !pEnt->IsAlive() || pEnt->GetTeam() == g::pLocal->GetTeam() || !i::EngineClient->IsConnected()) {

			deqLagRecords[i].clear();
			continue;
		}

		if (pEnt->IsDormant()) {

			continue;
		}

		if (!deqLagRecords[i].empty()) {

			for (int at = 0; at < deqLagRecords[i].size(); at++) {

				auto pRecords = deqLagRecords[i];
				auto pRecord = pRecords.at(at);

				if (pRecord.bDormant)
					pRecords.erase(pRecords.begin() + at);
			}
		}

		bool bUpdate = deqLagRecords[i].empty() || pEnt->GetSimulationTime() > pEnt->GetOldSimulationTime();

		if (bUpdate && !deqLagRecords[i].empty()) {

			auto layer = pEnt->GetAnimationOverlays()[11];
			auto previousLayer = deqLagRecords[i].front().layer;

			if (layer.flCycle == previousLayer[11].flCycle) {

				pEnt->GetSimulationTime() = pEnt->GetOldSimulationTime();
				bUpdate = false;
			}
		}

		if (bUpdate) {

			if (!deqLagRecords[i].empty()) {

				if ((pEnt->GetVecOrigin() - deqLagRecords[i].front().vecOrigin).LengthSqr() > 4096.f && pEnt->GetSimulationTime() > deqLagRecords[i].front().flSimulationTime)
					deqLagRecords[i].front().bValid = false;
			}
			deqLagRecords[i].emplace_front(playerrecord_t(pEnt));
			UpdatePlayer(pEnt);
		}

		while (deqLagRecords[i].size() > 32)
			deqLagRecords[i].pop_back();
	}
}

void LagComp::UpdatePlayer(CBaseEntity* pEnt) {

	playerrecord_t* pRecord = &deqLagRecords[pEnt->EntIndex()].front();
	playerrecord_t* pPreviousRecord = deqLagRecords[pEnt->EntIndex()].size() >= 2 ? &deqLagRecords[pEnt->EntIndex()].at(1) : nullptr;

	bool bCanDesync = !pEnt->GetPlayerInfo().bFakePlayer;
	CAnimState* pAnimstate = pEnt->AnimState();

	float flCurrentTime = i::GlobalVars->flCurrentTime;
	float flFrameTime = i::GlobalVars->flFrameTime;

	pEnt->GetAnimationLayers(pRecord->layer);

	pAnimstate->flLastUpdateTime -= i::GlobalVars->flIntervalPerTick;

	i::GlobalVars->flCurrentTime = pEnt->GetSimulationTime();
	i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;

	pEnt->GetEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
	pEnt->SetAbsOrigin(pEnt->GetVecOrigin());
	VelocityFix(pEnt, pRecord, pPreviousRecord);

	pEnt->IsClientSideAnimation() = true;
	pAnimstate->Update(pEnt->GetEyeAngles());
	resolver::Resolver(pEnt, pRecord, pPreviousRecord, bCanDesync, pAnimstate);
	pEnt->UpdateClientSideAnimations();
	pEnt->IsClientSideAnimation() = false;

	pEnt->SetupBonesFix(pRecord->matrix);
	pEnt->SetAnimationLayers(pRecord->layer);

	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flFrameTime = flFrameTime;

	pRecord->StoreData(pEnt);
}

float LagComp::LerpTime() {

	static auto cl_interp = i::ConVar->FindVar(("cl_interp"));
	static auto cl_interp_ratio = i::ConVar->FindVar(("cl_interp_ratio"));
	static auto sv_client_min_interp_ratio = i::ConVar->FindVar(("sv_client_min_interp_ratio"));
	static auto sv_client_max_interp_ratio = i::ConVar->FindVar(("sv_client_max_interp_ratio"));
	static auto cl_updaterate = i::ConVar->FindVar(("cl_updaterate"));
	static auto sv_minupdaterate = i::ConVar->FindVar(("sv_minupdaterate"));
	static auto sv_maxupdaterate = i::ConVar->FindVar(("sv_maxupdaterate"));

	auto flUpdateRate = std::clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
	auto flLerpRatio = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());

	return std::clamp(flLerpRatio / flUpdateRate, cl_interp->GetFloat(), 1.0f);
}

void LagComp::VelocityFix(CBaseEntity* pEnt, playerrecord_t* pRecord, playerrecord_t* pPreviousRecord) {

	if (pPreviousRecord == nullptr)
		return;

	auto vecOriginDifference = pEnt->GetVecOrigin() - pPreviousRecord->vecOrigin;
	auto flTimeDifference = pEnt->GetSimulationTime() - pPreviousRecord->flSimulationTime;

	pEnt->GetVelocity() = vecOriginDifference / flTimeDifference;

	float flAnimationSpeed = 0.f;

	if (pEnt->GetFlags() & FL_ONGROUND && pPreviousRecord->nFlags & FL_ONGROUND && pRecord->layer[11].flWeight > 0.f && pRecord->layer[11].flWeight < 1.f && pRecord->layer[11].flPlaybackRate == pPreviousRecord->layer[11].flPlaybackRate) {

		auto flAnimationModifier = 0.35f * (1.f - pRecord->layer[11].flWeight);

		if (flAnimationModifier > 0.f && flAnimationModifier < 1.f)
			flAnimationSpeed = pRecord->flMaxSpeed * (flAnimationModifier + 0.55f);
	}

	if (flAnimationSpeed > 0.f) {
		pEnt->GetVelocity().x *= flAnimationSpeed;
		pEnt->GetVelocity().y *= flAnimationSpeed;
	}

	if (!(pEnt->GetFlags() & FL_ONGROUND))
		pEnt->GetVelocity().z -= i::ConVar->FindVar("sv_gravity")->GetFloat() * flTimeDifference * 0.5f;

	pEnt->SetAbsVelocity(pEnt->GetVelocity());
}

//void LagComp::DisableInterpolation() {
//
//	if (!g::pLocal)
//		return;
//
//	for (int i = 0; i < 65; i++) {
//
//		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));
//
//		if (!pEnt || !pEnt->IsAlive() || pEnt->IsDormant())
//			continue;
//
//		VarMapping_t* pMap = GetVarMap(pEnt);
//		if (!pMap)
//			continue;
//
//		for (int i = 0; i < pMap->m_nInterpolatedEntries; i++) {
//
//			VarMapEntry_t* pEnt = &pMap->m_Entries[i];
//			pEnt->m_bNeedsToInterpolate = false;
//		}
//	}
//}