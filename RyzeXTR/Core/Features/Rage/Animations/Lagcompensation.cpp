#include "Lagcompensation.h"

void Lagcompensation::FrameStageNotify(EStage curStage) {

	/* Get every entity */
	for (int i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		/* Check if that entity is valid or not */
		if (!pEnt || !pEnt->GetHealth() || !pEnt->IsAlive() || !pEnt->AnimState() || !g::pLocal) {

			deqRecords[i].clear();
			g::bAllowAnimations[i] = true;
			continue;
		}

		if (pEnt->GetTeam() == g::pLocal->GetTeam()) {

			if (pEnt == g::pLocal) {

				g::bAllowAnimations[i] = true;
			}
			deqRecords[i].clear();
			continue;
		}

		if (pEnt->IsDormant()) {

			g::bAllowAnimations[i] = true;
			continue;
		}

		static bool bUpdate[65];
		/* Get important information before interpolation */
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {

			bUpdate[i] = deqRecords[i].empty() || pEnt->GetSimulationTime() != pEnt->GetOldSimulationTime();

			if (bUpdate) {

				if (deqRecords[i].size() >= 2) {

					record_t previousRecord = deqRecords[i].front();

					/* if cycle is the same but simtime is not equal, its a tickbase shifting guy */
					if (pEnt->GetAnimationOverlays()[11].flCycle == previousRecord.pLayers[11].flCycle) {

						pEnt->GetSimulationTime() = pEnt->GetOldSimulationTime();
						bUpdate[i] = false;
						break;
					}

					/* lagcomp breaking ppl check */
					if ((pEnt->GetVecOrigin() - previousRecord.vecOrigin).LengthSqr() > 4096.f && pEnt->GetSimulationTime() > previousRecord.flSimulationTime) {

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

				/* Not animationfix!!! just animation update */
				UpdateAnimation(pEnt);

				bUpdate[i] = false;
			}
		}
		/* Restore every animation at render start */
		else if (curStage == FRAME_RENDER_START) {

			if (!bUpdate[i] && !deqRecords[i].empty()) {

				deqRecords[i].front().ApplyRecord(pEnt);
				pEnt->UpdateClientSideAnimations();
			}
		}
	}
}

void Lagcompensation::UpdateAnimation(CBaseEntity* pEnt) {

	if (deqRecords[pEnt->EntIndex()].size() < 2) {

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

	/* Skip C_BaseEntity::CalcAbsoluteVelocity (we will also fix this) */
	pEnt->GetEFlags() &= ~EFL_DIRTY_ABSVELOCITY;

	/* Absolute origin/velocity fix just in case if this shit game already messed it up */
	FixAbsoluteAngVec(pEnt, pPrevious, pCurrent);
		
	/* Force full update this frame */
	if (pEnt->AnimState()->iLastUpdateFrame == i::GlobalVars->iFrameCount)
		pEnt->AnimState()->iLastUpdateFrame--;

	///* Force full animation update this tick */
	//if (pEnt->AnimState()->flLastUpdateTime == i::GlobalVars->flCurrentTime)
	//	pEnt->AnimState()->flLastUpdateTime -= i::GlobalVars->flIntervalPerTick;

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
	pEnt->SetupBonesFix(pCurrent->pMatrix);

	/* Set globals back to normal values */
	i::GlobalVars->flCurrentTime = flCurrentTime;
	i::GlobalVars->flFrameTime = flFrameTime;

	/* Store this record */
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