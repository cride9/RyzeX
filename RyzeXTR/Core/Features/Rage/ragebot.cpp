#include "ragebot.h"
#include "../../globals.h"
#include "autowall.h"
#include "../../SDK/math.h"
#include "../Visuals/ESP.h"
#include "doubletap.h"
#include "Animations/resolver.h"

bool CheckShootingCondition(CUserCmd* pCmd);

void CRageBot::CreateMove(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket) {

	if (!pLocal || !cfg::rage::enable)
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (!pWeapon || !pWeapon->GetCSWpnData())
		return;

	for (int i = 0; i < 65; i++) {

		CBaseEntity* pEnt = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEnt || !pEnt->IsAlive() || pEnt->IsDormant() || pEnt->GetTeam() == pLocal->GetTeam() || pEnt->HasImmunity())
			continue;

		if (lagcomp.deqLagRecords[i].size() < 2)
			continue;

		float flSimtime = 0.f;

		Vector vecHitboxPosition = HitScan(pEnt, flSimtime, pWeapon);

		if (vecHitboxPosition == Vector(0.f, 0.f, 0.f))
			continue;

		float flCurrentTime = TICKS_TO_TIME(prediction.GetTickBase(pCmd, pLocal));

		if (CheckShootingCondition(pCmd)) {

			if (!GetAsyncKeyState(cfg::antiaim::idealTickBind))
				AutoStop(pCmd, pWeapon->GetCSWpnData()->flMaxSpeed[0] * 0.10f);
			else
				misc::bRetreat = true;

			Vector vecAimPoint = M::CalcAngle(pLocal->GetEyePosition(), vecHitboxPosition).Normalize().Clamp();

			if (CanShoot(pEnt, pWeapon, vecAimPoint, ConfigHitChance(pWeapon))) {

				misc::bRetreat = true;
				static CConVar* weapon_recoil_scale = i::ConVar->FindVar("weapon_recoil_scale");
				vecAimPoint -= (pLocal->GetAimPunch() * weapon_recoil_scale->GetFloat());

				pCmd->angViewPoint = vecAimPoint;
				pCmd->iButtons |= IN_ATTACK;

				pCmd->iTickCount = TIME_TO_TICKS(flSimtime + lagcomp.LerpTime());
				bSendPacket = (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind)) ? bSendPacket : (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) ? g::bWaiting ? true : false : true;;
			}
		}
	}
}

Vector CRageBot::HitScan(CBaseEntity* pEnt, float& flSimulationTime, CBaseCombatWeapon* pWeapon) {

	auto pRecord = &lagcomp.deqLagRecords[pEnt->EntIndex()].front();

	int iMinimumDamage = ConfigMinimumDamage(pWeapon);
	auto vecHitboxes = ConfigHitboxes(pWeapon);
	std::pair<int, int> pMultipoints = ConfigMultipoint(pWeapon);

	std::vector<float> DamageList;
	std::vector<std::pair<Vector, LagComp::playerrecord_t*>> PositionList;

	// loop trough all hitbox
	for (auto hitboxID : vecHitboxes) {

		float flRadius = 0.f;
		Vector vecHitboxPosition = pEnt->GetHitboxPosition(hitboxID, pRecord->matrix, flRadius);

		if (flRadius != 0)
			vecHitboxPosition = CreatePoints(vecHitboxPosition, flRadius, hitboxID, pEnt->EntIndex());

		float flDamage = autowall.GetDamage(g::pLocal, vecHitboxPosition, hitboxID);

		if (flDamage > pEnt->GetHealth() + 10) {

			resolver::targetedRecord = pRecord;
			return vecHitboxPosition;
		}

		if (flDamage < iMinimumDamage) {

			std::vector<LagComp::playerrecord_t*> validBacktrackRecords;

			for (int tick = 0; tick < lagcomp.deqLagRecords[pEnt->EntIndex()].size(); tick++) {

				auto pBacktrack = &lagcomp.deqLagRecords[pEnt->EntIndex()].at(tick);

				if (pBacktrack->IsValid(pBacktrack->flSimulationTime, pBacktrack->bValid)) {

					validBacktrackRecords.push_back(pBacktrack);
				}
			}

			if (validBacktrackRecords.empty())
				continue;

			validBacktrackRecords.front()->Apply(pEnt);
			vecHitboxPosition = pEnt->GetHitboxPosition(hitboxID, validBacktrackRecords.front()->matrix, flRadius);

			if (flRadius != 0)
				vecHitboxPosition = CreatePoints(vecHitboxPosition, flRadius, hitboxID, pEnt->EntIndex());

			flDamage = autowall.GetDamage(g::pLocal, vecHitboxPosition, hitboxID);

			if (flDamage > pEnt->GetHealth() + 10) {

				resolver::targetedRecord = validBacktrackRecords.front();
				return vecHitboxPosition;
			}

			if (flDamage > iMinimumDamage) {

				resolver::targetedRecord = validBacktrackRecords.front();
				DamageList.push_back(flDamage);
				PositionList.push_back({ vecHitboxPosition, validBacktrackRecords.front() });
			}

			continue;
		}

		DamageList.push_back(flDamage);
		PositionList.push_back({ vecHitboxPosition, pRecord});
	}

	if (!DamageList.empty()) {
		int highestDamage = 0;
		for (int i = 1; i < DamageList.size(); i++)
			if (DamageList.at(highestDamage) < DamageList.at(i))
				highestDamage = i;

		resolver::targetedRecord = PositionList.at(highestDamage).second;
		flSimulationTime = PositionList.at(highestDamage).second->flSimulationTime;
		return PositionList.at(highestDamage).first;
	}

	return Vector(0.f, 0.f, 0.f);
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
		return 100;
	}
	else {
		return cfg::rage::etcHitchance;
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
		else if (index == 1) {
			vecHitboxList.push_back(HITBOX_UPPER_CHEST);
		}
		else if (index == 2) {
			vecHitboxList.push_back(HITBOX_CHEST);
			vecHitboxList.push_back(HITBOX_THORAX);
		}
		else if (index == 3) {
			vecHitboxList.push_back(HITBOX_STOMACH);
			vecHitboxList.push_back(HITBOX_PELVIS);
		}
		else if (index == 4) {
			vecHitboxList.push_back(HITBOX_RIGHT_FOREARM);
			vecHitboxList.push_back(HITBOX_LEFT_FOREARM);

			vecHitboxList.push_back(HITBOX_LEFT_UPPER_ARM);
			vecHitboxList.push_back(HITBOX_RIGHT_UPPER_ARM);

			vecHitboxList.push_back(HITBOX_RIGHT_HAND);
			vecHitboxList.push_back(HITBOX_LEFT_HAND);
		}
		else if (index == 5) {
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

	if (!g::pLocal->GetWeapon())
		return false;

	const auto time = TICKS_TO_TIME(prediction.GetTickBase(g::pCmd, g::pLocal));

	const auto info = g::pLocal->GetWeapon()->GetCSWpnData();

	if (!info)
		return false;

	if (g::pLocal->GetNextAttack() > time || g::pLocal->GetWeapon()->GetNextPrimaryAttack() > time)
		return false;

	if (g::pLocal->GetWeapon()->GetAmmo() < 1)
		return false;

	if (pCmd->iButtons & IN_ATTACK)
		return false;

	return true;
}

bool CRageBot::CanShoot(CBaseEntity* pEnt, CBaseCombatWeapon* pWeapon, Vector vecFrom, int iChance) {

	auto final_hitchance = 0;
	auto weapon_info = pWeapon->GetCSWpnData();

	if (!weapon_info)
		return false;

	auto forward = Vector(0, 0, 0);
	auto right = Vector(0, 0, 0);
	auto up = Vector(0, 0, 0);

	M::AngleVectors(vecFrom, &forward, &right, &up);

	forward.Normalize();
	right.Normalize();
	up.Normalize();

	//auto is_special_weapon = pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AWP || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1 || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20 || pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SSG08;

	static auto setup_spread_values = true;
	static float spread_values[256][6];

	if (setup_spread_values)
	{
		setup_spread_values = false;

		for (auto i = 0; i < 256; ++i)
		{
			M::RandomSeed(i + 1);

			auto a = M::RandomFloat(0.0f, 1.0f);
			auto b = M::RandomFloat(0.0f, 6.283185307f);
			auto c = M::RandomFloat(0.0f, 1.0f);
			auto d = M::RandomFloat(0.0f, 6.283185307f);

			spread_values[i][0] = a;
			spread_values[i][1] = c;

			auto sin_b = 0.0f, cos_b = 0.0f;
			M::SinCos(b, &sin_b, &cos_b);

			auto sin_d = 0.0f, cos_d = 0.0f;
			M::SinCos(b, &sin_d, &cos_d);

			spread_values[i][2] = sin_b;
			spread_values[i][3] = cos_b;
			spread_values[i][4] = sin_d;
			spread_values[i][5] = cos_d;
		}
	}

	auto hits = 0;

	for (auto i = 0; i < 256; ++i)
	{
		auto inaccuracy = spread_values[i][0] * pWeapon->GetInaccuracy();
		auto spread = spread_values[i][1] * pWeapon->GetSpread();

		auto spread_x = spread_values[i][3] * inaccuracy + spread_values[i][5] * spread;
		auto spread_y = spread_values[i][2] * inaccuracy + spread_values[i][4] * spread;

		auto direction = Vector(0, 0, 0);

		direction.x = forward.x + right.x * spread_x + up.x * spread_y;
		direction.y = forward.y + right.y * spread_x + up.y * spread_y;
		direction.z = forward.z + right.z * spread_x + up.z * spread_y; //-V778

		auto end = g::pLocal->GetEyePosition() + direction * weapon_info->flRange;

		Trace_t Trace;

		i::EngineTrace->ClipRayToEntity(Ray_t(g::pLocal->GetEyePosition(), end), MASK_SHOT | CONTENTS_GRATE, pEnt, &Trace);

		if (Trace.pHitEntity == pEnt)
			hits++;
	}

	final_hitchance = (int)((float)hits / 2.56f);

	if (final_hitchance > iChance)
		return true;

	return false;
}

void CRageBot::AutoStop(CUserCmd* pCmd, float IdealSpeed) {

	// Credit to @Monthyx
	// Fast stop source from obelus

	if (g::pLocal->GetWeapon()->GetItemDefinitionIndex() == (WEAPON_SSG08 || WEAPON_AWP))
		if (!CheckShootingCondition(pCmd))
			return;

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

Vector CRageBot::CreatePoints(Vector vecAngle, float flRadius, int nHitbox, int entIndex) {

	if (flRadius <= 0)
		return vecAngle;

	std::pair<int, int> multiPoints = ConfigMultipoint(g::pLocal->GetWeapon());
	static int multiPoint[18];

	if (nHitbox == HITBOX_HEAD) {

		switch (multiPoint[nHitbox]) {
			
		case 0: vecAngle;
			g::multiPoint[entIndex][nHitbox][0] = vecAngle;
			break;

		case 1: vecAngle.x += (flRadius * (multiPoints.first / 200.f)); // left
			vecAngle.y += (flRadius * (multiPoints.first / 200.f));
			g::multiPoint[entIndex][nHitbox][1] = vecAngle;
			break;

		case 2: vecAngle.x -= (flRadius * (multiPoints.first / 200.f)); // right
			vecAngle.y -= (flRadius * (multiPoints.first / 200.f));
			g::multiPoint[entIndex][nHitbox][2] = vecAngle;
			break;

		case 3: vecAngle.z += (flRadius * (multiPoints.first / 200.f)); // up
			g::multiPoint[entIndex][nHitbox][3] = vecAngle;
			break;

		case 4: vecAngle.x += (flRadius * (multiPoints.first / 200.f)); // upper left
				vecAngle.y += (flRadius * (multiPoints.first / 200.f));
				vecAngle.z += (flRadius * (multiPoints.first / 200.f));
				g::multiPoint[entIndex][nHitbox][4] = vecAngle;
			break;

		case 5: vecAngle.x -= (flRadius * (multiPoints.first / 200.f)); // upper right
				vecAngle.y -= (flRadius * (multiPoints.first / 200.f));
				vecAngle.z += (flRadius * (multiPoints.first / 200.f));
				g::multiPoint[entIndex][nHitbox][5] = vecAngle;
			break;

		case 6: vecAngle.x += (flRadius * (multiPoints.first / 200.f)); // downer left
			vecAngle.y += (flRadius * (multiPoints.first / 200.f));
			vecAngle.z -= (flRadius * (multiPoints.first / 200.f));
			g::multiPoint[entIndex][nHitbox][6] = vecAngle;
			break;

		case 7: vecAngle.x -= (flRadius * (multiPoints.first / 200.f)); // downer right
			vecAngle.y -= (flRadius * (multiPoints.first / 200.f));
			vecAngle.z -= (flRadius * (multiPoints.first / 200.f));
			g::multiPoint[entIndex][nHitbox][7] = vecAngle;
			break;
		}
		multiPoint[nHitbox]++;		

		if (multiPoint[nHitbox] > 7)
			multiPoint[nHitbox] = 0;
	}
	else {

		switch (multiPoint[nHitbox]) {

		case 0: vecAngle;
			g::multiPoint[entIndex][nHitbox][0] = vecAngle;
			break;

		case 1: vecAngle.x += (flRadius * (multiPoints.second / 200.f)); // left
			vecAngle.y += (flRadius * (multiPoints.second / 200.f));
			g::multiPoint[entIndex][nHitbox][1] = vecAngle;
			break;

		case 2: vecAngle.x -= (flRadius * (multiPoints.second / 200.f)); // right
			vecAngle.y -= (flRadius * (multiPoints.second / 200.f));
			g::multiPoint[entIndex][nHitbox][2] = vecAngle;
			break;


		case 3: vecAngle.z += (flRadius * (multiPoints.second / 200.f)); // up
			g::multiPoint[entIndex][nHitbox][3] = vecAngle;
			break;

		case 4: vecAngle.z -= (flRadius * (multiPoints.second / 200.f)); // down
			g::multiPoint[entIndex][nHitbox][4] = vecAngle;
			break;

		case 5: vecAngle.x += (flRadius * (multiPoints.second / 200.f)); // upper left
				vecAngle.y += (flRadius * (multiPoints.second / 200.f));
				vecAngle.z += (flRadius * (multiPoints.second / 200.f));
				g::multiPoint[entIndex][nHitbox][5] = vecAngle;
			break;

		case 6: vecAngle.x -= (flRadius * (multiPoints.second / 200.f)); // upper right
				vecAngle.y -= (flRadius * (multiPoints.second / 200.f));
				vecAngle.z += (flRadius * (multiPoints.second / 200.f));
				g::multiPoint[entIndex][nHitbox][6] = vecAngle;
			break;

		case 7: vecAngle.x += (flRadius * (multiPoints.second / 200.f)); // downer left
				vecAngle.y += (flRadius * (multiPoints.second / 200.f));
				vecAngle.z -= (flRadius * (multiPoints.second / 200.f));
				g::multiPoint[entIndex][nHitbox][7] = vecAngle;
			break;

		case 8: vecAngle.x -= (flRadius * (multiPoints.second / 200.f)); // downer right
				vecAngle.y -= (flRadius * (multiPoints.second / 200.f));
				vecAngle.z -= (flRadius * (multiPoints.second / 200.f));
				g::multiPoint[entIndex][nHitbox][8] = vecAngle;
			break;
		}
		multiPoint[nHitbox]++;

		if (multiPoint[nHitbox] > 8)
			multiPoint[nHitbox] = 0;
	}

	return vecAngle;
}