#include "ragebot.h"
#include "../../globals.h"
#include "autowall.h"
#include "../../SDK/math.h"
#include "../Visuals/ESP.h"
#include "exploits.h"
#include "../Misc/misc.h"
#include "../../SDK/RayTracer rebuilt/CRayTrace.h"
#include "Animations/EnemyAnimations.h"
#include "../Misc/Playerlist.h"

#include "../../SDK/InputSystem.h"
#include "../Networking/networking.h"

bool HitscanComparator(const Hitscan_t& a, const Hitscan_t& b) {

	// Sort by bSafe in descending order
	if (a.bSafe && !b.bSafe) {

		if (a.iHitbox == HITBOX_HEAD && !a.bHead)
			return false;
		else if (a.iHitbox == HITBOX_HEAD && a.bHead)
			return true;

		return true;
	}
	else if (!a.bSafe && b.bSafe) {

		if (b.iHitbox == HITBOX_HEAD && !b.bHead)
			return true;
		else if (b.iHitbox == HITBOX_HEAD && b.bHead)
			return false;

		return false;
	}

	// prioritize high ground


	/*if (a.pRecord->bResolved && a.bHead)
		return true;
	else if (b.pRecord->bResolved && b.bHead)
		return false;*/
	
	if (a.bHead && !b.bHead)
		return true;
	else if (!a.bHead && b.bHead)
		return false;

	// Sort by hitbox, prioritize body over head if both are lethal
	if (a.bLethal && b.bLethal) {

		if (a.iHitbox == HITBOX_HEAD && b.iHitbox != HITBOX_HEAD)
			return false;
		else if (a.iHitbox != HITBOX_HEAD && b.iHitbox == HITBOX_HEAD)
			return true;
	}

	// Sort by flDamage in descending order
	return a.flDamage > b.flDamage;

	// If all else is equal, maintain the original order
	return false;
}

bool LowestFov(CBaseEntity* pEnt1, CBaseEntity* pEnt2) {

	if (pEnt1 == nullptr)
		return false; // pEnt1 is nullptr, move it to the back
	if (pEnt2 == nullptr)
		return true; // pEnt2 is nullptr, move it to the back

	auto firstPriority = playerList::arrPlayers[pEnt1->EntIndex()].iPriority;
	auto secondPriority = playerList::arrPlayers[pEnt2->EntIndex()].iPriority;

	if (firstPriority == 1 && secondPriority != 1)
		return true; // pEnt1 has priority 1, move it to the front
	if (firstPriority != 1 && secondPriority == 1)
		return false; // pEnt2 has priority 1, move it to the front

	const Vector vecEyePosition = g::pLocal->GetEyePosition(false );
	Vector vecCalcAngle;

	auto pEnt1Hitbox = pEnt1->GetHitboxPosition( HITBOX_UPPER_CHEST, lagcomp.GetLog(pEnt1->EntIndex()).pCachedMatrix.data());
	auto pEnt2Hitbox = pEnt2->GetHitboxPosition( HITBOX_UPPER_CHEST, lagcomp.GetLog(pEnt2->EntIndex()).pCachedMatrix.data());

	M::VectorAngles( pEnt1Hitbox - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween1 = ( g::vecOriginalViewAngle - vecCalcAngle.NormalizeAngle( ) );

	M::VectorAngles( pEnt2Hitbox - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween2 = ( g::vecOriginalViewAngle - vecCalcAngle.NormalizeAngle( ) );

	float flFirstFov = abs( (vecDistanceBetween1).NormalizeAngle().Length2D( ) );
	float flSecondFov = abs( (vecDistanceBetween2).NormalizeAngle().Length2D( ) );

	return flFirstFov < flSecondFov;
}

bool LowestHealth(CBaseEntity* pEnt1, CBaseEntity* pEnt2) {
	if (pEnt1->GetHealth() != pEnt2->GetHealth())
		return pEnt1->GetHealth() < pEnt2->GetHealth();
	else
		return false;
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
	if ( !pLocal || !cfg::rage::enable || (!IPT::HandleInput(cfg::rage::ragebotbind) && cfg::rage::ragebotbind) || g::bUpdatingSkins) {
		exploits::bCanCharge = true;
		rageBotData.pAimbotTarget = nullptr;
		return;
	}

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );
	if (!pWeapon || !pWeapon->GetCSWpnData()) {
		exploits::bCanCharge = true;
		rageBotData.pAimbotTarget = nullptr;
		return;
	}

	if (pWeapon->IsKnife() || pWeapon->IsGrenade()) {
		exploits::bCanCharge = true;
		rageBotData.pAimbotTarget = nullptr;
		return;
	}

	misc::RevolverCreateMove();
	Vector vecEyePosition = pLocal->GetEyePosition(false);

	misc::bPeeking = false;
	if (Vector vecHitscan = Hitscan(pLocal, pWeapon, vecEyePosition); vecHitscan != Vector(0, 0, 0)) {

		misc::bPeeking = false;
		exploits::bCanCharge = false;
		if (cfg::rage::betweenshots)
			rageBotData.bCanShoot = false;

		AutoStop(pLocal, pWeapon, rageBotData.pAimbotTarget, g::pCmd, vecHitscan);
		// get better eye position.
		// calculate aim angle.
		Vector shootAngle;
		M::VectorAngles(vecHitscan - vecEyePosition, shootAngle); // https://www.unknowncheats.me/forum/counterstrike-global-offensive/137492-math-hack-1-coding-aimbot-stop-using-calcangle.html

		if (/*CheckShootingCondition(pCmd, pLocal, pWeapon)*/pLocal->CanShoot((CWeaponCSBase*)pWeapon)) {

			if (Hitchance(rageBotData.pAimbotTarget, pWeapon, shootAngle, ConfigHitChance(pWeapon), vecEyePosition)) {

				rageBotData.bCanShoot = true;
				Vector vecAngle = (shootAngle -= (pLocal->GetAimPunch() * recoilScale->GetFloat()));
				pCmd->angViewPoint = vecAngle;
				if (!cfg::rage::bSilentAim)
					i::EngineClient->SetViewAngles(vecAngle);

				pCmd->iButtons |= IN_ATTACK;

				bSetTickCount = true;
				rageBotData.iTickcount = pCmd->iTickCount;
				pCmd->iTickCount = CalculateTickCount(rageBotData.flTargetSimulation);
				bSendPacketThisTick = ShouldSendPacket(bSendPacket);
			}
			else {
				rageBotData.bCanShoot = false;
				if (ConfigAutoScope(pWeapon) && IsAutoScopeable(pWeapon->GetItemDefinitionIndex()) && !pLocal->IsScoped()) //only scope if we have a scoped weapon and we arent scoped
					if (!pLocal->IsResumingScope())
						pCmd->iButtons |= IN_ZOOM;
			}
		}
	}
	else {

		rageBotData.pAimbotTarget = nullptr;
		exploits::bCanCharge = true;
	}
}

void CRageBot::SelectTargets( CBaseEntity* pLocal )
{
	vecTargets.clear( );

	for ( int i = 1; i < i::GlobalVars->nMaxClients; i++ )
	{
		CBaseEntity* pEntity = static_cast< CBaseEntity* >( i::EntityList->GetClientEntity( i ) );

		if ( !pEntity || !pEntity->IsAlive( ) || pEntity->IsDormant( ) || pEntity->GetTeam( ) == pLocal->GetTeam( ) || pEntity->HasImmunity( ) )
			continue;

		//if (cfg::antiaim::idealTick && IPT::HandleInput(cfg::antiaim::idealTickBind)) 
		/*{

			Vector vecInterpolatedEyePosition = InterpolateLocalEyePosition(pLocal->GetEyePosition(false), 5);

			FireBulletData_t data = { };
			data.vecPosition = vecInterpolatedEyePosition;
			data.vecDirection = (pEntity->GetHitboxPosition(HITBOX_HEAD, lagcomp.GetLog(i).pCachedMatrix.data()) - vecInterpolatedEyePosition).Normalized();

			if (autowall.SimulateFireBullet(pLocal, pLocal->GetWeapon(), data))
				misc::bPeeking = true;
		}*/
		vecTargets.emplace_back( pEntity );
	}
	
	if ( !vecTargets.empty( ) )
		std::sort( vecTargets.begin( ), vecTargets.end( ), LowestHealth );
}

std::array<Lagcompensation::LagRecord_t*, 2> ChooseTargetRecord(Lagcompensation::AnimationInfo_t* pLog) {

	Lagcompensation::LagRecord_t* pOnShotRecord = nullptr;
	Lagcompensation::LagRecord_t* pSafeRecord = nullptr;
	Lagcompensation::LagRecord_t* pSafeBackTrackRecord = nullptr;

	int iSafeRecord = 0;
	int iHeadShottableRecordIndex = 128;
	for (int i = 0; i < pLog->iLastValid; i++) {

		// Break from the loop if we got 2 entites
		if (pOnShotRecord && pSafeRecord)
			break;

		// get a reference from that record
		Lagcompensation::LagRecord_t& refRecord = pLog->pRecord.at(i);

		// haha lol
		if (iHeadShottableRecordIndex > i) {
			if ((refRecord.bBreakingLagcompensation && i == 0) || !refRecord.bBreakingLagcompensation) {

				Trace_t traceData;
				Vector vecHitboxPosition = pLog->pEntity->GetHitboxPosition(HITBOX_HEAD, refRecord.pMatricies[RESOLVE]);
				i::EngineTrace->ClipRayToEntity(Ray_t(g::vecEyePosition, vecHitboxPosition), MASK_SHOT, pLog->pEntity, &traceData);
				if (traceData.iHitGroup == HITGROUP_HEAD) {
					iHeadShottableRecordIndex = i;
				}
			}
		}

		// check for onshot because that's what we call the safest headshot option
		if (refRecord.bDidShot && !pOnShotRecord) {

			// lagcomp breaking entities cannot be bcaktracked
			if (refRecord.bBreakingLagcompensation && i == 0) {
				pOnShotRecord = &refRecord;
				continue;
			}

			// not lagcomp breaking entities can be easily backtracked and even force it
			else if (!refRecord.bBreakingLagcompensation) {
				pOnShotRecord = &refRecord;
				continue;
			}
		}

		// first let's get the safest resolves
		if (!refRecord.bResolved)
			continue;

		// non backtrackable entity -> bcs breaking lagcomp -> server is not saving this guy
		// TODO: bypass cl_lagcompensation check -> disallow the client to be lagcompensated
		if (refRecord.bBreakingLagcompensation && i == 0 && !pSafeRecord) {
			pSafeRecord = &refRecord;
			continue;
		}
		// not breaking the lagcompensation -> can be backtracked
		else if (!refRecord.bBreakingLagcompensation && !pSafeRecord) {
			pSafeRecord = &refRecord;
			iSafeRecord = i;
			continue;
		}
	}

	// if we got the 2 safe stuff let's just return them
	if (pOnShotRecord && pSafeRecord)
		return std::array<Lagcompensation::LagRecord_t*, 2>{pOnShotRecord, pSafeRecord};

	// if we 
	if (pOnShotRecord || pSafeRecord) {

		// loop from the back to the record that we added lately (to not scan the same target)
		for (int i = pLog->iLastValid; i > iSafeRecord; i--) {

			// break out from the loop if we got a backtrackable record
			if (pSafeBackTrackRecord)
				break;

			try {

				// get a reference from that record
				Lagcompensation::LagRecord_t& refRecord = pLog->pRecord.at(i);

				// same stuff, lagcomp break -> cant be backtracked
				if (refRecord.bBreakingLagcompensation)
					continue;

				if (refRecord.bResolved)
					pSafeBackTrackRecord = &refRecord;
			}
			catch (std::out_of_range) {


			}
		}
	}

	// if we got the 2 record let's return them
	if (pSafeBackTrackRecord && pSafeRecord)
		return std::array<Lagcompensation::LagRecord_t*, 2>{pSafeRecord, pSafeBackTrackRecord};

	// if we reach this part, we didn't have any record... let's return the recent record and return nothing else
	return std::array<Lagcompensation::LagRecord_t*, 2>{ &pLog->pRecord.at(iHeadShottableRecordIndex == 128 ? 0 : iHeadShottableRecordIndex), nullptr};
}

Vector CRageBot::Hitscan( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition) {

	if (g::bUpdatingSkins)
		return Vector(0, 0, 0);

	SelectTargets(pLocal);
	if (vecTargets.empty())
		return Vector(0, 0, 0);

	static std::vector<Vector> multiPointed = { Vector(0, 0, 0) };
	std::array<bool, HITBOX_MAX> vecSelectedHitboxes = ConfigHitboxes(pWeapon);
	std::array<bool, HITBOX_MAX> vecSelectedSafePoints = ConfigSafeHitboxes(pWeapon);
	int iMinimumDamage = IPT::HandleInput(cfg::rage::overrideBind) ? ConfigOverrideDamage(pWeapon) : ConfigMinimumDamage(pWeapon);
	bool bForceSafe = ConfigForceSafe(pWeapon);
	float flDamage = 0.f;

	std::vector<Hitscan_t> vecRecordSave{};

	for (CBaseEntity* pEntity : vecTargets) {

		if (!pEntity)
			continue;

		if (playerList::arrPlayers[pEntity->EntIndex()].iPriority == FRIEND)
			continue;

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pEntity->EntIndex());

		if (!pLog || pLog->pRecord.empty() || pLog->pEntity != pEntity || pLog->iLastValid >= pLog->pRecord.size() || pLog->iFirstValid >= pLog->pRecord.size()) 
			continue;
		
		std::array<Lagcompensation::LagRecord_t*, 2> arrRecords = ChooseTargetRecord(pLog);

		bool bBacktrack = false;
		size_t iTick = 0;

		for (auto pCurrentApplied : arrRecords) {

			if (!pCurrentApplied)
				continue;

			pCurrentApplied->ApplyMatrix(pEntity, RESOLVE);
			for (size_t iHitbox = 0; iHitbox < HITBOX_MAX; iHitbox++) {

				multiPointed.clear();
				if (!vecSelectedHitboxes[iHitbox])
					continue;

				multiPointed = CreatePoints(vecEyePosition, pWeapon, pCurrentApplied, iHitbox);

				if (cfg::rage::iAimbotFov < 180) {
					Vector output;
					M::VectorAngles(multiPointed.front() - vecEyePosition, output);
					Vector vecDistanceBetween = (g::vecOriginalViewAngle - output.NormalizeAngle());

					if (abs((vecDistanceBetween).NormalizeAngle().Length2D()) > cfg::rage::iAimbotFov)
						continue;
				}

				for (Vector& vecPoint : multiPointed) {

					int iSafePoint = SafePoint(vecEyePosition, pWeapon, pCurrentApplied, vecPoint, iHitbox);
					if (vecSelectedSafePoints[iHitbox]) {

						if (bForceSafe && iSafePoint < 3)
							continue;

						if (iSafePoint < 2)
							continue;
					}

					FireBulletData_t data;
					if (flDamage = autowall.GetDamage(pLocal, vecEyePosition, vecPoint, pWeapon, &data); flDamage > 0 /*|| (flDamage >= pEntity->GetHealth() + 5 && iHitbox != HITBOX_HEAD)*/) {

						if (playerList::arrPlayers[data.enterTrace.pHitEntity->EntIndex()].iPriority == FRIEND)
							continue;

						vecRecordSave.emplace_back(Hitscan_t(pCurrentApplied, vecPoint, flDamage, iHitbox, data.enterTrace.iHitGroup, iSafePoint == 3, flDamage >= pEntity->GetHealth(), bBacktrack));
					}
				}
			}
			bBacktrack = true;
		}
	}

	if (vecRecordSave.empty())
		return Vector(0, 0, 0);

	if (vecRecordSave.size() > 1)
		std::sort(vecRecordSave.begin(), vecRecordSave.end(), HitscanComparator);

	auto pRecord = &vecRecordSave.front();

	if (iMinimumDamage > 100)
		iMinimumDamage = pRecord->pRecord->pEntity->GetHealth() + (iMinimumDamage - 100);

	if (pRecord->flDamage < iMinimumDamage)
		if (iMinimumDamage - pRecord->flDamage > 10)
			return Vector(0, 0, 0);

	rageBotData.SetTarget(pRecord->pRecord, vecEyePosition, pRecord->bBacktrack, pRecord->pRecord->flResolverAngle);
	rageBotData.flDamage = pRecord->flDamage;
	rageBotData.iHitbox = pRecord->iHitgroup;

	return pRecord->vecPoint;
}

bool CRageBot::Hitchance( CBaseEntity* pEnt, CBaseCombatWeapon* pWeapon, Vector vecFrom, int iChance, Vector vecEyePosition) {

	float flFinalHitchance = 0;
	CCSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData( );

	if ( !pWeaponInfo )
		return false;

	//if ( exploits::bIsShiftingTicks || ( cfg::rage::doubletap && IPT::HandleInput( cfg::rage::doubletapkey ) && i::GlobalVars->flCurrentTime - pWeapon->GetLastShotTime( ) <= TICKS_TO_TIME( 15 ) ) )
	//	return true;

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

		if ( Trace.pHitEntity == pEnt)
			iHits++;
	}

	flFinalHitchance = static_cast< int >( ( float( iHits ) / ( iAccuracry / 100.f ) ) );

	if (flFinalHitchance >= iChance) {
		rageBotData.flHitchance = flFinalHitchance;
		return true;
	}

	return false;
}

void CRageBot::AutoStop( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CBaseEntity* pTarget, CUserCmd* pCmd, Vector vecShootPosition ) {

	// Credit to @Monthyx
	// Fast stop source from obelus
	if ( !ConfigAutoStop( pWeapon ) )
		return;

	if ( !(pLocal->GetFlags( ) & FL_ONGROUND ) && !ConfigAutoStopInAir( pWeapon ) )
		return;

	// server is currently in nospread, no need to autostop
	if ( i::ConVar->FindVar( "weapon_accuracy_nospread" )->GetInt( ) >= 1 )
		return;

	if (!ConfigAutoStopBetweenShots(pWeapon) && !CheckShootingCondition( pCmd, pLocal, pWeapon) )
			return;

	if ( rageBotData.bCanShoot )
		return;

	if (misc::bRetreat && IPT::HandleInput(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick)
		return;

	if (exploits::bIsShiftingTicks && !(IPT::HandleInput(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick))
		return;

	float flIdealSpeed = ( 0.28f ) * (pLocal->IsScoped( ) ? pWeapon->GetCSWpnData( )->flMaxSpeed[ 1 ] : pWeapon->GetCSWpnData( )->flMaxSpeed[ 0 ] );

	pCmd->iButtons &= ~IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;

	// Get the ideal speed for shooting (playstyle)
	Vector velocity = pLocal->GetVelocity( );
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
		return 110;
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
		return 110;
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

void CRageBot::AddHitbox(int index, std::array<bool, HITBOX_MAX>& vecHitboxList) {

	// server only allows headshots, so let's only push_back head and return!
	if (i::ConVar->FindVar("mp_damage_headshot_only")->GetBool()) {
		vecHitboxList[HITBOX_HEAD] = true;
		return;
	}
	switch (index)
	{
	case 0:
		vecHitboxList[HITBOX_HEAD] = true;
		break;

	case 1:
		vecHitboxList[HITBOX_UPPER_CHEST] = true;
		break;

	case 2:
		vecHitboxList[HITBOX_CHEST] = true;;
		break;

	case 3:
		vecHitboxList[HITBOX_STOMACH] = true;
		break;

	case 4:
		vecHitboxList[HITBOX_RIGHT_FOREARM] = true;
		vecHitboxList[HITBOX_LEFT_FOREARM] = true;

		vecHitboxList[HITBOX_LEFT_UPPER_ARM] = true;
		vecHitboxList[HITBOX_RIGHT_UPPER_ARM] = true;
		break;

	case 5:
		vecHitboxList[HITBOX_RIGHT_FOOT] = true;
		vecHitboxList[HITBOX_LEFT_FOOT] = true;
		break;
	}
};

std::array<bool, HITBOX_MAX> CRageBot::ConfigHitboxes( CBaseCombatWeapon * pWeapon ) {

	// "head", "upper chest", "lower chest", "stomach", "arms", "legs"

	std::array<bool, HITBOX_MAX> vecHitboxes = { false };
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if (cfg::rage::forceBaim && IPT::HandleInput(cfg::rage::forceBaimKey)) {
		AddHitbox(2, vecHitboxes);
		AddHitbox(3, vecHitboxes);
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

	std::array<bool, HITBOX_MAX> arrHitboxes = { false };
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

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

	std::array<bool, HITBOX_MAX> arrHitboxes = { false };
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

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

bool CRageBot::CheckShootingCondition( CUserCmd * pCmd, CBaseEntity * pLocal, CBaseCombatWeapon* pWeapon ) {

	if ( !pLocal || !pWeapon)
		return false;

	float flServerTime = TICKS_TO_TIME( pLocal->GetTickBase( ) );
	const CCSWeaponInfo* pWeaponData = pWeapon->GetCSWpnData( );

	if ( !pWeaponData )
		return false;

	if (pWeapon->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_C4 )
		return false;

	if (pWeapon->GetAmmo( ) < 1 )
		return false;

	if (pWeapon->GetItemDefinitionIndex() != WEAPON_REVOLVER)
		if (pWeapon->GetNextPrimaryAttack( ) > flServerTime || pWeapon->GetNextSecondaryAttack( ) > flServerTime )
			return false;

	if (pLocal->GetNextAttack() > flServerTime)
		return false;

	if (pWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
	{
		const auto fl_postpone_fire_ready = pWeapon->GetFireReadyTime();
		if (fl_postpone_fire_ready > TICKS_TO_TIME(networking.GetCorrectedTickbase()))
			return false;
	}

	return true;
}

std::vector<Vector> CRageBot::CreatePoints(Vector vecEyePosition, CBaseCombatWeapon* pWeapon, Lagcompensation::LagRecord_t* pRecord, int iHitbox) {

	std::array<bool, HITBOX_MAX> vecSelectedMultipoint = ConfigMultiHitboxes(pWeapon);
	std::vector<Vector> refVecPoints{};
	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);
	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;

	float flRadius = 0.f;
	mstudiobbox_t refStudioBox;
	Vector vecCenter = pRecord->pEntity->GetHitboxPosition(iHitbox, pRecord->pMatricies[RESOLVE], flRadius, &refStudioBox);
	matrix3x4_t& refMatrixBone = pRecord->pMatricies[RESOLVE][refStudioBox.iBone];

	if (!vecSelectedMultipoint[iHitbox]) {
		refVecPoints.emplace_back(vecCenter);
		return refVecPoints;
	}

	float flHitboxDistance = flRadius * ((iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints) * 0.01f);

	Vector vecCurrentAngles;
	M::VectorAngles(vecCenter - vecEyePosition, vecCurrentAngles);
	Vector vecForward; M::AngleVectors(vecCurrentAngles, &vecForward);

	const Vector vecRight = vecForward.CrossProduct(Vector(0, 0, 1));
	const Vector vecLeft = Vector(-vecRight.x, -vecRight.y, vecRight.z);
	const Vector vecTop = Vector(0, 0, 1);

	refVecPoints.emplace_back(vecCenter);
	if (iHitbox == HITBOX_HEAD) {
		refVecPoints.emplace_back(vecCenter + vecTop * flHitboxDistance);
		refVecPoints.emplace_back(vecCenter + (vecTop * flHitboxDistance) + (vecLeft * (flHitboxDistance * 0.67f)));
		refVecPoints.emplace_back(vecCenter + (vecTop * flHitboxDistance) + (vecRight * (flHitboxDistance * 0.67f)));
		refVecPoints.emplace_back(vecCenter - vecTop * flHitboxDistance);
	}
	refVecPoints.emplace_back(vecCenter + vecLeft * flHitboxDistance);
	refVecPoints.emplace_back(vecCenter + vecRight * flHitboxDistance);

	//for (auto& something : refVecPoints)
	//	g::drawList.emplace_back(something);

	//g::drawList.emplace_back(vecCenter);
	//if (iHitbox == HITBOX_HEAD) {
	//	g::drawList.emplace_back(vecCenter + vecTop * flHitboxDistance);
	//	g::drawList.emplace_back(vecCenter - vecTop * flHitboxDistance);
	//}
	//g::drawList.emplace_back(vecCenter + vecLeft * flHitboxDistance);
	//g::drawList.emplace_back(vecCenter + vecRight * flHitboxDistance);

	return refVecPoints;
}

std::vector<Vector> CRageBot::CreatePoints( CBaseEntity * pTarget, CBaseEntity * pLocal, CBaseCombatWeapon * pWeapon, Vector vecAngle, float flRadius, int iHitbox, bool bBuildSideOnly) {

	static int iMultiOptimization[HITBOX_MAX];
	static std::vector<Vector> output{};
	output.clear();

	if (flRadius <= 0)
		return output;

	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;
	float flHitboxDistance = flRadius * ( (iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints ) * 0.01f );

	if (!(pTarget->GetFlags() & FL_ONGROUND) && iHitbox == HITBOX_HEAD)
		flHitboxDistance = 0.7f;

	//bool bGenerateMore = (iHitbox == HITBOX_HEAD ? *pHeadPoints > 50.f ? true : false : *pBodyPoints > 50.f ? true : false);

	if (bBuildSideOnly) {
		if (iHitbox == HITBOX_HEAD) {
			output.push_back(vecAngle + Vector(0.f, 0.f, flHitboxDistance));
			output.push_back(vecAngle + Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle - Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle + Vector(0.f, flHitboxDistance, 0.f));
			output.push_back(vecAngle - Vector(0.f, flHitboxDistance, 0.f));
			//output.push_back(vecAngle - Vector(0.f, 0.f, flHitboxDistance));
		}
		else {
			output.push_back(vecAngle + Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle - Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle + Vector(0.f, flHitboxDistance, 0.f));
			output.push_back(vecAngle - Vector(0.f, flHitboxDistance, 0.f));
		}
	}
	else {
		if (iHitbox == HITBOX_HEAD) {
			output.push_back(vecAngle);
			output.push_back(vecAngle + Vector(0.f, 0.f, flHitboxDistance));
			output.push_back(vecAngle + Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle - Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle + Vector(0.f, flHitboxDistance, 0.f));
			output.push_back(vecAngle - Vector(0.f, flHitboxDistance, 0.f));
		}
		else {
			output.push_back(vecAngle);
			output.push_back(vecAngle + Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle - Vector(flHitboxDistance, 0.f, 0.f));
			output.push_back(vecAngle + Vector(0.f, flHitboxDistance, 0.f));
			output.push_back(vecAngle - Vector(0.f, flHitboxDistance, 0.f));
		}
	}

	return output;
}

#pragma runtime_checks( "", off )
bool CRageBot::bCollidePoint(const Vector& vecStart, const Vector& vecEnd, mstudiobbox_t* pHitbox, matrix3x4_t* aMatrix) {

	Ray_t Ray(vecStart, vecEnd);

	Trace_t Trace;
	Trace.flFraction = 1.0f;
	Trace.bStartSolid = false;
	// original: 55 8B EC 83 E4 F8 F3 0F 10 42
	// kittenpopo: 55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F
	typedef int(__fastcall* ClipRayToHitbox_t)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, Trace_t&);
	static auto sig = (void*)((DWORD)(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F"))));
	return ((ClipRayToHitbox_t)(sig))(Ray, pHitbox, aMatrix[pHitbox->iBone], Trace) >= 0;
}
#pragma runtime_checks( "", restore )

int CRageBot::SafePoint( Vector & vecEyePosition, CBaseCombatWeapon * pWeapon, Lagcompensation::LagRecord_t * pRecord, Vector & vecShootposition, int iHitbox) {

	if (pRecord->pMatricies[EMatrixType::RIGHT]->GetOrigin() == Vector(0, 0, 0) ||
		pRecord->pMatricies[EMatrixType::LEFT]->GetOrigin() == Vector(0, 0, 0) ||
		pRecord->pMatricies[EMatrixType::CENTER]->GetOrigin() == Vector(0, 0, 0))
	{
		return 0;
	}

	if (!pRecord)
		return 0;

	int iSafePoint = 0;
	Vector vecStart = vecEyePosition;
	Vector vecEnd = vecShootposition;
	//Vector vecForward = vecStart + ((vecEnd - vecStart) * 8192.0f);

	const Model_t* model = pRecord->pEntity->GetModel();
	if (!model)
		return 0;

	studiohdr_t* studioHdr = i::ModelInfo->GetStudioModel(model);
	if (!studioHdr)
		return 0;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(iHitbox);
	if (!studioBox)
		return 0;

	if (bCollidePoint(vecStart, vecEnd, studioBox, pRecord->pMatricies[RIGHT]))
		iSafePoint++;

	if (bCollidePoint(vecStart, vecEnd, studioBox, pRecord->pMatricies[LEFT]))
		iSafePoint++;

	if (bCollidePoint(vecStart, vecEnd, studioBox, pRecord->pMatricies[CENTER]))
		iSafePoint++;

	return iSafePoint;
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

	if (IPT::HandleInput(cfg::rage::doubletapkey) && cfg::rage::doubletap && exploits::bCharged)
		return g::pCmd->iTickCount;

	return lagcomp.FixTickCount(flSimulationTime);

	// calculate lerp remainder.
	float flLerpRemainder = std::fmodf( lagcomp.GetClientInterpAmount( ), i::GlobalVars->flIntervalPerTick );

	// get real simulation time and calculate interp fraction.
	if ( flLerpRemainder > 0.f )
		flSimulationTime += i::GlobalVars->flIntervalPerTick - flLerpRemainder;

	return TIME_TO_TICKS(flSimulationTime);
}

bool CRageBot::ShouldSendPacket(bool& bSendPacket) {

	if (cfg::antiaim::fakeduck && IPT::HandleInput(cfg::antiaim::fakeduckbind))
		return bSendPacket;

	if (cfg::rage::doubletap && IPT::HandleInput(cfg::rage::doubletapkey))
		return false;

	return true;
}

std::array<Vector, 6> CRageBot::HitboxPoints(Lagcompensation::LagRecord_t* pRecord, CBaseCombatWeapon* pWeapon, Vector& vecShootPosition, int iHitbox) {

	std::array<Vector, 6> arrPoints = { Vector(0, 0, 0) };

	const Model_t* model = pRecord->pEntity->GetModel();
	if (!model)
		return arrPoints;

	studiohdr_t* studioHdr = i::ModelInfo->GetStudioModel(model);
	if (!studioHdr)
		return arrPoints;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(iHitbox);
	if (!studioBox)
		return arrPoints;

	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;

	if (!(pRecord->iFlags & FL_ONGROUND))
		*pHeadPoints = 0.7f; // This will allow the aimbot to shoot head between legs when jumping

	float flHitboxDistance = ((iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints) * 0.01f);

	Vector vecTransformedCenter = (studioBox->vecBBMin + studioBox->vecBBMax) * 0.5f;
	M::VectorTransform(vecTransformedCenter, pRecord->pMatricies[1][studioBox->iBone], vecTransformedCenter);

	float flDistance = vecTransformedCenter.DistTo(vecShootPosition);
	float flRadiusTransformed = max(studioBox->flRadius - flDistance, 0.f);

	if (studioBox->flRadius <= 0.f) {

		matrix3x4_t matRotationMatrix;
		M::AngleMatrix(studioBox->angOffsetOrientation, matRotationMatrix);

		matrix3x4_t matAppliedMatrix;
		M::ConcatTransforms(pRecord->pMatricies[1][studioBox->iBone], matRotationMatrix, matAppliedMatrix);

		Vector vecOrigin = matAppliedMatrix.GetOrigin();
		Vector vecCenter = (studioBox->vecBBMin + studioBox->vecBBMax) * 0.5f;

		if (iHitbox == HITBOX_RIGHT_FOOT || iHitbox == HITBOX_LEFT_FOOT) {

			float flFeetPoint = (studioBox->vecBBMin.z - vecCenter.z) * 0.875f;

			if (iHitbox == HITBOX_LEFT_FOOT)
				flFeetPoint *= -1;

			arrPoints[0] = Vector(vecCenter.x, vecCenter.y, vecCenter.z + flFeetPoint);
			
			flFeetPoint = (studioBox->vecBBMin.x - vecCenter.x) * flHitboxDistance;
			arrPoints[1] = Vector(vecCenter.x + flFeetPoint, vecCenter.y, vecCenter.z);

			flFeetPoint = (studioBox->vecBBMax.x - vecCenter.x) * flHitboxDistance;
			arrPoints[2] = Vector(vecCenter.x + flFeetPoint, vecCenter.y, vecCenter.z);
		}
		// max could be only 3
		for (size_t i = 0; i < 3; i++) {

			if (arrPoints[i] == Vector(0, 0, 0))
				continue;

			arrPoints[i] = Vector(arrPoints[i].DotProduct(matAppliedMatrix[0]), arrPoints[i].DotProduct(matAppliedMatrix[1]), arrPoints[i].DotProduct(matAppliedMatrix[2]));
			arrPoints[i] += vecOrigin;
		}
	}
	else {

		Vector vecCenter = (studioBox->vecBBMin + studioBox->vecBBMax) * 0.5f;

		if (iHitbox == HITBOX_HEAD) {

			arrPoints[0] = vecCenter;

			// rotation matrix 45 degrees.
			constexpr float rotation = 0.70710678f;

			// top/back 45 deg.
			// this is the best spot to shoot at.
			arrPoints[1] = Vector(
				studioBox->vecBBMax.x + (rotation * flHitboxDistance),
				studioBox->vecBBMax.y + (-rotation * flHitboxDistance),
				studioBox->vecBBMax.z
			);

			// right side
			arrPoints[2] = Vector(
				studioBox->vecBBMax.x, 
				studioBox->vecBBMax.y, 
				studioBox->vecBBMax.z + flHitboxDistance
			);

			// left side
			arrPoints[3] = Vector(
				studioBox->vecBBMax.x,
				studioBox->vecBBMax.y,
				studioBox->vecBBMax.z - flHitboxDistance
			);

			// back side
			arrPoints[4] = Vector(
				studioBox->vecBBMax.x,
				studioBox->vecBBMax.y - flHitboxDistance,
				studioBox->vecBBMax.z
			);

			if (auto pAnimState = pRecord->pEntity->AnimState();
				pAnimState != nullptr &&
				pRecord->vecVelocity.Length() <= 0.1f &&
				pRecord->vecEyeAngles.x <= pAnimState->flEyePitch) {

				// bottom side
				arrPoints[5] = Vector(
					studioBox->vecBBMax.x - flHitboxDistance,
					studioBox->vecBBMax.y,
					studioBox->vecBBMax.z
				);
			}
		}
		else if (iHitbox == HITBOX_STOMACH) {

			arrPoints[0] = vecCenter;

			// back side
			arrPoints[1] = Vector(
				vecCenter.x,
				studioBox->vecBBMax.y - flHitboxDistance,
				vecCenter.z
			);
		}
		else if (iHitbox == HITBOX_PELVIS) {

			// back side
			arrPoints[0] = Vector(
				vecCenter.x,
				studioBox->vecBBMax.y - flHitboxDistance,
				vecCenter.z
			);
		}
		else if (iHitbox == HITBOX_UPPER_CHEST) {

			// back side
			arrPoints[0] = Vector(
				vecCenter.x,
				studioBox->vecBBMax.y - flHitboxDistance,
				vecCenter.z
			);
		}
		else if (iHitbox == HITBOX_THORAX || iHitbox == HITBOX_CHEST) {

			arrPoints[0] = vecCenter;

			// back side
			arrPoints[1] = Vector(
				vecCenter.x,
				studioBox->vecBBMax.y - flHitboxDistance,
				vecCenter.z
			);
		}
		else if (iHitbox == HITBOX_RIGHT_CALF || iHitbox == HITBOX_LEFT_CALF) {

			arrPoints[0] = vecCenter;

			// back side
			arrPoints[1] = Vector(
				studioBox->vecBBMax.x - (studioBox->flRadius * 0.5f),
				studioBox->vecBBMax.y,
				studioBox->vecBBMax.z
			);
		}
		else if (iHitbox == HITBOX_RIGHT_THIGH || iHitbox == HITBOX_LEFT_THIGH) {

			arrPoints[0] = vecCenter;
		}
		else if (iHitbox == HITBOX_RIGHT_UPPER_ARM || iHitbox == HITBOX_LEFT_UPPER_ARM) {

			arrPoints[0] = vecCenter;
		}

		for (size_t i = 0; i < 5; i++) {

			if (arrPoints[i] == Vector(0, 0, 0))
				continue;

			M::VectorTransform(arrPoints[i], pRecord->pMatricies[1][studioBox->iBone], arrPoints[i]);
		}
	}
	return arrPoints;
}

bool CRageBot::ConfigAutoStopInAir(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::m_bAutoStopInAir[0];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::m_bAutoStopInAir[1];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::m_bAutoStopInAir[2];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::m_bAutoStopInAir[4];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::m_bAutoStopInAir[3];
	}
	else {
		return cfg::rage::m_bAutoStopInAir[5];
	}
}

bool CRageBot::ConfigAutoStopBetweenShots(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::betweenshots[0];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::betweenshots[1];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::betweenshots[2];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::betweenshots[4];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::betweenshots[3];
	}
	else {
		return cfg::rage::betweenshots[5];
	}
}

bool CRageBot::ConfigAutoStopAggressiveness(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::autostopAggressiveness[0];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::autostopAggressiveness[1];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::autostopAggressiveness[2];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::autostopAggressiveness[4];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::autostopAggressiveness[3];
	}
	else {
		return cfg::rage::autostopAggressiveness[5];
	}
}