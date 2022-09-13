#include "Lagcompensation.h"
#include "resolver.h"

void LagComp::UpdateLagRecords() {

	if (!g::pLocal)
		return;

	for (int i = 0; i < 65; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		// sanity checks
		if (!pEnt || !pEnt->IsAlive() || pEnt->GetTeam() == g::pLocal->GetTeam() || !i::EngineClient->IsConnected()) {

			deqLagRecords[i].clear();
			continue;
		}

		// check if this dude is dormant or not
		if (pEnt->IsDormant()) {

			g::bAllowAnimations[pEnt->EntIndex()] = true;
			continue;
		}

		// check for dormancy inside lagrecords and delete them
		if (!deqLagRecords[i].empty()) {

			for (int at = 0; at < deqLagRecords[i].size(); at++) {

				auto pRecords = deqLagRecords[i];
				auto pRecord = pRecords.at(at);

				if (pRecord.bDormant)
					pRecords.erase(pRecords.begin() + at);
			}
		}

		// check if there is time for an update (lets assume the guy is not using break lagcomp)
		bool bUpdate = deqLagRecords[i].empty() || pEnt->GetSimulationTime() > pEnt->GetOldSimulationTime();

		// check if he's cycle is the same as the old cycle, and somehow we got an update
		// this update is a false alert :/// probably breaking lagcomp and simtime going crazy
		if (bUpdate && !deqLagRecords[i].empty()) {

			auto layer = pEnt->GetAnimationOverlays()[11];
			auto previousLayer = deqLagRecords[i].front().layer;

			if (layer.flCycle == previousLayer[11].flCycle) {

				pEnt->GetSimulationTime() = pEnt->GetOldSimulationTime();
				bUpdate = false;
			}
		}

		// let's update
		if (bUpdate) {

			// another check for breaking lagcomp
			if (!deqLagRecords[i].empty()) {

				if ((pEnt->GetVecOrigin() - deqLagRecords[i].front().vecOrigin).LengthSqr() > 4096.f && pEnt->GetSimulationTime() > deqLagRecords[i].front().flSimulationTime)
					deqLagRecords[i].front().bValid = false;
			}
			// create a new record
			deqLagRecords[i].emplace_front(playerrecord_t(pEnt));
			// update this records "animation" and save it
			UpdatePlayer(pEnt);
		}

		// if there's too much record delete the old ones
		// kys
		while (deqLagRecords[i].size() > 32)
			deqLagRecords[i].pop_back();
	}
}

void LagComp::UpdatePlayer(CBaseEntity* pEnt) {

	// get record pointers
	playerrecord_t* pRecord = &deqLagRecords[pEnt->EntIndex()].front();
	playerrecord_t* pPreviousRecord = deqLagRecords[pEnt->EntIndex()].size() >= 2 ? &deqLagRecords[pEnt->EntIndex()].at(1) : nullptr;

	static bool& bInvalidate = **reinterpret_cast<bool**>(util::FindSignature("client.dll", "C6 05 ? ? ? ? ? 89 47 70") + 2);

	// bot check
	bool bCanDesync = !pEnt->GetPlayerInfo().bFakePlayer;
	CAnimState* pAnimstate = pEnt->AnimState();

	float flCurrentTime = i::GlobalVars->flCurrentTime;
	float flFrameTime = i::GlobalVars->flFrameTime;

	pEnt->GetAnimationLayers(pRecord->layer);

	// force a full animation this tick
	pAnimstate->flLastUpdateTime -= i::GlobalVars->flIntervalPerTick;
	// bypass framecount check
	pAnimstate->iLastUpdateFrame--;
	// when calling UpdateClientSideAnimations those values will be updated to the actual last update values that we updated

	// fake lagging player fixes (not proper but does the job against normal fakelags)
	i::GlobalVars->flCurrentTime = pEnt->GetSimulationTime();
	i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;

	// retarded valve things
	// bypasses some bonesetup/accumulate layer stuff
	pEnt->GetEFlags() &= ~EFL_DIRTY_ABSVELOCITY;

	// again, client fucks up velocity, we need to fix it manually (calculate like server does)
	VelocityFix(pEnt, pRecord, pPreviousRecord);

	// thats just pure braincancer (absorigin updated every frame, but server only updated origin every tick)
	// this fixes some origin differences that can cause aimbots to miss moving targets
	if (pPreviousRecord != nullptr) {

		pEnt->SetAbsOrigin(M::Interpolate(pPreviousRecord->vecOrigin, pRecord->vecOrigin, lagcomp.LerpTime()));
		pEnt->SetAbsAngles(M::Interpolate(pPreviousRecord->vecAbsAngles, pRecord->vecAbsAngles, lagcomp.LerpTime()));

		// yeah against bhopping niggers
		M::Extrapolate(pEnt, pPreviousRecord->vecOrigin, pEnt->GetVelocity(), pEnt->GetFlags(), pPreviousRecord->nFlags & FL_ONGROUND);
	}

	// ghetto fix
	if (!(pEnt->GetFlags() & FL_ONGROUND))
		pAnimstate->flDurationInAir = 0.1f;

	const bool bBackupBoneCache = bInvalidate;

	// allow animations this tick/frame whatever
	g::bAllowAnimations[pEnt->EntIndex()] = true;

	// update animationstate to get the latest animationlayers (idk why but this function does update animationlayers)
	pAnimstate->Update(pEnt->GetEyeAngles());

	// do resolver before updating animations so we already have a good resolved angle
	resolver::Resolver(pEnt, pRecord, pPreviousRecord, bCanDesync, pAnimstate);

	// update the data that we want to feed with the server
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/client/cstrike15/c_cs_player.cpp#L5950
	pEnt->UpdateClientSideAnimations();

	// disable animations if its not a valid update time
	// SO CSGO WONT FUCK UP ANIMATIONS	
	g::bAllowAnimations[pEnt->EntIndex()] = false;

	// build our fixed matrix for accurate ragebot matrix (delay is not fixed like this)
	// TODO: fix the freaking delay in enemy and local animations
	pEnt->SetupBonesFix(pRecord->matrix);
	pEnt->SetAnimationLayers(pRecord->layer);

	// thats just some shit when chaning animlayers
	pEnt->InvalidatePhysicsRecursive(ANGLES_CHANGED);
	pEnt->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
	pEnt->InvalidatePhysicsRecursive(SEQUENCE_CHANGED);

	bInvalidate = bBackupBoneCache;

	// backup globals to not fuck up the game
	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flFrameTime = flFrameTime;

	// store this data
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

void LagComp::DisableInterpolation() {

	if (!g::pLocal)
		return;

	for (int i = 0; i < 65; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEnt || !pEnt->IsAlive() || pEnt->IsDormant())
			continue;

		VarMapping_t* pMap = pEnt->GetVarMap();
		if (!pMap)
			continue;

		for (int i = 0; i < pMap->m_nInterpolatedEntries; i++) {

			VarMapEntry_t* pEnt = &pMap->m_Entries[i];
			pEnt->m_bNeedsToInterpolate = false;
		}
	}
}