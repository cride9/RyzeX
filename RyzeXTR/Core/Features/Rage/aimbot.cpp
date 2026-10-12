#include "aimbot.h"
#include "Animations/EnemyAnimations.h"
#include "Animations/ExperimentalResolver.h"
#include "../../Lua/Lua.h"
#include "../../Interface/Classes/CCSGameRulesProxy.h"
#include "StudioHitboxGeometry.h"
#include "RecordTraceScope.h"

void CAimBot::CreateMove( CUserCmd*  pCmd, CBaseEntity*  pLocal ) {

	static CConVar* recoilScale = i::ConVar->FindVar( XorStr( "weapon_recoil_scale" ) );

	if ( !cfg::rage::bEnable || ( !IPT::HandleInput( cfg::rage::iAimbotKey ) && cfg::rage::iAimbotKey != 0 ) )
		return ResetAimbotData( true );

	if ( ( *GameRules )->m_bFreezePeriod( ) )
		return ResetAimbotData( true );

	/* Run R8 Revolver cock */
	misc::RevolverCreateMove( );

	/* Create new data for aimbot */
	aimData = rageBotData_t( );
	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );

	if ( pWeapon == nullptr )
		return ResetAimbotData( );

	if ( pWeapon->IsGrenade( ) || pWeapon->IsKnife( ) || !pWeapon->IsWeapon( ) )
		return ResetAimbotData( true );

	if ( !pWeapon->GetAmmo( ) )
		return ResetAimbotData( );

	/* Get currently equipped weapon config */
	curConfig = GetWeaponConfiguration( pWeapon->GetItemDefinitionIndex( ) );
	curConfig.pWeapon = pWeapon;
	exploits::bDisableDefensive = false;

	/* Make a vector out of every valid & tragetable entity */
	std::vector<Lagcompensation::AnimationInfo_t*> vecTargets = GetTargetableEntities( pLocal );

	if ( vecTargets.empty( ) )
		return ResetAimbotData( true );

	/* Use the current shoot position prepared by local animations */
	vecEyePosition = localAnim->GetShootPosition( );

	/* Scan stored entities & safepoint */
	Vector vecShootPosition = ScanHitboxes( vecTargets, pLocal );

	/* Run predicted autostop */
	bool bSkipPostStop = false/*AutoStop(vecTargets, pLocal, pCmd, false)*/;

	if ( aimData.pRecord == nullptr )
		return ResetAimbotData( true );

	/* Prepare doubletap if needed */
	if ( cfg::rage::bDoubletap && IPT::HandleInput( cfg::rage::iDoubletapKey ) )
		exploits::bDisableDefensive = true;

	/* Apply the selected record to the desired entity */
	aimData.pRecord->ApplyMatrix( aimData.pAimbotTarget, RESOLVE );

	/* Tell exploits that dont recharge */
	exploits::bCanCharge = false;

	/* Auto zoom & m_bResumeZoom check */
	if ( curConfig.bAutoScope && !pLocal->IsScoped( ) && !pLocal->IsResumingScope( ) )
		pCmd->iButtons |= IN_ZOOM;

	/* Calculate the aim angle with vectorAngles */
	Vector vecAimAngle = ( M::VectorAngles( vecShootPosition - vecEyePosition ) -= ( pLocal->GetAimPunch( ) * recoilScale->GetFloat( ) ) );

	/* Calculate current hitchance & accuracy boost */
	aimData.bFireReady = pLocal->CanShoot(pWeapon, exploits::GetShotTickbase(pCmd->iCommandNumber, pLocal->GetTickBase()));
	aimData.bCanShoot = HitChance( pCmd, pLocal, vecShootPosition, vecAimAngle, aimData.pRecord );

	/* Run normal autostop */
	if ( !bSkipPostStop )
		AutoStop( vecTargets, pLocal, pCmd, true );

	/* Wait til we can shoot */
	if ( !aimData.bCanShoot || !aimData.bFireReady )
		return ResetAimbotData( );

	/* Shoot entity when we can & have enough hitchance */
	pCmd->angViewPoint = vecAimAngle;
	if ( !cfg::rage::bSilentAim ) i::EngineClient->SetViewAngles( vecAimAngle );

	lagcomp.GetLog( aimData.pRecord->iEntIndex ).iShotAmount++;
	pCmd->iButtons |= IN_ATTACK;
	aimData.iTickcount = pCmd->iTickCount;
	aimData.iDesiredTickcount = TIME_TO_TICKS( aimData.flTargetSimulation + lagcomp.GetClientInterpAmount( ) );

	/* Sending packet in prediction will cause hit registration delay ( only in CHLClient::CreateMove() ) */
	bShouldSendPacket = true;
	hitlogData = aimData;
	experimentalResolver.RecordShot(aimData, pCmd->iCommandNumber);
}

void CAimBot::ResetAimbotData( bool bShouldRecharge ) {

	aimData.ClearTarget( );
	if ( bShouldRecharge ) {

		exploits::bCanCharge = bShouldRecharge;
	}
}

bool CAimBot::PostPrediction( CUserCmd* pCmd, bool&  bSendPacket ) {

	if ( !( pCmd->iButtons & IN_ATTACK ) )
		return false;

	if ( aimData.iDesiredTickcount != -1 )
		pCmd->iTickCount = aimData.iDesiredTickcount;

	if ( cfg::antiaim::bFakeDuck && IPT::HandleInput( cfg::antiaim::iFakeDuckKey ) )
		bShouldSendPacket = false;

	if ( bShouldSendPacket )
		bSendPacket = true;

	aimData.iTickcount = pCmd->iTickCount;
	aimData.iDesiredTickcount = -1;
	return bShouldSendPacket = false;
}

CAimBot::SCAN CAimBot::ShouldSkipRecord( Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pFirstRecord ) {

	if ( pRecord->bBreakingLagcompensation )
		return SCAN_BREAK;

	if ( cfg::rage::iAimbotFov < 180 ) {

		float flRadius = 0.f;
		Vector vecDistanceBetween = ( g::vecOriginalViewAngle - M::VectorAngles( pRecord->vecOrigin - vecEyePosition ) );

		if ( fabsf( ( vecDistanceBetween ).Normalize( ).Length2D( ) ) > cfg::rage::iAimbotFov )
			return SCAN_SKIP;
	}

	return SCAN_CONTINUE;
}

bool CAimBot::GetIntersectionPoint(Lagcompensation::LagRecord_t* record, int hitboxIndex, Vector& point) {
	if (!record || !record->pEntity || !record->bAnimationMatrixValid || !record->bSafeMatrixValid
		|| hitboxIndex < 0 || hitboxIndex >= HITBOX_MAX) return false;
	auto& cached = record->intersectionPoints[hitboxIndex];
	if (!cached.computed) {
		cached.computed = true;
		if (hitboxIndex == HITBOX_HEAD && record->iResolverMode == static_cast<int>(resolver::Mode::Experimental)) {
			if (record->bHasHeadOverlap && record->iHeadHypothesisCount == static_cast<int>(hitbox::MaxShapes)) {
				cached.point = hitbox::FromVector(record->vecRageSafePoint);
				cached.valid = hitbox::ContainsAll(record->headHypotheses.data(), record->headHypotheses.size(), cached.point, .03);
			}
		} else {
			const auto box = record->pEntity->StudioHitbox(hitboxIndex);
			std::array<hitbox::Shape, 4> shapes{};
			int index = 0;
			for (const auto type : { LEFT, RIGHT, CENTER, RESOLVE })
				shapes[index++] = hitbox::FromStudio(box, record->pMatricies[type]);
			hitbox::Vec common;
			if (hitbox::FindIntersection(shapes.data(), shapes.size(), shapes.back().Center(), common)) {
				// Certify again after narrowing to the game's float coordinates.
				cached.point = hitbox::FromVector(hitbox::ToVector(common));
				cached.valid = hitbox::ContainsAll(shapes.data(), shapes.size(), cached.point, .03);
			}
		}
	}
	if (!cached.valid) return false;
	point = hitbox::ToVector(cached.point);
	return point.IsValid();
}

Vector CAimBot::ScanHitboxes(std::vector<Lagcompensation::AnimationInfo_t*>& vecIn, CBaseEntity* pLocal) {
	scanStatistics = {};
	if (!pLocal || !curConfig.pWeapon || !vecEyePosition.IsValid()) return {};
	static CConVar* lagcompensation = i::ConVar->FindVar("cl_lagcompensation");
	struct Job {
		Lagcompensation::LagRecord_t* record;
		int age;
		float minimum, upper;
		int memo = -1;
	};
	struct RayResult {
		std::array<float, 3> direction;
		float damage;
		FireBulletData_t data{};
		RayResult(const std::array<float, 3>& key, float value, const FireBulletData_t& bullet) : direction(key), damage(value) { data = bullet; }
		RayResult(const RayResult& other) : direction(other.direction), damage(other.damage) { data = other.data; }
	};
	struct GeometryResults {
		Lagcompensation::LagRecord_t* record;
		int count;
		std::vector<RayResult> rays;
	};
	std::vector<Job> jobs;
	std::vector<GeometryResults> memo;
	jobs.reserve(vecIn.size() * 16);
	for (auto log : vecIn) {
		if (!log || !log->pEntity || log->pRecord.empty()) continue;
		log->iLastValid = std::clamp(log->iLastValid, 0, static_cast<int>(log->pRecord.size()) - 1);
		if (lagcompensation && lagcompensation->GetInt() == 0) log->iLastValid = 0;
		const float minimum = curConfig.iMinimumDamage > 100
			? (std::max)(log->pEntity->GetHealth(), 4) + curConfig.iMinimumDamage - 100 : curConfig.iMinimumDamage;
		for (int age = 0; age <= log->iLastValid; ++age) {
			if (cfg::rage::bLimitScan && (age & 1)) continue;
			auto record = &log->pRecord[age];
			const auto skip = ShouldSkipRecord(record, &log->pRecord.front());
			if (skip == SCAN_BREAK) break;
			if (skip == SCAN_SKIP || !record->bValid || !record->bAnimationMatrixValid || record->pEntity != log->pEntity) continue;
			const float upper = autowall.GetDamageUpperBound(vecEyePosition, curConfig.pWeapon, record);
			if (std::floor(upper) < minimum) { ++scanStatistics.boundSkips; continue; }
			jobs.push_back({record, age, minimum, upper});
		}
	}
	// Explore the records with the best possible result first. A later query
	// is skipped only when its optimistic ranking cannot improve the best hit.
	std::stable_sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) { return a.upper > b.upper; });
	Hitscan_t best;
	bool found = false;
	auto canImprove = [&](const Job& job, int hitboxIndex, bool headOverlap) {
		if (!found) return true;
		Hitscan_t optimistic;
		optimistic.flDamage = std::floor(job.upper);
		optimistic.iTransformedDamage = static_cast<int>(job.minimum);
		optimistic.bHeadOverlap = headOverlap;
		optimistic.bSafe = true;
		optimistic.bBaimLethal = hitboxIndex != HITBOX_HEAD && job.upper > job.record->pEntity->GetHealth();
		return optimistic < best;
	};
	auto damage = [&](Job& job, Vector point, FireBulletData_t& data) {
		const Vector direction = (point - vecEyePosition).Normalized();
		if (!direction.IsValid() || direction.LengthSqr() <= 0.f) return -1.f;
		const std::array<float, 3> key{direction.x, direction.y, direction.z};
		const int count = job.record->pEntity->GetCachedBoneData().Count();
		if (count <= 0 || count > MAXSTUDIOBONES) return -1.f;
		if (job.memo < 0) {
			for (size_t index = 0; index < memo.size(); ++index) {
				const auto& group = memo[index];
				const auto a = group.record;
				const auto b = job.record;
				if (a->pEntity == b->pEntity && group.count == count && a->vecOrigin == b->vecOrigin
					&& a->vecMins == b->vecMins && a->vecMaxs == b->vecMaxs
					&& std::memcmp(a->pMatricies[RESOLVE], b->pMatricies[RESOLVE], count * sizeof(matrix3x4a_t)) == 0) {
					job.memo = static_cast<int>(index); break;
				}
			}
			if (job.memo < 0) {
				job.memo = static_cast<int>(memo.size());
				memo.push_back({job.record, count, {}});
				memo.back().rays.reserve(16);
			}
		}
		auto& rays = memo[job.memo].rays;
		for (const auto& result : rays) {
			if (std::memcmp(result.direction.data(), key.data(), sizeof(key)) == 0) {
				++scanStatistics.reusedRays; data = result.data; return result.damage;
			}
		}
		// The memo lives only inside this call. Every real trace restores the
		// target's live bones, so other targets cannot alter a cached ray's world.
		RecordTraceScope scope(job.record->pEntity);
		if (!scope.Ready()) return -1.f;
		job.record->ApplyMatrix(job.record->pEntity, RESOLVE);
		++scanStatistics.damageCalls;
		const float value = autowall.GetDamage(pLocal, vecEyePosition, point, curConfig.pWeapon, job.record, &data);
		rays.push_back({key, value, data});
		return value;
	};
	for (int phase = 0; phase < 2; ++phase) {
		for (auto& job : jobs) {
			auto record = job.record;
			const bool rejected = experimentalResolver.IsRejected(record);
			for (int hitboxIndex : curConfig.vecHitboxes[NORMAL]) {
				if (hitboxIndex < 0 || hitboxIndex >= HITBOX_MAX) continue;
				const bool experimentalHead = record->iResolverMode == static_cast<int>(resolver::Mode::Experimental) && hitboxIndex == HITBOX_HEAD;
				const bool headPotential = experimentalHead && record->bHasHeadOverlap;
				if (!canImprove(job, hitboxIndex, phase == 0 && headPotential)) { ++scanStatistics.boundSkips; continue; }
				Vector common;
				const bool hasCommon = GetIntersectionPoint(record, hitboxIndex, common);
				std::vector<Vector> points;
				if (phase == 0) {
					if (!hasCommon) continue;
					points.push_back(common);
				} else {
					const bool multipoint = std::find(curConfig.vecHitboxes[MULTIPOINT].begin(), curConfig.vecHitboxes[MULTIPOINT].end(), hitboxIndex) != curConfig.vecHitboxes[MULTIPOINT].end();
					points = CreatePoints(vecEyePosition, curConfig.pWeapon, record, hitboxIndex, RESOLVE, multipoint);
				}
				const bool playerSafe = playerList::arrPlayers[record->iEntIndex].bSafePoint;
				const bool forceSafe = std::find(curConfig.vecHitboxes[SAFE].begin(), curConfig.vecHitboxes[SAFE].end(), hitboxIndex) != curConfig.vecHitboxes[SAFE].end();
				for (auto point : points) {
					if (!point.IsValid() || (phase == 1 && hasCommon && point == common)) continue;
					// A failed common head point was already tested, including a
					// trace intercepted by a body hitbox. Never relabel it as head.
					if (phase == 1 && experimentalHead && record->bHasHeadOverlap && point == record->vecRageSafePoint) continue;
					const bool overlapHead = phase == 0 && experimentalHead;
					if (!canImprove(job, hitboxIndex, overlapHead)) { ++scanStatistics.boundSkips; continue; }
					int coverage = phase == 0 ? 3 : 0;
					if (phase == 1 && (curConfig.bSafePoint || playerSafe || forceSafe || experimentalHead || rejected))
						coverage = autowall.SafePoint(vecEyePosition, record, point, hitboxIndex);
					const bool safe = coverage == 3;
					if ((rejected || playerSafe || forceSafe) && !safe) continue;
					if (curConfig.bSafePoint && coverage < 2) continue;
					const Vector ray = (point - vecEyePosition).Normalized();
					Job rayJob = job;
					rayJob.upper = (std::min)(job.upper, autowall.GetDamageUpperBound(vecEyePosition, curConfig.pWeapon, record, &ray));
					if (std::floor(rayJob.upper) < job.minimum || !canImprove(rayJob, hitboxIndex, overlapHead)) {
						++scanStatistics.boundSkips; continue;
					}
					if (phase == 0) ++scanStatistics.intersectionPoints; else ++scanStatistics.fallbackPoints;
					FireBulletData_t data{};
					const float value = damage(job, point, data);
					if (!std::isfinite(value) || value < job.minimum) continue;
					if (overlapHead && data.enterTrace.iHitbox != HITBOX_HEAD) continue;
					Hitscan_t candidate(record, point, data, hitboxIndex != HITBOX_HEAD && value > record->pEntity->GetHealth(),
						safe, static_cast<int>(job.minimum), job.age, overlapHead);
					if (!found || candidate < best) { best = candidate; found = true; }
				}
			}
		}
	}
	if (!found) return {};
	aimData.SetTarget(best.pRecord, vecEyePosition, best.bBacktrack);
	aimData.bSafe = best.bSafe;
	aimData.flDamage = static_cast<int>(best.flDamage);
	aimData.iHitbox = best.iHitbox;
	aimData.iHitGroup = best.iHitgroup;
	aimData.vecTargetShootPosition = best.vecPoint;
	return best.vecPoint;
}

bool CAimBot::AutoStop( std::vector<Lagcompensation::AnimationInfo_t*>& vecIn, CBaseEntity*  pLocal, CUserCmd*  pCmd, bool bSkipCheck ) {

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar( XorStr( "weapon_accuracy_nospread" ) );
	if ( !curConfig.bAutostop )
		return false;

	if ( aimData.bCanShoot && !curConfig.bConditions[ CONDITION_BETWEEN_SHOTS ] )
		return false;

	if ( !aimData.bFireReady && !curConfig.bConditions[ CONDITION_BETWEEN_SHOTS ] )
		return false;

	if ( ( pCmd->iButtons & IN_JUMP || !( pLocal->GetFlags( ) & FL_ONGROUND ) ) && !curConfig.bConditions[ CONDITION_INAIR ] )
		return false;

	if ( weapon_accuracy_nospread->GetInt( ) == 1 )
		return false;

	if (exploits::iShiftAmount)
	{
		pCmd->iButtons &= ~(
			IN_FORWARD |
			IN_BACK |
			IN_MOVELEFT |
			IN_MOVERIGHT
			);

		return true;
	}

	if ( !bSkipCheck ) {

		for ( auto it : vecIn ) {

			if ( !it->pEntity )
				continue;

			float flTransformedDamage = curConfig.iMinimumDamage;
			if ( curConfig.iMinimumDamage > 100 )
				flTransformedDamage = max( it->pEntity->GetHealth( ), 8 ) + ( curConfig.iMinimumDamage - 100 );

			if ( autowall.GetDamage( pLocal,
				vecEyePosition,
				it->pEntity->GetHitboxPosition( HITBOX_HEAD, it->pRecord.front( ).pMatricies[ RESOLVE ] ),
				curConfig.pWeapon, &it->pRecord.front( ) ) < flTransformedDamage )
				return false;
		}
	}

	float flMultiplier = 0.28f;
	switch ( curConfig.iAutostopValue ) {
	case 1: flMultiplier = 0.26f;
		break;
	case 2: flMultiplier = 0.24f;
		break;
	case 3: flMultiplier = 0.20f;
		break;
	}

	float flIdealSpeed = ( flMultiplier ) * ( pLocal->IsScoped( ) ? curConfig.pWeapon->GetCSWpnData( )->flMaxSpeed[ 1 ] : curConfig.pWeapon->GetCSWpnData( )->flMaxSpeed[ 0 ] );

	pCmd->iButtons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);

	// Get the ideal speed for shooting (playstyle)
	Vector vecVelocity = pLocal->GetVelocity( );
	Vector vecDirection{};
	Vector vecRealView{};

	if ( flIdealSpeed > vecVelocity.Length2D( ) )
		return false;

	M::VectorAngles( vecVelocity, vecDirection );
	i::EngineClient->GetViewAngles( vecRealView );

	vecDirection.y = vecRealView.y - vecDirection.y;

	Vector vecForward{};
	M::AngleVectors( vecDirection, &vecForward );

	static CConVar* cl_forwardspeed = i::ConVar->FindVar( XorStr( "cl_forwardspeed" ) );
	static CConVar* cl_sidespeed = i::ConVar->FindVar( XorStr( "cl_sidespeed" ) );

	auto flNegativeForwardSpeed = -cl_forwardspeed->GetFloat( );
	auto flNegativeSideSpeed = -cl_sidespeed->GetFloat( );

	auto flNegativeForwardDirection = vecForward * flNegativeForwardSpeed;
	auto flNegativeSideDirection = vecForward * flNegativeSideSpeed;

	pCmd->flForwardMove = flNegativeForwardDirection.x;
	pCmd->flSideMove = flNegativeSideDirection.y;

	return true;
}

bool CAimBot::HitChance( CUserCmd*  pCmd, CBaseEntity*  pLocal, Vector vecWorldPosition, Vector vecAimPosition, Lagcompensation::LagRecord_t*  pRecord ) {

	matrix3x4a_t* pMatrix = pRecord->pMatricies[ RESOLVE ];

	if ( curConfig.iHitchance <= 1 )
		return true;

	CBaseCombatWeapon* pWeapon = curConfig.pWeapon;
	const CCSWeaponInfo* pWeaponData = curConfig.pWeapon->GetCSWpnData( );
	const Vector vecShootPosition = vecEyePosition;
	const float flGetSpread = prediction.flSpread;
	const float flGetInaccuracy = prediction.flInaccuracy;

	if ( vecShootPosition.DistTo( vecWorldPosition ) > pWeaponData->flRange )
		return false;

	//if (HasEnoughAccuracy(pLocal, flGetInaccuracy))
	//	return true;

	Vector vecForward = Vector( 0, 0, 0 );
	Vector vecRight = Vector( 0, 0, 0 );
	Vector vecUp = Vector( 0, 0, 0 );

	M::AngleVectors( vecAimPosition, &vecForward, &vecRight, &vecUp );

	vecForward.Normalize( );
	vecRight.Normalize( );
	vecUp.Normalize( );

	bool bSpecialWeapon = pWeapon->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_AWP || pWeapon->GetItemDefinitionIndex( ) == EItemDefinitionIndex::WEAPON_SSG08;
	int iAccuracry = bSpecialWeapon ? 256 : 128;

	static bool bSetupSpreadValues = true;
	static float flSpreadValues[ 256 ][ 6 ]{};

	if ( bSetupSpreadValues )
	{
		bSetupSpreadValues = false;

		for ( auto i = 0; i < 256; ++i )
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
			M::SinCos( d, &flSinD, &flCosD );

			flSpreadValues[ i ][ 2 ] = flSinB;
			flSpreadValues[ i ][ 3 ] = flCosB;
			flSpreadValues[ i ][ 4 ] = flSinD;
			flSpreadValues[ i ][ 5 ] = flCosD;
		}
	}

	int iHits = 0;
	int iAccuracyHits = 0;
	for ( auto i = 0; i < iAccuracry; ++i )
	{
		float flInacc = flSpreadValues[ i ][ 0 ] * flGetInaccuracy;
		float flSpread = flSpreadValues[ i ][ 1 ] * flGetSpread;

		float flSpreadX = flSpreadValues[ i ][ 3 ] * flInacc + flSpreadValues[ i ][ 5 ] * flSpread;
		float flSpreadY = flSpreadValues[ i ][ 2 ] * flInacc + flSpreadValues[ i ][ 4 ] * flSpread;

		Vector vecDirection = Vector( 0, 0, 0 );
		vecDirection.x = vecForward.x + vecRight.x * flSpreadX + vecUp.x * flSpreadY;
		vecDirection.y = vecForward.y + vecRight.y * flSpreadX + vecUp.y * flSpreadY;
		vecDirection.z = vecForward.z + vecRight.z * flSpreadX + vecUp.z * flSpreadY;

		Vector vecEnd = vecEyePosition + vecDirection * pWeaponData->flRange;

		if ( autowall.bCollidePoint( vecShootPosition, vecEnd, aimData.pRecord->pEntity->StudioHitbox( aimData.iHitbox ), pMatrix ) )
			iHits++;

		//if (autowall.bTraceMeantForHitbox(vecShootPosition, vecEnd, aimData.iHitbox, pRecord))
		//	iAccuracyHits++;
	}

	int flFinalHitchance = static_cast< int >( iHits / ( static_cast< float >( iAccuracry ) / 100.f ) );
	//int flFinalAccuracyBoost = static_cast<int>(static_cast<float>(iAccuracyHits) / (iAccuracry / 100.f));

	/* Hitchance * accuracyboost% = how much shot MUST hit head */
	//int flHitchanceAccuracy = static_cast<int>(curConfig.iHitchance * (static_cast<float>(curConfig.iAccuracyBoost) / 100.f));
	//if (flFinalAccuracyBoost < flHitchanceAccuracy)
	//	return false;

	if ( flFinalHitchance >= curConfig.iHitchance ) {
		aimData.flHitchance = flFinalHitchance;
		return true;
	}

	return false;
}

bool CAimBot::HasEnoughAccuracy( CBaseEntity*  pLocal, float flWeaponInAccuracy ) {

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar( XorStr( "weapon_accuracy_nospread" ) );
	if ( weapon_accuracy_nospread->GetInt( ) == 1 )
		return true;

	CBaseCombatWeapon* pWeapon = curConfig.pWeapon;

	// dont spamshot please
	if ( pWeapon->GetNextPrimaryAttack( ) == i::GlobalVars->flCurrentTime )
		return false;

	// jumpscout
	if ( pWeapon->GetItemDefinitionIndex( ) == WEAPON_SSG08 || pWeapon->GetItemDefinitionIndex( ) == WEAPON_REVOLVER )
		if ( !( pLocal->GetFlags( ) & FL_ONGROUND ) )
			if ( flWeaponInAccuracy <= 0.009f )
				return true;

	return false;
}

weaponConfig_t CAimBot::GetWeaponConfiguration( short iItemDefinitionIndex ) {

	using namespace cfg::rage;

	int iWeapon = OTHER;

	if ( iItemDefinitionIndex == WEAPON_USP_SILENCER || iItemDefinitionIndex == WEAPON_HKP2000 || iItemDefinitionIndex == WEAPON_ELITE || iItemDefinitionIndex == WEAPON_P250 || iItemDefinitionIndex == WEAPON_FIVESEVEN || iItemDefinitionIndex == WEAPON_CZ75A || iItemDefinitionIndex == WEAPON_GLOCK || iItemDefinitionIndex == WEAPON_TEC9 )
		iWeapon = PISTOL;
	else if ( iItemDefinitionIndex == WEAPON_REVOLVER || iItemDefinitionIndex == WEAPON_DEAGLE )
		iWeapon = HEAVY_PISTOL;
	else if ( iItemDefinitionIndex == WEAPON_SSG08 )
		iWeapon = SCOUT;
	else if ( iItemDefinitionIndex == WEAPON_AWP )
		iWeapon = AWP;
	else if ( iItemDefinitionIndex == WEAPON_SCAR20 || iItemDefinitionIndex == WEAPON_G3SG1 )
		iWeapon = AUTO;
	else if ( iItemDefinitionIndex == WEAPON_TASER )
		iWeapon = ZEUS;

	weaponConfig_t ret{};

	ret.iMinimumDamage = ( bOverride && IPT::HandleInput( iOverrideBind ) ) ? iOverride[ iWeapon ] : iMinDamages[ iWeapon ];
	ret.iHitchance = iHitchances[ iWeapon ];
	ret.iAccuracyBoost = iAccuracyBoost[ iWeapon ];
	ret.iHeadScale = iHeadPoints[ iWeapon ];
	ret.iBodyScale = iBodyPoints[ iWeapon ];
	ret.bSafePoint = bSafePoint[ iWeapon ];
	ret.bAutostop = bAutostop[ iWeapon ];
	ret.iAutostopValue = bAutostopAggressiveness[ iWeapon ];
	ret.bConditions[ CONDITION_BETWEEN_SHOTS ] = bConditions[ iWeapon ][ CONDITION_BETWEEN_SHOTS ];
	ret.bConditions[ CONDITION_INAIR ] = bConditions[ iWeapon ][ CONDITION_INAIR ];
	ret.bAutoScope = ( iWeapon == SCOUT || iWeapon == AWP || iWeapon == AUTO ) ? bAutoScope[ iWeapon ] : false;
	GetHitBoxes( NORMAL, ret.vecHitboxes[ NORMAL ], iWeapon );
	GetHitBoxes( MULTIPOINT, ret.vecHitboxes[ MULTIPOINT ], iWeapon );
	GetHitBoxes( SAFE, ret.vecHitboxes[ SAFE ], iWeapon );

	return ret;
}

void CAimBot::GetHitBoxes( int i, std::vector<int>& vecOut, int iWeapon ) {

	if ( i::ConVar->FindVar( XorStr( "mp_damage_headshot_only" ) )->GetBool( ) ) {
		vecOut.push_back( HITBOX_HEAD );
		return;
	}

	if ( cfg::rage::bForceBaim && IPT::HandleInput( cfg::rage::iForceBaimKey ) ) {

		vecOut.push_back( HITBOX_STOMACH );
		vecOut.push_back( HITBOX_CHEST );
		return;
	}

	for ( size_t iIndex = 0; iIndex < 6; iIndex++ )
	{
		if ( !cfg::rage::bHitboxes[ i ][ iWeapon ][ iIndex ] )
			continue;

		switch ( iIndex )
		{
		case 0:
			vecOut.push_back( HITBOX_HEAD );
			break;

		case 1:
			vecOut.push_back( HITBOX_UPPER_CHEST );
			break;

		case 2:
			vecOut.push_back( HITBOX_CHEST );
			break;

		case 3:
			vecOut.push_back( HITBOX_STOMACH );
			break;

		case 4:
			vecOut.push_back( HITBOX_RIGHT_FOREARM );
			vecOut.push_back( HITBOX_LEFT_FOREARM );
			break;

		case 5:
			vecOut.push_back( HITBOX_RIGHT_FOOT );
			vecOut.push_back( HITBOX_LEFT_FOOT );
			break;
		}
	}
}

std::vector<Lagcompensation::AnimationInfo_t*> CAimBot::GetTargetableEntities( CBaseEntity*  pLocal ) {

	std::vector<Lagcompensation::AnimationInfo_t*> ret{};

	CCSWeaponInfo* pWeaponData = curConfig.pWeapon->GetCSWpnData( );
	if ( !pWeaponData )
		return ret;

	for ( size_t i = 1; i <= i::GlobalVars->nMaxClients; i++ ) {

		CBaseEntity* pEntity = static_cast< CBaseEntity* >( i::EntityList->GetClientEntity( i ) );

		if ( !pEntity || !pEntity->IsEnemy( pLocal ) || !pEntity->IsAlive( ) || pEntity->IsDormant( ) || pEntity->HasImmunity( ) )
			continue;

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog( i );
		if ( !pLog || pLog->pRecord.empty( ) || pLog->pEntity != pEntity )
			continue;

		// OPTIMIZATION: skip entities that are further than our weapons range
		if ( pLog->pEntity->GetVecOrigin( ).DistTo( pLocal->GetVecOrigin( ) ) > pWeaponData->flRange )
			continue;

		if ( playerList::arrPlayers[ i ].bWhiteList )
			continue;

		bool bHasValidRecord = false;
		pLog->iLastValid = 0;
		for ( auto j = 0u; j < pLog->pRecord.size( ); j++ ) {

			Lagcompensation::LagRecord_t* pCurrentRecord = &pLog->pRecord.at( j );
			pCurrentRecord->bValid = pCurrentRecord->IsValid();

			if (pCurrentRecord->bValid)
			{
				pLog->iLastValid = static_cast<int>(j);
				bHasValidRecord = true;
			}
		}

		if (!bHasValidRecord)
			continue;

		ret.push_back( pLog );
	}

	std::stable_sort(
		ret.begin(),
		ret.end(),
		[](const auto* a, const auto* b)
		{
			const bool aPriority =
				playerList::arrPlayers[
					a->pEntity->EntIndex()
				].bPriority;

			const bool bPriority =
				playerList::arrPlayers[
					b->pEntity->EntIndex()
				].bPriority;

			if (aPriority != bPriority)
				return aPriority;

			return a->pEntity->GetHealth() <
				b->pEntity->GetHealth();
		}
	);

	return ret;
}

std::vector<Vector> CAimBot::CreatePoints( Vector vecEyePosition, CBaseCombatWeapon*  pWeapon, Lagcompensation::LagRecord_t*  pRecord, int iHitbox, EMatrixType iType, bool bShouldMultipoint ) {

	std::vector<Vector> refVecPoints{};

	float flRadius = 0.f;
	Vector vecCenter = pRecord->pEntity->GetHitboxPosition( iHitbox, pRecord->pMatricies[ iType ], flRadius );
	if (iHitbox == HITBOX_HEAD && iType == RESOLVE && pRecord->bHasHeadOverlap
		&& pRecord->iResolverMode == static_cast<int>(resolver::Mode::Experimental))
		refVecPoints.push_back(pRecord->vecRageSafePoint);

	if ( flRadius < 0 || !bShouldMultipoint )
	{
		refVecPoints.push_back(vecCenter);
		return refVecPoints;
	}

	float flHitboxDistance = flRadius * ( ( iHitbox == HITBOX_HEAD ? curConfig.iHeadScale : curConfig.iBodyScale ) * 0.01f );

	Vector vecCurrentAngles = M::VectorAngles( vecCenter - vecEyePosition );
	Vector vecForward; M::AngleVectors( vecCurrentAngles, &vecForward );

	const Vector vecRight = vecForward.CrossProduct( Vector( 0, 0, 1 ) );
	const Vector vecLeft = Vector( -vecRight.x, -vecRight.y, vecRight.z );
	const Vector vecTop = Vector( 0, 0, 1 );

	refVecPoints.push_back(vecCenter);
	if ( iHitbox == HITBOX_HEAD )
		refVecPoints.push_back( vecCenter + vecTop * flHitboxDistance );
	
	refVecPoints.push_back( vecCenter + vecLeft * flHitboxDistance );
	refVecPoints.push_back( vecCenter + vecRight * flHitboxDistance );

	return refVecPoints;
}
