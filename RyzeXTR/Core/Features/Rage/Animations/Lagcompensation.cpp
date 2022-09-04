#include "Lagcompensation.h"
#include "resolver.h"

bool LagComp::playerrecord_t::IsValid(float flSimulationTime, bool bValid, float flRange) {

	if (!bValid)
		return false;

	INetChannelInfo* pNetChannelInfo = i::EngineClient->GetNetChannelInfo();

	if (!pNetChannelInfo)
		return false;

	static CConVar* sv_maxunlag = i::ConVar->FindVar("sv_maxunlag");

	const float flIncoming = pNetChannelInfo->GetLatency(FLOW_INCOMING);
	const float flOutGoing = pNetChannelInfo->GetLatency(FLOW_OUTGOING);

	const float flCorrect = std::clamp(flIncoming + flOutGoing + lagcomp.LerpTime(), 0.f, sv_maxunlag->GetFloat());

	return fabsf(flCorrect - (i::GlobalVars->flCurrentTime - flSimulationTime)) < flRange;
}

void LagComp::playerrecord_t::Apply(CBaseEntity* pEnt) {

	pEnt->SetPoseParameters(flPoseParamater);
	pEnt->GetEyeAngles() = vecEyeAngles;
	pEnt->GetVelocity() = vecVelocity;
	pEnt->GetLowerBodyYaw() = flLowerBodyYawTarget;
	pEnt->GetDuckAmount() = flDuckAmount;
	pEnt->GetFlags() = nFlags;
	pEnt->GetVecOrigin() = vecOrigin;
	pEnt->SetAbsOrigin(vecOrigin);
	if (pEnt->AnimState())
		memcpy(pEnt->AnimState(), pAnimstate, sizeof(CAnimState));

	memcpy(pEnt->GetCachedBoneData().Base(), matrix, pEnt->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
}

void LagComp::playerrecord_t::Restore(CBaseEntity* pEnt) {

	pEnt->GetVelocity() = vecVelocity;
	pEnt->GetFlags() = nFlags;
	pEnt->GetEFlags() = nEFlags;
	pEnt->GetDuckAmount() = flDuckAmount;
	pEnt->SetAnimationLayers(layer);
	pEnt->GetLowerBodyYaw() = flLowerBodyYawTarget;
	pEnt->GetVecOrigin() = vecOrigin;
	pEnt->SetAbsOrigin(vecOrigin);

	memcpy(pEnt->GetCachedBoneData().Base(), matrix, pEnt->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
}

float LagComp::LerpTime() {

	static CConVar* cl_interp = i::ConVar->FindVar("cl_interp");
	static CConVar* cl_interp_ratio = i::ConVar->FindVar("cl_interp_ratio");
	static CConVar* cl_updaterate = i::ConVar->FindVar("cl_updaterate");

	return max(cl_interp->GetFloat(), cl_interp_ratio->GetFloat() / cl_updaterate->GetFloat());
}

void LagComp::PostPlayerUpdate() {

	for (int i = 0; i < 65; i++) {

		static float flOldSimulationTime[65];
		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!g::pLocal || !pEnt || !pEnt->IsAlive() || pEnt->IsDormant() || g::pLocal->GetTeam() == pEnt->GetTeam() || !i::EngineClient->IsConnected() || !i::EngineClient->IsInGame()) {

			if (!deqLagRecords[i].empty())
				deqLagRecords[i].clear();

			continue;
		}

		if (flOldSimulationTime[i] < pEnt->GetSimulationTime()) {

			deqLagRecords[i].emplace_front(playerrecord_t(pEnt));
			playerrecord_t pBackup = playerrecord_t(pEnt);
			playerrecord_t* pRecord = &deqLagRecords[i].front();
			playerrecord_t* pPrevious = deqLagRecords[i].size() >= 2 ? &deqLagRecords[i].at(1) : nullptr;

			UpdateAnimations(pEnt, pRecord, pPrevious);

			pEnt->SetAnimationLayers(pBackup.layer);
			pEnt->SetupBonesFix(pRecord->matrix);

			pBackup.Restore(pEnt);

			flOldSimulationTime[i] = pEnt->GetSimulationTime();
		}

		while (deqLagRecords[i].size() > 32)
			deqLagRecords[i].pop_back();
	}
}

void LagComp::UpdateAnimations(CBaseEntity* pEnt, playerrecord_t* pRecord, playerrecord_t* pPrevious) {

	if (pPrevious == nullptr) {

		resolver::Resolver(pEnt, pRecord, pPrevious, true, pEnt->AnimState());

		pRecord->vecVelocity = pEnt->GetVelocity();
		pRecord->Apply(pEnt);

		UpdatePlayer(pEnt);

		return;
	}

	Vector vecVelocity = pEnt->GetVelocity();

	pEnt->SetAnimationLayers(pRecord->layer);
	pEnt->SetAbsOrigin(pRecord->vecOrigin);
	pEnt->SetAbsAngles(pRecord->vecAbsAngles);
	pEnt->GetVelocity() = pPrevious->vecVelocity;

	pRecord->vecVelocity = vecVelocity;

	pRecord->bDidShot = pRecord->flLastShotTime > pPrevious->flSimulationTime && pRecord->flLastShotTime <= pRecord->flSimulationTime;

	Vector vecPrevOrigin = pPrevious->vecOrigin;
	int nPrevFlags = pPrevious->nFlags;

	for (int i = 0; i < pRecord->nChoked; ++i) {

		const float flSimulationTime = pPrevious->flSimulationTime + TICKS_TO_TIME(i + 1);
		const float flLerp = 1.f - (pRecord->flSimulationTime - flSimulationTime) / (pRecord->flSimulationTime - pPrevious->flSimulationTime);

		if (!pRecord->bDidShot) {

			Vector vecEyeAngles = M::Interpolate(pPrevious->vecEyeAngles, pRecord->vecEyeAngles, flLerp);
			pEnt->GetEyeAngles().y = vecEyeAngles.y;
		}

		pEnt->GetDuckAmount() = M::Interpolate(pPrevious->flDuckAmount, pRecord->flDuckAmount, flLerp);

		if (pRecord->nChoked - 1 == i) {

			pEnt->GetVelocity() = vecVelocity;
			pEnt->GetFlags() = pRecord->nFlags;
		}
		else {

			M::Extrapolate(pEnt, vecPrevOrigin, pEnt->GetVelocity(), pEnt->GetFlags(), nPrevFlags & FL_ONGROUND);
			nPrevFlags = pEnt->GetFlags();

			pRecord->vecVelocity = (pRecord->vecOrigin - pPrevious->vecOrigin) * (1.f / TICKS_TO_TIME(pRecord->nChoked));
		}

		if (pRecord->bDidShot) {

			pEnt->GetEyeAngles() = pPrevious->vecEyeAngles;

			if (pRecord->flLastShotTime <= flSimulationTime) {
				pEnt->GetEyeAngles() = pRecord->vecEyeAngles;
			}
		}

		const float flBackupSimulationTime = pEnt->GetSimulationTime();

		pEnt->GetSimulationTime() = flSimulationTime;

		resolver::Resolver(pEnt, pRecord, pPrevious, true, pEnt->AnimState());
		UpdatePlayer(pEnt);

		pEnt->GetSimulationTime() = flBackupSimulationTime;
	}
}

void LagComp::UpdatePlayer(CBaseEntity* pEnt) {

	static bool& bInvalidateBoneCache = **reinterpret_cast<bool**>(util::FindSignature("client.dll", "C6 05 ? ? ? ? ? 89 47 70") + 0x2);

	const float flFrameTime = i::GlobalVars->flFrameTime;
	const float flCurtime = i::GlobalVars->flCurrentTime;

	CAnimState* const pAnimstate = pEnt->AnimState();

	if (!pAnimstate)
		return;

	i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
	i::GlobalVars->flCurrentTime = pEnt->GetSimulationTime();

	pEnt->GetEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
	pEnt->AnimState()->flDurationInAir = 0.1f;

	if (pAnimstate->iLastUpdateFrame == i::GlobalVars->iFrameCount)
		pAnimstate->iLastUpdateFrame -= 1;

	const bool backupBoneCache = bInvalidateBoneCache;

	pEnt->IsClientSideAnimation() = true;

	pEnt->AnimState()->Update(pEnt->GetEyeAngles());
	pEnt->UpdateClientSideAnimations();

	pEnt->IsClientSideAnimation() = false;

	pEnt->InvalidatePhysicsRecursive(ANGLES_CHANGED);
	pEnt->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
	pEnt->InvalidatePhysicsRecursive(SEQUENCE_CHANGED);

	bInvalidateBoneCache = backupBoneCache;

	i::GlobalVars->flFrameTime = flFrameTime;
	i::GlobalVars->flCurrentTime = flCurtime;
}