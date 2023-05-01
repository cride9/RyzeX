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

	auto pEnt1Hitbox = std::get<0>( pEnt1 )->GetHitboxPosition( HITBOX_UPPER_CHEST, std::get<1>(pEnt1)->pMatrix );
	auto pEnt2Hitbox = std::get<0>( pEnt2 )->GetHitboxPosition( HITBOX_UPPER_CHEST, std::get<1>(pEnt2)->pMatrix);

	M::VectorAngles( pEnt1Hitbox - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween1 = ( g::vecOriginalViewAngle - vecCalcAngle.NormalizeAngle( ) );

	M::VectorAngles( pEnt2Hitbox - vecEyePosition, vecCalcAngle );
	Vector vecDistanceBetween2 = ( g::vecOriginalViewAngle - vecCalcAngle.NormalizeAngle( ) );

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
	if ( !pLocal || !cfg::rage::enable ) {
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

	Vector vecEyePosition = pLocal->GetEyePosition( );

	if (Vector vecHitscan = Hitscan(pLocal, pWeapon, vecEyePosition); vecHitscan != Vector(0, 0, 0)) {

		exploits::bCanCharge = false;
		if (cfg::rage::betweenshots)
			rageBotData.bCanShoot = false;

		AutoStop(pLocal, pWeapon, rageBotData.pAimbotTarget, g::pCmd, vecHitscan);
		// get better eye position.
		// calculate aim angle.
		Vector shootAngle;
		M::VectorAngles(vecHitscan - vecEyePosition, shootAngle); // https://www.unknowncheats.me/forum/counterstrike-global-offensive/137492-math-hack-1-coding-aimbot-stop-using-calcangle.html

		if (CheckShootingCondition(pCmd, pLocal)) {

			if (Hitchance(rageBotData.pAimbotTarget, pWeapon, shootAngle, ConfigHitChance(pWeapon), vecEyePosition, rageBotData.iTargetedHitbox)) {

				rageBotData.bCanShoot = true;

				pCmd->angViewPoint = (shootAngle -= (pLocal->GetAimPunch() * recoilScale->GetFloat()));
				pCmd->iButtons |= IN_ATTACK;
				pCmd->iTickCount = CalculateTickCount(rageBotData.flTargetSimulation);
				rageBotData.iCommand = pCmd->iCommandNumber;

				ShouldSendPacket(bSendPacket);
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

Lagcompensation::LagRecord_t* CRageBot::CheckOnShotRecord(Lagcompensation::AnimationInfo_t* pLog) {

	for (size_t i = 0; i < min(pLog->iLastValid, pLog->pRecord.size()); i++) 
		if (pLog->pRecord.at(i).bDidShot && pLog->pRecord.at(i).bValid)
			return &pLog->pRecord.at(i);
	
	return nullptr;
}

bool CRageBot::CheckBaimRecord(CBaseEntity* pLocal, Lagcompensation::LagRecord_t* pLog, Vector& vecEyePosition, CBaseCombatWeapon* pWeapon) {

	//autowall.SimulateFireBullet(pLocal, pWeapon, data);
	float flDamage = autowall.GetDamage(pLocal, vecEyePosition, pLog->pEntity->GetHitboxPosition(HITBOX_STOMACH, pLog->pMatrix), pWeapon, nullptr, pLog->pEntity);
	return flDamage > pLog->pEntity->GetHealth() + 10;
}

Vector CRageBot::Hitscan( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition) {

	std::array<bool, HITBOX_MAX> vecSelectedHitboxes = ConfigHitboxes(pWeapon);
	std::array<bool, HITBOX_MAX> vecSelectedSafePoints = ConfigSafeHitboxes(pWeapon);
	std::array<bool, HITBOX_MAX> vecSelectedMultipoint = ConfigMultiHitboxes(pWeapon);
	int iMinimumDamage = ConfigMinimumDamage(pWeapon);
	bool bForceSafe = ConfigForceSafe(pWeapon);

	for (size_t i = 1; i < i::GlobalVars->nMaxClients; i++) {
		
		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		/* Entity checks */
		if (!pEntity || !g::pLocal || !pEntity->IsAlive() || !g::pLocal->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity() || g::pLocal->GetTeam() == pEntity->GetTeam())
			continue;

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(i);

		/* Lagcomp checks */
		if (!pLog || !pLog->pEntity || pLog->pEntity != pEntity || pLog->pRecord.size() < 2)
			continue;

		Lagcompensation::LagRecord_t* pCurrentApplied = CheckOnShotRecord(pLog);

		if (!pCurrentApplied) 
			pCurrentApplied = &pLog->pRecord.at(pLog->iFirstValid);

		bool bSafePoint = false;

		int iBone = -1;
		float flRadius = -1.f, flDamage = -1.f;
		Vector vecMins, vecMaxs;
		for (size_t iHitbox = 0; iHitbox < HITBOX_MAX; iHitbox++) {

			if (!vecSelectedHitboxes[iHitbox])
				continue;

			if (CheckBaimRecord(pLocal, pCurrentApplied, vecEyePosition, pWeapon))
				iHitbox = HITBOX_STOMACH;

			Vector vecHitboxPosition = pCurrentApplied->pEntity->GetHitboxPosition(iHitbox, pCurrentApplied->pMatrix, vecMins, vecMaxs, flRadius);

			Vector output;
			M::VectorAngles(vecHitboxPosition - vecEyePosition, output);
			Vector vecDistanceBetween = (g::vecOriginalViewAngle - output.NormalizeAngle());

			if (abs((vecDistanceBetween).NormalizeAngle().Length2D()) > cfg::rage::iAimbotFov)
				continue;

			{ // I did separate them because of variable redefinition, this way the compiler can handle multiple declerations
				std::array<Vector, 3> multiPointed = { Vector(0, 0, 0) };

				if (vecSelectedMultipoint[iHitbox])
					multiPointed = CreatePoints(pCurrentApplied->pEntity, pLocal, pWeapon, vecHitboxPosition, flRadius, iHitbox);
				else
					multiPointed[0] = vecHitboxPosition;

				for (Vector& vecPoint : multiPointed) {

					if (vecPoint == Vector(0, 0, 0))
						continue;

					if (vecSelectedSafePoints[iHitbox]) {

						if (SafePoint(vecEyePosition, pWeapon, pCurrentApplied, vecPoint, iHitbox))
							bSafePoint = true;

						if (bForceSafe && !bSafePoint)
							continue;
					}

					if (flDamage = autowall.GetDamage(pLocal, vecEyePosition, vecPoint, pWeapon, nullptr, pCurrentApplied->pEntity); flDamage > iMinimumDamage || flDamage > pEntity->GetHealth() + 5) {

						rageBotData.SetTarget(pCurrentApplied, iHitbox);
						return vecPoint;
					}
				}

			}
			
			if (!cfg::rage::m_bEnableBacktrack || (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey) && cfg::antiaim::defensive))
				continue;

			const unsigned int iLastValid = pLog->iLastValid;
			if (iLastValid >= pLog->pRecord.size())
				continue;

			if ((pCurrentApplied->vecOrigin - pLog->pRecord.at(iLastValid).vecOrigin).Length2D() < 5.f && !pLog->pRecord.at(iLastValid).bDidShot)
				continue;

			{
				pCurrentApplied = &pLog->pRecord.at(iLastValid);
				pEntity->SetBoneCache(pCurrentApplied->pMatrix);

				if (!pCurrentApplied->bValid)
					continue;

				Vector vecHitboxPosition = pCurrentApplied->pEntity->GetHitboxPosition(iHitbox, pCurrentApplied->pMatrix, vecMins, vecMaxs, flRadius);
				std::array<Vector, 3> multiPointed = { Vector(0, 0, 0) };

				if (vecSelectedMultipoint[iHitbox])
					multiPointed = CreatePoints(pCurrentApplied->pEntity, pLocal, pWeapon, vecHitboxPosition, flRadius, iHitbox);
				else
					multiPointed[0] = vecHitboxPosition;

				for (Vector& vecPoint : multiPointed) {

					if (vecPoint == Vector(0, 0, 0))
						continue;

					if (vecSelectedSafePoints[iHitbox]) {

						if (SafePoint(vecEyePosition, pWeapon, pCurrentApplied, vecPoint, iHitbox))
							bSafePoint = true;

						if (bForceSafe && !bSafePoint)
							continue;
					}

					if (flDamage = autowall.GetDamage(pLocal, vecEyePosition, vecPoint, pWeapon, nullptr, pCurrentApplied->pEntity); flDamage > iMinimumDamage || flDamage > pEntity->GetHealth() + 5) {

						rageBotData.SetTarget(pCurrentApplied, iHitbox);
						return vecPoint;
					}
				}
			}
		}
	}

	return Vector(0, 0, 0);
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

void CRageBot::AutoStop( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CBaseEntity* pTarget, CUserCmd* pCmd, Vector vecShootPosition ) {

	// Credit to @Monthyx
	// Fast stop source from obelus

	static auto m_bStopinAir = []( CBaseCombatWeapon* pWeapon ) -> bool {
		
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

	static auto m_bBetweenShots = []( CBaseCombatWeapon* pWeapon ) -> bool {
		
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

	static auto m_iAggressiveness = [](CBaseCombatWeapon* pWeapon) -> int {

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

	if (misc::bRetreat && GetAsyncKeyState(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick)
		return;

	float flIdealSpeed = ( 0.28f ) * ( g::pLocal->IsScoped( ) ? pWeapon->GetCSWpnData( )->flMaxSpeed[ 1 ] : pWeapon->GetCSWpnData( )->flMaxSpeed[ 0 ] );

	/*int predictTick = 0;
	switch (m_iAggressiveness(pWeapon))
	{
	case 1:
		predictTick = 2; break;
	case 2:
		predictTick = 4; break;
	case 3:
		predictTick = 8; break;
	default :
		predictTick = 1; break;
	}

	Vector vecInterpolatedEyePosition = InterpolateLocalEyePosition(g::pLocal->GetEyePosition(), predictTick);

	FireBulletData_t data = { };
	data.vecPosition = vecInterpolatedEyePosition;
	data.vecDirection = (vecShootPosition - vecInterpolatedEyePosition).Normalized();

	if (!autowall.SimulateFireBullet(g::pLocal, g::pLocal->GetWeapon(), data))
		return;*/

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

std::array<bool, HITBOX_MAX> CRageBot::ConfigHitboxes( CBaseCombatWeapon * pWeapon ) {

	// "head", "upper chest", "lower chest", "stomach", "arms", "legs"

	std::array<bool, HITBOX_MAX> vecHitboxes = {};
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	static auto AddHitbox = []( int index, std::array<bool, HITBOX_MAX>& vecHitboxList ) {

		// server only allows headshots, so let's only push_back head and return!
		if ( i::ConVar->FindVar( "mp_damage_headshot_only" )->GetBool( ) ) {
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


	if (cfg::rage::forceBaim && GetKeyState(cfg::rage::forceBaimKey)) {
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

	std::array<bool, HITBOX_MAX> arrHitboxes = {};
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	static auto AddHitbox = [](int index, std::array<bool, HITBOX_MAX>& vecHitboxList) {

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

	static auto AddHitbox = [](int index, std::array<bool, HITBOX_MAX>& vecHitboxList) {

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

	if (pLocal->GetNextAttack() > flServerTime)
		return false;

	return true;
}

std::array<Vector, 3> CRageBot::CreatePoints( CBaseEntity * pTarget, CBaseEntity * pLocal, CBaseCombatWeapon * pWeapon, Vector vecAngle, float flRadius, int iHitbox) {

	std::array<Vector, 3> output = {Vector(0, 0, 0)};

	if ( flRadius <= 0 )
		return output = { vecAngle, Vector(0, 0, 0), Vector(0, 0, 0) };

	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;
	float flHitboxDistance = flRadius * ( (iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints ) * 0.01f );

	bool bGenerateMore = (iHitbox == HITBOX_HEAD ? *pHeadPoints > 50.f ? true : false : *pBodyPoints > 50.f ? true : false);

	static int iMultiOptimization[HITBOX_MAX];

	output[0] = vecAngle;
	switch (iMultiOptimization[iHitbox] % 5)
	{
	case 0:
		output[1] = (vecAngle + Vector(flHitboxDistance, 0.f, 0.f));
		if (bGenerateMore)
			output[2] = (vecAngle + Vector(flHitboxDistance * 0.5f, 0.f, 0.f));
		break;
	case 1:
		output[1] = (vecAngle + Vector(0.f, flHitboxDistance, 0.f));
		if (bGenerateMore)
			output[2] = (vecAngle + Vector(0.f, flHitboxDistance * 0.5f, 0.f));
		break;
	case 2:
		output[1] = (vecAngle + Vector(0.f, 0.f, flHitboxDistance));
		if (bGenerateMore)
			output[2] = (vecAngle + Vector(0.f, 0.f, flHitboxDistance * 0.5f));
		break;
	case 3:
		output[1] = (vecAngle + Vector(0.f, -flHitboxDistance, 0.f));
		if (bGenerateMore)
			output[2] = (vecAngle + Vector(0.f, -flHitboxDistance * 0.5f, 0.f));
		break;
	case 4:
		output[1] = (vecAngle + Vector(-flHitboxDistance, 0.f, 0.f));
		if (bGenerateMore)
			output[2] = (vecAngle + Vector(-flHitboxDistance * 0.5f, 0.f, 0.f));
		break;
	}
	if (!bGenerateMore)
		output[2] = Vector(0, 0, 0);
	iMultiOptimization[iHitbox]++;

	return output;
}

bool CRageBot::SafePoint( Vector & vecEyePosition, CBaseCombatWeapon * pWeapon, Lagcompensation::LagRecord_t * pRecord, Vector & vecShootposition, int iHitbox) {

	if (pRecord->pRightMatrix->GetOrigin() == Vector(0, 0, 0) ||
		pRecord->pLeftMatrix->GetOrigin() == Vector(0, 0, 0) ||
		pRecord->pCenterMatrix->GetOrigin() == Vector(0, 0, 0))
		return false;

	/* Just decleare a static enum so we can make it look cleaner */
	static enum SIDE {
		RIGHT,
		LEFT,
		CENTER
	};

	/* Make arrays to store informations */
	bool bHit[3] = { false };

	/* Prepare tracing output */
	Trace_t traceOutput;
	Ray_t traceRay(vecEyePosition, vecShootposition);

	/* We need to set bonecache for every trace so csgo will think the enemy is there (so it can return if we hit it) */
	pRecord->pEntity->SetBoneCache(pRecord->pRightMatrix);
	if (i::EngineTrace->ClipRayToEntity(traceRay, MASK_SHOT, pRecord->pEntity, &traceOutput); traceOutput.pHitEntity == pRecord->pEntity)
		bHit[RIGHT] = true;

	pRecord->pEntity->SetBoneCache(pRecord->pLeftMatrix);
	if (i::EngineTrace->ClipRayToEntity(traceRay, MASK_SHOT, pRecord->pEntity, &traceOutput); traceOutput.pHitEntity == pRecord->pEntity)
		bHit[LEFT] = true;

	pRecord->pEntity->SetBoneCache(pRecord->pCenterMatrix);
	if (i::EngineTrace->ClipRayToEntity(traceRay, MASK_SHOT, pRecord->pEntity, &traceOutput); traceOutput.pHitEntity == pRecord->pEntity)
		bHit[CENTER] = true;

	/* Set back matrix to not mess up any ragebot calculations */
	pRecord->pEntity->SetBoneCache(pRecord->pMatrix);

	if (bHit[RIGHT] && bHit[LEFT] && bHit[CENTER])
		return true;

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

	return lagcomp.FixTickCount(flSimulationTime);

	// calculate lerp remainder.
	float flLerpRemainder = std::fmodf( lagcomp.GetClientInterpAmount( ), i::GlobalVars->flIntervalPerTick );

	// get real simulation time and calculate interp fraction.
	if ( flLerpRemainder > 0.f )
		flSimulationTime += i::GlobalVars->flIntervalPerTick - flLerpRemainder;

	return TIME_TO_TICKS(flSimulationTime);
}

bool CRageBot::ShouldSendPacket(bool& bSendPacket) {

	if (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind))
		return bSendPacket;

	if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey) && !g::bWaiting)
		return false;

	if (g::bWaiting) {
		return true;
	}

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
	float flHitboxDistance = ((iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints) * 0.01f);

	if (!(pRecord->fFlags & FL_ONGROUND))
		*pHeadPoints = 0.7f; // This will allow the aimbot to shoot head between legs when jumping

	Vector vecTransformedCenter = (studioBox->vecBBMin + studioBox->vecBBMax) * 0.5f;
	M::VectorTransform(vecTransformedCenter, pRecord->pMatrix[studioBox->iBone], vecTransformedCenter);

	float flDistance = vecTransformedCenter.DistTo(vecShootPosition);
	float flRadiusTransformed = max(studioBox->flRadius - flDistance, 0.f);

	if (studioBox->flRadius <= 0.f) {

		matrix3x4_t matRotationMatrix;
		M::AngleMatrix(studioBox->angOffsetOrientation, matRotationMatrix);

		matrix3x4_t matAppliedMatrix;
		M::ConcatTransforms(pRecord->pMatrix[studioBox->iBone], matRotationMatrix, matAppliedMatrix);

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

			M::VectorTransform(arrPoints[i], pRecord->pMatrix[studioBox->iBone], arrPoints[i]);
		}
	}
	return arrPoints;
}