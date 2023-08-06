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

bool HitscanComparator(Hitscan_t& a, Hitscan_t& b) {

	int aPoints = a.GetRecordPoints();
	int bPoints = b.GetRecordPoints();
	a.pRecord->flDesyncDelta < b.pRecord->flDesyncDelta ? aPoints++ : bPoints++;

	return aPoints > bPoints;
}

bool LowestHealth(CBaseEntity* pEnt1, CBaseEntity* pEnt2) {
	if (pEnt1->GetHealth() != pEnt2->GetHealth())
		return pEnt1->GetHealth() < pEnt2->GetHealth();
	else
		return false;
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

	static CConVar* recoilScale = i::ConVar->FindVar(XorStr("weapon_recoil_scale"));
	if ( !pLocal || !cfg::rage::bEnable || (!IPT::HandleInput(cfg::rage::iAimbotKey) && cfg::rage::iAimbotKey) || g::bUpdatingSkins) {
		exploits::bCanCharge = true;
		rageBotData.ClearTarget();
		hitlogData.ClearTarget();
		return;
	}

	if (!pLocal->IsAlive()) {
		rageBotData.ClearTarget();
		hitlogData.ClearTarget();
		return;
	}

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );
	if (!pWeapon || !pWeapon->GetCSWpnData()) {
		exploits::bCanCharge = true;
		rageBotData.ClearTarget();
		hitlogData.ClearTarget();
		return;
	}

	if (pWeapon->IsKnife() || pWeapon->IsGrenade()) {
		exploits::bCanCharge = true;
		rageBotData.ClearTarget();
		hitlogData.ClearTarget();
		return;
	}

	if (exploits::bIsCurrentlyCharging) {
		exploits::bCanCharge = true;
		rageBotData.ClearTarget();
		hitlogData.ClearTarget();
		return;
	}

	misc::RevolverCreateMove();
	Vector vecEyePosition = g::vecEyePosition;

	if (Vector vecHitscan = Hitscan(pLocal, pWeapon, vecEyePosition); vecHitscan != Vector(0, 0, 0)) {

		if (!rageBotData.pAimbotTarget)
			return;

		exploits::bCanCharge = false;

		Vector shootAngle;
		M::VectorAngles(vecHitscan - vecEyePosition, shootAngle); // https://www.unknowncheats.me/forum/counterstrike-global-offensive/137492-math-hack-1-coding-aimbot-stop-using-calcangle.html

		if (pLocal->CanShoot((CWeaponCSBase*)pWeapon)) {

			if (rageBotData.bCanShoot = Hitchance(rageBotData.pAimbotTarget, pWeapon, shootAngle, ConfigHitChance(pWeapon), vecEyePosition); rageBotData.bCanShoot) {

				Vector vecAngle = (shootAngle -= (pLocal->GetAimPunch() * recoilScale->GetFloat()));

				pCmd->angViewPoint = vecAngle;
				if (!cfg::rage::bSilentAim)
					i::EngineClient->SetViewAngles(vecAngle);

				pCmd->iButtons |= IN_ATTACK;

				g::bWasShootingInChokeCycle = i::ClientState->nChokedCommands > 1;

				bSetTickCount = true;
				rageBotData.iTickcount = pCmd->iTickCount;
				pCmd->iTickCount = CalculateTickCount(rageBotData.flTargetSimulation);
				bSendPacketThisTick = ShouldSendPacket(bSendPacket);

				hitlogData = rageBotData;
			}
			else {
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

		vecTargets.emplace_back( pEntity );
	}
	
	if ( !vecTargets.empty( ) )
		std::sort( vecTargets.begin( ), vecTargets.end( ), LowestHealth );
}

std::vector<Lagcompensation::LagRecord_t*> ChooseTargetRecord(Lagcompensation::AnimationInfo_t* pLog, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition) {

	if (pLog->pRecord.size() < 2)
		return {nullptr, nullptr};

	std::vector<Lagcompensation::LagRecord_t*> pRecords;
	for (int i = 0; i < pLog->iLastValid; i++) {

		Lagcompensation::LagRecord_t* pRecord = &pLog->pRecord.at(i);

		if (pRecord->bBreakingLagcompensation)
			continue;

		if (!pRecord->bValid)
			continue;

		if (pRecord->bDidShot) {
			pRecords.push_back(pRecord);
			continue;
		}

		//const int iHitboxToCheck = ((cfg::rage::bForceBaim && IPT::HandleInput(cfg::rage::iForceBaimKey)) || pRecord->pEntity->GetHealth() < pWeapon->GetCSWpnData()->iDamage) ? HITBOX_STOMACH : HITBOX_HEAD;
		const int iHitboxToCheck = HITBOX_HEAD;
		Vector vecHitboxPosition = pRecord->pEntity->GetHitboxPosition(iHitboxToCheck, pRecord->pMatricies[RESOLVE]);
		Trace_t traceData = Trace_t();
		CTraceFilter traceFilter = CTraceFilter(g::pLocal);

		i::EngineTrace->TraceRay(Ray_t(vecEyePosition, vecHitboxPosition), MASK_SHOT | CONTENTS_GRATE, &traceFilter, &traceData);
		if (traceData.pHitEntity != nullptr && traceData.pHitEntity == pLog->pEntity)
			if (ragebot.SafePoint(vecEyePosition, pWeapon, pRecord, vecHitboxPosition, iHitboxToCheck) == 3)
				pRecords.push_back(pRecord);

	}
	if (!pRecords.empty())
		return pRecords;

	return { &pLog->pRecord.front() };
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
	int iMinimumDamage = IPT::HandleInput(cfg::rage::iOverrideBind) ? ConfigOverrideDamage(pWeapon) : ConfigMinimumDamage(pWeapon);
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
		
		std::vector<Lagcompensation::LagRecord_t*> arrRecords = ChooseTargetRecord(pLog, pWeapon, vecEyePosition);

		bool bBacktrack = false;
		size_t iTick = 0;

		for (auto pCurrentApplied : arrRecords) {

			if (!pCurrentApplied)
				continue;

			pCurrentApplied->Apply(pEntity, false);
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

				int iScannedPoints = -1;
				for (Vector& vecPoint : multiPointed) {
					iScannedPoints++;
					int iSafePoint = SafePoint(vecEyePosition, pWeapon, pCurrentApplied, vecPoint, iHitbox);
					if (vecSelectedSafePoints[iHitbox]) {

						if ((bForceSafe || playerList::arrPlayers[pEntity->EntIndex()].bForceSafe) && iSafePoint < 3)
							continue;

						if (iSafePoint < 2)
							continue;
					}

					FireBulletData_t data;
					if (flDamage = autowall.GetDamage(pLocal, vecEyePosition, vecPoint, pWeapon, &data); flDamage > 0 /*|| (flDamage >= pEntity->GetHealth() + 5 && iHitbox != HITBOX_HEAD)*/) {

						if (data.enterTrace.pHitEntity && data.enterTrace.pHitEntity->IsPlayer())
							if (playerList::arrPlayers[data.enterTrace.pHitEntity->EntIndex()].iPriority == FRIEND)
								continue;

						bool bHead = false;
						if (iHitbox == HITBOX_HEAD) 
							bHead = bCollidePoint(vecEyePosition, vecPoint, pEntity->StudioHitbox(HITBOX_HEAD), pCurrentApplied->pMatricies[RESOLVE]);
						
						vecRecordSave.emplace_back(Hitscan_t(pCurrentApplied, vecPoint, flDamage, iHitbox, data.enterTrace.iHitGroup, iSafePoint == 3, flDamage > pEntity->GetHealth(), bBacktrack, bHead, data.enterTrace.iHitGroup != HITGROUP_HEAD, iScannedPoints == 0));
					}
				}
			}
		}
	}

	if (vecRecordSave.empty())
		return Vector(0, 0, 0);

	if (vecRecordSave.size() > 1)
		std::sort(vecRecordSave.begin(), vecRecordSave.end(), HitscanComparator);

	for (auto& refRecord : vecRecordSave) {

		float flTransformedDamage = iMinimumDamage;
		if (iMinimumDamage > 100)
			flTransformedDamage = refRecord.pRecord->pEntity->GetHealth() + (iMinimumDamage - 100);

		if (refRecord.flDamage < flTransformedDamage)
			continue;

		AutoStop(pLocal, pWeapon, refRecord.pRecord->pEntity, g::pCmd, refRecord.vecPoint);

		rageBotData.SetTarget(refRecord.pRecord, vecEyePosition, refRecord.bBacktrack);
		rageBotData.flDamage = refRecord.flDamage;
		rageBotData.iHitbox = refRecord.iHitbox;
		rageBotData.iHitGroup = refRecord.iHitgroup;

		return refRecord.vecPoint;
	}

	return Vector(0, 0, 0);
}

bool CRageBot::Hitchance( CBaseEntity* pEnt, CBaseCombatWeapon* pWeapon, Vector vecFrom, int iChance, Vector vecEyePosition) {

	if (!pEnt)
		return false;

	float flFinalHitchance = 0;
	CCSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData( );

	if ( !pWeaponInfo )
		return false;

	//if ( exploits::bIsShiftingTicks || ( cfg::rage::doubletap && IPT::HandleInput( cfg::rage::doubletapkey ) && i::GlobalVars->flCurrentTime - pWeapon->GetLastShotTime( ) <= TICKS_TO_TIME( 15 ) ) )
	//	return true;

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar(XorStr("weapon_accuracy_nospread"));
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

		Trace_t Trace{};
		i::EngineTrace->ClipRayToEntity( Ray_t( vecEyePosition, vecEnd ), MASK_SHOT | CONTENTS_GRATE, pEnt, &Trace );

		if (!&Trace)
			continue;

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

	if ((pCmd->iButtons & IN_JUMP || !(g::pLocal->GetFlags() & FL_ONGROUND)) && !ConfigAutoStopInAir(pWeapon))
		return;

	if (pWeapon->GetAmmo() == 0)
		return;

	// server is currently in nospread, no need to autostop
	if (i::ConVar->FindVar(XorStr("weapon_accuracy_nospread"))->GetInt() >= 1)
		return;

	if ( rageBotData.bCanShoot && !ConfigAutoStopBetweenShots(pWeapon))
		return;

	int iAggressiveness = ConfigAutoStopAggressiveness(pWeapon);

	float flMultiplier = 0.28f;
	switch (iAggressiveness) {
	case 1: flMultiplier = 0.26f;
		break;
	case 2: flMultiplier = 0.24f;
		break;
	case 3: flMultiplier = 0.20f;
		break;
	}

	float flIdealSpeed = (flMultiplier) * (pLocal->IsScoped( ) ? pWeapon->GetCSWpnData( )->flMaxSpeed[ 1 ] : pWeapon->GetCSWpnData( )->flMaxSpeed[ 0 ] );

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

	static auto cl_forwardspeed = i::ConVar->FindVar(XorStr("cl_forwardspeed"));
	static auto cl_sidespeed = i::ConVar->FindVar(XorStr("cl_sidespeed"));

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
		return std::make_pair( cfg::rage::iHeadPoints[0], cfg::rage::iBodyPoints[0]);
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return std::make_pair( cfg::rage::iHeadPoints[1], cfg::rage::iBodyPoints[1]);
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return std::make_pair( cfg::rage::iHeadPoints[2], cfg::rage::iBodyPoints[2]);
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return std::make_pair( cfg::rage::iHeadPoints[3], cfg::rage::iBodyPoints[3]);
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return std::make_pair( cfg::rage::iHeadPoints[4], cfg::rage::iBodyPoints[4]);
	}
	else {
		return std::make_pair( cfg::rage::iHeadPoints[5], cfg::rage::iBodyPoints[5]);
	}
}

int CRageBot::ConfigMinimumDamage( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::iMinDamages[0];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::iMinDamages[1];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::iMinDamages[2];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::iMinDamages[3];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::iMinDamages[4];
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 110;
	}
	else {
		return cfg::rage::iMinDamages[5];
	}
}

int CRageBot::ConfigOverrideDamage( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::iOverride[0];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::iOverride[1];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::iOverride[2];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::iOverride[3];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::iOverride[4];
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 110;
	}
	else {
		return cfg::rage::iOverride[5];
	}
}

int CRageBot::ConfigHitChance( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::iHitchances[0];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::iHitchances[1];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::iHitchances[2];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::iHitchances[3];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::iHitchances[4];
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		return 72;
	}
	else {
		return cfg::rage::iHitchances[5];
	}
}

bool CRageBot::ConfigForceSafe( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::bForceSafePoint[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::bForceSafePoint[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::bForceSafePoint[ 2 ];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::bForceSafePoint[ 4 ];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::bForceSafePoint[ 3 ];
	}
	else {
		return cfg::rage::bForceSafePoint[ 5 ];
	}
}

bool CRageBot::ConfigAutoScope( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::bAutoScope[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::bAutoScope[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::bAutoScope[ 2 ];
	}
	else {
		return false;
	}
}

bool CRageBot::ConfigAutoStop( CBaseCombatWeapon* pWeapon ) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex( );

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		return cfg::rage::bAutostop[ 0 ];
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		return cfg::rage::bAutostop[ 1 ];
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		return cfg::rage::bAutostop[ 2 ];
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		return cfg::rage::bAutostop[ 4 ];
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		return cfg::rage::bAutostop[ 3 ];
	}
	else {
		return cfg::rage::bAutostop[ 5 ];
	}
}

void CRageBot::AddHitbox(int index, std::array<bool, HITBOX_MAX>& vecHitboxList) {

	// server only allows headshots, so let's only push_back head and return!
	if (i::ConVar->FindVar(XorStr("mp_damage_headshot_only"))->GetBool()) {
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

	if (cfg::rage::bForceBaim && IPT::HandleInput(cfg::rage::iForceBaimKey)) {
		AddHitbox(2, vecHitboxes);
		AddHitbox(3, vecHitboxes);
		return vecHitboxes;
	}

	if ( iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[0][ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[1][ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[2][ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[3][ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[4][ i ] )
				AddHitbox( i, vecHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, vecHitboxes );
		AddHitbox( 3, vecHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bHitboxes[5][ i ] )
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
			if ( cfg::rage::bMultiHitboxes[0][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bMultiHitboxes[1][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bMultiHitboxes[2][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bMultiHitboxes[3][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bMultiHitboxes[4][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, arrHitboxes );
		AddHitbox( 3, arrHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bMultiHitboxes[5][ i ] )
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
			if ( cfg::rage::bSafeHitboxes[0][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_SSG08 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bSafeHitboxes[1][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_AWP ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bSafeHitboxes[2][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bSafeHitboxes[3][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9 ) {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bSafeHitboxes[4][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}
	else if ( iDefinitionIndex == WEAPON_TASER ) {
		AddHitbox( 2, arrHitboxes );
		AddHitbox( 3, arrHitboxes );
	}
	else {
		for ( int i = 0; i < 6; i++ ) {
			if ( cfg::rage::bSafeHitboxes[5][ i ] )
				AddHitbox( i, arrHitboxes );
		}
	}

	return arrHitboxes;
}

bool CRageBot::CheckShootingCondition( CUserCmd * pCmd, CBaseEntity * pLocal, CBaseCombatWeapon* pWeapon ) {

	if ( !pLocal || !pWeapon)
		return false;

	float flServerTime = TICKS_TO_TIME( networking.GetCorrectedTickbase() );
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

Vector g_CapsuleVertices[] =
{
	{ -0.01f, -0.01f, 1.00f },
	{ 0.51f, 0.00f, 0.86f },
	{ 0.44f, 0.25f, 0.86f },
	{ 0.25f, 0.44f, 0.86f },
	{ -0.01f, 0.51f, 0.86f },
	{ -0.26f, 0.44f, 0.86f },
	{ -0.45f, 0.25f, 0.86f },
	{ -0.51f, 0.00f, 0.86f },
	{ -0.45f, -0.26f, 0.86f },
	{ -0.26f, -0.45f, 0.86f },
	{ -0.01f, -0.51f, 0.86f },
	{ 0.25f, -0.45f, 0.86f },
	{ 0.44f, -0.26f, 0.86f },
	{ 0.86f, 0.00f, 0.51f },
	{ 0.75f, 0.43f, 0.51f },
	{ 0.43f, 0.75f, 0.51f },
	{ -0.01f, 0.86f, 0.51f },
	{ -0.44f, 0.75f, 0.51f },
	{ -0.76f, 0.43f, 0.51f },
	{ -0.87f, 0.00f, 0.51f },
	{ -0.76f, -0.44f, 0.51f },
	{ -0.44f, -0.76f, 0.51f },
	{ -0.01f, -0.87f, 0.51f },
	{ 0.43f, -0.76f, 0.51f },
	{ 0.75f, -0.44f, 0.51f },
	{ 1.00f, 0.00f, 0.01f },
	{ 0.86f, 0.50f, 0.01f },
	{ 0.49f, 0.86f, 0.01f },
	{ -0.01f, 1.00f, 0.01f },
	{ -0.51f, 0.86f, 0.01f },
	{ -0.87f, 0.50f, 0.01f },
	{ -1.00f, 0.00f, 0.01f },
	{ -0.87f, -0.50f, 0.01f },
	{ -0.51f, -0.87f, 0.01f },
	{ -0.01f, -1.00f, 0.01f },
	{ 0.49f, -0.87f, 0.01f },
	{ 0.86f, -0.51f, 0.01f },
	{ 1.00f, 0.00f, -0.02f },
	{ 0.86f, 0.50f, -0.02f },
	{ 0.49f, 0.86f, -0.02f },
	{ -0.01f, 1.00f, -0.02f },
	{ -0.51f, 0.86f, -0.02f },
	{ -0.87f, 0.50f, -0.02f },
	{ -1.00f, 0.00f, -0.02f },
	{ -0.87f, -0.50f, -0.02f },
	{ -0.51f, -0.87f, -0.02f },
	{ -0.01f, -1.00f, -0.02f },
	{ 0.49f, -0.87f, -0.02f },
	{ 0.86f, -0.51f, -0.02f },
	{ 0.86f, 0.00f, -0.51f },
	{ 0.75f, 0.43f, -0.51f },
	{ 0.43f, 0.75f, -0.51f },
	{ -0.01f, 0.86f, -0.51f },
	{ -0.44f, 0.75f, -0.51f },
	{ -0.76f, 0.43f, -0.51f },
	{ -0.87f, 0.00f, -0.51f },
	{ -0.76f, -0.44f, -0.51f },
	{ -0.44f, -0.76f, -0.51f },
	{ -0.01f, -0.87f, -0.51f },
	{ 0.43f, -0.76f, -0.51f },
	{ 0.75f, -0.44f, -0.51f },
	{ 0.51f, 0.00f, -0.87f },
	{ 0.44f, 0.25f, -0.87f },
	{ 0.25f, 0.44f, -0.87f },
	{ -0.01f, 0.51f, -0.87f },
	{ -0.26f, 0.44f, -0.87f },
	{ -0.45f, 0.25f, -0.87f },
	{ -0.51f, 0.00f, -0.87f },
	{ -0.45f, -0.26f, -0.87f },
	{ -0.26f, -0.45f, -0.87f },
	{ -0.01f, -0.51f, -0.87f },
	{ 0.25f, -0.45f, -0.87f },
	{ 0.44f, -0.26f, -0.87f },
	{ 0.00f, 0.00f, -1.00f },
};

std::vector<Vector> CRageBot::CreatePoints(Vector vecEyePosition, CBaseCombatWeapon* pWeapon, Lagcompensation::LagRecord_t* pRecord, int iHitbox, EMatrixType iType) {

	std::vector<Vector> refVecPoints{};
	std::array<bool, HITBOX_MAX> vecSelectedMultipoint = ConfigMultiHitboxes(pWeapon);
	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);
	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;

	float flRadius = 0.f;
	mstudiobbox_t* refStudioBox = nullptr;
	Vector vecCenter = pRecord->pEntity->GetHitboxPosition(iHitbox, pRecord->pMatricies[iType], flRadius);
	if (const Model_t* pModel = pRecord->pEntity->GetModel(); pModel)
		if (studiohdr_t* pStudioHdr = i::ModelInfo->GetStudioModel(pModel); pStudioHdr)
			refStudioBox = pStudioHdr->GetHitboxSet(0)->GetHitbox(iHitbox);
	
	if (!refStudioBox || flRadius < 0) {
		refVecPoints.push_back(vecCenter);
		return refVecPoints;
	}

	matrix3x4_t& refMatrixBone = pRecord->pMatricies[iType][refStudioBox->iBone];

	if (!vecSelectedMultipoint[iHitbox]) {
		refVecPoints.push_back(vecCenter);
		return refVecPoints;
	}

	float flHitboxDistance = flRadius * ((iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints) * 0.01f);

	Vector vecCurrentAngles;
	M::VectorAngles(vecCenter - vecEyePosition, vecCurrentAngles);
	Vector vecForward; M::AngleVectors(vecCurrentAngles, &vecForward);

	const Vector vecRight = vecForward.CrossProduct(Vector(0, 0, 1));
	const Vector vecLeft = Vector(-vecRight.x, -vecRight.y, vecRight.z);
	const Vector vecTop = Vector(0, 0, 1);

	bool bGenerateMore = (iHitbox == HITBOX_HEAD ? *pHeadPoints > 50.f ? true : false : *pBodyPoints > 50.f ? true : false);

	refVecPoints.push_back(vecCenter);
	if (iHitbox == HITBOX_HEAD) {
		refVecPoints.push_back(vecCenter + vecTop * flHitboxDistance);
		refVecPoints.push_back(vecCenter - vecTop * flHitboxDistance);

		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) + (vecLeft * (flHitboxDistance * 0.5f)));
		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) + (vecRight * (flHitboxDistance * 0.5f)));
		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) - (vecLeft * (flHitboxDistance * 0.5f)));
		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) - (vecRight * (flHitboxDistance * 0.5f)));

		/*if (bGenerateMore) {

			refVecPoints.push_back(vecCenter + vecTop * (flHitboxDistance * 0.5f));
			refVecPoints.push_back(vecCenter - vecTop * (flHitboxDistance * 0.5f));

			refVecPoints.push_back(vecCenter + (vecTop * (flHitboxDistance * 0.5f)) + (vecLeft * ((flHitboxDistance * 0.25f))));
			refVecPoints.push_back(vecCenter + (vecTop * (flHitboxDistance * 0.5f)) + (vecRight * ((flHitboxDistance * 0.25f))));
			refVecPoints.push_back(vecCenter + (vecTop * (flHitboxDistance * 0.5f)) - (vecLeft * ((flHitboxDistance * 0.25f))));
			refVecPoints.push_back(vecCenter + (vecTop * (flHitboxDistance * 0.5f)) - (vecRight * ((flHitboxDistance * 0.25f))));
		}*/
	}
	refVecPoints.push_back(vecCenter + vecLeft * flHitboxDistance);
	refVecPoints.push_back(vecCenter + vecRight * flHitboxDistance);

	/*if (bGenerateMore) {

		refVecPoints.push_back(vecCenter + vecLeft * (flHitboxDistance * 0.5f));
		refVecPoints.push_back(vecCenter + vecRight * (flHitboxDistance * 0.5f));
	}*/

	//for (auto& something : refVecPoints)
	//	g::drawList.push_back(something);

	return refVecPoints;
}

std::vector<Vector> CRageBot::CreatePoints( CBaseEntity * pTarget, Lagcompensation::LagRecord_t* pRecord, CBaseCombatWeapon * pWeapon, int iHitbox, bool bBuildSideOnly) {

	static int iMultiOptimization[HITBOX_MAX];
	static std::vector<Vector> output{};
	output.clear();

	std::pair<int, int> multiPoints = ConfigMultipoint(pWeapon);

	float flRadius = 0.f;
	Vector vecAngle = pRecord->pEntity->GetHitboxPosition(iHitbox, pRecord->pMatricies[RESOLVE], flRadius);
	if (flRadius <= 0) {
		output.push_back(vecAngle);
		return output;
	}

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;
	float flHitboxDistance = flRadius * ( (iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints ) * 0.01f );

	if (!(pTarget->GetFlags() & FL_ONGROUND) && iHitbox == HITBOX_HEAD)
		flHitboxDistance = 0.7f;

	bool bGenerateMore = (iHitbox == HITBOX_HEAD ? *pHeadPoints > 50.f ? true : false : *pBodyPoints > 50.f ? true : false);

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

	if (!pHitbox)
		return false;

	Ray_t Ray(vecStart, vecEnd);

	Trace_t Trace;
	Trace.flFraction = 1.0f;
	Trace.bStartSolid = false;
	// original: 55 8B EC 83 E4 F8 F3 0F 10 42
	// kittenpopo: 55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F
	typedef int(__fastcall* ClipRayToHitbox_t)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, Trace_t&);
	static auto sig = (void*)((DWORD)(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F"))));
	int iHit = ((ClipRayToHitbox_t)(sig))(Ray, pHitbox, aMatrix[pHitbox->iBone], Trace);
	return iHit >= 0;
}
#pragma runtime_checks( "", restore )

int CRageBot::SafePoint( Vector & vecEyePosition, CBaseCombatWeapon * pWeapon, Lagcompensation::LagRecord_t * pRecord, Vector vecShootposition, int iHitbox, int iMustIntersect) {

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

	mstudiobbox_t* studioBox = pRecord->pEntity->StudioHitbox(iHitbox);
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

	//if (IPT::HandleInput(cfg::rage::doubletapkey) && cfg::rage::doubletap && exploits::bCharged)
	//	return g::pCmd->iTickCount;

	return lagcomp.FixTickCount(flSimulationTime);

	// calculate lerp remainder.
	float flLerpRemainder = std::fmodf( lagcomp.GetClientInterpAmount( ), i::GlobalVars->flIntervalPerTick );

	// get real simulation time and calculate interp fraction.
	if ( flLerpRemainder > 0.f )
		flSimulationTime += i::GlobalVars->flIntervalPerTick - flLerpRemainder;

	return TIME_TO_TICKS(flSimulationTime);
}

bool CRageBot::ShouldSendPacket(bool& bSendPacket) {

	if (cfg::antiaim::bFakeDuck && IPT::HandleInput(cfg::antiaim::iFakeDuckKey))
		return bSendPacket;

	if (cfg::rage::bDoubletap && IPT::HandleInput(cfg::rage::iDoubletapKey) && exploits::iTicksToStore != 0)
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
		return cfg::rage::bConditions[0][1];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::bConditions[1][1];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::bConditions[2][1];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::bConditions[4][1];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::bConditions[3][1];
	}
	else {
		return cfg::rage::bConditions[5][1];
	}
}

bool CRageBot::ConfigAutoStopBetweenShots(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::bConditions[0][0];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::bConditions[1][0];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::bConditions[2][0];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::bConditions[4][0];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::bConditions[3][0];
	}
	else {
		return cfg::rage::bConditions[5][0];
	}
}

int CRageBot::ConfigAutoStopAggressiveness(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::bAutostopAggressiveness[0];
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::bAutostopAggressiveness[1];
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::bAutostopAggressiveness[2];
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::bAutostopAggressiveness[4];
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::bAutostopAggressiveness[3];
	}
	else {
		return cfg::rage::bAutostopAggressiveness[5];
	}
}

void CRageBot::CapsuleRebuild(Lagcompensation::LagRecord_t* pRecord, int iHitbox) {

	static float g_capsuleVertPositions[74][3] = {
	{ -0.01, -0.01, 1.0 },	{ 0.51, 0.0, 0.86 },	{ 0.44, 0.25, 0.86 },	{ 0.25, 0.44, 0.86 },	{ -0.01, 0.51, 0.86 },	{ -0.26, 0.44, 0.86 },	{ -0.45, 0.25, 0.86 },	{ -0.51, 0.0, 0.86 },	{ -0.45, -0.26, 0.86 },
	{ -0.26, -0.45, 0.86 },	{ -0.01, -0.51, 0.86 },	{ 0.25, -0.45, 0.86 },	{ 0.44, -0.26, 0.86 },	{ 0.86, 0.0, 0.51 },	{ 0.75, 0.43, 0.51 },	{ 0.43, 0.75, 0.51 },	{ -0.01, 0.86, 0.51 },	{ -0.44, 0.75, 0.51 },
	{ -0.76, 0.43, 0.51 },	{ -0.87, 0.0, 0.51 },	{ -0.76, -0.44, 0.51 },	{ -0.44, -0.76, 0.51 },	{ -0.01, -0.87, 0.51 },	{ 0.43, -0.76, 0.51 },	{ 0.75, -0.44, 0.51 },	{ 1.0, 0.0, 0.01 },		{ 0.86, 0.5, 0.01 },
	{ 0.49, 0.86, 0.01 },	{ -0.01, 1.0, 0.01 },	{ -0.51, 0.86, 0.01 },	{ -0.87, 0.5, 0.01 },	{ -1.0, 0.0, 0.01 },	{ -0.87, -0.5, 0.01 },	{ -0.51, -0.87, 0.01 },	{ -0.01, -1.0, 0.01 },	{ 0.49, -0.87, 0.01 },
	{ 0.86, -0.51, 0.01 },	{ 1.0, 0.0, -0.02 },	{ 0.86, 0.5, -0.02 },	{ 0.49, 0.86, -0.02 },	{ -0.01, 1.0, -0.02 },	{ -0.51, 0.86, -0.02 },	{ -0.87, 0.5, -0.02 },	{ -1.0, 0.0, -0.02 },	{ -0.87, -0.5, -0.02 },
	{ -0.51, -0.87, -0.02 },{ -0.01, -1.0, -0.02 },	{ 0.49, -0.87, -0.02 },	{ 0.86, -0.51, -0.02 },	{ 0.86, 0.0, -0.51 },	{ 0.75, 0.43, -0.51 },	{ 0.43, 0.75, -0.51 },	{ -0.01, 0.86, -0.51 },	{ -0.44, 0.75, -0.51 },
	{ -0.76, 0.43, -0.51 },	{ -0.87, 0.0, -0.51 },	{ -0.76, -0.44, -0.51 },{ -0.44, -0.76, -0.51 },{ -0.01, -0.87, -0.51 },{ 0.43, -0.76, -0.51 },	{ 0.75, -0.44, -0.51 },	{ 0.51, 0.0, -0.87 },	{ 0.44, 0.25, -0.87 },
	{ 0.25, 0.44, -0.87 },	{ -0.01, 0.51, -0.87 },	{ -0.26, 0.44, -0.87 },	{ -0.45, 0.25, -0.87 },	{ -0.51, 0.0, -0.87 },	{ -0.45, -0.26, -0.87 },{ -0.26, -0.45, -0.87 },{ -0.01, -0.51, -0.87 },{ 0.25, -0.45, -0.87 },
	{ 0.44, -0.26, -0.87 },	{ 0.0, 0.0, -1.0 },
	};

	static int g_capsuleLineIndices[117] = { -1,
	14,		0,	4,	16,	28,	40,	52,	64,	73,	70,	58,	46,	34,	22,	10,		-1,
	14,		0,	1,	13,	25,	37,	49,	61,	73,	67,	55,	43,	31,	19,	7,		-1,
	12,		61,	62,	63,	64,	65,	66,	67,	68,	69,	70,	71,	72,				-1,
	12,		49,	50,	51,	52,	53,	54,	55,	56,	57,	58,	59,	60,				-1,
	12,		37,	38,	39,	40,	41,	42,	43,	44,	45,	46,	47,	48,				-1,
	12,		25,	26,	27,	28,	29,	30,	31,	32,	33,	34,	35,	36,				-1,
	12,		13,	14,	15,	16,	17,	18,	19,	20,	21,	22,	23,	24,				-1,
	12,		1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,				-1
	};

	static auto VectorNormalize = [&](Vector& v1) -> void {

		v1 = v1.VectorNormalize();
	};

	static auto CrossProduct = [&](const float* v1, const float* v2, float* cross) -> void {

		cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
		cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
		cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
	};

	static auto VectorVectors = [&](const Vector& forward, Vector& right, Vector& up) -> void {

		Vector tmp;

		if (forward[0] == 0 && forward[1] == 0)
		{
			// pitch 90 degrees up/down from identity
			right[0] = 0;
			right[1] = -1;
			right[2] = 0;
			up[0] = -forward[2];
			up[1] = 0;
			up[2] = 0;
		}
		else
		{
			tmp[0] = 0; tmp[1] = 0; tmp[2] = 1.0;
			CrossProduct(forward.data(), tmp.data(), right.data());
			right.VectorNormalize();
			VectorNormalize(right);
			CrossProduct(right.data(), forward.data(), up.data());
			VectorNormalize(up);
		}
	};

	static auto MatrixSetColumn = [&](const Vector& in, int column, matrix3x4_t& out) -> void {

		out[0][column] = in.x;
		out[1][column] = in.y;
		out[2][column] = in.z;
	};

	static auto VectorMatrix = [&](const Vector& forward, matrix3x4_t& matrix) -> void {

		Vector right, up;
		VectorVectors(forward, right, up);

		MatrixSetColumn(forward, 0, matrix);
		MatrixSetColumn((right * -1), 1, matrix);
		MatrixSetColumn(up, 2, matrix);
	};

	static auto VectorRotate = [&](const float* in1, const matrix3x4_t& in2, float* out) -> void {

		out[0] = M::DotProduct(in1, in2[0]);
		out[1] = M::DotProduct(in1, in2[1]);
		out[2] = M::DotProduct(in1, in2[2]);
	};

	const auto pModel = pRecord->pEntity->GetModel();
	if (!pModel)
		return;

	studiohdr_t* pStudioModel = i::ModelInfo->GetStudioModel(pModel);
	if (!pStudioModel)
		return;

	mstudiobbox_t* pHitbox = pStudioModel->GetHitbox(iHitbox, 0);
	if (!pHitbox)
		return;

	Vector vStart, vEnd;
	vStart = M::VectorTransform(pHitbox->vecBBMin, pRecord->pMatricies[RESOLVE][pHitbox->iBone]);
	vEnd = M::VectorTransform(pHitbox->vecBBMax, pRecord->pMatricies[RESOLVE][pHitbox->iBone]);
	const float flRadius = pHitbox->flRadius;

	Vector vecCapsuleCoreNormal = (vStart - vEnd).Normalized();

	Vector vecCurrentAngles;
	M::VectorAngles((vStart + vEnd) * 0.5f, vecCurrentAngles);
	Vector vecForward; M::AngleVectors(vecCurrentAngles, &vecForward);

	matrix3x4_t matCapsuleRotationSpace;
	VectorMatrix(Vector(0, 0, 1), matCapsuleRotationSpace);

	matrix3x4_t matCapsuleSpace;
	VectorMatrix(vecCapsuleCoreNormal, matCapsuleSpace);

	Vector v[74];
	Vector vecLen = (vEnd - vStart);

	for (int i = 0; i < 74; i++) {

		Vector vecCapsuleVert = g_capsuleVertPositions[i];

		VectorRotate(vecCapsuleVert.data(), matCapsuleRotationSpace, vecCapsuleVert.data());
		VectorRotate(vecCapsuleVert.data(), matCapsuleSpace, vecCapsuleVert.data());

		vecCapsuleVert *= flRadius;

		if (g_capsuleVertPositions[i][2] > 0) {

			vecCapsuleVert += vecLen;
		}

		v[i] = vecCapsuleVert + vStart;
		g::drawList.push_back(v[i]);
	}
}