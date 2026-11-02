#include "aimbot.h"
#include "Animations/EnemyAnimations.h"
#include "Animations/ExperimentalResolver.h"
#include "../../Lua/Lua.h"
#include "../../Interface/Classes/CCSGameRulesProxy.h"
#include "StudioHitboxGeometry.h"
#include "MultipointGeometry.h"
#include "LagcompHistory.h"
#include "RecordTraceScope.h"
#include "exploits.h"
#include <optional>

void CAimBot::CreateMove( CUserCmd*  pCmd, CBaseEntity*  pLocal ) {
	hasValidPoint = false;

	static CConVar* recoilScale = i::ConVar->FindVar( XorStr( "weapon_recoil_scale" ) );

	if ( !cfg::rage::bEnable || ( !IPT::HandleInput( cfg::rage::iAimbotKey ) && cfg::rage::iAimbotKey != 0 ) ) {
		scanRuntime.Reset();
		return ResetAimbotData();
	}

	if ( ( *GameRules )->m_bFreezePeriod( ) )
		return ResetAimbotData();

	/* Run R8 Revolver cock */
	misc::RevolverCreateMove( );

	/* Create new data for aimbot */
	aimData = rageBotData_t( );
	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );

	if ( pWeapon == nullptr )
		return ResetAimbotData( );

	if ( pWeapon->IsGrenade( ) || pWeapon->IsKnife( ) || !pWeapon->IsWeapon( ) )
		return ResetAimbotData();

	if ( !pWeapon->GetAmmo( ) )
		return ResetAimbotData( );

	/* Get currently equipped weapon config */
	curConfig = GetWeaponConfiguration( pWeapon->GetItemDefinitionIndex( ) );
	curConfig.pWeapon = pWeapon;
	aimData.bFireReady = pLocal->CanShoot(pWeapon, pLocal->GetTickBase());

	/* Make a vector out of every valid & tragetable entity */
	std::vector<Lagcompensation::AnimationInfo_t*> vecTargets = GetTargetableEntities( pLocal );

	if ( vecTargets.empty( ) )
		return ResetAimbotData();

	/* Use the current shoot position prepared by local animations */
	vecEyePosition = localAnim->GetShootPosition( );

	/* Scan stored entities & safepoint */
	Vector vecShootPosition = ScanHitboxes( vecTargets, pLocal );
	// Keep point availability independent of weapon cooldown and hitchance.
	hasValidPoint = aimData.pRecord && vecShootPosition.IsValid()
		&& lagcomp.GetRecordTiming(aimData.flTargetSimulation).valid;

	// Check the braking endpoint before requiring a currently shootable target.
	const bool stopping = AutoStop(vecTargets, pLocal, pCmd);
	if (stopping) {
		// The first preview used the held movement key. Hitchance must see the
		// command after autostop, otherwise it can wait on stale movement.
		if (!prediction.RepredictMovement(pCmd, pLocal, g::vecOriginalViewAngle))
			return ResetAimbotData();
		g::vecEyePosition = pLocal->GetEyePosition(false);
		localAnim->CopyPlayerAnimationData(false, pLocal);
		localAnim->SetupShootPosition(pLocal);
		vecEyePosition = localAnim->GetShootPosition();
		// Reuse an already selected point and validate it below. Restarting a
		// full scan here could exhaust the per-frame budget before firing.
		if (!aimData.pRecord)
			vecShootPosition = ScanHitboxes(vecTargets, pLocal);
		hasValidPoint = aimData.pRecord && vecShootPosition.IsValid()
			&& lagcomp.GetRecordTiming(aimData.flTargetSimulation).valid;
	}
	if ( aimData.pRecord == nullptr )
		return ResetAimbotData();

	/* Apply the selected record to the desired entity */
	aimData.pRecord->ApplyMatrix( aimData.pAimbotTarget, RESOLVE );
	if (stopping) {
		const float damage = autowall.GetDamage(pLocal, vecEyePosition, vecShootPosition,
			curConfig.pWeapon, aimData.pRecord);
		const int minimum = curConfig.iMinimumDamage > 100
			? (std::max)(aimData.pAimbotTarget->GetHealth(), 4) + curConfig.iMinimumDamage - 100
			: curConfig.iMinimumDamage;
		const int coverage = (aimData.bSafe || curConfig.bSafePoint)
			? autowall.SafePoint(vecEyePosition, aimData.pRecord, vecShootPosition, aimData.iHitbox) : 0;
		if (!std::isfinite(damage) || damage < minimum || (aimData.bSafe && coverage != 3)
			|| (curConfig.bSafePoint && coverage < 2)) {
			hasValidPoint = false;
			return ResetAimbotData();
		}
		aimData.vecLocalShootPosition = vecEyePosition;
		aimData.flDamage = std::floor(damage);
	}

	/* Auto zoom & m_bResumeZoom check */
	if ( curConfig.bAutoScope && !pLocal->IsScoped( ) && !pLocal->IsResumingScope( ) )
		pCmd->iButtons |= IN_ZOOM;

	/* Calculate the aim angle with vectorAngles */
	Vector vecAimAngle = ( M::VectorAngles( vecShootPosition - vecEyePosition ) -= ( pLocal->GetAimPunch( ) * recoilScale->GetFloat( ) ) );

	/* Calculate current hitchance & accuracy boost */
	// Fire as soon as accuracy is sufficient, including while still moving.
	aimData.bCanShoot = aimData.bFireReady
		&& HitChance( pCmd, pLocal, vecShootPosition, vecAimAngle, aimData.pRecord );

	/* Wait til we can shoot */
	if ( !aimData.bCanShoot || !aimData.bFireReady )
		return ResetAimbotData( );
	const auto timing = lagcomp.GetRecordTiming(aimData.flTargetSimulation);
	if (!timing.valid) return ResetAimbotData();

	/* Shoot entity when we can & have enough hitchance */
	pCmd->angViewPoint = vecAimAngle;
	if ( !cfg::rage::bSilentAim ) i::EngineClient->SetViewAngles( vecAimAngle );

	pCmd->iButtons |= IN_ATTACK;
	aimData.iTickcount = pCmd->iTickCount;
	aimData.iDesiredTickcount = timing.commandTick;
	aimData.iBacktrackTicks = pCmd->iTickCount - timing.commandTick;
	aimData.shotTiming = timing;


	/* Sending packet in prediction will cause hit registration delay ( only in CHLClient::CreateMove() ) */
	bShouldSendPacket = true;
}

void CAimBot::ResetAimbotData() {
	aimData.ClearTarget();
}

void CAimBot::RevalidatePrediction(CUserCmd* pCmd, CBaseEntity* pLocal) {
	if (!bShouldSendPacket || !(pCmd->iButtons & IN_ATTACK) || !aimData.pRecord)
		return;
	const auto reject = [&] {
		pCmd->iButtons &= ~IN_ATTACK;
		bShouldSendPacket = false;
		aimData.ClearTarget();
	};
	const int tickbase = pLocal->GetTickBase();
	const auto timing = lagcomp.GetRecordTiming(aimData.flTargetSimulation, .2f, tickbase);
	if (!timing.valid || pLocal->GetWeapon() != curConfig.pWeapon
		|| !pLocal->CanShoot(curConfig.pWeapon, tickbase)) return reject();

	vecEyePosition = localAnim->GetShootPosition();
	static auto* recoilScale = i::ConVar->FindVar(XorStr("weapon_recoil_scale"));
	Vector angle = M::VectorAngles(aimData.vecTargetShootPosition - vecEyePosition)
		- pLocal->GetAimPunch() * recoilScale->GetFloat();
	RecordTraceScope trace(aimData.pAimbotTarget);
	if (!trace.Ready()) return reject();
	aimData.pRecord->ApplyMatrix(aimData.pAimbotTarget, RESOLVE);
	const float damage = autowall.GetDamage(pLocal, vecEyePosition, aimData.vecTargetShootPosition,
		curConfig.pWeapon, aimData.pRecord);
	const int minimum = curConfig.iMinimumDamage > 100
		? (std::max)(aimData.pAimbotTarget->GetHealth(), 4) + curConfig.iMinimumDamage - 100
		: curConfig.iMinimumDamage;
	if (!std::isfinite(damage) || damage < minimum
		|| !HitChance(pCmd, pLocal, aimData.vecTargetShootPosition, angle, aimData.pRecord)) return reject();
	if (aimData.bSafe && autowall.SafePoint(vecEyePosition, aimData.pRecord,
		aimData.vecTargetShootPosition, aimData.iHitbox) != 3) return reject();

	aimData.vecLocalShootPosition = vecEyePosition;
	aimData.flDamage = std::floor(damage);
	aimData.bCanShoot = aimData.bFireReady = true;
	aimData.iDesiredTickcount = timing.commandTick;
	aimData.shotTiming = timing;
	pCmd->angViewPoint = angle;
	if (!cfg::rage::bSilentAim) i::EngineClient->SetViewAngles(angle);
}

bool CAimBot::PostPrediction( CUserCmd* pCmd, bool&  bSendPacket ) {
	const bool armed = bShouldSendPacket;
	bShouldSendPacket = false;
	if (!pCmd || !(pCmd->iButtons & IN_ATTACK)) {
		aimData.iDesiredTickcount = -1;
		return false;
	}
	// Never rewrite the tick of a manual shot without an armed aimed target.
	if (!armed || !aimData.pRecord || !aimData.bCanShoot || !aimData.bFireReady || aimData.iDesiredTickcount < 0) {
		if (armed) pCmd->iButtons &= ~IN_ATTACK;
		aimData.iDesiredTickcount = -1;
		return false;
	}
	// Exploits can finalize a different command tickbase after the initial
	// prediction. Validate the selected timestamp against that final plan.
	const int tickbase = g::pLocal ? g::pLocal->GetTickBase() : -1;
	const auto timing = lagcomp.GetRecordTiming(aimData.flTargetSimulation, .2f, tickbase);
	if (!timing.valid || !g::pLocal || !g::pLocal->CanShoot(curConfig.pWeapon, tickbase)
		|| timing.commandTick != aimData.iDesiredTickcount
		|| std::fabs(timing.lerp - aimData.shotTiming.lerp) > .000001f) {
		pCmd->iButtons &= ~IN_ATTACK;
		aimData.ClearTarget();
		return false;
	}
	pCmd->iTickCount = timing.commandTick;
	aimData.shotTiming = timing;
	// Register only attacks that survived auto-pistol/late prediction changes.
	lagcomp.GetLog(aimData.pRecord->iEntIndex).iShotAmount++;
	hitlogData = aimData;
	experimentalResolver.RecordShot(hitlogData, pCmd->iCommandNumber);
	const bool send = !(cfg::antiaim::bFakeDuck && IPT::HandleInput(cfg::antiaim::iFakeDuckKey));
	if (send) bSendPacket = true;
	aimData.iTickcount = pCmd->iTickCount;
	aimData.iDesiredTickcount = -1;
	return send;
}

CAimBot::SCAN CAimBot::ShouldSkipRecord( Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pNewerRecord ) {

	// A teleport boundary belongs between two records. Its newest endpoint
	// remains usable; only traversal into the older side is forbidden.
	if ( pNewerRecord && pNewerRecord->bBreakingLagcompensation )
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
	hitscan::ScanTimer timer{scanStatistics};
	if (!pLocal || !curConfig.pWeapon || !vecEyePosition.IsValid()) return {};
	std::uint64_t signature = 14695981039346656037ull;
	auto hashValue = [&](const auto& value) { signature = hitscan::HashBytes(&value, sizeof(value), signature); };
	hashValue(pLocal->GetRefEHandle()); hashValue(pLocal->GetSpawnTime());
	hashValue(curConfig.pWeapon->GetRefEHandle()); hashValue(curConfig.iMinimumDamage);
	hashValue(curConfig.bSafePoint); hashValue(curConfig.iHeadScale); hashValue(curConfig.iBodyScale);
	hashValue(cfg::rage::iResolverMode);
	hashValue(scanRuntime.policy.adaptive);
	static CConVar* damageVars[] = {
		i::ConVar->FindVar("mp_damage_scale_ct_head"), i::ConVar->FindVar("mp_damage_scale_t_head"),
		i::ConVar->FindVar("mp_damage_scale_ct_body"), i::ConVar->FindVar("mp_damage_scale_t_body"),
		i::ConVar->FindVar("ff_damage_bullet_penetration"), i::ConVar->FindVar("ff_damage_reduction_bullets")
	};
	for (const auto variable : damageVars) if (variable) hashValue(variable->GetFloat());
	const auto weaponData = curConfig.pWeapon->GetCSWpnData();
	if (weaponData) {
		hashValue(weaponData->iDamage); hashValue(weaponData->flArmorRatio); hashValue(weaponData->flHeadShotMultiplier);
		hashValue(weaponData->flPenetration); hashValue(weaponData->flRange); hashValue(weaponData->flRangeModifier);
	}
	for (const auto& boxes : curConfig.vecHitboxes) {
		const auto size = boxes.size(); hashValue(size);
		for (int box : boxes) hashValue(box);
	}
	scanRuntime.Begin(i::GlobalVars->iFrameCount, signature, aimData.bFireReady);
	const bool adaptive = scanRuntime.policy.adaptive;
	static CConVar* lagcompensation = i::ConVar->FindVar("cl_lagcompensation");
	struct Job {
		Lagcompensation::LagRecord_t* record;
		int age;
		float minimum, upper;
		int memo = -1;
		bool prepared{};
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
		std::uint64_t hash;
		std::vector<RayResult> rays;
	};
	std::vector<Job> jobs;
	std::vector<GeometryResults> memo;
	std::array<std::vector<int>, 65> targetJobs, targetMemo;
	std::vector<int> targets;
	jobs.reserve(vecIn.size() * 32);
	memo.reserve(vecIn.size() * 3);
	for (auto log : vecIn) {
		if (!log || !log->pEntity || log->pRecord.empty()) continue;
		const int entityIndex = log->pEntity->EntIndex();
		if (entityIndex < 1 || entityIndex >= 65) continue;
		log->iLastValid = std::clamp(log->iLastValid, 0, static_cast<int>(log->pRecord.size()) - 1);
		if (lagcompensation && lagcompensation->GetInt() == 0) log->iLastValid = 0;
		const float minimum = static_cast<float>(curConfig.iMinimumDamage > 100
			? (std::max)(log->pEntity->GetHealth(), 4) + curConfig.iMinimumDamage - 100 : curConfig.iMinimumDamage);
		backtrack::HistoryWalk history(hitbox::FromVector(log->pEntity->GetVecOrigin()));
		for (int age = 0; age <= log->iLastValid; ++age) {
			auto record = &log->pRecord[age];
			if (!history.Visit(hitbox::FromVector(record->vecOrigin),!record->bDormant,record->bBreakingLagcompensation)) {
				++scanStatistics.historyBreaks; break;
			}
			if (cfg::rage::bLimitScan && (age & 1)) continue;
			const auto skip = ShouldSkipRecord(record,age ? &log->pRecord[age-1] : nullptr);
			if (skip == SCAN_BREAK) break;
			if (skip == SCAN_SKIP || !record->bValid || !record->bAnimationMatrixValid || record->pEntity != log->pEntity || record->iEntIndex != entityIndex) continue;
			++scanStatistics.recordsConsidered;
			const float upper = adaptive ? std::numeric_limits<float>::infinity()
				: autowall.GetDamageUpperBound(vecEyePosition, curConfig.pWeapon, record);
			if (std::floor(upper) < minimum) { ++scanStatistics.boundSkips; continue; }
			targetJobs[entityIndex].push_back(static_cast<int>(jobs.size()));
			jobs.push_back({record, age, minimum, upper, -1, !adaptive});
		}
		if (!targetJobs[entityIndex].empty()) targets.push_back(entityIndex);
	}
	// Explore the records with the best possible result first. A later query
	// is skipped only when its optimistic ranking cannot improve the best hit.
	if (!adaptive) std::stable_sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) { return a.upper > b.upper; });
	Hitscan_t best;
	bool found = false;
	bool stopped = false;
	int bestRequested = -1;
	std::optional<RecordTraceScope> traceScope;
	int appliedMemo = -1;
	auto exhausted = [&]() {
		if (!stopped && scanRuntime.Exhausted(scanStatistics)) { stopped = true; ++scanStatistics.budgetStops; }
		return stopped;
	};
	auto prepare = [&](Job& job) {
		if (!job.prepared) {
			job.upper = autowall.GetDamageUpperBound(vecEyePosition, curConfig.pWeapon, job.record);
			job.prepared = true; ++scanStatistics.recordVisits;
		}
		return std::floor(job.upper) >= job.minimum;
	};
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
			const auto poseHash = hitscan::HashBytes(job.record->pMatricies[RESOLVE], count * sizeof(matrix3x4a_t));
			const int entityIndex = job.record->iEntIndex;
			for (int index : targetMemo[entityIndex]) {
				const auto& group = memo[index];
				const auto a = group.record;
				const auto b = job.record;
				if (group.hash == poseHash && a->pEntity == b->pEntity && group.count == count && a->vecOrigin == b->vecOrigin
					&& a->vecMins == b->vecMins && a->vecMaxs == b->vecMaxs
					&& std::memcmp(a->pMatricies[RESOLVE], b->pMatricies[RESOLVE], count * sizeof(matrix3x4a_t)) == 0) {
					job.memo = static_cast<int>(index); break;
				}
			}
			if (job.memo < 0) {
				job.memo = static_cast<int>(memo.size());
				memo.push_back({job.record, count, poseHash, {}});
				targetMemo[entityIndex].push_back(job.memo);
				memo.back().rays.reserve(16);
			}
		}
		auto& rays = memo[job.memo].rays;
		for (const auto& result : rays) {
			if (std::memcmp(result.direction.data(), key.data(), sizeof(key)) == 0) {
				++scanStatistics.reusedRays; data = result.data; return result.damage;
			}
		}
		std::uint64_t probe = hitscan::HashBytes(key.data(), sizeof(key), signature);
		auto probeValue = [&](const auto& value) { probe = hitscan::HashBytes(&value, sizeof(value), probe); };
		probeValue(memo[job.memo].hash); probeValue(job.record->flSimulationTime);
		probeValue(job.record->pEntity->GetRefEHandle()); probeValue(job.record->pEntity->GetSpawnTime());
		probeValue(job.record->pEntity->GetModel()); probeValue(job.record->vecOrigin);
		probeValue(job.record->vecMins); probeValue(job.record->vecMaxs);
		probeValue(job.record->pEntity->GetHealth()); probeValue(job.record->pEntity->GetArmor());
		probeValue(job.record->pEntity->HasHelmet()); probeValue(job.record->pEntity->HasHeavyArmor());
		probeValue(job.record->pEntity->GetTeam());
		probeValue(job.record->uExperimentalEpoch); probeValue(playerList::arrPlayers[job.record->iEntIndex].bSafePoint);
		probeValue(job.minimum); probeValue(vecEyePosition);
		if (scanRuntime.Deferred(probe)) { ++scanStatistics.deferredPoints; return -1.f; }
		if (exhausted()) return -1.f;
		// Restore the previous target before any real query of a different pose.
		// Several points of the same geometry share one backup/apply/restore.
		if (appliedMemo != job.memo) {
			traceScope.reset(); appliedMemo = -1; traceScope.emplace(job.record->pEntity);
			if (!traceScope->Ready()) { traceScope.reset(); return -1.f; }
			job.record->ApplyMatrix(job.record->pEntity, RESOLVE); appliedMemo = job.memo;
		}
		const auto query = scanRuntime.Query(job.minimum, scanStatistics);
		const auto start = hitscan::Clock::now();
		++scanStatistics.damageCalls;
		const float value = autowall.GetDamage(pLocal, vecEyePosition, point, curConfig.pWeapon, job.record, &data, &query);
		const double micros = std::chrono::duration<double, std::micro>(hitscan::Clock::now() - start).count();
		scanStatistics.damageMicros += micros; scanRuntime.Charge(micros);
		scanStatistics.traceCalls += data.traceCalls; scanStatistics.exitSteps += data.exitSteps;
		if (data.failure == hitscan::Failure::LowDamage) ++scanStatistics.lowDamageStops;
		if (data.failure == hitscan::Failure::Deadline) { ++scanStatistics.deadlineStops; stopped = adaptive; }
		scanRuntime.Observe(probe, std::isfinite(value) && value >= job.minimum, data.failure);
		if (data.failure != hitscan::Failure::Deadline && data.failure != hitscan::Failure::Invalid)
			rays.push_back({key, value, data});
		return value;
	};
	std::uint32_t normalMask = 0, multipointMask = 0, safeMask = 0;
	for (int box : curConfig.vecHitboxes[NORMAL]) if (box >= 0 && box < HITBOX_MAX) normalMask |= 1u << box;
	for (int box : curConfig.vecHitboxes[MULTIPOINT]) if (box >= 0 && box < HITBOX_MAX) multipointMask |= 1u << box;
	for (int box : curConfig.vecHitboxes[SAFE]) if (box >= 0 && box < HITBOX_MAX) safeMask |= 1u << box;
	// mode 0: common points, 1: multipoint expansion, 2: one anchor,
	// 3: last good point, 4: rotating alternative. Every point gets current
	// geometry/safety checks and damage; no positive result persists across ticks.
	auto scanBox = [&](Job& job, int hitboxIndex, int mode, const Vector* hint = nullptr, unsigned choice = 0) {
		if (exhausted() || hitboxIndex < 0 || hitboxIndex >= HITBOX_MAX || !prepare(job)) return;
		auto record = job.record;
		const bool rejected = experimentalResolver.IsRejected(record);
		const bool experimentalHead = record->iResolverMode == static_cast<int>(resolver::Mode::Experimental) && hitboxIndex == HITBOX_HEAD;
		const bool headPotential = experimentalHead && record->bHasHeadOverlap;
		if (!canImprove(job, hitboxIndex, mode != 1 && mode != 4 && headPotential)) { ++scanStatistics.boundSkips; return; }
		Vector common;
		const bool hasCommon = GetIntersectionPoint(record, hitboxIndex, common);
		hitscan::Points<Vector> points;
		if (mode == 3 && hint) points.push_back(*hint);
		else if (mode == 0 || (mode == 2 && hasCommon)) {
			if (!hasCommon) return;
			points.push_back(common);
		} else {
			points = CreatePoints(vecEyePosition, curConfig.pWeapon, record, hitboxIndex, RESOLVE,
				(mode == 1 || mode == 4) && (multipointMask & (1u << hitboxIndex)) != 0);
			if (mode == 2 && !points.empty()) {
				const Vector center = points[points.size() - 1]; points = {}; points.push_back(center);
			}
		}
		if (mode == 4) {
			auto anchors = CreatePoints(vecEyePosition, curConfig.pWeapon, record, hitboxIndex, RESOLVE, false);
			if (anchors.empty()) return;
			const Vector anchor = hasCommon ? common : anchors[anchors.size() - 1];
			hitscan::Points<Vector> alternatives;
			for (const auto& point : points)
				if (!(point == anchor) && !(experimentalHead && record->bHasHeadOverlap && point == record->vecRageSafePoint))
					alternatives.push_back(point);
			if (alternatives.empty()) return;
			points = {}; points.push_back(alternatives[choice % alternatives.size()]);
		}
		const bool playerSafe = playerList::arrPlayers[record->iEntIndex].bSafePoint;
		const bool forceSafe = (safeMask & (1u << hitboxIndex)) != 0;
		for (auto point : points) {
			if (exhausted()) break;
			if (!point.IsValid() || (mode == 1 && hasCommon && point == common)) continue;
			if (mode == 1 && experimentalHead && record->bHasHeadOverlap && point == record->vecRageSafePoint) continue;
			const bool isCommon = hasCommon && point == common;
			const bool overlapHead = isCommon && experimentalHead;
			if (!canImprove(job, hitboxIndex, overlapHead)) { ++scanStatistics.boundSkips; continue; }
			int coverage = isCommon ? 3 : 0;
			if (!isCommon && (curConfig.bSafePoint || playerSafe || forceSafe || experimentalHead || rejected))
				coverage = autowall.SafePoint(vecEyePosition, record, point, hitboxIndex);
			const bool safe = coverage == 3;
			if ((rejected || playerSafe || forceSafe) && !safe) {
				if (rejected) ++scanStatistics.resolverRejectedPoints;
				++scanStatistics.safetyRejected; continue;
			}
			if (curConfig.bSafePoint && coverage < 2) { ++scanStatistics.safetyRejected; continue; }
			const Vector ray = (point - vecEyePosition).Normalized();
			Job rayJob = job;
			rayJob.upper = (std::min)(job.upper, autowall.GetDamageUpperBound(vecEyePosition, curConfig.pWeapon, record, &ray));
			if (std::floor(rayJob.upper) < job.minimum || !canImprove(rayJob, hitboxIndex, overlapHead)) {
				++scanStatistics.boundSkips; continue;
			}
			if (isCommon) ++scanStatistics.intersectionPoints; else ++scanStatistics.fallbackPoints;
			FireBulletData_t data{};
			const float value = damage(job, point, data);
			if (!std::isfinite(value) || value < job.minimum) continue;
			if (overlapHead && data.enterTrace.iHitbox != HITBOX_HEAD) continue;
			Hitscan_t candidate(record, point, data, hitboxIndex != HITBOX_HEAD && value > record->pEntity->GetHealth(),
				safe, static_cast<int>(job.minimum), job.age, overlapHead);
			if (!found || candidate < best) { best = candidate; found = true; bestRequested = hitboxIndex; }
		}
	};
	auto decisive = [&]() {
		return adaptive && found && best.bSafe && best.flDamage >= best.iTransformedDamage
			&& (best.bHeadOverlap || best.flDamage > best.pRecord->pEntity->GetHealth());
	};
	if (!adaptive) {
		for (int phase = 0; phase < 2; ++phase)
			for (auto& job : jobs)
				for (int box : curConfig.vecHitboxes[NORMAL]) scanBox(job, box, phase);
	} else if (!targets.empty()) {
		// Rotate the first target and one representative historical record on
		// every scan, including no-hit scans. Expensive failures cannot monopolize
		// all future command budgets.
		std::rotate(targets.begin(), targets.begin() + (scanRuntime.sequence - 1) % targets.size(), targets.end());
		std::array<hitscan::Points<int, 3>, 65> representatives;
		std::array<int, 65> explorers{};
		for (int index : targets) {
			const auto& list = targetJobs[index];
			auto& selected = representatives[index];
			auto add = [&](int job) {
				if (std::find(selected.begin(), selected.end(), job) == selected.end()) selected.push_back(job);
			};
			add(list.front());
			auto& hint = scanRuntime.hints[index];
			const auto entity = jobs[list.front()].record->pEntity;
			if (hint.handle != entity->GetRefEHandle() || hint.model != entity->GetModel() || hint.spawn != entity->GetSpawnTime()) hint.valid = false;
			int important = -1;
			for (int job : list) {
				if (hint.valid && jobs[job].record->flSimulationTime == hint.simulation) { important = job; break; }
				if (important < 0 && jobs[job].record->bDidShot) important = job;
			}
			if (important >= 0) add(important);
			else if (list.size() > 1) add(list.back());
			const auto cursor = scanRuntime.recordCursor[index] % list.size();
			for (size_t offset = 0; offset < list.size() && selected.size() < 3; ++offset) add(list[(cursor + offset) % list.size()]);
			explorers[index] = selected[selected.size() - 1];
		}
		const int previous = scanRuntime.previousTarget;
		if (previous > 0 && previous < 65 && scanRuntime.hints[previous].valid) {
			const auto& hint = scanRuntime.hints[previous];
			if (hint.hitbox >= 0 && hint.hitbox < HITBOX_MAX && (normalMask & (1u << hint.hitbox)))
				for (int job : targetJobs[previous]) if (jobs[job].record->flSimulationTime == hint.simulation) {
					const auto point = hitbox::ToVector(hint.point); scanBox(jobs[job], hint.hitbox, 3, &point); break;
				}
		}
		// Give each target its newest head/body opportunity before another
		// target's entire backtrack history or all peripheral multipoints.
		// Leave a third of the command budget for widening points: otherwise
		// continuously changing blocked poses can consume every scan in anchors.
		auto coarseDone = [&]() {
			return scanStatistics.damageCalls >= (std::max)(1, scanRuntime.CallLimit() * 2 / 3)
				|| scanStatistics.damageMicros >= scanRuntime.TimeLimit() * (2. / 3.);
		};
		for (int round = 0; round < 3 && !exhausted() && !decisive() && !coarseDone(); ++round)
			for (int index : targets) {
				if (exhausted() || decisive() || coarseDone()) break;
				if (round >= static_cast<int>(representatives[index].size())) continue;
				auto& selected = representatives[index];
				if (round == 0) {
					const unsigned visits = ++scanRuntime.targetVisits[index];
					if (selected.size() > 1 && (visits & 1u) == 0)
						std::swap(selected[0], selected[(visits & 3u) == 0 ? 1 : selected.size() - 1]);
				}
				const int jobIndex = selected[round];
				if (jobIndex == explorers[index]) {
					const auto& list = targetJobs[index];
					scanRuntime.recordCursor[index] = static_cast<unsigned>(std::find(list.begin(), list.end(), jobIndex) - list.begin() + 1);
				}
				auto& job = jobs[jobIndex];
				const auto visits = scanRuntime.targetVisits[index];
				if (round == 0 && (visits & 3u) == 0 && !curConfig.vecHitboxes[NORMAL].empty()) {
					const unsigned ordinal = visits / 4 - 1;
					const auto count = curConfig.vecHitboxes[NORMAL].size();
					scanBox(job, curConfig.vecHitboxes[NORMAL][ordinal % count], 4, nullptr, static_cast<unsigned>(ordinal / count));
				}
				int body = -1;
				for (int box : curConfig.vecHitboxes[NORMAL]) if (box != HITBOX_HEAD && box >= 0 && box < HITBOX_MAX) {
					if (body < 0) body = box;
					const auto studio = job.record->pEntity->StudioHitbox(box);
					if (studio && studio->iGroup == HITGROUP_STOMACH) { body = box; break; }
				}
				if (normalMask & (1u << HITBOX_HEAD)) scanBox(job, HITBOX_HEAD, 2);
				if (body >= 0 && !decisive() && !coarseDone()) scanBox(job, body, 2);
			}
		for (int round = 0; round < 3 && !exhausted() && !decisive(); ++round)
			for (int index : targets) {
				if (exhausted() || decisive()) break;
				if (round >= static_cast<int>(representatives[index].size())) continue;
				auto& job = jobs[representatives[index][round]];
				for (int box : curConfig.vecHitboxes[NORMAL]) {
					if (exhausted() || decisive()) break;
					scanBox(job, box, 0); if (!decisive()) scanBox(job, box, 1);
				}
			}
		// Spare budget explores all remaining records in target rounds.
		for (size_t round = 0; round < 32 && !exhausted() && !decisive(); ++round)
			for (int index : targets) {
				if (exhausted() || decisive()) break;
				const auto& list = targetJobs[index];
				if (round >= list.size()) continue;
				const int jobIndex = list[(round + scanRuntime.recordCursor[index]) % list.size()];
				const auto& selected = representatives[index];
				if (std::find(selected.begin(), selected.end(), jobIndex) != selected.end()) continue;
				for (int box : curConfig.vecHitboxes[NORMAL]) {
					if (exhausted() || decisive()) break;
					scanBox(jobs[jobIndex], box, 0); if (!decisive()) scanBox(jobs[jobIndex], box, 1);
				}
			}
	}
	traceScope.reset();
	if (!found) return {};
	if (adaptive) {
		const auto entity = best.pRecord->pEntity;
		scanRuntime.previousTarget = best.pRecord->iEntIndex;
		scanRuntime.hints[scanRuntime.previousTarget] = {entity->GetRefEHandle(), entity->GetModel(), entity->GetSpawnTime(),
			best.pRecord->flSimulationTime, hitbox::FromVector(best.vecPoint), bestRequested, true};
	}
	aimData.SetTarget(best.pRecord, vecEyePosition, best.bBacktrack);
	aimData.bSafe = best.bSafe;
	aimData.flDamage = best.flDamage;
	aimData.iHitbox = best.iHitbox;
	aimData.iHitGroup = best.iHitgroup;
	aimData.vecTargetShootPosition = best.vecPoint;
	return best.vecPoint;
}

bool CAimBot::AutoStop(std::vector<Lagcompensation::AnimationInfo_t*>& targets, CBaseEntity* pLocal, CUserCmd* pCmd) {
	if (!curConfig.bAutostop || pLocal->GetMoveType() != MOVETYPE_WALK)
		return false;

	const bool grounded = (pLocal->GetFlags() & FL_ONGROUND) && !(pCmd->iButtons & IN_JUMP);
	if (!grounded && !curConfig.bConditions[CONDITION_INAIR])
		return false;

	static auto* noSpread = i::ConVar->FindVar(XorStr("weapon_accuracy_nospread"));
	if (noSpread->GetInt() == 1)
		return false;
	const auto* data = curConfig.pWeapon->GetCSWpnData();
	if (!data || TICK_INTERVAL <= 0.f)
		return false;

	static auto* friction = i::ConVar->FindVar(XorStr("sv_friction"));
	static auto* stopSpeed = i::ConVar->FindVar(XorStr("sv_stopspeed"));
	static auto* accelerate = i::ConVar->FindVar(XorStr("sv_accelerate"));
	const float surface = (std::max)(0.f, pLocal->GetSurfaceFriction());
	const float drag = grounded ? (std::max)(0.f, friction->GetFloat()) * surface * TICK_INTERVAL : 0.f;
	const float control = (std::max)(0.f, stopSpeed->GetFloat());
	const float maxSpeed = data->flMaxSpeed[pLocal->IsScoped() ? 1 : 0];

	// CreateMove runs after speculative movement. Brake the saved command-start
	// velocity, since that is the state to which the final command will be applied.
	const auto saved = prediction.GetNetvars(pCmd->iCommandNumber);
	const bool baseline = saved.command == pCmd->iCommandNumber
		&& saved.epoch == prediction.GetEpoch()
		&& saved.localHandle == pLocal->GetRefEHandle() && saved.spawnTime == pLocal->GetSpawnTime();
	const Vector velocity = baseline ? saved.vecVelocity : pLocal->GetVelocity();
	const float speed = velocity.Length2D();

	float accelerationScale = (std::max)(250.f, maxSpeed);
	static auto* useWeaponSpeed = i::ConVar->FindVar(XorStr("sv_accelerate_use_weapon_speed"));
	if (useWeaponSpeed && useWeaponSpeed->GetInt())
		accelerationScale *= (std::min)(1.f, maxSpeed / 250.f);
	const bool ducking = (pCmd->iButtons & IN_DUCK) || (pLocal->GetFlags() & FL_DUCKING);
	const bool walking = !ducking && (pCmd->iButtons & IN_SPEED);
	// Slow scoped snipers keep their weapon acceleration while walking/ducking.
	const bool slowScoped = pLocal->IsScoped() && maxSpeed * .52f < 110.f;
	if (!slowScoped) {
		if (ducking) accelerationScale *= .34f;
		else if (walking) accelerationScale *= .52f;
	}
	const float counterStep = (std::max)(0.f, accelerate->GetFloat())
		* accelerationScale * surface * TICK_INTERVAL;

	// Simulate the same counter-strafe we will send, starting at the saved
	// position. Only a reachable end of the braking path may trigger pre-stop.
	Vector origin = baseline ? saved.vecOrigin : pLocal->GetVecOrigin();
	const Vector start = origin;
	float projectedSpeed = speed;
	int stopTicks = 0;
	const int horizons[] = { 16, 24, 32, 48 };
	const int phase = std::clamp(curConfig.iAutostopValue, 0, 3);
	const int limit = (std::max)(1, static_cast<int>(std::ceil(horizons[phase] / (64.f * TICK_INTERVAL))));
	const auto* collision = pLocal->GetCollideable();
	if (!collision) return false;
	CTraceFilter filter(pLocal);
	while (grounded && projectedSpeed > 1.f && stopTicks < limit) {
		projectedSpeed = (std::max)(0.f, projectedSpeed - (std::max)(projectedSpeed, control) * drag);
		if (projectedSpeed > counterStep) projectedSpeed -= counterStep;
		const Vector next = origin + Vector(velocity.x, velocity.y, 0.f)
			* (projectedSpeed / (std::max)(speed, 1.f)) * TICK_INTERVAL;
		Trace_t path;
		i::EngineTrace->TraceRay(Ray_t(origin, next, collision->OBBMins(), collision->OBBMaxs()), MASK_PLAYERSOLID, &filter, &path);
		if (path.bStartSolid || path.bAllSolid) return false;
		origin = path.vecEnd;
		++stopTicks;
		if (path.flFraction < 1.f) { projectedSpeed = 0.f; break; }
		// Do not project a grounded stop through a ledge or onto a steep slope.
		Trace_t floor;
		i::EngineTrace->TraceRay(Ray_t(origin, origin - Vector(0.f, 0.f, 2.f), collision->OBBMins(), collision->OBBMaxs()), MASK_PLAYERSOLID, &filter, &floor);
		if (floor.flFraction == 1.f || floor.plane.vecNormal.z < .7f) return false;
	}
	if (grounded && projectedSpeed > 1.f) return false;
	const float lead = (stopTicks + 2) * TICK_INTERVAL;
	const float now = TICKS_TO_TIME(pLocal->GetTickBase());
	float ready = (std::max)(pLocal->GetNextAttack(), curConfig.pWeapon->GetNextPrimaryAttack());
	if (curConfig.pWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
		ready = (std::max)(ready, curConfig.pWeapon->GetFireReadyTime());
	if (!aimData.bFireReady && !curConfig.bConditions[CONDITION_BETWEEN_SHOTS] && ready - now > lead)
		return false;

	if (!hasValidPoint) {
		if (!grounded || speed <= 1.f || (origin - start).Length2D() < .1f) return false;
		const Vector currentEye = vecEyePosition;
		const auto currentAim = aimData;
		// Use the normal hitboxes, minimum damage, penetration and safety checks.
		// A forecast can brake movement, but must never become an actual shot.
		vecEyePosition = currentEye + origin - pLocal->GetVecOrigin();
		const Vector point = ScanHitboxes(targets, pLocal);
		const bool predictedPoint = aimData.pRecord && point.IsValid()
			&& lagcomp.GetRecordTiming(aimData.flTargetSimulation).valid;
		aimData = currentAim;
		vecEyePosition = currentEye;
		if (!predictedPoint) return false;
	}

	pCmd->iButtons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
	pCmd->flForwardMove = pCmd->flSideMove = 0.f;
	const float remaining = (std::max)(0.f, speed - (std::max)(speed, control) * drag);
	// Near zero, let friction finish the stop. Full reverse input here would
	// overshoot and start accelerating in the opposite direction.
	if (speed <= 1.f || remaining <= counterStep)
		return true;

	Vector direction{}, forward{};
	M::VectorAngles(velocity, direction);
	direction.y = g::vecOriginalViewAngle.y - direction.y;
	M::AngleVectors(direction, &forward);
	static auto* forwardSpeed = i::ConVar->FindVar(XorStr("cl_forwardspeed"));
	static auto* sideSpeed = i::ConVar->FindVar(XorStr("cl_sidespeed"));
	pCmd->flForwardMove = -forward.x * forwardSpeed->GetFloat();
	pCmd->flSideMove = -forward.y * sideSpeed->GetFloat();
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
	const int requiredHits = static_cast<int>(std::ceil(curConfig.iHitchance * iAccuracry / 100.f));
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
		if (iHits + iAccuracry - i - 1 < requiredHits) return false;

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
	targetDiagnostics={};

	std::vector<Lagcompensation::AnimationInfo_t*> ret{};

	CCSWeaponInfo* pWeaponData = curConfig.pWeapon->GetCSWpnData( );
	if ( !pWeaponData )
		return ret;

	for ( int i = 1; i <= (std::min)(64,i::GlobalVars->nMaxClients); i++ ) {

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
		auto& diagnosis=targetDiagnostics[i];
		backtrack::HistoryWalk history(hitbox::FromVector(pEntity->GetVecOrigin()));
		for ( auto j = 0u; j < pLog->pRecord.size( ); j++ ) {

			Lagcompensation::LagRecord_t* pCurrentRecord = &pLog->pRecord.at( j );
			++diagnosis.records;
			const bool chain=history.Visit(hitbox::FromVector(pCurrentRecord->vecOrigin),!pCurrentRecord->bDormant,
				pCurrentRecord->bBreakingLagcompensation);
			const auto timing=lagcomp.GetRecordTiming(pCurrentRecord->flSimulationTime);
			pCurrentRecord->bValid=chain && pCurrentRecord->bAnimationMatrixValid && timing.valid;
			if (!chain) ++diagnosis.historyRejected;
			else if (!pCurrentRecord->bAnimationMatrixValid) ++diagnosis.matrixRejected;
			else if (!timing.valid) ++diagnosis.timingRejected;
			else ++diagnosis.validRecords;

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

hitscan::Points<Vector> CAimBot::CreatePoints(Vector vecEyePosition, CBaseCombatWeapon* pWeapon, Lagcompensation::LagRecord_t* pRecord,
    int iHitbox, EMatrixType iType, bool bShouldMultipoint) {
    hitscan::Points<Vector> points;
    if (!pRecord || !pRecord->pEntity || !vecEyePosition.IsValid() || iHitbox<0 || iHitbox>=HITBOX_MAX
        || iType<VISUAL || iType>=MAX || (iType==VISUAL ? !pRecord->bVisualMatrixValid : !pRecord->bAnimationMatrixValid)) return points;
    const auto shape=hitbox::FromStudio(pRecord->pEntity->StudioHitbox(iHitbox),pRecord->pMatricies[iType]);
    if (!multipoint::Valid(shape)) return points;
    const auto append=[&](hitbox::Vec candidate) {
        const Vector point=hitbox::ToVector(candidate);
        // Recheck after float conversion, including points at large map coordinates.
        if (!point.IsValid() || !shape.Contains(hitbox::FromVector(point))) return;
        for (const auto& previous:points) if ((previous-point).LengthSqr()<1e-6f) return;
        points.push_back(point);
    };
    if (iHitbox==HITBOX_HEAD && iType==RESOLVE && pRecord->bHasHeadOverlap
        && pRecord->iResolverMode==static_cast<int>(resolver::Mode::Experimental))
        append(hitbox::FromVector(pRecord->vecRageSafePoint));
    if (!bShouldMultipoint) { append(shape.Center()); return points; }
    const auto region=iHitbox==HITBOX_HEAD ? multipoint::Region::Head
        : iHitbox==HITBOX_LEFT_FOOT || iHitbox==HITBOX_RIGHT_FOOT ? multipoint::Region::Foot
        : iHitbox>=HITBOX_RIGHT_THIGH && iHitbox<=HITBOX_LEFT_CALF ? multipoint::Region::Leg : multipoint::Region::Body;
    const int scale=iHitbox==HITBOX_HEAD ? curConfig.iHeadScale : curConfig.iBodyScale;
    for (const auto candidate:multipoint::Generate(shape,hitbox::FromVector(vecEyePosition),region,scale)) append(candidate);
    return points;
}
