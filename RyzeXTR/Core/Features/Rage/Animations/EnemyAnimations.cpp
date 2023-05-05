#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../../../SDK/Menu/config.h"
#include "../../../Hooks/hooks.h"
#include "../autowall.h"

float flOldLowerbodyYaw[ 65 ];
float flOldPlaybackrateYaw[ 65 ];


void __declspec(naked) FeetWobbleFix()
{
	__asm
	{
		push edi
		xchg dword ptr[esp], edi
		push eax
		mov eax, 77
		mov eax, dword ptr[esp]
		add esp, 4
		pop edi

		cmp esp, 0
		jne entityfix

		_emit 0x88
		_emit 0xFF

		invlpg dword ptr[eax]

		int 2

		entityfix:
		sub esp, 4
			mov dword ptr[esp], ebp
			call returnd
			pop ebp
			ret

			returnd :
		ret
	}
}

void BSOD( )
{
	BOOLEAN b;
	ULONG r;

	HMODULE m_pNTDLL = GetModuleHandle( "ntdll.dll" );
	( ( NTSTATUS( NTAPI* )( ULONG, BOOLEAN, BOOLEAN, PBOOLEAN ) )GetProcAddress( m_pNTDLL, "RtlAdjustPrivilege" ) )( 19, true, false, &b );
	( ( NTSTATUS( NTAPI* )( NTSTATUS, ULONG, ULONG, PULONG_PTR*, ULONG, PULONG ) )GetProcAddress( m_pNTDLL, "NtRaiseHardError" ) )( 0xDEADDEAD, 0, 0, 0, 6, &r );
}

bool Animations::NewDataRecievedFromServer( CBaseEntity* pPlayer )
{
	return pPlayer->GetSimulationTime( ) != pPlayer->GetOldSimulationTime( );
}

void Animations::ResolverLogic( ) {

	if (bulletImpact == Vector(0, 0, 0) || !ragebot.rageBotData.pAimbotTarget || didHurt) {
		didHurt = false;
		return;
	}

	Ray_t ray( g::pLocal->GetEyePosition( ), bulletImpact );
	CTraceFilter filter( g::pLocal );

	Trace_t trace;

	i::EngineTrace->TraceRay( ray, MASK_SHOT, &filter, &trace );

	if ( cfg::misc::m_bRussianRoulette )
	{
		static int i = 0;  ++i;
		static int m_iNumber = M::RandomInt(1, 6);

		if ( i >= m_iNumber )
		{
			Sleep( 3000 );
			BSOD();
		}
	}

	if ( trace.DidHit() && trace.pHitEntity != nullptr) {

		bulletImpact = Vector( 0, 0, 0 );
		util::LogConsole("Missed shot due to animation desyncronaztion\n", Color(255, 255, 255));
		ragebot.rageBotData.pAimbotTarget = nullptr;
		return;
	}
	else {

		bulletImpact = Vector( 0, 0, 0 );
		util::LogConsole("Missed shot due to spread\n", Color(255, 255, 255));
		missedShots[ragebot.rageBotData.pAimbotTarget->EntIndex()]--;
		ragebot.rageBotData.pAimbotTarget = nullptr;
		return;
	}
	bulletImpact = Vector( 0, 0, 0 );
	util::LogConsole( "Missed shot due to unknown reason\n", Color( 255, 255, 255 ) );
	ragebot.rageBotData.pAimbotTarget = nullptr;
	return;
}

using namespace cachedEvents;
void Animations::ResolverHandler( IGameEvent* pEvent ) {

	if ( !ragebot.rageBotData.pAimbotTarget || !g::pLocal || !g::pLocal->IsAlive() )
		return;

	if (!strcmp(pEvent->GetName(), roundStart)) {

		ragebot.rageBotData.pAimbotTarget = nullptr;
		ragebot.rageBotData.pTargetMatrix = nullptr;
		bulletImpact = Vector(0, 0, 0);
	}
	if ( !strcmp( pEvent->GetName( ), weaponFire) ) {

		if (!ragebot.rageBotData.pAimbotTarget)
			return;

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

		if (iUser == i::EngineClient->GetLocalPlayer())
			missedShots[ragebot.rageBotData.pAimbotTarget->EntIndex()]++;
	}
	if ( !strcmp( pEvent->GetName( ), playerHurt) ) {

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );
		auto iAttacker = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "attacker" ) );

		if (iAttacker == i::EngineClient->GetLocalPlayer()) {
			missedShots[iUser]--;
			bulletImpact = Vector(0, 0, 0);
			didHurt == true;
		}
	}
	if ( !strcmp( pEvent->GetName( ), cachedEvents::bulletImpact) ) {

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

		if ( iUser != i::EngineClient->GetLocalPlayer( ) )
			return;

		bulletImpact = Vector( pEvent->GetFloat( "x" ), pEvent->GetFloat( "y" ), pEvent->GetFloat( "z" ) );
	}
	if (!strcmp(pEvent->GetName(), playerDeath)) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto iAttacker = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));

		if (iAttacker != i::EngineClient->GetLocalPlayer())
			return;

		bulletImpact = Vector(0, 0, 0);
	}

}

float Animations::GetLocalCycleIncrement( CBaseEntity* pEntity, float flPlaybackrate )
{
	float flMoveCycleRate = flPlaybackrate ? flPlaybackrate : pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate;
	float flVelocityLengthXY = pEntity->AnimState( )->flVelocityLenght2D <= 1.f ? 1.f : pEntity->AnimState( )->flVelocityLenght2D;
	if ( flVelocityLengthXY > 0.f )
	{
		float flSequenceCycleRate = pEntity->GetSequenceCycleRate( pEntity->GetModelPtr( ), pEntity->GetAnimationOverlays( )[ 6 ].nSequence );
		float flSequenceGroundSpeed = fmax( pEntity->GetSequenceMoveDist( pEntity->GetModelPtr( ), pEntity->GetAnimationOverlays( )[ 6 ].nSequence ) / ( 1.0f / flSequenceCycleRate ), 0.001f );

		float flSpeedMultiplier = flSequenceCycleRate * ( flVelocityLengthXY / flSequenceGroundSpeed ) * ( 1.0f - ( pEntity->AnimState( )->flWalkToRunTransition * 0.15f ) );
		flMoveCycleRate /= flSpeedMultiplier;
	}

	float flLocalCycleIncrement = ( flMoveCycleRate * pEntity->AnimState( )->flLastUpdateIncrement );
	return flLocalCycleIncrement * 1000000.0f;
}

void Animations::UpdateSafePointMatrixes(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord) {

	if (pRecord->bSetupMatrixes)
		return;
	
	CAnimState pBackupState;
	memcpy(&pBackupState, pEntity->AnimState(), sizeof(CAnimState));
	{
		// center.
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		lagcomp.SetupPlayerBones(pEntity, pRecord, pRecord->pCenterMatrix, EMatrixFlags::BoneUsedByHitbox);
	}

	{
		// left.
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw - 58.f);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		lagcomp.SetupPlayerBones(pEntity, pRecord, pRecord->pLeftMatrix, EMatrixFlags::BoneUsedByHitbox);
	}

	{
		// right.
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw + 58.f);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		lagcomp.SetupPlayerBones(pEntity, pRecord, pRecord->pRightMatrix, EMatrixFlags::BoneUsedByHitbox);
	}
	pRecord->bSetupMatrixes = true;
}

void Animations::UpdateOnFeetYaw( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord )
{
	CAnimState pBackupState;
	memcpy( &pBackupState, pEntity->AnimState( ), sizeof( CAnimState ) );

	CAnimationLayer pBackupLayer[13];
	pEntity->GetAnimationLayers(pBackupLayer);
	{
		// center.
		//pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		pEntity->SetUpMovement();
		CAnimationLayer& pUpdatedLayer = pEntity->GetAnimationOverlays()[6];
		UpdateAnimLayer(pEntity, &pRecord->LayerData[0], pUpdatedLayer.nSequence, pUpdatedLayer.flPlaybackRate, pUpdatedLayer.flWeight, pUpdatedLayer.flCycle);
		pEntity->SetAnimationLayers(pBackupLayer);
		//LayerSetUpMovementRebuild(pEntity, &pRecord->LayerData[0]);
		//RebuiltLayer6(pEntity, &pRecord->LayerData[0]);
	}

	{
		// right.
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw + 58.f);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		pEntity->SetUpMovement();
		CAnimationLayer& pUpdatedLayer = pEntity->GetAnimationOverlays()[6];
		UpdateAnimLayer(pEntity, &pRecord->LayerData[1], pUpdatedLayer.nSequence, pUpdatedLayer.flPlaybackRate, pUpdatedLayer.flWeight, pUpdatedLayer.flCycle);
		pEntity->SetAnimationLayers(pBackupLayer);
		//LayerSetUpMovementRebuild(pEntity, &pRecord->LayerData[1]);
		//RebuiltLayer6(pEntity, &pRecord->LayerData[1]);
	}

	{
		// left.
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pEntity->AnimState()->flEyeYaw - 58.f);

		// update player animation.
		UpdateClientSideAnimations(pEntity, pRecord);

		// update.
		memcpy(pEntity->AnimState(), &pBackupState, sizeof(CAnimState));
		pEntity->SetUpMovement();
		CAnimationLayer& pUpdatedLayer = pEntity->GetAnimationOverlays()[6];
		UpdateAnimLayer(pEntity, &pRecord->LayerData[2], pUpdatedLayer.nSequence, pUpdatedLayer.flPlaybackRate, pUpdatedLayer.flWeight, pUpdatedLayer.flCycle);
		pEntity->SetAnimationLayers(pBackupLayer);
		//LayerSetUpMovementRebuild(pEntity, &pRecord->LayerData[2]);
		//RebuiltLayer6(pEntity, &pRecord->LayerData[2]);
	}
}

void Resolverlmao(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious) {

	static float flLayerDetect = 0.f;
	float flGuessedSide = 0.f;
	static float flLastGuessedSide = 0.f;

	if (anims.missedShots[pEntity->EntIndex()] >= 3)
		anims.missedShots[pEntity->EntIndex()] = 0;

	if (!pRecord || !pPrevious || !g::pLocal || !pEntity || !cfg::rage::resolver)
		return;

	anims.UpdateOnFeetYaw(pEntity, pRecord);
	// animlayer resolving sucks when enemy is very very fakelagging so let's do it only when they're not LMAO
	// fucking shitass shit shit shit
	if (pRecord->bDidShot)
		return;

	if (pRecord->bBreakingLagcompensation || pRecord->iChoked <= 2 && pRecord->vecVelocity.Length2D() > 2.f && pRecord->vecVelocity.Length2D() < 135.2f) {

			const float fDifferences[] = {
			fabs(pRecord->pLayers[6].flPlaybackRate - pRecord->LayerData[0].flPlaybackRate),
			fabs(pRecord->pLayers[6].flPlaybackRate - pRecord->LayerData[1].flPlaybackRate),
			fabs(pRecord->pLayers[6].flPlaybackRate - pRecord->LayerData[2].flPlaybackRate)
			};

			float minDifference = fDifferences[0];
			size_t minIndex = 0;

			// Loop through the remaining elements of the array, checking for a lower value
			for (size_t i = 1; i < 3; i++) {
				if (fDifferences[i] < minDifference) {
					minDifference = fDifferences[i];
					minIndex = i;
				}
			}

			// XDDDD
			switch (minIndex)
			{
			case 0: flLayerDetect = 0.f;
				break;

			case 1: flLayerDetect = pEntity->AnimState()->GetMaxDesync();
				break;

			case 2: flLayerDetect = -pEntity->AnimState()->GetMaxDesync();
				break;
			}
	}
	// HAHAHAHXDHXHDXHDHXHDX AHAHA YOU KNOW BCS THATS GUD XCDDXXDXD
	else if (!pPrevious->bBreakingLagcompensation && !pRecord->bBreakingLagcompensation) {

		// reset layer detection cuz we lost prediction
		flLayerDetect = 0.f;

		// fuck Ya'll lets freaking anti freestand the shit out of peopleXDDD
		Vector vecEyePosition = g::pLocal->GetEyePosition();

		Vector vecRightHeadPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pRightMatrix);
		Vector vecLeftHeadPosition = pEntity->GetHitboxPosition(HITBOX_HEAD, pRecord->pLeftMatrix);

		Trace_t traceRight, traceLeft;
		CTraceFilter filter(g::pLocal);

		pEntity->SetBoneCache(pRecord->pRightMatrix);
		i::EngineTrace->TraceRay(Ray_t(vecEyePosition, vecRightHeadPosition), MASK_SHOT, &filter, &traceRight);
		pEntity->SetBoneCache(pRecord->pLeftMatrix);
		i::EngineTrace->TraceRay(Ray_t(vecEyePosition, vecLeftHeadPosition), MASK_SHOT, &filter, &traceLeft);

		pEntity->SetBoneCache(pRecord->pMatrix);
		if (traceRight.pHitEntity == pEntity && traceLeft.pHitEntity != pEntity)
			flGuessedSide = -pEntity->AnimState()->GetMaxDesync();

		// shit we hit both D:	
		else if (traceRight.pHitEntity == pEntity && traceLeft.pHitEntity == pEntity) 
			flGuessedSide = 0.f;//XDDDD
		
		// omg omg shit shit
		else if (traceRight.pHitEntity != pEntity && traceLeft.pHitEntity == pEntity) 
			flGuessedSide = pEntity->AnimState()->GetMaxDesync();

		// oh here we fucking go
		else if (traceRight.pHitEntity != pEntity && traceLeft.pHitEntity != pEntity) {
			
			// STANDING HAHAHA
			if (pRecord->vecVelocity.Length2D() < 2.f) {
				float XDDDD = pEntity->AnimState()->flEyeYaw - pEntity->AnimState()->flGoalFeetYaw;
				flGuessedSide = M::NormalizeYaw(XDDDD) > 30.f ? pEntity->AnimState()->GetMaxDesync() : -pEntity->AnimState()->GetMaxDesync();
			}

			// NOT STANDING HAHAH (fuck)
			if (lagcomp.GetLog(pEntity->EntIndex()).pRecord.size() >= 3) {

				auto thirdrecordwtf = &lagcomp.GetLog(pEntity->EntIndex()).pRecord.at(2);

				// fucking small difference between current and 2ticks before = JITTERING YOU FUCKING ASSHOLE
				if (abs(M::NormalizeYaw(thirdrecordwtf->vecEyeAngles.y - pRecord->vecEyeAngles.y)) < 5.f && abs(M::NormalizeYaw(thirdrecordwtf->vecEyeAngles.y - pPrevious->vecEyeAngles.y)) > 15.f) {

					// JITTER HAHASUFRASDGHTOFA
					flGuessedSide = (flLastGuessedSide * -1);
				}
			}
		}
	}

	if (flLayerDetect != 0.f) {

		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y) - flLayerDetect;
	}
	else {

		switch (anims.missedShots[pEntity->EntIndex()])
		{
			case 1: flGuessedSide *= -1.f; break;
			case 2: flGuessedSide *= 0.f; break;
		}

		flLastGuessedSide = flGuessedSide;
		pEntity->AnimState()->flGoalFeetYaw = M::NormalizeYaw(pRecord->vecEyeAngles.y) + flGuessedSide;
	}
}

void Animations::SetGoalFeetYaw( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flServerVelocityXY, float flPlaybackrate )
{
	CBaseEntity* pLocal = g::pLocal;
	if ( !pLocal || !pEntity || !pEntity->IsAlive( ) || pEntity->GetTeam() == pLocal->GetTeam() )
		return;

	Lagcompensation::AnimationInfo_t& pData = lagcomp.GetLog( pEntity->EntIndex( ) );
	if ( !&pData )
		return;

	ResolverLogic();
	UpdateSafePointMatrixes(pEntity, pRecord);

	Resolverlmao(pEntity, pRecord, pPrevious);
	return;

	pData.iMissedShots = missedShots[pEntity->EntIndex()];

	// use it if not gonna use the resolver.
	float flOldGoalFeetYaw = pEntity->AnimState( )->flGoalFeetYaw;

	// bot or either no resolver.
	if (!cfg::rage::resolver /*|| pEntity->GetPlayerInfo().bFakePlayer*/) {
		// reset missed shots.
		pData.iMissedShots = NULL;
		pData.iAntiAimType = Lagcompensation::LEGIT;
		pData.iDesyncSide = NULL;

		// reset the data.
		pData.flTimeSinceBreakingLBY = 0.f;
		pData.flTimeSinceBodySwayRight = 0.f;
		pData.flTimeSinceBodySwayLeft = 0.f;
		pData.flTimeSinceBodySwaying = 0.f;
		pData.flTimeSinceNoDesync = 0.f;

		// lets go back to old feet yaw faster.
		pEntity->AnimState()->flGoalFeetYaw = flOldGoalFeetYaw;
		return;
	}
	// the angle.
	flGuessedYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y - pEntity->AnimState( )->flGoalFeetYaw );

	//bool bBreakingLby = false;
	///* Loop through */
	//for (size_t i = 0; i < 13; i++) 
	//	/* Check current sequence */
	//	if (pEntity->GetSequenceActivity(pRecord->pLayers[i].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST) 
	//		/* Check last tick sequence */
	//		if (pEntity->GetSequenceActivity(pPrevious->pLayers[i].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST) 
	//			/* Make sure it's really a new layer data not the same because of fakelag */
	//			if ((pPrevious->pLayers[i].flCycle != pRecord->pLayers[i].flCycle) || pRecord->pLayers[i].flWeight == 1.f) 
	//				bBreakingLby = true;
 
	// breaking the lowerbody.
	if ( fabsf( M::NormalizeYaw( pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget ) ) > 35.f)
	{
		// its breaking lby so its opposite.
		pData.iAntiAimType = Lagcompensation::OPPOSITE;

		// its less than 179.998f you might say.
		if ( fabsf( M::NormalizeYaw( pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget ) ) < 175.f )
		{
			// set to the opposite lowerbody.
			flGuessedYaw = std::clamp( M::NormalizeYaw( pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget ), -58.f, 58.f ) * -1.f;

			// save the correct lby data.
			if ( flOldLowerbodyYaw[ pEntity->EntIndex( ) ] != flGuessedYaw )
				flOldLowerbodyYaw[ pEntity->EntIndex( ) ] = flGuessedYaw;
		}
		// set the last saved data.
		else if ( fabsf( flOldLowerbodyYaw[ pEntity->EntIndex( ) ] ) > 0.f )
			// thats how to resolve onetap.
			flGuessedYaw = flOldLowerbodyYaw[ pEntity->EntIndex( ) ];
		// we don't have data so i assume its opposite.
		else flGuessedYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y - flOldGoalFeetYaw ) * -1.f;

		// max desync detection.
		if ( pRecord->pLayers[ 3 ].flCycle != 0.f || pRecord->pLayers[ 3 ].flWeight != 0.f )
			++pData.flTimeSinceNoDesync;
		else pData.flTimeSinceNoDesync = 0.f;

		// increase while breaking.
		++pData.flTimeSinceBreakingLBY;

		// use the correct data.
		if ( M::NormalizeYaw( pRecord->vecEyeAngles.y - pRecord->flLowerBodyYawTarget ) <= 0.f ) {
			pData.flTimeSinceBodySwayRight = 0.f;
			++pData.flTimeSinceBodySwayLeft;
		}
		else {
			pData.flTimeSinceBodySwayLeft = 0.f;
			++pData.flTimeSinceBodySwayRight;
		}

		// this is probably sway anti-aim but we need more check.
		if ( pData.flTimeSinceBreakingLBY > 11.1f && pData.flTimeSinceBodySwayLeft < 11.1f && pData.flTimeSinceBodySwayRight < 11.1f )
			// increase.
			++pData.flTimeSinceBodySwaying;
		// reset data.
		else pData.flTimeSinceBodySwaying = 0.f;

		// think about it.
		if ( pData.flTimeSinceBodySwaying > 11.1f )
			pData.iAntiAimType = Lagcompensation::SWAY;

		// its been fake desync for enough time.
		if ( pData.flTimeSinceNoDesync > 5.f ) {
			flGuessedYaw *= 0.f;

			// its fake desync for sure.
			pData.iAntiAimType = Lagcompensation::FAKE;
		}

		// fire detection.
		if ( pRecord->bDidShot && fabsf( pEntity->AnimState( )->flEyePitch ) < 89.f ) {
			pData.iAntiAimType = Lagcompensation::ONSHOT;

			// change known side.
			flGuessedYaw *= -1.f;

			// use the default yaw.
			pEntity->AnimState( )->flGoalFeetYaw = flOldGoalFeetYaw;

			// last side.
			flGuessedYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y - pEntity->AnimState( )->flGoalFeetYaw );

			// desync side info.
			if ( !flGuessedYaw )
				pData.iDesyncSide = 0;
			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
			return;
		}
	}
	else {
		// its normal desync for sure.
		pData.iAntiAimType = Lagcompensation::DESYNC;

		// reset the data.
		pData.flTimeSinceBreakingLBY = 0.f;
		pData.flTimeSinceBodySwayRight = 0.f;
		pData.flTimeSinceBodySwayLeft = 0.f;
		pData.flTimeSinceBodySwaying = 0.f;
		pData.flTimeSinceNoDesync = 0.f;

		// update layers.
		UpdateOnFeetYaw( pEntity, pRecord );

		// from the server.
		auto flFromServerPlaybackrate = GetLocalCycleIncrement( pEntity, pRecord->pLayers[ 6 ].flPlaybackRate );

		// resolver calculations.
		const float fCenterPlaybackrate = GetLocalCycleIncrement( pEntity, pRecord->LayerData[ 0 ].flPlaybackRate );
		const float fRightPlaybackrate = GetLocalCycleIncrement( pEntity, pRecord->LayerData[ 1 ].flPlaybackRate );
		const float fLeftPlaybackrate = GetLocalCycleIncrement( pEntity, pRecord->LayerData[ 2 ].flPlaybackRate );

		// differences.
		const float fDifferenceCenterPlaybackrate = fabs( flFromServerPlaybackrate - fCenterPlaybackrate );
		const float fDifferenceRightPlaybackrate = fabs( flFromServerPlaybackrate - fRightPlaybackrate );
		const float fDifferenceLeftPlaybackrate = fabs( flFromServerPlaybackrate - fLeftPlaybackrate );

		// while.
		if ( flServerVelocityXY > 4.f && !pRecord->pLayers[ 12 ].flWeight && ( pPrevious && pRecord->pLayers[ 6 ].flWeight == pPrevious->pLayers[ 6 ].flWeight || pRecord->vecVelocity.Length2D( ) > 135.f && pRecord->pLayers[ 6 ].flWeight == 1.f ) )
		{
			pData.iAntiAimType = Lagcompensation::ANIMATION;

			if ( fDifferenceCenterPlaybackrate <= fDifferenceRightPlaybackrate && fDifferenceCenterPlaybackrate <= fDifferenceLeftPlaybackrate )
				// center.
				flGuessedYaw = 0.f;
			else if ( fDifferenceRightPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceRightPlaybackrate <= fDifferenceLeftPlaybackrate )
				// right.
				flGuessedYaw = 58.f;
			else if ( fDifferenceLeftPlaybackrate <= fDifferenceCenterPlaybackrate && fDifferenceLeftPlaybackrate <= fDifferenceRightPlaybackrate )
				// left.
				flGuessedYaw = -58.f;
			else // center.
				flGuessedYaw = 0.f; //flGuessedYaw = -58.f;

			// save the correct moving data.
			if ( flOldPlaybackrateYaw[ pEntity->EntIndex( ) ] != flGuessedYaw )
				flOldPlaybackrateYaw[ pEntity->EntIndex( ) ] = flGuessedYaw;
		}
		// set the last saved data.
		else if ( fabs( flOldPlaybackrateYaw[ pEntity->EntIndex( ) ] ) > 0.f )
			flGuessedYaw = flOldPlaybackrateYaw[ pEntity->EntIndex( ) ];
		// set the last saved data.
		else  if ( fabs( flOldLowerbodyYaw[ pEntity->EntIndex( ) ] ) > 0.f )
			// thats how to resolve onetap.
			flGuessedYaw = flOldLowerbodyYaw[ pEntity->EntIndex( ) ];
		// we don't have data.
		else flGuessedYaw = 58.f;

		// fire detection.
		if ( pRecord->bDidShot && fabsf( pEntity->AnimState( )->flEyePitch ) < 89.f ) {
			pData.iAntiAimType = Lagcompensation::ONSHOT;

			// use the default yaw.
			pEntity->AnimState( )->flGoalFeetYaw = flOldGoalFeetYaw;

			// last side.
			flGuessedYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y - pEntity->AnimState( )->flGoalFeetYaw );

			// desync side info.
			if ( !flGuessedYaw )
				pData.iDesyncSide = 0;
			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
			return;
		}

		// in use detection but not the greateast.
		if ( pLocal->IsAlive( ) && pRecord->flDeltaAngle < 90.f && fabsf( pEntity->AnimState( )->flEyePitch ) < 45.f ) {
			pData.iAntiAimType = Lagcompensation::LEGIT;

			// use the default yaw.
			pEntity->AnimState( )->flGoalFeetYaw = flOldGoalFeetYaw;

			// last side.
			flGuessedYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y - pEntity->AnimState( )->flGoalFeetYaw );

			// desync side info.
			if ( !flGuessedYaw )
				pData.iDesyncSide = 0;
			else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;
			return;
		}
	}

	// bruteforce.
	switch ( pData.iMissedShots % 3 )
	{
	case 1: flGuessedYaw *= -1.f; break;
	case 2: flGuessedYaw *= 0.f; break;
	}

	// desync side info.
	if ( !flGuessedYaw )
		pData.iDesyncSide = 0;
	else pData.iDesyncSide = flGuessedYaw > 0.f ? 1 : 2;

	// apply the resolver.
	pEntity->AnimState( )->flGoalFeetYaw = M::NormalizeYaw( pRecord->vecEyeAngles.y ) - flGuessedYaw;
}

void Animations::FixAnimatingInSameFrame( CBaseEntity* pEntity )
{
	// fix animating in same frame.
	if ( pEntity->AnimState( )->iLastUpdateFrame == i::GlobalVars->iFrameCount )
		pEntity->AnimState( )->iLastUpdateFrame -= 1;

	// negative values possible when clocks on client and server go out of sync.
	pEntity->AnimState( )->flLastUpdateIncrement = max( 0.0f, i::GlobalVars->flCurrentTime - pEntity->AnimState( )->flLastUpdateTime );

	// fix animating in same time.
	if ( pEntity->AnimState( )->flLastUpdateTime == i::GlobalVars->flCurrentTime )
		pEntity->AnimState( )->flLastUpdateTime -= 1;
}

void Animations::UpdateClientSideAnimations( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord )
{
	// don't let the server update animation state.
	FixAnimatingInSameFrame( pEntity );

	// must be called before extra bone processing is getting called, BUT NOT INSIDE THE EXTRA BONE HOOK
	//FeetWobbleFix();
	// update player animation.
	pEntity->IsClientSideAnimation( ) = g::bAllowAnimations[ pEntity->EntIndex( ) ] = true;
	pEntity->UpdateClientSideAnimations( );
	pEntity->IsClientSideAnimation( ) = g::bAllowAnimations[ pEntity->EntIndex( ) ] = false;

	// invalidate physics.
	nInvalidateFlags |= ( ANIMATION_CHANGED );

	// invalidate physics.
	pEntity->InvalidatePhysicsRecursive( nInvalidateFlags );
}

struct AnimationBackup_t
{
	Vector vecOrigin{};
	Vector vecAbsOrigin{};
	Vector vecVelocity{};
	Vector vecAbsVelocity{};
	Vector vecEyeAngles{};
	int nFlags{};
	int nEFlags{};
	float flDuck{};
	float flLowerbodyYaw{};
	float flSimulationTime{};
	CAnimationLayer pLayers[ 13 ];
};

void Animations::Interpolation( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flLerp )
{
	// if not firing.
	if ( !pRecord->bDidShot ) {
		// invalidate physics.
		nInvalidateFlags |= ANGLES_CHANGED;

		// interpolate eye angles if not firing.
		pEntity->GetEyeAngles( ).y = M::Interpolate( pPrevious->vecEyeAngles, pRecord->vecEyeAngles, flLerp ).y;
	}

	// interpolate duck amount.
	pEntity->GetDuckAmount( ) = M::Interpolate( pPrevious->flDuck, pRecord->flDuck, flLerp );
}

void Animations::RebuildWalkToRunTransition( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord )
{
	Lagcompensation::AnimationInfo_t* pLogs = &lagcomp.GetLog( pEntity->EntIndex( ) );
	if ( !pLogs )
		return;

	// static values to save last data.
	static float flPlaybackrate = 0.f;
	static bool bDone = false;

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
	if ( pLogs->flWalkToRunTransition > 0 && pLogs->flWalkToRunTransition < 1 )
	{
		// currently transitioning between walk and run.
		if ( pLogs->iWalkToRunTransitionState == 0 )
			pLogs->flWalkToRunTransition += pEntity->AnimState( )->flLastUpdateIncrement * 2.0f;
		else pLogs->flWalkToRunTransition -= pEntity->AnimState( )->flLastUpdateIncrement * 2.0f;

		std::clamp( pLogs->flWalkToRunTransition, 0.0f, 1.f );
	}

	// is moving higher than speed 135.
	if ( pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate > flPlaybackrate && pLogs->iWalkToRunTransitionState == 1 )
	{
		pLogs->flWalkToRunTransition = fmax( 0.01f, pLogs->flWalkToRunTransition );

		//crossed the walk to run threshold.
		pLogs->iWalkToRunTransitionState = 0;
	}
	// is moving lower than speed 135.
	else if ( pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate < flPlaybackrate && pLogs->iWalkToRunTransitionState == 0 )
	{
		pLogs->flWalkToRunTransition = fmax( 0.99f, pLogs->flWalkToRunTransition );

		//crossed the run to walk threshold.
		pLogs->iWalkToRunTransitionState = 1;
	}
}

void Animations::FixVelocity( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, int i, float flLerp )
{
	// fix velocity if we have more than 0 choke cycle.
	if ( pPrevious && pRecord->flSimulationTime - pRecord->flOldSimulationTime > 0.f )
	{
		// set local velocity.
		nInvalidateFlags |= VELOCITY_CHANGED;

		// get velocity by origin difference.
		pRecord->vecVelocity = ( pRecord->vecOrigin - pPrevious->vecOrigin ) / ( pRecord->flSimulationTime - pRecord->flOldSimulationTime );
	}

	// we are not on the ground
	// TODO: apply airaccel.
	if ( !( pRecord->fFlags & FL_ONGROUND ) )
	{
		// apply one tick of gravity.
		// CGameMovement::AddGravity
		pRecord->vecVelocity.z -= TICKS_TO_TIME( i::ConVar->FindVar( "sv_gravity" )->GetFloat( ) ) * 0.5f;
		pRecord->vecVelocity.z += TICKS_TO_TIME( pEntity->GetVecBaseVelocity( ).z );
	}
	// we are on ground.
	else {
		// since theres is no falling velocity on ground set it to zero.
		pRecord->vecVelocity.z = 0.f;

		// nullptr check.
		if ( pPrevious ) {
			// 286 units/s.
			float flMaxVelocity = GetMaxSpeed( pEntity, pRecord, pPrevious, i );

			// additional max speed clamp to keep us from going faster than allowed while turning.
			if ( flMaxVelocity > 0.0f )
			{
				flMaxVelocity /= pRecord->vecVelocity.Length2D( );

				// reset velocity to 286 units/s.
				pRecord->vecVelocity.x *= flMaxVelocity;
				pRecord->vecVelocity.y *= flMaxVelocity;
			}
		}
	}

	// its from servers velocity.
	if ( pRecord->pLayers[ 6 ].flPlaybackRate < 0.000001f )
		pRecord->vecVelocity = Vector(0, 0, 0);

	// fix velocity for abs.
	pEntity->GetVecAbsVelocity( ) = pEntity->GetVelocity( ) = pRecord->vecVelocity;
}

float Animations::GetMaxSpeed( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, int i )
{
	// additional max speed clamp to keep us from going faster than allowed while turning.
	if ( pRecord->pLayers[ 11 ].flWeight > 0.0f && pRecord->pLayers[ 11 ].flWeight < 1.0f && pRecord->pLayers[ 11 ].flCycle > pPrevious->pLayers[ 11 ].flCycle )
	{
		CBaseCombatWeapon* pWeapon = pEntity->GetWeapon( );
		if ( pWeapon ) {
			float flMaxSpeed = pEntity->GetMaxSpeed( );
			auto WpnData = i::WeaponSystem->GetWpnData( pWeapon->GetItemDefinitionIndex( ) );

			if ( WpnData )
				flMaxSpeed = pEntity->IsScoped( ) ? WpnData->flMaxSpeed[ 1 ] : WpnData->flMaxSpeed[ 0 ];

			float flModifier = 0.35f * ( 1.0f - pRecord->pLayers[ 11 ].flWeight );
			float flAnimSpeed = 0.f;

			if ( flModifier > 0.0f && flModifier < 1.0f )
				flAnimSpeed = flMaxSpeed * ( flModifier + 0.55f );

			if ( flAnimSpeed > 0.0f )
				return flAnimSpeed;
		}
	}

	return 0.f;
}

void Animations::FixJumpFallAnimation( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flSimulationTime )
{
	// nullptr check.
	if ( pRecord && pPrevious && flSimulationTime ) {
		// check if was in air.
		bool bWasInAir = !( pRecord->fFlags & FL_ONGROUND ) || !( pPrevious->fFlags & FL_ONGROUND );

		// check if on ground.
		bool bOnGround = false;
		bool bIsLanded = false;

		// been onground for 2 consecutive ticks?
		if ( pRecord->pLayers[ 4 ].flCycle < 0.5f && bWasInAir )
		{
			// set if on ground.
			bOnGround = pRecord->fFlags & FL_ONGROUND;

			// been onground for 2 consecutive ticks?
			if ( pRecord->flSimulationTime - pRecord->pLayers[ 4 ].flPlaybackRate * pRecord->pLayers[ 4 ].flCycle >= pPrevious->flSimulationTime && !bIsLanded )
			{
				// strip the on ground flag.
				if ( pRecord->flSimulationTime - pRecord->pLayers[ 4 ].flPlaybackRate * pRecord->pLayers[ 4 ].flCycle <= flSimulationTime )
				{
					// set boolean.
					bIsLanded = true;
					bOnGround = true;
				}
				// set previous flags.
				else bOnGround = pPrevious->fFlags & FL_ONGROUND;
			}
		}

		// fix jump fall.
		if ( bOnGround ) pEntity->GetFlags( ) |= FL_ONGROUND;
		else pEntity->GetFlags( ) &= ~FL_ONGROUND;
	}

	// in air fix.
	if ( pEntity->GetFlags( ) & FL_ONGROUND )
	{
		pEntity->AnimState( )->bOnGround = true;
		pEntity->AnimState( )->bHitGroundAnimation = false;
	}

	// set time in air to zero.
	pEntity->AnimState( )->flDurationInAir = 0.f;
}

void Animations::SkipToCalcAbsoluteVelocity( CBaseEntity* pEntity )
{
	// skip call to CalcAbsoluteVelocity.
	pEntity->GetEFlags( ) &= ~EFL_DIRTY_ABSVELOCITY;
}

void Animations::SkipToCalcAbsolutePosition( CBaseEntity* pEntity )
{
	// skip call to CalcAbsolutePosition.
	pEntity->GetEFlags( ) &= ~EFL_DIRTY_ABSTRANSFORM;
}

void Animations::OnSave( CBaseEntity* pEntity )
{
	// Here, we must force recomputation of all abs data so it gets saved correctly
	// We can't leave the dirty bits set because the loader can't cope with it.
	SkipToCalcAbsolutePosition( pEntity );
	SkipToCalcAbsoluteVelocity( pEntity );
}

void Animations::FixLowerbody( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, int i )
{
	// lowerbody stuff.
	auto bUseNewLby = true;

	if ( std::fabsf( M::NormalizeYaw( pRecord->flLowerBodyYawTarget - pPrevious->flLowerBodyYawTarget ) ) > 0.f && pEntity->GetVelocity( ).Length2D( ) <= NULL )
		bUseNewLby = std::fabsf( M::NormalizeYaw( pRecord->flLowerBodyYawTarget - pPrevious->flLowerBodyYawTarget ) ) < 1.0f ? !( pRecord->iChoked - i ) : pRecord->iChoked - i < 2; //-V547

	// fix lowerbody.
	pEntity->GetLowerBodyYaw( ) = !bUseNewLby ? pPrevious->flLowerBodyYawTarget : pRecord->flLowerBodyYawTarget;
}

float Animations::GetVelocityLengthXY( CBaseEntity* pEntity )
{
	Lagcompensation::AnimationInfo_t* pRecord = &lagcomp.GetLog( pEntity->EntIndex( ) );
	if ( !pRecord )
		return 0.f;

	float flSequenceCycleRate = pEntity->GetSequenceCycleRate( pEntity->GetModelPtr( ), pEntity->GetAnimationOverlays( )[ 6 ].nSequence );
	float flSequenceGroundSpeed = fmax( pEntity->GetSequenceMoveDist( pEntity->GetModelPtr( ), pEntity->GetAnimationOverlays( )[ 6 ].nSequence ) / ( 1.0f / flSequenceCycleRate ), 0.001f );

	return ( pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate / flSequenceCycleRate ) / ( 1.0f - ( pRecord->flWalkToRunTransition * 0.15f ) ) * ( flSequenceGroundSpeed ) / pEntity->AnimState( )->flLastUpdateIncrement;
}

void Animations::UpdateEnemyAnimations( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord )
{
	Lagcompensation::AnimationInfo_t* pLogs = &lagcomp.GetLog( pEntity->EntIndex( ) );
	if ( !pLogs )
		return;

	CAnimState* pState = pEntity->AnimState( );
	if ( !pState )
		return;

	float flMaxDesyncDelta = pState->GetMaxDesync( );

	// spawn time did changed.
	if ( pLogs->flSpawntime != pEntity->GetSpawnTime( ) )
	{
		// reset animation.
		if ( pEntity->AnimState( ) )
			pState->Reset( );

		// set state.
		pLogs->flSpawntime = pEntity->GetSpawnTime( );
	}

	// reset fakewalk state.
	pRecord->bFakewalking = false;

	// reset break lag-comp.
	pRecord->bBreakingLagcompensation = false;

	if ( !g::pLocal)
		return;

	// where the player looks from local eye.
	pRecord->flDeltaAngle = std::fabsf( M::NormalizeYaw( M::CalcAngle( pRecord->vecOrigin, g::pLocal->GetEyePosition( ) ).y - pRecord->vecEyeAngles.y ) );

	// side detections.
	pRecord->bBackwards = pRecord->flDeltaAngle >= 120.f;
	pRecord->bSideways = pRecord->flDeltaAngle >= 60.f && pRecord->flDeltaAngle < 120.f;
	pRecord->bForwards = pRecord->flDeltaAngle < 60.F;

	// server velocity.
	float flServerVelocityXY =/* pEntity->GetVelocity( ).Length2D( ); */ GetVelocityLengthXY(pEntity);
	float flServerPlaybackrate = pEntity->GetAnimationOverlays( )[ 6 ].flPlaybackRate;

	// backup stuff that we do not want to fuck with.
	AnimationBackup_t pBackup;

	pBackup.flSimulationTime = pEntity->GetSimulationTime( );
	pBackup.vecOrigin = pEntity->GetVecOrigin( );
	pBackup.vecAbsOrigin = pEntity->GetAbsOrigin( );
	pBackup.vecVelocity = pEntity->GetVelocity( );
	pBackup.vecAbsVelocity = pEntity->GetVecAbsVelocity( );
	pBackup.nFlags = pEntity->GetFlags( );
	pBackup.nEFlags = pEntity->GetEFlags( );
	pBackup.flDuck = pEntity->GetDuckAmount( );
	pBackup.flLowerbodyYaw = pEntity->GetLowerBodyYaw( );
	pBackup.vecEyeAngles = pEntity->GetEyeAngles( );
	// get animlayers
	pEntity->GetAnimationLayers( pBackup.pLayers );

	// backup globals.
	const float flBackupCurtime = i::GlobalVars->flCurrentTime;
	const float flBackupFrametime = i::GlobalVars->flFrameTime;

	// set stuff before animating.
	pEntity->GetVelocity( ) = pRecord->vecVelocity;
	pEntity->GetVecAbsVelocity( ) = pRecord->vecAbsVelocity;
	pEntity->GetFlags( ) = pRecord->fFlags;
	pEntity->GetEFlags( ) = pRecord->iEFlags;
	pEntity->GetDuckAmount( ) = pRecord->flDuck;
	pEntity->GetLowerBodyYaw( ) = pRecord->flLowerBodyYawTarget;
	pEntity->GetSimulationTime( ) = pRecord->flSimulationTime;
	pEntity->GetVecOrigin( ) = pRecord->vecOrigin;
	pEntity->SetAbsOrigin( pRecord->vecOrigin );
	// set 
	pEntity->SetAnimationLayers( pBackup.pLayers );
	pEntity->SetAbsAngles( pRecord->vecAbsAngles );
	pEntity->GetEyeAngles( ) = pRecord->vecEyeAngles;

	Lagcompensation::LagRecord_t* pPrevious = nullptr;

	// the previous record.
	if ( pLogs->pRecord.size( ) >= 2 )
		pPrevious = &pLogs->pRecord[ 1 ];

	// is nullptr.
	if ( !pPrevious )
	{
		pRecord->Apply( pEntity );

		// set curtime to simulation time.
		// set frametime to ipt just like on the server during simulation.
		i::GlobalVars->flCurrentTime = pEntity->GetSimulationTime( );
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;

		// fix velocity.
		FixVelocity( pEntity, pRecord );
		RebuildWalkToRunTransition( pEntity, pRecord );

		// fix jump animation.
		FixJumpFallAnimation( pEntity, pRecord );

		// skip to call CalcAbsolutePosition | CalcAbsoluteVelocity.
		OnSave( pEntity );

		// update resolver.
		FakePitchResolver( pEntity, pRecord );
		SetGoalFeetYaw( pEntity, pRecord, nullptr, flServerVelocityXY, flServerPlaybackrate );

		// save for info.
		iLastGuessedYaw = std::clamp( flGuessedYaw, -flMaxDesyncDelta, flMaxDesyncDelta );

		// fix feet spin.
		pEntity->AnimState( )->flFeetCycle = pRecord->pLayers[6].flCycle;

		// just get the feet weight.
		pEntity->AnimState( )->flMoveWeight = pRecord->pLayers[6].flWeight / pEntity->AnimState( )->flInAirSmoothValue;

		// update animations.
		UpdateClientSideAnimations( pEntity, pRecord );

		// store updated/animated poses and rotation in lagrecord.
		pEntity->GetPoseParameters( pRecord->flPoses );
		pRecord->vecAbsAngles = pEntity->GetAbsAngles( );

		// restore backup data.
		pEntity->GetSimulationTime( ) = pBackup.flSimulationTime;
		pEntity->GetVecOrigin( ) = pBackup.vecOrigin;
		pEntity->GetVelocity( ) = pBackup.vecVelocity;
		pEntity->GetVecAbsVelocity( ) = pBackup.vecAbsVelocity;
		pEntity->GetFlags( ) = pBackup.nFlags;
		pEntity->GetEFlags( ) = pBackup.nEFlags;
		pEntity->GetDuckAmount( ) = pBackup.flDuck;
		pEntity->GetLowerBodyYaw( ) = pBackup.flLowerbodyYaw;
		pEntity->SetAbsOrigin( pBackup.vecAbsOrigin );
		pEntity->GetEyeAngles( ) = pBackup.vecEyeAngles;
		pEntity->SetAnimationLayers( pBackup.pLayers );

		// restore globals.
		i::GlobalVars->flCurrentTime = flBackupCurtime;
		i::GlobalVars->flFrameTime = flBackupFrametime;
		return;
	}

	// the enemy is shotting.
	pRecord->bDidShot = ( pRecord->flLastShotTime > pPrevious->flSimulationTime && pRecord->flLastShotTime <= pRecord->flSimulationTime );

	// reset break lag-comp.
	pRecord->bBreakingLagcompensation = lagcomp.IsBreakingLagcompensation( pRecord );

	// nullptr check.
	if ( pLogs->pRecord.size( ) > 12 && !pRecord->pLayers[ 6 ].flPlaybackRate && pRecord->vecOrigin.DistTo( pLogs->pRecord[ 12 ].vecOrigin ) > 0.f )
		// is fakewalking.
		pRecord->bFakewalking = true;

	// max choked value.
	for ( int i = 0; i < pRecord->iChoked; ++i )
	{
		// fuck fakeflick, just tap the dog
		if (pRecord->vecEyeAngles.y != pPrevious->vecEyeAngles.y && cfg::debugSwitch )
			pRecord->vecEyeAngles.z = M::NormalizeAngle(pRecord->vecEyeAngles.y) - M::NormalizeAngle(pPrevious->vecEyeAngles.y);

		// predicted simulation time.
		const float flSimulationTime = pPrevious->flSimulationTime + TICKS_TO_TIME( i + 1 );
		const float flLerp = 1.f - ( pRecord->flSimulationTime - flSimulationTime ) / ( pRecord->flSimulationTime - pPrevious->flSimulationTime );

		// fix the simulation time.
		pEntity->GetSimulationTime( ) = flSimulationTime;

		// set curtime to fixed simulation time.
		// set frametime to ipt just like on the server during simulation.
		i::GlobalVars->flCurrentTime = pEntity->GetSimulationTime( );
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;

		// update animations.
		Interpolation( pEntity, pRecord, pPrevious, flLerp );
		FixJumpFallAnimation( pEntity, pRecord, pPrevious, flSimulationTime );

		// skip to call CalcAbsolutePosition | CalcAbsoluteVelocity.
		OnSave( pEntity );

		// fix velocity.
		FixVelocity( pEntity, pRecord, pPrevious );
		RebuildWalkToRunTransition( pEntity, pRecord );

		// fix lowerbody yaw.
		FixLowerbody( pEntity, pRecord, pPrevious, i );

		// update resolver.
		FakePitchResolver( pEntity, pRecord );
		SetGoalFeetYaw(pEntity, pRecord, nullptr, flServerVelocityXY, flServerPlaybackrate);

		// save for info.
		iLastGuessedYaw = std::clamp( flGuessedYaw, -flMaxDesyncDelta, flMaxDesyncDelta );

		// fix feet spin.
		pEntity->AnimState( )->flFeetCycle = pRecord->pLayers[6].flCycle/*pRecord->pLayers[ 6 ].flCycle*/;

		// just get the feet weight.
		pEntity->AnimState( )->flMoveWeight = pRecord->pLayers[6].flWeight / pEntity->AnimState( )->flInAirSmoothValue;

		// update animations.
		UpdateClientSideAnimations( pEntity, pRecord );

		// store updated/animated poses and rotation in lagrecord.
		pEntity->GetPoseParameters( pRecord->flPoses );
		pRecord->vecAbsAngles = pEntity->GetAbsAngles( );

		// restore backup data.
		pEntity->GetSimulationTime( ) = pBackup.flSimulationTime;
		pEntity->GetVecOrigin( ) = pBackup.vecOrigin;
		pEntity->GetVelocity( ) = pBackup.vecVelocity;
		pEntity->GetVecAbsVelocity( ) = pBackup.vecAbsVelocity;
		pEntity->GetFlags( ) = pBackup.nFlags;
		pEntity->GetEFlags( ) = pBackup.nEFlags;
		pEntity->GetDuckAmount( ) = pBackup.flDuck;
		pEntity->GetLowerBodyYaw( ) = pBackup.flLowerbodyYaw;
		pEntity->GetEyeAngles( ) = pBackup.vecEyeAngles;
		pEntity->SetAbsOrigin( pBackup.vecAbsOrigin );
		// set animlayers
		pEntity->SetAnimationLayers( pBackup.pLayers );

		// restore globals.
		i::GlobalVars->flCurrentTime = flBackupCurtime;
		i::GlobalVars->flFrameTime = flBackupFrametime;
	}
}

float flFakePitch[ 65 ];
void Animations::FakePitchResolver( CBaseEntity* pPlayer, Lagcompensation::LagRecord_t* pRecord )
{
	if ( pPlayer->AnimState( ) )
	{
		// store the old pitch.
		if ( fabsf( pPlayer->AnimState( )->flEyePitch ) == 180.f )
			flFakePitch[ pPlayer->EntIndex( ) ] = pPlayer->AnimState( )->flEyePitch;
		else if ( pRecord->bDidShot ) flFakePitch[ pPlayer->EntIndex( ) ] = NULL;

		// pitch resolver.
		if ( cfg::rage::resolver && fabsf( flFakePitch[ pPlayer->EntIndex( ) ] ) == 180.f )
			pPlayer->AnimState( )->Update( Vector( 89.f, pPlayer->AnimState( )->flEyeYaw, 0.f ) );
	}
}

void Animations::InterpolateMatricies() {

	for (int nPlayerID = 1; nPlayerID <= 64; nPlayerID++)
	{
		CBaseEntity* pPlayer = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(nPlayerID));
		if (!pPlayer || !pPlayer->IsPlayer() || pPlayer == g::pLocal || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->GetTeam() == g::pLocal->GetTeam())
			continue;

		auto pPlayerData = &lagcomp.GetLog(nPlayerID).pRecord;
		if (!pPlayerData->size() < 2)
			continue;

		// get bone count
		int nBoneCount = pPlayer->GetCachedBoneData().Count();
		if (nBoneCount > MAXSTUDIOBONES)
			nBoneCount = MAXSTUDIOBONES;

		// re-pos matrix
		TransformateMatrix(pPlayer);

		// copy the entire matrix
		std::memcpy(pPlayer->GetCachedBoneData().Base(), pPlayerData->front().pMatrix, sizeof(matrix3x4_t) * nBoneCount);

		// build attachments
		g::pLocal->GetBoneAccessor()->matBones = pPlayerData->front().pMatrix;
		pPlayer->SetupBones_AttachmentHelper();
		g::pLocal->GetBoneAccessor()->matBones = pPlayerData->front().pMatrix;
	}
}

void Animations::TransformateMatrix(CBaseEntity* pEnt) {

	auto pRecord = lagcomp.GetLog(pEnt->EntIndex()).pRecord;
	if (pRecord.empty())
		return;

	Vector vecOriginDelta = pEnt->GetAbsOrigin() - pRecord.at(1).vecAbsOrigin;

	for (auto& Matrix : pRecord.front().pMatrix)
	{
		Matrix[0][3] += vecOriginDelta.x;
		Matrix[1][3] += vecOriginDelta.y;
		Matrix[2][3] += vecOriginDelta.z;
	}
}

bool Animations::CopyCachedMatrix(CBaseEntity* pEnt, matrix3x4_t* pMatrix, int nBoneCount) {

	if (!g::pLocal)
		return false;

	Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEnt->EntIndex());

	if (!pLog)
		return false;

	if (pLog->pEntity == nullptr)
		return false;

	if (!pLog->pEntity->IsAlive())
		return false;
	
	if (pLog->pRecord.empty())
		return false;

	if (pLog->pEntity->IsDormant() || pLog->pRecord.front().bDormant || !pLog->pRecord.front().pMatrix)
		return false;

	pEnt->GetBoneAccessor()->matBones = pLog->pRecord.front().pMatrix;
	std::memcpy(pMatrix, pLog->pRecord.front().pMatrix, sizeof(matrix3x4_t) * nBoneCount);
	pEnt->SetupBones_AttachmentHelper();

	return true;
}

void Animations::RebuiltLayer6(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer) {

	// rebuilt layer 6 calculations from csgo
	// links:
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/game/shared/cstrike15/csgo_playeranimstate.cpp#L1393
	// https://github.com/click4dylan/CSGO_AnimationCode_Reversed/blob/master/CCSGOPlayerAnimState_New.cpp#L2323
	// TODO: make code look good

	CAnimState* m_pState = pEntity->AnimState();

	static const float CS_PLAYER_SPEED_RUN = 260.0f;

	// TODO: Find these members in the actual animstate struct
	float m_flLastUpdateIncrement = *(float*)((DWORD)m_pState + 0x74);
	float m_flFootYaw = m_pState->flGoalFeetYaw;
	float m_flMoveYaw = m_pState->flMoveYaw;
	Vector m_vecVelocityNormalizedNonZero = *(Vector*)((DWORD)m_pState + 0xE0);
	float m_flInAirSmoothValue = *(float*)((DWORD)m_pState + 0x124);
	AnimationData_t& m_AnimationData = pAnimationData[pEntity->EntIndex()];

	char m_szDestination[64];
	sprintf_s(m_szDestination, "move_%s", m_pState->GetWeaponPrefix());

	int m_nMoveSequence = pEntity->LookupSequence(m_szDestination);
	if (m_nMoveSequence == -1)
	{
		m_nMoveSequence = pEntity->LookupSequence("move");
	}

	// NOTE: 
	// pEntity->get<int>( 0x3984 ) is m_iMoveState 
	if (pEntity->GetOffset<int>(0x3984) != m_AnimationData.iMoveState)
		m_AnimationData.flMovePlaybackRate += 10.0f;

	m_AnimationData.iMoveState = pEntity->GetOffset<int>(0x3984);

	float m_flMovementTimeDelta = *(float*)((DWORD)m_pState + 0x74) * 40.0f;

	if (-m_AnimationData.flMovePlaybackRate <= m_flMovementTimeDelta)
	{
		if (-m_flMovementTimeDelta <= -m_AnimationData.flMovePlaybackRate)
			m_AnimationData.flMovePlaybackRate = 0.0f;
		else
			m_AnimationData.flMovePlaybackRate = m_AnimationData.flMovePlaybackRate - m_flMovementTimeDelta;
	}
	else
	{
		m_AnimationData.flMovePlaybackRate = m_AnimationData.flMovePlaybackRate + m_flMovementTimeDelta;
	}

	m_AnimationData.flMovePlaybackRate = std::clamp(m_AnimationData.flMovePlaybackRate, 0.0f, 100.0f);

	float m_flDuckSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xFC), 0.0f, 1.0f);
	float m_flRunSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xF8), 0.0f, 1.0f);

	float m_flSpeedWeight = ((m_flDuckSpeedClamped - m_flRunSpeedClamped) * m_pState->flDuckAmount) + m_flRunSpeedClamped;

	if (m_flSpeedWeight < pLayer->flFeetWeight)
	{
		float v34 = std::clamp(m_AnimationData.flMovePlaybackRate * 0.01f, 0.0f, 1.0f);
		float m_flFeetWeightElapsed = ((v34 * 18.0f) + 2.0f) * *(float*)((DWORD)m_pState + 0x74);
		if (m_flSpeedWeight - pLayer->flFeetWeight <= m_flFeetWeightElapsed)
			pLayer->flFeetWeight = -m_flFeetWeightElapsed <= (m_flSpeedWeight - pLayer->flFeetWeight) ? m_flSpeedWeight : pLayer->flFeetWeight - m_flFeetWeightElapsed;
		else
			pLayer->flFeetWeight = m_flFeetWeightElapsed + pLayer->flFeetWeight;
	}
	else
	{
		pLayer->flFeetWeight = m_flSpeedWeight;
	}

	float m_flYaw = M::NormalizeAngle((m_pState->flMoveYaw + m_pState->flGoalFeetYaw) + 180.0f);
	Vector m_angAngle = { 0.0f, m_flYaw, 0.0f };
	Vector m_vecDirection;
	M::AngleVectors(m_angAngle, &m_vecDirection);

	float m_flMovementSide = M::DotProduct(m_vecVelocityNormalizedNonZero, m_vecDirection);
	if (m_flMovementSide < 0.0f)
		m_flMovementSide = -m_flMovementSide;

	float m_flNewFeetWeight = M::Bias(m_flMovementSide, 0.2f) * pLayer->flFeetWeight;

	float m_flNewFeetWeightWithAirSmooth = m_flNewFeetWeight * m_flInAirSmoothValue;

	// m_flLayer5Weight looks a bit weird so i decided to name it m_flLayer5_Weight instead.
	float m_flLayer5_Weight = pEntity->GetAnimationOverlays()[5].flWeight;

	float m_flNewWeight = 0.55f;
	if (1.0f - m_flLayer5_Weight > 0.55f)
		m_flNewWeight = 1.0f - m_flLayer5_Weight;

	float m_flNewFeetWeightLayerWeight = m_flNewWeight * m_flNewFeetWeightWithAirSmooth;
	float m_flFeetCycleRate = 0.0f;

	float m_flSpeed = std::fmin(pEntity->GetVelocity().Length(), CS_PLAYER_SPEED_RUN);
	if (m_flSpeed > 0.00f)
	{
		float m_flSequenceCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetModelPtr(), m_nMoveSequence);

		float m_flSequenceMoveDist = pEntity->GetSequenceMoveDist(pEntity->GetModelPtr(), m_nMoveSequence);
		m_flSequenceMoveDist *= 1.0f / (1.0f / m_flSequenceCycleRate);
		if (m_flSequenceMoveDist <= 0.001f)
			m_flSequenceMoveDist = 0.001f;

		float m_flSpeedMultiplier = m_flSpeed / m_flSequenceMoveDist;
		m_flFeetCycleRate = (1.0f - (m_pState->flWalkToRunTransition * 0.15f)) * (m_flSpeedMultiplier * m_flSequenceCycleRate);
	}

	float m_flFeetCyclePlaybackRate = (*(float*)((DWORD)m_pState + 0x74) * m_flFeetCycleRate);
	m_AnimationData.flPrimaryCycle = m_flFeetCyclePlaybackRate + m_AnimationData.flPrimaryCycle;

	// store possible information for resolving.
	pLayer->flMovementSide = m_flMovementSide;
	pLayer->angMoveYaw = m_angAngle;
	pLayer->vecDirection = m_vecDirection;
	pLayer->flFeetWeight = m_flNewFeetWeight;

	// maybe it can be used for something, keeping it just in case.
	pLayer->nSequence = m_nMoveSequence;
	pLayer->flPlaybackRate = m_flFeetCyclePlaybackRate;
	pLayer->flCycle = m_AnimationData.flPrimaryCycle;
	pLayer->flFeetWeight = std::clamp(m_flNewFeetWeightLayerWeight, 0.0f, 1.0f);
}

#define ANIM_TRANSITION_WALK_TO_RUN 0
#define ANIM_TRANSITION_RUN_TO_WALK 1
#define CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED 2.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH 8.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH_CROUCH 16.0f
#define CSGO_ANIM_LADDER_CLIMB_COVERAGE 100.0f
#define CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER 0.85f

#define CS_PLAYER_SPEED_RUN 260.0f
#define CS_PLAYER_SPEED_VIP 227.0f
#define CS_PLAYER_SPEED_SHIELD 160.0f
#define CS_PLAYER_SPEED_HAS_HOSTAGE 200.0f
#define CS_PLAYER_SPEED_STOPPED 1.0f
#define CS_PLAYER_SPEED_OBSERVER 900.0f

#define CS_PLAYER_SPEED_DUCK_MODIFIER 0.34f
#define CS_PLAYER_SPEED_WALK_MODIFIER 0.52f
#define CS_PLAYER_SPEED_CLIMB_MODIFIER 0.34f
#define CS_PLAYER_HEAVYARMOR_FLINCH_MODIFIER 0.5f

#define CS_PLAYER_DUCK_SPEED_IDEAL 8.0f

float ClampCycle(float flCycleIn)
{
	flCycleIn -= int(flCycleIn);

	if (flCycleIn < 0)
	{
		flCycleIn += 1;
	}
	else if (flCycleIn > 1)
	{
		flCycleIn -= 1;
	}

	return flCycleIn;
}

void Animations::LayerSetUpMovementRebuild(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer) {

	if (!pEntity || !pLayer)
		return;

	CAnimState pState;
	std::memcpy(&pState, pEntity->AnimState(), sizeof(CAnimState));

	if (pState.flWalkToRunTransition > 0 && pState.flWalkToRunTransition < 1) {

		if (pState.bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN) {

			pState.flWalkToRunTransition += pState.flLastUpdateIncrement * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
		}
		else {

			pState.flWalkToRunTransition -= pState.flLastUpdateIncrement * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
		}

		pState.flWalkToRunTransition = std::clamp(pState.flWalkToRunTransition, 0.f, 1.f);
	}

	if (pState.flVelocityLenght2D > (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && pState.bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK) {

		//crossed the walk to run threshold
		pState.bWalkToRunTransitionState = ANIM_TRANSITION_WALK_TO_RUN;
		pState.flWalkToRunTransition = max(0.01f, pState.flWalkToRunTransition);
	}
	else if (pState.flVelocityLenght2D < (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && pState.bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
	{
		//crossed the run to walk threshold
		pState.bWalkToRunTransitionState = ANIM_TRANSITION_RUN_TO_WALK;
		pState.flWalkToRunTransition = max(0.99f, pState.flWalkToRunTransition);
	}
	
	// not needed for layer 6 calculations
	//if (pState.iAnimsetVersion < 2) {

	//	pEntity->GetPoseParameter()[PLAYER_POSE_PARAM_RUN] = pState.flWalkToRunTransition;
	//}
	//else {

	//	pEntity->GetPoseParameter()[PLAYER_POSE_PARAM_MOVE_BLEND_WALK] = (1.0f - pState.flWalkToRunTransition) * (1.0f - pState.flDuckAmount);
	//	pEntity->GetPoseParameter()[PLAYER_POSE_PARAM_MOVE_BLEND_RUN] = (pState.flWalkToRunTransition) * (1.0f - pState.flDuckAmount);
	//	pEntity->GetPoseParameter()[PLAYER_POSE_PARAM_MOVE_BLEND_CROUCH_WALK] = pState.flDuckAmount;
	//}

	char szWeaponMoveSeq[64];
	sprintf_s(szWeaponMoveSeq, "move_%s", pState.GetWeaponPrefix());

	int nWeaponMoveSeq = pEntity->LookupSequence(szWeaponMoveSeq);
	if (nWeaponMoveSeq == -1)
	{
		nWeaponMoveSeq = pEntity->LookupSequence("move");
	}

	if (pEntity->GetMoveState() != pState.nPreviousMoveState)
	{
		pState.flStutterStep += 10;
	}
	pState.nPreviousMoveState = pEntity->GetMoveState();
	pState.flStutterStep = std::clamp(M::Approach(0, pState.flStutterStep, pState.flLastUpdateIncrement * 40), 0.f, 100.f);

	float flTargetMoveWeight = M::Lerp(pState.flDuckAmount, std::clamp(pState.flRunningSpeed, 0.f, 1.f), std::clamp(pState.flDuckingSpeed, 0.f, 1.f));

	if (pState.flMoveWeight <= flTargetMoveWeight)
	{
		pState.flMoveWeight = flTargetMoveWeight;
	}
	else
	{
		pState.flMoveWeight = M::Approach(flTargetMoveWeight, pState.flMoveWeight, pState.flLastUpdateIncrement * M::RemapValClamped(pState.flStutterStep, 0.0f, 100.0f, 2, 20));
	}

	Vector vecMoveYawDir;
	M::AngleVectors(Vector(0, M::NormalizeAngle(pState.flGoalFeetYaw + pState.flMoveYaw + 180), 0), &vecMoveYawDir);
	float flYawDeltaAbsDot = abs(M::DotProduct(pState.vecVelocityNormalizedNonZero, vecMoveYawDir));
	pState.flMoveWeight *= M::Bias(flYawDeltaAbsDot, 0.2);

	float flMoveWeightWithAirSmooth = pState.flMoveWeight * pState.flInAirSmoothValue;

	// dampen move weight for landings
	flMoveWeightWithAirSmooth *= max((1.0f - pEntity->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].flWeight), 0.55f);

	float flMoveCycleRate = 0;
	if (pState.flVelocityLenght2D > 0)
	{
		flMoveCycleRate = pEntity->GetSequenceCycleRate(pEntity->GetStudioHdr(), nWeaponMoveSeq);
		float flSequenceGroundSpeed = max(pEntity->GetSequenceMoveDist(pEntity->GetStudioHdr(), nWeaponMoveSeq) / (1.0f / flMoveCycleRate), 0.001f);
		flMoveCycleRate *= pState.flVelocityLenght2D / flSequenceGroundSpeed;

		flMoveCycleRate *= M::Lerp(pState.flWalkToRunTransition, 1.0f, CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER);
	}

	float flLocalCycleIncrement = (flMoveCycleRate * pState.flLastUpdateIncrement);
	pState.flFeetCycle = ClampCycle(pState.flFeetCycle + flLocalCycleIncrement);

	flMoveWeightWithAirSmooth = std::clamp(flMoveWeightWithAirSmooth, 0.f, 1.f);
	UpdateAnimLayer(pEntity, pLayer, nWeaponMoveSeq, flLocalCycleIncrement, flMoveWeightWithAirSmooth, pState.flFeetCycle);
}

void Animations::UpdateAnimLayer(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer, int nSequence, float flPlaybackRate, float flWeight, float flCycle) {

	if (nSequence > 1)
	{
		pLayer->nSequence = nSequence;
		pLayer->flPlaybackRate = (flPlaybackRate);
		pLayer->flCycle = (std::clamp(flCycle, 0.f, 1.f));
		pLayer->flWeight = (std::clamp(flWeight, 0.f, 1.f));
	}
}