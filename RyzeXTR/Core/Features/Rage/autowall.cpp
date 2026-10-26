#include "autowall.h"
#include "../../globals.h"
#include "../Misc/Playerlist.h"
#include "Animations/Lagcompensation.h"
#include "StudioHitboxGeometry.h"

float CAutoWall::GetDamage( CBaseEntity* pLocal, const Vector& vecEyePosition, const Vector& vecPoint, CBaseCombatWeapon* pWeapon, Lagcompensation::LagRecord_t* pRecord, FireBulletData_t* pDataOut, const hitscan::DamageQuery* query)
{
	if (!pLocal || !pWeapon || !pRecord || !pRecord->pEntity || !vecEyePosition.IsValid() || !vecPoint.IsValid()) {
		if (pDataOut) { *pDataOut = {}; pDataOut->failure = hitscan::Failure::Invalid; }
		return -1.0f;
	}

	// setup data
	FireBulletData_t data = { };
	data.vecPosition = vecEyePosition;
	data.vecDirection = ( vecPoint - vecEyePosition).Normalized( );
	data.iPenetrateCount = 4;

	if (!data.vecDirection.IsValid() || data.vecDirection.LengthSqr() <= 0.f) {
		data.failure = hitscan::Failure::Invalid;
		if (pDataOut) *pDataOut = data;
		return -1.f;
	}
	const bool hit = SimulateFireBullet(pLocal, pWeapon, data, pRecord, query);

	if ( pDataOut != nullptr )
		*pDataOut = data;

	return hit ? data.flCurrentDamage : -1.f;
}

float CAutoWall::GetDamageUpperBound(const Vector& source, CBaseCombatWeapon* weapon, Lagcompensation::LagRecord_t* record, const Vector* direction) {
	const float unknown = std::numeric_limits<float>::infinity();
	if (!weapon || !record || !record->pEntity || !source.IsValid()) return unknown;
	const auto data = i::WeaponSystem->GetWpnData(weapon->GetItemDefinitionIndex());
	if (!data || data->iDamage < 0 || !std::isfinite(data->flArmorRatio) || data->flArmorRatio < 0.f
		|| !std::isfinite(data->flHeadShotMultiplier) || data->flHeadShotMultiplier < 0.f) return unknown;

	double distance = 0.; // Unsupported model/transformation keeps the looser bound.
	const auto model = record->pEntity->GetModel();
	const int hitboxSet = record->pEntity->GetHitboxSet();
	auto& geometry = record->damageGeometry;
	if (!geometry.computed || geometry.model != model || geometry.hitboxSet != hitboxSet) {
		geometry = {};
		geometry.computed = true;
		geometry.model = model;
		geometry.hitboxSet = hitboxSet;
		const auto studio = model ? i::ModelInfo->GetStudioModel(model) : nullptr;
		const auto set = studio ? studio->GetHitboxSet(hitboxSet) : nullptr;
		const int bones = record->pEntity->GetCachedBoneData().Count();
		if (set && set->nHitboxes > 0 && set->nHitboxes <= static_cast<int>(geometry.shapes.size())) {
			geometry.count = set->nHitboxes;
			for (int index = 0; index < set->nHitboxes; ++index) {
				const auto box = set->GetHitbox(index);
				if (!box || box->iBone < 0 || box->iBone >= bones) { geometry.count = 0; break; }
				geometry.shapes[index] = hitbox::FromStudio(box, record->pMatricies[RESOLVE]);
				geometry.hitgroups[index] = box->iGroup;
				if (!geometry.shapes[index].valid) { geometry.count = 0; break; }
			}
		}
	}
	std::array<bool, HITGROUP_GEAR + 1> groups{};
	bool restrictGroups = direction && direction->IsValid() && direction->LengthSqr() > 0.f && geometry.count > 0;
	if (geometry.count > 0) {
		distance = std::numeric_limits<double>::infinity();
		const auto start = hitbox::FromVector(source);
		hitbox::Vec end;
		if (restrictGroups) {
			const auto ray = hitbox::FromVector(*direction);
			double length = 1.;
			for (int index = 0; index < geometry.count; ++index) {
				const auto& shape = geometry.shapes[index];
				const double size = shape.capsule ? std::sqrt((shape.b - shape.a).LengthSq()) + shape.radius
					: shape.extents[0] + shape.extents[1] + shape.extents[2];
				length = (std::max)(length, std::sqrt((shape.Center() - start).LengthSq()) + size + 1.);
			}
			end = start + ray * (length / std::sqrt(ray.LengthSq()));
		}
		for (int index = 0; index < geometry.count; ++index) {
			const auto& shape = geometry.shapes[index];
			if (restrictGroups) {
				auto expanded = shape;
				const auto center = shape.Center();
				const double tolerance = .05 + ((std::max)({std::fabs(start.x), std::fabs(start.y), std::fabs(start.z),
					std::fabs(center.x), std::fabs(center.y), std::fabs(center.z)})) * .000001;
				if (expanded.capsule) expanded.radius += tolerance;
				else for (auto& extent : expanded.extents) extent += tolerance;
				if (!expanded.Intersects(start, end)) continue;
				const int group = geometry.hitgroups[index];
				if (group < 0 || group > HITGROUP_GEAR) { restrictGroups = false; distance = 0.; break; }
				groups[group] = true;
			}
			distance = (std::min)(distance, std::sqrt((start - shape.Project(start, 0.)).LengthSq()));
		}
		if (restrictGroups && !std::isfinite(distance)) return 0.f;
	}
	// Glass/grates lose at least 5%; teammate flesh can have a smaller custom
	// loss. A zero flesh setting blocks penetration rather than granting damage.
	static auto flesh = i::ConVar->FindVar("ff_damage_bullet_penetration");
	const float fleshLoss = flesh ? flesh->GetFloat() : -1.f;
	const double loss = !std::isfinite(fleshLoss) || fleshLoss < 0.f ? 0.
		: fleshLoss > 0.f ? (std::min)(.05, static_cast<double>(fleshLoss)) : .05;
	const double raw = hitscan::RawDamageBound(data->iDamage, data->flRangeModifier, distance, loss);
	if (!std::isfinite(raw) || raw > (std::numeric_limits<float>::max)()) return unknown;
	float maximum = restrictGroups ? 0.f : static_cast<float>(raw);
	// The ray can hit another target hitbox before the requested point. Bound
	// all accepted hitgroups, including the unscaled unknown-group fallback.
	for (int group = HITGROUP_HEAD; group < HITGROUP_GEAR; ++group) {
		if (restrictGroups && !groups[group]) continue;
		float scaled = static_cast<float>(raw);
		ScaleDamage(group, record->pEntity, data->flArmorRatio, data->flHeadShotMultiplier, scaled);
		if (!std::isfinite(scaled)) return unknown;
		maximum = (std::max)(maximum, scaled);
	}
	return maximum + maximum * .0001f + .01f;
}

void CAutoWall::ScaleDamage( const int iHitGroup, CBaseEntity* pEntity, const float flWeaponArmorRatio, const float flWeaponHeadShotMultiplier, float& flDamage )
{
	// @ida traceattack: server.dll @ 55 8B EC 83 E4 F8 81 EC C0 00 00 00 56 8B 75 08 57 8B F9 C6 44 24 13 01

	const bool bHeavyArmor = pEntity->HasHeavyArmor( );

	static CConVar* mp_damage_scale_ct_head = i::ConVar->FindVar(XorStr("mp_damage_scale_ct_head"));
	static CConVar* mp_damage_scale_t_head = i::ConVar->FindVar(XorStr("mp_damage_scale_t_head"));

	static CConVar* mp_damage_scale_ct_body = i::ConVar->FindVar(XorStr("mp_damage_scale_ct_body"));
	static CConVar* mp_damage_scale_t_body = i::ConVar->FindVar(XorStr("mp_damage_scale_t_body"));

	float flHeadDamageScale = pEntity->GetTeam( ) == TEAM_CT ? mp_damage_scale_ct_head->GetFloat( ) : pEntity->GetTeam( ) == TEAM_TT ? mp_damage_scale_t_head->GetFloat( ) : 1.0f;
	const float flBodyDamageScale = pEntity->GetTeam( ) == TEAM_CT ? mp_damage_scale_ct_body->GetFloat( ) : pEntity->GetTeam( ) == TEAM_TT ? mp_damage_scale_t_body->GetFloat( ) : 1.0f;

	if ( bHeavyArmor )
		flHeadDamageScale *= 0.5f;

	switch ( iHitGroup )
	{
	case HITGROUP_HEAD:
		flDamage *= flWeaponHeadShotMultiplier * flHeadDamageScale;
		break;
	case HITGROUP_CHEST:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
	case HITGROUP_NECK:
		flDamage *= flBodyDamageScale;
		break;
	case HITGROUP_STOMACH:
		flDamage *= 1.25f * flBodyDamageScale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		flDamage *= 0.75f * flBodyDamageScale;
		break;
	default:
		break;
	}

	if ( pEntity->IsArmored( iHitGroup ) )
	{
		// @ida ontakedamage: server.dll @ 80 BF ? ? ? ? ? F3 0F 10 5C 24 ? F3 0F 10 35

		const int iArmor = pEntity->GetArmor( );
		float flHeavyArmorBonus = 1.0f, flArmorBonus = 0.5f, flArmorRatio = flWeaponArmorRatio * 0.5f;

		if ( bHeavyArmor )
		{
			flHeavyArmorBonus = 0.25f;
			flArmorBonus = 0.33f;
			flArmorRatio *= 0.20f;
		}

		float flDamageToHealth = flDamage * flArmorRatio;
		if ( const float flDamageToArmor = ( flDamage - flDamageToHealth ) * ( flHeavyArmorBonus * flArmorBonus ); flDamageToArmor > static_cast< float >( iArmor ) )
			flDamageToHealth = flDamage - static_cast< float >( iArmor ) / flArmorBonus;

		flDamage = flDamageToHealth;
	}
}

// @credits: https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/game/shared/util_shared.cpp#L757
void CAutoWall::ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, const unsigned int fMask, CTraceFilter* pFilter, Trace_t* pTrace, Lagcompensation::LagRecord_t* pRecord, const float flMinRange)
{
	// @ida util_cliptracetoplayers: client.dll @ E8 ? ? ? ? 0F 28 84 24 68 02 00 00
	if (i::ClientState->iDeltaTick <= 0)
		return;

	Trace_t trace = { };
	float flSmallestFraction = pTrace->flFraction;

	const Ray_t ray( vecAbsStart, vecAbsEnd );

	if ( pFilter != nullptr && !pFilter->ShouldHitEntity( pRecord->pEntity, fMask ) )
		return;

	// get bounding box
	const Vector vecMin = pRecord->vecMins;
	const Vector vecMax = pRecord->vecMaxs;

	// calculate world space center
	const Vector vecCenter = ( vecMax + vecMin ) * 0.5f;
	const Vector vecPosition = vecCenter + pRecord->vecOrigin;

	const Vector vecTo = vecPosition - vecAbsStart;
	Vector vecDirection = vecAbsEnd - vecAbsStart;
	const float flLength = vecDirection.NormalizeInPlace( );

	const float flRangeAlong = vecDirection.DotProduct( vecTo );
	float flRange = 0.0f;

	// calculate distance to ray
	if ( flRangeAlong < 0.0f )
		// off start point
		flRange = -vecTo.Length( );
	else if ( flRangeAlong > flLength )
		// off end point
		flRange = -( vecPosition - vecAbsEnd ).Length( );
	else
		// within ray bounds
		flRange = ( vecPosition - ( vecDirection * flRangeAlong + vecAbsStart ) ).Length( );

	constexpr float flMaxRange = 60.f;
	if ( flRange < flMinRange || flRange > flMaxRange )
		return;

	i::EngineTrace->ClipRayToEntity( ray, fMask | CONTENTS_HITBOX, pRecord->pEntity, &trace );

	if ( trace.flFraction < flSmallestFraction )
	{
		// we shortened the ray - save off the trace
		*pTrace = trace;
		flSmallestFraction = trace.flFraction;
	}
}

bool CAutoWall::TraceToExit( Trace_t& enterTrace, Trace_t& exitTrace, const Vector& vecPosition, const Vector& vecDirection, const CBaseEntity* pClipPlayer, Lagcompensation::LagRecord_t* pRecord, FireBulletData_t* bullet, const hitscan::DamageQuery* query)
{
	// @ida tracetoexit: client.dll @ 55 8B EC 83 EC 4C F3
	// server.dll @ 55 8B EC 83 EC 4C F3 0F 10 75

	float flDistance = 0.0f;
	int iStartContents = 0;
	bool startContentsKnown = false;

	while ( flDistance <= 90.0f )
	{
		if (query && query->Expired()) {
			if (bullet) bullet->failure = hitscan::Failure::Deadline;
			return false;
		}
		if (bullet) ++bullet->exitSteps;
		// add extra distance to our ray
		flDistance += 4.0f;

		// multiply the direction vector to the distance so we go outwards, add our position to it
		Vector vecStart = vecPosition + vecDirection * flDistance;

		const int iCurrentContents = i::EngineTrace->GetPointContents_WorldOnly( vecStart, MASK_SHOT_HULL | CONTENTS_HITBOX/*, nullptr*/ );
		if (!startContentsKnown) { iStartContents = iCurrentContents; startContentsKnown = true; }

		if ( !( iCurrentContents & MASK_SHOT_HULL ) || ( iCurrentContents & CONTENTS_HITBOX && iCurrentContents != iStartContents ) )
		{
			// setup our end position by deducting the direction by the extra added distance
			const Vector vecEnd = vecStart - ( vecDirection * 4.0f );

			// trace ray to world
			Ray_t rayWorld( vecStart, vecEnd );
			if (bullet) ++bullet->traceCalls;
			i::EngineTrace->TraceRay( rayWorld, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace );

			if (static CConVar* sv_clip_penetration_traces_to_players = i::ConVar->FindVar(XorStr("sv_clip_penetration_traces_to_players")); sv_clip_penetration_traces_to_players != nullptr && sv_clip_penetration_traces_to_players->GetBool())
			{
				CTraceFilter filter( pClipPlayer );
				ClipTraceToPlayers( vecEnd, vecStart, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &exitTrace, pRecord, -60.f );
			}

			// check if a hitbox is in-front of our enemy and if they are behind of a solid wall
			if ( exitTrace.bStartSolid && exitTrace.surface.uFlags & SURF_HITBOX )
			{
				// trace ray to entity
				Ray_t ray( vecStart, vecPosition );
				CTraceFilter filter( exitTrace.pHitEntity );

				if (bullet) ++bullet->traceCalls;
				i::EngineTrace->TraceRay( ray, MASK_SHOT_HULL, &filter, &exitTrace );

				if ( exitTrace.DidHit( ) && !exitTrace.bStartSolid )
				{
					vecStart = exitTrace.vecEnd;
					return true;
				}

				continue;
			}

			if ( exitTrace.DidHit( ) && !exitTrace.bStartSolid )
			{
				if (enterTrace.pHitEntity &&
					exitTrace.pHitEntity &&
					enterTrace.pHitEntity->IsBreakable() &&
					exitTrace.pHitEntity->IsBreakable())
				{
					return true;
				}

				if ( enterTrace.surface.uFlags & SURF_NODRAW || ( !( exitTrace.surface.uFlags & SURF_NODRAW ) && exitTrace.plane.vecNormal.DotProduct( vecDirection ) <= 1.0f ) )
				{
					const float flMultiplier = exitTrace.flFraction * 4.0f;
					vecStart -= vecDirection * flMultiplier;
					return true;
				}

				continue;
			}

			if ( !exitTrace.DidHit( ) || exitTrace.bStartSolid )
			{
				if ( enterTrace.pHitEntity != nullptr && enterTrace.pHitEntity->EntIndex( ) != 0 && enterTrace.pHitEntity->IsBreakable( ) )
				{
					// did hit breakable non world entity
					exitTrace = enterTrace;
					exitTrace.vecEnd = vecStart + vecDirection;
					return true;
				}

				continue;
			}
		}
	}

	if (bullet) bullet->failure = hitscan::Failure::NoExit;
	return false;
}

bool CAutoWall::HandleBulletPenetration( CBaseEntity* pLocal, const CCSWeaponInfo* pWeaponData, const surfacedata_t* pEnterSurfaceData, FireBulletData_t& data, Lagcompensation::LagRecord_t* pRecord, const hitscan::DamageQuery* query)
{
	// @ida handlebulletpenetration: client.dll @ E8 ? ? ? ? 83 C4 40 84 C0

	static CConVar* ff_damage_reduction_bullets = i::ConVar->FindVar(XorStr("ff_damage_reduction_bullets"));
	static CConVar* ff_damage_bullet_penetration = i::ConVar->FindVar(XorStr("ff_damage_bullet_penetration"));

	const float flReductionDamage = ff_damage_reduction_bullets->GetFloat( );
	const float flPenetrateDamage = ff_damage_bullet_penetration->GetFloat( );

	const MaterialHandle_t hEnterMaterial = pEnterSurfaceData->game.hMaterial;

	if ( data.iPenetrateCount == 0 && hEnterMaterial != CHAR_TEX_GRATE && hEnterMaterial != CHAR_TEX_GLASS && !( data.enterTrace.surface.uFlags & SURF_NODRAW ) )
		return false;

	if ( pWeaponData->flPenetration <= 0.0f || data.iPenetrateCount <= 0 )
		return false;

	Trace_t exitTrace = { };
	// A failed exit must not use an uninitialized surface/exit position. In
	// particular, a deadline interruption is unknown, never a blocked-ray cache.
	if (!TraceToExit(data.enterTrace, exitTrace, data.enterTrace.vecEnd, data.vecDirection, pLocal, pRecord, &data, query))
		return false;

	const surfacedata_t* pExitSurfaceData = i::PhysicsProps->GetSurfaceData( exitTrace.surface.nSurfaceProps );
	if (!pExitSurfaceData) { data.failure = hitscan::Failure::Invalid; return false; }
	const MaterialHandle_t hExitMaterial = pExitSurfaceData->game.hMaterial;

	const float flEnterPenetrationModifier = pEnterSurfaceData->game.flPenetrationModifier;
	const float flExitPenetrationModifier = pExitSurfaceData->game.flPenetrationModifier;

	float flDamageLostModifier = 0.16f;
	float flPenetrationModifier = 0.0f;

	if ( hEnterMaterial == CHAR_TEX_GRATE || hEnterMaterial == CHAR_TEX_GLASS )
	{
		flDamageLostModifier = 0.05f;
		flPenetrationModifier = 3.0f;
	}
	else if ( ( ( data.enterTrace.iContents >> 3 ) & CONTENTS_SOLID ) || ( ( data.enterTrace.surface.uFlags >> 7 ) & SURF_LIGHT ) )
	{
		flDamageLostModifier = 0.16f;
		flPenetrationModifier = 1.0f;
	}
	else if ( hEnterMaterial == CHAR_TEX_FLESH && flReductionDamage == 0.0f && data.enterTrace.pHitEntity != nullptr && data.enterTrace.pHitEntity->IsPlayer( ) && ( !pLocal->IsEnemy(data.enterTrace.pHitEntity) ) )
	{
		if ( flPenetrateDamage == 0.0f )
			return false;

		// shoot through teammates
		flDamageLostModifier = flPenetrateDamage;
		flPenetrationModifier = flPenetrateDamage;
	}
	else
	{
		flDamageLostModifier = 0.16f;
		flPenetrationModifier = ( flEnterPenetrationModifier + flExitPenetrationModifier ) * 0.5f;
	}

	if ( hEnterMaterial == hExitMaterial )
	{
		if ( hExitMaterial == CHAR_TEX_CARDBOARD || hExitMaterial == CHAR_TEX_WOOD )
			flPenetrationModifier = 3.0f;
		else if ( hExitMaterial == CHAR_TEX_PLASTIC )
			flPenetrationModifier = 2.0f;
	}

	const float flTraceDistance = ( exitTrace.vecEnd - data.enterTrace.vecEnd ).LengthSqr( );

	// penetration modifier
	const float flModifier = ( flPenetrationModifier > 0.0f ? 1.0f / flPenetrationModifier : 0.0f );

	// this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
	const float flLostDamage = ( data.flCurrentDamage * flDamageLostModifier + ( pWeaponData->flPenetration > 0.0f ? 3.75f / pWeaponData->flPenetration : 0.0f ) * ( flModifier * 3.0f ) ) + ( ( flModifier * flTraceDistance ) / 24.0f );

	// did we loose too much damage?
	if ( flLostDamage > data.flCurrentDamage )
		return false;

	// we can't use any of the damage that we've lost
	if ( flLostDamage > 0.0f )
		data.flCurrentDamage -= flLostDamage;

	// do we still have enough damage to deal?
	if ( data.flCurrentDamage < 1.0f )
		return false;

	data.vecPosition = exitTrace.vecEnd;
	--data.iPenetrateCount;
	return true;
}

bool CAutoWall::CanReachDamage(float raw, const CCSWeaponInfo* weapon, CBaseEntity* target, float minimum) {
	if (minimum <= 0.f || !weapon || !target || !std::isfinite(raw) || raw < 0.f
		|| !std::isfinite(weapon->flRangeModifier) || weapon->flRangeModifier <= 0.f || weapon->flRangeModifier > 1.f)
		return true;
	float maximum = raw;
	for (int group = HITGROUP_HEAD; group < HITGROUP_GEAR; ++group) {
		float scaled = raw;
		ScaleDamage(group, target, weapon->flArmorRatio, weapon->flHeadShotMultiplier, scaled);
		if (!std::isfinite(scaled)) return true;
		maximum = (std::max)(maximum, scaled);
	}
	return maximum + maximum * .0001f + .01f >= minimum;
}

bool CAutoWall::SimulateFireBullet( CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, FireBulletData_t& data, Lagcompensation::LagRecord_t* pRecord, const hitscan::DamageQuery* query)
{
	// @ida firebullet: client.dll @ 55 8B EC 83 E4 F0 81 EC ? ? ? ? F3 0F 7E

	if (!pWeapon || !pLocal || !pRecord || !pRecord->pEntity) {
		data.failure = hitscan::Failure::Invalid; return false;
	}

	CCSWeaponInfo* pWeaponData = i::WeaponSystem->GetWpnData( pWeapon->GetItemDefinitionIndex( ) );

	if ( pWeaponData == nullptr ) {
		data.failure = hitscan::Failure::Invalid; return false;
	}

	const float flMaxRange = pWeaponData->flRange;

	// the total number of surfaces any bullet can penetrate in a single flight is capped at 4
	if (data.iPenetrateCount > 4 || data.iPenetrateCount == 0)
		data.iPenetrateCount = 4;
	// set our current damage to what our gun's initial damage reports it will do
	data.flCurrentDamage = static_cast< float >( pWeaponData->iDamage );

	float flTraceLength = 0.0f;
	CTraceFilter filter( pLocal );

	while ( data.iPenetrateCount > 0 && data.flCurrentDamage >= 1.0f )
	{
		if (query && query->Expired()) { data.failure = hitscan::Failure::Deadline; return false; }
		if (query && !CanReachDamage(data.flCurrentDamage, pWeaponData, pRecord->pEntity, query->minimumDamage)) {
			data.failure = hitscan::Failure::LowDamage; return false;
		}
		const float flRemainingRange = flMaxRange - flTraceLength;

		if (flRemainingRange <= 0.0f)
			break;

		const Vector vecEnd =
			data.vecPosition + data.vecDirection * flRemainingRange;

		constexpr unsigned int kTraceMask =
			MASK_SHOT_HULL | CONTENTS_HITBOX;

		Ray_t ray(data.vecPosition, vecEnd);

		++data.traceCalls;
		i::EngineTrace->TraceRay(
			ray, kTraceMask, &filter, &data.enterTrace
		);

		// Separate extended player trace
		Trace_t playerTrace{};
		playerTrace.flFraction = 1.0f;

		constexpr float kPlayerExtension = 40.0f;

		ClipTraceToPlayers(
			data.vecPosition,
			vecEnd + data.vecDirection * kPlayerExtension,
			kTraceMask,
			&filter,
			&playerTrace,
			pRecord,
			0.0f
		);

		if (playerTrace.flFraction < 1.0f)
		{
			const float flCorrectedFraction =
				playerTrace.flFraction *
				((flRemainingRange + kPlayerExtension) / flRemainingRange);

			if (flCorrectedFraction < data.enterTrace.flFraction)
			{
				data.enterTrace = playerTrace;
				data.enterTrace.flFraction = flCorrectedFraction;
			}
		}

		// we didn't hit anything, stop tracing shoot
		if (data.enterTrace.flFraction >= 1.0f)
			break;

		const surfacedata_t* pEnterSurfaceData =
			i::PhysicsProps->GetSurfaceData(
				data.enterTrace.surface.nSurfaceProps
			);

		if (!pEnterSurfaceData)
			return false;

		const float flEnterPenetrationModifier =
			pEnterSurfaceData->game.flPenetrationModifier;

		// calculate the damage based on the distance the bullet traveled
		const float flSegmentLength =
			data.enterTrace.flFraction * flRemainingRange;

		flTraceLength += flSegmentLength;

		data.flCurrentDamage *= std::powf(
			pWeaponData->flRangeModifier,
			flSegmentLength / MAX_DAMAGE
		);

		// check is actually can shoot through
		if (flTraceLength > 3000.f || flEnterPenetrationModifier < 0.1f )
			break;

		// check is can do damage
		if (pRecord &&
			data.enterTrace.pHitEntity == pRecord->pEntity &&
			data.enterTrace.iHitGroup != HITGROUP_GENERIC &&
			data.enterTrace.iHitGroup != HITGROUP_GEAR &&
			pLocal->IsEnemy(data.enterTrace.pHitEntity))
		{
			// we got target - scale damage
			ScaleDamage( data.enterTrace.iHitGroup, data.enterTrace.pHitEntity, pWeaponData->flArmorRatio, pWeaponData->flHeadShotMultiplier, data.flCurrentDamage );
			return true;
		}

		if (query && !CanReachDamage(data.flCurrentDamage, pWeaponData, pRecord->pEntity, query->minimumDamage)) {
			data.failure = hitscan::Failure::LowDamage; return false;
		}
		// calling handlebulletpenetration here reduces our penetration �ounter, and if it returns true, we can't shoot through it
		if ( !HandleBulletPenetration( pLocal, pWeaponData, pEnterSurfaceData, data, pRecord, query) )
			break;
	}

	if (data.failure == hitscan::Failure::None) data.failure = hitscan::Failure::NoHit;
	return false;
}

bool CAutoWall::CanHitFloatingPoint(const Vector& vecPoint, const Vector& vecSource, Lagcompensation::LagRecord_t* pRecord) {

	if (!g::pLocal)
		return false;

	FireBulletData_t data = FireBulletData_t(vecSource, vecPoint);

	Vector vecAngles = M::CalcAngle(data.vecPosition, vecPoint);
	M::AngleVectors(vecAngles, &data.vecDirection);
	data.vecDirection.Normalize();

	if (!g::pLocal->GetWeapon())
		return false;

	data.iPenetrateCount = 1;
	float flTraceLength = 0.0f;

	CCSWeaponInfo* pWeaponData = g::pLocal->GetWeapon()->GetCSWpnData();

	if (!pWeaponData)
		return false;

	data.flCurrentDamage = static_cast<float>(pWeaponData->iDamage);
	float flTraceLengthRemaining = pWeaponData->flRange - flTraceLength;

	Vector vecEnd = data.vecPosition + data.vecDirection * flTraceLengthRemaining;

	CTraceFilter filter(g::pLocal);
	i::EngineTrace->TraceRay(Ray_t(data.vecPosition, vecEnd), MASK_SHOT | CONTENTS_HITBOX, &filter, &data.enterTrace);
	const surfacedata_t* pEnterSurfaceData = i::PhysicsProps->GetSurfaceData(data.enterTrace.surface.nSurfaceProps);

	if (data.enterTrace.flFraction == 1.f)
		return true;

	if (HandleBulletPenetration(g::pLocal, pWeaponData, pEnterSurfaceData, data, pRecord)) {
		return true;
	}

	return false;
}

#pragma runtime_checks( "", off )
bool CAutoWall::bCollidePoint(const Vector& vecStart, const Vector& vecEnd, mstudiobbox_t* pHitbox, matrix3x4a_t* aMatrix) {
	if (!pHitbox || !aMatrix || pHitbox->iBone < 0 || pHitbox->iBone >= MAXSTUDIOBONES)
		return false;
	if (resolver::GetMode(cfg::rage::iResolverMode) == resolver::Mode::Experimental) {
		const auto shape = hitbox::FromStudio(pHitbox, aMatrix);
		if (shape.valid) return shape.Intersects(hitbox::FromVector(vecStart), hitbox::FromVector(vecEnd));
	}

	Ray_t ray(vecStart, vecEnd);
	Trace_t trace;
	trace.flFraction = 1.0f;
	trace.bStartSolid = false;

	// Define a typedef for the ClipRayToHitbox function prototype
	using ClipRayToHitbox_t = int(__fastcall*)(const Ray_t&, mstudiobbox_t*, matrix3x4a_t&, Trace_t*);

	// Example pattern: "55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F"
	static auto clipRayToHitbox = reinterpret_cast<ClipRayToHitbox_t>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 F3 ? ? ? ? 81 ? ? ? ? ? 0F")));

	if (!clipRayToHitbox)
		return false;

	int hitIndex = clipRayToHitbox(ray, pHitbox, aMatrix[pHitbox->iBone], &trace);
	return hitIndex >= 0;
}
#pragma runtime_checks( "", restore )

int CAutoWall::SafePoint(Vector& vecEyePosition, Lagcompensation::LagRecord_t* pRecord, Vector vecShootposition, int iHitbox) {
	if (!pRecord || !pRecord->pEntity || !pRecord->bAnimationMatrixValid) return 0;
	if (pRecord->iResolverMode == static_cast<int>(resolver::Mode::Experimental) && !pRecord->bSafeMatrixValid) return 0;
	if (pRecord->iResolverMode == static_cast<int>(resolver::Mode::Experimental) && iHitbox == HITBOX_HEAD) {
		if (pRecord->iHeadHypothesisCount != static_cast<int>(hitbox::MaxShapes)) return 0;
		int hits = 0;
		for (const auto& shape : pRecord->headHypotheses)
			hits += shape.Intersects(hitbox::FromVector(vecEyePosition), hitbox::FromVector(vecShootposition));
		// Preserve the public 0..3 coverage scale. Full safety requires ALL
		// sampled heads, including the actual selected matrix, to intersect.
		return hits * 3 / pRecord->iHeadHypothesisCount;
	}

	mstudiobbox_t* studioBox = pRecord->pEntity->StudioHitbox(iHitbox);
	if (!studioBox)
		return 0;  // Skip this hitbox if it's invalid

	// Safepoint count — check how many desync matrices (LEFT, RIGHT, CENTER) the ray intersects
	// bCollidePoint uses ClipRayToHitbox which traces against the actual studio hitbox per matrix,
	// making the old bTraceMeantForHitbox (ClipRayToCollideable, physics hull) redundant and overly
	// restrictive: it rejected head points whenever the body hull occluded the head (enemy facing away).
	int iSafePoint = 0;
	for ( int iSafeSide = EMatrixType::LEFT; iSafeSide <= EMatrixType::CENTER; iSafeSide++) {

		// Increment iSafePoint based on the collidepoint results
		iSafePoint += autowall.bCollidePoint(vecEyePosition, vecShootposition, studioBox, pRecord->pMatricies[iSafeSide]);
	}
	return iSafePoint;
}

bool CAutoWall::bTraceMeantForHitbox(const Vector& vecEyePosition, const Vector& vecEnd, int iHitbox, Lagcompensation::LagRecord_t* pRecord, EMatrixType iMatrix) {

	// Initialize our trace data & information
	Trace_t traceData;
	Ray_t traceRay = Ray_t(vecEyePosition, vecEnd);

	// Apply bone cache for accurate tracing
	if (RESOLVE != iMatrix)
		pRecord->ApplyMatrix(pRecord->pEntity, iMatrix);

	// get collideable
	ICollideable* pCollideable = i::EngineTrace->GetCollideable(pRecord->pEntity);

	// trace a ray to the entity
	i::EngineTrace->ClipRayToCollideable(traceRay, MASK_SHOT, pCollideable, &traceData);

	// Restore
	if (RESOLVE != iMatrix)
		pRecord->ApplyMatrix(pRecord->pEntity, RESOLVE);

	// check if trace did hit the desired hitbox and not other
	// example: aiming for head -> its behind his chest -> return chest == head
	return traceData.iHitbox == iHitbox;
}
