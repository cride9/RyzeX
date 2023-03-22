#include "EnemyAnimations.h"
#include "../ragebot.h"
#include "../../../SDK/Menu/config.h"

float flOldLowerbodyYaw[ 65 ];
float flOldPlaybackrateYaw[ 65 ];

bool Animations::NewDataRecievedFromServer( CBaseEntity* pPlayer )
{
	return pPlayer->GetSimulationTime( ) != pPlayer->GetOldSimulationTime( );
}

void Animations::ResolverLogic( ) {

	if ( !ragebot.aimbotTarget || !g::pLocal || bulletImpact == Vector( 0, 0, 0 ) || !ragebot.targetMatrix || didHurt )
		return;

	Ray_t ray( g::pLocal->GetEyePosition( ), bulletImpact );
	CTraceFilter filter( g::pLocal );

	Trace_t trace;

	i::EngineTrace->TraceRay( ray, MASK_SHOT, &filter, &trace );

	if ( trace.pHitEntity == ragebot.aimbotTarget ) {

		ragebot.aimbotTarget = nullptr;
		bulletImpact = Vector( 0, 0, 0 );
		ragebot.targetMatrix = nullptr;
		//util::Print( "Missed shot due to animation desyncronaztion" );
	}
	else {

		//missedShots[ ragebot.aimbotTarget->EntIndex( ) ]--;
		ragebot.aimbotTarget = nullptr;
		ragebot.targetMatrix = nullptr;
		bulletImpact = Vector( 0, 0, 0 );
	}
}

void Animations::ResolverHandler( IGameEvent* pEvent ) {

	if ( !ragebot.aimbotTarget || !g::pLocal )
		return;

	if ( !strcmp( pEvent->GetName( ), "weapon_fire" ) ) {

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

		if ( iUser == i::EngineClient->GetLocalPlayer( ) ) {

			missedShots[ ragebot.aimbotTarget->EntIndex( ) ]++;
		}
	}
	if ( !strcmp( pEvent->GetName( ), "player_hurt" ) ) {

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );
		auto iAttacker = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "attacker" ) );

		if ( iAttacker == i::EngineClient->GetLocalPlayer( ) && iUser == i::EngineClient->GetPlayerForUserID( ragebot.aimbotTarget->EntIndex( ) ) ) {

			missedShots[ ragebot.aimbotTarget->EntIndex( ) ]--;
			didHurt = true;
		}
	}
	if ( !strcmp( pEvent->GetName( ), "bullet_impact" ) ) {

		auto iUser = i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

		if ( iUser != i::EngineClient->GetLocalPlayer( ) )
			return;

		didHurt = false;
		bulletImpact = Vector( pEvent->GetFloat( "x" ), pEvent->GetFloat( "y" ), pEvent->GetFloat( "z" ) );
	}
}

// removed resolver from Integral, do your own one here ;)
void Animations::SetGoalFeetYaw( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flServerVelocityXY, float flPlaybackrate, int brutePhase )
{
	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer( );
	if ( !pLocal || !pEntity || !pEntity->IsAlive( ) )
		return;

	Lagcompensation::AnimationInfo_t& pData = lagcomp.GetLog( pEntity->EntIndex( ) );
	if ( !&pData )
		return;

	if ( pEntity->GetPlayerInfo( ).bFakePlayer || !cfg::rage::resolver )
		return;

	int pIndex = pEntity->EntIndex( );

	static float randomResolve = ( i::GlobalVars->iTickCount % 16 != 0 ? 0 : M::RandomFloat( -20, 20 ) );
	if ( i::GlobalVars->iTickCount % 16 == 0 )
		randomResolve = M::RandomFloat( -20, 20 );

	switch ( brutePhase % 3 ) {

	case 0: flGuessedYaw = ( pEntity->AnimState( )->GetMaxDesync( ) / 2 ) + randomResolve;
		break;

	case 1: flGuessedYaw = -( pEntity->AnimState( )->GetMaxDesync( ) / 2 ) + randomResolve;
		break;
	}

	//apply the resolver.
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

	auto pLocal = g::pLocal;
	if ( !pLocal )
		return;

	// where the player looks from local eye.
	pRecord->flDeltaAngle = std::fabsf( M::NormalizeYaw( M::CalcAngle( pRecord->vecOrigin, pLocal->GetEyePosition( ) ).y - pRecord->vecEyeAngles.y ) );

	// side detections.
	pRecord->bBackwards = pRecord->flDeltaAngle >= 120.f;
	pRecord->bSideways = pRecord->flDeltaAngle >= 60.f && pRecord->flDeltaAngle < 120.f;
	pRecord->bForwards = pRecord->flDeltaAngle < 60.F;

	// server velocity.
	float flServerVelocityXY = 0.f; //GetVelocityLengthXY(pEntity); //Ouch Fps, might have to actually rebuild this nigger
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
		SetGoalFeetYaw( pEntity, pRecord, nullptr, flServerVelocityXY );

		// save for info.
		iLastGuessedYaw = std::clamp( flGuessedYaw, -flMaxDesyncDelta, flMaxDesyncDelta );

		// fix feet spin.
		pEntity->AnimState( )->flFeetCycle = pRecord->pLayers[ 6 ].flCycle;

		// just get the feet weight.
		pEntity->AnimState( )->flMoveWeight = pRecord->pLayers[ 6 ].flWeight / pEntity->AnimState( )->flInAirSmoothValue;

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
		SetGoalFeetYaw( pEntity, pRecord, pPrevious, flServerVelocityXY );

		// save for info.
		iLastGuessedYaw = std::clamp( flGuessedYaw, -flMaxDesyncDelta, flMaxDesyncDelta );

		// fix feet spin.
		pEntity->AnimState( )->flFeetCycle = pRecord->pLayers[ 6 ].flCycle;

		// just get the feet weight.
		pEntity->AnimState( )->flMoveWeight = pRecord->pLayers[ 6 ].flWeight / pEntity->AnimState( )->flInAirSmoothValue;

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