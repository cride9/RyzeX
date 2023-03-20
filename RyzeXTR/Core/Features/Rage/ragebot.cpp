#include "ragebot.h"
#include "../../globals.h"
#include "autowall.h"
#include "../../SDK/math.h"
#include "../Visuals/ESP.h"
#include "exploits.h"

bool CheckShootingCondition(CUserCmd* pCmd);

bool LowestHealth(CBaseEntity* pEnt1, CBaseEntity* pEnt2) {
	return pEnt1->GetHealth() < pEnt2->GetHealth();
}

bool HighestDamage(std::pair<Vector, float> damage1, std::pair<Vector, float> damage2) {
	return damage1.second > damage2.second;
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

void CRageBot::CreateMove(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket) {

	if (!pLocal || !cfg::rage::enable)
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (!pWeapon || !pWeapon->GetCSWpnData())
		return;

	Vector vecEyePosition = pLocal->GetEyePosition();

	if (CBaseEntity* pTarget = SelectTarget(pLocal, pWeapon, vecEyePosition); pTarget != nullptr) {

		if (Vector vecHitscan = Hitscan(pLocal, pTarget, pWeapon, vecEyePosition); vecHitscan != Vector(0, 0, 0)) {

			Vector shootAngle = M::CalcAngle(vecEyePosition, vecHitscan);

			if (cfg::rage::autostop && cfg::rage::betweenshots)
				AutoStop(pCmd, pWeapon->GetCSWpnData()->flMaxSpeed[0] * 0.10f);

			if (CheckShootingCondition(pCmd)) {

				if (cfg::rage::autostop && !cfg::rage::betweenshots)
					AutoStop(pCmd, pWeapon->GetCSWpnData()->flMaxSpeed[0] * 0.10f);

				if (Hitchance(pTarget, pWeapon, shootAngle, ConfigHitChance(pWeapon), vecEyePosition)) {

					aimbotTarget = pTarget;
					targetMatrix = backtrackRecord ? backtrackRecord->pMatrix : pTarget->GetCachedBoneData().Base();

					static CConVar* recoilScale = i::ConVar->FindVar("weapon_recoil_scale");
					pCmd->iTickCount = lagcomp.FixTickCount(flTargetSimulation);
					pCmd->angViewPoint = (shootAngle -= (pLocal->GetAimPunch() * recoilScale->GetFloat()));
					pCmd->iButtons |= IN_ATTACK;

					bSendPacket = (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind)) ? bSendPacket : (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) ? g::bWaiting ? true : false : true;
					g::onetapV2ShotHiding = pCmd->iCommandNumber;
				}
				else {

					if ( ConfigAutoScope( pWeapon ) && IsAutoScopeable( pWeapon->GetItemDefinitionIndex( ) ) && !pLocal->IsScoped( ) ) //only scope if we have a scoped weapon and we arent scoped
						pCmd->iButtons |= IN_ZOOM;
				}
			}
		}
	}
}

Vector CRageBot::Hitscan(CBaseEntity* pLocal, CBaseEntity* pTarget, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition) {

	std::vector<std::pair<Vector, float>> vectorDamagePairs = {};

	/* Loop through enemy hitboxes and scale damage, then return a valid position to shoot to */
	for (int hitboxID : ConfigHitboxes(pWeapon)) {

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(pTarget->EntIndex());

		if (!pLog->pEntity)
			continue;

		if (pLog->pRecord.size() <= 2)
			continue;

		float flRadius = 0.f;
		if (!backtrackRecord) {

			Vector hitboxPosition = pTarget->GetHitboxPosition(hitboxID, pLog->pRecord.front().pMatrix, flRadius);
			std::vector<Vector> vecHitboxPosition = CreatePoints(pTarget, pLocal, pWeapon, hitboxPosition, flRadius, hitboxID, pTarget->EntIndex(), vecEyePosition);

			for (Vector currentPoint : vecHitboxPosition)
				if (float flDamage = autowall.GetDamage(pLocal, currentPoint, hitboxID, vecEyePosition); flDamage > ConfigMinimumDamage(pWeapon))
					/* Get highest damage */
					vectorDamagePairs.push_back(std::make_pair(currentPoint, flDamage));
		}
		else {

			Vector hitboxPosition = pTarget->GetHitboxPosition(hitboxID, backtrackRecord->pMatrix, flRadius);
			std::vector<Vector> vecHitboxPosition = CreatePoints(pTarget, pLocal, pWeapon, hitboxPosition, flRadius, hitboxID, pTarget->EntIndex(), vecEyePosition);

			Lagcompensation::LagRecord_t backup(pTarget);
			backtrackRecord->Apply(pTarget, false);
			for (Vector currentPoint : vecHitboxPosition)
				if ( float flDamage = autowall.GetDamage( pLocal, currentPoint, hitboxID, vecEyePosition ); flDamage > ConfigMinimumDamage( pWeapon ) ) {
					/* Get highest damage */
					vectorDamagePairs.push_back( std::make_pair( currentPoint, flDamage ) );
				}
			backup.Apply(pTarget, true);
		}
	}

	if (vectorDamagePairs.empty())
		return Vector(0, 0, 0);

	/* Sort highest damage */
	std::sort(vectorDamagePairs.begin(), vectorDamagePairs.end(), HighestDamage);

	return vectorDamagePairs.front().first;
}

CBaseEntity* CRageBot::SelectTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vector& vecEyePosition) {

	std::vector<CBaseEntity*> entityHealths = {};
	for (int i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		/* Sanity checks */
		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant())
			continue;

		/* Ignore teammates */
		if (pEntity->GetTeam() == pLocal->GetTeam())
			continue;

		/* Add targetable entity */
		entityHealths.push_back(pEntity);
	}

	/* Check if we have any record */
	if (entityHealths.empty())
		return nullptr;

	/* Sort lowest health entities */
	std::sort(entityHealths.begin(), entityHealths.end(), LowestHealth);

	/* Loop through saved entites */
	for (CBaseEntity* curEnt : entityHealths) {

		// get animation info
		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog( curEnt->EntIndex( ) );
		if ( !pLog->pEntity )
			continue;

		if (pLog->pRecord.size() <= 2)
			continue;

		/* Loop through the selected hitboxes while we can hit something */
		for (int hitboxID : ConfigHitboxes(pWeapon)) {
			/* Trace player with its current bonedata */
			if (autowall.CanHitFloatingPoint(pLocal, pWeapon, curEnt->GetHitboxPosition(hitboxID, pLog->pRecord.front().pMatrix), vecEyePosition, ConfigMinimumDamage(pWeapon))) {
				flTargetSimulation = curEnt->GetSimulationTime();
				backtrackRecord = nullptr;
				return curEnt;
			}
			else if (!pLog->pRecord.empty() && pLog->pRecord.size() >= 2) {
				if ( Lagcompensation::LagRecord_t* recordScan = &pLog->pRecord.at(min(pLog->pRecord.size() - 1, 12)); recordScan != nullptr) {
					if (autowall.CanHitFloatingPoint(pLocal, pWeapon, curEnt->GetHitboxPosition(hitboxID, curEnt->GetCachedBoneData().Base()), vecEyePosition, ConfigMinimumDamage(pWeapon))) {
						flTargetSimulation = recordScan->flSimulationTime;
						backtrackRecord = recordScan;
						return curEnt;
					}
				}
			}
		}
	}


	/* If we reach here, that means we cannot hit any player */
	/* TODO: backtrackable entity checks */
	return nullptr;
}

bool CRageBot::Hitchance(CBaseEntity* pEnt, CBaseCombatWeapon* pWeapon, Vector vecFrom, int iChance, Vector vecEyePosition) {

	float flFinalHitchance = 0;
	CCSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

	if (!pWeaponInfo)
		return false;

	Vector vecForward = Vector(0, 0, 0);
	Vector vecRight = Vector(0, 0, 0);
	Vector vecUp = Vector(0, 0, 0);

	M::AngleVectors(vecFrom, &vecForward, &vecRight, &vecUp);

	vecForward.Normalize();
	vecRight.Normalize();
	vecUp.Normalize();

	//auto is_special_weapon = pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AWP || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1 || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20 || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SSG08;

	static bool bSetupSpreadValues = true;
	static float flSpreadValues[256][6];

	if (bSetupSpreadValues)
	{
		bSetupSpreadValues = false;

		for (auto i = 0; i < 256; ++i)
		{
			M::RandomSeed(i + 1);

			float a = M::RandomFloat(0.0f, 1.0f);
			float b = M::RandomFloat(0.0f, 6.283185307f);
			float c = M::RandomFloat(0.0f, 1.0f);
			float d = M::RandomFloat(0.0f, 6.283185307f);

			flSpreadValues[i][0] = a;
			flSpreadValues[i][1] = c;

			auto flSinB = 0.0f, flCosB = 0.0f;
			M::SinCos(b, &flSinB, &flCosB);

			auto flSinD = 0.0f, flCosD = 0.0f;
			M::SinCos(b, &flSinD, &flCosD);

			flSpreadValues[i][2] = flSinB;
			flSpreadValues[i][3] = flCosB;
			flSpreadValues[i][4] = flSinD;
			flSpreadValues[i][5] = flCosD;
		}
	}

	int iHits = 0;

	for (auto i = 0; i < 256; ++i)
	{
		float flInacc = flSpreadValues[i][0] * pWeapon->GetInaccuracy();
		float flSpread = flSpreadValues[i][1] * pWeapon->GetSpread();

		float flSpreadX = flSpreadValues[i][3] * flInacc + flSpreadValues[i][5] * flSpread;
		float flSpreadY = flSpreadValues[i][2] * flInacc + flSpreadValues[i][4] * flSpread;

		Vector vecDirection = Vector(0, 0, 0);

		vecDirection.x = vecForward.x + vecRight.x * flSpreadX + vecUp.x * flSpreadY;
		vecDirection.y = vecForward.y + vecRight.y * flSpreadX + vecUp.y * flSpreadY;
		vecDirection.z = vecForward.z + vecRight.z * flSpreadX + vecUp.z * flSpreadY; //-V778

		Vector vecEnd = vecEyePosition + vecDirection * pWeaponInfo->flRange;

		Trace_t Trace;

		i::EngineTrace->ClipRayToEntity(Ray_t(vecEyePosition, vecEnd), MASK_SHOT | CONTENTS_GRATE, pEnt, &Trace);

		if (Trace.pHitEntity == pEnt)
			iHits++;
	}

	flFinalHitchance = (int)((float)iHits / 2.56f);

	if (flFinalHitchance > iChance)
		return true;

	return false;
}

void CRageBot::AutoStop(CUserCmd* pCmd, float IdealSpeed) {

	// Credit to @Monthyx
	// Fast stop source from obelus

	//if (g::pLocal->GetWeapon()->GetItemDefinitionIndex() == (WEAPON_SSG08 || WEAPON_AWP))
	//	if (!CheckShootingCondition(pCmd))
	//		return;

	if (!(g::pLocal->GetFlags() & FL_ONGROUND))
		return;

	pCmd->iButtons &= ~IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;

	// Get the ideal speed for shooting (playstyle)
	Vector velocity = g::pLocal->GetVelocity();
	Vector direction;
	Vector real_view;

	if (IdealSpeed > velocity.Length2D())
		return;

	M::VectorAngles(velocity, direction);
	i::EngineClient->GetViewAngles(real_view);

	direction.y = real_view.y - direction.y;

	Vector forward;
	M::AngleVectors(direction, &forward);

	static auto cl_forwardspeed = i::ConVar->FindVar("cl_forwardspeed");
	static auto cl_sidespeed = i::ConVar->FindVar("cl_sidespeed");

	auto negative_forward_speed = -cl_forwardspeed->GetFloat();
	auto negative_side_speed = -cl_sidespeed->GetFloat();

	auto negative_forward_direction = forward * negative_forward_speed;
	auto negative_side_direction = forward * negative_side_speed;

	pCmd->flForwardMove = negative_forward_direction.x;
	pCmd->flSideMove = negative_side_direction.y;
}

std::pair<int, int> CRageBot::ConfigMultipoint(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return std::make_pair(cfg::rage::autoHeadPoints, cfg::rage::autoBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return std::make_pair(cfg::rage::scoutHeadPoints, cfg::rage::scoutBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return std::make_pair(cfg::rage::awpHeadPoints, cfg::rage::awpBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return std::make_pair(cfg::rage::heavypistolHeadPoints, cfg::rage::heavypistolBodyPoints);
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return std::make_pair(cfg::rage::pistolHeadPoints, cfg::rage::pistolBodyPoints);
	}
	else {
		return std::make_pair(cfg::rage::etcHeadPoints, cfg::rage::etcBodyPoints);
	}
}

int CRageBot::ConfigMinimumDamage(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::autoMindmg;
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::scoutMindmg;
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::awpMindmg;
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::heavypistolMindmg;
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::pistolMindmg;
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		return 100;
	}
	else {
		return cfg::rage::etcMindmg;
	}

}

int CRageBot::ConfigHitChance(CBaseCombatWeapon* pWeapon) {

	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		return cfg::rage::autoHitchance;
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		return cfg::rage::scoutHitchance;
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		return cfg::rage::awpHitchance;
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		return cfg::rage::heavypistolHitchance;
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		return cfg::rage::pistolHitchance;
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		return 72;
	}
	else {
		return cfg::rage::etcHitchance;
	}
}

int CRageBot::ConfigAutoScope( CBaseCombatWeapon* pWeapon ) {

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

std::vector<int> CRageBot::ConfigHitboxes(CBaseCombatWeapon* pWeapon) {

	// "head", "upper chest", "lower chest", "stomach", "arms", "legs"

	std::vector<int> vecHitboxes;
	auto iDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	static auto AddHitbox = [](int index, std::vector<int>& vecHitboxList) {

		if (index == 0) {
			vecHitboxList.push_back(HITBOX_HEAD);
		}
		if (index == 1) {
			vecHitboxList.push_back(HITBOX_UPPER_CHEST);
		}
		if (index == 2) {
			vecHitboxList.push_back(HITBOX_CHEST);
			vecHitboxList.push_back(HITBOX_THORAX);
		}
		if (index == 3) {
			vecHitboxList.push_back(HITBOX_STOMACH);
			vecHitboxList.push_back(HITBOX_PELVIS);
		}
		if (index == 4) {
			vecHitboxList.push_back(HITBOX_RIGHT_FOREARM);
			vecHitboxList.push_back(HITBOX_LEFT_FOREARM);

			vecHitboxList.push_back(HITBOX_LEFT_UPPER_ARM);
			vecHitboxList.push_back(HITBOX_RIGHT_UPPER_ARM);

			vecHitboxList.push_back(HITBOX_RIGHT_HAND);
			vecHitboxList.push_back(HITBOX_LEFT_HAND);
		}
		if (index == 5) {
			vecHitboxList.push_back(HITBOX_RIGHT_THIGH);
			vecHitboxList.push_back(HITBOX_LEFT_THIGH);

			vecHitboxList.push_back(HITBOX_RIGHT_CALF);
			vecHitboxList.push_back(HITBOX_LEFT_CALF);

			vecHitboxList.push_back(HITBOX_RIGHT_FOOT);
			vecHitboxList.push_back(HITBOX_LEFT_FOOT);
		}
	};

	if (iDefinitionIndex == WEAPON_SCAR20 || iDefinitionIndex == WEAPON_G3SG1) {
		for (int i = 0; i < sizeof(cfg::rage::autoHitboxes); i++) {
			if (cfg::rage::autoHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_SSG08) {
		for (int i = 0; i < sizeof(cfg::rage::scoutHitboxes); i++) {
			if (cfg::rage::scoutHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_AWP) {
		for (int i = 0; i < sizeof(cfg::rage::awpHitboxes); i++) {
			if (cfg::rage::awpHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_REVOLVER || iDefinitionIndex == WEAPON_DEAGLE) {
		for (int i = 0; i < sizeof(cfg::rage::heavypistolHitboxes); i++) {
			if (cfg::rage::heavypistolHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_USP_SILENCER || iDefinitionIndex == WEAPON_HKP2000 || iDefinitionIndex == WEAPON_ELITE || iDefinitionIndex == WEAPON_P250 || iDefinitionIndex == WEAPON_FIVESEVEN || iDefinitionIndex == WEAPON_CZ75A || iDefinitionIndex == WEAPON_GLOCK || iDefinitionIndex == WEAPON_TEC9) {
		for (int i = 0; i < sizeof(cfg::rage::pistolHitboxes[i]); i++) {
			if (cfg::rage::pistolHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}
	else if (iDefinitionIndex == WEAPON_TASER) {
		AddHitbox(2, vecHitboxes);
		AddHitbox(3, vecHitboxes);
	}
	else {
		for (int i = 0; i < sizeof(cfg::rage::etcHitboxes); i++) {
			if (cfg::rage::etcHitboxes[i])
				AddHitbox(i, vecHitboxes);
		}
	}

	return vecHitboxes;
}

bool CheckShootingCondition(CUserCmd* pCmd) {

	auto local = g::pLocal;
	auto pWeapon = g::pLocal->GetWeapon();

	if (!local || !pWeapon)
		return false;

	auto time = TICKS_TO_TIME(local->GetTickBase());

	const auto info = pWeapon->GetCSWpnData();

	if (!info)
		return false;

	const auto is_zeus = pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_TASER;

	if (pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_C4)
		return false;

	if (pWeapon->GetAmmo() < 1)
		return false;

	if (pWeapon->GetNextPrimaryAttack() > time || pWeapon->GetNextSecondaryAttack() > time)
		return false;

	return true;
}

std::vector<Vector> CRageBot::CreatePoints(CBaseEntity* pTarget, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vector vecAngle, float flRadius, int nHitbox, int entIndex, Vector vecEyePosition) {

	if (flRadius <= 0)
		return std::vector<Vector>{vecAngle};

	std::pair<int, int> multiPoints = ConfigMultipoint(g::pLocal->GetWeapon());
	std::vector<Vector> points;
	int iMinimumDamage = ConfigMinimumDamage(pWeapon);

	int* pHeadPoints = &multiPoints.first;
	int* pBodyPoints = &multiPoints.second;
	Vector vecOriginalAngle = vecAngle;

	if (nHitbox == HITBOX_HEAD) {

		/* First check if we can hit the hitbox middle */
		points.push_back(vecOriginalAngle);

		/* Check every point of the hitbox by making a 3D cube inside the hitbox */
		/* I'm too lazy to actually make it perfectly round like hitboxes are */

		float flHeadDistance = flRadius * (*pHeadPoints / 150.f);

		/* Single axises */
		{
			points.push_back(vecOriginalAngle + Vector(flHeadDistance, 0.f, 0.f));
			points.push_back(vecOriginalAngle + Vector(0.f, flHeadDistance, 0.f));
			points.push_back(vecOriginalAngle + Vector(0.f, 0.f, flHeadDistance));
			points.push_back(vecOriginalAngle - Vector(0.f, flHeadDistance, 0.f));
			points.push_back(vecOriginalAngle - Vector(flHeadDistance, 0.f, 0.f));

		}

		return points;
		//visual::headPoints[pTarget->EntIndex()][nHitbox] = points;

		//for (Vector vecPoint : points) {

		//	if (autowall.CanHitFloatingPoint(pLocal, pWeapon, vecPoint, vecEyePosition, iMinimumDamage)) {
		//		visual::selectedPoint[pTarget->EntIndex()][nHitbox] = vecPoint;
		//		return vecPoint;
		//	}
		//}
	}
	else {

		/* First check if we can hit the hitbox middle */
		points.push_back(vecOriginalAngle);

		/* Check every point of the hitbox by making a 3D cube inside the hitbox */
		/* I'm too lazy to actually make it perfectly round like hitboxes are */

		float flBodyDistance = flRadius * (*pBodyPoints / 150.f);

		/* Single axises */
		{
			points.push_back(vecOriginalAngle + Vector(flBodyDistance, 0.f, 0.f));
			points.push_back(vecOriginalAngle + Vector(0.f, flBodyDistance, 0.f));
			points.push_back(vecOriginalAngle - Vector(0.f, flBodyDistance, 0.f));
			points.push_back(vecOriginalAngle - Vector(flBodyDistance, 0.f, 0.f));
		}

		return points;
		/*visual::bodyPoints[pTarget->EntIndex()][nHitbox] = points;

		for (Vector vecPoint : points)
			if (autowall.CanHitFloatingPoint(pLocal, pWeapon, vecPoint, vecEyePosition, iMinimumDamage)) {
				visual::selectedPoint[pTarget->EntIndex()][nHitbox] = vecPoint;
				return vecPoint;
			}*/
	}

	return std::vector<Vector>{vecAngle};
}