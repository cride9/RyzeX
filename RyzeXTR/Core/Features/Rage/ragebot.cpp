#include "ragebot.h"
#include "../../globals.h"
#include "autowall.h"
#include "../../SDK/math.h"
#include "../Visuals/ESP.h"
#include "exploits.h"
#include "../Misc/misc.h"
#include "../../SDK/RayTracer rebuilt/CRayTrace.h"
#include "Animations/EnemyAnimations.h"

bool LowestFov( std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pEnt1, std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pEnt2 ) {

	const Vector vecEyePosition = g::pLocal->GetEyePosition( );
	Vector vecCalcAngle;

	auto pEnt1Hitbox = std::get<0>( pEnt1 )->GetHitboxPosition( HITBOX_UPPER_CHEST );
	auto pEnt2Hitbox = std::get<0>( pEnt2 )->GetHitboxPosition( HITBOX_UPPER_CHEST );

	if ( !pEnt1Hitbox.has_value( ) || !pEnt2Hitbox.has_value( ) )
		return true;

	M::VectorAngles( pEnt1Hitbox.value( ) - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween1 = ( ragebot.rageBotData.vecOldViewAngles - vecCalcAngle.NormalizeAngle( ) );

	M::VectorAngles( pEnt2Hitbox.value( ) - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween2 = ( ragebot.rageBotData.vecOldViewAngles - vecCalcAngle.NormalizeAngle( ) );

	float flFirstFov = abs( vecDistanceBetween1.Length2D( ) );
	float flSecondFov = abs( vecDistanceBetween2.Length2D( ) );

	return flFirstFov < flSecondFov;
}

bool LowestHealth( std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pEnt1, std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pEnt2 ) {
	if ( std::get<0>( pEnt1 )->GetHealth( ) != std::get<0>( pEnt2 )->GetHealth( ) )
		return std::get<0>( pEnt1 )->GetHealth( ) < std::get<0>( pEnt2 )->GetHealth( );
	else
		return LowestFov( pEnt1, pEnt2 );
}

bool HighestDamage( std::tuple<Vector, float, int>& damage1, std::tuple<Vector, float, int>& damage2 ) {
	return std::get<1>( damage1 ) > std::get<1>( damage2 );
}

bool IsAutoScopeable( short iItemDefinitionIndex )
{
	switch ( iItemDefinitionIndex )
	{
	case WEAPON_SSG08:
	case WEAPON_AWP:
	case WEAPON_SG556:
	case WEAPON_AUG:
	case WEAPON_SCAR20:
	case WEAPON_G3SG1:
		return true;
	default:
		return false;
	}
}

Vector VelocityExtrapolate( CBaseEntity* player, Vector aimPos )
{
	return aimPos + ( player->GetVelocity( ) * i::GlobalVars->flIntervalPerTick );
}

void CRageBot::CreateMove( CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket ) {

	static CConVar* recoilScale = i::ConVar->FindVar( "weapon_recoil_scale" );
	if ( !pLocal || !cfg::rage::enable )
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );
	if ( !pWeapon || !pWeapon->GetCSWpnData( ) )
		return;

	if ( pWeapon->IsKnife( ) || pWeapon->IsGrenade( ) )
		return;

	Vector vecEyePosition = pLocal->GetEyePosition( );

	if ( std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pTuple = SelectTarget( pLocal, pWeapon, vecEyePosition ); std::get<0>( pTuple ) != nullptr ) {

		static int iTargetedHitbox = 0;
		if ( Vector vecHitscan = Hitscan( pLocal, pTuple, pWeapon, vecEyePosition, iTargetedHitbox ); vecHitscan != Vector( 0, 0, 0 ) ) {

			CBaseEntity* pTarget = std::get<0>( pTuple );

			if ( cfg::rage::betweenshots )
				rageBotData.bCanShoot = false;

			// get better eye position.
			// calculate aim angle.
			Vector shootAngle;
			M::VectorAngles( vecHitscan - vecEyePosition, shootAngle ); // https://www.unknowncheats.me/forum/counterstrike-global-offensive/137492-math-hack-1-coding-aimbot-stop-using-calcangle.html

			if ( CheckShootingCondition( pCmd, pLocal ) ) {

				if ( Hitchance( pTarget, pWeapon, shootAngle, ConfigHitChance( pWeapon ), vecEyePosition, iTargetedHitbox ) ) {

					if ( cfg::antiaim::idealTick && GetAsyncKeyState( cfg::antiaim::idealTickBind ) )
						misc::bRetreat = true;

					rageBotData.bCanShoot = true;
					rageBotData.pAimbotTarget = pTarget;
					rageBotData.pTargetMatrix = std::get<1>( pTuple )->pMatrix;

					pCmd->angViewPoint = ( shootAngle -= ( pLocal->GetAimPunch( ) * recoilScale->GetFloat( ) ) );

					pCmd->iButtons |= IN_ATTACK;
					pCmd->iTickCount = CalculateTickCount( rageBotData.flTargetSimulation );

					if ( !( cfg::antiaim::fakeduck && GetAsyncKeyState( cfg::antiaim::fakeduckbind ) ) && !( cfg::rage::doubletap && GetKeyState( cfg::rage::doubletapkey ) ) || g::bWaiting )
						bSendPacket = true;
				}
				else {
					rageBotData.bCanShoot = false;
					if ( ConfigAutoScope( pWeapon ) && IsAutoScopeable( pWeapon->GetItemDefinitionIndex( ) ) && !pLocal->IsScoped( ) ) //only scope if we have a scoped weapon and we arent scoped
						pCmd->iButtons |= IN_ZOOM;
				}
			}
		}
	}
}

Vector CRageBot::Hitscan( CBaseEntity* pLocal, std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> pTuple, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition, int& iTargetedHitbox ) {

	std::array<bool, HITBOX_MAX> multiPointHitboxes = ConfigMultiHitboxes( pWeapon );
	std::array<bool, HITBOX_MAX> safePointHitboxes = ConfigSafeHitboxes( pWeapon );
	std::tuple<Vector, float, int, float> flBestDamage = std::make_tuple( Vector( 0, 0, 0 ), 0.f, 0, 0.f );

	/* Loop through enemy hitboxes and scale damage, then return a valid position to shoot to */
	for ( const int& hitboxID : ConfigHitboxes( pWeapon ) ) {

		/* Handle override and minimum damage */
		int iMinimumDamage = GetAsyncKeyState( cfg::rage::overrideBind ) ? ConfigOverrideDamage( pWeapon ) : ConfigMinimumDamage( pWeapon );

		/* Easier to handle pointers instead of typing out every time */
		Lagcompensation::LagRecord_t* pCurrentRecord = std::get<1>( pTuple );
		CBaseEntity* pRecordEntity = pCurrentRecord->pEntity;
		matrix3x4_t* pRecordMatrix = pCurrentRecord->pMatrix;

		/* Needed variables for later usage */
		float flRadius = 0.f, flDamage = -1;

		/* Get the targetable entities current hitbox for scanning */
		Vector hitboxPosition = pRecordEntity->GetHitboxPosition( hitboxID, pRecordMatrix, flRadius );

		/* Check if player selected this hitbox for multipoint or not */
		if ( multiPointHitboxes[ hitboxID ] ) {

			/* Generate multipoints */
			std::vector<Vector> vecHitboxPosition;
			vecHitboxPosition = CreatePoints( pRecordEntity, pLocal, pWeapon, hitboxPosition, flRadius, hitboxID, vecEyePosition );

			pRecordEntity->SetBoneCache( pRecordMatrix );
			/* Loop through the multipoint points */
			for ( Vector& currentPoint : vecHitboxPosition ) {

				if ( safePointHitboxes[ hitboxID ] ) {
					if ( SafePoint( vecEyePosition, pWeapon, pCurrentRecord, currentPoint, flDamage ) ) {
						if ( flDamage > iMinimumDamage && flDamage > std::get<1>( flBestDamage ) ) {
							flBestDamage = std::make_tuple( currentPoint, flDamage, hitboxID, pCurrentRecord->flSimulationTime );
							goto safepoint;
						}
					}

					/* If forcing safe point to that hitbox don't scan for not safe points */
					if ( ConfigForceSafe( pWeapon ) )
						continue;
				}
				if ( flDamage = autowall.GetDamage( pLocal, currentPoint ); flDamage >= iMinimumDamage || flDamage > pRecordEntity->GetHealth( ) + 5 ) {
					/* Check if this hitbox has more damage than the other */
					if ( flDamage > iMinimumDamage && flDamage > std::get<1>( flBestDamage ) ) {
						flBestDamage = std::make_tuple( currentPoint, flDamage, hitboxID, pCurrentRecord->flSimulationTime );
					}
				}
			}
		}
		/* Not multipoint selected hitbox so just scan the middle of it */
		else {
			pRecordEntity->SetBoneCache( pRecordMatrix );
			if ( safePointHitboxes[ hitboxID ] ) {
				if ( SafePoint( vecEyePosition, pWeapon, pCurrentRecord, hitboxPosition, flDamage ) ) {
					if ( flDamage > iMinimumDamage && flDamage > std::get<1>( flBestDamage ) ) {
						flBestDamage = std::make_tuple( hitboxPosition, flDamage, hitboxID, pCurrentRecord->flSimulationTime );
						goto safepoint;
					}
				}

				/* If forcing safe point to that hitbox don't scan for not safe points */
				if ( ConfigForceSafe( pWeapon ) )
					continue;
			}
			if ( flDamage = autowall.GetDamage( pLocal, hitboxPosition ); flDamage >= iMinimumDamage || flDamage > pRecordEntity->GetHealth( ) + 5 ) {
				if ( flDamage > iMinimumDamage && flDamage > std::get<1>( flBestDamage ) ) {
					flBestDamage = std::make_tuple( hitboxPosition, flDamage, hitboxID, pCurrentRecord->flSimulationTime );
				}
			}
		}
	}

safepoint:
	/* Return the highest damage hitbox position with the id & simtime for later usage */
	iTargetedHitbox = std::get<2>( flBestDamage );
	rageBotData.flTargetSimulation = std::get<3>( flBestDamage );
	return std::get<0>( flBestDamage );
}

std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*> CRageBot::SelectTarget( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition ) {

	auto m_iPredictiveTicks = []( CBaseCombatWeapon* pWeapon ) -> int {

		auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

		if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
			return cfg::rage::autostopAggressiveness[ 0 ];
		}
		else if ( iDefinitionIndex == WEAPON_SSG08 ) {
			return cfg::rage::autostopAggressiveness[ 1 ];
		}
		else if ( iDefinitionIndex == WEAPON_AWP ) {
			return cfg::rage::autostopAggressiveness[ 2 ];
		}
		else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
			return cfg::rage::autostopAggressiveness[ 4 ];
		}
		else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
			return cfg::rage::autostopAggressiveness[ 3 ];
		}
		else {
			return cfg::rage::autostopAggressiveness[ 5 ];
		}
	};

	static int predictTick = 0;
	switch ( m_iPredictiveTicks( pWeapon ) )
	{
	case 1:
		predictTick = 2; break;
	case 2:
		predictTick = 4; break;
	case 3:
		predictTick = 8; break;
	default:
		predictTick = 1; break;
	}

	FireBulletData_t data = { };
	data.vecPosition = InterpolateLocalEyePosition( pLocal->GetEyePosition( ), predictTick );

	std::vector<std::tuple<CBaseEntity*, Lagcompensation::LagRecord_t*>> vecEntities;
	for ( int i = 0; i < i::GlobalVars->nMaxClients; i++ ) {

		CBaseEntity* pEntity = static_cast< CBaseEntity* >( i::EntityList->GetClientEntity( i ) );

		/* Sanity checks */
		if ( !pEntity || !pEntity->IsAlive( ) || pEntity->IsDormant( ) || pEntity->HasImmunity( ) )
			continue;

		/* Ignore teammates */
		if ( pEntity->GetTeam( ) == pLocal->GetTeam( ) || pEntity == g::pLocal )
			continue;

		/* Get current entity lagcomp data */
		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog( i );

		if ( !pLog->pEntity )
			continue;

		if ( pLog->pRecord.size( ) < 2 )
			continue;

		/* Prepare to scan records */
		std::vector<int> vecSelectedHitboxes = ConfigHitboxes( pWeapon );
		CBaseEntity* pAddedEntity = nullptr;
		for ( int iTick = 0; iTick < pLog->iLastValid; iTick++ ) {

			/* If we already have that entity, break the record searching */
			if ( pLog->pEntity == pAddedEntity )
				break;

			/* Get current record pointer to not call .at(iTick) everytime (?does it even save performance?) */
			Lagcompensation::LagRecord_t* pCurrentRecord = &pLog->pRecord.at( iTick );

			/* Check for a valid record */
			if ( !pCurrentRecord->bValid )
				continue;

			/* Not enough difference, not worth scanning it (probably standing)				first record check */
			if ( ( pLog->pRecord.front( ).vecOrigin - pCurrentRecord->vecOrigin ).Length2D( ) < 25 && iTick >= 1 )
				continue;

			/* Apply current matrix once to save some performance */
			pEntity->SetBoneCache( pCurrentRecord->pMatrix );

			for ( int& iHitbox : vecSelectedHitboxes ) {

				Vector vecHitboxPosition = pLog->pEntity->GetHitboxPosition( iHitbox, pCurrentRecord->pMatrix );
				data.vecDirection = ( vecHitboxPosition - data.vecPosition ).Normalized( );

				/* Check if we can shoot this record and add it if we can, sort later */
				if ( autowall.SimulateFireBullet( pLocal, pWeapon, data ) ) {
					pAddedEntity = pLog->pEntity;
					vecEntities.push_back( std::make_tuple( pLog->pEntity, &pLog->pRecord.at( iTick ) ) );
					break;
				}
			}
		}
	}

	/* Check if we have any record */
	if ( vecEntities.empty( ) )
		return std::make_tuple( nullptr, nullptr );

	/* Sort lowest health/fov entities */
	std::sort( vecEntities.begin( ), vecEntities.end( ), cfg::rage::aimbotTargetSelection ? LowestHealth : LowestFov );

	/* Got an entity, start autostopping if we can */
	AutoStop( pLocal, pWeapon, std::get<0>( vecEntities.front( ) ), g::pCmd );

	/* Return the lowest health/fov */
	return vecEntities.front( );
}

bool CRageBot::Hitchance( CBaseEntity* pEnt, CBaseCombatWeapon* pWeapon, Vector vecFrom, int iChance, Vector vecEyePosition, int iHitbox ) {

	float flFinalHitchance = 0;
	CCSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData( );

	if ( !pWeaponInfo )
		return false;

	if ( exploits::bIsShiftingTicks || ( cfg::rage::doubletap && GetKeyState( cfg::rage::doubletapkey ) && i::GlobalVars->flCurrentTime - pWeapon->GetLastShotTime( ) <= TICKS_TO_TIME( 15 ) ) )
		return true;

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar( "weapon_accuracy_nospread" );
	// server is currently in nospread, no need to calculate anything, just shoot
	if ( weapon_accuracy_nospread->GetInt( ) >= 1 )
		return true;

	Vector vecForward = Vector( 0, 0, 0 );
	Vector vecRight = Vector( 0, 0, 0 );
	Vector vecUp = Vector( 0, 0, 0 );

	M::AngleVectors( vecFrom, &vecForward, &vecRight, &vecUp );

	vecForward.Normalize( );
	vecRight.Normalize( );
	vecUp.Normalize( );

	bool bSpecialWeapon = pWeapon->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_AWP || pWeapon->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_SSG08;
	int iAccuracry = bSpecialWeapon ? 256 : 128;

	static bool bSetupSpreadValues = true;
	static float flSpreadValues[ 256 ][ 6 ];

	if ( bSetupSpreadValues )
	{
		bSetupSpreadValues = false;

		for ( auto i = 0; i < iAccuracry; ++i )
		{
			M::RandomSeed( i + 1 );

			float a = M::RandomFloat( 0.0f, 1.0f );
			float b = M::RandomFloat( 0.0f, 6.283185307f );
			float c = M::RandomFloat( 0.0f, 1.0f );
			float d = M::RandomFloat( 0.0f, 6.283185307f );

			flSpreadValues[ i ][ 0 ] = a;
			flSpreadValues[ i ][ 1 ] = c;

			auto flSinB = 0.0f, flCosB = 0.0f;
			M::SinCos( b, &flSinB, &flCosB );

			auto flSinD = 0.0f, flCosD = 0.0f;
			M::SinCos( b, &flSinD, &flCosD );

			flSpreadValues[ i ][ 2 ] = flSinB;
			flSpreadValues[ i ][ 3 ] = flCosB;
			flSpreadValues[ i ][ 4 ] = flSinD;
			flSpreadValues[ i ][ 5 ] = flCosD;
		}
	}

	int iHits = 0;

	float flGetInaccuracy = pWeapon->GetInaccuracy( );
	float flGetSpread = pWeapon->GetSpread( );

	for ( auto i = 0; i < iAccuracry; ++i )
	{
		float flInacc = flSpreadValues[ i ][ 0 ] * flGetInaccuracy;
		float flSpread = flSpreadValues[ i ][ 1 ] * flGetSpread;

		float flSpreadX = flSpreadValues[ i ][ 3 ] * flInacc + flSpreadValues[ i ][ 5 ] * flSpread;
		float flSpreadY = flSpreadValues[ i ][ 2 ] * flInacc + flSpreadValues[ i ][ 4 ] * flSpread;

		Vector vecDirection = Vector( 0, 0, 0 );
		vecDirection.x = vecForward.x + vecRight.x * flSpreadX + vecUp.x * flSpreadY;
		vecDirection.y = vecForward.y + vecRight.y * flSpreadX + vecUp.y * flSpreadY;
		vecDirection.z = vecForward.z + vecRight.z * flSpreadX + vecUp.z * flSpreadY; //-V778

		Vector vecEnd = vecEyePosition + vecDirection * pWeaponInfo->flRange;

		Trace_t Trace;
		i::EngineTrace->ClipRayToEntity( Ray_t( vecEyePosition, vecEnd ), MASK_SHOT | CONTENTS_GRATE, pEnt, &Trace );

		if ( Trace.pHitEntity == pEnt )
			iHits++;
	}

	flFinalHitchance = static_cast< int >( ( float( iHits ) / ( iAccuracry / 100.f ) ) );

	if ( flFinalHitchance >= iChance )
		return true;

	return false;
}

void CRageBot::AutoStop( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CBaseEntity* pTarget, CUserCmd* pCmd ) {

	// Credit to @Monthyx
	// Fast stop source from obelus

	auto m_bStopinAir = []( CBaseCombatWeapon* pWeapon ) -> bool {
		
		auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

		if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
			return cfg::rage::m_bAutoStopInAir[ 0 ];
		}
		else if ( iDefinitionIndex == WEAPON_SSG08 ) {
			return cfg::rage::m_bAutoStopInAir[ 1 ];
		}
		else if ( iDefinitionIndex == WEAPON_AWP ) {
			return cfg::rage::m_bAutoStopInAir[ 2 ];
		}
		else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
			return cfg::rage::m_bAutoStopInAir[ 4 ];
		}
		else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
			return cfg::rage::m_bAutoStopInAir[ 3 ];
		}
		else {
			return cfg::rage::m_bAutoStopInAir[ 5 ];
		}
	};

	auto m_bBetweenShots = []( CBaseCombatWeapon* pWeapon ) -> bool {
		
		auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

		if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
			return cfg::rage::betweenshots[ 0 ];
		}
		else if ( iDefinitionIndex == WEAPON_SSG08 ) {
			return cfg::rage::betweenshots[ 1 ];
		}
		else if ( iDefinitionIndex == WEAPON_AWP ) {
			return cfg::rage::betweenshots[ 2 ];
		}
		else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
			return cfg::rage::betweenshots[ 4 ];
		}
		else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
			return cfg::rage::betweenshots[ 3 ];
		}
		else {
			return cfg::rage::betweenshots[ 5 ];
		}
	};

	if ( !ConfigAutoStop( pWeapon ) )
		return;

	if ( !( g::pLocal->GetFlags( ) & FL_ONGROUND ) && !m_bStopinAir( pWeapon ) )
		return;

	// server is currently in nospread, no need to autostop
	if ( i::ConVar->FindVar( "weapon_accuracy_nospread" )->GetInt( ) >= 1 )
		return;

	if ( !m_bBetweenShots(pWeapon) && !CheckShootingCondition( pCmd, pLocal ) )
			return;

	if ( rageBotData.bCanShoot )
		return;

	float flIdealSpeed = ( .33f * 0.85f ) * ( g::pLocal->IsScoped( ) ? pWeapon->GetCSWpnData( )->flMaxSpeed[ 1 ] : pWeapon->GetCSWpnData( )->flMaxSpeed[ 0 ] );

	//int predictTick = 0;
	//switch (m_iPredictiveTicks( m_iPredictiveTicks ))
	//{
	//case 1:
	//	predictTick = 2; break;
	//case 2:
	//	predictTick = 4; break;
	//case 3:
	//	predictTick = 8; break;
	//default :
	//	predictTick = 1; break;
	//}

	//Vector vecInterpolatedEyePosition = InterpolateLocalEyePosition(g::pLocal->GetEyePosition(), predictTick);

	//FireBulletData_t data = { };
	//data.vecPosition = vecInterpolatedEyePosition;
	//auto vecHitboxPosition = pTarget->GetHitboxPosition(HITBOX_UPPER_CHEST);
	//if (!vecHitboxPosition.has_value())
	//	return;

	//data.vecDirection = (vecHitboxPosition.value() - vecInterpolatedEyePosition).Normalized();

	//if (!autowall.SimulateFireBullet(g::pLocal, g::pLocal->GetWeapon(), data))
	//	return;

	pCmd->iButtons &= ~IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;

	// Get the ideal speed for shooting (playstyle)
	Vector velocity = g::pLocal->GetVelocity( );
	Vector direction;
	Vector real_view;

	if ( flIdealSpeed > velocity.Length2D( ) )
		return;

	M::VectorAngles( velocity, direction );
	i::EngineClient->GetViewAngles( real_view );

	direction.y = real_view.y - direction.y;

	Vector forward;
	M::AngleVectors( direction, &forward );

	static auto cl_forwardspeed = i::ConVar->FindVar( "cl_forwardspeed" );
	static auto cl_sidespeed = i::ConVar->FindVar( "cl_sidespeed" );

	auto negative_forward_speed = -cl_forwardspeed->GetFloat( );
	auto negative_side_speed = -cl_sidespeed->GetFloat( );

	auto negative_forward_direction = forward * negative_forward_speed;
	auto negative_side_direction = forward * negative_side_speed;

	pCmd->flForwardMove = negative_forward_direction.x;
	pCmd->flSideMove = negative_side_direction.y;
}

std::pair<int, int> CRageBot::ConfigMultipoint( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return std::make_pair( cfg::rage::autoHeadPoints, cfg::rage::autoBodyPoints );
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return std::make_pair( cfg::rage::scoutHeadPoints, cfg::rage::scoutBodyPoints );
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return std::make_pair( cfg::rage::awpHeadPoints, cfg::rage::awpBodyPoints );
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return std::make_pair( cfg::rage::heavypistolHeadPoints, cfg::rage::heavypistolBodyPoints );
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return std::make_pair( cfg::rage::pistolHeadPoints, cfg::rage::pistolBodyPoints );
	}
	else {
		return std::make_pair( cfg::rage::etcHeadPoints, cfg::rage::etcBodyPoints );
	}
}

int CRageBot::ConfigMinimumDamage( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::autoMindmg;
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::scoutMindmg;
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::awpMindmg;
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::heavypistolMindmg;
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::pistolMindmg;
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 100;
	}
	else {
		return cfg::rage::etcMindmg;
	}
}

int CRageBot::ConfigOverrideDamage( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::autoOverride;
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::scoutOverride;
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::awpOverride;
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::heavypistolOverride;
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::pistolOverride;
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 100;
	}
	else {
		return cfg::rage::etcOverride;
	}
}

int CRageBot::ConfigHitChance( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::autoHitchance;
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::scoutHitchance;
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::awpHitchance;
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::heavypistolHitchance;
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::pistolHitchance;
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 72;
	}
	else {
		return cfg::rage::etcHitchance;
	}
}

bool CRageBot::ConfigForceSafe( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::forceSafePoint[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::forceSafePoint[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::forceSafePoint[ 2 ];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::forceSafePoint[ 4 ];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::forceSafePoint[ 3 ];
	}
	else {
		return cfg::rage::forceSafePoint[ 5 ];
	}
}

bool CRageBot::ConfigAutoScope( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::autoscope[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::autoscope[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::autoscope[ 2 ];
	}
	else {
		return false;
	}
}

bool CRageBot::ConfigAutoStop( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::autostop[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::autostop[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::autostop[ 2 ];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::autostop[ 4 ];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::autostop[ 3 ];
	}
	else {
		return cfg::rage::autostop[ 5 ];
	}
}

std::vector<int> CRageBot::ConfigHitboxes( CBaseCombatWeapon * pWeapon ) {

	// "head", "upper chest", "lower chest", "stomach", "arms", "legs"

	std::vector<int> vecHitboxes;
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	static auto AddHitbox = []( int index, std::vector<int>& vecHitboxList ) {

		// server only allows headshots, so let's only push_back head and return!
		if ( i::ConVar->FindVar( "mp_damage_headshot_only" )->GetBool( ) )
		{
			vecHitboxList.push_back( HITBOX_HEAD );
			return;
		}

		if ( index == 0 ) {
			vecHitboxList.push_back( HITBOX_HEAD );
		}
		if ( index == 1 ) {
			vecHitboxList.push_back( HITBOX_UPPER_CHEST );
		}
		if ( index == 2 ) {
			vecHitboxList.push_back( HITBOX_CHEST );
			vecHitboxList.push_back( HITBOX_THORAX );
		}
		if ( index == 3 ) {
			vecHitboxList.push_back( HITBOX_STOMACH );
			vecHitboxList.push_back( HITBOX_PELVIS );
		}
		if ( index == 4 ) {
			vecHitboxList.push_back( HITBOX_RIGHT_FOREARM );
			vecHitboxList.push_back( HITBOX_LEFT_FOREARM );

			vecHitboxList.push_back( HITBOX_LEFT_UPPER_ARM );
			vecHitboxList.push_back( HITBOX_RIGHT_UPPER_ARM );

			vecHitboxList.push_back( HITBOX_RIGHT_HAND );
			vecHitboxList.push_back( HITBOX_LEFT_HAND );
		}
		if ( index == 5 ) {
			vecHitboxList.push_back( HITBOX_RIGHT_THIGH );
			vecHitboxList.push_back( HITBOX_LEFT_THIGH );

			vecHitboxList.push_back( HITBOX_RIGHT_CALF );
			vecHitboxList.push_back( HITBOX_LEFT_CALF );

			vecHitboxList.push_back( HITBOX_RIGHT_FOOT );
			vecHitboxList.push_back( HITBOX_LEFT_FOOT );
		}
	};

	if ( cfg::rage::forceBaim && GetAsyncKeyState( cfg::rage::forceBaimKey ) ) {
		AddHitbox( 2, vecHitboxes );
		AddHitbox( 3, vecHitboxes );
		return vecHitboxes;
	}

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::autoHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::scoutHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::awpHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::heavypistolHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::pistolHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, vecHitboxes );
		AddHitbox( 3, vecHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::etcHitboxes[ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}

	return vecHitboxes;
}

std::array<bool, HITBOX_MAX> CRageBot::ConfigMultiHitboxes( CBaseCombatWeapon * pWeapon ) {

	std::array<bool, HITBOX_MAX> arrHitboxes = {};
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	static auto AddHitbox = []( int index, std::array<bool, HITBOX_MAX>& vecHitboxList ) {

		if ( index == 0 ) {
			vecHitboxList[ HITBOX_HEAD ] = true;
		}
		if ( index == 1 ) {
			vecHitboxList[ HITBOX_UPPER_CHEST ] = true;
		}
		if ( index == 2 ) {
			vecHitboxList[ HITBOX_CHEST ] = true;
			vecHitboxList[ HITBOX_THORAX ] = true;
		}
		if ( index == 3 ) {
			vecHitboxList[ HITBOX_STOMACH ] = true;
			vecHitboxList[ HITBOX_PELVIS ] = true;
		}
		if ( index == 4 ) {
			vecHitboxList[ HITBOX_RIGHT_FOREARM ] = true;
			vecHitboxList[ HITBOX_LEFT_FOREARM ] = true;

			vecHitboxList[ HITBOX_LEFT_UPPER_ARM ] = true;
			vecHitboxList[ HITBOX_RIGHT_UPPER_ARM ] = true;

			vecHitboxList[ HITBOX_RIGHT_HAND ] = true;
			vecHitboxList[ HITBOX_LEFT_HAND ] = true;
		}
		if ( index == 5 ) {
			vecHitboxList[ HITBOX_RIGHT_THIGH ] = true;
			vecHitboxList[ HITBOX_LEFT_THIGH ] = true;

			vecHitboxList[ HITBOX_RIGHT_CALF ] = true;
			vecHitboxList[ HITBOX_LEFT_CALF ] = true;

			vecHitboxList[ HITBOX_RIGHT_FOOT ] = true;
			vecHitboxList[ HITBOX_LEFT_FOOT ] = true;
		}
	};

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::autoMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::scoutMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::awpMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::heavypistolMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::pistolMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, arrHitboxes );
		AddHitbox( 3, arrHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::etcMultiHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}

	return arrHitboxes;
}

std::array<bool, HITBOX_MAX> CRageBot::ConfigSafeHitboxes( CBaseCombatWeapon * pWeapon ) {

	std::array<bool, HITBOX_MAX> arrHitboxes = {};
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	static auto AddHitbox = []( int index, std::array<bool, HITBOX_MAX>& vecHitboxList ) {

		if ( index == 0 ) {
			vecHitboxList[ HITBOX_HEAD ] = true;
		}
		if ( index == 1 ) {
			vecHitboxList[ HITBOX_UPPER_CHEST ] = true;
		}
		if ( index == 2 ) {
			vecHitboxList[ HITBOX_CHEST ] = true;
			vecHitboxList[ HITBOX_THORAX ] = true;
		}
		if ( index == 3 ) {
			vecHitboxList[ HITBOX_STOMACH ] = true;
			vecHitboxList[ HITBOX_PELVIS ] = true;
		}
		if ( index == 4 ) {
			vecHitboxList[ HITBOX_RIGHT_FOREARM ] = true;
			vecHitboxList[ HITBOX_LEFT_FOREARM ] = true;

			vecHitboxList[ HITBOX_LEFT_UPPER_ARM ] = true;
			vecHitboxList[ HITBOX_RIGHT_UPPER_ARM ] = true;

			vecHitboxList[ HITBOX_RIGHT_HAND ] = true;
			vecHitboxList[ HITBOX_LEFT_HAND ] = true;
		}
		if ( index == 5 ) {
			vecHitboxList[ HITBOX_RIGHT_THIGH ] = true;
			vecHitboxList[ HITBOX_LEFT_THIGH ] = true;

			vecHitboxList[ HITBOX_RIGHT_CALF ] = true;
			vecHitboxList[ HITBOX_LEFT_CALF ] = true;

			vecHitboxList[ HITBOX_RIGHT_FOOT ] = true;
			vecHitboxList[ HITBOX_LEFT_FOOT ] = true;
		}
	};

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::autoSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::scoutSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::awpSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::heavypistolSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::pistolSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, arrHitboxes );
		AddHitbox( 3, arrHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::etcSafeHitboxes[ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}

	return arrHitboxes;
}

bool CRageBot::CheckShootingCondition( CUserCmd * pCmd, CBaseEntity * pLocal ) {

	if ( !pLocal || !pLocal->GetWeapon( ) )
		return false;

	float flServerTime = TICKS_TO_TIME( pLocal->GetTickBase( ) );
	const CCSWeaponInfo* pWeaponData = pLocal->GetWeapon( )->GetCSWpnData( );

	if ( !pWeaponData )
		return false;

	if ( pLocal->GetWeapon( )->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_C4 )
		return false;

	if ( pLocal->GetWeapon( )->GetAmmo( ) < 1 )
		return false;

	if ( pLocal->GetWeapon( )->GetNextPrimaryAttack( ) > flServerTime || g::pLocal->GetWeapon( )->GetNextSecondaryAttack( ) > flServerTime )
		return false;

	// can't shoot yet, wait for next shootable tick
	if ( cfg::antiaim::fakeduck && GetAsyncKeyState( cfg::antiaim::fakeduckbind ) && g::pLocal->GetDuckAmount( ) != 0.f )
		return false;

	return true;
}

std::vector<Vector> CRageBot::CreatePoints( CBaseEntity * pTarget, CBaseEntity * pLocal, CBaseCombatWeapon * pWeapon, Vector vecAngle, float flRadius, int nHitbox, Vector vecEyePosition, bool bGenerateNew ) {

	if ( flRadius <= 0 )
		return std::vector<Vector>{vecAngle};

	std::pair<int, int> multiPoints = ConfigMultipoint( g::pLocal->GetWeapon( ) );
	std::vector<Vector> points;

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;

	float flHitboxDistance = flRadius * ( ( nHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints ) / 150.f );
	points.push_back( vecAngle );

	int iOptimization[ 65 ][ HITBOX_MAX ];
	switch ( iOptimization[ pTarget->EntIndex( ) ][ nHitbox ] % 5 ) {
	case 0:
		points.push_back( vecAngle + Vector( flHitboxDistance, 0.f, 0.f ) );
		break;

	case 1:
		points.push_back( vecAngle + Vector( 0.f, flHitboxDistance, 0.f ) );
		break;

	case 2:
		points.push_back( vecAngle + Vector( 0.f, 0.f, flHitboxDistance ) );
		break;

	case 3:
		points.push_back( vecAngle + Vector( 0.f, -flHitboxDistance, 0.f ) );
		break;

	case 4:
		points.push_back( vecAngle + Vector( -flHitboxDistance, 0.f, 0.f ) );
		break;
	}
	iOptimization[ pTarget->EntIndex( ) ][ nHitbox ]++;

	return points;
}

bool CRageBot::SafePoint( Vector & vecEyePosition, CBaseCombatWeapon * pWeapon, Lagcompensation::LagRecord_t * pRecord, Vector & vecShootposition, float& flMinDamage ) {

	bool hitLeft = false, hitRight = false, hitCenter = false;
	float dmgLeft = 0.f, dmgRight = 0.f, dmgCenter = 0.f;

	pRecord->pEntity->SetBoneCache( pRecord->pRightMatrix );
	dmgRight = autowall.GetDamage( g::pLocal, vecShootposition );

	pRecord->pEntity->SetBoneCache( pRecord->pLeftMatrix );
	dmgLeft = autowall.GetDamage( g::pLocal, vecShootposition );

	pRecord->pEntity->SetBoneCache( pRecord->pCenterMatrix );
	dmgCenter = autowall.GetDamage( g::pLocal, vecShootposition );

	// if we hit every single angle that means that is a safepoint to shoot
	if ( dmgLeft > 0 && dmgRight > 0 && dmgCenter > 0 ) {

		// manipulate the shooting position
		vecShootposition = vecShootposition;
		flMinDamage = ( dmgLeft + dmgRight + dmgCenter ) / 3;
		return true;
	}
	return false;
}

Vector CRageBot::InterpolateLocalEyePosition( Vector vecEyePosition, int iInterpolateTick ) {

	// linear extrapolation
	// calculating the next ticks position and calculating autostop with it
	// and also picking targets with it

	// calculating how much distance we make each tick
	Vector vecPredictedEyePosition = vecEyePosition + ( g::pLocal->GetVelocity( ) * ( i::GlobalVars->flIntervalPerTick * iInterpolateTick ) );

	return vecPredictedEyePosition;
}

int CRageBot::CalculateTickCount( float flSimulationTime ) {

	return lagcomp.FixTickCount( flSimulationTime );

	// calculate lerp remainder.
	float flLerpRemainder = std::fmodf( lagcomp.GetClientInterpAmount( ), i::GlobalVars->flIntervalPerTick );

	// get real simulation time and calculate interp fraction.
	if ( flLerpRemainder > 0.f )
		flSimulationTime += i::GlobalVars->flIntervalPerTick - flLerpRemainder;

	return flSimulationTime;
}